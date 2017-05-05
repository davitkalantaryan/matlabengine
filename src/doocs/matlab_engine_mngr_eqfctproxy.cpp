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

#include "matlab_engine_mngr_eqfctproxy.hpp"
#include "matlab_pipe_name.h"

typedef const void* TypeConstVoidPtr;

#define _SERI_RAW1 matlab::engine::versioning::SERI_TYPE::RAW1

static bool s_bIsInited = false;
static matlab::engine::mngr::EqFctProxy* s_pProxys[MAX_ENGINE_NUMBERS];

matlab::engine::mngr::EqFctProxy::EqFctProxy(int a_nEngineNumber)
	:
	::EqFct("NAME = location"),

	m_engineName("ENGINE.NAME",this),
	m_matlabScriptName("MATLAB.SCRIPT.NAME",this),
	m_commandToStartMatlabEngine("COMMAND.TO.START.ENGINE",this),
	m_numberOfExpectedOutputs("EXPECTED.OUTPUTS",this),
    m_engineNumber("ENGINE.NUMBER",this),

	m_argInt("ARG.INT",this),
	m_argFloat("ARG.FLOAT",this),
	m_argDouble("ARG.DOUBLE",this),
	m_argString("ARG.STRING",this),
    m_argVoid("ARG.VOID",this),

    m_vArray(MAXIMUM_NUMBER_OF_IN_AND_OUTS),
    m_serializeDes(NULL, CURRENT_SERIALIZER_VERSION2, _SERI_RAW1)
{
	if (!s_bIsInited) { memset(s_pProxys, 0, sizeof(s_pProxys)); s_bIsInited = true; }
	m_engineName.set_ro_access();
	m_bPendingTask = false;
	if ((a_nEngineNumber <= 0) || (a_nEngineNumber > MAX_ENGINE_NUMBERS)){ m_nEngineNumber = -1; }
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
}

void  matlab::engine::mngr::EqFctProxy::init(void)
{
	if ((m_nEngineNumber < 0) || (m_nEngineNumber >= MAX_ENGINE_NUMBERS)) {
		int nEngNumberDoocs = m_engineNumber.value();
		if((nEngNumberDoocs<0) || (nEngNumberDoocs>= MAX_ENGINE_NUMBERS)){
			set_error(1, "Wrong engine number");
			return;
		}
		m_nEngineNumber = nEngNumberDoocs;
		s_pProxys[m_nEngineNumber] = this;
	}
	else {
		m_engineNumber.set_value(m_nEngineNumber);
	}

	sys_mask_.set_value(2);
}

void  matlab::engine::mngr::EqFctProxy::cancel(void)
{
	if((m_nEngineNumber>=0)||(m_nEngineNumber<MAX_ENGINE_NUMBERS)){s_pProxys[m_nEngineNumber]=NULL;}
}


matlab::engine::mngr::EqFctProxy* matlab::engine::mngr::EqFctProxy::EngineControled2(int a_nEngine)
{
	if (!s_bIsInited) { memset(s_pProxys, 0, sizeof(s_pProxys)); s_bIsInited = true; }
	matlab::engine::mngr::EqFctProxy* pRet =
		a_nEngine < MAX_ENGINE_NUMBERS ? s_pProxys[a_nEngine] : NULL;
	return pRet;
}


PersistantArgsGroup& matlab::engine::mngr::EqFctProxy::GetArray()
{
	return m_vArray;
}


int  matlab::engine::mngr::EqFctProxy::SendCommandToMatlab(int a_nNumOfInputs)
{
	int nReceived;
	int32_ttt nExpectedOuts;
	const char* cpcScrName = "";
	
	if (m_bPendingTask) {
		ReceiveResultFromMatlab();
	}

	a_nNumOfInputs = a_nNumOfInputs > MAXIMUM_NUMBER_OF_IN_AND_OUTS ?
		MAXIMUM_NUMBER_OF_IN_AND_OUTS : a_nNumOfInputs;
	cpcScrName = m_matlabScriptName.value();
	nExpectedOuts = m_numberOfExpectedOutputs.value();

	nReceived = m_serializeDes.SendScriptNameAndArrays(
		&m_vFuncs, &m_socketTcp,
		cpcScrName, nExpectedOuts, a_nNumOfInputs, 
		(TypeConstVoidPtr*)(m_vArray.Array()));

	if (nReceived < 0) {
		return nReceived;
	}

	return 0;
}


int  matlab::engine::mngr::EqFctProxy::ReceiveResultFromMatlab()
{
	int nReceived(-1);
	int32_ttt nOutputs;

	if (!m_bPendingTask) {
		return -2;
	}
	
	nReceived = m_serializeDes.ReceiveHeaderScriptNameAndArrays2(
		&m_vFuncs, &m_socketTcp, 10,
		MAXIMUM_NUMBER_OF_IN_AND_OUTS, (void**)(m_vArray.Array()), &nOutputs);

	if (nReceived != _SOCKET_TIMEOUT_) { return _SOCKET_TIMEOUT_; }

	else if (nReceived < 0) {
		m_socketTcp.Close();
		nReceived= TryConnect();
		if (nReceived) {/* setError */}
		m_bPendingTask = false;
		return -2;
	}

	return 0;
}


int matlab::engine::mngr::EqFctProxy::TryConnect()
{
	///
	return 0;
}


/*//////////////////////////////////////////////////////////*/

matlab::engine::mngr::D_engineNum::D_engineNum(const char* a_pn, ::EqFct* a_par)
	:
	D_int(a_pn, a_par)
{
}


void matlab::engine::mngr::D_engineNum::set(
	EqAdr* a_dcsAdr, EqData* a_fromUser, EqData* a_toUser, EqFct* a_fct)
{
	D_int::set(a_dcsAdr, a_fromUser, a_toUser, a_fct);
}
