/*****************************************************************************
 * File   : mmatlab_engine_mathandlemexbase.cpp
 * created: 2017 Apr 25
 *****************************************************************************
 * Author:	D.Kalantaryan, Tel:+49(0)33762/77552 kalantar
 * Email :	davit.kalantaryan@desy.de
 * Mail  :	DESY, Platanenallee 6, 15738 Zeuthen
 *****************************************************************************
 * Description
 *   ...
 ****************************************************************************/

#include "matlab_engine_mathandleeng.hpp"
#include <stddef.h>
#include <memory.h>

#ifdef WIN32
#include <windows.h>
#else
#include <unistd.h>
#endif

#define ERROR_VAR_NAME  "a__Str"
#define DUMP_WARNING(...) do{}while(0)


matlab::engine::MatHandleEng::MatHandleEng()
    :
      m_pEngine(NULL),
      m_nRun(0),
      m_fifoJobs(8)
{
    //
}


matlab::engine::MatHandleEng::~MatHandleEng()
{
    //Stop();
}


int matlab::engine::MatHandleEng::Start(const std::string& a_strEngineCommand)
{
    if(m_nRun == 1){return START_RET::ALREADY_RUN;}
	m_strEngineCommand = a_strEngineCommand;
    m_threadMat = STD::thread(&MatHandleEng::MatlabThreadFunction,this);
    m_nReturn = 0;
    while((m_pEngine==NULL)&&(m_nReturn==0)){Sleep(10);}
    return m_nReturn;
}


void matlab::engine::MatHandleEng::Stop()
{
    if(m_nRun == 0){return;}
    m_nRun = 0;
    m_semaMat.post();
    m_threadMat.join();
}


mxArray* matlab::engine::MatHandleEng::newGetVariable(const char* a_workspace, const char* a_name)
{
    DUMP_WARNING(a_workspace);
    return engGetVariable(m_pEngine,a_name);
}


int matlab::engine::MatHandleEng::newPutVariable(
        const char* a_workspace, const char* a_name,const mxArray *a_pm)
{
    DUMP_WARNING(a_workspace);
    return engPutVariable(m_pEngine,a_name,a_pm);
}


mxArray* matlab::engine::MatHandleEng::newEvalStringWithTrap(const char* a_stringToEval)
{
    char vcError[256];

    engEvalString(m_pEngine,"lasterror reset");
    engEvalString(m_pEngine,a_stringToEval);
    engEvalString(m_pEngine,"aLastError=lasterror;" ERROR_VAR_NAME "=aLastError.message;");
    mxArray* pRet = engGetVariable(m_pEngine,ERROR_VAR_NAME);
    engEvalString(m_pEngine,"clear " ERROR_VAR_NAME);

    vcError[255]=0;
    mxGetString(pRet,vcError,255);

    if(vcError[0] != 0 ){printf("error=%s\n",vcError);return pRet;} // No memory leack???

    return NULL;
}


mxArray* matlab::engine::MatHandleEng::newCallMATLABWithTrap(
	int         a_nlhs,       /* number of expected outputs */
	mxArray     *a_plhs[],    /* pointer array to outputs */
	int         a_nrhs,       /* number of inputs */
	mxArray     *a_prhs[],    /* pointer array to inputs */
	const char  *a_fcn_name   /* name of function to execute */)
{
#define BUFFER_SIZE1    4095
#define VAR_NAME_SZ1    31
    mxArray* pRet = NULL;
    int i, nOffset(0);
    char vcEvalStr[BUFFER_SIZE1+1];
    char vcVarName[VAR_NAME_SZ1+1];

    if(a_nlhs){nOffset = snprintf(vcEvalStr,BUFFER_SIZE1,"[");}
    for(i=0;i<a_nlhs-1;++i){nOffset += snprintf(vcEvalStr+nOffset,BUFFER_SIZE1-nOffset,"out__%d,",i);}
    if(a_nlhs){nOffset += snprintf(vcEvalStr+nOffset,BUFFER_SIZE1-nOffset,"out_%d] = ",a_nlhs-1);}
    nOffset += snprintf(vcEvalStr+nOffset,BUFFER_SIZE1-nOffset,"%s",a_fcn_name);
    if(a_nrhs){nOffset += snprintf(vcEvalStr+nOffset,BUFFER_SIZE1-nOffset,"(");}
    for(i=0;i<a_nrhs-1;++i){
        nOffset += snprintf(vcEvalStr+nOffset,BUFFER_SIZE1-nOffset,"in__%d,",i);
        snprintf(vcVarName,VAR_NAME_SZ1,"in__%d",i);
        engPutVariable(m_pEngine,vcVarName,a_prhs[i]);
    }
    if(a_nrhs){
        nOffset += snprintf(vcEvalStr+nOffset,BUFFER_SIZE1-nOffset,"in__%d);",a_nrhs-1);
        snprintf(vcVarName,VAR_NAME_SZ1,"in__%d",a_nrhs-1);
        engPutVariable(m_pEngine,vcVarName,a_prhs[a_nrhs-1]);
    }

    pRet = this->newEvalStringWithTrap(vcEvalStr);

    if(!pRet)
    {
        //
        for(i=0;i<a_nlhs;++i){
            snprintf(vcVarName,VAR_NAME_SZ1,"out_%d",i);
            a_plhs[i] = engGetVariable(m_pEngine,vcVarName);
        }
    }

    return pRet;
}


void matlab::engine::MatHandleEng::MatlabThreadFunction()
{
	const char* cpcCommandToRun;
	std::string strEngineCommand;

    if(m_pEngine){return;}

    m_nRun = 0;

	if (m_strEngineCommand == ""){
#ifdef WIN32
		cpcCommandToRun = NULL;
#else
		strEngineCommand = std::string("/export/doocs/bin/matlab_R2016a")+" -nodesktop"+" -nodisplay";
		cpcCommandToRun = strEngineCommand.c_str();
#endif
	}
	else {
		strEngineCommand = m_strEngineCommand + " -nodesktop" + " -nodisplay";
		cpcCommandToRun = strEngineCommand.c_str();
	}

	m_pEngine = engOpen(cpcCommandToRun);

    if(!m_pEngine){m_nReturn=START_RET::ENG_ERROR;return;}
    m_nRun=1;
    m_nReturn=START_RET::STARTED;

    //if(m_pEngine) {engGetVisible(m_pEngine,&bEngVisible) ;}
    //printf("nVisible=%d\n",(int)bEngVisible);

    while(m_nRun){

        m_semaMat.wait();
        HandleAllJobs();

    }//while(m_pEngine){

    engClose(m_pEngine);
    m_pEngine = NULL;
}


void matlab::engine::MatHandleEng::SyncCallOnMatlabThread(void* a_owner, TypeClbK a_fpClb, void* a_arg)
{
    if(m_pEngine){
        SLsnCallbackItem newItem;
        newItem.owner = a_owner;
        newItem.clbk = a_fpClb;
        newItem.arg = a_arg;
        newItem.isSync = 1;
        m_fifoJobs.AddElement(&newItem);
        m_semaMat.post();
        newItem.sema.wait();
    }
}


void matlab::engine::MatHandleEng::AsyncCallOnMatlabThread(
        void* a_owner, TypeClbK a_fpClb, void* a_arg)
{
    if(m_pEngine){
        SLsnCallbackItem* pNewItem = new SLsnCallbackItem;
        pNewItem->owner = a_owner;
        pNewItem->clbk = a_fpClb;
        pNewItem->arg = a_arg;
        pNewItem->isSync = 0;
        m_fifoJobs.AddElement(pNewItem);
        m_semaMat.post();
    }
}

void matlab::engine::MatHandleEng::HandleAllJobs(void)
{
    SLsnCallbackItem*	pClbkItem;

    while (m_fifoJobs.Extract(&pClbkItem))
    {
        (*pClbkItem->clbk)(pClbkItem->owner, pClbkItem->arg);
        if(pClbkItem->isSync){pClbkItem->sema.post();}
        else {delete pClbkItem;}
    }

}
