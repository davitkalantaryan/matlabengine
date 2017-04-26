/*
 *	File: matlab_engine_mathandlemexbase.hpp
 *
 *	Created on: 17 Apr, 2017
 *	Author    : Davit Kalantaryan (Email: davit.kalantaryan@desy.de)
 *
 *
 */

#ifndef __matlab_engine_mathandlemexbase_hpp__
#define __matlab_engine_mathandlemexbase_hpp__

#include "matlab_engine_mathandlebase.hpp"

namespace matlab{ namespace engine{


class MatHandleMexBase : public MatHandleBase
{
public:
	MatHandleMexBase() {}
	virtual ~MatHandleMexBase() {}

	virtual void Start() __OVERRIDE__  {}
	virtual void Stop() __OVERRIDE__ {}

	mxArray* newGetVariable(const char* workspace, const char* name) __OVERRIDE__;
	int newPutVariable(const char *workspace, const char *varname, const mxArray *pm) __OVERRIDE__;
	mxArray* newEvalStringWithTrap(const char* stringToEval)__OVERRIDE__;
	mxArray	*newCallMATLABWithTrap(
		int         nlhs,       /* number of expected outputs */
		mxArray     *plhs[],    /* pointer array to outputs */
		int         nrhs,       /* number of inputs */
		mxArray     *prhs[],    /* pointer array to inputs */
		const char  *fcn_name   /* name of function to execute */
	) __OVERRIDE__;
	virtual void CallOnMatlabThread(void*, TypeClbK, void*) __OVERRIDE__ {}

};

}}

#endif // #ifndef __matlab_engine_mathandlemex_hpp__
