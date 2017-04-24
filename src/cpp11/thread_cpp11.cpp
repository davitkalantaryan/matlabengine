/*****************************************************************************
 * File:    thread_cpp11.cpp
 * created: 2017 Apr 24
 *****************************************************************************
 * Author:	D.Kalantaryan, Tel:+49(0)33762/77552 kalantar
 * Email:	davit.kalantaryan@desy.de
 * Mail:	DESY, Platanenallee 6, 15738 Zeuthen
 *****************************************************************************
 * Description
 *   ...
 ****************************************************************************/

#ifndef __CPP11_DEFINED__

#include "thread_cpp11.hpp"

STD::thread::thread()
    :
      m_threadHandle((thread_native_handle)0),
      m_startRoutine(NULL),
      m_threadArg(NULL)
{
    //
}


STD::thread::thread(TypeClbKVoid a_fpStartFunc,void* a_arg)
{
    ConstructThreadVoid(a_fpStartFunc,a_arg);
}


STD::thread::~thread()
{
    //
}


STD::thread& STD::thread::operator=(const STD::thread& rS)
{
    return *this;
}


void STD::thread::join()
{
#ifdef WIN32
#error Not implemented yet
#else // #ifdef WIN32
    pthread_join(m_threadHandle,NULL);
#endif // #ifdef WIN32
}


void STD::thread::ConstructThreadVoid(TypeClbKVoid a_fpStartFunc,void* a_arg)
{
    m_startRoutine  = a_fpStartFunc;
    m_threadArg     = a_arg;
#ifdef WIN32
#error should be implemented
        unsigned ThreadID;
        m_Thread = (HANDLE)_beginthreadex( NULL, 0, (unsigned int(__stdcall*)(void*))&ThreadDv::ThreadFuncStatic, this, 0, &ThreadID );
#else  // #ifdef WIN32
        pthread_attr_t      tattr;
        pthread_attr_init(&tattr);
        //pthread_attr_setscope(&tattr, PTHREAD_SCOPE_PROCESS);
        pthread_attr_setdetachstate(&tattr, PTHREAD_CREATE_JOINABLE);
        pthread_create( &m_threadHandle,  &tattr, &thread::ThreadStartupRoutine, this );
        pthread_attr_destroy(&tattr);

#endif // #ifdef WIN32
}


STD::SYSTHRRETTYPE STD::thread::ThreadStartupRoutine(void* a_thisThr)
{
    STD::thread* pThread = (STD::thread*)a_thisThr;

    (*(pThread->m_startRoutine))(pThread->m_threadArg);
#ifdef WIN32
#error not implemented
#else // #ifdef WIN32
    pthread_exit(NULL);
#endif // #ifdef WIN32
    return (SYSTHRRETTYPE)0;
}


#endif  // #ifndef __CPP11_DEFINED__
