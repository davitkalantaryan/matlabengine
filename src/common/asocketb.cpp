#include "stdafx.h"


#if 1
#ifdef WIN32
#ifndef socklen_t
#define	socklen_t	int
#endif/* #ifndef socklen_t */
#ifndef snprintf
#define snprintf _snprintf
#endif /*#ifndef snprintf*/
#else
#endif
#endif

#include <string.h>

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

#ifdef WIN32
	#ifndef __SOCET_LIB_LOADED
		#pragma comment(lib, "Ws2_32.lib")
		#define __SOCET_LIB_LOADED
	#endif //__SOCET_LIB_LOADED

#else
#endif

#include "asocketb.h"

int ASocketB::m_nStaticFlags = 0;

#ifndef LINKAGE_SRC
#define LINKAGE_SRC
#endif


LINKAGE_SRC ASocketB::ASocketB(const int& a_nSocket)
	:	m_socket(a_nSocket)
{
#ifdef WIN32
	if (!m_nStaticFlags) { ASocketB::Initialize(); }
#endif
}



LINKAGE_SRC ASocketB::~ASocketB()
{
	ASocketB::Close();
}



/*
 * Initialize socket library
 */
LINKAGE_SRC int ASocketB::Initialize()
{
#ifdef WIN32
	WORD wVersionRequested;
	WSADATA wsaData;

	wVersionRequested = MAKEWORD(2,2);

	if( WSAStartup(wVersionRequested, &wsaData) != 0)
	{
		return 0;
	}

	/* Confirm that the WinSock DLL supports 2.2.		*/
	/* Note that if the DLL supports versions greater	*/
	/* than 2.2 in addition to 2.2, it will still return*/
	/* 2.2 in wVersion since that is the version we		*/
	/* requested.										*/

	if( LOBYTE(wsaData.wVersion) != 2 ||
		HIBYTE(wsaData.wVersion) != 2 )
	{
		WSACleanup();
		return 0;
	}

#endif

	++m_nStaticFlags;

	return 1;
}



/*
 * Cleanup socket library
 */
LINKAGE_SRC void ASocketB::Cleanup()
{
#ifdef WIN32
	WSACleanup();
#endif

	if (m_nStaticFlags > 0) --m_nStaticFlags;
}



LINKAGE_SRC void ASocketB::Close()
{
	if( m_socket > 0 )
#ifdef	WIN32
		closesocket( m_socket );
#else
		close( m_socket );
#endif
	m_socket = -1;
}



LINKAGE_SRC ASocketB::operator const int&()const
{
	return m_socket;
}



LINKAGE_SRC ASocketB::operator int&()
{
	return m_socket;
}



LINKAGE_SRC ASocketB& ASocketB::operator=(const int& a_nSocket)
{
	m_socket = a_nSocket;

	return *this;
}



LINKAGE_SRC int ASocketB::DublicateSocket(const int& a_nProcID, void* a_pProtInfo)const
{
#ifdef WIN32
	WSAPROTOCOL_INFO* pProtInfo = (WSAPROTOCOL_INFO*)a_pProtInfo;
	return WSADuplicateSocket( m_socket, (DWORD)a_nProcID, pProtInfo );
#else
	throw "For berkley sockets not implemented!";
	return ( a_nProcID == (int)((size_t)a_pProtInfo) ) ? 1 : 0;
#endif
}


#include <time.h>


LINKAGE_SRC long ASocketB::SleepM(long int msec)
{

#ifdef WIN32
	Sleep(msec);
	return msec;

#else
	struct timespec waitspec;
	waitspec.tv_sec = msec/1000;
	waitspec.tv_nsec = (msec%1000)*1000000;
	return (long)nanosleep(&waitspec, NULL);
#endif
}



LINKAGE_SRC long ASocketB::SleepU(long int usec)
{

#ifdef WIN32
	struct timeval tv;
	int maxsd, fh1;
	fd_set rfds;

	fh1 = (int)socket( AF_INET, SOCK_STREAM, 0 );

	//fd_set rfds;
	FD_ZERO( &rfds );
	FD_SET( (unsigned int)fh1, &rfds );
	maxsd = fh1 + 1;

	//struct timeval tv;
	tv.tv_sec = usec/1000000L;
	tv.tv_usec = usec%1000000L;
	
	return (long)select(maxsd, &rfds, 0, 0, &tv);

#else
	usleep(usec);
	return usec;
#endif
}



LINKAGE_SRC long ASocketB::SleepN(long int nanoSec)
{
#ifdef WIN32
	return 1000*ASocketB::SleepU(nanoSec/1000);
#else
	struct timespec waitspec;
	waitspec.tv_sec = nanoSec/1000000000;
	waitspec.tv_nsec = nanoSec%1000000000;
	return (long)nanosleep(&waitspec, NULL);
#endif
}


#ifdef _MSC_VER
#if(_MSC_VER >= 1400)
//#define		_WINSOCK_DEPRECATED_NO_WARNINGS
#pragma warning(disable : 4996)
#endif
#endif


LINKAGE_SRC void ASocketB::GetIPAddress(const struct sockaddr_in* a_addr, char* a_pcBuffer, int a_nBuffLen)
{
	
	if(!a_addr)
		return;

	const sockaddr_in& addr = *a_addr;

	const char* cpcStrIP = inet_ntoa(addr.sin_addr);

	int nStrLen = (int)strlen(cpcStrIP);

	nStrLen = nStrLen<a_nBuffLen ? nStrLen : a_nBuffLen;

	memcpy(a_pcBuffer,cpcStrIP,nStrLen);
}


#ifdef WIN32
//#include <Ws2tcpip.h>
//#include <Wspiapi.h>
//#include <ws2def>
#ifndef NI_NUMERICSERV
#define NI_NUMERICSERV  0x08  /* Return numeric form of the service (port #) */
#endif
typedef int  (WSAAPI* getnameinfo_type) (const struct sockaddr*,
                                       socklen_t, char*, DWORD,
                                       char*, DWORD, int);
#endif


LINKAGE_SRC void ASocketB::GetHostName(const struct sockaddr_in* a_addr, char* a_pcBuffer, int a_nBuffLen)
{

	const sockaddr_in& addr = *a_addr;

//#if !defined(WIN32) || (_MSC_VER >= 1400)
#ifndef WIN32
	if(!a_addr)
		return;
	getnameinfo((struct sockaddr *) &addr,sizeof (struct sockaddr),a_pcBuffer,a_nBuffLen, NULL, 0, NI_NUMERICSERV);
#else
	//ASocketB::GetIPAddress(a_addr,a_pcBuffer,a_nBuffLen);
	//getnameinfo((struct sockaddr *) &addr,sizeof (struct sockaddr),a_pcBuffer,a_nBuffLen, NULL, 0, 0);

	if(!a_addr)
		return;
	
	HMODULE h = GetModuleHandleA ("ws2_32.dll");
	getnameinfo_type getnameinfo_ptr = (getnameinfo_type)GetProcAddress (h, "getnameinfo");

	if(getnameinfo_ptr)
		(*getnameinfo_ptr)((struct sockaddr *) &addr,sizeof (struct sockaddr),a_pcBuffer,a_nBuffLen, NULL, 0, NI_NUMERICSERV);
#endif
}



LINKAGE_SRC int ASocketB::GetPort(const struct sockaddr_in* a_addr)
{
	if(!a_addr)
		return 0;
	return (int) ntohs(a_addr->sin_port);
}
