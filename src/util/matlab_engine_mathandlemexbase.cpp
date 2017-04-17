/*
 *	File: matlab_engine_mathandlemexbase.cpp
 *
 *	Created on: 17 Apr, 2017
 *	Author    : Davit Kalantaryan (Email: davit.kalantaryan@desy.de)
 *
 *
 */

#include "matlab_engine_mathandlemexbase.hpp"
#include <stddef.h>


mxArray* matlab::engine::MatHandleMexBase::newGetVariable(const char* a_workspace, const char* a_name)
{
	return mexGetVariable(a_workspace, a_name);
}


mxArray* matlab::engine::MatHandleMexBase::newEvalStringWithTrap(const char* a_stringToEval)
{
	return ::mexEvalStringWithTrap(a_stringToEval);
}


mxArray* matlab::engine::MatHandleMexBase::newCallMATLABWithTrap(
	int         a_nlhs,       /* number of expected outputs */
	mxArray     *a_plhs[],    /* pointer array to outputs */
	int         a_nrhs,       /* number of inputs */
	mxArray     *a_prhs[],    /* pointer array to inputs */
	const char  *a_fcn_name   /* name of function to execute */)
{
	return mexCallMATLABWithTrap(a_nlhs, a_plhs, a_nrhs, a_prhs, a_fcn_name);
}
