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

		static bool readableError()
		{
			return _readableError;
		}

		static bool timeoutError()
		{
			return _timeoutError;
		}

		static bool shutdownError()
		{
			return _shutdownError;
		}

	private:
		void checkReadableObserverCount(std::size_t oro);
		void checkTimeoutObserverCount(std::size_t ot);
		void checkShutdownObserverCount(std::size_t os);
		void checkSocketAlive();
	private:
		std::stringstream	_str;
		std::string			_beat;
		Poco::Net::StreamSocket		_socket;
		Poco::Net::SocketReactor&	_reactor;
		Poco::Net::WebSocket*		_pWS;

		Poco::Observer<WSQTTHandler, Poco::Net::ReadableNotification> _or;
		Poco::Observer<WSQTTHandler, Poco::Net::TimeoutNotification>  _ot;
		Poco::Observer<WSQTTHandler, Poco::Net::ShutdownNotification> _os;
		Poco::Observer<WSQTTHandler, Poco::Net::ErrorNotification>	  _err;

		static std::string _data;
		static bool _readableError;
		static bool _timeoutError;
		static bool _shutdownError;
		static bool _once;
		static bool _closeOnTimeout;
		Poco::Logger& logger;
	};
}