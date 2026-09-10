#include "RAMSEY_TABU_CORE.h"

#include <algorithm>
#include <chrono>
#include <cmath>
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
    const int number_of_distances = config_.order / 2;
    distances_.assign(number_of_distances, 0);
    std::bernoulli_distribution initial_color(config_.initial_blue_probability);
    for (std::uint8_t& value : distances_) {
        value = initial_color(rng_) ? 1 : 0;
    }
    incidence_.resize(number_of_distances);
}

int DistanceSpaceTabuSearch::circular_distance(int order, int u, int v) {
    int difference = std::abs(u - v) % order;
    return std::min(difference, order - difference);
}

std::vector<SupportSeed> DistanceSpaceTabuSearch::enumerate_triangle_supports(int order, Color color) {
    if (order < 3) {
        throw std::invalid_argument("The circulant order must be at least 3");
    }

    std::set<std::vector<std::uint16_t>> unique_supports;
    for (int u = 1; u < order; ++u) {
        for (int v = u + 1; v < order; ++v) {
            std::vector<std::uint16_t> support = {
                static_cast<std::uint16_t>(circular_distance(order, 0, u)),
                static_cast<std::uint16_t>(circular_distance(order, 0, v)),
                static_cast<std::uint16_t>(circular_distance(order, u, v))};
            std::sort(support.begin(), support.end());
            support.erase(std::unique(support.begin(), support.end()), support.end());
            unique_supports.insert(support);
        }
    }

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
            throw std::invalid_argument("A support contains an invalid circular distance");
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
        throw std::invalid_argument("Distance-vector size does not match the circulant order");
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

void DistanceSpaceTabuSearch::add_all(const std::vector<SupportSeed>& seeds) {
    for (const SupportSeed& seed : seeds) {
        add_support(seed);
    }
}

void DistanceSpaceTabuSearch::update_weights() {
    if (config_.adaptive_weight_increment <= 0.0) {
        return;
    }
    for (Support& support : supports_) {
        if (support.q == 0) {
            score_ -= penalty(support, support.q);
            support.weight += config_.adaptive_weight_increment;
            score_ += penalty(support, support.q);
        }
    }
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
            const std::size_t before_separation = supports_.size();
            add_all(separate(distances_));
            ++result.separation_calls;
            if (supports_.size() > before_separation) {
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
            const std::size_t before = supports_.size();
            add_all(verification.violated_supports);
            if (supports_.size() == before) {
                throw std::runtime_error("The verifier rejected a zero-score state without returning a new support");
            }
        }

        const bool periodic_separation = config_.separation_period > 0 &&
            iteration > 0 && iteration % config_.separation_period == 0;
        if (periodic_separation) {
            add_all(separate(distances_));
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
            add_all(separate(distances_));
            ++result.separation_calls;
        }

        if (config_.adaptive_weight_period > 0 && iteration > 0 &&
            iteration % config_.adaptive_weight_period == 0) {
            update_weights();
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
