#pragma once

#include "math.hpp"

#include <cmath>
#include <tuple>
#include <cstring>
#include <cassert>
#include <immintrin.h>
#include <xmmintrin.h>
#include <smmintrin.h>
#include <type_traits>

namespace hr
{
	template<typename TDataType, size_t NComponents>
	class Vector
	{
		TDataType mData[NComponents];

		static_assert(NComponents >= 1, "Number of components must be equal or greater than 1");
		static_assert(std::is_arithmetic<TDataType>::value, "Data type must be arithmetic (e.g.: float, unsigned char, etc.)");
		static_assert(std::is_trivially_copyable_v<TDataType>, "For performance reasons, the data type should be trivially copyable");

	public:
		using DataType = TDataType;
		static constexpr size_t NumComponents = NComponents;

	public:
		//static methods

		static TDataType calcDot(const Vector<TDataType, NComponents>& vec)
		{
			TDataType sum();
			for (size_t i = 0; i < NComponents; i++)
				sum += (vec.mData[i] * vec.mData[i]);

			return sum;
		}

		static TDataType calcDot(const Vector<TDataType, NComponents> &vec1, const Vector<TDataType, NComponents> &vec2)
		{
			TDataType sum();
			for (size_t i = 0; i < NComponents; i++)
				sum += (vec1.mData[i] * vec2.mData[i]);

			return sum;
		}

		//instance methods

		Vector() = default;

		explicit Vector(const TDataType scalar)
		{
			for (size_t i = 0; i < NComponents; i++)
				mData[i] = scalar;
		}

		explicit Vector(const TDataType* const values)
		{
			for (size_t i = 0; i < NComponents; i++)
				mData[i] = values[i];
		}

		TDataType* data()
		{
			return mData;
		}

		const TDataType* data() const
		{
			return mData;
		}

		TDataType& operator[] (const size_t index)
		{
			return mData[index % NComponents];
		}

		const TDataType& operator[] (const size_t index) const
		{
			return mData[index % NComponents];
		}

		Vector& operator=(const Vector& v)
		{
			for (size_t i = 0; i < NComponents; i++)
				mData[i] = v[i];
			return *this;
		}

		Vector& operator=(const TDataType* values)
		{
			for (size_t i = 0; i < NComponents; i++)
				mData[i] = values[i];
			return *this;
		}

		Vector& operator=(const TDataType scalar)
		{
			for (size_t i = 0; i < NComponents; i++)
				mData[i] = scalar;
			return *this;
		}

		void operator+=(const Vector& v)
		{
			for (size_t i = 0; i < NComponents; i++)
				mData[i] += v[i];
		}

		void operator-=(const Vector& v)
		{
			for (size_t i = 0; i < NComponents; i++)
				mData[i] -= v[i];
		}

		void operator*=(const Vector& v)
		{
			for (size_t i = 0; i < NComponents; i++)
				mData[i] *= v[i];
		}

		void operator/=(const Vector& v)
		{
			for (size_t i = 0; i < NComponents; i++)
				mData[i] /= v[i];
		}

		void operator+=(const TDataType* const values)
		{
			for (size_t i = 0; i < NComponents; i++)
				mData[i] += values[i];
		}

		void operator-=(const TDataType* const values)
		{
			for (size_t i = 0; i < NComponents; i++)
				mData[i] -= values[i];
		}

		void operator*=(const TDataType* const values)
		{
			for (size_t i = 0; i < NComponents; i++)
				mData[i] *= values[i];
		}

		void operator/=(const TDataType* const values)
		{
			for (size_t i = 0; i < NComponents; i++)
				mData[i] /= values[i];
		}

		void operator+=(const TDataType scalar)
		{
			for (size_t i = 0; i < NComponents; i++)
				mData[i] += scalar;
		}

		void operator-=(const TDataType scalar)
		{
			for (size_t i = 0; i < NComponents; i++)
				mData[i] -= scalar;
		}

		void operator*=(const TDataType scalar)
		{
			for (size_t i = 0; i < NComponents; i++)
				mData[i] *= scalar;
		}

		void operator/=(const TDataType scalar)
		{
			for (size_t i = 0; i < NComponents; i++)
				mData[i] /= scalar;
		}

		Vector operator+(const Vector& vec) const
		{
			Vector result;
			for (size_t i = 0; i < NComponents; i++)
				result[i] = mData[i] + vec[i];

			return result;
		}

		Vector operator-(const Vector& vec) const
		{
			Vector result;
			for (size_t i = 0; i < NComponents; i++)
				result[i] = mData[i] - vec[i];

			return result;
		}

		Vector operator*(const Vector& vec) const
		{
			Vector result;
			for (size_t i = 0; i < NComponents; i++)
				result[i] = mData[i] * vec[i];

			return result;
		}

		Vector operator/(const Vector& vec) const
		{
			Vector result;
			for (size_t i = 0; i < NComponents; i++)
				result[i] = mData[i] / vec[i];

			return result;
		}

		Vector operator+(const TDataType* const values) const
		{
			Vector result;
			for (size_t i = 0; i < NComponents; i++)
				result[i] = mData[i] + values[i];

			return result;
		}

		Vector operator-(const TDataType* const values) const
		{
			Vector result;
			for (size_t i = 0; i < NComponents; i++)
				result[i] = mData[i] - values[i];

			return result;
		}

		Vector operator*(const TDataType* const values) const
		{
			Vector result;
			for (size_t i = 0; i < NComponents; i++)
				result[i] = mData[i] * values[i];

			return result;
		}

		Vector operator/(const TDataType* const values) const
		{
			Vector result;
			for (size_t i = 0; i < NComponents; i++)
				result[i] = mData[i] / values[i];

			return result;
		}

		Vector operator+(const TDataType scalar) const
		{
			Vector result;
			for (size_t i = 0; i < NComponents; i++)
				result[i] = mData[i] + scalar;

			return result;
		}

		Vector operator-(const TDataType scalar) const
		{
			Vector result;
			for (size_t i = 0; i < NComponents; i++)
				result[i] = mData[i] - scalar;

			return result;
		}

		Vector operator*(const TDataType scalar) const
		{
			Vector result;
			for (size_t i = 0; i < NComponents; i++)
				result[i] = mData[i] * scalar;

			return result;
		}

		Vector operator/(const TDataType scalar) const
		{
			Vector result;
			for (size_t i = 0; i < NComponents; i++)
				result[i] = mData[i] / scalar;

			return result;
		}

		bool operator==(const Vector &vec) const
		{
			for (size_t i = 0; i < NComponents; i++)
			{
				if (mData[i] != vec.mData[i])
					return false;
			}

			return true;
		}

		bool operator==(const TDataType* const values) const
		{
			for (size_t i = 0; i < NComponents; i++)
			{
				if (mData[i] != values[i])
					return false;
			}

			return true;
		}

		Vector& set(const Vector &vec)
		{
			for (size_t i = 0; i < NComponents; i++)
				mData[i] = vec[i];

			return *this;
		}

		Vector& set(const TDataType* const values)
		{
			for (size_t i = 0; i < NComponents; i++)
				mData[i] = values[i];

			return *this;
		}

		Vector& set(const TDataType scalar)
		{
			for (size_t i = 0; i < NComponents; i++)
				mData[i] = scalar;

			return *this;
		}

		Vector& clamp(const TDataType min, const TDataType max)
		{
			for (size_t i = 0; i < NComponents; i++)
			{
				mData[i] = (mData[i] < min) ? min : ((mData[i] > max) ? max : mData[i]);
			}

			return *this;
		}

		void write(TDataType* const dest) const
		{
			for (size_t i = 0; i < NComponents; i++)
				dest[i] = mData[i];
		}

		TDataType getDot() const
		{
			return Vector::calcDot(*this);
		}

		TDataType getDot(const Vector &vec) const
		{
			return Vector::calcDot(*this, vec);
		}
	};

	using Vector3f = Vector<float, 3>;
	using Vector3d = Vector<double, 3>;
	using Vector4f = Vector<float, 4>;

	template<>
	class alignas(16) Vector<float, 3>
	{
		float mData[4];

	public:
		using DataType = float;
		static constexpr size_t NumComponents{3};

	public:
		//static methods

		static float calcDot(const Vector& vec)
		{
			__m128 vecTmp = _mm_load_ps(vec.mData);
			return _mm_cvtss_f32(_mm_dp_ps(vecTmp, vecTmp, 0x70 | 0xF));
		}

		static float calcDot(const Vector& vec1, const Vector& vec2)
		{
			return _mm_cvtss_f32(_mm_dp_ps(_mm_load_ps(vec1.mData), _mm_load_ps(vec2.mData), 0x70 | 0xF));
		}

		static float calcMagnitude(const Vector &vec)
		{
			__m128 vecTmp = _mm_load_ps(vec.mData);
			return _mm_cvtss_f32(_mm_sqrt_ss(_mm_dp_ps(vecTmp, vecTmp, 0x70 | 0xF)));
		}

		static float calcMagnitudeInverse(const Vector &vec)
		{
			__m128 vecTmp = _mm_load_ps(vec.mData);
			return _mm_cvtss_f32(_mm_rsqrt_ss(_mm_dp_ps(vecTmp, vecTmp, 0x70 | 0xF)));
		}

		static float calcDistance(const Vector &vec1, const Vector &vec2)
		{
			__m128 vecTmp = _mm_sub_ps(_mm_load_ps(vec1.mData), _mm_load_ps(vec2.mData));
			return _mm_cvtss_f32(_mm_sqrt_ss(_mm_dp_ps(vecTmp, vecTmp, 0x70 | 0xF)));
		}

		static Vector calcNormalize(const Vector& vec)
		{
			Vector result;

			__m128 vecTmp = _mm_load_ps(vec.mData);
			__m128 vecMag = _mm_rsqrt_ps(_mm_dp_ps(vecTmp, vecTmp, 0xF0 | 0xF));
			_mm_store_ps(result.mData, _mm_mul_ps(vecTmp, vecMag));

			return result;
		}

		static Vector calcMin(const Vector& vec1, const Vector& vec2)
		{
			Vector result;
			_mm_store_ps(result.mData, _mm_min_ps(_mm_load_ps(vec1.mData), _mm_load_ps(vec2.mData)));
			return result;
		}

		static Vector calcMax(const Vector& vec1, const Vector& vec2)
		{
			Vector result;
			_mm_store_ps(result.mData, _mm_max_ps(_mm_load_ps(vec1.mData), _mm_load_ps(vec2.mData)));
			return result;
		}

		static std::tuple<Vector, Vector> calcMinMax(const Vector& vec1, const Vector& vec2)
		{
			Vector resultMin, resultMax;

			__m128 v1 = _mm_load_ps(vec1.mData);
			__m128 v2 = _mm_load_ps(vec2.mData);
			_mm_store_ps(resultMin.mData, _mm_min_ps(v1, v2));
			_mm_store_ps(resultMax.mData, _mm_max_ps(v1, v2));
			return { resultMin , resultMax };
		}

		static Vector calcPointAt(const Vector& origin, const Vector& direction, const float t)
		{
			assert(t >= 0.0f && t <= 1.0f);

			Vector result;
			__m128 tmp;

			tmp = _mm_mul_ps(_mm_load_ps(direction.mData), _mm_load_ps1(&t));
			tmp = _mm_add_ps(tmp, _mm_load_ps(origin.mData));

			_mm_store_ps(result.mData, tmp);
			return result;
		}

		static Vector calcLinear(const Vector &from, const Vector &to, const float t)
		{
			assert(t >= 0.0f && t <= 1.0f);

			Vector result;
			__m128 tmp;

			tmp = _mm_mul_ps(_mm_load_ps(from.mData), _mm_set_ps1(1.0f - t));
			tmp = _mm_add_ps(tmp, _mm_mul_ps(_mm_load_ps(to.mData), _mm_load_ps1(&t)));

			_mm_store_ps(result.mData, tmp);
			return result;
		}

		static Vector calcProject(const Vector& vecA, const Vector& vecB)
		{
			// returns the vector projection of vecA onto vecB

			auto m = vecB.getMagnitude();
			m = vecA.getDot(vecB) / (m * m);
			return (vecB * m);
		}

		static Vector calcReflect(const Vector& vec, const Vector& planeNormal)
		{
			return vec - (calcProject(vec, planeNormal) * 2.0f);
		}

		static Vector calcCrossProduct(const Vector& p, const Vector& q)
		{
			__m128 vec1 = _mm_load_ps(p.mData);
			__m128 vec2 = _mm_load_ps(q.mData);

			__m128 xa = _mm_mul_ps(_mm_shuffle_ps(vec1, vec1, _MM_SHUFFLE(3, 0, 2, 1)), _mm_shuffle_ps(vec2, vec2, _MM_SHUFFLE(3, 1, 0, 2)));
			__m128 xb = _mm_mul_ps(_mm_shuffle_ps(vec1, vec1, _MM_SHUFFLE(3, 1, 0, 2)), _mm_shuffle_ps(vec2, vec2, _MM_SHUFFLE(3, 0, 2, 1)));

			Vector result;
			_mm_store_ps(result.mData, _mm_sub_ps(xa, xb));
			return result;
		}

		static Vector calcLerp(const Vector& from, const Vector& to, const float t)
		{
			assert(t >= 0.0f && t <= 1.0f);

			__m128 tmp;

			tmp = _mm_mul_ps(_mm_load_ps(from.mData), _mm_set_ps1(1.0f - t));
			tmp = _mm_add_ps(tmp, _mm_mul_ps(_mm_load_ps(to.mData), _mm_load_ps1(&t)));

			Vector result;
			_mm_store_ps(result.mData, tmp);
			return result;
		}

		static Vector fromSpherical(float rad, float theta, float phi)
		{
			/*
			* This uses the ISO convention: radius r, inclination theta (radians) and azimuth phi (also radians)
			* Also:
			*	- theta: the angle in the XY plane from the X axis
			*	- phi: the angle from the positive Z axis to the vector
			*/

			auto thetaTrig = Math::sinCos(theta);
			auto phiTrig = Math::sinCos(phi);

			Vector result;
			result.mData[0] = rad * std::get<0>(thetaTrig) * std::get<1>(phiTrig);
			result.mData[1] = rad * std::get<0>(thetaTrig) * std::get<0>(phiTrig);
			result.mData[2] = rad * std::get<1>(thetaTrig);

			return result;
		}

		static Vector evalSplineCatmullRom(const Vector& p1, const Vector& p2, const Vector& p3, const Vector& p4, float t)
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

		static Vector evalSplineHermite(const Vector& p1, const Vector& p2, const Vector& p3, const Vector& p4, float t)
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

		//instance methods

		constexpr Vector() = default;

		explicit constexpr Vector(const float scalar)
			: mData{ scalar, scalar, scalar, scalar }
		{ }

		explicit constexpr Vector(const float vx, const float vy, const float vz)
			: mData{ vx, vy, vz, 0.0f }
		{ }

		explicit constexpr Vector(const float v[3])
			: mData{ v[0], v[1], v[2], 0.0f }
		{ }

		explicit Vector(const __m128 vecDat)
		{
			_mm_store_ps(mData, vecDat);
		}

		constexpr float* data()
		{
			return mData;
		}

		constexpr const float* data() const
		{
			return mData;
		}

		constexpr float& operator[] (const size_t index)
		{
			return mData[index % 3];
		}

		constexpr const float& operator[] (const size_t index) const
		{
			return mData[index % 3];
		}

		Vector& operator=(const float v[3])
		{
			std::memcpy(mData, v, sizeof(float) * 3);
			return *this;
		}

		Vector& operator=(const float n)
		{
			_mm_store_ps(mData, _mm_load_ps1(&n));
			return *this;
		}

		void operator+=(const Vector& v)
		{
			_mm_store_ps(mData, _mm_add_ps(_mm_load_ps(mData), _mm_load_ps(v.mData)));
		}

		void operator-=(const Vector& v)
		{
			_mm_store_ps(mData, _mm_sub_ps(_mm_load_ps(mData), _mm_load_ps(v.mData)));
		}

		void operator*=(const Vector& v)
		{
			_mm_store_ps(mData, _mm_mul_ps(_mm_load_ps(mData), _mm_load_ps(v.mData)));
		}

		void operator/=(const Vector& v)
		{
			_mm_store_ps(mData, _mm_div_ps(_mm_load_ps(mData), _mm_load_ps(v.mData)));
		}

		constexpr void operator+=(const float v[3])
		{
			mData[0] += v[0];
			mData[1] += v[1];
			mData[2] += v[2];
		}

		constexpr void operator-=(const float v[3])
		{
			mData[0] -= v[0];
			mData[1] -= v[1];
			mData[2] -= v[2];
		}

		constexpr void operator*=(const float v[3])
		{
			mData[0] *= v[0];
			mData[1] *= v[1];
			mData[2] *= v[2];
		}

		constexpr void operator/=(const float v[3])
		{
			mData[0] /= v[0];
			mData[1] /= v[1];
			mData[2] /= v[2];
		}

		void operator+=(const float n)
		{
			_mm_store_ps(mData, _mm_add_ps(_mm_load_ps(mData), _mm_load_ps1(&n)));
		}

		void operator-=(const float n)
		{
			_mm_store_ps(mData, _mm_sub_ps(_mm_load_ps(mData), _mm_load_ps1(&n)));
		}

		void operator*=(const float n)
		{
			_mm_store_ps(mData, _mm_mul_ps(_mm_load_ps(mData), _mm_load_ps1(&n)));
		}

		void operator/=(const float n)
		{
			_mm_store_ps(mData, _mm_div_ps(_mm_load_ps(mData), _mm_load_ps1(&n)));
		}

		Vector operator-() const
		{
			return Vector(_mm_mul_ps(_mm_load_ps(mData), _mm_set_ps1(-1.0f)));
		}

		Vector operator+(const Vector &vec) const
		{
			return Vector(_mm_add_ps(_mm_load_ps(mData), _mm_load_ps(vec.mData)));
		}

		Vector operator-(const Vector &vec) const
		{
			return Vector(_mm_sub_ps(_mm_load_ps(mData), _mm_load_ps(vec.mData)));
		}

		Vector operator*(const Vector &vec) const
		{
			return Vector(_mm_mul_ps(_mm_load_ps(mData), _mm_load_ps(vec.mData)));
		}

		Vector operator/(const Vector &vec) const
		{
			return Vector(_mm_div_ps(_mm_load_ps(mData), _mm_load_ps(vec.mData)));
		}

		Vector operator+(const float vec[3]) const
		{
			return Vector(mData[0] + vec[0], mData[1] + vec[1], mData[2] + vec[2]);
		}

		Vector operator-(const float vec[3]) const
		{
			return Vector(mData[0] - vec[0], mData[1] - vec[1], mData[2] - vec[2]);
		}

		Vector operator*(const float vec[3]) const
		{
			return Vector(mData[0] * vec[0], mData[1] * vec[1], mData[2] * vec[2]);
		}

		Vector operator/(const float vec[3]) const
		{
			return Vector(mData[0] / vec[0], mData[1] / vec[1], mData[2] / vec[2]);
		}

		Vector operator+(const float n) const
		{
			return Vector(_mm_add_ps(_mm_load_ps(mData), _mm_load_ps1(&n)));
		}

		Vector operator-(const float n) const
		{
			return Vector(_mm_sub_ps(_mm_load_ps(mData), _mm_load_ps1(&n)));
		}

		Vector operator*(const float n) const
		{
			return Vector(_mm_mul_ps(_mm_load_ps(mData), _mm_load_ps1(&n)));
		}

		Vector operator/(const float n) const
		{
			return Vector(_mm_div_ps(_mm_load_ps(mData), _mm_load_ps1(&n)));
		}

		bool operator==(const Vector &vec) const
		{
			if (Math::isZero(mData[0] - vec.mData[0]) && Math::isZero(mData[1] - vec.mData[1]) && Math::isZero(mData[2] - vec.mData[2]))
				return true;
			return false;
		}

		bool operator==(const float vec[3]) const
		{
			if (Math::isZero(mData[0] - vec[0]) && Math::isZero(mData[1] - vec[1]) && Math::isZero(mData[2] - vec[2]))
				return true;
			return false;
		}

		template<typename TTargetType>
		Vector<TTargetType, 3> convert() const;

		Vector& set(const Vector &vec)
		{
			_mm_store_ps(mData, _mm_load_ps(vec.mData));
			return *this;
		}

		Vector& set(const float vec[3])
		{
			std::memcpy(mData, vec, sizeof(float) * 3);
			return *this;
		}

		Vector& set(const float vx, const float vy, const float vz)
		{
			mData[0] = vx;
			mData[1] = vy;
			mData[2] = vz;
			return *this;
		}

		Vector& set(const float val)
		{
			_mm_store_ps(mData, _mm_load_ps1(&val));
			return *this;
		}

		void write(float dest[3]) const
		{
			std::memcpy(dest, mData, sizeof(float) * 3);
		}

		float getDot() const
		{
			return Vector::calcDot(*this);
		}

		float getDot(const Vector &vec) const
		{
			return Vector::calcDot(*this, vec);
		}

		float getDot(const float vec[3]) const
		{
			return ((mData[0] * vec[0]) + (mData[1] * vec[1]) + (mData[2] * vec[2]));
		}

		float getDot(const float x, const float y, const float z) const
		{
			return ((mData[0] * x) + (mData[1] * y) + (mData[2] * z));
		}

		float getMagnitude() const
		{
			return Vector::calcMagnitude(*this);
		}

		float getMagnitudeInv() const
		{
			return Vector::calcMagnitudeInverse(*this);
		}

		float getDistance(const Vector &vec) const
		{
			return Vector::calcDistance(*this, vec);
		}

		float getDistance(const float x, const float y, const float z) const
		{
			return Vector::calcDistance(*this, Vector(x, y, z));
		}

		float getDistance(const float vec[3]) const
		{
			return Vector::calcDistance(*this, Vector(vec));
		}

		bool isZero(const float precision) const
		{
			return ((std::abs(mData[0]) < precision) && (std::abs(mData[1]) < precision) && (std::abs(mData[2]) < precision));
		}

		bool isEqual(const Vector &vec, const float precision) const
		{
			return ((std::abs(mData[0] - vec.mData[0]) < precision) && (std::abs(mData[1] - vec.mData[1]) < precision) && (std::abs(mData[2] - vec.mData[2]) < precision));
		}

		Vector& normalize()
		{
			__m128 vecTmp = _mm_load_ps(mData);
			__m128 vecMag = _mm_rsqrt_ps(_mm_dp_ps(vecTmp, vecTmp, 0x70 | 0xF));
			_mm_store_ps(mData, _mm_mul_ps(vecTmp, vecMag));

			return *this;
		}

		Vector& clamp(const float min, const float max)
		{
			__m128 tmp = _mm_load_ps(mData);
			tmp = _mm_max_ps(tmp, _mm_load_ps1(&min));
			tmp = _mm_min_ps(tmp, _mm_load_ps1(&max));
			_mm_store_ps(mData, tmp);

			return *this;
		}

		Vector& abs()
		{
			__m128 mask = _mm_castsi128_ps(_mm_set1_epi32(0x7FFFFFFF));
			_mm_store_ps(mData, _mm_and_ps(_mm_load_ps(mData), mask));

			return *this;
		}

		Vector& neg()
		{
			__m128 mask = _mm_castsi128_ps(_mm_set1_epi32(0x80000000));
			_mm_store_ps(mData, _mm_xor_ps(_mm_load_ps(mData), mask));

			return *this;
		}

		Vector& mad(float opMul, float opAdd)
		{
			__m128 tmp = _mm_load_ps(mData);
			_mm_store_ps(mData, _mm_add_ps(_mm_mul_ps(tmp, _mm_load_ps1(&opMul)), _mm_load_ps1(&opAdd)));

			return *this;
		}

		Vector project(const Vector& vec) const
		{
			return Vector::calcProject(*this, vec);
		}

		Vector reflect(const Vector& planeNormal) const
		{
			return Vector::calcReflect(*this, planeNormal);
		}

		Vector crossProduct(const Vector &vec) const
		{
			Vector result;

			__m128 vec1 = _mm_load_ps(mData);
			__m128 vec2 = _mm_load_ps(vec.mData);

			__m128 xa = _mm_mul_ps(_mm_shuffle_ps(vec1, vec1, _MM_SHUFFLE(3, 0, 2, 1)), _mm_shuffle_ps(vec2, vec2, _MM_SHUFFLE(3, 1, 0, 2)));
			__m128 xb = _mm_mul_ps(_mm_shuffle_ps(vec1, vec1, _MM_SHUFFLE(3, 1, 0, 2)), _mm_shuffle_ps(vec2, vec2, _MM_SHUFFLE(3, 0, 2, 1)));

			_mm_store_ps(result.mData, _mm_sub_ps(xa, xb));
			return result;
		}

		void storeInterpolate(const Vector &to, const float t)
		{
			__m128 tmp;

			tmp = _mm_mul_ps(_mm_load_ps(mData), _mm_set_ps1(1.0f - t));
			tmp = _mm_add_ps(tmp, _mm_mul_ps(_mm_load_ps(to.mData), _mm_load_ps1(&t)));
			_mm_store_ps(mData, tmp);
		}

		void storeInterpolateNormals(const Vector &n1, const Vector &n2, const float t)
		{
			float a = acos(n1.getDot(n2));
			float sinA = 1.0f / (sinf(a) + 0.00001f);

			float t0 = sin((1.0f - t)*a);
			float t1 = sin(t*a);

			mData[0] = (t0 * n1.mData[0] + t1 * n2.mData[0]) * sinA;
			mData[1] = (t0 * n1.mData[1] + t1 * n2.mData[1]) * sinA;
			mData[2] = (t0 * n1.mData[2] + t1 * n2.mData[2]) * sinA;
		}

		void storeClosestInSegment(const Vector &point, const Vector &p1, const Vector &p2)
		{
			Vector lineDir = p2 - p1;

			float t = lineDir.getDot();
			if (Math::isZero(t))
			{
				mData[0] = p1.mData[0];
				mData[1] = p1.mData[1];
				mData[2] = p1.mData[2];
				return;
			}

			t = ((point.mData[0] - p1.mData[0])*lineDir.mData[0] + (point.mData[1] - p1.mData[1])*lineDir.mData[1] + (point.mData[2] - p1.mData[2])*lineDir.mData[2]) / t;
			t = Math::fClamp(t, 0.0f, 1.0f);

			mData[0] = p1.mData[0] + (t * lineDir.mData[0]);
			mData[1] = p1.mData[1] + (t * lineDir.mData[1]);
			mData[2] = p1.mData[2] + (t * lineDir.mData[2]);
		}
	};

	template<>
	class alignas(32) Vector<double, 3>
	{
		double mData[4];

	public:
		using DataType = double;
		static constexpr size_t NumComponents{3};

	public:
		//static methods

		static double calcDot(const Vector& vec)
		{
			__m256d data = _mm256_load_pd(vec.mData);
			data = _mm256_mul_pd(data, data);

			__m128d low = _mm256_castpd256_pd128(data);
			__m128d high = _mm_shuffle_pd(_mm256_extractf128_pd(data, 1), _mm_setzero_pd(), 0b10); //must zero out w

			__m128d sum = _mm_add_pd(low, high);
			__m128d dot = _mm_add_pd(sum, _mm_shuffle_pd(sum, sum, 0b01));
			return _mm_cvtsd_f64(dot);
		}

		static double calcDot(const Vector& vec1, const Vector& vec2)
		{
			__m256d data = _mm256_mul_pd(_mm256_load_pd(vec1.mData), _mm256_load_pd(vec2.mData));

			__m128d low = _mm256_castpd256_pd128(data);
			__m128d high = _mm_shuffle_pd(_mm256_extractf128_pd(data, 1), _mm_setzero_pd(), 0b10); //must zero out w

			__m128d sum = _mm_add_pd(low, high);
			__m128d dot = _mm_add_pd(sum, _mm_shuffle_pd(sum, sum, 0b01));
			return _mm_cvtsd_f64(dot);
		}

		static double calcMagnitude(const Vector& vec)
		{
			__m128d temp = _mm_set1_pd(vec.getDot());
			return _mm_cvtsd_f64(_mm_sqrt_pd(temp));
		}

		static double calcMagnitudeInverse(const Vector& vec)
		{
			__m128d temp = _mm_set1_pd(vec.getDot());
			return _mm_cvtsd_f64(_mm_invsqrt_pd(temp));
		}

		static double calcDistance(const Vector& vec1, const Vector& vec2)
		{
			Vector tmp;

			__m256d newVec = _mm256_sub_pd(_mm256_load_pd(vec1.mData), _mm256_load_pd(vec2.mData));
			_mm256_storeu_pd(tmp.mData, newVec);

			return calcMagnitude(tmp);
		}

		static Vector calcNormalize(const Vector& vec)
		{
			__m128d temp = _mm_invsqrt_pd(_mm_set1_pd(vec.getDot()));
			__m256d invMag = _mm256_permute2f128_pd(_mm256_castpd128_pd256(temp), _mm256_castpd128_pd256(temp), 0x20);

			Vector result;
			_mm256_storeu_pd(result.mData, _mm256_mul_pd(_mm256_load_pd(vec.mData), invMag));
			return result;
		}

		static Vector calcMin(const Vector& vec1, const Vector& vec2)
		{
			Vector result;
			_mm256_storeu_pd(result.mData, _mm256_min_pd(_mm256_load_pd(vec1.mData), _mm256_load_pd(vec2.mData)));
			return result;
		}

		static Vector calcMax(const Vector& vec1, const Vector& vec2)
		{
			Vector result;
			_mm256_storeu_pd(result.mData, _mm256_max_pd(_mm256_load_pd(vec1.mData), _mm256_load_pd(vec2.mData)));
			return result;
		}

		static std::tuple<Vector, Vector> calcMinMax(const Vector& vec1, const Vector& vec2)
		{
			Vector resultMin, resultMax;

			__m256d v1 = _mm256_load_pd(vec1.mData);
			__m256d v2 = _mm256_load_pd(vec2.mData);
			_mm256_storeu_pd(resultMin.mData, _mm256_min_pd(v1, v2));
			_mm256_storeu_pd(resultMax.mData, _mm256_max_pd(v1, v2));
			return { resultMin , resultMax };
		}

		static Vector calcPointAt(const Vector& origin, const Vector& direction, const double t)
		{
			assert(t >= 0.0 && t <= 1.0);

			Vector result;
			__m256d tmp;

			tmp = _mm256_mul_pd(_mm256_load_pd(direction.mData), _mm256_set1_pd(t));
			tmp = _mm256_add_pd(tmp, _mm256_load_pd(origin.mData));

			_mm256_storeu_pd(result.mData, tmp);
			return result;
		}

		static Vector calcLinear(const Vector& from, const Vector& to, const double t)
		{
			assert(t >= 0.0 && t <= 1.0);

			Vector result;
			__m256d tmp;

			tmp = _mm256_mul_pd(_mm256_load_pd(from.mData), _mm256_set1_pd(1.0 - t));
			tmp = _mm256_add_pd(tmp, _mm256_mul_pd(_mm256_load_pd(to.mData), _mm256_set1_pd(t)));

			_mm256_storeu_pd(result.mData, tmp);
			return result;
		}

		static Vector calcProject(const Vector& vecA, const Vector& vecB)
		{
			// returns the vector projection of vecA onto vecB

			auto m = vecB.getMagnitude();
			m = vecA.getDot(vecB) / (m * m);
			return (vecB * m);
		}

		static Vector calcReflect(const Vector& vec, const Vector& planeNormal)
		{
			return vec - (calcProject(vec, planeNormal) * 2.0);
		}

		static Vector calcCrossProduct(const Vector& p, const Vector& q)
		{
			__m256d vec1 = _mm256_load_pd(p.mData);
			__m256d vec2 = _mm256_load_pd(q.mData);

			__m256d xa = _mm256_mul_pd(_mm256_permute4x64_pd(vec1, _MM_SHUFFLE(3, 0, 2, 1)), _mm256_permute4x64_pd(vec2, _MM_SHUFFLE(3, 1, 0, 2)));
			__m256d xb = _mm256_mul_pd(_mm256_permute4x64_pd(vec1, _MM_SHUFFLE(3, 1, 0, 2)), _mm256_permute4x64_pd(vec2, _MM_SHUFFLE(3, 0, 2, 1)));

			Vector result;
			_mm256_storeu_pd(result.mData, _mm256_sub_pd(xa, xb));
			return result;
		}

		static Vector calcLerp(const Vector& from, const Vector& to, const double t)
		{
			assert(t >= 0.0 && t <= 1.0);

			__m256d tmp;

			tmp = _mm256_mul_pd(_mm256_load_pd(from.mData), _mm256_set1_pd(1.0 - t));
			tmp = _mm256_add_pd(tmp, _mm256_mul_pd(_mm256_load_pd(to.mData), _mm256_set1_pd(t)));

			Vector result;
			_mm256_storeu_pd(result.mData, tmp);
			return result;
		}

		static Vector fromSpherical(double rad, double theta, double phi)
		{
			/*
			* This uses the ISO convention: radius r, inclination theta (radians) and azimuth phi (also radians)
			* Also:
			*	- theta: the angle in the XY plane from the X axis
			*	- phi: the angle from the positive Z axis to the vector
			*/

			auto thetaTrig = std::make_tuple(Math::sin(theta), Math::cos(theta));
			auto phiTrig = std::make_tuple(Math::sin(phi), Math::cos(phi));

			Vector result;
			result.mData[0] = rad * std::get<0>(thetaTrig) * std::get<1>(phiTrig);
			result.mData[1] = rad * std::get<0>(thetaTrig) * std::get<0>(phiTrig);
			result.mData[2] = rad * std::get<1>(thetaTrig);

			return result;
		}

		//instance methods

		constexpr Vector() = default;

		explicit constexpr Vector(const Vector3f& vec)
			: mData{ vec[0], vec[1], vec[2], 0.0 }
		{ }

		explicit constexpr Vector(const double scalar)
			: mData{ scalar, scalar, scalar, scalar }
		{ }

		explicit constexpr Vector(const double vx, const double vy, const double vz)
			: mData{ vx, vy, vz, 0.0 }
		{ }

		explicit constexpr Vector(const double v[3])
			: mData{ v[0], v[1], v[2], 0.0 }
		{ }

		explicit Vector(const __m256d vecDat)
		{
			_mm256_storeu_pd(mData, vecDat);
		}

		constexpr double* data()
		{
			return mData;
		}

		constexpr const double* data() const
		{
			return mData;
		}

		constexpr double& operator[] (const size_t index)
		{
			return mData[index % 3];
		}

		constexpr const double& operator[] (const size_t index) const
		{
			return mData[index % 3];
		}

		Vector& operator=(const double v[3])
		{
			std::memcpy(mData, v, sizeof(double) * 3);
			return *this;
		}

		Vector& operator=(const double n)
		{
			_mm256_storeu_pd(mData, _mm256_set1_pd(n));
			return *this;
		}

		void operator+=(const Vector& v)
		{
			_mm256_storeu_pd(mData, _mm256_add_pd(_mm256_load_pd(mData), _mm256_load_pd(v.mData)));
		}

		void operator-=(const Vector& v)
		{
			_mm256_storeu_pd(mData, _mm256_sub_pd(_mm256_load_pd(mData), _mm256_load_pd(v.mData)));
		}

		void operator*=(const Vector& v)
		{
			_mm256_storeu_pd(mData, _mm256_mul_pd(_mm256_load_pd(mData), _mm256_load_pd(v.mData)));
		}

		void operator/=(const Vector& v)
		{
			_mm256_storeu_pd(mData, _mm256_div_pd(_mm256_load_pd(mData), _mm256_load_pd(v.mData)));
		}

		constexpr void operator+=(const double v[3])
		{
			mData[0] += v[0];
			mData[1] += v[1];
			mData[2] += v[2];
		}

		constexpr void operator-=(const double v[3])
		{
			mData[0] -= v[0];
			mData[1] -= v[1];
			mData[2] -= v[2];
		}

		constexpr void operator*=(const double v[3])
		{
			mData[0] *= v[0];
			mData[1] *= v[1];
			mData[2] *= v[2];
		}

		constexpr void operator/=(const double v[3])
		{
			mData[0] /= v[0];
			mData[1] /= v[1];
			mData[2] /= v[2];
		}

		void operator+=(const double n)
		{
			_mm256_storeu_pd(mData, _mm256_add_pd(_mm256_load_pd(mData), _mm256_set1_pd(n)));
		}

		void operator-=(const double n)
		{
			_mm256_storeu_pd(mData, _mm256_sub_pd(_mm256_load_pd(mData), _mm256_set1_pd(n)));
		}

		void operator*=(const double n)
		{
			_mm256_storeu_pd(mData, _mm256_mul_pd(_mm256_load_pd(mData), _mm256_set1_pd(n)));
		}

		void operator/=(const double n)
		{
			_mm256_storeu_pd(mData, _mm256_div_pd(_mm256_load_pd(mData), _mm256_set1_pd(n)));
		}

		Vector operator-() const
		{
			return Vector(_mm256_mul_pd(_mm256_load_pd(mData), _mm256_set1_pd(-1.0)));
		}

		Vector operator+(const Vector& vec) const
		{
			return Vector(_mm256_add_pd(_mm256_load_pd(mData), _mm256_load_pd(vec.mData)));
		}

		Vector operator-(const Vector& vec) const
		{
			return Vector(_mm256_sub_pd(_mm256_load_pd(mData), _mm256_load_pd(vec.mData)));
		}

		Vector operator*(const Vector& vec) const
		{
			return Vector(_mm256_mul_pd(_mm256_load_pd(mData), _mm256_load_pd(vec.mData)));
		}

		Vector operator/(const Vector& vec) const
		{
			return Vector(_mm256_div_pd(_mm256_load_pd(mData), _mm256_load_pd(vec.mData)));
		}

		Vector operator+(const double vec[3]) const
		{
			return Vector(mData[0] + vec[0], mData[1] + vec[1], mData[2] + vec[2]);
		}

		Vector operator-(const double vec[3]) const
		{
			return Vector(mData[0] - vec[0], mData[1] - vec[1], mData[2] - vec[2]);
		}

		Vector operator*(const double vec[3]) const
		{
			return Vector(mData[0] * vec[0], mData[1] * vec[1], mData[2] * vec[2]);
		}

		Vector operator/(const double vec[3]) const
		{
			return Vector(mData[0] / vec[0], mData[1] / vec[1], mData[2] / vec[2]);
		}

		Vector operator+(const double n) const
		{
			return Vector(_mm256_add_pd(_mm256_load_pd(mData), _mm256_set1_pd(n)));
		}

		Vector operator-(const double n) const
		{
			return Vector(_mm256_sub_pd(_mm256_load_pd(mData), _mm256_set1_pd(n)));
		}

		Vector operator*(const double n) const
		{
			return Vector(_mm256_mul_pd(_mm256_load_pd(mData), _mm256_set1_pd(n)));
		}

		Vector operator/(const double n) const
		{
			return Vector(_mm256_div_pd(_mm256_load_pd(mData), _mm256_set1_pd(n)));
		}

		bool operator==(const Vector& vec) const
		{
			if (Math::isZero(mData[0] - vec.mData[0]) && Math::isZero(mData[1] - vec.mData[1]) && Math::isZero(mData[2] - vec.mData[2]))
				return true;
			return false;
		}

		bool operator==(const double vec[3]) const
		{
			if (Math::isZero(mData[0] - vec[0]) && Math::isZero(mData[1] - vec[1]) && Math::isZero(mData[2] - vec[2]))
				return true;
			return false;
		}

		template<typename TTargetType>
		Vector<TTargetType, 3> convert() const;

		Vector& set(const Vector& vec)
		{
			_mm256_storeu_pd(mData, _mm256_load_pd(vec.mData));
			return *this;
		}

		Vector& set(const double vec[3])
		{
			std::memcpy(mData, vec, sizeof(double) * 3);
			return *this;
		}

		Vector& set(const double vx, const double vy, const double vz)
		{
			mData[0] = vx;
			mData[1] = vy;
			mData[2] = vz;
			return *this;
		}

		Vector& set(const double val)
		{
			_mm256_storeu_pd(mData, _mm256_set1_pd(val));
			return *this;
		}

		void write(double dest[3]) const
		{
			std::memcpy(dest, mData, sizeof(double) * 3);
		}

		double getDot() const
		{
			return Vector::calcDot(*this);
		}

		double getDot(const Vector& vec) const
		{
			return Vector::calcDot(*this, vec);
		}

		double getDot(const double vec[3]) const
		{
			return ((mData[0] * vec[0]) + (mData[1] * vec[1]) + (mData[2] * vec[2]));
		}

		double getDot(const double x, const double y, const double z) const
		{
			return ((mData[0] * x) + (mData[1] * y) + (mData[2] * z));
		}

		double getMagnitude() const
		{
			return Vector::calcMagnitude(*this);
		}

		double getMagnitudeInv() const
		{
			return Vector::calcMagnitudeInverse(*this);
		}

		double getDistance(const Vector& vec) const
		{
			return Vector::calcDistance(*this, vec);
		}

		double getDistance(const double& x, const double& y, const double& z) const
		{
			return Vector::calcDistance(*this, Vector(x, y, z));
		}

		double getDistance(const double vec[3]) const
		{
			return Vector::calcDistance(*this, Vector(vec));
		}

		bool isZero(const double precision) const
		{
			return ((std::abs(mData[0]) < precision) && (std::abs(mData[1]) < precision) && (std::abs(mData[2]) < precision));
		}

		bool isEqual(const Vector& vec, const double precision) const
		{
			return ((std::abs(mData[0] - vec.mData[0]) < precision) && (std::abs(mData[1] - vec.mData[1]) < precision) && (std::abs(mData[2] - vec.mData[2]) < precision));
		}

		Vector& normalize()
		{
			*this = calcNormalize(*this);
			return *this;
		}

		Vector& clamp(const double min, const double max)
		{
			__m256d tmp = _mm256_load_pd(mData);
			tmp = _mm256_max_pd(tmp, _mm256_set1_pd(min));
			tmp = _mm256_min_pd(tmp, _mm256_set1_pd(max));
			_mm256_storeu_pd(mData, tmp);

			return *this;
		}

		Vector& abs()
		{
			__m256d mask = _mm256_castsi256_pd(_mm256_set1_epi64x(static_cast<int64_t>(0x8000000000000000)));
			_mm256_storeu_pd(mData, _mm256_andnot_pd(mask, _mm256_load_pd(mData)));

			return *this;
		}

		Vector& neg()
		{
			__m256d mask = _mm256_castsi256_pd(_mm256_set1_epi64x(static_cast<int64_t>(0x8000000000000000)));
			_mm256_storeu_pd(mData, _mm256_xor_pd(_mm256_load_pd(mData), mask));

			return *this;
		}

		Vector& mad(double opMul, double opAdd)
		{
			__m256d tmp = _mm256_load_pd(mData);
			_mm256_storeu_pd(mData, _mm256_add_pd(_mm256_mul_pd(tmp, _mm256_set1_pd(opMul)), _mm256_set1_pd(opAdd)));

			return *this;
		}

		Vector project(const Vector& vec) const
		{
			return Vector::calcProject(*this, vec);
		}

		Vector reflect(const Vector& planeNormal) const
		{
			return Vector::calcReflect(*this, planeNormal);
		}

		Vector crossProduct(const Vector& vec) const
		{
			Vector result;

			__m256d vec1 = _mm256_load_pd(mData);
			__m256d vec2 = _mm256_load_pd(vec.mData);

			__m256d xa = _mm256_mul_pd(_mm256_permute4x64_pd(vec1, _MM_SHUFFLE(3, 0, 2, 1)), _mm256_permute4x64_pd(vec2, _MM_SHUFFLE(3, 1, 0, 2)));
			__m256d xb = _mm256_mul_pd(_mm256_permute4x64_pd(vec1, _MM_SHUFFLE(3, 1, 0, 2)), _mm256_permute4x64_pd(vec2, _MM_SHUFFLE(3, 0, 2, 1)));

			_mm256_storeu_pd(result.mData, _mm256_sub_pd(xa, xb));
			return result;
		}

		void storeInterpolate(const Vector& to, const double t)
		{
			__m256d tmp;

			tmp = _mm256_mul_pd(_mm256_load_pd(mData), _mm256_set1_pd(1.0 - t));
			tmp = _mm256_add_pd(tmp, _mm256_mul_pd(_mm256_load_pd(to.mData), _mm256_set1_pd(t)));
			_mm256_storeu_pd(mData, tmp);
		}

		void storeInterpolateNormals(const Vector& n1, const Vector& n2, const double t)
		{
			double a = std::acos(n1.getDot(n2));
			double sinA = 1.0 / (std::sin(a) + 0.0000001);

			double t0 = std::sin((1.0 - t) * a);
			double t1 = std::sin(t * a);

			mData[0] = (t0 * n1.mData[0] + t1 * n2.mData[0]) * sinA;
			mData[1] = (t0 * n1.mData[1] + t1 * n2.mData[1]) * sinA;
			mData[2] = (t0 * n1.mData[2] + t1 * n2.mData[2]) * sinA;
		}

		void storeClosestInSegment(const Vector& point, const Vector& p1, const Vector& p2)
		{
			Vector lineDir = p2 - p1;

			double t = lineDir.getDot();
			if (Math::isZero(t))
			{
				mData[0] = p1.mData[0];
				mData[1] = p1.mData[1];
				mData[2] = p1.mData[2];
				return;
			}

			t = ((point.mData[0] - p1.mData[0]) * lineDir.mData[0] + (point.mData[1] - p1.mData[1]) * lineDir.mData[1] + (point.mData[2] - p1.mData[2]) * lineDir.mData[2]) / t;
			t = Math::fClamp(t, 0.0, 1.0);

			mData[0] = p1.mData[0] + (t * lineDir.mData[0]);
			mData[1] = p1.mData[1] + (t * lineDir.mData[1]);
			mData[2] = p1.mData[2] + (t * lineDir.mData[2]);
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

		//static methods

		static float calcMagnitude(const Vector &vec)
		{
			float final;

			__m128 vecTmp = _mm_load_ps(vec.mData);
			_mm_store_ss(&final, _mm_sqrt_ss(_mm_dp_ps(vecTmp, vecTmp, 0xF0 | 0xF)));
			return final;
		}

		static float calcMagnitudeInverse(const Vector &vec)
		{
			float final;

			__m128 vecTmp = _mm_load_ps(vec.mData);
			_mm_store_ss(&final, _mm_rsqrt_ss(_mm_dp_ps(vecTmp, vecTmp, 0xF0 | 0xF)));
			return final;
		}

		static float calcDistance(const Vector &vec1, const Vector &vec2)
		{
			float final;

			__m128 vecTmp = _mm_sub_ps(_mm_load_ps(vec1.mData), _mm_load_ps(vec2.mData));
			_mm_store_ss(&final, _mm_sqrt_ss(_mm_dp_ps(vecTmp, vecTmp, 0xF0 | 0xF)));
			return final;
		}

		static float calcDot(const Vector &vec1, const Vector &vec2)
		{
			float final;

			_mm_store_ss(&final, _mm_dp_ps(_mm_load_ps(vec1.mData), _mm_load_ps(vec2.mData), 0xF0 | 0xF));
			return final;
		}

		static float calcDot(const Vector3f &vec1, const Vector &vec2)
		{
			float final;

			_mm_store_ss(&final, _mm_dp_ps(_mm_setr_ps(vec1.data()[0], vec1.data()[1], vec1.data()[2], 1.0f), _mm_load_ps(vec2.mData), 0xF0 | 0xF));
			return final;
		}

		//instance methods

		constexpr Vector() = default;

		explicit constexpr Vector(const Vector3f &v)
			: mData{ v[0], v[1], v[2], 1.0f }
		{ }

		explicit constexpr Vector(const Vector3f &v, const float vw)
			: mData{ v[0], v[1], v[2], vw }
		{ }

		explicit constexpr Vector(const float scalar)
			: mData{ scalar, scalar, scalar, scalar }
		{ }

		explicit constexpr Vector(const float vx, const float vy, const float vz, const float vw)
			: mData{ vx, vy, vz, vw }
		{ }

		explicit constexpr Vector(const float v[4])
			: mData{ v[0], v[1], v[2], v[3] }
		{ }

		explicit Vector(const __m128 vecDat)
		{
			_mm_store_ps(mData, vecDat);
		}

		constexpr float* data()
		{
			return mData;
		}

		constexpr const float* data() const
		{
			return mData;
		}

		constexpr float& operator[] (const size_t index)
		{
			return mData[index % 4];
		}

		constexpr const float& operator[] (const size_t index) const
		{
			return mData[index % 4];
		}

		Vector& operator=(const float vec[4])
		{
			_mm_store_ps(mData, _mm_load_ps(vec));
			return *this;
		}

		Vector& operator=(const float n)
		{
			_mm_store_ps(mData, _mm_load_ps1(&n));
			return *this;
		}

		void operator+=(const Vector& v)
		{
			_mm_store_ps(mData, _mm_add_ps(_mm_load_ps(mData), _mm_load_ps(v.mData)));
		}

		void operator-=(const Vector& v)
		{
			_mm_store_ps(mData, _mm_sub_ps(_mm_load_ps(mData), _mm_load_ps(v.mData)));
		}

		void operator*=(const Vector& v)
		{
			_mm_store_ps(mData, _mm_mul_ps(_mm_load_ps(mData), _mm_load_ps(v.mData)));
		}

		void operator/=(const Vector& v)
		{
			_mm_store_ps(mData, _mm_div_ps(_mm_load_ps(mData), _mm_load_ps(v.mData)));
		}

		void operator+=(const float n)
		{
			_mm_store_ps(mData, _mm_add_ps(_mm_load_ps(mData), _mm_load_ps1(&n)));
		}

		void operator-=(const float n)
		{
			_mm_store_ps(mData, _mm_sub_ps(_mm_load_ps(mData), _mm_load_ps1(&n)));
		}

		void operator*=(const float n)
		{
			_mm_store_ps(mData, _mm_mul_ps(_mm_load_ps(mData), _mm_load_ps1(&n)));
		}

		void operator/=(const float n)
		{
			_mm_store_ps(mData, _mm_div_ps(_mm_load_ps(mData), _mm_load_ps1(&n)));
		}

		Vector operator-() const
		{
			return Vector(_mm_mul_ps(_mm_load_ps(mData), _mm_set_ps1(-1.0f)));
		}

		Vector& set(const Vector &vec)
		{
			_mm_store_ps(mData, _mm_load_ps(vec.mData));
			return *this;
		}

		Vector& set(const Vector3f& vec, const float w)
		{
			_mm_store_ps(mData, _mm_load_ps(vec.data()));
			mData[3] = w;
			return *this;
		}

		Vector& set(const float vec[4])
		{
			_mm_store_ps(mData, _mm_load_ps(vec));
			return *this;
		}

		Vector& set(const float x, const float y, const float z, const float w)
		{
			mData[0] = x;
			mData[1] = y;
			mData[2] = z;
			mData[3] = w;
			return *this;
		}

		void write(float dest[4]) const
		{
			std::memcpy(dest, mData, sizeof(float) * 4);
		}

		float getMagnitude() const
		{
			return Vector::calcMagnitude(*this);
		}

		Vector& normalize()
		{
			__m128 vecTmp = _mm_load_ps(mData);
			__m128 vecMag = _mm_rsqrt_ps(_mm_dp_ps(vecTmp, vecTmp, 0xF0 | 0xF));
			_mm_store_ps(mData, _mm_mul_ps(vecTmp, vecMag));

			return *this;
		}

		Vector& clamp(const float min, const float max)
		{
			__m128 tmp = _mm_load_ps(mData);
			tmp = _mm_max_ps(tmp, _mm_load_ps1(&min));
			tmp = _mm_min_ps(tmp, _mm_load_ps1(&max));
			_mm_store_ps(mData, tmp);

			return *this;
		}

		Vector& abs()
		{
			__m128 mask = _mm_castsi128_ps(_mm_set1_epi32(0x7FFFFFFF));
			_mm_store_ps(mData, _mm_and_ps(_mm_load_ps(mData), mask));

			return *this;
		}

		Vector& neg()
		{
			__m128 mask = _mm_castsi128_ps(_mm_set1_epi32(0x80000000));
			_mm_store_ps(mData, _mm_xor_ps(_mm_load_ps(mData), mask));

			return *this;
		}

		float getDistance(const Vector &vec) const
		{
			return Vector::calcDistance(*this, vec);
		}

		float getDot(const Vector &vec) const
		{
			return Vector::calcDot(*this, vec);
		}

		float getDot(const Vector3f &vec) const
		{
			return Vector::calcDot(vec, *this);
		}

		void storeInterpolate(const Vector& from, const Vector& to, const float t)
		{
			__m128 tmp;

			tmp = _mm_mul_ps(_mm_load_ps(from.mData), _mm_set_ps1(1.0f - t));
			tmp = _mm_add_ps(tmp, _mm_mul_ps(_mm_load_ps(to.mData), _mm_load_ps1(&t)));
			_mm_store_ps(mData, tmp);
		}

		void storeInterpolate(const float from[4], const float to[4], const float t)
		{
			__m128 tmp;

			tmp = _mm_mul_ps(_mm_load_ps(from), _mm_set_ps1(1.0f - t));
			tmp = _mm_add_ps(tmp, _mm_mul_ps(_mm_load_ps(to), _mm_load_ps1(&t)));
			_mm_store_ps(mData, tmp);
		}

		void storeInterpolate(const Vector &to, const float t)
		{
			__m128 tmp;

			tmp = _mm_mul_ps(_mm_load_ps(mData), _mm_set_ps1(1.0f - t));
			tmp = _mm_add_ps(tmp, _mm_mul_ps(_mm_load_ps(to.mData), _mm_load_ps1(&t)));
			_mm_store_ps(mData, tmp);
		}
	};

	static_assert(alignof(Vector3f) == 16, "For performance reasons, this class must be aligned to a 16 byte boundary");
	static_assert(alignof(Vector3d) == 32, "For performance reasons, this class must be aligned to a 32 byte boundary");
	static_assert(alignof(Vector4f) == 16, "For performance reasons, this class must be aligned to a 16 byte boundary");
	static_assert(std::is_trivially_copyable_v<Vector3f>, "For performance reasons, this class should be trivially copyable");
	static_assert(std::is_trivially_copyable_v<Vector3d>, "For performance reasons, this class should be trivially copyable");
	static_assert(std::is_trivially_copyable_v<Vector4f>, "For performance reasons, this class should be trivially copyable");

	template<typename TTargetType>
	Vector<TTargetType, 3> Vector<float, 3>::convert() const
	{
		if constexpr (std::is_same_v<TTargetType, float>)
		{
			Vector<float, 3> newVec;
			_mm_store_ps(newVec.data(), _mm_load_ps(mData));

			return newVec;
		}
		else if constexpr (std::is_same_v<TTargetType, double>)
		{
			Vector<double, 3> newVec;
			_mm256_storeu_pd(newVec.data(), _mm256_cvtps_pd(_mm_load_ps(mData)));

			return newVec;
		}
		else
		{
			static_assert(std::is_same_v<TTargetType, double> || std::is_same_v<TTargetType, float>, "Unsupported data type");
			return {};
		}
	}

	template<typename TTargetType>
	Vector<TTargetType, 3> Vector<double,3>::convert() const
	{
		if constexpr (std::is_same_v<TTargetType, double>)
		{
			Vector<double, 3> newVec;
			_mm256_storeu_pd(newVec.data(), _mm256_load_pd(mData));

			return newVec;
		}
		else if constexpr (std::is_same_v<TTargetType, float>)
		{
			Vector<float, 3> newVec;
			_mm_store_ps(newVec.data(), _mm256_cvtpd_ps(_mm256_load_pd(mData)));

			return newVec;
		}
		else
		{
			static_assert(std::is_same_v<TTargetType, double> || std::is_same_v<TTargetType, float>, "Unsupported data type");
			return {};
		}
	}
}
