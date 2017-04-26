/*
 *	File: matlab_engine_mathandlebase.cpp
 *
 *	Created on: 12 Apr, 2017
 *	Author    : Davit Kalantaryan (Email: davit.kalantaryan@desy.de)
 *
 *
 */

#include "matlab_engine_mathandlebase.hpp"
#include <stddef.h>
#include <memory.h>

matlab::engine::MatHandleBase::MatHandleBase()
	:
	m_fifoJobs(8)
{
}

matlab::engine::MatHandleBase::~MatHandleBase()
{
}


int matlab::engine::MatHandleBase::newFrprint(int a_out, const char* a_fmt, ...)
{
	int nReturn;
	char vcBufferIn[4096];
	char vcBufferFin[4096];
	va_list argList;

	va_start(argList, a_fmt);
	nReturn=vsnprintf(vcBufferIn, 4095, a_fmt, argList);
	va_end(argList);

	a_out = a_out < 1 ? 1 : (a_out > 2 ? 2 : a_out);
	snprintf(vcBufferFin, 4095, "fprintf(%d,'%s');drawnow;", a_out, vcBufferIn);
	this->newEvalStringWithTrap(vcBufferFin);

	return nReturn;
}

void matlab::engine::MatHandleBase::PrintFromAnyThread(const char* a_cpcString)
{
	void* pString = (void*)a_cpcString;
	this->CallOnMatlabThreadC(this, &MatHandleBase::PrintOnMatlabThreadPrivate, pString);
}


void matlab::engine::MatHandleBase::PrintOnMatlabThreadPrivate(void* a_string)
{
	const char* cpcString = (const char*)a_string;
	this->newFrprint(STDPIPES::STDOUT, cpcString);
}


void matlab::engine::MatHandleBase::AddMatlabJob(void* a_owner, TypeClbK a_fpClb, void* a_arg)
{
	SLsnCallbackItem newItem;
	newItem.owner = a_owner;
	newItem.clbk = a_fpClb;
	newItem.arg = a_arg;
	m_fifoJobs.AddElement(newItem);
}


void matlab::engine::MatHandleBase::HandleAllJobs(void)
{
	SLsnCallbackItem	clbkItem;

	while (m_fifoJobs.Extract(&clbkItem))
	{
		(*clbkItem.clbk)(clbkItem.owner, clbkItem.arg);
	}

}


/*/////////////////////*/
#if 0
static void* ToByteStream(int32_ttt a_type, int a_nNumOfArgs, 
	const mxArray* a_vInputs[], int32_ttt* a_pnLength)
{
	for (int i(0); i < a_nNumOfArgs; ++i)
	{
		//
	}
}
#endif
