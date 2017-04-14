/*
 *	File: matlab_engine_mathandlebase.cpp
 *
 *	Created on: 12 Apr, 2017
 *	Author    : Davit Kalantaryan (Email: davit.kalantaryan@desy.de)
 *
 *
 */

#include "matlab_engine_mathandlebase.hpp"
#include <stddef.h>

matlab::engine::MatHandleBase::~MatHandleBase()
{
}


int matlab::engine::MatHandleBase::mexFrprint(int a_out, const char* a_fmt, ...)
{
	int nReturn;
	char vcBufferIn[4096];
	char vcBufferFin[4096];
	va_list argList;

	va_start(argList, a_fmt);
	nReturn=vsnprintf(vcBufferIn, 4095, a_fmt, argList);
	va_end(argList);

	a_out = a_out < 1 ? 1 : (a_out > 2 ? 2 : a_out);
	snprintf(vcBufferFin, 4095, "fprintf(%d,'%s');", a_out, vcBufferIn);
	mexEvalStringWithTrap(vcBufferFin);

	return nReturn;
}


namespace matlab{ namespace engine{

typedef void* type_void_ptr;

void* GetFuncPointer(int a_fa,...)
{
	type_void_ptr pRet;
	va_list argList;

	va_start(argList, a_fa);
	pRet = va_arg(argList, type_void_ptr);
	va_end(argList);
	return pRet;
}

}}
