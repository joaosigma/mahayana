#include "frustum.hpp"

#include "..\openGL.hpp"

namespace HorseRadish { namespace OpenGL { namespace Tools {

void Frustum::extractPlanes(const HorseRadish::Vector4 &col1, const HorseRadish::Vector4 &col2, const HorseRadish::Vector4 &col3, const HorseRadish::Vector4 &col4)
{
	mPlanes[PlaneLeft].Set(col4.x + col1.x, col4.y + col1.y, col4.z + col1.z, col4.w + col1.w);
	mPlanes[PlaneRight].Set(col4.x - col1.x, col4.y - col1.y, col4.z - col1.z, col4.w - col1.w);

	mPlanes[PlaneTop].Set(col4.x - col2.x, col4.y - col2.y, col4.z - col2.z, col4.w - col2.w);
	mPlanes[PlaneBottom].Set(col4.x + col2.x, col4.y + col2.y, col4.z + col2.z, col4.w + col2.w);

	mPlanes[PlaneLeft].Normalize();
	mPlanes[PlaneRight].Normalize();
	mPlanes[PlaneBottom].Normalize();
	mPlanes[PlaneTop].Normalize();

	mPlanes[PlaneNear].Set(col4.x + col3.x, col4.y + col3.y, col4.z + col3.z, 0.0f);
	mPlanes[PlaneFar].Set(col4.x - col3.x, col4.y - col3.y, col4.z - col3.z, 0.0f);
	mPlanes[PlaneNear].NormalizeNormal();
	mPlanes[PlaneFar].NormalizeNormal();
	mPlanes[PlaneNear].SetD(-(mPlanes[PlaneNear].GetDotNormal(mPosition) + mZNear));
	mPlanes[PlaneFar].SetD(-(mPlanes[PlaneNear].GetDotNormal(mPosition) - mZFar));
}

bool Frustum::sweptSpherePlaneIntersect(float &t0, float &t1, const HorseRadish::Plane &plane, const HorseRadish::Vector &sphereCenter, const float &sphereRadius, const HorseRadish::Vector &sweepDir) const
{
	float b_dot_n, d_dot_n, tmp0, tmp1;

	b_dot_n = plane.GetDistance(sphereCenter);
	d_dot_n = plane.GetDotNormal(sweepDir);

	if (HorseRadish::Math::isZero(d_dot_n))
	{
		if (b_dot_n <= sphereRadius)
		{
			t0 = 0.0f;
			t1 = HorseRadish::Math::INFINITY;
			return true;
		}
		return false;
	}

	d_dot_n = 1.0f / d_dot_n;
	tmp0 = (sphereRadius - b_dot_n) * d_dot_n;
	tmp1 = (-sphereRadius - b_dot_n) * d_dot_n;
	t0 = HorseRadish::Math::fMin(tmp0, tmp1);
	t1 = HorseRadish::Math::fMax(tmp0, tmp1);
	return true;
}

void Frustum::getCorners(HorseRadish::Vector points[8]) const
{
    /*

    7---------------6
    |\              |\
    | \             | \
    |  \  zfar      |  \
    |   \           |   \
    |    \          |    \
    4-----\---------5     \
    \      \         \     \
     \      \         \     \
      \      3---------------2
       \     |          \    |
        \    |           \   |
         \   |    znear   \  |
          \  |             \ |
           \ 0---------------1

    */

	if (points == nullptr)
		return;

	mPlanes[PlaneNear].TestIntersectPlanes(mPlanes[PlaneLeft], mPlanes[PlaneBottom], points + 0);
	mPlanes[PlaneNear].TestIntersectPlanes(mPlanes[PlaneRight], mPlanes[PlaneBottom], points + 1);
	mPlanes[PlaneNear].TestIntersectPlanes(mPlanes[PlaneRight], mPlanes[PlaneTop], points + 2);
	mPlanes[PlaneNear].TestIntersectPlanes(mPlanes[PlaneLeft], mPlanes[PlaneTop], points + 3);

	mPlanes[PlaneFar].TestIntersectPlanes(mPlanes[PlaneLeft], mPlanes[PlaneBottom], points + 4);
	mPlanes[PlaneFar].TestIntersectPlanes(mPlanes[PlaneRight], mPlanes[PlaneBottom], points + 5);
	mPlanes[PlaneFar].TestIntersectPlanes(mPlanes[PlaneRight], mPlanes[PlaneTop], points + 6);
	mPlanes[PlaneFar].TestIntersectPlanes(mPlanes[PlaneLeft], mPlanes[PlaneTop], points + 7);
}

Frustum::IntersectionType Frustum::classifyFrustum(const Frustum &frustum) const
{
	HorseRadish::Vector corners[8];
	frustum.getCorners(corners);

	{
		auto inside = false;
		auto outside = false;
		for (auto i = 0; i < 8; i++)
		{
			auto resultado = testPoint(corners[i]);
			inside |= resultado;
			outside |= !resultado;
		}

		if (inside && !outside)
			return IntersectionType::FullInside;

		if (inside && outside)
			return IntersectionType::FrustumIntersect;
	}

	for (auto i = 0; i < 6; i++)
	{
		if (mPlanes[i].ClassifyPoint(corners[0]) != HorseRadish::Plane::Position::BEHIND)	continue;
		if (mPlanes[i].ClassifyPoint(corners[1]) != HorseRadish::Plane::Position::BEHIND)	continue;
		if (mPlanes[i].ClassifyPoint(corners[2]) != HorseRadish::Plane::Position::BEHIND)	continue;
		if (mPlanes[i].ClassifyPoint(corners[3]) != HorseRadish::Plane::Position::BEHIND)	continue;
		if (mPlanes[i].ClassifyPoint(corners[4]) != HorseRadish::Plane::Position::BEHIND)	continue;
		if (mPlanes[i].ClassifyPoint(corners[5]) != HorseRadish::Plane::Position::BEHIND)	continue;
		if (mPlanes[i].ClassifyPoint(corners[6]) != HorseRadish::Plane::Position::BEHIND)	continue;
		if (mPlanes[i].ClassifyPoint(corners[7]) != HorseRadish::Plane::Position::BEHIND)	continue;

		return IntersectionType::FullOutside;
	}

	//now invert the test in relation to the other frustum

	getCorners(corners);

	for (int i = 0; i < 6; i++)
	{
		if (frustum.mPlanes[i].ClassifyPoint(corners[0]) != HorseRadish::Plane::Position::BEHIND)	continue;
		if (frustum.mPlanes[i].ClassifyPoint(corners[1]) != HorseRadish::Plane::Position::BEHIND)	continue;
		if (frustum.mPlanes[i].ClassifyPoint(corners[2]) != HorseRadish::Plane::Position::BEHIND)	continue;
		if (frustum.mPlanes[i].ClassifyPoint(corners[3]) != HorseRadish::Plane::Position::BEHIND)	continue;
		if (frustum.mPlanes[i].ClassifyPoint(corners[4]) != HorseRadish::Plane::Position::BEHIND)	continue;
		if (frustum.mPlanes[i].ClassifyPoint(corners[5]) != HorseRadish::Plane::Position::BEHIND)	continue;
		if (frustum.mPlanes[i].ClassifyPoint(corners[6]) != HorseRadish::Plane::Position::BEHIND)	continue;
		if (frustum.mPlanes[i].ClassifyPoint(corners[7]) != HorseRadish::Plane::Position::BEHIND)	continue;

		return IntersectionType::FullOutside;
	}

	return IntersectionType::FrustumIntersect;
}

float Frustum::dotNormals(const PlaneIndex planeA, const PlaneIndex planeB) const
{
	if (planeA < 0 || planeA>5 || planeB < 0 || planeB>5)
		return 0.0f;

	HorseRadish::Vector normalB;
	mPlanes[planeB].GetNormal(normalB);
	return mPlanes[planeA].GetDotNormal(normalB);
}

void Frustum::calculateFrustum(const float * const transformation)
{
	HorseRadish::Vector4 col1, col2, col3, col4;
	HorseRadish::Matrix matTrans;

	matTrans.Set(transformation);

	matTrans.GetCol1(col1);
	matTrans.GetCol2(col2);
	matTrans.GetCol3(col3);
	matTrans.GetCol4(col4);

	extractPlanes(col1, col2, col3, col4);
}

void Frustum::calculateFrustum(const float * const matProjection, const float * const matModelview)
{
	HorseRadish::Vector4 col1, col2, col3, col4;
	HorseRadish::Matrix matTrans;

	matTrans.Set(matProjection);
	matTrans *= matModelview;

	matTrans.GetCol1(col1);
	matTrans.GetCol2(col2);
	matTrans.GetCol3(col3);
	matTrans.GetCol4(col4);

	extractPlanes(col1, col2, col3, col4);
}

void Frustum::calculateFrustum(const float * const modelViewMatrix, const float * const projectionMatrix, const HorseRadish::Vector &pos, const float zNear, const float zFar)
{
	HorseRadish::Matrix fClip, auxMat;
	HorseRadish::Vector4 col1, col2, col3, col4;

	auxMat.Set(modelViewMatrix);
	fClip.Set(projectionMatrix);
	fClip *= auxMat;

	this->mPosition = pos;
	this->mZNear = zNear;
	this->mZFar = zFar;

	fClip.GetCol1(col1);
	fClip.GetCol2(col2);
	fClip.GetCol3(col3);
	fClip.GetCol4(col4);

	extractPlanes(col1, col2, col3, col4);
}

void Frustum::setIndividualPlane(const PlaneIndex planeIndex, const HorseRadish::Plane &plane)
{
	if (planeIndex < 0 || planeIndex>5)
		return;
	mPlanes[planeIndex] = plane;
}

void Frustum::setFrustum(const Frustum *const frustum)
{
	mPlanes[0] = frustum->mPlanes[0];
	mPlanes[1] = frustum->mPlanes[1];
	mPlanes[2] = frustum->mPlanes[2];
	mPlanes[3] = frustum->mPlanes[3];
	mPlanes[4] = frustum->mPlanes[4];
	mPlanes[5] = frustum->mPlanes[5];
}

void Frustum::setFrustum(const HorseRadish::Vector &bboxMin, const HorseRadish::Vector &bboxMax)
{
	mPlanes[PlaneLeft].Set(1.0f, 0.0f, 0.0f, -bboxMin.x);
	mPlanes[PlaneRight].Set(-1.0f, 0.0f, 0.0f, bboxMax.x);
	mPlanes[PlaneTop].Set(0.0f, -1.0f, 0.0f, bboxMax.y);
	mPlanes[PlaneBottom].Set(0.0f, 1.0f, 0.0f, -bboxMin.y);
	mPlanes[PlaneNear].Set(0.0f, 0.0f, -1.0f, bboxMax.z);
	mPlanes[PlaneFar].Set(0.0f, 0.0f, 1.0f, -bboxMin.z);
}

void Frustum::setFrustum(const HorseRadish::Vector &center, const float radius)
{
	mPlanes[PlaneLeft].Set(1.0f, 0.0f, 0.0f, -(center.x - radius));
	mPlanes[PlaneRight].Set(-1.0f, 0.0f, 0.0f, center.x + radius);
	mPlanes[PlaneTop].Set(0.0f, -1.0f, 0.0f, center.y + radius);
	mPlanes[PlaneBottom].Set(0.0f, 1.0f, 0.0f, -(center.y - radius));
	mPlanes[PlaneNear].Set(0.0f, 0.0f, -1.0f, center.z + radius);
	mPlanes[PlaneFar].Set(0.0f, 0.0f, 1.0f, -(center.z - radius));
}

void Frustum::setFrustum(const HorseRadish::BBox &bbox)
{
	HorseRadish::Vector minP, maxP;

	bbox.GetMax(maxP);
	bbox.GetMin(minP);
	setFrustum(minP, maxP);
}

Frustum::Frustum()
	: mZNear(0.0f), mZFar(0.0f)
{
}

bool Frustum::testCube(const HorseRadish::Vector &point, const float &size) const
{
	HorseRadish::Vector pCubo[8];

	pCubo[0].Set(point);
	pCubo[1].Set(pCubo[0]);	pCubo[1].z += size;
	pCubo[2].Set(pCubo[1]);	pCubo[2].y += size;
	pCubo[3].Set(pCubo[0]);	pCubo[3].y += size;
	pCubo[4].Set(pCubo[0]);	pCubo[4].x += size;
	pCubo[5].Set(pCubo[4]);	pCubo[5].z += size;
	pCubo[6].Set(pCubo[5]);	pCubo[6].y += size;
	pCubo[7].Set(pCubo[4]);	pCubo[7].y += size;

	for (int iCurPlane = 0; iCurPlane < 6; iCurPlane++)
	{
		if (mPlanes[iCurPlane].GetDistance(pCubo[0]) > 0.0f)
			continue;
		if (mPlanes[iCurPlane].GetDistance(pCubo[1]) > 0.0f)
			continue;
		if (mPlanes[iCurPlane].GetDistance(pCubo[2]) > 0.0f)
			continue;
		if (mPlanes[iCurPlane].GetDistance(pCubo[3]) > 0.0f)
			continue;
		if (mPlanes[iCurPlane].GetDistance(pCubo[4]) > 0.0f)
			continue;
		if (mPlanes[iCurPlane].GetDistance(pCubo[5]) > 0.0f)
			continue;
		if (mPlanes[iCurPlane].GetDistance(pCubo[6]) > 0.0f)
			continue;
		if (mPlanes[iCurPlane].GetDistance(pCubo[7]) > 0.0f)
			continue;

		return false;
	}

	return true;
}

bool Frustum::testBox(const HorseRadish::Vector &min, const HorseRadish::Vector &max) const
{
	HorseRadish::Vector pBox[8];

	pBox[0].Set(min);
	pBox[1].Set(min);	pBox[1].z = max.z;
	pBox[2].Set(min);	pBox[2].y = max.y;
	pBox[3].Set(max);	pBox[3].x = min.x;

	pBox[4].Set(min);	pBox[4].x = max.x;
	pBox[5].Set(max);	pBox[5].y = min.y;
	pBox[6].Set(max);	pBox[6].z = min.z;
	pBox[7].Set(max);

	for (auto iPlane = 0; iPlane < 6; iPlane++)
	{
		if (mPlanes[iPlane].GetDistance(pBox[0]) > 0.0f)
			continue;
		if (mPlanes[iPlane].GetDistance(pBox[1]) > 0.0f)
			continue;
		if (mPlanes[iPlane].GetDistance(pBox[2]) > 0.0f)
			continue;
		if (mPlanes[iPlane].GetDistance(pBox[3]) > 0.0f)
			continue;
		if (mPlanes[iPlane].GetDistance(pBox[4]) > 0.0f)
			continue;
		if (mPlanes[iPlane].GetDistance(pBox[5]) > 0.0f)
			continue;
		if (mPlanes[iPlane].GetDistance(pBox[6]) > 0.0f)
			continue;
		if (mPlanes[iPlane].GetDistance(pBox[7]) > 0.0f)
			continue;

		return false;
	}

	return true;
}

bool Frustum::testBox(const HorseRadish::BBox &bbox) const
{
	HorseRadish::Vector minP, maxP;

	bbox.GetMax(maxP);
	bbox.GetMin(minP);
	return (testBox(minP, maxP));
}

bool Frustum::testSphere(const HorseRadish::Vector &center, const float &radius) const
{
	for (auto iPlane = 0; iPlane < 6; iPlane++)
	{
		if (mPlanes[iPlane].GetDistance(center) <= -radius)
			return false;
	}
	
	return true;
}

bool Frustum::testSphere(const HorseRadish::BSphere &bsphere) const
{
	HorseRadish::Vector ponto;

	bsphere.GetCenter(ponto);

	return testSphere(ponto, bsphere.GetRadius());
}

bool Frustum::testSphereBox(const HorseRadish::BSphere &bsphere, const HorseRadish::BBox &bbox) const
{
	if (testSphere(bsphere) == false)
		return false;
	return testBox(bbox);
}

bool Frustum::testPoint(const HorseRadish::Vector &point) const
{
	for (auto iPlane = 0; iPlane < 6; iPlane++)
	{
		if (mPlanes[iPlane].GetDistance(point) <= 0.0f)
			return false;
	}

	return true;
}

bool Frustum::testPolygon(const HorseRadish::Vector * const points, const int numPoints) const
{
	for (int iPlane = 0; iPlane < 6; iPlane++)
	{
		int j;
		for (j = 0; j < numPoints; j++)
		{
			if (mPlanes[iPlane].GetDistance(points[j]) > 0.0f)
				break;
		}

		if (j != numPoints)
			continue;
		return false;
	}

	return true;
}

bool Frustum::testSquare(const HorseRadish::Vector points[4]) const
{
	for (int iPlane = 0; iPlane < 6; iPlane++)
	{
		if (mPlanes[iPlane].GetDistance(points[0]) > 0.0f)
			continue;
		if (mPlanes[iPlane].GetDistance(points[1]) > 0.0f)
			continue;
		if (mPlanes[iPlane].GetDistance(points[2]) > 0.0f)
			continue;
		if (mPlanes[iPlane].GetDistance(points[3]) > 0.0f)
			continue;

		return false;
	}

	return true;
}

bool Frustum::testTri(const HorseRadish::Vector points[3]) const
{
	for (int iPlane = 0; iPlane < 6; iPlane++)
	{
		if (mPlanes[iPlane].GetDistance(points[0]) > 0.0f)
			continue;
		if (mPlanes[iPlane].GetDistance(points[1]) > 0.0f)
			continue;
		if (mPlanes[iPlane].GetDistance(points[2]) > 0.0f)
			continue;

		return false;
	}

	return true;
}

bool Frustum::testSweptSphere(const HorseRadish::Vector &sphereCenter, const float &sphereRadius, const HorseRadish::Vector &sweepDir) const
{
	float displacements[12], a, b;
	HorseRadish::Vector auxCenter;

	int cnt = 0;
	bool inFrustum = false;

	for (int iPlane = 0; iPlane < 6; iPlane++)
	{
		if (sweptSpherePlaneIntersect(a, b, mPlanes[iPlane], sphereCenter, sphereRadius, sweepDir))
		{
			if (a >= 0.f)
				displacements[cnt++] = a;
			if (b >= 0.f)
				displacements[cnt++] = b;
		}
	}

	for (int i = 0; i < cnt; i++)
	{
		auxCenter.x = sweepDir.x * displacements[i] + sphereCenter.x;
		auxCenter.y = sweepDir.y * displacements[i] + sphereCenter.y;
		auxCenter.z = sweepDir.z * displacements[i] + sphereCenter.z;

		inFrustum |= testSphere(auxCenter, sphereRadius * 1.1f);
	}

	return inFrustum;
}

} } }