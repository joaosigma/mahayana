#pragma once

#include "common/bvolumes.hpp"
#include "common/matrix.hpp"
#include "common/plane.hpp"
#include "common/vector.hpp"

#include <array>

namespace hr::gl::tools
{
    class Frustum
    {
    public:
        enum class IntersectionType
        {
            FullInside,
            FullOutside,
            FrustumIntersect
        };
        enum class PlaneIndex : size_t
        {
            Left = 0,
            Right = 1,
            Top = 2,
            Bottom = 3,
            Near = 4,
            Far = 5
        };

    private:
        std::array<hr::Plane<float>, 6> mPlanes{hr::Plane<float>::zero(), hr::Plane<float>::zero(), hr::Plane<float>::zero(),
                                                hr::Plane<float>::zero(), hr::Plane<float>::zero(), hr::Plane<float>::zero()};
        hr::Vector3f mPosition;
        float mZNear{0.0f}, mZFar{0.0f};

        void extractPlanes(const hr::Vector4f& col1, const hr::Vector4f& col2, const hr::Vector4f& col3, const hr::Vector4f& col4);
        bool sweptSpherePlaneIntersect(float& t0,
                                       float& t1,
                                       const hr::Plane<float>& plane,
                                       const hr::Vector3f& sphereCenter,
                                       const float& sphereRadius,
                                       const hr::Vector3f& sweepDir) const;

    public:
        Frustum() = default;

        bool testCube(const hr::Vector3f& point, const float& size) const;
        bool testBox(const hr::Vector3f& min, const hr::Vector3f& max) const;
        bool testBox(const hr::BBox<>& bbox) const;
        bool testSphere(const hr::Vector3f& center, const float& radius) const;
        bool testSphere(const hr::BSphere<hr::Vector3f>& bsphere) const;
        bool testSphereBox(const hr::BSphere<hr::Vector3f>& bsphere, const hr::BBox<>& bbox) const;
        bool testPoint(const hr::Vector3f& point) const;
        bool testPolygon(const hr::Vector3f* const points, size_t numPoints) const;
        bool testSquare(const hr::Vector3f points[4]) const;
        bool testTri(const hr::Vector3f points[3]) const;
        bool testSweptSphere(const hr::Vector3f& sphereCenter, const float& sphereRadius, const hr::Vector3f& sweepDir) const;

        float getZNear(void) const
        {
            return mZNear;
        }

        float getZFar(void) const
        {
            return mZFar;
        }

        const hr::Vector3f& getCamPosition(void) const
        {
            return mPosition;
        }

        const hr::Plane<float>& getPlane(PlaneIndex planeIndex) const
        {
            return mPlanes[static_cast<size_t>(planeIndex)];
        }

        void getCorners(hr::Vector3f points[8]) const;

        IntersectionType classifyFrustum(const Frustum& frustum) const;

        float dotNormals(const PlaneIndex planeA, const PlaneIndex planeB) const;

        void calculateFrustum(const hr::Matrix4f& transformation);
        void calculateFrustum(const hr::Matrix4f& projection, const hr::Matrix4f& modelview);
        void calculateFrustum(const hr::Matrix4f& modelView, const hr::Matrix4f& projection, const hr::Vector3f& pos, float zNear, float zFar);

        void setIndividualPlane(const PlaneIndex planeIndex, const hr::Plane<float>& plane);
        void setFrustum(const hr::Vector3f& bboxMin, const hr::Vector3f& bboxMax);
        void setFrustum(const hr::Vector3f& center, const float radius);
        void setFrustum(const hr::BBox<>& bbox);

        void setZNear(const float ZNear)
        {
            mZNear = ZNear;
        }

        void setZFar(const float ZFar)
        {
            mZFar = ZFar;
        }

        void setCamPosition(const float x, const float y, const float z)
        {
            mPosition = Vector3f{x, y, z};
        }

        void setCamPosition(const hr::Vector3f& pos)
        {
            mPosition = pos;
        }
    };
}
