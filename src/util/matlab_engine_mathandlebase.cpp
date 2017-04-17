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
	snprintf(vcBufferFin, 4095, "fprintf(%d,'%s');", a_out, vcBufferIn);
	this->newEvalStringWithTrap(vcBufferFin);

	return nReturn;
}


mxArray* matlab::engine::MatHandleBase::MatlabArrayToMatlabByteStream(int a_nNumOfArgs, mxArray*a_Inputs[])
{
	mxArray* pcByteArray = NULL;
	mxArray* pCellArray = mxCreateCellMatrix(1, a_nNumOfArgs);
	mxArray** ppInputs = const_cast<mxArray**>(a_Inputs);

	if (!pCellArray)
	{
		// Make some report
		return NULL;
	}

	for (int i(0); i < a_nNumOfArgs; ++i)
	{
		mxSetCell(pCellArray, i, ppInputs[i]);
	}

	this->newCallMATLABWithTrap(1, &pcByteArray, 1, &pCellArray, "getByteStreamFromArray");
	mxDestroyArray(pCellArray);

	return pcByteArray;
}


int matlab::engine::MatHandleBase::HandleIncomData(mxArray** a_pInc, int a_nMxArraySize, const void* a_data,int a_nBytesNumber)
{
	int nReturn(-1);
	void* pSerializedData;
	mxArray* pExceptionReturned;
	mxArray* pCellArrayFromByteStream;
	mxArray* pSerializedInputCell = mxCreateNumericMatrix(1, a_nBytesNumber, mxUINT8_CLASS, mxREAL);
	
	if (!pSerializedInputCell) {/*report*/goto returnPoint; }
	pSerializedData = mxGetData(pSerializedInputCell);
	memcpy(pSerializedData, a_data, a_nBytesNumber);

	pExceptionReturned = this->newCallMATLABWithTrap(1, &pCellArrayFromByteStream, 1,
		&pSerializedInputCell, "getArrayFromByteStream");
	mxDestroyArray(pSerializedInputCell);
	if (pExceptionReturned) { pCellArrayFromByteStream = pExceptionReturned;goto returnPoint; }
	if (!pCellArrayFromByteStream) {/*report*/goto returnPoint; }

	nReturn = (int)mxGetN(pCellArrayFromByteStream);
	nReturn = nReturn > a_nMxArraySize ? a_nMxArraySize : nReturn;

	for (int i(0); i < nReturn; ++i)
	{
		a_pInc[i] = mxGetCell(pCellArrayFromByteStream, i);
	}

returnPoint:
	return nReturn;
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


namespace matlab{ namespace engine{

typedef void* type_void_ptr;

void* GetFuncPointer(int a_fa,...)
{
	type_void_ptr pRet;
	va_list argList;

	va_start(argList, a_fa);
	pRet = va_arg(argList, type_void_ptr);
	va_end(argList);
	return pRet;
}

}}
