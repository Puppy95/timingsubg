#ifndef _SJTREESUBG_H_
#define _SJTREESUBG_H_

#include "../timing/edge.h"
#include "../util/util.h"
class timingconf;
class vf2;
class query;
class gstream;
class gtransaction;


class vf2subg{
public:
	vf2subg(int _winsz, string _runtime);
	vf2subg();
	~vf2subg();
	void initial();

	void run(int _mode, gstream* _G, query* Q, timingconf* _tconf);
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

	int win_size;
	queue<pair<dEdge*, bool> > cur_edges;
	timingconf* tconf;
	vf2* V;
	gstream* G;
	query* Q;



	int seen_eNum;
	int unmat_eNum;
	int del_eNum;
	int gap_log;
	vector<qEdge*> cacheMatEdge;
	string fruntime;

	double SUMtime;
	double AVGtime;
	long long int AVGspace;
	long long int AVGnoMS;
	long long int Throughput;

};


#endif 
