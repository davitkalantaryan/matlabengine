/*
 *	File: matlab_engine_serializer.hpp
 *
 *	Created on: 12 Apr, 2017
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
#ifndef __matlab_engine_serializer_hpp__
#define __matlab_engine_serializer_hpp__

// https://de.mathworks.com/help/matlab/apiref/mexcallmatlab.html
#ifndef MAXIMUM_NUMBER_OF_IN_AND_OUTS
#define MAXIMUM_NUMBER_OF_IN_AND_OUTS	50
#endif

#define	SERIALIZER_VERSION		3
#define	MATLAB_HEADER_LENGTH	24

#include <stddef.h>

#ifndef u_char_ttt_defined
typedef unsigned char	u_char_ttt;
#define u_char_ttt_defined
#endif
#ifndef int32_ttt_defined
typedef int				int32_ttt;
#define int32_ttt_defined
#endif

namespace matlab{ namespace engine{

struct SDataHeader { int32_ttt version,seriType,allMinusHeader,numberOfOuts,byteStrLength,reserved; };
struct TYPE { enum SERI_TYPE{ MAT_UNDOCU = 0, RAW1 }; };

class Serializer
{
	struct OFFSET{enum{VERSION=0,TYPE=4,NUM_AFTER_HEADER=8,NUM_OUTS=12,BYTE_LENGTH=16,RESERVED=20};};
public:
	Serializer();
	~Serializer();

	const u_char_ttt*	GetBufferForSend3()const; // To send data
	u_char_ttt*	GetBufferForReceive3(); // To receive
	int			Resize3(
		int32_ttt newTypeOfSeri,
		int32_ttt newOverAllMinusHeader, 
		int32_ttt newByteStreamLength,
		int32_ttt numOfOutsOrErrCode);

	int			SetSendParams3(
		int32_ttt newTypeOfSeri,
		const char* scriptName, 
		int32_ttt inOrOutpLen, const void* inOrOut,
		int32_ttt nOutsOrErr);

	const char*	MatlabScriptName3()const;
	u_char_ttt*	VaribleByteStream3();
	int32_ttt	VaribleByteStreamLength3()const;
	int32_ttt	NumOfExpOutsOrError3()const;
	int32_ttt	SendBufLength3()const;
	int32_ttt	SecondReceiveBufLength3()const;
	int32_ttt	SeriType()const;

private:
	char*		MatlabScriptNamePrivate3()const;

private:
	int32_ttt	m_nBufferMaxSize3;
	int32_ttt	m_nReserved3;
	u_char_ttt*	m_pWholeBuffer3;

private:
	int32_ttt*	m_pnVersion3;
	int32_ttt*	m_pnTypeOfSerialization3;
	int32_ttt*	m_pnAllMinusHeaderLength3;
	int32_ttt*	m_pnNumOfExpOutsOrError3;
	int32_ttt*	m_pnMatlabByteStreamLength3;
	int32_ttt*	m_pnReserved3;
};

}}

#endif // #ifndef __matlab_engine_serializer_hpp__
