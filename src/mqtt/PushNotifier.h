/// Locally Message Queue Clinet
#pragma once
#include "Poco/Task.h"
#include "Poco/Net/SocketAddress.h"
#include "Poco/Net/WebSocket.h"
#include "Poco/Net/HTTPClientSession.h"
#include "Poco/SharedPtr.h"
#include <sstream>

namespace Reach {
	class PushNotifier : public Poco::Task
	{
	public:
		PushNotifier();
		~PushNotifier();

		virtual void runTask();
		virtual void cancel();
	protected:
		void initWebSocket();
		void messageArrived();
		void heartBeat();
	private:
		Poco::Net::SocketAddress sa;
		Poco::Net::WebSocket* ws;
		bool stop;
		bool conne_stat;
		std::string clientId;
		const int msgSize = 4096;
		Poco::Net::HTTPClientSession cs;
		std::ostringstream json;
	};
}