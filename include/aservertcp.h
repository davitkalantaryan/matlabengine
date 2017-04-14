#ifndef __aservertcp_h
#define __aservertcp_h

#include "asockettcp.h"
#include <stddef.h>


class AServerTCP : protected ASocketTCP
{

public:
	LINKAGE_HDR AServerTCP();

	LINKAGE_HDR virtual ~AServerTCP(){}

	LINKAGE_HDR int				StartServer(int a_nPort, long int a_lnTimeout, struct sockaddr_in* bufForRemAddress = NULL);
	
	LINKAGE_HDR void			StopServer(int a_nQUIT_FLAG = 1);

protected:
	/*
	 * ServerAccept: server waiting for new connection
	 * Parameter:
	 *	a_ppClient:	accepted client socket
	 * Return:
	 *    < 0:	error
	 *   	0:	timeout
	 *	1:	ok
	 */
	LINKAGE_HDR int				ServerAccept(int& a_nClientSocket, long int a_lnTimeout, struct sockaddr_in* remAddress);

	/*
	 * CreateServer: create server
	 * Parameter:
	 *	port: 	port number
	 * Return:
	 * 	0:	ok
	 *    < 0:	error
	 */
	LINKAGE_HDR int				CreateServer(int a_nPort, bool a_bLoopback = false);

	LINKAGE_HDR int				RunServer(long int a_lnTimeout, struct sockaddr_in* bufForRemAddress);

private:
	LINKAGE_HDR virtual int		AddClient(class ASocketTCP& a_ClientSocket, struct sockaddr_in* bufForRemAddress) = 0;


protected:
	volatile int	m_nQUIT_FLAG;
};



#endif  /* #ifndef __aservertcp_h */
