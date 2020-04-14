#pragma once

#include "Poco/Util/Application.h"
#include "Poco/FileStream.h"
#include "../Command.h"
#include "../RESTfulRequestHandler.h"
#include "../../qrcode/QrCode.hpp"
#include "Poco/StreamConverter.h"
#include "Poco/TextConverter.h"
#include "Poco/UnicodeConverter.h"
#include "Poco/UTF16Encoding.h"
#include "Poco/Stopwatch.h"

namespace Reach {

	using Poco::Util::Application;
	using Poco::FileOutputStream;
	using qrcodegen::QrCode;
	using Poco::OutputStreamConverter;
	using Poco::UTF8Encoding;
	using Poco::UTF16Encoding;
	using Poco::TextConverter;
	using Poco::UnicodeConverter;
	using Poco::Stopwatch;
	///RS_GreateQRCode
	class GreateQRCode : public Command
	{
	public:
		GreateQRCode(const std::string& text, const std::string& path)
			:_text(text), _path(path)
		{
			sw.reset();
		}

		void run()
		{
			sw.start();
			Application& app = Application::instance();
			/// const char *text = "Hello, world!";              // User-supplied text
			const QrCode::Ecc errCorLvl = QrCode::Ecc::MEDIUM;  // Error correction level

			// Make and print the QR Code symbol
			const QrCode qr = QrCode::encodeText(_text.data(), errCorLvl);
			//printQr(qr);
			poco_debug_f2(app.logger(), "version : %d, mask : %d", qr.getVersion(), qr.getMask());

			saveBmp(qr, app.config().getInt("qrcode.border", 2), app.config().getInt("qrcode.scale", 4));

			sw.stop();
			poco_debug_f1(app.logger(), "GreateQRCode : %?d millisecond", sw.elapsed() / 1000);
		}

		void printQr(const QrCode &qr) {

			int border = 0;
			HANDLE _hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
			DWORD written;

			for (int y = -border; y < qr.getSize() + border; y++) {
				for (int x = -border; x < qr.getSize() + border; x++) {
					std::string utf8 = (qr.getModule(x, y) ? "\xE2\x96\xA0" : "  ");
					Poco::UTF16String ucs2;
					UnicodeConverter::convert(utf8, ucs2);

					WriteConsoleW(_hConsole, ucs2.data(), static_cast<DWORD>(ucs2.size()), &written, NULL);
				}
				std::cout << std::endl;
			}
			std::cout << std::endl;
		}

		void saveBmp(const QrCode &qr, int border, float scale = 1) {

			if (border < 0)
				throw std::domain_error("Border must be non-negative");
			if (border > INT_MAX / 2 || border * 2 > INT_MAX - qr.getSize())
				throw std::overflow_error("Border too large");

			Application& app = Application::instance();

			int	unWidth, unHeight, unDataBytes, biBit = 24;
			//int border = 4; // do not support

			unHeight = unWidth = (qr.getSize() + border * 2) * scale;

			int iLineByteCnt = ((unWidth  * biBit / 8) + 3) >> 2 << 2;//iLineByteCnt divide by 4
			unDataBytes = unHeight * iLineByteCnt;

			BITMAPFILEHEADER kFileHeader;
			kFileHeader.bfType = 0x4d42;  // "BM"
			kFileHeader.bfSize = sizeof(BITMAPFILEHEADER) +
				sizeof(BITMAPINFOHEADER) +
				unDataBytes;
			kFileHeader.bfReserved1 = 0;
			kFileHeader.bfReserved2 = 0;
			kFileHeader.bfOffBits = sizeof(BITMAPFILEHEADER) +
				sizeof(BITMAPINFOHEADER);

			BITMAPINFOHEADER kInfoHeader;
			kInfoHeader.biSize = sizeof(BITMAPINFOHEADER);
			kInfoHeader.biWidth = unWidth;
			kInfoHeader.biHeight = -unHeight;
			kInfoHeader.biPlanes = 1;
			kInfoHeader.biBitCount = biBit;
			kInfoHeader.biCompression = BI_RGB;
			kInfoHeader.biSizeImage = 0;
			kInfoHeader.biXPelsPerMeter = 0;
			kInfoHeader.biYPelsPerMeter = 0;
			kInfoHeader.biClrUsed = 0;
			kInfoHeader.biClrImportant = 0;

			std::vector<char> data(unDataBytes, 0xFF);

			for (int y = -border; y < qr.getSize() + border; y++) {
				for (int x = -border; x < qr.getSize() + border; x++) {
					char cpixle = (qr.getModule(x, y) ? 0x00 : 0xFF);
					if (x < 0 || y < 0)
						continue;
					int X = (x + border) * scale;
					int Y = (y + border) * scale;
					for (int i = 0; i < scale; ++i) {
						for (int j = 0; j < scale; ++j) {
							int pos = (Y + i) * iLineByteCnt + (X + j) * 3;
							data[pos] = cpixle;
							data[pos + 1] = cpixle;
							data[pos + 2] = cpixle;
						}
					}
				}
			}

			FileOutputStream ofs(_path);
			ofs.write((char*)&kFileHeader, sizeof(kFileHeader));
			ofs.write((char*)&kInfoHeader, sizeof(kInfoHeader));
			ofs.write((char*)data.data(), data.size());
			ofs.close();
		}

	private:
		std::string _text;
		std::string _path;
		Stopwatch sw;
	};

	class GreateQRCodeRequestHandler : public RESTfulRequestHandler
	{
	public:
		void handleRequest(HTTPServerRequest& request, HTTPServerResponse& response)
		{
			poco_trace_f1(Application::instance().logger(), "Request from %s", request.clientAddress().toString());
			RESTfulRequestHandler::handleCORS(request, response);

			HTMLForm form(request, request.stream());
			std::string qrcode = form.get("qrcode", "");
			std::string path = form.get("path", "");
			GreateQRCode command(qrcode, path);
			command.execute();

			return response.sendBuffer(command().data(), command().length());
		}
	};
}
