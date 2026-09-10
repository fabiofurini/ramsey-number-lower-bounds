#ifndef RAMSEY_TABU_SEARCH_HEADER
#define RAMSEY_TABU_SEARCH_HEADER

#include "RAMSEY_TABU_CORE.h"
#include "global_variables.h"

/* New primal-only distance-space tabu search.  It does not alter models 1--4. */
double RAMSEY_TABU_SEARCH_solve(data* RAMSEY_instance,
                            const ramsey_tabu::TabuConfig& tabu_config);

#endif
