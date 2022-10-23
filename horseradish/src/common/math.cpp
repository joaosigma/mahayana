#include "math.hpp"

namespace hr
{
	void Math::SIMD::mad(float *values, size_t numValues, float mulVal, float addVal)
	{
		static_assert((std::alignment_of_v<float> % 4) == 0);

		auto remain = reinterpret_cast<uintptr_t>(values) % 16;
		if (remain != 0)
		{
			remain = (16 - remain) / 4;
			while ((remain > 0) && (numValues > 0))
			{
				*values = (*values) * mulVal + addVal;
				remain--;
				numValues--;
				values++;
			}
		}

		if (numValues <= 0) return;
		
		assert((reinterpret_cast<uintptr_t>(values) % 16) == 0);

		__m128 do1, do2, do3, do4;
		__m128 mulReg = _mm_set_ps1(mulVal);
		__m128 addReg = _mm_set_ps1(addVal);
		for (; numValues >= 16; numValues -= 16, values += 16)
		{
			do1 = _mm_load_ps(values + 0);
			do2 = _mm_load_ps(values + 4);
			do3 = _mm_load_ps(values + 8);
			do4 = _mm_load_ps(values + 12);

			do1 = _mm_add_ps(_mm_mul_ps(do1, mulReg), addReg);
			do2 = _mm_add_ps(_mm_mul_ps(do2, mulReg), addReg);
			do3 = _mm_add_ps(_mm_mul_ps(do3, mulReg), addReg);
			do4 = _mm_add_ps(_mm_mul_ps(do4, mulReg), addReg);

			_mm_store_ps(values + 0, do1);
			_mm_store_ps(values + 4, do2);
			_mm_store_ps(values + 8, do3);
			_mm_store_ps(values + 12, do4);
		}

		for (; numValues > 0; numValues--, values++)
			*values = (*values) * mulVal + addVal;
	}

	void Math::SIMD::mad(double* values, size_t numValues, double mulVal, double addVal)
	{
		static_assert((std::alignment_of_v<double> % 8) == 0);

		auto remain = reinterpret_cast<uintptr_t>(values) % 32;
		if (remain != 0)
		{
			remain = (32 - remain) / 8;
			while ((remain > 0) && (numValues > 0))
			{
				*values = (*values) * mulVal + addVal;
				remain--;
				numValues--;
				values++;
			}
		}

		if (numValues <= 0) return;

		assert((reinterpret_cast<uintptr_t>(values) % 32) == 0);

		__m256d do1, do2, do3, do4;
		__m256d mulReg = _mm256_broadcast_sd(&mulVal);
		__m256d addReg = _mm256_broadcast_sd(&addVal);
		for (; numValues >= 16; numValues -= 16, values += 16)
		{
			do1 = _mm256_load_pd(values + 0);
			do2 = _mm256_load_pd(values + 4);
			do3 = _mm256_load_pd(values + 8);
			do4 = _mm256_load_pd(values + 12);

			do1 = _mm256_add_pd(_mm256_mul_pd(do1, mulReg), addReg);
			do2 = _mm256_add_pd(_mm256_mul_pd(do2, mulReg), addReg);
			do3 = _mm256_add_pd(_mm256_mul_pd(do3, mulReg), addReg);
			do4 = _mm256_add_pd(_mm256_mul_pd(do4, mulReg), addReg);

			_mm256_store_pd(values + 0, do1);
			_mm256_store_pd(values + 4, do2);
			_mm256_store_pd(values + 8, do3);
			_mm256_store_pd(values + 12, do4);
		}

		for (; numValues > 0; numValues--, values++)
			*values = (*values) * mulVal + addVal;
	}
}