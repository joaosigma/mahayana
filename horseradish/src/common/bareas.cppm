export module core:bareas;

import std;
import :vector;

export namespace hr
{
    class BRect
    {
        float mMinX{std::numeric_limits<float>::infinity()}, mMinY{std::numeric_limits<float>::infinity()};
        float mMaxX{-std::numeric_limits<float>::infinity()}, mMaxY{-std::numeric_limits<float>::infinity()};

    public:
        constexpr BRect(float minX, float minY, float maxX, float maxY) noexcept
          : mMinX(minX), mMinY(minY), mMaxX(maxX), mMaxY(maxY)
        {}

        BRect(float centerX, float centerY, float expandAmount) noexcept
          : mMinX(centerX), mMinY(centerY), mMaxX(centerX), mMaxY(centerY)
        {
            expand(expandAmount);
        }

        explicit BRect(std::span<const Vector3f> points) noexcept;
        explicit BRect(std::span<const BRect> brects) noexcept;

        void operator+=(const BRect& brect) noexcept
        {
            merge(brect.mMaxX, brect.mMaxY);
            merge(brect.mMinX, brect.mMinY);
        }

        void operator+=(const Vector3f& pt) noexcept
        {
            merge(pt[0], pt[1]);
        }

        Vector3f min() const noexcept
        {
            return Vector3f{mMinX, mMinY, 0.0f};
        }

        Vector3f max() const noexcept
        {
            return Vector3f{mMaxX, mMaxY, 0.0f};
        }

        void minMax(Vector3f& min, Vector3f& max) const noexcept
        {
            min = Vector3f{mMinX, mMinY, 0.0f};
            max = Vector3f{mMaxX, mMaxY, 0.0f};
        }

        Vector3f center() const noexcept
        {
            return Vector3f{(mMinX + mMaxX) * 0.5f, (mMinY + mMaxY) * 0.5f, 0.0f};
        }

        float width() const noexcept
        {
            return (mMaxX - mMinX);
        }

        float height() const noexcept
        {
            return (mMaxY - mMinY);
        }

        float area() const noexcept
        {
            return ((mMaxX - mMinX) * (mMaxY - mMinY));
        }

        void corners(std::span<Vector3f, 4> points) const noexcept;

        void merge(const Vector3f& pt) noexcept;
        void merge(std::span<const Vector3f> points) noexcept;
        void merge(float x, float y) noexcept;

        BRect& set(float minX, float minY, float maxX, float maxY) noexcept
        {
            mMinX = minX;
            mMinY = minY;
            mMaxX = maxX;
            mMaxY = maxY;
            return *this;
        }

        BRect& setExpand(float centerX, float centerY, float expandAmount) noexcept
        {
            mMinX = centerX;
            mMinY = centerY;
            mMaxX = centerX;
            mMaxY = centerY;
            expand(expandAmount);
            return *this;
        }

        BRect& setExpand(float centerX, float centerY, float expandX, float expandY) noexcept
        {
            mMinX = centerX;
            mMinY = centerY;
            mMaxX = centerX;
            mMaxY = centerY;
            expand(expandX, expandY);
            return *this;
        }

        BRect& setMinMax(const Vector3f& vec) noexcept
        {
            mMinX = vec[0];
            mMinY = vec[1];
            mMaxX = vec[0];
            mMaxY = vec[1];
            return *this;
        }

        BRect& setMinMax(float x, float y) noexcept
        {
            mMinX = x;
            mMinY = y;
            mMaxX = x;
            mMaxY = y;
            return *this;
        }

        BRect& setMin(const Vector3f& min) noexcept
        {
            mMinX = min[0];
            mMinY = min[1];
            return *this;
        }

        BRect& setMin(float x, float y) noexcept
        {
            mMinX = x;
            mMinY = y;
            return *this;
        }

        BRect& setMax(const Vector3f& max) noexcept
        {
            mMaxX = max[0];
            mMaxY = max[1];
            return *this;
        }

        BRect& setMax(float x, float y) noexcept
        {
            mMaxX = x;
            mMaxY = y;
            return *this;
        }

        void translate(const Vector3f& translation) noexcept;
        void expand(float amountX, float amountY) noexcept;
        void expand(float amount) noexcept
        {
            expand(amount, amount);
        }

        std::optional<BRect> crossSection(const BRect& brect) const noexcept;

        bool contains(const Vector3f& point) const
        {
            return (point[0] >= mMinX && point[1] >= mMinY && point[0] <= mMaxX && point[1] <= mMaxY);
        }

        bool contains(float x, float y) const
        {
            return (x >= mMinX && y >= mMinY && x <= mMaxX && y <= mMaxY);
        }

        bool containsX(float x) const
        {
            return (x >= mMinX && x <= mMaxX);
        }

        bool containsY(float y) const
        {
            return (y >= mMinY && y <= mMaxY);
        }
    };
}
