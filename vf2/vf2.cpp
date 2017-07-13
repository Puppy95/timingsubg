#include "vf2.h"
#include "Vmatch.h"
#include "../timing/match.h"
#include "../timing/query.h"
#include "../timing/timingconf.h"

vf2::vf2(timingconf* _tconf, query* _q)
{
	this->q = _q;
	this->tconf = _tconf;
	this->adjacent_list.clear();
	this->answers.clear();
	this->initial();
}

vf2::~vf2()
{
	cout << "\nIN destruct vf2..." << endl;

#ifdef DEBUG_TRACK
	util::track("IN destruct vf2");
#endif

	cout << "OUT destruct vf2..." << endl;

#ifdef DEBUG_TRACK
	util::track("OUT destruct vf2");
#endif

}

void vf2::initial()
{
	this->build_edgelist();
#ifdef DEBUG_TRACK
	util::track("vf2:\n"+ this->vf2_str());
#endif
}

/*  */
bool vf2::getMatEdges(dEdge* _e, vector<qEdge*>& _match_edges)
{
#ifdef DEBUG_TRACK
	util::track("IN getMat");
#endif
	this->q->get_matches(_e, _match_edges);
#ifdef DEBUG_TRACK
	util::track("OUT getMat");
#endif
	return true;
}

/* 
 * 
 * */
bool vf2::insert(dEdge* _e, qEdge* _qe)
{
	Vmatch* _vm = new Vmatch(_qe, _e);
#ifdef DEBUG_TRACK
	util::track("Insert match: "+_vm->to_str());
	util::track(this->vf2_str());
#endif

	/* add match into adjacent_list */
	this->add(_vm);
#ifdef COLORING
	this->insert_color(_vm);

#ifdef REDCC
	if(_vm->is_red)
	{
		/* call turn red 
		 * update redCC in turn_red();
		 * each merge may concern more than two CC
		 * color reachable matches in turn_red();
		 * recursively call turn_red();
		 * use visited_mat to avoid dup computing
		 * */
		set<Vmatch*> _visited_mat;
		_visited_mat.clear();
		this->turn_red(_m, _visited_mat);
	}

	bool _full_red =false;
	/* Check fullness of redCC vector */
	
	if(! _full_red) return false;
#else
	if(_vm->is_red == false) return false;
#endif

#endif

#ifdef VF2OPT
    if(! this->q->is_rightmost(_qe))
    {
        return false;
    }
#endif

	/* run vf2  */
	vector<match> _new_match;
	this->isomorphism(_vm, _new_match);	

	if(_new_match.empty() == false)
	{
		this->new_match_str(_new_match);
		this->answers.insert(this->answers.begin(), _new_match.begin(), _new_match.end());
	}
#ifdef DEBUG_TRACK
	util::track("OUT Insert match: "+_vm->to_str());
	util::track(this->vf2_str());
#endif
	return true;
}

/*  */
bool vf2::remove(dEdge* _e, qEdge* _qe){
	match _m(_qe, _e);
#ifdef DEBUG_TRACK
	util::track("Remove match: "+_m.to_str());
	util::track(this->vf2_str());
#endif
	
	Vmatch* _vm = NULL;
	/* find and remove the exact Vmatch */
	_vm = this->del(_e, _qe);	
	/*  */
#ifdef COLORING
	bool _is_red = _vm->is_red;
	/* update color of _vm and all adjacent vm */
	this->remove_color(_vm);
#ifdef REDCC
	if(_vm->is_red)
	{
		/* update reachable Vmatches */	

		/* decompose redCC :
		 *  we can recompute over remaining red Vmatch
		 *  build a set in redCC
		 * */
	}
#endif
	if(! _is_red) return false;
#endif
	vector<match> _exp_ans;
	_exp_ans.clear();
	this->answer_expired(_vm, _exp_ans);

	if(_exp_ans.empty() == false)
	{
		this->expired_match_str(_exp_ans);
	}
	
#ifdef DEBUG_TRACK
	util::track("OUt remove e qe");
	util::track(this->vf2_str());
#endif
	return true;
}
	
long long int vf2::to_size()
{
	int map_multi = 1;
	long long int map_sz = 0;
	long long int sz = sizeof(vf2);

	map<int, vector<Vmatch*> >::iterator itr;
	itr = this->adjacent_list.begin();
	int vsz = sizeof(Vmatch*)+Vmatch::to_size();
	while(itr != this->adjacent_list.end())
	{
		map_sz += sizeof(int);
		map_sz += itr->second.size() * vsz / 2;
		itr ++;
	}
	sz += map_sz * map_multi;/* map multi */

	vector<match>::iterator vitr = this->answers.begin();
	while(vitr != this->answers.end())
	{
		sz += (*vitr).to_size();
		vitr ++;
	}

	return sz;
}



/* add _vm into adjacent_list */
bool vf2::add(Vmatch* _vm)
{
#ifdef DEBUG_TRACK
	util::track("IN add _vm");
#endif
	vector<Vmatch*>* _vs = this->getAdjVec(_vm->e->s);
	vector<Vmatch*>* _vt = this->getAdjVec(_vm->e->t);
	_vs->push_back(_vm);
	_vt->push_back(_vm);

#ifdef DEBUG_TRACK
	util::track("OUT add _vm");
#endif
	return true;
}

/* update color of _vm and all adjacent vm
 * for insertion
 * */
bool vf2::insert_color(Vmatch* _vm)
{
#ifdef DEBUG_TRACK
	util::track("IN insert_color: "+_vm->to_str());
#endif
	bool _isred = this->check_red(_vm);
	_vm->is_red = _isred;

	vector<Vmatch*> _adj_vmat;
	this->getAdjVmat(_vm->e, _adj_vmat);
	for(int i = 0; i < (int)_adj_vmat.size(); i ++)
	{
		if(_adj_vmat[i]->is_red == true) continue;

		_adj_vmat[i]->is_red = this->check_red(_adj_vmat[i]);

#ifdef DEBUG_TRACK
		{
			
		}
#endif
	}

#ifdef DEBUG_TRACK
	util::track("OUT insert_color");
#endif
	return true;
}

/* remove _vm from adjacent_list
 * return removed Vmatch 
 * */
Vmatch* vf2::del(dEdge* _e, qEdge* _qe)
{
#ifdef DEBUG_TRACK
	util::track("IN del");
#endif
	vector<Vmatch*>* _vs = this->getAdjVec(_e->s);
	vector<Vmatch*>* _vt = this->getAdjVec(_e->t);
	
	Vmatch* _ret = NULL;
	vector<Vmatch*>::iterator itr;
	for(itr = _vs->begin(); itr != _vs->end(); ++itr)
	{
		if((*itr)->exact(_qe, _e))
		{
			_ret = (*itr);
			_vs->erase(itr);
			break;
		}
	}
	if(_ret == NULL){
		cout << "err ret Vmat not exist" << endl;
		exit(-1);
	}

	for(itr = _vt->begin(); itr != _vt->end(); ++itr)
	{
		if((*itr)->exact(_qe, _e))
		{
			if(_ret != (*itr)){
				cout << "err ret not equal" << endl;
				exit(-1);
			}
			_vt->erase(itr);
			break;
		}
	}
#ifdef DEBUG_TRACK
	util::track("OUt del");
#endif

	return _ret;
}

/* update color of _vm and all adjacent vm */
bool vf2::remove_color(Vmatch* _vm)
{
#ifdef DEBUG_TRACK
	util::track("IN remove_color");
#endif
	vector<Vmatch*> _adj_vmat;
	this->getAdjVmat(_vm->e, _adj_vmat);
	for(int i = 0; i < (int)_adj_vmat.size(); i ++)
	{
		if(_adj_vmat[i]->is_red == false) continue;

		_adj_vmat[i]->is_red = this->check_red(_adj_vmat[i]);
	}
	
#ifdef DEBUG_TRACK
	util::track("OUT remove_color");
#endif
	return true;
}

bool vf2::check_red(Vmatch* _vm)
{
#ifdef DEBUG_TRACK
	util::track("IN check_red: "+ _vm->to_str());
	bool _ori = _vm->is_red;
#endif
	set<qEdge*> _adj_edges;
	this->getAdjVMedge(_vm->e, _adj_edges);
	set<qEdge*>* _neighbors = &(this->qedge_neigbor[_vm->qe]);
	set<qEdge*>::iterator itr = _neighbors->begin();
	bool _isred = true;
	while(itr != _neighbors->end())
	{
		if(_adj_edges.find(*itr) == _adj_edges.end())
		{
			_isred = false;
			break;
		}
		itr ++;
	}

	_vm->is_red = _isred;
#ifdef DEBUG_TRACK
	bool _then = _vm->is_red;
	if(_ori != _then){
		stringstream _ss;
		_ss << "from " << _ori << " turn " << _then << endl;
		util::track(_ss);
	}
	util::track("OUT check_red");
#endif
	return _vm->is_red;
}


bool vf2::isomorphism(Vmatch* _vm, vector<match>& _new_match)
{
#ifdef DEBUG_TRACK
	util::track("IN iso vm new "+_vm->to_str());
#endif
	match _m(_vm->qe, _vm->e);
	qEdge** _order = this->get_order(_vm->qe);
#ifdef DEBUG_TRACK
	{
		stringstream _ss;
		_ss << "ORDER:" << endl;
		for(int i = 0; i < this->q->size(); i ++)
		{
			_ss << _order[i]->to_str() << endl;
		}
		util::track(_ss);
	}
#endif
	int _depth = 0;
	set<Vmatch*> _vset;
	this->getAdjSmat(_vm->e, _vset);

	this->isomorphism(_m, _depth, _order, _vset,  _new_match);

	delete[] _order;
#ifdef DEBUG_TRACK
	util::track("OUT iso vm new");
#endif
	return ! _new_match.empty();
}

bool vf2::isomorphism(match& _m, int _depth, qEdge** _order, set<Vmatch*>& _neighbor, vector<match>& _new_match)
{
	if(_m.size() >= this->q->size())
	{
		_new_match.push_back(_m);
		return true;
	}
#ifdef DEBUG_TRACK
	util::track("IN iso multiparam");
	{
		stringstream _ss;
		_ss << "\t\t_m: " << _m.to_str() << endl;
		_ss << "\t\t_depth: " << _depth << endl;
		_ss << "\t\t_order[depth+1]: " << _order[_depth+1]->to_str() << endl;
		_ss << "\t\t_neighbor size: " << _neighbor.size() << endl;
		set<Vmatch*>::iterator itr = _neighbor.begin();
		while(itr != _neighbor.end())
		{
			_ss << "\t\t\t" << (*itr)->to_str() << endl;
			++ itr;
		}

		_ss << "\t\t_new_match size: " << _new_match.size() << endl;
		util::track(_ss);
	}
#endif

	qEdge* _qe = _order[_depth+1];
	set<Vmatch*>::iterator itr = _neighbor.begin();
	while(itr != _neighbor.end())
	{
		if((*itr)->qe != _qe){
			++ itr;
			continue;
		}

#ifdef COLORING
		if((*itr)->is_red == false)
		{
			++ itr;
			continue;
		}
#endif
		match _tmpm((*itr)->qe, (*itr)->e);
		if(! this->q->compatible(&_m, &_tmpm))
		{
#ifdef DEBUG_TRACK
			{
				stringstream _ss;
				_ss << "UNcompatible:" << endl;
				_ss << _m.to_str() << endl;
				_ss << _tmpm.to_str() << endl;
				util::track(_ss);
			}
#endif
			++ itr;
			continue;
		}
		
		_tmpm.add(&_m);
		set<Vmatch*> _new_neighbor;
		this->new_neighbor(_neighbor, (*itr)->e, _new_neighbor);
		this->isomorphism(_tmpm, _depth+1, _order, _new_neighbor, _new_match);

		++ itr;
	}

#ifdef DEBUG_TRACK
	util::track("OUT iso multiparam");
#endif
	return true;
}

void vf2::new_neighbor(set<Vmatch*>& _old, dEdge* _e, set<Vmatch*>& _new)
{
#ifdef DEBUG_TRACK
	util::track("IN new neigh");
#endif
	_new.clear();
	_new.insert(_old.begin(), _old.end());
	set<Vmatch*> _tmpvset;
	this->getAdjSmat(_e, _tmpvset);
#ifdef COLORING
	set<Vmatch*>::iterator sitr = _tmpvset.begin();
	while(sitr != _tmpvset.end())
	{
		if((*sitr)->is_red == true)
		{
			_new.insert(*sitr);
		}
		++ sitr;
	}
#else
	_new.insert(_tmpvset.begin(), _tmpvset.end());
#endif

#ifdef DEBUG_TRACK
	util::track("OUT new neigh");
#endif
}

bool vf2::answer_expired(Vmatch* _vm, vector<match>& _exp_match)
{
#ifdef DEBUG_TRACK
	util::track("IN ans expire");
#endif
	vector<match>::iterator itr=this->answers.begin();
	_exp_match.clear();
	while(itr!=this->answers.end())
	{
		if(itr->contain(_vm->qe, _vm->e))
		{
			_exp_match.push_back(*itr);
			itr = this->answers.erase(itr);
		}
		else
		{
			++ itr;
		}
	}
#ifdef DEBUG_TRACK
	util::track("OUT ans expire");
#endif
	return ! _exp_match.empty();
}

bool vf2::turn_red(Vmatch* _vm, set<Vmatch*>& _visited_mat)
{
	
	return _vm != NULL && _visited_mat.empty();
}

bool vf2::un_red(Vmatch* _vm, set<Vmatch*>& _visited_mat)
{
	
	return _vm != NULL && _visited_mat.empty();
}



/* 
 * join _mat1 with those in _node 
 * results is put into _jrlist
 * */
bool vf2::join(vector<Vmatch>& _mat1, vector<Vmatch>& _mat2,  vector<match>& _join_result)
{
	_join_result.clear();

#ifdef DEBUG_TRACK
	util::track("IN JOIN Vmat");
	{

	}
#endif


	for(int i = 0; i < (int)_mat1.size(); i ++)
	{
		for(int j = 0; j < (int)_mat2.size(); j ++)
		{
			qEdge* _q1 = _mat1[i].qe;
			qEdge* _q2 = _mat2[j].qe;
			dEdge* _e1 = _mat1[i].e;
			dEdge* _e2 = _mat2[j].e;

			if( this->q->compatible(_q1, _e1, _q2, _e2) )
			{
				match _m(_q1, _e1);
				_m.add(_q2, _e2);

				_join_result.push_back(_m);
			}
		}
	}
	return true;
}

bool vf2::join(vector<match>& _mat1, vector<match>& _mat2, vector<match>& _join_result)
{
	_join_result.clear();

#ifdef DEBUG_TRACK
	{
		stringstream _ss;
		_ss << "matches in _mat1: " << endl << vf2::matches_str(_mat1) << endl;
		_ss << "matches in _mat2: " << endl << vf2::matches_str(_mat2) << endl;
		util::track(_ss);
	}
#endif


	for(int i = 0; i < (int)_mat1.size(); i ++)
	{
		for(int j = 0; j < (int)_mat2.size(); j ++)
		{
			if( this->q->compatible(&_mat1[i], &_mat2[j]) )
			{
				match _m(_mat1[i], _mat2[j]);

				_join_result.push_back(_m);
			}
		}
	}
	return true;
}
	
bool vf2::getAdjVmat(dEdge* _e, vector<Vmatch*>& _adj_vmat)
{
#ifdef DEBUG_TRACK
	util::track("IN getAdjVmat");
#endif
	vector<Vmatch*>* _vs = &(this->adjacent_list[_e->s]);
	vector<Vmatch*>* _vt = &(this->adjacent_list[_e->t]);

	_adj_vmat = *_vs;

	_adj_vmat.insert(_adj_vmat.end(), _vt->begin(), _vt->end());

#ifdef DEBUG_TRACK
	util::track("OUT getAdjVmat");
#endif
	return true;
}
	
bool vf2::getAdjSmat(dEdge* _e, set<Vmatch*>& _adj_smat)
{
#ifdef DEBUG_TRACK
	util::track("IN getAdjSmat");
#endif
	vector<Vmatch*>* _vs = &(this->adjacent_list[_e->s]);
	vector<Vmatch*>* _vt = &(this->adjacent_list[_e->t]);
	_adj_smat.clear();

	_adj_smat.insert(_vs->begin(), _vs->end());
	_adj_smat.insert(_vt->begin(), _vt->end());
#ifdef DEBUG_TRACK
	util::track("OUT getAdjSmat");
#endif

	return true;
}
	
bool vf2::getAdjVMedge(dEdge* _e, set<qEdge*>& _adj_edges)
{
#ifdef DEBUG_TRACK
	util::track("IN getAdjVMedge: "+_e->to_str());

#endif
	vector<Vmatch*>* _vs = &(this->adjacent_list[_e->s]);
	vector<Vmatch*>* _vt = &(this->adjacent_list[_e->t]);
	_adj_edges.clear();
	
	vector<Vmatch*>::iterator itr;
	for(itr = _vs->begin(); itr != _vs->end(); ++itr)
	{
		_adj_edges.insert((*itr)->qe);	
	}
	for(itr = _vt->begin(); itr != _vt->end(); ++itr)
	{
		_adj_edges.insert((*itr)->qe);	
	}

#ifdef DEBUG_TRACK
	{
		stringstream _ss;
		_ss << "\tadjqedge size: " << _adj_edges.size() << endl;
		set<qEdge*>::iterator itr = _adj_edges.begin();
		while(itr != _adj_edges.end())
		{
			_ss << "\t\t" << (*itr)->to_str() << endl;
			++ itr;
		}
		util::track(_ss);
	}
	util::track("OUT getAdjVMedge");
#endif
	return true;	
}
	
vector<Vmatch*>* vf2::getAdjVec(int _i)
{
	return &(this->adjacent_list[_i]);
}

string vf2::new_match_str(vector<match>& _mat_vec){
	stringstream _ss;
	_ss << "There are " << _mat_vec.size() << " new matches" << endl;

	for(int i = 0; i < (int)_mat_vec.size(); i ++)
	{
		_ss << i << "\t" << _mat_vec[i].to_str() << endl;
	}


	cout << _ss.str() << endl;
#ifdef DEBUG_TRACK
	util::track(_ss);
#endif

	return _ss.str();
}
	
string vf2::expired_match_str(vector<match>& _mat_vec)
{
	if(_mat_vec.empty()) return "expire NONE";
	stringstream _ss;
	_ss << "There are " << _mat_vec.size() << " expired matches" << endl;
	for(int i = 0; i < (int)_mat_vec.size(); i ++)
	{
		_ss << "\t" << _mat_vec[i].to_str() << endl;
	}


	cout << _ss.str() << endl;
#ifdef DEBUG_TRACK
	util::track(_ss);
#endif

	return _ss.str();
}

string vf2::matches_str(vector<match>& _mat_vec)
{
	if(_mat_vec.empty()) return "\tNONE";

	stringstream _ss;

	for(int i = 0; i < (int)_mat_vec.size(); i ++)
	{
		_ss << "\t" << _mat_vec[i].to_str() << endl;
	}

	return _ss.str();
}

string vf2::vf2_str(){
	stringstream _ss;
	map<int, vector<Vmatch*> >::iterator itr;
	itr = this->adjacent_list.begin();
	while(itr != this->adjacent_list.end())
	{
		_ss << itr->first << ": " << endl;
		for(int i = 0; i < (int)itr->second.size(); i ++)
		{
			_ss << "\t\t\t" << itr->second[i]->to_str() << endl;
		}

		++ itr;
	}


	return _ss.str();
}

string vf2::q2list_str()
{
	stringstream _ss;
	map<qEdge*, set<qEdge*> >::iterator itr = this->qedge_neigbor.begin();
	while(itr != this->qedge_neigbor.end())
	{
		_ss << itr->first->to_str() << endl;
		set<qEdge*>::iterator sitr = itr->second.begin();
		while(sitr != itr->second.end())
		{
			_ss << "\t\t" << (*sitr)->to_str() << endl;
			sitr ++;
		}

		itr ++;
	}
	return _ss.str();
}

/*
 */
void vf2::build_edgelist()
{

#ifdef DEBUG_TRACK
	util::track("In build_edgelist");
#endif

	vector<qEdge*> _query_edges = this->q->left2right_leaf;


	for(int i = 0; i < (int)(_query_edges.size()); i ++)
	{
		qEdge* _qi = _query_edges[i];
		for(int j = i+1; j < (int)(_query_edges.size()); j ++)
		{
			qEdge* _qj = _query_edges[j];
			if(_qi->adj(_qj))
			{
				this->qedge_neigbor[_qi].insert(_qj);
				this->qedge_neigbor[_qj].insert(_qi);
			}
		}
	}

#ifdef RUN_COMMENT
	cout << "q2liststr:\n" << this->q2list_str() << endl;
#endif

#ifdef DEBUG_TRACK
	util::track(this->q2list_str());
	util::track("Out build_edgelist");
#endif
}

qEdge** vf2::get_order(qEdge* _qe)
{
	map<qEdge*, bool> _visited_qe;
	this->initial(_visited_qe);	
	_visited_qe[_qe] =true;
	qEdge** _order = new qEdge*[this->q->size()];
	_order[0] = _qe;
	for(int i = 1; i < this->q->size(); i ++)
	{
		_order[i] = this->next_qe(_visited_qe);
	}

#ifdef DEBUG_TRACK
	{
		stringstream _ss;
		_ss << "\nORDER:" << endl;
		
		for(int i = 1; i < this->q->size(); i ++)
		{
			_ss << "\t\t" << _order[i]->to_str() << endl;
		}
	}
#endif

	return _order;
}

void vf2::initial(map<qEdge*, bool>& _v_qe)
{/* init visited */
		_v_qe.clear();
		for(int i = 0; i< (int)this->q->size(); i ++)
		{
			_v_qe[this->q->left2right_leaf[i]] = false;
		}
}
	
qEdge* vf2::next_qe(map<qEdge*, bool>& _v_qe)
{
	map<qEdge*, bool>::iterator it1 = _v_qe.begin();
	map<qEdge*, bool>::iterator it2;

	while(it1 != _v_qe.end())
	{
		if(it1->second == false){
			++ it1;
			continue;
		}

		it2 = _v_qe.begin();
		while(it2 != _v_qe.end())
		{
			if(it2->second == true)
			{
				++ it2;
				continue;
			}
			
			if(it1->first->adj(it2->first))
			{
				it2->second = true;
				return it2->first;
			}

			++ it2;
		}

		++ it1;
	}

	cout << "err next_qe" << endl;
	exit(-1);
	return NULL;
}
