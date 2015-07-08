#pragma once

#include "Vector.hpp"
#include "Ray.hpp"
#include "Plane.hpp"
#include "Math.hpp"

namespace HorseRadish
{
	class BBox;
	class BSphere;

	class BBox
	{
		Vector3f minPt, maxPt;

	public:
		enum class Position{
			Inside, Outside, Intersect
		};

		BBox() : minPt(Math::INFINITY), maxPt(-Math::INFINITY) { }
		BBox(const BBox& bbox) : minPt(bbox.minPt), maxPt(bbox.maxPt) { }
		explicit BBox(const Vector3f * const points, const unsigned int numVec);
		explicit BBox(const BBox * const bboxes, const unsigned int numBBox);
		~BBox(){ return; }

		BBox& operator=(const BBox& bbox);
		void operator+=(const BBox& bbox);
		void operator+=(const Vector3f& pt);

		void GetMin(Vector3f &point) const { point.set(minPt); }
		void GetMax(Vector3f &point) const { point.set(maxPt); }
		void GetMin(float * const point) const { minPt.write(point); }
		void GetMax(float * const point) const { maxPt.write(point); }
		void GetMinMax(Vector3f &min, Vector3f &max) const { min.set(minPt); max.set(maxPt); }
		void GetMinMax(float * const min, float * const max) const { minPt.write(min); maxPt.write(max); }
		void GetCenter(Vector3f &point) const { point.set((minPt[0] + maxPt[0])*0.5f, (minPt[1] + maxPt[1])*0.5f, (minPt[2] + maxPt[2])*0.5f); }
		void GetCenter(float * const point) const { point[0] = (minPt[0] + maxPt[0])*0.5f; point[1] = (minPt[1] + maxPt[1])*0.5f; point[2] = (minPt[2] + maxPt[2])*0.5f; }
		void GetDims(Vector3f &point) const { point.set(maxPt[0] - minPt[0], maxPt[1] - minPt[1], maxPt[2] - minPt[2]); }
		void GetDims(float * const point) const { point[0] = maxPt[0] - minPt[0]; point[1] = maxPt[1] - minPt[1]; point[2] = maxPt[2] - minPt[2]; }
		float GetRadius(void) const;
		float GetRadiusMinimum(void) const;
		float GetVolume(void) const;
		void GetCorners(Vector3f points[8]) const;
		void GetGeom(Vector3f points[36]) const;
		void GetBoundingSphere(Vector3f &center, float &radius) const;
		void GetBoundingSphere(BSphere &bsphere) const;
		float GetPlaneDistance(const Plane &plane) const;
		const float* GetMin() const { return minPt.data(); }
		const float* GetMax() const { return maxPt.data(); }
		Vector3f GetMainAxis() const;

		void Merge(const Vector3f &pt);
		void Merge(const float * const pt);
		void Merge(const Vector3f * const pts, const int numPts);
		void Merge(const float &x, const float &y, const float &z);
		void MergeSphere(const Vector3f &sphereCenter, const float sphereRadius);
		void MergeSphere(const BSphere &bsphere);
		void MergeBox(const Vector3f &boxCenter, const float boxWidth, const float boxHeight, const float boxDepth);

		void Set(const BBox * const bbox) { minPt.set(bbox->minPt); maxPt.set(bbox->maxPt); }
		void Set(const float &value) { minPt.set(value); maxPt.set(value); }
		void Set(const float &minValue, const float &maxValue) { minPt.set(minValue); maxPt.set(maxValue); }
		void SetMin(const Vector3f &min) { minPt.set(min); }
		void SetMax(const Vector3f &max) { maxPt.set(max); }
		void SetMinMax(const float * const min, const float * const max) { minPt.set(min); maxPt.set(max); }
		void SetMinMax(const Vector3f &min, const Vector3f &max) { minPt.set(min); maxPt.set(max); }

		void Reset(){ minPt[0] = minPt[1] = minPt[2] = Math::INFINITY; maxPt[0] = maxPt[1] = maxPt[2] = -Math::INFINITY; }

		void Translate(const Vector3f &translation);
		void Translate(BBox& bbox, const Vector3f &translation) const;
		void Expand(const float amount);

		bool ContainsPoint(const Vector3f &point) const;

		bool Intersects(const BBox &bbox) const;
		bool Intersects(const Vector3f &lineStart, const Vector3f &lineEnd) const;
		bool Intersects(const Ray &ray, float * const rayHitDistance = nullptr) const;

		Position Classify(const BBox &bbox);
	};

	class BSphere
	{
		float x, y, z, radius;

		float calcDist(const float &px, const float &py, const float &pz) const;
		float calcDist(const float * const vec) const;
		float calcDist(const BSphere &bsphere) const;
		float calcDist(const Vector3f &vec) const;

	public:
		enum class Position{
			Inside, Outside, Intersect
		};

		BSphere() : x(0), y(0), z(0), radius(Math::INFINITY) { }
		BSphere(const BSphere& bsphere) : x(bsphere.x), y(bsphere.y), z(bsphere.z), radius(bsphere.radius) { }
		~BSphere() {}

		BSphere& operator=(const BSphere& bsphere);
		void operator+=(const BSphere& bsphere);

		float GetRadius() const { return radius; }
		void GetCenter(Vector3f &center) const { center.set(x, y, z); }
		void GetCenter(float * const center) const { center[0] = x; center[1] = y; center[2] = z; }

		void Merge(const Vector3f &pt);
		void Merge(const float * const pt);
		void Merge(const Vector3f * const pts, const int numPts);
		void Merge(const float &x, const float &y, const float &z);
		void Merge(const BSphere &sphere);
		void Merge(const BBox &bbox);

		void Expand(const float &amount);
		void Contract(const float &amount);

		void Set(const float &newx, const float &newy, const float &newz, const float &newradius) { x = newx; y = newy; z = newz; radius = newradius; }
		void Set(const Vector3f &center, const float &newradius) { x = center[0]; y = center[1]; z = center[2]; radius = newradius; }
		void Set(const BBox &bbox) { bbox.GetBoundingSphere(*this); }
		void SetCenter(const float &newx, const float &newy, const float &newz) { x = newx; y = newy; z = newz; }
		void SetCenter(const Vector3f &center) { x = center[0]; y = center[1]; z = center[2]; }
		void SetRadius(const float &newradius) { radius = newradius; }
		void Reset(){ x = y = z = 0.0f; radius = Math::INFINITY; }
		void Translate(const Vector3f &translation) { x += translation[0]; y += translation[1]; z += translation[2]; }
		void Translate(BSphere& bsphereDest, const Vector3f &translation) const { bsphereDest.x = x + translation[0]; bsphereDest.y = y + translation[1]; bsphereDest.z = z + translation[2]; bsphereDest.radius = radius; }

		bool ContainsPoint(const Vector3f &point) const;
		bool ContainsPoint(const float &x, const float &y, const float &z) const;

		bool Intersects(const BSphere &bsphere) const;
		bool Intersects(const Vector3f &lineStart, const Vector3f &lineEnd) const;
		bool Intersects(const Ray &ray) const;
		bool Intersects(const Ray &ray, float &rayHitDistance1, float &rayHitDistance2) const;

		Position ClassifyBSphere(const BSphere &bsphere);
		Position ClassifyBBox(const BBox &bbox);
	};

} //HorseRadish
