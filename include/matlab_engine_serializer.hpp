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

#include "common_serializer.hpp"
#include "common_socketbase.hpp"
#include "matlab_engine_mathandlebase.hpp"

// https://de.mathworks.com/help/matlab/apiref/mexcallmatlab.html
#ifndef MAXIMUM_NUMBER_OF_IN_AND_OUTS
#define MAXIMUM_NUMBER_OF_IN_AND_OUTS	50
#endif

namespace matlab{ namespace engine{

struct SERI_TYPE { enum { MAT_UNDOCU = 0, RAW1 }; };
struct ACTION_TYPE { enum { REMOTE_CALL = 0, NNN }; };

class Serializer : public common::Serializer
{
public:
	Serializer(MatHandleBase* matHandle);
	virtual ~Serializer();

	int	SendScriptNameAndArrays(
		common::SocketBase* socket,
		int32_ttt version,
		int32_ttt typeOfSeri,
		const char* scriptName,
		int32_ttt numOfOutsOrError,
		int32_ttt numOfArrays,
		const void* vpArrays[]);

	int	ReceiveScriptNameAndArrays(
		common::SocketBase* socket,
		int32_ttt numOfArrays,
		void* vpArrays[],
		int32_ttt* numOfArraysOut,
		long timeoutMS);

	const char*	MatlabScriptName3()const;

	int32_ttt NumOfExpOutsOrError()const;

private:
	char*		MatlabScriptNamePrivate3()const;

private:
	MatHandleBase*		m_matHandle;
};

}}

#endif // #ifndef __matlab_engine_serializer_hpp__
