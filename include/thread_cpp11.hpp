/*****************************************************************************
 * File:    thread_cpp11.hpp
 * created: 2017 Apr 24
 *****************************************************************************
 * Author:	D.Kalantaryan, Tel:+49(0)33762/77552 kalantar
 * Email:	davit.kalantaryan@desy.de
 * Mail:	DESY, Platanenallee 6, 15738 Zeuthen
 *****************************************************************************
 * Description
 *   ...
 ****************************************************************************/
#ifndef THREAD_CPP11_HPP
#define THREAD_CPP11_HPP

#include <common_defination.h>

#ifdef __CPP11_DEFINED__
#include <thread>
#define STD std
#else  // #ifdef __CPP11_DEFINED__

namespace STD{

#include <stddef.h>
#ifdef WIN32
#include <windows.h>
typedef HANDLE thread_native_handle;
typedef DWORD SYSTHRRETTYPE;
#else
#include <pthread.h>
typedef pthread_t thread_native_handle;
typedef void* SYSTHRRETTYPE;
#endif

typedef void (__THISCALL__ * TypeClbKVoid)(void*owner);

class thread
{
public:
    thread();
    thread(TypeClbKVoid func,void* arg);
    template<typename TClass>
    thread(void (TClass::*a_fpClbK)(),TClass* a_owner);
    template<typename TClass,typename TArg>
    thread(void (TClass::*a_fpClbK)(TArg a_arg),TClass* owner,TArg arg);
    virtual ~thread();

    STD::thread& operator=(const STD::thread& rS);

    void join();

private:
    template<typename TClass,typename TArg>
    struct SArgs{
        SArgs(void (TClass::*a_startRoutine)(TArg a_aarg),TClass* a_owner,const TArg& a_arg);
        void (TClass::*startRoutine)(TArg a_arg);TClass* owner;TArg arg;
    };

private:
    void ConstructThreadVoid(TypeClbKVoid func,void* arg);
    static STD::SYSTHRRETTYPE ThreadStartupRoutine(void* thisThr);

protected:
    thread_native_handle    m_threadHandle;
    TypeClbKVoid            m_startRoutine;
    void*                   m_threadArg;

};

#include "thread_cpp11.tos"

}

#endif // #ifdef __CPP11_DEFINED__

#endif // THREAD_CPP11_HPP
