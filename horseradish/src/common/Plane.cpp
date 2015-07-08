#include "Plane.hpp"

#include "Math.hpp"

namespace HorseRadish
{
	Plane::Plane(const Plane &plane)
	{
		a = plane.a;
		b = plane.b;
		c = plane.c;
		d = plane.d;
	}

	Plane::Plane(const float nx, const float ny, const float nz, const float nd)
	{
		a = nx; b = ny; c = nz;
		d = nd;
	}

	Plane::Plane(const Vector3f &nN, const float nd)
	{
		a = nN[0];
		b = nN[1];
		c = nN[2];
		d = nd;
	}

	void Plane::SetFromPoints(const Vector3f &p0, const Vector3f &p1, const Vector3f &p2)
	{
		Vector3f normal;

		normal.storeNormal(p0, p1, p2);
		a = normal[0];
		b = normal[1];
		c = normal[2];
		d = -normal.getDot(p0);
	}

	void Plane::SetFromPoints(const float *p0, const float *p1, const float *p2)
	{
		Vector3f normal;

		normal.storeNormal(p0, p1, p2);
		a = normal[0];
		b = normal[1];
		c = normal[2];
		d = -normal.getDot(p0);
	}

	void Plane::Lerp(const Plane &p2, const float factor, Plane &result)
	{
		result.a = a*factor;
		result.b = b*factor;
		result.c = c*factor;

		Vector3f vecAux(p2.a, p2.b, p2.c);
		vecAux *= (1.0f - factor);

		result.a += vecAux[0];
		result.b += vecAux[1];
		result.c += vecAux[2];
		result.NormalizeNormal();

		result.d = d*factor + p2.d*(1.0f - factor);
	}

	void Plane::Normalize()
	{
		float sizeInv;

		sizeInv = 1.0f / sqrtf(a*a + b*b + c*c);
		a *= sizeInv;
		b *= sizeInv;
		c *= sizeInv;
		d *= sizeInv;
	}

	void Plane::NormalizeNormal()
	{
		float sizeInv;

		sizeInv = 1.0f / sqrtf(a*a + b*b + c*c);
		a *= sizeInv;
		b *= sizeInv;
		c *= sizeInv;
	}

	bool Plane::TestIntersectRay(const Vector3f &origin, const Vector3f &dir) const
	{
		Vector3f result;
		return TestIntersectRay(origin, dir);
	}

	bool Plane::TestIntersectRay(const Vector3f &origin, const Vector3f &dir, Vector3f& result) const
	{
		float dot = dir.getDot(a, b, c);
		if (Math::isZero(dot))
			return false;

		float t = -(origin.getDot(a, b, c) + d) / dot;
		if (t < 0.0f)
			return false;

		result[0] = origin[0] + (dir[0] * t);
		result[1] = origin[1] + (dir[1] * t);
		result[2] = origin[2] + (dir[2] * t);

		return true;
	}

	bool Plane::TestIntersectLine(const Vector3f &p1, const Vector3f &p2) const
	{
		Vector3f result;
		return TestIntersectLine(p1, p2);
	}

	bool Plane::TestIntersectLine(const Vector3f &p1, const Vector3f &p2, Vector3f& result) const
	{
		auto dir = p2 - p1;

		float dot = dir.getDot(a, b, c);
		if (Math::isZero(dot))
			return false;

		float t = -(p1.getDot(a, b, c) + d) / dot;

		result[0] = p1[0] + (dir[0] * t);
		result[1] = p1[1] + (dir[1] * t);
		result[2] = p1[2] + (dir[2] * t);
		
		return true;
	}

	bool Plane::TestIntersectLineSegment(const Vector3f &p1, const Vector3f &p2) const
	{
		Vector3f result;
		return TestIntersectLineSegment(p1, p2);
	}

	bool Plane::TestIntersectLineSegment(const Vector3f &p1, const Vector3f &p2, Vector3f& result) const
	{
		auto dir = p2 - p1;

		float dot = dir.getDot(a, b, c);
		if (Math::isZero(dot))
			return false;

		float t = -(p1.getDot(a, b, c) + d) / dot;
		if (t<0.0f || t>1.0f)
			return false;

		result[0] = p1[0] + (dir[0] * t);
		result[1] = p1[1] + (dir[1] * t);
		result[2] = p1[2] + (dir[2] * t);

		return true;
	}

	bool Plane::TestIntersectPlanes(const Plane &p2, const Plane &p3) const
	{
		Vector3f result;
		return TestIntersectPlanes(p2, p3);
	}

	bool Plane::TestIntersectPlanes(const Plane &p2, const Plane &p3, Vector3f& result) const
	{
		float denominator;
		Vector3f pNormal, p2Normal, p3Normal, tmp1, tmp2, tmp3;

		pNormal.set(a, b, c);
		p2Normal.set(p2.a, p2.b, p2.c);
		p3Normal.set(p3.a, p3.b, p3.c);
		tmp1.storeCrossProduct(p2Normal, p3Normal);
		denominator = pNormal.getDot(tmp1);

		if (Math::isZero(denominator))
			return false;

		tmp1.storeCrossProduct(p2Normal, p3Normal);
		tmp2.storeCrossProduct(p3Normal, pNormal);
		tmp3.storeCrossProduct(pNormal, p2Normal);
		tmp1 *= d;
		tmp2 *= p2.d;
		tmp3 *= p3.d;

		result = tmp1 + tmp2 + tmp3;
		result /= (-denominator);

		return true;
	}

	bool Plane::TestIntersectSweptSphere(const float &sphereRadius, const Vector3f &spherePrevPos, const Vector3f &sphereCurPos, Vector3f * const hitPoint, float * const hitTime) const
	{
		float d0 = GetDotCoord(spherePrevPos);
		float d1 = GetDotCoord(sphereCurPos);

		if (Math::fAbs(d0) <= sphereRadius)
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

	Plane::Position Plane::ClassifyPoint(const Vector3f &point) const
	{
		float calcDot = point.getDot(a, b, c) + d;

		if (Math::isZero(calcDot))
			return Plane::Position::COPLANAR;
		if (calcDot > 0.0f)
			return Plane::Position::FRONT;
		return Plane::Position::BEHIND;
	}

	Plane::Position Plane::ClassifyTri(const Vector3f &p1, const Vector3f &p2, const Vector3f &p3) const
	{
		Position c1, c2, c3;

		c1 = ClassifyPoint(p1);
		c2 = ClassifyPoint(p2);
		c3 = ClassifyPoint(p3);

		if (c1 == c2 && c2 == c3)
			return c1;

		return Position::INTERSECT;
	}

} //HorseRadish