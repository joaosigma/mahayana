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

	Plane::Plane(const Vector &nN, const float nd)
	{
		a = nN.x; b = nN.y; c = nN.z;
		d = nd;
	}

	void Plane::SetFromPoints(const Vector &p0, const Vector &p1, const Vector &p2)
	{
		Vector normal;

		normal.StoreNormal(p0, p1, p2);
		a = normal.x;
		b = normal.y;
		c = normal.z;
		d = -normal.GetDot(p0);
	}

	void Plane::SetFromPoints(const float *p0, const float *p1, const float *p2)
	{
		Vector normal;

		normal.StoreNormal(p0, p1, p2);
		a = normal.x;
		b = normal.y;
		c = normal.z;
		d = -normal.GetDot(p0);
	}

	void Plane::Lerp(const Plane &p2, const float factor, Plane &result)
	{
		result.a = a*factor;
		result.b = b*factor;
		result.c = c*factor;

		Vector vecAux(p2.a, p2.b, p2.c);
		vecAux *= (1.0f - factor);

		result.a += vecAux.x;
		result.b += vecAux.y;
		result.c += vecAux.z;
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

	bool Plane::TestIntersectRay(const Vector &origin, const Vector &dir, Vector *resultado) const
	{
		float dot = dir.GetDot(a, b, c);
		if (Math::isZero(dot))
			return false;

		float t = -(origin.GetDot(a, b, c) + d) / dot;
		if (t < 0.0f)
			return false;

		if (resultado)
		{
			resultado->x = origin.x + (dir.x*t);
			resultado->y = origin.y + (dir.y*t);
			resultado->z = origin.z + (dir.z*t);
		}

		return true;
	}

	bool Plane::TestIntersectLine(const Vector &p1, const Vector &p2, Vector *resultado) const
	{
		auto dir = p2 - p1;

		float dot = dir.GetDot(a, b, c);
		if (Math::isZero(dot))
			return false;

		float t = -(p1.GetDot(a, b, c) + d) / dot;

		if (resultado)
		{
			resultado->x = p1.x + (dir.x*t);
			resultado->y = p1.y + (dir.y*t);
			resultado->z = p1.z + (dir.z*t);
		}

		return true;
	}

	bool Plane::TestIntersectLineSegment(const Vector &p1, const Vector &p2, Vector *resultado) const
	{
		auto dir = p2 - p1;

		float dot = dir.GetDot(a, b, c);
		if (Math::isZero(dot))
			return false;

		float t = -(p1.GetDot(a, b, c) + d) / dot;
		if (t<0.0f || t>1.0f)
			return false;

		if (resultado)
		{
			resultado->x = p1.x + (dir.x*t);
			resultado->y = p1.y + (dir.y*t);
			resultado->z = p1.z + (dir.z*t);
		}

		return true;
	}

	bool Plane::TestIntersectPlanes(const Plane &p2, const Plane &p3, Vector *resultado) const
	{
		float denominator;
		Vector pNormal, p2Normal, p3Normal, tmp1, tmp2, tmp3;

		pNormal.Set(a, b, c);
		p2Normal.Set(p2.a, p2.b, p2.c);
		p3Normal.Set(p3.a, p3.b, p3.c);
		tmp1.StoreCrossProduct(p2Normal, p3Normal);
		denominator = pNormal.GetDot(tmp1);

		if (Math::isZero(denominator))
			return false;

		tmp1.StoreCrossProduct(p2Normal, p3Normal);
		tmp2.StoreCrossProduct(p3Normal, pNormal);
		tmp3.StoreCrossProduct(pNormal, p2Normal);
		tmp1 *= d;
		tmp2 *= p2.d;
		tmp3 *= p3.d;

		if (resultado)
		{
			(*resultado) = tmp1 + tmp2 + tmp3;
			(*resultado) /= (-denominator);
		}

		return true;
	}

	bool Plane::TestIntersectSweptSphere(const float &sphereRadius, const Vector &spherePrevPos, const Vector &sphereCurPos, Vector * const hitPoint, float * const hitTime) const
	{
		float d0 = GetDotCoord(spherePrevPos);
		float d1 = GetDotCoord(sphereCurPos);

		if (Math::fAbs(d0) <= sphereRadius)
		{
			if (hitPoint)
				hitPoint->Set(spherePrevPos);
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
				hitPoint->StoreInterpolate(spherePrevPos, sphereCurPos, htime);
			return true;
		}

		return false;
	}

	Plane::Position Plane::ClassifyPoint(const Vector &point) const
	{
		float calcDot = point.GetDot(a, b, c) + d;

		if (Math::isZero(calcDot))
			return Plane::Position::COPLANAR;
		if (calcDot > 0.0f)
			return Plane::Position::FRONT;
		return Plane::Position::BEHIND;
	}

	Plane::Position Plane::ClassifyTri(const Vector &p1, const Vector &p2, const Vector &p3) const
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