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

#define	NUMBER_OF_ENGINE_FINDERS	100
#define EQFCT_MNGR_CODE				302

#include "eq_fct.h"
#include "common_defination.h"
#include "matlab_engine_clienttcp.hpp"
#include "matlab_pipe_name.h"
#include "thread_cpp11.hpp"
#include "common_fifofast.hpp"
#include "common_unnamedsemaphorelite.hpp"
#include <vector>
#include <mutex_cpp11.hpp>

namespace matlab{ namespace engine{ namespace mngr{

class D_EngineAttach : public D_string
{
public:
	D_EngineAttach(int engineNumber,const char* pn, ::EqFct* par);
	virtual ~D_EngineAttach();

protected:
	virtual void set(EqAdr * dcsAdr, EqData *fromUser, EqData * toUser, EqFct * fct)__OVERRIDE__;

protected:
	int		m_nEngineNumber;
};


class D_EngineDetach : public D_fct
{
public:
	D_EngineDetach(int engineNumber, const char* pn, ::EqFct* par);
	virtual ~D_EngineDetach();

protected:
	virtual void set(EqAdr * dcsAdr, EqData *fromUser, EqData * toUser, EqFct * fct)__OVERRIDE__;

protected:
	int		m_nEngineNumber;
};


class D_EngineStarter : public D_string
{
public:
	D_EngineStarter(const char* pn, ::EqFct* par);
protected:
	void set(EqAdr * dcsAdr, EqData *fromUser, EqData * toUser, EqFct * fct)__OVERRIDE__;
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
	enum {PRP_NONE=0,PRP_ATTACH=1,PRP_DETACH=2};
public:
	EqFctMngr();
	virtual ~EqFctMngr();

	void AddAttachProperty(int engineNumber);
	void AddDetachProperty(int engineNumber);

protected:
	void AddNewSocketPrivat(int socket, int engineNumber);
	void RemoveSocketPrivat(int indexInContainer);
	void UpdateList();

protected:
	virtual int		fct_code(void) __OVERRIDE__;
	virtual void    update(void) __OVERRIDE__;
	virtual void	init(void) __OVERRIDE__;
	virtual void	cancel(void) __OVERRIDE__;
	virtual void    names(EqAdr *, EqData *)__OVERRIDE__;
	virtual void    set(EqAdr *, EqData *, EqData *)__OVERRIDE__;
	void			CleanUp();
	void			ThreadFunctionMonitorExistingEngines();
	void			ThreadFunctionFindNewEngine(int threadIndex);

protected:
	D_debugLevel					m_debugLevel;
	D_EngineStarter					m_newEngineStarter;

	D_EngineAttach*					m_vEngineAttachs2[MAX_ENGINE_NUMBERS];
	D_EngineDetach*					m_vEngineDetachs2[MAX_ENGINE_NUMBERS];

	ClientTcp						m_socketInitial3;
	std::vector<int>				m_vEngineSockets3;
	std::vector<int>				m_vEngineSocketsIndexes3;
	volatile int					m_nRun;
	int								m_vnAttachOrDetach[MAX_ENGINE_NUMBERS];
	bool							m_vbIsEngine[MAX_ENGINE_NUMBERS];
	STD::mutex						m_mutexForSockets;
	STD::thread						m_threadMonitorExistingEngines;
	STD::thread						m_vThreadFindNewEngines[NUMBER_OF_ENGINE_FINDERS];
	
};

}}}


#endif  // #ifndef __matlab_engine_mngr_eqfctmngr_hpp__
