#pragma once

#include "Types.hpp"

#include <xmmintrin.h>
#include <emmintrin.h>
#include <stdlib.h>
#include <math.h>

#pragma warning( push )
#pragma warning( disable : 4800 )	//remove warning "forcing value to bool 'true' or 'false' (performance warning)"
#pragma warning( disable : 4018 )	//remove warning "signed/unsigned mismatch"

#undef INFINITY

namespace HorseRadish
{
	class Math
	{
	public:

		class SIMD
		{
		public:
			static HALIGN_16BYTES const __m128 fOne;
			static HALIGN_16BYTES const __m128 fHalfOne;
			static HALIGN_16BYTES const __m128 fPi;
			static HALIGN_16BYTES const __m128 fDeg2Rad;
			static HALIGN_16BYTES const __m128 fRad2Deg;
			static HALIGN_16BYTES const __m128 fUByteMax;
			static HALIGN_16BYTES const __m128 fUByteMaxInv;

			static void sinCosG(__m128 degrees, __m128 * const s, __m128 * const c);
			static void sinCosR(__m128 radians, __m128 * const s, __m128 * const c);
			static __m128 log(__m128 x);
			static __m128 exp(__m128 x);
		};

		static const float PI;
		static const float TWO_PI;
		static const float HALF_PI;
		static const float ONEFOURTH_PI;
		static const float E;
		static const float SQRT_TWO;
		static const float SQRT_THREE;
		static const float SQRT_1OVER2;
		static const float SQRT_1OVER3;
		static const float DEG2RAD;
		static const float RAD2DEG;
		static const float INFINITY;
		static const float EPSILON;

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
		static float				sinR(const float &radians);
		static float				sinG(const float &degrees);
		static float				cosR(const float &radians);
		static float				cosG(const float &degrees);
		static void					sinCosR(const float radians, float &s, float &c);
		static void					sinCosR(const float radians, float &s, float &c, const float mulResult);
		static void					sinCosG(const float degrees, float &s, float &c);
		static void					normalize(float * const vector);
		static float				normalizeRet(float * const vector);
		static float				angleNormalize360(const float &angle);
		static float				angleNormalize180(const float &angle);
		static float				angleDelta(const float &angle1, const float &angle2);
		static float				floor(const float &f);
		static float				ceil(const float &f);
		static float				nearestInt(const float &f);
		static int					ftoi(const float &f);
		static bool					isZero(const float &f);
		static bool					isZero(const float &f, const float &epsilon);
		static void					roundCloseZero(float *f);
		static void					roundCloseZero(float *f, const float &epsilon);
		static float				htof(const unsigned short &val);

		static int					iPow(int base, int exp);
		static unsigned int			iPow2(const unsigned int &exp);
		static int					iLog2(const float &f);
		static int					iLog2(const int &i);
		static int					iPrevPowerOfTwo(const int &x);
		static int					iProxPowerOfTwo(const int &x);
		static bool					iIsPowerOfTwo(const int &x);
		static int					iBitCount(const int &x);
		static int					iBitReverse(const int &x);
		static bool					iHasZero(const int &x);
		static bool					iMayHaveZero(const int &x);
		static int					iMax(const int &x, const int &y);
		static int					iMin(const int &x, const int &y);
		static int					iClamp(const int &val, const int &min, const int &max);
		static int					iClampZero(const int &val);
		static int					iClampZero(const int &val, const int &max);
		static int					iAbs(const int &x);

		static float				fPow(const float &x, const float &y);
		static float				fExp(const float &f);
		static float				fLog(const float &f);
		static float				fClamp(const float val, const float min, const float max);
		static float				fCycle(const float val, const float min, const float max);
		static float				fAbs(const float &f);
		static float				fMax(const float x, const float y, const float z);
		static float				fMax(const float x, const float y);
		static float				fMin(const float x, const float y, const float z);
		static float				fMin(const float x, const float y);
		static void					fMinMax(float *inOutMin, float *inOutMax, const float x);
		static bool					fIsNaN(const float &x);
		static bool					fIsInf(const float &x);
		static bool					fIsInd(const float &x);
		static bool					fIsDenormal(const float &x);
		static unsigned short		ftoh(const float &val);
		static void					fMAD(float *buf, const unsigned int num, const float mulVal, const float addVal);

		static float WaveEvalLinear(const float * const items, const int numItems, const float t);
		static float WaveEvalSnap(const float * const items, const int numItems, const float t);

		static void	EvalCatmullRom(const float * const p1, const float * const p2, const float * const p3, const float * const p4, const float t, float *output);
		static void	EvalHermite(const float * const p1, const float * const p2, const float * const p3, const float * const p4, const float t, float *output);
	};

	inline
		float Math::sqrt(const float &x)
	{
		__m128 auxVal;
		float retVal;

		auxVal = _mm_load_ss(&x);
		_mm_store_ss(&retVal, _mm_sqrt_ss(auxVal));
		return retVal;
	}

	inline
		float Math::sqrtInv(const float &x)
	{
		__m128 auxVal;
		float retVal;

		auxVal = _mm_load_ss(&x);
		_mm_store_ss(&retVal, _mm_rsqrt_ss(auxVal));
		return retVal;
	}

	inline
		float Math::sinR(const float &radians)
	{
		return sinf(radians);
	}

	inline
		float Math::sinG(const float &degrees)
	{
		return sinf(degrees * 0.01745329251994329576923690f);
	}

	inline
		float Math::cosR(const float &radians)
	{
		return cosf(radians);
	}

	inline
		float Math::cosG(const float &degrees)
	{
		return cosf(degrees * 0.01745329251994329576923690f);
	}

	inline
		void Math::sinCosR(const float radians, float &s, float &c)
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

	inline
		void Math::sinCosR(const float radians, float &s, float &c, const float mulResult)
	{
#ifdef _M_X64
		__m128 sin, cos;
		SIMD::sinCosR(_mm_load_ps1(&radians), &sin, &cos);
		s = sin.m128_f32[0] * mulResult;
		c = cos.m128_f32[0] * mulResult;
#else
		_asm 
		{
			fld		radians
			fsincos
			fld		mulResult
			fmul	st(2), st(0)
			fmulp	st(1), st(0)
			mov		ecx, c
			mov		edx, s
			fstp	dword ptr[ecx]
			fstp	dword ptr[edx]
		}
#endif
	}

	inline
		void Math::sinCosG(const float degrees, float &s, float &c)
	{
#ifdef _M_X64
		__m128 sin, cos;
		SIMD::sinCosG(_mm_load_ps1(&degrees), &sin, &cos);
		s = sin.m128_f32[0];
		c = cos.m128_f32[0];
#else
		_asm
		{
			fld		degrees
			fmul	Math::DEG2RAD
			fsincos
			mov		ecx, c
			mov		edx, s
			fstp	dword ptr[ecx]
			fstp	dword ptr[edx]
		}
#endif
	}

	inline
		void Math::normalize(float * const vector)
	{
		__m128 val1, val2, val3, val4;

		val1 = _mm_load_ss(vector + 0);
		val2 = _mm_load_ss(vector + 1);
		val3 = _mm_load_ss(vector + 2);
		val4 = _mm_add_ss(_mm_mul_ss(val1, val1), _mm_mul_ss(val2, val2));
		val4 = _mm_rsqrt_ss(_mm_add_ss(val4, _mm_mul_ss(val3, val3)));

		_mm_store_ss(vector + 0, _mm_mul_ss(val1, val4));
		_mm_store_ss(vector + 1, _mm_mul_ss(val2, val4));
		_mm_store_ss(vector + 2, _mm_mul_ss(val3, val4));
	}

	inline
		float Math::normalizeRet(float * const vector)
	{
#ifdef _M_X64
		float tam = (vector[0]*vector[0]) + (vector[1]*vector[1]) + (vector[2]*vector[2]);
		float tamInv = 1.0f / sqrt(tam);

		vector[0] *= tamInv;
		vector[1] *= tamInv;
		vector[2] *= tamInv;
		return tam;	
#else
		float veclength;

		__asm
		{
			mov		ecx, vector

			fld[ecx]
			fld[ecx + 4]
			fld[ecx + 8]

			fld		st(2)
			fmul	st(0), st(3)
			fld		st(2)
			fmul	st(0), st(3)
			fld		st(2)
			fmul	st(0), st(3)

			faddp	st(1), st(0)
			faddp	st(1), st(0)
			fst		veclength
			fsqrt
			fld1
			fdivrp	st(1), st(0)

			fmul	st(3), st(0)
			fmul	st(2), st(0)
			fmulp	st(1), st(0)

			fstp[ecx + 8]
			fstp[ecx + 4]
			fstp[ecx]
		}

		return veclength;
#endif
	}

	inline
		float Math::angleNormalize360(const float &angle)
	{
		if ((angle >= 360.0f) || (angle < 0.0f))
			return (angle - (floor(angle * 0.002777777777778f) * 360.0f));
		return angle;
	}

	inline
		float Math::angleNormalize180(const float &angle)
	{
		float angle360 = angleNormalize360(angle);
		return ((angle360>180.0f) ? (angle360 - 360.0f) : angle360);
	}

	inline
		float Math::angleDelta(const float &angle1, const float &angle2)
	{
		return angleNormalize180(angle1 - angle2);
	}

	inline
		float Math::floor(const float &f)
	{
		return floorf(f);
	}

	inline
		float Math::ceil(const float &f)
	{
		return ceilf(f);
	}

	inline
		float Math::nearestInt(const float &f)
	{
		return floorf(f + 0.5f);
	}

	inline
		int Math::ftoi(const float &f)
	{
		return _mm_cvtss_si32(_mm_load_ss(&f));
	}

	inline
		bool Math::isZero(const float &f)
	{
		unsigned int intAux = ((unsigned int&)f) & 0x7fffffff;
		return (((float&)(intAux)) <= Math::EPSILON);
	}

	inline
		bool Math::isZero(const float &f, const float &epsilon)
	{
		unsigned int intAux = ((unsigned int&)f) & 0x7fffffff;
		return (((float&)(intAux)) <= epsilon);
	}

	inline
		void Math::roundCloseZero(float *f)
	{
#ifdef _M_X64
		if (Math::isZero(*f))
			*f = 0.0f;	
#else
		unsigned int intAux;
		__asm
		{
			mov		eax, f
			mov		ecx, DWORD PTR[eax]
			and		ecx, 7FFFFFFFh
			mov		intAux, ecx

			fldz
			fld[intAux]
			fld[Math::EPSILON]

			fcomip	st(0), st(1)
			ffreep	st(0)
			fld[eax]
			fcmovnb	st(0), st(1)
			fstp[eax]
			ffreep	st(0)
		}
#endif
	}

	inline
		void Math::roundCloseZero(float *f, const float &epsilon)
	{
#ifdef _M_X64
		if (Math::isZero(*f, epsilon))
			*f = 0.0f;	
#else
		unsigned int intAux;
		__asm
		{
			mov		eax, f
			mov		ecx, DWORD PTR[eax]
			and		ecx, 7FFFFFFFh
			mov		intAux, ecx

			fldz
			fld[intAux]
			fld[epsilon]

			fcomip	st(0), st(1)
			ffreep	st(0)
			fld[eax]
			fcmovnb	st(0), st(1)
			fstp[eax]
			ffreep	st(0)
		}
#endif
	}

	inline
		int Math::iPow(int base, int exp)
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

	inline
		unsigned int Math::iPow2(const unsigned int &exp)
	{
		return (1 << exp);
	}

	inline
		int Math::iLog2(const float &f)
	{
		return ((((*((int *)&f)) >> 23) & ((1 << 8) - 1)) - 127);
	}

	inline
		int Math::iLog2(const int &i)
	{
		return iLog2((float)i);
	}

	inline
		int Math::iPrevPowerOfTwo(const int &x)
	{
		return (iProxPowerOfTwo(x) >> 1);
	}

	inline
		int Math::iProxPowerOfTwo(const int &x)
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

	inline
		bool Math::iIsPowerOfTwo(const int &x)
	{
		return ((x & (-x)) == x);
	}

	inline
		int Math::iBitCount(const int &x)
	{
		int a = x;
		a -= ((a >> 1) & 0x55555555);
		a = (((a >> 2) & 0x33333333) + (a & 0x33333333));
		a = (((a >> 4) + a) & 0x0f0f0f0f);
		a += (a >> 8);
		return ((a + (a >> 16)) & 0x0000003f);
	}

	inline
		int Math::iBitReverse(const int &x)
	{
		int a = x;
		a = (((a >> 1) & 0x55555555) | ((a & 0x55555555) << 1));
		a = (((a >> 2) & 0x33333333) | ((a & 0x33333333) << 2));
		a = (((a >> 4) & 0x0f0f0f0f) | ((a & 0x0f0f0f0f) << 4));
		a = (((a >> 8) & 0x00ff00ff) | ((a & 0x00ff00ff) << 8));
		return ((a >> 16) | (a << 16));
	}

	inline
		bool Math::iHasZero(const int &x)
	{
		return (~((((x & 0x7F7F7F7F) + 0x7F7F7F7F) | x) | 0x7F7F7F7F));
	}

	inline
		bool Math::iMayHaveZero(const int &x)
	{
		return (((x + 0x7efefeff) ^ ~x) & 0x81010100);
	}

	inline
		int Math::iMax(const int &x, const int &y)
	{
		int aux = x - y;
		return (x - (aux & (aux >> 31)));
	}

	inline
		int Math::iMin(const int &x, const int &y)
	{
		int aux = y - x;
		return (x + (aux & (aux >> 31)));
	}

	inline
		int Math::iClamp(const int &val, const int &min, const int &max)
	{
		return iMin(iMax(val, min), max);
	}

	inline
		int Math::iClampZero(const int &val)
	{
		return iMax(0, val);
	}

	inline
		int Math::iClampZero(const int &val, const int &max)
	{
		unsigned int inrangemask = 0xFFFFFFFF + (((unsigned)val) > max);
		unsigned int lessthan0mask = 0xFFFFFFFF + (val >= 0);
		int result = (inrangemask & val);
		return (result |= ((~inrangemask) & (~lessthan0mask)) & max);
	}

	inline
		int Math::iAbs(const int &x)
	{
		int y = x >> 31;
		return ((x ^ y) - y);
		//return x - ((x+x) & (x>>31));
	}

	inline
		float Math::fPow(const float &x, const float &y)
	{
		return powf(x, y);
	}

	inline
		float Math::fExp(const float &f)
	{
		return expf(f);
	}

	inline
		float Math::fLog(const float &f)
	{
		return logf(f);
	}

	inline
		float Math::fClamp(const float val, const float min, const float max)
	{
		float retVal;

		_mm_store_ss(&retVal, _mm_min_ss(_mm_max_ss(_mm_load_ss(&val), _mm_load_ss(&min)), _mm_load_ss(&max)));
		return retVal;
	}

	inline
		float Math::fCycle(const float val, const float min, const float max)
	{
#ifdef _M_X64
		if (val < min)
			return min;
		if (val > max)
			return max;
		return val;	
#else
		float result;
		__asm
		{
			fld[val]
			fld[max]
			fld[min]

			fcomi	st(0), st(2)
			fcmovnb	st(0), st(2)
			fstp	st(2)

			fcomi	st(0), st(1)
			fcmovb	st(0), st(1)

			fstp	result
			ffreep	st(0)
		}
		return result;
#endif
	}

	inline
		float Math::fAbs(const float &f)
	{
		unsigned int intAux = ((unsigned int&)f) & 0x7fffffff;
		return ((float&)(intAux));
	}

	inline
		float Math::fMax(const float x, const float y, const float z)
	{
		float result;
		__m128 auxVal;

		auxVal = _mm_max_ss(_mm_load_ss(&x), _mm_load_ss(&y));
		_mm_store_ss(&result, _mm_max_ss(auxVal, _mm_load_ss(&z)));
		return result;
	}

	inline
		float Math::fMax(const float x, const float y)
	{
		float result;

		_mm_store_ss(&result, _mm_max_ss(_mm_load_ss(&x), _mm_load_ss(&y)));
		return result;
	}

	inline
		float Math::fMin(const float x, const float y, const float z)
	{
		float result;
		__m128 auxVal;

		auxVal = _mm_min_ss(_mm_load_ss(&x), _mm_load_ss(&y));
		_mm_store_ss(&result, _mm_min_ss(auxVal, _mm_load_ss(&z)));
		return result;
	}

	inline
		float Math::fMin(const float x, const float y)
	{
		float result;

		_mm_store_ss(&result, _mm_min_ss(_mm_load_ss(&x), _mm_load_ss(&y)));
		return result;
	}

	inline
		void Math::fMinMax(float *inOutMin, float *inOutMax, const float x)
	{

#ifdef _M_X64
		if (x < inOutMin[0])
			inOutMin[0] = x;
		if (x > inOutMax[0])
			inOutMax[0] = x;
#else
		__asm
		{
			mov		eax, inOutMin
			mov		ecx, inOutMax

			fld[x]
			fld[eax]
			fld[ecx]

			fcomi	st(0), st(2)
			fcmovb	st(0), st(2)
			fstp[ecx]

			fcomi	st(0), st(1)
			fcmovnb	st(0), st(1)
			fstp[eax]
			ffreep	st(0)
		}
#endif
	}

	inline
		bool Math::fIsNaN(const float &x)
	{
		return (((*(const unsigned long *)&x) & 0x7f800000) == 0x7f800000);
	}

	inline
		bool Math::fIsInf(const float &x)
	{
		return (((*(const unsigned long *)&x) & 0x7fffffff) == 0x7f800000);
	}

	inline
		bool Math::fIsInd(const float &x)
	{
		return ((*(const unsigned long *)&x) == 0xffc00000);
	}

	inline
		bool Math::fIsDenormal(const float &x)
	{
		return (((*(const unsigned long *)&x) & 0x7f800000) == 0x00000000 && ((*(const unsigned long *)&x) & 0x007fffff) != 0x00000000);
	}

} //HorseRadish

#pragma warning( pop ) 
