/*
 *	File: matlab_engine_servertcpbase.hpp
 *
 *	Created on: 14 Apr, 2017
 *	Author    : Davit Kalantaryan (Email: davit.kalantaryan@desy.de)
 *
 *  This file implements following class ...
 *
 */

#ifndef __matlab_engine_servertcpmex_hpp__
#define __matlab_engine_servertcpmex_hpp__

#include "matlab_engine_servertcpbase.hpp"
#include "matlab_engine_mathandlemex.hpp"

namespace matlab{ namespace engine{

class ServerTcpMex : public ServerTcpBase
{
public:
	ServerTcpMex();
	virtual ~ServerTcpMex();

private:
	MatHandleMex	m_matlabHandle;

};

}}

#endif // #ifndef __matlab_engine_servertcpmex_hpp__
