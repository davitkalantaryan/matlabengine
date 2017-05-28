/*
 *	File: matlab_engine_mathandlemex.hpp
 *
 *	Created on: 14 Apr, 2017
 *	Author    : Davit Kalantaryan (Email: davit.kalantaryan@desy.de)
 *
 *
 */

#ifndef __matlab_engine_mathandlemex_hpp__
#define __matlab_engine_mathandlemex_hpp__

#include "matlab_engine_mathandlemexbase.hpp"

namespace matlab{ namespace engine{

struct SLsnCallbackItem { void*owner; TypeClbK clbk; void*arg; };

class MatHandleMex : public MatHandleMexBase
{
public:
	MatHandleMex();
	virtual ~MatHandleMex();

	int Start(const std::string& engineCommand) __OVERRIDE__;
	void Stop() __OVERRIDE__;

	void SyncCallOnMatlabThread(void* owner, TypeClbK fpClb,void*arg) __OVERRIDE__;
	void AsyncCallOnMatlabThread(void*, TypeClbK, void*) __OVERRIDE__;

private:
	static void* ListenerCallbackStatic(void* a_arg);
	void AddMatlabJob(void* owner, TypeClbK fpClb, void* arg);
	void HandleAllJobs(void);

private:
	void*									m_pListener;
	common::FifoFast<SLsnCallbackItem, 8>	m_fifoJobs;

};

}}

#endif // #ifndef __matlab_engine_mathandlemex_hpp__
