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
#include "matlab_engine_servertcpmex.hpp"

#pragma warning(disable : 4996)

static void CleanUpMexFile(void);
static void PrintHelp(void);

static int s_nDebug = 1;
static matlab::engine::ServerTcpMex	s_serverTcp;

void mexFunction(int a_nNumOuts, mxArray *a_Outputs[],
	int a_nNumInps, const mxArray*a_Inputs[])
{
	static int snAtExitRegistered = 0;
	int nError(0);

	mexPrintf("Version=19\n");

	if (a_nNumInps   && mxIsChar(a_Inputs[0]))
	{
		int nOptionLen = (int)(mxGetM(a_Inputs[0]) * mxGetN(a_Inputs[0])) + 15;
		char* pcOption = (char*)alloca(nOptionLen);
		if (mxGetString(a_Inputs[0], pcOption, nOptionLen)){
			mexErrMsgTxt("Unable to get the string!\n");
			return;
		}

		if (strcmp(pcOption, "--debug") == 0){ s_nDebug = 1; }
		else if (strcmp(pcOption, "--no-debug") == 0){ s_nDebug = 0; }
		else if (strcmp(pcOption, "--lock") == 0){ if (!mexIsLocked()){ mexLock(); } }
		else if (strcmp(pcOption, "--unlock") == 0){ if (mexIsLocked()){ mexUnlock(); } }
		else if (strcmp(pcOption, "--diary-on") == 0){ mexPrintf("Not implemented!!\n"); }
		else if (strcmp(pcOption, "--diary-off") == 0){ mexPrintf("Not implemented!!\n"); }
		else if (strcmp(pcOption, "--help") == 0){ PrintHelp(); }

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

	if (!s_serverTcp.GetRun()){s_serverTcp.StartServer();}


	if (nError){a_Outputs[0] = mxCreateString("Error during processing mex file!");}
	else 
	{
		if (a_nNumOuts) { a_Outputs[0] = mxCreateString(""); }
	}

}


static void PrintHelp(void)
{
	mexPrintf("provide DOOCS address as string argument and the array \n"
		"Options are following:\n"
		"\t--debug      : makes this server more verbose\n"
		"\t--no-debug   : makes this server less verbose\n"
		"\t--lock       : locks the mex file for accidentally not unloading from MATLAB (by default locked)\n"
		"\t--unlock     : unlocks the mex file, then 'clear all' or 'clear matlab_pipe_server'\n"
		"\t\t             will unload pipe server from MATLAB workspace\n"
		"\t--diary-on   : Prepares the pipe\n"
		"\t--diary-off  : Destroys the pipe\n");
}


static void CleanUpMexFile(void)
{
	// 1.
	s_serverTcp.StopServer();
	mexPrintf("matlab_server destroyed!\n");
}
