#ifndef RANSEY_M1_function_HEADER
#define RANSEY_M1_function_HEADER


#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <time.h>
#include <math.h>
#include <string.h>
#include <float.h>

#include "global_variables.h"
#include "global_functions.h"


/***********************************************************************************/
double RAMSEY_MODEL_1_solve(data *RAMSEY_instance);
/***********************************************************************************/

/***********************************************************************************/
void RAMSEY_MODEL_1_free(data *RAMSEY_instance);
/***********************************************************************************/

/***********************************************************************************/
void RAMSEY_MODEL_1_load(data *RAMSEY_instance);
/***********************************************************************************/


#endif



