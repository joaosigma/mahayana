#pragma once

#include "vector.hpp"
#include "ray.hpp"
#include "plane.hpp"
#include "math.hpp"

#include <limits>

namespace hr
{
	class BBox;
	class BSphere;

	class BBox
	{
		Vector3f mMinPt, mMaxPt;

	public:
		enum class Position { Inside, Outside, Intersect };

		BBox()
			: mMinPt(std::numeric_limits<float>::infinity())
			, mMaxPt(-std::numeric_limits<float>::infinity())
		{ }

		BBox(const BBox& bbox)
			: mMinPt(bbox.mMinPt)
			, mMaxPt(bbox.mMaxPt)
		{ }

		explicit BBox(const Vector3f * const points, size_t numVec);
		explicit BBox(const BBox * const bboxes, size_t numBBox);

		BBox& operator=(const BBox& bbox)
		{
			mMinPt.set(bbox.mMinPt);
			mMaxPt.set(bbox.mMaxPt);
			return *this;
		}

		void operator+=(const BBox& bbox)
		{
			merge(bbox.mMinPt);
			merge(bbox.mMaxPt);
		}

		void operator+=(const Vector3f& pt)
		{
			merge(pt);
		}

		Vector3f min() const
		{
			return mMinPt;
		}

		Vector3f max() const
		{
			return mMaxPt;
		}

		void min(Vector3f &point) const
		{
			point.set(mMinPt);
		}

		void max(Vector3f &point) const
		{
			point.set(mMaxPt);
		}

		void min(float * const point) const
		{
			mMinPt.write(point);
		}

		void max(float * const point) const
		{
			mMaxPt.write(point);
		}

		void minMax(Vector3f &min, Vector3f &max) const
		{
			min.set(mMinPt);
			max.set(mMaxPt);
		}

		void minMax(float * const min, float * const max) const
		{
			mMinPt.write(min);
			mMaxPt.write(max);
		}

		Vector3f center() const
		{
			return ((mMinPt + mMaxPt) * 0.5f);
		}

		void center(Vector3f &point) const
		{
			point = (mMinPt + mMaxPt) * 0.5f;
		}

		void dims(Vector3f &point) const
		{
			point = mMaxPt - mMinPt;
		}

		float radius(void) const;
		float radiusMinimum(void) const;
		
		float volume(void) const
		{
			return ((mMaxPt[0] - mMinPt[0]) * (mMaxPt[1] - mMinPt[1]) * (mMaxPt[2] - mMinPt[2]));
		}

		void corners(Vector3f points[8]) const;
		void geom(Vector3f points[36]) const;
		void boundingSphere(Vector3f &center, float &radius) const;
		BSphere boundingSphere() const;
		float planeDistance(const Plane &plane) const;
		Vector3f mainAxis() const;

		void merge(const Vector3f &pt);
		void merge(const float * const pt);
		void merge(const Vector3f * const pts, size_t numPts);
		void merge(const float &x, const float &y, const float &z);
		void mergeSphere(const Vector3f &sphereCenter, const float sphereRadius);
		void mergeSphere(const BSphere &bsphere);
		void mergeBox(const Vector3f &boxCenter, const float boxWidth, const float boxHeight, const float boxDepth);

		void setMin(const Vector3f &min)
		{
			mMinPt.set(min);
		}

		void setMax(const Vector3f &max)
		{
			mMaxPt.set(max);
		}

		void setMinMax(const float min[3], const float max[3])
		{
			mMinPt.set(min);
			mMaxPt.set(max);
		}

		void setMinMax(const Vector3f &min, const Vector3f &max)
		{
			mMinPt.set(min);
			mMaxPt.set(max);
		}

		void reset()
		{
			mMinPt[0] = mMinPt[1] = mMinPt[2] = std::numeric_limits<float>::infinity();
			mMaxPt[0] = mMaxPt[1] = mMaxPt[2] = -std::numeric_limits<float>::infinity();
		}

		void translate(const Vector3f &translation)
		{
			mMinPt += translation;
			mMaxPt += translation;
		}

		void expand(const float amount)
		{
			mMinPt -= amount;
			mMaxPt += amount;
		}

		bool containsPoint(const Vector3f &point) const
		{
			return (point[0] >= mMinPt[0]) && (point[1] >= mMinPt[1]) && (point[2] >= mMinPt[2]) && (point[0] <= mMaxPt[0]) && (point[1] <= mMaxPt[1]) && (point[2] <= mMaxPt[2]);
		}

		bool intersects(const BBox &bbox) const
		{
			return (bbox.mMaxPt[0] >= mMinPt[0]) && (bbox.mMaxPt[1] >= mMinPt[1]) && (bbox.mMaxPt[2] >= mMinPt[2]) && (bbox.mMinPt[0] <= mMaxPt[0]) && (bbox.mMinPt[1] <= mMaxPt[1]) && (bbox.mMinPt[2] <= mMaxPt[2]);
		}

		bool intersects(const Vector3f &lineStart, const Vector3f &lineEnd) const;
		bool intersects(const Ray &ray, float * const rayHitDistance = nullptr) const;

		Position classify(const BBox &bbox);
	};

	class BSphere
	{
		float mCenter[3], mRadius;

		float calcDist(const float &px, const float &py, const float &pz) const;
		float calcDist(const float * const vec) const;
		float calcDist(const BSphere &bsphere) const;
		float calcDist(const Vector3f &vec) const;

	public:
		enum class Position { Inside, Outside, Intersect };

		BSphere()
			: mCenter{ 0.0f, 0.0f, 0.0f }
			, mRadius(std::numeric_limits<float>::infinity())
		{ }

		BSphere(const BSphere& bsphere)
			: mCenter{ bsphere.mCenter[0], bsphere.mCenter[1], bsphere.mCenter[2] }
			, mRadius(bsphere.mRadius)
		{ }
		
		explicit BSphere(const Vector3f& center, float radius)
			: mCenter{ center[0], center[1], center[2] }
			, mRadius(radius)
		{ }

		BSphere& operator=(const BSphere& bsphere)
		{
			mCenter[0] = bsphere.mCenter[0];
			mCenter[1] = bsphere.mCenter[1];
			mCenter[2] = bsphere.mCenter[2];
			mRadius = bsphere.mRadius;
			return *this;
		}

		void operator+=(const BSphere& bsphere)
		{
			merge(bsphere);
		}

		float radius() const
		{
			return mRadius;
		}

		Vector3f center() const
		{
			return Vector3f(mCenter[0], mCenter[1], mCenter[2]);
		}

		void center(Vector3f &center) const
		{
			center.set(mCenter[0], mCenter[1], mCenter[2]);
		}

		void center(float * const center) const
		{
			center[0] = mCenter[0];
			center[1] = mCenter[1];
			center[2] = mCenter[2];
		}

		void merge(const Vector3f &pt)
		{
			mRadius = std::fmax(mRadius, calcDist(pt));
		}

		void merge(const float * const pt)
		{
			mRadius = std::fmax(mRadius, calcDist(pt));
		}

		void merge(const Vector3f * const pts, size_t numPts)
		{
			for (size_t i = 0; i < numPts; i++)
				mRadius = std::fmax(mRadius, calcDist(pts[i]));
		}

		void merge(const float &x, const float &y, const float &z)
		{
			mRadius = std::fmax(mRadius, calcDist(x, y, z));
		}

		void merge(const BSphere &sphere)
		{
			mRadius = std::fmax(mRadius, calcDist(sphere) + sphere.mRadius);
		}

		void merge(const BBox &bbox)
		{
			auto boxSphere = bbox.boundingSphere();
			mRadius = std::fmax(mRadius, calcDist(boxSphere) + boxSphere.mRadius);
		}
		
		void expand(const float &amount)
		{
			mRadius += amount;
		}

		void set(float x, float y, float z, const float &radius)
		{
			mCenter[0] = x;
			mCenter[1] = y;
			mCenter[2] = z;
			mRadius = radius;
		}

		void set(const Vector3f &center, float radius)
		{
			mCenter[0] = center[0];
			mCenter[1] = center[1];
			mCenter[2] = center[2];
			mRadius = radius;
		}

		void set(const BBox &bbox)
		{
			*this = bbox.boundingSphere();
		}

		void setCenter(float x, float y, float z)
		{
			mCenter[0] = x;
			mCenter[1] = y;
			mCenter[2] = z;
		}

		void setCenter(const Vector3f &center)
		{
			mCenter[0] = center[0];
			mCenter[1] = center[1];
			mCenter[2] = center[2];
		}

		void setRadius(float radius)
		{
			mRadius = radius;
		}

		void reset()
		{
			mCenter[0] = mCenter[1] = mCenter[2] = 0.0f;
			mRadius = std::numeric_limits<float>::infinity();
		}

		void translate(const Vector3f &translation)
		{
			mCenter[0] += translation[0];
			mCenter[1] += translation[1];
			mCenter[2] += translation[2];
		}

		bool containsPoint(const Vector3f &point) const;
		bool containsPoint(const float &x, const float &y, const float &z) const;

		bool intersects(const BSphere &bsphere) const;
		bool intersects(const Vector3f &lineStart, const Vector3f &lineEnd) const;
		bool intersects(const Ray &ray) const;
		bool intersects(const Ray &ray, float &rayHitDistance1, float &rayHitDistance2) const;

		Position classifyBSphere(const BSphere &bsphere);
		Position classifyBBox(const BBox &bbox);
	};
}
