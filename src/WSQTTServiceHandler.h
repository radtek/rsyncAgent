#pragma once
#include "Poco/Net/StreamSocket.h"
#include "Poco/Net/SocketReactor.h"
#include "Poco/Net/SocketNotification.h"
#include "Poco/Net/WebSocket.h"
#include "Poco/Observer.h"
#include "Poco/Mutex.h"

namespace Reach {
	using namespace Poco;
	using namespace Poco::Net;

	class WSQTTServiceHandler
	{
	public:
		WSQTTServiceHandler(StreamSocket& socket, SocketReactor& reactor);
		~WSQTTServiceHandler();

		void onReadable(ReadableNotification* pNf);
		void onWritable(WritableNotification* pNf);
		void onTimeout(TimeoutNotification* pNf);
		void onShutdown(ShutdownNotification * pNf);
	private:
		WebSocket			_socket;
		SocketReactor&		_reactor;
		FastMutex			_Mutex;
	};
}