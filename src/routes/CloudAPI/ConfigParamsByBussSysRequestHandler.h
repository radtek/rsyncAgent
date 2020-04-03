#pragma once

#include "../Command.h"
#include "../RESTfulRequestHandler.h"
#include "Poco/Util/Application.h"
#include "CloudCommand.h"

namespace Reach {

	using Poco::Util::Application;

	///RS_ConfigParamsByBussSys
	class ConfigParamsByBussSys : public Command, public CloudCommand
	{
	public:
		ConfigParamsByBussSys(const std::string& cmd, const std::string& val, const std::string& url);
		void run();
	protected:
		virtual void mixValue();
	private:
		std::string _cmd;
		std::string _val;
	};

	class ConfigParamsByBussSysRequestHandler : public RESTfulRequestHandler
	{
	public:
		void handleRequest(HTTPServerRequest& request, HTTPServerResponse& response)
		{
			Application& app = Application::instance();
			poco_debug_f1(app.logger(), "Request from %s", request.clientAddress().toString());
			RESTfulRequestHandler::handleCORS(request, response);

			HTMLForm form(request, request.stream());
			std::string cmd = form.get("cmd", "");
			std::string value = form.get("val", "");
			std::string url = app.config().getString("rsigncloud");
			ConfigParamsByBussSys command(cmd, value, url);
			command.execute();

			return response.sendBuffer(command().data(), command().length());
		}
	};
}