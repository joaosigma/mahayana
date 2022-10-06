#pragma once

#include "ray.hpp"
#include "math.hpp"
#include "vector.hpp"

namespace hr
{
	template<typename TDataType>
	class Plane
	{
		//scalar equation of plane: ax + by + cz = d
		TDataType mA, mB, mC, mD;

		static_assert(std::is_arithmetic<TDataType>::value, "Data type must be arithmetic (e.g.: float, unsigned char, etc.)");
		static_assert(std::is_trivially_copyable_v<TDataType>, "For performance reasons, the data type should be trivially copyable");

	public:
		using DataType = TDataType;
		using VectorType = Vector<TDataType, 3>;
		using RayType = Ray<Vector<TDataType, 3>>;

		enum class Position { CoPlanar, Front, Behind, Intersect };

	public:
		constexpr Plane() noexcept
		{
			mA = mB = mC = mD = 0;
		}

		constexpr Plane(const Plane&) = default;
		constexpr Plane& operator=(const Plane&) = default;
		constexpr Plane(Plane&&) = default;
		constexpr Plane& operator=(Plane&&) = default;

		explicit constexpr Plane(TDataType a, TDataType b, TDataType c, TDataType d)
			: mA{ a }, mB{ b }, mC{ c }, mD{ d }
		{ }

		explicit constexpr Plane(const VectorType& normal, TDataType d)
			: mA{ normal[0] }, mB{ normal[1] }, mC{ normal[2] }, mD{ d }
		{ }

		Plane& set(TDataType ax, TDataType by, TDataType cz, TDataType d)
		{
			mA = ax;
			mB = by;
			mC = cz;
			mD = d;
			return *this;
		}

		Plane& setNormal(const VectorType& normal)
		{
			mA = normal[0];
			mB = normal[1];
			mC = normal[2];
			return *this;
		}

		Plane& setNormal(TDataType ax, TDataType by, TDataType cz)
		{
			mA = ax;
			mB = by;
			mC = cz;
			return *this;
		}

		Plane& setD(TDataType d)
		{
			mD = d;
			return *this;
		}

		Plane lerp(const Plane& p, TDataType factor) const
		{
			Plane result;
			result.mA = mA * factor;
			result.mB = mB * factor;
			result.mC = mC * factor;

			VectorType vecAux(p.mA, p.mB, p.mC);
			vecAux *= (1.0f - factor);

			result.mA += vecAux[0];
			result.mB += vecAux[1];
			result.mC += vecAux[2];
			result.normalizeNormal();

			result.mD = (mD * factor) + (p.mD * (1.0f - factor));

			return result;
		}

		void normalize(void)
		{
			auto sizeInv = Math::sqrtInv(mA * mA + mB * mB + mC * mC);
			mA *= sizeInv;
			mB *= sizeInv;
			mC *= sizeInv;
			mD *= sizeInv;
		}

		void normalizeNormal(void)
		{
			
			auto sizeInv = Math::sqrtInv(mA * mA + mB * mB + mC * mC);
			mA *= sizeInv;
			mB *= sizeInv;
			mC *= sizeInv;
		}

		void negateNormal()
		{
			mA *= -1.0f;
			mB *= -1.0f;
			mC *= -1.0f;
		}

		void negateD()
		{
			mD *= -1.0f;
		}

		void calcD(const VectorType& pointOnPlane)
		{
			mD = -pointOnPlane.getDot(mA, mB, mC);
		}

		TDataType distance(const VectorType& point) const
		{
			return (point.getDot(mA, mB, mC) + mD);
		}

		VectorType normal() const
		{
			return VectorType(mA, mB, mC);
		}

		TDataType d() const
		{
			return mD;
		}

		TDataType getDotCoord(const VectorType& point) const
		{
			return (point.getDot(mA, mB, mC) + mD);
		}

		TDataType getDotNormal(const VectorType& point) const
		{
			return (point.getDot(mA, mB, mC));
		}

		bool intersects(const RayType& ray) const
		{
			TDataType hitDistance;
			return intersects(ray);
		}

		bool intersects(const RayType& ray, TDataType& hitDistance) const
		{
			auto dot = ray.dir().getDot(mA, mB, mC);
			if (Math::isZero(dot))
				return false;

			hitDistance = -(ray.origin().getDot(mA, mB, mC) + mD) / dot;
			return (hitDistance >= 0.0f);
		}

		bool intersects(const VectorType& lineStart, const VectorType& lineEnd) const
		{
			TDataType hitDistance;
			return intersects(lineStart, lineEnd);
		}

		bool intersects(const VectorType& lineStart, const VectorType& lineEnd, VectorType& result) const
		{
			RayType ray{ lineStart, VectorType::calcNormalize(lineEnd - lineStart) };

			float hitDistance;
			if (!intersects(ray, hitDistance))
				return false;

			if (hitDistance > VectorType::calcDistance(lineStart, lineEnd))
				return false;

			result = ray.pointAt(hitDistance);

			return true;
		}

		bool intersects(const Plane& p2, const Plane& p3, VectorType& result) const
		{
			VectorType pNormal, p2Normal, p3Normal, tmp1, tmp2, tmp3;

			pNormal.set(mA, mB, mC);
			p2Normal.set(p2.mA, p2.mB, p2.mC);
			p3Normal.set(p3.mA, p3.mB, p3.mC);
			tmp1 = VectorType::calcCrossProduct(p2Normal, p3Normal);

			auto denominator = pNormal.getDot(tmp1);
			if (Math::isZero(denominator))
				return false;

			tmp1 = Vector3f::calcCrossProduct(p2Normal, p3Normal);
			tmp2 = Vector3f::calcCrossProduct(p3Normal, pNormal);
			tmp3 = Vector3f::calcCrossProduct(pNormal, p2Normal);
			tmp1 *= mD;
			tmp2 *= p2.mD;
			tmp3 *= p3.mD;

			result = tmp1 + tmp2 + tmp3;
			result /= (-denominator);

			return true;
		}

		Position classifyPoint(const VectorType& point) const
		{
			auto calcDot = point.getDot(mA, mB, mC) + mD;

			if (Math::isZero(calcDot))
				return Plane::Position::CoPlanar;
			if (calcDot > 0.0f)
				return Plane::Position::Front;
			return Plane::Position::Behind;
		}

		Position classifyTri(const VectorType& p1, const VectorType& p2, const VectorType& p3) const
		{
			auto c1 = classifyPoint(p1);
			auto c2 = classifyPoint(p2);
			auto c3 = classifyPoint(p3);

			return (((c1 == c2) && (c2 == c3)) ? c1 : Position::Intersect);
		}
	};
}
