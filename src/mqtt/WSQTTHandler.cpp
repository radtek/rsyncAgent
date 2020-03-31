#include "WSQTTHandler.h"
#include "Poco/Logger.h"
#include "Poco/ConsoleChannel.h"
#include "Poco/Util/Application.h"
#include "Poco/Net/HTTPClientSession.h"
#include "Poco/Net/HTTPRequest.h"
#include "Poco/Net/HTTPResponse.h"
#include "Poco/JSON/Object.h"
#include "Poco/LoggingFactory.h"
#include "Poco/LoggingRegistry.h"
#include "MQTTAsyncChannel.h"
#include "../CustomChannel.h"
using Poco::JSON::Object;
using namespace Reach;
using namespace Poco;
using namespace Poco::Net;
using namespace Poco::Util;

std::string WSQTTHandler::_data;
bool WSQTTHandler::_readableError = false;
bool WSQTTHandler::_timeoutError = false;
bool WSQTTHandler::_shutdownError = false;
bool WSQTTHandler::_once = false;
bool WSQTTHandler::_closeOnTimeout = false;

WSQTTHandler::WSQTTHandler(Poco::Net::StreamSocket& socket, Poco::Net::SocketReactor& reactor)
	:_socket(socket),
	_reactor(reactor),
	_or(*this, &WSQTTHandler::onReadable),
	_ot(*this, &WSQTTHandler::onTimeout),
	_os(*this, &WSQTTHandler::onShutdown),
	_err(*this, &WSQTTHandler::onError),
	logger(Application::instance().logger()),
	_pWS(0)
{
	Application& app = Application::instance();
	std::string classid = app.config().getString("clientId", "fakeid");
	std::string peerclientId = app.config().getString("peerclientId", "fakeid");
	int second = app.config().getInt("timeout", 60);

	LoggingFactory& fact = LoggingFactory::defaultFactory();
	LoggingRegistry& reg = LoggingRegistry::defaultRegistry();
	fact.registerChannelClass("MQTTAsyncChannel", new Instantiator<MQTTAsyncChannel, Channel>);
	reg.registerChannel("MQTTAsyncChannel", fact.createChannel("MQTTAsyncChannel"));

	fact.registerChannelClass("CustomChannel", new Instantiator<CustomChannel, Channel>);
	reg.registerChannel("CustomChannel", fact.createChannel("CustomChannel"));

	_reactor.setTimeout(Timespan(second, 0));
	HTTPClientSession cs("192.168.0.75", 7099);
	HTTPRequest requset(HTTPRequest::HTTP_GET, Poco::format("/websocket/%s", classid), HTTPRequest::HTTP_1_1);
	HTTPResponse response;
	_pWS = new WebSocket(cs, requset, response);

	checkReadableObserverCount(0);
	_reactor.addEventHandler(*_pWS, _or);
	checkReadableObserverCount(1);

	checkTimeoutObserverCount(0);
	_reactor.addEventHandler(*_pWS, _ot);
	checkTimeoutObserverCount(1);

	checkShutdownObserverCount(0);
	_reactor.addEventHandler(*_pWS, _ot);
	checkShutdownObserverCount(1);

	_reactor.addEventHandler(*_pWS, _err);

	poco_information(logger, "WSQTTHandler created");
	logger.setLevel(Message::PRIO_DEBUG);

	Object o;
	o.set("mag", "HeartBeat");
	o.set("to", peerclientId);
	std::ostringstream json;
	o.stringify(json);

	_beat = json.str();
}

WSQTTHandler::~WSQTTHandler()
{
	checkReadableObserverCount(1);
	_reactor.removeEventHandler(*_pWS, Observer<WSQTTHandler, ReadableNotification>(*this, &WSQTTHandler::onReadable));
	checkReadableObserverCount(0);

	checkTimeoutObserverCount(1);
	_reactor.removeEventHandler(*_pWS, Observer<WSQTTHandler, TimeoutNotification>(*this, &WSQTTHandler::onTimeout));
	checkTimeoutObserverCount(0);

	checkShutdownObserverCount(1);
	_reactor.removeEventHandler(*_pWS, Observer<WSQTTHandler, ShutdownNotification>(*this, &WSQTTHandler::onShutdown));
	checkShutdownObserverCount(0);

	_reactor.removeEventHandler(*_pWS, Observer<WSQTTHandler, ErrorNotification>(*this, &WSQTTHandler::onError));
	delete _pWS;

	poco_information(logger, "WSQTTHandler destory");
}

void WSQTTHandler::onReadable(Poco::Net::ReadableNotification* pNf)
{
	pNf->release();
	poco_assert(_pWS);
	try
	{
		int flag = 0;
		char buffer[4096] = { 0 };
		int n = _pWS->receiveFrame(buffer, sizeof(buffer), flag);
		if (n > 0) {
			_str.write(buffer, n);
			LoggingRegistry& reg = LoggingRegistry::defaultRegistry();
			MQTTAsyncChannel* pMQTTAsyncChannel = dynamic_cast<MQTTAsyncChannel*>(reg.channelForName("MQTTAsyncChannel"));
			poco_assert(pMQTTAsyncChannel != 0);
			Message Msg("MQTTAsyncClient", _str.str(), Message::PRIO_INFORMATION);
			pMQTTAsyncChannel->log(Msg);
			poco_debug_f4(logger, "receiveBytes: %s, size:%d, flag:%d from %s", _str.str(), n, flag, _socket.peerAddress().toString());
		}
		else 
		{
			poco_debug(logger, "receive empty Bytes!");
			delete this;
		}
	}
	catch (Poco::Exception& exc)
	{
		logger.log(exc);
	}
}

void WSQTTHandler::checkSocketAlive()
{
	try
	{
		int n = _pWS->sendFrame(_beat.data(), (int)_beat.length());
		poco_debug_f2(logger, "send heart beat message:%s, size:%d", _beat, n);
	}
	catch (Poco::Exception& exc)
	{
		logger.log(exc);
		delete this;
	}
}

void WSQTTHandler::onTimeout(Poco::Net::TimeoutNotification* pNf)
{
	poco_debug(logger, "TimeoutNotification");

	pNf->release();
	if (_closeOnTimeout)
	{
		_reactor.stop();
		delete this;
	}

	checkSocketAlive();
}

void WSQTTHandler::onShutdown(Poco::Net::ShutdownNotification* pNf)
{
	poco_debug(logger, "ShutdownNotification");

	pNf->release();
	delete this;
}

void WSQTTHandler::onError(Poco::Net::ErrorNotification* pNf)
{
	poco_debug(logger, "ErrorNotification");
	pNf->release();
}

void WSQTTHandler::checkReadableObserverCount(std::size_t oro)
{
	if (((oro == 0) && _reactor.hasEventHandler(_socket, _or)) ||
		((oro > 0) && !_reactor.hasEventHandler(_socket, _or)))
	{
		_readableError = true;
	}
}

void WSQTTHandler::checkTimeoutObserverCount(std::size_t ot)
{
	if (((ot == 0) && _reactor.hasEventHandler(_socket, _ot)) ||
		((ot > 0) && _reactor.hasEventHandler(_socket, _ot)))
	{
		_timeoutError = true;
	}
}

void WSQTTHandler::checkShutdownObserverCount(std::size_t os)
{
	if (((os == 0) && _reactor.hasEventHandler(_socket, _os)) ||
		((os > 0) && _reactor.hasEventHandler(_socket, _os)))
	{
		_shutdownError = true;
	}
}