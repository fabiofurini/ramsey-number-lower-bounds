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
#include "RAMSEY_TABU_SEARCH.h"
#include "RAMSEY_MODEL_5.h"


/***********************************************************************************/
int main(int argc, char** argv)
/***********************************************************************************/
{

	data RAMSEY_instance;

	// The tabu search (input 4 = 6) is a heuristic, not a formulation: it has its own compact
	// parameter list and bypasses all branch-and-cut setup.
	// RAMSEY t m n 6 time seed max_iter tenure_min tenure_max stagnation perturb
	//        separation_period heur_restarts heur_iterations weight_period weight_increment id_test
	// Selector 5 is still accepted here, with a notice: that was the tabu search's value before
	// the linear-distance formulation took the slot, and the archived tabu campaign scripts use it.
	if (argc == 18 && (atoi(argv[4]) == 6 || atoi(argv[4]) == 5))
	{
		if (atoi(argv[4]) == 5)
		{
			cout << "\nNOTE: on this short command line the tabu search is now input 4 = 6;"
			     << " 5 is still accepted and is what you got.\n";
		}
		RAMSEY_instance.PARAM_SIZE_GRAPH = atoi(argv[1]);
		RAMSEY_instance.PARAM_M = atoi(argv[2]);
		RAMSEY_instance.PARAM_N = atoi(argv[3]);
		RAMSEY_instance.PARAM_ALGO = 6;
		RAMSEY_instance.PARAM_CIRCULANT = 1;
		RAMSEY_instance.PARAM_TIME_LIMIT = atof(argv[5]);
		RAMSEY_instance.RANDOM_SEED = atoi(argv[6]);
		RAMSEY_instance.ID_TEST = atoi(argv[17]);

		ramsey_tabu::TabuConfig tabu_config;
		tabu_config.order = RAMSEY_instance.PARAM_SIZE_GRAPH;
		tabu_config.blue_target = RAMSEY_instance.PARAM_M;
		tabu_config.red_target = RAMSEY_instance.PARAM_N;
		tabu_config.time_limit_seconds = RAMSEY_instance.PARAM_TIME_LIMIT;
		tabu_config.seed = static_cast<unsigned int>(RAMSEY_instance.RANDOM_SEED);
		tabu_config.max_iterations = atoll(argv[7]);
		tabu_config.tabu_tenure_min = atoi(argv[8]);
		tabu_config.tabu_tenure_max = atoi(argv[9]);
		tabu_config.stagnation_limit = atoll(argv[10]);
		tabu_config.perturbation_size = atoi(argv[11]);
		tabu_config.separation_period = atoll(argv[12]);
		RAMSEY_instance.PARAM_NUM_RESTARTS_MNTS = atoi(argv[13]);
		RAMSEY_instance.PARAM_NUM_ITERATIONS_MNTS = atoi(argv[14]);
		tabu_config.adaptive_weight_period = atoll(argv[15]);
		tabu_config.adaptive_weight_increment = atof(argv[16]);

		cout << "\n****RAMSEY TABU SEARCH ALGORITHM 6****\n";
		RAMSEY_TABU_SEARCH_solve(&RAMSEY_instance, tabu_config);
		cout << "\nDONE!\n\n";
		return 0;
	}

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

		// Selector numbering: on this 34-argument line the formulations are 1, 2, 3, 4 and 5,
		// with 5 the linear-distance (Toeplitz) model.  The tabu search is 6, and it only exists
		// on the short 18-argument line, because it is a heuristic and not a formulation.
		// During development the linear-distance model was 6; that value is refused here rather
		// than reinterpreted, so no old command line can quietly run something else.
		if (RAMSEY_instance.PARAM_ALGO == 6)
		{
			cout << "\n**WRONG INPUT** input 4 = 6 now selects the tabu search, which takes the"
			     << " short 18-argument command line. The linear-distance model is input 4 = 5.\n";
			exit(-1);
		}

		srand(RAMSEY_instance.RANDOM_SEED);

		// Initialize cut data structures
		RAMSEY_instance.CUTS_M.loaded = false;
		RAMSEY_instance.CUTS_M.num_lines = 0;
		RAMSEY_instance.CUTS_M.lines = NULL;
		RAMSEY_instance.CUTS_M.has_clique_data = false;
		RAMSEY_instance.CUTS_N.loaded = false;
		RAMSEY_instance.CUTS_N.num_lines = 0;
		RAMSEY_instance.CUTS_N.lines = NULL;
		RAMSEY_instance.CUTS_N.has_clique_data = false;
		RAMSEY_instance.RECORDED_CUTS_M.clear();
		RAMSEY_instance.RECORDED_CUTS_N.clear();
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

		load_cuts_from_file(&RAMSEY_instance, true, &RAMSEY_instance.CUTS_M, false);

		load_cuts_from_file(&RAMSEY_instance, false, &RAMSEY_instance.CUTS_N, false);

		cout << "**************************************\n";
	}
	else if (RAMSEY_instance.LOAD_CUTS_FROM_FILE == -100)
	{
		if (RAMSEY_instance.PARAM_ALGO != 3 && RAMSEY_instance.PARAM_ALGO != 5)
		{
			cout << "LOAD_CUTS_FROM_FILE = -100 is supported only by MODEL 3 and MODEL 5\n";
			exit(-1);
		}
		cout << "\nRecording newly generated no-clique cuts of MODEL " << RAMSEY_instance.PARAM_ALGO << " to CUTS/\n";
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

	if(RAMSEY_instance.PARAM_ALGO==5)
	{
		//DISTANCE (Toeplitz), NOT circulant: the colour of {i,j} depends on |i-j|.
		//The class is hereditary in the order, so a single infeasible order is a threshold.

		cout << "\n\n****RAMSEY BRANCH-AND-CUT ALGORITHM 5 (distance / Toeplitz)****\n";

		if(RAMSEY_instance.PARAM_CIRCULANT!=0)
		{
			cout << "NOTE: input 6 (PARAM_CIRCULANT) is meaningless for MODEL 5 and is forced to 0\n";
		}
		// Forced, not merely ignored: PARAM_CIRCULANT==1 makes the CPLEX-based separator fix
		// vertex 0 into the clique.  Fixing a vertex is in fact legitimate in this class too
		// (translate the clique so its smallest vertex is 0), but the restriction is written
		// for the circulant case and must not be relied on here; see the long note at the
		// clique-solver call sites in RAMSEY_MODEL_5.cpp.
		RAMSEY_instance.PARAM_CIRCULANT=0;

		RAMSEY_MODEL_5_load(&RAMSEY_instance);

		RAMSEY_MODEL_5_solve(&RAMSEY_instance);

		RAMSEY_MODEL_5_free(&RAMSEY_instance);
	}

	/////////////////////////////////////
	memory_deallocation(&RAMSEY_instance);
	/////////////////////////////////////


	printf("\nDONE!\n\n");

	return 1;
}
