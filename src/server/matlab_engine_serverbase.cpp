/*
 *	File: matlab_engine_serverbase.cpp
 *
 *	Created on: 12 Apr, 2017
 *	Author    : Davit Kalantaryan (Email: davit.kalantaryan@desy.de)
 *
 *  This file implements all functions ...
 *		1) ...
 *		2) ...
 *		...
 *
 *
 */

//#include "matrix.h"
//#include <stdio.h>
#include "matlab_engine_serverbase.hpp"
#include "matlab_bytestream_routines.h"

#ifdef WIN32
#else  // #ifdef WIN32
#include <signal.h>
#define SIGNAL_ARGUMENTS    int a_nSigNum, siginfo_t * /*a_pSigInfo*/, void *
typedef void(*TYPE_SIG_HANDLER)(SIGNAL_ARGUMENTS);
static void SignalHandlerSimple(SIGNAL_ARGUMENTS);
#endif // #ifdef WIN32

#ifndef _SOCKET_TIMEOUT_
#define	_SOCKET_TIMEOUT_		-2001
#endif

typedef const void* TypeConstVoidPtr;

namespace matlab {namespace engine {

class SocketForServer : public common::SocketBase{
public:
	SocketForServer(void* rcvSend,TypeRecvFunc a_fpReceive,TypeSenderFunc a_fpSend);
	int Recv(void* buffer, int bufSize, long timeoutMS)__OVERRIDE__;
	int Send(const void* buffer, int bufSize)__OVERRIDE__;
private:
	void*			pRcvSend;
	TypeRecvFunc	fpReceive;
	TypeSenderFunc	fpSend;
};
}}

static matlab::engine::ServerBase* s_pCurServer;

matlab::engine::ServerBase::ServerBase(
	TypeRecvFunc a_funcReceive, TypeSenderFunc a_funcSend,
	TypeCloseFunc a_funcClose,MatHandleBase* a_pMatHandle)
	:
        m_nRun(0),
        m_fpReceive(a_funcReceive),
	m_fpSend(a_funcSend),
	m_fpClose(a_funcClose),
	m_pMatHandle(a_pMatHandle),
	m_firstItem(NULL),
	m_jobQueuee(CASH_AND_MAX_QUEUEE_SIZE)
{
	m_pCurrentItem = NULL;
}


matlab::engine::ServerBase::~ServerBase()
{
	StopMServer();
}


int matlab::engine::ServerBase::StartMServer(int a_nEngNumber)
{
	int nError;
	if (m_nRun != 0) { return 0; }
	m_nRun = 1;
	nError = m_pMatHandle->Start();
	if (nError == START_RET::ENG_ERROR) { m_nRun = 0;return START_RET::ENG_ERROR;}
	m_resourceThread = STD::thread(&matlab::engine::ServerBase::ResourceThread, this);
	m_nReturnFromServerThread = 0;
	m_nServerRuns = 0;
    m_serverThread = STD::thread(&matlab::engine::ServerBase::ServerThread, this, a_nEngNumber);
    while ((m_nReturnFromServerThread==0)&&(m_nServerRuns==0)){Sleep(10);}
	if (m_nReturnFromServerThread) { StopMServer(); }
	return m_nReturnFromServerThread;
}


void  matlab::engine::ServerBase::StopMServer(void)
{
	if (m_nRun == 0) { return; }
	SResourceJob newJob;
	m_nRun = 0;
	StopServerPrivate();
	newJob.code = RESRC_REQ_TYPES::FINISH_LOOP;
	newJob.arg = NULL;
	m_jobQueuee.AddElement(newJob);
	m_semaphoreForResource.post();
	m_serverThread.join();
	m_resourceThread.join();
	m_pMatHandle->Stop();
}


int matlab::engine::ServerBase::GetRun()
{
	return m_nRun;
}


void matlab::engine::ServerBase::ServerThread(int a_nEngNumber)
{
	StartServerPrivate(a_nEngNumber);
}


void matlab::engine::ServerBase::AddClient(void* a_client)
{
	SResourceJob newJob;
	
	newJob.code = RESRC_REQ_TYPES::NEW_CLIENT;
	newJob.arg = a_client;
	m_jobQueuee.AddElement(newJob);
	m_semaphoreForResource.post();
}


void matlab::engine::ServerBase::DeteleCurrentClient()
{
    if(m_pCurrentItem){DeteleClient(m_pCurrentItem);}
}

void matlab::engine::ServerBase::DeteleClient(struct SConnectionItem* a_clientItem)
{
	SResourceJob newJob;

	newJob.code = RESRC_REQ_TYPES::DELETE_CLIENT;
	newJob.arg = a_clientItem;
	m_jobQueuee.AddElement(newJob);
	m_semaphoreForResource.post();
}


void matlab::engine::ServerBase::DeleteItemInResourceThread(SConnectionItem* a_item)
{
	a_item->run = 0;
	(*m_fpClose)(a_item->senderReceiver);
	a_item->serverThread.join();
	if (!a_item->prev && !a_item->next) { m_firstItem = NULL; } // last item
	else if (!a_item->prev && a_item->next) { a_item->next->prev = NULL; m_firstItem = a_item->next; }
	else if (a_item->prev && !a_item->next) { a_item->prev->next = NULL; }
	else { a_item->prev->next = a_item->next;  a_item->next->prev = a_item->prev; }
	delete a_item;
}


void matlab::engine::ServerBase::ResourceThread(void)
{
	SResourceJob nextJob;
	SConnectionItem *pNewItem, *tmpItem,*tmpItemNext;
	
	m_nRun = 1;

	while (m_nRun == 1)
	{
		m_semaphoreForResource.wait();

		while (m_jobQueuee.Extract(&nextJob))
		{
			switch (nextJob.code)
			{
			case RESRC_REQ_TYPES::FINISH_LOOP:
				m_nRun = 0;
				break;
			case RESRC_REQ_TYPES::NEW_CLIENT:
				pNewItem = new SConnectionItem(this, nextJob.arg);
				if (!pNewItem) { break; }// error log		

				if (!m_firstItem) { m_firstItem = pNewItem; break; }

				tmpItem = m_firstItem;
				tmpItemNext = tmpItem->next;
				while (tmpItemNext) { tmpItemNext = tmpItem->next; if (tmpItemNext) { tmpItem = tmpItemNext; } }
				tmpItem->next = pNewItem;
				pNewItem->prev = tmpItem;
				break;

			case RESRC_REQ_TYPES::DELETE_CLIENT:
				DeleteItemInResourceThread((SConnectionItem*)nextJob.arg);
				break;

			default:
				break;
			}// switch (m_nResourceJob)
		} // while (m_jobQueuee.Extract(&nextJob))
	} // while (m_nRun == 1)

	// Time to clear all resources
	while (m_firstItem)
	{
		DeleteItemInResourceThread(m_firstItem);
	}
}


void matlab::engine::ServerBase::ContactThread(struct SConnectionItem* a_item)
{
	SocketForServer aSocket(a_item->senderReceiver,m_fpReceive,m_fpSend);
	int nAction; 
	
	a_item->run = 1;

	while (m_nRun && a_item->run)
	{
		nAction=a_item->serializer.ReceiveHeader(&aSocket, SERVER_BIG_TIMEOUT_MS);
		switch (nAction)
		{
		case _SOCKET_TIMEOUT_:
			break;
		case ACTION_TYPE::REMOTE_CALL:
			m_pMatHandle->SyncCallOnMatlabThreadC(
				this, &matlab::engine::ServerBase::CallMatlabFunction, (void*)a_item);
			break;
		default:
			a_item->run = 0;
			break;
		}
				
	} // while (m_nRun && a_item->run)

	//(*m_fpClose)(a_item->senderReceiver);
	DeteleClient(a_item);
}


void matlab::engine::ServerBase::CallMatlabFunction(void* a_arg)
{
	struct SConnectionItem* pItem = (struct SConnectionItem*)a_arg;
	const char* cpcInfoOrError = "Error";
	mxArray* pReturnFromCallMatlab = NULL;
	mxArray* vOutputs[MAXIMUM_NUMBER_OF_IN_AND_OUTS];
	mxArray* vInputs[MAXIMUM_NUMBER_OF_IN_AND_OUTS];
	SocketForServer aSocket(pItem->senderReceiver, m_fpReceive, m_fpSend);
	int nReturn(-1);
	int32_ttt nInputs;
	int32_ttt i,nOutputs(0);
	int nIsAnsExist = 0;
	int32_ttt nSeriType;

#ifdef WIN32
#else  // #ifdef WIN32
	struct sigaction sigAction, sigActionOld;
#endif  // #ifdef WIN32

    s_pCurServer = this;
	m_pCurrentItem = pItem;

	if (pItem->serializer.OverAllLengthMinusHeader() <= 0) { goto returnPoint; }
	try {
		nInputs=pItem->serializer.ReceiveScriptNameAndArrays2(
			&aSocket,10000,
			MAXIMUM_NUMBER_OF_IN_AND_OUTS, (void**)vInputs);
		if (nInputs < 0) {goto returnPoint;}
		nSeriType = pItem->serializer.SeriType();
		nOutputs = pItem->serializer.NumOfExpOutsOrError()>MAXIMUM_NUMBER_OF_IN_AND_OUTS ?
			MAXIMUM_NUMBER_OF_IN_AND_OUTS : pItem->serializer.NumOfExpOutsOrError();

		if (((nInputs == 0) && (nOutputs<2)) || (nOutputs == 0)) // try any remaining outs
		{
			vOutputs[0] = m_pMatHandle->newGetVariable("base", "ans");
			if (vOutputs[0])
			{
				nIsAnsExist = 1;
				m_pMatHandle->newEvalStringWithTrap("old__ans=ans;clear ans");
				mxDestroyArray(vOutputs[0]);
			}
		}

		if ((nInputs == 0) && (nOutputs<2)) // eval string is called
		{
			pReturnFromCallMatlab = m_pMatHandle->newEvalStringWithTrap(
				pItem->serializer.MatlabScriptName3());
		}
		else
		{
			pReturnFromCallMatlab = m_pMatHandle->newCallMATLABWithTrap(nOutputs, vOutputs, 
				nInputs,vInputs, pItem->serializer.MatlabScriptName3());
		}

		if (((nInputs == 0) && (nOutputs<2)) || (nOutputs == 0)) // try any remaining outs
		{
			vOutputs[0] = m_pMatHandle->newGetVariable("base", "ans");
            if (vOutputs[0]) { nOutputs = 1; }
			else if (nIsAnsExist){ m_pMatHandle->newEvalStringWithTrap("ans=old__ans;"); }
			if (nIsAnsExist) { m_pMatHandle->newEvalStringWithTrap("clear old__ans"); }
		}

		if (pReturnFromCallMatlab){nOutputs=1;vOutputs[0]=pReturnFromCallMatlab; goto returnPoint;}

		nReturn = 0;
		cpcInfoOrError = "OK";

	}
	catch (...)
	{
		goto returnPoint;
	}

returnPoint:
	if (pReturnFromCallMatlab)
	{
		m_pMatHandle->newFrprint(STDPIPES::STDERR,
			"Error during calling script by request from remote.\\n"
			"script name is \"%s\"\\n", pItem->serializer.MatlabScriptName3());
	}
	
	// Disabling pipe error exit
#ifdef WIN32
#else  // #ifdef WIN32
	sigemptyset(&sigAction.sa_mask);
	sigAction.sa_flags = SA_SIGINFO;
	sigAction.sa_sigaction = (TYPE_SIG_HANDLER)SignalHandlerSimple;
	sigaction(SIGPIPE, &sigAction, &sigActionOld);
#endif  // #ifdef WIN32
	pItem->serializer.SendScriptNameAndArrays(
		&aSocket, pItem->serializer.Version(),nSeriType,
		cpcInfoOrError, nReturn,nOutputs, (TypeConstVoidPtr*)vOutputs);

	if(!pReturnFromCallMatlab){
		for(i=0;i<nOutputs;++i){mxDestroyArray(vOutputs[i]);}
	}

#ifdef WIN32
#else  // #ifdef WIN32
	sigaction(SIGPIPE, &sigActionOld,NULL);
#endif  // #ifdef WIN32
	
	m_pMatHandle->newEvalStringWithTrap("drawnow");
	m_pCurrentItem = NULL;
	s_pCurServer = NULL;
}


/*//////////////////////*/
#if 0
void*						senderReceiver;
struct SConnectionItem*		prev;
struct SConnectionItem*		next;
mxArray* vInputs[MAXIMUM_NUMBER_OF_IN_AND_OUTS];
volatile int				run;
int32_ttt					numOfInputs;
matlab::engine::Serializer	serializer;
STD::thread					serverThread;
#endif
matlab::engine::SConnectionItem::SConnectionItem(ServerBase* a_pParent, void* a_senderReceiver)
	:
    senderReceiver(a_senderReceiver),
	prev(NULL), next(NULL),
	serializer(a_pParent->m_pMatHandle),
	serverThread(&matlab::engine::ServerBase::ContactThread, a_pParent, this)
{
}

matlab::engine::SConnectionItem::~SConnectionItem()
{ 
}



/*///////////////////////////////////////////////////////////////////////////*/
matlab::engine::SocketForServer::SocketForServer(
	void* a_pRcvSend,TypeRecvFunc a_fpReceive, TypeSenderFunc a_fpSend)
	:
	pRcvSend(a_pRcvSend),
	fpReceive(a_fpReceive),
	fpSend(a_fpSend)
{
}

int matlab::engine::SocketForServer::Recv(void* a_buffer, int a_bufSize, long a_timeoutMS)
{
	return (*fpReceive)(pRcvSend,a_buffer, a_bufSize, a_timeoutMS);
}


int matlab::engine::SocketForServer::Send(const void* a_buffer, int a_bufSize)
{
	return (*fpSend)(pRcvSend, a_buffer, a_bufSize);
}



/*///////////////////////////////////////////////////////////////////////////////////////////////*/

#ifdef WIN32
#else // #ifdef WIN32
static void SignalHandlerSimple(SIGNAL_ARGUMENTS)
{

	switch (a_nSigNum)
	{
	case SIGPIPE:
        if (s_pCurServer) {
            s_pCurServer->DeteleCurrentClient();
        }
		break;
	default:
		break;
	}
}
#endif // #ifdef WIN32
