/*
 *	File: namedpideserver.h
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
#ifndef __namedpipeserver_h__
#define __namedpipeserver_h__

#include "namedpipebase.h"

class NamedPipeServer : public NamedPipeBase
{
public:
	NamedPipeServer(const char* pipe_name=NULL);
	virtual ~NamedPipeServer(){}

	int CreateServer(const char* pipe_name, int inp_buffer_size=DEFAULT_PIPE_SIZE, int out_buffer_size=DEFAULT_PIPE_SIZE);
	int WaitForConnection()const;

};


#endif  // #ifndef __namedpipeserver_h__
