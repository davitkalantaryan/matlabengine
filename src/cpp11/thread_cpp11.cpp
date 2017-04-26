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

#include "thread_cpp11.hpp"

#ifndef __CPP11_DEFINED__

static STD::SYSTHRRETTYPE ThreadStartupRoutine(void* a_thisThr);

namespace STD{struct SThreadArgs{
    SThreadArgs(TypeClbKVoid a_stFnc,void* a_thrArg):startRoutine(a_stFnc),thrArg(a_thrArg){}
    TypeClbKVoid startRoutine;void* thrArg;};}

STD::thread::thread()
{
    InitAllMembersPrivate();
}


STD::thread::thread(TypeClbKVoid a_fpStartFunc,void* a_arg)
{
    InitAllMembersPrivate();
    ConstructThreadVoid(a_fpStartFunc,a_arg);
}


STD::thread::~thread()
{
    if(0==(--m_nDublicates)){
        join();
    }
}


void STD::thread::InitAllMembersPrivate()
{
    m_pThreadHandle = new thread_native_handle;
    if(!m_pThreadHandle){throw "Low memory!";}
    *m_pThreadHandle = (thread_native_handle)0;
    m_nDublicates = 1;
}


STD::thread& STD::thread::operator=(const STD::thread& a_rS)
{
    m_nDublicates = (++a_rS.m_nDublicates);
    m_pThreadHandle = a_rS.m_pThreadHandle;

    return *this;
}


bool STD::thread::joinable() const
{
    if(m_pThreadHandle && *m_pThreadHandle)
    {
        return true;
    }

    return false;
}


void STD::thread::join()
{
    if(joinable())
    {
#ifdef WIN32
#error Not implemented yet
#else // #ifdef WIN32
        pthread_join(*m_pThreadHandle,NULL);
#endif // #ifdef WIN32

        *m_pThreadHandle = (thread_native_handle)0;
    }
}


void STD::thread::ConstructThreadVoid(TypeClbKVoid a_fpStartFunc,void* a_arg)
{

    if((!m_pThreadHandle)||(*m_pThreadHandle)){return;}

    m_nDublicates = 1;
    SThreadArgs* pArgs = new SThreadArgs(a_fpStartFunc,a_arg);

#ifdef WIN32
#error should be implemented
    unsigned ThreadID;
    m_Thread = (HANDLE)_beginthreadex( NULL, 0, (unsigned int(__stdcall*)(void*))&ThreadDv::ThreadFuncStatic, this, 0, &ThreadID );
#else  // #ifdef WIN32
    pthread_attr_t      tattr;
    pthread_attr_init(&tattr);
    //pthread_attr_setscope(&tattr, PTHREAD_SCOPE_PROCESS);
    pthread_attr_setdetachstate(&tattr, PTHREAD_CREATE_JOINABLE);
    pthread_create( m_pThreadHandle,  &tattr, &ThreadStartupRoutine,pArgs );
    pthread_attr_destroy(&tattr);

#endif // #ifdef WIN32

}


static STD::SYSTHRRETTYPE ThreadStartupRoutine(void* a_thrArgs)
{
    STD::SThreadArgs* pArgs = (STD::SThreadArgs*)a_thrArgs;

    (*(pArgs->startRoutine))(pArgs->thrArg);

#ifdef WIN32
#error not implemented
#else // #ifdef WIN32
    ::pthread_exit(NULL);
#endif // #ifdef WIN32

    delete pArgs;
    return (STD::SYSTHRRETTYPE)0;
}


#endif  // #ifndef __CPP11_DEFINED__
