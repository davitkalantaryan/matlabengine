//
// file:		multi_engine.cpp
//

#include "multi_engine.hpp"
#include <stdio.h>
#include <stdlib.h>

#ifdef _WIN32
#include <Windows.h>
#define SleepMsIntr(_x)		SleepEx((_x),TRUE)	
#else
#include <unistd.h>
#define SleepMsIntr(_x)		usleep(1000*(_x))
#endif

#define INP_BUFF_LEN_MIN1				127
#define EVAL_STRING_BUFFER_LENGTH_MIN1	2047
#ifndef HANDLE_MEM_DEF
#define HANDLE_MEM_DEF(_pointer,...)	do{if(!(_pointer)){throw "low memory";}}while(0)
#endif

static void GenerateInputOrOutName(int a_nTaskNumber, int a_nIndex, bool a_bIsInput, char* a_pcBuffer);


multi::CEngine::CEngine(int a_nEngineNumber)
	:
	m_pEngine(NULL),
	m_pLastTask(NULL)
{
	m_nLastFinishedtaskNumber=m_isEngineRunning=m_isError=m_shouldRun = m_isStarted = 0;
	m_nEngineNumber = (uint64_t)a_nEngineNumber;

	StartEngine();
}


multi::CEngine::~CEngine()
{
	StopEngine();
	while(m_listAllTasks.first()){
		delete m_listAllTasks.first()->data;
		m_listAllTasks.RemoveData(m_listAllTasks.first());
	}
}


int	multi::CEngine::number()const
{
	return (int)m_nEngineNumber;
}


int multi::CEngine::lastFinishedTask()const
{
	return (int)m_nLastFinishedtaskNumber;
}


const ::common::List<multi::CEngine::EngineTask*>& multi::CEngine::listOfTasks()const
{
	return m_listAllTasks;
}


multi::CEngine::TaskStatus::Type multi::CEngine::taskStatus()const
{
	return m_isEngineRunning ? TaskStatus::Running : TaskStatus::Stopped;
}


const multi::CEngine::EngineTask* multi::CEngine::getTask(int a_nTaskNumber)const
{
	EngineTask* pTask(NULL);
	m_hashTasks.FindEntry(&a_nTaskNumber, 4, &pTask);
	return pTask;
}


const multi::CEngine::EngineTask* multi::CEngine::getLastTask()const
{
	return m_pLastTask;
}

int multi::CEngine::StartEngine()
{
	if(m_isStarted){return 1;}
	m_shouldRun = 1;
	m_threadForEngine = STDN::thread(&CEngine::EngineThread, this);
	while((!m_isError)&&(!m_isStarted)){SleepMsIntr(1);}
	if(m_isError){StopEngine();return -1;}
	return 0;
}


void multi::CEngine::StopEngine()
{
	if(!m_shouldRun){return;}
	m_shouldRun = 0;
	m_semaEngine.post();
	m_threadForEngine.join();
}


#ifdef _WIN32
#define MS_VC_EXCEPTION 0x406d1388
typedef struct tagTHREADNAME_INFO
{
	DWORD dwType;        // must be 0x1000
	LPCSTR szName;       // pointer to name (in same addr space)
	DWORD dwThreadID;    // thread ID (-1 caller thread)
	DWORD dwFlags;       // reserved for future use, most be zero
} THREADNAME_INFO;
#else
#ifndef _GNU_SOURCE
#define _GNU_SOURCE             /* See feature_test_macros(7) */
#endif
#include <pthread.h>
#endif


void multi::CEngine::EngineThread()
{
	EngineTask* pTask;

#ifdef _WIN32
	THREADNAME_INFO info;
	info.dwType = 0x1000;
	info.szName = "multi::CEngine::EngineThread";
	//info.dwThreadID = GetThreadId(a_target_thread->thrd);
	info.dwThreadID = GetCurrentThreadId();
	info.dwFlags = 0;

	__try {
		RaiseException(MS_VC_EXCEPTION, 0, sizeof(info) / sizeof(DWORD), (CONST ULONG_PTR*)&info);
	}
	__except (EXCEPTION_CONTINUE_EXECUTION) {
	}
	//return 0;
#else
	pthread_t curThread = pthread_self();
	pthread_setname_np(&curThread, "multi::CEngine::EngineThread");
#endif

	m_pEngine = engOpen(NULL);
	if(!m_pEngine){m_isError=1;return;}
	engSetVisible(m_pEngine, true);
	m_isStarted = 1;

	while(m_shouldRun){
		m_isEngineRunning = 0;
		m_semaEngine.wait();
		m_isEngineRunning = 1;
		while (m_shouldRun && m_fifoTasks.Extract(&pTask)) {
			runFunctionInEngineThread(pTask);
		}
	}

	engClose(m_pEngine);
	m_pEngine = NULL;

}


void multi::CEngine::runFunctionInEngineThread(EngineTask* a_pTask)
{
	int  i, nOffset(0);
	char vcEvalStringBuffer[EVAL_STRING_BUFFER_LENGTH_MIN1+1];
	char argumentName[INP_BUFF_LEN_MIN1+1];
	
	engEvalString(m_pEngine, "lasterror reset");
	a_pTask->taskStatus = TaskStatus::Running;

	if(a_pTask->numberOfOutputs){
		GenerateInputOrOutName(a_pTask->taskNumber,0,false, argumentName);
		nOffset += snprintf(vcEvalStringBuffer+nOffset, EVAL_STRING_BUFFER_LENGTH_MIN1-nOffset,"[%s",argumentName);
		for(i=1;i<a_pTask->numberOfOutputs;++i){
			GenerateInputOrOutName(a_pTask->taskNumber, i, false, argumentName);
			nOffset += snprintf(vcEvalStringBuffer+nOffset, EVAL_STRING_BUFFER_LENGTH_MIN1-nOffset,",%s",argumentName);
		}
		nOffset += snprintf(vcEvalStringBuffer+nOffset, EVAL_STRING_BUFFER_LENGTH_MIN1-nOffset,"]=");
	}

	nOffset += snprintf(vcEvalStringBuffer+nOffset, EVAL_STRING_BUFFER_LENGTH_MIN1-nOffset,"%s",a_pTask->funcName.c_str());

	if(a_pTask ->numberOfInputs>0){
		GenerateInputOrOutName(a_pTask->taskNumber, 0, true, argumentName);
		nOffset += snprintf(vcEvalStringBuffer + nOffset, EVAL_STRING_BUFFER_LENGTH_MIN1 - nOffset, "(%s", argumentName);
		engPutVariable(m_pEngine, argumentName,(a_pTask->inputs)[0]);
		//mxDestroyArray((a_pTask->inputsOrOutputs)[i]);
		//(a_pTask->inputsOrOutputs)[i] = NULL;
		for (i = 1; i < a_pTask->numberOfInputs; ++i) {
			GenerateInputOrOutName(a_pTask->taskNumber, i, true, argumentName);
			nOffset += snprintf(vcEvalStringBuffer+nOffset, EVAL_STRING_BUFFER_LENGTH_MIN1-nOffset,",%s",argumentName);
			engPutVariable(m_pEngine, argumentName,(a_pTask->inputs)[i]);
			//mxDestroyArray((a_pTask->inputsOrOutputs)[i]);
			//(a_pTask->inputsOrOutputs)[i]=NULL;
		}
		nOffset += snprintf(vcEvalStringBuffer + nOffset, EVAL_STRING_BUFFER_LENGTH_MIN1 - nOffset, ");");
	}

	engEvalString(m_pEngine, vcEvalStringBuffer);
	mxClassID nVal;

	for (i = 0; i < a_pTask->numberOfOutputs; ++i) {
		GenerateInputOrOutName(a_pTask->taskNumber, i, false, argumentName);
		(a_pTask->outputs)[i]=engGetVariable(m_pEngine, argumentName);
		nVal = mxGetClassID((a_pTask->outputs)[i]);
	}

	m_nLastFinishedtaskNumber = (uint64_t)a_pTask->taskNumber;
	a_pTask->taskStatus = TaskStatus::Stopped;
	if(a_pTask->pSemaToInform){a_pTask->pSemaToInform->post();a_pTask->pSemaToInform=NULL;}
}



void multi::CEngine::addFunction(int a_nTaskNumber,const char* a_functionName, int a_nNumOuts, int a_nNumInps, const mxArray*a_Inputs[], ::common::UnnamedSemaphoreLite* a_pSemaToInform, int a_nIndex)
{
	EngineTask* pNewTask = new EngineTask(a_nTaskNumber,a_functionName,a_nNumOuts,a_nNumInps,a_Inputs, a_pSemaToInform, a_nIndex);  // new will  throw exception
	m_pLastTask = pNewTask;
	m_hashTasks.AddEntry(&a_nTaskNumber,4, pNewTask);
	m_fifoTasks.AddElement(pNewTask);
	pNewTask->itemForIteration = m_listAllTasks.AddData(pNewTask);
	m_semaEngine.post();
}


bool multi::CEngine::addFunctionIfFree(int a_nTaskNumber, const char* a_functionName, int a_nNumOuts, int a_nNumInps, const mxArray*a_Inputs[], ::common::UnnamedSemaphoreLite* a_pSemaToInform, int a_nIndex)
{
	if(!m_isEngineRunning){
		addFunction(a_nTaskNumber,a_functionName,a_nNumOuts,a_nNumInps,a_Inputs, a_pSemaToInform, a_nIndex);
		return true;
	}
	return false;
}


void multi::CEngine::DeleteTask(int a_nTaskNumber)
{
	EngineTask* pTask;

	if(m_hashTasks.RemoveEntry2(&a_nTaskNumber,4,&pTask)){
		if(pTask==m_pLastTask){m_pLastTask=NULL;}
		delete pTask;
	}
}


void multi::CEngine::DeleteLastTask()
{
	if(m_pLastTask){
		m_hashTasks.RemoveEntry(&(m_pLastTask->taskNumber),4);
		delete m_pLastTask;
		m_pLastTask = NULL;
	}
}


#define REPLACE_BY_INDEX		"--replace-by-index"


/*///////////////////////////////////////////////////////////////////////////////////////////////////////*/
multi::CEngine::EngineTask::EngineTask(
	int a_nTaskNumber, const char* a_functionName, int a_nNumOuts, int a_nNumInps, 
	const mxArray*a_Inputs[], ::common::UnnamedSemaphoreLite* a_pSemaToInform, int a_nIndex)
	:
	taskNumber(a_nTaskNumber),
	numberOfOutputs(a_nNumOuts),
	numberOfInputs(a_nNumInps),
	funcName(a_functionName)
{
	static const size_t  scunReplaceStrLengthPlus1(strlen(REPLACE_BY_INDEX)+1);
	char vcPossibleReplaceBuffer[128];
	this->index = (uint64_t)a_nIndex;
	this->pSemaToInform = a_pSemaToInform;
	this->itemForIteration = NULL;
	this->taskStatus = multi::CEngine::TaskStatus::Stopped;
	this->outputs = NULL;
	this->inputs = NULL;

	if(a_nNumOuts >0){
		//this->outputs = (PtrMxArray*)malloc(a_nNumOuts*sizeof(mxArray*));
		this->outputs = new PtrMxArray[a_nNumOuts+1];
		HANDLE_MEM_DEF(this->outputs);
		this->outputs[a_nNumOuts] = NULL;
	}


	if(a_nNumInps >0){
		//this->inputs = (mxArray**)calloc(a_nNumInps,sizeof(mxArray*));
		this->inputs = new PtrMxArray[a_nNumInps];
		HANDLE_MEM_DEF(this->inputs);
		for(int i(0);i<a_nNumInps;++i){
			if(mxIsChar(a_Inputs[i])){
				mxGetString(a_Inputs[i], vcPossibleReplaceBuffer,scunReplaceStrLengthPlus1);
				if(strncmp(vcPossibleReplaceBuffer,REPLACE_BY_INDEX, scunReplaceStrLengthPlus1)==0){
					(this->inputs)[i] = mxCreateNumericMatrix(1,1,mxUINT32_CLASS, mxREAL);
					HANDLE_MEM_DEF((this->inputs)[i]);
					*( (uint32_t*)mxGetData( (this->inputs)[i]) ) = (uint32_t)a_nIndex+1;
					continue;
				}
			}
			(this->inputs)[i]= mxDuplicateArray(a_Inputs[i]);
			HANDLE_MEM_DEF((this->inputs)[i]);
		}
	}

}



multi::CEngine::EngineTask::~EngineTask()
{
	//int i;
#if 0
	for (i=0; i < this->numberOfInputs; ++i) {
		if(this->inputs[i]){
			mxDestroyArray(this->inputs[i]);
		}
	}


	for (i = 0; i < this->numberOfOutputs; ++i) {
		if (this->outputs[i]) {
			mxDestroyArray(this->outputs[i]);
		}
	}

#endif
}


/*//////////////////////////////////////////////////////////////////////////////////////////////*/

static void GenerateInputOrOutName(int a_nTaskNumber, int a_nIndex, bool a_bIsInput, char* a_pcBuffer)
{
	const char* cpcAddString = a_bIsInput ? "input__" : "output__";
	snprintf(a_pcBuffer, INP_BUFF_LEN_MIN1, "argument_t%.5d_i%.3d_%s", a_nTaskNumber, a_nIndex, cpcAddString);
}
