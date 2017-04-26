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
#include <common_socketbase.hpp>
#include <common_defination.h>

namespace matlab{ namespace engine{

class ClientTcp : public common::SocketBase, public ASocketTCP
{
public:
	virtual ~ClientTcp() {}
	int ConnectToServer(const char* serverName, int pid);
	virtual int Recv(void* buffer, int bufSize,long timeoutMS)__OVERRIDE__;
	virtual int Send(const void* buffer, int bufSize)__OVERRIDE__;

};

}}

#endif // #ifndef __matlab_engine_clienttcp_hpp__
