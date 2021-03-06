#pragma once

#include "Command.h"
#include "RESTfulRequestHandler.h"
#include "Poco/Util/Application.h"

namespace Reach {
	/// RS_DevryptData
	using Poco::Util::Application;
	class DevryptData : public Command
	{
	public:
		DevryptData(const std::string& symKey, const std::string& data);
		virtual void run();
	private:
		std::string _kv;
		std::string _data;
		const int _maxLen = 1024;
	};

	class DevryptDataRequestHandler : public RESTfulRequestHandler
	{
	public:
		void handleRequest(HTTPServerRequest& request, HTTPServerResponse& response)
		{
			Application& app = Application::instance();
			poco_debug_f1(app.logger(), "Request from %s", request.clientAddress().toString());

			RESTfulRequestHandler::handleCORS(request, response);

			HTMLForm form(request, request.stream());
			std::string SymKey(form.get("symKey", ""));
			std::string data(form.get("encDataStr", ""));

			DevryptData command(SymKey, data);
			command.execute();

			return response.sendBuffer(command().data(), command().length());
		}
	};
}
