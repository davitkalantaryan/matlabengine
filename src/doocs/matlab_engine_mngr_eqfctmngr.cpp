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
#include "thread_cpp11.hpp"
#include <errno.h>

int g_nDebugLevel = 0;

matlab::engine::mngr::EqFctMngr::EqFctMngr()
	:
	::EqFct("NAME = location"),
	
	m_debugLevel("DEBUG.LEVEL", this),
	m_newEngineStarter("START.NEW.ENGINE start new engine on host", this)
{
	EqFctProxy::InitProxies();
	memset(m_vEngineAttachs2, 0, sizeof(m_vEngineAttachs2));
	memset(m_vEngineDetachs2, 0, sizeof(m_vEngineDetachs2));
	memset(m_vnAttachOrDetach, 0, sizeof(m_vnAttachOrDetach));
	memset(m_vbIsEngine, 0, sizeof(m_vbIsEngine));

	if(m_socketInitial3.CreateSocket()) { throw "Unable to create socket!"; }
	AddNewSocketPrivat((int)m_socketInitial3, 0);

	m_nRun = 1;

	m_threadMonitorExistingEngines=STD::thread(&EqFctMngr::ThreadFunctionMonitorExistingEngines,this);

	for (int i(0);i<NUMBER_OF_ENGINE_FINDERS;++i)
	{
		m_vThreadFindNewEngines[i]=STD::thread(&EqFctMngr::ThreadFunctionFindNewEngine,this,i);
	}
}


matlab::engine::mngr::EqFctMngr::~EqFctMngr()
{
	CleanUp();
	EqFctProxy::DestroyProxies();
}


int matlab::engine::mngr::EqFctMngr::fct_code(void)
{
	return EQFCT_MNGR_CODE;
}

void matlab::engine::mngr::EqFctMngr::cancel(void)
{
	CleanUp();
}


void matlab::engine::mngr::EqFctMngr::names(EqAdr * a_dcsAdr, EqData * a_eqData)
{
	UpdateList();
	DEBUG_APP_RAW2(1, "!!!!!!!!!!!! names\n");
	::EqFct::names(a_dcsAdr, a_eqData);
}


void matlab::engine::mngr::EqFctMngr::set(EqAdr * a_dcsAdr, EqData * a_fromUser,EqData * a_toUser)
{
	UpdateList();
	//printf("!!!!!!!!!!!!!!!!!!!!!!!");
	DEBUG_APP_RAW2(0, "!!!!!!!!!!!!!!!!!!!!! set\n");
	::EqFct::set(a_dcsAdr,a_fromUser,a_toUser);
}


void matlab::engine::mngr::EqFctMngr::update(void)
{
	UpdateList();
}

void  matlab::engine::mngr::EqFctMngr::init(void)
{
	DEBUG_APP_RAW2(1, "!!!!!");
}


void  matlab::engine::mngr::EqFctMngr::UpdateList()
{
	if(!g_bUpdateNeeded){return;}
	g_bUpdateNeeded = false;

	for (int i(0); i < MAX_ENGINE_NUMBERS; ++i) {
		if (!m_vbIsEngine[i]) {
			if(m_vnAttachOrDetach[i]==PRP_ATTACH){rem_property(m_vEngineAttachs2[i]);}
			else if(m_vnAttachOrDetach[i]==PRP_DETACH){rem_property(m_vEngineDetachs2[i]);}
		}  // if (!m_vbIsEngine[i]){
		else if (m_vnAttachOrDetach[i]==PRP_NONE) {
			if (EqFctProxy::EngineControled2(i)) { AddDetachProperty(i); }
			else { AddAttachProperty(i); }
		} // else if (m_vnAttachOrDetach[i] ==PRP_NONE) { for if (!m_vbIsEngine[i]){
	} // for (int i(0); i < MAX_ENGINE_NUMBERS; ++i) {
}


void matlab::engine::mngr::EqFctMngr::CleanUp()
{
	int i;
	if (m_nRun == 0) { return; }
	m_nRun = 0;

	for(i=0;i<NUMBER_OF_ENGINE_FINDERS;++i){m_vThreadFindNewEngines[i].join();}

	m_socketInitial3.Close();
	m_threadMonitorExistingEngines.join();
	
	for (int i(0); i < MAX_ENGINE_NUMBERS; ++i)
	{
		if(m_vnAttachOrDetach[i]==PRP_ATTACH){rem_property(m_vEngineAttachs2[i]);}
		else if(m_vnAttachOrDetach[i]==PRP_DETACH){rem_property(m_vEngineDetachs2[i]);}

		if(m_vEngineAttachs2[i]){delete m_vEngineAttachs2[i];m_vEngineAttachs2[i]=NULL;}
		if(m_vEngineDetachs2[i]){delete m_vEngineDetachs2[i];m_vEngineDetachs2[i]=NULL;}
	}
}


void matlab::engine::mngr::EqFctMngr::AddAttachProperty(int a_nEngineNumber)
{
	if(!m_vEngineAttachs2[a_nEngineNumber]){
		char vcPropName[128];
		char vcHostName[64];
		gethostname(vcHostName, 127);
		snprintf(vcPropName, 127, "ATTACH.%s:%d", vcHostName, a_nEngineNumber);
		m_vEngineAttachs2[a_nEngineNumber] = new D_EngineAttach(a_nEngineNumber, vcPropName, NULL);
	}
	
	if(m_vEngineAttachs2[a_nEngineNumber]){
		add_property(m_vEngineAttachs2[a_nEngineNumber]);
		m_vnAttachOrDetach[a_nEngineNumber] = PRP_ATTACH;
	}
}


void matlab::engine::mngr::EqFctMngr::AddDetachProperty(int a_nEngineNumber)
{
	if (!m_vEngineDetachs2[a_nEngineNumber]) {
		char vcPropName[128];
		char vcHostName[64];
		gethostname(vcHostName, 127);
		snprintf(vcPropName, 127, "DETACH.%s:%d", vcHostName, a_nEngineNumber);
		m_vEngineDetachs2[a_nEngineNumber] = new D_EngineDetach(a_nEngineNumber, vcPropName, NULL);
	}

	if (m_vEngineDetachs2[a_nEngineNumber]) { 
		add_property(m_vEngineDetachs2[a_nEngineNumber]); 
		m_vnAttachOrDetach[a_nEngineNumber] = PRP_DETACH;
	}
}


void matlab::engine::mngr::EqFctMngr::AddNewSocketPrivat(int a_nSocket, int a_nEngineNumber)
{
	m_vEngineSockets3.push_back(a_nSocket);
	m_vEngineSocketsIndexes3.push_back(a_nEngineNumber);
}


void matlab::engine::mngr::EqFctMngr::RemoveSocketPrivat(int a_nIndexInContainer)
{
	m_vEngineSockets3.erase(m_vEngineSockets3.begin() + a_nIndexInContainer);
	m_vEngineSocketsIndexes3.erase(m_vEngineSocketsIndexes3.begin() + a_nIndexInContainer);
}


void  matlab::engine::mngr::EqFctMngr::ThreadFunctionMonitorExistingEngines()
{
	std::vector<int> vSockets, vSocketsEmpty;
	std::vector<char> vReturn(1);
	int nSockMax,nFounded,nReturn;
	size_t i, unRetSize;

	vSockets = m_vEngineSockets3;

	while (m_nRun)
	{
		//if(vSockets.size()<unNumberOfSockets){vSockets.resize(unNumberOfSockets);}
		
		nSockMax = -1;
#ifdef WIN32
		nReturn=ClientTcp::selectMltCpp(-1,&nSockMax, vSockets,vSocketsEmpty, vSocketsEmpty, vReturn);
#else
		nReturn=ClientTcp::selectMltCpp(-1,&nSockMax,vSocketsEmpty,vSocketsEmpty,vSockets,vReturn);
#endif
		if (nReturn<=0){
			fprintf(stderr,"!!!!! fl:%s,ln:%d -> returned not positive lasterr=%d!\n",
				__FILE__,__LINE__,
#ifdef WIN32
				(int)GetLastError()
#else
				(int)errno
#endif
			);
			Sleep(500);
			continue;
		}
		nFounded = vReturn[0] ? 1 : 0;
		unRetSize = vReturn.size();
		m_mutexForSockets.lock();
		for(i=1;(nFounded<nReturn)&&(i<unRetSize);++i){
			if(vReturn[i]){
				ClientTcp::CloseSt(vSockets[i]);
				++nFounded;
				m_vbIsEngine[m_vEngineSocketsIndexes3[i]] = false;
				RemoveSocketPrivat(i);
			}
		}
		vSockets = m_vEngineSockets3;
		m_mutexForSockets.unlock();
//#ifdef WIN32
		g_bUpdateNeeded = true;
		printf("!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n");
//#endif
	} // while (m_nRun)
}


void  matlab::engine::mngr::EqFctMngr::ThreadFunctionFindNewEngine(int a_nThreadIndex)
{
	ClientTcp socketToSearch, socketInitialDublicate;
	const int _cnDenomi = MAX_ENGINE_NUMBERS%NUMBER_OF_ENGINE_FINDERS;
	const int _cnForOneThread = _cnDenomi?
		(MAX_ENGINE_NUMBERS / (NUMBER_OF_ENGINE_FINDERS - 1)):
		(MAX_ENGINE_NUMBERS / NUMBER_OF_ENGINE_FINDERS);
	const int cnFrom(a_nThreadIndex*_cnForOneThread), _cnMaxEnd(cnFrom + _cnForOneThread);
	const int cnTo(_cnMaxEnd>MAX_ENGINE_NUMBERS? MAX_ENGINE_NUMBERS: _cnMaxEnd);
	int i, nConnect;
	int nSocket;
	int nIterations;

	while (m_nRun)
	{
		nIterations = 0;
		for (i = cnFrom; (i < cnTo) && m_nRun; ++i)
		{
			if(!m_vbIsEngine[i]){
				nConnect = socketToSearch.ConnectToServer("localhost", i);
				if(nConnect==0){
					m_vbIsEngine[i] = true;
					nSocket = (int)socketToSearch;
					socketToSearch.SetSockDescriptor(-1);
					
					m_mutexForSockets.lock();
					
					AddNewSocketPrivat(nSocket, i);
					socketInitialDublicate.SetSockDescriptor((int)m_socketInitial3);
					m_socketInitial3.SetSockDescriptor(-1);
					m_socketInitial3.CreateSocket();
					m_vEngineSockets3[0] = (int)m_socketInitial3;
					socketInitialDublicate.Close();
					g_bUpdateNeeded = true;

					m_mutexForSockets.unlock();
				}
				else { socketToSearch.Close();}
				++nIterations;
			}//if(!m_vbIsEngine[i]){
		} // for (int i(0); i < MAX_ENGINE_NUMBERS; ++i)
		if (nIterations == 0) { Sleep(1000); }
		Sleep(1);
	} // while (m_nRun)
}


/*//////////////////////////////////////*/
matlab::engine::mngr::D_EngineAttach::D_EngineAttach(
	int a_nEngineNumber,const char* a_pn, ::EqFct* a_par)
	:
	D_string(a_pn,a_par)
{
	m_nEngineNumber = a_nEngineNumber;
}


matlab::engine::mngr::D_EngineAttach::~D_EngineAttach()
{
}


void matlab::engine::mngr::D_EngineAttach::set(
	EqAdr * a_dcsAdr,
	EqData * a_fromUser,
	EqData * a_toUser,
	EqFct * a_fct
)
{
	EqFctMngr* pMngr = (EqFctMngr*)a_fct;

	std::string strUserStr = a_fromUser->get_string();
	DEBUG_APP_RAW2(1, "!!!!!!!!!!!!!!strUserStr=%s\n", strUserStr.c_str());

	if (strUserStr == "") {
		char vcHostName[128];
		char vcBuffer[128];

		gethostname(vcHostName, 127);
		snprintf(vcBuffer, 127, "%s:%d", vcHostName, m_nEngineNumber);
		strUserStr = vcBuffer;
	}

	add_location(EQFCT_PROXY_CODE, strUserStr.c_str(),(void*)((size_t)(m_nEngineNumber+1)));
	D_string::set(a_dcsAdr, a_fromUser, a_toUser, a_fct);
	pMngr->rem_property(this);
	pMngr->AddDetachProperty(m_nEngineNumber);
}


/*//////////////////////////////////////*/
matlab::engine::mngr::D_EngineDetach::D_EngineDetach(
	int a_nEngineNumber, const char* a_pn, ::EqFct* a_par)
	:
	D_fct(a_pn, a_par)
{
	m_nEngineNumber = a_nEngineNumber;
}


matlab::engine::mngr::D_EngineDetach::~D_EngineDetach()
{
}


void matlab::engine::mngr::D_EngineDetach::set(
	EqAdr * a_dcsAdr,
	EqData * a_fromUser,
	EqData * a_toUser,
	EqFct * a_fct
)
{
	EqFctMngr* pMngr = (EqFctMngr*)a_fct;
	EqFctProxy* pProxy = EqFctProxy::EngineControled2(m_nEngineNumber);

	if (!pProxy) { fprintf(stderr,"NULL engine!\n");return; }
	rem_location(pProxy->name_str());
	D_fct::set(a_dcsAdr, a_fromUser, a_toUser, a_fct);

	pMngr->rem_property(this);
	pMngr->AddAttachProperty(m_nEngineNumber);
}


/*//////////////////////////////////////*/
//D_EngineStarter
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



/*//////////////////////////////////////*/
//D_EngineStarter
matlab::engine::mngr::D_EngineStarter::D_EngineStarter(const char* a_pn, ::EqFct* a_par)
	:
	D_string(a_pn, a_par)
{
}


void matlab::engine::mngr::D_EngineStarter::set(
	EqAdr *a_adr,
	EqData * a_fromUser,
	EqData *a_toUser,
	EqFct * a_fct
)
{
	D_string::set(a_adr, a_fromUser, a_toUser, a_fct);

	std::string strUserStr = a_fromUser->get_string();
	EqFctProxy::StartNewEngine(strUserStr);
}
