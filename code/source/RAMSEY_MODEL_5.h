#ifndef RAMSEY_MODEL_5_HEADER
#define RAMSEY_MODEL_5_HEADER

#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <time.h>
#include <math.h>
#include <string.h>
#include <float.h>

#include "global_variables.h"
#include "global_functions.h"

// MODEL 5 = MODEL 3 in the LINEAR distance class (Toeplitz graphs) instead of the circulant one:
// the colour of {i,j} depends on |i-j|, not on min(|i-j|, t-|i-j|).
// t-1 binary variables y_d (d = 1..t-1), y_d = 1 meaning "distance d is blue".
// The class is hereditary in the order, so one infeasible order determines the threshold.

// Shared with MODEL 3 (defined in RAMSEY_MODEL_3.cpp): both depend on sizes only, not on the
// geometry, so MODEL 5 reuses them instead of duplicating them.
/***********************************************************************************/
int edge_number(int size);
/***********************************************************************************/

/***********************************************************************************/
int ex_value(int size,int k);
/***********************************************************************************/

/***********************************************************************************/
int mapping_lin(data *RAMSEY_instance,int i,int j);
/***********************************************************************************/

/***********************************************************************************/
double RAMSEY_MODEL_5_solve(data *RAMSEY_instance);
/***********************************************************************************/

/***********************************************************************************/
void RAMSEY_MODEL_5_free(data *RAMSEY_instance);
/***********************************************************************************/

/***********************************************************************************/
void RAMSEY_MODEL_5_load(data *RAMSEY_instance);
/***********************************************************************************/

#endif
