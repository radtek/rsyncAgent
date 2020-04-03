#pragma once

#include "Poco/Net/HTTPRequestHandler.h"
#include "Poco/Net/HTTPServerRequest.h"
#include "Poco/Net/HTTPServerResponse.h"
#include "Poco/Net/WebSocket.h"
#include "Poco/Net/NetException.h"
#include "Poco/Util/Application.h"
#include "Poco/LoggingRegistry.h"
#include "Poco/String.h"
#include "CustomChannel.h"
#include "Poco/Net/SocketAcceptor.h"
#include "Poco/Net/SocketReactor.h"
#include "Poco/Net/WebSocket.h"
#include "WSQTTServiceHandler.h"

namespace Reach {
	
	using Poco::Net::HTTPRequestHandler;
	using Poco::Net::WebSocket;
	using Poco::Net::WebSocketException;
	using Poco::Net::HTTPResponse;
	using Poco::Net::HTTPServerRequest;
	using Poco::Net::HTTPServerResponse;
	using Poco::Util::Application;
	using Poco::LoggingRegistry;
	using Poco::Net::SocketAcceptor;
	using Poco::Net::SocketReactor;

	class WebSocketRequestHandler : public HTTPRequestHandler
		/// Handle a WebSocket connection.
	{
	public:
		void handleRequest(HTTPServerRequest& request, HTTPServerResponse& response)
		{	
			Application& app = Application::instance();
			try
			{
				WebSocket ws(request, response);
				SocketReactor reactor;
				WSQTTServiceHandler* wsh = new WSQTTServiceHandler(ws, reactor);
				reactor.run();
			}
			catch (WebSocketException& exc)
			{
				app.logger().log(exc);
				poco_debug_f1(app.logger(), "Rasied WebSocketException! ,details : %s", exc.displayText());
				switch (exc.code())
				{
				case WebSocket::WS_ERR_HANDSHAKE_UNSUPPORTED_VERSION:
					response.set("Sec-WebSocket-Version", WebSocket::WEBSOCKET_VERSION);
					// fallthrough
				case WebSocket::WS_ERR_NO_HANDSHAKE:
				case WebSocket::WS_ERR_HANDSHAKE_NO_VERSION:
				case WebSocket::WS_ERR_HANDSHAKE_NO_KEY:
					response.setStatusAndReason(HTTPResponse::HTTP_BAD_REQUEST);
					response.setContentLength(0);
					response.send();
					break;
				}
			}
			catch (Poco::Exception& exc)
			{
				app.logger().log(exc);
				poco_debug_f1(app.logger(),"Rasied Exception! ,details : %s", exc.displayText());
			}
		}
	};

}