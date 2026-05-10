export module core:plane;

import std;
import :ray;
import :vector;

export namespace hr
{
    template<typename TDataType>
    class Plane
    {
        static_assert(std::is_arithmetic_v<TDataType>, "Data type must be arithmetic (e.g.: float, unsigned char, etc.)");

        // scalar equation of plane: ax + by + cz = d
        TDataType mA, mB, mC, mD;

    public:
        using DataType = TDataType;
        using VectorType = Vector<TDataType, 3>;
        using RayType = Ray<Vector<TDataType, 3>>;

        enum class Position
        {
            CoPlanar,
            Front,
            Behind,
            Intersect
        };

    public:
        static constexpr Plane zero() noexcept
        {
            return Plane{TDataType()};
        }

        static Plane lerp(const Plane& from, const Plane& to, TDataType factor) noexcept
        {
            Plane result;
            result.mA = from.mA * factor;
            result.mB = from.mB * factor;
            result.mC = from.mC * factor;

            VectorType vecAux(to.mA, to.mB, to.mC);
            vecAux *= (1.0f - factor);

            result.mA += vecAux[0];
            result.mB += vecAux[1];
            result.mC += vecAux[2];
            result.normalizeNormal();

            result.mD = (from.mD * factor) + (to.mD * (1.0f - factor));

            return result;
        }

    public:
        constexpr Plane() noexcept = delete;

        explicit constexpr Plane(TDataType scalar) noexcept
          : mA{scalar}, mB{scalar}, mC{scalar}, mD{scalar}
        {}
        explicit constexpr Plane(TDataType a, TDataType b, TDataType c, TDataType d) noexcept
          : mA{a}, mB{b}, mC{c}, mD{d}
        {}

        explicit constexpr Plane(const VectorType& normal, TDataType d) noexcept
          : mA{normal[0]}, mB{normal[1]}, mC{normal[2]}, mD{d}
        {}

        Plane& replaceNormal(const VectorType& normal) noexcept
        {
            mA = normal[0];
            mB = normal[1];
            mC = normal[2];
            return *this;
        }

        Plane& replaceNormal(TDataType ax, TDataType by, TDataType cz) noexcept
        {
            mA = ax;
            mB = by;
            mC = cz;
            return *this;
        }

        Plane& replaceD(TDataType d) noexcept
        {
            mD = d;
            return *this;
        }

        Plane& normalize(void) noexcept
        {
            auto sizeInv = Math::sqrtInv(mA * mA + mB * mB + mC * mC);
            mA *= sizeInv;
            mB *= sizeInv;
            mC *= sizeInv;
            mD *= sizeInv;

            return *this;
        }

        Plane& normalizeNormal(void) noexcept
        {
            auto sizeInv = Math::sqrtInv(mA * mA + mB * mB + mC * mC);
            mA *= sizeInv;
            mB *= sizeInv;
            mC *= sizeInv;

            return *this;
        }

        Plane& negateNormal() noexcept
        {
            mA *= -1.0f;
            mB *= -1.0f;
            mC *= -1.0f;

            return *this;
        }

        Plane& negateD() noexcept
        {
            mD *= -1.0f;

            return *this;
        }

        void calcD(const VectorType& pointOnPlane) noexcept
        {
            mD = -pointOnPlane.dot(mA, mB, mC);
        }

        TDataType distance(const VectorType& point) const noexcept
        {
            return (point.dot(mA, mB, mC) + mD);
        }

        VectorType normal() const noexcept
        {
            return VectorType(mA, mB, mC);
        }

        TDataType d() const noexcept
        {
            return mD;
        }

        TDataType dotCoord(const VectorType& point) const noexcept
        {
            return (point.getDot(mA, mB, mC) + mD);
        }

        TDataType dotNormal(const VectorType& point) const noexcept
        {
            return point.dot(mA, mB, mC);
        }

        bool intersects(const RayType& ray) const noexcept
        {
            TDataType hitDistance;
            return intersects(ray, hitDistance);
        }

        bool intersects(const RayType& ray, TDataType& hitDistance) const noexcept
        {
            auto dot = ray.dir().dot(mA, mB, mC);
            if (Math::isZero(dot))
                return false;

            hitDistance = -(ray.origin().dot(mA, mB, mC) + mD) / dot;
            return (hitDistance >= 0.0f);
        }

        bool intersects(const VectorType& lineStart, const VectorType& lineEnd) const noexcept
        {
            VectorType result;
            return intersects(lineStart, lineEnd, result);
        }

        bool intersects(const VectorType& lineStart, const VectorType& lineEnd, VectorType& result) const noexcept
        {
            RayType ray{lineStart, VectorType::calcNormalize(lineEnd - lineStart)};

            float hitDistance;
            if (!intersects(ray, hitDistance))
                return false;

            if (hitDistance > lineStart.distance(lineEnd))
                return false;

            result = ray.pointAt(hitDistance);

            return true;
        }

        bool intersects(const Plane& p2, const Plane& p3, VectorType& result) const noexcept
        {
            VectorType tmp1, tmp2, tmp3;

            VectorType pNormal{mA, mB, mC};
            VectorType p2Normal{p2.mA, p2.mB, p2.mC};
            VectorType p3Normal{p3.mA, p3.mB, p3.mC};
            tmp1 = VectorType::calcCrossProduct(p2Normal, p3Normal);

            auto denominator = pNormal.dot(tmp1);
            if (Math::isZero(denominator))
                return false;

            tmp1 = Vector3f::calcCrossProduct(p2Normal, p3Normal);
            tmp2 = Vector3f::calcCrossProduct(p3Normal, pNormal);
            tmp3 = Vector3f::calcCrossProduct(pNormal, p2Normal);
            tmp1 *= mD;
            tmp2 *= p2.mD;
            tmp3 *= p3.mD;

            result = tmp1 + tmp2 + tmp3;
            result /= (-denominator);

            return true;
        }

        Position classifyPoint(const VectorType& point) const noexcept
        {
            auto calcDot = point.dot(mA, mB, mC) + mD;

            if (Math::isZero(calcDot))
                return Plane::Position::CoPlanar;
            if (calcDot > 0.0f)
                return Plane::Position::Front;
            return Plane::Position::Behind;
        }

        Position classifyTri(const VectorType& p1, const VectorType& p2, const VectorType& p3) const noexcept
        {
            auto c1 = classifyPoint(p1);
            auto c2 = classifyPoint(p2);
            auto c3 = classifyPoint(p3);

            return (((c1 == c2) && (c2 == c3)) ? c1 : Position::Intersect);
        }
    };

    static_assert(std::is_trivially_copyable_v<Plane<size_t>>, "For performance reasons, the Plane<size_t> data type should be trivially copyable");
    static_assert(std::is_trivially_copyable_v<Plane<float>>, "For performance reasons, the Plane<float> data type should be trivially copyable");
}
