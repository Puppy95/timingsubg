#include "sjtree.h"
#include "sjNode.h"
#include "../timing/match.h"
#include "../timing/query.h"
#include "../timing/timingconf.h"

sjtree::sjtree(string _f_e2freq, query* _q)
{
	this->q = _q;
    this->f_e2freq = _f_e2freq;
	this->edge2node.clear();
	this->initial();
}

sjtree::~sjtree()
{
	cout << "\nIN destruct sjtree..." << endl;

#ifdef DEBUG_TRACK
	util::track("IN destruct sjtree");
#endif
	/* free edge2oplist & edge2node */
	{/* sjNode  */
		sjNode* _cur_sj = this->sjroot;
		queue<sjNode*> _q_sj;
		_q_sj.push(_cur_sj);
		if(_cur_sj == NULL){
			cout << "err: root NULL" << endl;
			exit(-1);
		}

		while(! _q_sj.empty())
		{
			sjNode* _n = _q_sj.front();
			if(_n->get_left() != NULL){
				_q_sj.push(_n->get_left());
			}
			if(_n->get_right() != NULL){
				_q_sj.push(_n->get_right());
			}
			_q_sj.pop();
			delete _n;
		}
	}

	cout << "OUT destruct sjtree..." << endl;

#ifdef DEBUG_TRACK
	util::track("OUT destruct sjtree");
#endif

}
	
long long int sjtree::to_size()
{
	long long int _sz = sizeof(sjtree);

	{/* sjNode  */
		sjNode* _cur_sj = this->sjroot;
		queue<sjNode*> _q_sj;
		_q_sj.push(_cur_sj);
		if(_cur_sj == NULL){
			cout << "err: root NULL" << endl;
			exit(-1);
		}

		while(! _q_sj.empty())
		{
			sjNode* _n = _q_sj.front();
			if(_n->get_left() != NULL){
				_q_sj.push(_n->get_left());
			}
			if(_n->get_right() != NULL){
				_q_sj.push(_n->get_right());
			}
			_q_sj.pop();
			_sz += _n->to_size();
		}
	}
	

	return _sz;
}

void sjtree::initial()
{
	this->build_sj_tree();
#ifdef DEBUG_TRACK
	util::track("sjtree:\n"+ this->sjtree_str());
#endif
}

/*  */
bool sjtree::getSJlist(dEdge* _e, vector<sjNode*>& _sj_vec, vector<qEdge*>& _match_edges)
{
	this->q->get_matches(_e, _match_edges);
	_sj_vec.clear();
	for(int i = 0; i < (int)_match_edges.size(); i ++)
	{
		_sj_vec.push_back( this->edge2node[_match_edges[i]] );
	}
	return true;
}


/*  */
bool sjtree::remove(dEdge* _e, sjNode* _node){
#ifdef DEBUG_TRACK
	{
		stringstream _ss;
		_ss << "RM " << _e->to_str() << endl;
		_ss << "FROM " << _node->to_str() << endl;
		util::track(_ss);
	}
#endif

	sjNode* _cur_sj = _node;
	
	vector<match> rm_prev;
	vector<match> rm_vec;
	while(_cur_sj!= NULL)
	{
		_cur_sj->remove_edge(_e, rm_vec);
		if(_cur_sj->is_root()){
			this->expired_match_str(rm_vec);
		}

		_cur_sj = _cur_sj->get_father();
	}

	return true;
}


/* 
 * 
 * */
bool sjtree::insert(dEdge* _e, qEdge* _qe)
{
	match _m(_qe, _e);
#ifdef DEBUG_TRACK
	util::track("Insert match: "+_m.to_str());
#endif


	vector<match> _new;
	vector<match> _join_result;
	sjNode* _cur_sj = this->edge2node[_qe];
	
	_new.push_back(_m);
	_cur_sj->add_matches(_new);

	while(_cur_sj->is_root() == false)
	{
		this->join(_new, _cur_sj->get_sibling(), _join_result);


#ifdef DEBUG_TRACK
		{
			stringstream _ss;
			_ss << "\n\nJOIN result: " << endl;
			_ss << "\t" << sjtree::matches_str(_join_result) << endl;

			util::track(_ss);
		}
#endif
		_cur_sj = _cur_sj->get_father();

		_new = _join_result;
		_cur_sj->add_matches(_new);

		if(_cur_sj->is_root() && !_new.empty())
		{
			this->new_match_str(_new);
		}
	}

#ifdef DEBUG_TRACK
	util::track("OUT Insert match: "+_m.to_str());
#endif
	return true;
}


/* 
 * join _matches with those in _node 
 * results is put into _jrlist
 * */
bool sjtree::join(vector<match>& _matches, sjNode* _node,  vector<match>& _join_result)
{
	_join_result.clear();

	vector<match> _mat_node;

	_node->get_all_matches(_mat_node);

#ifdef DEBUG_TRACK
	{
		stringstream _ss;
		_ss << "join with _node : " << _node->to_str() << endl;
		_ss << "matches in _node: " << endl << sjtree::matches_str(_mat_node) << endl;
		_ss << "incoming matches: " << endl << sjtree::matches_str(_matches) << endl;
		util::track(_ss);
	}
#endif


	for(int i = 0; i < (int)_matches.size(); i ++)
	{
		for(int j = 0; j < (int)_mat_node.size(); j ++)
		{
			if( this->q->compatible(&(_matches[i]), &(_mat_node[j])) )
			{
				match _m(_matches[i], _mat_node[j]);
				_join_result.push_back(_m);
			}
		}
	}
	return true;
}


string sjtree::new_match_str(vector<match>& _mat_vec){
	stringstream _ss;
	_ss << "There are " << _mat_vec.size() << " new matches" << endl;

	for(int i = 0; i < (int)_mat_vec.size(); i ++)
	{
		_ss << i << "\t" << _mat_vec[i].to_str() << endl;
	}


#ifdef DEBUG_TRACK
	cout << _ss.str() << endl;
	util::track(_ss);
#endif

	return _ss.str();
}
	
string sjtree::expired_match_str(vector<match>& _mat_vec)
{
	if(_mat_vec.empty()) return "expire NONE";
	stringstream _ss;
	_ss << "There are " << _mat_vec.size() << " expired matches" << endl;
	for(int i = 0; i < (int)_mat_vec.size(); i ++)
	{
		_ss << "\t" << _mat_vec[i].to_str() << endl;
	}


#ifdef DEBUG_TRACK
	cout << _ss.str() << endl;
	util::track(_ss);
#endif

	return _ss.str();
}

string sjtree::matches_str(vector<match>& _mat_vec)
{
	if(_mat_vec.empty()) return "\tNONE";

	stringstream _ss;

	for(int i = 0; i < (int)_mat_vec.size(); i ++)
	{
		_ss << "\t" << _mat_vec[i].to_str() << endl;
	}

	return _ss.str();
}

string sjtree::sjtree_str(){
	stringstream _ss;
	sjNode* _cur_sj = this->sjroot;
	queue<sjNode*> _q_sj;
	_q_sj.push(_cur_sj);
	if(_cur_sj == NULL) return "sjroot=NULL";

	while(! _q_sj.empty())
	{
		sjNode* _n = _q_sj.front();
		_ss << _n->to_str() << endl;
		if(_n->get_left() != NULL){
			_q_sj.push(_n->get_left());
		}
		if(_n->get_right() != NULL){
			_q_sj.push(_n->get_right());
		}
		_q_sj.pop();
	}

	return _ss.str();
}

string sjtree::q2sj_str()
{
	stringstream _ss;
	map<qEdge*, sjNode*>::iterator itr = this->edge2node.begin();
	while(itr != this->edge2node.end())
	{
		_ss << itr->first->to_str() << "\t" << itr->second->to_str() << endl;
		itr ++;
	}
	return _ss.str();
}

    
void sjtree::order_selection(string _f_e2freq, vector<qEdge*>& _query_edges)
{
    map<string, int> elabel2freq;
    ifstream fin(_f_e2freq.c_str(), ios::in);
    if(!fin){
        cerr << "err open file: " << _f_e2freq << endl;
        exit(-1);
    }

    char _buf[100];
    int _freq = 0;
    while(!fin.eof())
    {
        memset(_buf, 0, sizeof(_buf));
        fin.getline(_buf, 99, '\n');
        if(strlen(_buf) < 3) continue;
        stringstream _ss;
        _ss << _buf;
        _ss >> _buf >> _freq;

        elabel2freq[string(_buf)] = _freq;
    }
    
    multimap<int, qEdge*> freq2qe;
    for(int i = 0; i < (int) _query_edges.size(); i ++)
    {
        qEdge* _qe = _query_edges[i];
        int _freq = elabel2freq[_qe->label()];
        freq2qe.insert(pair<int, qEdge*>(_freq, _qe));
    }

    multimap<int, qEdge*>::iterator itr;
    itr = freq2qe.begin();
    _query_edges.clear();
    while(itr != freq2qe.end())
    {
        _query_edges.push_back(itr->second);
        itr ++;
    }

    return;
}
/*
 * build_tree: sjroot
 * load edge2node
 * set leftmost
 * set tc_or_upper
 */
void sjtree::build_sj_tree()
{

#ifdef DEBUG_TRACK
	util::track("In build_sj_tree");
#endif

	if(! this->edge2node.empty()){
		cout << "err not empty for edge2node" << endl;
		exit(-1);
	}

	vector<qEdge*> _query_edges = this->q->left2right_leaf;
    this->order_selection(this->f_e2freq, _query_edges);


	for(int i = 0; i < (int)(_query_edges.size()); i ++)
	{
		qEdge* _cur_qe = _query_edges[i];
		sjNode* _cur_sj = NULL;
		if(i == 0)
		{
			_cur_sj = new sjNode( _cur_qe, true );
		}
		else
		{
			_cur_sj = new sjNode( _cur_qe, false );
		}

		this->edge2node[ _cur_qe ] = _cur_sj;

		/* sjroot is the final root */
		if(i == 0) 
			this->sjroot = _cur_sj;
		else
		{
			this->sjroot = new sjNode(this->sjroot, _cur_sj);
		}
	}
#ifdef RUN_COMMENT
	cout << "q2sjstr:\n" << this->q2sj_str() << endl;
#endif

#ifdef DEBUG_TRACK
	util::track(this->q2sj_str());
	util::track("Out build_sj_tree");
#endif
}

