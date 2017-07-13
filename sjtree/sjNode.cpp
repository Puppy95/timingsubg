#include "sjNode.h"
#include "../util/util.h"

sjNode::sjNode(qEdge* _qedge, bool _leftmost){
	this->mat_vec.clear();
	this->subquery.clear();
	this->subquery.push_back(_qedge);
	this->sibling = NULL;
	this->father = NULL;
	this->left_child = NULL;
	this->right_child = NULL;

	this->flag_leftmost = _leftmost;
}

sjNode::sjNode(sjNode* _left, sjNode* _right){
	this->mat_vec.clear();
	this->set_children(_left, _right);
	this->subquery.clear();
	vector<qEdge*>* _subq = NULL;
	_subq = _left->get_subq();
	this->subquery.insert(this->subquery.end(), _subq->begin(), _subq->end());
	_subq = _right->get_subq();
	this->subquery.insert(this->subquery.end(), _subq->begin(), _subq->end());

	this->sibling = NULL;
	this->father = NULL;
	_left->set_sibling(_right);
	_right->set_sibling(_left);
	_left->set_father(this);
	_right->set_father(this);

	this->flag_leftmost = false;
}

sjNode::~sjNode(){
	cout << "IN destruct sjNode: " << this->to_querystr() << endl;


	cout << "OUT destruct sjNode" << endl;
}
	
int sjNode::to_size()
{
	int _sz = sizeof(sjNode);
	int _mat_num = this->mat_vec.size();
	int _mat_sz = this->subquery.size()*(sizeof(dEdge*)+sizeof(qEdge*));
	_sz += _mat_num * _mat_sz;

	return _sz;
}

bool sjNode::is_root(){
	
	return this->father == NULL;
}

bool sjNode::is_leaf(){
	return this->left_child == NULL && this->right_child == NULL;
}

bool sjNode::is_leftmost(){
	return this->flag_leftmost;
}
bool sjNode::has_sibling(){
	return this->sibling != NULL;
}

bool sjNode::at_left(){
	if(this->father == NULL) return false;
	return this->father->get_left() == this;
}

bool sjNode::at_right(){
	if(this->father == NULL) return false;
	return this->father->get_right() == this;
}

sjNode*  sjNode::get_sibling(){
	return this->sibling;
}

void sjNode::set_sibling(sjNode* _sibling){
	this->sibling = _sibling;
}

void sjNode::set_children(sjNode* _left, sjNode* _right){
	this->left_child = _left;
	this->right_child = _right;
}

sjNode* sjNode::get_left(){
	return this->left_child;
}

sjNode* sjNode::get_right(){
	return this->right_child;
}

void sjNode::set_father(sjNode* _father){
	this->father = _father;
}

sjNode*  sjNode::get_father(){
	return this->father;
}

vector<qEdge*>* sjNode::get_subq(){
	return &(this->subquery);
}


/* 
 * 
 * */
int sjNode::remove_edge(dEdge* _e, vector<match>& _rm_set){

#ifdef DEBUG_TRACK
	{
		stringstream _ss;
		_ss << "rm: " <<  _e->to_str() << endl;
		_ss << "from sjnode " << this->to_str() << endl;
		_ss << this->to_matches_str() << endl;
		util::track(_ss);
	}
#endif
	_rm_set.clear();
	int _count = 0;
	vector<match>::iterator itr = this->mat_vec.begin();
	while(itr != this->mat_vec.end())
	{
		if(itr->contain(_e))
		{
			_rm_set.push_back(*itr);

			itr = this->mat_vec.erase(itr);
			_count ++;
		}
		else
		{
			itr ++;
		}
	}


	return _count;
}
	
int sjNode::remove_match(vector<match>& _expire_set, vector<match>& _rm_set)
{
	return _expire_set.size() != _rm_set.size();
}

/* */
bool sjNode::add_matches(vector<match>& _join_result){
	this->mat_vec.insert(this->mat_vec.end(), _join_result.begin(), _join_result.end());
	return true;
}

void sjNode::get_all_matches(vector<match>& _all_mat){

	_all_mat.clear();
	for(int i = 0; i < (int)this->mat_vec.size(); i ++)
	{
		_all_mat.push_back(this->mat_vec[i]);
	}

	return; 
}

string sjNode::to_matches_str(){
	
	stringstream _ss;
	_ss << "There are " <<this->mat_vec.size() << " matches of " << this->to_str() << " as following:" << endl;

	vector<match>::iterator itr = this->mat_vec.begin();
	while(itr != this->mat_vec.end())
	{
		_ss << "\t\t" << itr->to_str() << endl;
		itr ++;
	}

	return _ss.str();
}
	

string sjNode::to_str(){
	stringstream _ss;
	_ss << this->to_querystr();
#ifdef QUERYSTR
	return _ss.str();
#endif

	if(this->left_child != NULL){
		_ss << " left" << this->left_child->to_querystr();
	}else{
		_ss << " left:NULL";
	}

	if(this->right_child != NULL){
		_ss << " right" << this->right_child->to_querystr();
	}else{
		_ss << " right:NULL";
	}

	if(this->sibling != NULL){
		_ss << " sibling" << this->sibling->to_querystr();
	}else{
		_ss << " sibling:NULL";
	}

	if(this->father != NULL){
		_ss << " father" << this->father->to_querystr();
	}else{
		_ss << " father:NULL";
	}

	return _ss.str();
}

string sjNode::to_querystr()
{
	stringstream _ss;

#ifdef DEBUG_TRACK
	{
		stringstream _tmpss;
		_tmpss << "size of subq = " << this->subquery.size() << endl;
		if(this->subquery[0] == NULL){
			_tmpss << "NULL err subq" << endl;
		}
		if(this->subquery.size()>1){
			_tmpss << "second null: " << (this->subquery[1] == NULL) << endl;
		}
//		util::track(_tmpss);
	}
#endif

	_ss << "[(" << this->subquery[0]->to_str() << ")";
	for(int i = 1; i< (int)this->subquery.size(); i ++){
		_ss << ", (" << this->subquery[i]->to_str() << ")";
	}
	_ss << "]";
	
	return _ss.str();
}
