#include "BVolumes.hpp"

#include <cassert>

namespace HorseRadish
{
	BBox::BBox(const Vector3f * const points, const unsigned int numVec)
	{
		minPt.set(Math::INFINITY);
		maxPt.set(-Math::INFINITY);

		Merge(points, numVec);
	}

	BBox::BBox(const BBox * const bboxes, const unsigned int numBBox)
	{
		minPt.set(Math::INFINITY);
		maxPt.set(-Math::INFINITY);

		for (unsigned int i = 0; i < numBBox; i++)
		{
			Merge(bboxes[i].maxPt);
			Merge(bboxes[i].minPt);
		}
	}

	BBox& BBox::operator=(const BBox& bbox)
	{
		minPt.set(bbox.minPt);
		maxPt.set(bbox.maxPt);
		return *this;
	}

	void BBox::operator+=(const BBox& bbox)
	{
		Merge(bbox.minPt);
		Merge(bbox.maxPt);
	}

	void BBox::operator+=(const Vector3f& pt)
	{
		Merge(pt);
	}

	float BBox::GetRadius(void) const
	{
		Vector3f center;

		GetCenter(center);
		return center.getDistance(minPt);
	}

	float BBox::GetRadiusMinimum(void) const
	{
		Vector3f center;
		float minDist;

		GetCenter(center);

		minDist = Math::fAbs(minPt[0] - center[0]);
		minDist = Math::fMin(minDist, Math::fAbs(minPt[1] - center[1]));
		return Math::fMin(minDist, Math::fAbs(minPt[2] - center[2]));
	}

	float BBox::GetVolume(void) const
	{
		return ((maxPt[0] - minPt[0]) * (maxPt[1] - minPt[1]) * (maxPt[2] - minPt[2]));
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
	void BBox::GetCorners(Vector3f points[8]) const
	{
		points[0].set(minPt[0], minPt[1], minPt[2]);
		points[1].set(minPt[0], minPt[1], maxPt[2]);
		points[2].set(minPt[0], maxPt[1], minPt[2]);
		points[3].set(minPt[0], maxPt[1], maxPt[2]);
		points[4].set(maxPt[0], minPt[1], minPt[2]);
		points[5].set(maxPt[0], minPt[1], maxPt[2]);
		points[6].set(maxPt[0], maxPt[1], minPt[2]);
		points[7].set(maxPt[0], maxPt[1], maxPt[2]);
	}

	void BBox::GetGeom(Vector3f points[36]) const
	{
		Vector3f corners[8];

		GetCorners(corners);

		//front
		points[0 * 6 + 0] = corners[1];	points[0 * 6 + 1] = corners[7];	points[0 * 6 + 2] = corners[3];
		points[0 * 6 + 3] = corners[1];	points[0 * 6 + 4] = corners[5];	points[0 * 6 + 5] = corners[7];
		//right
		points[1 * 6 + 0] = corners[5];	points[1 * 6 + 1] = corners[6];	points[1 * 6 + 2] = corners[7];
		points[1 * 6 + 3] = corners[5];	points[1 * 6 + 4] = corners[4];	points[1 * 6 + 5] = corners[6];
		//back
		points[2 * 6 + 0] = corners[4];	points[2 * 6 + 1] = corners[2];	points[2 * 6 + 2] = corners[6];
		points[2 * 6 + 3] = corners[4];	points[2 * 6 + 4] = corners[0];	points[2 * 6 + 5] = corners[2];
		//left
		points[3 * 6 + 0] = corners[0];	points[3 * 6 + 1] = corners[3];	points[3 * 6 + 2] = corners[2];
		points[3 * 6 + 3] = corners[0];	points[3 * 6 + 4] = corners[1];	points[3 * 6 + 5] = corners[3];
		//down
		points[4 * 6 + 0] = corners[0];	points[4 * 6 + 1] = corners[5];	points[4 * 6 + 2] = corners[1];
		points[4 * 6 + 3] = corners[0];	points[4 * 6 + 4] = corners[4];	points[4 * 6 + 5] = corners[5];
		//up
		points[5 * 6 + 0] = corners[3];	points[5 * 6 + 1] = corners[6];	points[5 * 6 + 2] = corners[2];
		points[5 * 6 + 3] = corners[3];	points[5 * 6 + 4] = corners[7];	points[5 * 6 + 5] = corners[6];
	}

	void BBox::GetBoundingSphere(Vector3f &center, float &radius) const
	{
		GetCenter(center);
		radius = center.getDistance(minPt);
	}

	void BBox::GetBoundingSphere(BSphere &bsphere) const
	{
		Vector3f center;

		GetCenter(center);
		bsphere.SetCenter(center);

		bsphere.SetRadius(center.getDistance(minPt));
	}

	float BBox::GetPlaneDistance(const Plane &plane) const
	{
		Vector3f center, planeNormal;
		float d1, d2;

		GetCenter(center);

		d1 = plane.GetDotCoord(center);
		plane.GetNormal(planeNormal);

		d2 = Math::fAbs((maxPt[0] - center[0]) * planeNormal[0]);
		d2 += Math::fAbs((maxPt[1] - center[1]) * planeNormal[1]);
		d2 += Math::fAbs((maxPt[2] - center[2]) * planeNormal[2]);

		if ((d1 - d2) > 0.0f)
			return (d1 - d2);
		if ((d1 + d2) < 0.0f)
			return (d1 + d2);
		return 0.0f;
	}

	Vector3f BBox::GetMainAxis() const
	{
		float tamX, tamY, tamZ;

		tamX = maxPt[0] - minPt[0];
		tamY = maxPt[1] - minPt[1];
		tamZ = maxPt[2] - minPt[2];

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

	void BBox::Merge(const Vector3f &pt)
	{
		__m128 vecData;

		vecData = _mm_loadu_ps(pt.data());
		_mm_storeu_ps(minPt.data(), _mm_min_ps(_mm_loadu_ps(minPt.data()), vecData));
		_mm_storeu_ps(maxPt.data(), _mm_max_ps(_mm_loadu_ps(maxPt.data()), vecData));
	}

	void BBox::Merge(const float * const pt)
	{
		minPt[0] = Math::fMin(minPt[0], pt[0]);
		minPt[1] = Math::fMin(minPt[1], pt[1]);
		minPt[2] = Math::fMin(minPt[2], pt[2]);
		maxPt[0] = Math::fMax(maxPt[0], pt[0]);
		maxPt[1] = Math::fMax(maxPt[1], pt[1]);
		maxPt[2] = Math::fMax(maxPt[2], pt[2]);
	}

	void BBox::Merge(const Vector3f * const pts, const int numPts)
	{
		__m128 vec1, vec2, finalMin, finalMax;
		int i, j;

		finalMin = _mm_loadu_ps(minPt.data());
		finalMax = _mm_loadu_ps(maxPt.data());

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

		_mm_storeu_ps(minPt.data(), finalMin);
		_mm_storeu_ps(maxPt.data(), finalMax);
	}

	void BBox::Merge(const float &x, const float &y, const float &z)
	{
		__m128 vecData;

		vecData = _mm_set_ps(0.0f, z, y, x);
		_mm_storeu_ps(minPt.data(), _mm_min_ps(_mm_loadu_ps(minPt.data()), vecData));
		_mm_storeu_ps(maxPt.data(), _mm_max_ps(_mm_loadu_ps(maxPt.data()), vecData));
	}

	void BBox::MergeSphere(const Vector3f &sphereCenter, const float sphereRadius)
	{
		__m128 smin, smax, aux, finalMin, finalMax;

		smin = smax = _mm_loadu_ps(sphereCenter.data());
		aux = _mm_set_ps1(sphereRadius);
		smin = _mm_sub_ps(smin, aux);
		smax = _mm_add_ps(smax, aux);

		finalMin = _mm_loadu_ps(minPt.data());
		finalMax = _mm_loadu_ps(maxPt.data());
		finalMin = _mm_min_ps(finalMin, smin);
		finalMax = _mm_max_ps(finalMax, smin);
		_mm_storeu_ps(minPt.data(), _mm_min_ps(finalMin, smax));
		_mm_storeu_ps(maxPt.data(), _mm_max_ps(finalMax, smax));
	}

	void BBox::MergeSphere(const BSphere &bsphere)
	{
		Vector3f pts[8], scenter;
		float sradius;

		bsphere.GetCenter(scenter);
		sradius = bsphere.GetRadius();

		pts[0].set(scenter[0] - sradius, scenter[1] + sradius, scenter[2] + sradius);
		pts[1].set(scenter[0] - sradius, scenter[1] - sradius, scenter[2] + sradius);
		pts[2].set(scenter[0] + sradius, scenter[1] + sradius, scenter[2] + sradius);
		pts[3].set(scenter[0] + sradius, scenter[1] - sradius, scenter[2] + sradius);

		pts[4].set(scenter[0] - sradius, scenter[1] + sradius, scenter[2] - sradius);
		pts[5].set(scenter[0] - sradius, scenter[1] - sradius, scenter[2] - sradius);
		pts[6].set(scenter[0] + sradius, scenter[1] + sradius, scenter[2] - sradius);
		pts[7].set(scenter[0] + sradius, scenter[1] - sradius, scenter[2] - sradius);

		Merge(pts, 8);
	}

	void BBox::MergeBox(const Vector3f &boxCenter, const float boxWidth, const float boxHeight, const float boxDepth)
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

		Merge(pts, 8);
	}

	void BBox::Translate(const Vector3f &translation)
	{
		minPt += translation;
		maxPt += translation;
	}

	void BBox::Translate(BBox& bbox, const Vector3f &translation) const
	{
		bbox.minPt = minPt;
		bbox.maxPt = maxPt;
		bbox.minPt += translation;
		bbox.maxPt += translation;
	}

	void BBox::Expand(const float amount)
	{
		minPt -= amount;
		maxPt += amount;
	}

	bool BBox::ContainsPoint(const Vector3f &point) const
	{
		if (point[0]<minPt[0] || point[1]<minPt[1] || point[2]<minPt[2] || point[0]>maxPt[0] || point[1]>maxPt[1] || point[2]>maxPt[2])
			return false;
		return true;
	}

	bool BBox::Intersects(const BBox &bbox) const
	{
		return !((bbox.maxPt[0] < minPt[0]) || (bbox.maxPt[1] < minPt[1]) || (bbox.maxPt[2] < minPt[2]) || (bbox.minPt[0] > maxPt[0]) || (bbox.minPt[1] > maxPt[1]) || (bbox.minPt[2] > maxPt[2]));
	}

	bool BBox::Intersects(const Vector3f &lineStart, const Vector3f &lineEnd) const
	{
		float ld[3];

		auto center = (this->minPt + this->maxPt) * 0.5f;
		auto extents = this->maxPt - center;
		auto lineDir = (lineEnd - lineStart) * 0.5f;
		auto lineCenter = lineStart + lineDir;
		auto dir = lineCenter - center;

		ld[0] = Math::fAbs(lineDir[0]);
		if (Math::fAbs(dir[0]) > (extents[0] + ld[0]))
			return false;

		ld[1] = Math::fAbs(lineDir[1]);
		if (Math::fAbs(dir[1]) > (extents[1] + ld[1]))
			return false;

		ld[2] = Math::fAbs(lineDir[2]);
		if (Math::fAbs(dir[2]) > (extents[2] + ld[2]))
			return false;

		auto cross = lineDir.crossProduct(dir);

		if (Math::fAbs(cross[0]) > (extents[1] * ld[2] + extents[2] * ld[1]))
			return false;
		if (Math::fAbs(cross[1]) > (extents[0] * ld[2] + extents[2] * ld[0]))
			return false;
		if (Math::fAbs(cross[2]) > (extents[0] * ld[1] + extents[1] * ld[0]))
			return false;
		return true;
	}

	bool BBox::Intersects(const Ray &ray, float * const rayHitDistance) const
	{
		int ax0, ax1, ax2, side, inside;
		float f, scale;
		Vector3f hit;

		//helper
		auto rayOrigin = ray.GetOrigin();
		auto rayDir = ray.GetDirection();

		//by omission
		if (rayHitDistance != nullptr)
			*rayHitDistance = 0.0f;

		ax0 = -1;
		inside = 0;
		scale = 0.0f;
		for (int i = 0; i < 3; i++)
		{
			if (rayOrigin[i] < this->minPt[i]) {
				side = 0;
			}
			else if (rayOrigin[i] > this->maxPt[i]) {
				side = 1;
			}
			else {
				inside++;
				continue;
			}

			if (rayDir[i] == 0.0f)
				continue;

			assert((side == 0) || (side == 1));

			f = (rayOrigin[i] - ((side == 0) ? this->minPt : this->maxPt)[i]);
			if ((ax0 < 0) || (Math::fAbs(f) > Math::fAbs(scale * rayDir[i])))
			{
				scale = -(f / rayDir[i]);
				ax0 = i;
			}
		}

		if (ax0 < 0)
			return (inside == 3);

		if (rayHitDistance != nullptr)
			*rayHitDistance = scale;

		ax1 = (ax0 + 1) % 3;
		ax2 = (ax0 + 2) % 3;
		hit[ax1] = rayOrigin[ax1] + scale * rayDir[ax1];
		hit[ax2] = rayOrigin[ax2] + scale * rayDir[ax2];

		return ((hit[ax1] >= this->minPt[ax1]) && (hit[ax1] <= this->maxPt[ax1]) && (hit[ax2] >= this->minPt[ax2]) && (hit[ax2] <= this->maxPt[ax2]));
	}

	BBox::Position BBox::Classify(const BBox &bbox)
	{
		if ((bbox.minPt[0] > maxPt[0]) || (bbox.maxPt[0] < minPt[0]))
			return Position::Outside;
		if ((bbox.minPt[1] > maxPt[1]) || (bbox.maxPt[1] < minPt[1]))
			return Position::Outside;
		if ((bbox.minPt[2] > maxPt[2]) || (bbox.maxPt[2] < minPt[2]))
			return Position::Outside;

		if ((bbox.minPt[0] > minPt[0]) && (bbox.maxPt[0] < maxPt[0]))
		{
			if ((bbox.minPt[1] > minPt[1]) && (bbox.maxPt[1] < maxPt[1]))
			{
				if ((bbox.minPt[2] > minPt[2]) && (bbox.maxPt[2] < maxPt[2]))
					return Position::Inside;
			}
		}

		return Position::Intersect;
	}

	float BSphere::calcDist(const float &px, const float &py, const float &pz) const
	{
		float d1 = x - px;
		float d2 = y - py;
		float d3 = z - pz;
		return sqrtf(d1*d1 + d2*d2 + d3*d3);
	}

	float BSphere::calcDist(const float * const vec) const
	{
		float d1 = x - vec[0];
		float d2 = y - vec[1];
		float d3 = z - vec[2];
		return sqrtf(d1*d1 + d2*d2 + d3*d3);
	}

	float BSphere::calcDist(const BSphere &bsphere) const
	{
		float d1 = x - bsphere.x;
		float d2 = y - bsphere.y;
		float d3 = z - bsphere.z;
		return sqrtf(d1*d1 + d2*d2 + d3*d3);
	}

	float BSphere::calcDist(const Vector3f &vec) const
	{
		float d1 = x - vec[0];
		float d2 = y - vec[1];
		float d3 = z - vec[2];
		return sqrtf(d1*d1 + d2*d2 + d3*d3);
	}

	BSphere& BSphere::operator=(const BSphere& bsphere)
	{
		x = bsphere.x;
		y = bsphere.y;
		z = bsphere.z;
		radius = bsphere.radius;
		return *this;
	}

	void BSphere::operator+=(const BSphere& bsphere)
	{
		Merge(bsphere);
	}

	void BSphere::Merge(const Vector3f &pt)
	{
		radius = Math::fMax(radius, calcDist(pt));
	}

	void BSphere::Merge(const float * const pt)
	{
		radius = Math::fMax(radius, calcDist(pt));
	}

	void BSphere::Merge(const Vector3f * const pts, const int numPts)
	{
		for (int i = 0; i < numPts; i++)
			this->radius = Math::fMax(this->radius, calcDist(pts[i]));
	}

	void BSphere::Merge(const float &x, const float &y, const float &z)
	{
		radius = Math::fMax(radius, calcDist(x, y, z));
	}

	void BSphere::Merge(const BSphere &sphere)
	{
		radius = Math::fMax(radius, calcDist(sphere) + sphere.radius);
	}

	void BSphere::Merge(const BBox &bbox)
	{
		BSphere boxSphere;

		bbox.GetBoundingSphere(boxSphere);
		radius = Math::fMax(radius, calcDist(boxSphere) + boxSphere.radius);
	}

	void BSphere::Expand(const float &amount)
	{
		this->radius += amount;
	}

	void BSphere::Contract(const float &amount)
	{
		this->radius -= amount;
	}

	bool BSphere::ContainsPoint(const Vector3f &point) const
	{
		if (calcDist(point) < radius)
			return true;
		return false;
	}

	bool BSphere::ContainsPoint(const float &px, const float &py, const float &pz) const
	{
		if (calcDist(px, py, pz) < radius)
			return true;
		return false;
	}

	bool BSphere::Intersects(const BSphere &bsphere) const
	{
		auto radiusSum = bsphere.radius + this->radius;

		auto vecDiff = Vector3f(bsphere.x, bsphere.y, bsphere.z) - Vector3f(this->x, this->y, this->z);

		return !(vecDiff.getDot() > (radiusSum * radiusSum));
	}

	bool BSphere::Intersects(const Vector3f &lineStart, const Vector3f &lineEnd) const
	{
		auto origem = Vector3f(this->x, this->y, this->z);

		auto s = lineStart - origem;
		auto e = lineEnd - origem;
		auto r = e - s;

		auto a = -s.getDot(r);
		auto radiusSquared = this->radius * this->radius;

		if (a <= 0.0f)
			return (s.getDot() < radiusSquared);

		if (a >= r.getDot())
			return (e.getDot() < radiusSquared);

		r = s + r * (a / r.getDot());
		return (r.getDot() < radiusSquared);
	}

	bool BSphere::Intersects(const Ray &ray) const
	{
		auto rayDir = ray.GetDirection();

		auto p = ray.GetOrigin() - Vector3f(this->x, this->y, this->z);
		auto a = rayDir.getDot();
		auto b = rayDir.getDot(p);
		auto c = p.getDot() - (this->radius * this->radius);
		auto d = (b * b) - (c * a);

		return !(d < 0.0f);
	}

	bool BSphere::Intersects(const Ray &ray, float &rayHitDistance1, float &rayHitDistance2) const
	{
		//by omission
		rayHitDistance1 = rayHitDistance2 = 0.0f;

		//helper
		auto rayDir = ray.GetDirection();

		auto p = ray.GetOrigin() - Vector3f(this->x, this->y, this->z);
		auto a = rayDir.getDot();
		auto b = rayDir.getDot(p);
		auto c = p.getDot() - (this->radius * this->radius);
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

	BSphere::Position BSphere::ClassifyBSphere(const BSphere &bsphere)
	{
		float centerDist = calcDist(bsphere);

		if ((centerDist + bsphere.radius) < radius)
			return BSphere::Position::Inside;
		if (centerDist - bsphere.radius > radius)
			return BSphere::Position::Outside;
		return BSphere::Position::Intersect;
	}

	BSphere::Position BSphere::ClassifyBBox(const BBox &bbox)
	{
		Vector3f pontos[8];
		unsigned char numIn, numOut;

		bbox.GetCorners(pontos);
		numIn = numOut = 0;

		for (int i = 0; i < 4; i++){
			calcDist(pontos[i]) < radius ? numIn++ : numOut++;
		}

		if (numIn != 0 && numOut != 0)
			return BSphere::Position::Intersect;

		for (int i = 4; i < 8; i++){
			calcDist(pontos[i]) < radius ? numIn++ : numOut++;
		}

		if (numIn != 0 && numOut != 0)
			return BSphere::Position::Intersect;
		if (numIn == 0)
			return BSphere::Position::Outside;
		return BSphere::Position::Inside;
	}

} //HorseRadish