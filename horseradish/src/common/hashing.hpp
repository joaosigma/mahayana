#pragma once

#include "types.hpp"

namespace hr
{
	class Hashing
	{
	public:
		static hUInt8 calculateCRC8(const void * const buffer, size_t bufferSize);
		static hUInt16 calculateCRC16(const void * const buffer, size_t bufferSize);
		static hUInt32 calculateCRC32(const void * const buffer, size_t bufferSize);
		
		static void calculateMD5(const void * const buffer, size_t bufferSize, hData128 * const hash);

		static void calculateSHA256(const void * const buffer, size_t bufferSize, hData128 * const hash);
		
		static hUInt32 superFastHash(const void * const buffer, size_t bufferSize);
	};
}