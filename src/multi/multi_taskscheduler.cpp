// 
// file:	multi_taskscheduler.cpp
// 

#include "multi_taskscheduler.hpp"

static int s_nEngineNumber = 0;


multi::TaskScheduler::TaskScheduler()
	:
	m_taskForGui(this,0)
{
	m_nWork = 1;
}


multi::TaskScheduler::~TaskScheduler()
{
	int i;
	const int cnCurrentNumber = (int)m_vectorEngines.size();
	m_nWork = 0;
	m_semaForStartingCalc.post(cnCurrentNumber);
	for (i = 0; i < cnCurrentNumber; ++i) {
		delete m_vectorEngines[i];
	}
}


multi::Task* multi::TaskScheduler::calcAndWait(int a_nNumOfThreads,const char* a_functionName, int a_nNumOuts, int a_nNumInps, const mxArray*a_Inputs[])
{
	m_taskForGui.PrepareTaskForCalc(a_nNumOfThreads,a_functionName, a_nNumOuts, a_nNumInps, a_Inputs);
	m_fifoSubTasks.AddElements(m_taskForGui.getSubTasksPtr(),a_nNumOfThreads);
	m_semaForStartingCalc.post(a_nNumOfThreads);
	m_semaWhenFinishedForGui.wait();
	return &m_taskForGui;
}


void multi::TaskScheduler::calcAndSaveNoWait(const char* a_cpcTaskName, int a_nNumOfThreads,const char* a_functionName, int a_nNumOuts, int a_nNumInps, const mxArray*a_Inputs[])
{
	common::listN::ListItem<STaskItem>* pItem;
	const uint32_t unStrLenPlus1((uint32_t)strlen(a_cpcTaskName)+1);

	if(!m_savedTasks.FindEntry(a_cpcTaskName, unStrLenPlus1,&pItem)){
		STaskItem aItem;
		aItem.task = new multi::Task(this, 1);
		pItem = m_listSavedTasks.AddData(aItem);
		pItem->data.taskName = (char*)m_savedTasks.AddEntry2(a_cpcTaskName, unStrLenPlus1,pItem);
	}

	pItem->data.task->PrepareTaskForCalc(a_nNumOfThreads, a_functionName, a_nNumOuts, a_nNumInps, a_Inputs);
	m_fifoSubTasks.AddElements(pItem->data.task->getSubTasksPtr(), a_nNumOfThreads);
	m_semaForStartingCalc.post(a_nNumOfThreads);
}


void multi::TaskScheduler::setNumberOfEngines(int a_nNumber)
{
	int i;
	const int cnCurrentNumber = (int)m_vectorEngines.size();
	if(a_nNumber<1){return;}
	else if(a_nNumber>cnCurrentNumber){
		m_vectorEngines.resize(a_nNumber);
		for(i=cnCurrentNumber;i<a_nNumber;++i){
			m_vectorEngines[i]=new CEngine(s_nEngineNumber++,&m_semaForStartingCalc,&m_fifoSubTasks,&m_nWork);
		}
	}
	else if(a_nNumber < cnCurrentNumber){
		for(i= a_nNumber;i< cnCurrentNumber;++i){
			delete m_vectorEngines[i];
		}
		m_vectorEngines.resize(a_nNumber);
	}
}


int multi::TaskScheduler::numberOfEngines()const
{
	return (int)m_vectorEngines.size();
}


void multi::TaskScheduler::FinalizeTask(Task* a_pTask)
{
	if(a_pTask->isForSaving()){
		//a_pTask->
	}
	else{
		m_semaWhenFinishedForGui.post();
	}
}


bool multi::TaskScheduler::GetSavedTask(Task** a_pBuffer)
{
}


bool multi::TaskScheduler::GetSavedTaskAndRemove(Task** a_pBuffer)
{
}
