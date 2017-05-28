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


#ifdef WIN32
#include <windows.h>
#define DEF_COMMAND	"matlab_engine_server --engine-number 0"
#else
#ifndef Sleep
#define Sleep(_x_) usleep(1000*(_x_))
#endif
#define DEF_COMMAND	"matlab_engine_server --engine-number 0"
#endif

#include "matlab_engine_mngr_eqfctproxy.hpp"
#include "matlab_pipe_name.h"
#include "common_fifofast.hpp"
#include "common_unnamedsemaphorelite.hpp"
#include "thread_cpp11.hpp"


typedef const void* TypeConstVoidPtr;

#define _SERI_RAW1 matlab::engine::versioning::SERI_TYPE::RAW1

static common::FifoFast<std::string>	s_fifoCreate(8);
static common::UnnamedSemaphoreLite		s_semaCreate;
static STD::thread						s_threadCreateEngine;
static volatile int						s_nRunCreatorThread = 0;

bool g_bUpdateNeeded = false;

static bool s_bIsInited = false;
static matlab::engine::mngr::EqFctProxy* s_pProxys[MAX_ENGINE_NUMBERS];

static void ThreadFunctionForEngineCreation(void)
{
	std::string strCommand;
	s_nRunCreatorThread = 1;
	while (s_nRunCreatorThread){
		s_semaCreate.wait();
		while (s_fifoCreate.Extract(&strCommand)){system(strCommand.c_str());}
	} // while (s_nRunCreatorThread)
}


int matlab::engine::mngr::EqFctProxy::InitProxies()
{
	if (!s_bIsInited) { 
		s_bIsInited = true;
		memset(s_pProxys, 0, sizeof(s_pProxys));
		s_threadCreateEngine = STD::thread(&ThreadFunctionForEngineCreation);
	}

	return 0;
}


void matlab::engine::mngr::EqFctProxy::DestroyProxies()
{
	if (s_bIsInited) {
		s_bIsInited = false;
		s_nRunCreatorThread = 0;
		s_semaCreate.post();
		s_threadCreateEngine.join();
	}
}

matlab::engine::mngr::EqFctProxy::EqFctProxy(int a_nEngineNumber)
	:
	::EqFct("NAME = location"),

	m_matlabScriptName("MATLAB.SCRIPT.NAME",this),
	m_numberOfExpectedOutputs("NUMBER.OF.EXPECTED.OUTS",this),
	m_numberOfLastOutputs("NUMBER.OF.OUTS",this),
    m_engineNumber("ENGINE.NUMBER",this),
	m_engineStartCommand("COMMAND.STARTED.ENGINE",this),
	m_isConnected2("IS.CONNECTED",this),
	m_recvTimeoutMs("RECEIVE.TIMEOUT.MS  <0 waits forever",this),
	m_engineStarter("START.ENGINE",this),
	m_maxTryCount("MAX.TRY.COUNT",this),

	m_argVoid("ARG.VOID", this),
	m_argInt("ARG.INT",this),
	m_argFloat("ARG.FLOAT",this),
	m_argString("ARG.STRING", this),
	m_argBool("ARG.BOOL", this),
	m_argDouble("ARG.DOUBLE",this),
	m_argSpectrum("ARG.SPECTRUM not implemented yet",1024,this),
	m_expression("MAT.EXPRESSION provide expression to evaluate in MATLAB",this),

    m_vArray(MAXIMUM_NUMBER_OF_IN_AND_OUTS),
    m_serializeDes(NULL, CURRENT_SERIALIZER_VERSION2, _SERI_RAW1)
{
	//ftime(&m_lastStartTime);
	m_lastStartTime.time = 0;
	m_lastStartTime.millitm = 0;
	m_nNumOfTryOfStart = 0;
	m_bIsConnected2 = false;
	m_bAnyResultsExist = false;
	InitProxies();
	m_bPendingTask = false;
	if ((a_nEngineNumber <= 0) || (a_nEngineNumber > MAX_ENGINE_NUMBERS)){m_nEngineNumber = -1;}
	else { m_nEngineNumber = a_nEngineNumber-1;s_pProxys[m_nEngineNumber] = this; }
}


matlab::engine::mngr::EqFctProxy::~EqFctProxy()
{
	if((m_nEngineNumber>=0)||(m_nEngineNumber<MAX_ENGINE_NUMBERS)){s_pProxys[m_nEngineNumber]=NULL;}
}


int matlab::engine::mngr::EqFctProxy::fct_code(void)
{
	return EQFCT_PROXY_CODE;
}

void matlab::engine::mngr::EqFctProxy::update(void)
{
	TryConnect();
}

void  matlab::engine::mngr::EqFctProxy::init(void)
{
	int nReturn(-1);
	if ((m_nEngineNumber < 0) || (m_nEngineNumber >= MAX_ENGINE_NUMBERS)) {
		int nEngNumberDoocs = m_engineNumber.value();
		if((nEngNumberDoocs<0) || (nEngNumberDoocs>= MAX_ENGINE_NUMBERS)){
			set_error(1, "Wrong engine number");
			return;
		}
		m_nEngineNumber = nEngNumberDoocs;
		s_pProxys[m_nEngineNumber] = this;
		g_bUpdateNeeded = true;
	}
	else {
		m_engineNumber.set_value(m_nEngineNumber);
	}

	sys_mask_.set_value(2);
	m_isConnected2.set_ro_access();
	TryConnect();
	
}

void  matlab::engine::mngr::EqFctProxy::cancel(void)
{
	if((m_nEngineNumber>=0)||(m_nEngineNumber<MAX_ENGINE_NUMBERS)){s_pProxys[m_nEngineNumber]=NULL;}
}


void matlab::engine::mngr::EqFctProxy::set(EqAdr * a_dcsAdr, EqData * a_fromUser, EqData * a_toUser)
{
	DEBUG_APP_RAW2(1, "!!!!!!!!!!!!!!!!!!!!! set\n");
	::EqFct::set(a_dcsAdr, a_fromUser, a_toUser);
}


void matlab::engine::mngr::EqFctProxy::get(EqAdr * a_dcsAdr,EqData * a_fromUser, EqData * a_toUser)
{
	DEBUG_APP_RAW2(1, "!!!!!!!!!!!!!!!!!!!!! set\n");
	::EqFct::get(a_dcsAdr, a_fromUser, a_toUser);
}



void matlab::engine::mngr::EqFctProxy::GetForProperties(EqData * a_fromUser, EqData * a_toUser)
{
	int nReturn;
	DEBUG_APP_RAW2(0, "!!!!!!!!!!!!!!!!!!!!! set\n");

	if (!m_bPendingTask) {
		if (m_bAnyResultsExist) {
			/*send to user last result*/
			GetForPropertiesPrivate(a_fromUser, a_toUser);
		}
		else {
			/*warn user that there is no results*/
		}
		return;
	}

	nReturn = ReceiveResultFromMatlab();
	if (nReturn == _SOCKET_TIMEOUT_) {/*warning about timeout*/return; }
	else if (nReturn) {/*send error to user*/return; }

	GetForPropertiesPrivate(a_fromUser, a_toUser);
}


void matlab::engine::mngr::EqFctProxy::GetForPropertiesPrivate(
	EqData*,//a_fromUser 
	EqData * a_toUser)
{
#define DEBUG_LEVEL 0

#if 0
#define	DATA_NULL               0
#define	DATA_INT                1
#define	DATA_FLOAT              2
#define	DATA_STRING             3
#define	DATA_BOOL               4
#define	DATA_DOUBLE             6
#define	DATA_SPECTRUM           19
#endif // #if 0

	if(m_numberOfLastOutputs.value()<1){return;}

	switch (a_toUser->type())
	{
	case DATA_NULL:
		DEBUG_APP_RAW2(DEBUG_LEVEL, "DATA_NULL\n");
		break;
	case DATA_INT:
	{
		DEBUG_APP_RAW2(DEBUG_LEVEL, "DATA_INT\n");

		int nValue;
		m_vArray.GetValue(0, &nValue);
		a_toUser->set(nValue);
	}
	break;
	case DATA_FLOAT:
	{
		DEBUG_APP_RAW2(DEBUG_LEVEL, "DATA_FLOAT\n");

		float nValue;
		m_vArray.GetValue(0, &nValue);
		a_toUser->set(nValue);
	}
	break;
	case DATA_STRING:
	{
		DEBUG_APP_RAW2(DEBUG_LEVEL, "DATA_STRING\n");

		char nValue[512];
		m_vArray.GetValue(0, nValue,511);
		a_toUser->set(nValue);
	}
	break;
	case DATA_BOOL:
	{
		DEBUG_APP_RAW2(DEBUG_LEVEL, "DATA_BOOL\n");

		int nValue;
		m_vArray.GetValue(0, &nValue);
		a_toUser->set(nValue);
	}
	break;
	case DATA_DOUBLE:
	{
		DEBUG_APP_RAW2(DEBUG_LEVEL, "DATA_DOUBLE\n");

		double nValue;
		m_vArray.GetValue(0, &nValue);
		a_toUser->set(nValue);
	}
	break;
	case DATA_SPECTRUM:
		DEBUG_APP_RAW2(DEBUG_LEVEL, "DATA_SPECTRUM\n");
		break;
	default:
		break;
	}
}


void matlab::engine::mngr::EqFctProxy::SetForProperties(EqData * a_fromUser, EqData * /*a_toUser*/)
{
#if 0
#define	DATA_NULL               0
#define	DATA_INT                1
#define	DATA_FLOAT              2
#define	DATA_STRING             3
#define	DATA_BOOL               4
#define	DATA_DOUBLE             6
#define	DATA_SPECTRUM           19
#endif // #if 0
	switch (a_fromUser->type())
	{
	case DATA_NULL:
		SendCommandToMatlab(0);
		break;
	case DATA_INT:
	{
		int nValue = a_fromUser->get_int();
		m_vArray.SetValue(0, nValue);
		SendCommandToMatlab(1);
	}
	break;
	case DATA_FLOAT:
	{
		float nValue = a_fromUser->get_float();
		m_vArray.SetValue(0, nValue);
		SendCommandToMatlab(1);
	}
	break;
	case DATA_STRING:
	{
		std::string nValue = a_fromUser->get_string();
		m_vArray.SetValue(0, nValue.c_str());
		SendCommandToMatlab(1);
	}
	break;
	case DATA_BOOL:
	{
		bool nValue = (bool)a_fromUser->get_int();
		m_vArray.SetValue(0, nValue);
		SendCommandToMatlab(1);
	}
	break;
	case DATA_DOUBLE:
	{
		double nValue = a_fromUser->get_double();
		m_vArray.SetValue(0, nValue);
		SendCommandToMatlab(1);
	}
	break;
	case DATA_SPECTRUM:
		break;
	default:
		break;
	}
}


void  matlab::engine::mngr::EqFctProxy::StartNewEngine(const std::string& a_commandToStart)
{
	s_fifoCreate.AddElement(a_commandToStart);
	s_semaCreate.post();
}


matlab::engine::mngr::EqFctProxy* matlab::engine::mngr::EqFctProxy::EngineControled2(int a_nEngine)
{
	if (!s_bIsInited) { memset(s_pProxys, 0, sizeof(s_pProxys)); s_bIsInited = true; }
	matlab::engine::mngr::EqFctProxy* pRet =
		a_nEngine < MAX_ENGINE_NUMBERS ? s_pProxys[a_nEngine] : NULL;
	return pRet;
}


int  matlab::engine::mngr::EqFctProxy::SendCommandToMatlab(int a_nNumOfInputs, const char* a_cpcScriptName)
{
	int nReceived;
	int32_ttt nExpectedOuts;
	const char* cpcScrName = ((a_cpcScriptName==NULL)||(a_cpcScriptName[0]==0)) ? 
		m_matlabScriptName.value(): a_cpcScriptName;
	
	if (m_bPendingTask) {
		ReceiveResultFromMatlab();
		if (m_bPendingTask){/* warn about busy*/ return -1;}
	}

	if(!m_bIsConnected2){ m_nNumOfTryOfStart=0;TryConnect();}

	a_nNumOfInputs = a_nNumOfInputs > MAXIMUM_NUMBER_OF_IN_AND_OUTS ?
		MAXIMUM_NUMBER_OF_IN_AND_OUTS : a_nNumOfInputs;
	//cpcScrName = m_matlabScriptName.value();
	nExpectedOuts = m_numberOfExpectedOutputs.value();

	m_numberOfLastOutputs.set_value(0);
	m_bPendingTask = true;
	nReceived = m_serializeDes.SendScriptNameAndArrays(
		&m_vFuncs, &m_socketTcp,
		cpcScrName, nExpectedOuts, a_nNumOfInputs, 
		(TypeConstVoidPtr*)(m_vArray.Array()));

	if (nReceived < 0) {
		m_socketTcp.Close();
		m_bIsConnected2 = false;
		m_isConnected2.set_value(0);
		TryConnect();
		return nReceived;
	}

	return 0;
}


int  matlab::engine::mngr::EqFctProxy::ReceiveResultFromMatlab()
{
	int nReceived(-1);
	int32_ttt nOutputs;
	int lnTimeoutMs = m_recvTimeoutMs.value();

	if (!m_bPendingTask) {
		return -2;
	}
	
	nReceived = m_serializeDes.ReceiveHeaderScriptNameAndArrays2(
		&m_vFuncs, &m_socketTcp, lnTimeoutMs,
		MAXIMUM_NUMBER_OF_IN_AND_OUTS, (void**)(m_vArray.Array()), &nOutputs);

	if (nReceived == _SOCKET_TIMEOUT_) { return _SOCKET_TIMEOUT_; }
	else if (nReceived < 0) {
		m_socketTcp.Close();
		m_bIsConnected2 = false;
		m_isConnected2.set_value(0);
		TryConnect();
		if (nReceived) {/* setError */}
		m_bPendingTask = false;
		return -2;
	}

	m_numberOfLastOutputs.set_value(nOutputs);
	m_bPendingTask = false;
	m_bAnyResultsExist = true;
	return 0;
}

#ifdef WIN32
#include <conio.h>
#endif

void matlab::engine::mngr::EqFctProxy::TryConnect()
{
#ifdef WIN32
	//printf("Press any key to continue!"); fflush(stdout);
	//_getch();
	//printf("\n");
#endif
	long timeDiffMs;
	int nMaxTryCount(m_maxTryCount.value());
	struct timeb currentTime;

	if(m_bIsConnected2){return;}
	if(m_nNumOfTryOfStart++>nMaxTryCount){return;}
	int nReturn( m_socketTcp.ConnectToServer("localhost", m_nEngineNumber));
	if (nReturn){

		ftime(&currentTime);
		timeDiffMs =
			(currentTime.time - m_lastStartTime.time) * 1000 +
			(currentTime.millitm - m_lastStartTime.millitm);

		if(timeDiffMs>10000){
			std::string strCommandToStartEngine = m_engineStartCommand.value();
			printf("!!!!!!!!!!++++++++!!!!!! val1=%s, val2=%s\n",
				strCommandToStartEngine.size() ? strCommandToStartEngine.c_str() : "nill",
				m_engineStartCommand.value() ? m_engineStartCommand.value() : "nill");
			if (strCommandToStartEngine.size())EqFctProxy::StartNewEngine(strCommandToStartEngine);
			Sleep(100);
			nReturn = m_socketTcp.ConnectToServer("localhost", m_nEngineNumber);
			m_lastStartTime= currentTime;
		}

	}
	m_bIsConnected2 = nReturn?false:true;
	m_isConnected2.set_value(m_bIsConnected2 ? 1 : 0);
	m_nNumOfTryOfStart = m_bIsConnected2?0: m_nNumOfTryOfStart;
}


/*//////////////////////////////////////////////////////////*/

matlab::engine::mngr::D_expression::D_expression(const char* a_pn, ::EqFct* a_par)
	:
	D_string(a_pn, a_par)
{
}


void matlab::engine::mngr::D_expression::set(
	EqAdr* a_dcsAdr, EqData* a_fromUser, EqData* a_toUser, EqFct* a_fct)
{
	EqFctProxy* pPar = (EqFctProxy*)a_fct;

	D_string::set(a_dcsAdr, a_fromUser, a_toUser, a_fct);

	std::string nValue = a_fromUser->get_string();
	pPar->SendCommandToMatlab(0, nValue.c_str());
}


void matlab::engine::mngr::D_expression::get(
	EqAdr* a_dcsAdr, EqData* a_fromUser, EqData* a_toUser, EqFct* a_fct)
{
	EqFctProxy* pPar = (EqFctProxy*)a_fct;

	D_string::get(a_dcsAdr, a_fromUser, a_toUser, a_fct);
	pPar->GetForProperties(a_fromUser, a_toUser);

	//std::string nValue = a_fromUser->get_string();
	//pPar->SendCommandToMatlab(0, nValue.c_str());
}

/*//////////////////////////////////////////////////////////*/
//D_startEngine
matlab::engine::mngr::D_startEngine::D_startEngine(const char* a_pn, ::EqFct* a_par)
	:
	D_fct(a_pn, a_par)
{
}


void matlab::engine::mngr::D_startEngine::set(
	EqAdr* a_dcsAdr, EqData* a_fromUser, EqData* a_toUser, EqFct* a_fct)
{
	EqFctProxy* pParent = (EqFctProxy*)a_fct;

	D_fct::set(a_dcsAdr, a_fromUser, a_toUser, a_fct);
	if(pParent->m_bIsConnected2){return;}
	pParent->m_nNumOfTryOfStart = 0;
	pParent->TryConnect();
}
