#ifndef __asockettcp_h
#define __asockettcp_h


#include "asocketb.h"

//#define RECVDATAINF


class ASocketTCP : public ASocketB  
{
public:
	LINKAGE_HDR ASocketTCP();

	LINKAGE_HDR ASocketTCP(const char* hostName, int port, long int timeout=1000);

	/*
	 * CreateSocket: initialise client, connect to server
	 * Parameter:
	 *	name:		hostname, on which server is running
	 *	port:		port number
	 * Return:
	 * 	0:	ok
	 *    < 0:	error
	 */
	LINKAGE_HDR int CreateSocket();

	/*
	 * CreateClient: initialise client, connect to server
	 * Parameter:
	 *	name:		hostname, on which server is running
	 *	port:		port number
	 * Return:
	 * 	0:	ok
	 *    < 0:	error
	 */
	LINKAGE_HDR int CreateClient(const char *a_szName, int a_nPort, long int a_lnTimeout = 1000);

	/*
	 * CreateClient: initialise client, connect to server
	 * Parameter:
	 *	name:		hostname, on which server is running
	 *	port:		port number
	 * Return:
	 * 	0:	ok
	 *    < 0:	error
	 */
	LINKAGE_HDR int ConnectToServer(const char *a_szName, int a_nPort, long int a_lnTimeout = 1000);

	/*
	 * RecvData:
	 * Parameter:
	 *    a_szBuff:		buff to save data
	 *    a_nSize:	buff size
	 *    a_nTimeout:  timeout for receive in seconds
	 * Return:
	 *    < 0:	error
	 *   other:	data length received
	 */
	LINKAGE_HDR int RecvData(void* Buffer, int Buf_Size, long SelectTm, long RcvTmout);

#ifdef RECVDATAINF
	LINKAGE_HDR int RecvDataInf( char* a_szBuff, int a_nSize )const;
#endif


	/* SendData:
	 * Parameter:
	 *    a_szBuff:		send data buff
	 *    a_nSize:	data length to send
	 * Return:
	 *    < 0:	error
	 *   other:	data length sent
	 */
	LINKAGE_HDR int SendData(const void* Buffer, int Buf_Size)const;

	LINKAGE_HDR int DisableNagle();

};

#endif  /* #ifndef __asockettcp_h */
