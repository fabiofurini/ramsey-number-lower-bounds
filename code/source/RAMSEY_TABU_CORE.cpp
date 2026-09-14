#include "RAMSEY_TABU_CORE.h"

#include <algorithm>
#include <chrono>
#include <cmath>
#include <functional>
#include <limits>
#include <stdexcept>

namespace ramsey_tabu {

namespace {
constexpr double kEpsilon = 1e-9;
}

DistanceSpaceTabuSearch::DistanceSpaceTabuSearch(const TabuConfig& config)
    : config_(config), rng_(config.seed) {
    if (config_.order < 3) {
        throw std::invalid_argument("The circulant order must be at least 3");
    }
    if (config_.tabu_tenure_min < 1 || config_.tabu_tenure_max < config_.tabu_tenure_min) {
        throw std::invalid_argument("Invalid tabu-tenure interval");
    }
    const int distance_count = number_of_distances(config_.order, config_.geometry);
    distances_.assign(distance_count, 0);
    std::bernoulli_distribution initial_color(config_.initial_blue_probability);
    for (std::uint8_t& value : distances_) {
        value = initial_color(rng_) ? 1 : 0;
    }
    incidence_.resize(distance_count);
}

int DistanceSpaceTabuSearch::circular_distance(int order, int u, int v) {
    int difference = std::abs(u - v) % order;
    return std::min(difference, order - difference);
}

std::vector<SupportSeed> DistanceSpaceTabuSearch::enumerate_triangle_supports(int order, Color color) {
    return enumerate_small_clique_supports(order, 3, color, DistanceGeometry::Circulant);
}

int DistanceSpaceTabuSearch::linear_distance(int order, int u, int v) {
    if (order < 3 || u < 0 || u >= order || v < 0 || v >= order || u == v) {
        throw std::invalid_argument("Invalid endpoints for a linear distance");
    }
    return std::abs(u - v);
}

int DistanceSpaceTabuSearch::edge_distance(int order, int u, int v, DistanceGeometry geometry) {
    if (geometry == DistanceGeometry::Circulant) {
        return circular_distance(order, u, v);
    }
    if (geometry == DistanceGeometry::Linear) {
        return linear_distance(order, u, v);
    }
    throw std::invalid_argument("Unknown distance geometry");
}

int DistanceSpaceTabuSearch::number_of_distances(int order, DistanceGeometry geometry) {
    if (order < 3) {
        throw std::invalid_argument("The circulant order must be at least 3");
    }
    if (geometry == DistanceGeometry::Circulant) {
        return order / 2;
    }
    if (geometry == DistanceGeometry::Linear) {
        return order - 1;
    }
    throw std::invalid_argument("Unknown distance geometry");
}

std::vector<SupportSeed> DistanceSpaceTabuSearch::enumerate_small_clique_supports(
    int order, int clique_size, Color color, DistanceGeometry geometry) {
    if (clique_size != 3 && clique_size != 4) {
        throw std::invalid_argument("Only K3 and K4 support families can be pre-enumerated");
    }
    if (order < clique_size) {
        return std::vector<SupportSeed>();
    }
    const int distance_count = number_of_distances(order, geometry);

    std::set<std::vector<std::uint16_t>> unique_supports;
    std::vector<int> vertices(1, 0);
    const auto record_support = [&]() {
        std::vector<std::uint16_t> support;
        for (int i = 0; i < clique_size; ++i) {
            for (int j = i + 1; j < clique_size; ++j) {
                const int distance = edge_distance(order, vertices[i], vertices[j], geometry);
                if (distance < 1 || distance > distance_count) {
                    throw std::logic_error("Clique support contains a distance outside its geometry");
                }
                support.push_back(static_cast<std::uint16_t>(distance));
            }
        }
        std::sort(support.begin(), support.end());
        support.erase(std::unique(support.begin(), support.end()), support.end());
        unique_supports.insert(support);
    };
    std::function<void(int)> extend = [&](int first) {
        if (static_cast<int>(vertices.size()) == clique_size) {
            record_support();
            return;
        }
        const int missing = clique_size - static_cast<int>(vertices.size());
        for (int vertex = first; vertex <= order - missing; ++vertex) {
            vertices.push_back(vertex);
            extend(vertex + 1);
            vertices.pop_back();
        }
    };
    extend(1);

    std::vector<SupportSeed> result;
    result.reserve(unique_supports.size());
    for (const std::vector<std::uint16_t>& support : unique_supports) {
        result.push_back({color, support, 1.0});
    }
    return result;
}

bool DistanceSpaceTabuSearch::add_support(const SupportSeed& seed) {
    if (seed.distances.empty()) {
        return false;
    }
    Support support{seed.color, seed.distances, 0, seed.weight};
    std::sort(support.distances.begin(), support.distances.end());
    support.distances.erase(std::unique(support.distances.begin(), support.distances.end()),
                            support.distances.end());
    for (std::uint16_t distance : support.distances) {
        if (distance == 0 || distance > distances_.size()) {
            throw std::invalid_argument("A support contains an invalid distance for the active geometry");
        }
    }
    const int color_key = support.color == Color::Blue ? 0 : 1;
    if (!support_keys_.insert({color_key, support.distances}).second) {
        return false;
    }
    support.q = q_for(support);
    score_ += penalty(support, support.q);
    const std::size_t support_id = supports_.size();
    supports_.push_back(support);
    for (std::uint16_t distance : support.distances) {
        incidence_[distance - 1].push_back(support_id);
    }
    return true;
}

std::size_t DistanceSpaceTabuSearch::support_count() const {
    return supports_.size();
}

void DistanceSpaceTabuSearch::set_distances(const std::vector<std::uint8_t>& distances) {
    if (distances.size() != distances_.size()) {
        throw std::invalid_argument("Distance-vector size does not match the active geometry");
    }
    for (std::uint8_t value : distances) {
        if (value > 1) {
            throw std::invalid_argument("Distance-vector entries must be binary");
        }
    }
    distances_ = distances;
    rebuild_counters();
}

const std::vector<std::uint8_t>& DistanceSpaceTabuSearch::distances() const {
    return distances_;
}

double DistanceSpaceTabuSearch::score() const {
    return score_;
}

int DistanceSpaceTabuSearch::q_for(const Support& support) const {
    int q = 0;
    for (std::uint16_t distance : support.distances) {
        const bool blue = distances_[distance - 1] != 0;
        if ((support.color == Color::Blue && !blue) ||
            (support.color == Color::Red && blue)) {
            ++q;
        }
    }
    return q;
}

double DistanceSpaceTabuSearch::penalty(const Support& support, int q) const {
    if (q == 0) {
        return support.weight;
    }
    if (q == 1) {
        return support.weight * config_.near_penalty_one;
    }
    return 0.0;
}

double DistanceSpaceTabuSearch::recompute_score() const {
    double result = 0.0;
    for (const Support& support : supports_) {
        result += penalty(support, q_for(support));
    }
    return result;
}

double DistanceSpaceTabuSearch::score_after_flip(int distance_index) const {
    if (distance_index < 0 || distance_index >= static_cast<int>(distances_.size())) {
        throw std::out_of_range("Invalid distance index");
    }
    const bool currently_blue = distances_[distance_index] != 0;
    double result = score_;
    for (std::size_t support_id : incidence_[distance_index]) {
        const Support& support = supports_[support_id];
        const int q_change = support.color == Color::Blue
            ? (currently_blue ? 1 : -1)
            : (currently_blue ? -1 : 1);
        result += penalty(support, support.q + q_change) - penalty(support, support.q);
    }
    return result;
}

void DistanceSpaceTabuSearch::flip(int distance_index) {
    if (distance_index < 0 || distance_index >= static_cast<int>(distances_.size())) {
        throw std::out_of_range("Invalid distance index");
    }
    const bool currently_blue = distances_[distance_index] != 0;
    for (std::size_t support_id : incidence_[distance_index]) {
        Support& support = supports_[support_id];
        const int q_change = support.color == Color::Blue
            ? (currently_blue ? 1 : -1)
            : (currently_blue ? -1 : 1);
        score_ += penalty(support, support.q + q_change) - penalty(support, support.q);
        support.q += q_change;
    }
    distances_[distance_index] = currently_blue ? 0 : 1;
}

void DistanceSpaceTabuSearch::rebuild_counters() {
    score_ = 0.0;
    for (Support& support : supports_) {
        support.q = q_for(support);
        score_ += penalty(support, support.q);
    }
}

bool DistanceSpaceTabuSearch::debug_check_invariants() const {
    if (std::fabs(score_ - recompute_score()) > kEpsilon) {
        return false;
    }
    for (const Support& support : supports_) {
        if (support.q != q_for(support)) {
            return false;
        }
    }
    return true;
}

std::size_t DistanceSpaceTabuSearch::add_all(const std::vector<SupportSeed>& seeds) {
    std::size_t added = 0;
    for (const SupportSeed& seed : seeds) {
        if (add_support(seed)) {
            ++added;
        }
    }
    return added;
}

bool DistanceSpaceTabuSearch::update_weights() {
    if (config_.adaptive_weight_increment <= 0.0) {
        return false;
    }
    bool changed = false;
    for (Support& support : supports_) {
        if (support.q == 0) {
            score_ -= penalty(support, support.q);
            support.weight += config_.adaptive_weight_increment;
            score_ += penalty(support, support.q);
            changed = true;
        }
    }
    return changed;
}

int DistanceSpaceTabuSearch::choose_move(const std::vector<long long>& tabu_until,
                                          long long iteration,
                                          double best_score) const {
    const auto select_best = [&](const bool enforce_tabu) {
        double chosen_score = std::numeric_limits<double>::infinity();
        std::vector<int> ties;
        for (int distance = 0; distance < static_cast<int>(distances_.size()); ++distance) {
            const double candidate_score = score_after_flip(distance);
            const bool aspirational = candidate_score + kEpsilon < best_score;
            if (enforce_tabu && iteration < tabu_until[distance] && !aspirational) {
                continue;
            }
            if (candidate_score + kEpsilon < chosen_score) {
                chosen_score = candidate_score;
                ties.clear();
                ties.push_back(distance);
            } else if (std::fabs(candidate_score - chosen_score) <= kEpsilon) {
                ties.push_back(distance);
            }
        }
        return ties;
    };

    std::vector<int> ties = select_best(true);
    // In a small distance space all moves can be tabu at once.  Falling back to
    // the best move is the conventional aspiration-of-last-resort rule.
    if (ties.empty()) {
        ties = select_best(false);
    }
    std::uniform_int_distribution<std::size_t> tie_break(0, ties.size() - 1);
    return ties[tie_break(rng_)];
}

void DistanceSpaceTabuSearch::perturb() {
    const int count = std::min(config_.perturbation_size, static_cast<int>(distances_.size()));
    if (count <= 0) {
        return;
    }
    std::vector<int> candidates(distances_.size());
    for (int i = 0; i < static_cast<int>(candidates.size()); ++i) {
        candidates[i] = i;
    }
    std::shuffle(candidates.begin(), candidates.end(), rng_);
    for (int i = 0; i < count; ++i) {
        flip(candidates[i]);
    }
}

RunResult DistanceSpaceTabuSearch::run(const SeparationCallback& separate,
                                       const VerificationCallback& verify) {
    RunResult result;
    std::vector<long long> tabu_until(distances_.size(), 0);
    const auto start = std::chrono::steady_clock::now();
    add_all(separate(distances_));
    ++result.separation_calls;
    result.pool_epochs = 1;

    std::vector<std::uint8_t> best = distances_;
    double best_score = score_;
    long long last_improvement = 0;

    for (long long iteration = 0; iteration < config_.max_iterations; ++iteration) {
        const double elapsed = std::chrono::duration<double>(std::chrono::steady_clock::now() - start).count();
        if (elapsed >= config_.time_limit_seconds) {
            result.time_limit_reached = true;
            result.iterations = iteration;
            break;
        }

        if (score_ <= kEpsilon) {
            // A zero pool score only says that the currently stored supports
            // are satisfied.  First invoke the same red separator used during
            // search; an inexpensive newly found conflict avoids an immediate
            // exact verification.
            const std::size_t added_by_separation = add_all(separate(distances_));
            ++result.separation_calls;
            if (added_by_separation > 0) {
                best = distances_;
                best_score = score_;
                last_improvement = iteration;
                ++result.pool_epochs;
                continue;
            }

            // The exact gate is the fallback and the only acceptance authority.
            const VerificationResult verification = verify(distances_);
            ++result.verification_calls;
            if (verification.feasible) {
                result.feasible = true;
                result.iterations = iteration;
                result.final_score = score_;
                result.best_score = 0.0;
                result.best_distances = distances_;
                return result;
            }
            if (add_all(verification.violated_supports) == 0) {
                throw std::runtime_error("The verifier rejected a zero-score state without returning a new support");
            }
            best = distances_;
            best_score = score_;
            last_improvement = iteration;
            ++result.pool_epochs;
        }

        const bool periodic_separation = config_.separation_period > 0 &&
            iteration > 0 && iteration % config_.separation_period == 0;
        if (periodic_separation) {
            if (add_all(separate(distances_)) > 0) {
                best = distances_;
                best_score = score_;
                last_improvement = iteration;
                ++result.pool_epochs;
            }
            ++result.separation_calls;
        }

        const int move = choose_move(tabu_until, iteration, best_score);
        if (move < 0) {
            throw std::runtime_error("No admissible tabu move exists");
        }
        flip(move);
        std::uniform_int_distribution<int> tenure(config_.tabu_tenure_min, config_.tabu_tenure_max);
        tabu_until[move] = iteration + tenure(rng_) + 1;

        if (score_ + kEpsilon < best_score) {
            best_score = score_;
            best = distances_;
            last_improvement = iteration;
            if (add_all(separate(distances_)) > 0) {
                best = distances_;
                best_score = score_;
                last_improvement = iteration;
                ++result.pool_epochs;
            }
            ++result.separation_calls;
        }

        if (config_.adaptive_weight_period > 0 && iteration > 0 &&
            iteration % config_.adaptive_weight_period == 0) {
            if (update_weights()) {
                best = distances_;
                best_score = score_;
                last_improvement = iteration;
                ++result.pool_epochs;
            }
        }
        if (config_.stagnation_limit > 0 && iteration - last_improvement >= config_.stagnation_limit) {
            perturb();
            std::fill(tabu_until.begin(), tabu_until.end(), iteration);
            last_improvement = iteration;
        }
        result.iterations = iteration + 1;
    }

    result.final_score = score_;
    result.best_score = best_score;
    result.best_distances = best;
    return result;
}

} // namespace ramsey_tabu
