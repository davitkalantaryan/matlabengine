/*
 *	File: namedpipeserver.cpp
 *
 *	Created on: Sep 24, 2016
 *	Author    : Davit Kalantaryan (Email: davit.kalantaryan@desy.de)
 *
 *  This file implements all functions ...
 *		1) ...
 *		2) ...
 *		...
 *
 *
 */

#include "namedpipeserver.h"

NamedPipeServer::NamedPipeServer(const char* a_pipe_name)
{
}


int NamedPipeServer::CreateServer(const char* a_pipe_name,int a_nInpBufSize, int a_nOutBufSize)
{
	m_Pipe = CreateNamedPipeA(
		a_pipe_name,             // pipe name 
		PIPE_ACCESS_DUPLEX,       // read/write access 
		PIPE_TYPE_MESSAGE |       // message type pipe 
		PIPE_READMODE_MESSAGE |   // message-read mode 
		PIPE_WAIT,                // blocking mode 
		PIPE_UNLIMITED_INSTANCES, // max. instances  
		a_nInpBufSize,            // output buffer size 
		a_nOutBufSize,            // input buffer size 
		10000,                    // client time-out 
		NULL);

	return (m_Pipe != INVALID_HANDLE_VALUE) ? 0 : GetLastError();
}


int NamedPipeServer::WaitForConnection()const
{
	int nLastError, nIteration(0);
	BOOL fConnected = FALSE;

	while (fConnected==FALSE)
	{
		fConnected = ConnectNamedPipe(m_Pipe, NULL);

		if (!fConnected)
		{
			nLastError = GetLastError();
			switch (nLastError)
			{
			case ERROR_NO_DATA:
				DisconnectNamedPipe(m_Pipe); // The pipe is being closed.
				break;
			case ERROR_PIPE_CONNECTED:
				fConnected = TRUE;
				break;
			default:
				break;
			}
			if (nIteration++ >= 1000){ return nLastError; }
		}

	}
	return 0;
}
