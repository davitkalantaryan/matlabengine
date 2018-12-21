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

namespace multi {

class CEngine
{
public:
	class EngineTask;
	struct TaskStatus{enum Type{Stopped,Running};};
	CEngine(int a_nEngineNumber);
	virtual ~CEngine();

	int					lastFinishedTask()const;
	int					number()const;
	int					StartEngine();
	void				StopEngine();
	void				addFunction(int a_nTaskNumber, const char* a_functionName, int a_nNumOuts, int a_nNumInps, const mxArray*a_Inputs[], ::common::UnnamedSemaphoreLite* a_pSemaToInform);
	bool				addFunctionIfFree(int a_nTaskNumber, const char* a_functionName, int a_nNumOuts, int a_nNumInps, const mxArray*a_Inputs[], ::common::UnnamedSemaphoreLite* a_pSemaToInform);
	TaskStatus::Type	taskStatus()const;
	const EngineTask*	getTask(int a_nTaskNumber)const;
	const EngineTask*	getLastTask()const;
	void				DeleteTask(int a_nTaskNumber);
	void				DeleteLastTask();
	const ::common::List<EngineTask*>& listOfTasks()const;

protected:
	void				EngineThread();
	void				runFunctionInEngineThread(EngineTask*);

public:
	class EngineTask 
	{
	public:
		int										taskNumber;
		int										numberOfOutputs;
		int										numberOfInputs;
		int										arraySize; // max(numberOfOutputs,numberOfInputs)
		::std::string							funcName;
		mxArray**								inputsOrOutputs;
		int64_t									taskStatus : 3;
		int64_t									isFncNameProvided : 1;
		::common::listN::ListItem<EngineTask*>*	itemForIteration;
	private:
		friend class CEngine;
		::common::UnnamedSemaphoreLite*	pSemaToInform;
		EngineTask(int a_nTaskNumber, const char* a_functionName, int a_nNumOuts, int a_nNumInps, const mxArray*a_Inputs[], ::common::UnnamedSemaphoreLite* a_pSema);
		~EngineTask();
	};

protected:
	::Engine*										m_pEngine;
	EngineTask*										m_pLastTask;
	uint64_t										m_isStarted : 1;
	uint64_t										m_shouldRun : 1;
	uint64_t										m_isError : 1;
	uint64_t										m_isEngineRunning : 1;
	uint64_t										m_nEngineNumber : 24;
	uint64_t										m_nLastFinishedtaskNumber : 24;
	::STDN::thread									m_threadForEngine;
	::common::UnnamedSemaphoreLite					m_semaEngine;
	::common::listN::Fifo<EngineTask*>				m_fifoTasks;
	::common::HashTbl<EngineTask*>					m_hashTasks;
	::common::List<EngineTask*>						m_listAllTasks;
};

}



#endif  // #ifndef __matlabengine_multi_engine_hpp__
