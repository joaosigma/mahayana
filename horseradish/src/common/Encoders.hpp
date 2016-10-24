#pragma once

#include "Stream.hpp"

#include <string>
#include <vector>

namespace HorseRadish
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
		static void encodeBase64(const void * const buffer, size_t bufferSize, HorseRadish::Streams::Stream &streamOut);

		static size_t decodeBase64RequiredSize(size_t numBase64Chars);
		static size_t decodeBase64(const std::string &dataBase64, void* bufferOut);
		static size_t decodeBase64(const std::string &dataBase64, std::vector<unsigned char> &bufferOut);
		static size_t decodeBase64(const std::string &dataBase64, HorseRadish::Streams::Stream &streamOut);

		static std::string encodeHex(const void * const buffer, size_t bufferSize, bool toUppercase);
		static void encodeHex(const void * const buffer, size_t bufferSize, bool toUppercase, std::string &stringOut);
		static void encodeHex(const void * const buffer, size_t bufferSize, bool toUppercase, HorseRadish::Streams::Stream &streamOut);
		static void encodeHexByte(const unsigned char valByte, char * const outHex);

		static size_t decodeHexRequiredSize(size_t numHexChars);
		static size_t decodeHex(const std::string &dataHex, HorseRadish::Streams::Stream &streamOut);
		static unsigned char decodeHexByte(const char * const dataHex);
	};

} //HorseRadish
