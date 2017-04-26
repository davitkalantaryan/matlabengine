/*****************************************************************************
 * File   : mmatlab_engine_mathandlemexbase.hpp
 * created: 2017 Apr 25
 *****************************************************************************
 * Author:	D.Kalantaryan, Tel:+49(0)33762/77552 kalantar
 * Email :	davit.kalantaryan@desy.de
 * Mail  :	DESY, Platanenallee 6, 15738 Zeuthen
 *****************************************************************************
 * Description
 *   ...
 ****************************************************************************/

#ifndef __matlab_engine_mathandleeng_hpp__
#define __matlab_engine_mathandleeng_hpp__

#include <engine.h>
#include <thread_cpp11.hpp>
#include "matlab_engine_mathandlebase.hpp"
#include <common_unnamedsemaphorelite.hpp>

namespace matlab{ namespace engine{


class MatHandleEng : public MatHandleBase
{
public:
    MatHandleEng();
    virtual ~MatHandleEng();

    virtual void Start() __OVERRIDE__ ;
    virtual void Stop() __OVERRIDE__;

	mxArray* newGetVariable(const char* workspace, const char* name) __OVERRIDE__;
    int newPutVariable(const char *workspace, const char *varname,const mxArray *pm)__OVERRIDE__;
	mxArray* newEvalStringWithTrap(const char* stringToEval)__OVERRIDE__;
	mxArray	*newCallMATLABWithTrap(
		int         nlhs,       /* number of expected outputs */
		mxArray     *plhs[],    /* pointer array to outputs */
		int         nrhs,       /* number of inputs */
		mxArray     *prhs[],    /* pointer array to inputs */
		const char  *fcn_name   /* name of function to execute */
	) __OVERRIDE__;
    virtual void CallOnMatlabThread(void* owner, TypeClbK fpClb, void*arg)__OVERRIDE__;

private:
    void MatlabThreadFunction();

private:
    Engine*                         m_pEngine;
    STD::thread                     m_threadMat;
    common::UnnamedSemaphoreLite    m_semaMat;
    volatile int                    m_nRun;
    volatile int                    m_nReturn;
};

}}

#endif // #ifndef __matlab_engine_mathandleeng_hpp__
