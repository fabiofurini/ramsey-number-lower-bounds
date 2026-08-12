

#include "CLIQUE_CPLEX.h"


///////////////////////////////////////////////////////////////////////////////
//#define print_clique_model
///////////////////////////////////////////////////////////////////////////////


/***************************************************************************/
void clique_free_cplex (data *RAMSEY_instance)
/***************************************************************************/
{
	RAMSEY_instance->status=CPXfreeprob(RAMSEY_instance->env_clique,&(RAMSEY_instance->lp_clique));
	if(RAMSEY_instance->status!=0) {printf("error in CPXfreeprob\n");exit(-1);}

	RAMSEY_instance->status=CPXcloseCPLEX(&(RAMSEY_instance->env_clique));
	if(RAMSEY_instance->status!=0) {printf("error in CPXcloseCPLEX\n");exit(-1);}

}

/***************************************************************************/
void clique_load_cplex (data *RAMSEY_instance)
/***************************************************************************/
{

	RAMSEY_instance->env_clique=CPXopenCPLEX(&RAMSEY_instance->status);
	if (RAMSEY_instance->status!=0)
	{
		printf("cannot open CPLEX environment \n ");
		exit(-1);

	}
	RAMSEY_instance->lp_clique=CPXcreateprob(RAMSEY_instance->env_clique,&RAMSEY_instance->status,"clique");
	if (RAMSEY_instance->status!=0)
	{
		printf("cannot create problem\n");
		exit(-1);
	}
	CPXchgobjsen(RAMSEY_instance->env_clique,RAMSEY_instance->lp_clique,CPX_MAX);


	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	// * Set printing *

	CPXsetintparam (RAMSEY_instance->env_clique, CPX_PARAM_SCRIND, CPX_OFF);

	// * Set number of CPU*

	RAMSEY_instance->status = CPXsetintparam (RAMSEY_instance->env_clique, CPX_PARAM_THREADS, 1);
	if (RAMSEY_instance->status)
	{
		printf ("error for CPX_PARAM_EPRHS\n");
	}

	// * Set time limit *

	RAMSEY_instance->status = CPXsetdblparam (RAMSEY_instance->env_clique, CPX_PARAM_TILIM,RAMSEY_instance->PARAM_TIME_LIMIT);
	if (RAMSEY_instance->status)
	{
		printf ("error for CPX_PARAM_EPRHS\n");
	}

	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


	RAMSEY_instance->ccnt=RAMSEY_instance->PARAM_SIZE_GRAPH;
	RAMSEY_instance->obj=(double*) calloc(RAMSEY_instance->ccnt,sizeof(double));
	RAMSEY_instance->lb=(double*) calloc(RAMSEY_instance->ccnt,sizeof(double));
	RAMSEY_instance->ub=(double*) calloc(RAMSEY_instance->ccnt,sizeof(double));
	RAMSEY_instance->xctype=(char*) calloc(RAMSEY_instance->ccnt,sizeof(char));


	char **colname=(char**) calloc(RAMSEY_instance->ccnt,sizeof(char*));
	for(int i=0;i<RAMSEY_instance->ccnt;i++){colname[i]=(char*) calloc(1000,sizeof(char));}


	for(int i=0; i<RAMSEY_instance->ccnt; i++)
	{
		RAMSEY_instance->obj[i]=1.0;
		RAMSEY_instance->lb[i]=0.0;
		RAMSEY_instance->ub[i]=1.0;

		if(RAMSEY_instance->PARAM_CIRCULANT==1 && i==0)
		{
			RAMSEY_instance->lb[i]=1.0;
		}

		printf(colname[i], "x%d",i);
		RAMSEY_instance->xctype[i]='B';
	}

	RAMSEY_instance->status=CPXnewcols(RAMSEY_instance->env_clique,RAMSEY_instance->lp_clique,RAMSEY_instance->ccnt,RAMSEY_instance->obj,RAMSEY_instance->lb,RAMSEY_instance->ub,RAMSEY_instance->xctype,colname);
	if(RAMSEY_instance->status!=0)
	{
		printf("error in CPXnewcols\n");
		exit(-1);
	}

	for(int i=0;i<RAMSEY_instance->ccnt;i++){free(colname[i]);}
	free(colname);

	free(RAMSEY_instance->obj);
	free(RAMSEY_instance->lb);
	free(RAMSEY_instance->ub);
	free(RAMSEY_instance->xctype);




	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	//adding all (relaxed) edge constraints x_i + x_j <= 2
	for(int i=0; i<RAMSEY_instance->PARAM_SIZE_GRAPH; i++)
	{
		for(int j=i+1; j<RAMSEY_instance->PARAM_SIZE_GRAPH; j++)
		{

			RAMSEY_instance->rcnt=1;
			RAMSEY_instance->nzcnt=2;
			RAMSEY_instance->rhs=(double*) calloc(RAMSEY_instance->rcnt,sizeof(double));
			RAMSEY_instance->sense=(char*) calloc(RAMSEY_instance->rcnt,sizeof(double));

			RAMSEY_instance->rhs[0]=2;
			RAMSEY_instance->sense[0]='L';

			RAMSEY_instance->rmatbeg=(int*) calloc(RAMSEY_instance->rcnt,sizeof(int));
			RAMSEY_instance->rmatind=(int*) calloc(RAMSEY_instance->nzcnt,sizeof(int));
			RAMSEY_instance->rmatval=(double*) calloc(RAMSEY_instance->nzcnt,sizeof(double));

			RAMSEY_instance->rmatval[0]=1.0;
			RAMSEY_instance->rmatind[0]=i;

			RAMSEY_instance->rmatval[1]=1.0;
			RAMSEY_instance->rmatind[1]=j;

			RAMSEY_instance->status=CPXaddrows(RAMSEY_instance->env_clique,RAMSEY_instance->lp_clique,0,RAMSEY_instance->rcnt,RAMSEY_instance->nzcnt,RAMSEY_instance->rhs,RAMSEY_instance->sense,RAMSEY_instance->rmatbeg,RAMSEY_instance->rmatind,RAMSEY_instance->rmatval,NULL,NULL);
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
	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////





#ifdef print_clique_model
	cout << "\n\nPRINT CLIQUE CPLEX\n\n";
	RAMSEY_instance->status=CPXwriteprob(RAMSEY_instance->env_clique,RAMSEY_instance->lp_clique,"clique.lp",NULL);
	if(RAMSEY_instance->status!=0)
	{
		printf("error in CPXwriteprob\n");
		exit(-1);
	}
#endif


}

/***************************************************************************/
int  clique_solve_cplex_fixing(data *RAMSEY_instance,int color)
/***************************************************************************/
{

	//CPXsetintparam (RAMSEY_instance->env_clique, CPX_PARAM_SCRIND, CPX_ON);


	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	//setting the constraints according to the fixing

	int counter=0;
	for(int i=0; i<RAMSEY_instance->PARAM_SIZE_GRAPH; i++)
	{
		for(int j=i+1; j<RAMSEY_instance->PARAM_SIZE_GRAPH; j++)
		{

			if(RAMSEY_instance->edge_fixing[i][j]<0.5)
			{

				int indices=counter;
				double bd=1;

				RAMSEY_instance->status = CPXchgrhs(RAMSEY_instance->env_clique,RAMSEY_instance->lp_clique, 1, &indices, &bd);
				if(RAMSEY_instance->status!=0)
				{
					printf("error in CPXchgbds\n");
					exit(-1);
				}

			}
			else
			{
				int indices=counter;
				double bd=2;

				RAMSEY_instance->status = CPXchgrhs(RAMSEY_instance->env_clique,RAMSEY_instance->lp_clique, 1, &indices, &bd);
				if(RAMSEY_instance->status!=0)
				{
					printf("error in CPXchgbds\n");
					exit(-1);
				}
			}

			counter++;

		}
	}
	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


	RAMSEY_instance->status=CPXmipopt(RAMSEY_instance->env_clique,RAMSEY_instance->lp_clique);
	if(RAMSEY_instance->status!=0)
	{
		printf("error in CPXmipopt\n");
		exit(-1);
	}

	int stat=CPXgetstat(RAMSEY_instance->env_clique,RAMSEY_instance->lp_clique);
	if(RAMSEY_instance->status!=0)
	{
		cout << "problem in CPXgetstat";
		exit(-1);
	}


	RAMSEY_instance->status=CPXgetmipx(RAMSEY_instance->env_clique,RAMSEY_instance->lp_clique,RAMSEY_instance->CLIQUE_SOL,0,RAMSEY_instance->PARAM_SIZE_GRAPH-1);
	if(RAMSEY_instance->status!=0)
	{
		printf("error in CPXgetmipx\n");
		exit(-1);
	}

	RAMSEY_instance->status=CPXgetmipobjval(RAMSEY_instance->env_clique,RAMSEY_instance->lp_clique,&RAMSEY_instance->CLIQUE_VAL);
	if(RAMSEY_instance->status!=0)
	{
		cout << "problem in CPXgetmipobjval";
		exit(-1);
	}


	//	RAMSEY_instance->status=CPXwriteprob(RAMSEY_instance->env_clique,RAMSEY_instance->lp_clique,"clique_FIXING.lp",NULL);

	return stat;

}


/***************************************************************************/
void clique_blue_solve_cplex(data *RAMSEY_instance)
/***************************************************************************/
{

	//CPXsetintparam (RAMSEY_instance->env_clique, CPX_PARAM_SCRIND, CPX_ON);

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	//setting the constraints according to the blue edges

	int counter=0;
	for(int i=0; i<RAMSEY_instance->PARAM_SIZE_GRAPH; i++)
	{
		for(int j=i+1; j<RAMSEY_instance->PARAM_SIZE_GRAPH; j++)
		{

			if(RAMSEY_instance->X_CALLBACK[position_b(RAMSEY_instance,i,j)]<0.5)
			{
				int indices=counter;
				double bd=1;

				RAMSEY_instance->status = CPXchgrhs(RAMSEY_instance->env_clique,RAMSEY_instance->lp_clique, 1, &indices, &bd);
				if(RAMSEY_instance->status!=0)
				{
					printf("error in CPXchgbds\n");
					exit(-1);
				}

			}
			else
			{
				int indices=counter;
				double bd=2;

				RAMSEY_instance->status = CPXchgrhs(RAMSEY_instance->env_clique,RAMSEY_instance->lp_clique, 1, &indices, &bd);
				if(RAMSEY_instance->status!=0)
				{
					printf("error in CPXchgbds\n");
					exit(-1);
				}
			}

			counter++;

		}
	}
	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////



	RAMSEY_instance->status=CPXmipopt(RAMSEY_instance->env_clique,RAMSEY_instance->lp_clique);
	if(RAMSEY_instance->status!=0)
	{
		printf("error in CPXmipopt\n");
		exit(-1);
	}

	RAMSEY_instance->status=CPXgetmipx(RAMSEY_instance->env_clique,RAMSEY_instance->lp_clique,RAMSEY_instance->CLIQUE_SOL,0,RAMSEY_instance->PARAM_SIZE_GRAPH-1);
	if(RAMSEY_instance->status!=0)
	{
		printf("error in CPXgetmipx\n");
		exit(-1);
	}

	RAMSEY_instance->status=CPXgetmipobjval(RAMSEY_instance->env_clique,RAMSEY_instance->lp_clique,&RAMSEY_instance->CLIQUE_VAL);
	if(RAMSEY_instance->status!=0)
	{
		cout << "problem in CPXgetmipobjval";
		exit(-1);
	}


	//	RAMSEY_instance->status=CPXwriteprob(RAMSEY_instance->env_clique,RAMSEY_instance->lp_clique,"clique_BLUE.lp",NULL);


}




/***************************************************************************/
void clique_red_solve_cplex(data *RAMSEY_instance)
/***************************************************************************/
{

	//CPXsetintparam (RAMSEY_instance->env_clique, CPX_PARAM_SCRIND, CPX_ON);


	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	//setting the constraints according to the red edges

	int counter=0;
	for(int i=0; i<RAMSEY_instance->PARAM_SIZE_GRAPH; i++)
	{
		for(int j=i+1; j<RAMSEY_instance->PARAM_SIZE_GRAPH; j++)
		{

			if(RAMSEY_instance->X_CALLBACK[position_r(RAMSEY_instance,i,j)]<0.5)
			{

				int indices=counter;
				double bd=1;

				RAMSEY_instance->status = CPXchgrhs(RAMSEY_instance->env_clique,RAMSEY_instance->lp_clique, 1, &indices, &bd);
				if(RAMSEY_instance->status!=0)
				{
					printf("error in CPXchgbds\n");
					exit(-1);
				}

			}
			else
			{
				int indices=counter;
				double bd=2;

				RAMSEY_instance->status = CPXchgrhs(RAMSEY_instance->env_clique,RAMSEY_instance->lp_clique, 1, &indices, &bd);
				if(RAMSEY_instance->status!=0)
				{
					printf("error in CPXchgbds\n");
					exit(-1);
				}
			}

			counter++;

		}
	}
	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////




	RAMSEY_instance->status=CPXmipopt(RAMSEY_instance->env_clique,RAMSEY_instance->lp_clique);
	if(RAMSEY_instance->status!=0)
	{
		printf("error in CPXmipopt\n");
		exit(-1);
	}

	RAMSEY_instance->status=CPXgetmipx(RAMSEY_instance->env_clique,RAMSEY_instance->lp_clique,RAMSEY_instance->CLIQUE_SOL,0,RAMSEY_instance->PARAM_SIZE_GRAPH-1);
	if(RAMSEY_instance->status!=0)
	{
		printf("error in CPXgetmipx\n");
		exit(-1);
	}

	RAMSEY_instance->status=CPXgetmipobjval(RAMSEY_instance->env_clique,RAMSEY_instance->lp_clique,&RAMSEY_instance->CLIQUE_VAL);
	if(RAMSEY_instance->status!=0)
	{
		cout << "problem in CPXgetmipobjval";
		exit(-1);
	}

	//	RAMSEY_instance->status=CPXwriteprob(RAMSEY_instance->env_clique,RAMSEY_instance->lp_clique,"clique_RED.lp",NULL);


}
