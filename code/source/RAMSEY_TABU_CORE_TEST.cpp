#include "RAMSEY_TABU_CORE.h"

#include <algorithm>
#include <cassert>
#include <cmath>
#include <cstdint>
#include <functional>
#include <iostream>
#include <random>
#include <set>
#include <vector>

using ramsey_tabu::Color;
using ramsey_tabu::DistanceGeometry;
using ramsey_tabu::DistanceSpaceTabuSearch;
using ramsey_tabu::SupportSeed;
using ramsey_tabu::TabuConfig;

namespace {

std::set<std::vector<std::uint16_t>> support_set(const std::vector<SupportSeed>& supports) {
    std::set<std::vector<std::uint16_t>> result;
    for (const SupportSeed& support : supports) result.insert(support.distances);
    return result;
}

std::set<std::vector<std::uint16_t>> exhaustive_vertex_supports(
    const int order, const int clique_size, const DistanceGeometry geometry) {
    std::set<std::vector<std::uint16_t>> result;
    std::vector<int> vertices;
    std::function<void(int)> extend = [&](const int first) {
        if (static_cast<int>(vertices.size()) == clique_size) {
            std::vector<std::uint16_t> support;
            for (int i = 0; i < clique_size; ++i) {
                for (int j = i + 1; j < clique_size; ++j) {
                    support.push_back(static_cast<std::uint16_t>(
                        DistanceSpaceTabuSearch::edge_distance(order, vertices[i], vertices[j], geometry)));
                }
            }
            std::sort(support.begin(), support.end());
            support.erase(std::unique(support.begin(), support.end()), support.end());
            result.insert(support);
            return;
        }
        const int missing = clique_size - static_cast<int>(vertices.size());
        for (int vertex = first; vertex <= order - missing; ++vertex) {
            vertices.push_back(vertex);
            extend(vertex + 1);
            vertices.pop_back();
        }
    };
    extend(0);
    return result;
}

void test_geometry() {
    assert(DistanceSpaceTabuSearch::number_of_distances(11, DistanceGeometry::Circulant) == 5);
    assert(DistanceSpaceTabuSearch::number_of_distances(11, DistanceGeometry::Linear) == 10);
    assert(DistanceSpaceTabuSearch::edge_distance(11, 0, 8, DistanceGeometry::Circulant) == 3);
    assert(DistanceSpaceTabuSearch::edge_distance(11, 0, 8, DistanceGeometry::Linear) == 8);
    assert(DistanceSpaceTabuSearch::edge_distance(10, 0, 5, DistanceGeometry::Circulant) == 5);
    assert(DistanceSpaceTabuSearch::edge_distance(10, 0, 9, DistanceGeometry::Linear) == 9);
}

void test_small_clique_supports() {
    for (const DistanceGeometry geometry : {DistanceGeometry::Circulant, DistanceGeometry::Linear}) {
        for (int clique_size = 3; clique_size <= 4; ++clique_size) {
            for (int order = clique_size; order <= 10; ++order) {
                const std::set<std::vector<std::uint16_t>> expected =
                    exhaustive_vertex_supports(order, clique_size, geometry);
                const std::vector<SupportSeed> blue = DistanceSpaceTabuSearch::enumerate_small_clique_supports(
                    order, clique_size, Color::Blue, geometry);
                const std::vector<SupportSeed> red = DistanceSpaceTabuSearch::enumerate_small_clique_supports(
                    order, clique_size, Color::Red, geometry);
                assert(support_set(blue) == expected);
                assert(support_set(red) == expected);
                for (const SupportSeed& support : blue) {
                    assert(support.color == Color::Blue);
                    for (const std::uint16_t distance : support.distances) {
                        assert(distance >= 1 && distance <=
                               DistanceSpaceTabuSearch::number_of_distances(order, geometry));
                    }
                }
            }
        }
    }
    assert(support_set(DistanceSpaceTabuSearch::enumerate_triangle_supports(7, Color::Blue)) ==
           exhaustive_vertex_supports(7, 3, DistanceGeometry::Circulant));
}

void test_complete_pool_against_vertex_reference() {
    for (const DistanceGeometry geometry : {DistanceGeometry::Circulant, DistanceGeometry::Linear}) {
        for (int order = 3; order <= 8; ++order) {
            TabuConfig config;
            config.order = order;
            config.geometry = geometry;
            config.seed = 19;
            DistanceSpaceTabuSearch tabu(config);
            const std::vector<SupportSeed> blue = DistanceSpaceTabuSearch::enumerate_small_clique_supports(
                order, 3, Color::Blue, geometry);
            const std::vector<SupportSeed> red = DistanceSpaceTabuSearch::enumerate_small_clique_supports(
                order, 4, Color::Red, geometry);
            for (const SupportSeed& support : blue) tabu.add_support(support);
            for (const SupportSeed& support : red) tabu.add_support(support);
            const std::set<std::vector<std::uint16_t>> blue_reference = support_set(blue);
            const std::set<std::vector<std::uint16_t>> red_reference = support_set(red);
            const int width = DistanceSpaceTabuSearch::number_of_distances(order, geometry);
            for (unsigned int mask = 0; mask < (1U << width); ++mask) {
                std::vector<std::uint8_t> values(width, 0);
                for (int d = 0; d < width; ++d) values[d] = (mask >> d) & 1U;
                tabu.set_distances(values);
                bool violation = false;
                for (const std::vector<std::uint16_t>& support : blue_reference) {
                    bool all_blue = true;
                    for (const std::uint16_t distance : support) all_blue = all_blue && values[distance - 1] != 0;
                    violation = violation || all_blue;
                }
                for (const std::vector<std::uint16_t>& support : red_reference) {
                    bool all_red = true;
                    for (const std::uint16_t distance : support) all_red = all_red && values[distance - 1] == 0;
                    violation = violation || all_red;
                }
                assert((tabu.score() == 0.0) == !violation);
                assert(tabu.debug_check_invariants());
            }
        }
    }
}

void test_incremental_deltas() {
    for (const DistanceGeometry geometry : {DistanceGeometry::Circulant, DistanceGeometry::Linear}) {
        TabuConfig config;
        config.order = 11;
        config.geometry = geometry;
        config.seed = 19;
        config.near_penalty_one = 0.2;
        DistanceSpaceTabuSearch tabu(config);
        for (const SupportSeed& support : DistanceSpaceTabuSearch::enumerate_small_clique_supports(
                 11, 3, Color::Blue, geometry)) tabu.add_support(support);
        for (const SupportSeed& support : DistanceSpaceTabuSearch::enumerate_small_clique_supports(
                 11, 4, Color::Red, geometry)) tabu.add_support(support);
        std::mt19937 rng(91);
        std::uniform_int_distribution<int> distance(
            0, DistanceSpaceTabuSearch::number_of_distances(11, geometry) - 1);
        for (int step = 0; step < 1000; ++step) {
            const int move = distance(rng);
            const double predicted = tabu.score_after_flip(move);
            tabu.flip(move);
            assert(std::fabs(predicted - tabu.score()) < 1e-9);
            assert(tabu.debug_check_invariants());
        }
    }
}

void test_verification_gate() {
    TabuConfig config;
    config.order = 5;
    config.max_iterations = 3;
    config.time_limit_seconds = 1.0;
    config.seed = 1;
    DistanceSpaceTabuSearch tabu(config);
    int verification_calls = 0;
    const ramsey_tabu::RunResult result = tabu.run(
        [](const std::vector<std::uint8_t>&) { return std::vector<SupportSeed>(); },
        [&verification_calls](const std::vector<std::uint8_t>&) {
            ++verification_calls;
            ramsey_tabu::VerificationResult verified;
            verified.feasible = true;
            return verified;
        });
    assert(result.feasible);
    assert(verification_calls == 1);
    assert(result.pool_epochs == 1);
}

} // namespace

int main() {
    test_geometry();
    test_small_clique_supports();
    test_complete_pool_against_vertex_reference();
    test_incremental_deltas();
    test_verification_gate();
    std::cout << "RAMSEY_TABU_CORE_TEST: PASS" << std::endl;
    return 0;
}
