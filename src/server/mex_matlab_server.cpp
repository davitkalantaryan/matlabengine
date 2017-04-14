/*
 *	File: mex_matlab_pipe_server.cpp
 *
 *	Created on: 12 Apr, 2017
 *	Author: Davit Kalantaryan (Email: davit.kalantaryan@desy.de)
 *
 *
 */

//#define _CREATE_PIPE_INSIDE_

#include <mex.h>
#include <thread>
#include <process.h>
#include <listener_on_gui_thread.hpp>
#include "namedpipeserver.h"
#include "matlab_pipe_name.h"
#include <mutex>
#include "matlab_engine_serializer.hpp"

#pragma warning(disable : 4996)

static void CleanUpMexFile(void);
static void PrintHelp(void);
static void PipeServerThreadFunction(void);
static void OutputHandlerFunction(void);
static void* make_diary_function_call(void* pIsOn);

static matlab::engine::Serializer	s_serializeDes;
//static int s_nMatlabScriptNameLength = 0;
//static char*	s_pcMatlabScriptName = NULL;
static std::mutex	s_mutexForBuffers;
static volatile int s_nRun = 0;
static int s_nDebug = 1;
static std::thread* s_pPipeServerThread = NULL;
static void*	s_pListener = NULL;
static NamedPipeServer s_PipeServer;
//static int s_nArrayLength = 0;

static std::thread* s_pOutputHandlerThread = NULL;
static NamedPipeServer s_OutputPipeServer;
static int s_nIsDiaryInited = 0;
static int s_nIsDiaryOn;

void mexFunction(int a_nNumOuts, mxArray *a_Outputs[],
	int a_nNumInps, const mxArray*a_Inputs[])
{
	static int snAtExitRegistered = 0;
	int nError(0);

	mexPrintf("Version=15, pid=%d\n",(int)_getpid());

	if (a_nNumInps   && mxIsChar(a_Inputs[0]))
	{
		int nOptionLen = (int)(mxGetM(a_Inputs[0]) * mxGetN(a_Inputs[0])) + 15;
		char* pcOption = (char*)alloca(nOptionLen);
		if (mxGetString(a_Inputs[0], pcOption, nOptionLen)){
			mexErrMsgTxt("Unable to get the string!\n");
			return;
		}

		if (strcmp(pcOption, "debug") == 0){ s_nDebug = 1; }
		else if (strcmp(pcOption, "no-debug") == 0){ s_nDebug = 0; }
		else if (strcmp(pcOption, "lock") == 0){ if (!mexIsLocked()){ mexLock(); } }
		else if (strcmp(pcOption, "unlock") == 0){ if (mexIsLocked()){ mexUnlock(); } }
		else if (strcmp(pcOption, "diary-on") == 0){ make_diary_function_call((void*)1); }
		else if (strcmp(pcOption, "diary-off") == 0){ make_diary_function_call(NULL); }
		else if (strcmp(pcOption, "help") == 0){ PrintHelp(); }

		if (a_nNumOuts){ a_Outputs[0] = mxCreateString(""); }
		return;
	}

	if (!snAtExitRegistered)
	{
		snAtExitRegistered = 1;
		mexLock();
		mexAtExit([](){
			if (s_nDebug){ mexPrintf("matlab_pipe_server:AtExit called!\n"); }
			CleanUpMexFile();
			snAtExitRegistered = 0;
		});
	} // if (!snAtExitRegistered)

	if (!s_pPipeServerThread)
	{
		s_pListener = CreateMListener();
		if (!s_pListener)
		{
			CleanUpMexFile();
			mexErrMsgTxt("Unable to create listener for MALLAB!\n");
			if (a_nNumOuts){ a_Outputs[0] = mxCreateString(""); }
			return;
		}
		s_pPipeServerThread = new std::thread(&PipeServerThreadFunction);
		if (!s_pPipeServerThread)
		{
			CleanUpMexFile();
			mexErrMsgTxt("Low memory\n");
			if (a_nNumOuts){ a_Outputs[0] = mxCreateString(""); }
			return;
		}

#ifdef _CREATE_PIPE_INSIDE_
		s_pOutputHandlerThread = new std::thread(&OutputHandlerFunction);
		if (!s_pOutputHandlerThread)
		{
			CleanUpMexFile();
			mexErrMsgTxt("Low memory\n");
			if (a_nNumOuts){ a_Outputs[0] = mxCreateString(""); }
			return;
		}
#endif
	}  // if (!s_pPipeServerThread)


	if (nError){a_Outputs[0] = mxCreateString("Error during processing mex file!");}
	else 
	{
		char vcBuffer[32];
		_snprintf(vcBuffer, 31, "%d", (int)_getpid());
		a_Outputs[0] = mxCreateString(vcBuffer);
	}

}


#if 0
if (strcmp(pcOption, "debug") == 1){ s_nDebug = 1; }
else if (strcmp(pcOption, "no-debug") == 0){ s_nDebug = 0; }
else if (strcmp(pcOption, "lock") == 0){ if (!mexIsLocked()){ mexLock(); } }
else if (strcmp(pcOption, "unlock") == 0){ if (mexIsLocked()){ mexUnlock(); } }
else if (strcmp(pcOption, "help") == 0){ PrintHelp(); }
#endif


static void PrintHelp(void)
{
	mexPrintf("provide DOOCS address as string argument and the array \n"
		"Options are following:\n"
		"\tdebug      : makes this server more verbose\n"
		"\tno-debug   : makes this server less verbose\n"
		"\tlock       : locks the mex file for accidentally not unloading from MATLAB (by default locked)\n"
		"\tunlock     : unlocks the mex file, then 'clear all' or 'clear matlab_pipe_server'\n"
		"\t             will unload pipe server from MATLAB workspace\n"
		"\tdiary-on   : Prepares the pipe\n"
		"\tdiary-off  : Destroys the pipe\n");
}


#if 0
static volatile int s_nRun = 0;
static int s_nDebug = 0;
static std::thread* s_pPipeServerThread = NULL;
static void*	s_pListener = NULL;
static NamedPipeServer s_PipeServer;
static int s_nPipeBufferLength = 0;
static u_char*	s_pPipeBuffer = NULL;
static int s_nMatlabScriptNameLength = 0;
static char*	s_pcMatlabScriptName = NULL;
static int s_nArrayLength = 0;
static mxArray* s_prhsSlm = NULL;

static std::thread* s_pOutputHandlerThread = NULL;
static NamedPipeServer s_OutputPipeServer;
#endif


static void CleanUpMexFile(void)
{
	// 1.
	s_nRun = 0;

	// 2.
	if (s_pOutputHandlerThread)
	{
		TerminateThread((HANDLE)s_pOutputHandlerThread->native_handle(), 0);
		s_pOutputHandlerThread->join();
		delete s_pOutputHandlerThread;
		s_pOutputHandlerThread = NULL;
	}

	// 2.
	if (s_pPipeServerThread)
	{
		TerminateThread((HANDLE)s_pPipeServerThread->native_handle(),0);
		s_pPipeServerThread->join();
		delete s_pPipeServerThread;
		s_pPipeServerThread = NULL;
	}

	// 3.
	if (s_pListener)
	{
		DestroyMListener(s_pListener);
		s_pListener = NULL;
	}

	// 4.
	s_PipeServer.ClosePipe();

	s_OutputPipeServer.ClosePipe();
	
	// 5, 6.
#if 0
	if (s_nPipeBufferLength)
	{
		free(s_pPipeBuffer);
		s_pPipeBuffer = NULL;
		s_nPipeBufferLength = 0;
	}

	// 7, 8.
	if (s_nMatlabScriptNameLength)
	{
		free(s_pcMatlabScriptName);
		s_pcMatlabScriptName = NULL;
		s_nMatlabScriptNameLength = 0;
	}
#endif // #if 0

#if 0
	static int s_nArrayLength = 0;
	static mxArray* s_prhsSlm = NULL;

	if (s_nArrayLength)
	{
		mxDestroyArray(s_prhsSlm);
		s_nArrayLength = 0;
	}
#endif
	mexPrintf("matlab_pipe_server destroyed!\n");
}


#define CHECK_RETURN(__return) do{}while(0)

static void* PrintfFromRmThread(void* a_arg)
{
	const char* cpcToPrint = (const char*)a_arg;
	mexPrintf(cpcToPrint);
	mexEvalString("drawnow");
	return "";
}


static void* CallMatlabFunctionStatic(void*)
{
	int nReturn(-1);
	const char* cpcInfoOrError = "Error";
	int i;
	const u_char_ttt* pRetBuffer=NULL;
	int32_ttt  retBufLen=0;
	mxArray* pSerializedInputCell;
	mxArray* pInputCell = NULL;
	mxArray* pSerializedOutputCell = NULL;
	mxArray* pReturnFromCallMatlab = NULL;
	mxArray* vInputs[MAXIMUM_NUMBER_OF_IN_AND_OUTS];
	mxArray* vOutputs[MAXIMUM_NUMBER_OF_IN_AND_OUTS];
	void* pSerializedData;
	int nInputs;
	int nOutputs;

	int32_ttt nBSL;

	s_mutexForBuffers.lock();

	if (s_serializeDes.InOutBytesNumber() <= 0){goto returnPoint;}
	try {
		nBSL = s_serializeDes.InOutBytesNumber();
		mexPrintf("nBSL=%d\n",(int)nBSL); mexEvalString("drawnow");
		pSerializedInputCell = mxCreateNumericMatrix(1, s_serializeDes.InOutBytesNumber(), mxUINT8_CLASS, mxREAL);
		if (!pSerializedInputCell) {/*report*/goto returnPoint;}
		pSerializedData = mxGetData(pSerializedInputCell);
		memcpy(pSerializedData, s_serializeDes.InputsOrOutputs(), nBSL);

		pReturnFromCallMatlab=mexCallMATLABWithTrap(1, &pInputCell, 1, &pSerializedInputCell, "getArrayFromByteStream");
		if (pReturnFromCallMatlab) { goto exceptionPoint; }
		if (!pInputCell) {/*report*/goto returnPoint; }

		nInputs = (int)mxGetN(pInputCell);
		//nInputs *= (int)mxGetM(pInputCell);
		nInputs = nInputs > MAXIMUM_NUMBER_OF_IN_AND_OUTS ?
			MAXIMUM_NUMBER_OF_IN_AND_OUTS : nInputs;
		nOutputs = s_serializeDes.NumOfExpOutsOrError() > MAXIMUM_NUMBER_OF_IN_AND_OUTS ?
			MAXIMUM_NUMBER_OF_IN_AND_OUTS : s_serializeDes.NumOfExpOutsOrError();

		if ((nInputs == 0) && (nOutputs == 0)) // eval string is called
		{
			pReturnFromCallMatlab = mexEvalStringWithTrap(s_serializeDes.MatlabScriptName());
		}
		else
		{
			for (i = 0; i < nInputs; ++i) { vInputs[i] = mxGetCell(pInputCell, i); }
			pReturnFromCallMatlab = mexCallMATLABWithTrap(nOutputs, vOutputs, nInputs, vInputs, s_serializeDes.MatlabScriptName());
		}

		if (pReturnFromCallMatlab)
		{
		exceptionPoint:
			pSerializedOutputCell = MatlabArrayToMatlabByteStream(1, &pReturnFromCallMatlab);
			if (!pSerializedOutputCell) { goto returnPoint; }
			pRetBuffer = (const u_char_ttt*)mxGetData(pSerializedOutputCell);
			retBufLen = GetByteStreamLen(pSerializedOutputCell);
			goto returnPoint;
		}

		pSerializedOutputCell = MatlabArrayToMatlabByteStream(nOutputs, vOutputs);
		if (!pSerializedOutputCell){goto returnPoint;}
		pRetBuffer = (const u_char_ttt*)mxGetData(pSerializedOutputCell);
		retBufLen = GetByteStreamLen(pSerializedOutputCell);
		nReturn = 0;
		cpcInfoOrError = "OK";
		mexEvalString("drawnow");

	}
	catch (...)
	{
		goto returnPoint;
	}

returnPoint:
	if (pReturnFromCallMatlab)
	{
		mexPrintf(
			"Error during calling script by request from remote.\n"
			"script name is \"%s\"\n", s_serializeDes.MatlabScriptName());
	}
	s_serializeDes.SetSendParams(cpcInfoOrError,pRetBuffer, retBufLen, nReturn);
	s_PipeServer.Write(s_serializeDes.GetOverAllBufferForSend(), s_serializeDes.OverAllMinus8Byte() + 8);
	s_mutexForBuffers.unlock();
	if (pSerializedInputCell) { mxDestroyArray(pSerializedInputCell); }
	mexEvalStringWithTrap("drawnow");

	return NULL;

}

#if 0
#define		MATLAB_PIPE_CLOSE		0
#define		MATLAB_PIPE_NEW_DATA	1
#endif

static void PipeServerThreadFunction(void)
{
	matlab::engine::SDataHeader aInfo;
	bool bMoreData;
	int nReadReturn; // <0 command >=0 length

	if (s_PipeServer.CreateServer(GenerateMatlabPipeName(_getpid()), NECESSARY_BUF_SIZE, NECESSARY_BUF_SIZE))
	{
		// Make an report
		return;
	}

	s_nRun = 1;

	while (s_nRun)
	{
		if (s_PipeServer.WaitForConnection()){/*report it*/continue; }
		SynchronCallOnGUIthread(s_pListener, "Connected!\n", &PrintfFromRmThread);
		while (s_nRun)
		{
			nReadReturn = s_PipeServer.Read(&aInfo, 8, bMoreData);
			if (nReadReturn != 8){ break; }

			switch (aInfo.overallMin8)
			{
			case MATLAB_PIPE_CLOSE:
				break;
			default:  // default is calling matlab routine
				s_mutexForBuffers.lock();
				s_serializeDes.Resize(aInfo.overallMin8,aInfo.numberOfOutsOrError);
				nReadReturn = s_PipeServer.Read(s_serializeDes.GetBufferForReceive(), aInfo.overallMin8, bMoreData);
				s_mutexForBuffers.unlock();
				SynchronCallOnGUIthread(s_pListener, NULL, &CallMatlabFunctionStatic);
				break;
			}
		} // while (s_nRun)
	} // while (s_nRun)

	s_PipeServer.ClosePipe();
}




//////////////
static void* make_diary_function_call(void* a_pIsOn)
{
	char vcBuffer[512];
	int nIsOn = (int)((size_t)a_pIsOn);

	Sleep(1);

	if (!s_nIsDiaryInited)
	{
		mexPrintf("Preparing diary to the file '%s'!\n", GenerateMatlabPipeNameV(_getpid(), 2));
		_snprintf(vcBuffer, 511, "diary('%s');", GenerateMatlabPipeNameV(_getpid(), 2));
		mexEvalString(vcBuffer);
		s_nIsDiaryInited = 1;
		s_nIsDiaryOn = ~nIsOn;
	}

	if (nIsOn){ if (!s_nIsDiaryOn){ mexEvalString("diary on"); s_nIsDiaryOn = 1; } }
	else{ if (s_nIsDiaryOn){ mexEvalString("diary off"); s_nIsDiaryOn = 0; } }
	
	//mexEvalString(vcBuffer);

	return NULL;
}


static void* report_about_connection_disconnection(void* a_pIsCon)
{
	char vcBuffer[512];
	int nIsCon = (int)((size_t)a_pIsCon);

	if (nIsCon)
	{
		mexPrintf("Connected!\n");
		_snprintf(vcBuffer, 511, "diary('%s');diary on", GenerateMatlabPipeNameV(_getpid(), 2));
	}
	else
	{
		mexPrintf("Disconnected!\n");
	}
	mexEvalString("drawnow");

	return NULL;
}


static void OutputHandlerFunction(void)
{
	int nReadReturn;
	bool bIsMoreData;
	std::string aMatlabOutput;
	char cRead;
	int nConnectionOk;

	if (s_OutputPipeServer.CreateServer(GenerateMatlabPipeNameV(_getpid(),2), NECESSARY_BUF_SIZE, NECESSARY_BUF_SIZE))
	{
		// Make an report
		return;
	}

	s_nRun = 1;
	//PostJobForGUIthread(s_pListener, NULL, &make_diary_function_call);//

	while (s_nRun)
	{
		if (s_OutputPipeServer.WaitForConnection()){/*report it*/continue; }
		PostJobForGUIthread(s_pListener, (void*)1, &report_about_connection_disconnection);//
		nConnectionOk = 1;
		while (nConnectionOk)
		{
			bIsMoreData = true;
			while (bIsMoreData)
			{
				nReadReturn = s_OutputPipeServer.Read(&cRead, 1, bIsMoreData);
				if (nReadReturn != 1)
				{
					bIsMoreData = false;
					nConnectionOk = 0;
					PostJobForGUIthread(s_pListener, (void*)0, &report_about_connection_disconnection);//
					break;
				}
				aMatlabOutput.push_back(cRead);
			} // while (bIsMoreData)
			//SynchronCallOnGUIthread(s_pListener, (void*)(&aMatlabOutput), &handle_matlab_output);//PostJobForGUIthread
		}
	} // while (s_nRun)
}
