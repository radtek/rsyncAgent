#pragma once

#include "Poco/Util/Application.h"
#include "Poco/Net/HTTPClientSession.h"
#include "Poco/URI.h"
#include "Poco/JSON/Parser.h"
#include "Poco/DynamicStruct.h"
#include "Poco/JSON/Object.h"
#include "Poco/FileStream.h"
#include "../Command.h"
#include "../RESTfulRequestHandler.h"
#include "CloudCommand.h"


namespace Reach {

	using Poco::Util::Application;
	using Poco::URI;
	using Poco::JSON::Parser;
	using Poco::JSON::Object;
	using Poco::DynamicStruct;
	using Poco::FileInputStream;

	///RS_CloudSignByP7
	class CloudSignByP7 : public Command, public CloudCommand
	{
	public:
		CloudSignByP7(const std::string& msg, const std::string& keySn,
					  const std::string& transid, const std::string& token,
					  const std::string& url):
			CloudCommand(url),
			_dealData(msg),_keySn(keySn),
			_token(token),_transid(transid),
			_action("T")
		{
		}

		void run()
		{
			mixValue();
			sendRequest();

			if (!success())
				throw CloudCommandException(extract("head", "message"),
					std::stoi(extract("head", "code"), 0, 16));

		}
	protected:
		virtual void mixValue()
		{
			if (_dealData.size() > max_len)
				throw CloudCommandException(RAR_ERRORSTACKOVERFLOW);

			Application& app = Application::instance();
			FileInputStream in(Utility::getCommand("CloudSignByP7"));
			DynamicStruct ds = *parse(in).extract<Object::Ptr>();
			ds["bodyJson"]["token"] = _token;
			ds["bodyJson"]["dealData"] = _dealData;
			ds["bodyJson"]["keySn"] = _keySn;
			ds["bodyJson"]["transid"] = _transid;

			ds["bodyJson"]["authCode"] = app.config().getString("authCode", "");
			ds["bodyJson"]["clientId"] = app.config().getString("clientId", "456789");
			ds["body"] = ds["bodyJson"].toString();
			ds.erase("bodyJson");

			prepare(ds.toString());
			poco_information_f1(app.logger(), "CloudSignByP7 mixValue:\n%s", ds.toString());
		}

	private:
		std::string _action;
		std::string _dealData;
		std::string _keySn;
		std::string _token;
		std::string _transid;
		const size_t max_len = 3000;
	};

	class CloudSignByP7RequestHandler : public RESTfulRequestHandler
	{
	public:
		void handleRequest(HTTPServerRequest& request, HTTPServerResponse& response)
		{
			Application& app = Application::instance();
			poco_information_f1(app.logger(), "Request from %s", request.clientAddress().toString());
			RESTfulRequestHandler::handleCORS(request, response);

			HTMLForm form(request, request.stream());
			std::string msg = form.get("msg", "");
			std::string keySn = form.get("keySn", "");
			std::string transid = form.get("transid", "");
			std::string token = form.get("token", "");
			std::string url = app.config().getString("rsigncloud");
			CloudSignByP7 command(msg, keySn, transid, token, url);
			command.execute();

			return response.sendBuffer(command().data(), command().length());
		}
	};
}