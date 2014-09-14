#pragma once

#include "Platform.hpp"
#include "Types.hpp"

namespace HorseRadish
{
	class Hashing
	{
		struct DataMD5{
			unsigned int state[4], bits[2];
			unsigned char in[64];
		};

		struct DataSHA1{
			unsigned int state[5];
			unsigned int count[2];
			unsigned char buffer[64];
		};

		static const unsigned char crctable8[256];
		static const unsigned short crctable16[256];
		static const unsigned long crctable32[256];

		static void initMD5(DataMD5 * const dataMD5);
		static void updateMD5(DataMD5 * const dataMD5, const unsigned char *buffer, unsigned int bufferSize);
		static void transformMD5(unsigned int state[4], unsigned int const in[16]);
		static void finishMD5(DataMD5 * const dataMD5, unsigned char digest[16]);

		static void transformSHA1(unsigned int state[5], const unsigned char buffer[64]);
		static void initSHA1(DataSHA1 * const context);
		static void updateSHA1(DataSHA1 * const context, const unsigned char* data, unsigned int len);
		static void finishSHA1(unsigned char digest[20], DataSHA1 * const context);

	public:
		static unsigned char CalculateCRC8(const void * const buffer, unsigned int bufferSize);
		static unsigned short CalculateCRC16(const void * const buffer, unsigned int bufferSize);
		static unsigned long CalculateCRC32(const void * const buffer, unsigned int bufferSize);
		static void CalculateMD5(const void * const buffer, const unsigned int bufferSize, hData128 * const hash);
		static void CalculateSHA1(const void * const buffer, const unsigned int bufferSize, hData160 * const hash);
		static unsigned long CalculateMD5Short(const void * const buffer, const unsigned int bufferSize);
		static unsigned int SuperFastHash(const void * const buffer, unsigned int bufferSize);
	};

} //HorseRadish