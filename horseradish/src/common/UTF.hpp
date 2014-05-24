#pragma once
#ifndef __HUTF__
#define __HUTF__

#include "String.hpp"
#include "Platform.hpp"
#include "Types.hpp"

namespace HorseRadish
{

class UTF
{
	static const char trailingBytesUTF8[256];

	static bool validateChar(const unsigned char * const charPtr);

public:
	enum Encoding{
		UTF8,
		UTF16,
		UTF32,
		ASCII,
		Windows
		};

	static const HorseRadish::hChar BomUTF8[];
	static const HorseRadish::hChar BomUTF16BigE[];
	static const HorseRadish::hChar BomUTF16LittleE[];
	static const HorseRadish::hChar BomUTF32BigE[];
	static const HorseRadish::hChar BomUTF32LittleE[];

	static int ValidateUTF8(const void * const bufferUTF8, int *numChars, int *numBytes);
	static int ConvertUTF8To(const String &stringIn, const Encoding targetEncoding, void *outBuffer, const int outBufferSize);
	static int ConvertUTF8To(const void * const bufferInUTF8, const Encoding targetEncoding, void *outBuffer, const int outBufferSize);
	static int ConvertToUTF8(const Encoding inEncoding, const void *inBuffer, void *outBuffer, const int outBufferSize);
};

}//namespace HorseRadish

#endif