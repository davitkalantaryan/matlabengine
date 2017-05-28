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
#include <common_argument_parser.hpp>

#ifdef WIN32
#else  // #ifdef WIN32
#include <sys/types.h>
#include <sys/stat.h>
typedef void(*TYPE_SIG_HANDLER)(int);
//static void SignalHandlerSimple(int);
#endif // #ifdef WIN32

static volatile int s_nRun;

int main(int a_argc, char* a_argv[])
{
	int nReturn,nFork(1);
    int nEngineNumber(0);
	const char* cpcEngineCommand = "";
    common::argument_parser aParser;

    aParser.AddOption("--engine-number",1,"0");
	aParser.AddOption("--fork", 1,"1");
	aParser<<"--no-fork";
    aParser.AddOption("-en",1,"0");
    aParser<<"--help"<<"-h";

    if(a_argc>1)
    {
		char** argv = a_argv + 1;
        int nArgs(a_argc-1);
        aParser.ParseCommandLine<int&,char*>(nArgs, argv);
		if (nArgs) { cpcEngineCommand = argv[0]; }
    }

    if(aParser["--help"] || aParser["-h"]){
        printf("%s\n", aParser.HelpString().c_str());
        return 0;
    }

    if(aParser["--engine-number"]){nEngineNumber=atoi(aParser["--engine-number"]);}
    else if(aParser["-en"]){nEngineNumber=atoi(aParser["-en"]);}

	if (aParser["--fork"]) { nFork = 1;nFork = atoi(aParser["--fork"]); }
	if (aParser["--no-fork"]) { nFork = 0; }

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
#endif  // #if 0
#endif // #ifdef WIN32

    //freopen( "/dev/null", "w", stderr);
    //freopen( "/dev/null", "w", stdout);
	printf("fork=%d, engineNumber=%d, engineCommand=%s\n",nFork, nEngineNumber, cpcEngineCommand);

	if (nFork)
	{
#ifdef WIN32
#else // #ifdef WIN32
		if (fork())exit(0);
		//if( 0 != chdir( WORKING_DIR ) ){fprintf( stderr, "chdir failed!!!" ); return 1;}
		setsid();
		umask(0);

		stdin = freopen("/dev/null", "r", stdin);
		stdout = freopen("/dev/null", "w", stdout);
		//stderr = freopen( "/dev/null", "w", stderr);

		if (fork())exit(0);
#endif // #ifdef WIN32
	}

    matlab::engine::ServerTcpEng aServer;
	nReturn = aServer.StartMServer(nEngineNumber, cpcEngineCommand);

	if (nReturn) {
		fprintf(stderr,"EngineNumber=%d, Unable to bind to the port!\n", nEngineNumber);
		goto returnPoint;
	}

    s_nRun = 1;

    while(s_nRun){

        Sleep(100000);
    }

returnPoint:
    aServer.StopMServer();

    return nReturn;
}


#ifdef WIN32
#else // #ifdef WIN32
static void SignalHandlerSimple(int)
{
    s_nRun = 0;
}
#endif // #ifdef WIN32
