/*
 *	File: matlab_engine_servertcp.cpp
 *
 *	Created on: 13 Apr, 2017
 *	Author    : Davit Kalantaryan (Email: davit.kalantaryan@desy.de)
 *
 *  This file implements following class ...
 *
 */

#include "matlab_engine_servertcpbase.hpp"
#include "matlab_pipe_name.h"

static int RecvFuncStatic(void* receiver, void* buffer, int bufSize, long timeoutMs);
static int SendFuncStatic(void* sender, const void* buffer, int bufSize);
static void CloseFuncStatic(void* senderReceiver);

matlab::engine::ServerTcpBase::ServerTcpBase(MatHandleBase* a_pMatHandle)
	:
	ServerBase(&RecvFuncStatic,&SendFuncStatic,&CloseFuncStatic, a_pMatHandle)
{
}


matlab::engine::ServerTcpBase::~ServerTcpBase()
{
}


int matlab::engine::ServerTcpBase::AddClient(class ASocketTCP& a_ClientSocket, struct sockaddr_in* bufForRemAddress)
{
	int nSocket = (int)a_ClientSocket;
	void* recvSend = (void*)((size_t)nSocket);
	
	a_ClientSocket.SendData("r", 1);
	a_ClientSocket.SetSockDescriptor(-1);
	ServerBase::AddClient(recvSend);
	return 0;
}


void matlab::engine::ServerTcpBase::StartServerPrivate(int a_nEngNumber)
{
	char vcBuffer[64];
	int nPort(GenerateMatlabServerPortNumber2(a_nEngNumber));
	
	if (nPort < 0) { m_nReturnFromServerThread = nPort; return; }
	ASocketB::Initialize();
	gethostname(vcBuffer, 63);

#ifdef _CD_VERSION__
	m_nReturnFromServerThread = AServerTCP::CreateServer(a_nPort,false, true);
#else
	m_nReturnFromServerThread = AServerTCP::CreateServer(nPort,false);
#endif

	if (m_nReturnFromServerThread != 0){
		AServerTCP::Close();
		return;
	}

	snprintf(vcBuffer, 63, "%s:%d\\n", vcBuffer, a_nEngNumber);
	m_pMatHandle->PrintFromAnyThread(vcBuffer);
	
	m_nServerRuns = 1;
	AServerTCP::RunServer(1000, &m_bufForRemAddress);
	m_nServerRuns = 0;

	ASocketB::Cleanup();
}


void matlab::engine::ServerTcpBase::StopServerPrivate(void)
{
	AServerTCP::m_nQUIT_FLAG = 1;
}


/*////////////////////////////////////////////////////////////////////////////////////////////*/

static int RecvFuncStatic(void* a_receiver, void* a_buffer, int a_bufSize, long a_timeoutMs)
{
	int nReturn;
	ASocketTCP aSocket;
	long lnClientSmallTiomeout = a_timeoutMs < 0 ? SMALL_TIMEOUT_MS : a_timeoutMs;
	
	aSocket.SetSockDescriptor((int)((size_t)a_receiver));
	nReturn = aSocket.RecvData(a_buffer, a_bufSize, a_timeoutMs, lnClientSmallTiomeout);
	aSocket.SetSockDescriptor(-1);
	return nReturn;
}


static int SendFuncStatic(void* a_sender, const void* a_buffer, int a_bufSize)
{
	int nReturn;
	ASocketTCP aSocket;
	aSocket.SetSockDescriptor((int)((size_t)a_sender));
	nReturn = aSocket.SendData(a_buffer, a_bufSize);
	aSocket.SetSockDescriptor(-1);
	return nReturn;
}


static void CloseFuncStatic(void* a_senderReceiver)
{
	ASocketTCP aSocket;
	aSocket.SetSockDescriptor((int)((size_t)a_senderReceiver));
	aSocket.Close();
}
