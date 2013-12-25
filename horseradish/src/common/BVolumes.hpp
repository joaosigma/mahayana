#pragma once
#ifndef __HBOUNDING_VOLUMES__
#define __HBOUNDING_VOLUMES__

#include "Vector.hpp"
#include "Ray.hpp"
#include "Plane.hpp"
#include "Math.hpp"

namespace HorseRadish
{

class BBox;
class BSphere;

//§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§
//§§§§§§	 Bounding box	§§§§§
//§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§
class BBox
{
    Vector minPt,maxPt;

public:
	enum BoxPosition{
		Inside, Outside, Intersect
		};

    BBox(): minPt(Math::INFINITY), maxPt(-Math::INFINITY) { }
    BBox(const BBox& bbox): minPt(bbox.minPt), maxPt(bbox.maxPt) { }
	explicit BBox(const Vector * const points, const unsigned int numVec);
	explicit BBox(const BBox * const bboxes, const unsigned int numBBox);
	~BBox(){return;}

	BBox& operator=(const BBox& bbox);
	void operator+=(const BBox& bbox);
	void operator+=(const Vector& pt);

	void	GetMin(Vector &point) const {point.Set(minPt);}
	void	GetMax(Vector &point) const {point.Set(maxPt);}
	void	GetMin(float * const point) const {minPt.Write(point);}
	void	GetMax(float * const point) const {maxPt.Write(point);}
	void	GetMinMax(Vector &min, Vector &max) const {min.Set(minPt);max.Set(maxPt);}
	void	GetMinMax(float * const min, float * const max) const {minPt.Write(min);maxPt.Write(max);}
	void	GetCenter(Vector &point) const {point.Set((minPt.x+maxPt.x)*0.5f,(minPt.y+maxPt.y)*0.5f,(minPt.z+maxPt.z)*0.5f);}
	void	GetCenter(float * const point) const {point[0]=(minPt.x+maxPt.x)*0.5f;point[1]=(minPt.y+maxPt.y)*0.5f;point[2]=(minPt.z+maxPt.z)*0.5f;}
	void	GetDims(Vector &point) const {point.Set(maxPt.x-minPt.x, maxPt.y-minPt.y, maxPt.z-minPt.z);}
	void	GetDims(float * const point) const {point[0]=maxPt.x-minPt.x;point[1]=maxPt.y-minPt.y;point[2]=maxPt.z-minPt.z;}
	float	GetRadius(void) const;
	float	GetRadiusMinimum(void) const;
	float	GetVolume(void) const;
	void	GetCorners(Vector points[8]) const;
	void	GetGeom(Vector points[36]) const;
	void	GetBoundingSphere(Vector &center, float &radius) const;
	void	GetBoundingSphere(BSphere &bsphere) const;
	float	GetPlaneDistance(const Plane &plane) const;
	const float* GetMin() const {return minPt;}
	const float* GetMax() const {return maxPt;}
	Vector GetMainAxis() const;

	void Merge(const Vector &pt);
	void Merge(const float * const pt);
	void Merge(const Vector * const pts, const int numPts);
	void Merge(const float &x, const float &y, const float &z);
	void MergeSphere(const Vector &sphereCenter, const float sphereRadius);
	void MergeSphere(const BSphere &bsphere);
	void MergeBox(const Vector &boxCenter, const float boxWidth, const float boxHeight, const float boxDepth);

	void Set(const BBox * const bbox) {minPt.Set(bbox->minPt); maxPt.Set(bbox->maxPt);}
	void Set(const float &value) {minPt.Set(value); maxPt.Set(value);}
	void Set(const float &minValue, const float &maxValue) {minPt.Set(minValue); maxPt.Set(maxValue);}
	void SetMin(const Vector &min) {minPt.Set(min);}
	void SetMax(const Vector &max) {maxPt.Set(max);}
	void SetMinMax(const float * const min, const float * const max) {minPt.Set(min); maxPt.Set(max);}
	void SetMinMax(const Vector &min, const Vector &max) {minPt.Set(min); maxPt.Set(max);}
	void Reset(){minPt.x=minPt.y=minPt.z=Math::INFINITY; maxPt.x=maxPt.y=maxPt.z=-Math::INFINITY;}
	void Translate(const Vector &translation);
	void Translate(BBox& bbox, const Vector &translation) const;
	void Expand(const float amount);

	bool ContainsPoint(const Vector &point) const;

	bool Intersects(const BBox &bbox) const;
	bool Intersects(const Vector &lineStart, const Vector &lineEnd) const;
	bool Intersects(const Ray &ray, float * const rayHitDistance = nullptr) const;

	BoxPosition Classify(const BBox &bbox);
};

//§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§
//§§§§§§	 Bounding sphere	§§§§§
//§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§
class BSphere
{
    float x,y,z,radius;

	float calcDist(const float &px, const float &py, const float &pz) const;
	float calcDist(const float * const vec) const;
	float calcDist(const BSphere &bsphere) const;
	float calcDist(const Vector &vec) const;

public:
	enum POSITION{
		INSIDE, OUTSIDE, INTERSECT
		};

    BSphere() : x(0), y(0), z(0), radius(Math::INFINITY) { }
	BSphere(const BSphere& bsphere) : x(bsphere.x), y(bsphere.y), z(bsphere.z), radius(bsphere.radius) { }
	~BSphere() {}

	BSphere& operator=(const BSphere& bsphere);
	void operator+=(const BSphere& bsphere);

	float	GetRadius() const {return radius;}
	void	GetCenter(Vector &center) const {center.Set(x,y,z);}
	void	GetCenter(float * const center) const {center[0]=x;center[1]=y;center[2]=z;}

	void Merge(const Vector &pt);
	void Merge(const float * const pt);
	void Merge(const Vector * const pts, const int numPts);
	void Merge(const float &x, const float &y, const float &z);
	void Merge(const BSphere &sphere);
	void Merge(const BBox &bbox);

	void Expand(const float &amount);
	void Contract(const float &amount);

	void Set(const float &newx, const float &newy, const float &newz, const float &newradius) {x=newx; y=newy; z=newz; radius=newradius;}
	void Set(const Vector &center, const float &newradius) {x=center.x; y=center.y; z=center.z; radius=newradius;}
	void Set(const BBox &bbox) {bbox.GetBoundingSphere(*this);}
	void SetCenter(const float &newx, const float &newy, const float &newz) {x=newx; y=newy; z=newz;}
	void SetCenter(const Vector &center) {x=center.x; y=center.y; z=center.z;}
	void SetRadius(const float &newradius) {radius=newradius;}
	void Reset(){x=y=z=0.0f; radius=Math::INFINITY;}
	void Translate(const Vector &translation) {x+=translation.x; y+=translation.y; z+=translation.z;}
	void Translate(BSphere& bsphereDest, const Vector &translation) const {bsphereDest.x=x+translation.x; bsphereDest.y=y+translation.y; bsphereDest.z=z+translation.z; bsphereDest.radius=radius;}
	
	bool ContainsPoint(const Vector &point) const;
	bool ContainsPoint(const float &x, const float &y, const float &z) const;

	bool Intersects(const BSphere &bsphere) const;
	bool Intersects(const Vector &lineStart, const Vector &lineEnd) const;
	bool Intersects(const Ray &ray) const;
	bool Intersects(const Ray &ray, float &rayHitDistance1, float &rayHitDistance2) const;

	POSITION ClassifyBSphere(const BSphere &bsphere);
	POSITION ClassifyBBox(const BBox &bbox);
};

}//namespace HorseRadish

#endif