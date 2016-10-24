#include "Encoders.hpp"

namespace HorseRadish
{
	bool Encoders::isBase64Char(const char caracter)
	{
		return (((caracter >= '0') && (caracter <= '9')) || ((caracter >= 'A') && (caracter <= 'Z')) || ((caracter >= 'a') && (caracter <= 'z')) || (caracter == '+') || (caracter == '/'));
	}

	unsigned int Encoders::findBase64Char(const char caracter)
	{
		if ((caracter >= '0') && (caracter <= '9'))
			return ((caracter - '0') + 52);
		if ((caracter >= 'A') && (caracter <= 'Z'))
			return ((caracter - 'A') + 0);
		if ((caracter >= 'a') && (caracter <= 'z'))
			return ((caracter - 'a') + 26);
		if (caracter == '+')
			return 62;
		if (caracter == '/')
			return 63;
		return -1;
	}

	std::string Encoders::encodeBase64(const void * const buffer, size_t bufferSize)
	{
		std::string stringOut;

		Encoders::encodeBase64(buffer, bufferSize, stringOut);
		return stringOut;
	}

	void Encoders::encodeBase64(const void * const buffer, size_t bufferSize, std::string &stringOut)
	{
		stringOut.reserve(stringOut.size() + (4 * (bufferSize + 3) / 3 + 2));

		size_t i = 0;
		unsigned char char_array_3[3];
		unsigned char char_array_4[4];

		auto bufferWalker = reinterpret_cast<const unsigned char*>(buffer);

		while (bufferSize--)
		{
			char_array_3[i++] = *(bufferWalker++);
			if (i == 3)
			{
				char_array_4[0] = (char_array_3[0] & 0xfc) >> 2;
				char_array_4[1] = ((char_array_3[0] & 0x03) << 4) + ((char_array_3[1] & 0xf0) >> 4);
				char_array_4[2] = ((char_array_3[1] & 0x0f) << 2) + ((char_array_3[2] & 0xc0) >> 6);
				char_array_4[3] = char_array_3[2] & 0x3f;

				stringOut += Encoders::base64Chars[char_array_4[0]];
				stringOut += Encoders::base64Chars[char_array_4[1]];
				stringOut += Encoders::base64Chars[char_array_4[2]];
				stringOut += Encoders::base64Chars[char_array_4[3]];
				i = 0;
			}
		}

		if (i)
		{
			for (size_t j = i; j < 3; j++)
				char_array_3[j] = '\0';

			char_array_4[0] = (char_array_3[0] & 0xfc) >> 2;
			char_array_4[1] = ((char_array_3[0] & 0x03) << 4) + ((char_array_3[1] & 0xf0) >> 4);
			char_array_4[2] = ((char_array_3[1] & 0x0f) << 2) + ((char_array_3[2] & 0xc0) >> 6);
			char_array_4[3] = char_array_3[2] & 0x3f;

			for (size_t j = 0; (j < i + 1); j++)
				stringOut += Encoders::base64Chars[char_array_4[j]];

			while ((i++ < 3))
				stringOut += '=';
		}
	}

	void Encoders::encodeBase64(const void * const buffer, size_t bufferSize, HorseRadish::Streams::Stream &streamOut)
	{
		size_t i = 0;
		unsigned char char_array_3[3];
		unsigned char char_array_4[4];

		auto bufferWalker = reinterpret_cast<const unsigned char*>(buffer);

		while (bufferSize--)
		{
			char_array_3[i++] = *(bufferWalker++);
			if (i == 3)
			{
				char_array_4[0] = (char_array_3[0] & 0xfc) >> 2;
				char_array_4[1] = ((char_array_3[0] & 0x03) << 4) + ((char_array_3[1] & 0xf0) >> 4);
				char_array_4[2] = ((char_array_3[1] & 0x0f) << 2) + ((char_array_3[2] & 0xc0) >> 6);
				char_array_4[3] = char_array_3[2] & 0x3f;

				streamOut.write(Encoders::base64Chars + char_array_4[0], sizeof(char));
				streamOut.write(Encoders::base64Chars + char_array_4[1], sizeof(char));
				streamOut.write(Encoders::base64Chars + char_array_4[2], sizeof(char));
				streamOut.write(Encoders::base64Chars + char_array_4[3], sizeof(char));
				i = 0;
			}
		}

		if (i)
		{
			for (size_t j = i; j < 3; j++)
				char_array_3[j] = '\0';

			char_array_4[0] = (char_array_3[0] & 0xfc) >> 2;
			char_array_4[1] = ((char_array_3[0] & 0x03) << 4) + ((char_array_3[1] & 0xf0) >> 4);
			char_array_4[2] = ((char_array_3[1] & 0x0f) << 2) + ((char_array_3[2] & 0xc0) >> 6);
			char_array_4[3] = char_array_3[2] & 0x3f;

			for (size_t j = 0; (j < i + 1); j++)
				streamOut.write(Encoders::base64Chars + char_array_4[j], sizeof(char));

			char endChar = '=';
			while ((i++ < 3))
				streamOut.write(&endChar, sizeof(char));
		}
	}

	size_t Encoders::decodeBase64RequiredSize(size_t numBase64Chars)
	{
		//base64 takes 137% of the original size, so we must divide by 1.37 or simply multiply by 0.73 which can be approximated by 3/4
		return (3 * numBase64Chars / 4);
	}

	size_t Encoders::decodeBase64(const std::string &dataBase64, void* bufferOut)
	{
		if (dataBase64.empty())
			return 0;

		size_t in_len = dataBase64.size();
		size_t i = 0;
		size_t in_ = 0;
		size_t bytesWritten = 0;
		unsigned char char_array_4[4], char_array_3[3];

		while (in_len-- && (dataBase64[in_] != '=') && Encoders::isBase64Char(dataBase64[in_]))
		{
			char_array_4[i++] = dataBase64[in_];
			in_++;
			if (i == 4)
			{
				char_array_4[0] = Encoders::findBase64Char(char_array_4[0]);
				char_array_4[1] = Encoders::findBase64Char(char_array_4[1]);
				char_array_4[2] = Encoders::findBase64Char(char_array_4[2]);
				char_array_4[3] = Encoders::findBase64Char(char_array_4[3]);

				char_array_3[0] = (char_array_4[0] << 2) + ((char_array_4[1] & 0x30) >> 4);
				char_array_3[1] = ((char_array_4[1] & 0xf) << 4) + ((char_array_4[2] & 0x3c) >> 2);
				char_array_3[2] = ((char_array_4[2] & 0x3) << 6) + char_array_4[3];

				memcpy(bufferOut, char_array_3, sizeof(unsigned char) * 3);
				bufferOut = reinterpret_cast<unsigned char*>(bufferOut)+3;
				bytesWritten += 3;

				i = 0;
			}
		}

		if (i)
		{
			for (size_t j = i; j < 4; j++)
				char_array_4[j] = 0;

			char_array_4[0] = Encoders::findBase64Char(char_array_4[0]);
			char_array_4[1] = Encoders::findBase64Char(char_array_4[1]);
			char_array_4[2] = Encoders::findBase64Char(char_array_4[2]);
			char_array_4[3] = Encoders::findBase64Char(char_array_4[3]);

			char_array_3[0] = (char_array_4[0] << 2) + ((char_array_4[1] & 0x30) >> 4);
			char_array_3[1] = ((char_array_4[1] & 0xf) << 4) + ((char_array_4[2] & 0x3c) >> 2);
			char_array_3[2] = ((char_array_4[2] & 0x3) << 6) + char_array_4[3];

			for (size_t j = 0; (j < i - 1); j++)
			{
				memcpy(bufferOut, char_array_3 + j, sizeof(unsigned char));
				bufferOut = reinterpret_cast<unsigned char*>(bufferOut);
				bytesWritten++;
			}
		}

		return bytesWritten;
	}

	size_t Encoders::decodeBase64(const std::string &dataBase64, std::vector<unsigned char> &bufferOut)
	{
		if (dataBase64.empty())
			return 0;

		bufferOut.reserve(bufferOut.capacity() + Encoders::decodeBase64RequiredSize(dataBase64.size()));

		size_t in_len = dataBase64.size();
		size_t i = 0;
		size_t in_ = 0;
		size_t bytesWritten = 0;
		unsigned char char_array_4[4], char_array_3[3];

		while (in_len-- && (dataBase64[in_] != '=') && Encoders::isBase64Char(dataBase64[in_]))
		{
			char_array_4[i++] = dataBase64[in_];
			in_++;
			if (i == 4)
			{
				char_array_4[0] = Encoders::findBase64Char(char_array_4[0]);
				char_array_4[1] = Encoders::findBase64Char(char_array_4[1]);
				char_array_4[2] = Encoders::findBase64Char(char_array_4[2]);
				char_array_4[3] = Encoders::findBase64Char(char_array_4[3]);

				char_array_3[0] = (char_array_4[0] << 2) + ((char_array_4[1] & 0x30) >> 4);
				char_array_3[1] = ((char_array_4[1] & 0xf) << 4) + ((char_array_4[2] & 0x3c) >> 2);
				char_array_3[2] = ((char_array_4[2] & 0x3) << 6) + char_array_4[3];

				bufferOut.push_back(char_array_3[0]);
				bufferOut.push_back(char_array_3[1]);
				bufferOut.push_back(char_array_3[2]);
				bytesWritten += 3;

				i = 0;
			}
		}

		if (i)
		{
			for (size_t j = i; j < 4; j++)
				char_array_4[j] = 0;

			char_array_4[0] = Encoders::findBase64Char(char_array_4[0]);
			char_array_4[1] = Encoders::findBase64Char(char_array_4[1]);
			char_array_4[2] = Encoders::findBase64Char(char_array_4[2]);
			char_array_4[3] = Encoders::findBase64Char(char_array_4[3]);

			char_array_3[0] = (char_array_4[0] << 2) + ((char_array_4[1] & 0x30) >> 4);
			char_array_3[1] = ((char_array_4[1] & 0xf) << 4) + ((char_array_4[2] & 0x3c) >> 2);
			char_array_3[2] = ((char_array_4[2] & 0x3) << 6) + char_array_4[3];

			for (size_t j = 0; (j < i - 1); j++)
			{
				bufferOut.push_back(char_array_3[0]);
				bytesWritten++;
			}
		}

		return bytesWritten;
	}

	size_t Encoders::decodeBase64(const std::string &dataBase64, HorseRadish::Streams::Stream &streamOut)
	{
		if (dataBase64.empty())
			return 0;

		size_t in_len = dataBase64.size();
		size_t i = 0;
		size_t in_ = 0;
		size_t bytesWritten = 0;
		unsigned char char_array_4[4], char_array_3[3];

		while (in_len-- && (dataBase64[in_] != '=') && Encoders::isBase64Char(dataBase64[in_]))
		{
			char_array_4[i++] = dataBase64[in_];
			in_++;
			if (i == 4)
			{
				char_array_4[0] = Encoders::findBase64Char(char_array_4[0]);
				char_array_4[1] = Encoders::findBase64Char(char_array_4[1]);
				char_array_4[2] = Encoders::findBase64Char(char_array_4[2]);
				char_array_4[3] = Encoders::findBase64Char(char_array_4[3]);

				char_array_3[0] = (char_array_4[0] << 2) + ((char_array_4[1] & 0x30) >> 4);
				char_array_3[1] = ((char_array_4[1] & 0xf) << 4) + ((char_array_4[2] & 0x3c) >> 2);
				char_array_3[2] = ((char_array_4[2] & 0x3) << 6) + char_array_4[3];

				streamOut.write(char_array_3, sizeof(unsigned char) * 3);
				bytesWritten += 3;

				i = 0;
			}
		}

		if (i)
		{
			for (size_t j = i; j < 4; j++)
				char_array_4[j] = 0;

			char_array_4[0] = Encoders::findBase64Char(char_array_4[0]);
			char_array_4[1] = Encoders::findBase64Char(char_array_4[1]);
			char_array_4[2] = Encoders::findBase64Char(char_array_4[2]);
			char_array_4[3] = Encoders::findBase64Char(char_array_4[3]);

			char_array_3[0] = (char_array_4[0] << 2) + ((char_array_4[1] & 0x30) >> 4);
			char_array_3[1] = ((char_array_4[1] & 0xf) << 4) + ((char_array_4[2] & 0x3c) >> 2);
			char_array_3[2] = ((char_array_4[2] & 0x3) << 6) + char_array_4[3];

			for (int j = 0; (j < i - 1); j++)
			{
				streamOut.write(char_array_3 + j, sizeof(unsigned char));
				bytesWritten++;
			}
		}

		return bytesWritten;
	}

	std::string Encoders::encodeHex(const void * const buffer, size_t bufferSize, bool toUppercase)
	{
		std::string stringOut;

		Encoders::encodeHex(buffer, bufferSize, toUppercase, stringOut);
		return stringOut;
	}

	void Encoders::encodeHex(const void * const buffer, size_t bufferSize, bool toUppercase, std::string &stringOut)
	{
		stringOut.reserve(stringOut.size() + ((bufferSize * 2) + 1));

		auto bufferHex = toUppercase ? Encoders::hexEncodeLookupUpper : Encoders::hexEncodeLookupLower;
		auto bufferWalker = reinterpret_cast<const unsigned char*>(buffer);

		for (size_t i = 0; i < bufferSize; i++, bufferWalker++)
		{
			auto hexPair = bufferHex + ((*bufferWalker) * 2);

			stringOut += hexPair[0];
			stringOut += hexPair[1];
		}
	}

	void Encoders::encodeHex(const void * const buffer, size_t bufferSize, bool toUppercase, HorseRadish::Streams::Stream &streamOut)
	{
		auto bufferHex = toUppercase ? Encoders::hexEncodeLookupUpper : Encoders::hexEncodeLookupLower;
		auto bufferWalker = reinterpret_cast<const unsigned char*>(buffer);

		for (size_t i = 0; i < bufferSize; i++, bufferWalker++)
		{
			auto hexPair = bufferHex + ((*bufferWalker) * 2);

			streamOut.write(hexPair, sizeof(unsigned char) * 2);
		}
	}

	void Encoders::encodeHexByte(const unsigned char valByte, char * const outHex)
	{
		auto hexPair = Encoders::hexEncodeLookupUpper + (valByte * 2);
		
		outHex[0] = hexPair[0];
		outHex[1] = hexPair[1];
	}

	size_t Encoders::decodeHexRequiredSize(size_t numHexChars)
	{
		return (numHexChars / 2);
	}

	size_t Encoders::decodeHex(const std::string &dataHex, HorseRadish::Streams::Stream &streamOut)
	{
		if (dataHex.empty() || ((dataHex.size() % 2) != 0))
			return 0;

		size_t inSize = dataHex.size();
		size_t bytesWritten = 0;
		auto *inWalker = reinterpret_cast<const unsigned char *>(dataHex.c_str());

		for (size_t i = 0; i < inSize; i += 2)
		{
			int valHex = Encoders::hexDecodeLookup[*inWalker++] << 4;
			valHex |= Encoders::hexDecodeLookup[*inWalker++];

			streamOut.write(&valHex, 1);
			bytesWritten++;
		}

		return bytesWritten;
	}

	unsigned char Encoders::decodeHexByte(const char * const dataHex)
	{
		return ((Encoders::hexDecodeLookup[dataHex[0]] << 4) | Encoders::hexDecodeLookup[dataHex[1]]);
	}

	const unsigned char Encoders::hexEncodeLookupLower[] = {
		"000102030405060708090a0b0c0d0e0f"
		"101112131415161718191a1b1c1d1e1f"
		"202122232425262728292a2b2c2d2e2f"
		"303132333435363738393a3b3c3d3e3f"
		"404142434445464748494a4b4c4d4e4f"
		"505152535455565758595a5b5c5d5e5f"
		"606162636465666768696a6b6c6d6e6f"
		"707172737475767778797a7b7c7d7e7f"
		"808182838485868788898a8b8c8d8e8f"
		"909192939495969798999a9b9c9d9e9f"
		"a0a1a2a3a4a5a6a7a8a9aaabacadaeaf"
		"b0b1b2b3b4b5b6b7b8b9babbbcbdbebf"
		"c0c1c2c3c4c5c6c7c8c9cacbcccdcecf"
		"d0d1d2d3d4d5d6d7d8d9dadbdcdddedf"
		"e0e1e2e3e4e5e6e7e8e9eaebecedeeef"
		"f0f1f2f3f4f5f6f7f8f9fafbfcfdfeff"
	};

	const unsigned char Encoders::hexEncodeLookupUpper[] = {
		"000102030405060708090A0B0C0D0E0F"
		"101112131415161718191A1B1C1D1E1F"
		"202122232425262728292A2B2C2D2E2F"
		"303132333435363738393A3B3C3D3E3F"
		"404142434445464748494A4B4C4D4E4F"
		"505152535455565758595A5B5C5D5E5F"
		"606162636465666768696A6B6C6D6E6F"
		"707172737475767778797A7B7C7D7E7F"
		"808182838485868788898A8B8C8D8E8F"
		"909192939495969798999A9B9C9D9E9F"
		"A0A1A2A3A4A5A6A7A8A9AAABACADAEAF"
		"B0B1B2B3B4B5B6B7B8B9BABBBCBDBEBF"
		"C0C1C2C3C4C5C6C7C8C9CACBCCCDCECF"
		"D0D1D2D3D4D5D6D7D8D9DADBDCDDDEDF"
		"E0E1E2E3E4E5E6E7E8E9EAEBECEDEEEF"
		"F0F1F2F3F4F5F6F7F8F9FAFBFCFDFEFF"
	};

	const unsigned char Encoders::hexDecodeLookup[] = {
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, // gap before first hex digit
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		0, 1, 2, 3, 4, 5, 6, 7, 8, 9,       // 0123456789
		0, 0, 0, 0, 0, 0, 0,             // :;<=>?@ (gap)
		10, 11, 12, 13, 14, 15,         // ABCDEF 
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, // GHIJKLMNOPQRS (gap)
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, // TUVWXYZ[/]^_` (gap)
		10, 11, 12, 13, 14, 15          // abcdef 
	};

} //HorseRadish