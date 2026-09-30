#include "RAMSEY_TABU_SEARCH.h"

#include <algorithm>
#include <chrono>
#include <fstream>
#include <iostream>
#include <memory>

namespace {

using ramsey_tabu::Color;
using ramsey_tabu::DistanceGeometry;
using ramsey_tabu::SupportSeed;

int distance_index(const int order, const int u, const int v, const DistanceGeometry geometry) {
    return ramsey_tabu::DistanceSpaceTabuSearch::edge_distance(order, u, v, geometry) - 1;
}

std::vector<std::uint16_t> support_from_clique(const double* clique, const int order,
                                                const DistanceGeometry geometry) {
    std::vector<std::uint16_t> support;
    for (int u = 0; u < order; ++u) {
        if (clique[u] < 0.5) {
            continue;
        }
        for (int v = u + 1; v < order; ++v) {
            if (clique[v] >= 0.5) {
                    support.push_back(static_cast<std::uint16_t>(distance_index(order, u, v, geometry) + 1));
            }
        }
    }
    std::sort(support.begin(), support.end());
    support.erase(std::unique(support.begin(), support.end()), support.end());
    return support;
}

void set_color_graph(const std::vector<std::uint8_t>& distances, const bool blue,
                     const DistanceGeometry geometry, std::vector<std::vector<int>>& matrix,
                     std::vector<int*>& rows) {
    const int order = static_cast<int>(matrix.size());
    for (int u = 0; u < order; ++u) {
        for (int v = 0; v < order; ++v) {
            if (u == v) {
                matrix[u][v] = 0;
                continue;
            }
            const bool edge_is_blue = distances[distance_index(order, u, v, geometry)] != 0;
            matrix[u][v] = edge_is_blue == blue ? 1 : 0;
        }
        rows[u] = matrix[u].data();
    }
}

const char* geometry_name(const DistanceGeometry geometry) {
    return geometry == DistanceGeometry::Circulant ? "circulant" : "linear";
}

void write_certificate(const data* instance, const std::vector<std::uint8_t>& distances,
                       const DistanceGeometry geometry) {
    const std::string prefix = geometry == DistanceGeometry::Linear ? "colorings/tabu_linear_m"
                                                                     : "colorings/tabu_m";
    const std::string filename = prefix + std::to_string(instance->PARAM_M) +
        "_n" + std::to_string(instance->PARAM_N) + "_SIZE" +
        std::to_string(instance->PARAM_SIZE_GRAPH) + "_id" +
        std::to_string(instance->ID_TEST) + ".txt";
    std::ofstream out(filename.c_str());
    out << "order " << instance->PARAM_SIZE_GRAPH << '\n';
    out << "geometry " << geometry_name(geometry) << '\n';
    out << "blue_distances";
    for (std::size_t i = 0; i < distances.size(); ++i) {
        if (distances[i] != 0) {
            out << ' ' << i + 1;
        }
    }
    out << '\n';
    out << "red_distances";
    for (std::size_t i = 0; i < distances.size(); ++i) {
        if (distances[i] == 0) {
            out << ' ' << i + 1;
        }
    }
    out << '\n';
}

} // namespace

double RAMSEY_TABU_SEARCH_solve(data* RAMSEY_instance,
                            const ramsey_tabu::TabuConfig& tabu_config) {
    using ramsey_tabu::DistanceSpaceTabuSearch;
    using ramsey_tabu::VerificationResult;

    if (tabu_config.small_clique_mask < 0 || tabu_config.small_clique_mask > 3) {
        std::cerr << "small_clique_mask must be in {0,1,2,3}" << std::endl;
        return -1.0;
    }
    const bool enumerate_blue = (tabu_config.small_clique_mask & 1) != 0;
    const bool enumerate_red = (tabu_config.small_clique_mask & 2) != 0;
    if ((enumerate_blue && tabu_config.blue_target != 3 && tabu_config.blue_target != 4) ||
        (enumerate_red && tabu_config.red_target != 3 && tabu_config.red_target != 4)) {
        std::cerr << "A complete small-clique pool is available only for K3 and K4" << std::endl;
        return -1.0;
    }
    // 1 = circulant (every vertex is a valid anchor); -1 = linear/Toeplitz
    // (anchor fixed at vertex t-1, valid by translation).
    const int is_circulant = (tabu_config.geometry == DistanceGeometry::Circulant) ? 1 : -1;

    const int order = RAMSEY_instance->PARAM_SIZE_GRAPH;
    std::vector<std::vector<int>> graph(order, std::vector<int>(order, 0));
    std::vector<int*> graph_rows(order, nullptr);
    std::vector<double> clique(order, 0.0);

    RAMSEY_instance->n_calls = 0;
    RAMSEY_instance->n_calls_heur = 0;
    RAMSEY_instance->time_MNTS = 0.0;
    RAMSEY_instance->n_CLISAT_successes = 0;
    RAMSEY_instance->n_MNTS_successes = 0;
    RAMSEY_instance->n_SimpleHeur_successes = 0;
    RAMSEY_instance->n_CLISAT_opt = 0;
    RAMSEY_instance->initialize_ug(order);

    DistanceSpaceTabuSearch tabu(tabu_config);
    const auto preenumeration_start = std::chrono::steady_clock::now();
    std::size_t preenumerated_blue = 0;
    std::size_t preenumerated_red = 0;
    if (enumerate_blue) {
        for (const SupportSeed& support : DistanceSpaceTabuSearch::enumerate_small_clique_supports(
                 order, tabu_config.blue_target, Color::Blue, tabu_config.geometry)) {
            if (tabu.add_support(support)) {
                ++preenumerated_blue;
            }
        }
    }
    if (enumerate_red) {
        for (const SupportSeed& support : DistanceSpaceTabuSearch::enumerate_small_clique_supports(
                 order, tabu_config.red_target, Color::Red, tabu_config.geometry)) {
            if (tabu.add_support(support)) {
                ++preenumerated_red;
            }
        }
    }
    const double preenumeration_seconds = std::chrono::duration<double>(
        std::chrono::steady_clock::now() - preenumeration_start).count();

    long long blue_separation_calls = 0;
    long long red_separation_calls = 0;
    long long blue_dynamic_candidates = 0;
    long long red_dynamic_candidates = 0;

    const auto separate_color = [&](const std::vector<std::uint8_t>& distances, const Color color,
                                    const int target) {
        const bool blue = color == Color::Blue;
        set_color_graph(distances, blue, tabu_config.geometry, graph, graph_rows);
        std::fill(clique.begin(), clique.end(), 0.0);
        const int size = RAMSEY_instance->clique_solve_BB_edge_fixing(
            graph_rows.data(), clique.data(), is_circulant, target,
            true, 0.0, RAMSEY_instance->PARAM_NUM_RESTARTS_MNTS,
            RAMSEY_instance->PARAM_NUM_ITERATIONS_MNTS, !blue);
        if (size < target) {
            return std::vector<SupportSeed>();
        }
        return std::vector<SupportSeed>{{color, support_from_clique(clique.data(), order,
                                                                       tabu_config.geometry), 1.0}};
    };

    const auto separate = [&](const std::vector<std::uint8_t>& distances) {
        std::vector<SupportSeed> result;
        if (!enumerate_blue) {
            ++blue_separation_calls;
            std::vector<SupportSeed> supports = separate_color(distances, Color::Blue,
                                                                RAMSEY_instance->PARAM_M);
            blue_dynamic_candidates += static_cast<long long>(supports.size());
            result.insert(result.end(), supports.begin(), supports.end());
        }
        if (!enumerate_red) {
            ++red_separation_calls;
            std::vector<SupportSeed> supports = separate_color(distances, Color::Red,
                                                                RAMSEY_instance->PARAM_N);
            red_dynamic_candidates += static_cast<long long>(supports.size());
            result.insert(result.end(), supports.begin(), supports.end());
        }
        return result;
    };

    const auto verify_exactly = [&](const std::vector<std::uint8_t>& distances) {
        VerificationResult result;
        set_color_graph(distances, true, tabu_config.geometry, graph, graph_rows);
        std::fill(clique.begin(), clique.end(), 0.0);
        const int blue_size = RAMSEY_instance->clique_solve_BB_edge_fixing(
            graph_rows.data(), clique.data(), is_circulant, RAMSEY_instance->PARAM_M,
            false, 0.0, 0, 0, false);
        if (blue_size >= RAMSEY_instance->PARAM_M) {
            result.violated_supports.push_back(
                {Color::Blue, support_from_clique(clique.data(), order, tabu_config.geometry), 1.0});
            return result;
        }

        set_color_graph(distances, false, tabu_config.geometry, graph, graph_rows);
        std::fill(clique.begin(), clique.end(), 0.0);
        const int red_size = RAMSEY_instance->clique_solve_BB_edge_fixing(
            graph_rows.data(), clique.data(), is_circulant, RAMSEY_instance->PARAM_N,
            false, 0.0, 0, 0, true);
        if (red_size >= RAMSEY_instance->PARAM_N) {
            result.violated_supports.push_back(
                {Color::Red, support_from_clique(clique.data(), order, tabu_config.geometry), 1.0});
            return result;
        }
        result.feasible = true;
        return result;
    };

    const ramsey_tabu::RunResult result = tabu.run(separate, verify_exactly);
    std::cout << "TABU geometry\t" << geometry_name(tabu_config.geometry) << '\n'
              << "TABU variables\t" << DistanceSpaceTabuSearch::number_of_distances(
                     order, tabu_config.geometry) << '\n'
              << "TABU small_clique_mask\t" << tabu_config.small_clique_mask << '\n'
              << "TABU preenumerated_blue_supports\t" << preenumerated_blue << '\n'
              << "TABU preenumerated_red_supports\t" << preenumerated_red << '\n'
              << "TABU dynamic_blue_candidates\t" << blue_dynamic_candidates << '\n'
              << "TABU dynamic_red_candidates\t" << red_dynamic_candidates << '\n'
              << "TABU blue_separation_calls\t" << blue_separation_calls << '\n'
              << "TABU red_separation_calls\t" << red_separation_calls << '\n'
              << "TABU pool_epochs\t" << result.pool_epochs << '\n'
              << "TABU preenumeration_seconds\t" << preenumeration_seconds << '\n'
              << "TABU iterations\t" << result.iterations << '\n'
              << "TABU supports\t" << tabu.support_count() << '\n'
              << "TABU best_score\t" << result.best_score << '\n'
              << "TABU separations\t" << result.separation_calls << '\n'
              << "TABU exact_verifications\t" << result.verification_calls << '\n'
              << "TABU clique_calls\t" << RAMSEY_instance->n_calls << '\n'
              << "TABU simple_heuristic_hits\t" << RAMSEY_instance->n_SimpleHeur_successes << '\n'
              << "TABU mnts_hits\t" << RAMSEY_instance->n_MNTS_successes << '\n'
              << "TABU clisat_hits\t" << RAMSEY_instance->n_CLISAT_successes << '\n'
              << "TABU clisat_optimal_calls\t" << RAMSEY_instance->n_CLISAT_opt << '\n'
              << "TABU feasible\t" << (result.feasible ? 1 : 0) << std::endl;
    if (result.feasible) {
        write_certificate(RAMSEY_instance, result.best_distances, tabu_config.geometry);
        std::cout << "TABU certificate written" << std::endl;
        return 1.0;
    }
    return 0.0;
}
