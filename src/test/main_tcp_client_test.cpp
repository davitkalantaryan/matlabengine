
#include <asockettcp.h>
#include <string.h>

int main()
{
	const char* cpcNewTask = "figure(getArrayFromByteStream(getByteStreamFromArray(11)))";
	//const char* cpcNewTask = "path";
	ASocketTCP aSocket;
	int nRet,nSize = (int)strlen(cpcNewTask);
	
	ASocketB::Initialize();
	nRet = aSocket.CreateClient("wgs13", 1980);
	if (nRet) {goto retPoint;}
	aSocket.SendData(&nSize, 4);
	aSocket.SendData(cpcNewTask, nSize);
	aSocket.Close();

retPoint:
	ASocketB::Cleanup();
	return 0;
}
