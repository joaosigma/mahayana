#pragma once

#include "types.hpp"

#include <algorithm>
#include <cassert>
#include <cmath>
#include <cstdint>
#include <emmintrin.h>
#include <xmmintrin.h>

#include <limits>

#pragma warning(push)
#pragma warning(disable : 4800) // remove warning "forcing value to bool 'true' or 'false' (performance warning)"
#pragma warning(disable : 4018) // remove warning "signed/unsigned mismatch"

namespace hr
{
    class Math
    {
    public:
        class SIMD
        {
        public:
            static void mad(float* values, size_t numValues, float mulVal, float addVal);
            static void mad(double* values, size_t numValues, double mulVal, double addVal);
        };

        template<class T>
        static constexpr T Pi = T(3.1415926535897932385);
        template<class T>
        static constexpr T PiHalf = T(1.5707963267948966192); // PI / 2.0
        template<class T>
        static constexpr T E = T(2.71828182845904523536);
        template<class T>
        static constexpr T Sqrt2 = T(1.41421356237309504880); // SQRT(2.0)
        template<class T>
        static constexpr T Sqrt3 = T(1.73205080756887729352); // SQRT(3.0)
        template<class T>
        static constexpr T Deg2Rad = T(0.017453292519943295769);
        template<class T>
        static constexpr T Rad2Deg = T(57.29577951308232087679);
        template<class T>
        static constexpr T UByteMax = T(255.0);
        template<class T>
        static constexpr T UByteMaxInv = T(0.003921568627450980392); // 1.0 / 255.0

        static float sqrt(const float x)
        {
            return _mm_cvtss_f32(_mm_sqrt_ss(_mm_set_ss(x)));
        }

        static float sqrtInv(const float x)
        {
            return _mm_cvtss_f32(_mm_rsqrt_ss(_mm_set_ss(x)));
        }

        static double sqrt(const double x)
        {
            auto temp = _mm_set1_pd(x);
            return _mm_cvtsd_f64(_mm_sqrt_sd(temp, temp));
        }

        static double sqrtInv(const double x)
        {
            /* AVX512
            auto temp = _mm_set1_pd(x);
            return _mm_cvtsd_f64(_mm_rsqrt28_sd(temp, temp));
            */

            return 1.0 / std::sqrt(x);
        }

        static float sin(const float radians)
        {
            return Math::cos(Math::PiHalf<float> - radians);
        }

        static float cos(const float radians)
        {
            return std::cos(radians);
        }

        static double sin(const double radians)
        {
            return Math::cos(Math::PiHalf<float> - radians);
        }

        static double cos(const double radians)
        {
            return std::cos(radians);
        }

        static void sinCos(float radians, float& s, float& c)
        {
            __m128 mmCos;
            __m128 mmSin = _mm_sincos_ps(&mmCos, _mm_set1_ps(radians));
            s = _mm_cvtss_f32(mmSin);
            c = _mm_cvtss_f32(mmCos);
        }

        static void sinCos(double radians, double& s, double& c)
        {
            __m128d mmCos;
            __m128d mmSin = _mm_sincos_pd(&mmCos, _mm_set1_pd(radians));
            s = _mm_cvtsd_f64(mmSin);
            c = _mm_cvtsd_f64(mmCos);
        }

        static std::tuple<float, float> sinCos(float radians)
        {
            float s, c;
            Math::sinCos(radians, s, c);
            return {s, c};
        }

        static std::tuple<double, double> sinCos(double radians)
        {
            double s, c;
            Math::sinCos(radians, s, c);
            return {s, c};
        }

        static float floor(const float val)
        {
            return std::floor(val);
        }

        static double floor(const double val)
        {
            return std::floor(val);
        }

        static float ceil(const float val)
        {
            return std::ceil(val);
        }

        static double ceil(const double val)
        {
            return std::ceil(val);
        }

        static float nearestInt(const float val)
        {
            return std::floor(val + 0.5f);
        }

        static double nearestInt(const double val)
        {
            return std::floor(val + 0.5);
        }

        template<typename T>
        static T ftoi(const float val)
        {
            static_assert(std::is_integral_v<T>, "Target must must be either [u]int32_t or [u]int64_t");

            /* AVX512
            if constexpr(std::is_same_v<T, uint32_t>)
                return _mm_cvtss_u32(_mm_set_ss(val));
            */
            if constexpr (std::is_same_v<T, int32_t>)
                return _mm_cvtss_si32(_mm_set_ss(val));
#if defined(_M_X64)
            /* AVX512
            else if constexpr (std::is_same_v<T, uint64_t>)
                return _mm_cvtss_u64(_mm_set_ss(val));
            */
            else if constexpr (std::is_same_v<T, int64_t>)
                return _mm_cvtss_si64(_mm_set_ss(val));
#endif

            static_assert(std::is_same_v<T, uint32_t> || std::is_same_v<T, int32_t> || std::is_same_v<T, uint64_t> || std::is_same_v<T, int64_t>);
        }
        static int64_t ftoi(const double val)
        {
            /* AVX512
            return _mm_cvtsd_i64(_mm_set_sd(val));
            */

            return static_cast<int64_t>(std::llround(val));
        }

        static bool isZero(const double d)
        {
            return (std::abs(d) <= std::numeric_limits<double>::epsilon());
        }

        static int iPrevPowerOfTwo(const int x)
        {
            // std::bit_floor
            return (iProxPowerOfTwo(x) >> 1);
        }

        static int iProxPowerOfTwo(const int& x)
        {
            // std::bit_ceil
            int in;

            in = x - 1;
            in |= in >> 16;
            in |= in >> 8;
            in |= in >> 4;
            in |= in >> 2;
            in |= in >> 1;
            return (in + 1);
        }

        static float fClamp(const float val, const float min, const float max)
        {
            return _mm_cvtss_f32(_mm_min_ss(_mm_max_ss(_mm_load_ss(&val), _mm_load_ss(&min)), _mm_load_ss(&max)));
        }

        static double fClamp(const double val, const double min, const double max)
        {
            return _mm_cvtsd_f64(_mm_min_sd(_mm_max_sd(_mm_load_sd(&val), _mm_load_sd(&min)), _mm_load_sd(&max)));
        }

        static bool fAlmostEqual(const float a, const float b, int maxUlps = 4)
        {
            // make sure maxUlps is non-negative and small enough that the default NAN won't compare as equal to anything.
            assert(maxUlps > 0 && maxUlps < 4 * 1024 * 1024);

            // make aInt lexicographically ordered as a twos-complement int
            int aInt = *(int*)&a;
            if (aInt < 0)
                aInt = 0x80000000 - aInt;

            // make bInt lexicographically ordered as a twos-complement int
            int bInt = *(int*)&b;
            if (bInt < 0)
                bInt = 0x80000000 - bInt;

            int intDiff = abs(aInt - bInt);
            return (intDiff <= maxUlps);
        }
    };

}

#pragma warning(pop)
