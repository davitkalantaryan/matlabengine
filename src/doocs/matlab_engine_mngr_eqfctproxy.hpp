/*****************************************************************************
 * File   : matlab_engine_mngr_eqfctproxy.hpp
 * created: 2017 May 02
 *****************************************************************************
 * Author:	D.Kalantaryan, Tel:+49(0)33762/77552 kalantar
 * Email :	davit.kalantaryan@desy.de
 * Mail  :	DESY, Platanenallee 6, 15738 Zeuthen
 *****************************************************************************
 * Description
 *   ...
 ****************************************************************************/

#ifndef __matlab_engine_mngr_eqfctproxy_hpp__
#define __matlab_engine_mngr_eqfctproxy_hpp__

#ifndef DEBUG_APP_RAW0
extern int g_nDebugLevel;

#define DEBUG_APP_RAW0(_level_,_fnc_,...)	do{if((_level_)<=g_nDebugLevel){_fnc_(__VA_ARGS__);}}\
												while(0)
#define DEBUG_APP_RAW1(_level_,_fd_,...)	DEBUG_APP_RAW0(_level_,fprintf,_fd_,__VA_ARGS__)
#define DEBUG_APP_RAW2(_level_,...)			DEBUG_APP_RAW0(_level_,printf,__VA_ARGS__)
#endif  // #ifndef DEBUG_APP_RAW0

#define EQFCT_PROXY_CODE	301
#ifndef use_mat_matrix
#define use_mat_matrix
#endif

#include "eq_fct.h"
#include "common_defination.h"
#include "matlab_engine_clienttcp.hpp"
#include "matlab_engine_serializer.hpp"
#include "mat_matrix.h"

namespace matlab{ namespace engine{ namespace mngr{

template<typename D_Type,typename DataType>
class D_Proxy : public D_Type
{
public:
	D_Proxy(const char* pn, ::EqFct* par);
	virtual ~D_Proxy();

protected:
	virtual void set(EqAdr * dcsAdr, EqData *fromUser, EqData * toUser, EqFct * fct)__OVERRIDE__;
	virtual void get(EqAdr * dcsAdr, EqData *fromUser, EqData * toUser, EqFct * fct)__OVERRIDE__;
};

template<class DataType>
class D_Proxy<D_fct, DataType> : public D_fct
{
public:
	D_Proxy(const char* pn, ::EqFct* par);
	virtual ~D_Proxy();

protected:
	virtual void set(EqAdr * dcsAdr, EqData *fromUser, EqData * toUser, EqFct * fct)__OVERRIDE__;
};


class D_engineNum : public D_int
{
public:
	D_engineNum(const char* pn, ::EqFct* par);
	void set(EqAdr * dcsAdr, EqData *fromUser, EqData * toUser, EqFct * fct)__OVERRIDE__;
};


class EqFctProxy : public ::EqFct
{
public:
	EqFctProxy(int a_nEngineNumber);
	virtual ~EqFctProxy();

	PersistantArgsGroup& GetArray();

	int SendCommandToMatlab(int numOfInputs);
	int ReceiveResultFromMatlab();

	static EqFctProxy*	EngineControled2(int engine);

protected:
	virtual int		fct_code(void) __OVERRIDE__;
	virtual void    update(void) __OVERRIDE__;
	virtual void	init(void) __OVERRIDE__;
	virtual void	cancel(void) __OVERRIDE__;

	int				TryConnect();

protected:
    D_string						m_engineName;
	D_string						m_matlabScriptName;
	D_string						m_commandToStartMatlabEngine;
	D_int							m_numberOfExpectedOutputs;
	D_engineNum						m_engineNumber;

	D_Proxy<D_int, int>				m_argInt;
	D_Proxy<D_float, float>			m_argFloat;
	D_Proxy<D_double, double>		m_argDouble;
	D_Proxy<D_string, char*>		m_argString;
	D_Proxy<D_fct, void>			m_argVoid;

    PersistantArgsGroup				m_vArray;
    Serializer						m_serializeDes;
    ClientTcp						m_socketTcp;
    versioning::FncPointers			m_vFuncs;
    int								m_nEngineNumber;
    bool							m_bPendingTask;
};

}}}


#include "matlab_engine_mngr_eqfctproxy.tos"


#endif  // #ifndef __matlab_engine_mngr_eqfctproxy_hpp__
