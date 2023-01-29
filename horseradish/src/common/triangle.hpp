#pragma once

#include "math.hpp"
#include "plane.hpp"
#include "vector.hpp"

#include <immintrin.h>
#include <xmmintrin.h>
#include <smmintrin.h>

namespace hr
{
	/*
	Stores the (three) points of a triangle and its respective normal

	NOTE: the winding order is counter-clockwise
	*/
	template<typename TVectorType = Vector3f>
	class Triangle
	{
		static_assert(std::is_same_v<TVectorType, Vector3f> || std::is_same_v<TVectorType, Vector3d>, "Base type must be either Vector3f or Vector3d");
		static_assert(std::is_trivially_copyable_v<TVectorType>, "For performance reasons, the vector type should be trivially copyable");

		TVectorType mPoints[3];

		template<class T>
		static constexpr T kZero = T(0.0);
		template<class T>
		static constexpr T kOne = T(1.0);
		template<class T>
		static constexpr T kThree = T(3.0);

		using BaseDataType = typename TVectorType::DataType;

	public:
		using DataType = typename TVectorType;
		using PlaneType = Plane<typename TVectorType::DataType>;

		struct Hit {
			typename TVectorType::DataType rayT;
			typename TVectorType::DataType barycentricU, barycentricV;
		};

	public:
		static TVectorType calcNormal(const TVectorType& p1, const TVectorType& p2, const TVectorType& p3) noexcept
		{
			static_assert(std::is_same_v<TVectorType, Vector3f> || std::is_same_v<TVectorType, Vector3d>, "Base type must be either Vector3f or Vector3d");

			if constexpr (std::is_same_v<TVectorType, Vector3f>)
			{
				__m128 vec1, vec2, tmp1, tmp2;

				tmp1 = _mm_load_ps(p1.data());
				vec1 = _mm_sub_ps(_mm_load_ps(p2.data()), tmp1);
				vec2 = _mm_sub_ps(_mm_load_ps(p3.data()), tmp1);

				tmp1 = _mm_mul_ps(_mm_shuffle_ps(vec1, vec1, _MM_SHUFFLE(3, 0, 2, 1)), _mm_shuffle_ps(vec2, vec2, _MM_SHUFFLE(3, 1, 0, 2)));
				tmp2 = _mm_mul_ps(_mm_shuffle_ps(vec1, vec1, _MM_SHUFFLE(3, 1, 0, 2)), _mm_shuffle_ps(vec2, vec2, _MM_SHUFFLE(3, 0, 2, 1)));
				vec1 = _mm_sub_ps(tmp1, tmp2);

				vec2 = _mm_rsqrt_ps(_mm_dp_ps(vec1, vec1, 0x70 | 0xF));
				return TVectorType{ _mm_mul_ps(vec1, vec2) };
			}
			else
			{
				__m256d vec0 = _mm256_load_pd(p1.data());
				__m256d vec1 = _mm256_sub_pd(_mm256_load_pd(p2.data()), vec0);
				__m256d vec2 = _mm256_sub_pd(_mm256_load_pd(p3.data()), vec0);

				__m256d xa = _mm256_mul_pd(_mm256_permute4x64_pd(vec1, _MM_SHUFFLE(3, 0, 2, 1)), _mm256_permute4x64_pd(vec2, _MM_SHUFFLE(3, 1, 0, 2)));
				__m256d xb = _mm256_mul_pd(_mm256_permute4x64_pd(vec1, _MM_SHUFFLE(3, 1, 0, 2)), _mm256_permute4x64_pd(vec2, _MM_SHUFFLE(3, 0, 2, 1)));
				Vector3d result{ _mm256_sub_pd(xa, xb) };

				result.normalize();
				return result;
			}
		}

		static TVectorType calcNormal(const typename TVectorType::DataType p1[3], const typename TVectorType::DataType p2[3], const typename TVectorType::DataType p3[3]) noexcept
		{
			return calcNormal(TVectorType{ p1 }, TVectorType{ p2 }, TVectorType{ p3 });
		}

		static PlaneType calcPlane(const TVectorType& p1, const TVectorType& p2, const TVectorType& p3) noexcept
		{
			auto normal = calcNormal(p1, p2, p3);
			return PlaneType{ normal[0], normal[1] , normal[2], -normal.getDot(p1) };
		}

		static PlaneType calcPlane(const typename TVectorType::DataType p1[3], const typename TVectorType::DataType p2[3], const typename TVectorType::DataType p3[3]) noexcept
		{
			auto normal = calcNormal(p1, p2, p3);
			return PlaneType{ normal[0], normal[1] , normal[2], -normal.getDot(p1) };
		}

	public:
		constexpr Triangle() = default;

		constexpr explicit Triangle(const TVectorType& p1, const TVectorType& p2, const TVectorType& p3) noexcept
			: mPoints{ p1, p2, p3 }
		{ }

		const TVectorType& operator[](const size_t index) const noexcept
		{
			return mPoints[index % 3];
		}

		TVectorType normal() const noexcept
		{
			return calcNormal(mPoints[0], mPoints[1], mPoints[2]);
		}

		TVectorType orthocenter() const noexcept
		{
			return (mPoints[0] + mPoints[1] + mPoints[2]) / kThree<BaseDataType>;
		}

		PlaneType plane() const noexcept
		{
			auto normal = calcNormal(mPoints[0], mPoints[1], mPoints[2]);
			return PlaneType{ normal[0], normal[1], normal[2], -normal.getDot(mPoints[0])};
		}

		bool intersects(const Ray<Vector3d>& ray, Hit& hit) const noexcept
		{
			auto v0v1 = mPoints[1] - mPoints[0];
			auto v0v2 = mPoints[2] - mPoints[0];
			auto pvec = ray.direction().crossProduct(v0v2);
			auto det = v0v1.getDot(pvec);
			if (det <= 1e-6) //cull back face
				return false;

			auto invDet = kOne<BaseDataType> / det;

			auto tvec = ray.origin() - mPoints[0];
			auto u = tvec.getDot(pvec) * invDet;
			if ((u < kZero<BaseDataType>) || (u > kOne<BaseDataType>))
				return false;

			auto qvec = tvec.crossProduct(v0v1);
			auto v = ray.direction().getDot(qvec) * invDet;
			if ((v < kZero<BaseDataType>) || ((u + v) > kOne<BaseDataType>))
				return false;

			hit.rayT = v0v2.getDot(qvec) * invDet;
			hit.barycentricU = u;
			hit.barycentricV = v;

			return true;
		}

		bool intersects(const Ray<Vector3d>& ray, const typename TVectorType::DataType rayDistMin, const typename TVectorType::DataType rayDistMax, Hit& hit) const noexcept
		{
			auto v0v1 = mPoints[1] - mPoints[0];
			auto v0v2 = mPoints[2] - mPoints[0];
			auto pvec = ray.direction().crossProduct(v0v2);
			auto det = v0v1.dot(pvec);
			if (det <= 1e-6) //cull back face
				return false;

			auto invDet = kOne<BaseDataType> / det;

			auto tvec = ray.origin() - mPoints[0];
			auto u = tvec.dot(pvec) * invDet;
			if ((u < kZero<BaseDataType>) || (u > kOne<BaseDataType>))
				return false;

			auto qvec = tvec.crossProduct(v0v1);
			auto v = ray.direction().dot(qvec) * invDet;
			if ((v < kZero<BaseDataType>) || ((u + v) > kOne<BaseDataType>))
				return false;

			auto t = v0v2.dot(qvec) * invDet;
			if ((t < rayDistMin) || (t >= rayDistMax))
				return false;

			hit.rayT = t;
			hit.barycentricU = u;
			hit.barycentricV = v;

			return true;
		}
	};
}
