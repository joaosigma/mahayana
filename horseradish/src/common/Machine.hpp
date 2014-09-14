#pragma once

#include "String.hpp"
#include "Types.hpp"

namespace HorseRadish
{

	class Machine
	{
	public:
		enum CPUFeature { SSE = (1 << 0), SSE2 = (1 << 1), HyperThreading = (1 << 2), CMov = (1 << 3) };

	private:
		Machine() { }

	public:

		static bool CPUGetVendorID(String &outputValue);
		static bool CPUGetProcessorName(String &outputValue);
		static bool CPUCheckFeatures(const CPUFeature &featuresCheck);

		static hUInt32 FastCompressGetSize(const hUInt32 uncompressedSize);
		static hUInt32 FastCompress(unsigned char * const dest, const unsigned char * const source, const hUInt32 sourceSize);
		static hUInt32 FastDecompress(unsigned char * const dest, const unsigned char * const source, const hUInt32 sourceSize);

		static void AsmBufferClear(void* dest, size_t bytes);
		static void AsmBufferCopy(void* dest, const void* src, size_t bytes);
		static void AsmBufferCopyAligned(void* dest, const void* src, size_t multiple128Bytes);
		static void AsmBufferSetUBYTE(void* dest, unsigned char val, size_t bytes);
		static void AsmBufferSetUI32(void* dest, unsigned int val, size_t bytes);
		static void AsmFloat2UByte(unsigned char *dest, const float *src, const unsigned int num, const float mulVal, const float addVal);
		static void AsmUByte2Float(float *dest, const unsigned char *src, const unsigned int num, const float mulVal, const float addVal);
	};

} //HorseRadish

