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

#include "matlab_engine_serverbase.hpp"
#include "matlab_bytestream_routines.h"

#ifdef WIN32
#else  // #ifdef WIN32
#define SIGNAL_ARGUMENTS    int a_nSigNum, siginfo_t * /*a_pSigInfo*/, void *
typedef void(*TYPE_SIG_HANDLER)(SIGNAL_ARGUMENTS);
#endif // #ifdef WIN32

#ifndef _SOCKET_TIMEOUT_
#define	_SOCKET_TIMEOUT_		-2001
#endif

matlab::engine::ServerBase::ServerBase(
	TypeRecvFunc a_funcReceive, TypeSenderFunc a_funcSend,
	TypeCloseFunc a_funcClose,MatHandleBase* a_pMatHandle)
	:
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
	StopServer();
}


int matlab::engine::ServerBase::StartServer(void)
{
	if (m_nRun != 0) { return 0; }
	m_resourceThread = std::thread(&matlab::engine::ServerBase::ResourceThread, this);
	m_serverThread = std::thread(&matlab::engine::ServerBase::ServerThread, this);
	return 0;
}


void  matlab::engine::ServerBase::StopServer(void)
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
}


int matlab::engine::ServerBase::GetRun()
{
	return m_nRun;
}


void matlab::engine::ServerBase::ServerThread(void)
{
	/*int nRet = */StartServerPrivate();
}


void matlab::engine::ServerBase::AddClient(void* a_client)
{
	SResourceJob newJob;
	
	newJob.code = RESRC_REQ_TYPES::NEW_CLIENT;
	newJob.arg = a_client;
	m_jobQueuee.AddElement(newJob);
	m_semaphoreForResource.post();
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
	SDataHeader aInfo;
	int nReadReturn; // <0 command >=0 length

	a_item->run = 1;	

	while (m_nRun && a_item->run)
	{
		nReadReturn = (*m_fpReceive)(a_item->senderReceiver,&aInfo, 
			MATLAB_HEADER_LENGTH, SERVER_BIG_TIMEOUT_MS);
		
		if (nReadReturn == _SOCKET_TIMEOUT_) { continue; }
		else if (nReadReturn != MATLAB_HEADER_LENGTH) { break; }

		switch (aInfo.allMinusHeader)
		{
		case CLIENT_REQ_TYPES::CLOSE:
			a_item->run = 0;
			break;
		default:  // default is calling matlab routine
			a_item->mutexForBuffers.lock();
			a_item->serializer.Resize3(aInfo.seriType, aInfo.allMinusHeader, aInfo.byteStrLength,
				aInfo.numberOfOuts);
			nReadReturn = (*m_fpReceive)(a_item->senderReceiver, 
				a_item->serializer.GetBufferForReceive3(), aInfo.allMinusHeader, SMALL_TIMEOUT_MS);
			a_item->mutexForBuffers.unlock();
			m_pMatHandle->CallOnMatlabThreadC(this,&matlab::engine::ServerBase::CallMatlabFunction, (void*)a_item);
			break;
		}
	} // while (s_nRun)

	//(*m_fpClose)(a_item->senderReceiver);
}


void matlab::engine::ServerBase::CallMatlabFunction(void* a_arg)
{
	struct SConnectionItem* pItem = (struct SConnectionItem*)a_arg;
	const char* cpcInfoOrError = "Error";
	void* pSerializedByteStream = NULL;
	mxArray* pReturnFromCallMatlab = NULL;
	mxArray* vInputs[MAXIMUM_NUMBER_OF_IN_AND_OUTS];
	mxArray* vOutputs[MAXIMUM_NUMBER_OF_IN_AND_OUTS];
	int nReturn(-1);
	int nInputs;
	int nOutputs;
	int nIsAnsExist = 0;
	int32_ttt nBSLin, nBSLout(0), nSeriType;

	m_pCurrentItem = pItem;

	// Disabling pipe error exit
#ifdef WIN32
#else  // #ifdef WIN32
	struct sigaction sigAction, sigActionOld;
	sigemptyset(&sigAction.sa_mask);
	sigAction.sa_flags = SA_SIGINFO;
	sigAction.sa_sigaction = (TYPE_SIG_HANDLER)SignalHandlerSimple;
	sigaction(SIGPIPE, &sigAction, &sigActionOld);
#endif  // #ifdef WIN32

	pItem->mutexForBuffers.lock();

	if (pItem->serializer.SecondReceiveBufLength3() <= 0) { goto returnPoint; }
	try {
		nBSLin = pItem->serializer.VaribleByteStreamLength3();
		nSeriType = pItem->serializer.SeriType();

		nInputs = m_pMatHandle->ByteStreamToMatlabArray(
			nSeriType, MAXIMUM_NUMBER_OF_IN_AND_OUTS,vInputs,
			nBSLin,pItem->serializer.VaribleByteStream3());

		if (nInputs < 0) { goto returnPoint; }

		nOutputs = pItem->serializer.NumOfExpOutsOrError3() > MAXIMUM_NUMBER_OF_IN_AND_OUTS ?
			MAXIMUM_NUMBER_OF_IN_AND_OUTS : pItem->serializer.NumOfExpOutsOrError3();

		if (((nInputs == 0) && (nOutputs<2)) || (nOutputs == 0)) // try any remaining outs
		{
			vOutputs[0] = m_pMatHandle->newGetVariable("base", "ans");
			if (vOutputs[0])
			{
				nIsAnsExist = 1;
				m_pMatHandle->newEvalStringWithTrap("old__ans=ans;clear ans");
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
				nInputs, vInputs, pItem->serializer.MatlabScriptName3());
		}

		if (((nInputs == 0) && (nOutputs<2)) || (nOutputs == 0)) // try any remaining outs
		{
			vOutputs[0] = m_pMatHandle->newGetVariable("base", "ans");
			if (vOutputs[0]) { nOutputs = 1; }
			else if (nIsAnsExist){ m_pMatHandle->newEvalStringWithTrap("ans=old__ans;"); }
			if (nIsAnsExist) { m_pMatHandle->newEvalStringWithTrap("clear old__ans"); }
		}

		if (pReturnFromCallMatlab)
		{
			pSerializedByteStream = m_pMatHandle->MatlabArrayToByteStream1(
				nSeriType,1, &pReturnFromCallMatlab,&nBSLout);
			goto returnPoint;
		}

		pSerializedByteStream = m_pMatHandle->MatlabArrayToByteStream1(nSeriType,nOutputs,vOutputs, &nBSLout);
		if (!pSerializedByteStream) { pSerializedByteStream = ""; goto returnPoint; }
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
	pItem->serializer.SetSendParams3(nSeriType,cpcInfoOrError, nBSLout, pSerializedByteStream,nReturn);
	m_fpSend(pItem->senderReceiver, pItem->serializer.GetBufferForSend3(), 
		pItem->serializer.SendBufLength3());
	pItem->mutexForBuffers.unlock();
	m_pMatHandle->newEvalStringWithTrap("drawnow");
#ifdef WIN32
#else  // #ifdef WIN32
	sigaction(SIGPIPE, &sigActionOld,NULL);
#endif  // #ifdef WIN32
	m_pCurrentItem = NULL;

}


/*//////////////////////*/
matlab::engine::SConnectionItem::SConnectionItem(ServerBase* a_pParent, void* a_senderReceiver)
	:
	serverThread(&matlab::engine::ServerBase::ContactThread, a_pParent,this),
	senderReceiver(a_senderReceiver),
	prev(NULL),next(NULL)
{
}

matlab::engine::SConnectionItem::~SConnectionItem()
{ 
}



/*///////////////////////////////////////////////////////////////////////////////////////////////*/

#ifdef WIN32
#else // #ifdef WIN32
static void SignalHandlerSimple(SIGNAL_ARGUMENTS)
{

	switch (a_nSigNum)
	{
	case SIGPIPE:
		if (m_pCurrentItem) { DeteleClient(m_pCurrentItem); }
		break;
	default:
		break;
	}
}
#endif // #ifdef WIN32
