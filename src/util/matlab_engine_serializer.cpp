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
	m_pWholeBuffer((u_char_ttt*)malloc(8)),
	m_pnOverAllMinus8Byte((int32_ttt*)m_pWholeBuffer),
	m_pnNumOfExpOutsOrError((int32_ttt*)(m_pWholeBuffer+4))
{
	m_pWholeBuffer = (u_char_ttt*)malloc(8);
	if (!m_pWholeBuffer)
	{
		throw "Low memory";
	}
	m_nBufferMaxSize = 8;
}


matlab::engine::Serializer::~Serializer()
{
	free(m_pWholeBuffer);
}


int matlab::engine::Serializer::Resize(int32_ttt a_nNewOverAllMinus8Byte, int32_ttt a_nNumOfOutsOrErrCode)
{
	int nNewBufferSize = a_nNewOverAllMinus8Byte + 8;
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
		m_pnOverAllMinus8Byte = (int32_ttt*)m_pWholeBuffer;
		m_pnNumOfExpOutsOrError = (int32_ttt*)(m_pWholeBuffer + 4);
	}

	*m_pnOverAllMinus8Byte = a_nNewOverAllMinus8Byte;
	*m_pnNumOfExpOutsOrError = a_nNumOfOutsOrErrCode;
	return 0;
}

u_char_ttt* matlab::engine::Serializer::GetBufferForReceive()
{
	return m_pWholeBuffer+8;
}


u_char_ttt* matlab::engine::Serializer::GetOverAllBufferForSend()
{
	return m_pWholeBuffer;
}


const char* matlab::engine::Serializer::MatlabScriptName()const
{
	return (const char*)this->MatlabScriptNamePrivate();
}


char* matlab::engine::Serializer::MatlabScriptNamePrivate()const
{
	return (char*)(m_pWholeBuffer + 8);
}


u_char_ttt* matlab::engine::Serializer::InputsOrOutputs()
{
	int32_ttt nMatlabScrNameLen = (int32_ttt)strlen(MatlabScriptName());
	return m_pWholeBuffer + 9 + nMatlabScrNameLen;
}


int32_ttt matlab::engine::Serializer::InOutBytesNumber()const
{
	return (*m_pnOverAllMinus8Byte) - 1 - (int32_ttt)strlen(MatlabScriptName());
}

int32_ttt matlab::engine::Serializer::OverAllMinus8Byte()const
{
	return *m_pnOverAllMinus8Byte;
}

int32_ttt matlab::engine::Serializer::NumOfExpOutsOrError()const
{
	return *m_pnNumOfExpOutsOrError;
}


int matlab::engine::Serializer::SetSendParams(const char* a_scriptName,
	const u_char_ttt* a_pInOrOut, int a_nInOrOutpLen, int32_ttt a_nOutsOrErr)
{
	int32_ttt nStrLenPlus1 = (int32_ttt)strlen(a_scriptName) + 1;
	int32_ttt nOverAllMinus8 = nStrLenPlus1 + a_nInOrOutpLen;

	int nReturn = this->Resize(nOverAllMinus8, a_nOutsOrErr);

	if (nReturn) { return nReturn; }

	memcpy(this->MatlabScriptNamePrivate(), a_scriptName, nStrLenPlus1);
	if (a_pInOrOut) { memcpy(this->InputsOrOutputs(), a_pInOrOut, a_nInOrOutpLen); }
	return 0;
}
