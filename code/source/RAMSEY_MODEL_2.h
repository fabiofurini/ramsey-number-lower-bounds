#ifndef RANSEY_M2_function_HEADER
#define RANSEY_M2_function_HEADER


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
double RAMSEY_MODEL_2_solve(data *RAMSEY_instance);
/***********************************************************************************/

/***********************************************************************************/
void RAMSEY_MODEL_2_free(data *RAMSEY_instance);
/***********************************************************************************/

/***********************************************************************************/
void RAMSEY_MODEL_2_load(data *RAMSEY_instance);
/***********************************************************************************/


#endif



