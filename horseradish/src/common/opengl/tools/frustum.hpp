#pragma once

#include "common\vector.hpp"
#include "common\Matrix.hpp"
#include "common\Ray.hpp"

namespace HorseRadish { namespace OpenGL { namespace Tools {
			
class Frustum
{
public:
	enum class IntersectionType { FullInside, FullOutside, FrustumIntersect };
	enum PlaneIndex : unsigned int { PlaneLeft = 0, PlaneRight = 1, PlaneTop = 2, PlaneBottom = 3, PlaneNear = 4, PlaneFar = 5 };

private:
	HorseRadish::Plane mPlanes[6];
	HorseRadish::Vector3f mPosition;
	float mZNear, mZFar;

	void extractPlanes(const HorseRadish::Vector4f &col1, const HorseRadish::Vector4f &col2, const HorseRadish::Vector4f &col3, const HorseRadish::Vector4f &col4);
	bool sweptSpherePlaneIntersect(float &t0, float &t1, const HorseRadish::Plane &plane, const HorseRadish::Vector3f &sphereCenter, const float &sphereRadius, const HorseRadish::Vector3f &sweepDir) const;

public:
	Frustum();

	bool testCube(const HorseRadish::Vector3f &point, const float &size) const;
	bool testBox(const HorseRadish::Vector3f &min, const HorseRadish::Vector3f &max) const;
	bool testBox(const HorseRadish::BBox &bbox) const;
	bool testSphere(const HorseRadish::Vector3f &center, const float &radius) const;
	bool testSphere(const HorseRadish::BSphere &bsphere) const;
	bool testSphereBox(const HorseRadish::BSphere &bsphere, const HorseRadish::BBox &bbox) const;
	bool testPoint(const HorseRadish::Vector3f &point) const;
	bool testPolygon(const HorseRadish::Vector3f * const points, const int numPoints) const;
	bool testSquare(const HorseRadish::Vector3f points[4]) const;
	bool testTri(const HorseRadish::Vector3f points[3]) const;
	bool testSweptSphere(const HorseRadish::Vector3f &sphereCenter, const float &sphereRadius, const HorseRadish::Vector3f &sweepDir) const;

	float getZNear(void) const { return mZNear; }
	float getZFar(void) const { return mZFar; }
	const HorseRadish::Vector3f& getCamPosition(void) const { return mPosition; }
	const HorseRadish::Plane& getPlane(const PlaneIndex planeIndex) const { return mPlanes[planeIndex]; }
	void getCorners(HorseRadish::Vector3f points[8]) const;

	IntersectionType classifyFrustum(const Frustum &frustum) const;

	float dotNormals(const PlaneIndex planeA, const PlaneIndex planeB) const;

	void calculateFrustum(const float * const transformation);
	void calculateFrustum(const float * const matProjection, const float * const matModelview);
	void calculateFrustum(const float * const modelView, const float * const projectionMatrix, const HorseRadish::Vector3f &pos, const float zNear, const float zFar);

	void setIndividualPlane(const PlaneIndex planeIndex, const HorseRadish::Plane &plane);
	void setFrustum(const Frustum *const frustum);
	void setFrustum(const HorseRadish::Vector3f &bboxMin, const HorseRadish::Vector3f &bboxMax);
	void setFrustum(const HorseRadish::Vector3f &center, const float radius);
	void setFrustum(const HorseRadish::BBox &bbox);
	void setZNear(const float ZNear) { mZNear = ZNear; }
	void setZFar(const float ZFar) { mZFar = ZFar; }
	void setCamPosition(const float x, const float y, const float z) { mPosition.set(x, y, z); }
	void setCamPosition(const HorseRadish::Vector3f &pos) { mPosition.set(pos); }
};

} } }

