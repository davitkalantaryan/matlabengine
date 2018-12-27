//
// file:		multi_task.cpp
//

#include "multi_task.hpp"
#include <mex.h>
#include "multi_taskscheduler.hpp"

static int s_nTaskNumber = 0;

multi::SubTask::SubTask(Task* a_pParent, size_t a_subTaskIndex)
	:
	m_pParent(a_pParent),
	m_subTaskIndex((int)a_subTaskIndex)
{
}


multi::SubTask::~SubTask()
{
}


void multi::SubTask::AddTaskToDoneTasks()
{
	if(this->m_isError){m_pParent->m_isError=1;}
	m_pParent->SetSubTaskDone();
}


void multi::SubTask::GetOutput(int a_nOutIndex, mxArray **a_Output)const
{
	const int cnOutputSize = (int)m_outputs[a_nOutIndex].size();
	mxArray* pByteStream = mxCreateNumericMatrix(1, cnOutputSize, mxUINT8_CLASS, mxREAL);
	
	memcpy(mxGetData(pByteStream), m_outputs[a_nOutIndex].buffer(), cnOutputSize);
	mexCallMATLABWithTrap(1, a_Output, 1, &pByteStream, "getArrayFromByteStream");
	mxDestroyArray(pByteStream);
}



/*////////////////////////////////////////////////*/
#if 0
TaskScheduler*const					m_pScheduler;
int									m_nTaskNumber;
int									m_nNuberOfSubTasksDone;
int									m_nNuberOfInputs;
uint64_t							m_shouldBeSaved : 1;
const mxArray**						m_ppInputs;
::common::ResizableStore<CSubTask>	m_subTasks;
::common::ResizableStore<mxArray*>	m_inputs2;
::STDN::mutex						m_mutexForSubTasks;
const char*							m_cpcFunctionName;
#endif

multi::Task::Task(TaskScheduler* a_pScheduler,uint64_t a_isTaskForSaving)
	:
	m_pScheduler(a_pScheduler),
	m_cpcFunctionName(NULL)
{
	m_isRunning = 0;
	m_isTaskForSaving = a_isTaskForSaving;
}


multi::Task::~Task()
{
	if(m_isTaskForSaving){
		for(int i(0);i<m_nNuberOfInputs;++i){
			mxDestroyArray(const_cast<mxArray*>(m_ppInputs[i]));
		}
		free(const_cast<mxArray**>(m_ppInputs));
		free(const_cast<char*>(m_cpcFunctionName));
	}
}


uint64_t multi::Task::isError()const
{
	return m_isError;
}


uint64_t multi::Task::isSubtaskInError(int a_nSubtaskIndex)const
{
	return m_subTasks[a_nSubtaskIndex]->m_isError;
}


uint64_t multi::Task::isForSaving()const
{
	return m_isTaskForSaving;
}


int multi::Task::taskNumber()const
{
	return m_nTaskNumber;
}


void multi::Task::SetSubTaskDone()
{
	bool bInform;
	int nNumberOfSubTasks = (int)m_subTasks.size();
	m_mutexForSubTasks.lock();
	bInform = nNumberOfSubTasks <= (++m_nNuberOfSubTasksDone);
	m_mutexForSubTasks.unlock();
	m_isRunning = 0;
	if(bInform){
		m_pScheduler->FinalizeTask(this);
	}
}


const mxArray** multi::Task::inputs2()const
{
	return m_ppInputs;
}


int multi::Task::numberOfInputs()const
{
	return m_nNuberOfInputs;
}


void multi::Task::PrepareTaskForCalc(size_t a_nNumberOfTasks, const char* a_cpcFunctionName, int a_nNumberOfOutputs, int a_nNumberOfInputs, const mxArray** a_ppInputs)
{
	size_t i;

	m_isRunning = 1;
	m_nNumberOfOutputs = a_nNumberOfOutputs;
	m_isError = 0;
	m_subTasks.resize(a_nNumberOfTasks, this);
	for(i=0;i<a_nNumberOfTasks;++i){
		m_subTasks[i]->m_outputs.resize(a_nNumberOfOutputs);
	}

	if(!m_isTaskForSaving){
		m_cpcFunctionName = a_cpcFunctionName;
		m_ppInputs = a_ppInputs;
	}
	else{
		int j;
		//const int cnNuberOfInputsOld = 
		mxArray** ppInputs=const_cast<mxArray**>(m_ppInputs);
		char* pcFunctionName = const_cast<char*>(m_cpcFunctionName);
		size_t unFncNameLenPlus1 = strlen(a_cpcFunctionName)+1;
		pcFunctionName = (char*)realloc(pcFunctionName, unFncNameLenPlus1);
		memcpy(pcFunctionName,a_cpcFunctionName,unFncNameLenPlus1);

		for (j = 0; j < m_nNuberOfInputs; ++j) {
			if(ppInputs[j]){mxDestroyArray(ppInputs[j]);ppInputs[j]=NULL;}
		}
		ppInputs=(mxArray**)realloc(ppInputs,sizeof(mxArray*)*a_nNumberOfInputs);
		for(j=0;j<a_nNumberOfInputs;++j){
			ppInputs[j] = mxDuplicateArray(a_ppInputs[j]);
		}

		m_cpcFunctionName = pcFunctionName;
		m_ppInputs = (const mxArray**)ppInputs;
	}
	m_nNuberOfInputs = a_nNumberOfInputs;
	//m_mutexForSubTasks.lock();
	m_nNuberOfSubTasksDone = 0;
	//m_mutexForSubTasks.unlock();
	m_nTaskNumber = s_nTaskNumber++;
}


const char* multi::Task::functionName()const
{
	return m_cpcFunctionName;
}


void multi::Task::GetOutput(int a_nSubtaskIndex, int a_nOutIndex, mxArray **a_Output)const
{
	m_subTasks[a_nSubtaskIndex]->GetOutput(a_nOutIndex, a_Output);
}


const multi::Task::CSubTask*  multi::Task::getSubTasksPtr()const
{
	return m_subTasks.buffer();
}
