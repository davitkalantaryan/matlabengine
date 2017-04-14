/*
 *	File: matlab_pipe_name.cpp
 *
 *	Created on: 21 Dec, 2016
 *	Author: Davit Kalantaryan (Email: davit.kalantaryan@desy.de)
 *
 *
 */

#include <process.h>
#include <stdio.h>
#include "matlab_pipe_name.h"

#pragma warning(disable : 4996)

#ifdef __cplusplus
extern "C"
{
#endif


const char* GenerateMatlabPipeName(int a_nPid)
{
	static int snInited = 0;
	static char svcBuffer[256];

	if (snInited == 0)
	{
		_snprintf(svcBuffer, 255, "\\\\.\\pipe\\matlab_pipe_%d", a_nPid);
	}

	
	return svcBuffer;
}


const char* GenerateMatlabPipeNameV(int a_nPid,int a_nVersion)
{
	static int snInited = 0;
	static char svcBuffer[256];

	if (snInited == 0)
	{
		_snprintf(svcBuffer, 255, "\\\\.\\pipe\\matlab_pipe_%d_%d", a_nPid, a_nVersion);
	}


	return svcBuffer;
}


int GenerateMatlabServerPortNumber(int a_nPid, int a_nTry)
{
	int nPort = (a_nPid + a_nTry + MATLAB_BIND_PORT_NUMBER) % (0xffff);
	return nPort;
}


#ifdef __cplusplus
}
#endif
