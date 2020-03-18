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

namespace Reach {

	using Poco::Util::Application;
	using Poco::FileOutputStream;
	using qrcodegen::QrCode;
	using Poco::OutputStreamConverter;
	using Poco::UTF8Encoding;
	using Poco::UTF16Encoding;
	using Poco::TextConverter;
	using Poco::UnicodeConverter;
	///RS_GreateQRCode
	class GreateQRCode : public Command
	{
	public:
		GreateQRCode(const std::string& text, const std::string& path)
			:_text(text), _path(path)
		{
		}

		void run()
		{
			Application& app = Application::instance();
			/// const char *text = "Hello, world!";              // User-supplied text
			const QrCode::Ecc errCorLvl = QrCode::Ecc::MEDIUM;  // Error correction level

			// Make and print the QR Code symbol
			const QrCode qr = QrCode::encodeText(_text.data(), errCorLvl);
			printQr(qr);
			poco_information_f2(app.logger(), "version : %d, mask : %d", qr.getVersion(), qr.getMask());

			/*FileOutputStream ofs(_path);
			ofs << qr.toSvgString(0) << std::endl;
			ofs.close();*/

			saveBmp(qr, 0, 8);
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

			int	unWidth, unHeight, unDataBytes;
			//int border = 4; // do not support

			unHeight = unWidth = (qr.getSize() + border * 2) * scale;

			int iLineByteCnt = (((unWidth * 8) + 31) >> 5) << 2;
			unDataBytes = iLineByteCnt * unHeight * 8;

			BITMAPFILEHEADER kFileHeader;
			kFileHeader.bfType = 0x4d42;  // "BM"
			kFileHeader.bfSize = sizeof(BITMAPFILEHEADER) +
				sizeof(BITMAPINFOHEADER) +
				unDataBytes / 8;
			kFileHeader.bfReserved1 = 0;
			kFileHeader.bfReserved2 = 0;
			kFileHeader.bfOffBits = sizeof(BITMAPFILEHEADER) +
				sizeof(BITMAPINFOHEADER);

			BITMAPINFOHEADER kInfoHeader;
			kInfoHeader.biSize = sizeof(BITMAPINFOHEADER);
			kInfoHeader.biWidth = iLineByteCnt;
			kInfoHeader.biHeight = -unHeight;
			kInfoHeader.biPlanes = 1;
			kInfoHeader.biBitCount = 24;
			kInfoHeader.biCompression = BI_RGB;
			kInfoHeader.biSizeImage = 0;
			kInfoHeader.biXPelsPerMeter = 0;
			kInfoHeader.biYPelsPerMeter = 0;
			kInfoHeader.biClrUsed = 0;
			kInfoHeader.biClrImportant = 0;

			Poco::Buffer<char> data(0);

			int x ,y;
			for (int h = 0; h < unHeight; h++) {
				for (int w = 0; w < iLineByteCnt; w++) {
					//x = round(double(1 / scale * w));
					//y = round(double(1 / scale * h));
					x = round(double(1 / scale * (w - border)));
					y = round(double(1 / scale * (h - border)));
					char cpixle = (qr.getModule(x, y) ? 0x00 : 0xFF);
					int pos = (h * iLineByteCnt + w) * 3;
					data.append(cpixle);
					data.append(cpixle);
					data.append(cpixle);
				}
				std::cout << std::endl;
			}
			std::cout << std::endl;
			

			FileOutputStream ofs(_path);
			ofs.write((char*)&kFileHeader, sizeof(kFileHeader));
			ofs.write((char*)&kInfoHeader, sizeof(kInfoHeader));
			ofs.write((char*)data.begin(), data.size());
			ofs.close();
		}

	private:
		std::string _text;
		std::string _path;
	};

	class GreateQRCodeRequestHandler : public RESTfulRequestHandler
	{
	public:
		void handleRequest(HTTPServerRequest& request, HTTPServerResponse& response)
		{
			poco_information_f1(Application::instance().logger(), "Request from %s", request.clientAddress().toString());
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
