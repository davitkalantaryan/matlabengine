/*
 *	File: common_serializer.hpp
 *
 *	Created on: 26 Apr, 2017
 *	Author    : Davit Kalantaryan (Email: davit.kalantaryan@desy.de)
 *
 *  Input
 *		1) 4      B [ 0  -3         ]   Version
 *		2) 4      B [ 4  -7         ]   Type of serialization
 *		3) 4      B [ 8  -11        ]   Number of Bytes after header (OverAll-header)
 *		4) 4	  B [ 12 -15		]	Number of expected outputs
 *		5) 4	  B [ 16 -19		]	Number of Bytes in the MATLAB Byte stream
 *		6) 4	  B [ 20 -23		]	Reserved
 *		7) (X-8+1)B [ 8-(X)         ]   Serialized inputs
 *		8) (Y)    B [ (X+1)-(X+Y-1) ]   Matlab Script name (0 termination)
 *
 *  Output
 *		1) 4      B [ 0  -3         ]   Version
 *		2) 4      B [ 4  -7         ]   Type of serialization
 *		3) 4      B [ 8  -11        ]   Number of Bytes after header (OverAll-header)
 *		4) 4	  B [ 11 -15		]	Error code (0->NoError, (!=0)->Error)
 *		5) 4	  B [ 16 -19		]	Number of Bytes in the MATLAB Byte stream
 *		6) 4	  B [ 20 -23		]	Reserved
 *		7) (X-8+1)B [ 8-(X)         ]   Serialized outputs
 *		8) (Y)    B [ (X+1)-(X+Y-1) ]   Error string if an error
 *
 *
 */
#ifndef __common_serializer_hpp__
#define __common_serializer_hpp__

#define	COMMON_SERI_HEADER_LEN			64

#include <stddef.h>

#ifndef u_char_ttt_defined
typedef unsigned char	u_char_ttt;
#define u_char_ttt_defined
#endif
#ifndef int32_ttt_defined
typedef int				int32_ttt;
#define int32_ttt_defined
#endif

namespace common{

class Serializer
{
protected:
	struct OFFSET{enum{VERSION=0,TYPE=4,NUM_AFTER_HEADER=8,NUM_OUTS=12,BYTE_LENGTH=16,RESERVED=20};};
public:
	Serializer(int32_ttt a_version);
	virtual ~Serializer();

	int32_ttt OverAllLengthMinusHeader()const;
	int32_ttt SeriType()const;
	int32_ttt Version()const;

protected:
	int			Resize(int32_ttt newOverAllMinusHeader);

protected:
	int32_ttt	m_nBufferMaxSize;
	int32_ttt	m_nReserved;
	u_char_ttt*	m_pWholeBuffer;

protected:
	int32_ttt*	m_pnVersion;
	int32_ttt*	m_pnTypeOfSerialization;
	int32_ttt*	m_pnAllMinusHeaderLength;
	int32_ttt*	m_pnNumOfExpOutsOrError;
	int32_ttt*	m_pnArgumentByteStreamLength;
};

}

#endif // #ifndef __common_serializer_hpp__
