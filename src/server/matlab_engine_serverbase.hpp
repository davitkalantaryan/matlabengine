/*
 *	File: matlab_engine_serverbase.hpp
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
#ifndef __matlab_engine_serverbase_hpp__
#define __matlab_engine_serverbase_hpp__

#ifndef SERVER_BIG_TIMEOUT_MS
#define SERVER_BIG_TIMEOUT_MS		-1
#endif
#ifndef SMALL_TIMEOUT_MS
#define	SMALL_TIMEOUT_MS		20000
#endif
#define	CASH_AND_MAX_QUEUEE_SIZE	16

#include <memory.h>
#include <common_unnamedsemaphorelite.hpp>
#include "matlab_engine_mathandlebase.hpp"
#include "matlab_engine_serializer.hpp"
#include <thread_cpp11.hpp>
#include <mutex_cpp11.hpp>
#include <common_fifofast.hpp>
#include "matlab_pipe_name.h"

namespace matlab{ namespace engine{

typedef int (*TypeRecvFunc)(void*receiver,void*buffer, int bufSize, long timeoutMs);
typedef int(*TypeSenderFunc)(void*sender, const void*buffer, int bufSize);
typedef void(*TypeCloseFunc)(void*senderReceiver);

namespace CLIENT_REQ_TYPES { enum { CLOSE=MATLAB_PIPE_CLOSE_DEF}; }
namespace RESRC_REQ_TYPES { enum { FINISH_LOOP, NEW_CLIENT, DELETE_CLIENT}; }

class ServerBase
{
	friend struct SConnectionItem;
public:
	ServerBase(
		TypeRecvFunc funcReceive, TypeSenderFunc funcSend,
		TypeCloseFunc funcClose,MatHandleBase* pMatHandle);
	virtual ~ServerBase();

public: // These functions should be called in MATLAB run thread
	int StartMServer(int enngNumber,const std::string& engineCommand); // !=0 is error, and system will not be started
	void StopMServer(void);

	int GetRun();

    void DeteleCurrentClient();

protected:
	void AddClient(void* client);
	void DeteleClient(struct SConnectionItem* client);

private:
	virtual void StartServerPrivate(int engNumber) = 0; // !=0 is error, and system will not be started
	virtual void StopServerPrivate(void) = 0;

private:
	void DeleteItemInResourceThread(SConnectionItem* item);
	void CallMatlabFunction(void* a_arg);
	void ContactThread(struct SConnectionItem* connectionItem);
	void ResourceThread(void);
	void ServerThread(int engNumber);
	//void OutputHandlerThread(void);

	//virtual int CreateServer(void)=0;

protected:
	typedef struct SResourceJob { int code; void* arg; }SResourceJob;
protected:
    STD::thread							m_resourceThread;
    STD::thread							m_serverThread;
	volatile int						m_nRun;
	TypeRecvFunc						m_fpReceive;
	TypeSenderFunc						m_fpSend;
	TypeCloseFunc						m_fpClose;
	MatHandleBase*						m_pMatHandle;
	struct SConnectionItem*				m_firstItem;
	common::UnnamedSemaphoreLite		m_semaphoreForResource;
	common::FifoFast<SResourceJob, CASH_AND_MAX_QUEUEE_SIZE>	m_jobQueuee;
    SConnectionItem*					m_pCurrentItem;
	int									m_nReturnFromServerThread;
	volatile int						m_nServerRuns;
};


struct SConnectionItem{
	SConnectionItem(ServerBase* pParent,void* senderReceiver);
	~SConnectionItem();
	void*						senderReceiver;
	struct SConnectionItem*		prev;
	struct SConnectionItem*		next;
	volatile int				run;
	matlab::engine::Serializer	serializer;
	versioning::FncPointers		vFuncs;
	STD::thread					serverThread;
};

}}


#endif   // #ifndef __matlab_engine_serverbase_hpp__
