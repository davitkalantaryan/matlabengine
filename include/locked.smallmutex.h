#ifndef SMALLMUTEX_H
#define SMALLMUTEX_H


#ifdef WIN32
        #include <WINDOWS.H>
        //#include <ERRNO.H>
        //#define	EBUSY	1
        //#define	EPERM	1
        //#define	EDEADLK	1
#else
        #include <pthread.h>
#endif

class SmallMutex
{
#ifdef WIN32
        HANDLE			m_MutexLock;
public:
        SmallMutex(){m_MutexLock = CreateMutex( NULL, FALSE, NULL );}
        ~SmallMutex(){CloseHandle( m_MutexLock );}
        int Lock(){return WaitForSingleObject( m_MutexLock, INFINITE ) == WAIT_OBJECT_0 ? 0 : 1 ;}
        int UnLock(){return ReleaseMutex( m_MutexLock ) ? 0 : 1;}
#else/* #ifdef WIN32 */
        pthread_mutex_t	m_MutexLock;
public:
        SmallMutex()
        {
                pthread_mutexattr_t attr;
                pthread_mutexattr_init( &attr );
                pthread_mutexattr_settype( &attr, PTHREAD_MUTEX_ERRORCHECK );
                pthread_mutex_init( &m_MutexLock, &attr );
                pthread_mutexattr_destroy( &attr );
        }
        ~SmallMutex(){pthread_mutex_destroy( &m_MutexLock );}
        int Lock(){return pthread_mutex_lock( &m_MutexLock );}
        int UnLock(){return pthread_mutex_unlock( &m_MutexLock );}
#endif/* #ifdef WIN32*/
};

#endif // SMALLMUTEX_H
