#pragma once

#include "types.hpp"

namespace hr
{
	class Hashing
	{
	public:
		static uint8_t calculateCRC8(const void * const buffer, size_t bufferSize) noexcept;
		static uint16_t calculateCRC16(const void * const buffer, size_t bufferSize) noexcept;
		static uint32_t calculateCRC32(const void * const buffer, size_t bufferSize) noexcept;
		
		static void calculateMD5(const void * const buffer, size_t bufferSize, types::hData128 * const hash) noexcept;

		static void calculateSHA256(const void * const buffer, size_t bufferSize, types::hData128 * const hash) noexcept;
		
		static uint32_t superFastHash(const void * const buffer, size_t bufferSize) noexcept;
	};
}