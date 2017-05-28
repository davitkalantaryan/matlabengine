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


int matlab::engine::ClientTcp::ConnectToServer(const char* a_serverName, int a_nEngineNumber)
{
	char cResp;
	int nRet(-1);
	int nPort(GenerateMatlabServerPortNumber2(a_nEngineNumber));

	if (m_socket >= 0) { return 0; }

	if (nPort < 0) { return nPort; }
	nRet = ASocketTCP::CreateClient(a_serverName, nPort, 1000);
	if (nRet == 0)
	{
		nRet = ASocketTCP::RecvData(&cResp, 1, 10000, 1000);
		if ((nRet == 1) && (cResp == 'r')) { nRet = 0; }
	}//if (nRet == 0)

	return nRet;
}


int matlab::engine::ClientTcp::Recv(void* a_buffer, int a_nBufSize, long a_lnTimeoutMS)
{
	long lnClientSmallTiomeout = a_lnTimeoutMS < 0 ? SMALL_TIMEOUT_MS : a_lnTimeoutMS;
	return ASocketTCP::RecvData(a_buffer, a_nBufSize, a_lnTimeoutMS, lnClientSmallTiomeout);
}


int matlab::engine::ClientTcp::Send(const void* a_buffer, int a_nBufSize)
{
	return  ASocketTCP::SendData(a_buffer, a_nBufSize);
}


int	matlab::engine::ClientTcp::selectMltC(
	long a_lnTimeoutMs,int* a_pnMaxPlus1, int a_nNumberOfSockets,
	const int* a_pnSocketsIn, int* a_pnReadOut, int* a_pnWriteOut, int* a_pnExcpt)
{
	struct timeval*pTimeout;
	struct timeval aTimeout;
	fd_set rfds, wfds, efds;
	int nSelectReturn, i(0);

	if ((*a_pnMaxPlus1) < 0){
		for (; i < a_nNumberOfSockets; ++i)
		{
			if ((a_pnSocketsIn[i] + 1)>(*a_pnMaxPlus1)) {*a_pnMaxPlus1=(a_pnSocketsIn[i] + 1);}
		}
		i = 0;
	}

	FD_ZERO(&rfds);FD_ZERO(&wfds);FD_ZERO(&efds);

	for (; i < a_nNumberOfSockets; ++i)
	{
		FD_SET((unsigned int)a_pnSocketsIn[i], &rfds);
		FD_SET((unsigned int)a_pnSocketsIn[i], &wfds);
		FD_SET((unsigned int)a_pnSocketsIn[i], &efds);
	}


	if (a_lnTimeoutMs >= 0){
		aTimeout.tv_sec = a_lnTimeoutMs / 1000;
		aTimeout.tv_usec = (a_lnTimeoutMs % 1000) * 1000;
		pTimeout = &aTimeout;
	}
	else{pTimeout = NULL;}


	nSelectReturn = select((*a_pnMaxPlus1), &rfds, &wfds, &efds, pTimeout);

	switch (nSelectReturn)
	{
	case 0:	/* time out */
		return _SOCKET_TIMEOUT_;
#ifdef	WIN32
	case SOCKET_ERROR:
#else
	case -1:
#endif
		if (errno == EINTR)
		{
			/* interrupted by signal */
			return _EINTR_ERROR_;
		}

		return E_SELECT;
	default:
		if((nSelectReturn<0) || (nSelectReturn>a_nNumberOfSockets)){return E_FATAL;}
		break;
	}

	for (i=0; i < a_nNumberOfSockets; ++i)
	{
		if(a_pnReadOut){
			if(FD_ISSET(a_pnSocketsIn[i], &rfds)){ a_pnReadOut[i]=1;}
			else {a_pnReadOut[i]=0;}
		}
		if (a_pnWriteOut) {
			if (FD_ISSET(a_pnSocketsIn[i], &wfds)) { a_pnWriteOut[i] = 1; }
			else { a_pnWriteOut[i] = 0; }
		}
		if (a_pnReadOut) {
			if (FD_ISSET(a_pnSocketsIn[i], &efds)) { a_pnExcpt[i] = 1; }
			else { a_pnExcpt[i] = 0; }
		}
	}

	return nSelectReturn;
}



int	matlab::engine::ClientTcp::selectMltCpp(
	long a_lnTimeoutMs, int* a_pnMaxPlus1, 
	const std::vector<int>& a_rdfd, const std::vector<int>& a_wrfd, const std::vector<int>& a_erfd,
	std::vector<char>& a_return)
{
	struct timeval*pTimeout;
	struct timeval aTimeout;
	fd_set rfds, wfds, efds;
	size_t unNsoks, i, unMaxSize;
	int nSelectReturn;
	char cReturn;

	if ((*a_pnMaxPlus1) < 0) {
		unNsoks = a_rdfd.size();
		for (i=0;i<unNsoks;++i){if((a_rdfd[i]+1)>(*a_pnMaxPlus1)) {*a_pnMaxPlus1=(a_rdfd[i]+1);}}
		unNsoks = a_wrfd.size();
		for (i=0;i<unNsoks;++i){if((a_wrfd[i]+1)>(*a_pnMaxPlus1)) {*a_pnMaxPlus1=(a_wrfd[i]+1);}}
		unNsoks = a_erfd.size();
		for (i=0;i<unNsoks;++i){if((a_erfd[i]+1)>(*a_pnMaxPlus1)) {*a_pnMaxPlus1=(a_erfd[i]+1);}}
	}

	FD_ZERO(&rfds); FD_ZERO(&wfds); FD_ZERO(&efds);

	unMaxSize = unNsoks = a_rdfd.size();
	for (i=0;i<unNsoks;++i){FD_SET((unsigned int)a_rdfd[i],&rfds);}
	unNsoks = a_wrfd.size(); unMaxSize=(unNsoks>unMaxSize)? unNsoks:unMaxSize;
	for (i=0;i<unNsoks;++i){FD_SET((unsigned int)a_wrfd[i],&wfds);}
	unNsoks = a_erfd.size();unMaxSize=(unNsoks>unMaxSize)? unNsoks:unMaxSize;
	for (i=0;i<unNsoks;++i){FD_SET((unsigned int)a_erfd[i],&efds);}

	if (a_lnTimeoutMs >= 0) {
		aTimeout.tv_sec = a_lnTimeoutMs / 1000;
		aTimeout.tv_usec = (a_lnTimeoutMs % 1000) * 1000;
		pTimeout = &aTimeout;
	}
	else { pTimeout = NULL; }

	nSelectReturn = select((*a_pnMaxPlus1), &rfds, &wfds, &efds, pTimeout);
	//nSelectReturn = select((*a_pnMaxPlus1),NULL,NULL, &efds, pTimeout);
	if ((nSelectReturn<=0) || (nSelectReturn>((int)unMaxSize))) { return nSelectReturn; }

	if(a_return.size()<unMaxSize){a_return.resize(unMaxSize,0);}

	unNsoks = a_rdfd.size();
	for (i=0;i<unNsoks;++i){
		cReturn=0;
		if (FD_ISSET(a_rdfd[i], &rfds)) { SET_BIT(&cReturn,READ1_BIT,1); }
		else {SET_BIT(&cReturn,READ1_BIT,0); }
		a_return[i] = cReturn;
	}
	unNsoks = a_wrfd.size();
	for (i=0;i<unNsoks;++i){
		cReturn=a_return[i];
		if (FD_ISSET(a_wrfd[i], &wfds)) { SET_BIT(&cReturn,WRITE_BIT,1); }
		else {SET_BIT(&cReturn, WRITE_BIT,0); }
		a_return[i] = cReturn;
	}
	unNsoks = a_erfd.size();
	for (i=0;i<unNsoks;++i){
		cReturn=a_return[i];
		if (FD_ISSET(a_erfd[i], &efds)) { SET_BIT(&cReturn,ERROR_BIT,1); }
		else {SET_BIT(&cReturn, ERROR_BIT,0); }
		a_return[i] = cReturn;
	}

	return nSelectReturn;
}


void matlab::engine::ClientTcp::CloseSt(int a_socket)
{
	if (a_socket > 0)
#ifdef	WIN32
		closesocket(a_socket);
#else
		close(a_socket);
#endif
	a_socket = -1;
}
