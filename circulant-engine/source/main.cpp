#include "copt_oracle.h"
#include "dynamic_bitset.h"

#include <algorithm>
#include <array>
#include <chrono>
#include <cstdint>
#include <cstdlib>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <limits>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>

namespace {

enum class SearchMode { FAST_ONLY, CHECKER_ONLY, HYBRID };

using CompactWord = unsigned __int128;

struct Config {
    int order = 0;
    int blue_target = 0;
    int red_target = 0;
    SearchMode mode = SearchMode::HYBRID;
    std::uint64_t fast_budget = 1000;
    double copt_timeout = -1.0;
    double max_seconds = -1.0;
    bool enumerate_all = false;
    bool colour_symmetry = true;
    std::uint64_t max_solutions = 1;
    std::uint64_t trace_limit = 0;
    bool detailed_stats = false;
    int split_depth = 0;
    std::uint64_t split_count = 1;
    std::uint64_t split_index = 0;
    std::string replay_matrix;
};

struct FastResult {
    OracleStatus status = OracleStatus::UNKNOWN;
    std::uint64_t work = 0;
    bool work_counted = true;
};

class FastCliqueOracle {
public:
    FastCliqueOracle(int order, int maximum_target)
        : order_(order),
          common_(static_cast<std::size_t>(order)),
          rotated_(static_cast<std::size_t>(order)),
          rotate_scratch_(static_cast<std::size_t>(order)) {
        const int levels = std::max(2, maximum_target + 2);
        remaining_.reserve(static_cast<std::size_t>(levels));
        shifted_.reserve(static_cast<std::size_t>(levels));
        child_.reserve(static_cast<std::size_t>(levels));
        for (int i = 0; i < levels; ++i) {
            remaining_.emplace_back(static_cast<std::size_t>(order));
            shifted_.emplace_back(static_cast<std::size_t>(order));
            child_.emplace_back(static_cast<std::size_t>(order));
        }
    }

    const DynamicBitset &common_neighbours(const DynamicBitset &neighbourhood, int distance) {
        rotated_.rotate_left_from(
            neighbourhood, static_cast<std::size_t>(distance), rotate_scratch_);
        common_.and_from(neighbourhood, rotated_);
        return common_;
    }

    FastResult query_edge(
        const DynamicBitset &neighbourhood,
        int distance,
        int forbidden_clique_size,
        std::uint64_t budget,
        std::vector<int> *witness = nullptr) {
        const int residual_target = forbidden_clique_size - 2;
        if (residual_target <= 0) {
            FastResult result;
            result.status = OracleStatus::FOUND;
            return result;
        }
        if (order_ <= 64) {
            const std::uint64_t adjacency = neighbourhood.word(0);
            const std::uint64_t candidates =
                adjacency & rotate_left(adjacency, distance, order_);
            return query_scalar(candidates, adjacency, residual_target, budget, witness);
        }
        if (order_ <= 128) {
            const WideWord adjacency =
                static_cast<WideWord>(neighbourhood.word(0)) |
                (static_cast<WideWord>(neighbourhood.word(1)) << 64U);
            const WideWord candidates =
                adjacency & rotate_left(adjacency, distance, order_);
            return query_scalar(candidates, adjacency, residual_target, budget, witness);
        }
        return query_set(
            common_neighbours(neighbourhood, distance),
            neighbourhood,
            residual_target,
            budget,
            witness);
    }

    FastResult query_edge_compact(
        CompactWord adjacency,
        int distance,
        int forbidden_clique_size,
        std::uint64_t budget,
        std::vector<int> *witness = nullptr) {
        const int residual_target = forbidden_clique_size - 2;
        if (residual_target <= 0) {
            FastResult result;
            result.status = OracleStatus::FOUND;
            return result;
        }
        const CompactWord candidates =
            adjacency & rotate_left(adjacency, distance, order_);
        return query_scalar(candidates, adjacency, residual_target, budget, witness);
    }

    FastResult query_global(
        const DynamicBitset &neighbourhood,
        int forbidden_clique_size,
        std::uint64_t budget = 0) {
        const int residual_target = forbidden_clique_size - 1;
        if (residual_target <= 0) {
            FastResult result;
            result.status = OracleStatus::FOUND;
            return result;
        }
        if (order_ <= 64) {
            return query_scalar(
                neighbourhood.word(0), neighbourhood.word(0), residual_target, budget, nullptr);
        }
        if (order_ <= 128) {
            const WideWord adjacency =
                static_cast<WideWord>(neighbourhood.word(0)) |
                (static_cast<WideWord>(neighbourhood.word(1)) << 64U);
            return query_scalar(adjacency, adjacency, residual_target, budget, nullptr);
        }
        return query_set(neighbourhood, neighbourhood, residual_target, budget, nullptr);
    }

    FastResult query_global_compact(
        CompactWord adjacency,
        int forbidden_clique_size,
        std::uint64_t budget = 0) {
        const int residual_target = forbidden_clique_size - 1;
        if (residual_target <= 0) {
            FastResult result;
            result.status = OracleStatus::FOUND;
            return result;
        }
        return query_scalar(adjacency, adjacency, residual_target, budget, nullptr);
    }

private:
    using WideWord = CompactWord;

    template <typename Word>
    static Word logical_mask(int width) {
        const int capacity = static_cast<int>(8U * sizeof(Word));
        return width == capacity ? ~Word(0) : (Word(1) << width) - Word(1);
    }

    template <typename Word>
    static Word rotate_left(Word value, int shift, int width) {
        if (shift == 0) return value;
        return ((value << shift) | (value >> (width - shift))) & logical_mask<Word>(width);
    }

    static int popcount(std::uint64_t value) {
        return __builtin_popcountll(value);
    }

    static int popcount(WideWord value) {
        return __builtin_popcountll(static_cast<std::uint64_t>(value)) +
               __builtin_popcountll(static_cast<std::uint64_t>(value >> 64U));
    }

    static int pop_first(std::uint64_t &value) {
        const int bit = __builtin_ctzll(value);
        value &= value - 1ULL;
        return bit;
    }

    static int pop_first(WideWord &value) {
        const std::uint64_t low = static_cast<std::uint64_t>(value);
        const int bit = low != 0ULL
            ? __builtin_ctzll(low)
            : 64 + __builtin_ctzll(static_cast<std::uint64_t>(value >> 64U));
        value &= value - WideWord(1);
        return bit;
    }

    template <typename Word>
    FastResult query_scalar(
        Word candidates,
        Word neighbourhood,
        int target,
        std::uint64_t budget,
        std::vector<int> *witness) {
        budget_ = budget;
        work_ = 0;

        FastResult result;
        if (budget == 0) {
            result.work_counted = false;
            result.status = witness == nullptr
                ? (clique_dfs_unbounded<false>(candidates, neighbourhood, target, 0)
                       ? OracleStatus::FOUND : OracleStatus::NOT_FOUND_EXACT)
                : (clique_dfs_unbounded<true>(candidates, neighbourhood, target, 0)
                       ? OracleStatus::FOUND : OracleStatus::NOT_FOUND_EXACT);
        } else {
            result.status = clique_dfs_scalar(candidates, neighbourhood, target, 0);
            result.work = work_;
        }
        if (witness != nullptr && result.status == OracleStatus::FOUND) {
            witness->assign(path_fixed_.begin(), path_fixed_.begin() + target);
        }
        return result;
    }

    template <bool CaptureWitness, typename Word>
    bool clique_dfs_unbounded(Word candidates, Word neighbourhood, int need, int level) {
        if (need <= 0) return true;
        if (popcount(candidates) < need) return false;
        if (need == 1) {
            if (CaptureWitness) {
                path_fixed_[static_cast<std::size_t>(level)] = pop_first(candidates);
            }
            return true;
        }

        while (candidates != Word(0)) {
            const int vertex = pop_first(candidates);
            const Word next = candidates & (neighbourhood << vertex);
            if (CaptureWitness) path_fixed_[static_cast<std::size_t>(level)] = vertex;
            if (clique_dfs_unbounded<CaptureWitness>(
                    next, neighbourhood, need - 1, level + 1)) {
                return true;
            }
        }
        return false;
    }

    template <typename Word>
    OracleStatus clique_dfs_scalar(Word candidates, Word neighbourhood, int need, int level) {
        if (need <= 0) return OracleStatus::FOUND;
        if (popcount(candidates) < need) return OracleStatus::NOT_FOUND_EXACT;
        if (budget_ != 0 && work_ >= budget_) return OracleStatus::UNKNOWN;

        if (need == 1) {
            ++work_;
            path_fixed_[static_cast<std::size_t>(level)] = pop_first(candidates);
            return OracleStatus::FOUND;
        }

        while (candidates != Word(0)) {
            if (budget_ != 0 && work_ >= budget_) return OracleStatus::UNKNOWN;
            const int vertex = pop_first(candidates);
            ++work_;
            const Word next = candidates & (neighbourhood << vertex);
            path_fixed_[static_cast<std::size_t>(level)] = vertex;
            const OracleStatus child_status =
                clique_dfs_scalar(next, neighbourhood, need - 1, level + 1);
            if (child_status != OracleStatus::NOT_FOUND_EXACT) return child_status;
        }
        return OracleStatus::NOT_FOUND_EXACT;
    }

    FastResult query_set(
        const DynamicBitset &candidates,
        const DynamicBitset &neighbourhood,
        int target,
        std::uint64_t budget,
        std::vector<int> *witness) {
        neighbourhood_ = &neighbourhood;
        budget_ = budget;
        work_ = 0;
        capture_witness_ = witness != nullptr;
        path_.clear();

        FastResult result;
        result.status = clique_dfs(candidates, target, 0);
        result.work = work_;
        if (witness != nullptr && result.status == OracleStatus::FOUND) *witness = path_;
        return result;
    }

    OracleStatus clique_dfs(const DynamicBitset &candidates, int need, int level) {
        if (need <= 0) return OracleStatus::FOUND;
        if (candidates.count() < static_cast<std::size_t>(need)) {
            return OracleStatus::NOT_FOUND_EXACT;
        }
        if (budget_ != 0 && work_ >= budget_) return OracleStatus::UNKNOWN;

        DynamicBitset &remaining = remaining_[static_cast<std::size_t>(level)];
        remaining.copy_from(candidates);

        if (need == 1) {
            const int vertex = remaining.pop_first();
            if (vertex < 0) return OracleStatus::NOT_FOUND_EXACT;
            ++work_;
            if (capture_witness_) path_.push_back(vertex);
            return OracleStatus::FOUND;
        }

        int vertex = -1;
        while ((vertex = remaining.pop_first()) >= 0) {
            if (budget_ != 0 && work_ >= budget_) return OracleStatus::UNKNOWN;
            ++work_;

            DynamicBitset &shifted = shifted_[static_cast<std::size_t>(level)];
            DynamicBitset &next = child_[static_cast<std::size_t>(level)];
            shifted.shift_left_from(*neighbourhood_, static_cast<std::size_t>(vertex));
            next.and_from(remaining, shifted);

            if (capture_witness_) path_.push_back(vertex);
            const OracleStatus child_status = clique_dfs(next, need - 1, level + 1);
            if (child_status == OracleStatus::FOUND) return child_status;
            if (capture_witness_) path_.pop_back();
            if (child_status == OracleStatus::UNKNOWN) return child_status;
        }
        return OracleStatus::NOT_FOUND_EXACT;
    }

    int order_;
    const DynamicBitset *neighbourhood_ = nullptr;
    std::uint64_t budget_ = 0;
    std::uint64_t work_ = 0;
    bool capture_witness_ = false;
    DynamicBitset common_;
    DynamicBitset rotated_;
    DynamicBitset rotate_scratch_;
    std::vector<DynamicBitset> remaining_;
    std::vector<DynamicBitset> shifted_;
    std::vector<DynamicBitset> child_;
    std::vector<int> path_;
    std::array<int, 128> path_fixed_{{}};
};

struct Statistics {
    std::uint64_t nodes = 0;
    std::uint64_t branches = 0;
    std::array<std::uint64_t, 2> prunes{{0, 0}};
    std::uint64_t fast_calls = 0;
    std::uint64_t fast_work = 0;
    std::uint64_t fast_uncounted_calls = 0;
    std::uint64_t fast_work_max = 0;
    std::array<std::uint64_t, 5> fast_calls_over{{0, 0, 0, 0, 0}};
    std::uint64_t fast_budget_exhaustions = 0;
    std::uint64_t copt_calls = 0;
    std::uint64_t copt_found = 0;
    std::uint64_t copt_not_found = 0;
    std::uint64_t copt_unknown = 0;
    std::uint64_t copt_steps = 0;
    std::uint64_t copt_vertices_max = 0;
    std::uint64_t fallback_to_unbounded_fast = 0;
    std::uint64_t solutions = 0;
    std::uint64_t split_seen = 0;
    std::uint64_t split_kept = 0;
    double fast_seconds = 0.0;
    double copt_seconds = 0.0;
};

class HybridCirculantSearch {
public:
    explicit HybridCirculantSearch(const Config &config)
        : config_(config),
          max_distance_(config.order / 2),
          distance_colour_(static_cast<std::size_t>(max_distance_ + 1), -1),
          compact_neighbourhoods_{{CompactWord(0), CompactWord(0)}},
          neighbourhoods_{{DynamicBitset(static_cast<std::size_t>(config.order)),
                           DynamicBitset(static_cast<std::size_t>(config.order))}},
          fast_oracle_(config.order, std::max(config.blue_target, config.red_target)),
          copt_oracle_(config.copt_timeout),
          start_(Clock::now()) {}

    int run() {
        if (!config_.replay_matrix.empty()) return replay_matrix();
        dfs(0);

        std::string verdict;
        if (incomplete_) verdict = "INCOMPLETE";
        else if (stats_.solutions > 0) verdict = "FEASIBLE";
        else if (config_.split_count > 1) verdict = "EMPTY_SPLIT";
        else verdict = "EMPTY";

        print_summary(verdict, !incomplete_);
        return incomplete_ ? 3 : 0;
    }

private:
    using Clock = std::chrono::steady_clock;

    void print_summary(const std::string &verdict, bool complete) const {
        const double elapsed = seconds_since(start_);

        std::cout << "verdict=" << verdict << '\n';
        std::cout << "complete=" << (complete ? 1 : 0) << '\n';
        std::cout << "order=" << config_.order << '\n';
        std::cout << "blue_target=" << config_.blue_target << '\n';
        std::cout << "red_target=" << config_.red_target << '\n';
        std::cout << "mode=" << mode_name(config_.mode) << '\n';
        std::cout << "nodes=" << stats_.nodes << '\n';
        std::cout << "branches=" << stats_.branches << '\n';
        std::cout << "prunes_blue=" << stats_.prunes[0] << '\n';
        std::cout << "prunes_red=" << stats_.prunes[1] << '\n';
        std::cout << "solutions=" << stats_.solutions << '\n';
        std::cout << "detailed_stats=" << (config_.detailed_stats ? 1 : 0) << '\n';
        std::cout << "fast_calls=" << stats_.fast_calls << '\n';
        std::cout << "fast_work=" << stats_.fast_work << '\n';
        std::cout << "fast_uncounted_calls=" << stats_.fast_uncounted_calls << '\n';
        std::cout << "fast_work_max=" << stats_.fast_work_max << '\n';
        std::cout << "fast_calls_over_100=" << stats_.fast_calls_over[0] << '\n';
        std::cout << "fast_calls_over_1000=" << stats_.fast_calls_over[1] << '\n';
        std::cout << "fast_calls_over_10000=" << stats_.fast_calls_over[2] << '\n';
        std::cout << "fast_calls_over_50000=" << stats_.fast_calls_over[3] << '\n';
        std::cout << "fast_calls_over_250000=" << stats_.fast_calls_over[4] << '\n';
        std::cout << "fast_budget_exhaustions=" << stats_.fast_budget_exhaustions << '\n';
        std::cout << "copt_calls=" << stats_.copt_calls << '\n';
        std::cout << "copt_found=" << stats_.copt_found << '\n';
        std::cout << "copt_not_found=" << stats_.copt_not_found << '\n';
        std::cout << "copt_unknown=" << stats_.copt_unknown << '\n';
        std::cout << "copt_steps=" << stats_.copt_steps << '\n';
        std::cout << "copt_vertices_max=" << stats_.copt_vertices_max << '\n';
        std::cout << "fallback_to_unbounded_fast=" << stats_.fallback_to_unbounded_fast << '\n';
        std::cout << "split_seen=" << stats_.split_seen << '\n';
        std::cout << "split_kept=" << stats_.split_kept << '\n';
        std::cout << std::fixed << std::setprecision(6);
        std::cout << "fast_seconds=" << std::max(0.0, elapsed - stats_.copt_seconds) << '\n';
        std::cout << "fast_seconds_is_residual=1\n";
        std::cout << "copt_seconds=" << stats_.copt_seconds << '\n';
        std::cout << "elapsed_seconds=" << elapsed << '\n';
    }

    std::vector<int> load_circulant_matrix() const {
        std::ifstream input(config_.replay_matrix);
        if (!input) throw std::runtime_error("cannot open replay matrix: " + config_.replay_matrix);

        int matrix_order = 0;
        if (!(input >> matrix_order)) throw std::runtime_error("missing order in replay matrix");
        if (matrix_order != config_.order) {
            throw std::runtime_error("replay matrix order does not match ORDER");
        }

        std::vector<std::int8_t> matrix(
            static_cast<std::size_t>(matrix_order) * static_cast<std::size_t>(matrix_order));
        std::string token;
        if (!(input >> token)) throw std::runtime_error("missing replay matrix data");
        if (token == "JUMPS") {
            while (input >> token && token != "MATRIX") {}
            if (token != "MATRIX" || !(input >> token)) {
                throw std::runtime_error("missing MATRIX section in replay file");
            }
        }
        for (std::size_t position = 0; position < matrix.size(); ++position) {
            if (position != 0 && !(input >> token)) {
                throw std::runtime_error("replay matrix must contain exactly ORDER x ORDER bits");
            }
            if (token != "0" && token != "1") {
                throw std::runtime_error("replay matrix must contain exactly ORDER x ORDER bits");
            }
            matrix[position] = static_cast<std::int8_t>(token == "1" ? 1 : 0);
        }
        if (input >> token) throw std::runtime_error("extra entries after replay matrix");

        auto at = [&](int row, int column) -> int {
            return matrix[static_cast<std::size_t>(row) * matrix_order + column];
        };
        bool symmetric_full = true;
        bool historical_upper_triangle = true;
        for (int row = 0; row < matrix_order; ++row) {
            if (at(row, row) != 0) throw std::runtime_error("replay matrix diagonal is not zero");
            for (int column = 0; column < matrix_order; ++column) {
                int circular_difference = column - row;
                if (circular_difference < 0) circular_difference += matrix_order;
                if (at(row, column) != at(column, row) ||
                    at(row, column) != at(0, circular_difference)) {
                    symmetric_full = false;
                }
                if (column < row && at(row, column) != 0) {
                    historical_upper_triangle = false;
                } else if (column > row && at(row, column) != at(0, column - row)) {
                    historical_upper_triangle = false;
                }
            }
        }
        if (!symmetric_full && !historical_upper_triangle) {
            throw std::runtime_error(
                "replay matrix is neither full circulant nor historical upper-triangle format");
        }
        for (int distance = 1; distance < matrix_order; ++distance) {
            if (at(0, distance) != at(0, matrix_order - distance)) {
                throw std::runtime_error("replay distance colouring is not reflection-symmetric");
            }
        }

        std::vector<int> colours(static_cast<std::size_t>(max_distance_ + 1), -1);
        for (int distance = 1; distance <= max_distance_; ++distance) {
            // In the historical matrices, 1 is the graph avoiding BLUE_K.
            colours[static_cast<std::size_t>(distance)] = at(0, distance) == 1 ? 0 : 1;
        }
        return colours;
    }

    int replay_matrix() {
        const std::vector<int> colours = load_circulant_matrix();
        bool valid = true;
        int rejected_distance = -1;
        int rejected_colour = -1;

        for (int distance = 1; distance <= max_distance_; ++distance) {
            if (time_limit_reached()) {
                incomplete_ = true;
                valid = false;
                break;
            }
            const int colour = colours[static_cast<std::size_t>(distance)];
            ++stats_.nodes;
            ++stats_.branches;
            assign_distance(distance, colour);
            const OracleStatus status = forbidden_clique(colour, distance, nullptr);
            if (status == OracleStatus::FOUND) {
                ++stats_.prunes[static_cast<std::size_t>(colour)];
                rejected_distance = distance;
                rejected_colour = colour;
                valid = false;
                break;
            }
            if (status != OracleStatus::NOT_FOUND_EXACT) {
                incomplete_ = true;
                valid = false;
                break;
            }
        }

        if (valid) {
            ++stats_.solutions;
            std::cout << "solution_index=1 blue_distances=" << distance_set(0)
                      << " red_distances=" << distance_set(1) << '\n';
        }
        std::cout << "replay_matrix=" << config_.replay_matrix << '\n';
        std::cout << "replay_valid=" << (valid ? 1 : 0) << '\n';
        if (rejected_distance >= 0) {
            std::cout << "rejected_distance=" << rejected_distance << '\n';
            std::cout << "rejected_colour=" << (rejected_colour == 0 ? "blue" : "red") << '\n';
        }

        if (incomplete_) {
            print_summary("INCOMPLETE", false);
            return 3;
        }
        print_summary(valid ? "FEASIBLE" : "INVALID_REPLAY", true);
        return valid ? 0 : 4;
    }

    static const char *mode_name(SearchMode mode) {
        switch (mode) {
            case SearchMode::FAST_ONLY: return "fast";
            case SearchMode::CHECKER_ONLY: return "checker";
            case SearchMode::HYBRID: return "hybrid";
        }
        return "unknown";
    }

    static double seconds_since(Clock::time_point start) {
        return std::chrono::duration<double>(Clock::now() - start).count();
    }

    bool time_limit_reached() const {
        return config_.max_seconds >= 0.0 && seconds_since(start_) >= config_.max_seconds;
    }

    int target_for_colour(int colour) const {
        return colour == 0 ? config_.blue_target : config_.red_target;
    }

    void assign_distance(int distance, int colour) {
        distance_colour_[static_cast<std::size_t>(distance)] = static_cast<std::int8_t>(colour);
        const int reflected = config_.order - distance;
        if (config_.order <= 128) {
            CompactWord &compact = compact_neighbourhoods_[static_cast<std::size_t>(colour)];
            compact |= CompactWord(1) << distance;
            if (reflected != distance) compact |= CompactWord(1) << reflected;
        }
        if (config_.order > 128 || config_.mode != SearchMode::FAST_ONLY) {
            DynamicBitset &neighbourhood = neighbourhoods_[static_cast<std::size_t>(colour)];
            neighbourhood.set(static_cast<std::size_t>(distance));
            if (reflected != distance) neighbourhood.set(static_cast<std::size_t>(reflected));
        }
    }

    void undo_distance(int distance, int colour) {
        const int reflected = config_.order - distance;
        if (config_.order <= 128) {
            CompactWord &compact = compact_neighbourhoods_[static_cast<std::size_t>(colour)];
            compact &= ~(CompactWord(1) << distance);
            if (reflected != distance) compact &= ~(CompactWord(1) << reflected);
        }
        if (config_.order > 128 || config_.mode != SearchMode::FAST_ONLY) {
            DynamicBitset &neighbourhood = neighbourhoods_[static_cast<std::size_t>(colour)];
            neighbourhood.reset(static_cast<std::size_t>(distance));
            if (reflected != distance) neighbourhood.reset(static_cast<std::size_t>(reflected));
        }
        distance_colour_[static_cast<std::size_t>(distance)] = -1;
    }

    OracleResult run_copt(int colour, int distance) {
        const int target = target_for_colour(colour) - 2;
        const DynamicBitset &candidate = fast_oracle_.common_neighbours(
            neighbourhoods_[static_cast<std::size_t>(colour)], distance);
        stats_.copt_vertices_max = std::max<std::uint64_t>(
            stats_.copt_vertices_max, candidate.count());
        const Clock::time_point begin = Clock::now();
        OracleResult result = copt_oracle_.solve_induced(
            candidate,
            neighbourhoods_[static_cast<std::size_t>(colour)],
            config_.order,
            target);
        stats_.copt_seconds += seconds_since(begin);
        ++stats_.copt_calls;
        stats_.copt_steps += result.steps;
        if (result.status == OracleStatus::FOUND) ++stats_.copt_found;
        else if (result.status == OracleStatus::NOT_FOUND_EXACT) ++stats_.copt_not_found;
        else ++stats_.copt_unknown;
        return result;
    }

    FastResult run_fast(
        int colour,
        int distance,
        std::uint64_t budget,
        std::vector<int> *witness) {
        const std::uint64_t oracle_budget = budget == 0 && config_.detailed_stats
            ? std::numeric_limits<std::uint64_t>::max()
            : budget;
        FastResult result = config_.order <= 128
            ? fast_oracle_.query_edge_compact(
                  compact_neighbourhoods_[static_cast<std::size_t>(colour)],
                  distance,
                  target_for_colour(colour),
                  oracle_budget,
                  witness)
            : fast_oracle_.query_edge(
                  neighbourhoods_[static_cast<std::size_t>(colour)],
                  distance,
                  target_for_colour(colour),
                  oracle_budget,
                  witness);
        if (config_.detailed_stats) {
            ++stats_.fast_calls;
            if (!result.work_counted) ++stats_.fast_uncounted_calls;
            stats_.fast_work += result.work;
            stats_.fast_work_max = std::max(stats_.fast_work_max, result.work);
            if (result.work > 100) {
                ++stats_.fast_calls_over[0];
                if (result.work > 1000) {
                    ++stats_.fast_calls_over[1];
                    if (result.work > 10000) {
                        ++stats_.fast_calls_over[2];
                        if (result.work > 50000) {
                            ++stats_.fast_calls_over[3];
                            if (result.work > 250000) ++stats_.fast_calls_over[4];
                        }
                    }
                }
            }
        }
        return result;
    }

    OracleStatus forbidden_clique(int colour, int distance, std::vector<int> *witness) {
        if (config_.mode == SearchMode::CHECKER_ONLY) {
            OracleResult output = run_copt(colour, distance);
            if (output.status != OracleStatus::UNKNOWN) {
                if (witness != nullptr && output.status == OracleStatus::FOUND) {
                    *witness = std::move(output.witness);
                }
                return output.status;
            }
            ++stats_.fallback_to_unbounded_fast;
            return run_fast(colour, distance, 0, witness).status;
        }

        const std::uint64_t budget =
            config_.mode == SearchMode::FAST_ONLY ? 0 : config_.fast_budget;
        const FastResult fast = run_fast(colour, distance, budget, witness);
        if (fast.status != OracleStatus::UNKNOWN) return fast.status;

        ++stats_.fast_budget_exhaustions;
        OracleResult output = run_copt(colour, distance);
        if (output.status != OracleStatus::UNKNOWN) {
            if (witness != nullptr && output.status == OracleStatus::FOUND) {
                *witness = std::move(output.witness);
            }
            return output.status;
        }

        ++stats_.fallback_to_unbounded_fast;
        return run_fast(colour, distance, 0, witness).status;
    }

    bool valid_clique(const std::vector<int> &vertices, int colour) const {
        if (static_cast<int>(vertices.size()) != target_for_colour(colour)) return false;
        std::vector<int> sorted = vertices;
        std::sort(sorted.begin(), sorted.end());
        if (std::adjacent_find(sorted.begin(), sorted.end()) != sorted.end()) return false;
        for (int vertex : sorted) {
            if (vertex < 0 || vertex >= config_.order) return false;
        }
        const DynamicBitset &neighbourhood = neighbourhoods_[static_cast<std::size_t>(colour)];
        for (std::size_t i = 0; i + 1 < sorted.size(); ++i) {
            for (std::size_t j = i + 1; j < sorted.size(); ++j) {
                int difference = sorted[j] - sorted[i];
                difference %= config_.order;
                if (difference < 0) difference += config_.order;
                const bool adjacent = config_.order <= 128
                    ? (compact_neighbourhoods_[static_cast<std::size_t>(colour)] &
                       (CompactWord(1) << difference)) != 0
                    : neighbourhood.test(static_cast<std::size_t>(difference));
                if (!adjacent) return false;
            }
        }
        return true;
    }

    void trace_prune(int colour, int distance, const std::vector<int> &clique) {
        if (traces_emitted_ >= config_.trace_limit) return;
        ++traces_emitted_;
        std::cerr << "TRACE prune distance=" << distance
                  << " colour=" << (colour == 0 ? "blue" : "red") << " clique={";
        for (std::size_t i = 0; i < clique.size(); ++i) {
            if (i != 0) std::cerr << ',';
            std::cerr << clique[i];
        }
        std::cerr << "}\n";
    }

    bool verify_leaf() {
        for (int colour = 0; colour < 2; ++colour) {
            const FastResult check = config_.order <= 128
                ? fast_oracle_.query_global_compact(
                      compact_neighbourhoods_[static_cast<std::size_t>(colour)],
                      target_for_colour(colour))
                : fast_oracle_.query_global(
                      neighbourhoods_[static_cast<std::size_t>(colour)],
                      target_for_colour(colour));
            if (check.status == OracleStatus::FOUND) return false;
        }
        return true;
    }

    std::string distance_set(int colour) const {
        std::ostringstream output;
        output << '{';
        bool first = true;
        for (int distance = 1; distance <= max_distance_; ++distance) {
            if (distance_colour_[static_cast<std::size_t>(distance)] == colour) {
                if (!first) output << ',';
                output << distance;
                first = false;
            }
        }
        output << '}';
        return output.str();
    }

    void record_solution() {
        if (!verify_leaf()) {
            std::cerr << "INTERNAL ERROR: a complete colouring failed independent global verification\n";
            incomplete_ = true;
            stop_ = true;
            return;
        }
        ++stats_.solutions;
        std::cout << "solution_index=" << stats_.solutions
                  << " blue_distances=" << distance_set(0)
                  << " red_distances=" << distance_set(1) << '\n';
        if (!config_.enumerate_all || stats_.solutions >= config_.max_solutions) stop_ = true;
    }

    void dfs(int depth) {
        if (stop_ || incomplete_) return;
        if ((stats_.nodes & 0x3FFFULL) == 0ULL && time_limit_reached()) {
            incomplete_ = true;
            stop_ = true;
            return;
        }

        if (config_.split_count > 1 && depth == config_.split_depth) {
            const std::uint64_t split = stats_.split_seen++;
            if ((split % config_.split_count) != config_.split_index) return;
            ++stats_.split_kept;
        }

        ++stats_.nodes;
        if (depth == max_distance_) {
            record_solution();
            return;
        }

        const int distance = depth + 1;
        const bool fix_first_colour =
            config_.colour_symmetry && config_.blue_target == config_.red_target && distance == 1;
        const int colour_limit = fix_first_colour ? 1 : 2;

        for (int colour = 0; colour < colour_limit; ++colour) {
            if (stop_ || incomplete_) return;
            ++stats_.branches;
            assign_distance(distance, colour);

            const bool want_witness = traces_emitted_ < config_.trace_limit;
            std::vector<int> residual_witness;
            const OracleStatus status = forbidden_clique(
                colour, distance, want_witness ? &residual_witness : nullptr);
            if (status == OracleStatus::FOUND) {
                if (want_witness) {
                    std::vector<int> clique;
                    clique.reserve(static_cast<std::size_t>(target_for_colour(colour)));
                    clique.push_back(0);
                    clique.push_back(distance);
                    clique.insert(
                        clique.end(), residual_witness.begin(), residual_witness.end());
                    if (!valid_clique(clique, colour)) {
                        std::cerr << "INTERNAL ERROR: oracle returned an invalid clique witness\n";
                        incomplete_ = true;
                        stop_ = true;
                    } else {
                        trace_prune(colour, distance, clique);
                    }
                }
                ++stats_.prunes[static_cast<std::size_t>(colour)];
            } else if (status == OracleStatus::NOT_FOUND_EXACT) {
                dfs(depth + 1);
            } else {
                std::cerr << "INTERNAL ERROR: unresolved oracle query\n";
                incomplete_ = true;
                stop_ = true;
            }

            undo_distance(distance, colour);
        }
    }

    Config config_;
    int max_distance_;
    std::vector<std::int8_t> distance_colour_;
    std::array<CompactWord, 2> compact_neighbourhoods_;
    std::array<DynamicBitset, 2> neighbourhoods_;
    FastCliqueOracle fast_oracle_;
    CoptCliqueOracle copt_oracle_;
    Statistics stats_;
    Clock::time_point start_;
    bool stop_ = false;
    bool incomplete_ = false;
    std::uint64_t traces_emitted_ = 0;
};

void print_help(const char *program) {
    std::cout
        << "Usage: " << program << " ORDER BLUE_K RED_K [options]\n\n"
        << "Exact search for a two-colour circulant Ramsey colouring.\n\n"
        << "Options:\n"
        << "  --mode fast|checker|hybrid  oracle policy (default: hybrid)\n"
        << "  --budget N                 fast-oracle work before COPT fallback (default: 1000)\n"
        << "  --copt-timeout SEC         COPT timeout; UNKNOWN falls back to exact fast search\n"
        << "  --max-seconds SEC          stop the job as INCOMPLETE after this wall time\n"
        << "  --all                      enumerate rather than stop at the first colouring\n"
        << "  --max-solutions N          cap enumeration (default: 1)\n"
        << "  --trace-limit N            print the first N pruning witnesses to stderr\n"
        << "  --detailed-stats           collect per-fast-call work telemetry (slower)\n"
        << "  --no-colour-symmetry       do not fix distance 1 when BLUE_K == RED_K\n"
        << "  --split-depth D            partition at search depth D\n"
        << "  --split-count S            number of deterministic split classes\n"
        << "  --split-index I            class to execute, 0 <= I < S\n"
        << "  --replay-matrix FILE       certify a stored 0/1 circulant adjacency matrix\n"
        << "  --help                     show this message\n";
}

std::uint64_t parse_u64(const std::string &text, const char *name) {
    std::size_t consumed = 0;
    const unsigned long long value = std::stoull(text, &consumed);
    if (consumed != text.size()) throw std::invalid_argument(std::string("invalid ") + name);
    return static_cast<std::uint64_t>(value);
}

int parse_int(const std::string &text, const char *name) {
    std::size_t consumed = 0;
    const long value = std::stol(text, &consumed);
    if (consumed != text.size() || value < std::numeric_limits<int>::min() ||
        value > std::numeric_limits<int>::max()) {
        throw std::invalid_argument(std::string("invalid ") + name);
    }
    return static_cast<int>(value);
}

double parse_double(const std::string &text, const char *name) {
    std::size_t consumed = 0;
    const double value = std::stod(text, &consumed);
    if (consumed != text.size()) throw std::invalid_argument(std::string("invalid ") + name);
    return value;
}

Config parse_arguments(int argc, char **argv) {
    if (argc < 4) throw std::invalid_argument("ORDER, BLUE_K and RED_K are required");
    Config config;
    config.order = parse_int(argv[1], "ORDER");
    config.blue_target = parse_int(argv[2], "BLUE_K");
    config.red_target = parse_int(argv[3], "RED_K");

    for (int i = 4; i < argc; ++i) {
        const std::string option = argv[i];
        auto require_value = [&](const char *name) -> std::string {
            if (++i >= argc) throw std::invalid_argument(std::string("missing value for ") + name);
            return argv[i];
        };

        if (option == "--mode") {
            const std::string value = require_value("--mode");
            if (value == "fast") config.mode = SearchMode::FAST_ONLY;
            else if (value == "checker") config.mode = SearchMode::CHECKER_ONLY;
            else if (value == "hybrid") config.mode = SearchMode::HYBRID;
            else throw std::invalid_argument("--mode must be fast, checker or hybrid");
        } else if (option == "--budget") {
            config.fast_budget = parse_u64(require_value("--budget"), "budget");
        } else if (option == "--copt-timeout") {
            config.copt_timeout = parse_double(require_value("--copt-timeout"), "COPT timeout");
        } else if (option == "--max-seconds") {
            config.max_seconds = parse_double(require_value("--max-seconds"), "max seconds");
        } else if (option == "--all") {
            config.enumerate_all = true;
            config.max_solutions = std::numeric_limits<std::uint64_t>::max();
        } else if (option == "--max-solutions") {
            config.max_solutions = parse_u64(require_value("--max-solutions"), "max solutions");
        } else if (option == "--trace-limit") {
            config.trace_limit = parse_u64(require_value("--trace-limit"), "trace limit");
        } else if (option == "--detailed-stats") {
            config.detailed_stats = true;
        } else if (option == "--no-colour-symmetry") {
            config.colour_symmetry = false;
        } else if (option == "--split-depth") {
            config.split_depth = parse_int(require_value("--split-depth"), "split depth");
        } else if (option == "--split-count") {
            config.split_count = parse_u64(require_value("--split-count"), "split count");
        } else if (option == "--split-index") {
            config.split_index = parse_u64(require_value("--split-index"), "split index");
        } else if (option == "--replay-matrix") {
            config.replay_matrix = require_value("--replay-matrix");
        } else if (option == "--help") {
            print_help(argv[0]);
            std::exit(0);
        } else {
            throw std::invalid_argument("unknown option: " + option);
        }
    }

    if (config.order < 3) throw std::invalid_argument("ORDER must be at least 3");
    if (config.blue_target < 2 || config.red_target < 2) {
        throw std::invalid_argument("clique targets must be at least 2");
    }
    if (config.split_count == 0) throw std::invalid_argument("split count must be positive");
    if (config.split_index >= config.split_count) {
        throw std::invalid_argument("split index must be smaller than split count");
    }
    if (config.split_depth < 0 || config.split_depth > config.order / 2) {
        throw std::invalid_argument("split depth is outside the search depth");
    }
    if (config.split_count > 1 && config.split_depth == 0) {
        throw std::invalid_argument("split depth must be positive when split count > 1");
    }
    if (config.max_solutions == 0) throw std::invalid_argument("max solutions must be positive");
    return config;
}

}  // namespace

int main(int argc, char **argv) {
    try {
        if (argc == 2 && std::string(argv[1]) == "--help") {
            print_help(argv[0]);
            return 0;
        }
        const Config config = parse_arguments(argc, argv);
        HybridCirculantSearch search(config);
        return search.run();
    } catch (const std::exception &error) {
        std::cerr << "error: " << error.what() << "\n\n";
        print_help(argv[0]);
        return 2;
    }
}
