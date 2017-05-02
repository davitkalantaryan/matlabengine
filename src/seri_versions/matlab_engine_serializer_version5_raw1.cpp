/*****************************************************************************
 * File   : matlab_engine_serializer_version5_raw1_clnt.cpp
 * created: 2017 May 02
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
#include <mat_matrix.h>

static int32_ttt SerializeToResourseCurrent(
	void* matEng,
	matlab::engine::versioning::TpResourse* pResourse,
	int32_ttt numOfArgs, const void* vpArgs[]);
static void SeResToBtStream(
	int32_ttt a_byteStrLen, u_char_ttt* a_byteStream,
	int32_ttt numOfArgs, const void* vpArgs[],
	matlab::engine::versioning::TpResourse a_pResourse);
static int32_ttt DeSeriToResourseCurrent(
	void* a_matEng,
	matlab::engine::versioning::TpResourse* a_pResourse,
	int32_ttt a_byteStrLen, const u_char_ttt* a_byteStream);
static void DeseResToArgsCurrent(
	int32_ttt a_numOfArgs, void* a_vpArgs[],
	int32_ttt a_byteStrLen, const u_char_ttt* a_byteStream,
	matlab::engine::versioning::TpResourse a_pResourse);

static matlab::engine::versioning::Versionizer s_Versionizer(
	5, matlab::engine::versioning::SERI_TYPE::RAW1,
	SerializeToResourseCurrent, SeResToBtStream,
	DeSeriToResourseCurrent, DeseResToArgsCurrent);

typedef const nwArray* TypeConstMxArray;

static int32_ttt SerializeToResourseCurrent(
	void* /*a_matEng*/,
	matlab::engine::versioning::TpResourse* /*a_pResourse*/,
	int32_ttt a_numOfArgs, const void* a_vpArgs[])
{
	const TypeConstMxArray* vArrays = (const TypeConstMxArray*)a_vpArgs;
	int32_ttt nSingleArgLen(0);
	int32_ttt nSingleArgLenCorrected(0);
	int32_ttt nByteStreamLen(0);
	int32_ttt nM, nN, nElementSize;

	nByteStreamLen += 8; // For number of args

	for (int i(0); i < a_numOfArgs; ++i) {
		//nSingleArgLen = vArrays[i] ? (mxGetElementSize(vArrays[i])*mxGetNumberOfElements(vArrays[i])) : 0;
		if (!vArrays[i]) { continue; }
		nM = (int32_ttt)nwGetM(vArrays[i]); nN = (int32_ttt)nwGetN(vArrays[i]);
		nElementSize = (int32_ttt)nwGetElementSize(vArrays[i]);
		nSingleArgLen = nM*nN*nElementSize;
		nSingleArgLenCorrected = ((nSingleArgLen/8)+(nSingleArgLen%8?1:0)) * 8;
		nByteStreamLen += 16; // 4-nM, 4-nN, 4-nElSize, 4-forPadding
		nByteStreamLen += nSingleArgLenCorrected;
	}
	return nByteStreamLen;
}


static void SeResToBtStream(
	int32_ttt a_byteStrLen, u_char_ttt* a_byteStream,
	int32_ttt a_numOfArgs, const void* a_vpArgs[],
	matlab::engine::versioning::TpResourse /*a_pResourse*/)
{
	u_char_ttt* pBuff = a_byteStream;
	void* pDataForArg;
	const TypeConstMxArray* vArrays = (const TypeConstMxArray*)a_vpArgs;
	int32_ttt nM, nN, nElementSize, nClassID;
	int32_ttt nSingleArgLen(0);
	int32_ttt nSingleArgLenCorrected(0);
	int32_ttt nOffset(0);

	//*((int64_ttt*)(pBuff + nOffset)) = a_numOfArgs;
	nOffset += 8;

	for (int i(0); i < a_numOfArgs; ++i) {
		//nSingleArgLen = vArrays[i] ? (mxGetElementSize(vArrays[i])*mxGetNumberOfElements(vArrays[i])) : 0;
		if (!vArrays[i]) { continue; }
		nM = (int32_ttt)nwGetM(vArrays[i]); nN = (int32_ttt)nwGetN(vArrays[i]);
		nElementSize = (int32_ttt)nwGetElementSize(vArrays[i]);
		nSingleArgLen = nM*nN*nElementSize;
		nClassID = (int32_ttt)nwGetClassID(vArrays[i]);
		nSingleArgLenCorrected = ((nSingleArgLen/8)+(nSingleArgLen%8?1:0)) * 8;
		if ((nOffset+nSingleArgLenCorrected+16) > a_byteStrLen) { a_numOfArgs=i;break; }

		*((int32_ttt*)(pBuff + nOffset)) = nM; nOffset += 4;
		*((int32_ttt*)(pBuff + nOffset)) = nN; nOffset += 4;
		*((int32_ttt*)(pBuff + nOffset)) = nElementSize; nOffset += 4;
		*((int32_ttt*)(pBuff + nOffset)) = nClassID; nOffset += 4; // for padding
		pDataForArg = nwGetData(vArrays[i]);
		memcpy(pBuff+ nOffset, pDataForArg, nSingleArgLen);

		nOffset += nSingleArgLenCorrected;
	}

	*((int64_ttt*)(pBuff )) = a_numOfArgs;

}

static int32_ttt DeSeriToResourseCurrent(
	void* /*a_matEng*/,
	matlab::engine::versioning::TpResourse* /*a_pResourse*/,
	int32_ttt /*a_byteStrLen*/, const u_char_ttt* a_byteStream)
{
	int32_ttt nNumOfArray = (int32_ttt)(*((int64_ttt*)(a_byteStream)));
	return nNumOfArray;
}


static void DeseResToArgsCurrent(
	int32_ttt a_numOfArgs, void* a_vpArgs[],
	int32_ttt a_byteStrLen, const u_char_ttt* a_byteStream,
	matlab::engine::versioning::TpResourse /*a_pResourse*/)
{
	void* pDataForArg;
	const u_char_ttt* pBuff = a_byteStream;
	nwArray** vArrays = (nwArray**)a_vpArgs;
	int32_ttt nM, nN, nElementSize, nClassID;
	int32_ttt nOffset(0);
	int32_ttt nSingleArgLen(0);
	int32_ttt nSingleArgLenCorrected(0);
	int32_ttt nNumOfArray = (int32_ttt)(*((int64_ttt*)(a_byteStream)));
	
	nOffset += 8;
	nNumOfArray = nNumOfArray > a_numOfArgs ? a_numOfArgs : nNumOfArray;
	for (int i(0); i < nNumOfArray; ++i) {
		//a_vpArgs[i] = ;
		nM = *((int32_ttt*)(pBuff + nOffset)); nOffset += 4;
		nN = *((int32_ttt*)(pBuff + nOffset)); nOffset += 4;
		nElementSize = *((int32_ttt*)(pBuff + nOffset)); nOffset += 4;
		nClassID = *((int32_ttt*)(pBuff + nOffset)); nOffset += 4; // for padding
		nSingleArgLen = nM*nN*nElementSize;
		nSingleArgLenCorrected = ((nSingleArgLen/8)+(nSingleArgLen%8?1:0)) * 8;
		if ((nSingleArgLenCorrected<0)||(nOffset+nSingleArgLenCorrected>a_byteStrLen)) { break; }

		vArrays[i] = nwResizeNumericMatrix(vArrays[i],nM, nN, (mxClassID)nClassID, mxREAL);
		if (!vArrays[i]) { break; }
		pDataForArg = nwGetData(vArrays[i]);
		memcpy(pDataForArg,pBuff + nOffset, nSingleArgLen);

		nOffset += nSingleArgLenCorrected;
	}
}
