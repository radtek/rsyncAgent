/// RS_EncryptData
#include "EncryptDataRequestHandler.h"
#include "Poco/Exception.h"

using namespace Reach;

static std::string randomString(std::size_t keysize);

EncryptData::EncryptData(const std::string& data)
	:_data(data)
{
	Application& app = Application::instance();

	if (data.empty())
		throw Poco::InvalidArgumentException("RS_EncryptData", "The data is empty!", RAR_UNKNOWNERR);

	if (data.length() > _maxLen)
		throw Poco::InvalidArgumentException("RS_EncryptData", "The data must be small than 1024!", RAR_UNKNOWNERR);

	_kv = randomString(app.config().getUInt("keysize", 32));
}

#include "Poco/Random.h"

static std::string randomString(std::size_t keysize)
{
	if (keysize > 128)
		throw Poco::InvalidArgumentException("randomString", "The key is too large!");

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

#include "sm4/_sm4.h"
#include "Poco/MemoryStream.h"
#include "Poco/HexBinaryEncoder.h"

void EncryptData::run()
{
	Application& app = Application::instance();

	const int encrypt = 1;
	const int decrypt = 0;
	const int bsize = 128;

	sm4_context ctx;
	sm4_setkey_enc(&ctx, (unsigned char*)(_kv.data()));
	Poco::MemoryInputStream sink(_data.data(), _data.size());

	std::ostringstream o;
	Poco::HexBinaryEncoder hex(o);
	hex.rdbuf()->setLineLength(0);

	while (!sink.eof())
	{
		unsigned char rbuf[bsize] = { 0 };
		unsigned char wbuf[bsize] = { 0 };

		sink.read(reinterpret_cast<char*>(rbuf), bsize);
		std::streamsize cb = sink.gcount();
		sm4_crypt_ecb(&ctx, encrypt, cb, rbuf, wbuf);

		hex << wbuf;
	}

	add("symKey", _kv);
	add("encDataStr", o.str());
}