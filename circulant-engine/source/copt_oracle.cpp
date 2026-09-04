#include "copt_oracle.h"

#include "clique/clique_clisat.h"
#include "clique/clique_config.h"
#include "clique/clique_sat.h"
#include "clique/interdiction/ramsey.h"

#include <algorithm>

namespace {
using CoptSolver = Ramsey<
    copt::clique::CliqueX<bitgraph::ugraph, copt::clique::infoSAT, 128>>;
}

OracleResult CoptCliqueOracle::solve_induced(
    const DynamicBitset &candidate_vertices,
    const DynamicBitset &neighbourhood_of_zero,
    int graph_order,
    int target) const {

    OracleResult output;
    const std::vector<int> original_vertices = candidate_vertices.indices();

    if (target <= 0) {
        output.status = OracleStatus::FOUND;
        return output;
    }
    if (static_cast<int>(original_vertices.size()) < target) {
        output.status = OracleStatus::NOT_FOUND_EXACT;
        return output;
    }
    if (target == 1) {
        output.status = OracleStatus::FOUND;
        output.witness.push_back(original_vertices.front());
        return output;
    }

    const int subgraph_order = static_cast<int>(original_vertices.size());
    EdgeFixture fixture(subgraph_order);
    for (int i = 0; i < subgraph_order - 1; ++i) {
        for (int j = i + 1; j < subgraph_order; ++j) {
            int difference = original_vertices[j] - original_vertices[i];
            difference %= graph_order;
            if (difference < 0) difference += graph_order;
            if (neighbourhood_of_zero.test(static_cast<std::size_t>(difference))) {
                fixture.add_edge(i, j);
            }
        }
    }

    CoptSolver solver;
    solver.init(subgraph_order, target, target, timeout_seconds_);
    solver.set_edges(fixture);
    const auto result = solver.run_edge(
        false, false, -1.0, target, 1, 1, false, 0, false);

    output.steps = static_cast<std::uint64_t>(std::max(0, result.nb_steps));
    output.timed_out = result.tout_clisat;
    if (result.tout_clisat) {
        output.status = OracleStatus::UNKNOWN;
        return output;
    }

    if (result.w >= target && static_cast<int>(result.sol.size()) >= target) {
        output.status = OracleStatus::FOUND;
        output.witness.reserve(static_cast<std::size_t>(target));
        for (int local_vertex : result.sol) {
            if (local_vertex >= 0 && local_vertex < subgraph_order) {
                output.witness.push_back(original_vertices[static_cast<std::size_t>(local_vertex)]);
                if (static_cast<int>(output.witness.size()) == target) break;
            }
        }
        if (static_cast<int>(output.witness.size()) != target) {
            output.status = OracleStatus::UNKNOWN;
            output.witness.clear();
        }
        return output;
    }

    // run_edge is an exact target-clique decision procedure.  cert_opt means
    // that the *maximum clique value* is certified; it can be false when an
    // upper bound already proves w < target.  For separation, w < target and
    // no timeout is therefore already an exact NOT_FOUND answer.
    if (result.w < target) {
        output.status = OracleStatus::NOT_FOUND_EXACT;
        return output;
    }

    output.status = OracleStatus::UNKNOWN;
    return output;
}
