#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <time.h>
#include <iostream>
#include <fstream>
#include <math.h>
#include <float.h>

using namespace std;

#include "global_variables.h"
#include "global_functions.h"

#include "RAMSEY_MODEL_1.h"
#include "RAMSEY_MODEL_2.h"
#include "RAMSEY_MODEL_3.h"
#include "RAMSEY_MODEL_4.h"


/***********************************************************************************/
int main(int argc, char** argv)
/***********************************************************************************/
{

	data RAMSEY_instance;

	//////////////////////////////////////////////////////////////////////////

	RAMSEY_instance.SKIP_M_SEPARATION=false;
	RAMSEY_instance.SKIP_N_SEPARATION=false;

	if (argc == 34)
	{
		RAMSEY_instance.PARAM_SIZE_GRAPH=atoi(argv[1]);
		RAMSEY_instance.PARAM_M=atoi(argv[2]);
		RAMSEY_instance.PARAM_N=atoi(argv[3]);
		RAMSEY_instance.PARAM_ALGO=atoi(argv[4]);
		RAMSEY_instance.PARAM_OPTIONS=atoi(argv[5]);
		RAMSEY_instance.PARAM_CIRCULANT=atoi(argv[6]);
		RAMSEY_instance.PARAM_TIME_LIMIT=atof(argv[7]);
		RAMSEY_instance.PARAM_STRONGER_CUTS=atoi(argv[8]);
		RAMSEY_instance.PARAM_CPLEX=atoi(argv[9]);
		RAMSEY_instance.PARAM_MNTS=atoi(argv[10]);
		RAMSEY_instance.PARAM_TOUT_MNTS=atof(argv[11]);
		RAMSEY_instance.PARAM_NUM_RESTARTS_MNTS=atof(argv[12]);
		RAMSEY_instance.PARAM_NUM_ITERATIONS_MNTS=atof(argv[13]);
		RAMSEY_instance.PARAM_K_CUTS=atoi(argv[14]);
		RAMSEY_instance.PARAM_CLIQUE_JUMP_CUTS=atoi(argv[15]);
		RAMSEY_instance.PARAM_COVER_CUTS=atoi(argv[16]);
		RAMSEY_instance.PARAM_CUT_LOOP=atoi(argv[17]);
		RAMSEY_instance.AVOID_TRIANGLES=atoi(argv[18]);
		RAMSEY_instance.AVOID_QUADRANGLES=atoi(argv[19]);
		RAMSEY_instance.CHECK_SOLUTION=atoi(argv[20]);
		RAMSEY_instance.CUT_CALL_BACK_STRATEGY=atoi(argv[21]);
		RAMSEY_instance.BRANCHING_STRATEGY=atoi(argv[22]);
		RAMSEY_instance.TREE_EXPLORATION_STRATEGY = atoi(argv[23]);
		RAMSEY_instance.HEURFREQ = atoi(argv[24]);
		RAMSEY_instance.CLIQUE_TARGET = atoi(argv[25]);
		RAMSEY_instance.CLIQUE_TARGET_RESIZE = atoi(argv[26]);
		RAMSEY_instance.MULTIPLE_CUTS = atoi(argv[27]);
		RAMSEY_instance.RANDOM_SEED= atoi(argv[28]);
		RAMSEY_instance.BRANCHING_VARIABLE_SELECTION= atoi(argv[29]);
		RAMSEY_instance.NUMBER_OF_THREADS= atoi(argv[30]);
		RAMSEY_instance.LOAD_CUTS_FROM_FILE = atoi(argv[31]);
		RAMSEY_instance.MINIMIZE_CUTS = atoi(argv[32]);
		RAMSEY_instance.ID_TEST = atoi(argv[33]);

		srand(RAMSEY_instance.RANDOM_SEED);

		// Initialize cut data structures
		RAMSEY_instance.CUTS_M.loaded = false;
		RAMSEY_instance.CUTS_M.num_lines = 0;
		RAMSEY_instance.CUTS_M.lines = NULL;
		RAMSEY_instance.CUTS_N.loaded = false;
		RAMSEY_instance.CUTS_N.num_lines = 0;
		RAMSEY_instance.CUTS_N.lines = NULL;
	}
	else
	{
		cout << "argc\t" << argc << endl;
		cout << "**WRONG INPUT**\n\n";
		exit(-1);
	}

	cout << "\n**************************************\n\n";
	cout << "PARAM_SIZE_GRAPH->\t" <<RAMSEY_instance.PARAM_SIZE_GRAPH << endl;
	cout << "PARAM_M->\t" <<RAMSEY_instance.PARAM_M << endl;
	cout << "PARAM_N->\t" <<RAMSEY_instance.PARAM_N << endl;
	cout << "PARAM_ALGO->\t" <<RAMSEY_instance.PARAM_ALGO << endl;
	cout << "PARAM_OPTIONS->\t" <<RAMSEY_instance.PARAM_OPTIONS << endl;
	cout << "PARAM_CIRCULANT->\t" <<RAMSEY_instance.PARAM_CIRCULANT << endl;
	cout << "PARAM_TIME_LIMIT->\t" <<RAMSEY_instance.PARAM_TIME_LIMIT << endl;
	cout << "PARAM_STRONGER_CUTS->\t" <<RAMSEY_instance.PARAM_STRONGER_CUTS << endl;
	cout << "PARAM_CPLEX->\t" <<RAMSEY_instance.PARAM_CPLEX << endl;
	cout << "PARAM_MNTS->\t" <<RAMSEY_instance.PARAM_MNTS << endl;
	cout << "PARAM_TOUT_MNTS->\t" <<RAMSEY_instance.PARAM_TOUT_MNTS << endl;
	cout << "PARAM_NUM_RESTARTS_MNTS->\t" <<RAMSEY_instance.PARAM_NUM_RESTARTS_MNTS << endl;
	cout << "PARAM_NUM_ITERATIONS_MNTS->\t" <<RAMSEY_instance.PARAM_NUM_ITERATIONS_MNTS << endl;
	cout << "PARAM_K_CUTS->\t" <<RAMSEY_instance.PARAM_K_CUTS << endl;
	cout << "PARAM_CLIQUE_JUMP_CUTS->\t" <<RAMSEY_instance.PARAM_CLIQUE_JUMP_CUTS << endl;
	cout << "PARAM_COVER_CUTS->\t" <<RAMSEY_instance.PARAM_COVER_CUTS << endl;
	cout << "PARAM_CUT_LOOP->\t" <<RAMSEY_instance.PARAM_CUT_LOOP << endl;
	cout << "AVOID_TRIANGLES->\t" <<RAMSEY_instance.AVOID_TRIANGLES << endl;
	cout << "AVOID_QUADRANGLES->\t" <<RAMSEY_instance.AVOID_QUADRANGLES << endl;
	cout << "CHECK_SOLUTION->\t" <<RAMSEY_instance.CHECK_SOLUTION << endl;
	cout << "CUT_CALL_BACK_STRATEGY->\t" <<RAMSEY_instance.CUT_CALL_BACK_STRATEGY << endl;
	cout << "BRANCHING_STRATEGY->\t" <<RAMSEY_instance.BRANCHING_STRATEGY << endl;
	cout << "TREE_EXPLORATION_STRATEGY->\t" <<RAMSEY_instance.TREE_EXPLORATION_STRATEGY << endl;
	cout << "HEURFREQ->\t" << RAMSEY_instance.HEURFREQ << endl;
	cout << "CLIQUE_TARGET->\t" <<RAMSEY_instance.CLIQUE_TARGET << endl;
	cout << "CLIQUE_TARGET_RESIZE->\t" << RAMSEY_instance.CLIQUE_TARGET_RESIZE << endl;
	cout << "MULTIPLE_CUTS->\t" << RAMSEY_instance.MULTIPLE_CUTS << endl;
	cout << "RANDOM_SEED->\t" << RAMSEY_instance.RANDOM_SEED << endl;
	cout << "BRANCHING_VARIABLE_SELECTION->\t" << RAMSEY_instance.BRANCHING_VARIABLE_SELECTION << endl;
	cout << "NUMBER_OF_THREADS->\t" << RAMSEY_instance.NUMBER_OF_THREADS << endl;
	cout << "LOAD_CUTS_FROM_FILE->\t" <<RAMSEY_instance.LOAD_CUTS_FROM_FILE << endl;
	cout << "MINIMIZE_CUTS->\t" <<RAMSEY_instance.MINIMIZE_CUTS << endl;
	cout << "ID_TEST->\t" <<RAMSEY_instance.ID_TEST << endl;

	cout << "\n**************************************\n";
	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////

	RAMSEY_instance.CLIQUE_JUMP_CUTS_M=0;
	RAMSEY_instance.CLIQUE_JUMP_CUTS_N=0;
	RAMSEY_instance.TRIANGLES_CUTS_M=0;
	RAMSEY_instance.TRIANGLES_CUTS_N=0;
	RAMSEY_instance.QUADRANGLES_CUTS_M=0;
	RAMSEY_instance.QUADRANGLES_CUTS_N=0;

	/////////////////////////////////////
	memory_allocation(&RAMSEY_instance);
	/////////////////////////////////////

	/////////////////////////////////////
	FILL_RAMSEY_LOOK_UP(&RAMSEY_instance);
	/////////////////////////////////////

	/////////////////////////////////////
	// Load cuts from files (if requested)
	/////////////////////////////////////
	if (RAMSEY_instance.LOAD_CUTS_FROM_FILE == 1)
	{
		cout << "\n**************************************\n";
		cout << "Loading cuts from files...\n";

		// Search for file for PARAM_M: t<SIZE_GRAPH>_k<PARAM_M>.txt
		load_cuts_from_file(&RAMSEY_instance, RAMSEY_instance.PARAM_SIZE_GRAPH, RAMSEY_instance.PARAM_M, &RAMSEY_instance.CUTS_M, false);

		// Search for file for PARAM_N: t<SIZE_GRAPH>_k<PARAM_N>.txt
		load_cuts_from_file(&RAMSEY_instance, RAMSEY_instance.PARAM_SIZE_GRAPH, RAMSEY_instance.PARAM_N, &RAMSEY_instance.CUTS_N, false);

		cout << "**************************************\n";
	}
	else
	{
		cout << "\nSkipping cuts from files (LOAD_CUTS_FROM_FILE = 0)\n";
	}


	////////////////////////////////////////////////////////////////////////////////////////////////////
	////////////////////////////////////////////////////////////////////////////////////////////////////
	RAMSEY_instance.initialize_ug(RAMSEY_instance.PARAM_SIZE_GRAPH);

	for (int i = 0; i < RAMSEY_instance.PARAM_SIZE_GRAPH; i++)
	{
		for (int j = 0; j < RAMSEY_instance.PARAM_SIZE_GRAPH; j++)
		{
			RAMSEY_instance.edge_fixing[i][j]=1;
		}
	}
	////////////////////////////////////////////////////////////////////////////////////////////////////
	////////////////////////////////////////////////////////////////////////////////////////////////////

	if(RAMSEY_instance.PARAM_ALGO==1)
	{
		cout << "\n\n****RAMSEY BRANCH-AND-CUT ALGORITHM 1****\n";

		RAMSEY_MODEL_1_load(&RAMSEY_instance);

		RAMSEY_MODEL_1_solve(&RAMSEY_instance);

		RAMSEY_MODEL_1_free(&RAMSEY_instance);
	}

	if(RAMSEY_instance.PARAM_ALGO==2)
	{
		cout << "\n\n****RAMSEY BRANCH-AND-CUT ALGORITHM 2****\n";

		RAMSEY_MODEL_2_load(&RAMSEY_instance);

		RAMSEY_MODEL_2_solve(&RAMSEY_instance);

		RAMSEY_MODEL_2_free(&RAMSEY_instance);
	}

	if(RAMSEY_instance.PARAM_ALGO==3)
	{
		//ONLY CIRCULANT!

		cout << "\n\n****RAMSEY BRANCH-AND-CUT ALGORITHM 3****\n";

		RAMSEY_MODEL_3_load(&RAMSEY_instance);

		RAMSEY_MODEL_3_solve(&RAMSEY_instance);

		RAMSEY_MODEL_3_free(&RAMSEY_instance);
	}

	if(RAMSEY_instance.PARAM_ALGO==4)
	{
		//ONLY CIRCULANT!

		cout << "\n\n****RAMSEY BRANCH-AND-CUT ALGORITHM 4****\n";

		RAMSEY_MODEL_4_load(&RAMSEY_instance);

		RAMSEY_MODEL_4_solve(&RAMSEY_instance);

		RAMSEY_MODEL_4_free(&RAMSEY_instance);
	}

	/////////////////////////////////////
	memory_deallocation(&RAMSEY_instance);
	/////////////////////////////////////


	printf("\nDONE!\n\n");

	return 1;
}



