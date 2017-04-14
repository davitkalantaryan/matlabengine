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

#ifdef WIN32
#else  // #ifdef WIN32
#define SIGNAL_ARGUMENTS    int a_nSigNum, siginfo_t * /*a_pSigInfo*/, void *
typedef void(*TYPE_SIG_HANDLER)(SIGNAL_ARGUMENTS);
#endif // #ifdef WIN32

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
	m_resourceThread = std::thread(&matlab::engine::ServerBase::ResourceThread, this);
	m_serverThread = std::thread(&matlab::engine::ServerBase::ServerThread, this);
	return 0;
}


void  matlab::engine::ServerBase::StopServer(void)
{
	SResourceJob newJob;
	m_nRun = 0;
	StopServerPrivate();
	m_serverThread.join();
	newJob.code = RESRC_REQ_TYPES::FINISH_LOOP;
	newJob.arg = NULL;
	m_jobQueuee.AddElement(newJob);
	m_semaphoreForResource.post();
	m_resourceThread.join();
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
		nReadReturn = (*m_fpReceive)(a_item->senderReceiver,&aInfo, 8, TIMEOUT_TIME_MS);
		if (nReadReturn != 8) { break; }

		switch (aInfo.overallMin8)
		{
		case CLIENT_REQ_TYPES::MATLAB_PIPE_CLOSE:
			a_item->run = 0;
			break;
		default:  // default is calling matlab routine
			a_item->mutexForBuffers.lock();
			a_item->serializer.Resize(aInfo.overallMin8, aInfo.numberOfOutsOrError);
			nReadReturn = (*m_fpReceive)(a_item->senderReceiver, a_item->serializer.GetBufferForReceive(), 
				aInfo.overallMin8, TIMEOUT_TIME_MS);
			a_item->mutexForBuffers.unlock();
			m_pMatHandle->CallOnMatlabThreadC(this,&matlab::engine::ServerBase::CallMatlabFunction, (void*)a_item);
			break;
		}
	} // while (s_nRun)

	(*m_fpClose)(a_item->senderReceiver);
}


void matlab::engine::ServerBase::CallMatlabFunction(void* a_arg)
{
	struct SConnectionItem* pItem = (struct SConnectionItem*)a_arg;
	int nReturn(-1);
	const char* cpcInfoOrError = "Error";
	int i;
	const u_char_ttt* pRetBuffer = NULL;
	int32_ttt  retBufLen = 0;
	mxArray* pSerializedInputCell;
	mxArray* pInputCell = NULL;
	mxArray* pSerializedOutputCell = NULL;
	mxArray* pReturnFromCallMatlab = NULL;
	mxArray* vInputs[MAXIMUM_NUMBER_OF_IN_AND_OUTS];
	mxArray* vOutputs[MAXIMUM_NUMBER_OF_IN_AND_OUTS];
	void* pSerializedData;
	int nInputs;
	int nOutputs;
	char vcDebug[256];

	int32_ttt nBSL;

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

	if (pItem->serializer.InOutBytesNumber() <= 0) { goto returnPoint; }
	try {
		nBSL = pItem->serializer.InOutBytesNumber();
		snprintf(vcDebug, 255, "fprintf(1,'nBSL=%d\n');drawnow;", (int)nBSL);
		m_pMatHandle->mexEvalStringWithTrap(vcDebug);
		pSerializedInputCell = m_pMatHandle->mxCreateNumericMatrixC(1, pItem->serializer.InOutBytesNumber(), mxUINT8_CLASS, mxREAL);
		if (!pSerializedInputCell) {/*report*/goto returnPoint; }
		pSerializedData = m_pMatHandle->mxGetData(pSerializedInputCell);
		memcpy(pSerializedData, pItem->serializer.InputsOrOutputs(), nBSL);

		pReturnFromCallMatlab = m_pMatHandle->mexCallMATLABWithTrapC(1, &pInputCell, 1, &pSerializedInputCell, "getArrayFromByteStream");
		if (pReturnFromCallMatlab) { goto exceptionPoint; }
		if (!pInputCell) {/*report*/goto returnPoint; }

		nInputs = (int)mxGetN(pInputCell);
		//nInputs *= (int)mxGetM(pInputCell);
		nInputs = nInputs > MAXIMUM_NUMBER_OF_IN_AND_OUTS ?
			MAXIMUM_NUMBER_OF_IN_AND_OUTS : nInputs;
		nOutputs = pItem->serializer.NumOfExpOutsOrError() > MAXIMUM_NUMBER_OF_IN_AND_OUTS ?
			MAXIMUM_NUMBER_OF_IN_AND_OUTS : pItem->serializer.NumOfExpOutsOrError();

		if ((nInputs == 0) && (nOutputs == 0)) // eval string is called
		{
			pReturnFromCallMatlab = m_pMatHandle->mexEvalStringWithTrap(pItem->serializer.MatlabScriptName());
		}
		else
		{
			for (i = 0; i < nInputs; ++i) { vInputs[i] = m_pMatHandle->mxGetCellC(pInputCell, i); }
			pReturnFromCallMatlab = m_pMatHandle->mexCallMATLABWithTrapC(nOutputs, vOutputs, nInputs, vInputs, pItem->serializer.MatlabScriptName());
		}

		if (pReturnFromCallMatlab)
		{
		exceptionPoint:
			pSerializedOutputCell = MatlabArrayToMatlabByteStream(1, &pReturnFromCallMatlab);
			if (!pSerializedOutputCell) { goto returnPoint; }
			pRetBuffer = (const u_char_ttt*)mxGetData(pSerializedOutputCell);
			retBufLen = GetByteStreamLen(pSerializedOutputCell);
			goto returnPoint;
		}

		pSerializedOutputCell = MatlabArrayToMatlabByteStream(nOutputs, vOutputs);
		if (!pSerializedOutputCell) { goto returnPoint; }
		pRetBuffer = (const u_char_ttt*)mxGetData(pSerializedOutputCell);
		retBufLen = GetByteStreamLen(pSerializedOutputCell);
		nReturn = 0;
		cpcInfoOrError = "OK";
		m_pMatHandle->mexEvalStringWithTrap("drawnow");

	}
	catch (...)
	{
		goto returnPoint;
	}

returnPoint:
	if (pReturnFromCallMatlab)
	{
		m_pMatHandle->mexFrprint(STDPIPES::STDERR,
			"Error during calling script by request from remote.\\n"
			"script name is \"%s\"\\n", pItem->serializer.MatlabScriptName());
	}
	pItem->serializer.SetSendParams(cpcInfoOrError, pRetBuffer, retBufLen, nReturn);
	m_fpSend(pItem->senderReceiver, pItem->serializer.GetOverAllBufferForSend(), pItem->serializer.OverAllMinus8Byte() + 8);
	pItem->mutexForBuffers.unlock();
	if (pSerializedInputCell) { m_pMatHandle->mxDestroyArray(pSerializedInputCell); }
	m_pMatHandle->mexEvalStringWithTrap("drawnow");
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
