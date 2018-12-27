//
// file:		multi_engine.cpp
//

#include "multi_engine.hpp"
#include <stdio.h>
#include <stdlib.h>
#include <mex.h>
#include <signal.h>

#define IOI_INDEX_FOR_EXCEPTION		100000

#ifdef _WIN32
#include <Windows.h>
#define INTERRUPT_INP	ULONG_PTR
#define SleepMsIntr(_x)		SleepEx((_x),TRUE)	
#define ENGINE_START_STRING NULL
#else
#include <unistd.h>
#define INTERRUPT_INP	int
#define SleepMsIntr(_x)		usleep(1000*(_x))
#define ENGINE_START_STRING "init_root_and_call matlab_R2016b"
#endif

#define REPLACE_BY_INDEX				"--replace-by-index"
#define	OUT_NAME						"out__"
#define	INP_NAME						"in__"
#define	OUT_NAME_BS_3					OUT_NAME "as_byteStream__"
#define INP_BUFF_LEN_MIN1				127
#define EVAL_STRING_BUFFER_LENGTH_MIN1	2047
#ifndef HANDLE_MEM_DEF
#define HANDLE_MEM_DEF(_pointer,...)	do{if(!(_pointer)){throw "low memory";}}while(0)
#endif

static void GenerateInputOrOutName(int a_nTaskNumber, int a_nEngineNumber, int a_nSubTaskIndex, int a_nInOrOutIndex, const char* a_cpcExtraString, char* a_pcBuffer);
#define GenerateInputOrOutNameM( _pTask,_indexOutOrInp,_extraStr,_argumentName) \
			GenerateInputOrOutName((_pTask)->m_pParent->taskNumber(), (int)m_nEngineNumber, (_pTask)->m_subTaskIndex, (_indexOutOrInp), (_extraStr), (_argumentName))


multi::CEngine::CEngine(int a_nEngineNumber,::common::UnnamedSemaphoreLite* a_pSemaForStartingCalc, 
	::common::listN::Fifo<SubTask*>* a_pFifo, volatile int* a_pnRun)
	:
	m_pEngine(NULL),
	m_pSemaToStartCalc(a_pSemaForStartingCalc),
	m_pFifoToDoTasks(a_pFifo),
	m_pnRun(a_pnRun)
{
	m_nLastFinishedtaskNumber=m_isEngineRunning=m_isError=m_shouldRun = m_isStarted = 0;
	m_nEngineNumber = (uint64_t)a_nEngineNumber;

	StartEngine();
}


multi::CEngine::~CEngine()
{
	StopEngine();
}


int	multi::CEngine::engineNumber()const
{
	return (int)m_nEngineNumber;
}


uint64_t multi::CEngine::isRunning()const
{
	return m_isEngineRunning;
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


static void InterruptFunction(INTERRUPT_INP) {}


void multi::CEngine::StopEngine()
{
	if(!m_shouldRun){return;}
	m_shouldRun = 0;
	if(*m_pnRun){
#ifdef _WIN32
		QueueUserAPC(&InterruptFunction,m_threadForEngine.native_handle(),NULL);
#else
		pthread_kill(m_threadForEngine.native_handle(),SIGPIPE);
#endif
	}
	//m_semaEngine.post();
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
	SubTask* pTask;

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
	struct sigaction newAction;
	pthread_t curThread = pthread_self();
    pthread_setname_np(curThread, "multi::CEngine::EngineThread");

	newAction.sa_flags = 0;
	sigemptyset(&newAction.sa_mask);
	newAction.sa_restorer = NULL;
	newAction.sa_handler = InterruptFunction;

	sigaction(SIGPIPE, &newAction, NULL);
#endif

    m_pEngine = engOpen(ENGINE_START_STRING);
	if(!m_pEngine){m_isError=1;return;}
    engSetVisible(m_pEngine, false);
    engSetVisible(m_pEngine, true);
	m_isStarted = 1;

	while(m_shouldRun && *m_pnRun){
		m_isEngineRunning = 0;
		m_pSemaToStartCalc->wait();
		if (m_pFifoToDoTasks->Extract(&pTask)) {
			runFunctionInEngineThread(pTask);
			pTask->AddTaskToDoneTasks();
		}
	}

	engClose(m_pEngine);
	m_pEngine = NULL;

}


static inline void SetVariableToEngine(::Engine* a_pEngine, const mxArray* a_pVariable, const char* a_cpcVariableName, int a_nIndex)
{
	static const size_t  scunReplaceStrLengthPlus1(strlen(REPLACE_BY_INDEX) + 1);

	if (mxIsChar(a_pVariable)) {
		char vcPossibleReplaceBuffer[128];
		mxGetString(a_pVariable, vcPossibleReplaceBuffer, scunReplaceStrLengthPlus1);
		if (strncmp(vcPossibleReplaceBuffer, REPLACE_BY_INDEX, scunReplaceStrLengthPlus1) == 0) {
			mxArray* pNewVar = mxCreateNumericMatrix(1, 1, mxUINT32_CLASS, mxREAL);
			HANDLE_MEM_DEF(pNewVar," ");
			*((uint32_t*)mxGetData(pNewVar)) = (uint32_t)a_nIndex;
			engPutVariable(a_pEngine, a_cpcVariableName, pNewVar);
			return;
		}
	}

	engPutVariable(a_pEngine, a_cpcVariableName, a_pVariable);
}



void multi::CEngine::runFunctionInEngineThread(SubTask* a_pTask)
{
	mxArray* pOut;
	size_t unOutSize;
	const int cnOutputs= (int)a_pTask->m_outputs.size();
	const int cnInputs = (int)a_pTask->m_pParent->numberOfInputs();
	int  i, nOffset(0);
	char vcEvalStringBuffer[EVAL_STRING_BUFFER_LENGTH_MIN1+1];
	char argumentName[INP_BUFF_LEN_MIN1+1];
	char argumentNameBS[INP_BUFF_LEN_MIN1+1];
	
	engEvalString(m_pEngine, "lasterror reset");
	engEvalString(m_pEngine, "clear all");
	//a_pTask->taskStatus = TaskStatus::Running;
	a_pTask->m_isError = 0;

	if(cnOutputs>0){
		GenerateInputOrOutNameM(a_pTask,0, OUT_NAME, argumentName);
		nOffset += snprintf(vcEvalStringBuffer+nOffset, EVAL_STRING_BUFFER_LENGTH_MIN1-nOffset,"[%s",argumentName);
		for(i=1;i<cnOutputs;++i){
			GenerateInputOrOutNameM(a_pTask,i, OUT_NAME, argumentName);
			nOffset += snprintf(vcEvalStringBuffer+nOffset, EVAL_STRING_BUFFER_LENGTH_MIN1-nOffset,",%s",argumentName);
		}
		nOffset += snprintf(vcEvalStringBuffer+nOffset, EVAL_STRING_BUFFER_LENGTH_MIN1-nOffset,"]=");
	}

	nOffset += snprintf(vcEvalStringBuffer+nOffset, EVAL_STRING_BUFFER_LENGTH_MIN1-nOffset,"%s",a_pTask->m_pParent->functionName());

	if(cnInputs>0){
		const mxArray** ppInputs = a_pTask->m_pParent->inputs2();
		GenerateInputOrOutNameM(a_pTask, 0, INP_NAME, argumentName);
		nOffset += snprintf(vcEvalStringBuffer + nOffset, EVAL_STRING_BUFFER_LENGTH_MIN1 - nOffset, "(%s", argumentName);
		SetVariableToEngine(m_pEngine, ppInputs[0],argumentName,a_pTask->m_subTaskIndex);
		for (i = 1; i < cnInputs; ++i) {
			GenerateInputOrOutNameM(a_pTask, i, INP_NAME, argumentName);
			nOffset += snprintf(vcEvalStringBuffer+nOffset, EVAL_STRING_BUFFER_LENGTH_MIN1-nOffset,",%s",argumentName);
			SetVariableToEngine(m_pEngine, ppInputs[i], argumentName, a_pTask->m_subTaskIndex);
		}
		nOffset += snprintf(vcEvalStringBuffer + nOffset, EVAL_STRING_BUFFER_LENGTH_MIN1 - nOffset, ");");
	}

	engEvalString(m_pEngine, vcEvalStringBuffer);
	GenerateInputOrOutNameM(a_pTask, IOI_INDEX_FOR_EXCEPTION, OUT_NAME, argumentName);
	snprintf(vcEvalStringBuffer, EVAL_STRING_BUFFER_LENGTH_MIN1, "%s=lasterror;", argumentName);
	engEvalString(m_pEngine, vcEvalStringBuffer);
	for (i = 0; i < cnOutputs; ++i) {
		GenerateInputOrOutNameM(a_pTask, i, OUT_NAME, argumentName);
		GenerateInputOrOutNameM(a_pTask, i, OUT_NAME_BS_3, argumentNameBS);
		snprintf(vcEvalStringBuffer, EVAL_STRING_BUFFER_LENGTH_MIN1, "%s=getByteStreamFromArray(%s);", argumentNameBS,argumentName);
		engEvalString(m_pEngine, vcEvalStringBuffer);

		pOut = engGetVariable(m_pEngine, argumentNameBS);
		if (pOut) {
			unOutSize = mxGetNumberOfElements(pOut);
			a_pTask->m_outputs[i].resize(unOutSize);
			if (unOutSize) { memcpy(a_pTask->m_outputs[i].buffer(), mxGetData(pOut), unOutSize); }
			mxDestroyArray(pOut);
		}
		else { 

			GenerateInputOrOutNameM(a_pTask, IOI_INDEX_FOR_EXCEPTION, OUT_NAME, argumentName);
			GenerateInputOrOutNameM(a_pTask, 0, OUT_NAME_BS_3, argumentNameBS);
			snprintf(vcEvalStringBuffer, EVAL_STRING_BUFFER_LENGTH_MIN1, "%s=getByteStreamFromArray(%s);", argumentNameBS, argumentName);
			engEvalString(m_pEngine, vcEvalStringBuffer);
			pOut = engGetVariable(m_pEngine, argumentNameBS);

			if(pOut){
				unOutSize = mxGetNumberOfElements(pOut);
				a_pTask->m_outputs[0].resize(unOutSize);
				if (unOutSize) { memcpy(a_pTask->m_outputs[0].buffer(), mxGetData(pOut), unOutSize); }  // error should go to first place
				mxDestroyArray(pOut);
			}
			else {a_pTask->m_outputs[i].resize(0);a_pTask->m_isError=1;}
			a_pTask->m_isError = 1;
			break;

		}

	}

	m_nLastFinishedtaskNumber = (uint64_t)a_pTask->m_pParent->taskNumber();
}



/*///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////*/

static void GenerateInputOrOutName(int a_nTaskNumber, int a_nEngineNumber, int a_nSubTaskIndex, int a_nInOrOutIndex, const char* a_cpcExtraString, char* a_pcBuffer)
{
	snprintf(a_pcBuffer, INP_BUFF_LEN_MIN1, "argument__t%.5d_e%.3d_sti%.3d_ioi%.3d_%s", a_nTaskNumber,a_nEngineNumber, a_nSubTaskIndex, a_nInOrOutIndex,a_cpcExtraString);
}
