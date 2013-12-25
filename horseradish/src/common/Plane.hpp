#pragma once
#ifndef __HPLANE__
#define __HPLANE__

#include "Vector.hpp"

namespace HorseRadish
{

class Plane
{
	float a,b,c,d;

public:
	enum POSITION{
		COPLANAR,
		FRONT,
		BEHIND,
		INTERSECT
		};

	Plane():a(0.0f),b(0.0f),c(0.0f),d(0.0f) {}
	~Plane() {}
	explicit Plane(const Plane &plane);
	explicit Plane(const float nx, const float ny, const float nz, const float nd);
	explicit Plane(const Vector &nN, const float nd);

	Plane& operator=(const Plane& plane)
		{a=plane.a;b=plane.b;c=plane.c;d=plane.d;return *this;}
	
	void Set(const float nx, const float ny, const float nz, const float nd)
		{a=nx;b=ny;c=nz;d=nd;}
	void SetNormal(const Vector &newNormal)
		{a=newNormal.x;b=newNormal.y;c=newNormal.z;}
	void SetNormal(const float nx, const float ny, const float nz)
		{a=nx;b=ny;c=nz;}
	void SetD(const float nd)
		{d=nd;}
	void SetFromPoints(const Vector &p0, const Vector &p1, const Vector &p2);
	void SetFromPoints(const float *p0, const float *p1, const float *p2);

	float GetDistance(const Vector &point) const
		{return (point.Dot(a,b,c)+d);}
	void GetNormal(Vector &normal) const
		{normal.Set(a,b,c);}
	float GetD() const
		{return d;}
	void GetComponents(Vector4 &coords) const
		{coords.Set(a,b,c,d);}
	int GetVertLocation() const
		{return ( ((a<0.f)?1:0) | ((b<0.f)?2:0) | ((c<0.f)?4:0) );}
	
	void Normalize(void);
	void NormalizeNormal(void);
	void NegateNormal()
		{a*=-1.0f;b*=-1.0f;c*=-1.0f;}
	void NegateD()
		{d*=-1.0f;}

	bool IntersectRay(const Vector &origin, const Vector &dir, Vector *resultado) const;
	bool IntersectLine(const Vector &p1, const Vector &p2, Vector *resultado) const;
	bool IntersectLineSegment(const Vector &p1, const Vector &p2, Vector *resultado) const;
	bool IntersectPlanes(const Plane &p2, const Plane &p3, Vector *resultado) const;
	bool IntersectSweptSphere(const float &sphereRadius, const Vector &spherePrevPos, const Vector &sphereCurPos, Vector * const hitPoint, float * const hitTime) const;

	void  Lerp(const Plane &p2, const float factor, Plane &result);
	void  CalcD(const Vector &pointOnPlane)
		{d=-pointOnPlane.Dot(a,b,c);}
	float DotCoord(const Vector &point) const
		{return (point.Dot(a,b,c)+d);}
	float DotNormal(const Vector &point) const
		{return (point.Dot(a,b,c));}
	
	POSITION ClassifyPoint(const Vector &point) const;
	POSITION ClassifyTri(const Vector &p1, const Vector &p2, const Vector &p3) const;
};

}//namespace HorseRadish

#endif