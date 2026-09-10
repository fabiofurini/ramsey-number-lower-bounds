
#include "RAMSEY_MODEL_5.h"

///////////////////////////////////////////////////////////////////////////////////////////////
// WHY_IS_CIRCULANT_MUST_BE_ZERO
//
// Every call below to the COPT-BG clique routine passes 0 for its `is_circulant` argument.  The
// question "why not force it to 1?" has a precise answer, so it is recorded once here rather than
// asserted at each call site.
//
// What the flag makes the routine do is apply the reduction
//
//         omega(G) >= TARGET      <=>      omega(N(v)) >= TARGET-1
//
// at one anchor vertex v, searching only inside N(v) and adding v to the answer.  That equivalence
// is NOT true for an arbitrary v: it needs v to lie in some clique of size TARGET.  In a
// vertex-transitive graph every vertex does, which is why the flag is sound for circulants and why
// the library's own comment there reads "any vertex would do here".
//
// A Toeplitz graph is not vertex-transitive, and the reduction is valid only at the two ends.  If
// S is a clique then so are S - min(S) and S + (t-1-max(S)), because translation preserves every
// |i-j| and keeps the set inside {0,...,t-1}: so SOME clique of maximum size contains vertex 0,
// and some contains vertex t-1, but nothing of the sort holds for the vertices in between.
//
// Measured, t = 11, blue distances {2,3}, red target 5.  The red graph has omega = 5, yet
// 1 + omega(N_red(v)) is 5 only for v in {0,1,4,5,6,9,10} and is 4 for v in {2,3,7,8}.  So at an
// anchor of the second kind the reduction concludes "no K_5" while a K_5 exists.
//
// And the anchor is not ours to choose: the routine sorts the graph and then anchors at its own
// last vertex.  Probed directly, with no branch-and-cut involved (scratchpad probe.cpp): on that
// instance is_circulant = 0 returns the clique {0,1,5,9,10} of size 5, while is_circulant = 1
// returns size 1, the trivial {0}, which is its way of saying "target not found".  The
// branch-and-cut then accepts the invalid colouring and reports it FEASIBLE.  Across the
// brute-force suite the flag produces 10 invalid colourings out of 154 instances, against 0 with
// the flag off (RESULTS/phase_s_is_circulant_1.csv in the campaign folder).
//
// So the reduction is usable here, but only at an anchor we control, and that is exactly what the
// PARAM_OPTIONS==3 detector further down does: it anchors at vertex 0, where translation makes the
// argument valid, and is complete because of it.  The flag cannot be forced because it hands the
// choice of anchor to a routine that assumes every choice is equivalent.  Extending our own
// anchored detector to the ordinary separation path is the way to recover the saving; passing 1 is
// not.
//
// Related: CLIQUE_CPLEX.cpp fixes vertex 0 into the clique when PARAM_CIRCULANT==1 -- that one IS
// a legitimate anchor here -- but the restriction is written for the circulant case, so Main.cpp
// forces that input to 0 for this model rather than relying on it.
///////////////////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////////////////////
// Partial-colouring propagator for MODEL 5 (distance / Toeplitz; t <= 127).
//
// The graph of one colour is T_t(D) with D the distances fixed to that colour: {i,j} is an edge
// iff |i-j| is in D.  If S is a clique of T_t(D) then S - min(S) is one too, because subtracting
// min(S) preserves every distance and keeps the whole set inside {0,...,t-1}.  So a clique may
// always be assumed to contain vertex 0, whose neighbourhood is exactly D itself.  Therefore
//
//     T_t(D) contains a K_k   <=>   the subgraph induced on D contains a K_{k-1}.
//
// That is a sound AND complete emptiness test of the fixed partial colouring, under any
// branching order, and it costs one bitset clique search on at most t-1 vertices.  Compared with
// the circulant propagator of MODEL 3 the anchor is a single vertex instead of a pair {0,d}: in
// the linear class translation is still available (as a partial map) but rotation and reflection
// are not, so the argument is re-derived rather than copied.
// The callback never reads fractional LP values: only local bounds (lb==1 blue, ub==0 red).
///////////////////////////////////////////////////////////////////////////////////////////////

// Vertex subsets as dynamic word arrays: NO limit on the graph order.  MODEL 3 keeps its
// 128-bit implementation untouched; here the bits run up to t-1 rather than t/2, so a fixed word
// type would bind twice as early, and the class this model searches has no reason to stop at 127.
//
// The recursion needs one scratch bitset per level.  They come from a single thread_local buffer
// that is grown on demand and then reused, so there is no allocation per branch-and-bound node and
// no race between CPLEX threads.
typedef unsigned long long ppw_t;

static inline int  pp_nw(int n){ return (n + 63) / 64; }
static inline void pp_zero(ppw_t *a, int nw){ for(int i=0;i<nw;i++){ a[i]=0ULL; } }
static inline void pp_copy(ppw_t *d, const ppw_t *a, int nw){ for(int i=0;i<nw;i++){ d[i]=a[i]; } }
static inline void pp_setbit(ppw_t *a, int b){ a[b>>6] |= (1ULL << (b & 63)); }
static inline void pp_clrbit(ppw_t *a, int b){ a[b>>6] &= ~(1ULL << (b & 63)); }
static inline int  pp_testbit(const ppw_t *a, int b){ return (int)((a[b>>6] >> (b & 63)) & 1ULL); }

static inline int pp_empty(const ppw_t *a, int nw)
{
	for(int i=0;i<nw;i++){ if(a[i]){ return 0; } }
	return 1;
}

static inline int pp_popcount(const ppw_t *a, int nw)
{
	int c=0;
	for(int i=0;i<nw;i++){ c += __builtin_popcountll(a[i]); }
	return c;
}

static inline int pp_lowest(const ppw_t *a, int nw)
{
	for(int i=0;i<nw;i++){ if(a[i]){ return (i<<6) + __builtin_ctzll(a[i]); } }
	return -1;
}

// d = (a << s) & (bits 0..n-1).  Used as "the neighbours of v above v": shifting the distance set
// by v gives {v+e : e in D}, and there is no wrap-around because the distances are linear.
static inline void pp_adj_up(ppw_t *d, const ppw_t *a, int s, int n, int nw)
{
	int ws = s >> 6, bs = s & 63;
	for(int i=nw-1;i>=0;i--)
	{
		ppw_t v = 0ULL;
		int j = i - ws;
		if(j >= 0)
		{
			v = a[j] << bs;
			if(bs && j > 0){ v |= a[j-1] >> (64 - bs); }
		}
		d[i] = v;
	}
	int last = (n - 1) >> 6, keep = n - (last << 6);
	for(int i=last+1;i<nw;i++){ d[i] = 0ULL; }
	if(keep < 64){ d[last] &= ((keep == 64) ? ~0ULL : ((1ULL << keep) - 1ULL)); }
}

static inline void pp_and(ppw_t *d, const ppw_t *a, const ppw_t *b, int nw)
{
	for(int i=0;i<nw;i++){ d[i] = a[i] & b[i]; }
}

// Scratch for the recursion: two bitsets per level (the shrinking candidate set and the
// intersection handed to the child).  Reused across calls, per thread.
static thread_local ppw_t *pp_buf = NULL;
static thread_local int    pp_buf_words = 0;

static ppw_t *pp_reserve(int words)
{
	if(pp_buf_words < words)
	{
		delete [] pp_buf;
		pp_buf = new ppw_t[words];
		pp_buf_words = words;
	}
	return pp_buf;
}

// Is there a K_depth inside cand?  N0 is the neighbourhood of vertex 0, i.e. the colour's fixed
// distance set read as a vertex mask; candidates only ever extend upward, so each clique is
// generated once (isomorph-free enumeration).  wit may be NULL; when it is not, on success
// wit[depth..1] holds the clique vertices beyond vertex 0.
static int pp_kclique(const ppw_t *cand, const ppw_t *N0, int depth, int n, int nw,
		ppw_t *scratch, int *wit)
{
	if(depth==0){ return 1; }
	if(depth==1)
	{
		if(pp_empty(cand,nw)){ return 0; }
		if(wit){ wit[1] = pp_lowest(cand,nw); }
		return 1;
	}
	if(pp_popcount(cand,nw) < depth){ return 0; }

	ppw_t *c   = scratch;              // our own shrinking copy of cand
	ppw_t *nxt = scratch + nw;         // the child's candidate set
	pp_copy(c, cand, nw);

	while(!pp_empty(c,nw))
	{
		int v = pp_lowest(c,nw);
		pp_clrbit(c,v);
		pp_adj_up(nxt, N0, v, n, nw);
		pp_and(nxt, c, nxt, nw);
		if(pp_kclique(nxt, N0, depth-1, n, nw, scratch + 2*nw, wit))
		{
			if(wit){ wit[depth] = v; }
			return 1;
		}
	}
	return 0;
}

// Largest clique this detector can be asked to report.  It bounds the witness array only, not the
// graph order: the target is min(m,n) .. max(m,n), never more than a few dozen.
#define PP_MAX_CLIQUE 256

// Complete K_k detector for the colour whose distance set is fixedD (bit d set, 1 <= d <= t-1).
// Fills verts[0..k-1] with a monochromatic K_k if one exists.  Complete for a partial colouring
// too: the statement is about the graph T_t(fixedD), whatever else is still undecided.  Soundness
// and completeness come from translation: if S is a clique of T_t(fixedD) then so is S - min(S),
// which lies in the same interval, so a clique may be assumed to contain vertex 0, whose
// neighbourhood is exactly fixedD.
static int pp_colour_find_kk(const ppw_t *fixedD, int k, int n, int nw, int *verts)
{
	if(k<=1){ verts[0]=0; return k==1 && n>=1; }
	if(pp_empty(fixedD,nw)){ return 0; }
	if(k > PP_MAX_CLIQUE){ return -1; }        // never happens for any Ramsey target in range
	int wit[PP_MAX_CLIQUE];
	ppw_t *sc = pp_reserve(2*nw*(k+2));
	if(pp_kclique(fixedD, fixedD, k-1, n, nw, sc, wit))
	{
		verts[0]=0;
		for(int j=1;j<k;j++){ verts[j]=wit[k-j]; }
		return 1;
	}
	return 0;
}

// Does that colour contain a K_k?
static int pp_colour_has_kk(const ppw_t *fixedD, int k, int n, int nw)
{
	if(k<=1){ return n>=k; }
	if(pp_empty(fixedD,nw)){ return 0; }
	ppw_t *sc = pp_reserve(2*nw*(k+2));
	return pp_kclique(fixedD, fixedD, k-1, n, nw, sc, NULL);
}

/***********************************************************************************/
int CPXPUBLIC mybranchcallback_PARTIAL_PRUNING_MODEL_5(CALLBACK_BRANCH_ARGS)
/***********************************************************************************/
{
	(*useraction_p)=CPX_CALLBACK_DEFAULT;
	data *RAMSEY_instance=(data *) cbhandle;

	clock_t pp_t0=clock();
	RAMSEY_instance->pp_calls++;

	int pp_m=RAMSEY_instance->n_variable_MODEL_5;
	int pp_n=RAMSEY_instance->PARAM_SIZE_GRAPH;

	if(CPXgetcallbacknodelb(xenv,cbdata,wherefrom,RAMSEY_instance->pp_lb,0,pp_m-1)!=0){ return 0; }
	if(CPXgetcallbacknodeub(xenv,cbdata,wherefrom,RAMSEY_instance->pp_ub,0,pp_m-1)!=0){ return 0; }

	// Two bitsets of pp_nw(pp_n) words, taken from the instance's own scratch (allocated once in
	// RAMSEY_MODEL_5_allocation, sized from the graph order), so there is no per-node allocation.
	const int pp_nwv=pp_nw(pp_n);
	ppw_t *fixedBlue=RAMSEY_instance->pp_maskB;
	ppw_t *fixedRed =RAMSEY_instance->pp_maskR;
	pp_zero(fixedBlue,pp_nwv);
	pp_zero(fixedRed,pp_nwv);

	int nfixed=0;
	for(int i=0;i<pp_m;i++)
	{
		if(RAMSEY_instance->pp_lb[i]>0.5){ pp_setbit(fixedBlue,i+1); nfixed++; }       // y=1: blue
		else if(RAMSEY_instance->pp_ub[i]<0.5){ pp_setbit(fixedRed,i+1); nfixed++; }   // y=0: red
	}

	int prune_blue=0, prune_red=0;
	if(pp_colour_has_kk(fixedBlue,RAMSEY_instance->PARAM_M,pp_n,pp_nwv)){ prune_blue=1; }
	else if(pp_colour_has_kk(fixedRed,RAMSEY_instance->PARAM_N,pp_n,pp_nwv)){ prune_red=1; }

	if(prune_blue||prune_red)
	{
		if(prune_blue){ RAMSEY_instance->pp_prunes_blue++; }
		else{ RAMSEY_instance->pp_prunes_red++; }
		RAMSEY_instance->pp_fixed_sum+=nfixed;
		if(nfixed>RAMSEY_instance->pp_max_fixed_at_prune){ RAMSEY_instance->pp_max_fixed_at_prune=nfixed; }
		int pp_depth=0;
		if(CPXgetcallbacknodeinfo(xenv,cbdata,wherefrom,0,CPX_CALLBACK_INFO_NODE_DEPTH,&pp_depth)==0)
		{
			RAMSEY_instance->pp_depth_sum+=pp_depth;
		}
		(*useraction_p)=CPX_CALLBACK_SET;   // zero branches created -> the node is fathomed
	}

	RAMSEY_instance->time_propagator+=(double)(clock()-pp_t0)/(double)CLOCKS_PER_SEC;
	return 0;
}

namespace
{
void initialize_cut_recording(data *RAMSEY_instance)
{
	if (RAMSEY_instance->LOAD_CUTS_FROM_FILE != -100)
	{
		return;
	}

	const string blue_filename = cut_file_name(RAMSEY_instance, true);
	const string red_filename = cut_file_name(RAMSEY_instance, false);
	ofstream blue_file(blue_filename.c_str(), ios::trunc);
	ofstream red_file(red_filename.c_str(), ios::trunc);
	if (!blue_file.is_open() || !red_file.is_open())
	{
		cout << "Cannot initialize the cut-recording files in CUTS/" << endl;
		exit(-1);
	}
	blue_file << "# RAMSEY_DISTANCE_CLIQUE_V1\n";
	red_file << "# RAMSEY_DISTANCE_CLIQUE_V1\n";
	RAMSEY_instance->RECORDED_CUTS_M.clear();
	RAMSEY_instance->RECORDED_CUTS_N.clear();
	cout << "Recording blue cuts to " << blue_filename << endl;
	cout << "Recording red cuts to " << red_filename << endl;
}

void record_callback_cut(data *RAMSEY_instance, bool blue, const vector<int>& distances)
{
	if (RAMSEY_instance->LOAD_CUTS_FROM_FILE != -100)
	{
		return;
	}

	// Store a unique distance support followed by the generating clique. The
	// clique is the lossless information used to recover distance multiplicities.
	vector<int> support = distances;
	sort(support.begin(), support.end());
	support.erase(unique(support.begin(), support.end()), support.end());
	if (support.empty()) return;
	vector<int> clique;
	for (int i = 0; i < RAMSEY_instance->PARAM_SIZE_GRAPH; ++i)
	{
		if (RAMSEY_instance->CLIQUE_SOL[i] > 0.5) clique.push_back(i + 1);
	}
	if (clique.size() < 2) return;
	lock_guard<mutex> lock(RAMSEY_instance->RECORDED_CUTS_MUTEX);
	set<vector<int> >& recorded = blue ? RAMSEY_instance->RECORDED_CUTS_M
									 : RAMSEY_instance->RECORDED_CUTS_N;
	if (!recorded.insert(support).second)
	{
		return;
	}

	ofstream file(cut_file_name(RAMSEY_instance, blue).c_str(), ios::app);
	if (!file.is_open())
	{
		cout << "Cannot append a recorded cut" << endl;
		exit(-1);
	}
	for (size_t i = 0; i < support.size(); ++i)
	{
		if (i > 0) file << ' ';
		file << support[i];
	}
	file << " |";
	for (size_t i = 0; i < clique.size(); ++i)
	{
		file << ' ' << clique[i];
	}
	file << '\n';
}
}

//#define PRINT_SOLUTION_MODEL_5
//#define PRINT_SOLUTION_CALLBACK
//#define PRINT_MODEL_5_LP

//#define print_cuts
//#define print_clique_size

//#define JUST_ONE_ROUND_OF_MINIMALIZATION

// edge_number() and ex_value() are shared with MODEL 3 (declared in RAMSEY_MODEL_3.h,
// defined in RAMSEY_MODEL_3.cpp): they depend on sizes only, not on the geometry.

/***********************************************************************************/
int mapping_lin(data *RAMSEY_instance,int i,int j)
/***********************************************************************************/
{
	// Index of the variable of the LINEAR distance |i-j|.  Distance d has index d-1, so the
	// range is 0 .. t-2.  This is the only place where MODEL 5 differs from MODEL 3 in the way
	// a pair of vertices is turned into a variable: no min(a, t-a) folding, no wrap-around.
	if(i==j)
	{
		return -1;
	}

	int d = (i<j) ? (j-i) : (i-j);

	if(d<1 || d>RAMSEY_instance->PARAM_SIZE_GRAPH-1)
	{
		cout << "ERROR: mapping_lin out of range\t" << i << "\t" << j << endl;
		exit(-1);
	}

	return d-1;
}

namespace
{

/***********************************************************************************/
void add_row_MODEL_5(data *RAMSEY_instance,const vector<int> &ind,const vector<double> &val,
		char sense,double rhs)
/***********************************************************************************/
{
	int nz=(int)ind.size();
	if(nz==0){ return; }

	int rmatbeg=0;
	double r=rhs;
	char s=sense;

	int st=CPXaddrows(RAMSEY_instance->env_MODEL_5,RAMSEY_instance->lp_MODEL_5,0,1,nz,
			&r,&s,&rmatbeg,&ind[0],&val[0],NULL,NULL);
	if(st!=0)
	{
		printf("error in CPXaddrows (MODEL 5)\n");
		exit(-1);
	}
}

/***********************************************************************************/
bool row_from_distances_MODEL_5(data *RAMSEY_instance,const int *dd,int len,bool blue)
/***********************************************************************************/
{
	// One clique-avoidance row from the distance multiset of a vertex set.
	//   blue:  sum_d c_d y_d <= (sum_d c_d) - 1        (not every distance of S can be blue)
	//   red :  sum_d c_d y_d >= 1                      (at least one of them must be blue)
	// With PARAM_COVER_CUTS = 1 the multiplicities c_d are replaced by the support form, which
	// is the stronger inequality; this mirrors MODEL 3 exactly, arm by arm.
	const int nv=RAMSEY_instance->n_variable_MODEL_5;

	vector<double> coef(nv,0.0);
	for(int i=0;i<len;i++)
	{
		int d=dd[i];
		if(d<1 || d>nv){ return false; }
		coef[d-1]+=1.0;
	}

	vector<int> ind;
	vector<double> val;
	double tot=0.0;
	for(int d=1;d<=nv;d++)
	{
		if(coef[d-1]>0.0)
		{
			double c=coef[d-1];
			if(RAMSEY_instance->PARAM_COVER_CUTS==1){ c=1.0; }
			ind.push_back(d-1);
			val.push_back(c);
			tot+=c;
		}
	}
	if(ind.empty()){ return false; }

	if(blue)
	{
		add_row_MODEL_5(RAMSEY_instance,ind,val,'L',tot-1.0);
	}
	else
	{
		add_row_MODEL_5(RAMSEY_instance,ind,val,'G',1.0);
	}
	return true;
}

}

/***********************************************************************************/
void RAMSEY_MODEL_5_allocation(data *RAMSEY_instance)
/***********************************************************************************/
{
	// DISTANCE MODEL: one variable per LINEAR distance d = 1..t-1 (MODEL 3 has t/2 of them,
	// one per circular distance).  Twice as many variables, and no d <-> t-d identification.
	RAMSEY_instance->n_variable_MODEL_5=RAMSEY_instance->PARAM_SIZE_GRAPH-1;

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
	RAMSEY_instance->time_minimization_rebuild=0;
	RAMSEY_instance->time_minimization_cliquecheck=0;
	RAMSEY_instance->sum_removed_jump_distance_red=0;
	RAMSEY_instance->min_removed_jump_distance_red=999999999;
	RAMSEY_instance->max_removed_jump_distance_red=-1;
	RAMSEY_instance->n_jump_attempts_red=0;
	RAMSEY_instance->n_jump_attempts_red_lowhalf=0;
	RAMSEY_instance->n_jump_attempts_red_highhalf=0;
	RAMSEY_instance->n_jumps_minimized_red_lowhalf=0;
	RAMSEY_instance->n_jumps_minimized_red_highhalf=0;

	RAMSEY_instance->X_CALLBACK=new double[RAMSEY_instance->n_variable_MODEL_5];
	RAMSEY_instance->cut_rmatind=new int[RAMSEY_instance->n_variable_MODEL_5];
	RAMSEY_instance->cut_rmatval=new double[RAMSEY_instance->n_variable_MODEL_5];
	RAMSEY_instance->X_MODEL_5=new double[RAMSEY_instance->n_variable_MODEL_5];

	// Allocate X_CALLBACK_TEMP for minimization (edge_fixing_TEMP and CLIQUE_SOL_TEMP are in memory_allocation)
	RAMSEY_instance->X_CALLBACK_TEMP=new double[RAMSEY_instance->n_variable_MODEL_5];

	// Partial-colouring propagator (PARAM_OPTIONS==1): counters + scratch, inert when off
	RAMSEY_instance->pp_calls=0;
	RAMSEY_instance->pp_prunes_blue=0;
	RAMSEY_instance->pp_prunes_red=0;
	RAMSEY_instance->pp_fixed_sum=0;
	RAMSEY_instance->pp_depth_sum=0;
	RAMSEY_instance->pp_max_fixed_at_prune=0;
	RAMSEY_instance->time_propagator=0;
	RAMSEY_instance->pp_fast_calls=0;
	RAMSEY_instance->pp_fast_hits_blue=0;
	RAMSEY_instance->pp_fast_hits_red=0;
	RAMSEY_instance->pp_lb=new double[RAMSEY_instance->n_variable_MODEL_5];
	RAMSEY_instance->pp_ub=new double[RAMSEY_instance->n_variable_MODEL_5];
	{
		int nwv=pp_nw(RAMSEY_instance->PARAM_SIZE_GRAPH);
		RAMSEY_instance->pp_maskB=new unsigned long long[nwv];
		RAMSEY_instance->pp_maskR=new unsigned long long[nwv];
		pp_zero(RAMSEY_instance->pp_maskB,nwv);
		pp_zero(RAMSEY_instance->pp_maskR,nwv);
	}

	RAMSEY_instance->n_calls=0;
	RAMSEY_instance->n_calls_heur=0;
	RAMSEY_instance->time_MNTS=0;
	RAMSEY_instance->n_CLISAT_successes=0;
	RAMSEY_instance->n_MNTS_successes=0;
	RAMSEY_instance->n_SimpleHeur_successes=0;
	RAMSEY_instance->n_CLISAT_opt=0;

}

/***********************************************************************************/
void RAMSEY_MODEL_5_deallocation(data *RAMSEY_instance)
/***********************************************************************************/
{

	delete []RAMSEY_instance->cut_rmatind;
	delete []RAMSEY_instance->cut_rmatval;
	delete []RAMSEY_instance->X_CALLBACK;
	delete []RAMSEY_instance->X_MODEL_5;

	// Deallocate X_CALLBACK_TEMP (edge_fixing_TEMP and CLIQUE_SOL_TEMP in memory_deallocation)
	delete []RAMSEY_instance->X_CALLBACK_TEMP;

	// Partial-colouring propagator scratch
	delete []RAMSEY_instance->pp_lb;
	delete []RAMSEY_instance->pp_ub;
	delete []RAMSEY_instance->pp_maskB;
	delete []RAMSEY_instance->pp_maskR;

}


/***********************************************************************************/
int CPXPUBLIC mycutcallback_LAZY_MODEL_5(CPXCENVptr env,void *cbdata,int wherefrom,void *cbhandle,int *useraction_p)
/***********************************************************************************/
{

	(*useraction_p)=CPX_CALLBACK_DEFAULT;

	data *RAMSEY_instance=(data *) cbhandle;


	RAMSEY_instance->status=CPXgetcallbacknodex(env,cbdata,wherefrom,RAMSEY_instance->X_CALLBACK,0,RAMSEY_instance->n_variable_MODEL_5-1);
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
	for(int i=0; i<RAMSEY_instance->n_variable_MODEL_5; i++)
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
				cout << (int)(RAMSEY_instance->X_CALLBACK[mapping_lin(RAMSEY_instance,i,j)]+0.5);
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
				cout << 1-(int)(RAMSEY_instance->X_CALLBACK[mapping_lin(RAMSEY_instance,i,j)]+0.5);
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
	//				if(RAMSEY_instance->X_CALLBACK[mapping_lin(RAMSEY_instance,i,j)]<0.5)
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
	//				if(RAMSEY_instance->X_CALLBACK[mapping_lin(RAMSEY_instance,i,j)]>0.5)
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

		// PARAM_OPTIONS==3: complete bitset K_M detector on the integral candidate, replacing
		// the max-clique separator (sound AND complete on a full colouring: any blue K_M goes
		// through {0,d} for one of its blue distances d, by translation+reflection). Only in
		// the configuration it exactly reproduces: CLIQUE_TARGET==1 (a clique of size >= M is
		// all the separator is asked for), no CPLEX-based separation, no cut minimization.
		int pp_fast_blue_done=0;
		// No order limit: the detector works on dynamic word arrays.
		if(RAMSEY_instance->PARAM_OPTIONS==3
			&& RAMSEY_instance->CLIQUE_TARGET==1 && RAMSEY_instance->PARAM_CPLEX!=1
			&& RAMSEY_instance->MINIMIZE_CUTS==0)
		{
			clock_t pp_t0=clock();
			RAMSEY_instance->pp_fast_calls++;
			const int pp_nwv=pp_nw(RAMSEY_instance->PARAM_SIZE_GRAPH);
			ppw_t *fixedD=RAMSEY_instance->pp_maskB;
			pp_zero(fixedD,pp_nwv);
			for(int i=0;i<RAMSEY_instance->n_variable_MODEL_5;i++)
			{
				if(RAMSEY_instance->X_CALLBACK[i]>0.5){ pp_setbit(fixedD,i+1); }
			}
			int pp_verts[PP_MAX_CLIQUE];
			for(int i=0;i<RAMSEY_instance->PARAM_SIZE_GRAPH;i++){ RAMSEY_instance->CLIQUE_SOL[i]=0; }
			if(pp_colour_find_kk(fixedD,RAMSEY_instance->PARAM_M,RAMSEY_instance->PARAM_SIZE_GRAPH,pp_nwv,pp_verts)==1)
			{
				for(int i=0;i<RAMSEY_instance->PARAM_M;i++){ RAMSEY_instance->CLIQUE_SOL[pp_verts[i]]=1; }
				RAMSEY_instance->CLIQUE_VAL=RAMSEY_instance->PARAM_M;
				RAMSEY_instance->pp_fast_hits_blue++;
			}
			else
			{
				RAMSEY_instance->CLIQUE_VAL=0;   // complete test: no blue K_M exists
			}
			pp_fast_blue_done=1;
			RAMSEY_instance->time_propagator+=(double)(clock()-pp_t0)/(double)CLOCKS_PER_SEC;
		}

		if(!pp_fast_blue_done)
		{

		for(int i=0;i<RAMSEY_instance->PARAM_SIZE_GRAPH;i++)
		{
			for(int j=i+1;j<RAMSEY_instance->PARAM_SIZE_GRAPH;j++)
			{
				if(RAMSEY_instance->X_CALLBACK[mapping_lin(RAMSEY_instance,i,j)]<0.5)
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
								0,   /* is_circulant: must be 0 here, see WHY_IS_CIRCULANT_MUST_BE_ZERO above */
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
								0,   /* is_circulant: must be 0 here, see WHY_IS_CIRCULANT_MUST_BE_ZERO above */
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

		} // end if(!pp_fast_blue_done)
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
				for(int k = 0; k < RAMSEY_instance->n_variable_MODEL_5; k++)
				{
					RAMSEY_instance->X_CALLBACK_TEMP[k] = RAMSEY_instance->X_CALLBACK[k];
				}

				// Keep trying to remove jumps until no more can be removed
				while(jump_removed)
				{
					jump_removed = false;

					// Try to remove each active jump
					for(int jump_idx = 0; jump_idx < RAMSEY_instance->n_variable_MODEL_5; jump_idx++)
					{
						// MINIMIZE_CUTS==4: full sweep on the low half of distances, stride-2 on the high half
						// (see the matching comment in the RED block for the rationale).
						if(RAMSEY_instance->MINIMIZE_CUTS == 4)
						{
							int lowhalf_boundary = RAMSEY_instance->n_variable_MODEL_5 / 2;
							if(jump_idx >= lowhalf_boundary && (jump_idx - lowhalf_boundary) % 2 != 0)
							{
								continue;
							}
						}

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
									if(RAMSEY_instance->X_CALLBACK_TEMP[mapping_lin(RAMSEY_instance, i, j)] < 0.5)
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

							if(RAMSEY_instance->MINIMIZE_CUTS == 1 || RAMSEY_instance->MINIMIZE_CUTS == 3 || RAMSEY_instance->MINIMIZE_CUTS == 4 || RAMSEY_instance->MINIMIZE_CUTS == 5)
							{
								test_clique_val = RAMSEY_instance->clique_solve_BB_edge_fixing_heur
										(
												RAMSEY_instance->edge_fixing_TEMP,
												RAMSEY_instance->CLIQUE_SOL_TEMP,
												0,   /* is_circulant: must be 0 here, see WHY_IS_CIRCULANT_MUST_BE_ZERO above */
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
												0,   /* is_circulant: must be 0 here, see WHY_IS_CIRCULANT_MUST_BE_ZERO above */
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
					if(RAMSEY_instance->MINIMIZE_CUTS == 3 || RAMSEY_instance->MINIMIZE_CUTS == 4 || RAMSEY_instance->MINIMIZE_CUTS == 5)
					{
						jump_removed = false;//ONLY ONE ROUND, regardless of whether this pass removed a jump
					}
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

			vector<int> cut_distances;
			RAMSEY_instance->nzcnt=0;

			for(int i=0; i<RAMSEY_instance->n_variable_MODEL_5; i++)
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
												RAMSEY_instance->cut_rmatval[mapping_lin(RAMSEY_instance,i,j)] ++;
												cut_distances.push_back(mapping_lin(RAMSEY_instance, i, j) + 1);
					}
				}
			}

			if( RAMSEY_instance->PARAM_COVER_CUTS == 1 )
			{
				if(RAMSEY_instance->PARAM_STRONGER_CUTS==1)
				{
					double dummy = ex_value( (int) RAMSEY_instance->CLIQUE_VAL , RAMSEY_instance->PARAM_M ) - edge_number( (int) RAMSEY_instance->CLIQUE_VAL );

					for(int i=0; i<RAMSEY_instance->n_variable_MODEL_5; i++)
					{

						RAMSEY_instance->cut_rmatval[i] = min( RAMSEY_instance->cut_rmatval[i] , (double) edge_number( (int) RAMSEY_instance->CLIQUE_VAL ) - ex_value( (int) RAMSEY_instance->CLIQUE_VAL , RAMSEY_instance->PARAM_M )  );

						dummy += RAMSEY_instance->cut_rmatval[i];
					}

					RAMSEY_instance->cut_RHS = dummy;
				}
				else
				{

					double dummy = ( edge_number( (int) RAMSEY_instance->CLIQUE_VAL ) - (RAMSEY_instance->CLIQUE_VAL - RAMSEY_instance->PARAM_M) - 1) - edge_number( (int) RAMSEY_instance->CLIQUE_VAL );

					for(int i=0; i<RAMSEY_instance->n_variable_MODEL_5; i++)
					{

						RAMSEY_instance->cut_rmatval[i] = min ( RAMSEY_instance->cut_rmatval[i] , (double) ( edge_number ( (int) RAMSEY_instance->CLIQUE_VAL )  -  ( edge_number( (int) RAMSEY_instance->CLIQUE_VAL )  - ( RAMSEY_instance->CLIQUE_VAL - RAMSEY_instance->PARAM_M )  - 1 )  )  );

						dummy += RAMSEY_instance->cut_rmatval[i];
					}

					RAMSEY_instance->cut_RHS = dummy;
				}
			}

#ifdef print_cuts
			for(int i=0; i<RAMSEY_instance->n_variable_MODEL_5; i++)
			{
				cout << "i\t" << i << "\t" << RAMSEY_instance->cut_rmatind[i] << "\t" << RAMSEY_instance->cut_rmatval[i] <<  endl;
			}
			cout << RAMSEY_instance->cut_RHS << endl;

			cout << "\n***BLUE CUT***\n";
			cin.get();
#endif



			RAMSEY_instance->status=CPXcutcallbackadd (env,cbdata,wherefrom,RAMSEY_instance->nzcnt,RAMSEY_instance->cut_RHS,'L',RAMSEY_instance->cut_rmatind,RAMSEY_instance->cut_rmatval,RAMSEY_instance->CUT_CALL_BACK_STRATEGY);
			if(RAMSEY_instance->status!=0){printf("CPXcutcallbackadd\n");exit(-1);}
			record_callback_cut(RAMSEY_instance, true, cut_distances);

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

		// PARAM_OPTIONS==3: same complete pre-check on the red side (distances with y=0)
		int pp_fast_red_done=0;
		// No order limit: the detector works on dynamic word arrays.
		if(RAMSEY_instance->PARAM_OPTIONS==3
			&& RAMSEY_instance->CLIQUE_TARGET==1 && RAMSEY_instance->PARAM_CPLEX!=1
			&& RAMSEY_instance->MINIMIZE_CUTS==0)
		{
			clock_t pp_t0=clock();
			const int pp_nwv=pp_nw(RAMSEY_instance->PARAM_SIZE_GRAPH);
			ppw_t *fixedD=RAMSEY_instance->pp_maskR;
			pp_zero(fixedD,pp_nwv);
			for(int i=0;i<RAMSEY_instance->n_variable_MODEL_5;i++)
			{
				if(RAMSEY_instance->X_CALLBACK[i]<0.5){ pp_setbit(fixedD,i+1); }
			}
			int pp_verts[PP_MAX_CLIQUE];
			for(int i=0;i<RAMSEY_instance->PARAM_SIZE_GRAPH;i++){ RAMSEY_instance->CLIQUE_SOL[i]=0; }
			if(pp_colour_find_kk(fixedD,RAMSEY_instance->PARAM_N,RAMSEY_instance->PARAM_SIZE_GRAPH,pp_nwv,pp_verts)==1)
			{
				for(int i=0;i<RAMSEY_instance->PARAM_N;i++){ RAMSEY_instance->CLIQUE_SOL[pp_verts[i]]=1; }
				RAMSEY_instance->CLIQUE_VAL=RAMSEY_instance->PARAM_N;
				RAMSEY_instance->pp_fast_hits_red++;
			}
			else
			{
				RAMSEY_instance->CLIQUE_VAL=0;   // complete test: no red K_N exists
			}
			pp_fast_red_done=1;
			RAMSEY_instance->time_propagator+=(double)(clock()-pp_t0)/(double)CLOCKS_PER_SEC;
		}

		if(!pp_fast_red_done)
		{

		for(int i=0;i<RAMSEY_instance->PARAM_SIZE_GRAPH;i++)
		{
			for(int j=i+1;j<RAMSEY_instance->PARAM_SIZE_GRAPH;j++)
			{
				if(1-RAMSEY_instance->X_CALLBACK[mapping_lin(RAMSEY_instance,i,j)]<0.5)
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
								0,   /* is_circulant: must be 0 here, see WHY_IS_CIRCULANT_MUST_BE_ZERO above */
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
								0,   /* is_circulant: must be 0 here, see WHY_IS_CIRCULANT_MUST_BE_ZERO above */
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

		} // end if(!pp_fast_red_done)
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
				for(int k = 0; k < RAMSEY_instance->n_variable_MODEL_5; k++)
				{
					RAMSEY_instance->X_CALLBACK_TEMP[k] = RAMSEY_instance->X_CALLBACK[k];
				}

				// Keep trying to remove jumps until no more can be removed
				while(jump_removed)
				{
					jump_removed = false;

					// For RED (1-X), remove red edges by flipping jumps 0 -> 1
					for(int jump_idx = 0; jump_idx < RAMSEY_instance->n_variable_MODEL_5; jump_idx++)
					{
						// MINIMIZE_CUTS==4: full sweep on the low half of distances (jump_idx+1 <= n_variable_MODEL_5/2),
						// where the empirical success rate is consistently ~1.4-1.5x higher; on the high half,
						// only test every other candidate (stride 2) to cut cost where the yield is lower.
						if(RAMSEY_instance->MINIMIZE_CUTS == 4)
						{
							int lowhalf_boundary = RAMSEY_instance->n_variable_MODEL_5 / 2;
							if(jump_idx >= lowhalf_boundary && (jump_idx - lowhalf_boundary) % 2 != 0)
							{
								continue;
							}
						}

						// MINIMIZE_CUTS==5: even more concentrated on the low half than mode 4 -
						// stride 2 on the low half (test every other candidate there too), and skip
						// the high half entirely. Scales with t via n_variable_MODEL_5, no fixed constant.
						if(RAMSEY_instance->MINIMIZE_CUTS == 5)
						{
							int lowhalf_boundary = RAMSEY_instance->n_variable_MODEL_5 / 2;
							if(jump_idx >= lowhalf_boundary)
							{
								continue;
							}
							if(jump_idx % 2 != 0)
							{
								continue;
							}
						}

						// In RED complement, only inactive X-jumps can be turned on to remove red edges
						if(RAMSEY_instance->X_CALLBACK_TEMP[jump_idx] < 0.5)
						{
							double old_jump_val = RAMSEY_instance->X_CALLBACK_TEMP[jump_idx];
							// Temporarily activate this jump in TEMP (removes corresponding red edges)
							RAMSEY_instance->X_CALLBACK_TEMP[jump_idx] = 1.0;
							RAMSEY_instance->n_jump_attempts_red++;
							bool this_is_lowhalf = (jump_idx + 1) <= (RAMSEY_instance->n_variable_MODEL_5 / 2);
							if (this_is_lowhalf)
								RAMSEY_instance->n_jump_attempts_red_lowhalf++;
							else
								RAMSEY_instance->n_jump_attempts_red_highhalf++;

							// Build edge_fixing_TEMP from X_CALLBACK_TEMP
							clock_t time_start_rebuild = clock();
							for(int i = 0; i < RAMSEY_instance->PARAM_SIZE_GRAPH; i++)
							{
								for(int j = i+1; j < RAMSEY_instance->PARAM_SIZE_GRAPH; j++)
								{
									if(1 - RAMSEY_instance->X_CALLBACK_TEMP[mapping_lin(RAMSEY_instance, i, j)] < 0.5)
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
							clock_t time_end_rebuild = clock();
							RAMSEY_instance->time_minimization_rebuild += (double)(time_end_rebuild - time_start_rebuild) / (double)CLOCKS_PER_SEC;

							// Test if RED clique is still large enough with this jump removed
							clock_t time_start_cliquecheck = clock();
							double test_clique_val;
							if(RAMSEY_instance->MINIMIZE_CUTS == 1 || RAMSEY_instance->MINIMIZE_CUTS == 3 || RAMSEY_instance->MINIMIZE_CUTS == 4 || RAMSEY_instance->MINIMIZE_CUTS == 5)
							{
								test_clique_val = RAMSEY_instance->clique_solve_BB_edge_fixing_heur
										(
												RAMSEY_instance->edge_fixing_TEMP,
												RAMSEY_instance->CLIQUE_SOL_TEMP,
												0,   /* is_circulant: must be 0 here, see WHY_IS_CIRCULANT_MUST_BE_ZERO above */
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
												0,   /* is_circulant: must be 0 here, see WHY_IS_CIRCULANT_MUST_BE_ZERO above */
												RAMSEY_instance->PARAM_N,
												RAMSEY_instance->PARAM_MNTS,
												RAMSEY_instance->PARAM_TOUT_MNTS,
												RAMSEY_instance->PARAM_NUM_RESTARTS_MNTS,
												RAMSEY_instance->PARAM_NUM_ITERATIONS_MNTS,
												1
										);
							}
							clock_t time_end_cliquecheck = clock();
							RAMSEY_instance->time_minimization_cliquecheck += (double)(time_end_cliquecheck - time_start_cliquecheck) / (double)CLOCKS_PER_SEC;

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
								// Diagnostic: track the circular distance (jump_idx+1) of removed jumps
								RAMSEY_instance->sum_removed_jump_distance_red += (jump_idx + 1);
								if (jump_idx + 1 < RAMSEY_instance->min_removed_jump_distance_red)
									RAMSEY_instance->min_removed_jump_distance_red = jump_idx + 1;
								if (jump_idx + 1 > RAMSEY_instance->max_removed_jump_distance_red)
									RAMSEY_instance->max_removed_jump_distance_red = jump_idx + 1;
								if (this_is_lowhalf)
									RAMSEY_instance->n_jumps_minimized_red_lowhalf++;
								else
									RAMSEY_instance->n_jumps_minimized_red_highhalf++;
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
					if(RAMSEY_instance->MINIMIZE_CUTS == 3 || RAMSEY_instance->MINIMIZE_CUTS == 4 || RAMSEY_instance->MINIMIZE_CUTS == 5)
					{
						jump_removed = false;//ONLY ONE ROUND, regardless of whether this pass removed a jump
					}

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

			vector<int> cut_distances;
			RAMSEY_instance->nzcnt=0;

			for(int i=0; i<RAMSEY_instance->n_variable_MODEL_5; i++)
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
						RAMSEY_instance->cut_rmatval[mapping_lin(RAMSEY_instance,i,j)] = RAMSEY_instance->cut_rmatval[mapping_lin(RAMSEY_instance,i,j)]+1;
						cut_distances.push_back(mapping_lin(RAMSEY_instance, i, j) + 1);
					}
				}
			}

			if (RAMSEY_instance->PARAM_COVER_CUTS == 1)
			{
				if(RAMSEY_instance->PARAM_STRONGER_CUTS==1)
				{
					for(int i=0; i<RAMSEY_instance->n_variable_MODEL_5; i++)
					{
						RAMSEY_instance->cut_rmatval[i] = min( RAMSEY_instance->cut_rmatval[i] , (double) edge_number( (int) RAMSEY_instance->CLIQUE_VAL ) - ex_value( (int) RAMSEY_instance->CLIQUE_VAL , RAMSEY_instance->PARAM_N )  );
					}
				}
				else
				{
					for(int i=0; i<RAMSEY_instance->n_variable_MODEL_5; i++)
					{
						RAMSEY_instance->cut_rmatval[i] = min( RAMSEY_instance->cut_rmatval[i] , (double) RAMSEY_instance->CLIQUE_VAL - RAMSEY_instance->PARAM_N + 1  );
					}
				}
			}


#ifdef print_cuts
			for(int i=0; i<RAMSEY_instance->n_variable_MODEL_5; i++)
			{
				cout << "i\t" << i << "\t" << RAMSEY_instance->cut_rmatind[i] << "\t" << RAMSEY_instance->cut_rmatval[i] <<  endl;
			}
			cout << RAMSEY_instance->cut_RHS << endl;

			cout << "\n***CUT RED***\n";
			cin.get();
#endif


			RAMSEY_instance->status=CPXcutcallbackadd (env,cbdata,wherefrom,RAMSEY_instance->nzcnt,RAMSEY_instance->cut_RHS,'G',RAMSEY_instance->cut_rmatind,RAMSEY_instance->cut_rmatval,RAMSEY_instance->CUT_CALL_BACK_STRATEGY);
			if(RAMSEY_instance->status!=0){printf("CPXcutcallbackadd\n");exit(-1);}
			record_callback_cut(RAMSEY_instance, false, cut_distances);

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
void RAMSEY_MODEL_5_parameter_setting(data *RAMSEY_instance)
/***********************************************************************************/
{

	// * Set printing *

	CPXsetintparam (RAMSEY_instance->env_MODEL_5, CPX_PARAM_SCRIND, CPX_ON);


	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	RAMSEY_instance->status = CPXsetintparam (RAMSEY_instance->env_MODEL_5, CPX_PARAM_HEURFREQ, RAMSEY_instance->HEURFREQ);
	if (RAMSEY_instance->status)
	{
		printf ("error for CPX_PARAM_HEURFREQ\n");
	}
	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	// * Set number of CPU*

	RAMSEY_instance->status = CPXsetintparam (RAMSEY_instance->env_MODEL_5, CPX_PARAM_THREADS, RAMSEY_instance->NUMBER_OF_THREADS);
	if (RAMSEY_instance->status)
	{
		printf ("error for CPX_PARAM_THREADS\n");
	}

	//	//-1	CPX_PARALLEL_OPPORTUNISTIC	Opportunistic	Enable opportunistic parallel search mode
	//	//0	CPX_PARALLEL_AUTO	AutoParallel	Automatic: let CPLEX decide whether to invoke deterministic or opportunistic search; default
	//	//1	CPX_PARALLEL_DETERMINISTIC	Deterministic	Enable deterministic parallel search mode
	//	RAMSEY_instance->status = CPXsetintparam (RAMSEY_instance->env_MODEL_5, CPX_PARAM_PARALLELMODE, CPX_PARALLEL_DETERMINISTIC);
	//	if (RAMSEY_instance->status)
	//	{
	//		printf ("error for CPX_PARALLEL_DETERMINISTIC\n");
	//	}


	// * Set time limit *

	RAMSEY_instance->status = CPXsetdblparam (RAMSEY_instance->env_MODEL_5, CPX_PARAM_TILIM,RAMSEY_instance->PARAM_TIME_LIMIT);
	if (RAMSEY_instance->status)
	{
		printf ("error for CPX_PARAM_TILIM\n");
	}

	/* set memory limit of the TREE */
	RAMSEY_instance->status = CPXsetdblparam (RAMSEY_instance->env_MODEL_5, CPXPARAM_MIP_Limits_TreeMemory, 1024*4);
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

	RAMSEY_instance->status = CPXsetintparam (RAMSEY_instance->env_MODEL_5, CPX_PARAM_MIPEMPHASIS,CPX_MIPEMPHASIS_FEASIBILITY);
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

	RAMSEY_instance->status = CPXsetintparam (RAMSEY_instance->env_MODEL_5, CPX_PARAM_NODESEL, RAMSEY_instance->TREE_EXPLORATION_STRATEGY);
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

	RAMSEY_instance->status = CPXsetintparam (RAMSEY_instance->env_MODEL_5, CPXPARAM_MIP_Strategy_VariableSelect,RAMSEY_instance->BRANCHING_VARIABLE_SELECTION);
	if (RAMSEY_instance->status)
	{
		printf ("error for CPXPARAM_MIP_Strategy_VariableSelect\n");
	}
	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


	// * Set exit after first solution *

	RAMSEY_instance->status = CPXsetintparam (RAMSEY_instance->env_MODEL_5, CPX_PARAM_INTSOLLIM,1);
	if (RAMSEY_instance->status)
	{
		printf ("error for CPX_PARAM_INTSOLLIM\n");
	}


	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


	CPXsetintparam(RAMSEY_instance->env_MODEL_5, CPX_PARAM_MIPCBREDLP, CPX_OFF);        // let MIP callbacks work on the original model
	//	CPXsetintparam(RAMSEY_instance->env_MODEL_5, CPX_PARAM_PRELINEAR, CPX_OFF);              // assure linear mappings between the presolved and original models
	CPXsetintparam(RAMSEY_instance->env_MODEL_5, CPX_PARAM_REDUCE, CPX_PREREDUCE_PRIMALONLY);
	//	CPXsetintparam(RAMSEY_instance->env_MODEL_5, CPXPARAM_Preprocessing_Reduce, CPX_PREREDUCE_PRIMALONLY);
	//	CPXsetintparam(RAMSEY_instance->env_MODEL_5, CPXPARAM_Preprocessing_Linear, 0);


	RAMSEY_instance->status = CPXsetlazyconstraintcallbackfunc(RAMSEY_instance->env_MODEL_5,mycutcallback_LAZY_MODEL_5,RAMSEY_instance);
	if (RAMSEY_instance->status)
	{
		printf ("error for CPXsetlazyconstraintcallbackfunc\n");
	}

	// Partial-colouring propagator: registered ONLY when PARAM_OPTIONS==2 (2 is unused by any
	// historic script or model; the (6,6) campaign command lines pass 1 here). Legacy-safe:
	// with any other value (0 and the historic 1 included) nothing is registered and the code
	// path is exactly the historic one.
	if(RAMSEY_instance->PARAM_OPTIONS==2 || RAMSEY_instance->PARAM_OPTIONS==3)
	{
		{   // no order limit: the propagator uses dynamic word arrays
			RAMSEY_instance->status = CPXsetbranchcallbackfunc(RAMSEY_instance->env_MODEL_5,mybranchcallback_PARTIAL_PRUNING_MODEL_5,RAMSEY_instance);
			if (RAMSEY_instance->status)
			{
				printf ("error for CPXsetbranchcallbackfunc\n");
			}
			else
			{
				cout << "\n***PARTIAL_PRUNING_PROPAGATOR_ACTIVE (PARAM_OPTIONS=" << RAMSEY_instance->PARAM_OPTIONS << (RAMSEY_instance->PARAM_OPTIONS==3? ", fast integral pre-check ON":"") << ")***\n" << endl;
			}
		}
	}


	RAMSEY_instance->status = CPXsetintparam(RAMSEY_instance->env_MODEL_5, CPX_PARAM_RANDOMSEED, RAMSEY_instance->RANDOM_SEED);
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
		RAMSEY_instance->status = CPXsetintparam (RAMSEY_instance->env_MODEL_5, 		CPX_PARAM_MIPORDIND,1);
		if (RAMSEY_instance->status)
		{
			printf ("error for CPX_PARAM_MIPORDIND\n");
		}

		int *indices=new int[RAMSEY_instance->n_variable_MODEL_5];
		int *priority=new int[RAMSEY_instance->n_variable_MODEL_5];
		int *direction=new int[RAMSEY_instance->n_variable_MODEL_5];

		//	CPX_BRANCH_GLOBAL	use global branching direction when setting the parameter CPX_PARAM_BRDIR
		//	CPX_BRANCH_DOWN	branch down first on variable indices[i]
		//	CPX_BRANCH_UP	branch up first on variable indices[i]

		if(RAMSEY_instance->BRANCHING_STRATEGY>2)
		{
			srand(RAMSEY_instance->BRANCHING_STRATEGY);
		}

		int counter_local=0;
		for(int i=0; i<RAMSEY_instance->n_variable_MODEL_5; i++)
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

		RAMSEY_instance-> status = CPXcopyorder (RAMSEY_instance->env_MODEL_5,RAMSEY_instance->lp_MODEL_5, RAMSEY_instance->n_variable_MODEL_5, indices, priority,direction);
		if (RAMSEY_instance->status)
		{
			printf ("error for CPXcopyorder\n");
		}

		//		cout << "priority\n";
		//		counter_local=0;
		//		for(int i=0; i<RAMSEY_instance->n_variable_MODEL_5; i++)
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
		RAMSEY_instance->status = CPXsetintparam (RAMSEY_instance->env_MODEL_5, CPX_PARAM_NODELIM, RAMSEY_instance->PARAM_CUT_LOOP);
		if (RAMSEY_instance->status)
		{
			printf ("error for CPX_PARAM_NODELIM\n");
		}

		clock_t time_start=clock();

		RAMSEY_instance->status=CPXmipopt(RAMSEY_instance->env_MODEL_5,RAMSEY_instance->lp_MODEL_5);
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

		RAMSEY_instance->status = CPXsetintparam (RAMSEY_instance->env_MODEL_5, CPX_PARAM_NODELIM, 2100000000);
		if (RAMSEY_instance->status)
		{
			printf ("error for CPX_PARAM_EPRHS\n");
		}

	}
	///////////////////////////////////////////////////////////////////////////////////////////////
	///////////////////////////////////////////////////////////////////////////////////////////////



}

/***********************************************************************************/
double RAMSEY_MODEL_5_solve(data *RAMSEY_instance)
/***********************************************************************************/
{


	//////////////////////////////////////////////////
	RAMSEY_MODEL_5_parameter_setting(RAMSEY_instance);
	//////////////////////////////////////////////////


	///////////////////////////////////////////////////////////////////////////////////////////////
	// * solving the model

	clock_t time_start=clock();

	RAMSEY_instance->status=CPXmipopt(RAMSEY_instance->env_MODEL_5,RAMSEY_instance->lp_MODEL_5);
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
	RAMSEY_instance->status=CPXgetmipx(RAMSEY_instance->env_MODEL_5,RAMSEY_instance->lp_MODEL_5,RAMSEY_instance->X_MODEL_5,0,RAMSEY_instance->n_variable_MODEL_5-1);
	if(RAMSEY_instance->status!=0)
	{
		SOL_FOUND=false;
		printf("error in CPXgetmipx\n");
	}

	RAMSEY_instance->objval=-1;
	RAMSEY_instance->status=CPXgetmipobjval(RAMSEY_instance->env_MODEL_5,RAMSEY_instance->lp_MODEL_5,&(RAMSEY_instance->objval));
	if(RAMSEY_instance->status!=0)
	{
		printf("error in CPXgetmipobjval\n");
	}


	RAMSEY_instance->bestobjval=-1;
	RAMSEY_instance->status=CPXgetbestobjval(RAMSEY_instance->env_MODEL_5,RAMSEY_instance->lp_MODEL_5,&(RAMSEY_instance->bestobjval));
	if(RAMSEY_instance->status!=0)
	{
		printf("error in CPXgetbestobjval\n");
	}

	RAMSEY_instance->lpstat=CPXgetstat(RAMSEY_instance->env_MODEL_5,RAMSEY_instance->lp_MODEL_5);
	RAMSEY_instance->nodecount = CPXgetnodecnt(RAMSEY_instance->env_MODEL_5, RAMSEY_instance->lp_MODEL_5);

	int cur_numcols=CPXgetnumcols(RAMSEY_instance->env_MODEL_5,RAMSEY_instance->lp_MODEL_5);
	int cur_numrows=CPXgetnumrows(RAMSEY_instance->env_MODEL_5,RAMSEY_instance->lp_MODEL_5);


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

	if(RAMSEY_instance->PARAM_OPTIONS==2 || RAMSEY_instance->PARAM_OPTIONS==3)
	{
		cout << "pp_calls ->\t" << RAMSEY_instance->pp_calls << endl;
		cout << "pp_prunes_blue ->\t" << RAMSEY_instance->pp_prunes_blue << endl;
		cout << "pp_prunes_red ->\t" << RAMSEY_instance->pp_prunes_red << endl;
		long long pp_prunes = RAMSEY_instance->pp_prunes_blue + RAMSEY_instance->pp_prunes_red;
		cout << "pp_avg_fixed_at_prune ->\t" << (pp_prunes? (double)RAMSEY_instance->pp_fixed_sum/pp_prunes : 0.0) << endl;
		cout << "pp_max_fixed_at_prune ->\t" << RAMSEY_instance->pp_max_fixed_at_prune << endl;
		cout << "pp_avg_depth_at_prune ->\t" << (pp_prunes? (double)RAMSEY_instance->pp_depth_sum/pp_prunes : 0.0) << endl;
		cout << "pp_fast_calls ->\t" << RAMSEY_instance->pp_fast_calls << endl;
		cout << "pp_fast_hits_blue ->\t" << RAMSEY_instance->pp_fast_hits_blue << endl;
		cout << "pp_fast_hits_red ->\t" << RAMSEY_instance->pp_fast_hits_red << endl;
		cout << "time_propagator ->\t" << RAMSEY_instance->time_propagator << endl;
	}
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
	cout << "time_minimization_rebuild\t" << RAMSEY_instance->time_minimization_rebuild << endl;
	cout << "time_minimization_cliquecheck\t" << RAMSEY_instance->time_minimization_cliquecheck << endl;
	cout << "n_variable_MODEL_5 (max possible distance)\t" << RAMSEY_instance->n_variable_MODEL_5 << endl;
	if (RAMSEY_instance->n_jumps_minimized_red > 0)
	{
		cout << "avg_removed_jump_distance_red\t" << (double)RAMSEY_instance->sum_removed_jump_distance_red / RAMSEY_instance->n_jumps_minimized_red << endl;
	}
	cout << "min_removed_jump_distance_red\t" << RAMSEY_instance->min_removed_jump_distance_red << endl;
	cout << "max_removed_jump_distance_red\t" << RAMSEY_instance->max_removed_jump_distance_red << endl;
	cout << "n_jump_attempts_red\t" << RAMSEY_instance->n_jump_attempts_red << endl;
	if (RAMSEY_instance->n_jump_attempts_red > 0)
	{
		cout << "jump_success_rate_red\t" << (double)RAMSEY_instance->n_jumps_minimized_red / RAMSEY_instance->n_jump_attempts_red << endl;
	}
	cout << "n_jump_attempts_red_lowhalf\t" << RAMSEY_instance->n_jump_attempts_red_lowhalf << endl;
	cout << "n_jump_attempts_red_highhalf\t" << RAMSEY_instance->n_jump_attempts_red_highhalf << endl;
	cout << "n_jumps_minimized_red_lowhalf\t" << RAMSEY_instance->n_jumps_minimized_red_lowhalf << endl;
	cout << "n_jumps_minimized_red_highhalf\t" << RAMSEY_instance->n_jumps_minimized_red_highhalf << endl;
	if (RAMSEY_instance->n_jump_attempts_red_lowhalf > 0)
		cout << "success_rate_lowhalf\t" << (double)RAMSEY_instance->n_jumps_minimized_red_lowhalf / RAMSEY_instance->n_jump_attempts_red_lowhalf << endl;
	if (RAMSEY_instance->n_jump_attempts_red_highhalf > 0)
		cout << "success_rate_highhalf\t" << (double)RAMSEY_instance->n_jumps_minimized_red_highhalf / RAMSEY_instance->n_jump_attempts_red_highhalf << endl;
	if (RAMSEY_instance->LOAD_CUTS_FROM_FILE == -100)
	{
		cout << "recorded_unique_cuts_blue\t" << RAMSEY_instance->RECORDED_CUTS_M.size() << endl;
		cout << "recorded_unique_cuts_red\t" << RAMSEY_instance->RECORDED_CUTS_N.size() << endl;
	}

	if(SOL_FOUND)
	{

#ifdef PRINT_SOLUTION_MODEL_5
		cout << "\n\nR:\n";
		for(int i=0; i<RAMSEY_instance->PARAM_SIZE_GRAPH; i++)
		{
			for(int j=0; j<RAMSEY_instance->PARAM_SIZE_GRAPH; j++)
			{
				cout << (int)(RAMSEY_instance->X_MODEL_5[mapping_lin(RAMSEY_instance,i,j)]+0.5);
			}
			cout << endl;
		}
		cout << endl;

		cout << "\n\nB:\n";
		for(int i=0; i<RAMSEY_instance->PARAM_SIZE_GRAPH; i++)
		{
			for(int j=0; j<RAMSEY_instance->PARAM_SIZE_GRAPH; j++)
			{
				cout << 1-(int)(RAMSEY_instance->X_MODEL_5[mapping_lin(RAMSEY_instance,i,j)]+0.5);
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
				if(RAMSEY_instance->X_MODEL_5[mapping_lin(RAMSEY_instance,i,j)]<0.5)
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
				if(RAMSEY_instance->X_MODEL_5[mapping_lin(RAMSEY_instance,i,j)]>0.5)
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
		sprintf(dummy_file,"colorings/col_m%d_n%d_SIZE%d_dist_br%d_id%d.txt",RAMSEY_instance->PARAM_M,RAMSEY_instance->PARAM_N,RAMSEY_instance->PARAM_SIZE_GRAPH, RAMSEY_instance->BRANCHING_STRATEGY, RAMSEY_instance->ID_TEST);

		cout << dummy_file << endl;

		ofstream out(dummy_file);

		out << RAMSEY_instance->PARAM_SIZE_GRAPH << endl;

		// Blue LINEAR distances, 1-based: this is the line the checker tools read.  (MODEL 3
		// iterates to PARAM_SIZE_GRAPH over an array of t/2 doubles and prints 0-based indices;
		// here the loop is over the variables and the distance itself is printed.)
		out << "JUMPS\n";
		for(int i=0; i<RAMSEY_instance->n_variable_MODEL_5; i++)
		{
			if((int)(RAMSEY_instance->X_MODEL_5[i]+0.5)>0.5)
			{
				out << i+1 << " ";
			}
		}
		out << endl;

		out << "MATRIX\n";

		for(int i=0; i<RAMSEY_instance->PARAM_SIZE_GRAPH; i++)
		{
			for(int j=0; j<RAMSEY_instance->PARAM_SIZE_GRAPH; j++)
			{
				out << (int)(RAMSEY_instance->X_MODEL_5[mapping_lin(RAMSEY_instance,i,j)]+0.5) << " ";
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
	RAMSEY_MODEL_5_deallocation(RAMSEY_instance);
	/////////////////////////////////////////////

	return RAMSEY_instance->objval;
}

/***********************************************************************************/
void RAMSEY_MODEL_5_free(data *RAMSEY_instance)
/***********************************************************************************/
{

	RAMSEY_instance->status=CPXfreeprob(RAMSEY_instance->env_MODEL_5,&(RAMSEY_instance->lp_MODEL_5));
	if(RAMSEY_instance->status!=0) {printf("error in CPXfreeprob\n");exit(-1);}

	RAMSEY_instance->status=CPXcloseCPLEX(&(RAMSEY_instance->env_MODEL_5));
	if(RAMSEY_instance->status!=0) {printf("error in CPXcloseCPLEX\n");exit(-1);}

}


namespace
{
void prepare_loaded_cut(data *RAMSEY_instance, const cut_line& line, bool blue,
		vector<int>& indices, vector<double>& coefficients, double& rhs, char& sense)
{
	sense = blue ? 'L' : 'G';
	if (line.clique_num_elements == 0)
	{
		// Compatibility with the original support format.
		rhs = blue ? line.num_elements - 1 : 1.0;
		for (int j = 0; j < line.num_elements; ++j)
		{
			const int index = line.values[j] - 1;
			if (index < 0 || index >= RAMSEY_instance->n_variable_MODEL_5)
			{
				cout << "Invalid distance index " << line.values[j] << " in cut file" << endl;
				exit(-1);
			}
			indices.push_back(index);
			coefficients.push_back(1.0);
		}
		return;
	}

	vector<int> distances;
	for (int i = 0; i < line.clique_num_elements; ++i)
	{
		const int vertex = line.clique_values[i] - 1;
		if (vertex < 0 || vertex >= RAMSEY_instance->PARAM_SIZE_GRAPH)
		{
			cout << "Invalid clique vertex " << line.clique_values[i] << " in cut file" << endl;
			exit(-1);
		}
		for (int j = i + 1; j < line.clique_num_elements; ++j)
		{
			const int other_vertex = line.clique_values[j] - 1;
			if (other_vertex < 0 || other_vertex >= RAMSEY_instance->PARAM_SIZE_GRAPH)
			{
				cout << "Invalid clique vertex " << line.clique_values[j] << " in cut file" << endl;
				exit(-1);
			}
			distances.push_back(mapping_lin(RAMSEY_instance, vertex, other_vertex) + 1);
		}
	}
	vector<int> multiplicity(RAMSEY_instance->n_variable_MODEL_5, 0);
	for (size_t j = 0; j < distances.size(); ++j)
		multiplicity[distances[j] - 1]++;
	const int pair_count = distances.size();
	const int clique_size = (int)((1.0 + sqrt(1.0 + 8.0 * pair_count)) / 2.0 + 1e-9);
	if (clique_size * (clique_size - 1) / 2 != pair_count)
	{
		cout << "A distance-multiset row does not contain all clique pairs" << endl;
		exit(-1);
	}
	const int target = blue ? RAMSEY_instance->PARAM_M : RAMSEY_instance->PARAM_N;
	if (clique_size < target)
	{
		cout << "A recorded clique is smaller than its target" << endl;
		exit(-1);
	}
	const int edges = edge_number(clique_size);
	const int turan_rhs = blue ? ex_value(clique_size, target)
		: edges - ex_value(clique_size, target);
	const int unit_rhs = blue ? edges - (clique_size - target) - 1
		: clique_size - target + 1;
	rhs = RAMSEY_instance->PARAM_STRONGER_CUTS == 1 ? turan_rhs : unit_rhs;
	const int coefficient_cap = RAMSEY_instance->PARAM_STRONGER_CUTS == 1
		? (blue ? edges - turan_rhs : turan_rhs)
		: clique_size - target + 1;
	if (blue && RAMSEY_instance->PARAM_COVER_CUTS == 1)
	{
		rhs -= edges;
	}
	for (int index = 0; index < RAMSEY_instance->n_variable_MODEL_5; ++index)
	{
		if (multiplicity[index] == 0) continue;
		const double coefficient = RAMSEY_instance->PARAM_COVER_CUTS == 1
			? min(multiplicity[index], coefficient_cap) : multiplicity[index];
		indices.push_back(index);
		coefficients.push_back(coefficient);
		if (blue && RAMSEY_instance->PARAM_COVER_CUTS == 1)
		{
			rhs += coefficient;
		}
	}
}

int add_loaded_cut_pool(data *RAMSEY_instance, cut_data *cuts, bool blue, const char *name)
{
	if (!cuts->loaded) return 0;
	cout << "\nAdding cuts from " << name << "..." << endl;
	int added = 0;
	for (int i = 0; i < cuts->num_lines; ++i)
	{
		vector<int> indices;
		vector<double> coefficients;
		double rhs;
		char sense;
		prepare_loaded_cut(RAMSEY_instance, cuts->lines[i], blue, indices, coefficients, rhs, sense);
		RAMSEY_instance->rcnt = 1;
		RAMSEY_instance->nzcnt = indices.size();
		RAMSEY_instance->rhs = (double*) calloc(1, sizeof(double));
		RAMSEY_instance->sense = (char*) calloc(1, sizeof(char));
		RAMSEY_instance->rmatbeg = (int*) calloc(1, sizeof(int));
		RAMSEY_instance->rmatind = (int*) calloc(RAMSEY_instance->nzcnt, sizeof(int));
		RAMSEY_instance->rmatval = (double*) calloc(RAMSEY_instance->nzcnt, sizeof(double));
		RAMSEY_instance->rhs[0] = rhs;
		RAMSEY_instance->sense[0] = sense;
		for (int j = 0; j < RAMSEY_instance->nzcnt; ++j)
		{
			RAMSEY_instance->rmatind[j] = indices[j];
			RAMSEY_instance->rmatval[j] = coefficients[j];
		}
		RAMSEY_instance->status = CPXaddrows(RAMSEY_instance->env_MODEL_5, RAMSEY_instance->lp_MODEL_5,
				0, 1, RAMSEY_instance->nzcnt, RAMSEY_instance->rhs, RAMSEY_instance->sense,
				RAMSEY_instance->rmatbeg, RAMSEY_instance->rmatind, RAMSEY_instance->rmatval, NULL, NULL);
		if (RAMSEY_instance->status != 0)
		{
			cout << "error in CPXaddrows for " << name << endl;
			exit(-1);
		}
		added++;
		free(RAMSEY_instance->rmatbeg);
		free(RAMSEY_instance->rmatval);
		free(RAMSEY_instance->rmatind);
		free(RAMSEY_instance->rhs);
		free(RAMSEY_instance->sense);
	}
	cout << "Added " << added << " cuts from " << name << endl;
	return added;
}
}

/***********************************************************************************/
void add_cuts_from_file_MODEL_5(data *RAMSEY_instance)
/***********************************************************************************/
{
	const int cuts_M_added = add_loaded_cut_pool(RAMSEY_instance, &RAMSEY_instance->CUTS_M, true, "CUTS_M");
	const int cuts_N_added = add_loaded_cut_pool(RAMSEY_instance, &RAMSEY_instance->CUTS_N, false, "CUTS_N");
	if (cuts_M_added > 0 || cuts_N_added > 0)
	{
		cout << "\nTotal cuts added from files: " << (cuts_M_added + cuts_N_added) << endl;
	}
}


/***********************************************************************************/
void RAMSEY_MODEL_5_load(data *RAMSEY_instance)
/***********************************************************************************/
{


	/////////////////////////////////////////////
	RAMSEY_MODEL_5_allocation(RAMSEY_instance);
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
	//				cout <<  mapping_lin(RAMSEY_instance,i,j) << "\t";
	//
	//			}
	//			cout << endl;
	//		}
	//		cout << endl;
	//
	//
	//		exit(-1);


	/////////////////////////////////////////////////////////////////////////////////////////////////////////
	RAMSEY_instance->env_MODEL_5=CPXopenCPLEX(&(RAMSEY_instance->status));
	if(RAMSEY_instance->status!=0)
	{
		printf("cannot open CPLEX environment\n");
		exit(-1);
	}

	RAMSEY_instance->lp_MODEL_5=CPXcreateprob(RAMSEY_instance->env_MODEL_5,&(RAMSEY_instance->status),"RAMSEY");
	if(RAMSEY_instance->status!=0)
	{
		printf("cannot create problem\n");
		exit(-1);
	}
	/////////////////////////////////////////////////////////////////////////////////////////////////////////


	/////////////////////////////////////////////////////////////////////////////////////////////////////////
	RAMSEY_instance->ccnt=RAMSEY_instance->n_variable_MODEL_5;

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

	for(int i=0; i<RAMSEY_instance->n_variable_MODEL_5; i++)
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


	RAMSEY_instance->status=CPXnewcols(RAMSEY_instance->env_MODEL_5,RAMSEY_instance->lp_MODEL_5,RAMSEY_instance->ccnt,RAMSEY_instance->obj,RAMSEY_instance->lb,RAMSEY_instance->ub,RAMSEY_instance->xctype,RAMSEY_instance->colname);
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
	initialize_cut_recording(RAMSEY_instance);
	add_cuts_from_file_MODEL_5(RAMSEY_instance);
	///////////////////////////////////////////////////////////////////////////////////////////////////////////
	///////////////////////////////////////////////////////////////////////////////////////////////////////////

	RAMSEY_instance->TRIANGLES_CUTS_M = 0;
	RAMSEY_instance->TRIANGLES_CUTS_N = 0;

	////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// Optional constraint families, distance geometry.
	//
	// A vertex set S of the interval {0,...,t-1} may be translated so that min S = 0 (a
	// distance-preserving injection into the same interval), so every family below is enumerated
	// with 0 as the smallest vertex.  D(S) is the multiset of linear distances inside S; a blue
	// K_M is excluded by  sum_{d in D(S)} c_d y_d <= (sum c_d) - 1  and a red K_N by
	// sum_{d in D(S)} c_d y_d >= 1, with c_d the multiplicity (PARAM_COVER_CUTS = 1 replaces the
	// multiplicities by the stronger support form, exactly as in MODEL 3).
	// No distance is ever folded: mapping_h and modulo play no role here.
	////////////////////////////////////////////////////////////////////////////////////////////////////////////

	////////////////////////////////////////////////////////////////////////////////////////
	// Triangles: S = {0, s, s+u}, distances s, u, s+u  (s <= u, s+u <= t-1)
	////////////////////////////////////////////////////////////////////////////////////////
	if(RAMSEY_instance->AVOID_TRIANGLES==1)
	{
		const int nv = RAMSEY_instance->n_variable_MODEL_5;
		const int tt = RAMSEY_instance->PARAM_SIZE_GRAPH;

		if(RAMSEY_instance->PARAM_M==3)
		{
			int cons_counter=0;
			RAMSEY_instance->SKIP_M_SEPARATION=true;
			for(int s=1; s<=tt-2; s++)
			{
				for(int u=s; s+u<=tt-1; u++)
				{
					int dd[3]={s,u,s+u};
					if(!row_from_distances_MODEL_5(RAMSEY_instance,dd,3,true)){ continue; }
					cons_counter++;
				}
			}
			cout << "SKIP_M_SEPARATION FLAG->\t" << RAMSEY_instance->SKIP_M_SEPARATION << "\t cons_counter \t" << cons_counter << endl;
			RAMSEY_instance->TRIANGLES_CUTS_M=cons_counter;
		}

		if(RAMSEY_instance->PARAM_N==3)
		{
			int cons_counter=0;
			RAMSEY_instance->SKIP_N_SEPARATION=true;
			for(int s=1; s<=tt-2; s++)
			{
				for(int u=s; s+u<=tt-1; u++)
				{
					int dd[3]={s,u,s+u};
					if(!row_from_distances_MODEL_5(RAMSEY_instance,dd,3,false)){ continue; }
					cons_counter++;
				}
			}
			cout << "SKIP_N_SEPARATION FLAG->\t" << RAMSEY_instance->SKIP_N_SEPARATION << "\t cons_counter \t" << cons_counter << endl;
			RAMSEY_instance->TRIANGLES_CUTS_N=cons_counter;
		}
		(void)nv;
	}

	RAMSEY_instance->QUADRANGLES_CUTS_M=0;
	RAMSEY_instance->QUADRANGLES_CUTS_N=0;

	////////////////////////////////////////////////////////////////////////////////////////
	// Quadrangles: S = {0, s, s+u, s+u+v}, six distances  (s <= v to skip the mirror)
	////////////////////////////////////////////////////////////////////////////////////////
	if(RAMSEY_instance->AVOID_QUADRANGLES==1)
	{
		const int tt = RAMSEY_instance->PARAM_SIZE_GRAPH;

		if(RAMSEY_instance->PARAM_M==4)
		{
			int cons_counter=0;
			RAMSEY_instance->SKIP_M_SEPARATION=true;
			for(int s=1; s<=tt-3; s++)
			{
				for(int u=1; s+u<=tt-2; u++)
				{
					for(int v=s; s+u+v<=tt-1; v++)
					{
						int dd[6]={s,u,v,s+u,u+v,s+u+v};
						if(!row_from_distances_MODEL_5(RAMSEY_instance,dd,6,true)){ continue; }
						cons_counter++;
					}
				}
			}
			cout << "SKIP_M_SEPARATION FLAG->\t" << RAMSEY_instance->SKIP_M_SEPARATION << "\t cons_counter \t" << cons_counter << endl;
			RAMSEY_instance->QUADRANGLES_CUTS_M=cons_counter;
		}

		if(RAMSEY_instance->PARAM_N==4)
		{
			int cons_counter=0;
			RAMSEY_instance->SKIP_N_SEPARATION=true;
			for(int s=1; s<=tt-3; s++)
			{
				for(int u=1; s+u<=tt-2; u++)
				{
					for(int v=s; s+u+v<=tt-1; v++)
					{
						int dd[6]={s,u,v,s+u,u+v,s+u+v};
						if(!row_from_distances_MODEL_5(RAMSEY_instance,dd,6,false)){ continue; }
						cons_counter++;
					}
				}
			}
			cout << "SKIP_N_SEPARATION FLAG->\t" << RAMSEY_instance->SKIP_N_SEPARATION << "\t cons_counter \t" << cons_counter << endl;
			RAMSEY_instance->QUADRANGLES_CUTS_N=cons_counter;
		}
	}

	////////////////////////////////////////////////////////////////////////////////////////
	// Arithmetic-progression cliques: S = {0, s, 2s, ..., (k-1)s}, all its pairwise distances
	// are multiples of s, so  sum_{j=1}^{k-1} y_{js} <= k-2  (blue) and >= 1 (red).
	////////////////////////////////////////////////////////////////////////////////////////
	if(RAMSEY_instance->PARAM_CLIQUE_JUMP_CUTS==1)
	{
		const int tt = RAMSEY_instance->PARAM_SIZE_GRAPH;
		int count_blue=0, count_red=0;

		if(RAMSEY_instance->SKIP_M_SEPARATION==false)
		{
			for(int s=1; s*(RAMSEY_instance->PARAM_M-1)<=tt-1; s++)
			{
				vector<int> ind; vector<double> val;
				for(int j=1;j<=RAMSEY_instance->PARAM_M-1;j++){ ind.push_back(j*s-1); val.push_back(1.0); }
				add_row_MODEL_5(RAMSEY_instance,ind,val,'L',RAMSEY_instance->PARAM_M-2);
				count_blue++;
			}
		}
		if(RAMSEY_instance->SKIP_N_SEPARATION==false)
		{
			for(int s=1; s*(RAMSEY_instance->PARAM_N-1)<=tt-1; s++)
			{
				vector<int> ind; vector<double> val;
				for(int j=1;j<=RAMSEY_instance->PARAM_N-1;j++){ ind.push_back(j*s-1); val.push_back(1.0); }
				add_row_MODEL_5(RAMSEY_instance,ind,val,'G',1.0);
				count_red++;
			}
		}
		cout << "\nCLIQUE_JUMP_CUTS (arithmetic progressions): blue " << count_blue
		     << "\tred " << count_red << endl;
	}

	////////////////////////////////////////////////////////////////////////////////////////
	// Degree cuts.  The blue neighbourhood of vertex i is  {i-d : d in D, d <= i} u
	// {i+d : d in D, d <= t-1-i}, so its size is  sum_{d<=i} y_d + sum_{d<=t-1-i} y_d.  It
	// contains no blue K_{M-1} and no red K_N, hence at most R(M-1,N)-1 vertices; the red
	// neighbourhood is the complement inside the t-1 available distances.  Unlike the circulant
	// model every vertex gives a different row, and i = (t-1)/2 gives the strongest one.
	// CAVEAT (identical to MODEL 3): with PARAM_K_CUTS = 2 the look-up table holds LOWER bounds,
	// so that setting is a heuristic restriction of the search space, not an exact model.
	////////////////////////////////////////////////////////////////////////////////////////
	if(RAMSEY_instance->PARAM_K_CUTS>=1)
	{
		const int tt = RAMSEY_instance->PARAM_SIZE_GRAPH;
		const int nv = RAMSEY_instance->n_variable_MODEL_5;
		int count_blue=0, count_red=0;

		int R_blue = RAMSEY_instance->LOOK_UP_RAMSEY[RAMSEY_instance->PARAM_M-1][RAMSEY_instance->PARAM_N];
		int R_red  = RAMSEY_instance->LOOK_UP_RAMSEY[RAMSEY_instance->PARAM_M][RAMSEY_instance->PARAM_N-1];

		if(RAMSEY_instance->PARAM_K_CUTS==2)
		{
			cout << "\n**WARNING** PARAM_K_CUTS = 2 uses LOWER bounds: the model is a restriction,"
			     << " an EMPTY answer is not a proof\n";
		}

		if(R_blue!=-1)
		{
			cout << "\nLOOK_UP_RAMSEY BLUE\t" << R_blue << endl;
			int cap = min(R_blue, tt);
			for(int i=0; 2*i<=tt-1; i++)
			{
				int hi_a = i, hi_b = tt-1-i;
				vector<int> ind; vector<double> val;
				for(int d=1; d<=nv; d++)
				{
					double c = (d<=hi_a ? 1.0 : 0.0) + (d<=hi_b ? 1.0 : 0.0);
					if(c>0.0){ ind.push_back(d-1); val.push_back(c); }
				}
				add_row_MODEL_5(RAMSEY_instance,ind,val,'L',cap-1);
				count_blue++;
			}
		}
		if(R_red!=-1)
		{
			cout << "\nLOOK_UP_RAMSEY RED\t" << R_red << endl;
			int cap = min(R_red, tt);
			for(int i=0; 2*i<=tt-1; i++)
			{
				int hi_a = i, hi_b = tt-1-i;
				vector<int> ind; vector<double> val;
				double tot = 0.0;
				for(int d=1; d<=nv; d++)
				{
					double c = (d<=hi_a ? 1.0 : 0.0) + (d<=hi_b ? 1.0 : 0.0);
					if(c>0.0){ ind.push_back(d-1); val.push_back(c); tot += c; }
				}
				double rhs = tot - (double)(cap-1);
				if(rhs < 1.0){ rhs = 1.0; }
				add_row_MODEL_5(RAMSEY_instance,ind,val,'G',rhs);
				count_red++;
			}
		}
		cout << "\nDEGREE_CUTS: blue " << count_blue << "\tred " << count_red << endl;
	}

	cout << "\n\n****MAXIMIZATION\n\n";
	CPXchgobjsen(RAMSEY_instance->env_MODEL_5,RAMSEY_instance->lp_MODEL_5,CPX_MAX);

	//	cout << "\n\n****MINIMIZATION\n\n";
	//	CPXchgobjsen(RAMSEY_instance->env_MODEL_5,RAMSEY_instance->lp_MODEL_5,CPX_MIN);


#ifdef	PRINT_MODEL_5_LP
	/////////////////////////////////////////////////////////////////////////////////////////////////////////
	// * writing the created ILP model on a file *
	RAMSEY_instance->status=CPXwriteprob(RAMSEY_instance->env_MODEL_5,RAMSEY_instance->lp_MODEL_5,"RAMSEY_MODEL_5.lp",NULL);
	if(RAMSEY_instance->status!=0)
	{
		printf("error in CPXwriteprob\n");
		exit(-1);
	}
	cout << "INITIAL MASTER WRITTEN\n";
	/////////////////////////////////////////////////////////////////////////////////////////////////////////
#endif


}
