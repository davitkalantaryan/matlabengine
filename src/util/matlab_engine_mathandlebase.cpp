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
{
}

matlab::engine::MatHandleBase::~MatHandleBase()
{
}


int matlab::engine::MatHandleBase::Exist(const char* a_name, const char* a_type)
{
	double* plfReturn;
	mxArray* vInputs[2];
	mxArray* aOutput=NULL;
	int nReturn,nNumOfInputs(1);

	vInputs[0] = mxCreateString(a_name);
	if (a_type) {
		vInputs[1] = mxCreateString(a_type);
		nNumOfInputs = 2;
	}
	this->newCallMATLABWithTrap(1, &aOutput, nNumOfInputs, vInputs, "exist");
	plfReturn = mxGetPr(aOutput);
	nReturn = (int)((*plfReturn)+0.5);
	mxDestroyArray(aOutput);
	if (a_type) { mxDestroyArray(vInputs[1]); }
	mxDestroyArray(vInputs[0]);
	return nReturn;
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
	this->AsyncCallOnMatlabThreadC(this, &MatHandleBase::PrintOnMatlabThreadPrivate, pString);
}


void matlab::engine::MatHandleBase::PrintOnMatlabThreadPrivate(void* a_string)
{
	const char* cpcString = (const char*)a_string;
	this->newFrprint(STDPIPES::STDOUT, cpcString);
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
