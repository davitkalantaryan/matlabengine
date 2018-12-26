// 
// file:	multi_taskscheduler.hpp
// 
#ifndef __multi_taskscheduler_hpp__
#define __multi_taskscheduler_hpp__

#include "multi_engine.hpp"
#include <common/common_hashtbl.hpp>
#include "multi_task.hpp"

namespace multi{

class TaskScheduler
{
public:
	TaskScheduler();
	~TaskScheduler();

	void calcAndWait(int a_nNumOfThreads, const char* a_functionName, int a_nNumOuts, int a_nNumInps, const mxArray*a_Inputs[]);
	void calcAndSaveNoWait(int a_nNumOfThreads, const char* a_functionName, int a_nNumOuts, int a_nNumInps, const mxArray*a_Inputs[]);

	void TaskDone(multi::Task* a_pTask );

private:
	struct CSubTask {SubTask* pSubTask;CSubTask():pSubTask(new SubTask){} ~CSubTask(){delete this->pSubTask;} SubTask* operator->(){return this->pSubTask;}};

private:
	multi::Task*						m_currentTask;
	::common::HashTbl< multi::Task* >	m_savedTasks;
	::common::UnnamedSemaphoreLite		m_semaForStartingCalc;
	::common::UnnamedSemaphoreLite		m_semaWhenFinished;
};

}  // namespace multi{


#endif  // #ifndef __multi_taskscheduler_hpp__
