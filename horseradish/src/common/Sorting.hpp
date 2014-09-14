#pragma once

#include "Platform.hpp"

#include "Types.hpp"

namespace HorseRadish
{
	class Sorting
	{
		static void radixByte0(const int &numero, const int * const HRESTRICT source, int * const HRESTRICT dest);
		static void radixByte1(const int &numero, const int * const HRESTRICT source, int * const HRESTRICT dest);
		static void radixByte2(const int &numero, const int * const HRESTRICT source, int * const HRESTRICT dest);
		static void radixByte3(const int &numero, const int * const HRESTRICT source, int * const HRESTRICT dest);

	public:

		static void RadixSort(int *baseArray, int *tempArray, const unsigned int numElements);
		static void RadixQueue(float *baseArray, float *tempArray, int *orderOut, int *orderTemp, const unsigned int numElements);
	};

} //HorseRadish
