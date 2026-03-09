#pragma once

#include "stream.hpp"

#include <span>
#include <string>
#include <string_view>
#include <vector>

namespace hr
{
    class Encoders
    {
    public:
        static std::string encodeBase64(std::span<const std::byte> buffer);
        static void encodeBase64(std::span<const std::byte> buffer, std::string& stringOut);
        static void encodeBase64(std::span<const std::byte> buffer, hr::streams::Stream& streamOut);

        static size_t decodeBase64RequiredSize(size_t numBase64Chars);
        static size_t decodeBase64(std::string_view dataBase64, std::span<std::byte> bufferOut);
        static size_t decodeBase64(std::string_view dataBase64, std::vector<unsigned char>& bufferOut);
        static size_t decodeBase64(std::string_view dataBase64, hr::streams::Stream& streamOut);
        static size_t decodeBase64(std::span<const std::byte> dataBase64, std::span<std::byte> bufferOut);
        static size_t decodeBase64(std::span<const std::byte> dataBase64, std::vector<unsigned char>& bufferOut);
        static size_t decodeBase64(std::span<const std::byte> dataBase64, hr::streams::Stream& streamOut);

        static std::string encodeHex(std::span<const std::byte> buffer, bool toUppercase);
        static void encodeHex(std::span<const std::byte> buffer, bool toUppercase, std::string& stringOut);
        static void encodeHex(std::span<const std::byte> buffer, bool toUppercase, hr::streams::Stream& streamOut);
        static void encodeHexByte(const unsigned char valByte, char* const outHex);

        static size_t decodeHexRequiredSize(size_t numHexChars);
        static size_t decodeHex(std::string_view dataHex, hr::streams::Stream& streamOut);
        static unsigned char decodeHexByte(std::string_view dataHex);
    };
}
