/*
 *	File: matlab_engine_mathandlemex.hpp
 *
 *	Created on: 14 Apr, 2017
 *	Author    : Davit Kalantaryan (Email: davit.kalantaryan@desy.de)
 *
 *
 */

#ifndef __matlab_engine_mathandlemex_hpp__
#define __matlab_engine_mathandlemex_hpp__

#include "matlab_engine_mathandlebase.hpp"
#include <common_fifofast.hpp>

namespace matlab{ namespace engine{

struct SLsnCallbackItem { void*arg;TypeClbK clbk;};

class MatHandleMex : public MatHandleBase
{
public:
	MatHandleMex();
	virtual ~MatHandleMex();

	void Start();
	void Stop();

	mxArray* mexEvalStringWithTrap(const char* stringToEval);
	mxArray *mxCreateNumericMatrixC(size_t m, size_t n, mxClassID classid, mxComplexity flag);
	void *mxGetData(const mxArray *pa);
	mxArray	*mexCallMATLABWithTrapC(
		int         nlhs,       /* number of expected outputs */
		mxArray     *plhs[],    /* pointer array to outputs */
		int         nrhs,       /* number of inputs */
		mxArray     *prhs[],    /* pointer array to inputs */
		const char  *fcn_name   /* name of function to execute */
	);
	mxArray *mxGetCellC(const mxArray *pa, size_t i);
	void mxDestroyArray(mxArray *pa);
	void CallOnMatlabThread(void* owner, TypeClbK fpClb,void*arg);

private:
	static void* ListenerCallbackStatic(void* a_arg);

private:
	void*									m_pListener;
	common::FifoFast<SLsnCallbackItem, 8>	m_fifoJobs;

};

}}

#endif // #ifndef __matlab_engine_mathandlemex_hpp__
