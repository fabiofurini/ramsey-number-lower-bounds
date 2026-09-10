#ifndef RAMSEY_TABU_CORE_H
#define RAMSEY_TABU_CORE_H

#include <cstdint>
#include <functional>
#include <random>
#include <set>
#include <utility>
#include <vector>

namespace ramsey_tabu {

enum class Color { Blue, Red };

struct SupportSeed {
    Color color;
    std::vector<std::uint16_t> distances; // one-based circular distances
    double weight = 1.0;
};

struct TabuConfig {
    int order = 0;
    int blue_target = 3;
    int red_target = 0;
    unsigned int seed = 1;
    long long max_iterations = 100000;
    double time_limit_seconds = 60.0;
    int tabu_tenure_min = 7;
    int tabu_tenure_max = 14;
    long long stagnation_limit = 5000;
    int perturbation_size = 4;
    long long separation_period = 100;
    long long adaptive_weight_period = 0; // 0 disables adaptive weights
    double adaptive_weight_increment = 0.0;
    double near_penalty_one = 0.0; // 0 is the baseline: true violations only
    double initial_blue_probability = 0.5;
};

struct VerificationResult {
    bool feasible = false;
    std::vector<SupportSeed> violated_supports;
};

struct RunResult {
    bool feasible = false;
    bool time_limit_reached = false;
    long long iterations = 0;
    long long separation_calls = 0;
    long long verification_calls = 0;
    double final_score = 0.0;
    double best_score = 0.0;
    std::vector<std::uint8_t> best_distances; // 1 = blue, 0 = red
};

using SeparationCallback = std::function<std::vector<SupportSeed>(const std::vector<std::uint8_t>&)>;
using VerificationCallback = std::function<VerificationResult(const std::vector<std::uint8_t>&)>;

class DistanceSpaceTabuSearch {
public:
    explicit DistanceSpaceTabuSearch(const TabuConfig& config);

    static int circular_distance(int order, int u, int v);
    static std::vector<SupportSeed> enumerate_triangle_supports(int order, Color color);

    bool add_support(const SupportSeed& seed);
    std::size_t support_count() const;

    void set_distances(const std::vector<std::uint8_t>& distances);
    const std::vector<std::uint8_t>& distances() const;
    double score() const;
    double recompute_score() const;
    double score_after_flip(int distance_index) const; // zero-based index
    void flip(int distance_index);
    bool debug_check_invariants() const;

    RunResult run(const SeparationCallback& separate,
                  const VerificationCallback& verify);

private:
    struct Support {
        Color color;
        std::vector<std::uint16_t> distances;
        int q = 0;
        double weight = 1.0;
    };

    TabuConfig config_;
    std::vector<std::uint8_t> distances_;
    std::vector<Support> supports_;
    std::vector<std::vector<std::size_t>> incidence_;
    std::set<std::pair<int, std::vector<std::uint16_t>>> support_keys_;
    mutable std::mt19937 rng_;
    double score_ = 0.0;

    int q_for(const Support& support) const;
    double penalty(const Support& support, int q) const;
    void rebuild_counters();
    void add_all(const std::vector<SupportSeed>& seeds);
    void update_weights();
    int choose_move(const std::vector<long long>& tabu_until,
                    long long iteration,
                    double best_score) const;
    void perturb();
};

} // namespace ramsey_tabu

#endif
