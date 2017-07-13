#ifndef _TENODE_H_
#define _TENODE_H_

#include "../util/list_T.h"
#include "../util/util.h"
#include "../timing/match.h"

class qEdge;

class sjNode{
public:
	sjNode(qEdge* _qedge, bool _leftmost=false);
	sjNode(sjNode* _left, sjNode* _right);
	~sjNode();

	int to_size();

	bool is_root();
	bool is_leaf();
	bool is_leftmost();
	bool has_sibling();
	bool at_left();
	bool at_right();
	/* the one corresponding to TCsubq */
	bool is_TCnode_or_upper();
	void setTC_or_upper(bool _flag);

	sjNode* get_sibling();
	void set_sibling(sjNode* _sibling);
	sjNode* get_father();
	void set_father(sjNode* _father);
	void set_children(sjNode* _left, sjNode* _right);
	sjNode* get_left();
	sjNode* get_right();
	vector<qEdge*>* get_subq();




	int remove_edge(dEdge* _e, vector<match>& _rm_set);
	int remove_match(vector<match>& _expire_set, vector<match>& _rm_set);

	/* */
	bool add_matches(vector<match>& _join_result);

	void get_all_matches(vector<match>& _all_mat);

	string to_matches_str();
	string to_str();
	string to_querystr();

private:	
	vector<match> mat_vec;

	vector<qEdge*> subquery;
	sjNode* father;
	sjNode* left_child;
	sjNode* right_child;
	sjNode* sibling;
	bool flag_leftmost;
};


#endif
