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


void* matlab::engine::MatHandleBase::MatlabArrayToByteStream1(
	int32_ttt a_type, int a_nNumOfArgs, mxArray** a_vInputs, int32_ttt* a_pnLength)
{
	*a_pnLength = 0;

	switch (a_type)
	{
	case TYPE::MAT_UNDOCU:
	{
		mxArray* pcByteArray = NULL;
		mxArray* pCellArray = mxCreateCellMatrix(1, a_nNumOfArgs);

		if (!pCellArray) {/*error handling?*/return NULL; }

		for (int i(0); i < a_nNumOfArgs; ++i)
		{
			mxSetCell(pCellArray, i, a_vInputs[i]);
		}

		this->newCallMATLABWithTrap(1, &pcByteArray, 1, &pCellArray, "getByteStreamFromArray");
		mxDestroyArray(pCellArray);

		if (!pcByteArray) { return NULL; }

		*a_pnLength = (int32_ttt)(mxGetNumberOfElements(pcByteArray)*mxGetElementSize(pcByteArray));
		return mxGetData(pcByteArray);
	}
	break; // case TYPE::MAT_UNDOCU:
	
	case TYPE::RAW1:
	{
	}
	break; // case TYPE::RAW1:
	
	default:
		return NULL;
	} // switch (a_type)

	return NULL;
}

void* matlab::engine::MatHandleBase::MatlabArrayToByteStream2(
	int32_ttt a_type, int a_nNumOfArgs, const mxArray* a_vInputs[], int32_ttt* a_pnLength)
{
	mxArray** ppInputs = const_cast<mxArray**>(a_vInputs);
	return MatlabArrayToByteStream1(a_type,a_nNumOfArgs, ppInputs,a_pnLength);
}


int  matlab::engine::MatHandleBase::ByteStreamToMatlabArray(
	int32_ttt a_type, 
	int a_nMaxOutSize, mxArray** a_outputs,
	int a_nByteStreamLen, const void* a_byteStream)
{
	int nReturn(-1);

	switch (a_type)
	{
	case TYPE::MAT_UNDOCU:
	{
		void* pSerializedData;
		mxArray* pExceptionReturned;
		mxArray* pCellArrayFromByteStream;
		mxArray* pSerializedInputCell = mxCreateNumericMatrix(1, a_nByteStreamLen,mxUINT8_CLASS,mxREAL);

		if (!pSerializedInputCell) {/*report*/return -1; }
		pSerializedData = mxGetData(pSerializedInputCell);
		memcpy(pSerializedData, a_byteStream, a_nByteStreamLen);

		pExceptionReturned = this->newCallMATLABWithTrap(1, &pCellArrayFromByteStream, 1,
			&pSerializedInputCell, "getArrayFromByteStream");
		mxDestroyArray(pSerializedInputCell);
		if (pExceptionReturned) { pCellArrayFromByteStream = pExceptionReturned; return -2; }
		if (!pCellArrayFromByteStream) {/*report*/return -3; }

		nReturn = (int)mxGetN(pCellArrayFromByteStream);
		nReturn = nReturn > a_nMaxOutSize ? a_nMaxOutSize : nReturn;

		for (int i(0); i < nReturn; ++i)
		{
			a_outputs[i] = mxDuplicateArray( mxGetCell(pCellArrayFromByteStream, i));
		}

		return nReturn;
	}
	break; // case TYPE::MAT_UNDOCU:
	default:
		break;
	} // switch (a_type)

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
