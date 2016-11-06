#include "plane.hpp"

#include "math.hpp"

namespace HorseRadish
{
	void Plane::setFromPoints(const Vector3f &p0, const Vector3f &p1, const Vector3f &p2)
	{
		Vector3f normal;

		normal.storeNormal(p0, p1, p2);
		mA = normal[0];
		mB = normal[1];
		mC = normal[2];
		mD = -normal.getDot(p0);
	}

	void Plane::setFromPoints(const float *p0, const float *p1, const float *p2)
	{
		Vector3f normal;

		normal.storeNormal(p0, p1, p2);
		mA = normal[0];
		mB = normal[1];
		mC = normal[2];
		mD = -normal.getDot(p0);
	}

	Plane Plane::lerp(const Plane &p2, float factor) const
	{
		Plane result;
		result.mA = mA*factor;
		result.mB = mB*factor;
		result.mC = mC*factor;

		Vector3f vecAux(p2.mA, p2.mB, p2.mC);
		vecAux *= (1.0f - factor);

		result.mA += vecAux[0];
		result.mB += vecAux[1];
		result.mC += vecAux[2];
		result.normalizeNormal();

		result.mD = (mD * factor) + (p2.mD * (1.0f - factor));

		return result;
	}

	void Plane::normalize()
	{
		float sizeInv = 1.0f / sqrtf(mA*mA + mB*mB + mC*mC);
		mA *= sizeInv;
		mB *= sizeInv;
		mC *= sizeInv;
		mD *= sizeInv;
	}

	void Plane::normalizeNormal()
	{
		float sizeInv = 1.0f / sqrtf(mA*mA + mB*mB + mC*mC);
		mA *= sizeInv;
		mB *= sizeInv;
		mC *= sizeInv;
	}

	bool Plane::testIntersectRay(const Vector3f &origin, const Vector3f &dir) const
	{
		Vector3f result;
		return testIntersectRay(origin, dir, result);
	}

	bool Plane::testIntersectRay(const Vector3f &origin, const Vector3f &dir, Vector3f& result) const
	{
		float dot = dir.getDot(mA, mB, mC);
		if (Math::isZero(dot))
			return false;

		float t = -(origin.getDot(mA, mB, mC) + mD) / dot;
		if (t < 0.0f)
			return false;

		result[0] = origin[0] + (dir[0] * t);
		result[1] = origin[1] + (dir[1] * t);
		result[2] = origin[2] + (dir[2] * t);

		return true;
	}

	bool Plane::testIntersectLine(const Vector3f &p1, const Vector3f &p2) const
	{
		Vector3f result;
		return testIntersectLine(p1, p2, result);
	}

	bool Plane::testIntersectLine(const Vector3f &p1, const Vector3f &p2, Vector3f& result) const
	{
		auto dir = p2 - p1;
		float dot = dir.getDot(mA, mB, mC);
		if (Math::isZero(dot))
			return false;

		float t = -(p1.getDot(mA, mB, mC) + mD) / dot;

		result[0] = p1[0] + (dir[0] * t);
		result[1] = p1[1] + (dir[1] * t);
		result[2] = p1[2] + (dir[2] * t);
		
		return true;
	}

	bool Plane::testIntersectLineSegment(const Vector3f &p1, const Vector3f &p2) const
	{
		Vector3f result;
		return testIntersectLineSegment(p1, p2, result);
	}

	bool Plane::testIntersectLineSegment(const Vector3f &p1, const Vector3f &p2, Vector3f& result) const
	{
		auto dir = p2 - p1;
		float dot = dir.getDot(mA, mB, mC);
		if (Math::isZero(dot))
			return false;

		float t = -(p1.getDot(mA, mB, mC) + mD) / dot;
		if (t<0.0f || t>1.0f)
			return false;

		result[0] = p1[0] + (dir[0] * t);
		result[1] = p1[1] + (dir[1] * t);
		result[2] = p1[2] + (dir[2] * t);

		return true;
	}

	bool Plane::testIntersectPlanes(const Plane &p2, const Plane &p3) const
	{
		Vector3f result;
		return testIntersectPlanes(p2, p3, result);
	}

	bool Plane::testIntersectPlanes(const Plane &p2, const Plane &p3, Vector3f& result) const
	{
		float denominator;
		Vector3f pNormal, p2Normal, p3Normal, tmp1, tmp2, tmp3;

		pNormal.set(mA, mB, mC);
		p2Normal.set(p2.mA, p2.mB, p2.mC);
		p3Normal.set(p3.mA, p3.mB, p3.mC);
		tmp1.storeCrossProduct(p2Normal, p3Normal);
		denominator = pNormal.getDot(tmp1);

		if (Math::isZero(denominator))
			return false;

		tmp1.storeCrossProduct(p2Normal, p3Normal);
		tmp2.storeCrossProduct(p3Normal, pNormal);
		tmp3.storeCrossProduct(pNormal, p2Normal);
		tmp1 *= mD;
		tmp2 *= p2.mD;
		tmp3 *= p3.mD;

		result = tmp1 + tmp2 + tmp3;
		result /= (-denominator);

		return true;
	}

	bool Plane::testIntersectSweptSphere(float sphereRadius, const Vector3f &spherePrevPos, const Vector3f &sphereCurPos, Vector3f * const hitPoint, float * const hitTime) const
	{
		float d0 = getDotCoord(spherePrevPos);
		float d1 = getDotCoord(sphereCurPos);

		if (std::abs(d0) <= sphereRadius)
		{
			if (hitPoint)
				hitPoint->set(spherePrevPos);
			if (hitTime)
				*hitTime = 0.0f;
			return true;
		}

		if (d0 > sphereRadius && d1 < sphereRadius)
		{
			float htime = (d0 - sphereRadius) / (d0 - d1);
			if (hitTime)
				*hitTime = htime;
			if (hitPoint)
				hitPoint->storeInterpolate(spherePrevPos, sphereCurPos, htime);
			return true;
		}

		return false;
	}

	Plane::Position Plane::classifyPoint(const Vector3f &point) const
	{
		float calcDot = point.getDot(mA, mB, mC) + mD;

		if (Math::isZero(calcDot))
			return Plane::Position::COPLANAR;
		if (calcDot > 0.0f)
			return Plane::Position::FRONT;

		return Plane::Position::BEHIND;
	}

	Plane::Position Plane::classifyTri(const Vector3f &p1, const Vector3f &p2, const Vector3f &p3) const
	{
		Position c1 = classifyPoint(p1);
		Position c2 = classifyPoint(p2);
		Position c3 = classifyPoint(p3);

		return (((c1 == c2) && (c2 == c3)) ? c1 : Position::INTERSECT);
	}
}