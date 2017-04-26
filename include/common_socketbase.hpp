#ifndef __common_socketbase_hpp__
#define __common_socketbase_hpp__


#ifndef _SOCKET_TIMEOUT_
#define	_SOCKET_TIMEOUT_		-2001
#endif


namespace common {
	class SocketBase
	{
	public:
		SocketBase();
		virtual ~SocketBase();

		virtual int Send(const void* data, int size)=0;
		virtual int Recv(void* data, int size,long timeoutMS)=0;
	};
}

#endif // #ifndef __common_socketbase_hpp__
