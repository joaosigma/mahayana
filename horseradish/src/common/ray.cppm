export module core:ray;

import std;
import :vector;

export namespace hr
{
    /*
    The ray class represents a unit ray, which means that the direction is always normalized.

    NOTE: any direction supplied to the object should already be normalized.
    */

    template<typename TVectorType = Vector3f>
    class Ray
    {
        static_assert(std::is_same_v<TVectorType, Vector3f> || std::is_same_v<TVectorType, Vector3d>, "Base type must be either Vector3f or Vector3d");
        static_assert(std::is_trivially_copyable_v<TVectorType>, "For performance reasons, the vector type should be trivially copyable");

        TVectorType mOrigin, mDirection;

    public:
        using DataType = typename TVectorType;

    public:
        constexpr Ray() = default;

        constexpr explicit Ray(TVectorType origin, TVectorType direction) noexcept
          : mOrigin{origin}, mDirection{direction}
        {}

        template<typename Self>
        constexpr auto& origin(this Self&& self) noexcept
        {
            return self.mOrigin;
        }

        template<typename Self>
        constexpr auto& direction(this Self&& self) noexcept
        {
            return self.mDirection;
        }

        constexpr TVectorType pointAt(typename TVectorType::DataType t) const noexcept
        {
            return TVectorType::calcPointAt(mOrigin, mDirection, t);
        }

        template<typename TTargetVectorType>
        Ray<TTargetVectorType> convert() const noexcept
        {
            return Ray<TTargetVectorType>{mOrigin.convert<TTargetVectorType::DataType>(), mDirection.convert<TTargetVectorType::DataType>()};
        }

        constexpr void negateDir() noexcept
        {
            mDirection *= -1.0f;
        }
    };
}
