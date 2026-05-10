module;

#include <cassert>
#include <immintrin.h>
#include <smmintrin.h>
#include <xmmintrin.h>

export module core:vector;

import std;
import :math;

export namespace hr
{
    /**
     * All operations are irregardless of convention, with the exception of the cross-product. And like matrices and quaternions, they use a left-hand
     * convention. This means that positive rotation is clockwise about the axis of rotation (as looking to the origin of axis): +X points right, +Y
     * points up and +Z points forward (to the horizon).
     */
    template<typename TDataType, size_t NComponents>
    class Vector
    {
        TDataType mData[NComponents] = {};

        static_assert(NComponents >= 1, "Number of components must be equal or greater than 1");
        static_assert(std::is_arithmetic<TDataType>::value, "Data type must be arithmetic (e.g.: float, unsigned char, etc.)");
        static_assert(std::is_trivially_copyable_v<TDataType>, "For performance reasons, the data type should be trivially copyable");

    public:
        using DataType = TDataType;
        static constexpr size_t NumComponents = NComponents;

    public:
        static Vector zero() noexcept
        {
            Vector vec;
            for (size_t i = 0; i < NComponents; i++)
                vec.mData[i] = TDataType();
            return vec;
        }

    public:
        explicit Vector(const TDataType scalar) noexcept
        {
            for (size_t i = 0; i < NComponents; i++)
                mData[i] = scalar;
        }

        explicit Vector(std::span<const TDataType> values) noexcept
        {
            if (values.size() != NComponents)
            {
                for (size_t i = 0; i < NComponents; i++)
                    mData[i] = TDataType();
            }
            else
            {
                for (size_t i = 0; i < NComponents; i++)
                    mData[i] = values[i];
            }
        }

        template<typename Self>
        constexpr auto data(this Self&& self) noexcept
        {
            return std::span{self.mData, NComponents}.first<NComponents>();
        }

        template<typename Self>
        constexpr auto& operator[](this Self&& self, const size_t index) noexcept
        {
            return self.mData[index % NComponents];
        }

        Vector& operator=(const Vector& v) noexcept
        {
            for (size_t i = 0; i < NComponents; i++)
                mData[i] = v[i];
            return *this;
        }

        Vector& operator=(const TDataType scalar) noexcept
        {
            for (size_t i = 0; i < NComponents; i++)
                mData[i] = scalar;
            return *this;
        }

        void operator+=(const Vector& v) noexcept
        {
            for (size_t i = 0; i < NComponents; i++)
                mData[i] += v[i];
        }

        void operator-=(const Vector& v) noexcept
        {
            for (size_t i = 0; i < NComponents; i++)
                mData[i] -= v[i];
        }

        void operator*=(const Vector& v) noexcept
        {
            for (size_t i = 0; i < NComponents; i++)
                mData[i] *= v[i];
        }

        void operator/=(const Vector& v) noexcept
        {
            for (size_t i = 0; i < NComponents; i++)
                mData[i] /= v[i];
        }

        void operator+=(const TDataType scalar) noexcept
        {
            for (size_t i = 0; i < NComponents; i++)
                mData[i] += scalar;
        }

        void operator-=(const TDataType scalar) noexcept
        {
            for (size_t i = 0; i < NComponents; i++)
                mData[i] -= scalar;
        }

        void operator*=(const TDataType scalar) noexcept
        {
            for (size_t i = 0; i < NComponents; i++)
                mData[i] *= scalar;
        }

        void operator/=(const TDataType scalar) noexcept
        {
            for (size_t i = 0; i < NComponents; i++)
                mData[i] /= scalar;
        }

        Vector operator+(const Vector& vec) const noexcept
        {
            Vector result;
            for (size_t i = 0; i < NComponents; i++)
                result[i] = mData[i] + vec[i];

            return result;
        }

        Vector operator-(const Vector& vec) const noexcept
        {
            Vector result;
            for (size_t i = 0; i < NComponents; i++)
                result[i] = mData[i] - vec[i];

            return result;
        }

        Vector operator*(const Vector& vec) const noexcept
        {
            Vector result;
            for (size_t i = 0; i < NComponents; i++)
                result[i] = mData[i] * vec[i];

            return result;
        }

        Vector operator/(const Vector& vec) const noexcept
        {
            Vector result;
            for (size_t i = 0; i < NComponents; i++)
                result[i] = mData[i] / vec[i];

            return result;
        }

        Vector operator+(const TDataType scalar) const noexcept
        {
            Vector result;
            for (size_t i = 0; i < NComponents; i++)
                result[i] = mData[i] + scalar;

            return result;
        }

        Vector operator-(const TDataType scalar) const noexcept
        {
            Vector result;
            for (size_t i = 0; i < NComponents; i++)
                result[i] = mData[i] - scalar;

            return result;
        }

        Vector operator*(const TDataType scalar) const noexcept
        {
            Vector result;
            for (size_t i = 0; i < NComponents; i++)
                result[i] = mData[i] * scalar;

            return result;
        }

        Vector operator/(const TDataType scalar) const noexcept
        {
            Vector result;
            for (size_t i = 0; i < NComponents; i++)
                result[i] = mData[i] / scalar;

            return result;
        }

        bool operator==(const Vector& vec) const noexcept
        {
            for (size_t i = 0; i < NComponents; i++)
            {
                if (mData[i] != vec.mData[i])
                    return false;
            }

            return true;
        }

        Vector& clamp(const TDataType min, const TDataType max) noexcept
        {
            for (size_t i = 0; i < NComponents; i++)
            {
                mData[i] = (mData[i] < min) ? min : ((mData[i] > max) ? max : mData[i]);
            }

            return *this;
        }

        void write(TDataType* const dest) const noexcept
        {
            for (size_t i = 0; i < NComponents; i++)
                dest[i] = mData[i];
        }

        TDataType dot() const noexcept
        {
            return Vector::dot(*this);
        }

        TDataType dot(const Vector& vec) const noexcept
        {
            return Vector::dot(*this, vec);
        }
    };

    using Vector3f = Vector<float, 3>;
    using Vector3d = Vector<double, 3>;
    using Vector4f = Vector<float, 4>;
    using Vector4d = Vector<double, 4>;

    template<>
    class alignas(16) Vector<float, 3>
    {
        float mData[4];

    public:
        using DataType = float;
        static constexpr size_t NumComponents{3};

    public:
        static constexpr Vector zero() noexcept
        {
            Vector vec;
            vec.mData[0] = vec.mData[1] = vec.mData[2] = vec.mData[3] = 0.0f;
            return vec;
        }

        static Vector calcNormalize(const Vector& vec) noexcept
        {
            Vector result;

            __m128 vecTmp = _mm_load_ps(vec.mData);
            __m128 vecMag = _mm_rsqrt_ps(_mm_dp_ps(vecTmp, vecTmp, 0x70 | 0xF));
            _mm_store_ps(result.mData, _mm_mul_ps(vecTmp, vecMag));

            return result;
        }

        static Vector calcMin(const Vector& vec1, const Vector& vec2) noexcept
        {
            Vector result;
            _mm_store_ps(result.mData, _mm_min_ps(_mm_load_ps(vec1.mData), _mm_load_ps(vec2.mData)));
            return result;
        }

        static Vector calcMax(const Vector& vec1, const Vector& vec2) noexcept
        {
            Vector result;
            _mm_store_ps(result.mData, _mm_max_ps(_mm_load_ps(vec1.mData), _mm_load_ps(vec2.mData)));
            return result;
        }

        static std::tuple<Vector, Vector> calcMinMax(const Vector& vec1, const Vector& vec2) noexcept
        {
            Vector resultMin, resultMax;

            __m128 v1 = _mm_load_ps(vec1.mData);
            __m128 v2 = _mm_load_ps(vec2.mData);
            _mm_store_ps(resultMin.mData, _mm_min_ps(v1, v2));
            _mm_store_ps(resultMax.mData, _mm_max_ps(v1, v2));
            return {resultMin, resultMax};
        }

        static Vector calcPointAt(const Vector& origin, const Vector& direction, const float t) noexcept
        {
            Vector result;
            __m128 tmp;

            tmp = _mm_mul_ps(_mm_load_ps(direction.mData), _mm_load_ps1(&t));
            tmp = _mm_add_ps(tmp, _mm_load_ps(origin.mData));

            _mm_store_ps(result.mData, tmp);
            return result;
        }

        static Vector calcProject(const Vector& vecA, const Vector& vecB) noexcept
        {
            // returns the vector projection of vecA onto vecB

            auto m = vecB.magnitude();
            m = vecA.dot(vecB) / (m * m);
            return (vecB * m);
        }

        static Vector calcReflect(const Vector& vec, const Vector& planeNormal) noexcept
        {
            return vec - (calcProject(vec, planeNormal) * 2.0f);
        }

        static Vector calcCrossProduct(const Vector& p, const Vector& q) noexcept
        {
            __m128 vec1 = _mm_load_ps(p.mData);
            __m128 vec2 = _mm_load_ps(q.mData);

            __m128 xa = _mm_mul_ps(_mm_shuffle_ps(vec1, vec1, _MM_SHUFFLE(3, 0, 2, 1)), _mm_shuffle_ps(vec2, vec2, _MM_SHUFFLE(3, 1, 0, 2)));
            __m128 xb = _mm_mul_ps(_mm_shuffle_ps(vec1, vec1, _MM_SHUFFLE(3, 1, 0, 2)), _mm_shuffle_ps(vec2, vec2, _MM_SHUFFLE(3, 0, 2, 1)));

            Vector result;
            _mm_store_ps(result.mData, _mm_sub_ps(xa, xb));
            return result;
        }

        static Vector calcLerp(const Vector& from, const Vector& to, const float t) noexcept
        {
            assert(t >= 0.0f && t <= 1.0f);

            __m128 tmp;

            tmp = _mm_mul_ps(_mm_load_ps(from.mData), _mm_set_ps1(1.0f - t));
            tmp = _mm_add_ps(tmp, _mm_mul_ps(_mm_load_ps(to.mData), _mm_load_ps1(&t)));

            Vector result;
            _mm_store_ps(result.mData, tmp);
            return result;
        }

        static Vector calcClosestInSegment(const Vector& point, const Vector& p1, const Vector& p2) noexcept
        {
            Vector lineDir = p2 - p1;

            float t = lineDir.dot();
            if (Math::isZero(t))
            {
                Vector result;
                result.mData[0] = p1.mData[0];
                result.mData[1] = p1.mData[1];
                result.mData[2] = p1.mData[2];

                return result;
            }

            t = ((point.mData[0] - p1.mData[0]) * lineDir.mData[0] + (point.mData[1] - p1.mData[1]) * lineDir.mData[1] + (point.mData[2] - p1.mData[2]) * lineDir.mData[2]) / t;
            t = Math::fClamp(t, 0.0f, 1.0f);

            Vector result;
            result.mData[0] = p1.mData[0] + (t * lineDir.mData[0]);
            result.mData[1] = p1.mData[1] + (t * lineDir.mData[1]);
            result.mData[2] = p1.mData[2] + (t * lineDir.mData[2]);

            return result;
        }

        static Vector fromSpherical(float rad, float theta, float phi) noexcept
        {
            /**
             * This uses the ISO convention: radius r, inclination theta (radians) and azimuth phi (also radians)
             * Also:
             *  - theta: the angle in the XY plane from the X axis
             *  - phi: the angle from the positive Z axis to the vector
             */

            auto thetaTrig = Math::sinCos(theta);
            auto phiTrig = Math::sinCos(phi);

            Vector result;
            result.mData[0] = rad * std::get<0>(thetaTrig) * std::get<1>(phiTrig);
            result.mData[1] = rad * std::get<0>(thetaTrig) * std::get<0>(phiTrig);
            result.mData[2] = rad * std::get<1>(thetaTrig);

            return result;
        }

        static Vector fromInterpolateNormals(const Vector& n1, const Vector& n2, const float t) noexcept
        {
            float a = acos(n1.dot(n2));
            float sinA = 1.0f / (std::sinf(a) + 0.00001f);

            float t0 = sin((1.0f - t) * a);
            float t1 = sin(t * a);

            Vector result;
            result.mData[0] = (t0 * n1.mData[0] + t1 * n2.mData[0]) * sinA;
            result.mData[1] = (t0 * n1.mData[1] + t1 * n2.mData[1]) * sinA;
            result.mData[2] = (t0 * n1.mData[2] + t1 * n2.mData[2]) * sinA;

            return result;
        }

        static Vector evalSplineCatmullRom(const Vector& p1, const Vector& p2, const Vector& p3, const Vector& p4, float t) noexcept
        {
            assert(t >= 0.0f && t <= 1.0f);

            float tSqr = t * t * 0.5f;
            float tSqrSqr = t * tSqr;
            t *= 0.5f;

            Vector result(0.0f);

            result += p1 * -tSqrSqr;
            result += p2 * tSqrSqr * 3.0f;
            result += p3 * tSqrSqr * -3.0f;
            result += p4 * tSqrSqr;

            result += p1 * tSqr * 2.0f;
            result += p2 * tSqr * -5.0f;
            result += p3 * tSqr * 4.0f;
            result += p4 * -tSqr;

            result += p1 * -t;
            result += p3 * t;

            result += p2;

            return result;
        }

        static Vector evalSplineHermite(const Vector& p1, const Vector& p2, const Vector& p3, const Vector& p4, float t) noexcept
        {
            assert(t >= 0.0f && t <= 1.0f);

            float tSqr = t * t;
            float tCube = t * tSqr;

            Vector d1 = p2 - p1;
            Vector d2 = p4 - p3;

            Vector result = p2 * (2.0f * tCube - 3.0f * tSqr + 1.0f);
            result += p3 * (-2.0f * tCube + 3.0f * tSqr);
            result += d1 * (tCube - 2.0f * tSqr + t);
            result += d2 * (tCube - tSqr);

            return result;
        }

    public:
        constexpr Vector() = default;

        explicit constexpr Vector(const float scalar) noexcept
        {
            mData[0] = mData[1] = mData[2] = scalar;
            mData[3] = 0.0f;
        }

        explicit constexpr Vector(const float vx, const float vy, const float vz) noexcept
          : mData{vx, vy, vz, 0.0f}
        {}

        explicit constexpr Vector(std::span<const float, 3> v) noexcept
        {
            mData[0] = v[0];
            mData[1] = v[1];
            mData[2] = v[2];
            mData[3] = 0.0f;
        }

        explicit Vector(const __m128 vecDat) noexcept
        {
            _mm_store_ps(mData, vecDat);
        }

        template<typename Self>
        constexpr auto data(this Self&& self) noexcept
        {
            return std::span{self.mData, 3}.first<3>();
        }

        template<typename Self>
        constexpr auto& operator[](this Self&& self, const size_t index) noexcept
        {
            return self.mData[index % 3];
        }

        void operator+=(const Vector& v) noexcept
        {
            _mm_store_ps(mData, _mm_add_ps(_mm_load_ps(mData), _mm_load_ps(v.mData)));
        }

        void operator-=(const Vector& v) noexcept
        {
            _mm_store_ps(mData, _mm_sub_ps(_mm_load_ps(mData), _mm_load_ps(v.mData)));
        }

        void operator*=(const Vector& v) noexcept
        {
            _mm_store_ps(mData, _mm_mul_ps(_mm_load_ps(mData), _mm_load_ps(v.mData)));
        }

        void operator/=(const Vector& v) noexcept
        {
            _mm_store_ps(mData, _mm_div_ps(_mm_load_ps(mData), _mm_load_ps(v.mData)));
        }

        void operator+=(const float s) noexcept
        {
            _mm_store_ps(mData, _mm_add_ps(_mm_load_ps(mData), _mm_load_ps1(&s)));
        }

        void operator-=(const float s) noexcept
        {
            _mm_store_ps(mData, _mm_sub_ps(_mm_load_ps(mData), _mm_load_ps1(&s)));
        }

        void operator*=(const float s) noexcept
        {
            _mm_store_ps(mData, _mm_mul_ps(_mm_load_ps(mData), _mm_load_ps1(&s)));
        }

        void operator/=(const float s) noexcept
        {
            _mm_store_ps(mData, _mm_div_ps(_mm_load_ps(mData), _mm_load_ps1(&s)));
        }

        Vector operator-() const noexcept
        {
            return Vector(_mm_mul_ps(_mm_load_ps(mData), _mm_set_ps1(-1.0f)));
        }

        Vector operator+(const Vector& vec) const noexcept
        {
            return Vector(_mm_add_ps(_mm_load_ps(mData), _mm_load_ps(vec.mData)));
        }

        Vector operator-(const Vector& vec) const noexcept
        {
            return Vector(_mm_sub_ps(_mm_load_ps(mData), _mm_load_ps(vec.mData)));
        }

        Vector operator*(const Vector& vec) const noexcept
        {
            return Vector(_mm_mul_ps(_mm_load_ps(mData), _mm_load_ps(vec.mData)));
        }

        Vector operator/(const Vector& vec) const noexcept
        {
            return Vector(_mm_div_ps(_mm_load_ps(mData), _mm_load_ps(vec.mData)));
        }

        Vector operator+(const float s) const noexcept
        {
            return Vector(_mm_add_ps(_mm_load_ps(mData), _mm_load_ps1(&s)));
        }

        Vector operator-(const float s) const noexcept
        {
            return Vector(_mm_sub_ps(_mm_load_ps(mData), _mm_load_ps1(&s)));
        }

        Vector operator*(const float s) const noexcept
        {
            return Vector(_mm_mul_ps(_mm_load_ps(mData), _mm_load_ps1(&s)));
        }

        Vector operator/(const float s) const noexcept
        {
            return Vector(_mm_div_ps(_mm_load_ps(mData), _mm_load_ps1(&s)));
        }

        bool operator==(const Vector& vec) const noexcept
        {
            return Math::isZero(mData[0] - vec.mData[0]) && Math::isZero(mData[1] - vec.mData[1]) && Math::isZero(mData[2] - vec.mData[2]);
        }

        template<typename TTargetType, size_t NTargetComponents>
        Vector<TTargetType, NTargetComponents> convert() const noexcept;

        template<typename TTargetType, size_t NTargetComponents>
        Vector<TTargetType, NTargetComponents> convert(TTargetType w) const noexcept;

        void write(float dest[3]) const noexcept
        {
            std::memcpy(dest, mData, sizeof(float) * 3);
        }

        float dot() const noexcept
        {
            __m128 vecTmp = _mm_load_ps(mData);
            return _mm_cvtss_f32(_mm_dp_ps(vecTmp, vecTmp, 0x70 | 0xF));
        }

        float dot(const Vector& vec) const noexcept
        {
            return _mm_cvtss_f32(_mm_dp_ps(_mm_load_ps(mData), _mm_load_ps(vec.mData), 0x70 | 0xF));
        }

        float dot(const float x, const float y, const float z) const noexcept
        {
            return dot(Vector{x, y, z});
        }

        float magnitude() const noexcept
        {
            __m128 vecTmp = _mm_load_ps(mData);
            return _mm_cvtss_f32(_mm_sqrt_ss(_mm_dp_ps(vecTmp, vecTmp, 0x70 | 0xF)));
        }

        float magnitudeInv() const noexcept
        {
            __m128 vecTmp = _mm_load_ps(mData);
            return _mm_cvtss_f32(_mm_rsqrt_ss(_mm_dp_ps(vecTmp, vecTmp, 0x70 | 0xF)));
        }

        float distance(const Vector& vec) const noexcept
        {
            __m128 vecTmp = _mm_sub_ps(_mm_load_ps(mData), _mm_load_ps(vec.mData));
            return _mm_cvtss_f32(_mm_sqrt_ss(_mm_dp_ps(vecTmp, vecTmp, 0x70 | 0xF)));
        }

        float distance(const float x, const float y, const float z) const noexcept
        {
            return distance(Vector{x, y, z});
        }

        bool isZero(const float precision) const noexcept
        {
            return (std::abs(mData[0]) < precision) && (std::abs(mData[1]) < precision) && (std::abs(mData[2]) < precision);
        }

        bool isEqual(const Vector& vec, const float precision) const noexcept
        {
            return (std::abs(mData[0] - vec.mData[0]) < precision) && (std::abs(mData[1] - vec.mData[1]) < precision) && (std::abs(mData[2] - vec.mData[2]) < precision);
        }

        Vector& normalize() noexcept
        {
            __m128 vecTmp = _mm_load_ps(mData);
            __m128 vecMag = _mm_rsqrt_ps(_mm_dp_ps(vecTmp, vecTmp, 0x70 | 0xF));
            _mm_store_ps(mData, _mm_mul_ps(vecTmp, vecMag));

            return *this;
        }

        Vector& clamp(const float min, const float max) noexcept
        {
            __m128 tmp = _mm_load_ps(mData);
            tmp = _mm_max_ps(tmp, _mm_load_ps1(&min));
            tmp = _mm_min_ps(tmp, _mm_load_ps1(&max));
            _mm_store_ps(mData, tmp);

            return *this;
        }

        Vector& abs() noexcept
        {
            __m128 mask = _mm_castsi128_ps(_mm_set1_epi32(0x7FFFFFFF));
            _mm_store_ps(mData, _mm_and_ps(_mm_load_ps(mData), mask));

            return *this;
        }

        Vector& neg() noexcept
        {
            __m128 mask = _mm_castsi128_ps(_mm_set1_epi32(0x80000000));
            _mm_store_ps(mData, _mm_xor_ps(_mm_load_ps(mData), mask));

            return *this;
        }

        Vector& mad(float opMul, float opAdd) noexcept
        {
            __m128 tmp = _mm_load_ps(mData);
            _mm_store_ps(mData, _mm_add_ps(_mm_mul_ps(tmp, _mm_load_ps1(&opMul)), _mm_load_ps1(&opAdd)));

            return *this;
        }

        Vector project(const Vector& vec) const noexcept
        {
            return Vector::calcProject(*this, vec);
        }

        Vector reflect(const Vector& planeNormal) const noexcept
        {
            return Vector::calcReflect(*this, planeNormal);
        }

        Vector crossProduct(const Vector& vec) const noexcept
        {
            return Vector::calcCrossProduct(*this, vec);
        }
    };

    template<>
    class alignas(32) Vector<double, 3>
    {
        double mData[4];

    public:
        using DataType = double;
        static constexpr size_t NumComponents{3};

    private:
        static __m128d mmDot(__m256d vec1, __m256d vec2) noexcept
        {
            __m256d data = _mm256_mul_pd(vec1, vec2);

            __m128d low = _mm256_castpd256_pd128(data);
            __m128d high = _mm_shuffle_pd(_mm256_extractf128_pd(data, 1), _mm_setzero_pd(), 0b10); // must zero out w

            __m128d sum = _mm_add_pd(low, high);
            return _mm_add_pd(sum, _mm_shuffle_pd(sum, sum, 0b01));
        }

        static __m128d mmDot(__m256d vec) noexcept
        {
            return mmDot(vec, vec);
        }

    public:
        static constexpr Vector zero() noexcept
        {
            Vector vec;
            vec.mData[0] = vec.mData[1] = vec.mData[2] = vec.mData[3] = 0.0f;
            return vec;
        }

        static Vector calcNormalize(const Vector& vec) noexcept
        {
            __m256d data = _mm256_load_pd(vec.mData);
            __m128d temp = _mm_invsqrt_pd(mmDot(data));
            __m256d invMag = _mm256_permute2f128_pd(_mm256_castpd128_pd256(temp), _mm256_castpd128_pd256(temp), 0x20);

            Vector result;
            _mm256_store_pd(result.mData, _mm256_mul_pd(data, invMag));
            return result;
        }

        static Vector calcMin(const Vector& vec1, const Vector& vec2) noexcept
        {
            Vector result;
            _mm256_store_pd(result.mData, _mm256_min_pd(_mm256_load_pd(vec1.mData), _mm256_load_pd(vec2.mData)));
            return result;
        }

        static Vector calcMax(const Vector& vec1, const Vector& vec2) noexcept
        {
            Vector result;
            _mm256_store_pd(result.mData, _mm256_max_pd(_mm256_load_pd(vec1.mData), _mm256_load_pd(vec2.mData)));
            return result;
        }

        static std::tuple<Vector, Vector> calcMinMax(const Vector& vec1, const Vector& vec2) noexcept
        {
            Vector resultMin, resultMax;

            __m256d v1 = _mm256_load_pd(vec1.mData);
            __m256d v2 = _mm256_load_pd(vec2.mData);
            _mm256_store_pd(resultMin.mData, _mm256_min_pd(v1, v2));
            _mm256_store_pd(resultMax.mData, _mm256_max_pd(v1, v2));
            return {resultMin, resultMax};
        }

        static Vector calcPointAt(const Vector& origin, const Vector& direction, const double t) noexcept
        {
            __m256d tmp = _mm256_mul_pd(_mm256_load_pd(direction.mData), _mm256_set1_pd(t));
            tmp = _mm256_add_pd(tmp, _mm256_load_pd(origin.mData));

            Vector result;
            _mm256_store_pd(result.mData, tmp);
            return result;
        }

        static Vector calcProject(const Vector& vecA, const Vector& vecB) noexcept
        {
            // returns the vector projection of vecA onto vecB

            auto m = vecB.magnitude();
            m = vecA.dot(vecB) / (m * m);
            return (vecB * m);
        }

        static Vector calcReflect(const Vector& vec, const Vector& planeNormal) noexcept
        {
            return vec - (calcProject(vec, planeNormal) * 2.0);
        }

        static Vector calcCrossProduct(const Vector& p, const Vector& q) noexcept
        {
            __m256d vec1 = _mm256_load_pd(p.mData);
            __m256d vec2 = _mm256_load_pd(q.mData);

            __m256d xa = _mm256_mul_pd(_mm256_permute4x64_pd(vec1, _MM_SHUFFLE(3, 0, 2, 1)), _mm256_permute4x64_pd(vec2, _MM_SHUFFLE(3, 1, 0, 2)));
            __m256d xb = _mm256_mul_pd(_mm256_permute4x64_pd(vec1, _MM_SHUFFLE(3, 1, 0, 2)), _mm256_permute4x64_pd(vec2, _MM_SHUFFLE(3, 0, 2, 1)));

            Vector result;
            _mm256_store_pd(result.mData, _mm256_sub_pd(xa, xb));
            return result;
        }

        static Vector calcLerp(const Vector& from, const Vector& to, const double t) noexcept
        {
            assert(t >= 0.0 && t <= 1.0);

            __m256d tmp = _mm256_mul_pd(_mm256_load_pd(from.mData), _mm256_set1_pd(1.0 - t));
            tmp = _mm256_add_pd(tmp, _mm256_mul_pd(_mm256_load_pd(to.mData), _mm256_set1_pd(t)));

            Vector result;
            _mm256_store_pd(result.mData, tmp);
            return result;
        }

        static Vector calcClosestInSegment(const Vector& point, const Vector& p1, const Vector& p2) noexcept
        {
            Vector lineDir = p2 - p1;

            double t = lineDir.dot();
            if (Math::isZero(t))
            {
                Vector result;
                result.mData[0] = p1.mData[0];
                result.mData[1] = p1.mData[1];
                result.mData[2] = p1.mData[2];

                return result;
            }

            t = ((point.mData[0] - p1.mData[0]) * lineDir.mData[0] + (point.mData[1] - p1.mData[1]) * lineDir.mData[1] + (point.mData[2] - p1.mData[2]) * lineDir.mData[2]) / t;
            t = Math::fClamp(t, 0.0, 1.0);

            Vector result;
            result.mData[0] = p1.mData[0] + (t * lineDir.mData[0]);
            result.mData[1] = p1.mData[1] + (t * lineDir.mData[1]);
            result.mData[2] = p1.mData[2] + (t * lineDir.mData[2]);

            return result;
        }

        static Vector fromSpherical(double rad, double theta, double phi) noexcept
        {
            /**
             * This uses the ISO convention: radius r, inclination theta (radians) and azimuth phi (also radians)
             * Also:
             *  - theta: the angle in the XY plane from the X axis
             *  - phi: the angle from the positive Z axis to the vector
             */

            auto thetaTrig = std::make_tuple(Math::sin(theta), Math::cos(theta));
            auto phiTrig = std::make_tuple(Math::sin(phi), Math::cos(phi));

            Vector result;
            result.mData[0] = rad * std::get<0>(thetaTrig) * std::get<1>(phiTrig);
            result.mData[1] = rad * std::get<0>(thetaTrig) * std::get<0>(phiTrig);
            result.mData[2] = rad * std::get<1>(thetaTrig);

            return result;
        }

        static Vector fromInterpolateNormals(const Vector& n1, const Vector& n2, const double t) noexcept
        {
            double a = std::acos(n1.dot(n2));
            double sinA = 1.0 / (std::sin(a) + 0.0000001);

            double t0 = std::sin((1.0 - t) * a);
            double t1 = std::sin(t * a);

            Vector result;
            result.mData[0] = (t0 * n1.mData[0] + t1 * n2.mData[0]) * sinA;
            result.mData[1] = (t0 * n1.mData[1] + t1 * n2.mData[1]) * sinA;
            result.mData[2] = (t0 * n1.mData[2] + t1 * n2.mData[2]) * sinA;

            return result;
        }

    public:
        constexpr Vector() = default;

        explicit constexpr Vector(const double scalar)
        {
            mData[0] = mData[1] = mData[2] = scalar;
            mData[3] = 0.0f;
        }

        explicit constexpr Vector(const double vx, const double vy, const double vz)
          : mData{vx, vy, vz, 0.0}
        {}

        explicit constexpr Vector(std::span<const double, 3> v) noexcept
          : mData{v[0], v[1], v[2], 0.0}
        {}

        explicit Vector(const __m256d vecDat) noexcept
        {
            _mm256_store_pd(mData, vecDat);
        }

        template<typename Self>
        constexpr auto data(this Self&& self) noexcept
        {
            return std::span{self.mData, 3}.first<3>();
        }

        template<typename Self>
        constexpr auto& operator[](this Self&& self, const size_t index) noexcept
        {
            return self.mData[index % 3];
        }

        void operator+=(const Vector& v) noexcept
        {
            _mm256_store_pd(mData, _mm256_add_pd(_mm256_load_pd(mData), _mm256_load_pd(v.mData)));
        }

        void operator-=(const Vector& v) noexcept
        {
            _mm256_store_pd(mData, _mm256_sub_pd(_mm256_load_pd(mData), _mm256_load_pd(v.mData)));
        }

        void operator*=(const Vector& v) noexcept
        {
            _mm256_store_pd(mData, _mm256_mul_pd(_mm256_load_pd(mData), _mm256_load_pd(v.mData)));
        }

        void operator/=(const Vector& v) noexcept
        {
            _mm256_store_pd(mData, _mm256_div_pd(_mm256_load_pd(mData), _mm256_load_pd(v.mData)));
        }

        void operator+=(const double n) noexcept
        {
            _mm256_store_pd(mData, _mm256_add_pd(_mm256_load_pd(mData), _mm256_set1_pd(n)));
        }

        void operator-=(const double n) noexcept
        {
            _mm256_store_pd(mData, _mm256_sub_pd(_mm256_load_pd(mData), _mm256_set1_pd(n)));
        }

        void operator*=(const double n) noexcept
        {
            _mm256_store_pd(mData, _mm256_mul_pd(_mm256_load_pd(mData), _mm256_set1_pd(n)));
        }

        void operator/=(const double n) noexcept
        {
            _mm256_store_pd(mData, _mm256_div_pd(_mm256_load_pd(mData), _mm256_set1_pd(n)));
        }

        Vector operator-() const noexcept
        {
            return Vector(_mm256_mul_pd(_mm256_load_pd(mData), _mm256_set1_pd(-1.0)));
        }

        Vector operator+(const Vector& vec) const noexcept
        {
            return Vector(_mm256_add_pd(_mm256_load_pd(mData), _mm256_load_pd(vec.mData)));
        }

        Vector operator-(const Vector& vec) const noexcept
        {
            return Vector(_mm256_sub_pd(_mm256_load_pd(mData), _mm256_load_pd(vec.mData)));
        }

        Vector operator*(const Vector& vec) const noexcept
        {
            return Vector(_mm256_mul_pd(_mm256_load_pd(mData), _mm256_load_pd(vec.mData)));
        }

        Vector operator/(const Vector& vec) const noexcept
        {
            return Vector(_mm256_div_pd(_mm256_load_pd(mData), _mm256_load_pd(vec.mData)));
        }

        Vector operator+(const double n) const noexcept
        {
            return Vector(_mm256_add_pd(_mm256_load_pd(mData), _mm256_set1_pd(n)));
        }

        Vector operator-(const double n) const noexcept
        {
            return Vector(_mm256_sub_pd(_mm256_load_pd(mData), _mm256_set1_pd(n)));
        }

        Vector operator*(const double n) const noexcept
        {
            return Vector(_mm256_mul_pd(_mm256_load_pd(mData), _mm256_set1_pd(n)));
        }

        Vector operator/(const double n) const noexcept
        {
            return Vector(_mm256_div_pd(_mm256_load_pd(mData), _mm256_set1_pd(n)));
        }

        bool operator==(const Vector& vec) const noexcept
        {
            return Math::isZero(mData[0] - vec.mData[0]) && Math::isZero(mData[1] - vec.mData[1]) && Math::isZero(mData[2] - vec.mData[2]);
        }

        template<typename TTargetType, size_t NTargetComponents>
        Vector<TTargetType, NTargetComponents> convert() const noexcept;

        template<typename TTargetType, size_t NTargetComponents>
        Vector<TTargetType, NTargetComponents> convert(TTargetType w) const noexcept;

        void write(double dest[3]) const noexcept
        {
            std::memcpy(dest, mData, sizeof(double) * 3);
        }

        double dot() const noexcept
        {
            return _mm_cvtsd_f64(mmDot(_mm256_load_pd(mData)));
        }

        double dot(const Vector& vec) const noexcept
        {
            return _mm_cvtsd_f64(mmDot(_mm256_load_pd(mData), _mm256_load_pd(vec.mData)));
        }

        double magnitude() const noexcept
        {
            return _mm_cvtsd_f64(_mm_sqrt_pd(mmDot(_mm256_load_pd(mData))));
        }

        double magnitudeInv() const noexcept
        {
            return _mm_cvtsd_f64(_mm_invsqrt_pd(mmDot(_mm256_load_pd(mData))));
        }

        double distance(const Vector& vec) const noexcept
        {
            Vector tmp;

            __m256d newVec = _mm256_sub_pd(_mm256_load_pd(mData), _mm256_load_pd(vec.mData));
            _mm256_store_pd(tmp.mData, newVec);

            return tmp.magnitude();
        }

        double distance(const double x, const double y, const double z) const noexcept
        {
            return distance(Vector(x, y, z));
        }

        bool isZero(const double precision) const noexcept
        {
            return (std::abs(mData[0]) < precision) && (std::abs(mData[1]) < precision) && (std::abs(mData[2]) < precision);
        }

        bool isEqual(const Vector& vec, const double precision) const
        {
            return (std::abs(mData[0] - vec.mData[0]) < precision) && (std::abs(mData[1] - vec.mData[1]) < precision) && (std::abs(mData[2] - vec.mData[2]) < precision);
        }

        Vector& normalize() noexcept
        {
            __m256d data = _mm256_load_pd(mData);
            __m128d temp = _mm_invsqrt_pd(mmDot(data));
            __m256d invMag = _mm256_permute2f128_pd(_mm256_castpd128_pd256(temp), _mm256_castpd128_pd256(temp), 0x20);

            _mm256_store_pd(mData, _mm256_mul_pd(data, invMag));

            return *this;
        }

        Vector& clamp(const double min, const double max) noexcept
        {
            __m256d tmp = _mm256_load_pd(mData);
            tmp = _mm256_max_pd(tmp, _mm256_set1_pd(min));
            tmp = _mm256_min_pd(tmp, _mm256_set1_pd(max));
            _mm256_store_pd(mData, tmp);

            return *this;
        }

        Vector& abs() noexcept
        {
            __m256d mask = _mm256_castsi256_pd(_mm256_set1_epi64x(static_cast<int64_t>(0x8000000000000000)));
            _mm256_store_pd(mData, _mm256_andnot_pd(mask, _mm256_load_pd(mData)));

            return *this;
        }

        Vector& neg() noexcept
        {
            __m256d mask = _mm256_castsi256_pd(_mm256_set1_epi64x(static_cast<int64_t>(0x8000000000000000)));
            _mm256_store_pd(mData, _mm256_xor_pd(_mm256_load_pd(mData), mask));

            return *this;
        }

        Vector& mad(double opMul, double opAdd) noexcept
        {
            __m256d tmp = _mm256_load_pd(mData);
            _mm256_store_pd(mData, _mm256_add_pd(_mm256_mul_pd(tmp, _mm256_set1_pd(opMul)), _mm256_set1_pd(opAdd)));

            return *this;
        }

        Vector project(const Vector& vec) const noexcept
        {
            return Vector::calcProject(*this, vec);
        }

        Vector reflect(const Vector& planeNormal) const noexcept
        {
            return Vector::calcReflect(*this, planeNormal);
        }

        Vector crossProduct(const Vector& vec) const noexcept
        {
            return Vector::calcCrossProduct(*this, vec);
        }
    };

    template<>
    class alignas(16) Vector<float, 4>
    {
        float mData[4];

    public:
        using DataType = float;
        static constexpr size_t NumComponents{4};

    public:
        static constexpr Vector zero() noexcept
        {
            Vector vec;
            vec.mData[0] = vec.mData[1] = vec.mData[2] = vec.mData[3] = 0.0f;
            return vec;
        }

        static Vector calcNormalize(const Vector& vec) noexcept
        {
            __m128 vecTmp = _mm_load_ps(vec.mData);
            __m128 vecMag = _mm_rsqrt_ps(_mm_dp_ps(vecTmp, vecTmp, 0xF0 | 0xF));

            Vector result;
            _mm_store_ps(result.mData, _mm_mul_ps(vecTmp, vecMag));
            return result;
        }

        static Vector calcMin(const Vector& vec1, const Vector& vec2) noexcept
        {
            Vector result;
            _mm_store_ps(result.mData, _mm_min_ps(_mm_load_ps(vec1.mData), _mm_load_ps(vec2.mData)));
            return result;
        }

        static Vector calcMax(const Vector& vec1, const Vector& vec2) noexcept
        {
            Vector result;
            _mm_store_ps(result.mData, _mm_max_ps(_mm_load_ps(vec1.mData), _mm_load_ps(vec2.mData)));
            return result;
        }

        static std::tuple<Vector, Vector> calcMinMax(const Vector& vec1, const Vector& vec2) noexcept
        {
            Vector resultMin, resultMax;

            __m128 v1 = _mm_load_ps(vec1.mData);
            __m128 v2 = _mm_load_ps(vec2.mData);
            _mm_store_ps(resultMin.mData, _mm_min_ps(v1, v2));
            _mm_store_ps(resultMax.mData, _mm_max_ps(v1, v2));
            return {resultMin, resultMax};
        }

        static Vector calcPointAt(const Vector& origin, const Vector& direction, const float t) noexcept
        {
            Vector result;
            __m128 tmp;

            tmp = _mm_mul_ps(_mm_load_ps(direction.mData), _mm_load_ps1(&t));
            tmp = _mm_add_ps(tmp, _mm_load_ps(origin.mData));

            _mm_store_ps(result.mData, tmp);
            return result;
        }

        static Vector calcLerp(const Vector& from, const Vector& to, const float t) noexcept
        {
            assert(t >= 0.0f && t <= 1.0f);

            __m128 tmp;

            tmp = _mm_mul_ps(_mm_load_ps(from.mData), _mm_set_ps1(1.0f - t));
            tmp = _mm_add_ps(tmp, _mm_mul_ps(_mm_load_ps(to.mData), _mm_load_ps1(&t)));

            Vector result;
            _mm_store_ps(result.mData, tmp);
            return result;
        }

    public:
        constexpr Vector() = default;

        explicit constexpr Vector(const float scalar) noexcept
        {
            mData[0] = mData[1] = mData[2] = mData[3] = scalar;
        }

        explicit constexpr Vector(const float vx, const float vy, const float vz, const float vw) noexcept
          : mData{vx, vy, vz, vw}
        {}

        explicit constexpr Vector(std::span<const float, 4> v) noexcept
          : mData{v[0], v[1], v[2], v[3]}
        {}

        explicit Vector(const __m128 vecDat) noexcept
        {
            _mm_store_ps(mData, vecDat);
        }

        template<typename Self>
        constexpr auto data(this Self&& self) noexcept
        {
            return std::span{self.mData, 4}.first<4>();
        }

        template<typename Self>
        constexpr auto& operator[](this Self&& self, const size_t index) noexcept
        {
            return self.mData[index % 4];
        }

        void operator+=(const Vector& v) noexcept
        {
            _mm_store_ps(mData, _mm_add_ps(_mm_load_ps(mData), _mm_load_ps(v.mData)));
        }

        void operator-=(const Vector& v) noexcept
        {
            _mm_store_ps(mData, _mm_sub_ps(_mm_load_ps(mData), _mm_load_ps(v.mData)));
        }

        void operator*=(const Vector& v) noexcept
        {
            _mm_store_ps(mData, _mm_mul_ps(_mm_load_ps(mData), _mm_load_ps(v.mData)));
        }

        void operator/=(const Vector& v) noexcept
        {
            _mm_store_ps(mData, _mm_div_ps(_mm_load_ps(mData), _mm_load_ps(v.mData)));
        }

        void operator+=(const float n) noexcept
        {
            _mm_store_ps(mData, _mm_add_ps(_mm_load_ps(mData), _mm_load_ps1(&n)));
        }

        void operator-=(const float n) noexcept
        {
            _mm_store_ps(mData, _mm_sub_ps(_mm_load_ps(mData), _mm_load_ps1(&n)));
        }

        void operator*=(const float n) noexcept
        {
            _mm_store_ps(mData, _mm_mul_ps(_mm_load_ps(mData), _mm_load_ps1(&n)));
        }

        void operator/=(const float n) noexcept
        {
            _mm_store_ps(mData, _mm_div_ps(_mm_load_ps(mData), _mm_load_ps1(&n)));
        }

        Vector operator-() const noexcept
        {
            return Vector(_mm_mul_ps(_mm_load_ps(mData), _mm_set_ps1(-1.0f)));
        }

        Vector operator+(const Vector& vec) const noexcept
        {
            return Vector(_mm_add_ps(_mm_load_ps(mData), _mm_load_ps(vec.mData)));
        }

        Vector operator-(const Vector& vec) const noexcept
        {
            return Vector(_mm_sub_ps(_mm_load_ps(mData), _mm_load_ps(vec.mData)));
        }

        Vector operator*(const Vector& vec) const noexcept
        {
            return Vector(_mm_mul_ps(_mm_load_ps(mData), _mm_load_ps(vec.mData)));
        }

        Vector operator/(const Vector& vec) const noexcept
        {
            return Vector(_mm_div_ps(_mm_load_ps(mData), _mm_load_ps(vec.mData)));
        }

        Vector operator+(const float s) const noexcept
        {
            return Vector(_mm_add_ps(_mm_load_ps(mData), _mm_load_ps1(&s)));
        }

        Vector operator-(const float s) const noexcept
        {
            return Vector(_mm_sub_ps(_mm_load_ps(mData), _mm_load_ps1(&s)));
        }

        Vector operator*(const float s) const noexcept
        {
            return Vector(_mm_mul_ps(_mm_load_ps(mData), _mm_load_ps1(&s)));
        }

        Vector operator/(const float s) const noexcept
        {
            return Vector(_mm_div_ps(_mm_load_ps(mData), _mm_load_ps1(&s)));
        }

        bool operator==(const Vector& vec) const noexcept
        {
            return Math::isZero(mData[0] - vec.mData[0]) && Math::isZero(mData[1] - vec.mData[1]) && Math::isZero(mData[2] - vec.mData[2]);
        }

        template<typename TTargetType, size_t NTargetComponents>
        Vector<TTargetType, NTargetComponents> convert() const noexcept;

        void write(float dest[4]) const noexcept
        {
            std::memcpy(dest, mData, sizeof(float) * 4);
        }

        float dot() const noexcept
        {
            float final;

            _mm_store_ss(&final, _mm_dp_ps(_mm_load_ps(mData), _mm_load_ps(mData), 0xF0 | 0xF));
            return final;
        }

        float dot(const Vector& vec) const noexcept
        {
            float final;

            _mm_store_ss(&final, _mm_dp_ps(_mm_load_ps(mData), _mm_load_ps(vec.mData), 0xF0 | 0xF));
            return final;
        }

        float dot(const float x, const float y, const float z, const float w) const noexcept
        {
            return dot(Vector{x, y, z, w});
        }

        float magnitude() const noexcept
        {
            float final;

            __m128 vecTmp = _mm_load_ps(mData);
            _mm_store_ss(&final, _mm_sqrt_ss(_mm_dp_ps(vecTmp, vecTmp, 0xF0 | 0xF)));
            return final;
        }

        float magnitudeInv() const noexcept
        {
            float final;

            __m128 vecTmp = _mm_load_ps(mData);
            _mm_store_ss(&final, _mm_rsqrt_ss(_mm_dp_ps(vecTmp, vecTmp, 0xF0 | 0xF)));
            return final;
        }

        float distance(const Vector& vec) const noexcept
        {
            float final;

            __m128 vecTmp = _mm_sub_ps(_mm_load_ps(mData), _mm_load_ps(vec.mData));
            _mm_store_ss(&final, _mm_sqrt_ss(_mm_dp_ps(vecTmp, vecTmp, 0xF0 | 0xF)));
            return final;
        }

        float distance(const float x, const float y, const float z, const float w) const noexcept
        {
            return distance(Vector(x, y, z, w));
        }

        bool isZero(const float precision) const noexcept
        {
            return (std::abs(mData[0]) < precision) && (std::abs(mData[1]) < precision) && (std::abs(mData[2]) < precision) && (std::abs(mData[3]) < precision);
        }

        bool isEqual(const Vector& vec, const float precision) const noexcept
        {
            return (std::abs(mData[0] - vec.mData[0]) < precision) && (std::abs(mData[1] - vec.mData[1]) < precision) && (std::abs(mData[2] - vec.mData[2]) < precision) &&
                   (std::abs(mData[3] - vec.mData[3]) < precision);
        }

        Vector& normalize() noexcept
        {
            __m128 vecTmp = _mm_load_ps(mData);
            __m128 vecMag = _mm_rsqrt_ps(_mm_dp_ps(vecTmp, vecTmp, 0xF0 | 0xF));
            _mm_store_ps(mData, _mm_mul_ps(vecTmp, vecMag));

            return *this;
        }

        Vector& clamp(const float min, const float max) noexcept
        {
            __m128 tmp = _mm_load_ps(mData);
            tmp = _mm_max_ps(tmp, _mm_load_ps1(&min));
            tmp = _mm_min_ps(tmp, _mm_load_ps1(&max));
            _mm_store_ps(mData, tmp);

            return *this;
        }

        Vector& abs() noexcept
        {
            __m128 mask = _mm_castsi128_ps(_mm_set1_epi32(0x7FFFFFFF));
            _mm_store_ps(mData, _mm_and_ps(_mm_load_ps(mData), mask));

            return *this;
        }

        Vector& neg() noexcept
        {
            __m128 mask = _mm_castsi128_ps(_mm_set1_epi32(0x80000000));
            _mm_store_ps(mData, _mm_xor_ps(_mm_load_ps(mData), mask));

            return *this;
        }

        Vector& mad(float opMul, float opAdd) noexcept
        {
            __m128 tmp = _mm_load_ps(mData);
            _mm_store_ps(mData, _mm_add_ps(_mm_mul_ps(tmp, _mm_load_ps1(&opMul)), _mm_load_ps1(&opAdd)));

            return *this;
        }
    };

    template<>
    class alignas(32) Vector<double, 4>
    {
        double mData[4];

    public:
        using DataType = double;
        static constexpr size_t NumComponents{4};

    private:
        static __m128d mmDot(__m256d vec1, __m256d vec2) noexcept
        {
            __m256d data = _mm256_mul_pd(vec1, vec2);

            __m128d low = _mm256_castpd256_pd128(data);
            __m128d high = _mm256_extractf128_pd(data, 1);

            __m128d sum = _mm_add_pd(low, high);
            return _mm_add_pd(sum, _mm_shuffle_pd(sum, sum, 0b01));
        }

        static __m128d mmDot(__m256d vec) noexcept
        {
            return mmDot(vec, vec);
        }

    public:
        static constexpr Vector zero() noexcept
        {
            Vector vec;
            vec.mData[0] = vec.mData[1] = vec.mData[2] = vec.mData[3] = 0.0f;
            return vec;
        }

        static Vector calcNormalize(const Vector& vec)
        {
            __m256d data = _mm256_load_pd(vec.mData);

            __m128d dot = mmDot(data);
            __m256d invMag = _mm256_invsqrt_pd(_mm256_set_m128d(dot, dot));

            Vector result;
            _mm256_store_pd(result.mData, _mm256_mul_pd(data, invMag));
            return result;
        }

        static Vector calcMin(const Vector& vec1, const Vector& vec2)
        {
            Vector result;
            _mm256_store_pd(result.mData, _mm256_min_pd(_mm256_load_pd(vec1.mData), _mm256_load_pd(vec2.mData)));
            return result;
        }

        static Vector calcMax(const Vector& vec1, const Vector& vec2)
        {
            Vector result;
            _mm256_store_pd(result.mData, _mm256_max_pd(_mm256_load_pd(vec1.mData), _mm256_load_pd(vec2.mData)));
            return result;
        }

        static std::tuple<Vector, Vector> calcMinMax(const Vector& vec1, const Vector& vec2)
        {
            Vector resultMin, resultMax;

            __m256d v1 = _mm256_load_pd(vec1.mData);
            __m256d v2 = _mm256_load_pd(vec2.mData);
            _mm256_store_pd(resultMin.mData, _mm256_min_pd(v1, v2));
            _mm256_store_pd(resultMax.mData, _mm256_max_pd(v1, v2));
            return {resultMin, resultMax};
        }

        static Vector calcPointAt(const Vector& origin, const Vector& direction, const double t)
        {
            __m256d tmp = _mm256_mul_pd(_mm256_load_pd(direction.mData), _mm256_set1_pd(t));
            tmp = _mm256_add_pd(tmp, _mm256_load_pd(origin.mData));

            Vector result;
            _mm256_store_pd(result.mData, tmp);
            return result;
        }

        static Vector calcLerp(const Vector& from, const Vector& to, const double t)
        {
            assert(t >= 0.0 && t <= 1.0);

            __m256d tmp = _mm256_mul_pd(_mm256_load_pd(from.mData), _mm256_set1_pd(1.0 - t));
            tmp = _mm256_add_pd(tmp, _mm256_mul_pd(_mm256_load_pd(to.mData), _mm256_set1_pd(t)));

            Vector result;
            _mm256_store_pd(result.mData, tmp);
            return result;
        }

    public:
        constexpr Vector() = default;

        explicit constexpr Vector(const double scalar)
        {
            mData[0] = mData[1] = mData[2] = mData[3] = scalar;
        }

        explicit constexpr Vector(const double vx, const double vy, const double vz, const double vw)
          : mData{vx, vy, vz, vw}
        {}

        explicit constexpr Vector(std::span<const double, 4> v) noexcept
          : mData{v[0], v[1], v[2], v[3]}
        {}

        explicit Vector(const __m256d vecDat) noexcept
        {
            _mm256_store_pd(mData, vecDat);
        }

        template<typename Self>
        constexpr auto data(this Self&& self) noexcept
        {
            return std::span{self.mData, 4}.first<4>();
        }

        template<typename Self>
        constexpr auto& operator[](this Self&& self, const size_t index) noexcept
        {
            return self.mData[index % 4];
        }

        void operator+=(const Vector& v) noexcept
        {
            _mm256_store_pd(mData, _mm256_add_pd(_mm256_load_pd(mData), _mm256_load_pd(v.mData)));
        }

        void operator-=(const Vector& v) noexcept
        {
            _mm256_store_pd(mData, _mm256_sub_pd(_mm256_load_pd(mData), _mm256_load_pd(v.mData)));
        }

        void operator*=(const Vector& v) noexcept
        {
            _mm256_store_pd(mData, _mm256_mul_pd(_mm256_load_pd(mData), _mm256_load_pd(v.mData)));
        }

        void operator/=(const Vector& v) noexcept
        {
            _mm256_store_pd(mData, _mm256_div_pd(_mm256_load_pd(mData), _mm256_load_pd(v.mData)));
        }

        void operator+=(const double n) noexcept
        {
            _mm256_store_pd(mData, _mm256_add_pd(_mm256_load_pd(mData), _mm256_set1_pd(n)));
        }

        void operator-=(const double n) noexcept
        {
            _mm256_store_pd(mData, _mm256_sub_pd(_mm256_load_pd(mData), _mm256_set1_pd(n)));
        }

        void operator*=(const double n) noexcept
        {
            _mm256_store_pd(mData, _mm256_mul_pd(_mm256_load_pd(mData), _mm256_set1_pd(n)));
        }

        void operator/=(const double n) noexcept
        {
            _mm256_store_pd(mData, _mm256_div_pd(_mm256_load_pd(mData), _mm256_set1_pd(n)));
        }

        Vector operator-() const noexcept
        {
            return Vector(_mm256_mul_pd(_mm256_load_pd(mData), _mm256_set1_pd(-1.0)));
        }

        Vector operator+(const Vector& vec) const noexcept
        {
            return Vector(_mm256_add_pd(_mm256_load_pd(mData), _mm256_load_pd(vec.mData)));
        }

        Vector operator-(const Vector& vec) const noexcept
        {
            return Vector(_mm256_sub_pd(_mm256_load_pd(mData), _mm256_load_pd(vec.mData)));
        }

        Vector operator*(const Vector& vec) const noexcept
        {
            return Vector(_mm256_mul_pd(_mm256_load_pd(mData), _mm256_load_pd(vec.mData)));
        }

        Vector operator/(const Vector& vec) const noexcept
        {
            return Vector(_mm256_div_pd(_mm256_load_pd(mData), _mm256_load_pd(vec.mData)));
        }

        Vector operator+(const double n) const noexcept
        {
            return Vector(_mm256_add_pd(_mm256_load_pd(mData), _mm256_set1_pd(n)));
        }

        Vector operator-(const double n) const noexcept
        {
            return Vector(_mm256_sub_pd(_mm256_load_pd(mData), _mm256_set1_pd(n)));
        }

        Vector operator*(const double n) const noexcept
        {
            return Vector(_mm256_mul_pd(_mm256_load_pd(mData), _mm256_set1_pd(n)));
        }

        Vector operator/(const double n) const noexcept
        {
            return Vector(_mm256_div_pd(_mm256_load_pd(mData), _mm256_set1_pd(n)));
        }

        bool operator==(const Vector& vec) const noexcept
        {
            return Math::isZero(mData[0] - vec.mData[0]) && Math::isZero(mData[1] - vec.mData[1]) && Math::isZero(mData[2] - vec.mData[2]) && Math::isZero(mData[3] - vec.mData[3]);
        }

        template<typename TTargetType, size_t NTargetComponents>
        Vector<TTargetType, NTargetComponents> convert() const noexcept;

        void write(double dest[4]) const noexcept
        {
            std::memcpy(dest, mData, sizeof(double) * 4);
        }

        double dot() const noexcept
        {
            return _mm_cvtsd_f64(mmDot(_mm256_load_pd(mData)));
        }

        double dot(const Vector& vec) const noexcept
        {
            return _mm_cvtsd_f64(mmDot(_mm256_load_pd(mData), _mm256_load_pd(vec.mData)));
        }

        double magnitude() const noexcept
        {
            return _mm_cvtsd_f64(_mm_sqrt_pd(mmDot(_mm256_load_pd(mData))));
        }

        double magnitudeInv() const noexcept
        {
            return _mm_cvtsd_f64(_mm_invsqrt_pd(mmDot(_mm256_load_pd(mData))));
        }

        double distance(const Vector& vec) const noexcept
        {
            Vector tmp;

            __m256d newVec = _mm256_sub_pd(_mm256_load_pd(mData), _mm256_load_pd(vec.mData));
            _mm256_store_pd(tmp.mData, newVec);

            return tmp.magnitude();
        }

        double distance(const double x, const double y, const double z, const double w) const noexcept
        {
            return distance(Vector(x, y, z, w));
        }

        bool isZero(const double precision) const noexcept
        {
            return (std::abs(mData[0]) < precision) && (std::abs(mData[1]) < precision) && (std::abs(mData[2]) < precision) && (std::abs(mData[3]) < precision);
        }

        bool isEqual(const Vector& vec, const double precision) const
        {
            return (std::abs(mData[0] - vec.mData[0]) < precision) && (std::abs(mData[1] - vec.mData[1]) < precision) && (std::abs(mData[2] - vec.mData[2]) < precision) &&
                   (std::abs(mData[3] - vec.mData[3]) < precision);
        }

        Vector& normalize() noexcept
        {
            __m256d data = _mm256_load_pd(mData);
            __m128d dot = mmDot(data);
            __m256d invMag = _mm256_invsqrt_pd(_mm256_set_m128d(dot, dot));
            _mm256_store_pd(mData, _mm256_mul_pd(data, invMag));

            return *this;
        }

        Vector& clamp(const double min, const double max) noexcept
        {
            __m256d tmp = _mm256_load_pd(mData);
            tmp = _mm256_max_pd(tmp, _mm256_set1_pd(min));
            tmp = _mm256_min_pd(tmp, _mm256_set1_pd(max));
            _mm256_store_pd(mData, tmp);

            return *this;
        }

        Vector& abs() noexcept
        {
            __m256d mask = _mm256_castsi256_pd(_mm256_set1_epi64x(static_cast<int64_t>(0x8000000000000000)));
            _mm256_store_pd(mData, _mm256_andnot_pd(mask, _mm256_load_pd(mData)));

            return *this;
        }

        Vector& neg() noexcept
        {
            __m256d mask = _mm256_castsi256_pd(_mm256_set1_epi64x(static_cast<int64_t>(0x8000000000000000)));
            _mm256_store_pd(mData, _mm256_xor_pd(_mm256_load_pd(mData), mask));

            return *this;
        }

        Vector& mad(double opMul, double opAdd) noexcept
        {
            __m256d tmp = _mm256_load_pd(mData);
            _mm256_store_pd(mData, _mm256_add_pd(_mm256_mul_pd(tmp, _mm256_set1_pd(opMul)), _mm256_set1_pd(opAdd)));

            return *this;
        }
    };

    static_assert(alignof(Vector3f) == 16, "For performance reasons, this class must be aligned to a 16 byte boundary");
    static_assert(alignof(Vector3d) == 32, "For performance reasons, this class must be aligned to a 32 byte boundary");
    static_assert(alignof(Vector4f) == 16, "For performance reasons, this class must be aligned to a 16 byte boundary");
    static_assert(alignof(Vector4d) == 32, "For performance reasons, this class must be aligned to a 32 byte boundary");
    static_assert(std::is_trivially_copyable_v<Vector3f>, "For performance reasons, this class should be trivially copyable");
    static_assert(std::is_trivially_copyable_v<Vector3d>, "For performance reasons, this class should be trivially copyable");
    static_assert(std::is_trivially_copyable_v<Vector4f>, "For performance reasons, this class should be trivially copyable");
    static_assert(std::is_trivially_copyable_v<Vector4d>, "For performance reasons, this class should be trivially copyable");

    template<typename TTargetType, size_t NTargetComponents>
    Vector<TTargetType, NTargetComponents> Vector<float, 3>::convert() const noexcept
    {
        static_assert(std::is_same_v<TTargetType, double> || std::is_same_v<TTargetType, float>, "Unsupported data type");
        static_assert(NTargetComponents == 3, "This overload only accepts 3 vector component conversions");

        if constexpr (std::is_same_v<TTargetType, float>)
        {
            Vector<float, 3> newVec;
            _mm_store_ps(newVec.data().data(), _mm_load_ps(mData));

            return newVec;
        }
        else
        {
            Vector<double, 3> newVec;
            _mm256_store_pd(newVec.data().data(), _mm256_cvtps_pd(_mm_load_ps(mData)));

            return newVec;
        }
    }

    template<typename TTargetType, size_t NTargetComponents>
    Vector<TTargetType, NTargetComponents> Vector<float, 3>::convert(TTargetType w) const noexcept
    {
        static_assert(std::is_same_v<TTargetType, double> || std::is_same_v<TTargetType, float>, "Unsupported data type");
        static_assert((NTargetComponents == 3) || (NTargetComponents == 4), "Can only convert between 3 or 4 vector components");

        if constexpr (NTargetComponents == 3)
        {
            if constexpr (std::is_same_v<TTargetType, float>)
            {
                Vector<float, 3> newVec;
                _mm_store_ps(newVec.data().data(), _mm_load_ps(mData));

                return newVec;
            }
            else
            {
                Vector<double, 3> newVec;
                _mm256_store_pd(newVec.data().data(), _mm256_cvtps_pd(_mm_load_ps(mData)));

                return newVec;
            }
        }
        else
        {
            if constexpr (std::is_same_v<TTargetType, float>)
            {
                Vector<float, 4> newVec;
                _mm_store_ps(newVec.data().data(), _mm_load_ps(mData));
                newVec.data()[3] = w;

                return newVec;
            }
            else
            {
                Vector<double, 4> newVec;
                _mm256_store_pd(newVec.data().data(), _mm256_cvtps_pd(_mm_load_ps(mData)));
                newVec.data()[3] = w;

                return newVec;
            }
        }
    }

    template<typename TTargetType, size_t NTargetComponents>
    Vector<TTargetType, NTargetComponents> Vector<double, 3>::convert() const noexcept
    {
        static_assert(std::is_same_v<TTargetType, double> || std::is_same_v<TTargetType, float>, "Unsupported data type");
        static_assert(NTargetComponents == 3, "This overload only accepts 3 vector component conversions");

        if constexpr (std::is_same_v<TTargetType, double>)
        {
            Vector<double, 3> newVec;
            _mm256_store_pd(newVec.data().data(), _mm256_load_pd(mData));

            return newVec;
        }
        else
        {
            Vector<float, 3> newVec;
            _mm_store_ps(newVec.data().data(), _mm256_cvtpd_ps(_mm256_load_pd(mData)));

            return newVec;
        }
    }

    template<typename TTargetType, size_t NTargetComponents>
    Vector<TTargetType, NTargetComponents> Vector<double, 3>::convert(TTargetType w) const noexcept
    {
        static_assert(std::is_same_v<TTargetType, double> || std::is_same_v<TTargetType, float>, "Unsupported data type");
        static_assert((NTargetComponents == 3) || (NTargetComponents == 4), "Can only convert between 3 or 4 vector components");

        if constexpr (NTargetComponents == 3)
        {
            if constexpr (std::is_same_v<TTargetType, double>)
            {
                Vector<double, 3> newVec;
                _mm256_store_pd(newVec.data().data(), _mm256_load_pd(mData));

                return newVec;
            }
            else
            {
                Vector<float, 3> newVec;
                _mm_store_ps(newVec.data().data(), _mm256_cvtpd_ps(_mm256_load_pd(mData)));

                return newVec;
            }
        }
        else
        {
            if constexpr (std::is_same_v<TTargetType, double>)
            {
                Vector<double, 4> newVec;
                _mm256_store_pd(newVec.data().data(), _mm256_load_pd(mData));
                newVec.data()[3] = w;

                return newVec;
            }
            else
            {
                Vector<float, 4> newVec;
                _mm_store_ps(newVec.data().data(), _mm256_cvtpd_ps(_mm256_load_pd(mData)));
                newVec.data()[3] = w;

                return newVec;
            }
        }
    }

    template<typename TTargetType, size_t NTargetComponents>
    Vector<TTargetType, NTargetComponents> Vector<float, 4>::convert() const noexcept
    {
        static_assert(std::is_same_v<TTargetType, double> || std::is_same_v<TTargetType, float>, "Unsupported data type");
        static_assert((NTargetComponents == 3) || (NTargetComponents == 4), "Can only convert between 3 or 4 vector components");

        // we only need to worry about the data type because the w component (in a 3 component vector) is ignored

        if constexpr (std::is_same_v<TTargetType, float>)
        {
            Vector<float, NTargetComponents> newVec;
            _mm_store_ps(newVec.data().data(), _mm_load_ps(mData));

            return newVec;
        }
        else
        {
            Vector<double, NTargetComponents> newVec;
            _mm256_store_pd(newVec.data().data(), _mm256_cvtps_pd(_mm_load_ps(mData)));

            return newVec;
        }
    }

    template<typename TTargetType, size_t NTargetComponents>
    Vector<TTargetType, NTargetComponents> Vector<double, 4>::convert() const noexcept
    {
        static_assert(std::is_same_v<TTargetType, double> || std::is_same_v<TTargetType, float>, "Unsupported data type");
        static_assert((NTargetComponents == 3) || (NTargetComponents == 4), "Can only convert between 3 or 4 vector components");

        // we only need to worry about the data type because the w component (in a 3 component vector) is ignored

        if constexpr (std::is_same_v<TTargetType, double>)
        {
            Vector<double, NTargetComponents> newVec;
            _mm256_store_pd(newVec.data().data(), _mm256_load_pd(mData));

            return newVec;
        }
        else
        {
            Vector<float, NTargetComponents> newVec;
            _mm_store_ps(newVec.data().data(), _mm256_cvtpd_ps(_mm256_load_pd(mData)));

            return newVec;
        }
    }
}
