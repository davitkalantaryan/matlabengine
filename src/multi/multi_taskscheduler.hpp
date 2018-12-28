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

typedef const multi::Task* contsTaskPtr;

class TaskScheduler
{
public:
	struct STaskItem;
	TaskScheduler();
	~TaskScheduler();

	multi::Task*								calcAndWait(int a_nNumOfThreads, const char* a_functionName, int a_nNumOuts, int a_nNumInps, const mxArray*a_Inputs[]);
	void										calcAndSaveNoWait(const char* taskName,int numOfThreads,const char* functionName,int numOuts, int numInps,const mxArray*inps[]);
	void										setNumberOfEngines(int a_nNumber);
	int											numberOfEngines()const;
	bool										GetSavedTask(const char* taskName, contsTaskPtr* a_pBuffer);
	bool										RemoveSavedTask(const char* taskName);
	const common::listN::ListItem<STaskItem>*	firstSavedTask()const;

public:  // used by engines
	void										FinalizeTask(Task* a_pTask);

private:
	//bool										FindSavedTask(const char* taskName, common::listN::ListItem<multi::TaskScheduler::STaskItem>** a_pBuffer);

public:
	struct STaskItem{ const char* taskName;const multi::Task* task; };

private:
	volatile int													m_nWork;
	multi::Task														m_taskForGui;
	::common::HashTbl< common::listN::ListItem<STaskItem>* >		m_hashSavedTasks;
	::common::UnnamedSemaphoreLite									m_semaForStartingCalc;
	::common::UnnamedSemaphoreLite									m_semaWhenFinishedForGui;
	::common::listN::Fifo<SubTask*>									m_fifoSubTasks;
	::std::vector<CEngine*>											m_vectorEngines;
	::common::List< STaskItem >										m_listSavedTasks;
};

}  // namespace multi{


#endif  // #ifndef __multi_taskscheduler_hpp__
