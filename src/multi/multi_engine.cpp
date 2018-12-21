//
// file:		multi_engine.cpp
//

#include "multi_engine.hpp"
#include <stdio.h>
#include <stdlib.h>
#include <mex.h>

#ifdef _WIN32
#include <Windows.h>
#define SleepMsIntr(_x)		SleepEx((_x),TRUE)	
#else
#include <unistd.h>
#define SleepMsIntr(_x)		usleep(1000*(_x))
#endif

#define	OUT_NAME						"out__"
#define	INP_NAME						"in__"
#define	OUT_NAME_BS_2					OUT_NAME "as_byteStream__"
#define INP_BUFF_LEN_MIN1				127
#define EVAL_STRING_BUFFER_LENGTH_MIN1	2047
#ifndef HANDLE_MEM_DEF
#define HANDLE_MEM_DEF(_pointer,...)	do{if(!(_pointer)){throw "low memory";}}while(0)
#endif

static void GenerateInputOrOutName(int a_nEngineNumber,int a_nTaskNumber,int a_nIndex, const char* a_cpcExtraString, char* a_pcBuffer);


multi::CEngine::CEngine(int a_nEngineNumber)
	:
	m_pEngine(NULL)
{
	m_nLastFinishedtaskNumber=m_isEngineRunning=m_isError=m_shouldRun = m_isStarted = 0;
	m_nEngineNumber = (uint64_t)a_nEngineNumber;

	StartEngine();
}


multi::CEngine::~CEngine()
{
	multi::CEngine::EngineTask* pTask;
	StopEngine();
	while(m_fifoToDoTasks.Extract(&pTask)){
		delete pTask;
	}
}


int	multi::CEngine::engineNumber()const
{
	return (int)m_nEngineNumber;
}


uint64_t multi::CEngine::isRunning()const
{
	return m_isEngineRunning;
}


const multi::CEngine::EngineTask* multi::CEngine::getFirstReadyTask()const
{
	multi::CEngine::EngineTask* pTask;
	if(m_fifoAlreadyDoneTasks.Extract(&pTask)){
		m_fifoDummyTasks.AddElement(pTask);
		return pTask;
	}
	return NULL;
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
		while (m_shouldRun && m_fifoToDoTasks.Extract(&pTask)) {
			runFunctionInEngineThread(pTask);
			m_fifoAlreadyDoneTasks.AddElement(pTask);
		}
	}

	engClose(m_pEngine);
	m_pEngine = NULL;

}


void multi::CEngine::runFunctionInEngineThread(EngineTask* a_pTask)
{
	mxArray* pOut;
	size_t unOutSize;
	const int cnOutputs= (int)a_pTask->outputs2.size();
	const int cnInputs = (int)a_pTask->inputs.size();
	int  i, nOffset(0);
	char vcEvalStringBuffer[EVAL_STRING_BUFFER_LENGTH_MIN1+1];
	char argumentName[INP_BUFF_LEN_MIN1+1];
	
	engEvalString(m_pEngine, "lasterror reset");
	a_pTask->taskStatus = TaskStatus::Running;

	if(cnOutputs>0){
		GenerateInputOrOutName((int)m_nEngineNumber,a_pTask->taskNumber,0, OUT_NAME, argumentName);
		nOffset += snprintf(vcEvalStringBuffer+nOffset, EVAL_STRING_BUFFER_LENGTH_MIN1-nOffset,"[%s",argumentName);
		for(i=1;i<cnOutputs;++i){
			GenerateInputOrOutName((int)m_nEngineNumber,a_pTask->taskNumber,i, OUT_NAME, argumentName);
			nOffset += snprintf(vcEvalStringBuffer+nOffset, EVAL_STRING_BUFFER_LENGTH_MIN1-nOffset,",%s",argumentName);
		}
		nOffset += snprintf(vcEvalStringBuffer+nOffset, EVAL_STRING_BUFFER_LENGTH_MIN1-nOffset,"]=");
	}

	nOffset += snprintf(vcEvalStringBuffer+nOffset, EVAL_STRING_BUFFER_LENGTH_MIN1-nOffset,"%s",a_pTask->funcName.c_str());

	if(cnInputs>0){
		GenerateInputOrOutName((int)m_nEngineNumber,a_pTask->taskNumber, 0, INP_NAME, argumentName);
		nOffset += snprintf(vcEvalStringBuffer + nOffset, EVAL_STRING_BUFFER_LENGTH_MIN1 - nOffset, "(%s", argumentName);
		engPutVariable(m_pEngine, argumentName,(a_pTask->inputs)[0]);
		mxDestroyArray((a_pTask->inputs)[0]);
		(a_pTask->inputs)[0] = NULL;
		for (i = 1; i < cnInputs; ++i) {
			GenerateInputOrOutName((int)m_nEngineNumber,a_pTask->taskNumber, i, INP_NAME, argumentName);
			nOffset += snprintf(vcEvalStringBuffer+nOffset, EVAL_STRING_BUFFER_LENGTH_MIN1-nOffset,",%s",argumentName);
			engPutVariable(m_pEngine, argumentName,(a_pTask->inputs)[i]);
			mxDestroyArray((a_pTask->inputs)[i]);
			(a_pTask->inputs)[i]=NULL;
		}
		nOffset += snprintf(vcEvalStringBuffer + nOffset, EVAL_STRING_BUFFER_LENGTH_MIN1 - nOffset, ");");
	}

	engEvalString(m_pEngine, vcEvalStringBuffer);

	for (i = 0; i < cnOutputs; ++i) {
		GenerateInputOrOutName((int)m_nEngineNumber,a_pTask->taskNumber, i, OUT_NAME, argumentName);
		snprintf(vcEvalStringBuffer, EVAL_STRING_BUFFER_LENGTH_MIN1, OUT_NAME_BS_2 "=getByteStreamFromArray(%s);",argumentName);
		engEvalString(m_pEngine, vcEvalStringBuffer);

		pOut = engGetVariable(m_pEngine, OUT_NAME_BS_2);
		if (pOut) {unOutSize = mxGetNumberOfElements(pOut);}
		else { unOutSize = 0; }

		a_pTask->outputs2[i]->resize(unOutSize);
		if(unOutSize){memcpy(a_pTask->outputs2[i]->buffer(), mxGetData(pOut), unOutSize);}
	}

	m_nLastFinishedtaskNumber = (uint64_t)a_pTask->taskNumber;
	a_pTask->taskStatus = TaskStatus::Stopped;
	if(a_pTask->pSemaToInform){a_pTask->pSemaToInform->post();a_pTask->pSemaToInform=NULL;}
}


void multi::CEngine::StartCalc()
{
	m_semaEngine.post();
}


void multi::CEngine::addFunction2(int a_nTaskNumber,const char* a_functionName, int a_nNumOuts, int a_nNumInps, const mxArray*a_Inputs[], ::common::UnnamedSemaphoreLite* a_pSemaToInform, int a_nIndex)
{
	EngineTask* pTask;

	m_isEngineRunning = 1;
	if(!m_fifoDummyTasks.Extract(&pTask)){pTask = new EngineTask;}
	pTask->init(a_nTaskNumber,a_functionName,a_nNumOuts,a_nNumInps,a_Inputs, a_pSemaToInform, a_nIndex);  // new will  throw exception
	m_fifoToDoTasks.AddElement(pTask);
}


bool multi::CEngine::addFunctionIfFree2(int a_nTaskNumber, const char* a_functionName, int a_nNumOuts, int a_nNumInps, const mxArray*a_Inputs[], ::common::UnnamedSemaphoreLite* a_pSemaToInform, int a_nIndex)
{
	if(!m_isEngineRunning){
		addFunction2(a_nTaskNumber,a_functionName,a_nNumOuts,a_nNumInps,a_Inputs, a_pSemaToInform, a_nIndex);
		return true;
	}
	return false;
}


#define REPLACE_BY_INDEX		"--replace-by-index"


/*///////////////////////////////////////////////////////////////////////////////////////////////////////*/
multi::CEngine::EngineTask::EngineTask()
	:
	taskNumber(-1),
	pSemaToInform(NULL)
{
	this->taskStatus = multi::CEngine::TaskStatus::Stopped;
}

multi::CEngine::EngineTask::~EngineTask()
{
	size_t i;
	const size_t cnInputs = (size_t)this->inputs.size();
	const size_t cnOutputs = (size_t)this->outputs2.size();
	for (i=0; i < cnInputs; ++i) {
		if (this->inputs[i]) {
			mxDestroyArray(this->inputs[i]);
		}
	}

	for (i=0; i < cnOutputs; ++i) {
		delete this->outputs2[i];
	}

}


void multi::CEngine::EngineTask::init(
	int a_nTaskNumber, const char* a_functionName, int a_nNumOuts2, int a_nNumInps2, 
	const mxArray*a_Inputs[], ::common::UnnamedSemaphoreLite* a_pSemaToInform, int a_nIndex)
{
	static const size_t  scunReplaceStrLengthPlus1(strlen(REPLACE_BY_INDEX)+1);
	//const int cnOutputs = (int)this->outputs.size();
	const int cnInputs = (int)this->inputs.size();
	int i;
	char vcPossibleReplaceBuffer[128];

	this->pSemaToInform = a_pSemaToInform;
	this->taskStatus = multi::CEngine::TaskStatus::Stopped;
	this->taskNumber = a_nTaskNumber;
	this->funcName = a_functionName;

	// todo:
	// anyhow deleted in the engine thread, but try to understand reason of crash
#if 1
	for(i=0;i<cnInputs;++i){
		if(this->inputs[i]){mxDestroyArray(this->inputs[i]);this->inputs[i]=NULL;}
	}
#endif


	this->inputs.resize(a_nNumInps2);
	this->resizeOutputs(a_nNumOuts2);

	for (i=0; i < a_nNumInps2; ++i) {
		if (mxIsChar(a_Inputs[i])) {
			mxGetString(a_Inputs[i], vcPossibleReplaceBuffer, scunReplaceStrLengthPlus1);
			if (strncmp(vcPossibleReplaceBuffer, REPLACE_BY_INDEX, scunReplaceStrLengthPlus1) == 0) {
				this->inputs[i] = mxCreateNumericMatrix(1, 1, mxUINT32_CLASS, mxREAL);
				HANDLE_MEM_DEF(this->inputs[i]);
				*((uint32_t*)mxGetData(this->inputs[i])) = (uint32_t)a_nIndex;
				continue;
			}
		}
		this->inputs[i] = mxDuplicateArray(a_Inputs[i]);
		HANDLE_MEM_DEF(this->inputs[i]);
	}
}


void multi::CEngine::EngineTask::resizeOutputs(size_t a_newSize)
{
	const size_t cunOldSize(this->outputs2.size());

	this->outputs2.resize(a_newSize);

	if(a_newSize> cunOldSize){
		for(size_t i(cunOldSize);i<a_newSize;++i){
			this->outputs2[i] = new ResizableStore<uint8_t*>;
		}
	}
}


void multi::CEngine::EngineTask::GetOutputs(int a_nNumOuts, mxArray *a_Outputs[])const
{
	//mxArray* pOutInit;
	mxArray* pByteStream;
	size_t unOutSize;
	const int cnOutputs = (int)this->outputs2.size();
	const int cnNumber = cnOutputs< a_nNumOuts? cnOutputs: a_nNumOuts;

	for(int i(0);i<cnNumber;++i){
		unOutSize = this->outputs2[i]->size();
		if (unOutSize > 0) {
			pByteStream = mxCreateNumericMatrix(1, unOutSize, mxUINT8_CLASS, mxREAL);

			memcpy(mxGetData(pByteStream), this->outputs2[i]->buffer(), unOutSize);
			//if (!mexCallMATLABWithTrap(1, &pOutInit, 1, &pByteStream, "getArrayFromByteStream")) {
			if (!mexCallMATLABWithTrap(1, &a_Outputs[i], 1, &pByteStream, "getArrayFromByteStream")) {
				//a_Outputs[i] = mxDuplicateArray(pOutInit);
				//mxDestroyArray(pOutInit);
			}
			else {
				a_Outputs[i] = mxCreateNumericMatrix(1, 0, mxUINT8_CLASS, mxREAL);
			}

			mxDestroyArray(pByteStream);
		}
		else {
			a_Outputs[0] = mxCreateNumericMatrix(1, 0, mxUINT8_CLASS, mxREAL);
		}
	}
}

/*///////////////////////////////////////////////////////////////////////////////////////////////*/



/*//////////////////////////////////////////////////////////////////////////////////////////////*/

static void GenerateInputOrOutName(int a_nEngineNumber,int a_nTaskNumber, int a_nIndex, const char* a_cpcExtraString, char* a_pcBuffer)
{
	snprintf(a_pcBuffer, INP_BUFF_LEN_MIN1, "argument__e%.3d_t%.4d_i%.3d_%s", a_nEngineNumber,a_nTaskNumber, a_nIndex, a_cpcExtraString);
}
