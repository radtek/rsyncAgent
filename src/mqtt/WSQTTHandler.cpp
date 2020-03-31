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
	loadConfiguration();
	initialize();
	setupChannel();
	addEventHandler();

	poco_information(logger, "WSQTTHandler created");
}

WSQTTHandler::~WSQTTHandler()
{
	removeEventHandler();
	poco_assert(_pWS);
	if(_pWS) delete _pWS;
	poco_information(logger, "WSQTTHandler destory");
}

void WSQTTHandler::loadConfiguration()
{
	Application& app = Application::instance();
	_clientId = app.config().getString("clientId", "INVALID_CLIENT_ID");
	_peerClientId = app.config().getString("peerclientId", "SERVICE");
	_timeoutSecond = app.config().getInt("timeout", 60);
	poco_information_f3(logger, "clientId:%s, peerclientId:%s, timeout:%d", _clientId, _peerClientId, _timeoutSecond);
}

void WSQTTHandler::initialize()
{
	logger.setLevel(Message::PRIO_DEBUG);
	_reactor.setTimeout(Timespan(_timeoutSecond, 0));
	_beat.assign(Poco::format("{ \"mag\" : \"HEARTBEAT\" , \"to\" : \"%s\" }", _peerClientId));

	poco_debug_f1(logger, "LocalNet WebSocket:%s", _socket.address().toString());

	_pWS = new WebSocket(
		HTTPClientSession(_socket.peerAddress()),
		HTTPRequest(HTTPRequest::HTTP_GET, Poco::format("/websocket/%s", _clientId), HTTPRequest::HTTP_1_1),
		HTTPResponse());
}

void WSQTTHandler::setupChannel()
{
	LoggingFactory& fact = LoggingFactory::defaultFactory();
	LoggingRegistry& reg = LoggingRegistry::defaultRegistry();
	fact.registerChannelClass("MQTTAsyncChannel", new Instantiator<MQTTAsyncChannel, Channel>);
	reg.registerChannel("MQTTAsyncChannel", fact.createChannel("MQTTAsyncChannel"));

	fact.registerChannelClass("CustomChannel", new Instantiator<CustomChannel, Channel>);
	reg.registerChannel("CustomChannel", fact.createChannel("CustomChannel"));
}

void WSQTTHandler::addEventHandler()
{
	_reactor.addEventHandler(*_pWS, _or);
	_reactor.addEventHandler(*_pWS, _ot);
	_reactor.addEventHandler(*_pWS, _ot);
	_reactor.addEventHandler(*_pWS, _err);
}

void WSQTTHandler::removeEventHandler()
{
	_reactor.removeEventHandler(*_pWS, Observer<WSQTTHandler, ReadableNotification>(*this, &WSQTTHandler::onReadable));
	_reactor.removeEventHandler(*_pWS, Observer<WSQTTHandler, TimeoutNotification>(*this, &WSQTTHandler::onTimeout));
	_reactor.removeEventHandler(*_pWS, Observer<WSQTTHandler, ShutdownNotification>(*this, &WSQTTHandler::onShutdown));
	_reactor.removeEventHandler(*_pWS, Observer<WSQTTHandler, ErrorNotification>(*this, &WSQTTHandler::onError));
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
			_str.assign(buffer, n);
			LoggingRegistry& reg = LoggingRegistry::defaultRegistry();
			MQTTAsyncChannel* pMQTTAsyncChannel = dynamic_cast<MQTTAsyncChannel*>(reg.channelForName("MQTTAsyncChannel"));
			poco_assert(pMQTTAsyncChannel != 0);
			Message Msg("MQTTAsyncClient", _str, Message::PRIO_INFORMATION);
			pMQTTAsyncChannel->log(Msg);
			poco_debug_f4(logger, "receiveBytes: %s, size:%d, flag:%d from %s", _str, n, flag, _socket.peerAddress().toString());
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