/*****************************************************************************
 * File   : mat_matrix.cpp
 * created: 2017 May 02
 *****************************************************************************
 * Author:	D.Kalantaryan, Tel:+49(0)33762/77552 kalantar
 * Email :	davit.kalantaryan@desy.de
 * Mail  :	DESY, Platanenallee 6, 15738 Zeuthen
 *****************************************************************************
 * Description
 *   ...
 ****************************************************************************/
#ifdef use_mat_matrix

#include "mat_matrix.h"
#include <memory.h>
#include <malloc.h>
#include <string.h>

#ifdef __cplusplus
extern "C" {
#endif

static size_t ClassIdToElementSize(mxClassID a_classid);
static nwArray* nwsCreateEmptyArray(void);

struct nwArray_struct{
	void*			pData;
	size_t			unM;
	size_t			unN;
	size_t			unElementSize;
	size_t			unMaxSize;
	mxClassID		classId;
	mxComplexity	complexity;
	int				isPointerFromOut;
	int				isLocked;
};

size_t nwGetM(const nwArray* a_pArray)
{
	return a_pArray->unM;
}

size_t nwGetN(const nwArray* a_pArray)
{
	return a_pArray->unN;
}


size_t nwGetElementSize(const nwArray* a_pArray)
{
	return a_pArray->unElementSize;
}


mxClassID nwGetClassID(const nwArray* a_pArray)
{
	return a_pArray->classId;
}


void* nwGetData(const nwArray* a_pArray)
{
	return a_pArray->pData;
}


nwArray* nwCreateEmptyPersistantArray(void)
{
	nwArray* pRetArray = nwsCreateEmptyArray();
	if (pRetArray) {pRetArray->isLocked = 1;}
	return pRetArray;
}

nwArray* nwResizeNumericMatrix(
	nwArray* a_pSrc,size_t a_m, size_t a_n, mxClassID a_classid, mxComplexity a_flag)
{
	size_t unElementSize = ClassIdToElementSize(a_classid);
	size_t unSizeToAlloc = unElementSize*a_m*a_n;

	if (!a_pSrc) { a_pSrc = nwsCreateEmptyArray(); if (!a_pSrc) { return NULL; } }

	if (unSizeToAlloc > a_pSrc->unMaxSize) {
		void* pTemp = realloc(a_pSrc->pData, unSizeToAlloc);
		if (!pTemp) { free(a_pSrc); return NULL; }
		a_pSrc->pData = pTemp;
		a_pSrc->unMaxSize = unSizeToAlloc;
	}

	a_pSrc->unM = a_m;
	a_pSrc->unN = a_n;
	a_pSrc->unElementSize = unElementSize;
	a_pSrc->classId = a_classid;
	a_pSrc->complexity = a_flag;

	return a_pSrc;
}


void nwDestroyArray(nwArray *a_pa)
{
	if ((!a_pa) && (a_pa->isLocked)) { return; }
	if (!a_pa->isPointerFromOut) { free(a_pa->pData); }
	free( a_pa);
}


/*/////////////////////////////////////////*/
#if 0
/**
 * Enumeration corresponding to all the valid mxArray types.
 */
typedef enum
{
	mxUNKNOWN_CLASS = 0,
	mxCELL_CLASS,
	mxSTRUCT_CLASS,
	mxLOGICAL_CLASS,
	mxCHAR_CLASS,
	mxVOID_CLASS,
	mxDOUBLE_CLASS,
	mxSINGLE_CLASS,
	mxINT8_CLASS,
	mxUINT8_CLASS,
	mxINT16_CLASS,
	mxUINT16_CLASS,
	mxINT32_CLASS,
	mxUINT32_CLASS,
	mxINT64_CLASS,
	mxUINT64_CLASS,
	mxFUNCTION_CLASS,
	mxOPAQUE_CLASS,
	mxOBJECT_CLASS, /* keep the last real item in the list */
#if defined(_LP64) || defined(_WIN64)
	mxINDEX_CLASS = mxUINT64_CLASS,
#else
	mxINDEX_CLASS = mxUINT32_CLASS,
#endif
	/* TEMPORARY AND NASTY HACK UNTIL mxSPARSE_CLASS IS COMPLETELY ELIMINATED */
	mxSPARSE_CLASS = mxVOID_CLASS /* OBSOLETE! DO NOT USE */
}
mxClassID;
#endif

static nwArray* nwsCreateEmptyArray(void)
{
	nwArray* pRetArray = (nwArray*)malloc(sizeof(nwArray));
	if (pRetArray) {memset(pRetArray, 0, sizeof(nwArray));}
	return pRetArray;
}

static size_t ClassIdToElementSize(mxClassID a_classid)
{
	switch (a_classid)
	{
	case mxUNKNOWN_CLASS:
		return 0;
	case mxSTRUCT_CLASS:
		return 0;
	case mxLOGICAL_CLASS: // true, false
		return 1;
	case mxCHAR_CLASS: // char*
		return 1;
	case mxVOID_CLASS:
		return 0;
	case mxDOUBLE_CLASS:
		return 8;
	case mxSINGLE_CLASS:
		return 4;
	case mxINT8_CLASS:
		return 1;
	case mxUINT8_CLASS:
		return 1;
	case mxINT16_CLASS:
		return 2;
	case mxUINT16_CLASS:
		return 2;
	case mxINT32_CLASS:
		return 4;
	case mxUINT32_CLASS:
		return 4;
	case mxINT64_CLASS:
		return 8;
	case mxUINT64_CLASS:
		return 8;
	case mxFUNCTION_CLASS:
		return 8;
	case mxOPAQUE_CLASS: // ???
		return 0;
	case mxOBJECT_CLASS: // ???
		return 0;
	default:
		return 0;
	}
	return 0;
}

#ifdef __cplusplus
}
#endif


/*///////////////////////////////////*/
#ifdef __cplusplus

PersistantArgsGroup::PersistantArgsGroup(int a_size)
	:
	m_nSize(a_size)
{
	m_ppArgs = (nwArray**)malloc(a_size * sizeof(nwArray*));
	if (!m_ppArgs) { throw "low memory!"; }

	for (int i(0); i < a_size; ++i) {
		m_ppArgs[i] = nwCreateEmptyPersistantArray();
		m_ppArgs[i]->isLocked = 1;
	}
}


PersistantArgsGroup::~PersistantArgsGroup()
{
	for (int i(m_nSize-1); i >= 0; --i) {
		m_ppArgs[i]->isLocked = 0;
		nwDestroyArray(m_ppArgs[i]);
	}
}


nwArray** PersistantArgsGroup::Array()
{
	return m_ppArgs;
}


nwTypeConstArray* PersistantArgsGroup::Array()const
{
	return (nwTypeConstArray*)m_ppArgs;
}


nwTypeArray& PersistantArgsGroup::operator[](int a_index)
{
	return m_ppArgs[a_index];
}

int PersistantArgsGroup::SetValue(int a_index, const bool& a_value)
{
	m_ppArgs[a_index] = nwResizeNumericMatrix(
		m_ppArgs[a_index], 1, 1, mxLOGICAL_CLASS, mxREAL);

	if (!m_ppArgs[a_index]) { return -1; }

	bool* pValue = (bool*)nwGetData(m_ppArgs[a_index]);
	*pValue = a_value;
	return 0;
}

int  PersistantArgsGroup::SetValue(int a_index, const double& a_value)
{
	m_ppArgs[a_index] = nwResizeNumericMatrix(
		m_ppArgs[a_index], 1, 1, mxDOUBLE_CLASS, mxREAL);

	if (!m_ppArgs[a_index]) { return -1; }

	double* pValue = (double*)nwGetData(m_ppArgs[a_index]);
	*pValue = a_value;
	return 0;
}


int  PersistantArgsGroup::SetValue(int a_index, const float& a_value)
{
	m_ppArgs[a_index] = nwResizeNumericMatrix(
		m_ppArgs[a_index], 1, 1, mxSINGLE_CLASS, mxREAL);

	if (!m_ppArgs[a_index]) { return -1; }

	float* pValue = (float*)nwGetData(m_ppArgs[a_index]);
	*pValue = a_value;
	return 0;
}


int  PersistantArgsGroup::SetValue(int a_index, const int8_ttt& a_value)
{
	m_ppArgs[a_index] = nwResizeNumericMatrix(
		m_ppArgs[a_index], 1, 1, mxINT8_CLASS, mxREAL);

	if (!m_ppArgs[a_index]) { return -1; }

	int8_ttt* pValue = (int8_ttt*)nwGetData(m_ppArgs[a_index]);
	*pValue = a_value;
	return 0;
}

int  PersistantArgsGroup::SetValue(int a_index, const uint8_ttt& a_value)
{
	m_ppArgs[a_index] = nwResizeNumericMatrix(
		m_ppArgs[a_index], 1, 1, mxUINT8_CLASS, mxREAL);

	if (!m_ppArgs[a_index]) { return -1; }

	uint8_ttt* pValue = (uint8_ttt*)nwGetData(m_ppArgs[a_index]);
	*pValue = a_value;
	return 0;
}


int  PersistantArgsGroup::SetValue(int a_index, const int16_ttt& a_value)
{
	m_ppArgs[a_index] = nwResizeNumericMatrix(
		m_ppArgs[a_index], 1, 1, mxINT16_CLASS, mxREAL);

	if (!m_ppArgs[a_index]) { return -1; }

	int16_ttt* pValue = (int16_ttt*)nwGetData(m_ppArgs[a_index]);
	*pValue = a_value;
	return 0;
}

int  PersistantArgsGroup::SetValue(int a_index, const uint16_ttt& a_value)
{
	m_ppArgs[a_index] = nwResizeNumericMatrix(
		m_ppArgs[a_index], 1, 1, mxUINT16_CLASS, mxREAL);

	if (!m_ppArgs[a_index]) { return -1; }

	uint16_ttt* pValue = (uint16_ttt*)nwGetData(m_ppArgs[a_index]);
	*pValue = a_value;
	return 0;
}

int  PersistantArgsGroup::SetValue(int a_index, const int32_ttt& a_value)
{
	m_ppArgs[a_index] = nwResizeNumericMatrix(
		m_ppArgs[a_index], 1, 1, mxINT32_CLASS, mxREAL);

	if (!m_ppArgs[a_index]) { return -1; }

	int32_ttt* pValue = (int32_ttt*)nwGetData(m_ppArgs[a_index]);
	*pValue = a_value;
	return 0;
}

int  PersistantArgsGroup::SetValue(int a_index, const uint32_ttt& a_value)
{
	m_ppArgs[a_index] = nwResizeNumericMatrix(
		m_ppArgs[a_index], 1, 1, mxUINT32_CLASS, mxREAL);

	if (!m_ppArgs[a_index]) { return -1; }

	uint32_ttt* pValue = (uint32_ttt*)nwGetData(m_ppArgs[a_index]);
	*pValue = a_value;
	return 0;
}


int  PersistantArgsGroup::SetValue(int a_index, const int64_ttt& a_value)
{
	m_ppArgs[a_index] = nwResizeNumericMatrix(
		m_ppArgs[a_index], 1, 1, mxINT64_CLASS, mxREAL);

	if (!m_ppArgs[a_index]) { return -1; }

	int64_ttt* pValue = (int64_ttt*)nwGetData(m_ppArgs[a_index]);
	*pValue = a_value;
	return 0;
}

int  PersistantArgsGroup::SetValue(int a_index, const uint64_ttt& a_value)
{
	m_ppArgs[a_index] = nwResizeNumericMatrix(
		m_ppArgs[a_index], 1, 1, mxUINT64_CLASS, mxREAL);

	if (!m_ppArgs[a_index]) { return -1; }

	uint64_ttt* pValue = (uint64_ttt*)nwGetData(m_ppArgs[a_index]);
	*pValue = a_value;
	return 0;
}


int PersistantArgsGroup::SetValue(int a_index, const nwTypeChar& a_value)
{
	size_t unStrlenPlus1(strlen(a_value)+1);

	m_ppArgs[a_index] = nwResizeNumericMatrix(
		m_ppArgs[a_index], 1, unStrlenPlus1, mxCHAR_CLASS, mxREAL);

	if (!m_ppArgs[a_index]) { return -1; }
	memcpy(m_ppArgs[a_index]->pData, a_value, unStrlenPlus1);
	return 0;
}


#endif  // #ifdef __cplusplus

#endif  // #ifdef mat_matrix
