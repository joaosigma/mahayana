#include "UTF.hpp"

#include "Types.hpp"

#include <windows.h>

namespace HorseRadish
{
	const HorseRadish::hChar UTF::BomUTF8[] = { 0xEF, 0xBB, 0xBF };
	const HorseRadish::hChar UTF::BomUTF16BigE[] = { 0xFE, 0xFF };
	const HorseRadish::hChar UTF::BomUTF16LittleE[] = { 0xFF, 0xFE };
	const HorseRadish::hChar UTF::BomUTF32BigE[] = { 0x00, 0x00, 0xFE, 0xFF };
	const HorseRadish::hChar UTF::BomUTF32LittleE[] = { 0xFF, 0xFE, 0x00, 0x00 };

	bool UTF::validateChar(const unsigned char * const charPtr)
	{
		int length;
		unsigned char a;

		length = trailingBytesUTF8[*charPtr] + 1;
		auto srcptr = charPtr + length;

		switch (length)
		{
			case 4: if ((a = (*--srcptr)) < 0x80 || a > 0xBF) return false;
			case 3: if ((a = (*--srcptr)) < 0x80 || a > 0xBF) return false;
			case 2: if ((a = (*--srcptr)) > 0xBF) return false;

				switch (*charPtr)
				{
				case 0xE0: if (a < 0xA0) return false; break;
				case 0xED: if (a > 0x9F) return false; break;
				case 0xF0: if (a < 0x90) return false; break;
				case 0xF4: if (a > 0x8F) return false; break;
				default:   if (a < 0x80) return false;
				}

			case 1: if (*charPtr >= 0x80 && *charPtr < 0xC2) return false;

			default: return false;
		}

		return (*charPtr <= 0xF4);
	}

	int UTF::ValidateUTF8(const void * const bufferUTF8, int *numChars, int *numBytes)
	{
		if (bufferUTF8 == nullptr)
			return -1;

		unsigned short numChar = 0;
		auto walker = reinterpret_cast<const unsigned char*>(bufferUTF8);

		for (; *walker != '\0'; walker += trailingBytesUTF8[*walker] + 1)
		{
			if (validateChar(walker) == false)
				return 0;

			numChar++;
		}

		if (numChars != nullptr)
			*numChars = numChar;
		if (numBytes != nullptr)
			*numBytes = walker - ((const unsigned char*)bufferUTF8);

		return 1;
	}

	int UTF::ConvertUTF8To(const String &stringIn, const Encoding targetEncoding, void *outBuffer, const int outBufferSize)
	{
		if ((outBuffer == nullptr) || (outBufferSize <= 0))
			return -1;

		if (targetEncoding == UTF::Encoding::Windows)
			return MultiByteToWideChar(CP_UTF8, 0, (LPCCH)stringIn.GetData(), -1, (LPWSTR)outBuffer, outBufferSize);

		return 0;
	}

	int UTF::ConvertUTF8To(const void * const bufferInUTF8, const Encoding targetEncoding, void *outBuffer, const int outBufferSize)
	{
		if ((bufferInUTF8 == nullptr) || (outBuffer == nullptr) || (outBufferSize <= 0))
			return -1;

		if (targetEncoding == UTF::Encoding::Windows)
			return MultiByteToWideChar(CP_UTF8, 0, (LPCCH)bufferInUTF8, -1, (LPWSTR)outBuffer, outBufferSize);

		return 0;
	}

	int UTF::ConvertToUTF8(const Encoding inEncoding, const void *inBuffer, void *outBuffer, const int outBufferSize)
	{
		if ((inBuffer == nullptr) || (outBuffer == nullptr) || (outBufferSize <= 0))
			return 0;

		if (inEncoding == UTF::Encoding::ASCII)
		{
			int numBytes = 0, numChars = 0;
			auto writer = reinterpret_cast<char*>(outBuffer);
			auto walker = reinterpret_cast<const unsigned char*>(inBuffer);

			while (*walker != 0)
			{
				auto curChar = *(walker++);

				if (curChar <= 127)
				{
					*writer++ = (char)curChar;
					numBytes++;
					numChars++;
				}
				else if (curChar <= 191)
				{
					*writer++ = 0xC280;
					*writer++ = (curChar - 128);
					numBytes += 2;
					numChars++;
				}
				else
				{
					*writer++ = 0xC380;
					*writer++ = (curChar - 192);
					numBytes += 2;
					numChars++;
				}
			}

			*writer = '\0';
			return numChars;
		}

		if (inEncoding == UTF::Encoding::UTF16)
		{
			int numBytes = 0, numChars = 0;
			auto writer = reinterpret_cast<char*>(outBuffer);
			auto walker = reinterpret_cast<const unsigned short*>(inBuffer);

			numBytes = numChars = 0;
			writer = (char*)outBuffer;
			walker = (unsigned short*)inBuffer;

			while (*walker != 0)
			{
				auto curChar = (unsigned short)(*(walker++));

				if (curChar < 0x80)
				{
					*writer++ = (char)curChar;
					numBytes++;
					numChars++;
				}
				else if (curChar < 0x800)
				{
					*writer++ = (curChar >> 6) | 0xC0;
					*writer++ = (curChar & 0x3F) | 0x80;
					numBytes += 2;
					numChars++;
				}
				else
				{
					*writer++ = (curChar >> 12) | 0xE0;
					*writer++ = ((curChar >> 6) & 0x3F) | 0x80;
					*writer++ = (curChar & 0x3F) | 0x80;
					numBytes += 3;
					numChars++;
				}
			}

			*writer = '\0';
			return numChars;
		}

		if (inEncoding == UTF::Encoding::UTF32)
		{
			int numBytes = 0, numChars = 0;
			auto writer = reinterpret_cast<char*>(outBuffer);
			auto walker = reinterpret_cast<const unsigned int*>(inBuffer);

			while (*walker != 0)
			{
				auto curChar = *(walker++);

				if (curChar < 0x80)
				{
					*writer++ = (char)curChar;
					numBytes++;
					numChars++;
				}
				else if (curChar < 0x800)
				{
					*writer++ = (curChar >> 6) | 0xC0;
					*writer++ = (curChar & 0x3F) | 0x80;
					numBytes += 2;
					numChars++;
				}
				else if (curChar < 0x10000)
				{
					*writer++ = (curChar >> 12) | 0xE0;
					*writer++ = ((curChar >> 6) & 0x3F) | 0x80;
					*writer++ = (curChar & 0x3F) | 0x80;
					numBytes += 3;
					numChars++;
				}
				else if (curChar < 0x110000)
				{
					*writer++ = (curChar >> 18) | 0xF0;
					*writer++ = ((curChar >> 12) & 0x3F) | 0x80;
					*writer++ = ((curChar >> 6) & 0x3F) | 0x80;
					*writer++ = (curChar & 0x3F) | 0x80;
					numBytes += 4;
					numChars++;
				}
			}

			*writer = '\0';
			return numChars;
		}

		if (inEncoding == UTF::Encoding::Windows)
		{
			auto numChars = WideCharToMultiByte(CP_UTF8, 0, (LPCWSTR)inBuffer, -1, (LPSTR)outBuffer, outBufferSize, nullptr, nullptr);
			return (numChars - 1);
		}

		return 0;
	}

	const char UTF::trailingBytesUTF8[256] = {
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
		2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 3, 3, 3, 3, 3, 3, 3, 3, 4, 4, 4, 4, 5, 5, 5, 5
	};

} //HorseRadish