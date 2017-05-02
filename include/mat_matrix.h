/*****************************************************************************
 * File   : mat_matrix.h
 * created: 2017 May 02
 *****************************************************************************
 * Author:	D.Kalantaryan, Tel:+49(0)33762/77552 kalantar
 * Email :	davit.kalantaryan@desy.de
 * Mail  :	DESY, Platanenallee 6, 15738 Zeuthen
 *****************************************************************************
 * Description
 *   ...
 ****************************************************************************/
#ifndef __mat_matrix_h__
#define __mat_matrix_h__

#include <matrix.h>
#include <stddef.h>

#ifndef u_char_ttt_defined
typedef unsigned char			u_char_ttt;
#define u_char_ttt_defined
#endif
#ifndef int8_ttt_defined
typedef char					int8_ttt;
#define int8_ttt_defined
#endif
#ifndef uint8_ttt_defined
typedef unsigned char			uint8_ttt;
#define uint8_ttt_defined
#endif
#ifndef int16_ttt_defined
typedef short int				int16_ttt;
#define int16_ttt_defined
#endif
#ifndef uint16_ttt_defined
typedef unsigned short int		uint16_ttt;
#define uint16_ttt_defined
#endif
#ifndef int32_ttt_defined
typedef int						int32_ttt;
#define int32_ttt_defined
#endif
#ifndef uint32_ttt_defined
typedef unsigned int			uint32_ttt;
#define uint32_ttt_defined
#endif
#ifndef int64_ttt_defined
typedef long long int			int64_ttt;
#define int64_ttt_defined
#endif
#ifndef uint64_ttt_defined
typedef unsigned long long int	uint64_ttt;
#define uint64_ttt_defined
#endif

#ifndef use_mat_matrix

#define nwArray					mxArray
#define nwGetM					mxGetM
#define nwGetN					mxGetN
#define nwGetElementSize		mxGetElementSize
#define nwGetClassID			mxGetClassID
#define nwGetData				mxGetData
#define nwDestroyArray			mxDestroyArray
#define nwResizeNumericMatrix(_pSrc,_m, _n, _classid, _flag) \
				mxCreateNumericMatrix((_m),(_n),(_classid),(_flag))

#else  // #ifndef use_mat_matrix


#ifdef __cplusplus
extern "C" {
#endif

typedef struct nwArray_struct	nwArray;

size_t nwGetM(const nwArray* pArray);
size_t nwGetN(const nwArray* pArray);
size_t nwGetElementSize(const nwArray* pArray);
mxClassID nwGetClassID(const nwArray* pArray);
void* nwGetData(const nwArray* pArray);
void nwDestroyArray(nwArray *pa);

nwArray *nwResizeNumericMatrix(
	nwArray* pSrc,
	size_t m, size_t n, mxClassID classid, mxComplexity flag);

nwArray* nwCreateEmptyPersistantArray(void);


#ifdef __cplusplus
}
#endif

#ifdef __cplusplus

typedef const nwArray* nwTypeConstArray;
typedef nwArray*	nwTypeArray;
typedef const char*	nwTypeChar;

class PersistantArgsGroup
{
public:
	PersistantArgsGroup(int size);
	~PersistantArgsGroup();

	nwArray**		Array();
	nwTypeConstArray* Array()const;

	nwTypeArray& operator[](int index);

	int SetValue(int index,const double& value);
	int SetValue(int index, const float& value);

	int SetValue(int index, const bool& value);
	int SetValue(int index, const int8_ttt& value);
	int SetValue(int index, const uint8_ttt& value);
	int SetValue(int index, const int16_ttt& value);
	int SetValue(int index, const uint16_ttt& value);
	int SetValue(int index, const int32_ttt& value);
	int SetValue(int index, const uint32_ttt& value);
	int SetValue(int index, const int64_ttt& value);
	int SetValue(int index, const uint64_ttt& value);
	int SetValue(int index, const nwTypeChar& value);

private:
	int			m_nSize;
	nwArray**	m_ppArgs;
};

#include "mat_matrix.tos"
#endif // #ifdef __cplusplus

#endif  // #ifndef use_mat_matrix

#endif  // #ifndef __mat_matrix_h__
