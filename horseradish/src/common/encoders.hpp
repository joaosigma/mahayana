#pragma once

#include "stream.hpp"

#include <string>
#include <vector>

namespace hr
{
    class Encoders
    {
    public:
        static std::string encodeBase64(const void* const buffer, size_t bufferSize);
        static void encodeBase64(const void* const buffer, size_t bufferSize, std::string& stringOut);
        static void encodeBase64(const void* const buffer, size_t bufferSize, hr::streams::Stream& streamOut);

        static size_t decodeBase64RequiredSize(size_t numBase64Chars);
        static size_t decodeBase64(const std::string& dataBase64, void* bufferOut);
        static size_t decodeBase64(const std::string& dataBase64, std::vector<unsigned char>& bufferOut);
        static size_t decodeBase64(const std::string& dataBase64, hr::streams::Stream& streamOut);
        static size_t decodeBase64(const char* const dataBase64, size_t dataSize, void* bufferOut);
        static size_t decodeBase64(const char* const dataBase64, size_t dataSize, std::vector<unsigned char>& bufferOut);
        static size_t decodeBase64(const char* const dataBase64, size_t dataSize, hr::streams::Stream& streamOut);

        static std::string encodeHex(const void* const buffer, size_t bufferSize, bool toUppercase);
        static void encodeHex(const void* const buffer, size_t bufferSize, bool toUppercase, std::string& stringOut);
        static void encodeHex(const void* const buffer, size_t bufferSize, bool toUppercase, hr::streams::Stream& streamOut);
        static void encodeHexByte(const unsigned char valByte, char* const outHex);

        static size_t decodeHexRequiredSize(size_t numHexChars);
        static size_t decodeHex(const std::string& dataHex, hr::streams::Stream& streamOut);
        static unsigned char decodeHexByte(const char* const dataHex);
    };
}
