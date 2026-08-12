

#include "check_solution.h"



/***********************************************************************************/
bool check_solution(data *RAMSEY_instance,double *X_SOLUTION)
/***********************************************************************************/
{


	int SOL_OK=true;


	cout << "\n\n**************************************************************\n\n";
	cout << "---->>>>>CHECK SOLUTION\n\n";

	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	for(int i=0;i<RAMSEY_instance->PARAM_SIZE_GRAPH;i++)
	{
		for(int j=i+1;j<RAMSEY_instance->PARAM_SIZE_GRAPH;j++)
		{
			if(X_SOLUTION[position_b(RAMSEY_instance,i,j)]<0.5){RAMSEY_instance->edge_fixing[i][j]=0;}
			else{RAMSEY_instance->edge_fixing[i][j]=1;}
		}
	}

	for (int i = 0; i < RAMSEY_instance->PARAM_SIZE_GRAPH; i++){RAMSEY_instance->CLIQUE_BLUE[i]=0;}

	clock_t time_start_blue=clock();

	RAMSEY_instance->size_CLIQUE_BLUE=RAMSEY_instance->clique_solve_BB_edge_fixing
			(
					RAMSEY_instance->edge_fixing,
					RAMSEY_instance->CLIQUE_BLUE,
					0,
					RAMSEY_instance->PARAM_SIZE_GRAPH,
					0,
					0,
					0,
					0,
					0
			);

	clock_t time_end_blue=clock();

	double time_blue=(double)(time_end_blue-time_start_blue)/(double)CLOCKS_PER_SEC;



	///////////////////////////////////////////	///////////////////////////////////////////

	cout << "size_CLIQUE_BLUE\t" << RAMSEY_instance->size_CLIQUE_BLUE << "\t time \t" << time_blue << endl;

	cout << "CLIQUE blue\n";
	for(int i=0; i<RAMSEY_instance->PARAM_SIZE_GRAPH; i++)
	{
		cout << (int)(RAMSEY_instance->CLIQUE_BLUE[i]+0.5);
	}
	cout << endl;
	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


	if(RAMSEY_instance->size_CLIQUE_BLUE>=RAMSEY_instance->PARAM_M)
	{
		cout << "\n\n****ERROR CLIQUE BLUE SIZE****\n\n";
		SOL_OK=false;
	}


	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	for(int i=0;i<RAMSEY_instance->PARAM_SIZE_GRAPH;i++)
	{
		for(int j=i+1;j<RAMSEY_instance->PARAM_SIZE_GRAPH;j++)
		{
			if(X_SOLUTION[position_r(RAMSEY_instance,i,j)]<0.5){RAMSEY_instance->edge_fixing[i][j]=0;}
			else{RAMSEY_instance->edge_fixing[i][j]=1;}
		}
	}

	for (int i = 0; i < RAMSEY_instance->PARAM_SIZE_GRAPH; i++){RAMSEY_instance->CLIQUE_RED[i]=0;}

	clock_t time_start_red=clock();


	RAMSEY_instance->size_CLIQUE_RED=RAMSEY_instance->clique_solve_BB_edge_fixing
			(
					RAMSEY_instance->edge_fixing,
					RAMSEY_instance->CLIQUE_RED,
					0,
					RAMSEY_instance->PARAM_SIZE_GRAPH,
					0,
					0,
					0,
					0,
					1
			);
	clock_t time_end_red=clock();

	double time_red=(double)(time_end_red-time_start_red)/(double)CLOCKS_PER_SEC;



	///////////////////////////////////////////	///////////////////////////////////////////

	cout << "size_CLIQUE_RED\t" << RAMSEY_instance->size_CLIQUE_RED << "\t time \t" << time_red << endl;

	cout << "CLIQUE red\n";
	for(int i=0; i<RAMSEY_instance->PARAM_SIZE_GRAPH; i++)
	{
		cout << (int)(RAMSEY_instance->CLIQUE_RED[i]+0.5);
	}
	cout << endl;
	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


	if(RAMSEY_instance->size_CLIQUE_RED>=RAMSEY_instance->PARAM_N)
	{
		cout << "\n\n****ERROR CLIQUE RED SIZE****\n\n";
		SOL_OK=false;

	}

	if(SOL_OK)
	{
		cout << "\n\n----->>>>SOLUTION OK!!!\n";
		cout << "\n\n**************************************************************\n\n";
	}


	//////////////////////////////////////////////////////////////
	ofstream info_SUMMARY("info_CHECK_SOLUTION.txt", ios::app);
	info_SUMMARY << fixed

			<<RAMSEY_instance->size_CLIQUE_BLUE << "\t"
			<<time_blue << "\t"
			<<RAMSEY_instance->size_CLIQUE_RED << "\t"
			<<time_red << "\t"
			<< SOL_OK << "\t"

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
			<<RAMSEY_instance->ID_TEST<< "\t"	<< endl;
	info_SUMMARY.close();
	char dummy[1000];
	sprintf(dummy,"SOLUTION_FILES/ID_TEST%d_checkSOL.sol",RAMSEY_instance->ID_TEST);
	ofstream info_FILE(dummy);
	info_FILE << fixed

			<<RAMSEY_instance->size_CLIQUE_BLUE << "\t"
			<<time_blue << "\t"
			<<RAMSEY_instance->size_CLIQUE_RED << "\t"
			<<time_red << "\t"
			<< SOL_OK << "\t"

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

	if(SOL_OK)
	{
		//////////////////////////////////////////////////////////////
		draw_tex_solution_both_cliques(RAMSEY_instance,X_SOLUTION);
		//////////////////////////////////////////////////////////////

		//////////////////////////////////////////////////////////////
		draw_tex_solution_red_cliques(RAMSEY_instance,X_SOLUTION);
		//////////////////////////////////////////////////////////////

		//////////////////////////////////////////////////////////////
		draw_tex_solution_blue_cliques(RAMSEY_instance,X_SOLUTION);
		//////////////////////////////////////////////////////////////

		//////////////////////////////////////////////////////////////
		draw_tex_matrix_bis(RAMSEY_instance,X_SOLUTION);
		//////////////////////////////////////////////////////////////

	}

	return SOL_OK;

}

/***********************************************************************************/
void draw_tex_solution_both_cliques(data *RAMSEY_instance,double *X_SOLUTION)
/***********************************************************************************/
{


	ofstream compact_file;

	char dummy[10000];
	sprintf(dummy,"./figures/graphRamsey_clique_both%d_%d_%dCirc%d_br%d_id%d.tex",RAMSEY_instance->PARAM_M,RAMSEY_instance->PARAM_N,RAMSEY_instance->PARAM_SIZE_GRAPH,RAMSEY_instance->PARAM_CIRCULANT, RAMSEY_instance->BRANCHING_STRATEGY,RAMSEY_instance->ID_TEST);

	compact_file.open(dummy);

	compact_file
	<< "\\documentclass[]{article}" << "\n"
	<< "\\usepackage{tikz}" << "\n"
	<< "\\begin{document}" << "\n"
	<< "\\begin{tikzpicture}" << "\n"
	<< endl;


	double angle_increase=360.0/RAMSEY_instance->PARAM_SIZE_GRAPH;
	double angle=0;
	for(int i=0; i<RAMSEY_instance->PARAM_SIZE_GRAPH; i++)
	{
		compact_file << "\\draw (" << angle + i*angle_increase << ":5cm) coordinate (" << i << ");" << endl;
	}

	for(int i=0; i<RAMSEY_instance->PARAM_SIZE_GRAPH; i++)
	{
		for(int j=i+1; j<RAMSEY_instance->PARAM_SIZE_GRAPH; j++)
		{
			if( X_SOLUTION[position_b(RAMSEY_instance,i,j)]>0.5 )
			{

				if( RAMSEY_instance->CLIQUE_BLUE[i]>0.5 && RAMSEY_instance->CLIQUE_BLUE[j]>0.5 )
				{
					compact_file << "\\draw[line width=1.5,blue] (" << i << ") -- (" << j << ");"  << endl;
				}
				else
				{
					compact_file << "\\draw[dashed,blue] (" << i << ") -- (" << j << ");"  << endl;
				}
			}
		}
	}



	for(int i=0; i<RAMSEY_instance->PARAM_SIZE_GRAPH; i++)
	{
		for(int j=i+1; j<RAMSEY_instance->PARAM_SIZE_GRAPH; j++)
		{
			if( X_SOLUTION[position_r(RAMSEY_instance,i,j)]>0.5 )
			{
				if( RAMSEY_instance->CLIQUE_RED[i]>0.5 && RAMSEY_instance->CLIQUE_RED[j]>0.5 )
				{
					compact_file << "\\draw[line width=1.5,red] (" << i << ") -- (" << j << ");"  << endl;
				}
				else
				{
					compact_file << "\\draw[dashed,red] (" << i << ") -- (" << j << ");"  << endl;
				}
			}
		}
	}


	for(int i=0; i<RAMSEY_instance->PARAM_SIZE_GRAPH; i++)
	{
		compact_file << "\\draw[circle,fill=white,minimum size=19pt,inner sep=0pt] (" << i << ") circle (4pt) node[] {};" << endl;
		//compact_file << "\\draw[circle,fill=white,minimum size=19pt,inner sep=0pt] (" << i << ") circle (7.5pt) node[label=right: \Huge ${"<< i <<"}$] {};" << endl;
	}


	compact_file
	<< "\\end{tikzpicture}" << "\n"
	<< "\\end{document}" << "\n"
	<< "\\begin{document}" << "\n"
	<< endl;


	compact_file.close();

}

/***********************************************************************************/
void draw_tex_solution_blue_cliques(data *RAMSEY_instance,double *X_SOLUTION)
/***********************************************************************************/
{


	ofstream compact_file;

	char dummy[10000];
	sprintf(dummy,"./figures/graphRamsey_clique_both%d_%d_%dCirc%d_br%d_id%d_red.tex",RAMSEY_instance->PARAM_M,RAMSEY_instance->PARAM_N,RAMSEY_instance->PARAM_SIZE_GRAPH,RAMSEY_instance->PARAM_CIRCULANT, RAMSEY_instance->BRANCHING_STRATEGY,RAMSEY_instance->ID_TEST);

	compact_file.open(dummy);

	compact_file
	<< "\\documentclass[]{article}" << "\n"
	<< "\\usepackage{tikz}" << "\n"
	<< "\\begin{document}" << "\n"
	<< "\\begin{tikzpicture}" << "\n"
	<< endl;


	double angle_increase=360.0/RAMSEY_instance->PARAM_SIZE_GRAPH;
	double angle=0;
	for(int i=0; i<RAMSEY_instance->PARAM_SIZE_GRAPH; i++)
	{
		compact_file << "\\draw (" << angle + i*angle_increase << ":5cm) coordinate (" << i << ");" << endl;
	}


	for(int i=0; i<RAMSEY_instance->PARAM_SIZE_GRAPH; i++)
	{
		for(int j=i+1; j<RAMSEY_instance->PARAM_SIZE_GRAPH; j++)
		{
			if( X_SOLUTION[position_r(RAMSEY_instance,i,j)]>0.5 )
			{
				if( RAMSEY_instance->CLIQUE_RED[i]>0.5 && RAMSEY_instance->CLIQUE_RED[j]>0.5 )
				{
					compact_file << "\\draw[line width=1.5,red] (" << i << ") -- (" << j << ");"  << endl;
				}
				else
				{
					compact_file << "\\draw[dashed,red] (" << i << ") -- (" << j << ");"  << endl;
				}
			}
		}
	}


	for(int i=0; i<RAMSEY_instance->PARAM_SIZE_GRAPH; i++)
	{
		compact_file << "\\draw[circle,fill=white,minimum size=19pt,inner sep=0pt] (" << i << ") circle (4pt) node[] {};" << endl;
		//compact_file << "\\draw[circle,fill=white,minimum size=19pt,inner sep=0pt] (" << i << ") circle (7.5pt) node[label=right: \\Huge ${"<< i <<"}$] {};" << endl;

	}


	compact_file
	<< "\\end{tikzpicture}" << "\n"
	<< "\\end{document}" << "\n"
	<< "\\begin{document}" << "\n"
	<< endl;


	compact_file.close();

}

/***********************************************************************************/
void draw_tex_solution_red_cliques(data *RAMSEY_instance,double *X_SOLUTION)
/***********************************************************************************/
{


	ofstream compact_file;

	char dummy[10000];
	sprintf(dummy,"./figures/graphRamsey_clique_both%d_%d_%dCirc%d_br%d_id%d_blue.tex",RAMSEY_instance->PARAM_M,RAMSEY_instance->PARAM_N,RAMSEY_instance->PARAM_SIZE_GRAPH,RAMSEY_instance->PARAM_CIRCULANT, RAMSEY_instance->BRANCHING_STRATEGY, RAMSEY_instance->ID_TEST);

	compact_file.open(dummy);

	compact_file
	<< "\\documentclass[]{article}" << "\n"
	<< "\\usepackage{tikz}" << "\n"
	<< "\\begin{document}" << "\n"
	<< "\\begin{tikzpicture}" << "\n"
	<< endl;


	double angle_increase=360.0/RAMSEY_instance->PARAM_SIZE_GRAPH;
	double angle=0;
	for(int i=0; i<RAMSEY_instance->PARAM_SIZE_GRAPH; i++)
	{
		compact_file << "\\draw (" << angle + i*angle_increase << ":5cm) coordinate (" << i << ");" << endl;
	}

	for(int i=0; i<RAMSEY_instance->PARAM_SIZE_GRAPH; i++)
	{
		for(int j=i+1; j<RAMSEY_instance->PARAM_SIZE_GRAPH; j++)
		{
			if( X_SOLUTION[position_b(RAMSEY_instance,i,j)]>0.5 )
			{

				if( RAMSEY_instance->CLIQUE_BLUE[i]>0.5 && RAMSEY_instance->CLIQUE_BLUE[j]>0.5 )
				{
					compact_file << "\\draw[line width=1.5,blue] (" << i << ") -- (" << j << ");"  << endl;
				}
				else
				{
					compact_file << "\\draw[dashed,blue] (" << i << ") -- (" << j << ");"  << endl;
				}
			}
		}
	}



	for(int i=0; i<RAMSEY_instance->PARAM_SIZE_GRAPH; i++)
	{
		compact_file << "\\draw[circle,fill=white,minimum size=19pt,inner sep=0pt] (" << i << ") circle (4pt) node[] {};" << endl;
		//compact_file << "\\draw[circle,fill=white,minimum size=19pt,inner sep=0pt] (" << i << ") circle (7.5pt) node[label=right: \\Huge ${"<< i <<"}$] {};" << endl;
	}


	compact_file
	<< "\\end{tikzpicture}" << "\n"
	<< "\\end{document}" << "\n"
	<< "\\begin{document}" << "\n"
	<< endl;


	compact_file.close();




}


/***********************************************************************************/
void draw_tex_matrix_bis(data *RAMSEY_instance,double *X_SOLUTION)
/***********************************************************************************/
{


	ofstream compact_file;

	char dummy[10000];
	sprintf(dummy,"./matrices/graphRamsey%d_%d_%d_matrix_bisCirc%d_br%d_id%d.tex",RAMSEY_instance->PARAM_M,RAMSEY_instance->PARAM_N,RAMSEY_instance->PARAM_SIZE_GRAPH,RAMSEY_instance->PARAM_CIRCULANT, RAMSEY_instance->BRANCHING_STRATEGY, RAMSEY_instance->ID_TEST);

	compact_file.open(dummy);

	compact_file
	<< "\\documentclass[]{article}" << "\n"
	//<< "\\usepackage{tikz}" << "\n"
	<< "\\usepackage{amsmath}" << "\n"
	<< "\\usepackage[table]{xcolor}" << "\n"


	<< "\\usepackage{mathtools}" << "\n"
	<< "\\usepackage{empheq}" << "\n"
	<< "\\usepackage[most]{tcolorbox}" << "\n"

	<< "\\usepackage{tabularx}" << "\n"
	<< "\\usepackage{booktabs}" << "\n"

	<< "\\usepackage{fancybox}" << "\n"

	<< "\\begin{document}" << "\n"

	<< "\\newcommand\\bb{\\cellcolor{blue!40}}" << "\n"
	<< "\\newcommand\\rr{\\cellcolor{red!40}}" << "\n"

	<< "\\newcommand\\bbb{\\cellcolor{blue!90}}" << "\n"
	<< "\\newcommand\\rrr{\\cellcolor{red!90}}" << "\n"

	<< "\\begin{align*}" << "\n"

	//	<< "\\resizebox{.8\\textwidth}{!}{" << "\n"
	<< "\\resizebox{\\textwidth}{!}{" << "\n"

	<< "\\cornersize{.01}" << "\n"
	<< "\\ovalbox{" << "\n"

	<< "\\renewcommand{\\arraystretch}{1.2}" << "\n"


	//<< "\\left("

	<<	"\\begin{tabular}{";

	for(int i=0; i<RAMSEY_instance->PARAM_SIZE_GRAPH+10; i++)
	{
		compact_file << "p{0.05\\textwidth}";
	}

	compact_file << "}" << "\n";

	//	for(int i=0; i<RAMSEY_instance->PARAM_SIZE_GRAPH; i++)
	//	{
	//		compact_file <<  i+1 ;
	//
	//		if(i != RAMSEY_instance->PARAM_SIZE_GRAPH-1)
	//			compact_file <<  "&";
	//	}
	//	compact_file << "\\\\ \n";

	compact_file << " &";

	for(int i=0; i<RAMSEY_instance->PARAM_SIZE_GRAPH; i++)
	{
		compact_file << "\\LARGE ${ " << i << "}$";

		if(i!=RAMSEY_instance->PARAM_SIZE_GRAPH-1)
		{
			compact_file << " &";
		}
	}
	compact_file << " &";

	compact_file << "\\\\ \n";

	compact_file << "\\cmidrule{2-"<< RAMSEY_instance->PARAM_SIZE_GRAPH +1 <<"}\n";

	for(int i=0; i<RAMSEY_instance->PARAM_SIZE_GRAPH; i++)
	{
		compact_file << "\\LARGE ${ " << i << "}$ &";
		//compact_file << " &";

		for(int j=0; j<RAMSEY_instance->PARAM_SIZE_GRAPH; j++)
		{
			if(j>i)
			{
				if( X_SOLUTION[position_b(RAMSEY_instance,i,j)]>0.5 )
				{
					if( (int)(RAMSEY_instance->CLIQUE_BLUE[i]+0.5)==1 &&  (int)(RAMSEY_instance->CLIQUE_BLUE[j]+0.5)==1 )
					{
						compact_file << " \\multicolumn{1}{|c|}{ \\bbb }";
					}
					else
					{
						compact_file << " \\multicolumn{1}{|c|}{ \\bb }";
					}
				}
				else
				{
					if( (int)(RAMSEY_instance->CLIQUE_RED[i]+0.5)==1 &&  (int)(RAMSEY_instance->CLIQUE_RED[j]+0.5)==1 )
					{
						compact_file << " \\multicolumn{1}{|c|}{  \\rrr }";
					}
					else
					{
						compact_file << " \\multicolumn{1}{|c|}{  \\rr }";

					}
				}
			}

			if(j<i)
			{
				if( X_SOLUTION[position_b(RAMSEY_instance,j,i)]>0.5 )
				{
					if( (int)(RAMSEY_instance->CLIQUE_BLUE[i]+0.5)==1 &&  (int)(RAMSEY_instance->CLIQUE_BLUE[j]+0.5)==1 )
					{
						compact_file << " \\multicolumn{1}{|c|}{ \\bbb }";
					}
					else
					{
						compact_file << " \\multicolumn{1}{|c|}{ \\bb }";
					}
				}
				else
				{
					if( (int)(RAMSEY_instance->CLIQUE_RED[i]+0.5)==1 &&  (int)(RAMSEY_instance->CLIQUE_RED[j]+0.5)==1 )
					{
						compact_file << " \\multicolumn{1}{|c|}{  \\rrr }";
					}
					else
					{
						compact_file << " \\multicolumn{1}{|c|}{  \\rr }";

					}
				}
			}


			if(j!=RAMSEY_instance->PARAM_SIZE_GRAPH-1)
			{
				compact_file << " &";
			}

		}
		compact_file << " &";
		compact_file << "\\\\ \n";
		compact_file << "\\cmidrule{2-"<< RAMSEY_instance->PARAM_SIZE_GRAPH +1 <<"}\n";
	}

	compact_file << "\\\\ \n";

	compact_file
	<< "\\end{tabular}"
	//<< "\\right)"
	<< "}" << "\n"
	<< "}" << "\n"
	<< "\\end{align*}" << "\n"
	<< "\\end{document}" << "\n"
	<< "\\begin{document}" << "\n"
	<< endl;



	compact_file.close();
}

/***********************************************************************************/
void write_row_sol(data *RAMSEY_instance,double *X_SOLUTION)
/***********************************************************************************/
{


	ofstream compact_file;

	char dummy[10000];
	sprintf(dummy,"./matrices/graphRamsey%d_%d_%d_matrix_rowCirc%d_br%d_id%d.tex",RAMSEY_instance->PARAM_M,RAMSEY_instance->PARAM_N,RAMSEY_instance->PARAM_SIZE_GRAPH,RAMSEY_instance->PARAM_CIRCULANT,RAMSEY_instance->BRANCHING_STRATEGY, RAMSEY_instance->ID_TEST);



	compact_file.open(dummy);

	compact_file
	<< "\\documentclass[]{article}" << "\n"
	//<< "\\usepackage{tikz}" << "\n"
	<< "\\usepackage{amsmath}" << "\n"
	<< "\\usepackage[table]{xcolor}" << "\n"


	<< "\\usepackage{mathtools}" << "\n"
	<< "\\usepackage{empheq}" << "\n"
	<< "\\usepackage[most]{tcolorbox}" << "\n"

	<< "\\usepackage{tabularx}" << "\n"
	<< "\\usepackage{booktabs}" << "\n"

	<< "\\usepackage{fancybox}" << "\n"

	<< "\\begin{document}" << "\n"

	<< "\\newcommand\\bb{\\cellcolor{blue!40}}" << "\n"
	<< "\\newcommand\\rr{\\cellcolor{red!40}}" << "\n"

	<< "\\newcommand\\bbb{\\cellcolor{blue!90}}" << "\n"
	<< "\\newcommand\\rrr{\\cellcolor{red!90}}" << "\n"

	<< "\\begin{align*}" << "\n"

	//	<< "\\resizebox{.8\\textwidth}{!}{" << "\n"
	<< "\\resizebox{\\textwidth}{!}{" << "\n"

	<< "\\cornersize{.01}" << "\n"
	<< "\\ovalbox{" << "\n"

	<< "\\renewcommand{\\arraystretch}{1.2}" << "\n"


	//<< "\\left("

	<<	"\\begin{tabular}{";



	int pos=1;

	int size=((RAMSEY_instance->PARAM_SIZE_GRAPH)/2.0) + 1;


	int max_size=40;

	for(int i=0; i<max_size+10; i++)
	{
		compact_file << "p{0.085\\textwidth}";
	}

	compact_file << "}" << "\n";


	while(1){

		for(int i=0; i<max_size; i++)
		{
			compact_file << "\\Huge $" << pos+i << "$";

			compact_file << " &";
			if(pos+1+i>=size){break;}

		}
		compact_file << " &";

		compact_file << "\\\\";
		compact_file << "\\\\";
		compact_file << "\n";


		for(int i=0; i<max_size; i++)
		{

			if( (int) (X_SOLUTION[position_b(RAMSEY_instance,0,pos++)] +0.5)==1 )
			{
				compact_file << " \\multicolumn{1}{|c|}{ \\bb }";
			}
			else
			{
				compact_file << " \\multicolumn{1}{|c|}{ \\rr }";
			}

			compact_file << " &";


			if(pos>=size){break;}

		}

		compact_file << "\\\\";
		compact_file << "\\\\";
		compact_file << "\n";


		if(pos>=size){break;}

	}


	compact_file
	<< "\\end{tabular}"
	//<< "\\right)"
	<< "}" << "\n"
	<< "}" << "\n"
	<< "\\end{align*}" << "\n"
	<< "\\end{document}" << "\n"
	<< "\\begin{document}" << "\n"
	<< endl;


	compact_file.close();





}
