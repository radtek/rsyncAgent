#pragma once
#include "Command.h"
#include "RESTfulRequestHandler.h"
#include "Poco/Util/Application.h"
#include "Poco/FileStream.h"
#include "Poco/Crypto/Cipher.h"

using Poco::Util::Application;

namespace Reach {
	///RS_DecryptFile
	class DecryptFile : public Command
	{
	public:
		DecryptFile(std::string& SymKey, std::string& EncryptFile, std::string& OutDir);
		virtual void run();
	protected:
		virtual void generateKey();
	private:
		bool _decrypted;
		std::string _kv;
		Poco::FileInputStream _sink;
		Poco::FileOutputStream _sank;
		Poco::Crypto::Cipher* _pCipher;
	};

	class DecryptFileBySM4 : public Command
	{
	public:
		DecryptFileBySM4(std::string& SymKey, std::string& EncryptFile, std::string& OutFile);
		virtual ~DecryptFileBySM4();
		virtual void run();
	private:
		std::string _kv;
		Poco::FileInputStream _sink;
		Poco::FileOutputStream _sank;
	};

	class DecryptFileRequestHandler : public RESTfulRequestHandler
	{
	public:
		void handleRequest(HTTPServerRequest& request, HTTPServerResponse& response)
		{
			poco_debug_f1(Application::instance().logger(), "Request from %s", request.clientAddress().toString());

			RESTfulRequestHandler::handleCORS(request, response);

			HTMLForm form(request, request.stream());
			std::string SymKey(form.get("symKey", ""));
			std::string EncryptFile(form.get("encFilePath", ""));
			std::string OutDir(form.get("dncDirectoryPath", ""));

			DecryptFileBySM4 command(SymKey, EncryptFile, OutDir);
			command.execute();

			return response.sendBuffer(command().data(), command().length());
		}
	};
}