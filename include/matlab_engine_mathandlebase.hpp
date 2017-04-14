/*
 *	File: matlab_engine_mathandlebase.hpp
 *
 *	Created on: 12 Apr, 2017
 *	Author    : Davit Kalantaryan (Email: davit.kalantaryan@desy.de)
 *
 *
 */

#ifndef __matlab_engine_mathandlebase_hpp__
#define __matlab_engine_mathandlebase_hpp__

#include <mex.h>
#include <stdarg.h>

#ifndef __THISCALL__
#ifdef _MSC_VER
#define __THISCALL__ __thiscall
#else
#define __THISCALL__ 
#endif
#endif

// This should be done after check
#define __OVERRIDE__	override

namespace matlab{ namespace engine{

typedef void (__THISCALL__ * TypeClbK)(void*owner,void*arg);

extern void* GetFuncPointer(int,...);
namespace STDPIPES { enum { STDOUT=1, STDERR=2 }; }

class MatHandleBase
{
public:
	virtual ~MatHandleBase();

	virtual void Start() = 0;
	virtual void Stop() = 0;

	virtual mxArray* mexEvalStringWithTrap(const char* stringToEval) = 0;
	virtual mxArray *mxCreateNumericMatrixC(size_t m, size_t n, mxClassID classid, mxComplexity flag)=0;
	virtual void *mxGetData(const mxArray *pa)=0;
	virtual mxArray	*mexCallMATLABWithTrapC(
		int         nlhs,       /* number of expected outputs */
		mxArray     *plhs[],    /* pointer array to outputs */
		int         nrhs,       /* number of inputs */
		mxArray     *prhs[],    /* pointer array to inputs */
		const char  *fcn_name   /* name of function to execute */
	)=0;
	virtual mxArray *mxGetCellC(const mxArray *pa, size_t i)=0;
	virtual void mxDestroyArray(mxArray *pa)=0;
	int mexFrprint(int out,const char* fmt,...);

	virtual void CallOnMatlabThread(void* owner, TypeClbK fpClb,void*arg)=0;
	template <typename TypeCls>
	void CallOnMatlabThreadC(TypeCls* a_owner, void (TypeCls::*a_fpClbK)(void*arg), void*a_arg) { 
		CallOnMatlabThread((void*)a_owner, (TypeClbK)GetFuncPointer(1, a_fpClbK),a_arg); }

};

}}

#endif // #ifndef __matlab_engine_mathandlebase_hpp__
