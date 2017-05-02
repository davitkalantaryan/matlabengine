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
#include "matlab_engine_serializer_versioning.hpp"


//#pragma warning(disable : 4996)

matlab::engine::Serializer::Serializer(MatHandleBase* a_pMatHandle, 
	int32_ttt a_version, int32_ttt a_seriType)
	:
	common::Serializer(a_version),
	m_matHandle(a_pMatHandle)
{
	*m_pnVersion = a_version;
	*m_pnTypeOfSerialization = a_seriType;
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
	return *m_pnNumOfExpOutsOrError;
}


char* matlab::engine::Serializer::MatlabScriptNamePrivate3()const
{
	return (char*)(m_pWholeBuffer +COMMON_SERI_HEADER_LEN+(*m_pnArgumentByteStreamLength));
}

int	matlab::engine::Serializer::SendScriptNameAndArrays(
	versioning::FncPointers* a_fncs,
	common::SocketBase* a_pSocket,
	const char* a_scriptName,
	int32_ttt a_numOfOutsOrError,
	int32_ttt a_numOfArrays,
	const void* a_vpArrays[])
{
	versioning::TpResourse aResource;
	int32_ttt nStrlenPlus1;
	int32_ttt nByteStreamLen(0);
	int32_ttt nVersion = *m_pnVersion;
	int32_ttt nSeriType = *m_pnTypeOfSerialization;

	if (!a_scriptName) { a_scriptName = ""; }
	nStrlenPlus1 = 1 + (int32_ttt)strlen(a_scriptName);

	a_fncs->fpSeriToRs = versioning::FindSeriToRes(nVersion, nSeriType, a_fncs);
	if (!a_fncs->fpSeriToRs) { return -1; } // Function to handle not found

	nByteStreamLen = (*a_fncs->fpSeriToRs)(m_matHandle, &aResource, a_numOfArrays, a_vpArrays);
	if (nByteStreamLen<0) { return nByteStreamLen; }
	
	// if shoulb be added
	common::Serializer::Resize(nStrlenPlus1 + nByteStreamLen);

	*m_pnAllMinusHeaderLength = nStrlenPlus1 + nByteStreamLen;
	*m_pnNumOfExpOutsOrError = a_numOfOutsOrError;
	*m_pnArgumentByteStreamLength = nByteStreamLen;

	// Next call
	a_fncs->fpSeRsToBtStr = versioning::FindSeResToBtStream(nVersion, nSeriType, a_fncs);
	if (!a_fncs->fpSeRsToBtStr) { return -1; } // Function to handle not found
	(*a_fncs->fpSeRsToBtStr)(nByteStreamLen, m_pWholeBuffer+COMMON_SERI_HEADER_LEN,
		a_numOfArrays,a_vpArrays, aResource);
	memcpy(m_pWholeBuffer+COMMON_SERI_HEADER_LEN+nByteStreamLen, a_scriptName, nStrlenPlus1);

	nByteStreamLen=a_pSocket->Send(m_pWholeBuffer,(*m_pnAllMinusHeaderLength)+COMMON_SERI_HEADER_LEN);
	return nByteStreamLen;
}


int matlab::engine::Serializer::ReceiveHeaderScriptNameAndArrays2(
	versioning::FncPointers* a_fncs,
	common::SocketBase* a_socket, long a_timeoutMS,
	int32_ttt a_numOfArraysIn,void* a_vpArrays[], int32_ttt* a_pnNumOfArraysOut)
{
	int nReturn(ReceiveHeader(a_socket, a_timeoutMS));
	if (nReturn < 0) return nReturn;
	*a_pnNumOfArraysOut = ReceiveScriptNameAndArrays2(
		a_fncs,a_socket,a_timeoutMS, a_numOfArraysIn,a_vpArrays);
	return nReturn;
}


int	matlab::engine::Serializer::ReceiveHeader(common::SocketBase* a_pSocket,long a_timeoutMS)
{
	int nReceived = a_pSocket->Recv(m_pWholeBuffer, COMMON_SERI_HEADER_LEN, a_timeoutMS);

	if (nReceived != COMMON_SERI_HEADER_LEN) {
		if (nReceived>0) { nReceived = -1; }
		return nReceived;
	}
	if (*m_pnAllMinusHeaderLength<0) { return ACTION_TYPE::REMOTE_CALL; }
	if (common::Serializer::Resize(*m_pnAllMinusHeaderLength)) { return -2; }
	return ACTION_TYPE::REMOTE_CALL;
}

int32_ttt	matlab::engine::Serializer::ReceiveScriptNameAndArrays2(
	versioning::FncPointers* a_fncs,
	common::SocketBase* a_pSocket, long a_timeoutMS,
	int32_ttt a_nNumOfArraysIn,
	void* a_vpArrays[])
{
	versioning::TpResourse aResource;
	int32_ttt& nVersion = *m_pnVersion;
	int32_ttt& nSeriType = *m_pnTypeOfSerialization;
	int32_ttt nNumOfArgs(0);
	int nReceived = a_pSocket->Recv(
		m_pWholeBuffer+COMMON_SERI_HEADER_LEN,*m_pnAllMinusHeaderLength,a_timeoutMS);

	if (nReceived != (*m_pnAllMinusHeaderLength)) { 
		nReceived=nReceived<0?nReceived:-2;return nReceived;
	}

	// First call
	a_fncs->fpDeseriToRs = versioning::FindDeSeriToRes(nVersion, nSeriType,a_fncs);
	if (!a_fncs->fpDeseriToRs) { return -1; } // Function to handle not found
	nNumOfArgs = (*a_fncs->fpDeseriToRs)(
		m_matHandle,&aResource,*m_pnArgumentByteStreamLength,
		m_pWholeBuffer+COMMON_SERI_HEADER_LEN);
	if (nNumOfArgs < 0) { return nNumOfArgs; }

	// second call
	a_fncs->fpDeseRsToArgs = versioning::FindDeseRsToArgs(nVersion, nSeriType, a_fncs);
	if (!a_fncs->fpDeseRsToArgs) { return nNumOfArgs ? -1 : 0; }// Function to handle not found
	(*a_fncs->fpDeseRsToArgs)(a_nNumOfArraysIn, a_vpArrays, *m_pnArgumentByteStreamLength,
		m_pWholeBuffer + COMMON_SERI_HEADER_LEN, aResource);
	
	return nNumOfArgs;
}
