#ifndef __asocketb_h__
#define __asocketb_h__

#if !defined(E_FATAL) & !defined(E_NO_BIND)

#include <errno.h>

#define E_FATAL			-1	/* fatal error */
#define E_NO_BIND		-7	/* can not bind address to port */
#define E_NO_CONNECT	-8	/* can not connect to server */
#define E_NO_LISTEN		-14	/* can not listen */
#define E_NO_SOCKET		-18	/* no socket generated */
#define E_RECEIVE		-25	/* error by receive */
#define E_SELECT		-28	/* error by select */
#define E_SEND			-29	/* error by send */
#define E_UNKNOWN_HOST		-38	/* can not find host */
#define E_WINSOCK_VERSION	-42	/* WINSOCK DLL version not requested */
#define	E_CONN_CLOSED		-46	/* connecttion closed by peer */

#ifndef INADDR_NONE
#define INADDR_NONE     0xffffffff
#endif

#ifdef	WIN32
//#include <Windows.h>
#ifndef SOCKET_INPROGRESS
#define	SOCKET_INPROGRESS(e)	(WSAGetLastError() == WSAEWOULDBLOCK)
#endif
#else
#include <netinet/in.h>
#if defined(EALREADY) && defined(EAGAIN)
#define	SOCKET_INPROGRESS(e)	(e == EINPROGRESS || e == EALREADY || e == EAGAIN)
#else
#ifdef  EALREADY
#define	SOCKET_INPROGRESS(e)	(e == EINPROGRESS || e == EALREADY)
#else
#ifdef  EAGAIN
#define	SOCKET_INPROGRESS(e)	(e == EINPROGRESS || e == EAGAIN)
#else
#define	SOCKET_INPROGRESS(e)	(e == EINPROGRESS)
#endif
#endif
#endif
#endif

#ifdef	WIN32
#define	SOCKET_WOULDBLOCK(e)	(WSAGetLastError() == WSAEWOULDBLOCK)
#else
#if defined(EAGAIN) && defined(EWOULDBLOCK)
#define	SOCKET_WOULDBLOCK(e)	(e == EAGAIN || e == EWOULDBLOCK)
#else
#ifdef	EWOULDBLOCK
#define	SOCKET_WOULDBLOCK(e)	(e == EWOULDBLOCK)
#else
#define	SOCKET_WOULDBLOCK(e)	(e == EAGAIN)
#endif
#endif
#endif


#define	_SOCKET_TIMEOUT_		-2001
#define	_NEGATIVE_ERROR_(x)		(((x)>0) ? -(x) : (x))
#define	_EINTR_ERROR_			_NEGATIVE_ERROR_(EINTR)


#define MAX_HOSTNAME_LENGTH     64

#endif/* #if !defined(E_FATAL) & !defined(E_NO_BIND) */

#ifndef LINKAGE_HDR
#define LINKAGE_HDR
#endif



/*
 * Base interface for all kind of sockets
 *
 */
class ASocketB  
{
public:
	LINKAGE_HDR ASocketB(const int& a_nSocket = -1);

	LINKAGE_HDR /*virtual */~ASocketB();

	/*
	 * Initialize socket library
	 */
	LINKAGE_HDR static int Initialize();
	
	/*
	 * Cleanup socket library
	 */
	LINKAGE_HDR static void Cleanup();

	/*
	 * Close open socket
	 */
	LINKAGE_HDR void Close();

	LINKAGE_HDR operator const int&()const;

	LINKAGE_HDR operator int&();

	LINKAGE_HDR void SetSockDescriptor(const int& a_nSocket){ m_socket = a_nSocket; }

	LINKAGE_HDR ASocketB& operator=(const int& a_nSocket);

	LINKAGE_HDR int DublicateSocket(const int& a_nProcID, void* a_pProtInfo)const;

	LINKAGE_HDR static long int	SleepM(long int msec);

	LINKAGE_HDR static long int	SleepU(long int usec);

	LINKAGE_HDR static long int	SleepN(long int nanoSec);

	LINKAGE_HDR static void		GetIPAddress(const struct sockaddr_in* a_addr, char* a_pcBuffer, int a_nBuffLen);

	LINKAGE_HDR static void		GetHostName(const struct sockaddr_in* a_addr, char* a_pcBuffer, int a_nBuffLen);

	LINKAGE_HDR static int		GetPort(const struct sockaddr_in* a_addr);

protected:
	//SOCKET				m_socket;
	int						m_socket;

	static int				m_nStaticFlags;

};


#if !defined(_CRT_LIB_) & !defined(SRC_USAGE) & !defined(USING_SOURCE) & !defined(LINKAGE_SRC) & !defined(SOCKT_SRC_USAGE)

#ifdef _MSC_VER

#ifndef NOT_ZNDESY
#ifndef LIBRARY_PATH
#ifdef _WIN64
#define LIBRARY_PATH	"Q:\\group\\pitz\\doocs\\lib\\libtcpsocket.lib"
//#define LIBRARY_PATH	"libtcpsocket.lib"
#else
#define	LIBRARY_PATH	"Q:\\group\\pitz\\doocs\\lib\\win32\\libtcpsocket.lib"
#endif  /* #ifdef _WIN64 */
#endif  /* #ifndef LIBRARY_PATH */
#else   /* #ifndef NOT_ZNDESY */
#ifndef LIBRARY_PATH
#error  LIBRARY_PATH should be defined. From this path "libsocket" library will be loaded
#endif
#endif  /* #ifndef NOT_ZNDESY */

#pragma comment(lib, LIBRARY_PATH)

#elif defined(__GNUC__)
#endif

#endif  /* #if !defined(_CRT_LIB_) & !defined(SRC_USAGE) & !defined(USING_SOURCE) */


#endif //#ifndef __asocketb_h__
