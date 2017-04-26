/*
 *	File: matlab_engine_serializer.cpp
 *
 *	Created on: 12 Apr, 2017
 *	Author    : Davit Kalantaryan (Email: davit.kalantaryan@desy.de)
 *
 *  Input
 *		1) 4      B [ 0-3           ]   Number of Bytes after 2 integers (OverAll-8)
 *		2) 4      B [ 4-7           ]   Number of expected outputs
 *		3) (X-8+1)B [ 8-(X)         ]   Matlab Script name (0 termination)
 *		4) (Y)    B [ (X+1)-(X+Y-1) ]   Serialized inputs
 *
 *  Output
 *		1) 4      B [ 0-3           ]   Overall number of Bytes
 *		2) 4      B [ 4-7           ]   Error code (0->NoError, (!=0)->Error)
 *		3) (X-8+1)B [ 8-(X)         ]   Output (Error) string (NULL terminated)
 *		4) (Y)    B [ (X+1)-(X+Y-1) ]   Serialized outputz
 *
 *
 */

#include "matlab_engine_serializer.hpp"
#include <malloc.h>
#include <string.h>
#include <matrix.h>

typedef const mxArray* TypeConstMxArray;

 //#pragma warning(disable : 4996)

matlab::engine::Serializer::Serializer(MatHandleBase* a_pMatHandle)
	:
	m_matHandle(a_pMatHandle)
{	
}

matlab::engine::Serializer::~Serializer()
{
}

const char* matlab::engine::Serializer::MatlabScriptName3()const
{
	return (const char*)this->MatlabScriptNamePrivate3();
}


int32_ttt matlab::engine::Serializer::NumOfExpOutsOrError()const
{
	return *m_pnNumOfExpOutsOrError3;
}


char* matlab::engine::Serializer::MatlabScriptNamePrivate3()const
{
	return (char*)(m_pWholeBuffer3 +COMMON_SERI_HEADER_LEN+(*m_pnMatlabByteStreamLength3));
}

int	matlab::engine::Serializer::SendScriptNameAndArrays(
	common::SocketBase* a_pSocket,
	int32_ttt a_nVersion,
	int32_ttt a_nTypeOfSeri,
	const char* a_scriptName,
	int32_ttt a_numOfOutsOrError,
	int32_ttt a_numOfArrays,
	const void* a_vpArrays[])
{
	mxArray* pcByteArray = NULL;
	void* pByteStream(NULL);
	int nStrlenPlus1;
	int nByteStreamLen(0);

	if (!a_scriptName) { a_scriptName = ""; }
	nStrlenPlus1 = 1 + (int)strlen(a_scriptName);

	switch (a_nTypeOfSeri)
	{
	case SERI_TYPE::MAT_UNDOCU:
	{
		const TypeConstMxArray* vArrays = (const TypeConstMxArray*)a_vpArrays;
		mxArray* pCellArray = mxCreateCellMatrix(1,a_numOfArrays);

		if (!pCellArray) {/*error handling?*/return 0; }
		for (int i(0); i < a_numOfArrays; ++i){
			mxSetCell(pCellArray, i, mxDuplicateArray(vArrays[i]));
		}
		m_matHandle->newCallMATLABWithTrap(1,&pcByteArray,1,&pCellArray,"getByteStreamFromArray");
		mxDestroyArray(pCellArray);
		if (!pcByteArray) { return 0; }
		nByteStreamLen = (int32_ttt)(mxGetNumberOfElements(pcByteArray)*mxGetElementSize(pcByteArray));
		pByteStream = mxGetData(pcByteArray);
	}
	break; // case TYPE::MAT_UNDOCU:

	case SERI_TYPE::RAW1:
	{
	}
	break; // case TYPE::RAW1:

	default:
		break;
	} // switch (a_type)

	common::Serializer::Resize(nStrlenPlus1 + nByteStreamLen);

	*m_pnVersion3 = SERIALIZER_VERSION;
	*m_pnTypeOfSerialization3 = a_nTypeOfSeri;
	*m_pnAllMinusHeaderLength3 = nStrlenPlus1 + nByteStreamLen;
	*m_pnNumOfExpOutsOrError3 = a_numOfOutsOrError;
	*m_pnMatlabByteStreamLength3 = nByteStreamLen;
	*m_pnReserved3 = 0;

	if(nByteStreamLen && pByteStream){
		memcpy(m_pWholeBuffer3+COMMON_SERI_HEADER_LEN,pByteStream,nByteStreamLen);
	}
	memcpy(m_pWholeBuffer3+COMMON_SERI_HEADER_LEN+nByteStreamLen,a_scriptName,nStrlenPlus1);

	nByteStreamLen=a_pSocket->Send(
		m_pWholeBuffer3, (*m_pnAllMinusHeaderLength3) + COMMON_SERI_HEADER_LEN);

	// Cleanup
	switch (a_nTypeOfSeri)
	{
	case SERI_TYPE::MAT_UNDOCU:
	{
		mxDestroyArray(pcByteArray);
	}
	break;
	default: break;
	}

	return nByteStreamLen;
}


int	matlab::engine::Serializer::ReceiveScriptNameAndArrays(
	common::SocketBase* a_pSocket,
	int32_ttt a_nNumOfArraysIn,
	void* a_vpArrays[],
	int32_ttt* a_pnNumOfArraysOut,
	long a_timeoutMS)
{
	mxArray* pCellArrayFromByteStream=NULL;
	int32_ttt& nNumOfArray = *a_pnNumOfArraysOut;
	int nReceived= a_pSocket->Recv(m_pWholeBuffer3,COMMON_SERI_HEADER_LEN,a_timeoutMS);

	nNumOfArray = 0;
	if (nReceived!=COMMON_SERI_HEADER_LEN){
		if(nReceived>0){nReceived=-1;}
		return nReceived;
	}
	if(*m_pnAllMinusHeaderLength3<0){return ACTION_TYPE::REMOTE_CALL;}
	common::Serializer::Resize(*m_pnAllMinusHeaderLength3);
	nReceived= a_pSocket->Recv(
		m_pWholeBuffer3+COMMON_SERI_HEADER_LEN,*m_pnAllMinusHeaderLength3,a_timeoutMS);
	if (nReceived != (*m_pnAllMinusHeaderLength3)) { return -2; }

	switch (*m_pnTypeOfSerialization3)
	{
	case SERI_TYPE::MAT_UNDOCU:
	{
		void* pSerializedData;
		mxArray** vOutputs = (mxArray**)a_vpArrays;
		mxArray* pExceptionReturned;
		mxArray* pSerializedInputCell = mxCreateNumericMatrix(
			1,*m_pnMatlabByteStreamLength3,mxUINT8_CLASS,mxREAL);

		if (!pSerializedInputCell) {/*report*/return -1; }
		pSerializedData = mxGetData(pSerializedInputCell);
		memcpy(pSerializedData,m_pWholeBuffer3+COMMON_SERI_HEADER_LEN,*m_pnMatlabByteStreamLength3);

		pExceptionReturned = m_matHandle->newCallMATLABWithTrap(
			1, &pCellArrayFromByteStream, 1,&pSerializedInputCell, "getArrayFromByteStream");
		mxDestroyArray(pSerializedInputCell);
		if (pExceptionReturned) { 
			m_matHandle->newPutVariable("base", "excpept", pExceptionReturned);
			return -2;
		}
		if (!pCellArrayFromByteStream) {/*report*/return -3; }

		nNumOfArray = (int)mxGetN(pCellArrayFromByteStream);
		nNumOfArray = nNumOfArray > a_nNumOfArraysIn ? a_nNumOfArraysIn : nNumOfArray;

		for (int i(0); i < nNumOfArray; ++i){
			vOutputs[i] = mxDuplicateArray(mxGetCell(pCellArrayFromByteStream,i));
		}
	}
	break; // case TYPE::MAT_UNDOCU:
	default:
		break;
	} // switch (a_type)


	  // Cleanup
	switch (*m_pnTypeOfSerialization3)
	{
	case SERI_TYPE::MAT_UNDOCU:
	{
		if(pCellArrayFromByteStream){mxDestroyArray(pCellArrayFromByteStream);}
	}
	break;
	default: break;
	}

	return ACTION_TYPE::REMOTE_CALL;
}
