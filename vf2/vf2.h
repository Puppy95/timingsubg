#ifndef _SJTREE_H_
#define _SJTREE_H_

#include "../timing/query.h"
#include "../util/list_T.h"
#include "../util/util.h"
#include "../timing/edge.h"
class timingconf;
class query;
class Vmatch;
class redCC;

class vf2{
public:
	vf2(timingconf* _tconf, query* _q);
	~vf2();

	void initial();

	/*  */
	bool getMatEdges(dEdge* _e, vector<qEdge*>& _match_edges);

	/*  */
	bool insert(dEdge* _e, qEdge* _qe);
	/*  */
	bool remove(dEdge* _e, qEdge* _qe);

	long long int to_size();

private:
	/* add _vm into adjacent_list */
	bool add(Vmatch* _vm);
	
	/* update color of _vm and all adjacent vm
	 * for insertion
	 * */
	bool insert_color(Vmatch* _vm);

	/* remove _vm from adjacent_list
	 * return removed Vmatch 
	 * */
	Vmatch* del(dEdge* _e, qEdge* _qe);

	/* update color of _vm and all adjacent vm */
	bool remove_color(Vmatch* _vm);

	/* return whether _m should be red */
	bool check_red(Vmatch* _vm);

	bool isomorphism(Vmatch* _vm, vector<match>& _new_match);
	bool isomorphism(match& _m, int _depth, qEdge** _order, set<Vmatch*>& _neighbor, vector<match>& _new_match);
	void new_neighbor(set<Vmatch*>& _old, dEdge* _e, set<Vmatch*>& _new);
	bool answer_expired(Vmatch* _vm, vector<match>& _exp_match);

	/* 
	 * turn _m into red
	 * check merge redCC: getNeighborRedCCs(), mergeRedCC()
	 *
	 *
	 *		each merge may concern more than two CC
	 * color reachable matches in turn_red();
	 * recursively call turn_red();
	 * use visited_mat to avoid dup computing
	 * */
	bool turn_red(Vmatch* _vm, set<Vmatch*>& _visited_mat);
	bool un_red(Vmatch* _vm, set<Vmatch*>& _visited_mat);

	/* 
	 * */
	string new_match_str(vector<match>& _mat_vec);
	string expired_match_str(vector<match>& _mat_vec);
	static string matches_str(vector<match>& _mat_vec);
	/* 
	 * join _matches with those in _node 
	 * results is put into _join_result
	 * */
	bool join(vector<Vmatch>& _mat1, vector<Vmatch>& _mat2, vector<match>& _join_result);
	bool join(vector<match>& _mat1, vector<match>& _mat2, vector<match>& _join_result);


	bool getAdjVmat(dEdge* _e, vector<Vmatch*>& _adj_vmat);
	bool getAdjSmat(dEdge* _e, set<Vmatch*>& _adj_smat);
	bool getAdjVMedge(dEdge* _e, set<qEdge*>& _adj_edges);
	vector<Vmatch*>* getAdjVec(int _i);

	string vf2_str();
	string q2list_str();
	void build_edgelist();

	qEdge** get_order(qEdge* _qe);
	void initial(map<qEdge*, bool>& _v_qe);
	qEdge* next_qe(map<qEdge*, bool>& _v_qe);

	query* q;
	timingconf* tconf;
	map<int, vector<Vmatch*> > adjacent_list;	
	map<qEdge*, set<qEdge*> > qedge_neigbor;
	vector<match> answers;
};

#endif
