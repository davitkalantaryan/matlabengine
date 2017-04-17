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

namespace matlab{ namespace engine{

class ClientTcp : public ASocketTCP
{
public:
	int ConnectToServer(const char* serverName, int pid);
	int RecvData(void* buffer, int bufSize,long timeoutMS);

};

}}

#endif // #ifndef __matlab_engine_clienttcp_hpp__
