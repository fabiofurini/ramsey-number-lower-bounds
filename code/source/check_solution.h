#ifndef CHECK_FUNCTIONS_HEADER
#define CHECK_FUNCTIONS_HEADER


#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <time.h>
#include <iostream>
#include <fstream>
#include <math.h>
#include <vector>
#include <set>
#include <string>
#include <sstream>
#include <errno.h>
#include <cstring>
#include <cstdlib>

using namespace std;

#include "global_variables.h"
#include "global_functions.h"

/***********************************************************************************/
bool check_solution(data *RAMSEY_instance,double *X_SOLUTION);
/***********************************************************************************/

/***********************************************************************************/
void draw_tex_solution_both_cliques(data *RAMSEY_instance,double *X_SOLUTION);
/***********************************************************************************/

/***********************************************************************************/
void draw_tex_matrix_bis(data *RAMSEY_instance,double *X_SOLUTION);
/***********************************************************************************/

/***********************************************************************************/
void write_row_sol(data *RAMSEY_instance,double *X_SOLUTION);
/***********************************************************************************/

/***********************************************************************************/
void draw_tex_solution_blue_cliques(data *RAMSEY_instance,double *X_SOLUTION);
/***********************************************************************************/

/***********************************************************************************/
void draw_tex_solution_red_cliques(data *RAMSEY_instance,double *X_SOLUTION);
/***********************************************************************************/

#endif
