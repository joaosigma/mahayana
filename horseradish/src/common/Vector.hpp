#pragma once

#include "Types.hpp"
#include "Math.hpp"

#include <cassert>
#include <xmmintrin.h>

namespace HorseRadish
{
	HALIGN_16BYTES
	class Vector
	{
	public:
		float x, y, z, w;

		//static methods

		static float CalcMagnitude(const Vector &vec)
		{
			return sqrt(vec.x*vec.x + vec.y*vec.y + vec.z*vec.z);
		}

		static float CalcDistance(const Vector &vec1, const Vector &vec2)
		{
			float d1 = vec1.x - vec2.x;
			float d2 = vec1.y - vec2.y;
			float d3 = vec1.z - vec2.z;
			return sqrt(d1*d1 + d2*d2 + d3*d3);
		}

		static float CalcDot(const Vector &vec1, const Vector &vec2)
		{
			return (vec1.x*vec2.x + vec1.y*vec2.y + vec1.z*vec2.z);
		}

		//instance methods

		inline Vector()
		{
			_mm_storeu_ps(&x, _mm_setzero_ps());
		}

		inline Vector(const Vector &v)
		{
			_mm_storeu_ps(&x, _mm_loadu_ps(&v.x));
		}

		explicit inline Vector(const float scalar)
		{
			_mm_storeu_ps(&x, _mm_load_ps1(&scalar));
		}

		explicit inline Vector(const float &vx, const float &vy, const float &vz)
		{
			x = vx; y = vy; z = vz; w = 0.0f;
		}

		explicit inline Vector(const float * const v)
		{
			x = v[0]; y = v[1]; z = v[2]; w = 0.0f;
		}

		explicit inline Vector(const __m128 vecDat)
		{
			_mm_storeu_ps(&x, vecDat);
		}

		inline ~Vector()
		{
			return;
		}

		inline operator float *(void)
		{
			return &x;
		}
		inline operator const float *(void) const
		{
			return &x;
		}

		inline Vector& operator=(const Vector& v)
		{
			_mm_storeu_ps(&x, _mm_loadu_ps(&v.x));
			return *this;
		}

		inline Vector& operator=(const float *v)
		{
			x = v[0]; y = v[1]; z = v[2];
			return *this;
		}

		inline Vector& operator=(const float n)
		{
			_mm_storeu_ps(&x, _mm_load_ps1(&n));
			return *this;
		}

		inline void operator+=(const Vector& v)
		{
			_mm_storeu_ps(&x, _mm_add_ps(_mm_loadu_ps(&x), _mm_loadu_ps(&v.x)));
		}

		inline void operator-=(const Vector& v)
		{
			_mm_storeu_ps(&x, _mm_sub_ps(_mm_loadu_ps(&x), _mm_loadu_ps(&v.x)));
		}

		inline void operator*=(const Vector& v)
		{
			_mm_storeu_ps(&x, _mm_mul_ps(_mm_loadu_ps(&x), _mm_loadu_ps(&v.x)));
		}

		inline void operator/=(const Vector& v)
		{
			_mm_storeu_ps(&x, _mm_div_ps(_mm_loadu_ps(&x), _mm_loadu_ps(&v.x)));
		}

		inline void operator+=(const float * const p)
		{
			x += p[0]; y += p[1]; z += p[2];
		}

		inline void operator-=(const float * const p)
		{
			x -= p[0]; y -= p[1]; z -= p[2];
		}

		inline void operator*=(const float * const p)
		{
			x *= p[0]; y *= p[1]; z *= p[2];
		}

		inline void operator/=(const float * const p)
		{
			x /= p[0]; y /= p[1]; z /= p[2];
		}

		inline void operator+=(const float &n)
		{
			_mm_storeu_ps(&x, _mm_add_ps(_mm_loadu_ps(&x), _mm_load_ps1(&n)));
		}

		inline void operator-=(const float &n)
		{
			_mm_storeu_ps(&x, _mm_sub_ps(_mm_loadu_ps(&x), _mm_load_ps1(&n)));
		}

		inline void operator*=(const float &n)
		{
			_mm_storeu_ps(&x, _mm_mul_ps(_mm_loadu_ps(&x), _mm_load_ps1(&n)));
		}

		inline void operator/=(const float &n)
		{
			_mm_storeu_ps(&x, _mm_div_ps(_mm_loadu_ps(&x), _mm_load_ps1(&n)));
		}

		inline float operator[](int index) const
		{
			assert((index >= 0) && (index < 3));
			return (&x)[index];
		}

		inline float& operator[](int index)
		{
			assert((index >= 0) && (index < 3));
			return (&x)[index];
		}

		inline Vector operator-() const
		{
			return Vector(_mm_mul_ps(_mm_loadu_ps(&x), _mm_set_ps1(-1.0f)));
		}

		inline Vector operator+(const Vector &vec) const
		{
			return Vector(_mm_add_ps(_mm_loadu_ps(&x), _mm_loadu_ps(&vec.x)));
		}

		inline Vector operator-(const Vector &vec) const
		{
			return Vector(_mm_sub_ps(_mm_loadu_ps(&x), _mm_loadu_ps(&vec.x)));
		}

		inline Vector operator*(const Vector &vec) const
		{
			return Vector(_mm_mul_ps(_mm_loadu_ps(&x), _mm_loadu_ps(&vec.x)));
		}

		inline Vector operator/(const Vector &vec) const
		{
			return Vector(_mm_div_ps(_mm_loadu_ps(&x), _mm_loadu_ps(&vec.x)));
		}

		inline Vector operator+(const float * const vec) const
		{
			return Vector(x + vec[0], y + vec[1], z + vec[2]);
		}

		inline Vector operator-(const float * const vec) const
		{
			return Vector(x - vec[0], y - vec[1], z - vec[2]);
		}

		inline Vector operator*(const float * const vec) const
		{
			return Vector(x*vec[0], y*vec[1], z*vec[2]);
		}

		inline Vector operator/(const float * const vec) const
		{
			return Vector(x / vec[0], y / vec[1], z / vec[2]);
		}

		inline Vector operator+(const float &n) const
		{
			return Vector(_mm_add_ps(_mm_loadu_ps(&x), _mm_load_ps1(&n)));
		}

		inline Vector operator-(const float &n) const
		{
			return Vector(_mm_sub_ps(_mm_loadu_ps(&x), _mm_load_ps1(&n)));
		}

		inline Vector operator*(const float &n) const
		{
			return Vector(_mm_mul_ps(_mm_loadu_ps(&x), _mm_load_ps1(&n)));
		}

		inline Vector operator/(const float &n) const
		{
			return Vector(_mm_div_ps(_mm_loadu_ps(&x), _mm_load_ps1(&n)));
		}

		inline bool operator==(const Vector &vec) const
		{
			if (Math::isZero(x - vec.x) && Math::isZero(y - vec.y) && Math::isZero(z - vec.z))
				return true;
			return false;
		}

		inline bool operator==(const float * const vec) const
		{
			if (Math::isZero(x - vec[0]) && Math::isZero(y - vec[1]) && Math::isZero(z - vec[2]))
				return true;
			return false;
		}

		inline void Set(const Vector &vec)
		{
			_mm_storeu_ps(&x, _mm_loadu_ps(&vec.x));
		}

		inline void Set(const Vector * const vec)
		{
			_mm_storeu_ps(&x, _mm_loadu_ps(&vec->x));
		}

		inline void Set(const float * const p)
		{
			x = p[0]; y = p[1]; z = p[2];
		}

		inline void Set(const float &vx, const float &vy, const float &vz)
		{
			x = vx; y = vy; z = vz;
		}

		inline void Set(const float &val)
		{
			_mm_storeu_ps(&x, _mm_load_ps1(&val));
		}

		inline void Write(float * const dest) const
		{
			dest[0] = x; dest[1] = y; dest[2] = z;
		}

		inline void WriteNeg(float * const dest) const
		{
			dest[0] = -x; dest[1] = -y; dest[2] = -z;
		}

		inline void WriteAdd(float * const dest) const
		{
			dest[0] += x; dest[1] += y; dest[2] += z;
		}

		inline float GetDot() const
		{
			return (x*x + y*y + z*z);
		}

		inline float GetDot(const Vector &vec) const
		{
			return (x*vec.x + y*vec.y + z*vec.z);
		}

		inline float GetDot(const float * const p) const
		{
			return (x*p[0] + y*p[1] + z*p[2]);
		}

		inline float GetDot(const float x, const float y, const float z) const
		{
			return (this->x*x + this->y*y + this->z*z);
		}

		inline float GetMagnitude() const
		{
			float final;
			__m128 tmp;

			tmp = _mm_loadu_ps(&x);
			tmp = _mm_mul_ps(tmp, tmp);
			tmp = _mm_add_ss(tmp, _mm_shuffle_ps(tmp, tmp, _MM_SHUFFLE(3, 2, 1, 1)));
			tmp = _mm_add_ss(tmp, _mm_movehl_ps(tmp, tmp));
			_mm_store_ss(&final, _mm_sqrt_ss(tmp));
			return final;
		}

		inline float GetMagnitudeInv() const
		{
			float final;
			__m128 tmp;

			tmp = _mm_loadu_ps(&x);
			tmp = _mm_mul_ps(tmp, tmp);
			tmp = _mm_add_ss(tmp, _mm_shuffle_ps(tmp, tmp, _MM_SHUFFLE(3, 2, 1, 1)));
			tmp = _mm_add_ss(tmp, _mm_movehl_ps(tmp, tmp));
			_mm_store_ss(&final, _mm_rsqrt_ss(tmp));
			return final;
		}

		inline float GetMagnitudeSquared() const
		{
			float final;
			__m128 tmp;

			tmp = _mm_loadu_ps(&x);
			tmp = _mm_mul_ps(tmp, tmp);
			tmp = _mm_add_ss(tmp, _mm_shuffle_ps(tmp, tmp, _MM_SHUFFLE(3, 2, 1, 1)));
			_mm_store_ss(&final, _mm_add_ss(tmp, _mm_movehl_ps(tmp, tmp)));
			return final;
		}

		inline float GetDistance(const Vector &vec) const
		{
			float final;
			__m128 tmp;

			tmp = _mm_sub_ps(_mm_loadu_ps(&vec.x), _mm_loadu_ps(&x));
			tmp = _mm_mul_ps(tmp, tmp);
			tmp = _mm_add_ss(tmp, _mm_shuffle_ps(tmp, tmp, _MM_SHUFFLE(3, 2, 1, 1)));
			tmp = _mm_add_ss(tmp, _mm_movehl_ps(tmp, tmp));
			_mm_store_ss(&final, _mm_sqrt_ss(tmp));
			return final;
		}

		inline float GetDistance(const float &x, const float &y, const float &z) const
		{
			float d1 = x - this->x;
			float d2 = y - this->y;
			float d3 = z - this->z;
			return sqrt(d1*d1 + d2*d2 + d3*d3);
		}

		inline float GetDistance(const float * const p) const
		{
			float d1 = p[0] - x;
			float d2 = p[1] - y;
			float d3 = p[2] - z;
			return sqrt(d1*d1 + d2*d2 + d3*d3);
		}

		inline bool Compare(const Vector &vec, const float &precision) const
		{
			if ((Math::fAbs(x - vec.x) < precision) && (Math::fAbs(y - vec.y) < precision) && (Math::fAbs(z - vec.z) < precision))
				return true;
			return false;
		}

		inline void Normalize()
		{
			__m128 aux, tmp;

			aux = _mm_loadu_ps(&x);
			tmp = _mm_mul_ps(aux, aux);
			tmp = _mm_add_ss(tmp, _mm_shuffle_ps(tmp, tmp, _MM_SHUFFLE(3, 2, 1, 1)));
			tmp = _mm_add_ss(tmp, _mm_movehl_ps(tmp, tmp));
			tmp = _mm_rsqrt_ss(tmp);
			_mm_storeu_ps(&x, _mm_mul_ps(aux, _mm_shuffle_ps(tmp, tmp, 0)));
		}

		inline void Clamp(const float &min, const float &max)
		{
			__m128 tmp;

			tmp = _mm_loadu_ps(&x);
			tmp = _mm_max_ps(tmp, _mm_load_ps1(&min));
			tmp = _mm_min_ps(tmp, _mm_load_ps1(&max));
			_mm_storeu_ps(&x, tmp);
		}

		inline void StoreNormal(const Vector &v1, const Vector &v2, const Vector &v3)
		{
			__m128 vec1, vec2, tmp1, tmp2;

			vec2 = _mm_loadu_ps(&v1.x);
			vec1 = _mm_sub_ps(_mm_loadu_ps(&v2.x), vec2);
			vec2 = _mm_sub_ps(_mm_loadu_ps(&v3.x), vec2);

			tmp1 = _mm_mul_ps(_mm_shuffle_ps(vec1, vec1, _MM_SHUFFLE(3, 0, 2, 1)), _mm_shuffle_ps(vec2, vec2, _MM_SHUFFLE(3, 1, 0, 2)));
			tmp2 = _mm_mul_ps(_mm_shuffle_ps(vec1, vec1, _MM_SHUFFLE(3, 1, 0, 2)), _mm_shuffle_ps(vec2, vec2, _MM_SHUFFLE(3, 0, 2, 1)));
			tmp1 = _mm_sub_ps(tmp1, tmp2);

			tmp2 = _mm_mul_ps(tmp1, tmp1);
			tmp2 = _mm_add_ss(tmp2, _mm_shuffle_ps(tmp2, tmp2, _MM_SHUFFLE(3, 2, 1, 1)));
			tmp2 = _mm_add_ss(tmp2, _mm_movehl_ps(tmp2, tmp2));
			tmp2 = _mm_rsqrt_ss(tmp2);
			_mm_storeu_ps(&x, _mm_mul_ps(tmp1, _mm_shuffle_ps(tmp2, tmp2, 0)));
		}

		inline void StoreNormal(const float * const v1, const float * const v2, const float * const v3)
		{
			float aux1[3], aux2[3], tamanho;

			aux1[0] = v2[0] - v1[0];
			aux1[1] = v2[1] - v1[1];
			aux1[2] = v2[2] - v1[2];

			aux2[0] = v3[0] - v1[0];
			aux2[1] = v3[1] - v1[1];
			aux2[2] = v3[2] - v1[2];

			x = (aux1[1] * aux2[2]) - (aux1[2] * aux2[1]);
			y = (aux1[2] * aux2[0]) - (aux1[0] * aux2[2]);
			z = (aux1[0] * aux2[1]) - (aux1[1] * aux2[0]);

			tamanho = 1.0f / sqrt(x*x + y*y + z*z);
			x *= tamanho;
			y *= tamanho;
			z *= tamanho;
		}

		inline void StoreVector(const Vector &p1, const Vector &p2)
		{
			x = p1.x - p2.x;
			y = p1.y - p2.y;
			z = p1.z - p2.z;
		}

		inline Vector CrossProduct(const Vector &vec) const
		{
			return Vector((this->y * vec.z) - (this->z * vec.y), (this->z * vec.x) - (this->x * vec.z), (this->x * vec.y) - (this->y * vec.x));
		}

		inline void StoreCrossProduct(const Vector &p, const Vector &q)
		{
			x = (p.y * q.z) - (p.z * q.y);
			y = (p.z * q.x) - (p.x * q.z);
			z = (p.x * q.y) - (p.y * q.x);
		}

		inline void StoreCrossProduct(const float * const p, const float * const q)
		{
			x = p[1] * q[2] - p[2] * q[1];
			y = p[2] * q[0] - p[0] * q[2];
			z = p[0] * q[1] - p[1] * q[0];
		}

		inline void StoreInterpolate(const Vector &from, const Vector &to, const float &t)
		{
			__m128 tmp;

			tmp = _mm_mul_ps(_mm_loadu_ps(&from.x), _mm_set_ps1(1.0f - t));
			tmp = _mm_add_ps(tmp, _mm_mul_ps(_mm_loadu_ps(&to.x), _mm_load_ps1(&t)));
			_mm_storeu_ps(&x, tmp);
		}

		inline void StoreInterpolate(const float * const from, const float * const to, const float &t)
		{
			x = from[0] * (1.0f - t) + to[0] * t;
			y = from[1] * (1.0f - t) + to[1] * t;
			z = from[2] * (1.0f - t) + to[2] * t;
		}

		inline void StoreInterpolate(const Vector &to, const float &t)
		{
			__m128 tmp;

			tmp = _mm_mul_ps(_mm_loadu_ps(&x), _mm_set_ps1(1.0f - t));
			tmp = _mm_add_ps(tmp, _mm_mul_ps(_mm_loadu_ps(&to.x), _mm_load_ps1(&t)));
			_mm_storeu_ps(&x, tmp);
		}

		inline void StoreInterpolateNormals(const Vector &n1, const Vector &n2, const float &t)
		{
			float a, sinA;

			a = acos(n1.GetDot(n2));
			sinA = 1.0f / sinf(a);

			x = (sin((1.0f - t)*a)*n1.x + sin(t*a)*n2.x) * sinA;
			y = (sin((1.0f - t)*a)*n1.y + sin(t*a)*n2.y) * sinA;
			z = (sin((1.0f - t)*a)*n1.z + sin(t*a)*n2.z) * sinA;
		}

		inline void StoreClosestInSegment(const Vector &point, const Vector &p1, const Vector &p2)
		{
			float t;
			Vector lineDir;

			lineDir.x = p2.x - p1.x;
			lineDir.y = p2.y - p1.y;
			lineDir.z = p2.z - p1.z;

			t = lineDir.GetDot();
			if (Math::isZero(t))
			{
				x = p1.x;
				y = p1.y;
				z = p1.z;
				return;
			}

			t = ((point.x - p1.x)*lineDir.x + (point.y - p1.y)*lineDir.y + (point.z - p1.z)*lineDir.z) / t;

			t = Math::fClamp(t, 0.0f, 1.0f);
			x = p1.x + (t * lineDir.x);
			y = p1.y + (t * lineDir.y);
			z = p1.z + (t * lineDir.z);
		}
	};

	HALIGN_16BYTES
	struct Vector4{
		float x, y, z, w;

		//static methods

		static float CalcMagnitude(const Vector4 &vec)
		{
			return sqrt(vec.x*vec.x + vec.y*vec.y + vec.z*vec.z + vec.w*vec.w);
		}

		static float CalcDistance(const Vector4 &vec1, const Vector4 &vec2)
		{
			float d1 = vec1.x - vec2.x;
			float d2 = vec1.y - vec2.y;
			float d3 = vec1.z - vec2.z;
			float d4 = vec1.w - vec2.w;
			return sqrt(d1*d1 + d2*d2 + d3*d3 + d4*d4);
		}

		static float CalcDot(const Vector4 &vec1, const Vector4 &vec2)
		{
			return (vec1.x*vec2.x + vec1.y*vec2.y + vec1.z*vec2.z + vec1.w*vec2.w);
		}

		static float CalcDot(const Vector &vec1, const Vector4 &vec2)
		{
			return (vec1.x*vec2.x + vec1.y*vec2.y + vec1.z*vec2.z + vec2.w);
		}

		//instance methods

		inline Vector4()
		{
			_mm_storeu_ps(&x, _mm_setzero_ps());
		}

		inline Vector4(const Vector &s)
		{
			_mm_storeu_ps(&x, _mm_loadu_ps(&s.x)); w = 1.0f;
		}

		inline Vector4(const Vector &s, const float vw)
		{
			_mm_storeu_ps(&x, _mm_loadu_ps(&s.x)); w = vw;
		}

		inline Vector4(const Vector4 &s)
		{
			_mm_storeu_ps(&x, _mm_loadu_ps(&s.x));
		}

		explicit inline Vector4(const float scalar)
		{
			_mm_storeu_ps(&x, _mm_load_ps1(&scalar));
		}

		explicit inline Vector4(const float vx, const float vy, const float vz, const float vw)
		{
			x = vx; y = vy; z = vz; w = vw;
		}

		explicit inline Vector4(const float * const v)
		{
			_mm_storeu_ps(&x, _mm_loadu_ps(v));
		}

		inline ~Vector4()
		{
			return;
		}

		inline operator float *(void)
		{
			return &x;
		}

		inline operator const float *(void) const
		{
			return &x;
		}

		inline Vector4& operator=(const Vector4 &v)
		{
			_mm_storeu_ps(&x, _mm_loadu_ps(&v.x));
			return *this;
		}

		inline Vector4& operator=(const float *n)
		{
			x = n[0]; y = n[1]; z = n[2]; w = n[3];
			return *this;
		}

		inline Vector4& operator=(const float n)
		{
			_mm_storeu_ps(&x, _mm_load_ps1(&n));
			return *this;
		}

		inline void operator+=(const Vector4& v)
		{
			_mm_storeu_ps(&x, _mm_add_ps(_mm_loadu_ps(&x), _mm_loadu_ps(&v.x)));
		}

		inline void operator-=(const Vector4& v)
		{
			_mm_storeu_ps(&x, _mm_sub_ps(_mm_loadu_ps(&x), _mm_loadu_ps(&v.x)));
		}

		inline void operator*=(const Vector4& v)
		{
			_mm_storeu_ps(&x, _mm_mul_ps(_mm_loadu_ps(&x), _mm_loadu_ps(&v.x)));
		}

		inline void operator/=(const Vector4& v)
		{
			_mm_storeu_ps(&x, _mm_div_ps(_mm_loadu_ps(&x), _mm_loadu_ps(&v.x)));
		}

		inline void operator+=(const float &n)
		{
			_mm_storeu_ps(&x, _mm_add_ps(_mm_loadu_ps(&x), _mm_load_ps1(&n)));
		}

		inline void operator-=(const float &n)
		{
			_mm_storeu_ps(&x, _mm_sub_ps(_mm_loadu_ps(&x), _mm_load_ps1(&n)));
		}

		inline void operator*=(const float &n)
		{
			_mm_storeu_ps(&x, _mm_mul_ps(_mm_loadu_ps(&x), _mm_load_ps1(&n)));
		}

		inline void operator/=(const float &n)
		{
			_mm_storeu_ps(&x, _mm_div_ps(_mm_loadu_ps(&x), _mm_load_ps1(&n)));
		}

		inline float operator[](int index) const
		{
			assert((index >= 0) && (index < 4));
			return (&x)[index];
		}

		inline float& operator[](int index)
		{
			assert((index >= 0) && (index < 4));
			return (&x)[index];
		}

		inline Vector operator-() const
		{
			return Vector(_mm_mul_ps(_mm_loadu_ps(&x), _mm_set_ps1(-1.0f)));
		}

		Vector4& Set(const Vector4 &vec)
		{
			_mm_storeu_ps(&x, _mm_loadu_ps(&vec.x));
			return *this;
		}

		Vector4& Set(const Vector& vec, const float &w)
		{
			_mm_storeu_ps(&x, _mm_loadu_ps(&vec.x));
			this->w = w;
			return *this;
		}

		Vector4& Set(const float *s)
		{
			_mm_storeu_ps(&x, _mm_loadu_ps(s));
			return *this;
		}

		Vector4& Set(const float &x, const float &y, const float &z, const float &w)
		{
			this->x = x;
			this->y = y;
			this->z = z;
			this->w = w;
			return *this;
		}

		float GetMagnitude() const
		{
			return Vector4::CalcMagnitude(*this);
		}

		void Normalize()
		{
			float tamanho = 1.0f / sqrt(x*x + y*y + z*z + w*w);
			x *= tamanho;
			y *= tamanho;
			z *= tamanho;
			w *= tamanho;
		}

		float GetDistance(const Vector4 &vec) const
		{
			return Vector4::CalcDistance(*this, vec);
		}

		float GetDot(const Vector4 &vec) const
		{
			return Vector4::CalcDot(*this, vec);
		}

		float GetDot(const Vector &vec) const
		{
			return Vector4::CalcDot(vec, *this);
		}

		inline void StoreInterpolate(const Vector4 &from, const Vector4 &to, const float &t)
		{
			__m128 tmp;

			tmp = _mm_mul_ps(_mm_loadu_ps(&from.x), _mm_set_ps1(1.0f - t));
			tmp = _mm_add_ps(tmp, _mm_mul_ps(_mm_loadu_ps(&to.x), _mm_load_ps1(&t)));
			_mm_storeu_ps(&x, tmp);
		}

		inline void StoreInterpolate(const float * const from, const float * const to, const float &t)
		{
			__m128 tmp;

			tmp = _mm_mul_ps(_mm_loadu_ps(from), _mm_set_ps1(1.0f - t));
			tmp = _mm_add_ps(tmp, _mm_mul_ps(_mm_loadu_ps(to), _mm_load_ps1(&t)));
			_mm_storeu_ps(&x, tmp);
		}

		inline void StoreInterpolate(const Vector4 &to, const float &t)
		{
			__m128 tmp;

			tmp = _mm_mul_ps(_mm_loadu_ps(&x), _mm_set_ps1(1.0f - t));
			tmp = _mm_add_ps(tmp, _mm_mul_ps(_mm_loadu_ps(&to.x), _mm_load_ps1(&t)));
			_mm_storeu_ps(&x, tmp);
		}
	};

} //HorseRadish
