#ifndef _SJTREESUBG_H_
#define _SJTREESUBG_H_

#include "../timing/edge.h"
#include "../util/util.h"
class timingconf;
class sjtree;
class query;
class sjNode;
class gstream;
class gtransaction;


class sjtreesubg{
public:
	sjtreesubg(int _winsz, string _config="defaultconfg");
	sjtreesubg();
	~sjtreesubg();
	void initial();

	void run(int _mode, gstream* _G, query* Q, timingconf* _tconf, string _f_e2freq);
	bool new_edge(dEdge* _e);
	bool expire_edge(dEdge* _e);
	bool check_expire_edge(dEdge* _newest);
	bool tuple_expire(dEdge* _e);
	timingconf* getconf();

	string exename;
	string stat_str();
	bool write_stat();

private:

	/* 
	 * path_dataset
	 * path_vertex2id
	 * path_vlabel2id
	 * path_elabel2id
	 * vNum
	 * eNum
	 * Max_Thread_Num 
	 * Max_Query_Size
	 * 
	 */

	gtransaction* next_tran();

	int win_size;
	queue<pair<dEdge*, bool> > cur_edges;
	timingconf* tconf;
	sjtree* S;
	gstream* G;
	query* Q;



	int seen_eNum;
	int unmat_eNum;
	int del_eNum;
	int gap_log;
	vector<qEdge*> cacheMatEdge;
	vector<sjNode*> cacheSJ;
	string fruntime;

	double SUMtime;
	double AVGtime;
	long long int AVGspace;
	long long int AVGnoMS;
	long long int Throughput;

};


#endif 
