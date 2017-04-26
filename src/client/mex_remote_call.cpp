/*
 *	File: mex_matlabengine_set_slm.cpp 
 *
 *	Created on: 18 Dec, 2016
 *	Author: Davit Kalantaryan (Email: davit.kalantaryan@desy.de)
 *
 *
 */

#define NO_RECEIVE	-10

#include <mex.h>
#include <string.h>
#include <malloc.h>
#include "matlab_engine_serializer.hpp"
#include <asockettcp.h>
#include "matlab_engine_clienttcp.hpp"
#include <string>
#include <matlab_engine_mathandlemexbase.hpp>
#include <matlab_bytestream_routines.h>
#include <map>

static void PrintHelp();
static void CleanMexFile(void);

struct SConnectItem {
	SConnectItem(const std::string& a_name=""):serverName(a_name), isJobActive(false),timeoutMS(-1){}
	std::string					serverName;
	matlab::engine::ClientTcp	socketTcp;
	bool						isJobActive;
	bool						isHandled;
	long						timeoutMS;
};

static matlab::engine::MatHandleMexBase		s_matHandle;
static int									s_nInited = 0;
static matlab::engine::Serializer			s_serializeDes(&s_matHandle);
static SConnectItem*						s_pCurrentConnect=NULL;
static int									s_nDebug = 0;
static std::map<std::string,SConnectItem*>	s_mpConnections;

typedef const void* TypeConstVoidPtr;

void mexFunction(int a_nNumOuts, mxArray *a_Outputs[],
	int a_nNumInps, const mxArray*a_Inputs[])
{
    int i,nReceived,nAdrStrLenPlus15;
	int32_ttt nOutputs,nSeriType = matlab::engine::SERI_TYPE::MAT_UNDOCU;

	if (!s_nInited) {
		ASocketB::Initialize();
#ifdef CPP11DEFINED
		mexAtExit([]() {
			if (s_nDebug) { mexPrintf("matlab_pipe_server:AtExit called!\n"); }
			if (!s_nInited) { CleanMexFile(); s_nInited = 0; }
		});
#else // #ifdef CPP11DEFINED
        mexAtExit(CleanMexFile);
#endif // #ifdef CPP11DEFINED
		s_nInited= 1;
	}

	if ((a_nNumInps<1) || (!mxIsChar(a_Inputs[0])) )
	{
		PrintHelp();
		return;
	}

	// Find the host for running the script
	nAdrStrLenPlus15 = (int)(mxGetM(a_Inputs[0]) * mxGetN(a_Inputs[0])) + 15;
	char* pcAddressOrHost = (char*)alloca(nAdrStrLenPlus15);
	if (mxGetString(a_Inputs[0], pcAddressOrHost, nAdrStrLenPlus15)){
		mexErrMsgTxt("Unable to get the string!\n");
		return;
	}

	if (strncmp("--debug", pcAddressOrHost, nAdrStrLenPlus15) == 0){ s_nDebug = 1; return; }
	else if (strncmp("--no-debug", pcAddressOrHost, nAdrStrLenPlus15) == 0){ s_nDebug = 0; return; }
	else if (strncmp("--help", pcAddressOrHost, nAdrStrLenPlus15) == 0) { PrintHelp(); return; }

	if ((a_nNumInps<2) || (!mxIsChar(a_Inputs[1])))
	{
		PrintHelp();
		return;
	}

	if (s_pCurrentConnect && (s_pCurrentConnect->serverName == pcAddressOrHost))
	{
	}
	else if (s_mpConnections.count(pcAddressOrHost))
	{
		s_pCurrentConnect = s_mpConnections[pcAddressOrHost];
		if (!s_pCurrentConnect) { mexPrintf("fl%s,ln%d\n",__FILE__,__LINE__); return; }
	}
	else 
	{
		int nPid = 0;
		char* pcPidHint;
		int nRet = 0;

		s_pCurrentConnect = new SConnectItem(pcAddressOrHost);
		if (!s_pCurrentConnect) { mexErrMsgTxt("Unable to locate memory!"); return; }

		pcPidHint = strchr(pcAddressOrHost, ':');
		if (pcPidHint) { pcPidHint[0] = 0; nPid = atoi(pcPidHint + 1); }
		//s_socketTcp.Close();
		nRet= s_pCurrentConnect->socketTcp.ConnectToServer(pcAddressOrHost, nPid);
		if (nRet != 0)
		{
			delete s_pCurrentConnect;
			s_pCurrentConnect = NULL;
			mexPrintf("Unable to connect {%s:%d}\n", pcAddressOrHost, nPid);
			return;
		}
		s_mpConnections.insert(std::pair<std::string, SConnectItem*>(pcAddressOrHost, s_pCurrentConnect));
		if (s_nDebug) { mexPrintf("server{%s,%d}\n", pcAddressOrHost, nPid); }
	}

	// The name of the script to run on remote host
	nAdrStrLenPlus15 = (int)(mxGetM(a_Inputs[1]) * mxGetN(a_Inputs[1])) + 15;
	char* pcScriptName = (char*)alloca(nAdrStrLenPlus15);
	if (mxGetString(a_Inputs[1], pcScriptName, nAdrStrLenPlus15)) {
		mexErrMsgTxt("Unable to get the string!\n");
		return;
	}


	if (strncmp("--timeout", pcScriptName, nAdrStrLenPlus15) == 0) {
		double* plfTimeout;
		if (a_nNumInps > 2 && mxIsDouble(a_Inputs[2])) {
			plfTimeout = mxGetPr(a_Inputs[2]);
			s_pCurrentConnect->timeoutMS = (long)(*plfTimeout);
		}
		else{
			a_Outputs[0] = mxCreateDoubleMatrix(1, 1, mxREAL);
			plfTimeout = mxGetPr(a_Outputs[0]);
			*plfTimeout = (double)s_pCurrentConnect->timeoutMS;
		}
		return;
	}
	else if (strncmp("--receive", pcScriptName, nAdrStrLenPlus15) == 0) {
		if (s_pCurrentConnect->isJobActive) { goto recieveResult; }
		else { mexPrintf("No running job!\n"); return; }
	}

	s_pCurrentConnect->isHandled = false;

	nReceived=s_serializeDes.SendScriptNameAndArrays(
		&s_pCurrentConnect->socketTcp,SERIALIZER_VERSION,nSeriType,
		pcScriptName,a_nNumOuts,a_nNumInps-2,(TypeConstVoidPtr*)(a_Inputs+2));

	if(nReceived<0){goto cleanCurrentConn;}
	s_pCurrentConnect->isJobActive = true;

	if (s_pCurrentConnect->timeoutMS == NO_RECEIVE) 
	{
		goto returnWithTimeout; 
	}
	
	// receive answer
recieveResult:
	a_nNumOuts = a_nNumOuts < 1 ? 1 : a_nNumOuts;
	nReceived=s_serializeDes.ReceiveScriptNameAndArrays(
		&s_pCurrentConnect->socketTcp,
		a_nNumOuts,(void**)a_Outputs, &nOutputs,s_pCurrentConnect->timeoutMS);

	if (nReceived == _SOCKET_TIMEOUT_){goto returnWithTimeout;}
	else if (nReceived < 0){goto cleanCurrentConn;}

	return;
returnWithTimeout:

	for (i = 0; i < a_nNumOuts; ++i)
	{
		a_Outputs[i] = mxCreateString("job_active");
	}
	return;

cleanCurrentConn:
	s_mpConnections.erase(s_pCurrentConnect->serverName);
	s_pCurrentConnect->socketTcp.Close();
	delete s_pCurrentConnect;
	s_pCurrentConnect = NULL;
	mexErrMsgTxt("Error durring calling script in the remote host");

}


static void PrintHelp(void)
{
	mexPrintf("provide DOOCS address as string argument and the array \n"
		"Options are following:\n"
		"\t--debug             : makes this MEX file more verbose\n"
		"\t--no-debug          : makes this MEX file less verbose\n"
		"\t--help              : prints this help\n"
		"\thostname,--timeout       : gets or sets timeout\n"
		"\thostname,--receive       : resieves the result of already set task\n"
	);
}


static void CleanMexFile(void)
{
    if (!s_nInited) { return;}

    if (s_nDebug) { mexPrintf("matlab_pipe_server:AtExit called!\n"); }

	std::map<std::string, SConnectItem*>::iterator it;

	for (it = s_mpConnections.begin(); it != s_mpConnections.end(); it++)
	{
		it->second->socketTcp.Close();
		delete it->second;
	}

	s_mpConnections.clear();

	ASocketB::Cleanup();

    s_nInited = 0;
}


#ifdef _DO_DEBUG_

int main()
{
	return 0;
}

#endif  // #ifndef NO_DEBUG_
