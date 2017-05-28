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

extern bool g_bUpdateNeeded;

#define EQFCT_PROXY_CODE	301
#ifndef use_mat_matrix
#define use_mat_matrix
#endif

#include <string>
#include "eq_fct.h"
#include "common_defination.h"
#include "matlab_engine_clienttcp.hpp"
#include "matlab_engine_serializer.hpp"
#include "mat_matrix.h"
#include <sys/timeb.h>
#include <sys/types.h>

namespace matlab{ namespace engine{ namespace mngr{

template <typename D_Type>
class D_arg : public D_Type
{
public:
	D_arg(const char* pn, ::EqFct* par);
	D_arg(const char* pn, int size, ::EqFct* par);
	virtual ~D_arg();

protected:
	virtual void set(EqAdr * dcsAdr, EqData *fromUser, EqData * toUser, EqFct * fct)__OVERRIDE__;
	virtual void get(EqAdr * dcsAdr, EqData *fromUser, EqData * toUser, EqFct * fct)__OVERRIDE__;
};


class D_expression : public D_string
{
public:
	D_expression(const char* pn, ::EqFct* par);
	void set(EqAdr * dcsAdr, EqData *fromUser, EqData * toUser, EqFct * fct)__OVERRIDE__;
	void get(EqAdr * dcsAdr, EqData *fromUser, EqData * toUser, EqFct * fct)__OVERRIDE__;
};


class D_startEngine : public D_fct
{
public:
	D_startEngine(const char* pn, ::EqFct* par);
	void set(EqAdr * dcsAdr, EqData *fromUser, EqData * toUser, EqFct * fct)__OVERRIDE__;
};


class EqFctProxy : public ::EqFct
{
	friend class D_startEngine;
	friend class D_expression;
public:
	EqFctProxy(int a_nEngineNumber);
	virtual ~EqFctProxy();

	static EqFctProxy*	EngineControled2(int engine);
	static void			StartNewEngine(const std::string& commandToStart);
	static int			InitProxies();
	static void			DestroyProxies();
	
	void				SetForProperties(EqData *fromUser, EqData *toUser);
	void				GetForProperties(EqData *fromUser, EqData *toUser);
	void				TryConnect();

protected:
	virtual int		fct_code(void) __OVERRIDE__;
	virtual void    update(void) __OVERRIDE__;
	virtual void	init(void) __OVERRIDE__;
	virtual void	cancel(void) __OVERRIDE__;
	virtual void    set(EqAdr *, EqData *, EqData *)__OVERRIDE__;
	virtual void    get(EqAdr *, EqData *, EqData *)__OVERRIDE__;

	int				SendCommandToMatlab(int numOfInputs,const char* a_cpcScriptName="");
	int				ReceiveResultFromMatlab();
	void			GetForPropertiesPrivate(EqData *fromUser, EqData *toUser);

protected:
	D_string						m_matlabScriptName;
	D_int							m_numberOfExpectedOutputs;
	D_int							m_numberOfLastOutputs;
	D_int							m_engineNumber;
	D_string						m_engineStartCommand;
	D_int							m_isConnected2;
	D_int							m_recvTimeoutMs;
	D_startEngine					m_engineStarter;
	D_int							m_maxTryCount;

#if 0
#define	DATA_NULL               0
#define	DATA_INT                1
#define	DATA_FLOAT              2
#define	DATA_STRING             3
#define	DATA_BOOL               4
#define	DATA_DOUBLE             6
#define	DATA_SPECTRUM           19
#endif // #if 0
	D_arg<D_fct>					m_argVoid;
	D_arg<D_int>					m_argInt;
	D_arg<D_float>					m_argFloat;
	D_arg<D_string>					m_argString;
	D_arg<D_int>					m_argBool;
	D_arg<D_double>					m_argDouble;
	D_arg<D_spectrum>				m_argSpectrum;
	D_expression					m_expression;

    PersistantArgsGroup				m_vArray;
    Serializer						m_serializeDes;
    ClientTcp						m_socketTcp;
    versioning::FncPointers			m_vFuncs;
    int								m_nEngineNumber;
    bool							m_bPendingTask;
	bool							m_bAnyResultsExist;
	bool							m_bIsConnected2;
	int								m_nNumOfTryOfStart;
	struct timeb					m_lastStartTime;
};

}}}


#include "matlab_engine_mngr_eqfctproxy.tos"


#endif  // #ifndef __matlab_engine_mngr_eqfctproxy_hpp__
