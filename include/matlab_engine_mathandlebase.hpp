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

#include "matrix.h"
#include <stdio.h>
#include <stdarg.h>
#include <common_fifofast.hpp>
#include <common_defination.h>
#ifdef WIN32
#include <windows.h>
#else
#include <unistd.h>
#ifndef Sleep
#define Sleep(__X__)	usleep(1000*(__X__))
#endif
#endif
#include <string>

namespace matlab{ namespace engine{

typedef void (__THISCALL__ * TypeClbK)(void*owner,void*arg);

namespace STDPIPES { enum { STDOUT=1, STDERR=2 }; }
namespace START_RET { enum { STARTED = 0, ALREADY_RUN = 1,ENG_ERROR=-1 }; }

class MatHandleBase
{
public:
	MatHandleBase();
	virtual ~MatHandleBase();

	virtual int Start(const std::string& engineCommand) = 0; // 0 no_error
	virtual void Stop() = 0;
	
	int newFrprint(int out, const char* fmt, ...);
	int Exist(const char* name, const char* type=NULL);

	virtual mxArray* newGetVariable(const char* workspace, const char* name)=0;
	virtual int newPutVariable(const char *workspace, const char *varname,const mxArray *pm) = 0;
	virtual mxArray* newEvalStringWithTrap(const char* stringToEval) = 0;
	virtual mxArray* newCallMATLABWithTrap(
		int         nlhs,       /* number of expected outputs */
		mxArray     *plhs[],    /* pointer array to outputs */
		int         nrhs,       /* number of inputs */
		mxArray     *prhs[],    /* pointer array to inputs */
		const char  *fcn_name   /* name of function to execute */
	)=0;

	virtual void SyncCallOnMatlabThread(void* owner, TypeClbK fpClb,void*arg)=0;
	virtual void AsyncCallOnMatlabThread(void* owner, TypeClbK fpClb, void*arg) = 0;
	template <typename TypeCls>
	void SyncCallOnMatlabThreadC(TypeCls* a_owner, void (TypeCls::*a_fpClbK)(void*arg), void*a_arg) { 
        SyncCallOnMatlabThread((void*)a_owner, (TypeClbK)GetFuncPointer_common(1, a_fpClbK),a_arg); }
	template <typename TypeCls>
	void AsyncCallOnMatlabThreadC(TypeCls* a_owner, void (TypeCls::*a_fpClbK)(void*arg), void*a_arg) {
		AsyncCallOnMatlabThread((void*)a_owner, (TypeClbK)GetFuncPointer_common(1, a_fpClbK), a_arg);
	}
	void PrintFromAnyThread(const char* string);	

private:
	void PrintOnMatlabThreadPrivate(void* string);	

};

}}

#endif // #ifndef __matlab_engine_mathandlebase_hpp__
