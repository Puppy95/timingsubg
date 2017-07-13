#include "rdfstream.h"
#include "../util/util.h"
#include "rdfquery.h"
#include "../timing/query.h"
#include "../timing/timingconf.h"
#include "../sjtree/sjtreesubg.h"

/* 
 * argv0 : exe
 * argv1 : dataset
 * argv2 : query
 * argv3 : winsz
 * argv4 : Max_Thread_Num
 * 
 * */
int main(int argc, char* argv[])
{
#ifdef DEBUG_TRACK
#endif

#ifdef THREAD_LOG
#endif
	
	if(argc < 6){
		cout << "err argc" << endl;
		exit(0);
	}
#ifdef GLOBAL_COMMENT
	cout << "num of argc: " << argc << endl;
#endif

	
	string _dataset, _query, _frtime;
	_dataset = string(argv[1]);
    string f_e2freq = "~/dataset/rdf_e2freq.sort";
	_query = string(argv[2]);
	_frtime = string(argv[5]);

	int _window, max_thread_num;
	{
		stringstream _ss;
		for(int i = 3; i < argc; i ++) 
			_ss << argv[i] << " ";
		_ss >> _window >> max_thread_num;
	}

	timingconf _tconf(_window, max_thread_num, _dataset, _query);
	rdfstream _NS(_dataset);
	rdfquery _Q(_query);
	_Q.parseQuery();
#ifdef RUN_COMMENT
	cout << "query Q: \n" << _Q.to_str() << endl;
	cout << "pre Q: \n" << _Q.timingorder_str() << endl;
#endif

	sjtreesubg ssubg(0, _frtime);
	ssubg.exename = string(argv[0]);
	ssubg.run(0, &_NS, &_Q, &_tconf, f_e2freq);
#ifdef GLOBAL_COMMENT
	cout << "finish sjtreesubg run" << endl;
#endif

	ssubg.write_stat();

	util::finalize();
}

