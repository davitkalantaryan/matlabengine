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
#include <common_fifofast.hpp>
#include "matlab_engine_serializer.hpp"

#ifndef __THISCALL__
#ifdef _MSC_VER
#define __THISCALL__ __thiscall
#else
#define __THISCALL__ 
#endif
#endif

// This should be done after check
#if __cplusplus >= 199711L
#define __OVERRIDE__	override
#else
#define __OVERRIDE__
#endif

namespace matlab{ namespace engine{

typedef void (__THISCALL__ * TypeClbK)(void*owner,void*arg);

extern void* GetFuncPointer(int,...);
namespace STDPIPES { enum { STDOUT=1, STDERR=2 }; }
struct SLsnCallbackItem { void*owner; TypeClbK clbk; void*arg; };

class MatHandleBase
{
public:
	MatHandleBase();
	virtual ~MatHandleBase();

	virtual void Start() = 0;
	virtual void Stop() = 0;

#if 1
	void* MatlabArrayToByteStream1(int32_ttt type, int numOfArgs, mxArray** inputs, int32_ttt* length);
	void* MatlabArrayToByteStream2(int32_ttt type, int numOfArgs, const mxArray* inputs[], int32_ttt* length);
	int ByteStreamToMatlabArray(int32_ttt type,int maxOutSize,mxArray** output, 
		int byteStreamLen,const void* byteStream);
#endif
	
	int newFrprint(int out, const char* fmt, ...);

	virtual mxArray* newGetVariable(const char* workspace, const char* name)=0;
	virtual mxArray* newEvalStringWithTrap(const char* stringToEval) = 0;
	virtual mxArray* newCallMATLABWithTrap(
		int         nlhs,       /* number of expected outputs */
		mxArray     *plhs[],    /* pointer array to outputs */
		int         nrhs,       /* number of inputs */
		mxArray     *prhs[],    /* pointer array to inputs */
		const char  *fcn_name   /* name of function to execute */
	)=0;

	virtual void CallOnMatlabThread(void* owner, TypeClbK fpClb,void*arg)=0;
	template <typename TypeCls>
	void CallOnMatlabThreadC(TypeCls* a_owner, void (TypeCls::*a_fpClbK)(void*arg), void*a_arg) { 
		CallOnMatlabThread((void*)a_owner, (TypeClbK)GetFuncPointer(1, a_fpClbK),a_arg); }
	void PrintFromAnyThread(const char* string);

protected:
	void AddMatlabJob(void* owner, TypeClbK fpClb, void* arg);
	void HandleAllJobs(void);

private:
	void PrintOnMatlabThreadPrivate(void* string);

private:
	common::FifoFast<SLsnCallbackItem, 8>	m_fifoJobs;

};

}}

#endif // #ifndef __matlab_engine_mathandlebase_hpp__
