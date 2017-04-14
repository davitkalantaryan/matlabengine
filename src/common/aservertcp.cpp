#include "stdafx.h"
#define SRC_USAGE
#include "aservertcp.h"


#ifdef WIN32
	#include <WinSock2.h>
#else
	#include <sys/socket.h>
	#include <unistd.h>
	#include <netdb.h>
	#include <fcntl.h>
	#include <memory.h>
#endif

#ifndef LINKAGE_SRC
#define LINKAGE_SRC
#endif


LINKAGE_SRC AServerTCP::AServerTCP()
	:	ASocketTCP( )
{
}



/*LINKAGE_SRC int AServerTCP::StartServer( int a_nPort )
{

	int nRet;


#ifdef _CD_VERSION__
	nRet = CreateServer( a_nPort, true );
#else
	nRet = CreateServer( a_nPort ) ;
#endif
	

	return StartServer2(a_nPort,1000L,NULL);
}*/


#include <stdio.h>



LINKAGE_SRC int AServerTCP::StartServer(int a_nPort, long int a_lnTimeout, struct sockaddr_in* a_bufForRemAddress)
{

	int nError;

#ifdef _CD_VERSION__
	nError = CreateServer( a_nPort, true );
#else
	nError = CreateServer( a_nPort ) ;
#endif

	if( nError != 0 )
	{
		Close();
		return nError;
	}

	return RunServer(a_lnTimeout, a_bufForRemAddress);
}



LINKAGE_SRC int AServerTCP::RunServer(long int a_lnTimeout, struct sockaddr_in* a_bufForRemAddress)
{

	int nError, nClientSocket;
	ASocketTCP aClientSocket;

	m_nQUIT_FLAG = 0;

	while( !m_nQUIT_FLAG )
	{

		if ((nError = ServerAccept(nClientSocket, a_lnTimeout, a_bufForRemAddress)) == 1)
		{
			aClientSocket.SetSockDescriptor( nClientSocket );

			if (!AddClient(aClientSocket, a_bufForRemAddress))
			{
//					delete client;
//					logError.Write( "Add job error!!!" );
			}
		}
//		else if( error == 2 )
//			break;

		if(nError<0)aClientSocket.Close();
		else aClientSocket.SetSockDescriptor(-1);

#ifndef _CD_VERSION__
		/*if( m_nSIG_INFO )
		{
//				logEvent.Write( "Job count in queue %d, semaphore %d", nCount, nSem );

			m_nSIG_INFO = 0;
		}*/
#endif
	}


	Close();

//	logEvent.Write("Server stopped!");

	return m_nQUIT_FLAG;
}



LINKAGE_SRC void AServerTCP::StopServer(int a_nQUIT_FLAG)
{
	m_nQUIT_FLAG = a_nQUIT_FLAG;
}



/*
 * ServerAccept: server waiting for new connection
 * Parameter:
 *	a_ppClient:	accepted client socket
 * Return:
 *    < 0:	error
 *   	0:	timeout
 *	1:	ok
 */
LINKAGE_SRC int AServerTCP::ServerAccept(int& a_nClientSocket, long int a_lnTimeout, struct sockaddr_in* a_bufForRemAddress)
{

	fd_set rfds;

	int maxsd = 0;
	int rtn = 0;

	FD_ZERO( &rfds );
	FD_SET( (unsigned int)m_socket, &rfds );
	
	maxsd = m_socket + 1;

	// In not windows cases pselect instead of select can be considered
	// The reason that in pselect m_Timout remains constant
	// But here we choose other solution
	struct timeval		aTimeout2;
	struct timeval*		pTimeout;


	if( a_lnTimeout >= 0 )
	{
#ifdef WIN32
		aTimeout2.tv_sec = a_lnTimeout/1000L;
		aTimeout2.tv_usec = (a_lnTimeout%1000L)*1000L ;
#else
		aTimeout2.tv_sec = (time_t)(a_lnTimeout/1000L);
		aTimeout2.tv_usec = (suseconds_t)((a_lnTimeout%1000L)*1000L) ;
#endif
		pTimeout = &aTimeout2;
	}
	else
	{
		pTimeout = NULL;
	}

	rtn = select(maxsd, &rfds, (fd_set *) 0, (fd_set *) 0, pTimeout);


	switch(rtn)
	{
		case 0:	/* time out */
			return 0;
#ifdef	WIN32
		case SOCKET_ERROR:
#else
		case -1:
#endif
			if( errno == EINTR )
			{
				/* interrupted by signal */
				return 2;
			}

			return(E_SELECT);
		default:
			break;
	}

	if( !FD_ISSET( m_socket, &rfds ) )
		return(E_FATAL);


	struct sockaddr_in addr;

#ifdef	WIN32
	int addr_len = sizeof(addr);
#else
	socklen_t addr_len = sizeof(addr);
#endif
	a_nClientSocket = (int)accept( m_socket, (struct sockaddr *)&addr, &addr_len);


#ifdef	WIN32
	if( a_nClientSocket == INVALID_SOCKET )
#else
	if( a_nClientSocket < 0)
#endif
	{
		return 0;
	}

#ifdef	WIN32
	{
		u_long non = 1;
		ioctlsocket( a_nClientSocket, FIONBIO, &non);
	}
#else
	int status;
	if( (status = fcntl( a_nClientSocket, F_GETFL, 0 )) != -1)
	{
		status |= O_NONBLOCK;
		fcntl( a_nClientSocket, F_SETFL, status );
	}
#endif

	if (a_bufForRemAddress)
	{
		//struct sockaddr_in* pIncAddr = (struct sockaddr_in*)a_pIncAddr;
		*a_bufForRemAddress = addr;
	}


	return 1;
}



LINKAGE_SRC int AServerTCP::CreateServer(int a_nPort, bool a_bLoopback)
{
	char l_host[MAX_HOSTNAME_LENGTH];
	int rtn = -1;

	ASocketB::Close();

	int i = 1;
    m_socket = (int)socket( AF_INET, SOCK_STREAM, 0 );
    setsockopt( m_socket, SOL_SOCKET, SO_REUSEADDR, (char*)&i, sizeof(i) );

#ifdef	WIN32
	if( m_socket == INVALID_SOCKET)
#else
	if( m_socket < 0 )
#endif
		return(E_NO_SOCKET);


	if( gethostname( l_host, MAX_HOSTNAME_LENGTH ) < 0 )
		return E_UNKNOWN_HOST;


	struct sockaddr_in addr;
	memset( (char *)&addr, 0, sizeof(struct sockaddr_in));
	addr.sin_family	= AF_INET;
#ifdef	WIN32
	addr.sin_port = htons((u_short)a_nPort);
#else
	addr.sin_port = htons( a_nPort );
#endif

	addr.sin_addr.s_addr = htonl( (a_bLoopback ? INADDR_LOOPBACK : INADDR_ANY ) );

#ifdef	WIN32
	{
		u_long non = 1;
		ioctlsocket( m_socket, FIONBIO, &non);
	}
#else
	int status;
	if( (status = fcntl( m_socket, F_GETFL, 0 )) != -1)
	{
		status |= O_NONBLOCK;
		fcntl( m_socket, F_SETFL, status );
	}
#endif

	int addr_len = sizeof(addr);
	rtn = bind( m_socket, (struct sockaddr *) &addr, addr_len );

#ifdef	WIN32
	if( rtn == SOCKET_ERROR )
#else
	if( rtn < 0 )
#endif
		return(E_NO_BIND);

	rtn = listen( m_socket, 64);

#ifdef	WIN32
	if( rtn == SOCKET_ERROR )
#else
	if( rtn < 0 )
#endif
		return(E_NO_LISTEN);

	return 0;
}
