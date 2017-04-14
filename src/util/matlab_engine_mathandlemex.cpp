/*
 *	File: matlab_engine_mathandlebase.cpp
 *
 *	Created on: 12 Apr, 2017
 *	Author    : Davit Kalantaryan (Email: davit.kalantaryan@desy.de)
 *
 *
 */

#include "matlab_engine_mathandlemex.hpp"
#include <stddef.h>
#include <listener_on_gui_thread.hpp>


matlab::engine::MatHandleMex::MatHandleMex()
	:
	m_pListener(NULL),
	m_fifoJobs(8)
{
}


matlab::engine::MatHandleMex::~MatHandleMex()
{
}


void matlab::engine::MatHandleMex::Start()
{
	m_pListener = CreateMListener();
}


void matlab::engine::MatHandleMex::Stop()
{
	if (m_pListener)
	{
		DestroyMListener(m_pListener);
		m_pListener = NULL;
	}
}


mxArray* matlab::engine::MatHandleMex::mexEvalStringWithTrap(const char* a_stringToEval)
{
	return ::mexEvalStringWithTrap(a_stringToEval);
}


mxArray* matlab::engine::MatHandleMex::mxCreateNumericMatrixC(size_t a_m, size_t a_n,
	mxClassID a_classid, mxComplexity a_flag)
{
	return mxCreateNumericMatrix(a_m, a_n, a_classid,a_flag);
}


void* matlab::engine::MatHandleMex::mxGetData(const mxArray *a_pa)
{
	return ::mxGetData(a_pa);
}


mxArray* matlab::engine::MatHandleMex::mexCallMATLABWithTrapC(
	int         a_nlhs,       /* number of expected outputs */
	mxArray     *a_plhs[],    /* pointer array to outputs */
	int         a_nrhs,       /* number of inputs */
	mxArray     *a_prhs[],    /* pointer array to inputs */
	const char  *a_fcn_name   /* name of function to execute */)
{
	return mexCallMATLABWithTrap(a_nlhs, a_plhs, a_nrhs, a_prhs, a_fcn_name);
}


mxArray* matlab::engine::MatHandleMex::mxGetCellC(const mxArray *a_pa, size_t a_i)
{
	return mxGetCell(a_pa, a_i);
}


void matlab::engine::MatHandleMex::mxDestroyArray(mxArray *a_pa)
{
	::mxDestroyArray(a_pa);
}


void matlab::engine::MatHandleMex::CallOnMatlabThread(void* a_owner, TypeClbK a_fpClb, void* a_arg)
{

	if (m_pListener) {
		SLsnCallbackItem newItem;
		newItem.arg = a_arg;
		newItem.clbk = a_fpClb;
		m_fifoJobs.AddElement(newItem);
		SynchronCallOnGUIthread(m_pListener, a_arg, &MatHandleMex::ListenerCallbackStatic);
	}
}


void* matlab::engine::MatHandleMex::ListenerCallbackStatic(void* a_arg)
{
	SLsnCallbackItem	clbkItem;
	MatHandleMex* pThis = (MatHandleMex*)a_arg;

	while (pThis->m_fifoJobs.Extract(&clbkItem))
	{
		(*clbkItem.clbk)(pThis, clbkItem.arg);
	}

	return NULL;
}
