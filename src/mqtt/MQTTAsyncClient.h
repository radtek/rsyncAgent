#pragma once
#include "MQTTAsync.h"
#include <string>

namespace Reach {

	extern "C"
	{
		typedef MQTTAsync_createOptions createOptions;
		typedef MQTTAsync_connectOptions connectOptions;
		typedef MQTTAsync_disconnectOptions disconectOptions;
		typedef MQTTAsync_SSLOptions sslOptions;
		typedef MQTTAsync_successData successData;
		typedef MQTTAsync_failureData failureData;
		typedef MQTTAsync_token token;
		typedef MQTTAsync_message message;
	}

//#define MQTTAsyncChannelInst \ 
//	MQTTAsyncChannel* pMQTTAsyncChannel;
//	LoggingRegistry& reg = LoggingRegistry::defaultRegistry();
//	pMQTTAsyncChannel = dynamic_cast<MQTTAsyncChannel*>(reg.channelForName("MQTTAsyncChannel"));

#define MQTTAsyncChannelinitializer \
	AutoPtr<MQTTAsyncChannel> pMQTTAsyncChannel; \
	do{	LoggingFactory& fact = LoggingFactory::defaultFactory();\
	pMQTTAsyncChannel = dynamic_cast<MQTTAsyncChannel*>(fact.createChannel("MQTTAsyncChannel"));}while(0)

#define CustomChannelinitializer \
	AutoPtr<CustomChannel> pCustomChannel;	\
	do{ LoggingFactory& fact = LoggingFactory::defaultFactory();\
	pCustomChannel = dynamic_cast<CustomChannel*>(fact.createChannel("CustomChannel"));}while(0)

	class MQTTAsyncClient
	{
	public:
		MQTTAsyncClient(bool useSSL);

		~MQTTAsyncClient();

		void connect(const std::string & accessKey, const std::string & instanceId, const std::string & secretKey);
		void connect(const std::string & user, const std::string & password);
		void disconnect();
		bool isConnected();

	protected:
		static void deliveryComplete(void * context, token token);
		static int messageArrived(void * context, char * topicName, int topicLen, message * msg);
		static void connectionLost(void * context, char * cause);
		static void onSuccess(void * context, successData * response);
		static void onFailure(void * context, failureData * response);

	private:
		void connect(const char * user, const char * password, bool useSSL, int keepAliveInterval);
		std::string generatorPassword(const std::string & secretKey);
		std::string generatorUsername(const std::string & accessKey, const std::string & instanceId);

		void initialize();
		void createOpts();
		void connectOpts();
		//void setConfigParameters();
	private:
		std::string host;
		std::size_t port;
		std::string groupId;
		std::string deviceId;
		std::string serverURI;
		std::string clientIdUrl;
		MQTTAsync client;
		//AutoPtr<MQTTAsyncChannel> _pMQTTAsyncChannel;
		createOptions create_opts;
		connectOptions conn_opts;
		disconectOptions disc_opts;
		sslOptions ssl_opts;
		bool _useSSL;
		static bool connected;
	};
}
