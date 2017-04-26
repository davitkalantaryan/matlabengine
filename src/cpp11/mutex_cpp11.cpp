/*****************************************************************************
 * File:    mutex_cpp11.cpp
 * created: 2017 Apr 24
 *****************************************************************************
 * Author:	D.Kalantaryan, Tel:+49(0)33762/77552 kalantar
 * Email:	davit.kalantaryan@desy.de
 * Mail:	DESY, Platanenallee 6, 15738 Zeuthen
 *****************************************************************************
 * Description
 *   ...
 ****************************************************************************/

#include "mutex_cpp11.hpp"

#ifndef __CPP11_DEFINED__

STD::mutex::mutex()
{
#ifdef WIN32
    m_mutexLock = CreateMutex( NULL, FALSE, NULL );
#else
    pthread_mutexattr_t attr;
    pthread_mutexattr_init( &attr );
    pthread_mutexattr_settype( &attr, PTHREAD_MUTEX_ERRORCHECK );
    pthread_mutex_init( &m_mutexLock, &attr );
    pthread_mutexattr_destroy( &attr );
#endif
}


STD::mutex::~mutex()
{
#ifdef WIN32
    CloseHandle( m_mutexLock );
#else
    pthread_mutex_destroy( &m_mutexLock );
#endif
}


void STD::mutex::lock()
{
#ifdef WIN32
    WaitForSingleObject( m_mutexLock, INFINITE );
#else
    pthread_mutex_lock( &m_mutexLock );
#endif
}


void STD::mutex::unlock()
{
#ifdef WIN32
    ReleaseMutex( m_mutexLock );
#else
    pthread_mutex_unlock( &m_mutexLock );
#endif
}


#endif  // #ifndef __CPP11_DEFINED__

