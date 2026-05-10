module;

#include <cassert>

export module core:quaternion;

import std;
import :vector;

export namespace hr
{
    /*
     * Implementation details:
     *   - the layout is x, y, z, w
     *   - product (multiplication) order: (q1 * q2) means apply rotation of q1 and then the rotation of q2
     *   - rotations are left-handed, which means positive rotation is clockwise about the axis of rotation (as looking to the origin of axis)
     *     - plus X points right, plus Y points up and plus Z points forward (to the horizon)
     *     - this is the same as in matrices
     *   - almost every operation assumes that the quaternion is normalized (unit quaternion)
     *   - dividing quaternion A with B is the same as multiplying A with the inverse of B (that why there's no operator/ overload)
     *
     * **NOTE**
     *
     * Don't forget that quaternion multiplication is right to left. So if we write (Qr = Qp * Qc), this means that the first rotation applied
     * is Qc and then Qp. And so, if we do (Qr = Qp * Qc), then we can:
     *	- (Qc = Qp.inversed * Qr) -> so we rotate by Qr first, then unrotate it by Qp, yielding Qc
     *	- (Qp = Qr * Qc.inversed) -> so we apply the inverse of Qc first and then Qr to obtain Qp
     *
     */
    template<typename TDataType>
    class alignas(alignof(TDataType) * 4) Quaternion
    {
        TDataType mData[4];

        using Vector3Type = typename Vector<TDataType, 3>;
        using Vector4Type = typename Vector<TDataType, 4>;

        template<class T>
        static constexpr T kHalf = T(0.5);
        template<class T>
        static constexpr T kZero = T(0.0);
        template<class T>
        static constexpr T kOne = T(1.0);
        template<class T>
        static constexpr T kTwo = T(2.0);

    public:
        using DataType = typename TDataType;

        enum class AxisOrder
        {
            XYZ,
            XZY,
            YXZ,
            YZX,
            ZXY,
            ZYX
        };

        static constexpr Quaternion identity() noexcept
        {
            Quaternion quat;
            quat.mData[0] = quat.mData[1] = quat.mData[2] = kZero<TDataType>;
            quat.mData[3] = kOne<TDataType>;

            return quat;
        }

        static constexpr Quaternion zero() noexcept
        {
            Quaternion quat;
            quat.mData[0] = quat.mData[1] = quat.mData[2] = quat.mData[3] = kZero<TDataType>;

            return quat;
        }

        template<typename TType>
        static constexpr Quaternion from(std::span<const TType> data) noexcept
        {
            static_assert(std::is_same_v<TType, float> || std::is_same_v<TType, double>, "Type must be either float or double");

            assert(data.size() == 4);
            if (data.size() != 4)
                return Quaternion::identity();

            Quaternion quat;

            if constexpr (std::is_same_v<TDataType, TType>)
            {
                for (int i = 0; i < 4; i++)
                    quat.mData[i] = data[i];
            }
            else
            {
                for (int i = 0; i < 4; i++)
                    quat.mData[i] = static_cast<TDataType>(data[i]);
            }

            return quat;
        }

        static constexpr Quaternion from(const TDataType qx, const TDataType qy, const TDataType qz, const TDataType qw) noexcept
        {
            Quaternion quat;
            quat.mData[0] = qx;
            quat.mData[1] = qy;
            quat.mData[2] = qz;
            quat.mData[3] = qw;

            return quat;
        }

        static constexpr Quaternion from(const Vector3Type& vec, const TDataType qw) noexcept
        {
            Quaternion quat;
            quat.mData[0] = vec[0];
            quat.mData[1] = vec[1];
            quat.mData[2] = vec[2];
            quat.mData[3] = qw;

            return quat;
        }

        static constexpr Quaternion from(const Vector4Type& vec) noexcept
        {
            Quaternion quat;
            quat.mData[0] = vec[0];
            quat.mData[1] = vec[1];
            quat.mData[2] = vec[2];
            quat.mData[3] = vec[3];

            return quat;
        }

        static Quaternion fromAxisAngle(const Vector3Type& unitVec, const TDataType angleDeg) noexcept;
        static Quaternion fromAxisAngle(const TDataType unitVecX, const TDataType unitVecY, const TDataType unitVecZ, const TDataType angleDeg) noexcept;

        static Quaternion fromMatrix3x3(std::span<const TDataType, 9> matrix) noexcept;
        static Quaternion fromMatrix4x4(std::span<const TDataType, 16> matrix) noexcept;
        static Quaternion fromEuler(const TDataType angX, const TDataType angY, const TDataType angZ, AxisOrder axisOrder) noexcept;
        static Quaternion fromVectors(const Vector3Type& from, const Vector3Type& to) noexcept;

        static Quaternion sLerp(const Quaternion& from, const Quaternion& to, const TDataType t) noexcept;
        static Quaternion nLerp(const Quaternion& from, const Quaternion& to, const TDataType t) noexcept;

    public:
        constexpr Quaternion() noexcept
        {
            mData[0] = mData[1] = mData[2] = kZero<TDataType>;
            mData[3] = kOne<TDataType>;
        }

        constexpr Quaternion(const Quaternion&) = default;
        constexpr Quaternion& operator=(const Quaternion&) = default;
        constexpr Quaternion(Quaternion&&) = default;
        constexpr Quaternion& operator=(Quaternion&&) = default;

        template<typename Self>
        constexpr auto data(this Self&& self) noexcept
        {
            return std::span{self.mData}.first<4>();
        }

        template<typename Self>
        constexpr auto& operator[](this Self&& self, const size_t index) noexcept
        {
            return self.mData[index % 4];
        }

        Quaternion& operator+=(const Quaternion& quat) noexcept;
        Quaternion& operator-=(const Quaternion& quat) noexcept;
        Quaternion& operator*=(const Quaternion& quat) noexcept;
        Quaternion& operator*=(const TDataType scalar) noexcept;

        Quaternion operator-() const noexcept;

        Quaternion operator+(const Quaternion& quat) const noexcept;
        Quaternion operator-(const Quaternion& quat) const noexcept;
        Quaternion operator*(const Quaternion& quat) const noexcept;
        Quaternion operator*(const TDataType scalar) const noexcept;

        bool isEqual(const Quaternion& quat, const TDataType precision) const noexcept
        {
            return (std::abs(mData[0] - quat.mData[0]) < precision) && (std::abs(mData[1] - quat.mData[1]) < precision) && (std::abs(mData[2] - quat.mData[2]) < precision) &&
                   (std::abs(mData[3] - quat.mData[3]) < precision);
        }

        Quaternion& scaleAngle(const TDataType scale) noexcept;
        Quaternion& normalize() noexcept;
        Quaternion& conjugate() noexcept;

        template<bool TUnitQuaternion = true>
        Quaternion& inverse() noexcept
        {
            if constexpr (TUnitQuaternion)
            {
                // since we're dealing with a unit quaternion, the inverse is the same as the conjugate
                mData[0] = -mData[0];
                mData[1] = -mData[1];
                mData[2] = -mData[2];
            }
            else
            {
                TDataType invMag = kOne<TDataType> / magnitudeSquared();
                mData[0] = -mData[0] * invMag;
                mData[1] = -mData[1] * invMag;
                mData[2] = -mData[2] * invMag;
                mData[3] *= invMag;
            }

            return *this;
        }

        Quaternion getConjugate() const noexcept;

        template<bool TUnitQuaternion = true>
        Quaternion getInverse() const noexcept
        {
            Quaternion res{*this};
            res.inverse<TUnitQuaternion>();

            return res;
        }

        TDataType magnitude() const noexcept;
        TDataType magnitudeSquared() const noexcept;
        TDataType dot(const Quaternion& quat) const noexcept;

        void getAxisAngle(Vector3Type& vec, TDataType& ang) const noexcept;
        void getEulerAngles(TDataType& angX, TDataType& angY, TDataType& angZ) const noexcept;

        Vector3Type unitRotate(const Vector3Type& vec) const noexcept;
        void unitRotate(const Vector3Type& vec, Vector3Type& dest) const noexcept;

        template<typename TNewDataType>
        Quaternion<TNewDataType> convert() const noexcept
        {
            static_assert(std::is_same_v<TNewDataType, float> || std::is_same_v<TNewDataType, double>, "New data type must be either float or double");

            if constexpr (std::is_same_v<TNewDataType, TDataType>)
            {
                return Quaternion{*this}; // it's just a copy
            }
            else
            {
                return Quaternion<TNewDataType>::from(static_cast<TNewDataType>(mData[0]), static_cast<TNewDataType>(mData[1]), static_cast<TNewDataType>(mData[2]),
                                                      static_cast<TNewDataType>(mData[3]));
            }
        }
    };

    using Quaternionf = Quaternion<float>;
    using Quaterniond = Quaternion<double>;

    static_assert(alignof(Quaternionf) == 16, "For performance reasons, this class must be aligned to a 16 byte boundary");
    static_assert(alignof(Quaterniond) == 32, "For performance reasons, this class must be aligned to a 32 byte boundary");
    static_assert(std::is_trivially_copyable_v<Quaternionf>, "For performance reasons, this class should be trivially copyable");
    static_assert(std::is_trivially_copyable_v<Quaterniond>, "For performance reasons, this class should be trivially copyable");

    extern template class Quaternion<float>;
    extern template class Quaternion<double>;
}
