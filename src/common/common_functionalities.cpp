/*****************************************************************************
 * File:    common_defination.h
 * created: 2017 Apr 24
 *****************************************************************************
 * Author:	D.Kalantaryan, Tel:+49(0)33762/77552 kalantar
 * Email:	davit.kalantaryan@desy.de
 * Mail:	DESY, Platanenallee 6, 15738 Zeuthen
 *****************************************************************************
 * Description
 *   ...
 ****************************************************************************/

#include <stdarg.h>

#ifdef __cplusplus
extern "C"{
#endif

typedef void* type_void_ptr;

void* GetFuncPointer_common(int a_fa,...)
{
    type_void_ptr pRet;
    va_list argList;

    va_start(argList, a_fa);
    pRet = va_arg(argList, type_void_ptr);
    va_end(argList);
    return pRet;
}

#ifdef __cplusplus
}
#endif
