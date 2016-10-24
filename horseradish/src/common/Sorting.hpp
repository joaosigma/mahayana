#pragma once

#include "platform.hpp"

#include "types.hpp"

namespace HorseRadish
{
	class Sorting
	{
		static void radixByte0(size_t howMuch, const int * const HRESTRICT source, int * const HRESTRICT dest);
		static void radixByte1(size_t howMuch, const int * const HRESTRICT source, int * const HRESTRICT dest);
		static void radixByte2(size_t howMuch, const int * const HRESTRICT source, int * const HRESTRICT dest);
		static void radixByte3(size_t howMuch, const int * const HRESTRICT source, int * const HRESTRICT dest);

	public:
		static void radixSort(int *baseArray, int *tempArray, size_t numElements);
		static void radixQueue(float *baseArray, float *tempArray, int *orderOut, int *orderTemp, size_t numElements);
	};

} //HorseRadish
