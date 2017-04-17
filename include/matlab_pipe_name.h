/*
 *	File: matlab_pipe_name.h 
 *
 *	Created on: 21 Dec, 2016
 *	Author: Davit Kalantaryan (Email: davit.kalantaryan@desy.de)
 *
 *
 */
#ifndef __matlab_pipe_name_h__
#define __matlab_pipe_name_h__

#ifndef getpid_gettid_defined
#ifdef WIN32
#include <windows.h>
#ifdef getpid
#undef getpid
#endif
#define getpid	GetCurrentProcessId
#define	gettid	GetCurrentThreadId
#else
#include <sys/types.h>
#include <unistd.h>
#endif
#endif // #ifndef getpid_gettid_defined

#define	MAX_TRY_NUMBERS			5
#define MATLAB_BIND_PORT_NUMBER	9183

#ifndef NECESSARY_BUF_SIZE
//#define NECESSARY_BUF_SIZE	3840064
#define NECESSARY_BUF_SIZE		4194304
#endif

#ifndef MATLAB_PIPE_COMMANDS

#define		MATLAB_PIPE_CLOSE_DEF		-1
//#define		MATLAB_PIPE_NEW_DATA	1 >0
//#define		MATLAB_MEX_EVAL_STRING	-2

#endif		// #ifndef MATLAB_PIPE_COMMANDS

#ifdef __cplusplus
extern "C"
{
#endif


extern const char* GenerateMatlabPipeName(int pid);
extern const char* GenerateMatlabPipeNameV(int pid, int version);
extern int GenerateMatlabServerPortNumber(int pid, int tryNumber);


#ifdef __cplusplus
}
#endif


#endif  // #ifndef __matlab_pipe_name_h__
