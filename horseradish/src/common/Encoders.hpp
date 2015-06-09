#pragma once

#include "Platform.hpp"
#include "String.hpp"
#include "Types.hpp"
#include "Stream.hpp"

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
		static HorseRadish::String EncodeBase64(const void * const buffer, unsigned int bufferSize);
		static void EncodeBase64(const void * const buffer, unsigned int bufferSize, HorseRadish::String &stringOut);
		static void EncodeBase64(const void * const buffer, unsigned int bufferSize, HorseRadish::Streams::Stream &streamOut);

		static unsigned int DecodeBase64RequiredSize(unsigned int numBase64Chars);
		static unsigned int DecodeBase64(const HorseRadish::String &dataBase64, HorseRadish::Streams::Stream &streamOut);

		static HorseRadish::String EncodeHex(const void * const buffer, unsigned int bufferSize, bool toUppercase);
		static void EncodeHex(const void * const buffer, unsigned int bufferSize, bool toUppercase, HorseRadish::String &stringOut);
		static void EncodeHex(const void * const buffer, unsigned int bufferSize, bool toUppercase, HorseRadish::Streams::Stream &streamOut);
		static void EncodeHexByte(const unsigned char valByte, char * const outHex);

		static unsigned int DecodeHexRequiredSize(unsigned int numHexChars);
		static unsigned int DecodeHex(const HorseRadish::String &dataHex, HorseRadish::Streams::Stream &streamOut);
		static unsigned char DecodeHexByte(const char * const dataHex);
	};

} //HorseRadish
