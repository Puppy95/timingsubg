#include "vf2subg.h"
#include "vf2.h"
#include "../timing/timingconf.h"
#include "../util/runtime.h"
#include "../util/util.h"
#include "../msforest/msforest.h"
#include "../gstream/gstream.h"
#include "../msforest/nodeOP.h"
#include "../msforest/teNode.h"


vf2subg::vf2subg(int _winsz, string _runtime)
{
/*
	if(_winsz != 0)
		this->win_size = _winsz;
*/	
	this->fruntime = _runtime;
	this->AVGnoMS = 0;
}

vf2subg::vf2subg()
{
	
}

vf2subg::~vf2subg()
{

}


void vf2subg::initial(){
	
}

void vf2subg::run(int _mode, gstream* _G, query* _Q, timingconf* _tconf){
	util::t2count.clear();
	util::reg_track(-10000);

#ifdef DEBUG_TRACK
	util::track("\nIn run@vf2subg");
	util::track("query:= " + _Q->to_str());
#endif

#ifdef GLOBAL_COMMENT
	cout << "In run@vf2subg";
	cout << ", mode=" << _mode << endl;
#endif
	/* initial */
	this->win_size = _tconf->getwinsz();
#ifdef GLOBAL_COMMENT
	cout << "win_size = " << this->win_size << endl;
#endif
	this->G = _G;
	this->G->load_edges(this->win_size);
	this->Q = _Q;
	this->tconf = _tconf;

	this->V = new vf2(this->tconf, this->Q);

#ifdef DEBUG_TRACK
	util::track("\nnew vf2");
#endif

#ifdef DEBUG_TRACK
#endif

	
#ifdef DEBUG_TRACK
	util::track("\nreset gstream");
#endif

	this->G->reset();
	this->cacheMatEdge.clear();

#ifdef DEBUG_TRACK
	util::track("stream runs...");	
#endif

#ifdef GLOBAL_COMMENT
	cout << "MAIN***** stream runs..." << endl;
#endif
	this->seen_eNum = 0;
	this->unmat_eNum = 0;
	this->del_eNum = 0;
	this->gap_log = this->G->size() / 1000;
	if(this->gap_log < 10000) this->gap_log = 10000;

	/* stream runs */
	runtime _rtime;
	runtime _whole;
	_rtime.initial();
	_whole.initial();

	bool _new_ret = false;
	long long int _space = 0;
	long long int _total_space = 0;
	long long int _avg_space = 0;
	/*  */
	while(this->G->hasnext())
	{
		this->seen_eNum ++;
		if(this->seen_eNum == this->win_size)
		{
			_whole.begin();
		}
		if(this->seen_eNum % (this->gap_log) == 0) 
		{
			cout << "[" << this->seen_eNum/1000 << "k, u" << this->unmat_eNum;
			cout << ", m" << (this->seen_eNum-this->unmat_eNum) << ", ";
			cout << "d" << (this->del_eNum) << ", w" << this->cur_edges.size();
			cout << "]  ";
		}
		
		if(this->seen_eNum % (5*this->gap_log) == 0)
		{
#ifdef GLOBAL_COMMENT
			cout << "\navg: " << _rtime.getavg();
			cout << "\tsum:"<< _rtime.getsum() <<  endl;
#endif
		}
		cout.flush();
	
		_rtime.begin();

		dEdge* _e = this->G->next();
		_new_ret = this->new_edge(_e);		
		if(_new_ret){
			this->cur_edges.push(pair<dEdge*, bool>(_e, true));
		}else{
			this->cur_edges.push(pair<dEdge*, bool>(_e, false));
		}


#if (defined SPAN)
		this->check_expire_edge(_e);
#endif

#if (defined I_AND_D) 
		this->expire_edge(_e);
#if (defined SPAN)
		"err: I_AND_D should not be defined"		
#endif
#endif

#if (defined TUPLE) 
		this->tuple_expire(_e);
#if (defined I_AND_D) || (defined SPAN)
		"err: TUPLE should not be defined"		
#endif

#endif

		_rtime.end();
		if(this->seen_eNum >= this->win_size)
		{
			_space = this->V->to_size();
			_total_space += _space;
		}
	}

#ifdef GLOBAL_COMMENT
	cout << "MAIN***** Finish vfstream, start to remove remaining edges" << endl;
#endif
	while(! this->cur_edges.empty())
	{
		if(this->cur_edges.front().second == true){
			dEdge* _e_rm = this->cur_edges.front().first;
			this->expire_edge(_e_rm);
		}
		
		this->cur_edges.pop();
	}

	_whole.end();
	double _sum_time = _whole.getsum()/1000.0;
	int _runtimes = this->seen_eNum-this->win_size;
	double _avg_time = _sum_time / _runtimes;
	_avg_space = _total_space / _runtimes;
	long long int _throughput = 1000 / _avg_time;
	int threadNum = 1;

	this->AVGtime = _avg_time;
	this->SUMtime = _sum_time;
	this->AVGspace = _avg_space/1000.0;
	this->Throughput = _throughput;

	delete this->V;

#ifdef GLOBAL_COMMENT
	cout << endl << "MAIN***** OUT stream runs@vf2subg..." << endl << endl;
#endif
}

bool vf2subg::new_edge(dEdge* _e)
{
#ifdef DEBUG_TRACK 
		{
			stringstream _ss;
			_ss << "\n====================" << this->seen_eNum << " = " << _e->to_str() << endl;
			util::track(_ss);
#ifdef STD_MAIN	
			cout << _ss.str() << endl;
#endif
		}
#endif
		this->V->getMatEdges(_e, this->cacheMatEdge);

		if(this->cacheMatEdge.empty())
		{
			this->unmat_eNum ++;
			return false;;
		}
#ifdef DEBUG_TRACK
		{
			stringstream _ss;
			_ss << "At count=" << this->seen_eNum;
			_ss << ": match " << this->cacheMatEdge.size() << " edges" << endl;

			for(int i = 0; i < (int)this->cacheMatEdge.size(); i ++)
			{
				_ss << i << ": " << this->cacheMatEdge[i]->to_str() << endl;
			}
			
			util::track(_ss);
#ifdef STD_MAIN	
			cout << _ss.str() << endl;
#endif
		}
#endif
		for(int i = 0; i < (int)this->cacheMatEdge.size(); i ++)
		{

#ifdef DEBUG_TRACK
			{
				stringstream _ss;
				_ss << "\n*****start match " << i << endl;
				util::track(_ss);
#ifdef STD_MAIN
				cout << "Main: " << _ss.str() << endl;
#endif
			}
#endif


			this->V->insert(_e, this->cacheMatEdge[i]);

			{
#ifdef DEBUG_TRACK
				stringstream _ss;
				_ss << "finish [" << _e->to_str() << ",";
				_ss << this->cacheMatEdge[i]->to_str() << "," <<  "]" << endl;
				util::track(_ss);
#ifdef STD_MAIN
				cout << _ss.str() << endl;
#endif
#endif
			}

		}
	return true;
}

bool vf2subg::expire_edge(dEdge* _e)
{
#ifdef DEBUG_TRACK
	util::track("IN expire_edge");
#endif
	this->V->getMatEdges(_e, this->cacheMatEdge);
#ifdef DEBUG_TRACK
	{
		stringstream _ss;
		_ss << "There are " << this->cacheMatEdge.size() << " qedges for ";
		_ss << _e->to_str() << endl;
		util::track(_ss);
	}
#endif
	for(int i = 0; i < (int)this->cacheMatEdge.size(); i ++)
	{
#ifdef DEBUG_TRACK
		{
			stringstream _ss;
			_ss << "expire[" << i << "]:" << endl;
			_ss << "\t[" << this->cacheMatEdge[i]->to_str() << "]";
			_ss << " -> " << "[" << _e->to_str() << "]" << endl;
			util::track(_ss);
		}
#endif

		this->V->remove(_e, this->cacheMatEdge[i]);

	}

#ifdef DEBUG_TRACK
	util::track("OUT expire_edge");
#endif
	return true;
}

bool vf2subg::check_expire_edge(dEdge* _newest)
{
	bool _expired = false;
	while(! this->cur_edges.empty())
	{
		if(this->G->is_expire(this->cur_edges.front().first, _newest))
		{
			if(this->cur_edges.front().second == true)
				this->expire_edge(this->cur_edges.front().first);

			this->cur_edges.pop();
			_expired = true;
			this->del_eNum ++;
		}
		else
		{
			break;
		}
	}

	bool win_limit= false;
#ifdef WIN_LIMIT
	win_limit =((int)this->cur_edges.size()) > this->win_size*2;
#endif

	if(! _expired && win_limit)
	{
			if(this->cur_edges.front().second == true)
				this->expire_edge(this->cur_edges.front().first);

			this->cur_edges.pop();
			_expired = true;
			this->del_eNum ++;
	}

	return _expired;
}
	
bool vf2subg::tuple_expire(dEdge* _e)
{
	if(this->win_size < (int)this->cur_edges.size())
	{
		if(this->cur_edges.front().second == false)
		{
			this->cur_edges.pop();
#ifdef DEBUG_TRACK
			{
				stringstream _ss;
				_ss << "expire NULL AT " << this->seen_eNum << endl;
				util::track(_ss);
#ifdef STD_MAIN 
				cout << _ss.str() << endl;
#endif
			}
#endif
			return true;
		}
#ifdef DEBUG_TRACK
		{
			stringstream _ss;
			_ss << "AT " << this->seen_eNum << " expire edge: " << this->cur_edges.front().first->to_str() << endl;
			util::track(_ss);
#ifdef STD_MAIN 
				cout << _ss.str() << endl;
#endif
		}
#endif

		this->expire_edge(this->cur_edges.front().first);
		this->cur_edges.pop();
		return _e != NULL ;
	}
	return false;
}

timingconf* vf2subg::getconf()
{
	return this->tconf;
}

string vf2subg::stat_str()
{
	stringstream _ss;
	_ss << this->exename;
	_ss << "\t"  << fixed<< this->win_size;
	_ss << "\t"  << fixed<< this->AVGtime;
	_ss << "\t"  << fixed<< this->Throughput;
	_ss << "\t"  << fixed<< this->AVGspace;
	_ss << "\t"  << fixed<< this->AVGnoMS;
	_ss << "\t"  << fixed<< this->SUMtime;
	_ss << "\t"  << fixed<< this->tconf->getmaxthreadNum();
	_ss << "\t"  << fixed<< this->Q->param_str();
	_ss << "\t"  << fixed<< this->seen_eNum;

	return _ss.str();
}

bool vf2subg::write_stat()
{
	fstream frun(this->fruntime.c_str(), ios::app|ios::out);
	if(!frun){
		cout << "err: " << this->fruntime << endl;
		exit(-1);
	}
	
	if((int)frun.tellg() < 10)
	{
		frun << "exe\t\t\tWinSize\tAVGtime\tThroughput\tAVGspace\tAVGnoms\tSUMtime\tThread\tQsize\tQno\ttimingratio\teNum" << endl;
	}

	frun << this->stat_str() << endl;
	frun.flush();
	frun.close();
	return true;
}
