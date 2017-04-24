/*****************************************************************************
 * File:    mutex_cpp11.hpp
 * created: 2017 Apr 24
 *****************************************************************************
 * Author:	D.Kalantaryan, Tel:+49(0)33762/77552 kalantar
 * Email:	davit.kalantaryan@desy.de
 * Mail:	DESY, Platanenallee 6, 15738 Zeuthen
 *****************************************************************************
 * Description
 *   ...
 ****************************************************************************/
#ifndef MUTEX_CPP11_HPP
#define MUTEX_CPP11_HPP

#include <common_defination.h>

#ifdef __CPP11_DEFINED__
#include <mutex>
#define STD std
#else  // #ifdef __CPP11_DEFINED__

namespace STD{

#include <stddef.h>
#ifdef WIN32
#include <windows.h>
typedef HANDLE  mutex_native_handle;
#else
#include <pthread.h>
typedef pthread_mutex_t mutex_native_handle;
#endif

class mutex
{
public:
    mutex();
    virtual ~mutex();

    void lock();
    void unlock();

private:
    mutex_native_handle m_mutexLock;
};

}

#endif // #ifdef __CPP11_DEFINED__

#endif // MUTEX_CPP11_HPP
