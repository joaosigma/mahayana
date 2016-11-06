#pragma once

#include "vector.hpp"

namespace HorseRadish
{
	HALIGN_16BYTES
	class Plane
	{
		float mA, mB, mC, mD; //scalar equation of plane: ax + by + cz = d

	public:
		enum class Position { COPLANAR, FRONT, BEHIND, INTERSECT };

		Plane()
			: mA(0.0f), mB(0.0f), mC(0.0f), mD(0.0f)
		{ }

		Plane(const Plane &plane)
			: mA(plane.mA), mB(plane.mB), mC(plane.mC), mD(plane.mD)
		{ }

		explicit Plane(float a, float b, float c, float d)
			: mA(a), mB(b), mC(c), mD(d)
		{ }

		explicit Plane(const Vector3f &nN, float d)
			: mA(nN[0]), mB(nN[1]), mC(nN[2]), mD(d)
		{ }

		Plane& operator=(const Plane& plane)
		{
			mA = plane.mA;
			mB = plane.mB;
			mC = plane.mC;
			mD = plane.mD;
			return *this;
		}

		void set(float a, float b, float c, float d)
		{
			mA = a;
			mB = b;
			mC = c;
			mD = d;
		}

		void setNormal(const Vector3f &newNormal)
		{
			mA = newNormal[0];
			mB = newNormal[1];
			mC = newNormal[2];
		}

		void setNormal(float ax, float by, float cz)
		{
			mA = ax;
			mB = by;
			mC = cz;
		}

		void setD(float nd)
		{
			mD = nd;
		}

		void setFromPoints(const Vector3f &p0, const Vector3f &p1, const Vector3f &p2);
		void setFromPoints(const float *p0, const float *p1, const float *p2);

		Plane lerp(const Plane &p2, float factor) const;

		void normalize(void);
		void normalizeNormal(void);

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

		void calcD(const Vector3f &pointOnPlane)
		{
			mD = -pointOnPlane.getDot(mA, mB, mC);
		}

		float distance(const Vector3f &point) const
		{
			return (point.getDot(mA, mB, mC) + mD);
		}

		Vector3f normal() const
		{
			return Vector3f(mA, mB, mC);
		}

		float d() const
		{
			return mD;
		}

		void getComponents(Vector4f &coords) const
		{
			coords.set(mA, mB, mC, mD);
		}

		int getVertLocation() const
		{
			return (((mA < 0.f) ? 1 : 0) | ((mB < 0.f) ? 2 : 0) | ((mC < 0.f) ? 4 : 0));
		}

		float getDotCoord(const Vector3f &point) const
		{
			return (point.getDot(mA, mB, mC) + mD);
		}

		float getDotNormal(const Vector3f &point) const
		{
			return (point.getDot(mA, mB, mC));
		}

		bool testIntersectRay(const Vector3f &origin, const Vector3f &dir) const;
		bool testIntersectRay(const Vector3f &origin, const Vector3f &dir, Vector3f& result) const;
		bool testIntersectLine(const Vector3f &p1, const Vector3f &p2) const;
		bool testIntersectLine(const Vector3f &p1, const Vector3f &p2, Vector3f& result) const;
		bool testIntersectLineSegment(const Vector3f &p1, const Vector3f &p2) const;
		bool testIntersectLineSegment(const Vector3f &p1, const Vector3f &p2, Vector3f& result) const;
		bool testIntersectPlanes(const Plane &p2, const Plane &p3) const;
		bool testIntersectPlanes(const Plane &p2, const Plane &p3, Vector3f& result) const;
		bool testIntersectSweptSphere(float sphereRadius, const Vector3f &spherePrevPos, const Vector3f &sphereCurPos, Vector3f * const hitPoint, float * const hitTime) const;

		Position classifyPoint(const Vector3f &point) const;
		Position classifyTri(const Vector3f &p1, const Vector3f &p2, const Vector3f &p3) const;
	};
}
