/*****************************************************************************
 * File   : matlab_engine_serializer_version5_matundoc.hpp
 * created: 2017 Apr 29
 *****************************************************************************
 * Author:	D.Kalantaryan, Tel:+49(0)33762/77552 kalantar
 * Email :	davit.kalantaryan@desy.de
 * Mail  :	DESY, Platanenallee 6, 15738 Zeuthen
 *****************************************************************************
 * Description
 *   ...
 ****************************************************************************/

#include "matlab_engine_serializer_versioning.hpp"
#include "matlab_engine_mathandlebase.hpp"
#include <memory.h>

static int32_ttt SerializeToResourseCurrent(
	void* matEng,
	matlab::engine::versioning::TpResourse* pResourse,
	int32_ttt numOfArgs, const void* vpArgs[]);
static void SeResToBtStream(
	int32_ttt a_byteStrLen, u_char_ttt* a_byteStream,
	matlab::engine::versioning::TpResourse a_pResourse);
static int32_ttt DeSeriToResourseCurrent(
	void* a_matEng,
	matlab::engine::versioning::TpResourse* a_pResourse,
	int32_ttt a_byteStrLen, const u_char_ttt* a_byteStream);
static void DeseResToArgsCurrent(
	int32_ttt a_numOfArgs, void* a_vpArgs[],
	matlab::engine::versioning::TpResourse a_pResourse);

#if 1
static matlab::engine::versioning::Versionizer s_Versionizer(
	5, matlab::engine::versioning::SERI_TYPE::MAT_UNDOCU,
	SerializeToResourseCurrent, SeResToBtStream, 
	DeSeriToResourseCurrent, DeseResToArgsCurrent);
#endif

typedef const mxArray* TypeConstMxArray;
static int32_ttt SerializeToResourseCurrent(
	void* a_matEng,
	matlab::engine::versioning::TpResourse* a_pResourse,
	int32_ttt a_numOfArgs, const void* a_vpArgs[])
{
	void* pByteStream;
	matlab::engine::MatHandleBase* pMatHandle = (matlab::engine::MatHandleBase*)a_matEng;
	const TypeConstMxArray* vArrays = (const TypeConstMxArray*)a_vpArgs;
	mxArray* pcByteArray = NULL;
	mxArray* pCellArray = mxCreateCellMatrix(1, a_numOfArgs);
	int32_ttt nByteStreamLen(0);

	if (!pCellArray) {/*error handling?*/return -1; }
	for (int i(0); i < a_numOfArgs; ++i) {
		mxSetCell(pCellArray, i, mxDuplicateArray(vArrays[i]));
	}
	pMatHandle->newCallMATLABWithTrap(1, &pcByteArray, 1, &pCellArray, "getByteStreamFromArray");
	mxDestroyArray(pCellArray);
	if (!pcByteArray) { return 0; }
	nByteStreamLen = (int32_ttt)(mxGetNumberOfElements(pcByteArray)*mxGetElementSize(pcByteArray));
	pByteStream = mxGetData(pcByteArray);

	*a_pResourse = pcByteArray;
	return nByteStreamLen;
}


static void SeResToBtStream(
	int32_ttt a_byteStrLen, u_char_ttt* a_byteStream,
	matlab::engine::versioning::TpResourse a_pResourse)
{
	void* pByteStream =NULL;
	mxArray* pcByteArray = (mxArray*)a_pResourse;
	int32_ttt nByteStreamLen(0);

	if (!pcByteArray)return;
	pByteStream = mxGetData(pcByteArray);
	nByteStreamLen = (int32_ttt)(mxGetNumberOfElements(pcByteArray)*mxGetElementSize(pcByteArray));
	nByteStreamLen = nByteStreamLen > a_byteStrLen ? a_byteStrLen : nByteStreamLen;
	if (nByteStreamLen && pByteStream) { memcpy(a_byteStream,pByteStream, nByteStreamLen); }
}

static int32_ttt DeSeriToResourseCurrent(
	void* a_matEng,
	matlab::engine::versioning::TpResourse* a_pResourse,
	int32_ttt a_byteStrLen, const u_char_ttt* a_byteStream)
{
	void* pByteStreamFromRemoteInMatlab;
	matlab::engine::MatHandleBase* pMatHandle = (matlab::engine::MatHandleBase*)a_matEng;
	mxArray* pExceptionReturned;
	mxArray* pSerializedMatInput = NULL;
	mxArray* pCellArrayFromByteStream = NULL;
	int32_ttt nNumOfArray(0);

	if (a_byteStrLen <= 0) { return 0; }
	pSerializedMatInput = mxCreateNumericMatrix(1, a_byteStrLen, mxUINT8_CLASS, mxREAL);

	if (!pSerializedMatInput) {/*report*/return -1; }
	pByteStreamFromRemoteInMatlab = mxGetData(pSerializedMatInput);
	memcpy(pByteStreamFromRemoteInMatlab, a_byteStream, a_byteStrLen);

	pExceptionReturned = pMatHandle->newCallMATLABWithTrap(
		1, &pCellArrayFromByteStream, 1, &pSerializedMatInput, "getArrayFromByteStream");
	mxDestroyArray(pSerializedMatInput);
	if (pExceptionReturned) {
		pMatHandle->newPutVariable("base", "excpept", pExceptionReturned);
		return -2;
	}
	if (!pCellArrayFromByteStream) {/*report*/return -3; }

	nNumOfArray = (int)mxGetN(pCellArrayFromByteStream);
	*a_pResourse = pCellArrayFromByteStream;
	return nNumOfArray;
}


static void DeseResToArgsCurrent(
	int32_ttt a_numOfArgs, void* a_vpArgs[],
	matlab::engine::versioning::TpResourse a_pResourse)
{
	mxArray* pCellArrayFromByteStream = (mxArray*)a_pResourse;
	int32_ttt nNumOfArray(0);
	
	if (!pCellArrayFromByteStream) { return; }
	nNumOfArray = (int32_ttt)mxGetN(pCellArrayFromByteStream);
	nNumOfArray = nNumOfArray > a_numOfArgs ? a_numOfArgs : nNumOfArray;
	for (int i(0); i < nNumOfArray; ++i) {
		a_vpArgs[i] = mxDuplicateArray(mxGetCell(pCellArrayFromByteStream, i));
	}
	mxDestroyArray(pCellArrayFromByteStream);
}
