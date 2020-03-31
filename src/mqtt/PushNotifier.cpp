#include "PushNotifier.h"
#include "Poco/Net/HTTPRequest.h"
#include "Poco/Net/HTTPResponse.h"
#include "Poco/Net/HttpClientSession.h"
#include "Poco/Net/WebSocket.h"
#include "Poco/JSON/Object.h"
#include "Poco/Logger.h"
#include "Poco/EventLogChannel.h"
#include "Poco/UUIDGenerator.h"
#include "Poco/Net/NetException.h"
#include <vector>

using namespace Reach;
using namespace Poco;
using namespace Poco::Net;
using namespace Poco::JSON;

PushNotifier::PushNotifier()
	:Task("PushNotifier"),
	sa("192.168.0.75", 7099),
	stop(false), conne_stat(false),
	clientId("fascist001"/*UUIDGenerator::defaultGenerator().create().toString()*/),
	cs(sa)
{
	AutoPtr<EventLogChannel> pChannel = new EventLogChannel;
	Logger::create("PushNotifier", pChannel);
	__asm int 3;
	initWebSocket();

	Object o;
	o.set("mag", "HeartBeat");
	o.set("to", clientId);
	//std::ostringstream json;
	o.stringify(json);
}

PushNotifier::~PushNotifier()
{
	delete ws;
	Logger::destroy("PushNotifier");
}

void PushNotifier::runTask()
{
	//authCode = 8125ec6846b54c71885e68c452f06c44
	do
	{
		try
		{
			heartBeat();
			messageArrived();
		}
		catch (Poco::Net::NetException& e)
		{
			if (typeid(e) == typeid(ConnectionResetException) ||
				typeid(e) == typeid(ConnectionAbortedException) ||
				typeid(e) == typeid(ConnectionRefusedException))
			{
				ws->connectNB(sa);
				poco_warning_f2(Logger::get("PushNotifier"), "warning: %s : %d", e.displayText(), e.code());
			}
		}
		catch (Poco::IOException& e)
		{
			poco_error_f2(Logger::get("PushNotifier"), "error: %s : %d", e.displayText(), e.code());
			stop = true;
		}

	} while (!stop);
}

void PushNotifier::cancel()
{
	stop = true;
}

void PushNotifier::initWebSocket()
{
	HTTPRequest requset(HTTPRequest::HTTP_GET, Poco::format("/websocket/%s", clientId), HTTPRequest::HTTP_1_1);
	HTTPResponse response;

	try
	{
		ws = new WebSocket(cs, requset, response);
		ws->setReceiveTimeout(Timespan(5, 0));
		//ws->setSendBufferSize(msgSize);
		//ws->setReceiveBufferSize(msgSize);
		//ws->setKeepAlive(true);
		poco_information(Logger::get("PushNotifier"), "PushNotifier connect!");
	}
	catch (Poco::Net::WebSocketException& e)
	{
		poco_error_f2(Logger::get("PushNotifier"), "error: %s : %d", e.displayText(), e.code());
	}
}

void PushNotifier::messageArrived()
{
	poco_assert(ws);
	if (!ws) return;

	try
	{
		int flags, n = 0;
		std::vector<char> buffer(msgSize, 0);
		n = ws->receiveFrame(&buffer[0], buffer.size(), flags);
		buffer.resize(n);
		std::string message;
		message.assign(buffer.begin(), buffer.end());
		poco_information_f2(Logger::get("PushNotifier"),"receive %s : %s", ws->peerAddress().toString(), message);
	}
	catch (Poco::TimeoutException&)
	{
	}
	catch (Poco::Net::WebSocketException& e)
	{
		poco_error_f2(Logger::get("PushNotifier"), "error: %s : %d", e.displayText(), e.code());
	}
}

void PushNotifier::heartBeat()
{
	poco_assert(ws);
	if(ws) ws->sendFrame(json.str().data(), json.str().size());
	poco_information_f1(Logger::get("PushNotifier"), "HeartBeat : %s", json.str());
}
