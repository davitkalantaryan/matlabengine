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
#include <string>
#include <common/common_unnamedsemaphorelite.hpp>

namespace multi {

class Task;
class CEngine;
class TaskScheduler;

class SubTask
{
	friend class Task;
	friend class CEngine;
public:
	SubTask(Task* a_pParent, size_t a_subTaskIndex);
	~SubTask();

	void	AddTaskToDoneTasks();
	void	GetOutput(int a_nOutIndex, mxArray **a_Output)const;

private:
	Task*const															m_pParent;
	uint64_t															m_isError : 1;
	::common::ResizableStore< ::common::ResizableStore<uint8_t*> >		m_outputs;
	const int															m_subTaskIndex;
};


class Task
{
	friend class SubTask;
	//friend class TaskScheduler;
public:
	struct CSubTask;
	Task(TaskScheduler* a_pScheduler, uint64_t a_isTaskForSaving);
	~Task();

	uint64_t		isSubtaskInError(int a_nSubtaskIndex)const;
	uint64_t		isError()const;
	uint64_t		isForSaving()const;
	const mxArray**	inputs2()const;
	int				taskNumber()const;
	int				numberOfInputs()const;
	void			PrepareTaskForCalc(size_t a_nNumberOfTasks, const char* a_cpcFunctionName,int a_nNumberOfOutputs, int a_nNumberOfInputs, const mxArray** a_ppInputs);
	const char*		functionName()const;
	void			GetOutput(int a_nSubtaskIndex,int a_nOutIndex, mxArray **a_Output)const;
	const CSubTask*	getSubTasksPtr()const;
	uint64_t		isRunning()const {return m_isRunning;}

private:
	void			SetSubTaskDone();

public:
	struct CSubTask {SubTask* pSubTask; ~CSubTask(){delete this->pSubTask;} SubTask* operator->(){return this->pSubTask;} const SubTask* operator->()const{return this->pSubTask;}
	CSubTask(Task* a_pParent, size_t a_subTaskIndex) :pSubTask(new SubTask(a_pParent,a_subTaskIndex)) {}  
	operator SubTask*(){return this->pSubTask;}  operator SubTask*()const{return this->pSubTask;}
	};
private:
	TaskScheduler*const						m_pScheduler;
	int										m_nTaskNumber;
	int										m_nNuberOfSubTasksDone;
	int										m_nNumberOfOutputs;
	int										m_nNuberOfInputs;
	uint64_t								m_isTaskForSaving : 1;
	uint64_t								m_isError : 1;
	uint64_t								m_isRunning : 1;
	const mxArray**							m_ppInputs;
	::common::ResizableStore<CSubTask>		m_subTasks;
	::STDN::mutex							m_mutexForSubTasks;
	const char*								m_cpcFunctionName;
};


}  // namespace multi {


#endif  // #ifndef __multi_task_hpp__
