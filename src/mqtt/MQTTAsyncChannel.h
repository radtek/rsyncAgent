#pragma once
#include "Poco/Channel.h"
#include "Poco/Runnable.h"
#include "Poco/Thread.h"
#include "Poco/NotificationQueue.h"

namespace Reach {
	using namespace Poco;

	class MQTTAsyncChannel : public Channel, public Runnable
	{
	public:
		MQTTAsyncChannel(Channel* pChannel = 0, Thread::Priority proi = Thread::PRIO_NORMAL);
		void setChannel(Channel* pChannel);
		Channel* getChannel() const;
		void open();
		void close();
		void log(const Message& msg);
		void setPropery(const std::string& name, const std::string& value);
	protected:
		~MQTTAsyncChannel();
		void run();
		void setPriority(const std::string& value);
	private:
		Channel*	_pChannel;
		Thread		_thread;
		FastMutex	_threadMutex;
		FastMutex	_channelMutex;
		NotificationQueue	_queue;
	};
}