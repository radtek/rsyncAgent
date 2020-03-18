#include "KeyDecryptDataRequestHandler.h"
#include "RequestHandleException.h"
#include "Reach/Data/Session.h"
#include "Poco/Base64Decoder.h"
#include "Poco/StreamCopier.h"
#include "Poco/RegularExpression.h"
#include "../Utility.h"

#include <cassert>

using namespace Reach;
using Reach::Data::Session;
using Poco::Base64Decoder;
using Poco::StreamCopier;
using Poco::RegularExpression;

KeyDecryptData::KeyDecryptData(const std::string& uid, const std::string& encryptBuffer)
	:_uid(uid), _encrypt_data(encryptBuffer)
{
}

void KeyDecryptData::run()
{
	Session session(Utility::getSession());

	if (_uid != session.contianer())
		throw RequestHandleException(RAR_UNIQUEIDUNCORRECT);

	std::string pattern("(\\S+)@@@(\\S+)");
	int options = 0;

	RegularExpression re(pattern, options);
	RegularExpression::Match mtch;

	if (!re.match(_encrypt_data, mtch)) {
		/// Decrypt data directly if the encrypt string was not contain @@@
		_decrypt_data = session.decryptData(_encrypt_data);
		if (_decrypt_data.empty())
			throw RequestHandleException(RAR_DECRYPTFAILED);

		return add("rsKey", _decrypt_data);
	}
	
	std::vector<std::string> tags;
	re.split(_encrypt_data, tags, options);
	std::string& encrypt = tags[1];
	std::string& cert = tags[2];

	assert(tags.size() > 2);
	//std::string content = SOF_ExportExChangeUserCert(_uid);
	enum certType { sign = 1, crypto };
	std::string content = session.getCertBase64String(certType::crypto);

	if (content != cert)
		throw RequestHandleException("certificate error", RAR_ERRDECRYPTCERT);

	_decrypt_data = session.decryptData(encrypt);
	if (_decrypt_data.empty())
		throw RequestHandleException(RAR_DECRYPTFAILED);

	add("rsKey", _decrypt_data);
}