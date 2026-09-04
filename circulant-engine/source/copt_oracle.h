#ifndef HYBRID_GENCYC_COPT_ORACLE_H
#define HYBRID_GENCYC_COPT_ORACLE_H

#include "dynamic_bitset.h"

#include <cstdint>
#include <vector>

enum class OracleStatus { FOUND, NOT_FOUND_EXACT, UNKNOWN };

struct OracleResult {
    OracleStatus status = OracleStatus::UNKNOWN;
    std::vector<int> witness;
    std::uint64_t steps = 0;
    bool timed_out = false;
};

class CoptCliqueOracle {
public:
    explicit CoptCliqueOracle(double timeout_seconds = -1.0)
        : timeout_seconds_(timeout_seconds) {}

    OracleResult solve_induced(
        const DynamicBitset &candidate_vertices,
        const DynamicBitset &neighbourhood_of_zero,
        int graph_order,
        int target) const;

private:
    double timeout_seconds_;
};

#endif
