//
// file:	multi_task.hpp
//

#ifndef __multi_task_hpp__
#define __multi_task_hpp__

#include <stdint.h>
#include <stddef.h>
#include <common/resizablestore.hpp>
#include <engine.h>
#include <cpp11+/mutex_cpp11.hpp>

namespace multi {

class Task;
class CEngine;
class TaskScheduler;

class SubTask
{
	friend class Task;
	friend class CEngine;
public:
	SubTask();
	~SubTask();

	bool AddTaskToDoneTasks();

private:
	Task*																m_pParent;
	::common::ResizableStore< ::common::ResizableStore<uint8_t*>* >		m_outputs;
	int																	m_subTaskIndex;
};


class Task
{
	friend class SubTask;
	friend class TaskScheduler;
public:
	Task(TaskScheduler* a_pScheduler);
	~Task();

	uint64_t		shouldBeSaved()const;
	const mxArray**	inputs2()const;
	int				taskNumber()const;
	int				numberOfInputs()const;

private:
	bool	SetSubTaskDone(SubTask* a_pTask);

//private:
//	struct CSubTask {SubTask* pSubTask;CSubTask():pSubTask(new SubTask){} ~CSubTask(){delete this->pSubTask;} SubTask* operator->(){return this->pSubTask;}};
private:
	TaskScheduler*						m_pScheduler;
	int									m_nTaskNumber;
	int									m_nNuberOfSubTasksDone;
	int									m_nNuberOfInputs;
	uint64_t							m_shouldBeSaved : 1;
	const mxArray**						m_ppInputs;
	::common::ResizableStore<SubTask*>	m_subTasks;
	::common::ResizableStore<mxArray*>	m_inputs2;
	::STDN::mutex						m_mutexForSubTasks;
};


}  // namespace multi {


#endif  // #ifndef __multi_task_hpp__
