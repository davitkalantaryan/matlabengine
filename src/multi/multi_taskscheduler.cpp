// 
// file:	multi_taskscheduler.cpp
// 

#include "multi_taskscheduler.hpp"

static int s_nTaskNumber = 0;
static int s_nEngineNumber = 0;


multi::TaskScheduler::TaskScheduler()
{
	m_currentTask = new Task(this);
}


multi::TaskScheduler::~TaskScheduler()
{
	delete m_currentTask;
}


void multi::TaskScheduler::calcAndWait(int a_nNumOfThreads,const char* a_functionName, int a_nNumOuts, int a_nNumInps, const mxArray*a_Inputs[])
{
	int nOldSize = (int)m_currentTask->m_subTasks.size();
	m_currentTask->m_nTaskNumber = s_nTaskNumber++;
	if(a_nNumOfThreads> nOldSize){
	}
}


void multi::TaskScheduler::calcAndSaveNoWait(int a_nNumOfThreads,const char* a_functionName, int a_nNumOuts, int a_nNumInps, const mxArray*a_Inputs[])
{
}


void multi::TaskScheduler::TaskDone(multi::Task* a_pTask)
{
	if(a_pTask->m_shouldBeSaved){
	}
	else{
		m_semaWhenFinished.post();
	}
}
