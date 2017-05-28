/*****************************************************************************
 * File   : rpc_matlab_engine_mngr_doocs_server.cpp
 * created: 2017 May 02
 *****************************************************************************
 * Author:	D.Kalantaryan, Tel:+49(0)33762/77552 kalantar
 * Email :	davit.kalantaryan@desy.de
 * Mail  :	DESY, Platanenallee 6, 15738 Zeuthen
 *****************************************************************************
 * Description
 *   ...
 ****************************************************************************/

 /*
 *  File       :	main_matlabengine_doocs_server.cpp
 *  Created on :	17 Dec 2016
 *  Created by :	Davit Kalantaryan  (davit.kalantaryan@desy.de)
 *
 */

#include "matlab_engine_mngr_eqfctmngr.hpp"
#include "matlab_engine_mngr_eqfctproxy.hpp"

const char*	object_name = "matlab_engine_mngr_doocs_server";
void interrupt_usr1_prolog(int) {}
void	eq_init_epilog() {}
void eq_cancel(void) {}
void post_init_prolog(void) {}
void	eq_init_prolog() {}

#ifdef WIN32
#include <conio.h>
#endif

EqFct* eq_create(int a_eq_code, void* a_arg)
{
	::EqFct* pRet = NULL;
	//const char* cpcNameString;
#ifdef WIN32
	DEBUG_APP_RAW2(2, "Press any key to continue!");
	DEBUG_APP_RAW0(2, getch);
	DEBUG_APP_RAW2(2, "\n");
#endif

	switch (a_eq_code)
	{
	case EQFCT_PROXY_CODE:
#ifdef WIN32
		DEBUG_APP_RAW2(1, "Press any key to continue!");
		DEBUG_APP_RAW0(1,getch);
		DEBUG_APP_RAW2(1,"\n");
#endif
		DEBUG_APP_RAW2(1,"!!!!!!!!!!!!!EQFCT_PROXY_CODE:%d\n", (int)((size_t)a_arg));
		pRet = new matlab::engine::mngr::EqFctProxy((int)((size_t)a_arg));
		break;
	case EQFCT_MNGR_CODE:
		pRet = new matlab::engine::mngr::EqFctMngr;
		break;
	default: break;
	}
	return pRet;
}

void refresh_prolog() {}
void refresh_epilog() {}	// called after "update"
void post_init_epilog(void) {}
void interrupt_usr1_epilog(int) {}
