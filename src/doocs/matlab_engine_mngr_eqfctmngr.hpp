/*****************************************************************************
 * File   : matlab_engine_mngr_eqfct.hpp
 * created: 2017 May 02
 *****************************************************************************
 * Author:	D.Kalantaryan, Tel:+49(0)33762/77552 kalantar
 * Email :	davit.kalantaryan@desy.de
 * Mail  :	DESY, Platanenallee 6, 15738 Zeuthen
 *****************************************************************************
 * Description
 *   ...
 ****************************************************************************/

#ifndef __matlab_engine_mngr_eqfctmngr_hpp__
#define __matlab_engine_mngr_eqfctmngr_hpp__

#define EQFCT_MNGR_CODE	302

#include "eq_fct.h"
#include "common_defination.h"
#include "matlab_engine_clienttcp.hpp"
#include "matlab_pipe_name.h"
#include "thread_cpp11.hpp"
#include "common_fifofast.hpp"
#include "common_unnamedsemaphorelite.hpp"

namespace matlab{ namespace engine{ namespace mngr{

class D_SingleEngine : public D_string
{
public:
	D_SingleEngine(int engineNumber,const char* pn, ::EqFct* par);
	virtual ~D_SingleEngine();

protected:
	virtual void set(EqAdr * dcsAdr, EqData *fromUser, EqData * toUser, EqFct * fct)__OVERRIDE__;
	virtual void get(EqAdr * dcsAdr, EqData *fromUser, EqData * toUser, EqFct * fct)__OVERRIDE__;

protected:
	int		m_nEngineNumber;
	bool	m_bIsHandled2;
};

class D_StartEngine : public D_string
{
public:
	D_StartEngine(int engineNumber, const char* pn, ::EqFct* par);
	virtual ~D_StartEngine();

protected:
	virtual void set(EqAdr * dcsAdr, EqData *fromUser, EqData * toUser, EqFct * fct)__OVERRIDE__;
	virtual void get(EqAdr * dcsAdr, EqData *fromUser, EqData * toUser, EqFct * fct)__OVERRIDE__;

protected:
	int		m_nEngineNumber;
	bool	m_bIsHandled2;
};

class D_debugLevel : public D_int
{
public:
	D_debugLevel(const char* pn, ::EqFct* par);
	virtual ~D_debugLevel();

protected:
	virtual void set(EqAdr * dcsAdr, EqData *fromUser, EqData * toUser, EqFct * fct)__OVERRIDE__;
	virtual void get(EqAdr * dcsAdr, EqData *fromUser, EqData * toUser, EqFct * fct)__OVERRIDE__;
};

class EqFctMngr : public ::EqFct
{
public:
	EqFctMngr();
	virtual ~EqFctMngr();

	void CreateNewEngine(const std::string& commandToStart);

protected:
	virtual int		fct_code(void) __OVERRIDE__;
	virtual void    update(void) __OVERRIDE__;
	virtual void	init(void) __OVERRIDE__;
	virtual void	cancel(void) __OVERRIDE__;
	void			CleanUp();
	void			ThreadFunctionForCheck();
	void			ThreadFunctionForEngineCreation();

protected:
	D_int							m_checkCycles;
	D_int							m_updateCounter;
	D_int							m_checkTimeMs;
	D_debugLevel					m_debugLevel;

	D_SingleEngine*					m_vEngines[MAX_ENGINE_NUMBERS];

	ClientTcp						m_socket2;
	common::FifoFast<std::string>	m_fifoCreate;
	common::UnnamedSemaphoreLite	m_semaCreate;
	volatile int					m_nRun;
	bool							m_vbIsHandledPrev[MAX_ENGINE_NUMBERS];
	bool							m_vbIsEngine[MAX_ENGINE_NUMBERS];
	STD::thread						m_threadCheck;
	STD::thread						m_threadCreateEngine;
	
};

}}}


#endif  // #ifndef __matlab_engine_mngr_eqfctmngr_hpp__
