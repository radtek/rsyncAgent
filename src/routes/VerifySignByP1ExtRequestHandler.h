#pragma once

#include "Command.h"
#include "RESTfulRequestHandler.h"
#include "RequestHandleException.h"
#include "Poco/Util/Application.h"
#include "Poco/String.h"
#include "Poco/Crypto/X509Certificate.h"
#include "Poco/Net/HTTPClientSession.h"
#include "Poco/StreamCopier.h"
#include "Poco/URI.h"
#include "../Utility.h"
#include <string>
#include <cassert>

namespace Reach {

	using Poco::format;
	using Poco::Util::Application;
	using Poco::Crypto::X509Certificate;
	using Poco::Net::HTTPResponse;
	using Poco::Net::HTTPRequest;
	using Poco::Net::HTTPClientSession;
	using Poco::StreamCopier;
	using Poco::URI;

	///RS_VerifySignByP1Ext
	class VerifySignByP1Ext : public Command
	{
	public:
		VerifySignByP1Ext(const std::string& base64, const std::string& msg, const std::string& signature)
			:_cert(base64), _msg(msg), _signature(signature)
		{
			generator();
		}

		void run()
		{
			Application& app = Application::instance();

			URI uri(app.config().getString("signAndVerify"));
			poco_debug_f2(app.logger(), "VerifySignByP1Ext, URI : %s , uri.Path : %s", uri.toString(), uri.getPath());

			HTTPResponse response;
			HTTPRequest request(HTTPRequest::HTTP_POST, uri.getPath());
			request.setContentLength((int)_buffer.length());

			HTTPClientSession session(uri.getHost(), uri.getPort());
			session.sendRequest(request) << (_buffer);

			std::istream& receive = session.receiveResponse(response);
			std::ostringstream ostr;
			StreamCopier::copyStream(receive, ostr);

			_verify = Utility::resultFormNet(ostr.str());

			if (!_verify) {
				throw RequestHandleException("VerifySignByP1Ext failed!", RAR_UNKNOWNERR);
			}
		}
	protected:
		void generator()
		{
			/// Note: TIMESTAMP is NOT Equual to TRANSID's
			std::string fmt(
				"{\n"
				"	\"BIZCODE\" : \"MACHINE.RAWVERIFYCERT\",\n"
				"	\"DATA\" : {\n"
				"					\"msg\" : \"%s\",\n"
				"					\"signedData\" : \"%s\",\n"
				"					\"cert\" : \"%s\",\n"
				"					\"iAlgorithm\" : \"%s\"\n"
				"			},\n"
				"	\"SYSID\" : \"b253fca87f6711e995aa005056a72395\",\n"
				"		\"TIMESTAMP\" : \"%s\",\n"
				"		\"TRANSID\" : \"%s\",\n"
				"		\"UNIT\" : \"RSYZ\"\n"
				"}");

			
			decideAlgorithm(_cert);

			poco_debug_f1(Application::instance().logger(), "VerifySignByP1Ext signatureAlgorithm: %s", _algorithm);
			format(_buffer, fmt, _msg, _signature, _cert, _algorithm,Utility::timestamp(), Utility::UniqueTransOrder());

			assert(Utility::testJSON(_buffer));
			poco_debug_f1(Application::instance().logger(), "VerifySignByP1Ext JSON: %s", _buffer);
		}
	protected:
		void decideAlgorithm(const std::string& cert)
		{
#define CERT_SIGNATURE_ALG_SM3SM2			"1.2.156.10197.1.501"
			//1 RSA 2 SM2
			std::string PEM;
			PEM.append("-----BEGIN CERTIFICATE-----\n");
			PEM.append(Utility::cat("\n", 64, cert));
			PEM.append("-----END CERTIFICATE-----\n");
			std::istringstream certStream(PEM);
			X509Certificate x509(certStream);
			const X509_ALGOR* alg = X509_get0_tbs_sigalg(x509.certificate());
			Poco::Buffer<char> oid(128);
			int num = OBJ_obj2txt(oid.begin(), oid.size(), alg->algorithm, 1);
			poco_assert(num > oid.capacity());
			if (Poco::icompare(std::string(oid.begin(), num), CERT_SIGNATURE_ALG_SM3SM2) == 0)
				_algorithm = "2"; /// SM2
			else
				_algorithm = "1"; /// RSA
		}
	private:

		bool _verify;
		std::string _cert;
		std::string _msg;
		std::string _signature;
		std::string _buffer;
		std::string _algorithm;
	};

	class VerifySignByP1ExtRequestHandler : public RESTfulRequestHandler
	{
	public:
		void handleRequest(HTTPServerRequest& request, HTTPServerResponse& response)
		{
			poco_debug_f1(Application::instance().logger(), "Request from %s", request.clientAddress().toString());

			RESTfulRequestHandler::handleCORS(request, response);

			HTMLForm form(request, request.stream());
			std::string base64(form.get("certBase64", ""));
			std::string msg(form.get("msg", ""));
			std::string signature(form.get("signdMsg", ""));

			VerifySignByP1Ext command(base64, msg, signature);
			command.execute();

			return response.sendBuffer(command().data(), command().length());
		}
	};
}