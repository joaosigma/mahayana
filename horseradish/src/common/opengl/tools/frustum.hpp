#pragma once

#include "common/vector.hpp"
#include "common/matrix.hpp"
#include "common/ray.hpp"

namespace HorseRadish { namespace OpenGL { namespace Tools
{
	class Frustum
	{
	public:
		enum class IntersectionType { FullInside, FullOutside, FrustumIntersect };
		enum PlaneIndex : size_t { PlaneLeft = 0, PlaneRight = 1, PlaneTop = 2, PlaneBottom = 3, PlaneNear = 4, PlaneFar = 5 };

	private:
		HorseRadish::Plane mPlanes[6];
		HorseRadish::Vector3f mPosition;
		float mZNear = 0.0f, mZFar = 0.0f;

		void extractPlanes(const HorseRadish::Vector4f &col1, const HorseRadish::Vector4f &col2, const HorseRadish::Vector4f &col3, const HorseRadish::Vector4f &col4);
		bool sweptSpherePlaneIntersect(float &t0, float &t1, const HorseRadish::Plane &plane, const HorseRadish::Vector3f &sphereCenter, const float &sphereRadius, const HorseRadish::Vector3f &sweepDir) const;

	public:
		Frustum()
		{ }

		Frustum& operator=(const Frustum& frustum)
		{
			mPlanes[0] = frustum.mPlanes[0];
			mPlanes[1] = frustum.mPlanes[1];
			mPlanes[2] = frustum.mPlanes[2];
			mPlanes[3] = frustum.mPlanes[3];
			mPlanes[4] = frustum.mPlanes[4];
			mPlanes[5] = frustum.mPlanes[5];
			return *this;
		}

		bool testCube(const HorseRadish::Vector3f &point, const float &size) const;
		bool testBox(const HorseRadish::Vector3f &min, const HorseRadish::Vector3f &max) const;
		bool testBox(const HorseRadish::BBox &bbox) const;
		bool testSphere(const HorseRadish::Vector3f &center, const float &radius) const;
		bool testSphere(const HorseRadish::BSphere &bsphere) const;
		bool testSphereBox(const HorseRadish::BSphere &bsphere, const HorseRadish::BBox &bbox) const;
		bool testPoint(const HorseRadish::Vector3f &point) const;
		bool testPolygon(const HorseRadish::Vector3f * const points, size_t numPoints) const;
		bool testSquare(const HorseRadish::Vector3f points[4]) const;
		bool testTri(const HorseRadish::Vector3f points[3]) const;
		bool testSweptSphere(const HorseRadish::Vector3f &sphereCenter, const float &sphereRadius, const HorseRadish::Vector3f &sweepDir) const;

		float getZNear(void) const
		{
			return mZNear;
		}

		float getZFar(void) const
		{
			return mZFar;
		}

		const HorseRadish::Vector3f& getCamPosition(void) const
		{
			return mPosition;
		}

		const HorseRadish::Plane& getPlane(const PlaneIndex planeIndex) const
		{
			return mPlanes[planeIndex];
		}

		void getCorners(HorseRadish::Vector3f points[8]) const;

		IntersectionType classifyFrustum(const Frustum &frustum) const;

		float dotNormals(const PlaneIndex planeA, const PlaneIndex planeB) const;

		void calculateFrustum(const HorseRadish::Matrix& transformation);
		void calculateFrustum(const HorseRadish::Matrix& projection, const HorseRadish::Matrix& modelview);
		void calculateFrustum(const HorseRadish::Matrix& modelView, const HorseRadish::Matrix& projection, const HorseRadish::Vector3f &pos, float zNear, float zFar);

		void setIndividualPlane(const PlaneIndex planeIndex, const HorseRadish::Plane &plane);
		void setFrustum(const HorseRadish::Vector3f &bboxMin, const HorseRadish::Vector3f &bboxMax);
		void setFrustum(const HorseRadish::Vector3f &center, const float radius);
		void setFrustum(const HorseRadish::BBox &bbox);
		
		void setZNear(const float ZNear)
		{
			mZNear = ZNear;
		}

		void setZFar(const float ZFar)
		{
			mZFar = ZFar;
		}

		void setCamPosition(const float x, const float y, const float z)
		{
			mPosition.set(x, y, z);
		}

		void setCamPosition(const HorseRadish::Vector3f &pos)
		{
			mPosition.set(pos);
		}
	};
} } }
