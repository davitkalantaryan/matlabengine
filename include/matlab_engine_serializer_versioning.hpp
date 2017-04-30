/*****************************************************************************
 * File   : matlab_engine_serializer_versioning.hpp
 * created: 2017 Apr 29
 *****************************************************************************
 * Author:	D.Kalantaryan, Tel:+49(0)33762/77552 kalantar
 * Email :	davit.kalantaryan@desy.de
 * Mail  :	DESY, Platanenallee 6, 15738 Zeuthen
 *****************************************************************************
 * Description
 *   ...
 ****************************************************************************/
#ifndef __matlab_engine_serializer_versioning_hpp__
#define __matlab_engine_serializer_versioning_hpp__

#define FUNCTION_INDEX_FR_VERS_AND_SERI_TYPE(_int1_,_int2_)	\
		((((uint64_ttt)(_int1_))<<32)|((uint64_ttt)(_int2_)))

#define FunctionIndex	FUNCTION_INDEX_FR_VERS_AND_SERI_TYPE

#ifndef u_char_ttt_defined
typedef unsigned char			u_char_ttt;
#define u_char_ttt_defined
#endif
#ifndef int32_ttt_defined
typedef int						int32_ttt;
#define int32_ttt_defined
#endif
#ifndef int64_ttt_defined
typedef long long int			int64_ttt;
#define int64_ttt_defined
#endif
#ifndef uint64_ttt_defined
typedef unsigned long long int	uint64_ttt;
#define uint64_ttt_defined
#endif

namespace matlab{ namespace engine{namespace versioning{

typedef void* TpResourse;
enum FNCS {SERI_TO_RES,RES_TO_BTSTREAM,DESERI_TO_RES,RES_TO_ARGS};
namespace SERI_TYPE { enum { MAT_UNDOCU = 0, RAW1 }; }

typedef int32_ttt (*TypeSerializeToResourse)(
	void* matEng,
	TpResourse* pResourse,
	int32_ttt numOfArgs, const void* vpArgs[]);

typedef void (*TypeSeResToBtStream)(
	int32_ttt byteStrLen, u_char_ttt* byteStream,
	TpResourse resourse);

typedef int32_ttt (*TypeDeSeriToResourse)(
	void* matEng,
	TpResourse* pResourse,
	int32_ttt byteStrLen, const u_char_ttt* byteStream);

typedef void (*TypeDeseResToArgs)(
	int32_ttt numOfArgs, void* vpArgs[],
	TpResourse resourse);

class Versionizer
{
public:
	Versionizer(
		int32_ttt version, int32_ttt type,
		TypeSerializeToResourse fpSeriToRs, TypeSeResToBtStream fpSeRsToBtStr,
		TypeDeSeriToResourse fpDeseriToRs, TypeDeseResToArgs fpDeseRsToArgs);
	~Versionizer();
private:
	int64_ttt	m_fncIndex;
};


struct FncPointers { 
	FncPointers();
	uint64_ttt fncIndex;
	TypeSerializeToResourse fpSeriToRs; TypeSeResToBtStream fpSeRsToBtStr;
	TypeDeSeriToResourse fpDeseriToRs; TypeDeseResToArgs fpDeseRsToArgs;
};

extern TypeSerializeToResourse	FindSeriToRes(int32_ttt version, int32_ttt type, FncPointers*);
extern TypeSeResToBtStream		FindSeResToBtStream(int32_ttt version, int32_ttt type, FncPointers*);
extern TypeDeSeriToResourse		FindDeSeriToRes(int32_ttt version, int32_ttt type, FncPointers*);
extern TypeDeseResToArgs		FindDeseRsToArgs(int32_ttt version, int32_ttt type, FncPointers*);

}}}


#endif  // #ifndef __matlab_engine_serializer_versioning_hpp__
