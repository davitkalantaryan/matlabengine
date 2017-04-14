/*
 *	File: listener_on_gui_thread_win.cpp 
 *
 *	Created on: 19 Dec, 2016
 *	Author: Davit Kalantaryan (Email: davit.kalantaryan@desy.de)
 *
 *
 */

#include "stdafx.h"
#include "listener_on_gui_thread.hpp"

#ifdef __cplusplus
extern "C"
{
#endif

typedef struct MListener{
	void*			pOwner;
	TYPE_CALLBACK	callBack;
	HWND			hWnd;
}MListener;

static int s_nListenerRegistered = 0;
#ifdef WIN32
static ATOM					s_RegStat;
#else
#endif

static LRESULT CALLBACK WndProcStat3(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);

void PostJobForGUIthread(void* a_pListener, void* a_pOwner, TYPE_CALLBACK a_fpCallback)
{
	MListener* pListener = (MListener*)a_pListener;
	pListener->callBack = a_fpCallback;
	pListener->pOwner = a_pOwner;

	::PostMessage(pListener->hWnd, _COMMON_CODE_, (WPARAM)pListener, (LPARAM)pListener);
}


void* SynchronCallOnGUIthread(void* a_pListener, void* a_pOwner, TYPE_CALLBACK a_fpCallback)
{
	MListener* pListener = (MListener*)a_pListener;
	pListener->callBack = a_fpCallback;
	pListener->pOwner = a_pOwner;

	::SendMessage(pListener->hWnd, _COMMON_CODE_, (WPARAM)pListener, (LPARAM)pListener);
	return pListener->pOwner;
}


void DestroyMListener(void* a_pListener)
{
	MListener* pListener = (MListener*)a_pListener;

	if (!pListener)return;
	DestroyWindow(pListener->hWnd);
	if (s_nListenerRegistered == 1)UnregisterClassW(_CLASS_NAME_L_, GetModuleHandle(0));
	free(pListener);
	--s_nListenerRegistered;
}


void* CreateMListener(void)
{

	MListener* pRet = (MListener*)calloc(1, sizeof(MListener));
	if (!pRet)return NULL;

	HINSTANCE	hInstance = GetModuleHandle(0);
	HICON		hIcon = LoadIcon(NULL, IDI_APPLICATION);
	WNDCLASSEXW wcex;

	if (s_nListenerRegistered == 0)
	{

		wcex.cbSize = sizeof(WNDCLASSEXW);

		wcex.style = CS_HREDRAW | CS_VREDRAW;
		wcex.lpfnWndProc = WndProcStat3;
		wcex.cbClsExtra = 0;
		wcex.cbWndExtra = 0;
		wcex.hInstance = hInstance;
		wcex.hIcon = hIcon;
		wcex.hCursor = LoadCursor(NULL, IDC_ARROW);
		wcex.hbrBackground = GetSysColorBrush(COLOR_3DFACE);
		wcex.lpszMenuName = NULL;
		wcex.lpszClassName = _CLASS_NAME_L_;
		//wcex.lpszClassName	= vcBuffer;
		wcex.hIconSm = hIcon;

		s_RegStat = RegisterClassExW(&wcex);

	}

	if (!s_RegStat)
	{
		free(pRet);
		return NULL;
	}

	//pRet->m_hWnd = CreateWindowExA chi ashxatum, heto haskanal
	pRet->hWnd = CreateWindowExW(
		0,										/* Extended possibilites for variation */
		_CLASS_NAME_L_,							/* Classname */
		//vcBuffer,								/* Classname */
		L"",									/* Title Text */
		WS_OVERLAPPEDWINDOW,					/* default window */
		0,										/* Windows decides the position */
		0,										/* where the window ends up on the screen */
		0,										/* The programs width */
		0,										/* and height in pixels */
		NULL,									/* The window is a child-window to desktop */
		(HMENU)0,								/* Menu */ /* No menu */
		hInstance,								/* Program Instance handler */
		NULL									/* No Window Creation data */
		);

	if (!pRet->hWnd)
	{
		UnregisterClassW(_CLASS_NAME_L_, GetModuleHandle(0));
		//UnregisterClassW(vcBuffer,GetModuleHandle(0));
		free(pRet);
		return NULL;
	}

	ShowWindow(pRet->hWnd, SW_HIDE);

	++s_nListenerRegistered;
	pRet->callBack = NULL;

	return pRet;
}


static LRESULT CALLBACK WndProcStat3(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	void* pRet;
	struct MListener* pListener = (struct MListener*)wParam;
	LRESULT lRet(0);

	switch (message)
	{
	case _COMMON_CODE_:
		pRet = (*pListener->callBack)(pListener->pOwner);
		pListener->pOwner = pRet;
		break;

	default:
		return DefWindowProc(hWnd, message, wParam, lParam);
		break;

	}

	return lRet;

}

#ifdef __cplusplus
}
#endif
