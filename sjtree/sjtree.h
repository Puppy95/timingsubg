#ifndef _SJTREE_H_
#define _SJTREE_H_

#include "../timing/query.h"
#include "../util/list_T.h"
#include "../util/util.h"
#include "../timing/edge.h"
#include "sjNode.h"
//in msNode.h: typedef pair<msNode*, List<match>* > JoinResult;
class timingconf;
class query;
class match;

class sjtree{
public:
	sjtree(string _f_e2freq, query* _q);
	~sjtree();

	void initial();

	long long int to_size();

	/*  */
	bool getSJlist(dEdge* _e, vector<sjNode*>& _sj_vec, vector<qEdge*>& _match_edges);

	/*  */
	bool remove(dEdge* _e, sjNode* _node);
	/*  */
	bool insert(dEdge* _e, qEdge* _qe);


private:
	/* 
	 * */
	string new_match_str(vector<match>& _mat_vec);
	string expired_match_str(vector<match>& _mat_vec);
	static string matches_str(vector<match>& _mat_vec);
	/* 
	 * join _matches with those in _node 
	 * results is put into _join_result
	 * */
	bool join(vector<match>& _matches, sjNode* _node,  vector<match>& _join_result);

	string sjtree_str();
	string q2sj_str();
    void order_selection(string _f_e2freq, vector<qEdge*>& _query_edges);
	void build_sj_tree();


	query* q;
	timingconf* tconf;
	sjNode* sjroot;
	map<qEdge*, sjNode*> edge2node;	
    string f_e2freq;
	/* should be saved by mutex */
};

#endif
