/*****************************************************************************
 * File   : matlab_engine_serializer_versioning.cpp
 * created: 2017 Apr 29
 *****************************************************************************
 * Author:	D.Kalantaryan, Tel:+49(0)33762/77552 kalantar
 * Email :	davit.kalantaryan@desy.de
 * Mail  :	DESY, Platanenallee 6, 15738 Zeuthen
 *****************************************************************************
 * Description
 *   ...
 ****************************************************************************/
#include <map>
#include "matlab_engine_serializer_versioning.hpp"
#include <memory.h>

#define DBL_INTS_TO_LONG FUNCTION_INDEX_FR_VERS_AND_SERI_TYPE

namespace matlab{namespace engine{ namespace versioning{

static ::std::map<uint64_ttt, FncPointers>*	s_pMapFunctions = NULL;

matlab::engine::versioning::Versionizer::Versionizer(
	int32_ttt a_version, int32_ttt a_type,
	TypeSerializeToResourse a_fpSeriToRs, TypeSeResToBtStream a_fpSeRsToBtStr,
	TypeDeSeriToResourse a_fpDeseriToRs, TypeDeseResToArgs a_fpDeseRsToArgs)
	:
	m_fncIndex(DBL_INTS_TO_LONG(a_version, a_type))
{
	FncPointers aNewFncs;

	aNewFncs.fpSeriToRs = a_fpSeriToRs;
	aNewFncs.fpSeRsToBtStr = a_fpSeRsToBtStr;
	aNewFncs.fpDeseriToRs = a_fpDeseriToRs;
	aNewFncs.fpDeseRsToArgs = a_fpDeseRsToArgs;

	if (!s_pMapFunctions) { s_pMapFunctions = new ::std::map<uint64_ttt, FncPointers>; }

	s_pMapFunctions->insert(
		::std::pair<uint64_ttt, FncPointers>(DBL_INTS_TO_LONG(a_version, a_type), aNewFncs));
}


matlab::engine::versioning::Versionizer::~Versionizer()
{
	s_pMapFunctions->erase(m_fncIndex);
	if(s_pMapFunctions->size()==0){delete s_pMapFunctions; s_pMapFunctions=NULL;}
}


TypeSerializeToResourse	FindSeriToRes(int32_ttt a_version, int32_ttt a_type, FncPointers* a_pFuncs)
{
	uint64_ttt ullnFncIndex(DBL_INTS_TO_LONG(a_version,a_type));

	if (ullnFncIndex != a_pFuncs->fncIndex) { 
		memset(a_pFuncs, 0, sizeof(FncPointers));
		a_pFuncs->fncIndex = ullnFncIndex;
	}
	if (!a_pFuncs->fpSeriToRs) {
		FncPointers aNewFncs;

		if (s_pMapFunctions->count(ullnFncIndex)) {
			aNewFncs = s_pMapFunctions->operator[](ullnFncIndex);
			a_pFuncs->fpSeriToRs = aNewFncs.fpSeriToRs;
			if (!a_pFuncs->fpSeriToRs) { a_pFuncs->fpSeriToRs = aNewFncs.fpSeriToRs; }
			if (!a_pFuncs->fpSeRsToBtStr) { a_pFuncs->fpSeRsToBtStr = aNewFncs.fpSeRsToBtStr; }
			if (!a_pFuncs->fpDeseriToRs) { a_pFuncs->fpDeseriToRs = aNewFncs.fpDeseriToRs; }
			if (!a_pFuncs->fpDeseRsToArgs) { a_pFuncs->fpDeseRsToArgs = aNewFncs.fpDeseRsToArgs; }
		}
	}

	return a_pFuncs->fpSeriToRs;
}


TypeSeResToBtStream	FindSeResToBtStream(int32_ttt a_version, int32_ttt a_type, FncPointers* a_pFuncs)
{
	uint64_ttt ullnFncIndex(DBL_INTS_TO_LONG(a_version, a_type));

	if (ullnFncIndex != a_pFuncs->fncIndex) { 
		memset(a_pFuncs, 0, sizeof(FncPointers)); 
		a_pFuncs->fncIndex = ullnFncIndex;
	}
	if (!a_pFuncs->fpSeRsToBtStr) {
		FncPointers aNewFncs;

		if (s_pMapFunctions->count(ullnFncIndex)) {
			aNewFncs = s_pMapFunctions->operator[](ullnFncIndex);
			a_pFuncs->fpSeRsToBtStr = aNewFncs.fpSeRsToBtStr;
			if (!a_pFuncs->fpSeriToRs) { a_pFuncs->fpSeriToRs = aNewFncs.fpSeriToRs; }
			if (!a_pFuncs->fpSeRsToBtStr) { a_pFuncs->fpSeRsToBtStr = aNewFncs.fpSeRsToBtStr; }
			if (!a_pFuncs->fpDeseriToRs) { a_pFuncs->fpDeseriToRs = aNewFncs.fpDeseriToRs; }
			if (!a_pFuncs->fpDeseRsToArgs) { a_pFuncs->fpDeseRsToArgs = aNewFncs.fpDeseRsToArgs; }
		}
	}

	return a_pFuncs->fpSeRsToBtStr;
}


TypeDeSeriToResourse FindDeSeriToRes(int32_ttt a_version, int32_ttt a_type, FncPointers* a_pFuncs)
{
	uint64_ttt ullnFncIndex(DBL_INTS_TO_LONG(a_version, a_type));

	if (ullnFncIndex != a_pFuncs->fncIndex) { 
		memset(a_pFuncs, 0, sizeof(FncPointers)); 
		a_pFuncs->fncIndex = ullnFncIndex;
	}
	if (!a_pFuncs->fpDeseriToRs) {
		FncPointers aNewFncs;

		if (s_pMapFunctions->count(ullnFncIndex)) {
			aNewFncs = s_pMapFunctions->operator[](ullnFncIndex);
			a_pFuncs->fpDeseriToRs = aNewFncs.fpDeseriToRs;
			if (!a_pFuncs->fpSeriToRs) { a_pFuncs->fpSeriToRs = aNewFncs.fpSeriToRs; }
			if (!a_pFuncs->fpSeRsToBtStr) { a_pFuncs->fpSeRsToBtStr = aNewFncs.fpSeRsToBtStr; }
			if (!a_pFuncs->fpDeseriToRs) { a_pFuncs->fpDeseriToRs = aNewFncs.fpDeseriToRs; }
			if (!a_pFuncs->fpDeseRsToArgs) { a_pFuncs->fpDeseRsToArgs = aNewFncs.fpDeseRsToArgs; }
		}
	}

	return a_pFuncs->fpDeseriToRs;
}


TypeDeseResToArgs FindDeseRsToArgs(int32_ttt a_version, int32_ttt a_type, FncPointers* a_pFuncs)
{
	uint64_ttt ullnFncIndex(DBL_INTS_TO_LONG(a_version, a_type));

	if (ullnFncIndex != a_pFuncs->fncIndex) { 
		memset(a_pFuncs, 0, sizeof(FncPointers)); 
		a_pFuncs->fncIndex = ullnFncIndex;
	}
	if (!a_pFuncs->fpDeseRsToArgs) {
		FncPointers aNewFncs;

		if (s_pMapFunctions->count(ullnFncIndex)) {
			aNewFncs = s_pMapFunctions->operator[](ullnFncIndex);
			a_pFuncs->fpDeseRsToArgs = aNewFncs.fpDeseRsToArgs;
			if (!a_pFuncs->fpSeriToRs) { a_pFuncs->fpSeriToRs = aNewFncs.fpSeriToRs; }
			if (!a_pFuncs->fpSeRsToBtStr) { a_pFuncs->fpSeRsToBtStr = aNewFncs.fpSeRsToBtStr; }
			if (!a_pFuncs->fpDeseriToRs) { a_pFuncs->fpDeseriToRs = aNewFncs.fpDeseriToRs; }
			if (!a_pFuncs->fpDeseRsToArgs) { a_pFuncs->fpDeseRsToArgs = aNewFncs.fpDeseRsToArgs; }
		}
	}

	return a_pFuncs->fpDeseRsToArgs;
}

/*////////////////////////////*/
FncPointers::FncPointers(){memset(this, 0, sizeof(FncPointers));}

}}}
