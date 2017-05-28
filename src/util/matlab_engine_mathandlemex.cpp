/*
 *	File: matlab_engine_mathandlemex.cpp
 *
 *	Created on: 12 Apr, 2017
 *	Author    : Davit Kalantaryan (Email: davit.kalantaryan@desy.de)
 *
 *
 */

#include "matlab_engine_mathandlemex.hpp"
#include <stddef.h>
#include <listener_on_gui_thread.hpp>
#include "matlab_bytestream_routines.h"


matlab::engine::MatHandleMex::MatHandleMex()
	:
	m_pListener(NULL),
	m_fifoJobs(8)
{
}


matlab::engine::MatHandleMex::~MatHandleMex()
{
}


int matlab::engine::MatHandleMex::Start(const std::string& /*engineCommand*/)
{
	if (m_pListener) {return START_RET::ALREADY_RUN;}	
	m_pListener = CreateMListener();
	if (!m_pListener){return START_RET::ENG_ERROR;}
	return START_RET::STARTED;
}


void matlab::engine::MatHandleMex::Stop()
{
	if (m_pListener)
	{
		DestroyMListener(m_pListener);
		m_pListener = NULL;
	}
}


void matlab::engine::MatHandleMex::SyncCallOnMatlabThread(void* a_owner, TypeClbK a_fpClb, void* a_arg)
{

	if (m_pListener) {
		AddMatlabJob(a_owner, a_fpClb, a_arg);
		SynchronCallOnGUIthread(m_pListener, this, &MatHandleMex::ListenerCallbackStatic);
	}
}


void matlab::engine::MatHandleMex::AsyncCallOnMatlabThread(void* a_owner, TypeClbK a_fpClb, void* a_arg)
{

	if (m_pListener) {
		AddMatlabJob(a_owner, a_fpClb, a_arg);
		PostJobForGUIthread(m_pListener, this, &MatHandleMex::ListenerCallbackStatic);
	}
}


void* matlab::engine::MatHandleMex::ListenerCallbackStatic(void* a_arg)
{
	MatHandleMex* pThis = (MatHandleMex*)a_arg;
	pThis->HandleAllJobs();
	return NULL;
}


void matlab::engine::MatHandleMex::AddMatlabJob(void* a_owner, TypeClbK a_fpClb, void* a_arg)
{
	SLsnCallbackItem newItem;
	newItem.owner = a_owner;
	newItem.clbk = a_fpClb;
	newItem.arg = a_arg;
	m_fifoJobs.AddElement(newItem);
}


void matlab::engine::MatHandleMex::HandleAllJobs(void)
{
	SLsnCallbackItem	clbkItem;

	while (m_fifoJobs.Extract(&clbkItem))
	{
		(*clbkItem.clbk)(clbkItem.owner, clbkItem.arg);
	}

}
