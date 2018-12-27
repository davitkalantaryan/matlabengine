
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
#include "multi_taskscheduler.hpp"
#include <common/lists.hpp>
#include <common/common_hashtbl.hpp>
#include <stdlib.h>
#include <string.h>
#include <vector>

static multi::TaskScheduler*					s_pScheduler = NULL;
static int										s_nIsMexLocked = 0;
static int										s_nInited = 0;

static void AtExitFunction(void);
static void PrintHelpForMex(void);
static double GetNumericData(const mxArray* a_numeric);
static void RunTasksOnTheEngines(int a_nNumberOfEnginesToUse, const char* a_functionName, int a_nNumOuts, mxArray *a_Outputs[],int a_nNumInps, const mxArray*a_Inputs[]);


void mexFunction(int a_nNumOuts, mxArray *a_Outputs[],
	int a_nNumInps, const mxArray*a_Inputs[])
{
	if(!s_nInited){
		mexPrintf("multiengine version 2.0.0\n");
		s_pScheduler = new multi::TaskScheduler;
		if(!s_pScheduler){return;}
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
		else if (strcmp(pcOptionOrFunctionName, "--output-of-task") == 0) {
		}
		else if ((strcmp(pcOptionOrFunctionName, "--set-number-of-engines") == 0)||(strcmp(pcOptionOrFunctionName, "-sen") == 0)) {
			const int cnCurrentNumber(s_pScheduler->numberOfEngines());
			int nEngineCounts;

			if (a_nNumInps < 2) {
				mexPrintf("Number of engines to set is not provided!\n");
				PrintHelpForMex();
				return;
			}  // if (a_nNumOuts < 2) {

			nEngineCounts = (int)GetNumericData(a_Inputs[1]);

			if(nEngineCounts<1){
				mexPrintf("ERROR: Number of Engines are %d\n", nEngineCounts);
				return;
			}
			else if (cnCurrentNumber < nEngineCounts) {
				mexPrintf("Number of Engines changed from %d to %d\n", cnCurrentNumber, nEngineCounts);
				s_pScheduler->setNumberOfEngines(nEngineCounts);
			}
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
		"\t3. --set-number-of-engines (-sen) -> Sets number of engines (ex. -sen 10)\n" 
		"\t4. --output-of-task\n");
}


static void AtExitFunction(void)
{
	if (s_nInited == 0) {return;}
	delete s_pScheduler;
	s_pScheduler = NULL;
	s_nInited = 0;
}


static void RunTasksOnTheEngines(int a_nNumberOfEnginesToUse,const char* a_functionName, int a_nNumOuts, mxArray *a_Outputs[], int a_nNumInps, const mxArray*a_Inputs[])
{
	mxArray* pOut;
	multi::Task* pTask;
	int i,j;
	const int cnCurrentNumber(s_pScheduler->numberOfEngines());

	if(cnCurrentNumber<a_nNumberOfEnginesToUse){
		mexPrintf("Number of Engines changed from %d to %d\n", cnCurrentNumber, a_nNumberOfEnginesToUse);
		s_pScheduler->setNumberOfEngines(a_nNumberOfEnginesToUse);
	}
	else if(a_nNumberOfEnginesToUse<1){
		mexPrintf("ERROR: Number of Engines are %d\n", a_nNumberOfEnginesToUse);
		return;
	}

	pTask=s_pScheduler->calcAndWait(a_nNumberOfEnginesToUse, a_functionName, a_nNumOuts, a_nNumInps, a_Inputs);

	if(pTask->isError()){
		for (j = 0; j < a_nNumberOfEnginesToUse; ++j) {
			if(pTask->isSubtaskInError(j)){
				pTask->GetOutput(j, 0, &pOut);
				//if(pOut){mexCallMATLABWithTrap(0,NULL, 1, &pOut, "rethrow");}
				if(pOut){mexCallMATLAB(0,NULL, 1, &pOut, "rethrow");}
				return;
			}
			
		}  // for (j = 0; j < a_nNumberOfEnginesToUse; ++j) {
	}

	for(i=0;i<a_nNumOuts;++i){
		a_Outputs[i]=mxCreateCellMatrix(1,a_nNumberOfEnginesToUse);
		for (j = 0; j < a_nNumberOfEnginesToUse; ++j) {
			pTask->GetOutput(j, i, &pOut);
			if(pOut){
				mxSetCell(a_Outputs[i],j,pOut);
			}
		}  // for (j = 0; j < a_nNumberOfEnginesToUse; ++j) {
	}
}



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
