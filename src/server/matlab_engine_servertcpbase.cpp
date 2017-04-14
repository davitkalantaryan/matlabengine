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
	
	a_ClientSocket.SetSockDescriptor(-1);
	ServerBase::AddClient(recvSend);
	return 0;
}


int matlab::engine::ServerTcpBase::StartServerPrivate(void)
{
	int nTry,nRet(-1);
	
	// If everything is ok, then AServerTCP::StartServer will not return
	for (nTry = 0;
		ServerBase::m_nRun && (nTry < MAX_TRY_NUMBERS) &&
		(nRet = AServerTCP::StartServer(GenerateMatlabServerPortNumber(getpid(), nTry), 1000));
		++nTry);

	return (nTry < (MAX_TRY_NUMBERS - 1)) ? 0 : -1;
}


void matlab::engine::ServerTcpBase::StopServerPrivate(void)
{
	AServerTCP::m_nQUIT_FLAG = 1;
}


/*////////////////////////////////////////////////////////////////////////////////////////////*/

static int RecvFuncStatic(void* a_receiver, void* a_buffer, int a_bufSize, long a_timeoutMs)
{
	ASocketTCP aSocket;
	aSocket.SetSockDescriptor((int)((size_t)a_receiver));
	return aSocket.RecvData(a_buffer, a_bufSize, a_timeoutMs, a_timeoutMs);
}


static int SendFuncStatic(void* a_sender, const void* a_buffer, int a_bufSize)
{
	ASocketTCP aSocket;
	aSocket.SetSockDescriptor((int)((size_t)a_sender));
	return aSocket.SendData(a_buffer, a_bufSize);
}


static void CloseFuncStatic(void* a_senderReceiver)
{
	ASocketTCP aSocket;
	aSocket.SetSockDescriptor((int)((size_t)a_senderReceiver));
	aSocket.Close();
}
