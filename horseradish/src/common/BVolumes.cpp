#include "BVolumes.hpp"

#include <cassert>

namespace HorseRadish
{
	BBox::BBox(const Vector * const points, const unsigned int numVec)
	{
		minPt.x = minPt.y = minPt.z = Math::INFINITY;
		maxPt.x = maxPt.y = maxPt.z = -Math::INFINITY;

		Merge(points, numVec);
	}

	BBox::BBox(const BBox * const bboxes, const unsigned int numBBox)
	{
		minPt.x = minPt.y = minPt.z = Math::INFINITY;
		maxPt.x = maxPt.y = maxPt.z = -Math::INFINITY;

		for (unsigned int i = 0; i < numBBox; i++)
		{
			Merge(bboxes[i].maxPt);
			Merge(bboxes[i].minPt);
		}
	}

	BBox& BBox::operator=(const BBox& bbox)
	{
		minPt.Set(bbox.minPt);
		maxPt.Set(bbox.maxPt);
		return *this;
	}

	void BBox::operator+=(const BBox& bbox)
	{
		Merge(bbox.minPt);
		Merge(bbox.maxPt);
	}

	void BBox::operator+=(const Vector& pt)
	{
		Merge(pt);
	}

	float BBox::GetRadius(void) const
	{
		Vector center;

		GetCenter(center);
		return center.GetDistance(minPt);
	}

	float BBox::GetRadiusMinimum(void) const
	{
		Vector center;
		float minDist;

		GetCenter(center);

		minDist = Math::fAbs(minPt.x - center.x);
		minDist = Math::fMin(minDist, Math::fAbs(minPt.y - center.y));
		return Math::fMin(minDist, Math::fAbs(minPt.z - center.z));
	}

	float BBox::GetVolume(void) const
	{
		return ((maxPt.x - minPt.x) * (maxPt.y - minPt.y) * (maxPt.z - minPt.z));
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
	void BBox::GetCorners(Vector points[8]) const
	{
		points[0].Set(minPt.x, minPt.y, minPt.z);
		points[1].Set(minPt.x, minPt.y, maxPt.z);
		points[2].Set(minPt.x, maxPt.y, minPt.z);
		points[3].Set(minPt.x, maxPt.y, maxPt.z);
		points[4].Set(maxPt.x, minPt.y, minPt.z);
		points[5].Set(maxPt.x, minPt.y, maxPt.z);
		points[6].Set(maxPt.x, maxPt.y, minPt.z);
		points[7].Set(maxPt.x, maxPt.y, maxPt.z);
	}

	void BBox::GetGeom(Vector points[36]) const
	{
		Vector corners[8];

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

	void BBox::GetBoundingSphere(Vector &center, float &radius) const
	{
		GetCenter(center);
		radius = center.GetDistance(minPt);
	}

	void BBox::GetBoundingSphere(BSphere &bsphere) const
	{
		Vector center;

		GetCenter(center);
		bsphere.SetCenter(center);

		bsphere.SetRadius(center.GetDistance(minPt));
	}

	float BBox::GetPlaneDistance(const Plane &plane) const
	{
		Vector center, planeNormal;
		float d1, d2;

		GetCenter(center);

		d1 = plane.GetDotCoord(center);
		plane.GetNormal(planeNormal);

		d2 = Math::fAbs((maxPt.x - center.x) * planeNormal.x);
		d2 += Math::fAbs((maxPt.y - center.y) * planeNormal.y);
		d2 += Math::fAbs((maxPt.z - center.z) * planeNormal.z);

		if ((d1 - d2) > 0.0f)
			return (d1 - d2);
		if ((d1 + d2) < 0.0f)
			return (d1 + d2);
		return 0.0f;
	}

	Vector BBox::GetMainAxis() const
	{
		float tamX, tamY, tamZ;

		tamX = maxPt.x - minPt.x;
		tamY = maxPt.y - minPt.y;
		tamZ = maxPt.z - minPt.z;

		if (tamX > tamY && tamX > tamZ)
			return Vector(1.0f, 0.0f, 0.0f);
		if (tamY > tamX && tamY > tamZ)
			return Vector(0.0f, 1.0f, 0.0f);
		if (tamZ > tamX && tamZ > tamY)
			return Vector(0.0f, 0.0f, 1.0f);

		if (Math::isZero(tamX - tamY) && Math::isZero(tamY - tamZ))
			return Vector(1.0f, 1.0f, 1.0);
		if (Math::isZero(tamX - tamY))
			return Vector(1.0f, 1.0f, 0.0);
		if (Math::isZero(tamX - tamZ))
			return Vector(1.0f, 0.0f, 1.0);
		return Vector(0.0f, 1.0f, 1.0);
	}

	void BBox::Merge(const Vector &pt)
	{
		__m128 vecData;

		vecData = _mm_loadu_ps(&pt.x);
		_mm_storeu_ps(&minPt.x, _mm_min_ps(_mm_loadu_ps(&minPt.x), vecData));
		_mm_storeu_ps(&maxPt.x, _mm_max_ps(_mm_loadu_ps(&maxPt.x), vecData));
	}

	void BBox::Merge(const float * const pt)
	{
		minPt.x = Math::fMin(minPt.x, pt[0]);
		minPt.y = Math::fMin(minPt.y, pt[1]);
		minPt.z = Math::fMin(minPt.z, pt[2]);
		maxPt.x = Math::fMax(maxPt.x, pt[0]);
		maxPt.y = Math::fMax(maxPt.y, pt[1]);
		maxPt.z = Math::fMax(maxPt.z, pt[2]);
	}

	void BBox::Merge(const Vector * const pts, const int numPts)
	{
		__m128 vec1, vec2, finalMin, finalMax;
		int i, j;

		finalMin = _mm_loadu_ps(&minPt.x);
		finalMax = _mm_loadu_ps(&maxPt.x);

		for (i = numPts, j = 0; i >= 2; i -= 2, j += 2)
		{
			vec1 = _mm_loadu_ps(&pts[j].x);
			vec2 = _mm_loadu_ps(&pts[j + 1].x);
			finalMin = _mm_min_ps(finalMin, vec1);
			finalMax = _mm_max_ps(finalMax, vec1);
			finalMin = _mm_min_ps(finalMin, vec2);
			finalMax = _mm_max_ps(finalMax, vec2);
		}

		if (i > 0)
		{
			vec1 = _mm_loadu_ps(&pts[j].x);
			finalMin = _mm_min_ps(finalMin, vec1);
			finalMax = _mm_max_ps(finalMax, vec1);
		}

		_mm_storeu_ps(&minPt.x, finalMin);
		_mm_storeu_ps(&maxPt.x, finalMax);
	}

	void BBox::Merge(const float &x, const float &y, const float &z)
	{
		__m128 vecData;

		vecData = _mm_set_ps(0.0f, z, y, x);
		_mm_storeu_ps(&minPt.x, _mm_min_ps(_mm_loadu_ps(&minPt.x), vecData));
		_mm_storeu_ps(&maxPt.x, _mm_max_ps(_mm_loadu_ps(&maxPt.x), vecData));
	}

	void BBox::MergeSphere(const Vector &sphereCenter, const float sphereRadius)
	{
		__m128 smin, smax, aux, finalMin, finalMax;

		smin = smax = _mm_loadu_ps(&sphereCenter.x);
		aux = _mm_set_ps1(sphereRadius);
		smin = _mm_sub_ps(smin, aux);
		smax = _mm_add_ps(smax, aux);

		finalMin = _mm_loadu_ps(&minPt.x);
		finalMax = _mm_loadu_ps(&maxPt.x);
		finalMin = _mm_min_ps(finalMin, smin);
		finalMax = _mm_max_ps(finalMax, smin);
		_mm_storeu_ps(&minPt.x, _mm_min_ps(finalMin, smax));
		_mm_storeu_ps(&maxPt.x, _mm_max_ps(finalMax, smax));
	}

	void BBox::MergeSphere(const BSphere &bsphere)
	{
		Vector pts[8], scenter;
		float sradius;

		bsphere.GetCenter(scenter);
		sradius = bsphere.GetRadius();

		pts[0].Set(scenter.x - sradius, scenter.y + sradius, scenter.z + sradius);
		pts[1].Set(scenter.x - sradius, scenter.y - sradius, scenter.z + sradius);
		pts[2].Set(scenter.x + sradius, scenter.y + sradius, scenter.z + sradius);
		pts[3].Set(scenter.x + sradius, scenter.y - sradius, scenter.z + sradius);

		pts[4].Set(scenter.x - sradius, scenter.y + sradius, scenter.z - sradius);
		pts[5].Set(scenter.x - sradius, scenter.y - sradius, scenter.z - sradius);
		pts[6].Set(scenter.x + sradius, scenter.y + sradius, scenter.z - sradius);
		pts[7].Set(scenter.x + sradius, scenter.y - sradius, scenter.z - sradius);

		Merge(pts, 8);
	}

	void BBox::MergeBox(const Vector &boxCenter, const float boxWidth, const float boxHeight, const float boxDepth)
	{
		Vector pts[8];

		pts[0].Set(boxCenter.x - boxWidth, boxCenter.y + boxHeight, boxCenter.z + boxDepth);
		pts[1].Set(boxCenter.x - boxWidth, boxCenter.y - boxHeight, boxCenter.z + boxDepth);
		pts[2].Set(boxCenter.x + boxWidth, boxCenter.y + boxHeight, boxCenter.z + boxDepth);
		pts[3].Set(boxCenter.x + boxWidth, boxCenter.y - boxHeight, boxCenter.z + boxDepth);

		pts[4].Set(boxCenter.x - boxWidth, boxCenter.y + boxHeight, boxCenter.z - boxDepth);
		pts[5].Set(boxCenter.x - boxWidth, boxCenter.y - boxHeight, boxCenter.z - boxDepth);
		pts[6].Set(boxCenter.x + boxWidth, boxCenter.y + boxHeight, boxCenter.z - boxDepth);
		pts[7].Set(boxCenter.x + boxWidth, boxCenter.y - boxHeight, boxCenter.z - boxDepth);

		Merge(pts, 8);
	}

	void BBox::Translate(const Vector &translation)
	{
		minPt += translation;
		maxPt += translation;
	}

	void BBox::Translate(BBox& bbox, const Vector &translation) const
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

	bool BBox::ContainsPoint(const Vector &point) const
	{
		if (point.x<minPt.x || point.y<minPt.y || point.z<minPt.z || point.x>maxPt.x || point.y>maxPt.y || point.z>maxPt.z)
			return false;
		return true;
	}

	bool BBox::Intersects(const BBox &bbox) const
	{
		return !((bbox.maxPt.x < this->minPt.x) || (bbox.maxPt.y < this->minPt.y) || (bbox.maxPt.z < this->minPt.z) || (bbox.minPt.x > this->maxPt.x) || (bbox.minPt.y > this->maxPt.y) || (bbox.minPt.z > this->maxPt.z));
	}

	bool BBox::Intersects(const Vector &lineStart, const Vector &lineEnd) const
	{
		float ld[3];

		auto center = (this->minPt + this->maxPt) * 0.5f;
		auto extents = this->maxPt - center;
		auto lineDir = (lineEnd - lineStart) * 0.5f;
		auto lineCenter = lineStart + lineDir;
		auto dir = lineCenter - center;

		ld[0] = Math::fAbs(lineDir.x);
		if (Math::fAbs(dir.x) > (extents.x + ld[0]))
			return false;

		ld[1] = Math::fAbs(lineDir.y);
		if (Math::fAbs(dir.y) > (extents.y + ld[1]))
			return false;

		ld[2] = Math::fAbs(lineDir.z);
		if (Math::fAbs(dir.z) > (extents.z + ld[2]))
			return false;

		auto cross = lineDir.CrossProduct(dir);

		if (Math::fAbs(cross.x) > (extents.y * ld[2] + extents.z * ld[1]))
			return false;
		if (Math::fAbs(cross.y) > (extents.x * ld[2] + extents.z * ld[0]))
			return false;
		if (Math::fAbs(cross.z) > (extents.x * ld[1] + extents.y * ld[0]))
			return false;
		return true;
	}

	bool BBox::Intersects(const Ray &ray, float * const rayHitDistance) const
	{
		int ax0, ax1, ax2, side, inside;
		float f, scale;
		Vector hit;

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
		if ((bbox.minPt.x > maxPt.x) || (bbox.maxPt.x < minPt.x))
			return Position::Outside;
		if ((bbox.minPt.y > maxPt.y) || (bbox.maxPt.y < minPt.y))
			return Position::Outside;
		if ((bbox.minPt.z > maxPt.z) || (bbox.maxPt.z < minPt.z))
			return Position::Outside;

		if ((bbox.minPt.x > minPt.x) && (bbox.maxPt.x < maxPt.x))
		{
			if ((bbox.minPt.y > minPt.y) && (bbox.maxPt.y < maxPt.y))
			{
				if ((bbox.minPt.z > minPt.z) && (bbox.maxPt.z < maxPt.z))
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

	float BSphere::calcDist(const Vector &vec) const
	{
		float d1 = x - vec.x;
		float d2 = y - vec.y;
		float d3 = z - vec.z;
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

	void BSphere::Merge(const Vector &pt)
	{
		radius = Math::fMax(radius, calcDist(pt));
	}

	void BSphere::Merge(const float * const pt)
	{
		radius = Math::fMax(radius, calcDist(pt));
	}

	void BSphere::Merge(const Vector * const pts, const int numPts)
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

	bool BSphere::ContainsPoint(const Vector &point) const
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

		auto vecDiff = Vector(bsphere.x, bsphere.y, bsphere.z) - Vector(this->x, this->y, this->z);

		return !(vecDiff.GetMagnitudeSquared() > (radiusSum * radiusSum));
	}

	bool BSphere::Intersects(const Vector &lineStart, const Vector &lineEnd) const
	{
		auto origem = Vector(this->x, this->y, this->z);

		auto s = lineStart - origem;
		auto e = lineEnd - origem;
		auto r = e - s;

		auto a = -s.GetDot(r);
		auto radiusSquared = this->radius * this->radius;

		if (a <= 0.0f)
			return (s.GetDot() < radiusSquared);

		if (a >= r.GetDot())
			return (e.GetDot() < radiusSquared);

		r = s + r * (a / r.GetDot());
		return (r.GetDot() < radiusSquared);
	}

	bool BSphere::Intersects(const Ray &ray) const
	{
		auto rayDir = ray.GetDirection();

		auto p = ray.GetOrigin() - Vector(this->x, this->y, this->z);
		auto a = rayDir.GetDot();
		auto b = rayDir.GetDot(p);
		auto c = p.GetDot() - (this->radius * this->radius);
		auto d = (b * b) - (c * a);

		return !(d < 0.0f);
	}

	bool BSphere::Intersects(const Ray &ray, float &rayHitDistance1, float &rayHitDistance2) const
	{
		//by omission
		rayHitDistance1 = rayHitDistance2 = 0.0f;

		//helper
		auto rayDir = ray.GetDirection();

		auto p = ray.GetOrigin() - Vector(this->x, this->y, this->z);
		auto a = rayDir.GetDot();
		auto b = rayDir.GetDot(p);
		auto c = p.GetDot() - (this->radius * this->radius);
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
		Vector pontos[8];
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