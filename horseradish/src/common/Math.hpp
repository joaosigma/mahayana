#pragma once

#include "Types.hpp"

#include <cmath>
#include <algorithm>
#include <xmmintrin.h>
#include <emmintrin.h>

#include <limits>

#pragma warning( push )
#pragma warning( disable : 4800 )	//remove warning "forcing value to bool 'true' or 'false' (performance warning)"
#pragma warning( disable : 4018 )	//remove warning "signed/unsigned mismatch"

namespace HorseRadish
{
	class Math
	{
	public:

		class SIMD
		{
		public:
			static const __m128 fOne;
			static const __m128 fHalfOne;
			static const __m128 fPi;
			static const __m128 fDeg2Rad;
			static const __m128 fRad2Deg;
			static const __m128 fUByteMax;
			static const __m128 fUByteMaxInv;

			static void sinCos(__m128 radians, __m128 * const s, __m128 * const c);
			static __m128 log(__m128 x);
			static __m128 exp(__m128 x);

			static void mad(float *values, size_t numValues, float mulVal, float addVal);
		};

		static constexpr float constPi() noexcept
		{
			return 3.14159265358979323846f;
		}

		static constexpr float constPiScaled(const float scaleValue) noexcept
		{
			return (scaleValue * 3.14159265358979323846f);
		}

		static constexpr float constE() noexcept
		{
			return 2.71828182845904523536f;
		}

		static constexpr float constSqrt2() noexcept
		{
			return 1.41421356237309504880f;
		}

		static constexpr float constSqrt3() noexcept
		{
			return 1.73205080756887729352f;
		}

		static constexpr float convDeg2Rad(float degrees) noexcept
		{
			return (degrees * 0.017453292519943295769f);
		}

		static constexpr float convRad2Deg(float radians) noexcept
		{
			return (radians * 57.29577951308232087679f);
		}

		static const float WaveTableSin[256];
		static const float WaveTableCos[256];
		static const float WaveTableBinary[2];
		static const float WaveTableScale[2];
		static const float WaveTablePulse[3];
		static const float WaveTableConvex[64];
		static const float WaveTableConcave[64];
		static const float WaveTableSOS[21];

		static float				sqrt(const float &x);
		static float				sqrtInv(const float &x);
		static float				sin(const float radians);
		static float				cos(const float radians);
		static void					sinCos(float radians, float &s, float &c);
		static void					sinCos(float radians, float &s, float &c, const float scaleResult);
		static float				floor(const float &f);
		static float				ceil(const float &f);
		static float				nearestInt(const float &f);
		static int					ftoi(const float &f);
		static bool					isZero(const float &f);

		static int					iPow(int base, int exp);
		static unsigned int			iPow2(const unsigned int &exp);
		static int					iLog2(const float &f);
		static int					iLog2(const int &i);
		static int					iLog2(const unsigned int &i);
		static int					iPrevPowerOfTwo(const int &x);
		static int					iProxPowerOfTwo(const int &x);
		static bool					iIsPowerOfTwo(const int &x);
		static int					iBitCount(const int &x);
		static int					iBitReverse(const int &x);
		static bool					iHasZero(const int &x);
		static bool					iMayHaveZero(const int &x);

		static float				fClamp(const float val, const float min, const float max);

		static float				htof(unsigned short val);
		static unsigned short		ftoh(float val);

		static float sampleWave(const float * const items, size_t numItems, float t);
	};

	inline float Math::sqrt(const float &x)
	{
		__m128 auxVal;
		float retVal;

		auxVal = _mm_load_ss(&x);
		_mm_store_ss(&retVal, _mm_sqrt_ss(auxVal));
		return retVal;
	}

	inline float Math::sqrtInv(const float &x)
	{
		__m128 auxVal;
		float retVal;

		auxVal = _mm_load_ss(&x);
		_mm_store_ss(&retVal, _mm_rsqrt_ss(auxVal));
		return retVal;
	}

	inline float Math::sin(const float radians)
	{
		return std::sin(radians);
	}

	inline float Math::cos(const float radians)
	{
		return std::cos(radians);
	}

	inline void Math::sinCos(float radians, float &s, float &c)
	{
#ifdef _M_X64
		__m128 sin, cos;
		SIMD::sinCosR(_mm_load_ps1(&radians), &sin, &cos);
		s = sin.m128_f32[0];
		c = cos.m128_f32[0];
#else
		_asm
		{
			fld		radians
			fsincos
			mov		ecx, c
			mov		edx, s
			fstp	dword ptr[ecx]
			fstp	dword ptr[edx]
		}
#endif
	}

	inline void Math::sinCos(float radians, float &s, float &c, const float scaleResult)
	{
#ifdef _M_X64
		__m128 sin, cos;
		SIMD::sinCosR(_mm_load_ps1(&radians), &sin, &cos);
		s = sin.m128_f32[0] * scaleResult;
		c = cos.m128_f32[0] * scaleResult;
#else
		_asm 
		{
			fld		radians
			fsincos
			fld		scaleResult
			fmul	st(2), st(0)
			fmulp	st(1), st(0)
			mov		ecx, c
			mov		edx, s
			fstp	dword ptr[ecx]
			fstp	dword ptr[edx]
		}
#endif
	}

	inline float Math::floor(const float &f)
	{
		return std::floor(f);
	}

	inline float Math::ceil(const float &f)
	{
		return std::ceil(f);
	}

	inline float Math::nearestInt(const float &f)
	{
		return std::floor(f + 0.5f);
	}

	inline int Math::ftoi(const float &f)
	{
		return _mm_cvtss_si32(_mm_load_ss(&f));
	}

	inline bool Math::isZero(const float &f)
	{
		return (std::abs(f) <= std::numeric_limits<float>::epsilon());
	}

	inline int Math::iPow(int base, int exp)
	{
		int result = 1;

		while (exp)
		{
			if (exp & 1)
				result *= base;
			exp >>= 1;
			base *= base;
		}

		return result;
	}

	inline unsigned int Math::iPow2(const unsigned int &exp)
	{
		return (1 << exp);
	}

	inline int Math::iLog2(const float &f)
	{
		return ((((*((int *)&f)) >> 23) & ((1 << 8) - 1)) - 127);
	}

	inline int Math::iLog2(const int &i)
	{
		return iLog2((float)i);
	}

	inline int Math::iLog2(const unsigned int &i)
	{
		return iLog2((float)i);
	}

	inline int Math::iPrevPowerOfTwo(const int &x)
	{
		return (iProxPowerOfTwo(x) >> 1);
	}

	inline int Math::iProxPowerOfTwo(const int &x)
	{
		int in;

		in = x - 1;
		in |= in >> 16;
		in |= in >> 8;
		in |= in >> 4;
		in |= in >> 2;
		in |= in >> 1;
		return (in + 1);
	}

	inline bool Math::iIsPowerOfTwo(const int &x)
	{
		return ((x & (-x)) == x);
	}

	inline int Math::iBitCount(const int &x)
	{
		int a = x;
		a -= ((a >> 1) & 0x55555555);
		a = (((a >> 2) & 0x33333333) + (a & 0x33333333));
		a = (((a >> 4) + a) & 0x0f0f0f0f);
		a += (a >> 8);
		return ((a + (a >> 16)) & 0x0000003f);
	}

	inline int Math::iBitReverse(const int &x)
	{
		int a = x;
		a = (((a >> 1) & 0x55555555) | ((a & 0x55555555) << 1));
		a = (((a >> 2) & 0x33333333) | ((a & 0x33333333) << 2));
		a = (((a >> 4) & 0x0f0f0f0f) | ((a & 0x0f0f0f0f) << 4));
		a = (((a >> 8) & 0x00ff00ff) | ((a & 0x00ff00ff) << 8));
		return ((a >> 16) | (a << 16));
	}

	inline bool Math::iHasZero(const int &x)
	{
		return (~((((x & 0x7F7F7F7F) + 0x7F7F7F7F) | x) | 0x7F7F7F7F));
	}

	inline bool Math::iMayHaveZero(const int &x)
	{
		return (((x + 0x7efefeff) ^ ~x) & 0x81010100);
	}

	inline float Math::fClamp(const float val, const float min, const float max)
	{
		float retVal;

		_mm_store_ss(&retVal, _mm_min_ss(_mm_max_ss(_mm_load_ss(&val), _mm_load_ss(&min)), _mm_load_ss(&max)));
		return retVal;
	}

}

#pragma warning( pop ) 
