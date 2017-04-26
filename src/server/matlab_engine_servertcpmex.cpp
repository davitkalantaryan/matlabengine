/*
 *	File: matlab_engine_servertcp.cpp
 *
 *	Created on: 13 Apr, 2017
 *	Author    : Davit Kalantaryan (Email: davit.kalantaryan@desy.de)
 *
 *  This file implements following class ...
 *
 */

#include "matlab_engine_servertcpmex.hpp"
#include "matlab_pipe_name.h"

matlab::engine::ServerTcpMex::ServerTcpMex()
	:
	ServerTcpBase(&m_matlabHandle)
{
	m_pMatHandle = &m_matlabHandle;
}


matlab::engine::ServerTcpMex::~ServerTcpMex()
{
}
