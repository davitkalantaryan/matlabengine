/*
 *	File: common_serializer.cpp
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

#include "common_serializer.hpp"
#include <malloc.h>
#include <string.h>

 //#pragma warning(disable : 4996)

common::Serializer::Serializer()
	:
	m_nBufferMaxSize3(0),
	m_nReserved3(0)
{
	m_pWholeBuffer3 = (u_char_ttt*)malloc(COMMON_SERI_HEADER_LEN);
	if (!m_pWholeBuffer3){throw "Low memory";}

	m_nBufferMaxSize3 = COMMON_SERI_HEADER_LEN;

	m_pnVersion3 = ((int32_ttt*)((void*)(m_pWholeBuffer3 + OFFSET::VERSION)));
	m_pnTypeOfSerialization3 = ((int32_ttt*)((void*)(m_pWholeBuffer3+ OFFSET::TYPE)));
	m_pnAllMinusHeaderLength3 = ((int32_ttt*)((void*)(m_pWholeBuffer3 + OFFSET::NUM_AFTER_HEADER)));
	m_pnNumOfExpOutsOrError3 = ((int32_ttt*)((void*)(m_pWholeBuffer3 + OFFSET::NUM_OUTS)));
	m_pnMatlabByteStreamLength3 = ((int32_ttt*)((void*)(m_pWholeBuffer3 + OFFSET::BYTE_LENGTH)));
	m_pnReserved3 = ((int32_ttt*)((void*)(m_pWholeBuffer3 + OFFSET::RESERVED)));


	*m_pnVersion3 = SERIALIZER_VERSION;
	*m_pnTypeOfSerialization3 = 0;
	*m_pnAllMinusHeaderLength3 = 0;
	*m_pnNumOfExpOutsOrError3 = 0;
	*m_pnMatlabByteStreamLength3 = 0;
	*m_pnReserved3 = 0;
	
}


common::Serializer::~Serializer()
{
	free(m_pWholeBuffer3);
}


int common::Serializer::Resize(int32_ttt a_nNewOverAllMinusHeader)
{
	int nNewBufferSize = a_nNewOverAllMinusHeader + COMMON_SERI_HEADER_LEN;
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

	return 0;
}


int32_ttt common::Serializer::SeriType()const
{
	return *m_pnTypeOfSerialization3;
}


int32_ttt common::Serializer::Version()const
{
	return *m_pnVersion3;
}


int32_ttt common::Serializer::OverAllLengthMinusHeader()const
{
	return *m_pnAllMinusHeaderLength3;
}
