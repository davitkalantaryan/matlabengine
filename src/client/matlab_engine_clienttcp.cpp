/*
 *	File: matlab_engine_clienttcp.cpp
 *
 *	Created on: 14 Apr, 2017
 *	Author    : Davit Kalantaryan (Email: davit.kalantaryan@desy.de)
 *
 *  This file implements following class ...
 *
 */

#include "matlab_engine_clienttcp.hpp"
#include "matlab_pipe_name.h"

#ifndef SMALL_TIMEOUT_MS
#define	SMALL_TIMEOUT_MS		20000
#endif


int matlab::engine::ClientTcp::ConnectToServer(const char* a_serverName, int a_nPid)
{
	char cResp;
	int nRet(-1);
	for (int nTry(0);nTry < MAX_TRY_NUMBERS;++nTry)
	{
		nRet = ASocketTCP::CreateClient(a_serverName, GenerateMatlabServerPortNumber(a_nPid, nTry), 1000);
		if (nRet == 0)
		{
			nRet = ASocketTCP::RecvData(&cResp, 1, 10000, 1000);
			if ((nRet == 1) && (cResp == 'r')) { nRet = 0; break; }
		}//if (nRet == 0)
	}

	return nRet;
}


int matlab::engine::ClientTcp::RecvData(void* a_buffer, int a_nBufSize, long a_lnTimeoutMS)
{
	long lnClientSmallTiomeout = a_lnTimeoutMS < 0 ? SMALL_TIMEOUT_MS : a_lnTimeoutMS;
	return ASocketTCP::RecvData(a_buffer, a_nBufSize, a_lnTimeoutMS, lnClientSmallTiomeout);
}