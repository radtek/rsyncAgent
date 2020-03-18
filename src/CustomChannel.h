#pragma once
#include "Poco/Channel.h"
#include "Poco/Util/Application.h"
#include "Poco/Net/Socket.h"
#include "Poco/Net/WebSocket.h"
#include "Poco/Mutex.h"

namespace Reach {
	using namespace Poco;
	using namespace Poco::Net;
	class CustomChannel : public Channel
	{
		struct SocketInfo
		{
			Socket ws;
			int flag;
		};
	public:
		CustomChannel();
		void addSocket(const Socket& ws, int flag = WebSocket::FRAME_TEXT);
		void removeSocket(const Socket & ws);
		void log(const Message& msg);
	protected:
		~CustomChannel();
		SocketInfo* find(const Socket & ws);
	private:
		FastMutex _socketMutex;
		typedef std::vector<SocketInfo*> SocketList;
		SocketList _waitList;
	};
}