//
// rsyncAgent.cpp
//
// This sample demonstrates the ServerApplication class.
//
// Copyright (c) 2004-2006, Applied Informatics Software Engineering GmbH.
// and Contributors.
//
// SPDX-License-Identifier:	BSL-1.0
//

#include "rsyncAgent.h"
#include "AdaptiveRecevier.h"
#include "mqtt/CloudEventRecevier.h"
#include "Poco/Util/Application.h"
#include "Poco/Util/Option.h"
#include "Poco/Util/OptionSet.h"
#include "Poco/Util/HelpFormatter.h"
#include "Poco/TaskManager.h"
#include "Poco/Net/ServerSocket.h"
#include "Poco/Net/HTTPServer.h"
#include "Poco/Net/HTTPServerParams.h"
#include "Poco/URI.h"
#include "mqtt/WSQTTHandler.h"
#include "Poco/Thread.h"
#include "Poco/Net/SocketReactor.h"

#include "RequestHandlerFactory.h"
#include <sstream>

using namespace Reach;

using namespace Poco;
using namespace Poco::Net;
using namespace Poco::Util;

rsyncAgent::rsyncAgent() : _helpRequested(false)
{
}

rsyncAgent::~rsyncAgent()
{
}

void rsyncAgent::initialize(Application& self)
{
	loadConfiguration(); // load default configuration files, if present
	ServerApplication::initialize(self);
	logger().information("starting up");
}

void rsyncAgent::uninitialize()
{
	logger().information("shutting down");
	ServerApplication::uninitialize();
}

void rsyncAgent::defineOptions(OptionSet& options)
{
	ServerApplication::defineOptions(options);

	options.addOption(
		Option("help", "h", "display help information on command line arguments")
		.required(false)
		.repeatable(false)
		.callback(OptionCallback<rsyncAgent>(this, &rsyncAgent::handleHelp)));
}

void rsyncAgent::handleHelp(const std::string& name, const std::string& value)
{
	_helpRequested = true;
	displayHelp();
	stopOptionsProcessing();
}

void rsyncAgent::displayHelp()
{
	HelpFormatter helpFormatter(options());
	helpFormatter.setCommand(commandName());
	helpFormatter.setUsage("OPTIONS");
	helpFormatter.setHeader("A sample application that demonstrates some of the features of the Util::Application class.");
	helpFormatter.format(std::cout);
}

int rsyncAgent::main(const ArgVec& args)
{
	Application& app = Application::instance();

	if (!_helpRequested)
	{
		TaskManager tm;
		tm.start(new AdaptiveRecevier);
		unsigned short port = (unsigned short)config().getInt("HTTPFormServer.port", 9980);
		ServerSocket svs(port);
		HTTPServer srv(new RequestHandlerFactory, svs, new HTTPServerParams);
		srv.start();
		poco_information_f1(Application::instance().logger(), "HTTPServer Listen from %s", svs.address().toString());
		
		if ("websocket" == app.config().getString("config.etype", "mqtt"))
		{
			Poco::URI wss(app.config().getString("config.etype.url", ""));
			SocketAddress sa(wss.getHost(), wss.getPort());
			SocketReactor reactor;
			SocketConnector<WSQTTHandler> connector(sa, reactor);

			Thread thread;
			thread.start(reactor);
			waitForTerminationRequest();
			reactor.stop();
			thread.join();
		}
		else
		{
			tm.start(new CloudEventRecevier);
			waitForTerminationRequest();
		}

		srv.stop();
		tm.cancelAll();
		tm.joinAll();
	}
	return Application::EXIT_OK;
}

POCO_SERVER_MAIN(rsyncAgent)
