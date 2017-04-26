/*****************************************************************************
 * File   : matlab_engine_servertcpeng.cpp
 * created: 2017 Apr 25
 *****************************************************************************
 * Author:	D.Kalantaryan, Tel:+49(0)33762/77552 kalantar
 * Email :	davit.kalantaryan@desy.de
 * Mail  :	DESY, Platanenallee 6, 15738 Zeuthen
 *****************************************************************************
 * Description
 *   ...
 ****************************************************************************/

#include "matlab_engine_servertcpeng.hpp"
#include "matlab_pipe_name.h"

matlab::engine::ServerTcpEng::ServerTcpEng()
	:
	ServerTcpBase(&m_matlabHandle)
{
	m_pMatHandle = &m_matlabHandle;
}


matlab::engine::ServerTcpEng::~ServerTcpEng()
{
}
