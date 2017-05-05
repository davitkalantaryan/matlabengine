/*****************************************************************************
 * File   : matlab_engine_mngr_eqfctmngr.cpp
 * created: 2017 May 02
 *****************************************************************************
 * Author:	D.Kalantaryan, Tel:+49(0)33762/77552 kalantar
 * Email :	davit.kalantaryan@desy.de
 * Mail  :	DESY, Platanenallee 6, 15738 Zeuthen
 *****************************************************************************
 * Description
 *   ...
 ****************************************************************************/

#include "matlab_engine_mngr_eqfctmngr.hpp"
#include "matlab_engine_mngr_eqfctproxy.hpp"
#include "matlab_pipe_name.h"

int g_nDebugLevel = 0;

matlab::engine::mngr::EqFctMngr::EqFctMngr()
	:
	::EqFct("NAME = location"),
	
	m_checkCycles("CHECK.CYCLES",this),
	m_updateCounter("UPDATE.COUNTER",this),
	m_checkTimeMs("CHECK.TIME.MS",this),
	m_debugLevel("DEBUG.LEVEL", this),

	m_fifoCreate(8),
	m_threadCheck(&EqFctMngr::ThreadFunctionForCheck,this),
	m_threadCreateEngine(&EqFctMngr::ThreadFunctionForEngineCreation,this)
{
	m_checkCycles.set_value(10);
	memset(m_vEngines, 0, sizeof(m_vEngines));
	memset(m_vbIsHandledPrev, 0, sizeof(m_vbIsHandledPrev));
	memset(m_vbIsEngine, 0, sizeof(m_vbIsEngine));
}


matlab::engine::mngr::EqFctMngr::~EqFctMngr()
{
	CleanUp();
}


int matlab::engine::mngr::EqFctMngr::fct_code(void)
{
	return EQFCT_MNGR_CODE;
}

void matlab::engine::mngr::EqFctMngr::cancel(void)
{
	CleanUp();
}

void matlab::engine::mngr::EqFctMngr::update(void)
{
	int nUpdateCounter = m_updateCounter.value();
	int nCheckCycles = m_checkCycles.value();
	if (++nUpdateCounter >= nCheckCycles)
	{
		char vcHostName[64];
		char vcPropName[128];

		gethostname(vcHostName, 127);
		
		for (int i(0); i < MAX_ENGINE_NUMBERS; ++i)
		{

			if (!m_vbIsEngine[i] && m_vEngines[i]) {
				rem_property(m_vEngines[i]);
				delete m_vEngines[i];
				m_vEngines[i] = NULL;
			}
			else if (m_vbIsEngine[i] && !m_vEngines[i]) {
				m_vbIsHandledPrev[i] = EqFctProxy::EngineControled2(i)?true:false;
				if (m_vbIsHandledPrev[i])	{snprintf(vcPropName, 127, "DETACH.%s:%d", vcHostName,i);}
				else						{snprintf(vcPropName, 127, "ATTACH.%s:%d", vcHostName,i); }
				m_vEngines[i] = new D_SingleEngine(i, vcPropName,NULL);
				add_property(m_vEngines[i]);
			}

			if (m_vEngines[i]) {
				if (m_vbIsHandledPrev[i] && (!EqFctProxy::EngineControled2(i))) {
					rem_property(m_vEngines[i]);
					delete m_vEngines[i];
					snprintf(vcPropName, 127, "U_ENGINE_%.3d", i);
					m_vEngines[i] = new D_SingleEngine(i, vcPropName, NULL);
					add_property(m_vEngines[i]);
				}// if (m_vbIsHandledPrev[i] && (!EqFctProxy::EngineControled(i))) {
				else if ((!m_vbIsHandledPrev[i]) && EqFctProxy::EngineControled2(i)) {
					rem_property(m_vEngines[i]);
					delete m_vEngines[i];
					snprintf(vcPropName, 127, "H_ENGINE_%.3d", i);
					m_vEngines[i] = new D_SingleEngine(i, vcPropName, NULL);
					add_property(m_vEngines[i]);
				}// if (m_vbIsHandledPrev[i] && (!EqFctProxy::EngineControled(i))) {

				m_vbIsHandledPrev[i] = EqFctProxy::EngineControled2(i)?true:false;;
			} // if (m_vEngines[i]) {

		}
		nUpdateCounter = 0;
	}

	m_updateCounter.set_value(nUpdateCounter);
}

void  matlab::engine::mngr::EqFctMngr::init(void)
{
}


void matlab::engine::mngr::EqFctMngr::CleanUp()
{
	if (m_nRun == 0) { return; }
	m_nRun = 0;
	m_semaCreate.post();
	m_threadCheck.join();
	m_threadCreateEngine.join();

	for (int i(0); i < MAX_ENGINE_NUMBERS; ++i)
	{
		if (m_vEngines[i]) {
			rem_property(m_vEngines[i]);
			delete m_vEngines[i]; m_vEngines[i]=NULL;
		}
	}
}


void  matlab::engine::mngr::EqFctMngr::CreateNewEngine(const std::string& a_commandToStart)
{
	m_fifoCreate.AddElement(a_commandToStart);
	m_semaCreate.post();
}


void  matlab::engine::mngr::EqFctMngr::ThreadFunctionForCheck()
{
	int i, nConnect;
	long lnTimeout;

	m_nRun = 1;

	while (m_nRun)
	{
		for (i = 0; i < MAX_ENGINE_NUMBERS; ++i)
		{
			nConnect = m_socket2.ConnectToServer("localhost", i);
			m_vbIsEngine[i] = (nConnect == 0);
			m_socket2.Close();
		} // for (int i(0); i < MAX_ENGINE_NUMBERS; ++i)
		lnTimeout = (long)(m_checkTimeMs.value());
		lnTimeout = lnTimeout <= 0 ? 1 : lnTimeout;
		Sleep(lnTimeout);
	} // while (m_nRun)
}


void  matlab::engine::mngr::EqFctMngr::ThreadFunctionForEngineCreation()
{
	std::string strCommand;

	m_nRun = 1;

	while (m_nRun)
	{
		m_semaCreate.wait();
		while (m_fifoCreate.Extract(&strCommand))
		{
			system(strCommand.c_str());
		}
	} // while (m_nRun)
}


/*//////////////////////////////////////*/
matlab::engine::mngr::D_SingleEngine::D_SingleEngine(int a_nEngineNumber,const char* a_pn, ::EqFct* a_par)
	:
	D_string(a_pn,a_par)
{
	m_nEngineNumber = a_nEngineNumber;
	m_bIsHandled2 = false;
}


matlab::engine::mngr::D_SingleEngine::~D_SingleEngine()
{
}


void matlab::engine::mngr::D_SingleEngine::set(
	EqAdr * a_dcsAdr,
	EqData * a_fromUser,
	EqData * a_toUser,
	EqFct * a_fct
)
{
	EqFctProxy* pProxy = EqFctProxy::EngineControled2(m_nEngineNumber);
	if (!pProxy && !m_bIsHandled2)
	{
		std::string strUserStr = a_fromUser->get_string();
		DEBUG_APP_RAW2(1,"!!!!!!!!!!!!!!strUserStr=%s\n", strUserStr.c_str());

        if (strUserStr == "") {
			char vcHostName[128];
			char vcBuffer[128];

			gethostname(vcHostName, 127);
			snprintf(vcBuffer, 127, "%s:%d", vcHostName, m_nEngineNumber);
			strUserStr = vcBuffer;
		}

		add_location(EQFCT_PROXY_CODE, strUserStr.c_str(), 
			(void*)((size_t)(m_nEngineNumber+1)));
		m_bIsHandled2 = true;

		D_string::set(a_dcsAdr, a_fromUser, a_toUser, a_fct);
	}
	else if (pProxy && m_bIsHandled2)
	{
		rem_location(pProxy->name_str());
	}
}


void matlab::engine::mngr::D_SingleEngine::get(
	EqAdr * a_dcsAdr,
	EqData * a_fromUser,
	EqData * a_toUser,
	EqFct * a_fct
)
{
	D_string::get(a_dcsAdr, a_fromUser, a_toUser, a_fct);
}


/*//////////////////////////////////////*/
matlab::engine::mngr::D_StartEngine::D_StartEngine(int a_nEngineNumber, const char* a_pn, ::EqFct* a_par)
	:
	D_string(a_pn, a_par)
{
	m_nEngineNumber = a_nEngineNumber;
	m_bIsHandled2 = false;
}


matlab::engine::mngr::D_StartEngine::~D_StartEngine()
{
}

#ifdef WIN32
#define DEF_COMMAND	"matlab_engine_server"
#else
#define DEF_COMMAND	"matlab_engine_server"
#endif


void matlab::engine::mngr::D_StartEngine::set(
	EqAdr *a_adr,
	EqData * a_fromUser,
	EqData *a_toUser,
	EqFct * a_fct
)
{
	EqFctMngr* pParent = (EqFctMngr*)a_fct;
	std::string strOldValue = value();
	std::string strUserStr = a_fromUser->get_string();

	if(strUserStr==""){if(strOldValue!=""){strUserStr=strOldValue;}else{strUserStr=DEF_COMMAND;}}
	pParent->CreateNewEngine(strUserStr);

	D_string::set(a_adr, a_fromUser, a_toUser, a_fct);
}


void matlab::engine::mngr::D_StartEngine::get(
	EqAdr *a_adr,
	EqData *a_fromUser,
	EqData * a_toUser,
	EqFct * a_fct
)
{
	D_string::get(a_adr, a_fromUser, a_toUser, a_fct);
}



/*//////////////////////////////////////*/
matlab::engine::mngr::D_debugLevel::D_debugLevel(const char* a_pn, ::EqFct* a_par)
	:
	D_int(a_pn, a_par)
{
}


matlab::engine::mngr::D_debugLevel::~D_debugLevel()
{
}


void matlab::engine::mngr::D_debugLevel::set(
	EqAdr *a_adr,
	EqData * a_fromUser,
	EqData *a_toUser,
	EqFct * a_fct
)
{
	int nNewDebugLevel = a_fromUser->get_int();
	
	g_nDebugLevel = nNewDebugLevel;
	D_int::set(a_adr,a_fromUser,a_toUser,a_fct);
}


void matlab::engine::mngr::D_debugLevel::get(
	EqAdr *a_adr,
	EqData *a_fromUser,
	EqData * a_toUser,
	EqFct * a_fct
)
{
	D_int::get(a_adr, a_fromUser, a_toUser, a_fct);
	a_toUser->set(g_nDebugLevel);
}
