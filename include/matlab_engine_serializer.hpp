/*
 *	File: matlab_engine_serializer.hpp
 *
 *	Created on: 12 Apr, 2017
 *	Author    : Davit Kalantaryan (Email: davit.kalantaryan@desy.de)
 *
 *  Input
 *		1) 4      B [ 0 -3          ]   Type of serialization
 *		2) 4      B [ 4 -7          ]   Number of Bytes after header (OverAll-header)
 *		3) 4	  B [ 8 -11			]	Number of expected outputs
 *		4) 4	  B [ 12-15			]	Number of Bytes in the MATLAB Byte stream
 *		5) (X-8+1)B [ 8-(X)         ]   Serialized inputs
 *		6) (Y)    B [ (X+1)-(X+Y-1) ]   Matlab Script name (0 termination)
 *
 *  Output
 *		1) 4      B [ 0 -3          ]   Type of serialization
 *		2) 4      B [ 4 -7          ]   Number of Bytes after header (OverAll-header)
 *		3) 4	  B [ 8 -11			]	Error code (0->NoError, (!=0)->Error)
 *		4) 4	  B [ 12-15			]	Number of Bytes in the MATLAB Byte stream
 *		5) (X-8+1)B [ 8-(X)         ]   Serialized outputs
 *		6) (Y)    B [ (X+1)-(X+Y-1) ]   Error string if an error
 *
 *
 */
#ifndef __matlab_engine_serializer_hpp__
#define __matlab_engine_serializer_hpp__

// https://de.mathworks.com/help/matlab/apiref/mexcallmatlab.html
#ifndef MAXIMUM_NUMBER_OF_IN_AND_OUTS
#define MAXIMUM_NUMBER_OF_IN_AND_OUTS	50
#endif

#define	MATLAB_HEADER_LENGTH	16

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

struct SDataHeader { int32_ttt seriType,overallMinHeader, numberOfOutsOrError,byteStrLength; };

class Serializer
{
public:
	struct TYPES { enum { MAT_UNDOCU = 0 }; };
	Serializer();
	~Serializer();

	const u_char_ttt*	GetOverAllBufferForSend2()const; // To send data
	u_char_ttt*	GetBufferForReceive2(); // To receive
	int			Resize2(
		int32_ttt newOverAllMinusHeader, 
		int32_ttt newByteStreamLength,
		int32_ttt numOfOutsOrErrCode);

	int			SetSendParams2(
		const char* scriptName, const u_char_ttt* inOrOut, 
		int inOrOutpLen, int32_ttt nOutsOrErr);

	const char*	MatlabScriptName2()const;
	u_char_ttt*	CellByteStream();
	int32_ttt	CellByteStreamLength()const;
	int32_ttt	NumOfExpOutsOrError2()const;
	int32_ttt	OverAllMinusHeader()const;

private:
	char*		MatlabScriptNamePrivate2()const;

private:
	int32_ttt	m_nBufferMaxSize;
	int32_ttt	m_nReserved;
	u_char_ttt*	m_pWholeBuffer;

private:
	int32_ttt*	m_pnTypeOfSerialization;
	int32_ttt*	m_pnOverAllMinusHeader2;
	int32_ttt*	m_pnNumOfExpOutsOrError2;
	int32_ttt*	m_pnMatlabByteStreamLength;
};

}}

#endif // #ifndef __matlab_engine_serializer_hpp__
