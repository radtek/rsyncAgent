/// RS_DevryptData
#include "DevryptDataRequestHandler.h"
#include "Poco/Exception.h"

using namespace Reach;

DevryptData::DevryptData(const std::string& symKey, const std::string& data)
	:_kv(symKey), _data(data)
{
	if (symKey.empty())
		throw Poco::InvalidArgumentException("RS_DevryptData", "symKey is empty!", RAR_UNKNOWNERR);

	if (data.empty())
		throw Poco::InvalidArgumentException("RS_DevryptData", "data is empty!", RAR_UNKNOWNERR);
}

#include "sm4/_sm4.h"
#include "Poco/MemoryStream.h"
#include "Poco/HexBinaryDecoder.h"
#include "Poco/StreamCopier.h"
#include "Poco/String.h"

void DevryptData::run()
{
	Application& app = Application::instance();

	const int encrypt = 1;
	const int decrypt = 0;
	const int bsize = 128;

	sm4_context ctx;
	sm4_setkey_dec(&ctx, (unsigned char*)(_kv.data()));

	Poco::MemoryInputStream sink(_data.data(), _data.size());
	Poco::HexBinaryDecoder hex(sink);
	std::ostringstream o;
	while (!hex.eof())
	{
		unsigned char rbuf[bsize] = { 0 };
		unsigned char wbuf[bsize] = { 0 };

		hex.read(reinterpret_cast<char*>(rbuf), bsize);
		std::streamsize cb = hex.gcount();
		if (!cb) continue;

		sm4_crypt_ecb(&ctx, decrypt, cb, rbuf, wbuf);
	
		o << wbuf;
	}

	poco_debug_f1(app.logger(), "DevryptData dncDataStr : \n%s", o.str());
	add("dncDataStr", o.str());
}