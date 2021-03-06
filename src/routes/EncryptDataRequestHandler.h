#pragma once

#include "Command.h"
#include "RESTfulRequestHandler.h"
#include "Poco/Util/Application.h"

namespace Reach {
	/// RS_EncryptData
	using Poco::Util::Application;
	class EncryptData : public Command
	{
	public:
		EncryptData(const std::string& data);
		virtual void run();
	private:
		std::string _kv;
		std::string _data;
		const int _maxLen = 1024;
	};

	class EncryptDataRequestHandler : public RESTfulRequestHandler
	{
	public:
		void handleRequest(HTTPServerRequest& request, HTTPServerResponse& response) {

			Application& app = Application::instance();
			poco_debug_f1(app.logger(), "Request from %s", request.clientAddress().toString());

			RESTfulRequestHandler::handleCORS(request, response);

			HTMLForm form(request, request.stream());
			std::string data(form.get("dataStr", ""));

			EncryptData command(data);
			command.execute();

			return response.sendBuffer(command().data(), command().length());
		}
	};
}
