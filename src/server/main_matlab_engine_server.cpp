/*****************************************************************************
 * File   : main_matlab_engine_server.cpp
 * created: 2017 Apr 24
 *****************************************************************************
 * Author:	D.Kalantaryan, Tel:+49(0)33762/77552 kalantar
 * Email :	davit.kalantaryan@desy.de
 * Mail  :	DESY, Platanenallee 6, 15738 Zeuthen
 *****************************************************************************
 * Description
 *   ...
 ****************************************************************************/

#include "matlab_engine_servertcpeng.hpp"
#include <signal.h>

#ifdef WIN32
#else  // #ifdef WIN32
typedef void(*TYPE_SIG_HANDLER)(int);
//static void SignalHandlerSimple(int);
#endif // #ifdef WIN32

static volatile int s_nRun;

int main()
{
#ifdef WIN32
#else
#if 0
    struct sigaction sigAction, sigActionOld;
    sigemptyset(&sigAction.sa_mask);
    //sigAction.sa_flags = SA_SIGINFO;
    //sigAction.sa_sigaction = (TYPE_SIG_HANDLER)SignalHandlerSimple;
    sigAction.sa_flags = 0;
    sigAction.sa_handler = (TYPE_SIG_HANDLER)SignalHandlerSimple;
    sigaction(SIGINT, &sigAction, &sigActionOld);
#endif
#endif // #ifdef WIN32

    freopen( "/dev/null", "w", stderr);
    //freopen( "/dev/null", "w", stdout);

    matlab::engine::ServerTcpEng aServer;
    aServer.StartMServer();
    s_nRun = 1;

    while(s_nRun){

#ifdef WIN32
        Sleep(100000);
#else  // #ifdef WIN32
        usleep(100000000);
#endif  // #ifdef WIN32
    }

    aServer.StopMServer();

    return 0;
}


#ifdef WIN32
#else // #ifdef WIN32
static void SignalHandlerSimple(int)
{
    s_nRun = 0;
}
#endif // #ifdef WIN32
