#include "Vmatch.h"
#include "../util/util.h"
#include "../timing/edge.h"

Vmatch::Vmatch(qEdge* _qe, dEdge* _e){
	this->qe = _qe;
	this->e = _e;
	
	this->is_red = false;
	this->red_cc= NULL;
}
	
Vmatch::Vmatch(const Vmatch& _vm)
{
	this->qe = _vm.qe;
	this->e = _vm.e;
	this->is_red = _vm.is_red;
	this->red_cc = _vm.red_cc;
}

	
Vmatch& Vmatch::operator=(const Vmatch& _vm)
{
	this->qe = _vm.qe;
	this->e = _vm.e;
	this->is_red = _vm.is_red;
	this->red_cc = _vm.red_cc;
	return *this;	
}

bool Vmatch::exact(qEdge* _qe, dEdge* _e)
{
	return this->qe == _qe && this->e == _e;
}

Vmatch::~Vmatch(){

}
	
int Vmatch::to_size()
{
#ifdef COLORING
	return sizeof(Vmatch);
#endif
	return sizeof(Vmatch)-sizeof(redCC*)-sizeof(bool);
}

string Vmatch::to_str(){
	stringstream _ss;
	_ss << "de[" << this->e->to_str() << "]\t";
	_ss << "qe[" << this->qe->to_str() <<"]\t";
	_ss << "is_red[" << this->is_red <<"]";
	return _ss.str(); 
}

