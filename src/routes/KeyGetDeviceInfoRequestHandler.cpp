/// RS_KeyGetDeviceInfo
#include "KeyGetDeviceInfoRequestHandler.h"
#include "RequestHandleException.h"
#include "Reach/Data/Session.h"
#include "../Utility.h"

using namespace Reach;

KeyGetDeviceInfo::KeyGetDeviceInfo(const std::string& uid, const std::string& type)
	:_uid(uid), _type(type)
{
}

void KeyGetDeviceInfo::run()
{
	if (_type != "1")
		throw Poco::InvalidArgumentException("KeyGetDeviceInfo","type error!", RAR_UNKNOWNERR);

	Session session(Utility::getSession());

	if(_uid != session.contianer())
		throw RequestHandleException(RAR_UNIQUEIDUNCORRECT);

	_info = session.getKeyID();

	add("info", _info);
}