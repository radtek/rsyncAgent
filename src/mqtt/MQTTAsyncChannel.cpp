#include "MQTTAsyncChannel.h"
#include "Poco/AutoPtr.h"
#include "Poco/Message.h"
#include "Poco/LoggingRegistry.h"

using namespace Reach;

class MQTTMessageNotification : public Notification
{
public:
	MQTTMessageNotification(const Message& msg) :
		_msg(msg)
	{
	}

	~MQTTMessageNotification()
	{
	}

	const Message& message() const
	{
		return _msg;
	}

private:
	Message _msg;
};

MQTTAsyncChannel::MQTTAsyncChannel(Channel* pChannel, Thread::Priority proi):
	_pChannel(pChannel),
	_thread("MQTTAsyncChannel")
{
	if (_pChannel) _pChannel->duplicate();
	_thread.setPriority(proi);
}

MQTTAsyncChannel::~MQTTAsyncChannel()
{
	try
	{
		close();
		if (_pChannel) _pChannel->release();
	}
	catch (...)
	{
		poco_unexpected();
	}
}

void MQTTAsyncChannel::run()
{
	Notification::Ptr nf = _queue.waitDequeueNotification();
	while (nf)
	{
		MQTTMessageNotification* pNf = dynamic_cast<MQTTMessageNotification*>(nf.get());
		{
			FastMutex::ScopedLock lock(_channelMutex);

			if (pNf && _pChannel) _pChannel->log(pNf->message());
		}
		nf = _queue.waitDequeueNotification();
	}
}

void MQTTAsyncChannel::setPriority(const std::string& value)
{
	Thread::Priority prio = Thread::PRIO_NORMAL;

	if (value == "lowest")
		prio = Thread::PRIO_LOWEST;
	else if (value == "low")
		prio = Thread::PRIO_LOW;
	else if (value == "normal")
		prio = Thread::PRIO_NORMAL;
	else if (value == "high")
		prio = Thread::PRIO_HIGH;
	else if (value == "highest")
		prio = Thread::PRIO_HIGHEST;
	else
		throw InvalidArgumentException("thread priority", value);

	_thread.setPriority(prio);
}

void MQTTAsyncChannel::setChannel(Channel* pChannel)
{
	FastMutex::ScopedLock lock(_channelMutex);

	if (_pChannel) _pChannel->release();
	_pChannel = pChannel;
	if (_pChannel) _pChannel->duplicate();
}

Channel* MQTTAsyncChannel::getChannel() const
{
	return _pChannel;
}

void MQTTAsyncChannel::open()
{
	FastMutex::ScopedLock lock(_threadMutex);

	if (!_thread.isRunning())
		_thread.start(*this);
}

void MQTTAsyncChannel::close()
{
	if (_thread.isRunning())
	{
		while (!_queue.empty()) Thread::sleep(100);

		do
		{
			_queue.wakeUpAll();
		} 
		while (!_thread.tryJoin(100));
	}
}

void MQTTAsyncChannel::log(const Message& msg)
{
	open();

	_queue.enqueueNotification(new MQTTMessageNotification(msg));
}

void MQTTAsyncChannel::setPropery(const std::string& name, const std::string& value)
{
	if (name == "channel")
		setChannel(LoggingRegistry::defaultRegistry().channelForName(value));
	else if (name == "priority")
		setPriority(value);
	else
		Channel::setProperty(name, value);
}