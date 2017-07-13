#ifndef _VMATCH_H_
#define _VMATCH_H_

#include "../util/list_T.h"
#include "../util/util.h"

class qEdge;
class dEdge;
class redCC;

class Vmatch {
public:
	Vmatch(qEdge* _qe, dEdge* _e);
	Vmatch(const Vmatch& _vm);
	~Vmatch();

	Vmatch& operator=(const Vmatch& _vm);
	bool exact(qEdge* _qe, dEdge* _e);
	
	static int to_size();
	string to_str();

	qEdge* qe;
	dEdge* e;
	bool is_red;
	redCC* red_cc;
private:	

};


#endif
