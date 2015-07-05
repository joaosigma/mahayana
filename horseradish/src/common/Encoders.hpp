#pragma once

#include "Stream.hpp"

#include <string>
#include <vector>

namespace HorseRadish
{
	class Encoders
	{
		static const char base64Chars[65];
		static const unsigned char hexEncodeLookupLower[513];
		static const unsigned char hexEncodeLookupUpper[513];
		static const unsigned char hexDecodeLookup[103];

		static bool isBase64Char(const char caracter);
		static unsigned int findBase64Char(const char caracter);

	public:
		static std::string EncodeBase64(const void * const buffer, unsigned int bufferSize);
		static void EncodeBase64(const void * const buffer, unsigned int bufferSize, std::string &stringOut);
		static void EncodeBase64(const void * const buffer, unsigned int bufferSize, HorseRadish::Streams::Stream &streamOut);

		static unsigned int DecodeBase64RequiredSize(unsigned int numBase64Chars);
		static unsigned int DecodeBase64(const std::string &dataBase64, void* bufferOut);
		static unsigned int DecodeBase64(const std::string &dataBase64, std::vector<unsigned char> &bufferOut);
		static unsigned int DecodeBase64(const std::string &dataBase64, HorseRadish::Streams::Stream &streamOut);

		static std::string EncodeHex(const void * const buffer, unsigned int bufferSize, bool toUppercase);
		static void EncodeHex(const void * const buffer, unsigned int bufferSize, bool toUppercase, std::string &stringOut);
		static void EncodeHex(const void * const buffer, unsigned int bufferSize, bool toUppercase, HorseRadish::Streams::Stream &streamOut);
		static void EncodeHexByte(const unsigned char valByte, char * const outHex);

		static unsigned int DecodeHexRequiredSize(unsigned int numHexChars);
		static unsigned int DecodeHex(const std::string &dataHex, HorseRadish::Streams::Stream &streamOut);
		static unsigned char DecodeHexByte(const char * const dataHex);
	};

} //HorseRadish
