
#include <asockettcp.h>
#include <string.h>
#include "matlab_engine_clienttcp.hpp"
#include "matlab_engine_serializer.hpp"
#include "mat_matrix.h"

#define _SERI_RAW1 matlab::engine::versioning::SERI_TYPE::RAW1
typedef const void* TypeConstVoidPtr;

int main()
{
	int nRet;

	ASocketB::Initialize();

#if 1
	PersistantArgsGroup vArray(1);
	int32_ttt nOutputs;
	int nReceived;
	matlab::engine::Serializer	serializeDes(NULL, CURRENT_SERIALIZER_VERSION2, _SERI_RAW1);
	matlab::engine::ClientTcp	socketTcp;
	matlab::engine::versioning::FncPointers vFuncs;

	nRet = socketTcp.ConnectToServer("localhost", 0);
	if (nRet) {goto returnPoint;}

	vArray.SetValue(0, 11.);

#if 1
	nReceived = serializeDes.SendScriptNameAndArrays(
		&vFuncs, &socketTcp,
		"close all", 0, 0, (TypeConstVoidPtr*)(vArray.Array()));
#else
	nReceived = serializeDes.SendScriptNameAndArrays(
		&vFuncs, &socketTcp,
		"figure", 0, 1, (TypeConstVoidPtr*)(vArray.Array()));
#endif

	if (nReceived < 0) { goto returnPoint; }

	nReceived = serializeDes.ReceiveHeaderScriptNameAndArrays2(
		&vFuncs, &socketTcp, -1,
		1, (void**)(vArray.Array()), &nOutputs);

returnPoint:
	socketTcp.Close();

#else // #if 1

	const char* cpcNewTask = "figure(getArrayFromByteStream(getByteStreamFromArray(11)))";
	//const char* cpcNewTask = "path";
	ASocketTCP aSocket;
	int nSize = (int)strlen(cpcNewTask);
	
	nRet = aSocket.CreateClient("wgs13", 1980);
	if (nRet) {goto retPoint;}
	aSocket.SendData(&nSize, 4);
	aSocket.SendData(cpcNewTask, nSize);
	aSocket.Close();

retPoint:
#endif // #if 0/1

	ASocketB::Cleanup();
	return 0;
}
