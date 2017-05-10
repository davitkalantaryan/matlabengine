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
#include <common_unnamedsemaphorelite.hpp>

#ifdef __CPP11_DEFINED__
#include <thread>
#define STD std
#else  // #ifdef __CPP11_DEFINED__

#include <stddef.h>
#ifdef WIN32
#include <windows.h>
#else
#include <pthread.h>
#endif

namespace STD{

#ifdef WIN32
typedef ::HANDLE thread_native_handle;
typedef DWORD SYSTHRRETTYPE;
#else
typedef ::pthread_t thread_native_handle;
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
    bool joinable() const;

private:
    void ConstructThreadVoid(TypeClbKVoid func,void* arg);
    void InitAllMembersPrivate();

protected:
    thread_native_handle*    m_pThreadHandle;
    mutable int              m_nDublicates;
};

} // namespace STD{

#include "thread_cpp11.tos"

#endif // #ifdef __CPP11_DEFINED__

#endif // THREAD_CPP11_HPP
