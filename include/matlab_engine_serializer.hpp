/*
 *	File: matlab_engine_serializer.hpp
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
#ifndef __matlab_engine_serializer_hpp__
#define __matlab_engine_serializer_hpp__

// https://de.mathworks.com/help/matlab/apiref/mexcallmatlab.html
#ifndef MAXIMUM_NUMBER_OF_IN_AND_OUTS
#define MAXIMUM_NUMBER_OF_IN_AND_OUTS	50
#endif

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

struct SDataHeader { int32_ttt overallMin8, numberOfOutsOrError; };

class Serializer
{
public:
	Serializer();
	~Serializer();

	u_char_ttt*	GetOverAllBufferForSend(); // To send data
	u_char_ttt*	GetBufferForReceive(); // To receive
	int			Resize(int32_ttt newOverAllMinus8Byte, int32_ttt numOfOutsOrErrCode);

	int			SetSendParams(const char* scriptName, const u_char_ttt* inOrOut, int inOrOutpLen, int32_ttt nOutsOrErr);

	const char*	MatlabScriptName()const;
	u_char_ttt*	InputsOrOutputs();
	int32_ttt	InOutBytesNumber()const;
	int32_ttt	NumOfExpOutsOrError()const;
	int32_ttt	OverAllMinus8Byte()const;

private:
	char*		MatlabScriptNamePrivate()const;

private:
	int32_ttt	m_nBufferMaxSize;
	u_char_ttt*	m_pWholeBuffer;

private:
	int32_ttt*	m_pnOverAllMinus8Byte;
	int32_ttt*	m_pnNumOfExpOutsOrError;
};

}}

#endif // #ifndef __matlab_engine_serializer_hpp__
