
/*
 *	File: mex_multiengine.cpp
 *
 *	Created on: 12 Apr, 2017
 *	Author: Davit Kalantaryan (Email: davit.kalantaryan@desy.de)
 *
 *
 */

 //#define _CREATE_PIPE_INSIDE_

#include <mex.h>
#include "multi_engine.hpp"
#include <common/lists.hpp>
#include <common/common_hashtbl.hpp>
#include <stdlib.h>
#include <string.h>
#include <vector>

#if 0

#define		OUT_NAME		"out__"
#define		IN_NAME			"in__"
#define		OUT_NAME_BS		OUT_NAME "byteStream__"

static int s_nInited = 0;
static int s_nRun = 0;
static ::STDN::thread  s_engineThread;
static ::common::UnnamedSemaphoreLite	s_sema;
static ::common::UnnamedSemaphoreLite	s_semaForGui;

//static mxArray*	s_pArrayOut = NULL;
static int s_nOutputExists = 0;
static size_t	s_unMaxOutSize = 0;
static size_t	s_unOutSize = 0;
static uint8_t*	s_pBuffer = NULL;


static const mxArray*	s_pArrayIn = NULL;
static ::Engine*	s_pEngine = NULL;

static void EngineThread();
static void AtExitFunction();

void mexFunction(int a_nNumOuts, mxArray *a_Outputs[],
	int a_nNumInps, const mxArray*a_Inputs[])
{
	if(!s_nInited){
		s_nRun = 1;
		s_engineThread = ::STDN::thread(EngineThread);
		mexAtExit(AtExitFunction);
		s_nInited = 1;
	}

	if(s_nOutputExists){

		if(s_unOutSize>0){
			mxArray* pOutInit;
			mxArray* pByteStream = mxCreateNumericMatrix(1, s_unOutSize, mxUINT8_CLASS, mxREAL);

			memcpy(mxGetData(pByteStream), s_pBuffer, s_unOutSize);
			if(!mexCallMATLABWithTrap(1, &pOutInit, 1, &pByteStream, "getArrayFromByteStream")){
				a_Outputs[0]=mxDuplicateArray(pOutInit);
				mxDestroyArray(pOutInit);
			}
			else{
				a_Outputs[0]=mxCreateNumericMatrix(1,0,mxUINT8_CLASS,mxREAL);
			}

			mxDestroyArray(pByteStream);
		}
		else{
			a_Outputs[0] = mxCreateNumericMatrix(1, 0, mxUINT8_CLASS, mxREAL);
		}
		s_nOutputExists = 0;
	}
	else{
		if((a_nNumInps<1)||(!mxIsNumeric(a_Inputs[0]))){
			mexPrintf("Provide value;\n");
		}
		s_pArrayIn = a_Inputs[0];
		s_sema.post();
		s_semaForGui.wait();
		s_pArrayIn = NULL;
		s_nOutputExists = 1;
	}
}


static void EngineThread()
{
	uint8_t* pcTmpBuffer;
	mxArray* pOut;
	s_pEngine=engOpen(NULL);

	if (!s_pEngine) { return; }
	
	while(s_nRun){
		s_sema.wait();
		if((!s_pArrayIn)||(!s_nRun)){goto semaPostPoint;}
		engPutVariable(s_pEngine,IN_NAME,s_pArrayIn);
		engEvalString(s_pEngine, OUT_NAME "=sin(" IN_NAME ");");
		engEvalString(s_pEngine, OUT_NAME_BS "=getByteStreamFromArray(" OUT_NAME ");");
		pOut = engGetVariable(s_pEngine, OUT_NAME_BS);
		if(pOut){
			s_unOutSize = mxGetNumberOfElements(pOut);
		}
		else {s_unOutSize=0;}

		if(s_unOutSize>0){
			if(s_unOutSize>s_unMaxOutSize){
				pcTmpBuffer = (uint8_t*)realloc(s_pBuffer,s_unOutSize);
				if(!pcTmpBuffer){goto semaPostPoint;}
				s_unMaxOutSize = s_unOutSize;
				s_pBuffer= pcTmpBuffer;
			}
			memcpy(s_pBuffer, mxGetData(pOut), s_unOutSize);
		}
	
	semaPostPoint:
		if(s_nRun){s_semaForGui.post();}
	}

	engClose(s_pEngine);
	s_pEngine = NULL;
}


static void AtExitFunction()
{
	s_nRun = 0;
	s_sema.post();
	s_engineThread.join();
}



#else

static int										s_nNextEngineNumber = 0;
static int										s_nIsMexLocked = 0;
static int										s_nTaskNumber = 0;
static int										s_nInited = 0;
static ::common::HashTbl<::multi::CEngine*>		s_hashEngines;
static ::common::List<::multi::CEngine*>		s_listEngines;
static ::std::vector<::multi::CEngine*>			s_vectorOfLastEngines;

static void AtExitFunction(void);
static void PrintHelpForMex(void);
static double GetNumericData(const mxArray* a_numeric);
static void RunTasksOnTheEngines(int a_nNumberOfEnginesToUse, const char* a_functionName, int a_nNumOuts, mxArray *a_Outputs[],int a_nNumInps, const mxArray*a_Inputs[]);


void mexFunction(int a_nNumOuts, mxArray *a_Outputs[],
	int a_nNumInps, const mxArray*a_Inputs[])
{
	if(!s_nInited){
		mexPrintf("multiengine version 1.0.0\n");
		mexLock();
		s_nIsMexLocked = 1;
		mexAtExit(&AtExitFunction);
		s_nInited = 1;
	}

	if(a_nNumInps <1){
		mexPrintf("Numbor of arguments should be  least one!\n");
		PrintHelpForMex();
		return;
	}


	if(mxGetClassID(a_Inputs[0])== mxFUNCTION_CLASS){
		const mxArray* pInput1;
		int nNumberOfEnginesToUse;
		// first check number of arguments
		if (a_nNumInps < 2) {
			mexPrintf("With function name the number of engines also should be provided!\n");
			PrintHelpForMex();
			return;
		}  // if (a_nNumOuts < 2) {
		nNumberOfEnginesToUse = (int)GetNumericData(a_Inputs[1]);
		pInput1 = a_Inputs[1];
		a_Inputs[1] = a_Inputs[0];
		RunTasksOnTheEngines(nNumberOfEnginesToUse, "feval", a_nNumOuts, a_Outputs,a_nNumInps-1, a_Inputs + 1);
		a_Inputs[1] = pInput1;
	}
	else if(mxIsChar(a_Inputs[0])){
		char* pcOptionOrFunctionName;
		size_t	unStringLengthPlus1 = mxGetNumberOfElements(a_Inputs[0])+1;

		pcOptionOrFunctionName = (char*)alloca(unStringLengthPlus1 + 1);
		mxGetString(a_Inputs[0], pcOptionOrFunctionName, unStringLengthPlus1);

		if(strcmp(pcOptionOrFunctionName,"--unlock")==0){
			if(s_nIsMexLocked){
				mexUnlock();
				s_nIsMexLocked = 0;
			}
		}
		else if (strcmp(pcOptionOrFunctionName, "--lock") == 0) {
			if (!s_nIsMexLocked) {
				mexLock();
				s_nIsMexLocked = 1;
			}
		}
		else if (strcmp(pcOptionOrFunctionName, "--help") == 0) {
			PrintHelpForMex();
		}
		else if (strcmp(pcOptionOrFunctionName, "--output-from-engine") == 0) {
			::multi::CEngine* pEngine;
			const ::multi::CEngine::EngineTask* pTask;
			int nEngineNumber;
			if (a_nNumInps < 2) {
				mexPrintf("Engine number to retrive output is not provided!\n");
				PrintHelpForMex();
				return;
			}  // if (a_nNumOuts < 2) {

			nEngineNumber = (int)GetNumericData(a_Inputs[1]);
			if ((nEngineNumber<0)||(!s_hashEngines.FindEntry(&nEngineNumber, 4, &pEngine))){return;}
			pTask = pEngine->getFirstReadyTask();
			if (!pTask) {return;}
			pTask->GetOutputs(a_nNumOuts, a_Outputs);
		}
		else if ((strcmp(pcOptionOrFunctionName, "--set-number-of-engines") == 0)||(strcmp(pcOptionOrFunctionName, "-sen") == 0)) {
			const int cnCurrentNumber = s_listEngines.count();
			int i, nEngineCounts, nEngineNumber;
			::multi::CEngine* pEngine;

			if (a_nNumInps < 2) {
				mexPrintf("Number of engines to set is not provided!\n");
				PrintHelpForMex();
				return;
			}  // if (a_nNumOuts < 2) {

			nEngineCounts = (int)GetNumericData(a_Inputs[1]);
			for(i= nEngineCounts;i<cnCurrentNumber;++i){
				nEngineNumber = s_listEngines.first()->data->engineNumber();
				s_hashEngines.RemoveEntry(&nEngineNumber, 4);
				delete s_listEngines.first()->data;
				s_listEngines.RemoveData(s_listEngines.first());
			}

			for(i= cnCurrentNumber;i< nEngineCounts;++i){
				nEngineNumber = s_nNextEngineNumber++;
				pEngine = new ::multi::CEngine(nEngineNumber);
				s_hashEngines.AddEntry(&nEngineNumber, 4, pEngine);
				s_listEngines.AddData(pEngine);
			}

			mexPrintf("Number of Engines changed from %d to %d\n", cnCurrentNumber, nEngineCounts);
		}
		else {  // we have function provided for running
			int nNumberOfEnginesToUse;
			// first check number of arguments
			if (a_nNumInps < 2) {
				mexPrintf("With function name the number of engines also should be provided!\n");
				PrintHelpForMex();
				return;
			}  // if (a_nNumOuts < 2) {
			nNumberOfEnginesToUse = (int)GetNumericData(a_Inputs[1]);
			RunTasksOnTheEngines(nNumberOfEnginesToUse,pcOptionOrFunctionName,a_nNumOuts, a_Outputs,a_nNumInps-2,a_Inputs+2);
		}
	}  // if(mxIsChar(a_Inputs[0])){
}



static void PrintHelpForMex(void)
{
	mexPrintf(
		"Mex file to runnning tasks in parallel threads\n"
		"Following are the options\n"
		"\t1. --unlock    -> For unlocking this MEX file (initially this MEX file is locked)\n"
		"\t2. --lock      -> For locking this MEX file if it is unlocked\n" 
		"\t3. --set-number-of-engines (-sen) -> Sets number of engines (ex. -sen 10)\n");
}


static void AtExitFunction(void)
{
	int nEngineNumber;

	if (s_nInited == 0) {return;}

	::multi::CEngine* pEngine;
	while(s_listEngines.first()){
		pEngine = s_listEngines.first()->data;
		nEngineNumber = pEngine->engineNumber();
		s_hashEngines.RemoveEntry(&nEngineNumber, 4);
		s_listEngines.RemoveData(s_listEngines.first());
		delete pEngine;
	}

	s_nInited = 0;
}


static void RunTasksOnTheEngines(int a_nNumberOfEnginesToUse,const char* a_functionName, int a_nNumOuts, mxArray *a_Outputs[], int a_nNumInps, const mxArray*a_Inputs[])
{
	static ::common::UnnamedSemaphoreLite	sSemaForGui;
	const int cnCurrentNumber = s_listEngines.count();
	int i;
	int nEngineNumber;
	int nTaskNumber = s_nTaskNumber++;
	::common::listN::ListItem< ::multi::CEngine* >* pItem;
	::multi::CEngine*  pEngine;
	bool bDoneForAll;

	if(a_nNumberOfEnginesToUse<1){
		mexPrintf("Wrong number of engines! (%d\n)", a_nNumberOfEnginesToUse);
		return;
	}

	if (a_nNumberOfEnginesToUse > cnCurrentNumber) {
		::multi::CEngine* pEngine;
		for (i = cnCurrentNumber; i < a_nNumberOfEnginesToUse; ++i) {
			nEngineNumber = s_nNextEngineNumber++;
			pEngine = new ::multi::CEngine(nEngineNumber);
			s_hashEngines.AddEntry(&nEngineNumber, 4, pEngine);
			s_listEngines.AddData(pEngine);
		}

		mexPrintf("Number of Engines changed from %d to %d\n", cnCurrentNumber, a_nNumberOfEnginesToUse);
	}

	s_vectorOfLastEngines.resize(a_nNumberOfEnginesToUse);

	for (i = 0, pItem=s_listEngines.first(); (i < a_nNumberOfEnginesToUse)&& pItem; pItem = pItem->next) {
		pEngine = pItem->data;
		if(pEngine->addFunctionIfFree2(nTaskNumber, a_functionName, a_nNumOuts,a_nNumInps,a_Inputs,&sSemaForGui,i)){
			s_vectorOfLastEngines[i++]=pEngine;
		}  // if(pEngine->addFunctionIfFree(nTaskNumber, a_functionName, a_nNumOuts,a_nNumInps,a_Inputs,&sSemaForGui)){
	}  // for (i = 0, pItem=s_listEngines.first(); (i < a_nNumberOfEnginesToUse)&& pItem; pItem = pItem->next) {


	if(i<a_nNumberOfEnginesToUse){
		mexPrintf("Number of Engines will be changed from %d to %d\n", cnCurrentNumber, cnCurrentNumber+(a_nNumberOfEnginesToUse-i));
		nEngineNumber = s_nNextEngineNumber++;
		pEngine = new ::multi::CEngine(nEngineNumber);
		s_hashEngines.AddEntry(&nEngineNumber, 4, pEngine);
		s_listEngines.AddData(pEngine);
		pEngine->addFunction2(nTaskNumber, a_functionName, a_nNumOuts, a_nNumInps, a_Inputs, &sSemaForGui,i);
		s_vectorOfLastEngines[i++] = pEngine;
	}


	for (i = 0; i < a_nNumberOfEnginesToUse; ++i) {
		s_vectorOfLastEngines[i]->StartCalc();
	}  // for (i = 0, pItem=s_listEngines.first(); (i < a_nNumberOfEnginesToUse)&& pItem; pItem = pItem->next) {

	
	sSemaForGui.wait();
	while(1){
		bDoneForAll = true;
		for (i = 0; i < a_nNumberOfEnginesToUse;++i) {
			if(s_vectorOfLastEngines[i]->isRunning()){bDoneForAll=false;break;}
		}
		if(bDoneForAll){break;}
		sSemaForGui.wait(2);
	}

	
	for(i=0;i< a_nNumOuts;++i){
		a_Outputs[i]=mxCreateNumericArray(0,0,mxINT8_CLASS,mxREAL);
	}
}


#endif


static double GetNumericData(const mxArray* a_numeric)
{
    mxClassID classId=mxGetClassID(a_numeric);

    switch(classId)
    {
    case mxDOUBLE_CLASS:
        return *mxGetPr(a_numeric);
    case mxSINGLE_CLASS:
        return (double)(*((float*)mxGetData( a_numeric )));
        break;
    case mxINT8_CLASS:
        return (double)(*((int8_t*)mxGetData( a_numeric )));
        break;
    case mxUINT8_CLASS:
        return (double)(*((uint8_t*)mxGetData( a_numeric )));
        break;
    case mxINT16_CLASS:
        return (double)(*((int16_t*)mxGetData( a_numeric )));
        break;
    case mxUINT16_CLASS:
        return (double)(*((uint16_t*)mxGetData( a_numeric )));
        break;

    case mxINT32_CLASS:
        return (double)(*((int32_t*)mxGetData( a_numeric )));
        break;
    case mxUINT32_CLASS:
        return (double)(*((uint32_t*)mxGetData( a_numeric )));
        break;
    case mxINT64_CLASS:
        return (double)(*((int64_t*)mxGetData( a_numeric )));
        break;
    case mxUINT64_CLASS:
        return (double)(*((uint64_t*)mxGetData( a_numeric )));
        break;
    default:
        break;
    }

    return 0;
}
