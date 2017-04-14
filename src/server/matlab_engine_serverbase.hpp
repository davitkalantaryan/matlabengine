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

#define TIMEOUT_TIME_MS				20000
#define	CASH_AND_MAX_QUEUEE_SIZE	16

#include "matlab_engine_serializer.hpp"
#include <thread>
#include <mutex>
#include "matlab_engine_mathandlebase.hpp"
#include <memory.h>
#include <common_unnamedsemaphorelite.hpp>
#include <common_fifofast.hpp>

namespace matlab{ namespace engine{

typedef int (*TypeRecvFunc)(void*receiver,void*buffer, int bufSize, long timeoutMs);
typedef int(*TypeSenderFunc)(void*sender, const void*buffer, int bufSize);
typedef void(*TypeCloseFunc)(void*senderReceiver);

namespace CLIENT_REQ_TYPES { enum { MATLAB_PIPE_CLOSE }; }
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
	virtual int StartServer(void); // !=0 is error, and system will not be started
	virtual void StopServer(void);

protected:
	void AddClient(void* client);
	void DeteleClient(struct SConnectionItem* client);

private:
	virtual int StartServerPrivate(void) = 0; // !=0 is error, and system will not be started
	virtual void StopServerPrivate(void) = 0;

private:
	void DeleteItemInResourceThread(SConnectionItem* item);
	void CallMatlabFunction(void* a_arg);
	void ContactThread(struct SConnectionItem* connectionItem);
	void ResourceThread(void);
	void ServerThread(void);
	//void OutputHandlerThread(void);

	//virtual int CreateServer(void)=0;

protected:
	typedef struct SResourceJob { int code; void* arg; }SResourceJob;
protected:
	std::thread							m_resourceThread;
	std::thread							m_serverThread;
	volatile int						m_nRun;
	TypeRecvFunc						m_fpReceive;
	TypeSenderFunc						m_fpSend;
	TypeCloseFunc						m_fpClose;
	MatHandleBase*						m_pMatHandle;
	struct SConnectionItem*				m_firstItem;
	common::UnnamedSemaphoreLite		m_semaphoreForResource;
	common::FifoFast<SResourceJob, CASH_AND_MAX_QUEUEE_SIZE>	m_jobQueuee;
	SConnectionItem*					m_pCurrentItem;
};


struct SConnectionItem{
	SConnectionItem(ServerBase* pParent,void* senderReceiver);
	~SConnectionItem();
	void*						senderReceiver;
	volatile int				run;
	matlab::engine::Serializer	serializer;
	std::mutex					mutexForBuffers;
	std::thread					serverThread;
	struct SConnectionItem*		prev;
	struct SConnectionItem*		next;
};

}}


#endif   // #ifndef __matlab_engine_serverbase_hpp__
