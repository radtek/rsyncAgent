#include "DecryptFileRequestHandler.h"
#include "RequestHandleException.h"
#include "ErrorCode.h"
#include "../Utility.h"
#include "Poco/FileStream.h"
#include "Poco/Crypto/CipherFactory.h"
#include "Poco/Crypto/CipherKey.h"
#include "Poco/Crypto/Cipher.h"
#include "Poco/Crypto/CryptoStream.h"
#include "Poco/Base64Encoder.h"
#include "Poco/Base64Decoder.h"
#include "Poco/StringTokenizer.h"
#include "Poco/StreamCopier.h"
#include <cassert>

using namespace Reach;
using Poco::Crypto::CipherFactory;
using Poco::Crypto::Cipher;
using Poco::Crypto::CipherKey;
using Poco::Crypto::CryptoOutputStream;
using Poco::Base64Decoder;
using Poco::StringTokenizer;

DecryptFile::DecryptFile(std::string& SymKey, std::string& EncryptFile, std::string& OutFile)
:_kv(SymKey), _sink(EncryptFile), _sank(OutFile)
{
	if (_sink.bad() || _sank.bad())
		throw RequestHandleException(RAR_UNKNOWNERR);

	generateKey();
}

void DecryptFile::generateKey()
{
	Application& app = Application::instance();

	std::istringstream istr(_kv);
	Base64Decoder decoder(istr);
	std::string s;
	Poco::StreamCopier::copyToString(decoder, s);
	StringTokenizer st(s, "$", StringTokenizer::TOK_IGNORE_EMPTY);
	assert(st.count() == 2);

	CipherKey::ByteVec key(st[0].begin(), st[0].end());
	CipherKey::ByteVec iv(st[1].begin(), st[1].end());

	///TODO: SMS4-CBC - Further write to config()
	//CipherKey ckey("SMS4-CBC", key, iv);
	CipherKey ckey("aes256",key, iv);
	CipherFactory& factory = CipherFactory::defaultFactory();
	_pCipher = factory.createCipher(ckey);

	poco_debug_f1(app.logger(), "EncryptFile CipherKey::ByteVec:\n%s", _kv);
}

void DecryptFile::run()
{
	Application& app = Application::instance();

	CryptoOutputStream decryptorStream(_sank, _pCipher->createDecryptor());
	Poco::StreamCopier::copyStream(_sink, decryptorStream);
	decryptorStream.close();

	poco_debug_f1(app.logger(), "EncryptFile CipherKey::ByteVec:\n%s", _kv);
}

#include "Poco/File.h"
DecryptFileBySM4::DecryptFileBySM4(std::string& SymKey, std::string& EncryptFile, std::string& OutFile)
	: _kv(SymKey), _sink(EncryptFile, std::ios::binary)
{
	if (_sink.bad())
		throw RequestHandleException(RAR_ERRNODECRYPT);

	Poco::File f(OutFile);
	if (!f.exists() && !f.createFile())
		throw Poco::FileNotFoundException("DecryptFileBySM4", OutFile);
	
	_sank.open(OutFile, std::ios_base::trunc | std::ios_base::out | std::ios::binary);
}

#include "sm4/_sm4.h"
DecryptFileBySM4::~DecryptFileBySM4()
{
	_sink.close();
	_sank.close();
}

void DecryptFileBySM4::run()
{
	const int encrypt = 1;
	const int decrypt = 0;
	const int bsize = 128;

	sm4_context ctx;
	sm4_setkey_dec(&ctx, (unsigned char*)(_kv.data()));

	Application& app = Application::instance();
	while (!_sink.eof())
	{
		unsigned char rbuf[bsize] = {0};
		unsigned char wbuf[bsize] = {0};

		_sink.read(reinterpret_cast<char*>(rbuf), bsize);
		std::streamsize cb = _sink.gcount();
		if (!cb) continue;

		sm4_crypt_ecb(&ctx, decrypt, static_cast<int>(cb), rbuf, wbuf);

		_sank.write(reinterpret_cast<char*>(wbuf),cb);
		_sank.flush();
	}
}