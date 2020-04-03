#include "ConfigParamsByBussSysRequestHandler.h"
#include "Poco/JSON/Parser.h"
#include "Poco/JSON/Object.h"
#include "Poco/FileStream.h"
#include "../src/Utility.h"

using namespace Reach;
using namespace Poco;
using namespace Poco::JSON;

ConfigParamsByBussSys::ConfigParamsByBussSys(const std::string& cmd, const std::string& val, const std::string& url)
:CloudCommand(url),_cmd(cmd), _val(val)
{
}

void ConfigParamsByBussSys::run()
{
	if (_cmd != "bussName")
		throw RequestHandleException(RAR_9016);


	mixValue();
	sendRequest();

	if (!success())
		throw CloudCommandException(extract("head", "message"),
			std::stoi(extract("head", "code"), 0, 16));

	Application& app = Application::instance();
	app.config().setString("authCode", extract("body","authCode"));
}

void ConfigParamsByBussSys::mixValue()
{
	Application& app = Application::instance();
	FileInputStream in(Utility::getCommand("CloudGetAuthCode"));
	DynamicStruct ds = *parse(in).extract<Object::Ptr>();

	ds["bodyJson"]["bussName"] = _val;
	ds["body"] = ds["bodyJson"].toString();
	ds.erase("bodyJson");

	prepare(ds.toString());
	poco_debug_f1(app.logger(), "CloudLogoutAuth mixValue:\n%s", ds.toString());
}