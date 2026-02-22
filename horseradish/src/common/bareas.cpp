#include "bareas.hpp"

#include <type_traits>

namespace hr
{
    static_assert(std::is_trivially_copyable_v<BRect>);

    BRect::BRect(const Vector3f* const points, size_t numVec)
    {
        mMinX = mMinY = std::numeric_limits<float>::infinity();
        mMaxX = mMaxY = -std::numeric_limits<float>::infinity();

        merge(points, numVec);
    }

    BRect::BRect(const BRect* const brects, size_t numBRect)
    {
        mMinX = mMinY = std::numeric_limits<float>::infinity();
        mMaxX = mMaxY = -std::numeric_limits<float>::infinity();

        for (size_t i = 0; i < numBRect; i++)
        {
            merge(brects[i].mMinX, brects[i].mMinY);
            merge(brects[i].mMaxX, brects[i].mMaxY);
        }
    }

    /*
    3-------------2
    |			  |
    |			  |
    |	  front	  |
    |			  |
    0-------------1
    */
    void BRect::corners(Vector3f points[4]) const
    {
        points[0] = Vector3f{mMinX, mMinY, 0.0f};
        points[1] = Vector3f{mMaxX, mMinY, 0.0f};
        points[2] = Vector3f{mMaxX, mMaxY, 0.0f};
        points[3] = Vector3f{mMinX, mMaxY, 0.0f};
    }

    void BRect::merge(const Vector3f& pt)
    {
        mMinX = std::fmin(mMinX, pt[0]);
        mMinY = std::fmin(mMinY, pt[1]);
        mMaxX = std::fmax(mMaxX, pt[0]);
        mMaxY = std::fmax(mMaxY, pt[1]);
    }

    void BRect::merge(const float* const pt)
    {
        mMinX = std::fmin(mMinX, pt[0]);
        mMinY = std::fmin(mMinY, pt[1]);
        mMaxX = std::fmax(mMaxX, pt[0]);
        mMaxY = std::fmax(mMaxY, pt[1]);
    }

    void BRect::merge(const Vector3f* const pts, size_t numPts)
    {
        for (size_t i = 0; i < numPts; i++)
        {
            mMinX = std::fmin(mMinX, pts[i][0]);
            mMinY = std::fmin(mMinY, pts[i][1]);
            mMaxX = std::fmax(mMaxX, pts[i][0]);
            mMaxY = std::fmax(mMaxY, pts[i][1]);
        }
    }

    void BRect::merge(const float& x, const float& y)
    {
        mMinX = std::fmin(mMinX, x);
        mMinY = std::fmin(mMinY, y);
        mMaxX = std::fmax(mMaxX, x);
        mMaxY = std::fmax(mMaxY, y);
    }

    void BRect::translate(const Vector3f& translation)
    {
        mMinX += translation[0];
        mMinY += translation[1];
        mMaxX += translation[0];
        mMaxY += translation[1];
    }

    void BRect::expand(const float amount)
    {
        mMinX -= amount;
        mMinY -= amount;
        mMaxX += amount;
        mMaxY += amount;
    }

    void BRect::expand(const float amountX, const float amountY)
    {
        mMinX -= amountX;
        mMinY -= amountY;
        mMaxX += amountX;
        mMaxY += amountY;
    }

    void BRect::crossSection(const BRect& brect, BRect& brectResult) const
    {
        brectResult.mMinX = std::fmax(mMinX, brect.mMinX);
        brectResult.mMinY = std::fmax(mMinY, brect.mMinY);
        brectResult.mMaxX = std::fmin(mMaxX, brect.mMaxX);
        brectResult.mMaxY = std::fmin(mMaxY, brect.mMaxY);

        if ((brectResult.mMinX <= brectResult.mMaxX) && (brectResult.mMinY <= brectResult.mMaxY))
            return;

        brectResult.reset();
    }
}
