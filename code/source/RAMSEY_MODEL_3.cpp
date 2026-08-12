
#include "RAMSEY_MODEL_3.h"

//#define PRINT_SOLUTION_MODEL_3
//#define PRINT_SOLUTION_CALLBACK
//#define PRINT_MODEL_3_LP

//#define print_cuts
//#define print_clique_size

//#define JUST_ONE_ROUND_OF_MINIMALIZATION

/***********************************************************************************/
int edge_number(int size)
/***********************************************************************************/
{
	return  ( ( size * (size - 1) ) / 2 );
}

/***********************************************************************************/
int ex_value(int size,int k)
/***********************************************************************************/
{
	if( size < k )
	{
		cout << "ERROR: ex_value" << size << "\t" << k << endl;
		exit(-1);
	}

	return  ( edge_number( size ) - magic_formula( size , k ) );
}

/***********************************************************************************/
void RAMSEY_MODEL_3_allocation(data *RAMSEY_instance)
/***********************************************************************************/
{
	RAMSEY_instance->n_variable_MODEL_3=RAMSEY_instance->PARAM_SIZE_GRAPH/2.0;

	RAMSEY_instance->time_clique_BB=0;
	RAMSEY_instance->time_clique_cplex=0;
	RAMSEY_instance->n_clique_calls_red=0;
	RAMSEY_instance->n_clique_calls_blue=0;
	RAMSEY_instance->n_cuts_red=0;
	RAMSEY_instance->n_cuts_blue=0;
	RAMSEY_instance->n_cuts_red_plus=0;
	RAMSEY_instance->n_cuts_blue_plus=0;

	RAMSEY_instance->n_jumps_minimized_blue=0;
	RAMSEY_instance->n_jumps_minimized_red=0;
	RAMSEY_instance->n_minimization_successes_blue=0;
	RAMSEY_instance->n_minimization_successes_red=0;
	RAMSEY_instance->time_minimization=0;

	RAMSEY_instance->X_CALLBACK=new double[RAMSEY_instance->n_variable_MODEL_3];
	RAMSEY_instance->cut_rmatind=new int[RAMSEY_instance->n_variable_MODEL_3];
	RAMSEY_instance->cut_rmatval=new double[RAMSEY_instance->n_variable_MODEL_3];
	RAMSEY_instance->X_MODEL_3=new double[RAMSEY_instance->n_variable_MODEL_3];

	// Allocate X_CALLBACK_TEMP for minimization (edge_fixing_TEMP and CLIQUE_SOL_TEMP are in memory_allocation)
	RAMSEY_instance->X_CALLBACK_TEMP=new double[RAMSEY_instance->n_variable_MODEL_3];

	RAMSEY_instance->n_calls=0;
	RAMSEY_instance->n_calls_heur=0;
	RAMSEY_instance->time_MNTS=0;
	RAMSEY_instance->n_CLISAT_successes=0;
	RAMSEY_instance->n_MNTS_successes=0;
	RAMSEY_instance->n_SimpleHeur_successes=0;
	RAMSEY_instance->n_CLISAT_opt=0;

}

/***********************************************************************************/
void RAMSEY_MODEL_3_deallocation(data *RAMSEY_instance)
/***********************************************************************************/
{

	delete []RAMSEY_instance->cut_rmatind;
	delete []RAMSEY_instance->cut_rmatval;
	delete []RAMSEY_instance->X_CALLBACK;
	delete []RAMSEY_instance->X_MODEL_3;

	// Deallocate X_CALLBACK_TEMP (edge_fixing_TEMP and CLIQUE_SOL_TEMP in memory_deallocation)
	delete []RAMSEY_instance->X_CALLBACK_TEMP;

}


/***********************************************************************************/
int CPXPUBLIC mycutcallback_LAZY_MODEL_3(CPXCENVptr env,void *cbdata,int wherefrom,void *cbhandle,int *useraction_p)
/***********************************************************************************/
{

	(*useraction_p)=CPX_CALLBACK_DEFAULT;

	data *RAMSEY_instance=(data *) cbhandle;


	RAMSEY_instance->status=CPXgetcallbacknodex(env,cbdata,wherefrom,RAMSEY_instance->X_CALLBACK,0,RAMSEY_instance->n_variable_MODEL_3-1);
	if(RAMSEY_instance->status!=0)
	{
		printf("cannot get the x\n");
		exit(-1);
	}


#ifdef PRINT_SOLUTION_CALLBACK

	double _OBJ_VALUE;
	RAMSEY_instance->status=CPXgetcallbacknodeobjval(env,cbdata,wherefrom,&_OBJ_VALUE);
	if(RAMSEY_instance->status!=0){
		printf("cannot get the x\n");
		exit(-1);
	}

	double _BEST_INTEGER;
	RAMSEY_instance->status=CPXgetcallbackinfo(env,cbdata,wherefrom,CPX_CALLBACK_INFO_BEST_INTEGER,&_BEST_INTEGER);
	if(RAMSEY_instance->status!=0){
		printf("cannot get the x\n");
		exit(-1);
	}
	double _BEST_REMAINING;
	RAMSEY_instance->status=CPXgetcallbackinfo(env,cbdata,wherefrom,CPX_CALLBACK_INFO_BEST_REMAINING,&_BEST_REMAINING);
	if(RAMSEY_instance->status!=0){
		printf("cannot get the x\n");
		exit(-1);
	}
	int _NODE;
	RAMSEY_instance->status=CPXgetcallbackinfo(env,cbdata,wherefrom,CPX_CALLBACK_INFO_NODE_COUNT,&_NODE);
	if(RAMSEY_instance->status!=0){
		printf("cannot get the x\n");
		exit(-1);
	}

	cout << "\n\n******************************************************\n\n";

	cout << _BEST_INTEGER << " _BEST_INTEGER " << _BEST_REMAINING  << " _BEST_REMAINING " << " _NODE " << _NODE <<  endl;

	cout << "_OBJ_VALUE\t" << _OBJ_VALUE << endl;

	cout << "\n\nSOL:\n";
	for(int i=0; i<RAMSEY_instance->n_variable_MODEL_3; i++)
	{
		cout << "i\t" << i << "\t" << RAMSEY_instance->X_CALLBACK[i] << endl;
	}


	cout << "\n\nB:\n";
	for(int i=0; i<RAMSEY_instance->PARAM_SIZE_GRAPH; i++)
	{
		for(int j=0; j<RAMSEY_instance->PARAM_SIZE_GRAPH; j++)
		{
			if(i==j)
			{
				cout << 1;
			}
			else{
				cout << (int)(RAMSEY_instance->X_CALLBACK[mapping(RAMSEY_instance,i,j)]+0.5);
			}
		}
		cout << endl;
	}
	cout << endl;

	cout << "\n\nR:\n";
	for(int i=0; i<RAMSEY_instance->PARAM_SIZE_GRAPH; i++)
	{
		for(int j=0; j<RAMSEY_instance->PARAM_SIZE_GRAPH; j++)
		{
			if(i==j)
			{
				cout << 1;
			}
			else
			{
				cout << 1-(int)(RAMSEY_instance->X_CALLBACK[mapping(RAMSEY_instance,i,j)]+0.5);
			}
		}
		cout << endl;
	}
	cout << endl;

#endif


	//		/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	//		/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	//		//PRINTING SOLUTINONS ON FILES
	//		int n_var=2*RAMSEY_instance->PARAM_SIZE_GRAPH*RAMSEY_instance->PARAM_SIZE_GRAPH;
	//		double *X_MODEL=new double[n_var];
	//
	//		int dummy_counter=0;
	//		for(int i=0; i<RAMSEY_instance->PARAM_SIZE_GRAPH; i++)
	//		{
	//			for(int j=0; j<RAMSEY_instance->PARAM_SIZE_GRAPH; j++)
	//			{
	//				if(RAMSEY_instance->X_CALLBACK[mapping(RAMSEY_instance,i,j)]<0.5)
	//				{
	//					X_MODEL[dummy_counter]=1;
	//					dummy_counter++;
	//				}
	//				else
	//				{
	//					X_MODEL[dummy_counter]=0;
	//					dummy_counter++;
	//				}
	//			}
	//		}
	//		for(int i=0; i<RAMSEY_instance->PARAM_SIZE_GRAPH; i++)
	//		{
	//			for(int j=0; j<RAMSEY_instance->PARAM_SIZE_GRAPH; j++)
	//			{
	//				if(RAMSEY_instance->X_CALLBACK[mapping(RAMSEY_instance,i,j)]>0.5)
	//				{
	//					X_MODEL[dummy_counter]=1;
	//					dummy_counter++;
	//				}
	//				else
	//				{
	//					X_MODEL[dummy_counter]=0;
	//					dummy_counter++;
	//				}
	//			}
	//		}
	//
	//		bool OK_SOL=check_solution(RAMSEY_instance,X_MODEL);
	//
	//		delete []X_MODEL;
	//		/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	//		/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////



	if(RAMSEY_instance->SKIP_M_SEPARATION==false)
	{

		RAMSEY_instance->n_clique_calls_blue++;

		for(int i=0;i<RAMSEY_instance->PARAM_SIZE_GRAPH;i++)
		{
			for(int j=i+1;j<RAMSEY_instance->PARAM_SIZE_GRAPH;j++)
			{
				if(RAMSEY_instance->X_CALLBACK[mapping(RAMSEY_instance,i,j)]<0.5)
				{
					RAMSEY_instance->edge_fixing[i][j]=0;
				}
				else
				{
					RAMSEY_instance->edge_fixing[i][j]=1;
				}
			}
		}

		if(RAMSEY_instance->PARAM_CPLEX==1)
		{
			clock_t time_start_b=clock();
			clique_solve_cplex_fixing(RAMSEY_instance,0);
			//clique_blue_solve_cplex(RAMSEY_instance);
			clock_t time_end_b=clock();
			RAMSEY_instance->time_clique_cplex+=(double)(time_end_b-time_start_b)/(double)CLOCKS_PER_SEC;
		}
		else
		{

			for (int i = 0; i < RAMSEY_instance->PARAM_SIZE_GRAPH; i++){RAMSEY_instance->CLIQUE_SOL[i]=0;}
			clock_t time_start_b_BB=clock();

			if(RAMSEY_instance->CLIQUE_TARGET==1)
			{
				RAMSEY_instance->CLIQUE_VAL=RAMSEY_instance->clique_solve_BB_edge_fixing
						(
								RAMSEY_instance->edge_fixing,
								RAMSEY_instance->CLIQUE_SOL,
								1,
								RAMSEY_instance->PARAM_M,
								RAMSEY_instance->PARAM_MNTS,
								RAMSEY_instance->PARAM_TOUT_MNTS,
								RAMSEY_instance->PARAM_NUM_RESTARTS_MNTS,
								RAMSEY_instance->PARAM_NUM_ITERATIONS_MNTS,
								0
						);
			}
			else
			{
				RAMSEY_instance->CLIQUE_VAL=RAMSEY_instance->clique_solve_BB_edge_fixing
						(
								RAMSEY_instance->edge_fixing,
								RAMSEY_instance->CLIQUE_SOL,
								1,
								RAMSEY_instance->PARAM_SIZE_GRAPH,
								RAMSEY_instance->PARAM_MNTS,
								RAMSEY_instance->PARAM_TOUT_MNTS,
								RAMSEY_instance->PARAM_NUM_RESTARTS_MNTS,
								RAMSEY_instance->PARAM_NUM_ITERATIONS_MNTS,
								0
						)	;
			}


			clock_t time_end_b_BB=clock();
			RAMSEY_instance->time_clique_BB+=(double)(time_end_b_BB-time_start_b_BB)/(double)CLOCKS_PER_SEC;
		}
		////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


#ifdef print_clique_size
		cout << "BLUE CLIQUE SIZE:\t"<< RAMSEY_instance->CLIQUE_VAL <<  "\t M \t" <<RAMSEY_instance->PARAM_M << endl;
		cin.get();
#endif

#ifdef PRINT_SOLUTION_CALLBACK
		cout << "CLIQUE_VAL BLUE\t" << RAMSEY_instance->CLIQUE_VAL << endl;
		cout << "CLIQUE BLUE\n";
		for(int i=0; i<RAMSEY_instance->PARAM_SIZE_GRAPH; i++){cout << (int)(RAMSEY_instance->CLIQUE_SOL[i]+0.5);}cout << endl;
#endif

		if(RAMSEY_instance->CLIQUE_VAL >=  RAMSEY_instance->PARAM_M )
		{

			// Minimize jump list
			if(RAMSEY_instance->MINIMIZE_CUTS >= 1)
			{
				clock_t time_start_minimization = clock();
				bool jump_removed = true;
				bool any_jump_removed = false;

				// Copy X_CALLBACK to X_CALLBACK_TEMP once at the beginning
				for(int k = 0; k < RAMSEY_instance->n_variable_MODEL_3; k++)
				{
					RAMSEY_instance->X_CALLBACK_TEMP[k] = RAMSEY_instance->X_CALLBACK[k];
				}

				// Keep trying to remove jumps until no more can be removed
				while(jump_removed)
				{
					jump_removed = false;

					// Try to remove each active jump
					for(int jump_idx = 0; jump_idx < RAMSEY_instance->n_variable_MODEL_3; jump_idx++)
					{
						// Check if this jump is currently active in TEMP
						if(RAMSEY_instance->X_CALLBACK_TEMP[jump_idx] > 0.5)
						{
							double old_jump_val = RAMSEY_instance->X_CALLBACK_TEMP[jump_idx];
							// Temporarily deactivate this jump in TEMP
							RAMSEY_instance->X_CALLBACK_TEMP[jump_idx] = 0.0;

							// Build edge_fixing_TEMP from X_CALLBACK_TEMP
							for(int i = 0; i < RAMSEY_instance->PARAM_SIZE_GRAPH; i++)
							{
								for(int j = i+1; j < RAMSEY_instance->PARAM_SIZE_GRAPH; j++)
								{
									if(RAMSEY_instance->X_CALLBACK_TEMP[mapping(RAMSEY_instance, i, j)] < 0.5)
									{
										RAMSEY_instance->edge_fixing_TEMP[i][j] = 0;
									}
									else
									{
										RAMSEY_instance->edge_fixing_TEMP[i][j] = 1;
									}
								}
							}

							// Initialize CLIQUE_SOL_TEMP for testing
							for(int i = 0; i < RAMSEY_instance->PARAM_SIZE_GRAPH; i++)
							{
								RAMSEY_instance->CLIQUE_SOL_TEMP[i] = 0;
							}

							// Test if BLUE clique is still large enough with this jump removed
							double test_clique_val;

							if(RAMSEY_instance->MINIMIZE_CUTS == 1)
							{
								test_clique_val = RAMSEY_instance->clique_solve_BB_edge_fixing_heur
										(
												RAMSEY_instance->edge_fixing_TEMP,
												RAMSEY_instance->CLIQUE_SOL_TEMP,
												1,
												RAMSEY_instance->PARAM_M,
												0,//RAMSEY_instance->PARAM_MNTS,
												0,//RAMSEY_instance->PARAM_TOUT_MNTS,
												0,//RAMSEY_instance->PARAM_NUM_RESTARTS_MNTS,
												0,//RAMSEY_instance->PARAM_NUM_ITERATIONS_MNTS,
												0
										);
							}
							else
							{
								test_clique_val = RAMSEY_instance->clique_solve_BB_edge_fixing
										(
												RAMSEY_instance->edge_fixing_TEMP,
												RAMSEY_instance->CLIQUE_SOL_TEMP,
												1,
												RAMSEY_instance->PARAM_M,
												RAMSEY_instance->PARAM_MNTS,
												RAMSEY_instance->PARAM_TOUT_MNTS,
												RAMSEY_instance->PARAM_NUM_RESTARTS_MNTS,
												RAMSEY_instance->PARAM_NUM_ITERATIONS_MNTS,
												0
										);
							}

							// Only commit if BLUE clique constraint is still satisfied
							if(test_clique_val >= RAMSEY_instance->PARAM_M)
							{
								// Copy CLIQUE_SOL_TEMP to CLIQUE_SOL (used to build the cut)
								for(int i = 0; i < RAMSEY_instance->PARAM_SIZE_GRAPH; i++)
								{
									RAMSEY_instance->CLIQUE_SOL[i] = RAMSEY_instance->CLIQUE_SOL_TEMP[i];
								}

								// Update CLIQUE_VAL
								RAMSEY_instance->CLIQUE_VAL = test_clique_val;
								jump_removed = true;
								any_jump_removed = true;
								RAMSEY_instance->n_jumps_minimized_blue++;
							}
							else
							{
								// Test failed: restore the jump in TEMP
								RAMSEY_instance->X_CALLBACK_TEMP[jump_idx] = old_jump_val;
							}
						}
					}

#ifdef JUST_ONE_ROUND_OF_MINIMALIZATION
					jump_removed = false;//ONLY ONE ROUND!
#endif
				}

				if(any_jump_removed)
				{
					RAMSEY_instance->n_minimization_successes_blue++;
				}

				clock_t time_end_minimization = clock();
				RAMSEY_instance->time_minimization += (double)(time_end_minimization - time_start_minimization) / (double)CLOCKS_PER_SEC;
			}


			if(RAMSEY_instance->CLIQUE_TARGET_RESIZE==1)
			{
				if(RAMSEY_instance->CLIQUE_VAL >  RAMSEY_instance->PARAM_M)
				{
					RAMSEY_instance->CLIQUE_VAL =  RAMSEY_instance->PARAM_M;

					int size=0;
					for(int i=0; i<RAMSEY_instance->PARAM_SIZE_GRAPH; i++)
					{
						if(size>=RAMSEY_instance->PARAM_M)
						{
							RAMSEY_instance->CLIQUE_SOL[i]=0;
						}
						else
						{
							if(RAMSEY_instance->CLIQUE_SOL[i]>0.5)
							{
								size++;
							}
						}
					}
				}
			}


			if( RAMSEY_instance->PARAM_STRONGER_CUTS == 1 )
			{
				RAMSEY_instance->cut_RHS = ex_value ( (int) RAMSEY_instance->CLIQUE_VAL , RAMSEY_instance->PARAM_M );
			}
			else
			{
				RAMSEY_instance->cut_RHS = edge_number( (int) RAMSEY_instance->CLIQUE_VAL )  - ( RAMSEY_instance->CLIQUE_VAL - RAMSEY_instance->PARAM_M )  - 1;
			}

			RAMSEY_instance->nzcnt=0;

			for(int i=0; i<RAMSEY_instance->n_variable_MODEL_3; i++)
			{
				RAMSEY_instance->cut_rmatind[RAMSEY_instance->nzcnt]=i;
				RAMSEY_instance->cut_rmatval[RAMSEY_instance->nzcnt]=0.0;
				RAMSEY_instance->nzcnt++;
			}

			for(int i=0; i<RAMSEY_instance->PARAM_SIZE_GRAPH; i++)
			{
				for(int j=i+1; j<RAMSEY_instance->PARAM_SIZE_GRAPH; j++)
				{
					if(RAMSEY_instance->CLIQUE_SOL[i]>0.5 && RAMSEY_instance->CLIQUE_SOL[j]>0.5)
					{
						RAMSEY_instance->cut_rmatval[mapping(RAMSEY_instance,i,j)] ++;
					}
				}
			}

			if( RAMSEY_instance->PARAM_COVER_CUTS == 1 )
			{
				if(RAMSEY_instance->PARAM_STRONGER_CUTS==1)
				{
					double dummy = ex_value( (int) RAMSEY_instance->CLIQUE_VAL , RAMSEY_instance->PARAM_M ) - edge_number( (int) RAMSEY_instance->CLIQUE_VAL );

					for(int i=0; i<RAMSEY_instance->n_variable_MODEL_3; i++)
					{

						RAMSEY_instance->cut_rmatval[i] = min( RAMSEY_instance->cut_rmatval[i] , (double) edge_number( (int) RAMSEY_instance->CLIQUE_VAL ) - ex_value( (int) RAMSEY_instance->CLIQUE_VAL , RAMSEY_instance->PARAM_M )  );

						dummy += RAMSEY_instance->cut_rmatval[i];
					}

					RAMSEY_instance->cut_RHS = dummy;
				}
				else
				{

					double dummy = ( edge_number( (int) RAMSEY_instance->CLIQUE_VAL ) - (RAMSEY_instance->CLIQUE_VAL - RAMSEY_instance->PARAM_M) - 1) - edge_number( (int) RAMSEY_instance->CLIQUE_VAL );

					for(int i=0; i<RAMSEY_instance->n_variable_MODEL_3; i++)
					{

						RAMSEY_instance->cut_rmatval[i] = min ( RAMSEY_instance->cut_rmatval[i] , (double) ( edge_number ( (int) RAMSEY_instance->CLIQUE_VAL )  -  ( edge_number( (int) RAMSEY_instance->CLIQUE_VAL )  - ( RAMSEY_instance->CLIQUE_VAL - RAMSEY_instance->PARAM_M )  - 1 )  )  );

						dummy += RAMSEY_instance->cut_rmatval[i];
					}

					RAMSEY_instance->cut_RHS = dummy;
				}
			}

#ifdef print_cuts
			for(int i=0; i<RAMSEY_instance->n_variable_MODEL_3; i++)
			{
				cout << "i\t" << i << "\t" << RAMSEY_instance->cut_rmatind[i] << "\t" << RAMSEY_instance->cut_rmatval[i] <<  endl;
			}
			cout << RAMSEY_instance->cut_RHS << endl;

			cout << "\n***BLUE CUT***\n";
			cin.get();
#endif



			RAMSEY_instance->status=CPXcutcallbackadd (env,cbdata,wherefrom,RAMSEY_instance->nzcnt,RAMSEY_instance->cut_RHS,'L',RAMSEY_instance->cut_rmatind,RAMSEY_instance->cut_rmatval,RAMSEY_instance->CUT_CALL_BACK_STRATEGY);
			if(RAMSEY_instance->status!=0){printf("CPXcutcallbackadd\n");exit(-1);}

			(*useraction_p)=CPX_CALLBACK_SET;

			if(RAMSEY_instance->PARAM_STRONGER_CUTS==0)
			{
				RAMSEY_instance->n_cuts_blue++;
			}
			else
			{
				RAMSEY_instance->n_cuts_blue_plus++;
			}
		}
	}
	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	//if(RAMSEY_instance->SKIP_N_SEPARATION==false && !cut_added)
	if(RAMSEY_instance->SKIP_N_SEPARATION==false)
	{

		//part on the red
		RAMSEY_instance->n_clique_calls_red++;

		for(int i=0;i<RAMSEY_instance->PARAM_SIZE_GRAPH;i++)
		{
			for(int j=i+1;j<RAMSEY_instance->PARAM_SIZE_GRAPH;j++)
			{
				if(1-RAMSEY_instance->X_CALLBACK[mapping(RAMSEY_instance,i,j)]<0.5)
				{
					RAMSEY_instance->edge_fixing[i][j]=0;
				}
				else
				{
					RAMSEY_instance->edge_fixing[i][j]=1;
				}
			}
		}

		if(RAMSEY_instance->PARAM_CPLEX==1)
		{
			clock_t time_start_r=clock();
			clique_solve_cplex_fixing(RAMSEY_instance,1);
			clock_t time_end_r=clock();
			RAMSEY_instance->time_clique_cplex+=(double)(time_end_r-time_start_r)/(double)CLOCKS_PER_SEC;
		}
		else
		{

			for (int i = 0; i < RAMSEY_instance->PARAM_SIZE_GRAPH; i++){RAMSEY_instance->CLIQUE_SOL[i]=0;}
			clock_t time_start_r_BB=clock();

			if(RAMSEY_instance->CLIQUE_TARGET==1)
			{
				RAMSEY_instance->CLIQUE_VAL=RAMSEY_instance->clique_solve_BB_edge_fixing
						(
								RAMSEY_instance->edge_fixing,
								RAMSEY_instance->CLIQUE_SOL,
								1,
								RAMSEY_instance->PARAM_N,
								RAMSEY_instance->PARAM_MNTS,
								RAMSEY_instance->PARAM_TOUT_MNTS,
								RAMSEY_instance->PARAM_NUM_RESTARTS_MNTS,
								RAMSEY_instance->PARAM_NUM_ITERATIONS_MNTS,
								1
						);
			}
			else{
				RAMSEY_instance->CLIQUE_VAL=RAMSEY_instance->clique_solve_BB_edge_fixing
						(
								RAMSEY_instance->edge_fixing,
								RAMSEY_instance->CLIQUE_SOL,
								1,
								RAMSEY_instance->PARAM_SIZE_GRAPH,
								RAMSEY_instance->PARAM_MNTS,
								RAMSEY_instance->PARAM_TOUT_MNTS,
								RAMSEY_instance->PARAM_NUM_RESTARTS_MNTS,
								RAMSEY_instance->PARAM_NUM_ITERATIONS_MNTS,
								1
						)	;
			}

			clock_t time_end_r_BB=clock();
			RAMSEY_instance->time_clique_BB+=(double)(time_end_r_BB-time_start_r_BB)/(double)CLOCKS_PER_SEC;
		}
		////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////



#ifdef print_clique_size
		cout << "RED CLIQUE SIZE:\t"<< RAMSEY_instance->CLIQUE_VAL <<  "\t N \t" <<RAMSEY_instance->PARAM_N << endl;
		cin.get();
#endif

#ifdef PRINT_SOLUTION_CALLBACK
		cout << "CLIQUE_VAL RED\t" << RAMSEY_instance->CLIQUE_VAL << endl;
		cout << "CLIQUE RED\n";
		for(int i=0; i<RAMSEY_instance->PARAM_SIZE_GRAPH; i++){cout << (int)(RAMSEY_instance->CLIQUE_SOL[i]+0.5);}cout << endl;
#endif


		if(RAMSEY_instance->CLIQUE_VAL >= RAMSEY_instance->PARAM_N )
		{

			// Minimize jump list
			if(RAMSEY_instance->MINIMIZE_CUTS >= 1)
			{
				clock_t time_start_minimization = clock();
				bool jump_removed = true;
				bool any_jump_removed = false;

				// Copy X_CALLBACK to X_CALLBACK_TEMP once at the beginning
				for(int k = 0; k < RAMSEY_instance->n_variable_MODEL_3; k++)
				{
					RAMSEY_instance->X_CALLBACK_TEMP[k] = RAMSEY_instance->X_CALLBACK[k];
				}

				// Keep trying to remove jumps until no more can be removed
				while(jump_removed)
				{
					jump_removed = false;

					// For RED (1-X), remove red edges by flipping jumps 0 -> 1
					for(int jump_idx = 0; jump_idx < RAMSEY_instance->n_variable_MODEL_3; jump_idx++)
					{
						// In RED complement, only inactive X-jumps can be turned on to remove red edges
						if(RAMSEY_instance->X_CALLBACK_TEMP[jump_idx] < 0.5)
						{
							double old_jump_val = RAMSEY_instance->X_CALLBACK_TEMP[jump_idx];
							// Temporarily activate this jump in TEMP (removes corresponding red edges)
							RAMSEY_instance->X_CALLBACK_TEMP[jump_idx] = 1.0;

							// Build edge_fixing_TEMP from X_CALLBACK_TEMP
							for(int i = 0; i < RAMSEY_instance->PARAM_SIZE_GRAPH; i++)
							{
								for(int j = i+1; j < RAMSEY_instance->PARAM_SIZE_GRAPH; j++)
								{
									if(1 - RAMSEY_instance->X_CALLBACK_TEMP[mapping(RAMSEY_instance, i, j)] < 0.5)
									{
										RAMSEY_instance->edge_fixing_TEMP[i][j] = 0;
									}
									else
									{
										RAMSEY_instance->edge_fixing_TEMP[i][j] = 1;
									}
								}
							}

							// Initialize CLIQUE_SOL_TEMP for testing
							for(int i = 0; i < RAMSEY_instance->PARAM_SIZE_GRAPH; i++)
							{
								RAMSEY_instance->CLIQUE_SOL_TEMP[i] = 0;
							}

							// Test if RED clique is still large enough with this jump removed
							double test_clique_val;
							if(RAMSEY_instance->MINIMIZE_CUTS == 1)
							{
								test_clique_val = RAMSEY_instance->clique_solve_BB_edge_fixing_heur
										(
												RAMSEY_instance->edge_fixing_TEMP,
												RAMSEY_instance->CLIQUE_SOL_TEMP,
												1,
												RAMSEY_instance->PARAM_N,
												0,//RAMSEY_instance->PARAM_MNTS,
												0,//RAMSEY_instance->PARAM_TOUT_MNTS,
												0,//RAMSEY_instance->PARAM_NUM_RESTARTS_MNTS,
												0,//RAMSEY_instance->PARAM_NUM_ITERATIONS_MNTS,
												1
										);
							}
							else
							{
								test_clique_val = RAMSEY_instance->clique_solve_BB_edge_fixing
										(
												RAMSEY_instance->edge_fixing_TEMP,
												RAMSEY_instance->CLIQUE_SOL_TEMP,
												1,
												RAMSEY_instance->PARAM_N,
												RAMSEY_instance->PARAM_MNTS,
												RAMSEY_instance->PARAM_TOUT_MNTS,
												RAMSEY_instance->PARAM_NUM_RESTARTS_MNTS,
												RAMSEY_instance->PARAM_NUM_ITERATIONS_MNTS,
												1
										);
							}

							// Only commit if RED clique constraint is still satisfied
							if(test_clique_val >= RAMSEY_instance->PARAM_N)
							{
								// Copy CLIQUE_SOL_TEMP to CLIQUE_SOL (used to build the cut)
								for(int i = 0; i < RAMSEY_instance->PARAM_SIZE_GRAPH; i++)
								{
									RAMSEY_instance->CLIQUE_SOL[i] = RAMSEY_instance->CLIQUE_SOL_TEMP[i];
								}

								// Update CLIQUE_VAL
								RAMSEY_instance->CLIQUE_VAL = test_clique_val;
								jump_removed = true;
								any_jump_removed = true;
								RAMSEY_instance->n_jumps_minimized_red++;
							}
							else
							{
								// Test failed: restore the jump in TEMP
								RAMSEY_instance->X_CALLBACK_TEMP[jump_idx] = old_jump_val;
							}
						}
					}

#ifdef JUST_ONE_ROUND_OF_MINIMALIZATION
					jump_removed = false;//ONLY ONE ROUND!
#endif

				}

				if(any_jump_removed)
				{
					RAMSEY_instance->n_minimization_successes_red++;
				}

				clock_t time_end_minimization = clock();
				RAMSEY_instance->time_minimization += (double)(time_end_minimization - time_start_minimization) / (double)CLOCKS_PER_SEC;
			}


			if(RAMSEY_instance->CLIQUE_TARGET_RESIZE==1)
			{

				if(RAMSEY_instance->CLIQUE_VAL >  RAMSEY_instance->PARAM_N)
				{
					RAMSEY_instance->CLIQUE_VAL =  RAMSEY_instance->PARAM_N;

					int size=0;
					for(int i=0; i<RAMSEY_instance->PARAM_SIZE_GRAPH; i++)
					{
						if(size>=RAMSEY_instance->PARAM_N)
						{
							RAMSEY_instance->CLIQUE_SOL[i]=0;
						}
						else
						{
							if(RAMSEY_instance->CLIQUE_SOL[i]>0.5)
							{
								size++;
							}
						}
					}
				}
			}


			if(RAMSEY_instance->PARAM_STRONGER_CUTS == 1)
			{
				RAMSEY_instance->cut_RHS = edge_number( (int) RAMSEY_instance->CLIQUE_VAL) - ex_value( (int) RAMSEY_instance->CLIQUE_VAL , RAMSEY_instance->PARAM_N );
			}
			else
			{
				RAMSEY_instance->cut_RHS = RAMSEY_instance->CLIQUE_VAL - RAMSEY_instance->PARAM_N + 1;
			}

			RAMSEY_instance->nzcnt=0;

			for(int i=0; i<RAMSEY_instance->n_variable_MODEL_3; i++)
			{
				RAMSEY_instance->cut_rmatind[RAMSEY_instance->nzcnt]=i;
				RAMSEY_instance->cut_rmatval[RAMSEY_instance->nzcnt]=0.0;
				RAMSEY_instance->nzcnt++;
			}

			for(int i=0; i<RAMSEY_instance->PARAM_SIZE_GRAPH; i++)
			{
				for(int j=i+1; j<RAMSEY_instance->PARAM_SIZE_GRAPH; j++)
				{
					if(RAMSEY_instance->CLIQUE_SOL[i]>0.5 && RAMSEY_instance->CLIQUE_SOL[j]>0.5)
					{
						RAMSEY_instance->cut_rmatval[mapping(RAMSEY_instance,i,j)] = RAMSEY_instance->cut_rmatval[mapping(RAMSEY_instance,i,j)]+1;
					}
				}
			}

			if (RAMSEY_instance->PARAM_COVER_CUTS == 1)
			{
				if(RAMSEY_instance->PARAM_STRONGER_CUTS==1)
				{
					for(int i=0; i<RAMSEY_instance->n_variable_MODEL_3; i++)
					{
						RAMSEY_instance->cut_rmatval[i] = min( RAMSEY_instance->cut_rmatval[i] , (double) edge_number( (int) RAMSEY_instance->CLIQUE_VAL ) - ex_value( (int) RAMSEY_instance->CLIQUE_VAL , RAMSEY_instance->PARAM_N )  );
					}
				}
				else
				{
					for(int i=0; i<RAMSEY_instance->n_variable_MODEL_3; i++)
					{
						RAMSEY_instance->cut_rmatval[i] = min( RAMSEY_instance->cut_rmatval[i] , (double) RAMSEY_instance->CLIQUE_VAL - RAMSEY_instance->PARAM_N + 1  );
					}
				}
			}


#ifdef print_cuts
			for(int i=0; i<RAMSEY_instance->n_variable_MODEL_3; i++)
			{
				cout << "i\t" << i << "\t" << RAMSEY_instance->cut_rmatind[i] << "\t" << RAMSEY_instance->cut_rmatval[i] <<  endl;
			}
			cout << RAMSEY_instance->cut_RHS << endl;

			cout << "\n***CUT RED***\n";
			cin.get();
#endif


			RAMSEY_instance->status=CPXcutcallbackadd (env,cbdata,wherefrom,RAMSEY_instance->nzcnt,RAMSEY_instance->cut_RHS,'G',RAMSEY_instance->cut_rmatind,RAMSEY_instance->cut_rmatval,RAMSEY_instance->CUT_CALL_BACK_STRATEGY);
			if(RAMSEY_instance->status!=0){printf("CPXcutcallbackadd\n");exit(-1);}

			(*useraction_p)=CPX_CALLBACK_SET;

			if(RAMSEY_instance->PARAM_STRONGER_CUTS==0)
			{
				RAMSEY_instance->n_cuts_red++;
			}
			else
			{
				RAMSEY_instance->n_cuts_red_plus++;
			}
		}
	}
	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#ifdef print_cuts
	cout << "END CALLBACK\n";
	cin.get();
#endif

	return 0;
}

/***********************************************************************************/
void RAMSEY_MODEL_3_parameter_setting(data *RAMSEY_instance)
/***********************************************************************************/
{

	// * Set printing *

	CPXsetintparam (RAMSEY_instance->env_MODEL_3, CPX_PARAM_SCRIND, CPX_ON);


	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	RAMSEY_instance->status = CPXsetintparam (RAMSEY_instance->env_MODEL_3, CPX_PARAM_HEURFREQ, RAMSEY_instance->HEURFREQ);
	if (RAMSEY_instance->status)
	{
		printf ("error for CPX_PARAM_HEURFREQ\n");
	}
	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	// * Set number of CPU*

	RAMSEY_instance->status = CPXsetintparam (RAMSEY_instance->env_MODEL_3, CPX_PARAM_THREADS, RAMSEY_instance->NUMBER_OF_THREADS);
	if (RAMSEY_instance->status)
	{
		printf ("error for CPX_PARAM_THREADS\n");
	}

	//	//-1	CPX_PARALLEL_OPPORTUNISTIC	Opportunistic	Enable opportunistic parallel search mode
	//	//0	CPX_PARALLEL_AUTO	AutoParallel	Automatic: let CPLEX decide whether to invoke deterministic or opportunistic search; default
	//	//1	CPX_PARALLEL_DETERMINISTIC	Deterministic	Enable deterministic parallel search mode
	//	RAMSEY_instance->status = CPXsetintparam (RAMSEY_instance->env_MODEL_3, CPX_PARAM_PARALLELMODE, CPX_PARALLEL_DETERMINISTIC);
	//	if (RAMSEY_instance->status)
	//	{
	//		printf ("error for CPX_PARALLEL_DETERMINISTIC\n");
	//	}


	// * Set time limit *

	RAMSEY_instance->status = CPXsetdblparam (RAMSEY_instance->env_MODEL_3, CPX_PARAM_TILIM,RAMSEY_instance->PARAM_TIME_LIMIT);
	if (RAMSEY_instance->status)
	{
		printf ("error for CPX_PARAM_TILIM\n");
	}

	/* set memory limit of the TREE */
	RAMSEY_instance->status = CPXsetdblparam (RAMSEY_instance->env_MODEL_3, CPXPARAM_MIP_Limits_TreeMemory, 1024*4);
	if (RAMSEY_instance->status)
	{
		printf ("error for CPXPARAM_MIP_Limits_TreeMemory\n");
	}


	// * Set MIP ENPHASIS *

	//0	CPX_MIPEMPHASIS_BALANCED	Balance optimality and feasibility; default
	//1	CPX_MIPEMPHASIS_FEASIBILITY	Emphasize feasibility over optimality
	//2	CPX_MIPEMPHASIS_OPTIMALITY	Emphasize optimality over feasibility
	//3	CPX_MIPEMPHASIS_BESTBOUND	Emphasize moving best bound
	//4	CPX_MIPEMPHASIS_HIDDENFEAS	Emphasize finding hidden feasible solutions

	RAMSEY_instance->status = CPXsetintparam (RAMSEY_instance->env_MODEL_3, CPX_PARAM_MIPEMPHASIS,CPX_MIPEMPHASIS_FEASIBILITY);
	if (RAMSEY_instance->status)
	{
		printf ("error for CPX_PARAM_MIPEMPHASIS\n");
	}


	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// * MIP node selection strategy *


	//	0
	//CPX_NODESEL_DFS
	//Depth-first search
	//1
	//CPX_NODESEL_BESTBOUND
	//Best-bound search; default
	//2
	//CPX_NODESEL_BESTEST
	//Best-estimate search
	//3
	//CPX_NODESEL_BESTEST_ALT
	//Alternative best-estimate search

	RAMSEY_instance->status = CPXsetintparam (RAMSEY_instance->env_MODEL_3, CPX_PARAM_NODESEL, RAMSEY_instance->TREE_EXPLORATION_STRATEGY);
	if (RAMSEY_instance->status)
	{
		printf ("error for CPX_PARAM_NODESEL\n");
	}
	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	//	-1	CPX_VARSEL_MININFEAS	Branch on variable with minimum infeasibility
	//	0	CPX_VARSEL_DEFAULT	Automatic: let CPLEX choose variable to branch on; default
	//	1	CPX_VARSEL_MAXINFEAS	Branch on variable with maximum infeasibility
	//	2	CPX_VARSEL_PSEUDO	Branch based on pseudo costs
	//	3	CPX_VARSEL_STRONG	Strong branching
	//	4	CPX_VARSEL_PSEUDOREDUCED	Branch based on pseudo reduced costs

	RAMSEY_instance->status = CPXsetintparam (RAMSEY_instance->env_MODEL_3, CPXPARAM_MIP_Strategy_VariableSelect,RAMSEY_instance->BRANCHING_VARIABLE_SELECTION);
	if (RAMSEY_instance->status)
	{
		printf ("error for CPXPARAM_MIP_Strategy_VariableSelect\n");
	}
	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


	// * Set exit after first solution *

	RAMSEY_instance->status = CPXsetintparam (RAMSEY_instance->env_MODEL_3, CPX_PARAM_INTSOLLIM,1);
	if (RAMSEY_instance->status)
	{
		printf ("error for CPX_PARAM_INTSOLLIM\n");
	}


	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


	CPXsetintparam(RAMSEY_instance->env_MODEL_3, CPX_PARAM_MIPCBREDLP, CPX_OFF);        // let MIP callbacks work on the original model
	//	CPXsetintparam(RAMSEY_instance->env_MODEL_3, CPX_PARAM_PRELINEAR, CPX_OFF);              // assure linear mappings between the presolved and original models
	CPXsetintparam(RAMSEY_instance->env_MODEL_3, CPX_PARAM_REDUCE, CPX_PREREDUCE_PRIMALONLY);
	//	CPXsetintparam(RAMSEY_instance->env_MODEL_3, CPXPARAM_Preprocessing_Reduce, CPX_PREREDUCE_PRIMALONLY);
	//	CPXsetintparam(RAMSEY_instance->env_MODEL_3, CPXPARAM_Preprocessing_Linear, 0);


	RAMSEY_instance->status = CPXsetlazyconstraintcallbackfunc(RAMSEY_instance->env_MODEL_3,mycutcallback_LAZY_MODEL_3,RAMSEY_instance);
	if (RAMSEY_instance->status)
	{
		printf ("error for CPXsetlazyconstraintcallbackfunc\n");
	}


	RAMSEY_instance->status = CPXsetintparam(RAMSEY_instance->env_MODEL_3, CPX_PARAM_RANDOMSEED, RAMSEY_instance->RANDOM_SEED);
	if (RAMSEY_instance->status )
	{
		printf ("error for CPX_PARAM_RANDOMSEED\n");
	}


	//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	if( RAMSEY_instance->BRANCHING_STRATEGY>0)
	{

		cout << "\n\n***SETTING_BRANCHING_PRIORITIES***\n\n";

		//setting the branching priorities of the branching variables
		RAMSEY_instance->status = CPXsetintparam (RAMSEY_instance->env_MODEL_3, 		CPX_PARAM_MIPORDIND,1);
		if (RAMSEY_instance->status)
		{
			printf ("error for CPX_PARAM_MIPORDIND\n");
		}

		int *indices=new int[RAMSEY_instance->n_variable_MODEL_3];
		int *priority=new int[RAMSEY_instance->n_variable_MODEL_3];
		int *direction=new int[RAMSEY_instance->n_variable_MODEL_3];

		//	CPX_BRANCH_GLOBAL	use global branching direction when setting the parameter CPX_PARAM_BRDIR
		//	CPX_BRANCH_DOWN	branch down first on variable indices[i]
		//	CPX_BRANCH_UP	branch up first on variable indices[i]

		if(RAMSEY_instance->BRANCHING_STRATEGY>2)
		{
			srand(RAMSEY_instance->BRANCHING_STRATEGY);
		}

		int counter_local=0;
		for(int i=0; i<RAMSEY_instance->n_variable_MODEL_3; i++)
		{

			indices[counter_local]=i;

			priority[counter_local]= 1;

			if(RAMSEY_instance->BRANCHING_STRATEGY==1)
			{
				priority[counter_local]= i;
			}

			if(RAMSEY_instance->BRANCHING_STRATEGY==2)
			{
				priority[counter_local]= 1000 - i;
			}

			if(RAMSEY_instance->BRANCHING_STRATEGY>2)
			{

				priority[counter_local]= randNum(1, 100);
			}

			direction[counter_local]=CPX_BRANCH_UP;

			counter_local++;

		}

		RAMSEY_instance-> status = CPXcopyorder (RAMSEY_instance->env_MODEL_3,RAMSEY_instance->lp_MODEL_3, RAMSEY_instance->n_variable_MODEL_3, indices, priority,direction);
		if (RAMSEY_instance->status)
		{
			printf ("error for CPXcopyorder\n");
		}

		//		cout << "priority\n";
		//		counter_local=0;
		//		for(int i=0; i<RAMSEY_instance->n_variable_MODEL_3; i++)
		//		{
		//			cout << priority[counter_local++] << "\t";
		//		}
		//		cout << endl;
		//		cin.get();

	}
	//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


	///////////////////////////////////////////////////////////////////////////////////////////////
	///////////////////////////////////////////////////////////////////////////////////////////////
	if(RAMSEY_instance->PARAM_CUT_LOOP!=-1)
	{
		RAMSEY_instance->status = CPXsetintparam (RAMSEY_instance->env_MODEL_3, CPX_PARAM_NODELIM, RAMSEY_instance->PARAM_CUT_LOOP);
		if (RAMSEY_instance->status)
		{
			printf ("error for CPX_PARAM_NODELIM\n");
		}

		clock_t time_start=clock();

		RAMSEY_instance->status=CPXmipopt(RAMSEY_instance->env_MODEL_3,RAMSEY_instance->lp_MODEL_3);
		if(RAMSEY_instance->status!=0)
		{
			printf("error in CPXmipopt\n");
			exit(-1);
		}

		cout << "DONE with MIPopt\n\n";

		clock_t time_end=clock();

		double CUT_LOOP_time=(double)(time_end-time_start)/(double)CLOCKS_PER_SEC;

		cout << "\n\n------>>>>CUT_LOOP_time\t" << CUT_LOOP_time <<endl;

		cout << "time_clique_BB ->\t" << RAMSEY_instance->time_clique_BB << endl;
		cout << "time_clique_cplex ->\t" << RAMSEY_instance->time_clique_cplex << endl;
		cout << "n_clique_calls_red ->\t" << RAMSEY_instance->n_clique_calls_red << endl;
		cout << "n_clique_calls_blue ->\t" << RAMSEY_instance->n_clique_calls_blue << endl;
		cout << "n_cuts_red\t" << RAMSEY_instance->n_cuts_red << endl;
		cout << "n_cuts_blue\t"<< RAMSEY_instance->n_cuts_blue<< endl;
		cout << "n_cuts_red_plus\t"<< RAMSEY_instance->n_cuts_red_plus<< endl;
		cout << "n_cuts_blue_plus\t"<< RAMSEY_instance->n_cuts_blue_plus<< endl;

		RAMSEY_instance->status = CPXsetintparam (RAMSEY_instance->env_MODEL_3, CPX_PARAM_NODELIM, 2100000000);
		if (RAMSEY_instance->status)
		{
			printf ("error for CPX_PARAM_EPRHS\n");
		}

	}
	///////////////////////////////////////////////////////////////////////////////////////////////
	///////////////////////////////////////////////////////////////////////////////////////////////



}

/***********************************************************************************/
double RAMSEY_MODEL_3_solve(data *RAMSEY_instance)
/***********************************************************************************/
{


	//////////////////////////////////////////////////
	RAMSEY_MODEL_3_parameter_setting(RAMSEY_instance);
	//////////////////////////////////////////////////


	///////////////////////////////////////////////////////////////////////////////////////////////
	// * solving the model

	clock_t time_start=clock();

	RAMSEY_instance->status=CPXmipopt(RAMSEY_instance->env_MODEL_3,RAMSEY_instance->lp_MODEL_3);
	if(RAMSEY_instance->status!=0)
	{
		printf("error in CPXmipopt\n");
		exit(-1);
	}

	cout << "DONE with MIPopt\n\n";

	clock_t time_end=clock();

	double RAMSEY_time=(double)(time_end-time_start)/(double)CLOCKS_PER_SEC;
	///////////////////////////////////////////////////////////////////////////////////////////////



	//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// * getting the solution

	bool SOL_FOUND=true;
	RAMSEY_instance->status=CPXgetmipx(RAMSEY_instance->env_MODEL_3,RAMSEY_instance->lp_MODEL_3,RAMSEY_instance->X_MODEL_3,0,RAMSEY_instance->n_variable_MODEL_3-1);
	if(RAMSEY_instance->status!=0)
	{
		SOL_FOUND=false;
		printf("error in CPXgetmipx\n");
	}

	RAMSEY_instance->objval=-1;
	RAMSEY_instance->status=CPXgetmipobjval(RAMSEY_instance->env_MODEL_3,RAMSEY_instance->lp_MODEL_3,&(RAMSEY_instance->objval));
	if(RAMSEY_instance->status!=0)
	{
		printf("error in CPXgetmipobjval\n");
	}


	RAMSEY_instance->bestobjval=-1;
	RAMSEY_instance->status=CPXgetbestobjval(RAMSEY_instance->env_MODEL_3,RAMSEY_instance->lp_MODEL_3,&(RAMSEY_instance->bestobjval));
	if(RAMSEY_instance->status!=0)
	{
		printf("error in CPXgetbestobjval\n");
	}

	RAMSEY_instance->lpstat=CPXgetstat(RAMSEY_instance->env_MODEL_3,RAMSEY_instance->lp_MODEL_3);
	RAMSEY_instance->nodecount = CPXgetnodecnt(RAMSEY_instance->env_MODEL_3, RAMSEY_instance->lp_MODEL_3);

	int cur_numcols=CPXgetnumcols(RAMSEY_instance->env_MODEL_3,RAMSEY_instance->lp_MODEL_3);
	int cur_numrows=CPXgetnumrows(RAMSEY_instance->env_MODEL_3,RAMSEY_instance->lp_MODEL_3);


	/////////////////////////////////////
	if(RAMSEY_instance->lpstat==103)
	{
		RAMSEY_instance->objval=-1;
		RAMSEY_instance->bestobjval=-1;
	}
	/////////////////////////////////////

	cout << "objval ->\t" << RAMSEY_instance->objval << endl;
	cout << "bestobjval ->\t" << RAMSEY_instance->bestobjval << endl;
	cout << "RAMSEY_time ->\t" << RAMSEY_time << endl;
	cout << "lpstat ->\t" << RAMSEY_instance->lpstat << endl;
	cout << "nodecount ->\t" << RAMSEY_instance->nodecount << endl;
	cout << "time_clique_BB ->\t" << RAMSEY_instance->time_clique_BB << endl;
	cout << "time_clique_cplex ->\t" << RAMSEY_instance->time_clique_cplex << endl;
	cout << "n_clique_calls_red ->\t" << RAMSEY_instance->n_clique_calls_red << endl;
	cout << "n_clique_calls_blue ->\t" << RAMSEY_instance->n_clique_calls_blue << endl;
	cout << "n_cuts_red\t" << RAMSEY_instance->n_cuts_red << endl;
	cout << "n_cuts_blue\t"<< RAMSEY_instance->n_cuts_blue<< endl;
	cout << "n_cuts_red_plus\t"<< RAMSEY_instance->n_cuts_red_plus<< endl;
	cout << "n_cuts_blue_plus\t"<< RAMSEY_instance->n_cuts_blue_plus<< endl;
	cout << "n_calls\t" << RAMSEY_instance->n_calls<< endl;
	cout << "n_SimpleHeur_successes\t" << RAMSEY_instance->n_SimpleHeur_successes<< endl;
	cout << "n_MNTS_successes\t" << RAMSEY_instance->n_MNTS_successes<< endl;
	cout << "time_MNTS\t" << RAMSEY_instance->time_MNTS<< endl;
	cout << "n_CLISAT_successes\t" << RAMSEY_instance->n_CLISAT_successes<< endl;
	cout << "n_CLISAT_opt\t" << RAMSEY_instance->n_CLISAT_opt<< endl;
	cout << "n_jumps_minimized_blue\t" << RAMSEY_instance->n_jumps_minimized_blue << endl;
	cout << "n_jumps_minimized_red\t" << RAMSEY_instance->n_jumps_minimized_red << endl;
	cout << "n_minimization_successes_blue\t" << RAMSEY_instance->n_minimization_successes_blue << endl;
	cout << "n_minimization_successes_red\t" << RAMSEY_instance->n_minimization_successes_red << endl;
	cout << "time_minimization\t" << RAMSEY_instance->time_minimization << endl;

	if(SOL_FOUND)
	{

#ifdef PRINT_SOLUTION_MODEL_3
		cout << "\n\nR:\n";
		for(int i=0; i<RAMSEY_instance->PARAM_SIZE_GRAPH; i++)
		{
			for(int j=0; j<RAMSEY_instance->PARAM_SIZE_GRAPH; j++)
			{
				cout << (int)(RAMSEY_instance->X_MODEL_3[mapping(RAMSEY_instance,i,j)]+0.5);
			}
			cout << endl;
		}
		cout << endl;

		cout << "\n\nB:\n";
		for(int i=0; i<RAMSEY_instance->PARAM_SIZE_GRAPH; i++)
		{
			for(int j=0; j<RAMSEY_instance->PARAM_SIZE_GRAPH; j++)
			{
				cout << 1-(int)(RAMSEY_instance->X_MODEL_3[mapping(RAMSEY_instance,i,j)]+0.5);
			}
			cout << endl;
		}
		cout << endl;
#endif

		int n_var=2*RAMSEY_instance->PARAM_SIZE_GRAPH*RAMSEY_instance->PARAM_SIZE_GRAPH;

		double *X_MODEL=new double[n_var];

		int dummy_counter=0;
		for(int i=0; i<RAMSEY_instance->PARAM_SIZE_GRAPH; i++)
		{
			for(int j=0; j<RAMSEY_instance->PARAM_SIZE_GRAPH; j++)
			{
				if(RAMSEY_instance->X_MODEL_3[mapping(RAMSEY_instance,i,j)]<0.5)
				{
					X_MODEL[dummy_counter]=1;
					dummy_counter++;
				}
				else
				{
					X_MODEL[dummy_counter]=0;
					dummy_counter++;
				}
			}
		}
		for(int i=0; i<RAMSEY_instance->PARAM_SIZE_GRAPH; i++)
		{
			for(int j=0; j<RAMSEY_instance->PARAM_SIZE_GRAPH; j++)
			{
				if(RAMSEY_instance->X_MODEL_3[mapping(RAMSEY_instance,i,j)]>0.5)
				{
					X_MODEL[dummy_counter]=1;
					dummy_counter++;
				}
				else
				{
					X_MODEL[dummy_counter]=0;
					dummy_counter++;
				}
			}
		}


		////////////////////////////////////////////////////////////////////////////////////////
		char dummy_file[10000];
		sprintf(dummy_file,"colorings/col_m%d_n%d_SIZE%d_circulant%d_br%d_id%d.txt",RAMSEY_instance->PARAM_M,RAMSEY_instance->PARAM_N,RAMSEY_instance->PARAM_SIZE_GRAPH,RAMSEY_instance->PARAM_CIRCULANT, RAMSEY_instance->BRANCHING_STRATEGY, RAMSEY_instance->ID_TEST);

		cout << dummy_file << endl;

		ofstream out(dummy_file);

		out << RAMSEY_instance->PARAM_SIZE_GRAPH << endl;

		out << "JUMPS\n";
		for(int i=0; i<RAMSEY_instance->PARAM_SIZE_GRAPH; i++)
		{
			if((int)(RAMSEY_instance->X_MODEL_3[i]+0.5)>0.5)
			{
				out << i << " ";
			}
		}
		out << endl;

		out << "MATRIX\n";

		for(int i=0; i<RAMSEY_instance->PARAM_SIZE_GRAPH; i++)
		{
			for(int j=0; j<RAMSEY_instance->PARAM_SIZE_GRAPH; j++)
			{
				out << (int)(RAMSEY_instance->X_MODEL_3[mapping(RAMSEY_instance,i,j)]+0.5) << " ";
			}
			out << endl;
		}
		out << endl;

		out.close();

		write_row_sol(RAMSEY_instance, X_MODEL);
		//////////////////////////////////////////////////////////////////


		////////////////////////////////////////////////////////////////////////
		if(RAMSEY_instance->CHECK_SOLUTION==1)
		{
			bool OK_SOL=check_solution(RAMSEY_instance, X_MODEL);
			if(!OK_SOL)
			{
				cout << "ERROR IN check_solution!!!!\n\n\n";
				exit(-1);
			}
		}
		////////////////////////////////////////////////////////////////////////

		delete [] X_MODEL;
	}

	//////////////////////////////////////////////////////////////
	ofstream info_SUMMARY("info_RAMSEY.txt", ios::app);
	info_SUMMARY << fixed

			<< RAMSEY_instance->objval  << "\t"
			<< RAMSEY_instance->bestobjval  << "\t"
			<< RAMSEY_instance->lpstat << "\t"
			<< RAMSEY_instance->nodecount  << "\t"
			<<  RAMSEY_time << "\t"
			<< cur_numcols  << "\t"
			<< cur_numrows  << "\t"
			<< RAMSEY_instance->time_clique_cplex << "\t"
			<< RAMSEY_instance->time_clique_BB << "\t"
			<< RAMSEY_instance->n_clique_calls_red << "\t"
			<< RAMSEY_instance->n_clique_calls_blue << "\t"
			<< RAMSEY_instance->n_cuts_red<< "\t"
			<< RAMSEY_instance->n_cuts_blue<< "\t"
			<< RAMSEY_instance->n_cuts_red_plus<< "\t"
			<< RAMSEY_instance->n_cuts_blue_plus<< "\t"
			<<RAMSEY_instance->CLIQUE_JUMP_CUTS_M<< "\t"
			<<RAMSEY_instance->CLIQUE_JUMP_CUTS_N<< "\t"
			<<RAMSEY_instance->TRIANGLES_CUTS_M<< "\t"
			<<RAMSEY_instance->TRIANGLES_CUTS_N<< "\t"
			<<RAMSEY_instance->QUADRANGLES_CUTS_M<< "\t"
			<<RAMSEY_instance->QUADRANGLES_CUTS_N<< "\t"
			<< RAMSEY_instance->n_calls << "\t"
			<< RAMSEY_instance->n_SimpleHeur_successes << "\t"
			<< RAMSEY_instance->n_MNTS_successes << "\t"
			<< RAMSEY_instance->time_MNTS << "\t"
			<< RAMSEY_instance->n_CLISAT_successes << "\t"
			<< RAMSEY_instance->n_CLISAT_opt << "\t"
			<< RAMSEY_instance->n_calls_heur << "\t"
			<<RAMSEY_instance->n_jumps_minimized_blue<< "\t"
			<<RAMSEY_instance->n_jumps_minimized_red<< "\t"
			<<RAMSEY_instance->n_minimization_successes_blue<< "\t"
			<<RAMSEY_instance->n_minimization_successes_red<< "\t"
			<<RAMSEY_instance->time_minimization<< "\t"
			<<RAMSEY_instance->PARAM_SIZE_GRAPH << "\t"
			<<RAMSEY_instance->PARAM_M << "\t"
			<<RAMSEY_instance->PARAM_N << "\t"
			<<RAMSEY_instance->PARAM_ALGO << "\t"
			<<RAMSEY_instance->PARAM_OPTIONS << "\t"
			<<RAMSEY_instance->PARAM_CIRCULANT << "\t"
			<<RAMSEY_instance->PARAM_TIME_LIMIT << "\t"
			<<RAMSEY_instance->PARAM_STRONGER_CUTS << "\t"
			<<RAMSEY_instance->PARAM_CPLEX << "\t"
			<<RAMSEY_instance->PARAM_MNTS << "\t"
			<<RAMSEY_instance->PARAM_TOUT_MNTS << "\t"
			<<RAMSEY_instance->PARAM_NUM_RESTARTS_MNTS << "\t"
			<<RAMSEY_instance->PARAM_NUM_ITERATIONS_MNTS << "\t"
			<<RAMSEY_instance->PARAM_K_CUTS << "\t"
			<<RAMSEY_instance->PARAM_CLIQUE_JUMP_CUTS << "\t"
			<<RAMSEY_instance->PARAM_COVER_CUTS << "\t"
			<<RAMSEY_instance->PARAM_CUT_LOOP << "\t"
			<<RAMSEY_instance->AVOID_TRIANGLES << "\t"
			<<RAMSEY_instance->AVOID_QUADRANGLES << "\t"
			<<RAMSEY_instance->CHECK_SOLUTION << "\t"
			<<RAMSEY_instance->CUT_CALL_BACK_STRATEGY << "\t"
			<<RAMSEY_instance->BRANCHING_STRATEGY<< "\t"
			<<RAMSEY_instance->TREE_EXPLORATION_STRATEGY<< "\t"
			<<RAMSEY_instance->HEURFREQ<< "\t"
			<<RAMSEY_instance->CLIQUE_TARGET<< "\t"
			<<RAMSEY_instance->CLIQUE_TARGET_RESIZE<< "\t"
			<<RAMSEY_instance->MULTIPLE_CUTS<< "\t"
			<<RAMSEY_instance->RANDOM_SEED<< "\t"
			<<RAMSEY_instance->BRANCHING_VARIABLE_SELECTION<< "\t"
			<<RAMSEY_instance->NUMBER_OF_THREADS<< "\t"
			<<RAMSEY_instance->LOAD_CUTS_FROM_FILE<< "\t"
			<<RAMSEY_instance->MINIMIZE_CUTS<< "\t"
			<<RAMSEY_instance->ID_TEST<< "\t"
			<< endl;
	info_SUMMARY.close();

	char dummy[1000];
	sprintf(dummy,"SOLUTION_FILES/ID_TEST%d.sol",RAMSEY_instance->ID_TEST);
	ofstream info_FILE(dummy);
	info_FILE << fixed
			<< RAMSEY_instance->objval  << "\t"
			<< RAMSEY_instance->bestobjval  << "\t"
			<< RAMSEY_instance->lpstat << "\t"
			<< RAMSEY_instance->nodecount  << "\t"
			<<  RAMSEY_time << "\t"
			<< cur_numcols  << "\t"
			<< cur_numrows  << "\t"
			<< RAMSEY_instance->time_clique_cplex << "\t"
			<< RAMSEY_instance->time_clique_BB << "\t"
			<< RAMSEY_instance->n_clique_calls_red << "\t"
			<< RAMSEY_instance->n_clique_calls_blue << "\t"
			<< RAMSEY_instance->n_cuts_red<< "\t"
			<< RAMSEY_instance->n_cuts_blue<< "\t"
			<< RAMSEY_instance->n_cuts_red_plus<< "\t"
			<< RAMSEY_instance->n_cuts_blue_plus<< "\t"
			<<RAMSEY_instance->CLIQUE_JUMP_CUTS_M<< "\t"
			<<RAMSEY_instance->CLIQUE_JUMP_CUTS_N<< "\t"
			<<RAMSEY_instance->TRIANGLES_CUTS_M<< "\t"
			<<RAMSEY_instance->TRIANGLES_CUTS_N<< "\t"
			<<RAMSEY_instance->QUADRANGLES_CUTS_M<< "\t"
			<<RAMSEY_instance->QUADRANGLES_CUTS_N<< "\t"
			<< RAMSEY_instance->n_calls << "\t"
			<< RAMSEY_instance->n_SimpleHeur_successes << "\t"
			<< RAMSEY_instance->n_MNTS_successes << "\t"
			<< RAMSEY_instance->time_MNTS << "\t"
			<< RAMSEY_instance->n_CLISAT_successes << "\t"
			<< RAMSEY_instance->n_CLISAT_opt << "\t"
			<< RAMSEY_instance->n_calls_heur << "\t"
			<<RAMSEY_instance->n_jumps_minimized_blue<< "\t"
			<<RAMSEY_instance->n_jumps_minimized_red<< "\t"
			<<RAMSEY_instance->n_minimization_successes_blue<< "\t"
			<<RAMSEY_instance->n_minimization_successes_red<< "\t"
			<<RAMSEY_instance->time_minimization<< "\t"
			<<RAMSEY_instance->PARAM_SIZE_GRAPH << "\t"
			<<RAMSEY_instance->PARAM_M << "\t"
			<<RAMSEY_instance->PARAM_N << "\t"
			<<RAMSEY_instance->PARAM_ALGO << "\t"
			<<RAMSEY_instance->PARAM_OPTIONS << "\t"
			<<RAMSEY_instance->PARAM_CIRCULANT << "\t"
			<<RAMSEY_instance->PARAM_TIME_LIMIT << "\t"
			<<RAMSEY_instance->PARAM_STRONGER_CUTS << "\t"
			<<RAMSEY_instance->PARAM_CPLEX << "\t"
			<<RAMSEY_instance->PARAM_MNTS << "\t"
			<<RAMSEY_instance->PARAM_TOUT_MNTS << "\t"
			<<RAMSEY_instance->PARAM_NUM_RESTARTS_MNTS << "\t"
			<<RAMSEY_instance->PARAM_NUM_ITERATIONS_MNTS << "\t"
			<<RAMSEY_instance->PARAM_K_CUTS << "\t"
			<<RAMSEY_instance->PARAM_CLIQUE_JUMP_CUTS << "\t"
			<<RAMSEY_instance->PARAM_COVER_CUTS << "\t"
			<<RAMSEY_instance->PARAM_CUT_LOOP << "\t"
			<<RAMSEY_instance->AVOID_TRIANGLES << "\t"
			<<RAMSEY_instance->AVOID_QUADRANGLES << "\t"
			<<RAMSEY_instance->CHECK_SOLUTION << "\t"
			<<RAMSEY_instance->CUT_CALL_BACK_STRATEGY << "\t"
			<<RAMSEY_instance->BRANCHING_STRATEGY<< "\t"
			<<RAMSEY_instance->TREE_EXPLORATION_STRATEGY<< "\t"
			<<RAMSEY_instance->HEURFREQ<< "\t"
			<<RAMSEY_instance->CLIQUE_TARGET<< "\t"
			<<RAMSEY_instance->CLIQUE_TARGET_RESIZE<< "\t"
			<<RAMSEY_instance->MULTIPLE_CUTS<< "\t"
			<<RAMSEY_instance->RANDOM_SEED<< "\t"
			<<RAMSEY_instance->BRANCHING_VARIABLE_SELECTION<< "\t"
			<<RAMSEY_instance->NUMBER_OF_THREADS<< "\t"
			<<RAMSEY_instance->LOAD_CUTS_FROM_FILE<< "\t"
			<<RAMSEY_instance->MINIMIZE_CUTS<< "\t"
			<<RAMSEY_instance->ID_TEST<< "\t"
			<< endl;
	info_FILE.close();
	//////////////////////////////////////////////////////////////

	/////////////////////////////////////////////
	clique_free_cplex(RAMSEY_instance);
	/////////////////////////////////////////////

	/////////////////////////////////////////////
	RAMSEY_MODEL_3_deallocation(RAMSEY_instance);
	/////////////////////////////////////////////

	return RAMSEY_instance->objval;
}

/***********************************************************************************/
void RAMSEY_MODEL_3_free(data *RAMSEY_instance)
/***********************************************************************************/
{

	RAMSEY_instance->status=CPXfreeprob(RAMSEY_instance->env_MODEL_3,&(RAMSEY_instance->lp_MODEL_3));
	if(RAMSEY_instance->status!=0) {printf("error in CPXfreeprob\n");exit(-1);}

	RAMSEY_instance->status=CPXcloseCPLEX(&(RAMSEY_instance->env_MODEL_3));
	if(RAMSEY_instance->status!=0) {printf("error in CPXcloseCPLEX\n");exit(-1);}

}


/***********************************************************************************/
void add_cuts_from_file(data *RAMSEY_instance)
/***********************************************************************************/
{
	int cuts_M_added = 0;
	int cuts_N_added = 0;

	// Add cuts from CUTS_M (blue cliques - <= constraints)
	if (RAMSEY_instance->CUTS_M.loaded)
	{
		cout << "\nAdding cuts from CUTS_M..." << endl;

		for (int i = 0; i < RAMSEY_instance->CUTS_M.num_lines; i++)
		{
			int num_vars = RAMSEY_instance->CUTS_M.lines[i].num_elements;

			// Allocate constraint arrays
			RAMSEY_instance->rcnt = 1;
			RAMSEY_instance->nzcnt = num_vars;
			RAMSEY_instance->rhs = (double*) calloc(RAMSEY_instance->rcnt, sizeof(double));
			RAMSEY_instance->sense = (char*) calloc(RAMSEY_instance->rcnt, sizeof(char));
			RAMSEY_instance->rmatbeg = (int*) calloc(RAMSEY_instance->rcnt, sizeof(int));
			RAMSEY_instance->rmatind = (int*) calloc(RAMSEY_instance->nzcnt, sizeof(int));
			RAMSEY_instance->rmatval = (double*) calloc(RAMSEY_instance->nzcnt, sizeof(double));

			// RHS = sum of all variables - 1
			RAMSEY_instance->rhs[0] = num_vars - 1;
			RAMSEY_instance->sense[0] = 'L';  // <= constraint
			RAMSEY_instance->rmatbeg[0] = 0;

			// Set variables (convert from 1-based in file to 0-based for CPLEX)
			for (int j = 0; j < num_vars; j++)
			{
				int var_index = RAMSEY_instance->CUTS_M.lines[i].values[j] - 1;  // Convert to 0-based
				RAMSEY_instance->rmatind[j] = var_index;

				// Sanity check: verify index is within valid range
				if (var_index < 0 || var_index >= RAMSEY_instance->n_variable_MODEL_3)
				{
					cout << "ERROR: Invalid variable index " << var_index
							<< " (from value " << RAMSEY_instance->CUTS_M.lines[i].values[j] << ")"
							<< " in CUTS_M line " << i
							<< " (valid range: 0-" << (RAMSEY_instance->n_variable_MODEL_3 - 1) << ")" << endl;
					exit(-1);
				}

				RAMSEY_instance->rmatval[j] = 1.0;
			}

			// Add constraint to CPLEX
			RAMSEY_instance->status = CPXaddrows(RAMSEY_instance->env_MODEL_3, RAMSEY_instance->lp_MODEL_3,
					0, RAMSEY_instance->rcnt, RAMSEY_instance->nzcnt,
					RAMSEY_instance->rhs, RAMSEY_instance->sense,
					RAMSEY_instance->rmatbeg, RAMSEY_instance->rmatind, RAMSEY_instance->rmatval,
					NULL, NULL);

			if (RAMSEY_instance->status != 0)
			{
				printf("error in CPXaddrows for CUTS_M\n");
				exit(-1);
			}

			cuts_M_added++;

			// Free temporary arrays
			free(RAMSEY_instance->rmatbeg);
			free(RAMSEY_instance->rmatval);
			free(RAMSEY_instance->rmatind);
			free(RAMSEY_instance->rhs);
			free(RAMSEY_instance->sense);
		}

		cout << "Added " << cuts_M_added << " cuts from CUTS_M" << endl;
	}

	// Add cuts from CUTS_N (red cliques - >= constraints)
	if (RAMSEY_instance->CUTS_N.loaded)
	{
		cout << "\nAdding cuts from CUTS_N..." << endl;

		for (int i = 0; i < RAMSEY_instance->CUTS_N.num_lines; i++)
		{
			int num_vars = RAMSEY_instance->CUTS_N.lines[i].num_elements;

			// Allocate constraint arrays
			RAMSEY_instance->rcnt = 1;
			RAMSEY_instance->nzcnt = num_vars;
			RAMSEY_instance->rhs = (double*) calloc(RAMSEY_instance->rcnt, sizeof(double));
			RAMSEY_instance->sense = (char*) calloc(RAMSEY_instance->rcnt, sizeof(char));
			RAMSEY_instance->rmatbeg = (int*) calloc(RAMSEY_instance->rcnt, sizeof(int));
			RAMSEY_instance->rmatind = (int*) calloc(RAMSEY_instance->nzcnt, sizeof(int));
			RAMSEY_instance->rmatval = (double*) calloc(RAMSEY_instance->nzcnt, sizeof(double));

			// RHS = 1
			RAMSEY_instance->rhs[0] = 1.0;
			RAMSEY_instance->sense[0] = 'G';  // >= constraint
			RAMSEY_instance->rmatbeg[0] = 0;

			// Set variables (convert from 1-based in file to 0-based for CPLEX)
			for (int j = 0; j < num_vars; j++)
			{
				int var_index = RAMSEY_instance->CUTS_N.lines[i].values[j] - 1;  // Convert to 0-based
				RAMSEY_instance->rmatind[j] = var_index;

				// Sanity check: verify index is within valid range
				if (var_index < 0 || var_index >= RAMSEY_instance->n_variable_MODEL_3)
				{
					cout << "ERROR: Invalid variable index " << var_index
							<< " (from value " << RAMSEY_instance->CUTS_N.lines[i].values[j] << ")"
							<< " in CUTS_N line " << i
							<< " (valid range: 0-" << (RAMSEY_instance->n_variable_MODEL_3 - 1) << ")" << endl;
					exit(-1);
				}

				RAMSEY_instance->rmatval[j] = 1.0;
			}

			// Add constraint to CPLEX
			RAMSEY_instance->status = CPXaddrows(RAMSEY_instance->env_MODEL_3, RAMSEY_instance->lp_MODEL_3,
					0, RAMSEY_instance->rcnt, RAMSEY_instance->nzcnt,
					RAMSEY_instance->rhs, RAMSEY_instance->sense,
					RAMSEY_instance->rmatbeg, RAMSEY_instance->rmatind, RAMSEY_instance->rmatval,
					NULL, NULL);

			if (RAMSEY_instance->status != 0)
			{
				printf("error in CPXaddrows for CUTS_N\n");
				exit(-1);
			}

			cuts_N_added++;

			// Free temporary arrays
			free(RAMSEY_instance->rmatbeg);
			free(RAMSEY_instance->rmatval);
			free(RAMSEY_instance->rmatind);
			free(RAMSEY_instance->rhs);
			free(RAMSEY_instance->sense);
		}

		cout << "Added " << cuts_N_added << " cuts from CUTS_N" << endl;
	}

	if (cuts_M_added > 0 || cuts_N_added > 0)
	{
		cout << "\nTotal cuts added from files: " << (cuts_M_added + cuts_N_added) << endl;
	}
}


/***********************************************************************************/
void RAMSEY_MODEL_3_load(data *RAMSEY_instance)
/***********************************************************************************/
{


	/////////////////////////////////////////////
	RAMSEY_MODEL_3_allocation(RAMSEY_instance);
	/////////////////////////////////////////////

	/////////////////////////////////////////////
	clique_load_cplex(RAMSEY_instance);
	/////////////////////////////////////////////


	//
	//
	//		for(int i=0; i<RAMSEY_instance->PARAM_SIZE_GRAPH; i++)
	//		{
	//			for(int j=0; j<RAMSEY_instance->PARAM_SIZE_GRAPH; j++)
	//			{
	//				cout <<  mapping(RAMSEY_instance,i,j) << "\t";
	//
	//			}
	//			cout << endl;
	//		}
	//		cout << endl;
	//
	//
	//		exit(-1);


	/////////////////////////////////////////////////////////////////////////////////////////////////////////
	RAMSEY_instance->env_MODEL_3=CPXopenCPLEX(&(RAMSEY_instance->status));
	if(RAMSEY_instance->status!=0)
	{
		printf("cannot open CPLEX environment\n");
		exit(-1);
	}

	RAMSEY_instance->lp_MODEL_3=CPXcreateprob(RAMSEY_instance->env_MODEL_3,&(RAMSEY_instance->status),"RAMSEY");
	if(RAMSEY_instance->status!=0)
	{
		printf("cannot create problem\n");
		exit(-1);
	}
	/////////////////////////////////////////////////////////////////////////////////////////////////////////


	/////////////////////////////////////////////////////////////////////////////////////////////////////////
	RAMSEY_instance->ccnt=RAMSEY_instance->n_variable_MODEL_3;

	cout << endl;
	cout << "number of variables\t" << RAMSEY_instance->ccnt << endl;
	cout << "number of edges\t" << (RAMSEY_instance->PARAM_SIZE_GRAPH*(RAMSEY_instance->PARAM_SIZE_GRAPH-1))/2 << endl;
	cout << endl;


	RAMSEY_instance->obj=(double*) calloc(RAMSEY_instance->ccnt,sizeof(double));
	RAMSEY_instance->lb=(double*) calloc(RAMSEY_instance->ccnt,sizeof(double));
	RAMSEY_instance->ub=(double*) calloc(RAMSEY_instance->ccnt,sizeof(double));
	RAMSEY_instance->xctype=(char*) calloc(RAMSEY_instance->ccnt,sizeof(char));

	RAMSEY_instance->colname=(char**) calloc(RAMSEY_instance->ccnt,sizeof(char*));
	for(int i=0;i<RAMSEY_instance->ccnt;i++){RAMSEY_instance->colname[i]=(char*) calloc(1000,sizeof(char));}

	int counter=0;

	for(int i=0; i<RAMSEY_instance->n_variable_MODEL_3; i++)
	{

		RAMSEY_instance->obj[counter]=0.0;

		//RAMSEY_instance->obj[counter]=i;

		//		int max=100;
		//		int min=10;
		//		int randNum = rand()%(max-min + 1) + min;
		//		RAMSEY_instance->obj[counter]=randNum;
		//		//cout << RAMSEY_instance->obj[counter] << endl;

		RAMSEY_instance->lb[counter]=0.0;
		RAMSEY_instance->ub[counter]=1.0;
		RAMSEY_instance->xctype[counter]='B';

		//		if(RAMSEY_instance->PARAM_M==RAMSEY_instance->PARAM_N && i==0)
		//		{
		//			RAMSEY_instance->lb[counter]=1.0;
		//		}


		sprintf(RAMSEY_instance->colname[counter], "y_%d",i+1);
		counter++;

	}


	RAMSEY_instance->status=CPXnewcols(RAMSEY_instance->env_MODEL_3,RAMSEY_instance->lp_MODEL_3,RAMSEY_instance->ccnt,RAMSEY_instance->obj,RAMSEY_instance->lb,RAMSEY_instance->ub,RAMSEY_instance->xctype,RAMSEY_instance->colname);
	if(RAMSEY_instance->status!=0)
	{
		printf("error in CPXnewcols\n");
		exit(-1);
	}

	free(RAMSEY_instance->obj);
	free(RAMSEY_instance->lb);
	free(RAMSEY_instance->ub);
	free(RAMSEY_instance->xctype);

	for(int i=0;i<RAMSEY_instance->ccnt;i++){free(RAMSEY_instance->colname[i]);}
	free(RAMSEY_instance->colname);

	///////////////////////////////////////////////////////////////////////////////////////////////////////////
	///////////////////////////////////////////////////////////////////////////////////////////////////////////
	// Add cuts from files (if loaded)
	///////////////////////////////////////////////////////////////////////////////////////////////////////////
	add_cuts_from_file(RAMSEY_instance);
	///////////////////////////////////////////////////////////////////////////////////////////////////////////
	///////////////////////////////////////////////////////////////////////////////////////////////////////////

	RAMSEY_instance->TRIANGLES_CUTS_M = 0;
	RAMSEY_instance->TRIANGLES_CUTS_N = 0;

	if(RAMSEY_instance->AVOID_TRIANGLES==1)
	{

		int *MM=new int[RAMSEY_instance->n_variable_MODEL_3];

		////////////////blue triangles ////////////////////////////////////////////////////////////////////////////
		//  y_s + y_t + y_h(s,t) <= 2

		if(RAMSEY_instance->PARAM_M==3)
		{
			int cons_counter=0;

			RAMSEY_instance->SKIP_M_SEPARATION=true;

			for(int s=0; s< RAMSEY_instance->n_variable_MODEL_3; s++)
			{
				for(int t=s; t< RAMSEY_instance->n_variable_MODEL_3; t++)
				{

					int ktemp = s + 1 + t + 1;
					int h = mapping_h(RAMSEY_instance, ktemp) - 1;

					if(t>h){continue;}

					MM[s]=0;
					MM[t]=0;
					MM[h]=0;


					MM[s]++;
					MM[t]++;
					MM[h]++;


					RAMSEY_instance->rcnt=1;
					RAMSEY_instance->nzcnt=3;
					RAMSEY_instance->rhs=(double*) calloc(RAMSEY_instance->rcnt,sizeof(double));
					RAMSEY_instance->sense=(char*) calloc(RAMSEY_instance->rcnt,sizeof(double));

					RAMSEY_instance->rhs[0]=2;

					RAMSEY_instance->sense[0]='L';

					RAMSEY_instance->rmatbeg=(int*) calloc(RAMSEY_instance->rcnt,sizeof(int));
					RAMSEY_instance->rmatind=(int*) calloc(RAMSEY_instance->nzcnt,sizeof(int));
					RAMSEY_instance->rmatval=(double*) calloc(RAMSEY_instance->nzcnt,sizeof(double));


					int ccnt = 0;
					RAMSEY_instance->rmatval[ccnt]=MM[s];
					RAMSEY_instance->rmatind[ccnt++]=s;

					if (t!=s)
					{
						RAMSEY_instance->rmatval[ccnt]=MM[t];
						RAMSEY_instance->rmatind[ccnt++]=t;
					}

					if (h!=s && h!=t)
					{
						RAMSEY_instance->rmatval[ccnt]=MM[h];
						RAMSEY_instance->rmatind[ccnt++]=h;
					}

					if (RAMSEY_instance->PARAM_COVER_CUTS)
					{
						for (int q=0; q < 3; q++)
						{
							if (RAMSEY_instance->rmatval[q] > 1)
							{
								RAMSEY_instance->rmatval[q] = 1;
							}
						}
						RAMSEY_instance->rhs[0] = ccnt - 1;
					}


					RAMSEY_instance->status=CPXaddrows(RAMSEY_instance->env_MODEL_3,RAMSEY_instance->lp_MODEL_3,0,RAMSEY_instance->rcnt, ccnt, RAMSEY_instance->rhs,RAMSEY_instance->sense,RAMSEY_instance->rmatbeg,RAMSEY_instance->rmatind,RAMSEY_instance->rmatval,NULL,NULL);
					if(RAMSEY_instance->status!=0)
					{
						printf("error in CPXaddrows\n");
						exit(-1);
					}
					cons_counter++;

					free(RAMSEY_instance->rmatbeg);
					free(RAMSEY_instance->rmatval);
					free(RAMSEY_instance->rmatind);
					free(RAMSEY_instance->rhs);
					free(RAMSEY_instance->sense);
				}

			}

			cout << "SKIP_M_SEPARATION FLAG->\t" << RAMSEY_instance->SKIP_M_SEPARATION << "\t cons_counter \t" << cons_counter << endl;

			RAMSEY_instance->TRIANGLES_CUTS_M=cons_counter;

		}

		////////////////red triangles ////////////////////////////////////////////////////////////////////////////
		//  y_s + y_t + y_h(s,t) >= 1

		if(RAMSEY_instance->PARAM_N==3)
		{
			int cons_counter=0;

			RAMSEY_instance->SKIP_N_SEPARATION=true;

			for(int s=0; s< RAMSEY_instance->n_variable_MODEL_3; s++)
			{
				for(int t=s; t< RAMSEY_instance->n_variable_MODEL_3; t++)
				{

					int ktemp = s + 1 + t + 1;
					int h = mapping_h(RAMSEY_instance, ktemp) - 1;

					if(t>h){continue;}

					MM[s]=0;
					MM[t]=0;
					MM[h]=0;


					MM[s]++;
					MM[t]++;
					MM[h]++;


					RAMSEY_instance->rcnt=1;
					RAMSEY_instance->nzcnt=3;
					RAMSEY_instance->rhs=(double*) calloc(RAMSEY_instance->rcnt,sizeof(double));
					RAMSEY_instance->sense=(char*) calloc(RAMSEY_instance->rcnt,sizeof(double));

					RAMSEY_instance->rhs[0]=1;

					RAMSEY_instance->sense[0]='G';

					RAMSEY_instance->rmatbeg=(int*) calloc(RAMSEY_instance->rcnt,sizeof(int));
					RAMSEY_instance->rmatind=(int*) calloc(RAMSEY_instance->nzcnt,sizeof(int));
					RAMSEY_instance->rmatval=(double*) calloc(RAMSEY_instance->nzcnt,sizeof(double));


					int ccnt = 0;
					RAMSEY_instance->rmatval[ccnt]=MM[s];
					RAMSEY_instance->rmatind[ccnt++]=s;

					if (t!=s)
					{
						RAMSEY_instance->rmatval[ccnt]=MM[t];
						RAMSEY_instance->rmatind[ccnt++]=t;
					}

					if (h!=s && h!=t)
					{
						RAMSEY_instance->rmatval[ccnt]=MM[h];
						RAMSEY_instance->rmatind[ccnt++]=h;
					}

					if (RAMSEY_instance->PARAM_COVER_CUTS)
					{

						if (RAMSEY_instance->rmatval[0] > 1)
						{
							RAMSEY_instance->rmatval[0] = 1;
						}
						if (RAMSEY_instance->rmatval[1] > 1)
						{
							RAMSEY_instance->rmatval[1] = 1;
						}
						if (RAMSEY_instance->rmatval[2] > 1)
						{
							RAMSEY_instance->rmatval[2] = 1;
						}
					}

					RAMSEY_instance->status=CPXaddrows(RAMSEY_instance->env_MODEL_3,RAMSEY_instance->lp_MODEL_3,0,RAMSEY_instance->rcnt, ccnt, RAMSEY_instance->rhs,RAMSEY_instance->sense,RAMSEY_instance->rmatbeg,RAMSEY_instance->rmatind,RAMSEY_instance->rmatval,NULL,NULL);
					if(RAMSEY_instance->status!=0)
					{
						printf("error in CPXaddrows\n");
						exit(-1);
					}
					cons_counter++;

					free(RAMSEY_instance->rmatbeg);
					free(RAMSEY_instance->rmatval);
					free(RAMSEY_instance->rmatind);
					free(RAMSEY_instance->rhs);
					free(RAMSEY_instance->sense);
				}

			}

			cout << "SKIP_N_SEPARATION FLAG->\t" << RAMSEY_instance->SKIP_N_SEPARATION << "\t cons_counter \t" << cons_counter << endl;

			RAMSEY_instance->TRIANGLES_CUTS_N=cons_counter;
		}


		delete []MM;
	}
	////////////////////////////////////////////////////////////////////////////////////////////


	/////////////////////////////////////// quadrangles /////////////////////////////////////////////////////
	////////////////////////////////////////////////////////////////////////////////////////////

	RAMSEY_instance->QUADRANGLES_CUTS_M=0;
	RAMSEY_instance->QUADRANGLES_CUTS_N=0;

	if(RAMSEY_instance->AVOID_QUADRANGLES==1)
	{

		int *MM=new int[RAMSEY_instance->n_variable_MODEL_3];

		//////////////////////////////  blue quadrangles //////////////////////////////////////
		//  y_s + y_t + y_u + y_h(s+t) + y_h(t+u) + y_h(s+t+u) <= 5


		if(RAMSEY_instance->PARAM_M==4)
		{
			int cons_counter=0;

			RAMSEY_instance->SKIP_M_SEPARATION=true;

			for(int s=0; s< RAMSEY_instance->n_variable_MODEL_3; s++)
			{
				for(int t=0; t< RAMSEY_instance->n_variable_MODEL_3; t++)
				{
					for(int u=0; u< RAMSEY_instance->n_variable_MODEL_3; u++)
					{


						int st = s + 1 + t + 1;
						int h_st = mapping_h(RAMSEY_instance, st) - 1;

						//						if(t>h_st){continue;}
						if(h_st < 0){continue;}

						int tu = t + 1 + u + 1;
						int h_tu = mapping_h(RAMSEY_instance, tu) - 1;

						//						if(u>h_tu){continue;}
						if(h_tu < 0){continue;}

						int stu = s + 1 + t + 1 + u + 1;
						int h_stu = mapping_h(RAMSEY_instance, stu) - 1;


						//						if(h_tu>h_stu){continue;}
						if(h_stu < 0){continue;}


						MM[s]=0;
						MM[t]=0;
						MM[u]=0;
						MM[h_st]=0;
						MM[h_tu]=0;
						MM[h_stu]=0;


						MM[s]++;
						MM[t]++;
						MM[u]++;
						MM[h_st]++;
						MM[h_tu]++;
						MM[h_stu]++;


						RAMSEY_instance->rcnt=1;
						RAMSEY_instance->nzcnt=6;
						RAMSEY_instance->rhs=(double*) calloc(RAMSEY_instance->rcnt,sizeof(double));
						RAMSEY_instance->sense=(char*) calloc(RAMSEY_instance->rcnt,sizeof(double));

						RAMSEY_instance->rhs[0]=5;

						RAMSEY_instance->sense[0]='L';

						RAMSEY_instance->rmatbeg=(int*) calloc(RAMSEY_instance->rcnt,sizeof(int));
						RAMSEY_instance->rmatind=(int*) calloc(RAMSEY_instance->nzcnt,sizeof(int));
						RAMSEY_instance->rmatval=(double*) calloc(RAMSEY_instance->nzcnt,sizeof(double));


						int ccnt = 0;
						RAMSEY_instance->rmatval[ccnt]=MM[s];
						RAMSEY_instance->rmatind[ccnt++]=s;

						if (t!=s)
						{
							RAMSEY_instance->rmatval[ccnt]=MM[t];
							RAMSEY_instance->rmatind[ccnt++]=t;
						}

						if (u!=s && u!=t)
						{
							RAMSEY_instance->rmatval[ccnt]=MM[u];
							RAMSEY_instance->rmatind[ccnt++]=u;
						}

						if (h_st!=s && h_st!=t && h_st != u)
						{
							RAMSEY_instance->rmatval[ccnt]=MM[h_st];
							RAMSEY_instance->rmatind[ccnt++]=h_st;
						}

						if (h_tu!=s && h_tu!=t && h_tu!=u && h_tu != h_st)
						{
							RAMSEY_instance->rmatval[ccnt]=MM[h_tu];
							RAMSEY_instance->rmatind[ccnt++]=h_tu;
						}

						if (h_stu!=s && h_stu!=t && h_stu!=u && h_stu!= h_st && h_stu!= h_tu)
						{
							RAMSEY_instance->rmatval[ccnt]=MM[h_stu];
							RAMSEY_instance->rmatind[ccnt++]=h_stu;
						}

						if (RAMSEY_instance->PARAM_COVER_CUTS)
						{
							for (int q=0; q < 6; q++)
								if (RAMSEY_instance->rmatval[q] > 1)
								{
									RAMSEY_instance->rmatval[q] = 1;
								}
							RAMSEY_instance->rhs[0] = ccnt - 1;
						}

						RAMSEY_instance->status=CPXaddrows(RAMSEY_instance->env_MODEL_3,RAMSEY_instance->lp_MODEL_3,0,RAMSEY_instance->rcnt, ccnt, RAMSEY_instance->rhs,RAMSEY_instance->sense,RAMSEY_instance->rmatbeg,RAMSEY_instance->rmatind,RAMSEY_instance->rmatval,NULL,NULL);
						if(RAMSEY_instance->status!=0)
						{
							printf("error in CPXaddrows\n");
							exit(-1);
						}
						cons_counter++;

						free(RAMSEY_instance->rmatbeg);
						free(RAMSEY_instance->rmatval);
						free(RAMSEY_instance->rmatind);
						free(RAMSEY_instance->rhs);
						free(RAMSEY_instance->sense);
					}

				}

			}

			cout << "SKIP_M_SEPARATION FLAG->\t" << RAMSEY_instance->SKIP_M_SEPARATION << "\t cons_counter \t" << cons_counter << endl;

			RAMSEY_instance->QUADRANGLES_CUTS_M=cons_counter;

		}



		//////////////////////////////  red quadrangles //////////////////////////////////////
		//  y_s + y_t + y_u + y_h(s+t) + y_h(t+u) + y_h(s+t+u) >= 1


		if(RAMSEY_instance->PARAM_N==4)
		{
			int cons_counter=0;

			RAMSEY_instance->SKIP_N_SEPARATION=true;

			for(int s=0; s< RAMSEY_instance->n_variable_MODEL_3; s++)
			{
				for(int t=0; t< RAMSEY_instance->n_variable_MODEL_3; t++)
				{
					for(int u=0; u< RAMSEY_instance->n_variable_MODEL_3; u++)
					{


						int st = s + 1 + t + 1;
						int h_st = mapping_h(RAMSEY_instance, st) - 1;

						//						if(t>h_st){continue;}
						if(h_st < 0){continue;}

						int tu = t + 1 + u + 1;
						int h_tu = mapping_h(RAMSEY_instance, tu) - 1;

						//						if(u>h_tu){continue;}
						if(h_tu < 0){continue;}

						int stu = s + 1 + t + 1 + u + 1;
						int h_stu = mapping_h(RAMSEY_instance, stu) - 1;


						//						if(h_tu>h_stu){continue;}
						if(h_stu < 0){continue;}


						MM[s]=0;
						MM[t]=0;
						MM[u]=0;
						MM[h_st]=0;
						MM[h_tu]=0;
						MM[h_stu]=0;


						MM[s]++;
						MM[t]++;
						MM[u]++;
						MM[h_st]++;
						MM[h_tu]++;
						MM[h_stu]++;


						RAMSEY_instance->rcnt=1;
						RAMSEY_instance->nzcnt=6;
						RAMSEY_instance->rhs=(double*) calloc(RAMSEY_instance->rcnt,sizeof(double));
						RAMSEY_instance->sense=(char*) calloc(RAMSEY_instance->rcnt,sizeof(double));

						RAMSEY_instance->rhs[0]=1;

						RAMSEY_instance->sense[0]='G';

						RAMSEY_instance->rmatbeg=(int*) calloc(RAMSEY_instance->rcnt,sizeof(int));
						RAMSEY_instance->rmatind=(int*) calloc(RAMSEY_instance->nzcnt,sizeof(int));
						RAMSEY_instance->rmatval=(double*) calloc(RAMSEY_instance->nzcnt,sizeof(double));


						int ccnt = 0;
						RAMSEY_instance->rmatval[ccnt]=MM[s];
						RAMSEY_instance->rmatind[ccnt++]=s;

						if (t!=s)
						{
							RAMSEY_instance->rmatval[ccnt]=MM[t];
							RAMSEY_instance->rmatind[ccnt++]=t;
						}

						if (u!=s && u!=t)
						{
							RAMSEY_instance->rmatval[ccnt]=MM[u];
							RAMSEY_instance->rmatind[ccnt++]=u;
						}

						if (h_st!=s && h_st!=t && h_st != u)
						{
							RAMSEY_instance->rmatval[ccnt]=MM[h_st];
							RAMSEY_instance->rmatind[ccnt++]=h_st;
						}

						if (h_tu!=s && h_tu!=t && h_tu!=u && h_tu != h_st)
						{
							RAMSEY_instance->rmatval[ccnt]=MM[h_tu];
							RAMSEY_instance->rmatind[ccnt++]=h_tu;
						}

						if (h_stu!=s && h_stu!=t && h_stu!=u && h_stu!= h_st && h_stu!= h_tu)
						{
							RAMSEY_instance->rmatval[ccnt]=MM[h_stu];
							RAMSEY_instance->rmatind[ccnt++]=h_stu;
						}

						if (RAMSEY_instance->PARAM_COVER_CUTS)
						{
							for (int q=0; q < 6; q++)
								if (RAMSEY_instance->rmatval[q] > 1)
								{
									RAMSEY_instance->rmatval[q] = 1;
								}
						}

						RAMSEY_instance->status=CPXaddrows(RAMSEY_instance->env_MODEL_3,RAMSEY_instance->lp_MODEL_3,0,RAMSEY_instance->rcnt, ccnt, RAMSEY_instance->rhs,RAMSEY_instance->sense,RAMSEY_instance->rmatbeg,RAMSEY_instance->rmatind,RAMSEY_instance->rmatval,NULL,NULL);
						if(RAMSEY_instance->status!=0)
						{
							printf("error in CPXaddrows\n");
							exit(-1);
						}
						cons_counter++;

						free(RAMSEY_instance->rmatbeg);
						free(RAMSEY_instance->rmatval);
						free(RAMSEY_instance->rmatind);
						free(RAMSEY_instance->rhs);
						free(RAMSEY_instance->sense);
					}

				}

			}

			cout << "SKIP_N_SEPARATION FLAG->\t" << RAMSEY_instance->SKIP_N_SEPARATION << "\t cons_counter \t" << cons_counter << endl;

			RAMSEY_instance->QUADRANGLES_CUTS_N=cons_counter;
		}

		delete []MM;
	}
	////////////////////////////////////////////////////////////////////////////////////////////



	////////////////////////////////////////////////////////////////////////////////////////////
	if(RAMSEY_instance->PARAM_CLIQUE_JUMP_CUTS==1)
	{
		int* SET=new int[RAMSEY_instance->PARAM_SIZE_GRAPH];
		int set_size = 0;
		int count_sets = 0;

		// uniform jumps from 0
		// sum_{k=1}^{m-1} y_{h(k*s)} <= m - 2, for all s in {1,..., hat q}
		if(RAMSEY_instance->SKIP_M_SEPARATION==false)
		{

			for(int s=0; s< RAMSEY_instance->n_variable_MODEL_3; s++)
			{

				set_size = 0;
				SET[0]=1;
				for(int k=1; k<RAMSEY_instance->PARAM_SIZE_GRAPH; k++)
				{
					SET[k]=0;
				}

				for(int k=1; k<=RAMSEY_instance->PARAM_M-1; k++)
				{
					int mm = modulo(RAMSEY_instance, (s+1)*k);
					if (SET[mm] == 0)
					{
						set_size++;
						SET[mm]=1;
					}
				}

				if (set_size < RAMSEY_instance->PARAM_M-1) continue;
				count_sets++;


				RAMSEY_instance->rcnt=1;
				RAMSEY_instance->nzcnt=RAMSEY_instance->n_variable_MODEL_3;
				RAMSEY_instance->rhs=(double*) calloc(RAMSEY_instance->rcnt,sizeof(double));
				RAMSEY_instance->sense=(char*) calloc(RAMSEY_instance->rcnt,sizeof(double));

				RAMSEY_instance->rhs[0]=RAMSEY_instance->PARAM_M-2;

				RAMSEY_instance->sense[0]='L';

				RAMSEY_instance->rmatbeg=(int*) calloc(RAMSEY_instance->rcnt,sizeof(int));
				RAMSEY_instance->rmatind=(int*) calloc(RAMSEY_instance->nzcnt,sizeof(int));
				RAMSEY_instance->rmatval=(double*) calloc(RAMSEY_instance->nzcnt,sizeof(double));


				for(int i=0; i<RAMSEY_instance->n_variable_MODEL_3; i++)
				{
					RAMSEY_instance->rmatval[i]=0.0;
					RAMSEY_instance->rmatind[i]=i;
				}

				for(int k=1; k<=RAMSEY_instance->PARAM_M-1; k++)
				{
					RAMSEY_instance->rmatval[mapping_h(RAMSEY_instance, (s+1)*k) - 1]++;

					if (mapping_h(RAMSEY_instance, (s+1)*k) == 0)
					{
						cout << "mapping_h is ZERO!\n";
						exit(1);
					}

				}

				RAMSEY_instance->status=CPXaddrows(RAMSEY_instance->env_MODEL_3,RAMSEY_instance->lp_MODEL_3,0,RAMSEY_instance->rcnt,
						RAMSEY_instance->nzcnt,
						RAMSEY_instance->rhs,RAMSEY_instance->sense,RAMSEY_instance->rmatbeg,RAMSEY_instance->rmatind,RAMSEY_instance->rmatval,NULL,NULL);
				if(RAMSEY_instance->status!=0)
				{
					printf("error in CPXaddrows\n");
					exit(-1);
				}

				free(RAMSEY_instance->rmatbeg);
				free(RAMSEY_instance->rmatval);
				free(RAMSEY_instance->rmatind);
				free(RAMSEY_instance->rhs);
				free(RAMSEY_instance->sense);

			}
		}
		cout << "number of uniform blue jumps added: " << count_sets << endl;

		RAMSEY_instance->CLIQUE_JUMP_CUTS_M=count_sets;


		///////////////////  uniform red jumps ///////////////////////////////////////////////////
		set_size = 0;
		count_sets = 0;

		// uniform jumps
		// sum_{k=1}^{m-1} y_{h(k*s)} <= m - 2, for all s in {1,..., hat q}
		if(RAMSEY_instance->SKIP_N_SEPARATION==false)
		{

			for(int s=0; s< RAMSEY_instance->n_variable_MODEL_3; s++)
			{

				set_size = 0;
				SET[0]=1;
				for(int k=1; k<RAMSEY_instance->PARAM_SIZE_GRAPH; k++)
				{
					SET[k]=0;
				}

				for(int k=1; k<=RAMSEY_instance->PARAM_N-1; k++)
				{
					int mm = modulo(RAMSEY_instance, (s+1)*k);
					if (SET[mm] == 0)
					{
						set_size++;
						SET[mm]=1;
					}
				}

				if (set_size < RAMSEY_instance->PARAM_N-1) continue;
				count_sets++;

				RAMSEY_instance->rcnt=1;
				RAMSEY_instance->nzcnt=RAMSEY_instance->n_variable_MODEL_3;
				RAMSEY_instance->rhs=(double*) calloc(RAMSEY_instance->rcnt,sizeof(double));
				RAMSEY_instance->sense=(char*) calloc(RAMSEY_instance->rcnt,sizeof(double));

				RAMSEY_instance->rhs[0]=1.0;

				RAMSEY_instance->sense[0]='G';

				RAMSEY_instance->rmatbeg=(int*) calloc(RAMSEY_instance->rcnt,sizeof(int));
				RAMSEY_instance->rmatind=(int*) calloc(RAMSEY_instance->nzcnt,sizeof(int));
				RAMSEY_instance->rmatval=(double*) calloc(RAMSEY_instance->nzcnt,sizeof(double));


				for(int i=0; i<RAMSEY_instance->n_variable_MODEL_3; i++)
				{
					RAMSEY_instance->rmatval[i]=0.0;
					RAMSEY_instance->rmatind[i]=i;
				}

				for(int k=1; k<=RAMSEY_instance->PARAM_N-1; k++)
				{
					RAMSEY_instance->rmatval[mapping_h(RAMSEY_instance, (s+1)*k) - 1]++;

					if (mapping_h(RAMSEY_instance, (s+1)*k) == 0)
					{
						cout << "mapping_h is ZERO!\n";
						exit(1);
					}
				}

				RAMSEY_instance->status=CPXaddrows(RAMSEY_instance->env_MODEL_3,RAMSEY_instance->lp_MODEL_3,0,RAMSEY_instance->rcnt,
						RAMSEY_instance->nzcnt,
						RAMSEY_instance->rhs,RAMSEY_instance->sense,RAMSEY_instance->rmatbeg,RAMSEY_instance->rmatind,RAMSEY_instance->rmatval,NULL,NULL);
				if(RAMSEY_instance->status!=0)
				{
					printf("error in CPXaddrows\n");
					exit(-1);
				}

				free(RAMSEY_instance->rmatbeg);
				free(RAMSEY_instance->rmatval);
				free(RAMSEY_instance->rmatind);
				free(RAMSEY_instance->rhs);
				free(RAMSEY_instance->sense);

			}
		}

		cout << "number of uniform red jumps added: " << count_sets << endl;

		RAMSEY_instance->CLIQUE_JUMP_CUTS_N=count_sets;

		delete[] SET;
	}
	////////////////////////////////////////////////////////////////////////////////////////////




	if(RAMSEY_instance->PARAM_K_CUTS>=1)
	{

		if(RAMSEY_instance->LOOK_UP_RAMSEY[RAMSEY_instance->PARAM_M-1][RAMSEY_instance->PARAM_N]!=-1)
		{

			cout << "\nLOOK_UP_RAMSEY BLUE\t" << RAMSEY_instance->LOOK_UP_RAMSEY[RAMSEY_instance->PARAM_M-1][RAMSEY_instance->PARAM_N] << endl <<endl;


			if(RAMSEY_instance->PARAM_SIZE_GRAPH%2==0)
			{

				RAMSEY_instance->rcnt=1;
				RAMSEY_instance->nzcnt=RAMSEY_instance->n_variable_MODEL_3;
				RAMSEY_instance->rhs=(double*) calloc(RAMSEY_instance->rcnt,sizeof(double));
				RAMSEY_instance->sense=(char*) calloc(RAMSEY_instance->rcnt,sizeof(double));

				RAMSEY_instance->rhs[0]=0.0;
				RAMSEY_instance->sense[0]='L';


				RAMSEY_instance->rmatbeg=(int*) calloc(RAMSEY_instance->rcnt,sizeof(int));
				RAMSEY_instance->rmatind=(int*) calloc(RAMSEY_instance->nzcnt,sizeof(int));
				RAMSEY_instance->rmatval=(double*) calloc(RAMSEY_instance->nzcnt,sizeof(double));

				int counter_local=0;
				for(int j=0; j<RAMSEY_instance->n_variable_MODEL_3-1; j++)
				{
					RAMSEY_instance->rmatval[counter_local]=2.0;
					RAMSEY_instance->rmatind[counter_local++]=j;
				}
				RAMSEY_instance->rmatval[counter_local]=1.0;
				RAMSEY_instance->rmatind[counter_local++]=RAMSEY_instance->n_variable_MODEL_3-1;


				int coeff=min(RAMSEY_instance->LOOK_UP_RAMSEY[RAMSEY_instance->PARAM_M-1][RAMSEY_instance->PARAM_N],RAMSEY_instance->PARAM_SIZE_GRAPH);

				RAMSEY_instance->rhs[0]=(coeff-1);

				cout << "RHS\t" << RAMSEY_instance->rhs[0] << endl;

				RAMSEY_instance->status=CPXaddrows(RAMSEY_instance->env_MODEL_3,RAMSEY_instance->lp_MODEL_3,0,RAMSEY_instance->rcnt,RAMSEY_instance->nzcnt,RAMSEY_instance->rhs,RAMSEY_instance->sense,RAMSEY_instance->rmatbeg,RAMSEY_instance->rmatind,RAMSEY_instance->rmatval,NULL,NULL);
				if(RAMSEY_instance->status!=0)
				{
					printf("error in CPXaddrows\n");
					exit(-1);
				}

				free(RAMSEY_instance->rmatbeg);
				free(RAMSEY_instance->rmatval);
				free(RAMSEY_instance->rmatind);
				free(RAMSEY_instance->rhs);
				free(RAMSEY_instance->sense);
			}
			else
			{

				RAMSEY_instance->rcnt=1;
				RAMSEY_instance->nzcnt=RAMSEY_instance->n_variable_MODEL_3;
				RAMSEY_instance->rhs=(double*) calloc(RAMSEY_instance->rcnt,sizeof(double));
				RAMSEY_instance->sense=(char*) calloc(RAMSEY_instance->rcnt,sizeof(double));

				RAMSEY_instance->rhs[0]=0.0;
				RAMSEY_instance->sense[0]='L';


				RAMSEY_instance->rmatbeg=(int*) calloc(RAMSEY_instance->rcnt,sizeof(int));
				RAMSEY_instance->rmatind=(int*) calloc(RAMSEY_instance->nzcnt,sizeof(int));
				RAMSEY_instance->rmatval=(double*) calloc(RAMSEY_instance->nzcnt,sizeof(double));

				int counter_local=0;
				for(int j=0; j<RAMSEY_instance->n_variable_MODEL_3; j++)
				{
					RAMSEY_instance->rmatval[counter_local]=1.0;
					RAMSEY_instance->rmatind[counter_local++]=j;
				}


				int coeff=min(RAMSEY_instance->LOOK_UP_RAMSEY[RAMSEY_instance->PARAM_M-1][RAMSEY_instance->PARAM_N],RAMSEY_instance->PARAM_SIZE_GRAPH);

				RAMSEY_instance->rhs[0]=(int)((coeff-1)/2.0);

				cout << "RHS\t" << RAMSEY_instance->rhs[0] << endl;

				RAMSEY_instance->status=CPXaddrows(RAMSEY_instance->env_MODEL_3,RAMSEY_instance->lp_MODEL_3,0,RAMSEY_instance->rcnt,RAMSEY_instance->nzcnt,RAMSEY_instance->rhs,RAMSEY_instance->sense,RAMSEY_instance->rmatbeg,RAMSEY_instance->rmatind,RAMSEY_instance->rmatval,NULL,NULL);
				if(RAMSEY_instance->status!=0)
				{
					printf("error in CPXaddrows\n");
					exit(-1);
				}

				free(RAMSEY_instance->rmatbeg);
				free(RAMSEY_instance->rmatval);
				free(RAMSEY_instance->rmatind);
				free(RAMSEY_instance->rhs);
				free(RAMSEY_instance->sense);

			}

		}

		if(RAMSEY_instance->LOOK_UP_RAMSEY[RAMSEY_instance->PARAM_M][RAMSEY_instance->PARAM_N-1]!=-1)
		{

			cout << "\nLOOK_UP_RAMSEY RED\t" << RAMSEY_instance->LOOK_UP_RAMSEY[RAMSEY_instance->PARAM_M][RAMSEY_instance->PARAM_N-1] << endl <<endl;


			if(RAMSEY_instance->PARAM_SIZE_GRAPH%2==0)
			{

				RAMSEY_instance->rcnt=1;
				RAMSEY_instance->nzcnt=RAMSEY_instance->n_variable_MODEL_3;
				RAMSEY_instance->rhs=(double*) calloc(RAMSEY_instance->rcnt,sizeof(double));
				RAMSEY_instance->sense=(char*) calloc(RAMSEY_instance->rcnt,sizeof(double));

				RAMSEY_instance->rhs[0]=0.0;
				RAMSEY_instance->sense[0]='G';


				RAMSEY_instance->rmatbeg=(int*) calloc(RAMSEY_instance->rcnt,sizeof(int));
				RAMSEY_instance->rmatind=(int*) calloc(RAMSEY_instance->nzcnt,sizeof(int));
				RAMSEY_instance->rmatval=(double*) calloc(RAMSEY_instance->nzcnt,sizeof(double));

				int counter_local=0;
				for(int j=0; j<RAMSEY_instance->n_variable_MODEL_3-1; j++)
				{
					RAMSEY_instance->rmatval[counter_local]=2.0;
					RAMSEY_instance->rmatind[counter_local++]=j;
				}
				RAMSEY_instance->rmatval[counter_local]=1.0;
				RAMSEY_instance->rmatind[counter_local++]=RAMSEY_instance->n_variable_MODEL_3-1;

				int coeff=min(RAMSEY_instance->LOOK_UP_RAMSEY[RAMSEY_instance->PARAM_M][RAMSEY_instance->PARAM_N-1],RAMSEY_instance->PARAM_SIZE_GRAPH);

				RAMSEY_instance->rhs[0]= 2*RAMSEY_instance->n_variable_MODEL_3 - coeff;

				if(RAMSEY_instance->rhs[0]==0){RAMSEY_instance->rhs[0]=1;}

				cout << "RHS\t" << RAMSEY_instance->rhs[0] << endl;

				RAMSEY_instance->status=CPXaddrows(RAMSEY_instance->env_MODEL_3,RAMSEY_instance->lp_MODEL_3,0,RAMSEY_instance->rcnt,RAMSEY_instance->nzcnt,RAMSEY_instance->rhs,RAMSEY_instance->sense,RAMSEY_instance->rmatbeg,RAMSEY_instance->rmatind,RAMSEY_instance->rmatval,NULL,NULL);
				if(RAMSEY_instance->status!=0)
				{
					printf("error in CPXaddrows\n");
					exit(-1);
				}

				free(RAMSEY_instance->rmatbeg);
				free(RAMSEY_instance->rmatval);
				free(RAMSEY_instance->rmatind);
				free(RAMSEY_instance->rhs);
				free(RAMSEY_instance->sense);
			}
			else
			{
				RAMSEY_instance->rcnt=1;
				RAMSEY_instance->nzcnt=RAMSEY_instance->n_variable_MODEL_3;
				RAMSEY_instance->rhs=(double*) calloc(RAMSEY_instance->rcnt,sizeof(double));
				RAMSEY_instance->sense=(char*) calloc(RAMSEY_instance->rcnt,sizeof(double));

				RAMSEY_instance->rhs[0]=0.0;
				RAMSEY_instance->sense[0]='G';


				RAMSEY_instance->rmatbeg=(int*) calloc(RAMSEY_instance->rcnt,sizeof(int));
				RAMSEY_instance->rmatind=(int*) calloc(RAMSEY_instance->nzcnt,sizeof(int));
				RAMSEY_instance->rmatval=(double*) calloc(RAMSEY_instance->nzcnt,sizeof(double));

				int counter_local=0;
				for(int j=0; j<RAMSEY_instance->n_variable_MODEL_3; j++)
				{
					RAMSEY_instance->rmatval[counter_local]=1.0;
					RAMSEY_instance->rmatind[counter_local++]=j;
				}


				int coeff=min(RAMSEY_instance->LOOK_UP_RAMSEY[RAMSEY_instance->PARAM_M][RAMSEY_instance->PARAM_N-1],RAMSEY_instance->PARAM_SIZE_GRAPH);

				RAMSEY_instance->rhs[0]= RAMSEY_instance->n_variable_MODEL_3 - (int)(coeff-1)/2.0;

				if(RAMSEY_instance->rhs[0]==0){RAMSEY_instance->rhs[0]=1;}

				cout << "RHS\t" << RAMSEY_instance->rhs[0] << endl;

				RAMSEY_instance->status=CPXaddrows(RAMSEY_instance->env_MODEL_3,RAMSEY_instance->lp_MODEL_3,0,RAMSEY_instance->rcnt,RAMSEY_instance->nzcnt,RAMSEY_instance->rhs,RAMSEY_instance->sense,RAMSEY_instance->rmatbeg,RAMSEY_instance->rmatind,RAMSEY_instance->rmatval,NULL,NULL);
				if(RAMSEY_instance->status!=0)
				{
					printf("error in CPXaddrows\n");
					exit(-1);
				}

				free(RAMSEY_instance->rmatbeg);
				free(RAMSEY_instance->rmatval);
				free(RAMSEY_instance->rmatind);
				free(RAMSEY_instance->rhs);
				free(RAMSEY_instance->sense);

			}
			///////////////////////////////////////////////////////////////////////////////////////////////////////
			///////////////////////////////////////////////////////////////////////////////////////////////////////





		}
	}


	cout << "\n\n****MAXIMIZATION\n\n";
	CPXchgobjsen(RAMSEY_instance->env_MODEL_3,RAMSEY_instance->lp_MODEL_3,CPX_MAX);

	//	cout << "\n\n****MINIMIZATION\n\n";
	//	CPXchgobjsen(RAMSEY_instance->env_MODEL_3,RAMSEY_instance->lp_MODEL_3,CPX_MIN);


#ifdef	PRINT_MODEL_3_LP
	/////////////////////////////////////////////////////////////////////////////////////////////////////////
	// * writing the created ILP model on a file *
	RAMSEY_instance->status=CPXwriteprob(RAMSEY_instance->env_MODEL_3,RAMSEY_instance->lp_MODEL_3,"RAMSEY_MODEL_3.lp",NULL);
	if(RAMSEY_instance->status!=0)
	{
		printf("error in CPXwriteprob\n");
		exit(-1);
	}
	cout << "INITIAL MASTER WRITTEN\n";
	/////////////////////////////////////////////////////////////////////////////////////////////////////////
#endif


}
