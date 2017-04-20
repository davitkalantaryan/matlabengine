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
	m_nBufferMaxSize3(0),
	m_nReserved3(0)
{
	m_pWholeBuffer3 = (u_char_ttt*)malloc(MATLAB_HEADER_LENGTH);
	if (!m_pWholeBuffer3){throw "Low memory";}

	m_nBufferMaxSize3 = MATLAB_HEADER_LENGTH;

	m_pnVersion3 = ((int32_ttt*)((void*)(m_pWholeBuffer3 + OFFSET::VERSION)));
	m_pnTypeOfSerialization3 = ((int32_ttt*)((void*)(m_pWholeBuffer3+ OFFSET::TYPE)));
	m_pnAllMinusHeaderLength3 = ((int32_ttt*)((void*)(m_pWholeBuffer3 + OFFSET::NUM_AFTER_HEADER)));
	m_pnNumOfExpOutsOrError3 = ((int32_ttt*)((void*)(m_pWholeBuffer3 + OFFSET::NUM_OUTS)));
	m_pnMatlabByteStreamLength3 = ((int32_ttt*)((void*)(m_pWholeBuffer3 + OFFSET::BYTE_LENGTH)));
	m_pnReserved3 = ((int32_ttt*)((void*)(m_pWholeBuffer3 + OFFSET::RESERVED)));


	*m_pnVersion3 = SERIALIZER_VERSION;
	*m_pnTypeOfSerialization3 = TYPE::RAW1;
	*m_pnAllMinusHeaderLength3 = 0;
	*m_pnNumOfExpOutsOrError3 = 0;
	*m_pnMatlabByteStreamLength3 = 0;
	*m_pnReserved3 = 0;
	
}


matlab::engine::Serializer::~Serializer()
{
	free(m_pWholeBuffer3);
}


int matlab::engine::Serializer::Resize3(
	int32_ttt a_nNewTypeOfSeri,
	int32_ttt a_nNewOverAllMinusHeader,
	int32_ttt a_nNewByteStreamLength,
	int32_ttt a_nNumOfOutsOrErrCode)
{
	int nNewBufferSize = a_nNewOverAllMinusHeader + MATLAB_HEADER_LENGTH;
	if (nNewBufferSize > m_nBufferMaxSize3)
	{
		u_char_ttt* pTempBuffer = (u_char_ttt*)realloc(m_pWholeBuffer3, nNewBufferSize);
		if (!pTempBuffer)
		{
			// handle this case
			return -1;
		}
		m_pWholeBuffer3 = pTempBuffer;
		m_nBufferMaxSize3 = nNewBufferSize;
		
		m_pnVersion3 = ((int32_ttt*)((void*)(m_pWholeBuffer3 + OFFSET::VERSION)));
		m_pnTypeOfSerialization3 = ((int32_ttt*)((void*)(m_pWholeBuffer3 + OFFSET::TYPE)));
		m_pnAllMinusHeaderLength3 = ((int32_ttt*)((void*)(m_pWholeBuffer3 + OFFSET::NUM_AFTER_HEADER)));
		m_pnNumOfExpOutsOrError3 = ((int32_ttt*)((void*)(m_pWholeBuffer3 + OFFSET::NUM_OUTS)));
		m_pnMatlabByteStreamLength3 = ((int32_ttt*)((void*)(m_pWholeBuffer3 + OFFSET::BYTE_LENGTH)));
		m_pnReserved3 = ((int32_ttt*)((void*)(m_pWholeBuffer3 + OFFSET::RESERVED)));
	}

	*m_pnVersion3 = SERIALIZER_VERSION;
	*m_pnTypeOfSerialization3 = a_nNewTypeOfSeri;
	*m_pnAllMinusHeaderLength3 = a_nNewOverAllMinusHeader;
	*m_pnNumOfExpOutsOrError3 = a_nNumOfOutsOrErrCode;
	*m_pnMatlabByteStreamLength3 = a_nNewByteStreamLength;
	*m_pnReserved3 = 0;

	return 0;
}

u_char_ttt* matlab::engine::Serializer::GetBufferForReceive3()
{
	return m_pWholeBuffer3+ MATLAB_HEADER_LENGTH;
}


const u_char_ttt* matlab::engine::Serializer::GetBufferForSend3()const
{
	return m_pWholeBuffer3;
}


const char* matlab::engine::Serializer::MatlabScriptName3()const
{
	return (const char*)this->MatlabScriptNamePrivate3();
}


char* matlab::engine::Serializer::MatlabScriptNamePrivate3()const
{
	return (char*)(m_pWholeBuffer3 + MATLAB_HEADER_LENGTH + (*m_pnMatlabByteStreamLength3));
}


u_char_ttt* matlab::engine::Serializer::VaribleByteStream3()
{
	return m_pWholeBuffer3 + MATLAB_HEADER_LENGTH;
}


int32_ttt matlab::engine::Serializer::VaribleByteStreamLength3()const
{
	return (*m_pnMatlabByteStreamLength3);
}

int32_ttt matlab::engine::Serializer::SecondReceiveBufLength3()const
{
	return (*m_pnAllMinusHeaderLength3);
}


int32_ttt matlab::engine::Serializer::SeriType()const
{
	return *m_pnTypeOfSerialization3;
}

int32_ttt matlab::engine::Serializer::SendBufLength3()const
{
	return (*m_pnAllMinusHeaderLength3) + MATLAB_HEADER_LENGTH;
}

int32_ttt matlab::engine::Serializer::NumOfExpOutsOrError3()const
{
	return (*m_pnNumOfExpOutsOrError3);
}


int matlab::engine::Serializer::SetSendParams3(
	int32_ttt a_nNewTypeOfSeri,
	const char* a_scriptName,
	int32_ttt a_nInOrOutpLen, const void* a_inOrOut,
	int32_ttt a_nOutsOrErr)
{
	int32_ttt nStrLenPlus1 = (int32_ttt)strlen(a_scriptName) + 1;
	int32_ttt nOverAllMinusHeader = nStrLenPlus1 + a_nInOrOutpLen;

	int nReturn = this->Resize3(a_nNewTypeOfSeri,nOverAllMinusHeader, a_nInOrOutpLen, a_nOutsOrErr);

	if (nReturn) { return nReturn; }

	memcpy(this->MatlabScriptNamePrivate3(), a_scriptName, nStrLenPlus1);
	if (a_nInOrOutpLen && a_inOrOut) {
		memcpy(this->VaribleByteStream3(), a_inOrOut, a_nInOrOutpLen);
	}
	return 0;
}
