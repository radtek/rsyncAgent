#include "WSQTTServiceHandler.h"
#include "Poco/Net/WebSocket.h"
#include "Poco/Util/Application.h"
#include "Poco/Thread.h"
#include "Poco/Timespan.h"
#include "mqtt/NoteNotification.h"

using namespace Reach;
using namespace Poco;
using namespace Poco::Net;
using namespace Poco::Util;

WSQTTServiceHandler::WSQTTServiceHandler(StreamSocket& socket, SocketReactor& reactor) :
	_socket(socket),
	_reactor(reactor)
{
	Application& app = Application::instance();
	poco_debug_f1(app.logger(), "WebSocket connection established. reactor thread %lu", Thread::currentTid());
	poco_debug_f1(app.logger(), "KeepAliveStatus:%b", _socket.getKeepAlive());
	poco_debug_f1(app.logger(), "peerAddress:%s", _socket.peerAddress().toString());

	_reactor.addEventHandler(_socket, Observer<WSQTTServiceHandler, ReadableNotification>(*this, &WSQTTServiceHandler::onReadable));
	_reactor.addEventHandler(_socket, Observer<WSQTTServiceHandler, WritableNotification>(*this, &WSQTTServiceHandler::onWritable));
	_reactor.addEventHandler(_socket, Observer<WSQTTServiceHandler, ErrorNotification>(*this, &WSQTTServiceHandler::onError));
	_reactor.addEventHandler(_socket, Observer<WSQTTServiceHandler, ShutdownNotification>(*this, &WSQTTServiceHandler::onShutdown));
}

WSQTTServiceHandler::~WSQTTServiceHandler()
{
	Application& app = Application::instance();

	_reactor.removeEventHandler(_socket, Observer<WSQTTServiceHandler, ReadableNotification>(*this, &WSQTTServiceHandler::onReadable));
	_reactor.removeEventHandler(_socket, Observer<WSQTTServiceHandler, WritableNotification>(*this, &WSQTTServiceHandler::onWritable));
	_reactor.removeEventHandler(_socket, Observer<WSQTTServiceHandler, ErrorNotification>(*this, &WSQTTServiceHandler::onError));
	_reactor.removeEventHandler(_socket, Observer<WSQTTServiceHandler, ShutdownNotification>(*this, &WSQTTServiceHandler::onShutdown));
	poco_debug_f1(app.logger(), "peerAddress:%s", _socket.peerAddress().toString());
	poco_debug_f1(app.logger(), "WebSocket connection closed. reactor thread %lu", Thread::currentTid());
}

void WSQTTServiceHandler::onReadable(ReadableNotification* pNf)
{
	pNf->release();

	Application& app = Application::instance();

	int flags, n = 0;
	char buffer[1024] = { 0 };
	std::string mag;

	try
	{
		n = _socket.receiveFrame(buffer, sizeof(buffer), flags);
		mag.assign(buffer, n);
		poco_debug_f3(app.logger(), "Frame received (mag=%s, length=%d, flags=0x%x).", mag, n, unsigned(flags));
		if (n > 0) {
			n = _socket.sendFrame(mag.data(), mag.size(), flags);
			poco_debug_f3(app.logger(), "Frame send (mag=%s, length=%d, flags=0x%x).", mag, n, unsigned(flags));
		}
		else
		{
			poco_debug(app.logger(), "receive empty Bytes!");
			_reactor.stop();
		}
	}
	catch (Poco::Exception& exc)
	{
		app.logger().log(exc);
		_reactor.stop();
	}
}

#include "Poco/NotificationQueue.h"

void WSQTTServiceHandler::onWritable(WritableNotification* pNf)
{
	pNf->release();

	Application& app = Application::instance();
	poco_trace_f1(app.logger(), "%s : onWritable", _socket.peerAddress().toString());
	AutoPtr<Notification> pf = NotificationQueue::defaultQueue().waitDequeueNotification(250);
	if (pf)
	{
		NoteNotification* pNote = dynamic_cast<NoteNotification*>(pf.get());
		{
			FastMutex::ScopedLock lock(_Mutex);
			std::string msg = pNote->note();
			int n = _socket.sendFrame(msg.data(), msg.size());
			poco_debug_f3(app.logger(),"send %s : %d, %s", n, _socket.peerAddress().toString(), msg);
		}
	}
}

void WSQTTServiceHandler::onError(ErrorNotification * pNf)
{
	pNf->release();
	Application& app = Application::instance();
	poco_debug(app.logger(), "onError");
	delete this;
}

void WSQTTServiceHandler::onShutdown(ShutdownNotification* pNf)
{
	pNf->release();
	Application& app = Application::instance();
	poco_debug(app.logger(),"onShutdown");
	delete this;
}

