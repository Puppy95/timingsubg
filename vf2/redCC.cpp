#include "redCC.h"
#include "Vmatch.h"
#include "../timing/edge.h"

RedCC::RedCC()
{

}

RedCC::RedCC(Vmatch* _v)
{
    vertices.insert(_v);
}
    
void RedCC::merge(RedCC* _red_cc)
{
    set<Vmatch*>::iterator itr = _red_cc->vertices.begin();
    while(itr != _red_cc->vertices.end())
    {
        (*itr)->red_cc = this;
        this->vertices.insert(*itr);
        itr ++;
    }
}

void RedCC::addVmat(Vmatch* _vmat){
    _vmat->red_cc = this;
    this->vertices.insert(_vmat);
}

void RedCC::decompose(RedCC* _red_cc)
{
    map<int, vector<Vmatch*>*> id2neighbor;
    set<Vmatch*>::iterator itr = _red_cc->vertices.begin();
    while(itr != _red_cc->vertices.end())
    {
        (*itr)->red_cc = NULL;
        int s = (*itr)->e->s;
        int t = (*itr)->e->t;
        vector<Vmatch*>* sVec, *tVec;
        sVec = id2neighbor[s];
        tVec = id2neighbor[t];
        if(sVec == NULL) {
            sVec = new vector<Vmatch*>();
            id2neighbor[s] = sVec;
        }
        if(tVec == NULL) {
            tVec = new vector<Vmatch*>();
            id2neighbor[t] = tVec;
        }
        sVec->push_back((*itr));
        tVec->push_back((*itr));
    }

    set<Vmatch*> vertex_set = _red_cc->vertices;
    itr = _red_cc->vertices.begin();
    RedCC* thisptr = _red_cc;
    while(!vertex_set.empty() && itr != _red_cc->vertices.end())
    {
        Vmatch* _cur_vmat = (*itr);
        if(_cur_vmat->red_cc != NULL)
        {
            itr ++;
            continue;
        }
        RedCC* newCC;
        if(thisptr != NULL) {
            newCC = thisptr;
            thisptr = NULL;
        } else
        {
            newCC = new RedCC();
        }

        newCC->addVmat(_cur_vmat);    

        int s = _cur_vmat->e->s;
        int t = _cur_vmat->e->t;
        queue<int> visitVertex;
        visitVertex.push(s);
        visitVertex.push(t);
        set<int> cur_visited_id;
        while(! visitVertex.empty())
        {
            int id = visitVertex.front();
            visitVertex.pop();
            cur_visited_id.insert(id);
            vector<Vmatch*>* iVec = id2neighbor[id];
            vector<Vmatch*>::iterator vitr = iVec->begin();
            while(vitr != iVec->end())
            {
                if((*vitr)->red_cc != NULL) {
                    vitr ++;
                    continue;
                }    
                (*vitr)->red_cc = newCC;
                int vs = (*vitr)->e->s;
                int vt = (*vitr)->e->t;
                if(cur_visited_id.find(vs) == cur_visited_id.end())
                {
                    visitVertex.push(vs);
                }
                if(cur_visited_id.find(vt) == cur_visited_id.end())
                {
                    visitVertex.push(vt);
                }
                vitr ++;
            }
        }

        itr ++;
    }
}

RedCC* RedCC::merge(set<RedCC*>& _setCC)
{
    if(_setCC.empty()) return NULL;

    set<RedCC*>::iterator itr = _setCC.begin();
    RedCC* ret = (*itr);
    itr ++;
    while(itr != _setCC.end())
    {
       ret->merge(*itr);
       delete (*itr);
       itr ++;
    }

    return ret;
}
