module;

#include <cstdint>

export module core:hashing;

import std;
import :types;

export namespace hr
{
    class Hashing
    {
    public:
        static uint8_t crc8(std::span<const std::byte> buffer) noexcept;
        static uint16_t crc16(std::span<const std::byte> buffer) noexcept;
        static uint32_t crc32(std::span<const std::byte> buffer) noexcept;

        static uint32_t murmur32(std::span<const std::byte> buffer) noexcept;
        static types::hData128 murmur128(std::span<const std::byte> buffer) noexcept;
    };
}
