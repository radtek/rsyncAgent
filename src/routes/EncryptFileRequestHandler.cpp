#include "EncryptFileRequestHandler.h"
#include "RequestHandleException.h"
#include "ErrorCode.h"
#include "Poco/File.h"
#include "Poco/FileStream.h"
#include "Poco/Crypto/CipherFactory.h"
#include "Poco/Crypto/CipherKey.h"
#include "Poco/Crypto/Cipher.h"
#include "Poco/Crypto/CryptoStream.h"
#include "Poco/Base64Encoder.h"
#include "Poco/StreamCopier.h"
#include "Poco/String.h"
#include "../Utility.h"
#include <cassert>
#include "sm4/_sm4.h"

using namespace Reach;
using Poco::File;
using Poco::Crypto::CipherFactory;
using Poco::Crypto::Cipher;
using Poco::Crypto::CipherKey;
using Poco::Crypto::CryptoOutputStream;
using Poco::Base64Encoder;

EncryptFile::EncryptFile(const std::string& SourceFile, const std::string& EncryptFile)
	:_sink(SourceFile), _sank(EncryptFile), _encrypted(false)
{
	if(_sink.bad())
		throw RequestHandleException(RAR_UNKNOWNERR);

	generateKey();
}

void EncryptFile::generateKey()
{
	Application& app = Application::instance();
	///TODO: SMS4-CBC - Further write to config()
	//CipherKey ckey("SMS4-CBC");
	CipherKey ckey("aes256");
	CipherFactory& factory = CipherFactory::defaultFactory();
	_pCipher = factory.createCipher(ckey);

	std::ostringstream keyStream;
	Base64Encoder encoder(keyStream);

	std::string delim("$");
	encoder.write(reinterpret_cast<const char*>(&ckey.getKey()[0]), ckey.keySize());
	encoder << delim;
	encoder.write(reinterpret_cast<const char*>(&ckey.getIV()[0]), ckey.ivSize());
	encoder.close();

	_symKey = keyStream.str();

	poco_debug_f1(app.logger(), "EncryptFile CipherKey::ByteVec:\n%s", _symKey);
}

void EncryptFile::run()
{
	Application& app = Application::instance();

	CryptoOutputStream encryptorStream(_sank, _pCipher->createEncryptor());
	Poco::StreamCopier::copyStream(_sink, encryptorStream);
	encryptorStream.close();

	add("symKey", _symKey);
}

#include "Poco/Random.h"

static std::string randomString(std::size_t keysize)
{
	if (keysize > 128 ) 
		throw Poco::InvalidArgumentException("randomString","The key is too large!");

	std::string randomStr;
	Poco::Random rnd;
	rnd.seed();
	const int n = 62;
	unsigned char d[] = {
		'a','b','c','d','e','f','g','h','i','j','k','l','m','n',
		'o','p','q','r','s','t','v','u','w','x','y','z',
		'0','1','2','3','4','5','6','7','8','9',
		'A','B','C','D','E','F','G','H','I','J','K','L','M','N',
		'O','P','Q','R','S','T','V','U','W','X','Y','Z' };
	for (int i = 0; i < keysize; ++i)
		randomStr += d[rnd.next() % n];

	return randomStr;
}

EncryptFileBySM4::EncryptFileBySM4(const std::string& SourceFile, const std::string& EncryptFile)
	:_sink(SourceFile, std::ios::binary), _sank(EncryptFile, std::ios::binary | std::ios_base::trunc),
	_encrypted(false)
{
	Application& app = Application::instance();

	if (_sink.bad())
		throw RequestHandleException(RAR_ERRNOENCRYPT);

	_symKey = randomString(app.config().getUInt("keysize", 32));
}

EncryptFileBySM4::~EncryptFileBySM4()
{
	_sink.close();
	_sank.close();
}

void EncryptFileBySM4::run()
{	
	Application& app = Application::instance();

	const int encrypt = 1;
	const int decrypt = 0;
	const int bsize = 128;

	sm4_context ctx;
	sm4_setkey_enc(&ctx, (unsigned char*)(_symKey.data()));
	
	while (!_sink.eof())
	{
		unsigned char rbuf[bsize] = { 4 };//special comment character
		unsigned char wbuf[bsize] = { 4 };//special comment character

		_sink.read(reinterpret_cast<char*>(rbuf), bsize);
		std::streamsize cb = _sink.gcount();
		sm4_crypt_ecb(&ctx, encrypt, static_cast<int>(cb), rbuf, wbuf);

		_sank.write(reinterpret_cast<char*>(wbuf), bsize);
		_sank.flush();
	}

	add("symKey", _symKey);
}