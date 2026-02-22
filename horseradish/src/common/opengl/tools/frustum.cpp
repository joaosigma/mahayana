#include "frustum.hpp"

#include "../openGL.hpp"

#include <limits>

namespace hr::gl::tools
{
    void Frustum::extractPlanes(const hr::Vector4f& col1, const hr::Vector4f& col2, const hr::Vector4f& col3, const hr::Vector4f& col4)
    {
        mPlanes[static_cast<size_t>(PlaneIndex::Left)] = hr::Plane<float>{col4[0] + col1[0], col4[1] + col1[1], col4[2] + col1[2], col4[3] + col1[3]};
        mPlanes[static_cast<size_t>(PlaneIndex::Right)] = hr::Plane<float>{col4[0] - col1[0], col4[1] - col1[1], col4[2] - col1[2], col4[3] - col1[3]};

        mPlanes[static_cast<size_t>(PlaneIndex::Top)] = hr::Plane<float>{col4[0] - col2[0], col4[1] - col2[1], col4[2] - col2[2], col4[3] - col2[3]};
        mPlanes[static_cast<size_t>(PlaneIndex::Bottom)] = hr::Plane<float>{col4[0] + col2[0], col4[1] + col2[1], col4[2] + col2[2], col4[3] + col2[3]};

        mPlanes[static_cast<size_t>(PlaneIndex::Left)].normalize();
        mPlanes[static_cast<size_t>(PlaneIndex::Right)].normalize();
        mPlanes[static_cast<size_t>(PlaneIndex::Bottom)].normalize();
        mPlanes[static_cast<size_t>(PlaneIndex::Top)].normalize();

        mPlanes[static_cast<size_t>(PlaneIndex::Near)] = hr::Plane<float>{col4[0] + col3[0], col4[1] + col3[1], col4[2] + col3[2], 0.0f};
        mPlanes[static_cast<size_t>(PlaneIndex::Far)] = hr::Plane<float>{col4[0] - col3[0], col4[1] - col3[1], col4[2] - col3[2], 0.0f};
        mPlanes[static_cast<size_t>(PlaneIndex::Near)].normalizeNormal();
        mPlanes[static_cast<size_t>(PlaneIndex::Far)].normalizeNormal();
        mPlanes[static_cast<size_t>(PlaneIndex::Near)].replaceD(-(mPlanes[static_cast<size_t>(PlaneIndex::Near)].dotNormal(mPosition) + mZNear));
        mPlanes[static_cast<size_t>(PlaneIndex::Far)].replaceD(-(mPlanes[static_cast<size_t>(PlaneIndex::Near)].dotNormal(mPosition) - mZFar));
    }

    bool Frustum::sweptSpherePlaneIntersect(float& t0,
                                            float& t1,
                                            const hr::Plane<float>& plane,
                                            const hr::Vector3f& sphereCenter,
                                            const float& sphereRadius,
                                            const hr::Vector3f& sweepDir) const
    {
        float b_dot_n, d_dot_n, tmp0, tmp1;

        b_dot_n = plane.distance(sphereCenter);
        d_dot_n = plane.dotNormal(sweepDir);

        if (hr::Math::isZero(d_dot_n))
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

    void Frustum::getCorners(hr::Vector3f points[8]) const
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

        if (!points)
            return;

        mPlanes[static_cast<size_t>(PlaneIndex::Near)].intersects(mPlanes[static_cast<size_t>(PlaneIndex::Left)], mPlanes[static_cast<size_t>(PlaneIndex::Bottom)], points[0]);
        mPlanes[static_cast<size_t>(PlaneIndex::Near)].intersects(mPlanes[static_cast<size_t>(PlaneIndex::Right)], mPlanes[static_cast<size_t>(PlaneIndex::Bottom)], points[1]);
        mPlanes[static_cast<size_t>(PlaneIndex::Near)].intersects(mPlanes[static_cast<size_t>(PlaneIndex::Right)], mPlanes[static_cast<size_t>(PlaneIndex::Top)], points[2]);
        mPlanes[static_cast<size_t>(PlaneIndex::Near)].intersects(mPlanes[static_cast<size_t>(PlaneIndex::Left)], mPlanes[static_cast<size_t>(PlaneIndex::Top)], points[3]);

        mPlanes[static_cast<size_t>(PlaneIndex::Far)].intersects(mPlanes[static_cast<size_t>(PlaneIndex::Left)], mPlanes[static_cast<size_t>(PlaneIndex::Bottom)], points[4]);
        mPlanes[static_cast<size_t>(PlaneIndex::Far)].intersects(mPlanes[static_cast<size_t>(PlaneIndex::Right)], mPlanes[static_cast<size_t>(PlaneIndex::Bottom)], points[5]);
        mPlanes[static_cast<size_t>(PlaneIndex::Far)].intersects(mPlanes[static_cast<size_t>(PlaneIndex::Right)], mPlanes[static_cast<size_t>(PlaneIndex::Top)], points[6]);
        mPlanes[static_cast<size_t>(PlaneIndex::Far)].intersects(mPlanes[static_cast<size_t>(PlaneIndex::Left)], mPlanes[static_cast<size_t>(PlaneIndex::Top)], points[7]);
    }

    Frustum::IntersectionType Frustum::classifyFrustum(const Frustum& frustum) const
    {
        hr::Vector3f corners[8];
        frustum.getCorners(corners);

        {
            auto inside = false;
            auto outside = false;
            for (size_t i = 0; i < 8; i++)
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

        for (size_t i = 0; i < 6; i++)
        {
            if (mPlanes[i].classifyPoint(corners[0]) != hr::Plane<float>::Position::Behind)
                continue;
            if (mPlanes[i].classifyPoint(corners[1]) != hr::Plane<float>::Position::Behind)
                continue;
            if (mPlanes[i].classifyPoint(corners[2]) != hr::Plane<float>::Position::Behind)
                continue;
            if (mPlanes[i].classifyPoint(corners[3]) != hr::Plane<float>::Position::Behind)
                continue;
            if (mPlanes[i].classifyPoint(corners[4]) != hr::Plane<float>::Position::Behind)
                continue;
            if (mPlanes[i].classifyPoint(corners[5]) != hr::Plane<float>::Position::Behind)
                continue;
            if (mPlanes[i].classifyPoint(corners[6]) != hr::Plane<float>::Position::Behind)
                continue;
            if (mPlanes[i].classifyPoint(corners[7]) != hr::Plane<float>::Position::Behind)
                continue;

            return IntersectionType::FullOutside;
        }

        // now invert the test in relation to the other frustum

        getCorners(corners);

        for (size_t i = 0; i < 6; i++)
        {
            if (frustum.mPlanes[i].classifyPoint(corners[0]) != hr::Plane<float>::Position::Behind)
                continue;
            if (frustum.mPlanes[i].classifyPoint(corners[1]) != hr::Plane<float>::Position::Behind)
                continue;
            if (frustum.mPlanes[i].classifyPoint(corners[2]) != hr::Plane<float>::Position::Behind)
                continue;
            if (frustum.mPlanes[i].classifyPoint(corners[3]) != hr::Plane<float>::Position::Behind)
                continue;
            if (frustum.mPlanes[i].classifyPoint(corners[4]) != hr::Plane<float>::Position::Behind)
                continue;
            if (frustum.mPlanes[i].classifyPoint(corners[5]) != hr::Plane<float>::Position::Behind)
                continue;
            if (frustum.mPlanes[i].classifyPoint(corners[6]) != hr::Plane<float>::Position::Behind)
                continue;
            if (frustum.mPlanes[i].classifyPoint(corners[7]) != hr::Plane<float>::Position::Behind)
                continue;

            return IntersectionType::FullOutside;
        }

        return IntersectionType::FrustumIntersect;
    }

    float Frustum::dotNormals(const PlaneIndex planeA, const PlaneIndex planeB) const
    {
        auto pa = static_cast<size_t>(planeA);
        auto pb = static_cast<size_t>(planeB);

        if ((pa > 5) || (pb > 5))
            return 0.0f;

        auto normalB = mPlanes[pb].normal();
        return mPlanes[pa].dotNormal(normalB);
    }

    void Frustum::calculateFrustum(const hr::Matrix4f& transformation)
    {
        hr::Vector4f col1 = transformation.getColumn(0);
        hr::Vector4f col2 = transformation.getColumn(1);
        hr::Vector4f col3 = transformation.getColumn(2);
        hr::Vector4f col4 = transformation.getColumn(3);

        extractPlanes(col1, col2, col3, col4);
    }

    void Frustum::calculateFrustum(const hr::Matrix4f& projection, const hr::Matrix4f& modelview)
    {
        auto matTrans = projection * modelview;
        hr::Vector4f col1 = matTrans.getColumn(0);
        hr::Vector4f col2 = matTrans.getColumn(1);
        hr::Vector4f col3 = matTrans.getColumn(2);
        hr::Vector4f col4 = matTrans.getColumn(3);

        extractPlanes(col1, col2, col3, col4);
    }

    void Frustum::calculateFrustum(const hr::Matrix4f& modelView, const hr::Matrix4f& projection, const hr::Vector3f& pos, float zNear, float zFar)
    {
        mPosition = pos;
        mZNear = zNear;
        mZFar = zFar;

        auto fClip = projection * modelView;
        hr::Vector4f col1 = fClip.getColumn(0);
        hr::Vector4f col2 = fClip.getColumn(1);
        hr::Vector4f col3 = fClip.getColumn(2);
        hr::Vector4f col4 = fClip.getColumn(3);

        extractPlanes(col1, col2, col3, col4);
    }

    void Frustum::setIndividualPlane(PlaneIndex planeIndex, const hr::Plane<float>& plane)
    {
        if (static_cast<size_t>(planeIndex) > 5)
            return;
        mPlanes[static_cast<size_t>(planeIndex)] = plane;
    }

    void Frustum::setFrustum(const hr::Vector3f& bboxMin, const hr::Vector3f& bboxMax)
    {
        mPlanes[static_cast<size_t>(PlaneIndex::Left)] = hr::Plane<float>{1.0f, 0.0f, 0.0f, -bboxMin[0]};
        mPlanes[static_cast<size_t>(PlaneIndex::Right)] = hr::Plane<float>{-1.0f, 0.0f, 0.0f, bboxMax[0]};
        mPlanes[static_cast<size_t>(PlaneIndex::Top)] = hr::Plane<float>{0.0f, -1.0f, 0.0f, bboxMax[1]};
        mPlanes[static_cast<size_t>(PlaneIndex::Bottom)] = hr::Plane<float>{0.0f, 1.0f, 0.0f, -bboxMin[1]};
        mPlanes[static_cast<size_t>(PlaneIndex::Near)] = hr::Plane<float>{0.0f, 0.0f, -1.0f, bboxMax[2]};
        mPlanes[static_cast<size_t>(PlaneIndex::Far)] = hr::Plane<float>{0.0f, 0.0f, 1.0f, -bboxMin[2]};
    }

    void Frustum::setFrustum(const hr::Vector3f& center, const float radius)
    {
        mPlanes[static_cast<size_t>(PlaneIndex::Left)] = hr::Plane<float>{1.0f, 0.0f, 0.0f, -(center[0] - radius)};
        mPlanes[static_cast<size_t>(PlaneIndex::Right)] = hr::Plane<float>{-1.0f, 0.0f, 0.0f, center[0] + radius};
        mPlanes[static_cast<size_t>(PlaneIndex::Top)] = hr::Plane<float>{0.0f, -1.0f, 0.0f, center[1] + radius};
        mPlanes[static_cast<size_t>(PlaneIndex::Bottom)] = hr::Plane<float>{0.0f, 1.0f, 0.0f, -(center[1] - radius)};
        mPlanes[static_cast<size_t>(PlaneIndex::Near)] = hr::Plane<float>{0.0f, 0.0f, -1.0f, center[2] + radius};
        mPlanes[static_cast<size_t>(PlaneIndex::Far)] = hr::Plane<float>{0.0f, 0.0f, 1.0f, -(center[2] - radius)};
    }

    void Frustum::setFrustum(const hr::BBox<>& bbox)
    {
        setFrustum(bbox.min(), bbox.max());
    }

    bool Frustum::testCube(const hr::Vector3f& point, const float& size) const
    {
        hr::Vector3f pCubo[8];

        pCubo[0] = point;
        pCubo[1] = pCubo[0];
        pCubo[1][2] += size;
        pCubo[2] = pCubo[1];
        pCubo[2][1] += size;
        pCubo[3] = pCubo[0];
        pCubo[3][1] += size;
        pCubo[4] = pCubo[0];
        pCubo[4][0] += size;
        pCubo[5] = pCubo[4];
        pCubo[5][2] += size;
        pCubo[6] = pCubo[5];
        pCubo[6][1] += size;
        pCubo[7] = pCubo[4];
        pCubo[7][1] += size;

        for (int iCurPlane = 0; iCurPlane < 6; iCurPlane++)
        {
            if (mPlanes[iCurPlane].distance(pCubo[0]) > 0.0f)
                continue;
            if (mPlanes[iCurPlane].distance(pCubo[1]) > 0.0f)
                continue;
            if (mPlanes[iCurPlane].distance(pCubo[2]) > 0.0f)
                continue;
            if (mPlanes[iCurPlane].distance(pCubo[3]) > 0.0f)
                continue;
            if (mPlanes[iCurPlane].distance(pCubo[4]) > 0.0f)
                continue;
            if (mPlanes[iCurPlane].distance(pCubo[5]) > 0.0f)
                continue;
            if (mPlanes[iCurPlane].distance(pCubo[6]) > 0.0f)
                continue;
            if (mPlanes[iCurPlane].distance(pCubo[7]) > 0.0f)
                continue;

            return false;
        }

        return true;
    }

    bool Frustum::testBox(const hr::Vector3f& min, const hr::Vector3f& max) const
    {
        hr::Vector3f pBox[8];

        pBox[0] = min;
        pBox[1] = min;
        pBox[1][2] = max[2];
        pBox[2] = min;
        pBox[2][1] = max[1];
        pBox[3] = max;
        pBox[3][0] = min[0];

        pBox[4] = min;
        pBox[4][0] = max[0];
        pBox[5] = max;
        pBox[5][1] = min[1];
        pBox[6] = max;
        pBox[6][2] = min[2];
        pBox[7] = max;

        for (auto iPlane = 0; iPlane < 6; iPlane++)
        {
            if (mPlanes[iPlane].distance(pBox[0]) > 0.0f)
                continue;
            if (mPlanes[iPlane].distance(pBox[1]) > 0.0f)
                continue;
            if (mPlanes[iPlane].distance(pBox[2]) > 0.0f)
                continue;
            if (mPlanes[iPlane].distance(pBox[3]) > 0.0f)
                continue;
            if (mPlanes[iPlane].distance(pBox[4]) > 0.0f)
                continue;
            if (mPlanes[iPlane].distance(pBox[5]) > 0.0f)
                continue;
            if (mPlanes[iPlane].distance(pBox[6]) > 0.0f)
                continue;
            if (mPlanes[iPlane].distance(pBox[7]) > 0.0f)
                continue;

            return false;
        }

        return true;
    }

    bool Frustum::testBox(const hr::BBox<>& bbox) const
    {
        return testBox(bbox.min(), bbox.max());
    }

    bool Frustum::testSphere(const hr::Vector3f& center, const float& radius) const
    {
        for (auto iPlane = 0; iPlane < 6; iPlane++)
        {
            if (mPlanes[iPlane].distance(center) <= -radius)
                return false;
        }

        return true;
    }

    bool Frustum::testSphere(const hr::BSphere<hr::Vector3f>& bsphere) const
    {
        return testSphere(bsphere.center(), bsphere.radius());
    }

    bool Frustum::testSphereBox(const hr::BSphere<hr::Vector3f>& bsphere, const hr::BBox<>& bbox) const
    {
        if (!testSphere(bsphere))
            return false;
        return testBox(bbox);
    }

    bool Frustum::testPoint(const hr::Vector3f& point) const
    {
        for (size_t iPlane = 0; iPlane < 6; iPlane++)
        {
            if (mPlanes[iPlane].distance(point) <= 0.0f)
                return false;
        }

        return true;
    }

    bool Frustum::testPolygon(const hr::Vector3f* const points, size_t numPoints) const
    {
        for (size_t iPlane = 0; iPlane < 6; iPlane++)
        {
            size_t j;
            for (j = 0; j < numPoints; j++)
            {
                if (mPlanes[iPlane].distance(points[j]) > 0.0f)
                    break;
            }

            if (j == numPoints)
                return false;
        }

        return true;
    }

    bool Frustum::testSquare(const hr::Vector3f points[4]) const
    {
        for (int iPlane = 0; iPlane < 6; iPlane++)
        {
            if (mPlanes[iPlane].distance(points[0]) > 0.0f)
                continue;
            if (mPlanes[iPlane].distance(points[1]) > 0.0f)
                continue;
            if (mPlanes[iPlane].distance(points[2]) > 0.0f)
                continue;
            if (mPlanes[iPlane].distance(points[3]) > 0.0f)
                continue;

            return false;
        }

        return true;
    }

    bool Frustum::testTri(const hr::Vector3f points[3]) const
    {
        for (int iPlane = 0; iPlane < 6; iPlane++)
        {
            if (mPlanes[iPlane].distance(points[0]) > 0.0f)
                continue;
            if (mPlanes[iPlane].distance(points[1]) > 0.0f)
                continue;
            if (mPlanes[iPlane].distance(points[2]) > 0.0f)
                continue;

            return false;
        }

        return true;
    }

    bool Frustum::testSweptSphere(const hr::Vector3f& sphereCenter, const float& sphereRadius, const hr::Vector3f& sweepDir) const
    {
        float displacements[12];
        hr::Vector3f auxCenter;

        size_t cnt = 0;
        for (size_t iPlane = 0; iPlane < 6; iPlane++)
        {
            float a, b;
            if (sweptSpherePlaneIntersect(a, b, mPlanes[iPlane], sphereCenter, sphereRadius, sweepDir))
            {
                if (a >= 0.f)
                    displacements[cnt++] = a;
                if (b >= 0.f)
                    displacements[cnt++] = b;
            }
        }

        bool inFrustum = false;
        for (size_t i = 0; i < cnt; i++)
        {
            auxCenter[0] = (sweepDir[0] * displacements[i]) + sphereCenter[0];
            auxCenter[1] = (sweepDir[1] * displacements[i]) + sphereCenter[1];
            auxCenter[2] = (sweepDir[2] * displacements[i]) + sphereCenter[2];

            inFrustum |= testSphere(auxCenter, sphereRadius * 1.1f);
        }

        return inFrustum;
    }
}
