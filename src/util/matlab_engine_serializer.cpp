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

 //#pragma warning(disable : 4996)

matlab::engine::Serializer::Serializer()
	:
	m_nBufferMaxSize(0),
	m_nReserved(0)
{
	m_pWholeBuffer = (u_char_ttt*)malloc(MATLAB_HEADER_LENGTH);
	if (!m_pWholeBuffer)
	{
		throw "Low memory";
	}
	m_nBufferMaxSize = MATLAB_HEADER_LENGTH;
	
	m_pnTypeOfSerialization = ((int32_ttt*)((void*)(m_pWholeBuffer+0)));
	m_pnOverAllMinusHeader2 = ((int32_ttt*)((void*)(m_pWholeBuffer + 4)));
	m_pnNumOfExpOutsOrError2 = ((int32_ttt*)((void*)(m_pWholeBuffer + 8)));
	m_pnMatlabByteStreamLength = ((int32_ttt*)((void*)(m_pWholeBuffer + 12)));

	*m_pnTypeOfSerialization = TYPES::MAT_UNDOCU;
	*m_pnOverAllMinusHeader2 = 0;
	*m_pnMatlabByteStreamLength = 0;
	*m_pnNumOfExpOutsOrError2 = 0;
	
}


matlab::engine::Serializer::~Serializer()
{
	free(m_pWholeBuffer);
}


int matlab::engine::Serializer::Resize2(
	int32_ttt a_nNewOverAllMinusHeader,
	int32_ttt a_nNewByteStreamLength,
	int32_ttt a_nNumOfOutsOrErrCode)
{
	int nNewBufferSize = a_nNewOverAllMinusHeader + MATLAB_HEADER_LENGTH;
	if (nNewBufferSize > m_nBufferMaxSize)
	{
		u_char_ttt* pTempBuffer = (u_char_ttt*)realloc(m_pWholeBuffer, nNewBufferSize);
		if (!pTempBuffer)
		{
			// handle this case
			return -1;
		}
		m_pWholeBuffer = pTempBuffer;
		m_nBufferMaxSize = nNewBufferSize;
		
		m_pnTypeOfSerialization = ((int32_ttt*)((void*)(m_pWholeBuffer + 0)));
		m_pnOverAllMinusHeader2 = ((int32_ttt*)((void*)(m_pWholeBuffer + 4)));
		m_pnNumOfExpOutsOrError2 = ((int32_ttt*)((void*)(m_pWholeBuffer + 8)));
		m_pnMatlabByteStreamLength = ((int32_ttt*)((void*)(m_pWholeBuffer + 12)));
	}

	//*m_pnTypeOfSerialization = TYPES::MAT_UNDOCU;
	*m_pnOverAllMinusHeader2 = a_nNewOverAllMinusHeader;
	*m_pnMatlabByteStreamLength = a_nNewByteStreamLength;
	*m_pnNumOfExpOutsOrError2 = a_nNumOfOutsOrErrCode;
	return 0;
}

u_char_ttt* matlab::engine::Serializer::GetBufferForReceive2()
{
	return m_pWholeBuffer+ MATLAB_HEADER_LENGTH;
}


const u_char_ttt* matlab::engine::Serializer::GetOverAllBufferForSend2()const
{
	return m_pWholeBuffer;
}


const char* matlab::engine::Serializer::MatlabScriptName2()const
{
	return (const char*)this->MatlabScriptNamePrivate2();
}


char* matlab::engine::Serializer::MatlabScriptNamePrivate2()const
{
	return (char*)(m_pWholeBuffer + MATLAB_HEADER_LENGTH + (*m_pnMatlabByteStreamLength));
}


u_char_ttt* matlab::engine::Serializer::CellByteStream()
{
	return m_pWholeBuffer + MATLAB_HEADER_LENGTH;
}


int32_ttt matlab::engine::Serializer::CellByteStreamLength()const
{
	return (*m_pnMatlabByteStreamLength);
}

int32_ttt matlab::engine::Serializer::OverAllMinusHeader()const
{
	return (*m_pnOverAllMinusHeader2);
}

int32_ttt matlab::engine::Serializer::NumOfExpOutsOrError2()const
{
	return (*m_pnNumOfExpOutsOrError2);
}


int matlab::engine::Serializer::SetSendParams2(const char* a_scriptName,
	const u_char_ttt* a_pInOrOut, int a_nInOrOutpLen, int32_ttt a_nOutsOrErr)
{
	int32_ttt nStrLenPlus1 = (int32_ttt)strlen(a_scriptName) + 1;
	int32_ttt nOverAllMinusHeader = nStrLenPlus1 + a_nInOrOutpLen;

	int nReturn = this->Resize2(nOverAllMinusHeader, a_nInOrOutpLen, a_nOutsOrErr);

	if (nReturn) { return nReturn; }

	memcpy(this->MatlabScriptNamePrivate2(), a_scriptName, nStrLenPlus1);
	if (a_nInOrOutpLen && a_pInOrOut) { 
		memcpy(this->CellByteStream(), a_pInOrOut, a_nInOrOutpLen);
	}
	return 0;
}
