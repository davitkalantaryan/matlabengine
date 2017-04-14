#include "stdafx.h"

#ifndef SRC_USAGE
#define SRC_USAGE
#define SRC_USAGE_DEFINED_INSIDE
#endif
#include "asockettcp.h"

#ifdef WIN32
#include <winsock2.h>
#else
#include <sys/socket.h>
#include <unistd.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <netinet/tcp.h>
#include <memory.h>
#endif

#include <time.h>

#ifndef MSEC
#include <sys/timeb.h>
#define MSEC(finish, start)	( (long)( (finish).millitm - (start).millitm ) + \
							(long)( (finish).time - (start).time ) * 1000 )
#endif

#ifndef LINKAGE_SRC
#define LINKAGE_SRC
#endif

#pragma warning(disable : 4996)


LINKAGE_SRC ASocketTCP::ASocketTCP()
	:	ASocketB(-1)
{
}



LINKAGE_SRC ASocketTCP::ASocketTCP(const char* a_szName, int a_nPort, long int a_lnTimeout)
	: ASocketB(-1)
{
	CreateClient(a_szName, a_nPort, a_lnTimeout);
}



LINKAGE_SRC int ASocketTCP::CreateSocket()
{

	//if (!m_nStaticFlags) {ASocketB::Initialize();}

	ASocketB::Close();

    m_socket = (int)socket( AF_INET, SOCK_STREAM, 0 );

#ifdef	WIN32
	if( m_socket == INVALID_SOCKET )
#else
	if( m_socket < 0 )
#endif
	{
		m_socket = -1;
		return E_NO_SOCKET;
	}
	
	return 0;
}



LINKAGE_SRC int ASocketTCP::ConnectToServer(const char* a_szName, int a_nPort, long int a_lnTimeout)
{
	char *host = NULL;
	char l_host[MAX_HOSTNAME_LENGTH];
	int rtn = -1;

	if(m_socket==-1)
	{
		CreateSocket();
	}


#ifdef	WIN32
	if( m_socket == INVALID_SOCKET  || m_socket < 0 )
#else
	if( m_socket < 0 )
#endif
	{
		return E_NO_SOCKET;
	}

	host = (char *)a_szName;

	if( host == NULL || *host == '\0' )
	{

		if( gethostname( l_host, MAX_HOSTNAME_LENGTH) < 0 )
		{
			return E_UNKNOWN_HOST;
		}

		host = l_host;
	}

	unsigned long ha;
	struct sockaddr_in addr;
	memset( (char *)&addr, 0, sizeof(struct sockaddr_in) );
	addr.sin_family = AF_INET;
#ifdef	WIN32
	addr.sin_port = htons( (u_short)a_nPort );
#else
	addr.sin_port = htons( a_nPort );
#endif

	if( ( ha = inet_addr(a_szName) ) == INADDR_NONE )
	{
		struct hostent* hostent_ptr = gethostbyname(a_szName);

		if( !hostent_ptr )
		{
			return E_UNKNOWN_HOST;
		}

		a_szName = inet_ntoa(*(struct in_addr *)hostent_ptr->h_addr_list[0]);

		if( ( ha = inet_addr(a_szName) ) == INADDR_NONE )
		{
			return E_UNKNOWN_HOST;
		}
	}

	memcpy( (char *) &addr.sin_addr, (char *)&ha, sizeof(ha) );

#ifdef	WIN32
	unsigned long on = 1;
	ioctlsocket( m_socket, FIONBIO, &on );
#else  /* #ifdef	WIN32 */
	int status;
	if( ( status = fcntl( m_socket, F_GETFL, 0 ) ) != -1 )
	{
		status |= O_NONBLOCK;
		fcntl( m_socket, F_SETFL, status );
	}
#endif  /* #ifdef	WIN32 */
    
	int addr_len = sizeof(addr);
	rtn = connect( m_socket, (struct sockaddr *) &addr, addr_len);

	if( rtn != 0 )
	{
		int nErrno2 = errno;///?
		if (!SOCKET_INPROGRESS(nErrno2))
			return E_NO_CONNECT;
	}


	//////////////////////////////////////////////////////////////////////////
#if 1
	fd_set rfds;
	int maxsd = 0;

	FD_ZERO( &rfds );
	FD_SET( (unsigned int)m_socket, &rfds );
	maxsd = (int)(m_socket + 1);

	struct timeval		aTimeout2;
	struct timeval* pTimeout;

	if( a_lnTimeout >= 0 )
	{
#ifdef WIN32
		aTimeout2.tv_sec = a_lnTimeout / 1000L;
		aTimeout2.tv_usec = (a_lnTimeout%1000L)*1000L ;
#else
		aTimeout2.tv_sec = (time_t)(a_lnTimeout / 1000L);
		aTimeout2.tv_usec = (suseconds_t)((a_lnTimeout%1000L)*1000L) ;
#endif
		pTimeout = &aTimeout2;
	}
	else
	{
		pTimeout = NULL;
	}

	rtn = select(maxsd, (fd_set *) 0, &rfds, (fd_set *) 0, pTimeout );

	switch(rtn)
	{
		case 0:	/* time out */
			return _SOCKET_TIMEOUT_;
#ifdef	WIN32
		case SOCKET_ERROR:
#else
		case -1:
#endif
			if( errno == EINTR )
			{
				/* interrupted by signal */
				return _EINTR_ERROR_;
			}

		    return E_SELECT;
		default:
			break;
	}

	if( !FD_ISSET( m_socket, &rfds ) )
	{
		return E_FATAL;
	}
#endif  /* #if 1 */

	///////////////////////////////////////////////////////////////////////////////
	
	return 0;
}



LINKAGE_SRC int ASocketTCP::CreateClient(const char *a_szName, int a_nPort, long int a_lnTimeout)
{
	int nRet(CreateSocket());

	if(nRet)return nRet;

	nRet = ConnectToServer(a_szName, a_nPort, a_lnTimeout);
	if (nRet)
	{
		Close();
		return nRet;
	}
	return 0;
}

#ifdef DEBUG_SOCKET_FUNCTIONS

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef WIN32
#define _LAST_CHAR_ '\\'
#else
#define _LAST_CHAR_ '/'
#endif
#define __MY_FILE__ (strrchr(__FILE__,_LAST_CHAR_) ? (strrchr(__FILE__,_LAST_CHAR_)+1):__FILE__)
//#define _PRINT_REPORT_

static int s_nRecvAndSend = 0;
#endif   // #ifdef DEBUG_SOCKET_FUNCTIONS

#ifndef __FUNCTION__
#define	__FUNCTION__	"unknown"
#endif

LINKAGE_SRC int ASocketTCP::RecvData(void* a_pBuffer, int a_nSize, long a_lnSelectTm, long a_nRcvTmout)
{

	char* pcBuffer = (char*)a_pBuffer;

#ifdef DEBUG_SOCKET_FUNCTIONS
    static int nRectIter = 0;
    printf("%.2d -> fl:\"%s\",ln:%d,fnc:\"%s(%d)\" => %.2d\n",
           ++s_nRecvAndSend,__MY_FILE__,__LINE__,__FUNCTION__,a_nSize,++nRectIter);
#endif // #ifdef DEBUG_SOCKET_FUNCTIONS

	fd_set rfds;
	char *cp = NULL;
	int maxsd = 0;
	//int len_to_read = 0;
	
	int nTry(0);

	FD_ZERO( &rfds );
	FD_SET( (unsigned int)m_socket, &rfds );
	maxsd = m_socket + 1;

	// In not windows cases pselect instead of select can be considered
	// The reason that in pselect m_Timout remains constant
	// But here we choose other solution
	struct timeval		aTimeout2;
	struct timeval*		pTimeout;

	if( a_lnSelectTm >= 0 )
	{
		aTimeout2.tv_sec = a_lnSelectTm / 1000;
		aTimeout2.tv_usec = (a_lnSelectTm%1000)*1000 ;
		pTimeout = &aTimeout2;
	}
	else
	{
		pTimeout = NULL;
	}


	nTry = select(maxsd, &rfds, (fd_set *) 0, (fd_set *) 0, pTimeout );

	switch(nTry)
	{
		case 0:	/* time out */
			return _SOCKET_TIMEOUT_;
#ifdef	WIN32
		case SOCKET_ERROR:
#else
		case -1:
#endif
			if( errno == EINTR )
			{
				/* interrupted by signal */
				return _EINTR_ERROR_;
			}

		    return E_SELECT;
		default:
			break;
	}

	if( !FD_ISSET( m_socket, &rfds ) )
	{
		return E_FATAL;
	}


	int nReceived = 0;
	nTry = 0;
	cp = pcBuffer;
	int nRemained = a_nSize;


	timeb				aSettingTime;
	timeb				aCurrentTime;

	ftime( &aSettingTime );

	int nTryReadZero(0);
	long lnReceiveTime;
	int nIteration(0);


	while( nRemained > 0 )
	{

		nTry = recv( m_socket, cp, nRemained, 0 );

		if(!nIteration++)
		{
			if(!nTry)
			{
				//printf("===============E_CONN_CLOSED\n");
				//exit(1);
				break;
			}
		}

#ifdef	WIN32
		if( nTry == SOCKET_ERROR )
#else
		if( nTry < 0 )
#endif
		{
			if( SOCKET_WOULDBLOCK(errno) )
			{
				ASocketB::SleepN(1);
			}
			else
			{
				return E_RECEIVE;
			}
		}
		else if( nTry == 0 )
		{
			if( a_nRcvTmout < 0 && nTryReadZero++ == 2 )
				break;
			ASocketB::SleepN(1);
		}
		else
		{
			cp += nTry;
			nRemained -= nTry;
			nReceived += nTry;
//			break;
		}


		ftime( &aCurrentTime );
		lnReceiveTime = MSEC(aCurrentTime,aSettingTime);
		if( a_nRcvTmout >= 0 && lnReceiveTime >= a_nRcvTmout )
		{

			//printf("nReceived = %d instead of %d\n",nReceived,a_nSize);
			break;
			//if( nTry == 0 && nReceived == 0 && a_nSize )return E_CONN_CLOSED;
			//return _SOCKET_TIMEOUT_;
		}
	}

	if( nTry == 0 && nReceived == 0 && a_nSize )
	{
		return E_CONN_CLOSED;
	}

	return nReceived;

}



#ifdef RECVDATAINF
LINKAGE_SRC int ASocketTCP::RecvDataInf( char* a_szBuff, int a_nSize )const
{
	fd_set rfds;
	char *cp = NULL;
	int maxsd = 0;
	int len_to_read = 0;
	int len_read = 0;
	int n = 0;

	FD_ZERO( &rfds );
	FD_SET( (unsigned int)m_socket, &rfds );
	maxsd = m_socket + 1;

	n = select(maxsd, &rfds, (fd_set *) 0, (fd_set *) 0, 0 );

	switch(n)
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
				return 0;
			}

		    return E_SELECT;
		default:
			break;
	}

	if( !FD_ISSET( m_socket, &rfds ) )
	{
		return E_FATAL;
	}

	n = 0;
	cp = a_szBuff;
	len_to_read = a_nSize;
	while( len_to_read > 0 )
	{
		n = recv( m_socket, cp, len_to_read, 0 );
#ifdef	WIN32
		if( n == SOCKET_ERROR )
#else
		if( n < 0 )
#endif
		{
			if( SOCKET_WOULDBLOCK(errno) )
			{
				break;
			}
			else
			{
				return E_RECEIVE;
			}
		}
		else if( n == 0 )
		{
			break;
		}
		else
		{
			cp += n;
			len_to_read -= n;
			len_read += n;
			break;
		}
	}

	if( n == 0 && len_read == 0 )
	{
		return E_CONN_CLOSED;
	}


	return len_read;
}
#endif



LINKAGE_SRC int ASocketTCP::SendData(const void* a_cpBuffer, int a_nSize)const
{
	const char* pcBuffer = (const char*)a_cpBuffer;
	const char *cp = NULL;
	int len_to_write = 0;
	int len_wrote = 0;
	int n = 0;

#ifdef DEBUG_SOCKET_FUNCTIONS
    static int nRectIter = 0;
    printf("%.2d -> fl:\"%s\",ln:%d,fnc:\"%s(%d)\" => %.2d\n",
           ++s_nRecvAndSend,__MY_FILE__,__LINE__,__FUNCTION__,a_nSize,++nRectIter);
#endif // #ifdef DEBUG_SOCKET_FUNCTIONS

	if( a_nSize <= 0 )
		return 0;

	len_to_write = a_nSize;
	cp = pcBuffer;
	while( len_to_write > 0 )
	{
		n = send( m_socket, cp, len_to_write, 0);
#ifdef	WIN32
		if( n == SOCKET_ERROR )
#else
		if( n < 0 )
#endif
		{
			if( SOCKET_WOULDBLOCK(errno) )
			{
//				break;
				continue;
			}
			else
			{
				return E_SEND;
			}
		}
		else
		{
			cp += n;
			len_to_write -= n;
			len_wrote += n;
		}
	}

	return len_wrote;
}



LINKAGE_SRC int ASocketTCP::DisableNagle()
{

	int flag = 1;
	
	int result = setsockopt(	m_socket,			/* socket affected */
								IPPROTO_TCP,		/* set option at TCP level */
								TCP_NODELAY,		/* name of option */
								(char *) &flag,		/* the cast is historical cruft */
								sizeof(int) );		/* length of option value */

	return result;
}


#ifdef SRC_USAGE_DEFINED_INSIDE
#undef SRC_USAGE
#endif
