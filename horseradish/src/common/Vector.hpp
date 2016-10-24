#pragma once

#include "math.hpp"

#include <cmath>
#include <cstring>
#include <cassert>
#include <xmmintrin.h>
#include <smmintrin.h>
#include <type_traits>

namespace HorseRadish
{
	template<typename TDataType, size_t NComponents>
	class Vector
	{
		TDataType mData[NComponents];

		static_assert(NComponents >= 1, "Number of components must be equal or greater than 1");
		static_assert(std::is_arithmetic<TDataType>::value, "Data type must be arithmetic (e.g.: float, unsigned char, etc.)");

	public:
		//static methods

		static TDataType calcDot(const Vector<TDataType, NComponents> &vec1, const Vector<TDataType, NComponents> &vec2)
		{
			TDataType sum();
			for (size_t i = 0; i < NComponents; i++)
				sum += (vec1.mData[i] * vec2.mData[i]);

			return sum;
		}

		//instance methods

		Vector()
		{
			for (size_t i = 0; i < NComponents; i++)
				mData[i] = TDataType();
		}

		explicit Vector(const TDataType& scalar)
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

		Vector& operator=(const TDataType& scalar)
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

		void operator+=(const TDataType& scalar)
		{
			for (size_t i = 0; i < NComponents; i++)
				mData[i] += scalar;
		}

		void operator-=(const TDataType& scalar)
		{
			for (size_t i = 0; i < NComponents; i++)
				mData[i] -= scalar;
		}

		void operator*=(const TDataType& scalar)
		{
			for (size_t i = 0; i < NComponents; i++)
				mData[i] *= scalar;
		}

		void operator/=(const TDataType& scalar)
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

		Vector operator+(const TDataType& scalar) const
		{
			Vector result;
			for (size_t i = 0; i < NComponents; i++)
				result[i] = mData[i] + scalar;

			return result;
		}

		Vector operator-(const TDataType& scalar) const
		{
			Vector result;
			for (size_t i = 0; i < NComponents; i++)
				result[i] = mData[i] - scalar;

			return result;
		}

		Vector operator*(const TDataType& scalar) const
		{
			Vector result;
			for (size_t i = 0; i < NComponents; i++)
				result[i] = mData[i] * scalar;

			return result;
		}

		Vector operator/(const TDataType& scalar) const
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

		Vector& set(const TDataType& scalar)
		{
			for (size_t i = 0; i < NComponents; i++)
				mData[i] = scalar;

			return *this;
		}

		Vector& clamp(const TDataType& min, const TDataType& max)
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
			return Vector::calcDot(*this, *this);
		}

		TDataType getDot(const Vector &vec) const
		{
			return Vector::calcDot(*this, vec);
		}
	};

	typedef Vector<float, 3> Vector3f;
	typedef Vector<float, 4> Vector4f;

	template<>
	class Vector<float, 3>
	{
		float mData[4];

	public:
		//static methods

		static float calcMagnitude(const Vector &vec)
		{
			float final;

			__m128 vecTmp = _mm_loadu_ps(vec.mData);
			_mm_store_ss(&final, _mm_sqrt_ss(_mm_dp_ps(vecTmp, vecTmp, 0x70 | 0xF)));
			return final;
		}

		static float calcMagnitudeInverse(const Vector &vec)
		{
			float final;

			__m128 vecTmp = _mm_loadu_ps(vec.mData);
			_mm_store_ss(&final, _mm_rsqrt_ss(_mm_dp_ps(vecTmp, vecTmp, 0x70 | 0xF)));
			return final;
		}

		static float calcDistance(const Vector &vec1, const Vector &vec2)
		{
			float final;

			__m128 vecTmp = _mm_sub_ps(_mm_loadu_ps(vec1.mData), _mm_loadu_ps(vec2.mData));
			_mm_store_ss(&final, _mm_sqrt_ss(_mm_dp_ps(vecTmp, vecTmp, 0x70 | 0xF)));
			return final;
		}

		static float calcDot(const Vector &vec1, const Vector &vec2)
		{
			float final;

			_mm_store_ss(&final, _mm_dp_ps(_mm_loadu_ps(vec1.mData), _mm_loadu_ps(vec2.mData), 0x70 | 0xF));
			return final;
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

		Vector()
		{
			_mm_storeu_ps(mData, _mm_setzero_ps());
		}

		explicit Vector(const float scalar)
		{
			_mm_storeu_ps(mData, _mm_load_ps1(&scalar));
		}

		explicit Vector(const float &vx, const float &vy, const float &vz)
		{
			mData[0] = vx;
			mData[1] = vy;
			mData[2] = vz;
			mData[3] = 0.0f;
		}

		explicit Vector(const float v[3])
		{
			mData[0] = v[0];
			mData[1] = v[1];
			mData[2] = v[2];
			mData[3] = 0.0f;
		}

		explicit Vector(const __m128 vecDat)
		{
			_mm_storeu_ps(mData, vecDat);
		}

		float* data()
		{
			return mData;
		}

		const float* data() const
		{
			return mData;
		}

		float& operator[] (const size_t index)
		{
			return mData[index % 3];
		}

		const float& operator[] (const size_t index) const
		{
			return mData[index % 3];
		}

		Vector& operator=(const Vector& v)
		{
			_mm_storeu_ps(mData, _mm_loadu_ps(v.mData));
			return *this;
		}

		Vector& operator=(const float v[3])
		{
			std::memcpy(mData, v, sizeof(float) * 3);
			return *this;
		}

		Vector& operator=(const float n)
		{
			_mm_storeu_ps(mData, _mm_load_ps1(&n));
			return *this;
		}

		void operator+=(const Vector& v)
		{
			_mm_storeu_ps(mData, _mm_add_ps(_mm_loadu_ps(mData), _mm_loadu_ps(v.mData)));
		}

		void operator-=(const Vector& v)
		{
			_mm_storeu_ps(mData, _mm_sub_ps(_mm_loadu_ps(mData), _mm_loadu_ps(v.mData)));
		}

		void operator*=(const Vector& v)
		{
			_mm_storeu_ps(mData, _mm_mul_ps(_mm_loadu_ps(mData), _mm_loadu_ps(v.mData)));
		}

		void operator/=(const Vector& v)
		{
			_mm_storeu_ps(mData, _mm_div_ps(_mm_loadu_ps(mData), _mm_loadu_ps(v.mData)));
		}

		void operator+=(const float v[3])
		{
			mData[0] += v[0];
			mData[1] += v[1];
			mData[2] += v[2];
		}

		void operator-=(const float v[3])
		{
			mData[0] -= v[0];
			mData[1] -= v[1];
			mData[2] -= v[2];
		}

		void operator*=(const float v[3])
		{
			mData[0] *= v[0];
			mData[1] *= v[1];
			mData[2] *= v[2];
		}

		void operator/=(const float v[3])
		{
			mData[0] /= v[0];
			mData[1] /= v[1];
			mData[2] /= v[2];
		}

		void operator+=(const float &n)
		{
			_mm_storeu_ps(mData, _mm_add_ps(_mm_loadu_ps(mData), _mm_load_ps1(&n)));
		}

		void operator-=(const float &n)
		{
			_mm_storeu_ps(mData, _mm_sub_ps(_mm_loadu_ps(mData), _mm_load_ps1(&n)));
		}

		void operator*=(const float &n)
		{
			_mm_storeu_ps(mData, _mm_mul_ps(_mm_loadu_ps(mData), _mm_load_ps1(&n)));
		}

		void operator/=(const float &n)
		{
			_mm_storeu_ps(mData, _mm_div_ps(_mm_loadu_ps(mData), _mm_load_ps1(&n)));
		}

		Vector operator-() const
		{
			return Vector(_mm_mul_ps(_mm_loadu_ps(mData), _mm_set_ps1(-1.0f)));
		}

		Vector operator+(const Vector &vec) const
		{
			return Vector(_mm_add_ps(_mm_loadu_ps(mData), _mm_loadu_ps(vec.mData)));
		}

		Vector operator-(const Vector &vec) const
		{
			return Vector(_mm_sub_ps(_mm_loadu_ps(mData), _mm_loadu_ps(vec.mData)));
		}

		Vector operator*(const Vector &vec) const
		{
			return Vector(_mm_mul_ps(_mm_loadu_ps(mData), _mm_loadu_ps(vec.mData)));
		}

		Vector operator/(const Vector &vec) const
		{
			return Vector(_mm_div_ps(_mm_loadu_ps(mData), _mm_loadu_ps(vec.mData)));
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

		Vector operator+(const float &n) const
		{
			return Vector(_mm_add_ps(_mm_loadu_ps(mData), _mm_load_ps1(&n)));
		}

		Vector operator-(const float &n) const
		{
			return Vector(_mm_sub_ps(_mm_loadu_ps(mData), _mm_load_ps1(&n)));
		}

		Vector operator*(const float &n) const
		{
			return Vector(_mm_mul_ps(_mm_loadu_ps(mData), _mm_load_ps1(&n)));
		}

		Vector operator/(const float &n) const
		{
			return Vector(_mm_div_ps(_mm_loadu_ps(mData), _mm_load_ps1(&n)));
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

		Vector& set(const Vector &vec)
		{
			_mm_storeu_ps(mData, _mm_loadu_ps(vec.mData));
			return *this;
		}

		Vector& set(const float vec[3])
		{
			std::memcpy(mData, vec, sizeof(float) * 3);
			return *this;
		}

		Vector& set(const float &vx, const float &vy, const float &vz)
		{
			mData[0] = vx;
			mData[1] = vy;
			mData[2] = vz;
			return *this;
		}

		Vector& set(const float &val)
		{
			_mm_storeu_ps(mData, _mm_load_ps1(&val));
			return *this;
		}

		void write(float dest[3]) const
		{
			std::memcpy(dest, mData, sizeof(float) * 3);
		}

		float getDot() const
		{
			return Vector::calcDot(*this, *this);
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

		float getDistance(const float &x, const float &y, const float &z) const
		{
			return Vector::calcDistance(*this, Vector(x, y, z));
		}

		float getDistance(const float vec[3]) const
		{
			return Vector::calcDistance(*this, Vector(vec));
		}

		bool isEqual(const Vector &vec, const float &precision) const
		{
			return ((std::abs(mData[0] - vec.mData[0]) < precision) && (std::abs(mData[1] - vec.mData[1]) < precision) && (std::abs(mData[2] - vec.mData[2]) < precision));
		}

		Vector& normalize()
		{
			__m128 vecTmp = _mm_loadu_ps(mData);
			__m128 vecMag = _mm_rsqrt_ps(_mm_dp_ps(vecTmp, vecTmp, 0x70 | 0xF));
			_mm_storeu_ps(mData, _mm_mul_ps(vecTmp, vecMag));

			return *this;
		}

		Vector& clamp(const float &min, const float &max)
		{
			__m128 tmp = _mm_loadu_ps(mData);
			tmp = _mm_max_ps(tmp, _mm_load_ps1(&min));
			tmp = _mm_min_ps(tmp, _mm_load_ps1(&max));
			_mm_storeu_ps(mData, tmp);

			return *this;
		}

		Vector& abs()
		{
			__m128 mask = _mm_castsi128_ps(_mm_set1_epi32(0x7FFFFFFF));
			_mm_storeu_ps(mData, _mm_and_ps(_mm_loadu_ps(mData), mask));

			return *this;
		}

		Vector& neg()
		{
			__m128 mask = _mm_castsi128_ps(_mm_set1_epi32(0x80000000));
			_mm_storeu_ps(mData, _mm_xor_ps(_mm_loadu_ps(mData), mask));

			return *this;
		}

		void storeNormal(const Vector &v1, const Vector &v2, const Vector &v3)
		{
			__m128 vec1, vec2, tmp1, tmp2;

			tmp1 = _mm_loadu_ps(v1.mData);
			vec1 = _mm_sub_ps(_mm_loadu_ps(v2.mData), tmp1);
			vec2 = _mm_sub_ps(_mm_loadu_ps(v3.mData), tmp1);

			tmp1 = _mm_mul_ps(_mm_shuffle_ps(vec1, vec1, _MM_SHUFFLE(3, 0, 2, 1)), _mm_shuffle_ps(vec2, vec2, _MM_SHUFFLE(3, 1, 0, 2)));
			tmp2 = _mm_mul_ps(_mm_shuffle_ps(vec1, vec1, _MM_SHUFFLE(3, 1, 0, 2)), _mm_shuffle_ps(vec2, vec2, _MM_SHUFFLE(3, 0, 2, 1)));
			vec1 = _mm_sub_ps(tmp1, tmp2);

			vec2 = _mm_rsqrt_ps(_mm_dp_ps(vec1, vec1, 0x70 | 0xF));
			_mm_storeu_ps(mData, _mm_mul_ps(vec1, vec2));
		}

		void storeNormal(const float vec1[3], const float vec2[3], const float vec3[3])
		{
			storeNormal(Vector(vec1), Vector(vec2), Vector(vec3));
		}

		Vector crossProduct(const Vector &vec) const
		{
			Vector result;

			__m128 vec1 = _mm_loadu_ps(mData);
			__m128 vec2 = _mm_loadu_ps(vec.mData);

			__m128 xa = _mm_mul_ps(_mm_shuffle_ps(vec1, vec1, _MM_SHUFFLE(3, 0, 2, 1)), _mm_shuffle_ps(vec2, vec2, _MM_SHUFFLE(3, 1, 0, 2)));
			__m128 xb = _mm_mul_ps(_mm_shuffle_ps(vec1, vec1, _MM_SHUFFLE(3, 1, 0, 2)), _mm_shuffle_ps(vec2, vec2, _MM_SHUFFLE(3, 0, 2, 1)));

			_mm_storeu_ps(result.mData, _mm_sub_ps(xa, xb));
			return result;
		}

		void storeCrossProduct(const Vector &p, const Vector &q)
		{
			__m128 vec1 = _mm_loadu_ps(p.mData);
			__m128 vec2 = _mm_loadu_ps(q.mData);

			__m128 xa = _mm_mul_ps(_mm_shuffle_ps(vec1, vec1, _MM_SHUFFLE(3, 0, 2, 1)), _mm_shuffle_ps(vec2, vec2, _MM_SHUFFLE(3, 1, 0, 2)));
			__m128 xb = _mm_mul_ps(_mm_shuffle_ps(vec1, vec1, _MM_SHUFFLE(3, 1, 0, 2)), _mm_shuffle_ps(vec2, vec2, _MM_SHUFFLE(3, 0, 2, 1)));

			_mm_storeu_ps(mData, _mm_sub_ps(xa, xb));
		}

		void storeCrossProduct(const float p[3], const float q[3])
		{
			storeCrossProduct(Vector(p), Vector(q));
		}

		void storeInterpolate(const Vector &from, const Vector &to, const float &t)
		{
			__m128 tmp;

			tmp = _mm_mul_ps(_mm_loadu_ps(from.mData), _mm_set_ps1(1.0f - t));
			tmp = _mm_add_ps(tmp, _mm_mul_ps(_mm_loadu_ps(to.mData), _mm_load_ps1(&t)));
			_mm_storeu_ps(mData, tmp);
		}

		void storeInterpolate(const float from[3], const float to[3], const float &t)
		{
			storeInterpolate(Vector(from), Vector(to), t);
		}

		void storeInterpolate(const Vector &to, const float &t)
		{
			__m128 tmp;

			tmp = _mm_mul_ps(_mm_loadu_ps(mData), _mm_set_ps1(1.0f - t));
			tmp = _mm_add_ps(tmp, _mm_mul_ps(_mm_loadu_ps(to.mData), _mm_load_ps1(&t)));
			_mm_storeu_ps(mData, tmp);
		}

		void storeInterpolateNormals(const Vector &n1, const Vector &n2, const float &t)
		{
			float a = acos(n1.getDot(n2));
			float sinA = 1.0f / sinf(a);

			mData[0] = (sin((1.0f - t)*a)*n1.mData[0] + sin(t*a)*n2.mData[0]) * sinA;
			mData[1] = (sin((1.0f - t)*a)*n1.mData[1] + sin(t*a)*n2.mData[1]) * sinA;
			mData[2] = (sin((1.0f - t)*a)*n1.mData[2] + sin(t*a)*n2.mData[2]) * sinA;
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
	class Vector<float, 4>
	{
		float mData[4];

	public:

		//static methods

		static float calcMagnitude(const Vector &vec)
		{
			float final;

			__m128 vecTmp = _mm_loadu_ps(vec.mData);
			_mm_store_ss(&final, _mm_sqrt_ss(_mm_dp_ps(vecTmp, vecTmp, 0xF0 | 0xF)));
			return final;
		}

		static float calcMagnitudeInverse(const Vector &vec)
		{
			float final;

			__m128 vecTmp = _mm_loadu_ps(vec.mData);
			_mm_store_ss(&final, _mm_rsqrt_ss(_mm_dp_ps(vecTmp, vecTmp, 0xF0 | 0xF)));
			return final;
		}

		static float calcDistance(const Vector &vec1, const Vector &vec2)
		{
			float final;

			__m128 vecTmp = _mm_sub_ps(_mm_loadu_ps(vec1.mData), _mm_loadu_ps(vec2.mData));
			_mm_store_ss(&final, _mm_sqrt_ss(_mm_dp_ps(vecTmp, vecTmp, 0xF0 | 0xF)));
			return final;
		}

		static float calcDot(const Vector &vec1, const Vector &vec2)
		{
			float final;

			_mm_store_ss(&final, _mm_dp_ps(_mm_loadu_ps(vec1.mData), _mm_loadu_ps(vec2.mData), 0xF0 | 0xF));
			return final;
		}

		static float calcDot(const Vector3f &vec1, const Vector &vec2)
		{
			float final;

			_mm_store_ss(&final, _mm_dp_ps(_mm_setr_ps(vec1.data()[0], vec1.data()[1], vec1.data()[2], 1.0f), _mm_loadu_ps(vec2.mData), 0xF0 | 0xF));
			return final;
		}

		//instance methods

		Vector()
		{
			_mm_storeu_ps(mData, _mm_setzero_ps());
		}

		explicit Vector(const Vector3f &s)
		{
			_mm_storeu_ps(mData, _mm_loadu_ps(s.data()));
			mData[3] = 1.0f;
		}

		explicit Vector(const Vector3f &s, const float vw)
		{
			_mm_storeu_ps(mData, _mm_loadu_ps(s.data()));
			mData[3] = vw;
		}

		explicit Vector(const float scalar)
		{
			_mm_storeu_ps(mData, _mm_load_ps1(&scalar));
		}

		explicit Vector(const float vx, const float vy, const float vz, const float vw)
		{
			mData[0] = vx;
			mData[1] = vy;
			mData[2] = vz;
			mData[3] = vw;
		}

		explicit Vector(const float vec[4])
		{
			_mm_storeu_ps(mData, _mm_loadu_ps(vec));
		}

		explicit Vector(const __m128 vecDat)
		{
			_mm_storeu_ps(mData, vecDat);
		}

		float* data()
		{
			return mData;
		}

		const float* data() const
		{
			return mData;
		}

		float& operator[] (const size_t index)
		{
			return mData[index % 4];
		}

		const float& operator[] (const size_t index) const
		{
			return mData[index % 4];
		}

		Vector& operator=(const Vector &v)
		{
			_mm_storeu_ps(mData, _mm_loadu_ps(v.mData));
			return *this;
		}

		Vector& operator=(const float vec[4])
		{
			_mm_storeu_ps(mData, _mm_loadu_ps(vec));
			return *this;
		}

		Vector& operator=(const float n)
		{
			_mm_storeu_ps(mData, _mm_load_ps1(&n));
			return *this;
		}

		void operator+=(const Vector& v)
		{
			_mm_storeu_ps(mData, _mm_add_ps(_mm_loadu_ps(mData), _mm_loadu_ps(v.mData)));
		}

		void operator-=(const Vector& v)
		{
			_mm_storeu_ps(mData, _mm_sub_ps(_mm_loadu_ps(mData), _mm_loadu_ps(v.mData)));
		}

		void operator*=(const Vector& v)
		{
			_mm_storeu_ps(mData, _mm_mul_ps(_mm_loadu_ps(mData), _mm_loadu_ps(v.mData)));
		}

		void operator/=(const Vector& v)
		{
			_mm_storeu_ps(mData, _mm_div_ps(_mm_loadu_ps(mData), _mm_loadu_ps(v.mData)));
		}

		void operator+=(const float &n)
		{
			_mm_storeu_ps(mData, _mm_add_ps(_mm_loadu_ps(mData), _mm_load_ps1(&n)));
		}

		void operator-=(const float &n)
		{
			_mm_storeu_ps(mData, _mm_sub_ps(_mm_loadu_ps(mData), _mm_load_ps1(&n)));
		}

		void operator*=(const float &n)
		{
			_mm_storeu_ps(mData, _mm_mul_ps(_mm_loadu_ps(mData), _mm_load_ps1(&n)));
		}

		void operator/=(const float &n)
		{
			_mm_storeu_ps(mData, _mm_div_ps(_mm_loadu_ps(mData), _mm_load_ps1(&n)));
		}

		Vector operator-() const
		{
			return Vector(_mm_mul_ps(_mm_loadu_ps(mData), _mm_set_ps1(-1.0f)));
		}

		Vector& set(const Vector &vec)
		{
			_mm_storeu_ps(mData, _mm_loadu_ps(vec.mData));
			return *this;
		}

		Vector& set(const Vector3f& vec, const float &w)
		{
			_mm_storeu_ps(mData, _mm_loadu_ps(vec.data()));
			mData[3] = w;
			return *this;
		}

		Vector& set(const float vec[4])
		{
			_mm_storeu_ps(mData, _mm_loadu_ps(vec));
			return *this;
		}

		Vector& set(const float &x, const float &y, const float &z, const float &w)
		{
			mData[0] = x;
			mData[1] = y;
			mData[2] = z;
			mData[3] = w;
			return *this;
		}

		float getMagnitude() const
		{
			return Vector::calcMagnitude(*this);
		}

		Vector& normalize()
		{
			__m128 vecTmp = _mm_loadu_ps(mData);
			__m128 vecMag = _mm_rsqrt_ps(_mm_dp_ps(vecTmp, vecTmp, 0xF0 | 0xF));
			_mm_storeu_ps(mData, _mm_mul_ps(vecTmp, vecMag));

			return *this;
		}

		Vector& clamp(const float &min, const float &max)
		{
			__m128 tmp = _mm_loadu_ps(mData);
			tmp = _mm_max_ps(tmp, _mm_load_ps1(&min));
			tmp = _mm_min_ps(tmp, _mm_load_ps1(&max));
			_mm_storeu_ps(mData, tmp);

			return *this;
		}

		Vector& abs()
		{
			__m128 mask = _mm_castsi128_ps(_mm_set1_epi32(0x7FFFFFFF));
			_mm_storeu_ps(mData, _mm_and_ps(_mm_loadu_ps(mData), mask));

			return *this;
		}

		Vector& neg()
		{
			__m128 mask = _mm_castsi128_ps(_mm_set1_epi32(0x80000000));
			_mm_storeu_ps(mData, _mm_xor_ps(_mm_loadu_ps(mData), mask));

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

			tmp = _mm_mul_ps(_mm_loadu_ps(from.mData), _mm_set_ps1(1.0f - t));
			tmp = _mm_add_ps(tmp, _mm_mul_ps(_mm_loadu_ps(to.mData), _mm_load_ps1(&t)));
			_mm_storeu_ps(mData, tmp);
		}

		void storeInterpolate(const float from[4], const float to[4], const float &t)
		{
			__m128 tmp;

			tmp = _mm_mul_ps(_mm_loadu_ps(from), _mm_set_ps1(1.0f - t));
			tmp = _mm_add_ps(tmp, _mm_mul_ps(_mm_loadu_ps(to), _mm_load_ps1(&t)));
			_mm_storeu_ps(mData, tmp);
		}

		void storeInterpolate(const Vector &to, const float t)
		{
			__m128 tmp;

			tmp = _mm_mul_ps(_mm_loadu_ps(mData), _mm_set_ps1(1.0f - t));
			tmp = _mm_add_ps(tmp, _mm_mul_ps(_mm_loadu_ps(to.mData), _mm_load_ps1(&t)));
			_mm_storeu_ps(mData, tmp);
		}
	};

} //HorseRadish
