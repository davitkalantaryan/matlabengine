// 
// file:	multi_engine.hpp
//

#ifndef __matlabengine_multi_engine_hpp__
#define __matlabengine_multi_engine_hpp__

#include <engine.h>
#include <stddef.h>
#include <stdint.h>
#include <cpp11+/thread_cpp11.hpp>
#include <common/common_unnamedsemaphorelite.hpp>
#include <common/lists.hpp>
#include <string>
#include <common/common_hashtbl.hpp>
#include "multi_task.hpp"
#include <common/resizablestore.hpp>

namespace multi {

class CEngine
{
public:
	CEngine(int a_nEngineNumber, ::common::UnnamedSemaphoreLite* a_pSema, ::common::listN::Fifo<SubTask*>* a_pFifo, volatile int* a_pnRun);
	virtual ~CEngine();

	int					engineNumber()const;
	void				addFunction2(int a_nTaskNumber, const char* a_functionName, int a_nNumOuts, int a_nNumInps, const mxArray*a_Inputs[], ::common::UnnamedSemaphoreLite* a_pSemaToInform, int a_nIndex);
	bool				addFunctionIfFree2(int a_nTaskNumber, const char* a_functionName, int a_nNumOuts, int a_nNumInps, const mxArray*a_Inputs[], ::common::UnnamedSemaphoreLite* a_pSemaToInform, int a_nIndex);
	uint64_t			isRunning()const;
	//const EngineTask*	getFirstReadyTask()const;
	//void				StartCalc();

protected:
	void				EngineThread();
	void				runFunctionInEngineThread(SubTask*);

	int					StartEngine();
	void				StopEngine();

protected:
	::Engine*										m_pEngine;
	uint64_t										m_isStarted : 1;
	uint64_t										m_shouldRun : 1;
	uint64_t										m_isError : 1;
	uint64_t										m_isEngineRunning : 1;
	uint64_t										m_nEngineNumber : 24;
	uint64_t										m_nLastFinishedtaskNumber : 24;
	::STDN::thread									m_threadForEngine;
	::common::UnnamedSemaphoreLite*					m_pSemaToStartCalc;
	::common::listN::Fifo<SubTask*>*				m_pFifoToDoTasks;
	volatile int*									m_pnRun;
	
};

}

#include "impl.multi_engine.hpp"

#endif  // #ifndef __matlabengine_multi_engine_hpp__
