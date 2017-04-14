/*
 *	File: matlab_bytestream_routines.cpp
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

#include <process.h>
#include <stdio.h>
#include "matlab_bytestream_routines.h"

#pragma warning(disable : 4996)

#ifdef __cplusplus
extern "C"
{
#endif

mxArray* MatlabArrayToMatlabByteStream(int a_nNumOfArgs, mxArray*a_Inputs[])
{
	mxArray* pcByteArray = NULL;
	mxArray* pCellArray = mxCreateCellMatrix(1, a_nNumOfArgs);
	mxArray** ppInputs = const_cast<mxArray**>(a_Inputs);

	if (!pCellArray)
	{
		// Make some report
		return NULL;
	}

	for (int i(0); i < a_nNumOfArgs; ++i)
	{
		mxSetCell(pCellArray, i, ppInputs[i]);
	}

	mexCallMATLAB(1, &pcByteArray, 1, &pCellArray, "getByteStreamFromArray");
	mxDestroyArray(pCellArray);

	return pcByteArray;
}


int32_ttt GetByteStreamLen(mxArray* a_matlabByteStream)
{
	if (a_matlabByteStream)
	{
		size_t unM, unN, unSingle;
		unM = mxGetM(a_matlabByteStream);
		unN = mxGetN(a_matlabByteStream);
		unSingle = mxGetElementSize(a_matlabByteStream);
		//if (s_nDebug) { int nClsID = mxGetClassID(pcByteArray); mexPrintf("clsId=%d,m=%d,n=%d,chunk_size=%d\n", nClsID, (int)unM, (int)unN, (int)unSingle); }
		return (int32_ttt)(unM*unN*unSingle);
	}
	else
	{
		mexPrintf("mexCallMATLAB\n");
		mexErrMsgTxt("Unable to serialize the input!\n");
		return 0;
	}

	return 0;
}


#ifdef __cplusplus
}
#endif
