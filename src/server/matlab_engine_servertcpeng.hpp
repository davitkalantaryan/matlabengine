/*****************************************************************************
 * File   : matlab_engine_servertcpeng.hpp
 * created: 2017 Apr 25
 *****************************************************************************
 * Author:	D.Kalantaryan, Tel:+49(0)33762/77552 kalantar
 * Email :	davit.kalantaryan@desy.de
 * Mail  :	DESY, Platanenallee 6, 15738 Zeuthen
 *****************************************************************************
 * Description
 *   ...
 ****************************************************************************/

#ifndef __matlab_engine_servertcpeng_hpp__
#define __matlab_engine_servertcpeng_hpp__

#include "matlab_engine_servertcpbase.hpp"
#include "matlab_engine_mathandleeng.hpp"

namespace matlab{ namespace engine{

class ServerTcpEng : public ServerTcpBase
{
public:
    ServerTcpEng();
    virtual ~ServerTcpEng();

private:
    MatHandleEng	m_matlabHandle;

};

}}

#endif // #ifndef __matlab_engine_servertcpeng_hpp__
