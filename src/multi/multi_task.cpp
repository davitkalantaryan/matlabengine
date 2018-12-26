//
// file:		multi_task.cpp
//

#include "multi_task.hpp"

multi::SubTask::SubTask()
{
}


multi::SubTask::~SubTask()
{
}


bool multi::SubTask::AddTaskToDoneTasks()
{
	return m_pParent->SetSubTaskDone(this);
}



/*////////////////////////////////////////////////*/

multi::Task::Task(TaskScheduler* a_pScheduler)
	:
	m_pScheduler(a_pScheduler)
{
	m_shouldBeSaved = 0;
}


multi::Task::~Task()
{
}


uint64_t multi::Task::shouldBeSaved()const
{
	return m_shouldBeSaved;
}


int multi::Task::taskNumber()const
{
	return m_nTaskNumber;
}


bool multi::Task::SetSubTaskDone(SubTask* a_pTask)
{
	bool bRet;
	int nNumberOfSubTasks = (int)m_subTasks.size();
	m_mutexForSubTasks.lock();
	bRet = nNumberOfSubTasks == (++m_nNuberOfSubTasksDone);
	m_mutexForSubTasks.unlock();
	m_subTasks[a_pTask->m_subTaskIndex]= a_pTask;
	return bRet;
}


const mxArray** multi::Task::inputs2()const
{
	return m_ppInputs;
}


int multi::Task::numberOfInputs()const
{
	return m_nNuberOfInputs;
}
