#include "bareas.hpp"

#include <type_traits>

namespace hr
{
    static_assert(std::is_trivially_copyable_v<BRect>);

    BRect::BRect(std::span<const Vector3f> points) noexcept
    {
        merge(points);
    }

    BRect::BRect(std::span<const BRect> brects) noexcept
    {
        for (const auto& brect : brects)
        {
            merge(brect.mMinX, brect.mMinY);
            merge(brect.mMaxX, brect.mMaxY);
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
    void BRect::corners(std::span<Vector3f, 4> points) const noexcept
    {
        points[0] = Vector3f{mMinX, mMinY, 0.0f};
        points[1] = Vector3f{mMaxX, mMinY, 0.0f};
        points[2] = Vector3f{mMaxX, mMaxY, 0.0f};
        points[3] = Vector3f{mMinX, mMaxY, 0.0f};
    }

    void BRect::merge(const Vector3f& pt) noexcept
    {
        mMinX = std::fmin(mMinX, pt[0]);
        mMinY = std::fmin(mMinY, pt[1]);
        mMaxX = std::fmax(mMaxX, pt[0]);
        mMaxY = std::fmax(mMaxY, pt[1]);
    }

    void BRect::merge(std::span<const Vector3f> points) noexcept
    {
        for (const auto& p : points)
        {
            mMinX = std::fmin(mMinX, p[0]);
            mMinY = std::fmin(mMinY, p[1]);
            mMaxX = std::fmax(mMaxX, p[0]);
            mMaxY = std::fmax(mMaxY, p[1]);
        }
    }

    void BRect::merge(float x, float y) noexcept
    {
        mMinX = std::fmin(mMinX, x);
        mMinY = std::fmin(mMinY, y);
        mMaxX = std::fmax(mMaxX, x);
        mMaxY = std::fmax(mMaxY, y);
    }

    void BRect::translate(const Vector3f& translation) noexcept
    {
        mMinX += translation[0];
        mMinY += translation[1];
        mMaxX += translation[0];
        mMaxY += translation[1];
    }

    void BRect::expand(float amountX, float amountY) noexcept
    {
        mMinX -= amountX;
        mMinY -= amountY;
        mMaxX += amountX;
        mMaxY += amountY;
    }

    std::optional<BRect> BRect::crossSection(const BRect& brect) const noexcept
    {
        BRect res{std::fmax(mMinX, brect.mMinX), std::fmax(mMinY, brect.mMinY), std::fmin(mMaxX, brect.mMaxX), std::fmin(mMaxY, brect.mMaxY)};

        if ((res.mMinX <= res.mMaxX) && (res.mMinY <= res.mMaxY))
            return res;

        return {};
    }
}
