#pragma once

#include "vector.hpp"

namespace HorseRadish
{
	HALIGN_16BYTES
	class Plane
	{
		float a, b, c, d;

	public:
		enum class Position{
			COPLANAR,
			FRONT,
			BEHIND,
			INTERSECT
		};

		Plane()
			: a(0.0f), b(0.0f), c(0.0f), d(0.0f)
		{
		}

		~Plane()
		{
		}

		explicit Plane(const Plane &plane);
		explicit Plane(const float nx, const float ny, const float nz, const float nd);
		explicit Plane(const Vector3f &nN, const float nd);

		Plane& operator=(const Plane& plane)
		{
			a = plane.a; b = plane.b; c = plane.c; d = plane.d; return *this;
		}

		void Set(const float nx, const float ny, const float nz, const float nd)
		{
			a = nx; b = ny; c = nz; d = nd;
		}

		void SetNormal(const Vector3f &newNormal)
		{
			a = newNormal[0];
			b = newNormal[1];
			c = newNormal[2];
		}

		void SetNormal(const float nx, const float ny, const float nz)
		{
			a = nx; b = ny; c = nz;
		}

		void SetD(const float nd)
		{
			d = nd;
		}

		void SetFromPoints(const Vector3f &p0, const Vector3f &p1, const Vector3f &p2);
		void SetFromPoints(const float *p0, const float *p1, const float *p2);

		void Lerp(const Plane &p2, const float factor, Plane &result);

		void Normalize(void);
		void NormalizeNormal(void);

		void NegateNormal()
		{
			a *= -1.0f; b *= -1.0f; c *= -1.0f;
		}

		void NegateD()
		{
			d *= -1.0f;
		}

		void CalcD(const Vector3f &pointOnPlane)
		{
			d = -pointOnPlane.getDot(a, b, c);
		}

		float GetDistance(const Vector3f &point) const
		{
			return (point.getDot(a, b, c) + d);
		}

		void GetNormal(Vector3f &normal) const
		{
			normal.set(a, b, c);
		}

		float GetD() const
		{
			return d;
		}

		void GetComponents(Vector4f &coords) const
		{
			coords.set(a, b, c, d);
		}

		int GetVertLocation() const
		{
			return (((a < 0.f) ? 1 : 0) | ((b < 0.f) ? 2 : 0) | ((c < 0.f) ? 4 : 0));
		}

		float GetDotCoord(const Vector3f &point) const
		{
			return (point.getDot(a, b, c) + d);
		}

		float GetDotNormal(const Vector3f &point) const
		{
			return (point.getDot(a, b, c));
		}

		bool TestIntersectRay(const Vector3f &origin, const Vector3f &dir) const;
		bool TestIntersectRay(const Vector3f &origin, const Vector3f &dir, Vector3f& result) const;
		bool TestIntersectLine(const Vector3f &p1, const Vector3f &p2) const;
		bool TestIntersectLine(const Vector3f &p1, const Vector3f &p2, Vector3f& result) const;
		bool TestIntersectLineSegment(const Vector3f &p1, const Vector3f &p2) const;
		bool TestIntersectLineSegment(const Vector3f &p1, const Vector3f &p2, Vector3f& result) const;
		bool TestIntersectPlanes(const Plane &p2, const Plane &p3) const;
		bool TestIntersectPlanes(const Plane &p2, const Plane &p3, Vector3f& result) const;
		bool TestIntersectSweptSphere(const float &sphereRadius, const Vector3f &spherePrevPos, const Vector3f &sphereCurPos, Vector3f * const hitPoint, float * const hitTime) const;

		Position ClassifyPoint(const Vector3f &point) const;
		Position ClassifyTri(const Vector3f &p1, const Vector3f &p2, const Vector3f &p3) const;
	};

} //HorseRadish
