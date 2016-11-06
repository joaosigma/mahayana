#pragma once

#include "stream.hpp"

#include <string>
#include <vector>

namespace hr
{
	class Encoders
	{
		static constexpr const char base64Chars[] = { 'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J',
													  'K', 'L', 'M', 'N', 'O', 'P', 'Q', 'R', 'S', 'T',
													  'U', 'V', 'W', 'X', 'Y', 'Z', 'a', 'b', 'c', 'd',
													  'e', 'f', 'g', 'h', 'i', 'j', 'k', 'l', 'm', 'n',
													  'o', 'p', 'q', 'r', 's', 't', 'u', 'v', 'w', 'x',
													  'y', 'z', '0', '1', '2', '3', '4', '5', '6', '7',
												      '8', '9', '+', '/' };

		static const unsigned char hexEncodeLookupLower[513];
		static const unsigned char hexEncodeLookupUpper[513];
		static const unsigned char hexDecodeLookup[103];

		static bool isBase64Char(const char caracter);
		static unsigned int findBase64Char(const char caracter);

	public:
		static std::string encodeBase64(const void * const buffer, size_t bufferSize);
		static void encodeBase64(const void * const buffer, size_t bufferSize, std::string &stringOut);
		static void encodeBase64(const void * const buffer, size_t bufferSize, hr::streams::Stream &streamOut);

		static size_t decodeBase64RequiredSize(size_t numBase64Chars);
		static size_t decodeBase64(const std::string &dataBase64, void* bufferOut);
		static size_t decodeBase64(const std::string &dataBase64, std::vector<unsigned char> &bufferOut);
		static size_t decodeBase64(const std::string &dataBase64, hr::streams::Stream &streamOut);
		static size_t decodeBase64(const char* const dataBase64, size_t dataSize, void* bufferOut);
		static size_t decodeBase64(const char* const dataBase64, size_t dataSize, std::vector<unsigned char> &bufferOut);
		static size_t decodeBase64(const char* const dataBase64, size_t dataSize, hr::streams::Stream &streamOut);

		static std::string encodeHex(const void * const buffer, size_t bufferSize, bool toUppercase);
		static void encodeHex(const void * const buffer, size_t bufferSize, bool toUppercase, std::string &stringOut);
		static void encodeHex(const void * const buffer, size_t bufferSize, bool toUppercase, hr::streams::Stream &streamOut);
		static void encodeHexByte(const unsigned char valByte, char * const outHex);

		static size_t decodeHexRequiredSize(size_t numHexChars);
		static size_t decodeHex(const std::string &dataHex, hr::streams::Stream &streamOut);
		static unsigned char decodeHexByte(const char * const dataHex);
	};
}
