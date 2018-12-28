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
	uint32_t unStrLenPlus1;
	int i;
	const int cnCurrentNumber = (int)m_vectorEngines.size();
	m_nWork = 0;
	m_semaForStartingCalc.post(cnCurrentNumber);
	for (i = 0; i < cnCurrentNumber; ++i) {
		delete m_vectorEngines[i];
	}

	while(m_listSavedTasks.first()){
		unStrLenPlus1 = (uint32_t)strlen(m_listSavedTasks.first()->data.taskName) + 1;
		m_hashSavedTasks.RemoveEntry(m_listSavedTasks.first()->data.taskName,unStrLenPlus1);
		delete m_listSavedTasks.first()->data.task;
		m_listSavedTasks.RemoveData(m_listSavedTasks.first());
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
	Task* pTask;

	if(!m_hashSavedTasks.FindEntry(a_cpcTaskName, unStrLenPlus1,&pItem)){
		STaskItem aItem;
		aItem.task = new multi::Task(this, 1);
		pItem = m_listSavedTasks.AddData(aItem);
		pItem->data.taskName = (char*)m_hashSavedTasks.AddEntry2(a_cpcTaskName, unStrLenPlus1,pItem);
	}
	pTask = const_cast<Task*>(pItem->data.task);

	pTask->PrepareTaskForCalc(a_nNumOfThreads, a_functionName, a_nNumOuts, a_nNumInps, a_Inputs);
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


bool multi::TaskScheduler::GetSavedTask(const char* a_taskName, contsTaskPtr* a_pBuffer)
{
	const uint32_t cunStrlenPlus1((uint32_t)strlen(a_taskName) + 1);
	common::listN::ListItem<multi::TaskScheduler::STaskItem>* pItem;

	if(m_hashSavedTasks.FindEntry(a_taskName, cunStrlenPlus1,&pItem)){
		*a_pBuffer = pItem->data.task;
		return true;
	}
	return false;
}


bool multi::TaskScheduler::RemoveSavedTask(const char* a_taskName)
{
	const uint32_t cunStrlenPlus1((uint32_t)strlen(a_taskName) + 1);
	common::listN::ListItem<multi::TaskScheduler::STaskItem>* pItem;

	if(m_hashSavedTasks.RemoveEntry2(a_taskName,cunStrlenPlus1,&pItem)){
		delete pItem->data.task;
		m_listSavedTasks.RemoveData(pItem);
		return true;
	}

	return false;
}


const common::listN::ListItem<multi::TaskScheduler::STaskItem>* multi::TaskScheduler::firstSavedTask()const
{
	return m_listSavedTasks.first();
}
