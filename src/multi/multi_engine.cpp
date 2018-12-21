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
}


multi::CEngine::~CEngine()
{
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


void multi::CEngine::EngineThread()
{
	EngineTask* pTask;

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
		nOffset = snprintf(vcEvalStringBuffer + nOffset, EVAL_STRING_BUFFER_LENGTH_MIN1 - nOffset, "(%s", argumentName);
		engPutVariable(m_pEngine, argumentName,(a_pTask->inputsOrOutputs)[0]);
		mxDestroyArray((a_pTask->inputsOrOutputs)[i]);
		(a_pTask->inputsOrOutputs)[i] = NULL;
		for (i = 1; i < a_pTask->numberOfInputs; ++i) {
			GenerateInputOrOutName(a_pTask->taskNumber, i, true, argumentName);
			nOffset += snprintf(vcEvalStringBuffer+nOffset, EVAL_STRING_BUFFER_LENGTH_MIN1-nOffset,",%s",argumentName);
			engPutVariable(m_pEngine, argumentName,(a_pTask->inputsOrOutputs)[i]);
			mxDestroyArray((a_pTask->inputsOrOutputs)[i]);
			(a_pTask->inputsOrOutputs)[i]=NULL;
		}
		nOffset += snprintf(vcEvalStringBuffer + nOffset, EVAL_STRING_BUFFER_LENGTH_MIN1 - nOffset, ");");
	}

	engEvalString(m_pEngine, vcEvalStringBuffer);

	for (i = 0; i < a_pTask->numberOfOutputs; ++i) {
		GenerateInputOrOutName(a_pTask->taskNumber, i, false, argumentName);
		(a_pTask->inputsOrOutputs)[i]=engGetVariable(m_pEngine, argumentName);
	}

	m_nLastFinishedtaskNumber = (uint64_t)a_pTask->taskNumber;
	a_pTask->taskStatus = TaskStatus::Stopped;
	if(a_pTask->pSemaToInform){a_pTask->pSemaToInform->post();a_pTask->pSemaToInform=NULL;}
}



void multi::CEngine::addFunction(int a_nTaskNumber,const char* a_functionName, int a_nNumOuts, int a_nNumInps, const mxArray*a_Inputs[], ::common::UnnamedSemaphoreLite* a_pSemaToInform)
{
	EngineTask* pNewTask = new EngineTask(a_nTaskNumber,a_functionName,a_nNumOuts,a_nNumInps,a_Inputs, a_pSemaToInform);  // new will  throw exception
	m_pLastTask = pNewTask;
	m_hashTasks.AddEntry(&a_nTaskNumber,4, pNewTask);
	m_fifoTasks.AddElement(pNewTask);
	pNewTask->itemForIteration = m_listAllTasks.AddData(pNewTask);
	m_semaEngine.post();
}


bool multi::CEngine::addFunctionIfFree(int a_nTaskNumber, const char* a_functionName, int a_nNumOuts, int a_nNumInps, const mxArray*a_Inputs[], ::common::UnnamedSemaphoreLite* a_pSemaToInform)
{
	if(!m_isEngineRunning){
		addFunction(a_nTaskNumber,a_functionName,a_nNumOuts,a_nNumInps,a_Inputs, a_pSemaToInform);
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



/*///////////////////////////////////////////////////////////////////////////////////////////////////////*/
multi::CEngine::EngineTask::EngineTask(int a_nTaskNumber, const char* a_functionName, int a_nNumOuts, int a_nNumInps, const mxArray*a_Inputs[], ::common::UnnamedSemaphoreLite* a_pSemaToInform)
	:
	taskNumber(a_nTaskNumber),
	numberOfOutputs(a_nNumOuts),
	numberOfInputs(a_nNumInps),
	funcName(a_functionName),
	arraySize(a_nNumInps>a_nNumOuts?a_nNumInps:a_nNumOuts)
{
	this->pSemaToInform = a_pSemaToInform;
	this->itemForIteration = NULL;
	this->taskStatus = multi::CEngine::TaskStatus::Stopped;
	this->inputsOrOutputs = NULL;

	if(arraySize>0){
		this->inputsOrOutputs = (mxArray**)calloc(arraySize,sizeof(mxArray*));
		HANDLE_MEM_DEF(this->inputsOrOutputs);
		for(int i(0);i<a_nNumInps;++i){
			(this->inputsOrOutputs)[i]= mxDuplicateArray(a_Inputs[i]);
			HANDLE_MEM_DEF((this->inputsOrOutputs)[i]);
		}
	}

}



multi::CEngine::EngineTask::~EngineTask()
{
	for (int i(0); i < this->arraySize; ++i) {
		if(this->inputsOrOutputs[i]){
			mxDestroyArray(this->inputsOrOutputs[i]);
		}
	}
}


/*//////////////////////////////////////////////////////////////////////////////////////////////*/

static void GenerateInputOrOutName(int a_nTaskNumber, int a_nIndex, bool a_bIsInput, char* a_pcBuffer)
{
	const char* cpcAddString = a_bIsInput ? "input__" : "output__";
	snprintf(a_pcBuffer, INP_BUFF_LEN_MIN1, "argument_t%.5d_i%.3d_%s", a_nTaskNumber, a_nIndex, cpcAddString);
}
