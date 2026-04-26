#include "encoders.hpp"

#include <array>

namespace hr
{
    namespace
    {
        constexpr const char base64Chars[] = {'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J', 'K', 'L', 'M', 'N', 'O', 'P', 'Q', 'R', 'S', 'T', 'U', 'V',
                                              'W', 'X', 'Y', 'Z', 'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i', 'j', 'k', 'l', 'm', 'n', 'o', 'p', 'q', 'r',
                                              's', 't', 'u', 'v', 'w', 'x', 'y', 'z', '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', '+', '/'};

        constexpr unsigned char hexEncodeLookupLower[] = {"000102030405060708090a0b0c0d0e0f"
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
                                                          "f0f1f2f3f4f5f6f7f8f9fafbfcfdfeff"};

        constexpr unsigned char hexEncodeLookupUpper[] = {"000102030405060708090A0B0C0D0E0F"
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
                                                          "F0F1F2F3F4F5F6F7F8F9FAFBFCFDFEFF"};

        constexpr unsigned char hexDecodeLookup[] = {
          0,  0,  0,  0,  0,  0,  0, 0, 0, 0, 0, 0, 0, 0, 0, 0,                                                                               // gap before first hex digit
          0,  0,  0,  0,  0,  0,  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, // 0123456789
          0,  0,  0,  0,  0,  0,  0,                                                                                                          // :;<=>?@ (gap)
          10, 11, 12, 13, 14, 15,                                                                                                             // ABCDEF
          0,  0,  0,  0,  0,  0,  0, 0, 0, 0, 0, 0, 0,                                                                                        // GHIJKLMNOPQRS (gap)
          0,  0,  0,  0,  0,  0,  0, 0, 0, 0, 0, 0, 0,                                                                                        // TUVWXYZ[/]^_` (gap)
          10, 11, 12, 13, 14, 15                                                                                                              // abcdef
        };

        constexpr bool isBase64Char(const char caracter)
        {
            return (((caracter >= '0') && (caracter <= '9')) || ((caracter >= 'A') && (caracter <= 'Z')) || ((caracter >= 'a') && (caracter <= 'z')) || (caracter == '+') ||
                    (caracter == '/'));
        }

        constexpr char findBase64Char(const char caracter)
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
            return static_cast<char>(-1);
        }
    }

    std::string Encoders::encodeBase64(std::span<const std::byte> buffer)
    {
        std::string stringOut;

        Encoders::encodeBase64(buffer, stringOut);
        return stringOut;
    }

    void Encoders::encodeBase64(std::span<const std::byte> buffer, std::string& stringOut)
    {
        stringOut.reserve(stringOut.size() + (4 * (buffer.size() + 3) / 3 + 2));

        size_t i = 0;
        std::array<unsigned char, 4> char_array_4;
        std::array<unsigned char, 3> char_array_3;

        auto bufferSize = buffer.size();
        auto bufferWalker = reinterpret_cast<const unsigned char*>(buffer.data());

        while (bufferSize--)
        {
            char_array_3[i++] = *(bufferWalker++);
            if (i == 3)
            {
                char_array_4[0] = (char_array_3[0] & 0xfc) >> 2;
                char_array_4[1] = ((char_array_3[0] & 0x03) << 4) + ((char_array_3[1] & 0xf0) >> 4);
                char_array_4[2] = ((char_array_3[1] & 0x0f) << 2) + ((char_array_3[2] & 0xc0) >> 6);
                char_array_4[3] = char_array_3[2] & 0x3f;

                stringOut += base64Chars[char_array_4[0]];
                stringOut += base64Chars[char_array_4[1]];
                stringOut += base64Chars[char_array_4[2]];
                stringOut += base64Chars[char_array_4[3]];
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
                stringOut += base64Chars[char_array_4[j]];

            while ((i++ < 3))
                stringOut += '=';
        }
    }

    void Encoders::encodeBase64(std::span<const std::byte> buffer, hr::streams::Stream& streamOut)
    {
        size_t i = 0;
        std::array<unsigned char, 4> char_array_4;
        std::array<unsigned char, 3> char_array_3;

        auto bufferSize = buffer.size();
        auto bufferWalker = reinterpret_cast<const unsigned char*>(buffer.data());

        hr::streams::StreamWriter streamWriter(streamOut);

        while (bufferSize--)
        {
            char_array_3[i++] = *(bufferWalker++);
            if (i == 3)
            {
                char_array_4[0] = (char_array_3[0] & 0xfc) >> 2;
                char_array_4[1] = ((char_array_3[0] & 0x03) << 4) + ((char_array_3[1] & 0xf0) >> 4);
                char_array_4[2] = ((char_array_3[1] & 0x0f) << 2) + ((char_array_3[2] & 0xc0) >> 6);
                char_array_4[3] = char_array_3[2] & 0x3f;

                streamWriter.write(base64Chars + char_array_4[0]);
                streamWriter.write(base64Chars + char_array_4[1]);
                streamWriter.write(base64Chars + char_array_4[2]);
                streamWriter.write(base64Chars + char_array_4[3]);
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
                streamWriter.write(base64Chars + char_array_4[j]);

            char endChar = '=';
            while ((i++ < 3))
                streamWriter.write(&endChar);
        }
    }

    size_t Encoders::decodeBase64RequiredSize(size_t numBase64Chars)
    {
        // base64 takes 137% of the original size, so we must divide by 1.37 or simply multiply by 0.73 which can be approximated by 3/4
        return (3 * numBase64Chars / 4);
    }

    size_t Encoders::decodeBase64(std::string_view dataBase64, std::span<std::byte> bufferOut)
    {
        if (dataBase64.empty())
            return 0;

        return Encoders::decodeBase64({dataBase64.data(), dataBase64.size()}, bufferOut);
    }

    size_t Encoders::decodeBase64(std::string_view dataBase64, std::vector<unsigned char>& bufferOut)
    {
        if (dataBase64.empty())
            return 0;

        return Encoders::decodeBase64({dataBase64.data(), dataBase64.size()}, bufferOut);
    }

    size_t Encoders::decodeBase64(std::string_view dataBase64, hr::streams::Stream& streamOut)
    {
        if (dataBase64.empty())
            return 0;

        return Encoders::decodeBase64({dataBase64.data(), dataBase64.size()}, streamOut);
    }

    size_t Encoders::decodeBase64(std::span<const std::byte> dataBase64, std::span<std::byte> bufferOut)
    {
        if (dataBase64.empty())
            return 0;

        size_t in_len = dataBase64.size();
        size_t i = 0;
        size_t in_ = 0;
        size_t bytesWritten = 0;
        std::array<unsigned char, 4> char_array_4;
        std::array<unsigned char, 3> char_array_3;

        auto data = reinterpret_cast<const unsigned char*>(dataBase64.data());

        while (in_len-- && (data[in_] != '=') && isBase64Char(data[in_]) && !bufferOut.empty())
        {
            char_array_4[i++] = data[in_];
            in_++;
            if (i == 4)
            {
                char_array_4[0] = findBase64Char(char_array_4[0]);
                char_array_4[1] = findBase64Char(char_array_4[1]);
                char_array_4[2] = findBase64Char(char_array_4[2]);
                char_array_4[3] = findBase64Char(char_array_4[3]);

                char_array_3[0] = (char_array_4[0] << 2) + ((char_array_4[1] & 0x30) >> 4);
                char_array_3[1] = ((char_array_4[1] & 0xf) << 4) + ((char_array_4[2] & 0x3c) >> 2);
                char_array_3[2] = ((char_array_4[2] & 0x3) << 6) + char_array_4[3];

                std::memcpy(bufferOut.data(), char_array_3.data(), char_array_3.size());
                bufferOut = bufferOut.subspan(3);
                bytesWritten += 3;

                i = 0;
            }
        }

        if (i)
        {
            for (size_t j = i; j < 4; j++)
                char_array_4[j] = 0;

            char_array_4[0] = findBase64Char(char_array_4[0]);
            char_array_4[1] = findBase64Char(char_array_4[1]);
            char_array_4[2] = findBase64Char(char_array_4[2]);
            char_array_4[3] = findBase64Char(char_array_4[3]);

            char_array_3[0] = (char_array_4[0] << 2) + ((char_array_4[1] & 0x30) >> 4);
            char_array_3[1] = ((char_array_4[1] & 0xf) << 4) + ((char_array_4[2] & 0x3c) >> 2);
            char_array_3[2] = ((char_array_4[2] & 0x3) << 6) + char_array_4[3];

            for (size_t j = 0; (j < i - 1); j++)
            {
                bufferOut[0] = static_cast<std::byte>(char_array_3[j]);
                bufferOut = bufferOut.subspan(1);
                bytesWritten++;
            }
        }

        return bytesWritten;
    }

    size_t Encoders::decodeBase64(std::span<const std::byte> dataBase64, std::vector<unsigned char>& bufferOut)
    {
        if (dataBase64.empty())
            return 0;

        bufferOut.reserve(bufferOut.capacity() + Encoders::decodeBase64RequiredSize(dataBase64.size()));

        size_t in_len = dataBase64.size();
        size_t i = 0;
        size_t in_ = 0;
        size_t bytesWritten = 0;
        std::array<unsigned char, 4> char_array_4;
        std::array<unsigned char, 3> char_array_3;

        auto data = reinterpret_cast<const unsigned char*>(dataBase64.data());

        while (in_len-- && (data[in_] != '=') && isBase64Char(data[in_]))
        {
            char_array_4[i++] = data[in_];
            in_++;
            if (i == 4)
            {
                char_array_4[0] = findBase64Char(char_array_4[0]);
                char_array_4[1] = findBase64Char(char_array_4[1]);
                char_array_4[2] = findBase64Char(char_array_4[2]);
                char_array_4[3] = findBase64Char(char_array_4[3]);

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

            char_array_4[0] = findBase64Char(char_array_4[0]);
            char_array_4[1] = findBase64Char(char_array_4[1]);
            char_array_4[2] = findBase64Char(char_array_4[2]);
            char_array_4[3] = findBase64Char(char_array_4[3]);

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

    size_t Encoders::decodeBase64(std::span<const std::byte> dataBase64, hr::streams::Stream& streamOut)
    {
        if (dataBase64.empty())
            return 0;

        size_t in_len = dataBase64.size();
        size_t i = 0;
        size_t in_ = 0;
        size_t bytesWritten = 0;
        std::array<unsigned char, 4> char_array_4;
        std::array<unsigned char, 3> char_array_3;

        hr::streams::StreamWriter streamWriter(streamOut);
        auto data = reinterpret_cast<const unsigned char*>(dataBase64.data());

        while (in_len-- && (data[in_] != '=') && isBase64Char(data[in_]))
        {
            char_array_4[i++] = data[in_];
            in_++;
            if (i == 4)
            {
                char_array_4[0] = findBase64Char(char_array_4[0]);
                char_array_4[1] = findBase64Char(char_array_4[1]);
                char_array_4[2] = findBase64Char(char_array_4[2]);
                char_array_4[3] = findBase64Char(char_array_4[3]);

                char_array_3[0] = (char_array_4[0] << 2) + ((char_array_4[1] & 0x30) >> 4);
                char_array_3[1] = ((char_array_4[1] & 0xf) << 4) + ((char_array_4[2] & 0x3c) >> 2);
                char_array_3[2] = ((char_array_4[2] & 0x3) << 6) + char_array_4[3];

                streamWriter.write<unsigned char, 3>(char_array_3);
                bytesWritten += 3;

                i = 0;
            }
        }

        if (i)
        {
            for (size_t j = i; j < 4; j++)
                char_array_4[j] = 0;

            char_array_4[0] = findBase64Char(char_array_4[0]);
            char_array_4[1] = findBase64Char(char_array_4[1]);
            char_array_4[2] = findBase64Char(char_array_4[2]);
            char_array_4[3] = findBase64Char(char_array_4[3]);

            char_array_3[0] = (char_array_4[0] << 2) + ((char_array_4[1] & 0x30) >> 4);
            char_array_3[1] = ((char_array_4[1] & 0xf) << 4) + ((char_array_4[2] & 0x3c) >> 2);
            char_array_3[2] = ((char_array_4[2] & 0x3) << 6) + char_array_4[3];

            for (size_t j = 0; (j < i - 1); j++)
            {
                streamWriter.write(char_array_3[j]);
                bytesWritten++;
            }
        }

        return bytesWritten;
    }

    std::string Encoders::encodeHex(std::span<const std::byte> buffer, bool toUppercase)
    {
        std::string stringOut;

        Encoders::encodeHex(buffer, toUppercase, stringOut);
        return stringOut;
    }

    void Encoders::encodeHex(std::span<const std::byte> buffer, bool toUppercase, std::string& stringOut)
    {
        stringOut.reserve(stringOut.size() + ((buffer.size() * 2) + 1));

        auto bufferHex = toUppercase ? hexEncodeLookupUpper : hexEncodeLookupLower;
        auto bufferWalker = reinterpret_cast<const unsigned char*>(buffer.data());

        for (size_t i = 0; i < buffer.size(); i++, bufferWalker++)
        {
            auto hexPair = bufferHex + ((*bufferWalker) * 2);

            stringOut += hexPair[0];
            stringOut += hexPair[1];
        }
    }

    void Encoders::encodeHex(std::span<const std::byte> buffer, bool toUppercase, hr::streams::Stream& streamOut)
    {
        auto bufferHex = toUppercase ? hexEncodeLookupUpper : hexEncodeLookupLower;
        auto bufferWalker = reinterpret_cast<const unsigned char*>(buffer.data());

        hr::streams::StreamWriter streamWriter(streamOut);

        for (size_t i = 0; i < buffer.size(); i++, bufferWalker++)
        {
            auto hexPair = bufferHex + ((*bufferWalker) * 2);

            streamWriter.write(std::span{hexPair, 2});
        }
    }

    void Encoders::encodeHexByte(const unsigned char valByte, char* const outHex)
    {
        auto hexPair = hexEncodeLookupUpper + (valByte * 2);

        outHex[0] = hexPair[0];
        outHex[1] = hexPair[1];
    }

    size_t Encoders::decodeHexRequiredSize(size_t numHexChars)
    {
        return (numHexChars / 2);
    }

    size_t Encoders::decodeHex(std::string_view dataHex, hr::streams::Stream& streamOut)
    {
        if (dataHex.empty() || ((dataHex.size() % 2) != 0))
            return 0;

        size_t inSize = dataHex.size();
        size_t bytesWritten = 0;
        auto* inWalker = reinterpret_cast<const unsigned char*>(dataHex.data());

        hr::streams::StreamWriter streamWriter(streamOut);

        for (size_t i = 0; i < inSize; i += 2)
        {
            int valHex = hexDecodeLookup[*inWalker++] << 4;
            valHex |= hexDecodeLookup[*inWalker++];

            streamWriter.write(valHex);
            bytesWritten++;
        }

        return bytesWritten;
    }

    unsigned char Encoders::decodeHexByte(std::string_view dataHex)
    {
        if (dataHex.size() < 2)
            return 0;

        return ((hexDecodeLookup[dataHex[0]] << 4) | hexDecodeLookup[dataHex[1]]);
    }
}
