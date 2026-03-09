#pragma once

#include "math.hpp"
#include "plane.hpp"
#include "ray.hpp"
#include "vector.hpp"

#include <limits>
#include <span>
#include <type_traits>

namespace hr
{
    namespace details
    {
        template<typename T>
        concept ValidVectorTypes = std::is_same_v<T, Vector3f> || std::is_same_v<T, Vector3d>;
    }

    template<details::ValidVectorTypes TVectorType>
    class BBox;
    template<details::ValidVectorTypes TVectorType>
    class BSphere;

    template<details::ValidVectorTypes TVectorType = Vector3f>
    class BBox final
    {
        static_assert(std::is_trivially_copyable_v<TVectorType>, "For performance reasons, the vector type should be trivially copyable");

        TVectorType mMinPt, mMaxPt;

    public:
        enum class Position
        {
            Inside,
            Outside,
            Intersect
        };

        constexpr BBox() noexcept
          : mMinPt{std::numeric_limits<TVectorType::DataType>::infinity()}, mMaxPt{-std::numeric_limits<TVectorType::DataType>::infinity()}
        {}

        constexpr BBox(const BBox&) = default;
        constexpr BBox& operator=(const BBox&) = default;
        constexpr BBox(BBox&&) = default;
        constexpr BBox& operator=(BBox&&) = default;
        constexpr ~BBox() = default;

        explicit constexpr BBox(const TVectorType& minPoint, const TVectorType& maxPoint) noexcept
          : mMinPt{minPoint}, mMaxPt{maxPoint}
        {}

        explicit constexpr BBox(std::span<const TVectorType> points) noexcept
        {
            merge(points);
        }

        explicit constexpr BBox(std::span<const BBox> bboxes) noexcept
        {
            for (const auto& box : bboxes)
                merge(box);
        }

        void operator+=(const BBox& bbox) noexcept
        {
            merge(bbox);
        }

        void operator+=(const TVectorType& pt) noexcept
        {
            merge(pt);
        }

        const TVectorType& min() const noexcept
        {
            return mMinPt;
        }

        const TVectorType& max() const noexcept
        {
            return mMaxPt;
        }

        TVectorType center() const noexcept
        {
            return ((mMinPt + mMaxPt) * 0.5);
        }

        TVectorType dims() const noexcept
        {
            return (mMaxPt - mMinPt);
        }

        typename TVectorType::DataType surfaceArea() const noexcept
        {
            auto dims = mMaxPt - mMinPt;
            return 2.0 * ((dims[0] * dims[1]) + (dims[0] * dims[2]) + (dims[1] * dims[2]));
        }

        typename TVectorType::DataType radius() const noexcept
        {
            return center().distance(mMinPt);
        }

        typename TVectorType::DataType radiusMinimum() const noexcept
        {
            auto center = center();

            auto minDist = std::abs(mMinPt[0] - center[0]);
            minDist = std::fmin(minDist, std::abs(mMinPt[1] - center[1]));
            return std::fmin(minDist, std::abs(mMinPt[2] - center[2]));
        }

        typename TVectorType::DataType volume() const noexcept
        {
            auto dims = mMaxPt - mMinPt;
            return (dims[0] * dims[1] * dims[2]);
        }

        //
        //	2---------------6
        //	|\				|
        //	| \				|\
        //	|  \  back		| \
        //	|	\			|  \
        //	|	 \			|   \
        //	0-----\---------4	 \
        //	 \	   \		 \	  \
        //	  \     \		  \	   \
        //	   \    3---------------7
        //		\   |			\   |
        //		 \  |			 \  |
        //		  \ |	  front	  \ |
        //		   \|			   \|
        //			1---------------5
        //
        void corners(TVectorType points[8]) const noexcept
        {
            points[0] = TVectorType{mMinPt[0], mMinPt[1], mMinPt[2]};
            points[1] = TVectorType{mMinPt[0], mMinPt[1], mMaxPt[2]};
            points[2] = TVectorType{mMinPt[0], mMaxPt[1], mMinPt[2]};
            points[3] = TVectorType{mMinPt[0], mMaxPt[1], mMaxPt[2]};
            points[4] = TVectorType{mMaxPt[0], mMinPt[1], mMinPt[2]};
            points[5] = TVectorType{mMaxPt[0], mMinPt[1], mMaxPt[2]};
            points[6] = TVectorType{mMaxPt[0], mMaxPt[1], mMinPt[2]};
            points[7] = TVectorType{mMaxPt[0], mMaxPt[1], mMaxPt[2]};
        }

        BSphere<TVectorType> boundingSphere() const noexcept
        {
            auto scenter = center();
            return BSphere<TVectorType>(scenter, scenter.distance(mMinPt));
        }

        typename TVectorType::DataType planeDistance(const Plane<TVectorType>& plane) const noexcept
        {
            auto center = center();

            auto d1 = plane.dotCoord(center);
            TVectorType planeNormal = plane.normal();

            auto d2 = std::abs((mMaxPt[0] - center[0]) * planeNormal[0]);
            d2 += std::abs((mMaxPt[1] - center[1]) * planeNormal[1]);
            d2 += std::abs((mMaxPt[2] - center[2]) * planeNormal[2]);

            if ((d1 - d2) > 0.0)
                return (d1 - d2);
            if ((d1 + d2) < 0.0)
                return (d1 + d2);
            return 0.0f;
        }

        TVectorType mainAxis() const noexcept
        {
            auto diff = mMaxPt - mMinPt;

            if (diff[0] > diff[1] && diff[0] > diff[2])
                return TVectorType(1.0, 0.0, 0.0);
            if (diff[1] > diff[0] && diff[1] > diff[2])
                return TVectorType(0.0, 1.0, 0.0);
            if (diff[2] > diff[0] && diff[2] > diff[1])
                return TVectorType(0.0, 0.0, 1.0);

            if (Math::isZero(diff[0] - diff[1]) && Math::isZero(diff[1] - diff[2]))
                return TVectorType(1.0, 1.0, 1.0);
            if (Math::isZero(diff[0] - diff[1]))
                return TVectorType(1.0, 1.0, 0.0);
            if (Math::isZero(diff[0] - diff[2]))
                return TVectorType(1.0, 0.0, 1.0);

            return TVectorType(0.0, 1.0, 1.0);
        }

        template<typename TTargetVectorType>
        BBox<TTargetVectorType> convert() const noexcept
        {
            return BBox<TTargetVectorType>{mMinPt.convert<TTargetVectorType::DataType>(), mMaxPt.convert<TTargetVectorType::DataType>()};
        }

        void merge(const TVectorType& pt) noexcept
        {
            if constexpr (std::is_same_v<TVectorType, Vector3f>)
            {
                __m128 vecData;

                vecData = _mm_loadu_ps(pt.data().data());
                _mm_storeu_ps(mMinPt.data().data(), _mm_min_ps(_mm_loadu_ps(mMinPt.data().data()), vecData));
                _mm_storeu_ps(mMaxPt.data().data(), _mm_max_ps(_mm_loadu_ps(mMaxPt.data().data()), vecData));
            }
            else if constexpr (std::is_same_v<TVectorType, Vector3d>)
            {
                __m256d vecData;

                vecData = _mm256_loadu_pd(pt.data().data());
                _mm256_storeu_pd(mMinPt.data().data(), _mm256_min_pd(_mm256_loadu_pd(mMinPt.data().data()), vecData));
                _mm256_storeu_pd(mMaxPt.data().data(), _mm256_max_pd(_mm256_loadu_pd(mMaxPt.data().data()), vecData));
            }

            static_assert(std::is_same_v<TVectorType, Vector3f> || std::is_same_v<TVectorType, Vector3d>, "Base type must be either Vector3f or Vector3d");
        }

        void merge(std::span<const TVectorType> points) noexcept
        {
            if constexpr (std::is_same_v<TVectorType, Vector3f>)
            {
                __m128 vec1, vec2, finalMin, finalMax;
                size_t i, j;

                finalMin = _mm_loadu_ps(mMinPt.data().data());
                finalMax = _mm_loadu_ps(mMaxPt.data().data());

                for (i = points.size(), j = 0; i >= 2; i -= 2, j += 2)
                {
                    vec1 = _mm_loadu_ps(points[j].data().data());
                    vec2 = _mm_loadu_ps(points[j + 1].data().data());
                    finalMin = _mm_min_ps(finalMin, vec1);
                    finalMax = _mm_max_ps(finalMax, vec1);
                    finalMin = _mm_min_ps(finalMin, vec2);
                    finalMax = _mm_max_ps(finalMax, vec2);
                }

                if (i > 0)
                {
                    vec1 = _mm_loadu_ps(points[j].data().data());
                    finalMin = _mm_min_ps(finalMin, vec1);
                    finalMax = _mm_max_ps(finalMax, vec1);
                }

                _mm_storeu_ps(mMinPt.data().data(), finalMin);
                _mm_storeu_ps(mMaxPt.data().data(), finalMax);
            }
            else if constexpr (std::is_same_v<TVectorType, Vector3d>)
            {
                __m256d vec1, vec2, finalMin, finalMax;
                size_t i, j;

                finalMin = _mm256_loadu_pd(mMinPt.data().data());
                finalMax = _mm256_loadu_pd(mMaxPt.data().data());

                for (i = points.size(), j = 0; i >= 2; i -= 2, j += 2)
                {
                    vec1 = _mm256_loadu_pd(points[j].data().data());
                    vec2 = _mm256_loadu_pd(points[j + 1].data().data());
                    finalMin = _mm256_min_pd(finalMin, vec1);
                    finalMax = _mm256_max_pd(finalMax, vec1);
                    finalMin = _mm256_min_pd(finalMin, vec2);
                    finalMax = _mm256_max_pd(finalMax, vec2);
                }

                if (i > 0)
                {
                    vec1 = _mm256_loadu_pd(points[j].data().data());
                    finalMin = _mm256_min_pd(finalMin, vec1);
                    finalMax = _mm256_max_pd(finalMax, vec1);
                }

                _mm256_storeu_pd(mMinPt.data().data(), finalMin);
                _mm256_storeu_pd(mMaxPt.data().data(), finalMax);
            }

            static_assert(std::is_same_v<TVectorType, Vector3f> || std::is_same_v<TVectorType, Vector3d>, "Base type must be either Vector3f or Vector3d");
        }

        void merge(const BBox& box) noexcept
        {
            merge(box.mMinPt);
            merge(box.mMaxPt);
        }

        void merge(const TVectorType& sphereCenter, const typename TVectorType::DataType sphereRadius) noexcept
        {
            if constexpr (std::is_same_v<TVectorType, Vector3f>)
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
            else if constexpr (std::is_same_v<TVectorType, Vector3d>)
            {
                __m256d smin, smax, aux, finalMin, finalMax;

                smin = smax = _mm256_loadu_pd(sphereCenter.data());
                aux = _mm256_set1_pd(sphereRadius);
                smin = _mm256_sub_pd(smin, aux);
                smax = _mm256_add_pd(smax, aux);

                finalMin = _mm256_loadu_pd(mMinPt.data());
                finalMax = _mm256_loadu_pd(mMaxPt.data());
                finalMin = _mm256_min_pd(finalMin, smin);
                finalMax = _mm256_max_pd(finalMax, smin);
                _mm_storeu_ps(mMinPt.data(), _mm256_min_pd(finalMin, smax));
                _mm_storeu_ps(mMaxPt.data(), _mm256_max_pd(finalMax, smax));
            }

            static_assert(std::is_same_v<TVectorType, Vector3f> || std::is_same_v<TVectorType, Vector3d>, "Base type must be either Vector3f or Vector3d");
        }

        void merge(const BSphere<TVectorType>& bsphere) noexcept
        {
            merge(bsphere.center(), bsphere.radius());
        }

        template<class TElement, class... TElements>
        void mergeAll(const TElement& element, const TElements&... elements) noexcept
        {
            static_assert(std::is_same_v<TElement, TVectorType> || std::is_same_v<TElement, BBox> || std::is_same_v<TElement, BSphere<TVectorType>>,
                          "Only certain types are permitted.");

            merge(element);
            if constexpr (sizeof...(TElements) > 0)
                mergeAll(elements...);
        }

        void setMin(const TVectorType& min) noexcept
        {
            mMinPt = min;
        }

        void setMax(const TVectorType& max) noexcept
        {
            mMaxPt = max;
        }

        void setMinMax(const typename TVectorType::DataType min[3], const typename TVectorType::DataType max[3]) noexcept
        {
            mMinPt = min;
            mMaxPt = max;
        }

        void setMinMax(const TVectorType& min, const TVectorType& max) noexcept
        {
            mMinPt = min;
            mMaxPt = max;
        }

        void reset() noexcept
        {
            mMinPt[0] = mMinPt[1] = mMinPt[2] = std::numeric_limits<TVectorType::DataType>::infinity();
            mMaxPt[0] = mMaxPt[1] = mMaxPt[2] = -std::numeric_limits<TVectorType::DataType>::infinity();
        }

        void translate(const TVectorType& translation) noexcept
        {
            mMinPt += translation;
            mMaxPt += translation;
        }

        void expand(const typename TVectorType::DataType amount) noexcept
        {
            mMinPt -= amount;
            mMaxPt += amount;
        }

        bool containsPoint(const TVectorType& point) const noexcept
        {
            return (point[0] >= mMinPt[0]) && (point[1] >= mMinPt[1]) && (point[2] >= mMinPt[2]) && (point[0] <= mMaxPt[0]) && (point[1] <= mMaxPt[1]) && (point[2] <= mMaxPt[2]);
        }

        bool intersects(const BBox& bbox) const noexcept
        {
            return (bbox.mMaxPt[0] >= mMinPt[0]) && (bbox.mMaxPt[1] >= mMinPt[1]) && (bbox.mMaxPt[2] >= mMinPt[2]) && (bbox.mMinPt[0] <= mMaxPt[0]) &&
                   (bbox.mMinPt[1] <= mMaxPt[1]) && (bbox.mMinPt[2] <= mMaxPt[2]);
        }

        bool intersects(const TVectorType& lineStart, const TVectorType& lineEnd) const noexcept
        {
            Ray<TVectorType> ray{lineStart, TVectorType::calcNormalize(lineEnd - lineStart)};
            return intersects(ray, -std::numeric_limits<TVectorType::DataType>::infinity(), TVectorType::calcDistance(lineStart, lineEnd));
        }

        bool intersects(const Ray<TVectorType>& ray) const noexcept
        {
            auto hitDistMin = -std::numeric_limits<TVectorType::DataType>::infinity();
            auto hitDistMax = std::numeric_limits<TVectorType::DataType>::infinity();
            return intersects(ray, hitDistMin, hitDistMax);
        }

        bool intersects(const Ray<TVectorType>& ray, typename TVectorType::DataType& hitDistanceMin, typename TVectorType::DataType& hitDistanceMax) const noexcept
        {
            auto t1 = (mMinPt - ray.origin()) / ray.direction();
            auto t2 = (mMaxPt - ray.origin()) / ray.direction();

            auto [tmin, tmax] = TVectorType::calcMinMax(t1, t2);

            hitDistanceMin = std::max(hitDistanceMin, std::max(tmin[0], std::max(tmin[1], tmin[2])));
            hitDistanceMax = std::min(hitDistanceMax, std::min(tmax[0], std::min(tmax[1], tmax[2])));

            return (hitDistanceMin < hitDistanceMax);
        }

        Position classify(const BBox& bbox) noexcept
        {
            if ((bbox.mMinPt[0] > mMaxPt[0]) || (bbox.mMaxPt[0] < mMinPt[0]))
                return Position::Outside;
            if ((bbox.mMinPt[1] > mMaxPt[1]) || (bbox.mMaxPt[1] < mMinPt[1]))
                return Position::Outside;
            if ((bbox.mMinPt[2] > mMaxPt[2]) || (bbox.mMaxPt[2] < mMinPt[2]))
                return Position::Outside;

            if ((bbox.mMinPt[0] > mMinPt[0]) && (bbox.mMaxPt[0] < mMaxPt[0]) && (bbox.mMinPt[1] > mMinPt[1]) && (bbox.mMaxPt[1] < mMaxPt[1]) && (bbox.mMinPt[2] > mMinPt[2]) &&
                (bbox.mMaxPt[2] < mMaxPt[2]))
                return Position::Inside;

            return Position::Intersect;
        }
    };

    template<details::ValidVectorTypes TVectorType = Vector3f>
    class BSphere final
    {
        static_assert(std::is_trivially_copyable_v<TVectorType>, "For performance reasons, the vector type should be trivially copyable");

        typename TVectorType::DataType mCenter[3], mRadius;

    public:
        using DataType = typename TVectorType;

    private:
        typename TVectorType::DataType calcDist(const BSphere& bsphere) const
        {
            return TVectorType::calcDistance(TVectorType{mCenter[0], mCenter[1], mCenter[2]}, TVectorType{bsphere.mCenter[0], bsphere.mCenter[1], bsphere.mCenter[2]});
        }

        typename TVectorType::DataType calcDist(const TVectorType& vec) const
        {
            return TVectorType::calcDistance(TVectorType{mCenter[0], mCenter[1], mCenter[2]}, vec);
        }

    public:
        enum class Position
        {
            Inside,
            Outside,
            Intersect
        };

        constexpr BSphere() = default;

        explicit constexpr BSphere(const TVectorType& center, typename TVectorType::DataType radius) noexcept
          : mCenter{center[0], center[1], center[2]}, mRadius{radius}
        {}

        void operator+=(const BSphere& bsphere) noexcept
        {
            mRadius = std::fmax(mRadius, BSphere::calcDist(bsphere) + bsphere.mRadius);
        }

        typename TVectorType::DataType radius() const noexcept
        {
            return mRadius;
        }

        TVectorType center() const noexcept
        {
            return TVectorType(mCenter[0], mCenter[1], mCenter[2]);
        }

        void merge(const TVectorType& pt) noexcept
        {
            mRadius = std::fmax(mRadius, BSphere::calcDist(pt));
        }

        void merge(std::span<const TVectorType> pts) noexcept
        {
            for (const auto& pt : pts)
                mRadius = std::fmax(mRadius, BSphere::calcDist(pt));
        }

        void merge(const BSphere& sphere) noexcept
        {
            mRadius = std::fmax(mRadius, BSphere::calcDist(sphere) + sphere.mRadius);
        }

        void merge(const BBox<TVectorType>& bbox) noexcept
        {
            merge(bbox.min());
            merge(bbox.max());
        }

        void expand(typename TVectorType::DataType amount) noexcept
        {
            mRadius += amount;
        }

        void set(typename TVectorType::DataType x, typename TVectorType::DataType y, typename TVectorType::DataType z, typename TVectorType::DataType radius) noexcept
        {
            mCenter[0] = x;
            mCenter[1] = y;
            mCenter[2] = z;
            mRadius = radius;
        }

        void set(const TVectorType& center, typename TVectorType::DataType radius) noexcept
        {
            mCenter[0] = center[0];
            mCenter[1] = center[1];
            mCenter[2] = center[2];
            mRadius = radius;
        }

        void set(const BBox<TVectorType>& bbox) noexcept
        {
            *this = bbox.boundingSphere();
        }

        void setCenter(typename TVectorType::DataType x, typename TVectorType::DataType y, typename TVectorType::DataType z) noexcept
        {
            mCenter[0] = x;
            mCenter[1] = y;
            mCenter[2] = z;
        }

        void setCenter(const TVectorType& center) noexcept
        {
            mCenter[0] = center[0];
            mCenter[1] = center[1];
            mCenter[2] = center[2];
        }

        void setRadius(typename TVectorType::DataType radius) noexcept
        {
            mRadius = radius;
        }

        void translate(const TVectorType& translation) noexcept
        {
            mCenter[0] += translation[0];
            mCenter[1] += translation[1];
            mCenter[2] += translation[2];
        }

        bool containsPoint(const TVectorType& point) const noexcept
        {
            return (BSphere::calcDist(point) < mRadius);
        }

        bool containsPoint(typename TVectorType::DataType x, typename TVectorType::DataType y, typename TVectorType::DataType z) const noexcept
        {
            return containsPoint(TVectorType{x, y, z});
        }

        bool intersects(const BSphere& bsphere) const noexcept
        {
            auto radiusSum = bsphere.mRadius + mRadius;
            auto vecDiff = Vector3f(bsphere.mCenter[0], bsphere.mCenter[1], bsphere.mCenter[2]) - Vector3f(mCenter[0], mCenter[1], mCenter[2]);

            return (vecDiff.getDot() <= (radiusSum * radiusSum));
        }

        bool intersects(const Ray<TVectorType>& ray) const noexcept
        {
            auto rayDir = ray.direction();

            auto p = ray.origin() - TVectorType(mCenter[0], mCenter[1], mCenter[2]);
            auto a = rayDir.getDot();
            auto b = rayDir.getDot(p);
            auto c = p.getDot() - (mRadius * mRadius);
            auto d = (b * b) - (c * a);

            return (d >= 0.0);
        }

        bool intersects(const Ray<TVectorType>& ray, typename TVectorType::DataType& hitDistance) const noexcept
        {
            auto rayDir = ray.direction();

            auto oc = ray.origin() - TVectorType(mCenter[0], mCenter[1], mCenter[2]);
            auto a = rayDir.getDot();
            auto b = rayDir.getDot(oc);
            auto c = oc.getDot() - (mRadius * mRadius);
            auto d = (b * b) - (c * a);

            if (d < 0.0)
                return false;

            hitDistance = (-b - Math::sqrt(d)) / a;
            return true;
        }

        bool intersects(const Ray<TVectorType>& ray,
                        const typename TVectorType::DataType rayDistMin,
                        const typename TVectorType::DataType rayDistMax,
                        typename TVectorType::DataType& hitDistance) const noexcept
        {
            auto oc = ray.origin() - TVectorType(mCenter[0], mCenter[1], mCenter[2]);
            auto a = ray.direction().dot();
            auto b = ray.direction().dot(oc);
            auto c = oc.dot() - (mRadius * mRadius);
            auto d = (b * b) - (c * a);

            if (d < 0.0)
                return false;

            d = Math::sqrt(d);
            a = 1.0 / a;

            hitDistance = (-b - d) * a;
            if ((hitDistance < rayDistMin) || (hitDistance >= rayDistMax))
            {
                hitDistance = (-b + d) * a;
                if ((hitDistance < rayDistMin) || (hitDistance >= rayDistMax))
                    return false;
            }

            return true;
        }

        Position classifyBSphere(const BSphere& bsphere) noexcept
        {
            auto centerDist = BSphere::calcDist(bsphere);

            if ((centerDist + bsphere.mRadius) < mRadius)
                return BSphere::Position::Inside;
            if (centerDist - bsphere.mRadius > mRadius)
                return BSphere::Position::Outside;

            return BSphere::Position::Intersect;
        }

        Position classifyBBox(const BBox<TVectorType>& bbox) noexcept
        {
            TVectorType pontos[8];
            size_t numIn, numOut;

            bbox.corners(pontos);
            numIn = numOut = 0;

            for (size_t i = 0; i < 4; i++)
                (BSphere::calcDist(pontos[i]) < mRadius) ? numIn++ : numOut++;

            if (numIn != 0 && numOut != 0)
                return BSphere::Position::Intersect;

            for (size_t i = 4; i < 8; i++)
                (BSphere::calcDist(pontos[i]) < mRadius) ? numIn++ : numOut++;

            if (numIn != 0 && numOut != 0)
                return BSphere::Position::Intersect;
            if (numIn == 0)
                return BSphere::Position::Outside;

            return BSphere::Position::Inside;
        }
    };
}
