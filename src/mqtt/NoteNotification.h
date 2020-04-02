#pragma once
#include "Poco/Notification.h"
#include <string>

namespace Reach {

	class NoteNotification : public Poco::Notification
	{
	public:
		NoteNotification(const std::string& msg) :
			_msg(msg)
		{
		}

		~NoteNotification()
		{
		}

		const std::string& note() const
		{
			return _msg;
		}

	private:
		std::string _msg;
	};
}
