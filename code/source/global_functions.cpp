

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


	RAMSEY_instance->LOOK_UP_RAMSEY[3][3]=6;//opt
	RAMSEY_instance->LOOK_UP_RAMSEY[3][4]=9;//opt
	RAMSEY_instance->LOOK_UP_RAMSEY[3][5]=14;//opt
	RAMSEY_instance->LOOK_UP_RAMSEY[3][6]=18;//opt
	RAMSEY_instance->LOOK_UP_RAMSEY[3][7]=23;//opt
	RAMSEY_instance->LOOK_UP_RAMSEY[3][8]=28;//opt
	RAMSEY_instance->LOOK_UP_RAMSEY[3][9]=36;//opt

	RAMSEY_instance->LOOK_UP_RAMSEY[3][10]=42;//UB
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
	RAMSEY_instance->LOOK_UP_RAMSEY[4][6]=41;//UB
	RAMSEY_instance->LOOK_UP_RAMSEY[4][7]=61;//UB
	RAMSEY_instance->LOOK_UP_RAMSEY[4][8]=84;//UB

	RAMSEY_instance->LOOK_UP_RAMSEY[4][9]=150;//UB
	RAMSEY_instance->LOOK_UP_RAMSEY[4][10]=149;//UB
	RAMSEY_instance->LOOK_UP_RAMSEY[4][11]=191;//UB
	RAMSEY_instance->LOOK_UP_RAMSEY[4][12]=238;//UB
	RAMSEY_instance->LOOK_UP_RAMSEY[4][13]=291;//UB
	RAMSEY_instance->LOOK_UP_RAMSEY[4][14]=349;//UB
	RAMSEY_instance->LOOK_UP_RAMSEY[4][15]=417;//UB


	RAMSEY_instance->LOOK_UP_RAMSEY[5][5]=48;//UB
	RAMSEY_instance->LOOK_UP_RAMSEY[5][6]=87;//UB
	RAMSEY_instance->LOOK_UP_RAMSEY[5][7]=143;//UB
	RAMSEY_instance->LOOK_UP_RAMSEY[5][8]=216;//UB
	RAMSEY_instance->LOOK_UP_RAMSEY[5][9]=316;//UB
	RAMSEY_instance->LOOK_UP_RAMSEY[5][10]=442;//UB
	RAMSEY_instance->LOOK_UP_RAMSEY[5][11]=633;//UB


	RAMSEY_instance->LOOK_UP_RAMSEY[6][6]=165;//UB
	RAMSEY_instance->LOOK_UP_RAMSEY[6][7]=298;//UB
	RAMSEY_instance->LOOK_UP_RAMSEY[6][8]=495;//UB
	RAMSEY_instance->LOOK_UP_RAMSEY[6][9]=780;//UB


	RAMSEY_instance->LOOK_UP_RAMSEY[7][7]=540;//UB

	if(RAMSEY_instance->PARAM_K_CUTS==2)
	{
		RAMSEY_instance->LOOK_UP_RAMSEY[3][10]=40;//LB
		RAMSEY_instance->LOOK_UP_RAMSEY[3][11]=47;//LB
		RAMSEY_instance->LOOK_UP_RAMSEY[3][12]=53;//LB
		RAMSEY_instance->LOOK_UP_RAMSEY[3][13]=60;//LB
		RAMSEY_instance->LOOK_UP_RAMSEY[3][14]=67;//LB
		RAMSEY_instance->LOOK_UP_RAMSEY[3][15]=74;//LB
		RAMSEY_instance->LOOK_UP_RAMSEY[3][16]=82;//LB
		RAMSEY_instance->LOOK_UP_RAMSEY[3][17]=92;//LB
		RAMSEY_instance->LOOK_UP_RAMSEY[3][18]=99;//LB
		RAMSEY_instance->LOOK_UP_RAMSEY[3][19]=106;//LB
		RAMSEY_instance->LOOK_UP_RAMSEY[3][20]=111;//LB
		RAMSEY_instance->LOOK_UP_RAMSEY[3][21]=122;//LB
		RAMSEY_instance->LOOK_UP_RAMSEY[3][22]=131;//LB
		RAMSEY_instance->LOOK_UP_RAMSEY[3][23]=139;//LB

		RAMSEY_instance->LOOK_UP_RAMSEY[3][24]=143;//LB
		RAMSEY_instance->LOOK_UP_RAMSEY[3][25]=154;//LB
		RAMSEY_instance->LOOK_UP_RAMSEY[3][26]=159;//LB
		RAMSEY_instance->LOOK_UP_RAMSEY[3][27]=172;//LB
		RAMSEY_instance->LOOK_UP_RAMSEY[3][28]=177;//LB
		RAMSEY_instance->LOOK_UP_RAMSEY[3][29]=190;//LB
		RAMSEY_instance->LOOK_UP_RAMSEY[3][30]=195;//LB
		RAMSEY_instance->LOOK_UP_RAMSEY[3][31]=206;//LB
		RAMSEY_instance->LOOK_UP_RAMSEY[3][32]=217;//LB
		RAMSEY_instance->LOOK_UP_RAMSEY[3][33]=224;//LB
		RAMSEY_instance->LOOK_UP_RAMSEY[3][34]=230;//LB
		RAMSEY_instance->LOOK_UP_RAMSEY[3][35]=242;//LB
		RAMSEY_instance->LOOK_UP_RAMSEY[3][36]=252;//LB
		RAMSEY_instance->LOOK_UP_RAMSEY[3][37]=264;//LB
		RAMSEY_instance->LOOK_UP_RAMSEY[3][38]=272;//LB
		RAMSEY_instance->LOOK_UP_RAMSEY[3][39]=284;//LB
		RAMSEY_instance->LOOK_UP_RAMSEY[3][40]=294;//LB
		RAMSEY_instance->LOOK_UP_RAMSEY[3][41]=308;//LB
		RAMSEY_instance->LOOK_UP_RAMSEY[3][42]=318;//LB
		RAMSEY_instance->LOOK_UP_RAMSEY[3][43]=332;//LB
		RAMSEY_instance->LOOK_UP_RAMSEY[3][44]=338;//LB
		RAMSEY_instance->LOOK_UP_RAMSEY[3][45]=354;//LB
		RAMSEY_instance->LOOK_UP_RAMSEY[3][46]=360;//LB


		RAMSEY_instance->LOOK_UP_RAMSEY[4][6]=36;//LB
		RAMSEY_instance->LOOK_UP_RAMSEY[4][7]=49;//LB
		RAMSEY_instance->LOOK_UP_RAMSEY[4][8]=59;//LB
		RAMSEY_instance->LOOK_UP_RAMSEY[4][9]=73;//LB
		RAMSEY_instance->LOOK_UP_RAMSEY[4][10]=92;//LB
		RAMSEY_instance->LOOK_UP_RAMSEY[4][11]=102;//LB
		RAMSEY_instance->LOOK_UP_RAMSEY[4][12]=128;//LB
		RAMSEY_instance->LOOK_UP_RAMSEY[4][13]=138;//LB
		RAMSEY_instance->LOOK_UP_RAMSEY[4][14]=147;//LB
		RAMSEY_instance->LOOK_UP_RAMSEY[4][15]=158;//LB


		RAMSEY_instance->LOOK_UP_RAMSEY[5][5]=42;//LB
		RAMSEY_instance->LOOK_UP_RAMSEY[5][6]=58;//LB
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
bool load_cuts_from_file(data *RAMSEY_instance, int t_value, int k_value, cut_data *cuts, bool print_cuts)
/***********************************************************************************/
{
	// Build the filename: t<t_value>_k<k_value>.txt
	stringstream filename_stream;
	filename_stream << "CUTS/t" << t_value << "_k" << k_value << ".txt";
	string filename = filename_stream.str();
	
	cout << "\nTrying to load cuts from file: " << filename << endl;
	
	// Open the file
	ifstream file(filename.c_str());
	if (!file.is_open())
	{
		cout << "File not found: " << filename << endl;
		cuts->loaded = false;
		cuts->num_lines = 0;
		cuts->lines = NULL;
		return false;
	}
	
	// Prima lettura: conta le righe e trova il numero massimo di elementi
	vector<vector<int> > temp_cuts;
	string line;
	
	while (getline(file, line))
	{
		if (line.empty()) continue;
		
		vector<int> row;
		istringstream iss(line);
		int value;
		
		while (iss >> value)
		{
			row.push_back(value);
		}
		
		if (!row.empty())
		{
			temp_cuts.push_back(row);
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
		
		for (int j = 0; j < cuts->lines[i].num_elements; j++)
		{
			cuts->lines[i].values[j] = temp_cuts[i][j];
		}
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
		}
		delete[] cuts->lines;
		cuts->lines = NULL;
	}
	
	cuts->num_lines = 0;
	cuts->loaded = false;
}


