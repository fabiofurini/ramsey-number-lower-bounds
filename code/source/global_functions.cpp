

#include "global_functions.h"





// The main function that prints all combinations of
// size r in arr[] of size n. This function mainly
// uses combinationUtil()
/***********************************************************************************/
void printCombination(int arr[], int n, int r)
/***********************************************************************************/
{

	// A temporary array to store all combination
	// one by one
	int data[r];

	// Print all combination using temporary array 'data[]'
	combinationUtil(arr, n, r, 0, data, 0);
}

/* arr[]  ---> Input Array
   n      ---> Size of input array
   r      ---> Size of a combination to be printed
   index  ---> Current index in data[]
   data[] ---> Temporary array to store current combination
   i      ---> index of current element in arr[]     */

/***********************************************************************************/
void combinationUtil(int arr[], int n, int r, int index, int data[], int i)
/***********************************************************************************/
{
	// Current combination is ready, print it
	if (index == r) {
		for (int j = 0; j < r; j++)
			cout <<" "<< data[j];
		cout <<"\n";
		return;
	}

	// When no more elements are there to put in data[]
	if (i >= n)
		return;

	// current is included, put next at next location
	data[index] = arr[i];
	combinationUtil(arr, n, r, index + 1, data, i + 1);

	// current is excluded, replace it with next
	// (Note that i+1 is passed, but index is not changed)
	combinationUtil(arr, n, r, index, data, i + 1);
}




/***********************************************************************************/
int modulo(data *RAMSEY_instance,int k)
/***********************************************************************************/
// the function returns k mod q in the interval {0,...,q-1}
{

	int a = k%RAMSEY_instance->PARAM_SIZE_GRAPH >= 0 ? k%RAMSEY_instance->PARAM_SIZE_GRAPH : k%RAMSEY_instance->PARAM_SIZE_GRAPH + RAMSEY_instance->PARAM_SIZE_GRAPH;

	return a;
}



/***********************************************************************************/
int mapping(data *RAMSEY_instance,int i,int j)
/***********************************************************************************/
{

	if(j==i)
	{
		return -1;
	}

	int a = j-i % RAMSEY_instance->PARAM_SIZE_GRAPH >=0 ? j-i % RAMSEY_instance->PARAM_SIZE_GRAPH : j-i % RAMSEY_instance->PARAM_SIZE_GRAPH + RAMSEY_instance->PARAM_SIZE_GRAPH;
	int b = i-j % RAMSEY_instance->PARAM_SIZE_GRAPH >=0 ? i-j % RAMSEY_instance->PARAM_SIZE_GRAPH : i-j % RAMSEY_instance->PARAM_SIZE_GRAPH + RAMSEY_instance->PARAM_SIZE_GRAPH;
	return min(a, b) - 1;
}


/***********************************************************************************/
int mapping_h(data *RAMSEY_instance,int ktemp)
/***********************************************************************************/
{

	int k=ktemp;
	int kp = RAMSEY_instance->PARAM_SIZE_GRAPH - k;
	// fixed  according to k = min { i+j mod q, q - (i+j) mod q}
	int a = k%RAMSEY_instance->PARAM_SIZE_GRAPH >= 0 ? k%RAMSEY_instance->PARAM_SIZE_GRAPH : k%RAMSEY_instance->PARAM_SIZE_GRAPH + RAMSEY_instance->PARAM_SIZE_GRAPH;
	int b = kp%RAMSEY_instance->PARAM_SIZE_GRAPH >= 0 ? kp%RAMSEY_instance->PARAM_SIZE_GRAPH : kp%RAMSEY_instance->PARAM_SIZE_GRAPH + RAMSEY_instance->PARAM_SIZE_GRAPH;

	//    cout << "k\t" << k << "\tkp\t" << kp << "\ta\t" << a << "\tb\t" << b << endl;
	k = min(a,b);

	return k;
}



/***********************************************************************************/
int modulo(int a,int b)
/***********************************************************************************/
{
	return (a % b + b) % b;
}

/***********************************************************************************/
int compare_non_increasing(const void *p, const void *q)
/***********************************************************************************/
{
	double l = ((struct object *)p)->score;
	double r = ((struct object *)q)->score;
	return ((l < r)  ? 1:-1);
}

/***********************************************************************************/
int compare_non_decreasing(const void *p, const void *q)
/***********************************************************************************/
{
	double l = ((struct object *)p)->score;
	double r = ((struct object *)q)->score;
	return ((l > r)  ? 1:-1);
}

/***********************************************************************************/
void FILL_RAMSEY_LOOK_UP(data *RAMSEY_instance)
/***********************************************************************************/
{

	RAMSEY_instance->LOOK_UP_RAMSEY=new int*[50];
	for(int i=0; i<50; i++)
	{
		RAMSEY_instance->LOOK_UP_RAMSEY[i]=new int[50];
	}

	for(int i=0; i<50; i++)
	{
		for(int j=0; j<50; j++)
		{
			RAMSEY_instance->LOOK_UP_RAMSEY[i][j]=-1;
		}
	}


	for(int i=1; i<50; i++)
	{
		RAMSEY_instance->LOOK_UP_RAMSEY[1][i]=1;
		RAMSEY_instance->LOOK_UP_RAMSEY[2][i]=i;
	}


	// Upper bounds updated from Radziszowski, Small Ramsey Numbers, DS1 revision #18 (2026), Table Ia/Ib.
	RAMSEY_instance->LOOK_UP_RAMSEY[3][3]=6;//opt
	RAMSEY_instance->LOOK_UP_RAMSEY[3][4]=9;//opt
	RAMSEY_instance->LOOK_UP_RAMSEY[3][5]=14;//opt
	RAMSEY_instance->LOOK_UP_RAMSEY[3][6]=18;//opt
	RAMSEY_instance->LOOK_UP_RAMSEY[3][7]=23;//opt
	RAMSEY_instance->LOOK_UP_RAMSEY[3][8]=28;//opt
	RAMSEY_instance->LOOK_UP_RAMSEY[3][9]=36;//opt

	RAMSEY_instance->LOOK_UP_RAMSEY[3][10]=41;//UB
	RAMSEY_instance->LOOK_UP_RAMSEY[3][11]=50;//UB
	RAMSEY_instance->LOOK_UP_RAMSEY[3][12]=59;//UB
	RAMSEY_instance->LOOK_UP_RAMSEY[3][13]=68;//UB
	RAMSEY_instance->LOOK_UP_RAMSEY[3][14]=77;//UB
	RAMSEY_instance->LOOK_UP_RAMSEY[3][15]=87;//UB
	RAMSEY_instance->LOOK_UP_RAMSEY[3][16]=97;//UB
	RAMSEY_instance->LOOK_UP_RAMSEY[3][17]=109;//UB
	RAMSEY_instance->LOOK_UP_RAMSEY[3][18]=120;//UB
	RAMSEY_instance->LOOK_UP_RAMSEY[3][19]=132;//UB
	RAMSEY_instance->LOOK_UP_RAMSEY[3][20]=145;//UB
	RAMSEY_instance->LOOK_UP_RAMSEY[3][21]=157;//UB
	RAMSEY_instance->LOOK_UP_RAMSEY[3][22]=171;//UB
	RAMSEY_instance->LOOK_UP_RAMSEY[3][23]=185;//UB

	RAMSEY_instance->LOOK_UP_RAMSEY[4][4]=18;//opt
	RAMSEY_instance->LOOK_UP_RAMSEY[4][5]=25;//opt
	RAMSEY_instance->LOOK_UP_RAMSEY[4][6]=40;//UB
	RAMSEY_instance->LOOK_UP_RAMSEY[4][7]=58;//UB
	RAMSEY_instance->LOOK_UP_RAMSEY[4][8]=79;//UB

	RAMSEY_instance->LOOK_UP_RAMSEY[4][9]=105;//UB
	RAMSEY_instance->LOOK_UP_RAMSEY[4][10]=135;//UB
	RAMSEY_instance->LOOK_UP_RAMSEY[4][11]=170;//UB
	RAMSEY_instance->LOOK_UP_RAMSEY[4][12]=210;//UB
	RAMSEY_instance->LOOK_UP_RAMSEY[4][13]=256;//UB
	RAMSEY_instance->LOOK_UP_RAMSEY[4][14]=307;//UB
	RAMSEY_instance->LOOK_UP_RAMSEY[4][15]=364;//UB


	RAMSEY_instance->LOOK_UP_RAMSEY[5][5]=46;//UB
	RAMSEY_instance->LOOK_UP_RAMSEY[5][6]=85;//UB
	RAMSEY_instance->LOOK_UP_RAMSEY[5][7]=133;//UB
	RAMSEY_instance->LOOK_UP_RAMSEY[5][8]=193;//UB
	RAMSEY_instance->LOOK_UP_RAMSEY[5][9]=282;//UB
	RAMSEY_instance->LOOK_UP_RAMSEY[5][10]=381;//UB
	RAMSEY_instance->LOOK_UP_RAMSEY[5][11]=511;//UB


	RAMSEY_instance->LOOK_UP_RAMSEY[6][6]=160;//UB
	RAMSEY_instance->LOOK_UP_RAMSEY[6][7]=270;//UB
	RAMSEY_instance->LOOK_UP_RAMSEY[6][8]=423;//UB
	RAMSEY_instance->LOOK_UP_RAMSEY[6][9]=651;//UB


	RAMSEY_instance->LOOK_UP_RAMSEY[7][7]=492;//UB

	// Lower bounds updated from Radziszowski, Small Ramsey Numbers, DS1 revision #18 (2026), Tables Ia/IIa/IIb.
	if(RAMSEY_instance->PARAM_K_CUTS==2)
	{
		RAMSEY_instance->LOOK_UP_RAMSEY[3][10]=40;//LB
		RAMSEY_instance->LOOK_UP_RAMSEY[3][11]=47;//LB
		RAMSEY_instance->LOOK_UP_RAMSEY[3][12]=53;//LB
		RAMSEY_instance->LOOK_UP_RAMSEY[3][13]=61;//LB
		RAMSEY_instance->LOOK_UP_RAMSEY[3][14]=67;//LB
		RAMSEY_instance->LOOK_UP_RAMSEY[3][15]=74;//LB
		RAMSEY_instance->LOOK_UP_RAMSEY[3][16]=82;//LB
		RAMSEY_instance->LOOK_UP_RAMSEY[3][17]=92;//LB
		RAMSEY_instance->LOOK_UP_RAMSEY[3][18]=100;//LB
		RAMSEY_instance->LOOK_UP_RAMSEY[3][19]=106;//LB
		RAMSEY_instance->LOOK_UP_RAMSEY[3][20]=111;//LB
		RAMSEY_instance->LOOK_UP_RAMSEY[3][21]=122;//LB
		RAMSEY_instance->LOOK_UP_RAMSEY[3][22]=131;//LB
		RAMSEY_instance->LOOK_UP_RAMSEY[3][23]=139;//LB

		RAMSEY_instance->LOOK_UP_RAMSEY[3][24]=143;//LB
		RAMSEY_instance->LOOK_UP_RAMSEY[3][25]=154;//LB
		RAMSEY_instance->LOOK_UP_RAMSEY[3][26]=161;//LB
		RAMSEY_instance->LOOK_UP_RAMSEY[3][27]=172;//LB
		RAMSEY_instance->LOOK_UP_RAMSEY[3][28]=179;//LB
		RAMSEY_instance->LOOK_UP_RAMSEY[3][29]=190;//LB
		RAMSEY_instance->LOOK_UP_RAMSEY[3][30]=197;//LB
		RAMSEY_instance->LOOK_UP_RAMSEY[3][31]=208;//LB
		RAMSEY_instance->LOOK_UP_RAMSEY[3][32]=217;//LB
		RAMSEY_instance->LOOK_UP_RAMSEY[3][33]=227;//LB
		RAMSEY_instance->LOOK_UP_RAMSEY[3][34]=234;//LB
		RAMSEY_instance->LOOK_UP_RAMSEY[3][35]=248;//LB
		RAMSEY_instance->LOOK_UP_RAMSEY[3][36]=255;//LB
		RAMSEY_instance->LOOK_UP_RAMSEY[3][37]=267;//LB
		RAMSEY_instance->LOOK_UP_RAMSEY[3][38]=278;//LB
		RAMSEY_instance->LOOK_UP_RAMSEY[3][39]=290;//LB
		RAMSEY_instance->LOOK_UP_RAMSEY[3][40]=298;//LB
		RAMSEY_instance->LOOK_UP_RAMSEY[3][41]=311;//LB
		RAMSEY_instance->LOOK_UP_RAMSEY[3][42]=320;//LB
		RAMSEY_instance->LOOK_UP_RAMSEY[3][43]=333;//LB
		RAMSEY_instance->LOOK_UP_RAMSEY[3][44]=339;//LB
		RAMSEY_instance->LOOK_UP_RAMSEY[3][45]=354;//LB
		RAMSEY_instance->LOOK_UP_RAMSEY[3][46]=362;//LB


		RAMSEY_instance->LOOK_UP_RAMSEY[4][6]=36;//LB
		RAMSEY_instance->LOOK_UP_RAMSEY[4][7]=49;//LB
		RAMSEY_instance->LOOK_UP_RAMSEY[4][8]=59;//LB
		RAMSEY_instance->LOOK_UP_RAMSEY[4][9]=73;//LB
		RAMSEY_instance->LOOK_UP_RAMSEY[4][10]=92;//LB
		RAMSEY_instance->LOOK_UP_RAMSEY[4][11]=102;//LB
		RAMSEY_instance->LOOK_UP_RAMSEY[4][12]=128;//LB
		RAMSEY_instance->LOOK_UP_RAMSEY[4][13]=139;//LB
		RAMSEY_instance->LOOK_UP_RAMSEY[4][14]=148;//LB
		RAMSEY_instance->LOOK_UP_RAMSEY[4][15]=159;//LB


		RAMSEY_instance->LOOK_UP_RAMSEY[5][5]=43;//LB
		RAMSEY_instance->LOOK_UP_RAMSEY[5][6]=59;//LB
		RAMSEY_instance->LOOK_UP_RAMSEY[5][7]=80;//LB
		RAMSEY_instance->LOOK_UP_RAMSEY[5][8]=101;//LB
		RAMSEY_instance->LOOK_UP_RAMSEY[5][9]=133;//LB
		RAMSEY_instance->LOOK_UP_RAMSEY[5][10]=149;//LB
		RAMSEY_instance->LOOK_UP_RAMSEY[5][11]=183;//LB

		RAMSEY_instance->LOOK_UP_RAMSEY[6][6]=102;//LB
		RAMSEY_instance->LOOK_UP_RAMSEY[7][7]=205;//LB
	}


	for(int i=0; i<50; i++)
	{
		for(int j=i+1; j<50; j++)
		{
			RAMSEY_instance->LOOK_UP_RAMSEY[j][i]=RAMSEY_instance->LOOK_UP_RAMSEY[i][j];
		}
	}


	//		for(int i=1; i<11; i++)
	//		{
	//			for(int j=1; j<11; j++)
	//			{
	//				cout << RAMSEY_instance->LOOK_UP_RAMSEY[i][j] << "\t";
	//			}
	//			cout << endl;
	//		}
	//		cout << endl;


}

/***********************************************************************************/
void memory_allocation(data *RAMSEY_instance)
/***********************************************************************************/
{

	RAMSEY_instance->subset=new int[RAMSEY_instance->PARAM_SIZE_GRAPH];
	RAMSEY_instance->data_subset=new int[RAMSEY_instance->PARAM_SIZE_GRAPH];


	RAMSEY_instance->CLIQUE_SOL=new double[RAMSEY_instance->PARAM_SIZE_GRAPH];
	RAMSEY_instance->CLIQUE_SOL_TEMP=new double[RAMSEY_instance->PARAM_SIZE_GRAPH];

	RAMSEY_instance->edge_fixing=new int*[RAMSEY_instance->PARAM_SIZE_GRAPH];

	for (int i = 0; i < RAMSEY_instance->PARAM_SIZE_GRAPH; i++)
	{
		RAMSEY_instance->edge_fixing[i]=new int[RAMSEY_instance->PARAM_SIZE_GRAPH];
	}


	RAMSEY_instance->CLIQUE_BLUE=new double[RAMSEY_instance->PARAM_SIZE_GRAPH];
	RAMSEY_instance->CLIQUE_RED=new double[RAMSEY_instance->PARAM_SIZE_GRAPH];


	RAMSEY_instance->edge_fixing_TEMP=new int*[RAMSEY_instance->PARAM_SIZE_GRAPH];
	for (int i = 0; i < RAMSEY_instance->PARAM_SIZE_GRAPH; i++)
	{
		RAMSEY_instance->edge_fixing_TEMP[i]=new int[RAMSEY_instance->PARAM_SIZE_GRAPH];
	}


}

/***********************************************************************************/
void memory_deallocation(data *RAMSEY_instance)
/***********************************************************************************/
{
	delete []RAMSEY_instance->subset;
	delete []RAMSEY_instance->data_subset;

	for (int i = 0; i < RAMSEY_instance->PARAM_SIZE_GRAPH; i++)
	{
		delete [] RAMSEY_instance->edge_fixing[i];
	}
	delete [] 	RAMSEY_instance->edge_fixing;

	for (int i = 0; i < RAMSEY_instance->PARAM_SIZE_GRAPH; i++)
	{
		delete [] RAMSEY_instance->edge_fixing_TEMP[i];
	}
	delete [] 	RAMSEY_instance->edge_fixing_TEMP;

	delete []RAMSEY_instance->CLIQUE_SOL;
	delete []RAMSEY_instance->CLIQUE_SOL_TEMP;

	delete []RAMSEY_instance->CLIQUE_BLUE;
	delete []RAMSEY_instance->CLIQUE_RED;

	// Free cuts memory
	free_cuts_data(&RAMSEY_instance->CUTS_M);
	free_cuts_data(&RAMSEY_instance->CUTS_N);

}


/***********************************************************************************/
int position_r(data *RAMSEY_instance, int i,int j)
/***********************************************************************************/
{
	return  RAMSEY_instance->PARAM_SIZE_GRAPH*i+j;
}

/***********************************************************************************/
int position_b(data *RAMSEY_instance, int i, int j)
/***********************************************************************************/
{
	return RAMSEY_instance->PARAM_SIZE_GRAPH*RAMSEY_instance->PARAM_SIZE_GRAPH +RAMSEY_instance->PARAM_SIZE_GRAPH*i+j;
}

/***********************************************************************************/
int position_theta(data *RAMSEY_instance)
/***********************************************************************************/
{
	return 2*RAMSEY_instance->PARAM_SIZE_GRAPH*RAMSEY_instance->PARAM_SIZE_GRAPH;
}


/***********************************************************************************/
int magic_formula(int K,int l)
/***********************************************************************************/
{

	//K is the size of clique size, l-1 is the target clique size
	int m=l-1;
	int alpha=(K/m)+1;
	int nalpha_1=m*alpha-K;
	int nalpha=(K-((nalpha_1-1)*(alpha-1)))/alpha;

	//	cout << "alpha\t"<< alpha << endl;
	//	cout << "n(alpha-1)\t"<<  nalpha_1  << endl;
	//	cout << "n(alpha)\t"<<  nalpha  << endl;
	//	cout << "magic function value\t" <<  nalpha_1*(alpha-1)*(alpha-2)/2+nalpha*(alpha-1)*(alpha)/2 << endl;

	return nalpha_1*(alpha-1)*(alpha-2)/2+nalpha*(alpha-1)*(alpha)/2;

}

/***********************************************************************************/
int magic_formula_bis(int K,int l)
/***********************************************************************************/
{

	double turan_exact = l*(l-1)/2.0 - 1;

	for (int i = 0; i < K-l; i++)
	{
		turan_exact = floor(turan_exact * (l+1+i)/(l-1+i));
	}

	return K*(K-1)/2 - turan_exact;
}


/***********************************************************************************/
int magic_formula_tris(int K,int l)
/***********************************************************************************/
{

	int value = floor( (1.0 - 1.0 / (l - 1)) * (K*K) / 2.0 ) ;

	return  K*(K-1)/2 - value;
}

/**********************************************************************************/
int randNum(int min,  int max)
/***********************************************************************************/
{
	return  rand()%(max-min + 1) + min;

}

/***********************************************************************************/
string cut_file_name(data *RAMSEY_instance, bool blue)
/***********************************************************************************/
{
	stringstream filename_stream;
	filename_stream << "CUTS/t" << RAMSEY_instance->PARAM_SIZE_GRAPH
				<< "_m" << RAMSEY_instance->PARAM_M
				<< "_n" << RAMSEY_instance->PARAM_N
				<< "_id" << RAMSEY_instance->ID_TEST
				<< (blue ? "_blue.txt" : "_red.txt");
	return filename_stream.str();
}

/***********************************************************************************/
bool load_cuts_from_file(data *RAMSEY_instance, bool blue, cut_data *cuts, bool print_cuts)
/***********************************************************************************/
{
	const string filename = cut_file_name(RAMSEY_instance, blue);
	
	cout << "\nTrying to load cuts from file: " << filename << endl;
	
	// Open the file
	ifstream file(filename.c_str());
	if (!file.is_open())
	{
		// Preserve compatibility with the cut files that predate experiment IDs.
		stringstream legacy_filename;
		legacy_filename << "CUTS/t" << RAMSEY_instance->PARAM_SIZE_GRAPH
						<< "_k" << (blue ? RAMSEY_instance->PARAM_M : RAMSEY_instance->PARAM_N) << ".txt";
		file.open(legacy_filename.str().c_str());
		if (!file.is_open())
		{
			cout << "File not found: " << filename << endl;
			cuts->loaded = false;
			cuts->num_lines = 0;
			cuts->lines = NULL;
			cuts->has_clique_data = false;
			return false;
		}
		cout << "Using legacy cut file: " << legacy_filename.str() << endl;
	}
	
	vector<vector<int> > temp_cuts;
	vector<vector<int> > temp_cliques;
	string line;
	bool first_nonempty_line = true;
	cuts->has_clique_data = false;
	
	while (getline(file, line))
	{
		if (line.empty()) continue;
		if (first_nonempty_line && line == "# RAMSEY_DISTANCE_CLIQUE_V1")
		{
			first_nonempty_line = false;
			continue;
		}
		first_nonempty_line = false;
		
		vector<int> row, clique;
		const size_t delimiter = line.find('|');
		const string distance_part = line.substr(0, delimiter);
		const string clique_part = delimiter == string::npos ? string() : line.substr(delimiter + 1);
		istringstream iss(distance_part);
		int value;
		while (iss >> value)
		{
			row.push_back(value);
		}
		
		if (!row.empty())
		{
			temp_cuts.push_back(row);
			if (delimiter != string::npos)
			{
				istringstream clique_stream(clique_part);
				while (clique_stream >> value) clique.push_back(value);
				if (clique.empty())
				{
					cout << "Missing clique data in " << filename << endl;
					return false;
				}
				cuts->has_clique_data = true;
			}
			temp_cliques.push_back(clique);
		}
	}
	
	file.close();
	
	// Allocate the structure
	cuts->num_lines = temp_cuts.size();
	cuts->lines = new cut_line[cuts->num_lines];
	
	// Copy the data
	for (int i = 0; i < cuts->num_lines; i++)
	{
		cuts->lines[i].num_elements = temp_cuts[i].size();
		cuts->lines[i].values = new int[cuts->lines[i].num_elements];
		cuts->lines[i].clique_num_elements = 0;
		cuts->lines[i].clique_values = NULL;
		
		for (int j = 0; j < cuts->lines[i].num_elements; j++)
		{
			cuts->lines[i].values[j] = temp_cuts[i][j];
		}
		cuts->lines[i].clique_num_elements = temp_cliques[i].size();
		cuts->lines[i].clique_values = temp_cliques[i].empty() ? NULL : new int[temp_cliques[i].size()];
		for (size_t j = 0; j < temp_cliques[i].size(); ++j)
			cuts->lines[i].clique_values[j] = temp_cliques[i][j];
	}
	
	cuts->loaded = true;
	
	// Print information about loaded cuts
	cout << "Cuts loaded successfully!" << endl;
	cout << "Number of cuts: " << cuts->num_lines << endl;
	
	// Print cuts if requested
	if (print_cuts)
	{
		cout << "\n=== CUT DETAILS ===" << endl;
		for (int i = 0; i < cuts->num_lines; i++)
		{
			cout << "Cut " << i << " (" << cuts->lines[i].num_elements << " elements): ";
			for (int j = 0; j < cuts->lines[i].num_elements; j++)
			{
				cout << cuts->lines[i].values[j];
				if (j < cuts->lines[i].num_elements - 1)
				{
					cout << " ";
				}
			}
			cout << endl;
		}
		cout << "===================\n" << endl;
	}
	
	return true;
}

/***********************************************************************************/
void free_cuts_data(cut_data *cuts)
/***********************************************************************************/
{
	if (cuts->loaded && cuts->lines != NULL)
	{
		for (int i = 0; i < cuts->num_lines; i++)
		{
			if (cuts->lines[i].values != NULL)
			{
				delete[] cuts->lines[i].values;
			}
			if (cuts->lines[i].clique_values != NULL)
			{
				delete[] cuts->lines[i].clique_values;
			}
		}
		delete[] cuts->lines;
		cuts->lines = NULL;
	}
	
	cuts->num_lines = 0;
	cuts->loaded = false;
	cuts->has_clique_data = false;
}
