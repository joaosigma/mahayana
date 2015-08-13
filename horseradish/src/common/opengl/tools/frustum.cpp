#include "frustum.hpp"

#include "..\openGL.hpp"

#include <limits>

namespace HorseRadish { namespace OpenGL { namespace Tools {

void Frustum::extractPlanes(const HorseRadish::Vector4f &col1, const HorseRadish::Vector4f &col2, const HorseRadish::Vector4f &col3, const HorseRadish::Vector4f &col4)
{
	mPlanes[PlaneLeft].Set(col4[0] + col1[0], col4[1] + col1[1], col4[2] + col1[2], col4[3] + col1[3]);
	mPlanes[PlaneRight].Set(col4[0] - col1[0], col4[1] - col1[1], col4[2] - col1[2], col4[3] - col1[3]);

	mPlanes[PlaneTop].Set(col4[0] - col2[0], col4[1] - col2[1], col4[2] - col2[2], col4[3] - col2[3]);
	mPlanes[PlaneBottom].Set(col4[0] + col2[0], col4[1] + col2[1], col4[2] + col2[2], col4[3] + col2[3]);

	mPlanes[PlaneLeft].Normalize();
	mPlanes[PlaneRight].Normalize();
	mPlanes[PlaneBottom].Normalize();
	mPlanes[PlaneTop].Normalize();

	mPlanes[PlaneNear].Set(col4[0] + col3[0], col4[1] + col3[1], col4[2] + col3[2], 0.0f);
	mPlanes[PlaneFar].Set(col4[0] - col3[0], col4[1] - col3[1], col4[2] - col3[2], 0.0f);
	mPlanes[PlaneNear].NormalizeNormal();
	mPlanes[PlaneFar].NormalizeNormal();
	mPlanes[PlaneNear].SetD(-(mPlanes[PlaneNear].GetDotNormal(mPosition) + mZNear));
	mPlanes[PlaneFar].SetD(-(mPlanes[PlaneNear].GetDotNormal(mPosition) - mZFar));
}

bool Frustum::sweptSpherePlaneIntersect(float &t0, float &t1, const HorseRadish::Plane &plane, const HorseRadish::Vector3f &sphereCenter, const float &sphereRadius, const HorseRadish::Vector3f &sweepDir) const
{
	float b_dot_n, d_dot_n, tmp0, tmp1;

	b_dot_n = plane.GetDistance(sphereCenter);
	d_dot_n = plane.GetDotNormal(sweepDir);

	if (HorseRadish::Math::isZero(d_dot_n))
	{
		if (b_dot_n <= sphereRadius)
		{
			t0 = 0.0f;
			t1 = std::numeric_limits<float>::infinity();
			return true;
		}
		return false;
	}

	d_dot_n = 1.0f / d_dot_n;
	tmp0 = (sphereRadius - b_dot_n) * d_dot_n;
	tmp1 = (-sphereRadius - b_dot_n) * d_dot_n;
	t0 = std::fmin(tmp0, tmp1);
	t1 = std::fmax(tmp0, tmp1);
	return true;
}

void Frustum::getCorners(HorseRadish::Vector3f points[8]) const
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

	mPlanes[PlaneNear].TestIntersectPlanes(mPlanes[PlaneLeft], mPlanes[PlaneBottom], points[0]);
	mPlanes[PlaneNear].TestIntersectPlanes(mPlanes[PlaneRight], mPlanes[PlaneBottom], points[1]);
	mPlanes[PlaneNear].TestIntersectPlanes(mPlanes[PlaneRight], mPlanes[PlaneTop], points[2]);
	mPlanes[PlaneNear].TestIntersectPlanes(mPlanes[PlaneLeft], mPlanes[PlaneTop], points[3]);

	mPlanes[PlaneFar].TestIntersectPlanes(mPlanes[PlaneLeft], mPlanes[PlaneBottom], points[4]);
	mPlanes[PlaneFar].TestIntersectPlanes(mPlanes[PlaneRight], mPlanes[PlaneBottom], points[5]);
	mPlanes[PlaneFar].TestIntersectPlanes(mPlanes[PlaneRight], mPlanes[PlaneTop], points[6]);
	mPlanes[PlaneFar].TestIntersectPlanes(mPlanes[PlaneLeft], mPlanes[PlaneTop], points[7]);
}

Frustum::IntersectionType Frustum::classifyFrustum(const Frustum &frustum) const
{
	HorseRadish::Vector3f corners[8];
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

	HorseRadish::Vector3f normalB;
	mPlanes[planeB].GetNormal(normalB);
	return mPlanes[planeA].GetDotNormal(normalB);
}

void Frustum::calculateFrustum(const float * const transformation)
{
	HorseRadish::Matrix matTrans;

	matTrans.set(transformation);

	HorseRadish::Vector4f col1 = matTrans.getColumn(0);
	HorseRadish::Vector4f col2 = matTrans.getColumn(1);
	HorseRadish::Vector4f col3 = matTrans.getColumn(2);
	HorseRadish::Vector4f col4 = matTrans.getColumn(3);

	extractPlanes(col1, col2, col3, col4);
}

void Frustum::calculateFrustum(const float * const matProjection, const float * const matModelview)
{
	HorseRadish::Matrix matTrans;

	matTrans.set(matProjection);
	matTrans *= matModelview;

	HorseRadish::Vector4f col1 = matTrans.getColumn(0);
	HorseRadish::Vector4f col2 = matTrans.getColumn(1);
	HorseRadish::Vector4f col3 = matTrans.getColumn(2);
	HorseRadish::Vector4f col4 = matTrans.getColumn(3);

	extractPlanes(col1, col2, col3, col4);
}

void Frustum::calculateFrustum(const float * const modelViewMatrix, const float * const projectionMatrix, const HorseRadish::Vector3f &pos, const float zNear, const float zFar)
{
	HorseRadish::Matrix fClip;

	fClip.set(projectionMatrix);
	fClip *= HorseRadish::Matrix(modelViewMatrix);

	this->mPosition = pos;
	this->mZNear = zNear;
	this->mZFar = zFar;

	HorseRadish::Vector4f col1 = fClip.getColumn(0);
	HorseRadish::Vector4f col2 = fClip.getColumn(1);
	HorseRadish::Vector4f col3 = fClip.getColumn(2);
	HorseRadish::Vector4f col4 = fClip.getColumn(3);

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

void Frustum::setFrustum(const HorseRadish::Vector3f &bboxMin, const HorseRadish::Vector3f &bboxMax)
{
	mPlanes[PlaneLeft].Set(1.0f, 0.0f, 0.0f, -bboxMin[0]);
	mPlanes[PlaneRight].Set(-1.0f, 0.0f, 0.0f, bboxMax[0]);
	mPlanes[PlaneTop].Set(0.0f, -1.0f, 0.0f, bboxMax[1]);
	mPlanes[PlaneBottom].Set(0.0f, 1.0f, 0.0f, -bboxMin[1]);
	mPlanes[PlaneNear].Set(0.0f, 0.0f, -1.0f, bboxMax[2]);
	mPlanes[PlaneFar].Set(0.0f, 0.0f, 1.0f, -bboxMin[2]);
}

void Frustum::setFrustum(const HorseRadish::Vector3f &center, const float radius)
{
	mPlanes[PlaneLeft].Set(1.0f, 0.0f, 0.0f, -(center[0] - radius));
	mPlanes[PlaneRight].Set(-1.0f, 0.0f, 0.0f, center[0] + radius);
	mPlanes[PlaneTop].Set(0.0f, -1.0f, 0.0f, center[1] + radius);
	mPlanes[PlaneBottom].Set(0.0f, 1.0f, 0.0f, -(center[1] - radius));
	mPlanes[PlaneNear].Set(0.0f, 0.0f, -1.0f, center[2] + radius);
	mPlanes[PlaneFar].Set(0.0f, 0.0f, 1.0f, -(center[2] - radius));
}

void Frustum::setFrustum(const HorseRadish::BBox &bbox)
{
	HorseRadish::Vector3f minP, maxP;

	bbox.GetMax(maxP);
	bbox.GetMin(minP);
	setFrustum(minP, maxP);
}

Frustum::Frustum()
	: mZNear(0.0f), mZFar(0.0f)
{
}

bool Frustum::testCube(const HorseRadish::Vector3f &point, const float &size) const
{
	HorseRadish::Vector3f pCubo[8];

	pCubo[0].set(point);
	pCubo[1].set(pCubo[0]);	pCubo[1][2] += size;
	pCubo[2].set(pCubo[1]);	pCubo[2][1] += size;
	pCubo[3].set(pCubo[0]);	pCubo[3][1] += size;
	pCubo[4].set(pCubo[0]);	pCubo[4][0] += size;
	pCubo[5].set(pCubo[4]);	pCubo[5][2] += size;
	pCubo[6].set(pCubo[5]);	pCubo[6][1] += size;
	pCubo[7].set(pCubo[4]);	pCubo[7][1] += size;

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

bool Frustum::testBox(const HorseRadish::Vector3f &min, const HorseRadish::Vector3f &max) const
{
	HorseRadish::Vector3f pBox[8];

	pBox[0].set(min);
	pBox[1].set(min);	pBox[1][2] = max[2];
	pBox[2].set(min);	pBox[2][1] = max[1];
	pBox[3].set(max);	pBox[3][0] = min[0];

	pBox[4].set(min);	pBox[4][0] = max[0];
	pBox[5].set(max);	pBox[5][1] = min[1];
	pBox[6].set(max);	pBox[6][2] = min[2];
	pBox[7].set(max);

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
	HorseRadish::Vector3f minP, maxP;

	bbox.GetMax(maxP);
	bbox.GetMin(minP);
	return (testBox(minP, maxP));
}

bool Frustum::testSphere(const HorseRadish::Vector3f &center, const float &radius) const
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
	HorseRadish::Vector3f ponto;

	bsphere.GetCenter(ponto);

	return testSphere(ponto, bsphere.GetRadius());
}

bool Frustum::testSphereBox(const HorseRadish::BSphere &bsphere, const HorseRadish::BBox &bbox) const
{
	if (testSphere(bsphere) == false)
		return false;
	return testBox(bbox);
}

bool Frustum::testPoint(const HorseRadish::Vector3f &point) const
{
	for (auto iPlane = 0; iPlane < 6; iPlane++)
	{
		if (mPlanes[iPlane].GetDistance(point) <= 0.0f)
			return false;
	}

	return true;
}

bool Frustum::testPolygon(const HorseRadish::Vector3f * const points, const int numPoints) const
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

bool Frustum::testSquare(const HorseRadish::Vector3f points[4]) const
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

bool Frustum::testTri(const HorseRadish::Vector3f points[3]) const
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

bool Frustum::testSweptSphere(const HorseRadish::Vector3f &sphereCenter, const float &sphereRadius, const HorseRadish::Vector3f &sweepDir) const
{
	float displacements[12], a, b;
	HorseRadish::Vector3f auxCenter;

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
		auxCenter[0] = sweepDir[0] * displacements[i] + sphereCenter[0];
		auxCenter[1] = sweepDir[1] * displacements[i] + sphereCenter[1];
		auxCenter[2] = sweepDir[2] * displacements[i] + sphereCenter[2];

		inFrustum |= testSphere(auxCenter, sphereRadius * 1.1f);
	}

	return inFrustum;
}

} } }