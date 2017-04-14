/*
 *	File: listener_on_gui_thread.hpp 
 *
 *	Created on: 19 Dec, 2016
 *	Author: Davit Kalantaryan (Email: davit.kalantaryan@desy.de)
 *
 *
 */

#ifndef __listener_on_gui_thread_hpp__
#define __listener_on_gui_thread_hpp__

#ifndef _CLASS_NAME_L_
#define		_CLASS_NAME_L_	L"MATLAB_Listener"
#endif

#ifndef _COMMON_CODE_
#ifdef WIN32
#define		_COMMON_CODE_		WM_USER+2
#else
#endif  // #ifdef WIN32
#endif  // #ifndef _COMMON_CODE_

#ifdef WIN32
#include <windows.h>
#else
#include <unistd.h>
#endif


#ifdef __cplusplus
extern "C"
{
#endif

typedef void*(*TYPE_CALLBACK)(void*);
void* CreateMListener(void);
void DestroyMListener(void* a_pListener);
void PostJobForGUIthread(void* a_pListener, void* a_pOwner, TYPE_CALLBACK a_fpCallback);
void* SynchronCallOnGUIthread(void* a_pListener, void* a_pOwner, TYPE_CALLBACK a_fpCallback);

#ifdef __cplusplus
}
#endif


#endif  // #ifndef __listener_on_gui_thread_hpp__
