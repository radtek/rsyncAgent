#include "CustomChannel.h"
#include "Poco/Util/Application.h"
#include "Poco/Net/WebSocket.h"
#include "Poco/Exception.h"
#include "Poco/Net/NetException.h"

using namespace Reach;
using namespace Poco;
using namespace Poco::Util;

CustomChannel::CustomChannel()
{

}

CustomChannel::~CustomChannel()
{

}

void CustomChannel::addSocket(const Socket& ws, int flag)
{
	Application& app = Application::instance();
	poco_debug_f3(app.logger(),"CustomChannel::addSocket socket:%s , peer:%s , _waitList :%u",
		ws.address().toString(), ws.peerAddress().toString(), _waitList.size());

	FastMutex::ScopedLock lock(_socketMutex);

	SocketInfo* pSi = find(ws);
	if (!pSi)
	{
		pSi = new SocketInfo;
		_waitList.push_back(pSi);
		pSi->ws = ws;
		pSi->flag = flag;
	}
}

void CustomChannel::removeSocket(const Socket& ws)
{
	Application& app = Application::instance();
	poco_debug_f3(app.logger(), "CustomChannel::addSocket socket:%s , peer:%s, _waitList :%u",
		ws.address().toString(), ws.peerAddress().toString(), _waitList.size());

	FastMutex::ScopedLock lock(_socketMutex);

	SocketList::iterator it = _waitList.begin();
	while (it != _waitList.end())
	{
		if ((*it)->ws == ws) {
			SocketInfo* pSi = *it;
			delete pSi;
			///Iterator following the last removed element.
			it = _waitList.erase(it);
		}
		else ++it;
	}
}

CustomChannel::SocketInfo* CustomChannel::find(const Socket& ws)
{
	SocketInfo* pSi = 0;

	for (SocketList::iterator it = _waitList.begin(); it != _waitList.end(); ++it)
	{
		if ((*it)->ws == ws)
			return *it;
	}

	return pSi;
}

void CustomChannel::log(const Message& msg)
{
	Application& app = Application::instance();
	poco_debug_f1(app.logger(), "CustomChannel::log Message :%s", msg.getText());

	FastMutex::ScopedLock lock(_socketMutex);

	for (SocketList::iterator it = _waitList.begin(); it != _waitList.end(); ++it) 
	{
		std::string text = msg.getText();
		//Socket si = (*it)->ws;
		try
		{
			WebSocket& ws = reinterpret_cast<WebSocket&>((*it)->ws);
			poco_debug_f2(app.logger(), "CustomChannel::log WebSocket Info socket:%s peer:%s",
				ws.address().toString(), ws.peerAddress().toString());
			ws.sendFrame(text.data(), text.length(), (*it)->flag);
		}
		catch (Poco::Net::NetException& exc)
		{
			poco_debug_f1(app.logger(), "CustomChannel::log :%s", exc.displayText());
		}
		/// Bug
		/// Poco::Net::ConnectionAbortedException unhandler
	}
}