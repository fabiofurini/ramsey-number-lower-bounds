#ifndef CLIQUE_HEADER
#define CLIQUE_HEADER


#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <time.h>
#include <iostream>
#include <fstream>
#include <math.h>
#include <string.h>
#include <errno.h>
#include <sstream>
#include <vector>
#include <algorithm>
#include <set>
#include <iomanip> // for precision

#include "global_variables.h"
#include "global_functions.h"

using namespace std;


/***************************************************************************/
void clique_load_cplex (data *RAMSEY_instance);
/***************************************************************************/

/***************************************************************************/
void clique_blue_solve_cplex (data *RAMSEY_instance);
/***************************************************************************/

/***************************************************************************/
void clique_red_solve_cplex (data *RAMSEY_instance);
/***************************************************************************/

/***************************************************************************/
void clique_free_cplex (data *RAMSEY_instance);
/***************************************************************************/

/***************************************************************************/
int clique_solve_cplex_fixing(data *RAMSEY_instance,int color);
/***************************************************************************/


#endif
