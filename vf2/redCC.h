#ifndef _REDCC_H_
#define _REDCC_H_

#include "../util/list_T.h"
#include "../util/util.h"

class Vmatch;

class RedCC{
public:
    RedCC();
    RedCC(Vmatch* _v);
    void merge(RedCC* _red_cc);
    void addVmat(Vmatch* _vmat);

    static void decompose(RedCC* _red_cc);
    static RedCC* merge(set<RedCC*>& _vecCC);

    set<Vmatch*> vertices;
};

#endif
