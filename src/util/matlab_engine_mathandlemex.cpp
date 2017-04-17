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
	m_pListener(NULL)
{
}


matlab::engine::MatHandleMex::~MatHandleMex()
{
}


void matlab::engine::MatHandleMex::Start()
{
	if (!m_pListener)
	{
		m_pListener = CreateMListener();
		if (m_pListener)
		{
			//m_nThreadID = gettid();
		}
	}//if (!m_pListener)
}


void matlab::engine::MatHandleMex::Stop()
{
	if (m_pListener)
	{
		DestroyMListener(m_pListener);
		m_pListener = NULL;
	}
}


void matlab::engine::MatHandleMex::CallOnMatlabThread(void* a_owner, TypeClbK a_fpClb, void* a_arg)
{

	if (m_pListener) {
		AddMatlabJob(a_owner, a_fpClb, a_arg);
		SynchronCallOnGUIthread(m_pListener, this, &MatHandleMex::ListenerCallbackStatic);
	}
}


void* matlab::engine::MatHandleMex::ListenerCallbackStatic(void* a_arg)
{
	MatHandleMex* pThis = (MatHandleMex*)a_arg;
	pThis->HandleAllJobs();
	return NULL;
}
