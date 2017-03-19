#include "bvolumes.hpp"

#include <cmath>
#include <cassert>

namespace hr
{
	BBox::BBox(const Vector3f * const points, size_t numVec)
	{
		mMinPt.set(std::numeric_limits<float>::infinity());
		mMaxPt.set(-std::numeric_limits<float>::infinity());

		merge(points, numVec);
	}

	BBox::BBox(const BBox * const bboxes, size_t numBBox)
	{
		mMinPt.set(std::numeric_limits<float>::infinity());
		mMaxPt.set(-std::numeric_limits<float>::infinity());

		for (size_t i = 0; i < numBBox; i++)
		{
			merge(bboxes[i].mMaxPt);
			merge(bboxes[i].mMinPt);
		}
	}

	float BBox::radius(void) const
	{
		auto center = this->center();
		return center.getDistance(mMinPt);
	}

	float BBox::radiusMinimum(void) const
	{
		auto center = this->center();

		float minDist = std::abs(mMinPt[0] - center[0]);
		minDist = std::fmin(minDist, std::abs(mMinPt[1] - center[1]));
		return std::fmin(minDist, std::abs(mMinPt[2] - center[2]));
	}
	
	/*

	2---------------6
	|\				|
	| \				|\
	|  \  back		| \
	|	\			|  \
	|	 \			|   \
	0-----\---------4	 \
	 \	   \		 \	  \
	  \     \		  \	   \
	   \    3---------------7
	 	\   |			\   |
	 	 \  |			 \  |
	 	  \ |	  front	  \ |
	       \|			   \|
	        1---------------5
	*/
	void BBox::corners(Vector3f points[8]) const
	{
		points[0].set(mMinPt[0], mMinPt[1], mMinPt[2]);
		points[1].set(mMinPt[0], mMinPt[1], mMaxPt[2]);
		points[2].set(mMinPt[0], mMaxPt[1], mMinPt[2]);
		points[3].set(mMinPt[0], mMaxPt[1], mMaxPt[2]);
		points[4].set(mMaxPt[0], mMinPt[1], mMinPt[2]);
		points[5].set(mMaxPt[0], mMinPt[1], mMaxPt[2]);
		points[6].set(mMaxPt[0], mMaxPt[1], mMinPt[2]);
		points[7].set(mMaxPt[0], mMaxPt[1], mMaxPt[2]);
	}

	void BBox::boundingSphere(Vector3f &center, float &radius) const
	{
		this->center(center);
		radius = center.getDistance(mMinPt);
	}

	BSphere BBox::boundingSphere() const
	{
		auto center = this->center();
		return BSphere(center, center.getDistance(mMinPt));
	}

	float BBox::planeDistance(const Plane &plane) const
	{
		auto center = this->center();

		float d1 = plane.getDotCoord(center);
		Vector3f planeNormal = plane.normal();

		float d2 = std::abs((mMaxPt[0] - center[0]) * planeNormal[0]);
		d2 += std::abs((mMaxPt[1] - center[1]) * planeNormal[1]);
		d2 += std::abs((mMaxPt[2] - center[2]) * planeNormal[2]);

		if ((d1 - d2) > 0.0f)
			return (d1 - d2);
		if ((d1 + d2) < 0.0f)
			return (d1 + d2);
		return 0.0f;
	}

	Vector3f BBox::mainAxis() const
	{
		float tamX = mMaxPt[0] - mMinPt[0];
		float tamY = mMaxPt[1] - mMinPt[1];
		float tamZ = mMaxPt[2] - mMinPt[2];

		if (tamX > tamY && tamX > tamZ)
			return Vector3f(1.0f, 0.0f, 0.0f);
		if (tamY > tamX && tamY > tamZ)
			return Vector3f(0.0f, 1.0f, 0.0f);
		if (tamZ > tamX && tamZ > tamY)
			return Vector3f(0.0f, 0.0f, 1.0f);

		if (Math::isZero(tamX - tamY) && Math::isZero(tamY - tamZ))
			return Vector3f(1.0f, 1.0f, 1.0);
		if (Math::isZero(tamX - tamY))
			return Vector3f(1.0f, 1.0f, 0.0);
		if (Math::isZero(tamX - tamZ))
			return Vector3f(1.0f, 0.0f, 1.0);

		return Vector3f(0.0f, 1.0f, 1.0);
	}

	void BBox::merge(const Vector3f &pt)
	{
		__m128 vecData;

		vecData = _mm_loadu_ps(pt.data());
		_mm_storeu_ps(mMinPt.data(), _mm_min_ps(_mm_loadu_ps(mMinPt.data()), vecData));
		_mm_storeu_ps(mMaxPt.data(), _mm_max_ps(_mm_loadu_ps(mMaxPt.data()), vecData));
	}

	void BBox::merge(const float * const pt)
	{
		mMinPt[0] = std::fmin(mMinPt[0], pt[0]);
		mMinPt[1] = std::fmin(mMinPt[1], pt[1]);
		mMinPt[2] = std::fmin(mMinPt[2], pt[2]);
		mMaxPt[0] = std::fmax(mMaxPt[0], pt[0]);
		mMaxPt[1] = std::fmax(mMaxPt[1], pt[1]);
		mMaxPt[2] = std::fmax(mMaxPt[2], pt[2]);
	}

	void BBox::merge(const Vector3f * const pts, size_t numPts)
	{
		__m128 vec1, vec2, finalMin, finalMax;
		size_t i, j;

		finalMin = _mm_loadu_ps(mMinPt.data());
		finalMax = _mm_loadu_ps(mMaxPt.data());

		for (i = numPts, j = 0; i >= 2; i -= 2, j += 2)
		{
			vec1 = _mm_loadu_ps(pts[j].data());
			vec2 = _mm_loadu_ps(pts[j + 1].data());
			finalMin = _mm_min_ps(finalMin, vec1);
			finalMax = _mm_max_ps(finalMax, vec1);
			finalMin = _mm_min_ps(finalMin, vec2);
			finalMax = _mm_max_ps(finalMax, vec2);
		}

		if (i > 0)
		{
			vec1 = _mm_loadu_ps(pts[j].data());
			finalMin = _mm_min_ps(finalMin, vec1);
			finalMax = _mm_max_ps(finalMax, vec1);
		}

		_mm_storeu_ps(mMinPt.data(), finalMin);
		_mm_storeu_ps(mMaxPt.data(), finalMax);
	}

	void BBox::merge(const float &x, const float &y, const float &z)
	{
		__m128 vecData;

		vecData = _mm_set_ps(0.0f, z, y, x);
		_mm_storeu_ps(mMinPt.data(), _mm_min_ps(_mm_loadu_ps(mMinPt.data()), vecData));
		_mm_storeu_ps(mMaxPt.data(), _mm_max_ps(_mm_loadu_ps(mMaxPt.data()), vecData));
	}

	void BBox::mergeSphere(const Vector3f &sphereCenter, const float sphereRadius)
	{
		__m128 smin, smax, aux, finalMin, finalMax;

		smin = smax = _mm_loadu_ps(sphereCenter.data());
		aux = _mm_set_ps1(sphereRadius);
		smin = _mm_sub_ps(smin, aux);
		smax = _mm_add_ps(smax, aux);

		finalMin = _mm_loadu_ps(mMinPt.data());
		finalMax = _mm_loadu_ps(mMaxPt.data());
		finalMin = _mm_min_ps(finalMin, smin);
		finalMax = _mm_max_ps(finalMax, smin);
		_mm_storeu_ps(mMinPt.data(), _mm_min_ps(finalMin, smax));
		_mm_storeu_ps(mMaxPt.data(), _mm_max_ps(finalMax, smax));
	}

	void BBox::mergeSphere(const BSphere &bsphere)
	{
		Vector3f pts[8];

		auto scenter = bsphere.center();
		float sradius = bsphere.radius();

		pts[0].set(scenter[0] - sradius, scenter[1] + sradius, scenter[2] + sradius);
		pts[1].set(scenter[0] - sradius, scenter[1] - sradius, scenter[2] + sradius);
		pts[2].set(scenter[0] + sradius, scenter[1] + sradius, scenter[2] + sradius);
		pts[3].set(scenter[0] + sradius, scenter[1] - sradius, scenter[2] + sradius);

		pts[4].set(scenter[0] - sradius, scenter[1] + sradius, scenter[2] - sradius);
		pts[5].set(scenter[0] - sradius, scenter[1] - sradius, scenter[2] - sradius);
		pts[6].set(scenter[0] + sradius, scenter[1] + sradius, scenter[2] - sradius);
		pts[7].set(scenter[0] + sradius, scenter[1] - sradius, scenter[2] - sradius);

		merge(pts, 8);
	}

	void BBox::mergeBox(const Vector3f &boxCenter, const float boxWidth, const float boxHeight, const float boxDepth)
	{
		Vector3f pts[8];

		pts[0].set(boxCenter[0] - boxWidth, boxCenter[1] + boxHeight, boxCenter[2] + boxDepth);
		pts[1].set(boxCenter[0] - boxWidth, boxCenter[1] - boxHeight, boxCenter[2] + boxDepth);
		pts[2].set(boxCenter[0] + boxWidth, boxCenter[1] + boxHeight, boxCenter[2] + boxDepth);
		pts[3].set(boxCenter[0] + boxWidth, boxCenter[1] - boxHeight, boxCenter[2] + boxDepth);

		pts[4].set(boxCenter[0] - boxWidth, boxCenter[1] + boxHeight, boxCenter[2] - boxDepth);
		pts[5].set(boxCenter[0] - boxWidth, boxCenter[1] - boxHeight, boxCenter[2] - boxDepth);
		pts[6].set(boxCenter[0] + boxWidth, boxCenter[1] + boxHeight, boxCenter[2] - boxDepth);
		pts[7].set(boxCenter[0] + boxWidth, boxCenter[1] - boxHeight, boxCenter[2] - boxDepth);

		merge(pts, 8);
	}

	bool BBox::intersects(const Vector3f &lineStart, const Vector3f &lineEnd) const
	{
		float ld[3];

		auto center = (mMinPt + mMaxPt) * 0.5f;
		auto extents = mMaxPt - center;
		auto lineDir = (lineEnd - lineStart) * 0.5f;
		auto lineCenter = lineStart + lineDir;
		auto dir = lineCenter - center;

		ld[0] = std::abs(lineDir[0]);
		if (std::abs(dir[0]) > (extents[0] + ld[0]))
			return false;

		ld[1] = std::abs(lineDir[1]);
		if (std::abs(dir[1]) > (extents[1] + ld[1]))
			return false;

		ld[2] = std::abs(lineDir[2]);
		if (std::abs(dir[2]) > (extents[2] + ld[2]))
			return false;

		auto cross = lineDir.crossProduct(dir);

		if (std::abs(cross[0]) > (extents[1] * ld[2] + extents[2] * ld[1]))
			return false;
		if (std::abs(cross[1]) > (extents[0] * ld[2] + extents[2] * ld[0]))
			return false;
		if (std::abs(cross[2]) > (extents[0] * ld[1] + extents[1] * ld[0]))
			return false;
		return true;
	}

	bool BBox::intersects(const Ray &ray, float * const rayHitDistance) const
	{
		int side;
		Vector3f hit;

		//helper
		auto rayOrigin = ray.origin();
		auto rayDir = ray.direction();

		//by omission
		if (rayHitDistance)
			*rayHitDistance = 0.0f;

		int ax0 = -1;
		int inside = 0;
		float scale = 0.0f;
		for (size_t i = 0; i < 3; i++)
		{
			if (rayOrigin[i] < mMinPt[i])
				side = 0;
			else if (rayOrigin[i] > mMaxPt[i])
				side = 1;
			else
			{
				inside++;
				continue;
			}

			if (rayDir[i] == 0.0f)
				continue;

			assert((side == 0) || (side == 1));

			float f = (rayOrigin[i] - ((side == 0) ? mMinPt : mMaxPt)[i]);
			if ((ax0 < 0) || (std::abs(f) > std::abs(scale * rayDir[i])))
			{
				scale = -(f / rayDir[i]);
				ax0 = i;
			}
		}

		if (ax0 < 0)
			return (inside == 3);

		if (rayHitDistance)
			*rayHitDistance = scale;

		int ax1 = (ax0 + 1) % 3;
		int ax2 = (ax0 + 2) % 3;
		hit[ax1] = rayOrigin[ax1] + scale * rayDir[ax1];
		hit[ax2] = rayOrigin[ax2] + scale * rayDir[ax2];

		return ((hit[ax1] >= mMinPt[ax1]) && (hit[ax1] <= mMaxPt[ax1]) && (hit[ax2] >= mMinPt[ax2]) && (hit[ax2] <= mMaxPt[ax2]));
	}

	BBox::Position BBox::classify(const BBox &bbox)
	{
		if ((bbox.mMinPt[0] > mMaxPt[0]) || (bbox.mMaxPt[0] < mMinPt[0]))
			return Position::Outside;
		if ((bbox.mMinPt[1] > mMaxPt[1]) || (bbox.mMaxPt[1] < mMinPt[1]))
			return Position::Outside;
		if ((bbox.mMinPt[2] > mMaxPt[2]) || (bbox.mMaxPt[2] < mMinPt[2]))
			return Position::Outside;

		if ((bbox.mMinPt[0] > mMinPt[0]) && (bbox.mMaxPt[0] < mMaxPt[0]))
		{
			if ((bbox.mMinPt[1] > mMinPt[1]) && (bbox.mMaxPt[1] < mMaxPt[1]))
			{
				if ((bbox.mMinPt[2] > mMinPt[2]) && (bbox.mMaxPt[2] < mMaxPt[2]))
					return Position::Inside;
			}
		}

		return Position::Intersect;
	}

	float BSphere::calcDist(const float &px, const float &py, const float &pz) const
	{
		float d1 = mCenter[0] - px;
		float d2 = mCenter[1] - py;
		float d3 = mCenter[2] - pz;
		return sqrtf(d1*d1 + d2*d2 + d3*d3);
	}

	float BSphere::calcDist(const float * const vec) const
	{
		float d1 = mCenter[0] - vec[0];
		float d2 = mCenter[1] - vec[1];
		float d3 = mCenter[2] - vec[2];
		return sqrtf(d1*d1 + d2*d2 + d3*d3);
	}

	float BSphere::calcDist(const BSphere &bsphere) const
	{
		float d1 = mCenter[0] - bsphere.mCenter[0];
		float d2 = mCenter[1] - bsphere.mCenter[1];
		float d3 = mCenter[2] - bsphere.mCenter[2];
		return sqrtf(d1*d1 + d2*d2 + d3*d3);
	}

	float BSphere::calcDist(const Vector3f &vec) const
	{
		float d1 = mCenter[0] - vec[0];
		float d2 = mCenter[1] - vec[1];
		float d3 = mCenter[2] - vec[2];
		return sqrtf(d1*d1 + d2*d2 + d3*d3);
	}

	bool BSphere::containsPoint(const Vector3f &point) const
	{
		return (calcDist(point) < mRadius);
	}

	bool BSphere::containsPoint(const float &px, const float &py, const float &pz) const
	{
		return (calcDist(px, py, pz) < mRadius);
	}

	bool BSphere::intersects(const BSphere &bsphere) const
	{
		auto radiusSum = bsphere.mRadius + mRadius;

		auto vecDiff = Vector3f(bsphere.mCenter[0], bsphere.mCenter[1], bsphere.mCenter[2]) - Vector3f(mCenter[0], mCenter[1], mCenter[2]);

		return (vecDiff.getDot() <= (radiusSum * radiusSum));
	}

	bool BSphere::intersects(const Vector3f &lineStart, const Vector3f &lineEnd) const
	{
		auto origin = Vector3f(mCenter[0], mCenter[1], mCenter[2]);

		auto s = lineStart - origin;
		auto e = lineEnd - origin;
		auto r = e - s;

		auto a = -s.getDot(r);
		auto radiusSquared = mRadius * mRadius;

		if (a <= 0.0f)
			return (s.getDot() < radiusSquared);

		if (a >= r.getDot())
			return (e.getDot() < radiusSquared);

		r = s + r * (a / r.getDot());
		return (r.getDot() < radiusSquared);
	}

	bool BSphere::intersects(const Ray &ray) const
	{
		auto rayDir = ray.direction();

		auto p = ray.origin() - Vector3f(mCenter[0], mCenter[1], mCenter[2]);
		auto a = rayDir.getDot();
		auto b = rayDir.getDot(p);
		auto c = p.getDot() - (mRadius * mRadius);
		auto d = (b * b) - (c * a);

		return !(d < 0.0f);
	}

	bool BSphere::intersects(const Ray &ray, float &rayHitDistance1, float &rayHitDistance2) const
	{
		//by omission
		rayHitDistance1 = rayHitDistance2 = 0.0f;

		//helper
		auto rayDir = ray.direction();

		auto p = ray.origin() - Vector3f(mCenter[0], mCenter[1], mCenter[2]);
		auto a = rayDir.getDot();
		auto b = rayDir.getDot(p);
		auto c = p.getDot() - (mRadius * mRadius);
		auto d = (b * b) - (c * a);

		if (d < 0.0f)
			return false;

		auto sqrtd = Math::sqrt(d);
		a = 1.0f / a;

		//if true: (ray.origin + rayDir * rayHitDistance1), (ray.origin + rayDir * rayHitDistance2)
		//if true: ray::GetPointOn(rayHitDistance1), ray::GetPointOn(rayHitDistance2)
		rayHitDistance1 = (-b + sqrtd) * a;
		rayHitDistance2 = (-b - sqrtd) * a;
		return true;
	}

	BSphere::Position BSphere::classifyBSphere(const BSphere &bsphere)
	{
		float centerDist = calcDist(bsphere);

		if ((centerDist + bsphere.mRadius) < mRadius)
			return BSphere::Position::Inside;
		if (centerDist - bsphere.mRadius > mRadius)
			return BSphere::Position::Outside;

		return BSphere::Position::Intersect;
	}

	BSphere::Position BSphere::classifyBBox(const BBox &bbox)
	{
		Vector3f pontos[8];
		unsigned char numIn, numOut;

		bbox.corners(pontos);
		numIn = numOut = 0;

		for (size_t i = 0; i < 4; i++)
			(calcDist(pontos[i]) < mRadius) ? numIn++ : numOut++;

		if (numIn != 0 && numOut != 0)
			return BSphere::Position::Intersect;

		for (size_t i = 4; i < 8; i++)
			(calcDist(pontos[i]) < mRadius) ? numIn++ : numOut++;

		if (numIn != 0 && numOut != 0)
			return BSphere::Position::Intersect;
		if (numIn == 0)
			return BSphere::Position::Outside;

		return BSphere::Position::Inside;
	}
}