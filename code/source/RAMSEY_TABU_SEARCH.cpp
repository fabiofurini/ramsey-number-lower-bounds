#include "RAMSEY_TABU_SEARCH.h"

#include <algorithm>
#include <fstream>
#include <iostream>
#include <memory>

namespace {

using ramsey_tabu::Color;
using ramsey_tabu::SupportSeed;

int distance_index(const int order, const int u, const int v) {
    return ramsey_tabu::DistanceSpaceTabuSearch::circular_distance(order, u, v) - 1;
}

std::vector<std::uint16_t> support_from_clique(const double* clique, const int order) {
    std::vector<std::uint16_t> support;
    for (int u = 0; u < order; ++u) {
        if (clique[u] < 0.5) {
            continue;
        }
        for (int v = u + 1; v < order; ++v) {
            if (clique[v] >= 0.5) {
                support.push_back(static_cast<std::uint16_t>(distance_index(order, u, v) + 1));
            }
        }
    }
    std::sort(support.begin(), support.end());
    support.erase(std::unique(support.begin(), support.end()), support.end());
    return support;
}

void set_color_graph(const std::vector<std::uint8_t>& distances, const bool blue,
                     std::vector<std::vector<int>>& matrix, std::vector<int*>& rows) {
    const int order = static_cast<int>(matrix.size());
    for (int u = 0; u < order; ++u) {
        for (int v = 0; v < order; ++v) {
            if (u == v) {
                matrix[u][v] = 0;
                continue;
            }
            const bool edge_is_blue = distances[distance_index(order, u, v)] != 0;
            matrix[u][v] = edge_is_blue == blue ? 1 : 0;
        }
        rows[u] = matrix[u].data();
    }
}

void write_certificate(const data* instance, const std::vector<std::uint8_t>& distances) {
    const std::string filename = "colorings/tabu_m" + std::to_string(instance->PARAM_M) +
        "_n" + std::to_string(instance->PARAM_N) + "_SIZE" +
        std::to_string(instance->PARAM_SIZE_GRAPH) + "_id" +
        std::to_string(instance->ID_TEST) + ".txt";
    std::ofstream out(filename.c_str());
    out << "order " << instance->PARAM_SIZE_GRAPH << '\n';
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

    if (RAMSEY_instance->PARAM_CIRCULANT != 1) {
        std::cerr << "The tabu search requires PARAM_CIRCULANT = 1" << std::endl;
        return -1.0;
    }
    if (tabu_config.blue_target != 3) {
        std::cerr << "The tabu search currently supports R(3,n) only" << std::endl;
        return -1.0;
    }

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
    for (const SupportSeed& triangle : DistanceSpaceTabuSearch::enumerate_triangle_supports(order, Color::Blue)) {
        tabu.add_support(triangle);
    }

    const auto separate_red = [&](const std::vector<std::uint8_t>& distances) {
        set_color_graph(distances, false, graph, graph_rows);
        std::fill(clique.begin(), clique.end(), 0.0);
        // Reuse the same target-clique separator used by the red callback of
        // legacy model 3.  The normal routine runs its built-in simple/MNTS
        // heuristics before falling back to the CliSAT exact search.
        const int size = RAMSEY_instance->clique_solve_BB_edge_fixing(
            graph_rows.data(), clique.data(), true, RAMSEY_instance->PARAM_N,
            true, 0.0, RAMSEY_instance->PARAM_NUM_RESTARTS_MNTS,
            RAMSEY_instance->PARAM_NUM_ITERATIONS_MNTS, true);
        if (size < RAMSEY_instance->PARAM_N) {
            return std::vector<SupportSeed>();
        }
        return std::vector<SupportSeed>{
            {Color::Red, support_from_clique(clique.data(), order), 1.0}};
    };

    const auto verify_exactly = [&](const std::vector<std::uint8_t>& distances) {
        VerificationResult result;
        set_color_graph(distances, true, graph, graph_rows);
        std::fill(clique.begin(), clique.end(), 0.0);
        const int blue_size = RAMSEY_instance->clique_solve_BB_edge_fixing(
            graph_rows.data(), clique.data(), true, RAMSEY_instance->PARAM_M,
            false, 0.0, 0, 0, false);
        if (blue_size >= RAMSEY_instance->PARAM_M) {
            result.violated_supports.push_back(
                {Color::Blue, support_from_clique(clique.data(), order), 1.0});
            return result;
        }

        set_color_graph(distances, false, graph, graph_rows);
        std::fill(clique.begin(), clique.end(), 0.0);
        const int red_size = RAMSEY_instance->clique_solve_BB_edge_fixing(
            graph_rows.data(), clique.data(), true, RAMSEY_instance->PARAM_N,
            false, 0.0, 0, 0, true);
        if (red_size >= RAMSEY_instance->PARAM_N) {
            result.violated_supports.push_back(
                {Color::Red, support_from_clique(clique.data(), order), 1.0});
            return result;
        }
        result.feasible = true;
        return result;
    };

    const ramsey_tabu::RunResult result = tabu.run(separate_red, verify_exactly);
    std::cout << "TABU iterations\t" << result.iterations << '\n'
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
        write_certificate(RAMSEY_instance, result.best_distances);
        std::cout << "TABU certificate written" << std::endl;
        return 1.0;
    }
    return 0.0;
}
