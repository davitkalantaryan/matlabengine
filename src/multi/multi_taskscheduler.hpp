// 
// file:	multi_taskscheduler.hpp
// 
#ifndef __multi_taskscheduler_hpp__
#define __multi_taskscheduler_hpp__

#include "multi_engine.hpp"
#include <common/common_hashtbl.hpp>
#include "multi_task.hpp"
#include <common/lists.hpp>
#include <vector>

namespace multi{

class TaskScheduler
{
public:
	TaskScheduler();
	~TaskScheduler();

	multi::Task* calcAndWait(int a_nNumOfThreads, const char* a_functionName, int a_nNumOuts, int a_nNumInps, const mxArray*a_Inputs[]);
	void calcAndSaveNoWait(const char* a_cpcTaskName,int a_nNumOfThreads, const char* a_functionName, int a_nNumOuts, int a_nNumInps, const mxArray*a_Inputs[]);
	void setNumberOfEngines(int a_nNumber);
	int	 numberOfEngines()const;
	void FinalizeTask(Task* a_pTask);
	bool GetSavedTask(Task** a_pBuffer);
	bool GetSavedTaskAndRemove(Task** a_pBuffer);

private:
	struct STaskItem{ char* taskName;multi::Task* task; };

private:
	volatile int												m_nWork;
	multi::Task													m_taskForGui;
	::common::HashTbl< common::listN::ListItem<STaskItem>* >	m_savedTasks;
	::common::UnnamedSemaphoreLite								m_semaForStartingCalc;
	::common::UnnamedSemaphoreLite								m_semaWhenFinishedForGui;
	::common::listN::Fifo<SubTask*>								m_fifoSubTasks;
	::std::vector<CEngine*>										m_vectorEngines;
	::common::List< STaskItem >									m_listSavedTasks;
};

}  // namespace multi{


#endif  // #ifndef __multi_taskscheduler_hpp__
