/// Locally Message Queue Clinet
#pragma once
#include "Poco/Net/SocketNotification.h"
#include "Poco/Net/SocketConnector.h"
#include "Poco/Net/StreamSocket.h"
#include "Poco/Net/SocketReactor.h"
#include "Poco/Net/WebSocket.h"
#include "Poco/Logger.h"
#include <sstream>

namespace Reach {
	class WSQTTHandler
	{
	public:
		WSQTTHandler(Poco::Net::StreamSocket& socket, Poco::Net::SocketReactor& reactor);
		~WSQTTHandler();

		void onReadable(Poco::Net::ReadableNotification* pNf);
		void onTimeout(Poco::Net::TimeoutNotification* pNf);
		void onShutdown(Poco::Net::ShutdownNotification* pNf);
		void onError(Poco::Net::ErrorNotification* pNf);

	private:
		void initialize();
		void loadConfiguration();

		void addEventHandler();
		void removeEventHandler();

		void checkSocketAlive();

	private:
		std::string	 _str;
		std::string  _beat;

		std::string			_clientId;
		std::string			_peerClientId;
		std::string			_servicePath;
		int					_timeoutSecond;
		Poco::Net::StreamSocket		_socket;
		Poco::Net::SocketReactor&	_reactor;
		Poco::Net::WebSocket*		_pWS;

		Poco::Observer<WSQTTHandler, Poco::Net::ReadableNotification> _or;
		Poco::Observer<WSQTTHandler, Poco::Net::TimeoutNotification>  _ot;
		Poco::Observer<WSQTTHandler, Poco::Net::ShutdownNotification> _os;
		Poco::Observer<WSQTTHandler, Poco::Net::ErrorNotification>	  _err;
		
		Poco::Logger& logger;
	};
}