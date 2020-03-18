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

namespace Reach {
	
	using Poco::Net::HTTPRequestHandler;
	using Poco::Net::WebSocket;
	using Poco::Net::WebSocketException;
	using Poco::Net::HTTPResponse;
	using Poco::Net::HTTPServerRequest;
	using Poco::Net::HTTPServerResponse;
	using Poco::Util::Application;
	using Poco::LoggingRegistry;

	class WebSocketRequestHandler : public HTTPRequestHandler
		/// Handle a WebSocket connection.
	{
	public:
		WebSocketRequestHandler()
		{
			LoggingRegistry& reg = LoggingRegistry::defaultRegistry();
			CustomChannel* pCustomChannel = dynamic_cast<CustomChannel*>(reg.channelForName("CustomChannel"));
			assert(pCustomChannel != 0);
		}
		void handleRequest(HTTPServerRequest& request, HTTPServerResponse& response)
		{
			Application& app = Application::instance();
			try
			{
				WebSocket ws(request, response);
				ws.setKeepAlive(true);
				app.logger().information("KeepAliveStatus:%b", ws.getKeepAlive());
				app.logger().information("WebSocket connection established.");
				char buffer[1024] = {0};
				int flags;
				int n;
				do
				{
					n = ws.receiveFrame(buffer, sizeof(buffer), flags);
					if (Poco::icompare(std::string(buffer,n), "registerMqttService") == 0)
					{
						poco_information(app.logger(), "registerMqttService addSocket");
						LoggingRegistry& reg = LoggingRegistry::defaultRegistry();
						CustomChannel* pCustomChannel = dynamic_cast<CustomChannel*>(reg.channelForName("CustomChannel"));
						assert(pCustomChannel != 0);
						pCustomChannel->addSocket(ws);
					}

					if (Poco::icompare(std::string(buffer, n), "unregisterMqttService") == 0)
					{
						poco_information(app.logger(), "unregisterMqttService addSocket");
						LoggingRegistry& reg = LoggingRegistry::defaultRegistry();
						CustomChannel* pCustomChannel = dynamic_cast<CustomChannel*>(reg.channelForName("CustomChannel"));
						assert(pCustomChannel != 0);
						pCustomChannel->removeSocket(ws);
					}
	
					app.logger().information(Poco::format("Frame received (length=%d, flags=0x%x).", n, unsigned(flags)));
					ws.sendFrame(buffer, n, flags);
				} while (n > 0 && (flags & WebSocket::FRAME_OP_BITMASK) != WebSocket::FRAME_OP_CLOSE);
				app.logger().information("WebSocket connection closed.");
			}
			catch (WebSocketException& exc)
			{
				app.logger().log(exc);
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
		}
	};

}