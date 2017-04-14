/*
 *	File: namedpidebase.h
 *
 *	Created on: Sep 24, 2016
 *	Author    : Davit Kalantaryan (Email: davit.kalantaryan@desy.de)
 *
 *  This file implements all functions ...
 *		1) ...
 *		2) ...
 *		...
 *
 *
 */
#ifndef __namedpipebase_h__
#define __namedpipebase_h__

#ifndef DEFAULT_PIPE_SIZE
#define DEFAULT_PIPE_SIZE	1024
#endif

#include <stddef.h>
#ifdef WIN32
#include <windows.h>
#ifndef PIPE_TYPE_defined
typedef HANDLE PIPE_TYPE;
#define PIPE_TYPE_defined
#endif // #ifndef PIPE_TYPE_defined
#else   // #ifdef WIN32
#endif  // #ifdef WIN32


class NamedPipeBase
{
public:
	NamedPipeBase();
	virtual ~NamedPipeBase();

	int				isOpen()const;
	virtual void	ClosePipe();
	int				Read(void* buffer, int buffer_length, bool& bIsMoreData)const;
	int				Write(const void* buffer, int buffer_length)const;

protected:
	PIPE_TYPE	m_Pipe;
};


#endif  // #ifndef __namedpipebase_h__
