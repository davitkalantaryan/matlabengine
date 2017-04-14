/*
 *	File: namedpidebase.cpp
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
#include "namedpipebase.h"

NamedPipeBase::NamedPipeBase()
	: m_Pipe(INVALID_HANDLE_VALUE)
{
}


NamedPipeBase::~NamedPipeBase()
{
	ClosePipe();
}


void NamedPipeBase::ClosePipe()
{
	if (m_Pipe != INVALID_HANDLE_VALUE)
	{
		DisconnectNamedPipe(m_Pipe);
		CloseHandle(m_Pipe);
		m_Pipe = INVALID_HANDLE_VALUE;
	}
}


int NamedPipeBase::Read(void* a_buffer, int a_buffer_length, bool& a_bIsMoreData)const
{
	a_bIsMoreData = false;
	DWORD dwSize;
	BOOL bRet = ReadFile(m_Pipe, a_buffer, a_buffer_length, &dwSize, NULL);

	if (bRet == FALSE)
	{
		int nReturn = GetLastError();

		switch (nReturn)
		{
		case ERROR_MORE_DATA:
			a_bIsMoreData = true;
			return a_buffer_length;
		case ERROR_NO_DATA:
			DisconnectNamedPipe(m_Pipe);
		default:
			return nReturn >= 0 ? -nReturn : nReturn;
		}
	}
	return (int)dwSize;
}


int NamedPipeBase::Write(const void* a_buffer, int a_buffer_length)const
{
	DWORD dwSize;
	BOOL bRet = WriteFile(m_Pipe, a_buffer, a_buffer_length, &dwSize, NULL);

	if (bRet == FALSE)
	{
		int nReturn = GetLastError();
		return nReturn >= 0 ? -nReturn : nReturn;
	}
	return (int)dwSize;
}


int NamedPipeBase::isOpen()const
{
	return (m_Pipe != INVALID_HANDLE_VALUE);
}
