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


class MatHandleMex : public MatHandleMexBase
{
public:
	MatHandleMex();
	virtual ~MatHandleMex();

	void Start() __OVERRIDE__;
	void Stop() __OVERRIDE__;

	void CallOnMatlabThread(void* owner, TypeClbK fpClb,void*arg) __OVERRIDE__;

private:
	static void* ListenerCallbackStatic(void* a_arg);

private:
	void*			m_pListener;

};

}}

#endif // #ifndef __matlab_engine_mathandlemex_hpp__
