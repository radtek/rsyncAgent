#pragma once

#include "Command.h"
#include "RESTfulRequestHandler.h"
#include "Poco/Util/Application.h"

namespace Reach {

	using Poco::Util::Application;
	/// RS_KeyGetDeviceInfo
	class KeyGetDeviceInfo : public Command
	{
	public:
		KeyGetDeviceInfo(const std::string& uid, const std::string& type);
		virtual void run();

	private:
		std::string _uid;
		std::string _type;
		std::string _info;
	};

	class KeyGetDeviceInfoRequestHandler : public RESTfulRequestHandler
	{
	public:
		void handleRequest(HTTPServerRequest& request, HTTPServerResponse& response)
		{
			poco_debug_f1(Application::instance().logger(), "Request from %s", request.clientAddress().toString());

			RESTfulRequestHandler::handleCORS(request, response);

			HTMLForm form(request, request.stream());
			std::string uid(form.get("containerId", ""));
			std::string type(form.get("type", ""));

			KeyGetDeviceInfo command(uid, type);
			command.execute();

			return response.sendBuffer(command().data(), command().length());
		}
	};
}