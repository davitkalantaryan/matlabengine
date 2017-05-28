/*
 *	File: matlab_engine_clienttcp.hpp
 *
 *	Created on: 14 Apr, 2017
 *	Author    : Davit Kalantaryan (Email: davit.kalantaryan@desy.de)
 *
 *  This file implements following class ...
 *
 */

#ifndef __matlab_engine_clienttcp_hpp__
#define __matlab_engine_clienttcp_hpp__

#include <asockettcp.h>
#include <vector>
#include <common_socketbase.hpp>
#include <common_defination.h>

#define READ1_BIT	0
#define	WRITE_BIT	1
#define	ERROR_BIT	2

#define GET_BIT(_variable_,_bitNum_) (  (1<<(_bitNum_)) & (_variable_)  )
#define SET_BIT(_variablePtr_,_bitNum_,_bitVal_) \
	*(_variablePtr_)=(  ((~(1<<(_bitNum_)))&(*(_variablePtr_))) | ((_bitVal_)<<(_bitNum_))   )

namespace matlab{ namespace engine{

class ClientTcp : public common::SocketBase, public ASocketTCP
{
public:
	virtual ~ClientTcp() {}
	int ConnectToServer(const char* serverName, int engineNumber);
	virtual int Recv(void* buffer, int bufSize,long timeoutMS)__OVERRIDE__;
	virtual int Send(const void* buffer, int bufSize)__OVERRIDE__;
	static int	selectMltC(
		long timeoutMs,int* a_pnMaxPlus1, int a_nNumberOfSockets,
		const int* a_pnSocketsIn, int* a_pnReadOut, int* a_pnWriteOut, int* a_pnExcpt);
	static int	selectMltCpp(
		long a_lnTimeoutMs, int* a_pnMaxPlus1,
		const std::vector<int>& a_rdfd, const std::vector<int>& a_wrfd, const std::vector<int>& a_erfd,
		std::vector<char>& a_return);
	static void CloseSt(int a_socket);

};

}}

#endif // #ifndef __matlab_engine_clienttcp_hpp__
