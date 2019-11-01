#pragma once

#include "common/vector.hpp"
#include "common/matrix.hpp"
#include "common/ray.hpp"

namespace hr::gl::tools
{
	class Frustum
	{
	public:
		enum class IntersectionType { FullInside, FullOutside, FrustumIntersect };
		enum PlaneIndex : size_t { PlaneLeft = 0, PlaneRight = 1, PlaneTop = 2, PlaneBottom = 3, PlaneNear = 4, PlaneFar = 5 };

	private:
		hr::Plane mPlanes[6];
		hr::Vector3f mPosition;
		float mZNear{ 0.0f }, mZFar{ 0.0f };

		void extractPlanes(const hr::Vector4f& col1, const hr::Vector4f& col2, const hr::Vector4f& col3, const hr::Vector4f& col4);
		bool sweptSpherePlaneIntersect(float& t0, float& t1, const hr::Plane& plane, const hr::Vector3f& sphereCenter, const float& sphereRadius, const hr::Vector3f& sweepDir) const;

	public:
		Frustum() = default;

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

		bool testCube(const hr::Vector3f& point, const float& size) const;
		bool testBox(const hr::Vector3f& min, const hr::Vector3f& max) const;
		bool testBox(const hr::BBox& bbox) const;
		bool testSphere(const hr::Vector3f& center, const float& radius) const;
		bool testSphere(const hr::BSphere& bsphere) const;
		bool testSphereBox(const hr::BSphere& bsphere, const hr::BBox& bbox) const;
		bool testPoint(const hr::Vector3f& point) const;
		bool testPolygon(const hr::Vector3f* const points, size_t numPoints) const;
		bool testSquare(const hr::Vector3f points[4]) const;
		bool testTri(const hr::Vector3f points[3]) const;
		bool testSweptSphere(const hr::Vector3f& sphereCenter, const float& sphereRadius, const hr::Vector3f& sweepDir) const;

		float getZNear(void) const
		{
			return mZNear;
		}

		float getZFar(void) const
		{
			return mZFar;
		}

		const hr::Vector3f& getCamPosition(void) const
		{
			return mPosition;
		}

		const hr::Plane& getPlane(const PlaneIndex planeIndex) const
		{
			return mPlanes[planeIndex];
		}

		void getCorners(hr::Vector3f points[8]) const;

		IntersectionType classifyFrustum(const Frustum& frustum) const;

		float dotNormals(const PlaneIndex planeA, const PlaneIndex planeB) const;

		void calculateFrustum(const hr::Matrix& transformation);
		void calculateFrustum(const hr::Matrix& projection, const hr::Matrix& modelview);
		void calculateFrustum(const hr::Matrix& modelView, const hr::Matrix& projection, const hr::Vector3f& pos, float zNear, float zFar);

		void setIndividualPlane(const PlaneIndex planeIndex, const hr::Plane& plane);
		void setFrustum(const hr::Vector3f& bboxMin, const hr::Vector3f& bboxMax);
		void setFrustum(const hr::Vector3f& center, const float radius);
		void setFrustum(const hr::BBox& bbox);

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

		void setCamPosition(const hr::Vector3f& pos)
		{
			mPosition.set(pos);
		}
	};
}
