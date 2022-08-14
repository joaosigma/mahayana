#include "math.hpp"

namespace hr
{
	void Math::SIMD::mad(float *values, size_t numValues, float mulVal, float addVal)
	{
		__m128 do1, do2, do3, do4, mulReg, addReg;
		
		mulReg = _mm_set_ps1(mulVal);
		addReg = _mm_set_ps1(addVal);

		if (auto remain = reinterpret_cast<uintptr_t>(values) % 16; remain == 4 || remain == 8 || remain == 12 || remain == 0)
		{
			auto walker = values;

			if (remain != 0)
			{
				remain = (16 - remain) / 4;
				while (remain > 0)
				{
					*walker = (*walker) * mulVal + addVal;
					remain--;
					walker++;
				}
			}

			auto leftOver = numValues;
			for (; leftOver >= 16; leftOver -= 16, walker += 16)
			{
				do1 = _mm_load_ps(walker + 0);
				do2 = _mm_load_ps(walker + 4);
				do3 = _mm_load_ps(walker + 8);
				do4 = _mm_load_ps(walker + 12);

				do1 = _mm_add_ps(_mm_mul_ps(do1, mulReg), addReg);
				do2 = _mm_add_ps(_mm_mul_ps(do2, mulReg), addReg);
				do3 = _mm_add_ps(_mm_mul_ps(do3, mulReg), addReg);
				do4 = _mm_add_ps(_mm_mul_ps(do4, mulReg), addReg);

				_mm_store_ps(walker + 0, do1);
				_mm_store_ps(walker + 4, do2);
				_mm_store_ps(walker + 8, do3);
				_mm_store_ps(walker + 12, do4);
			}

			for (; leftOver > 0; leftOver--, walker++)
				*walker = (*walker)*mulVal + addVal;

			return;
		}
		else
		{
			for (; numValues >= 16; numValues -= 16, values += 16)
			{
				do1 = _mm_loadu_ps(values + 0);
				do2 = _mm_loadu_ps(values + 4);
				do3 = _mm_loadu_ps(values + 8);
				do4 = _mm_loadu_ps(values + 12);

				do1 = _mm_add_ps(_mm_mul_ps(do1, mulReg), addReg);
				do2 = _mm_add_ps(_mm_mul_ps(do2, mulReg), addReg);
				do3 = _mm_add_ps(_mm_mul_ps(do3, mulReg), addReg);
				do4 = _mm_add_ps(_mm_mul_ps(do4, mulReg), addReg);

				_mm_storeu_ps(values + 0, do1);
				_mm_storeu_ps(values + 4, do2);
				_mm_storeu_ps(values + 8, do3);
				_mm_storeu_ps(values + 12, do4);
			}

			for (; numValues > 0; numValues--, values++)
				*values = (*values) * mulVal + addVal;
		}
	}

	void Math::SIMD::mad(double* values, size_t numValues, double mulVal, double addVal)
	{

	}
}