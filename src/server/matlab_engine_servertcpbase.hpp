/*
 *	File: matlab_engine_servertcpbase.hpp
 *
 *	Created on: 13 Apr, 2017
 *	Author    : Davit Kalantaryan (Email: davit.kalantaryan@desy.de)
 *
 *  This file implements following class ...
 *
 */

#ifndef __matlab_engine_servertcpbase_hpp__
#define __matlab_engine_servertcpbase_hpp__

#include "matlab_engine_serverbase.hpp"
#include <aservertcp.h>

namespace matlab{ namespace engine{

class ServerTcpBase : public ServerBase, private AServerTCP
{
public:
	ServerTcpBase(MatHandleBase* pMatHandle);
	virtual ~ServerTcpBase();

protected:
	int		AddClient(class ASocketTCP& a_ClientSocket, struct sockaddr_in* bufForRemAddress) __OVERRIDE__;
	virtual int		StartServerPrivate(void) __OVERRIDE__;
	virtual void	StopServerPrivate(void) __OVERRIDE__;
};

}}

#endif // #ifndef __matlab_engine_servertcpbase_hpp__
