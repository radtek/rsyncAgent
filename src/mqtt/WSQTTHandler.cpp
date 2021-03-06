#include "WSQTTHandler.h"
#include "Poco/Util/Application.h"
#include "Poco/Net/HTTPClientSession.h"
#include "Poco/Net/HTTPRequest.h"
#include "Poco/Net/HTTPResponse.h"
#include "Poco/NotificationQueue.h"
#include "NoteNotification.h"
#include "../Utility.h"
#include "../TokenManager.h"

using namespace Reach;
using namespace Poco;
using namespace Poco::Net;
using namespace Poco::Util;
using namespace Poco::JSON;

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
	//setupChannel();
	addEventHandler();

	poco_debug(logger, "WSQTTHandler created");
}

WSQTTHandler::~WSQTTHandler()
{
	removeEventHandler();
	poco_assert(_pWS);
	if(_pWS) delete _pWS;
	poco_debug(logger, "WSQTTHandler destory");
}
#include "Poco/URI.h"
void WSQTTHandler::loadConfiguration()
{
	Application& app = Application::instance();
	_clientId = app.config().getString("clientId", "INVALID_CLIENT_ID");
	_peerClientId = app.config().getString("peerclientId", "SERVICE");
	_timeoutSecond = app.config().getInt("timeout", 60);
	_servicePath = URI(app.config().getString("config.etype.url","")).getPath();
	poco_debug_f3(logger, "clientId:%s, peerclientId:%s, timeout:%d", _clientId, _peerClientId, _timeoutSecond);
}

void WSQTTHandler::initialize()
{
	_reactor.setTimeout(Timespan(_timeoutSecond, 0));
	_beat.assign(Poco::format("{ \"mag\" : \"HEARTBEAT\" , \"to\" : \"%s\" }", _peerClientId));

	poco_debug_f1(logger, "LocalNet WebSocket:%s", _socket.address().toString());

	_pWS = new WebSocket(
		HTTPClientSession(_socket.peerAddress()),
		HTTPRequest(HTTPRequest::HTTP_GET, Poco::format("%s%s", _servicePath, _clientId), HTTPRequest::HTTP_1_1),
		HTTPResponse());
}

void WSQTTHandler::addEventHandler()
{
	_reactor.addEventHandler(*_pWS, _or);
	_reactor.addEventHandler(*_pWS, _ot);
	_reactor.addEventHandler(*_pWS, _os);
	_reactor.addEventHandler(*_pWS, _err);
}

void WSQTTHandler::removeEventHandler()
{
	_reactor.removeEventHandler(*_pWS, _or);
	_reactor.removeEventHandler(*_pWS, _ot);
	_reactor.removeEventHandler(*_pWS, _os);
	_reactor.removeEventHandler(*_pWS, _err);
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
			JSON_PARSE(_str);
			poco_assert((ds["code"] == "0000") && ds.contains("data"));
			if (ds["code"] == "0000" && ds.contains("data")) {
				TokenManager::default().add(ds["data"]["transid"], ds["data"]);
			}
			NotificationQueue::defaultQueue().enqueueNotification(new NoteNotification(_str));
			poco_debug_f4(logger, "receiveBytes: %s, size:%d, flag:%d from %s", _str, n, flag, _socket.peerAddress().toString());
		}
	}
	catch (Poco::Exception& exc)
	{
		logger.log(exc);
		delete this;
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