#pragma once

#include "vector.hpp"

#include <span>

namespace hr
{
	/*
	* Implementation details:
	*   - the layout is x, y, z, w
	*   - product (multiplication) order: (q1 * q2) means apply rotation of q1 and then the rotation of q2
	*   - rotations are left-handed, which means positive rotation is clockwise about the axis of rotation (as looking to the origin of axis)
	*     - plus X points right, plus Y points up and plus Z points forward (to the horizon)
	*     - this is the same as in matrices
	*   - almost every operation assumes that the quaternion is normalized (unit quaternion)
	*/
	template<typename TDataType>
	class alignas(alignof(TDataType) * 4) Quaternion
	{
		TDataType mData[4];

		using Vector3Type = typename Vector<TDataType, 3>;
		using Vector4Type = typename Vector<TDataType, 4>;

		template<class T>
		static constexpr T kHalf = T(0.5L);
		template<class T>
		static constexpr T kZero = T(0.0L);
		template<class T>
		static constexpr T kOne = T(1.0L);
		template<class T>
		static constexpr T kTwo = T(2.0L);

	public:
		using DataType = typename TDataType;

		enum class AxisOrder { XYZ, XZY, YXZ, YZX, ZXY, ZYX };

		static constexpr Quaternion identity() noexcept
		{
			Quaternion quat;
			quat.mData[0] = quat.mData[1] = quat.mData[2] = kZero<TDataType>;
			quat.mData[3] = kOne<TDataType>;

			return quat;
		}

		static constexpr Quaternion zero()
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
			if (data.size() != 4) return Quaternion::identity();

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

		static constexpr Quaternion from(const TDataType qx, const TDataType qy, const TDataType qz, const TDataType qw)
		{
			Quaternion quat;
			quat.mData[0] = qx;
			quat.mData[1] = qy;
			quat.mData[2] = qz;
			quat.mData[3] = qw;

			return quat;
		}

		static constexpr Quaternion from(const Vector3Type &vec, const TDataType qw)
		{
			Quaternion quat;
			quat.mData[0] = vec[0];
			quat.mData[1] = vec[1];
			quat.mData[2] = vec[2];
			quat.mData[3] = qw;

			return quat;
		}

		static constexpr Quaternion from(const Vector4Type &vec)
		{
			Quaternion quat;
			quat.mData[0] = vec[0];
			quat.mData[1] = vec[1];
			quat.mData[2] = vec[2];
			quat.mData[3] = vec[3];

			return quat;
		}

		static Quaternion fromAxisAngle(const Vector3Type& unitVec, const TDataType angleDeg);
		static Quaternion fromAxisAngle(const TDataType unitVecX, const TDataType unitVecY, const TDataType unitVecZ, const TDataType angleDeg);

		static Quaternion fromMatrix3x3(const TDataType* const matrix) noexcept;
		static Quaternion fromMatrix4x4(const TDataType* const matrix) noexcept;
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

		constexpr TDataType* data() noexcept
		{
			return mData;
		}

		constexpr const TDataType* data() const noexcept
		{
			return mData;
		}

		constexpr TDataType& operator[](const size_t index) noexcept
		{
			return mData[index % 4];
		}

		constexpr const TDataType& operator[](const size_t index) const noexcept
		{
			return mData[index % 4];
		}

		Quaternion& operator+=(const Quaternion &quat) noexcept;
		Quaternion& operator-=(const Quaternion &quat) noexcept;
		Quaternion& operator*=(const Quaternion &quat) noexcept;
		Quaternion& operator*=(const TDataType scalar) noexcept;
		Quaternion& operator/=(const Quaternion &quat) noexcept;

		Quaternion operator+(const Quaternion& quat) const noexcept;
		Quaternion operator-(const Quaternion &quat) const noexcept;
		Quaternion operator*(const Quaternion &quat) const noexcept;
		Quaternion operator*(const TDataType scalar) const noexcept;
		Quaternion operator/(const Quaternion &quat) const noexcept;

		bool isEqual(const Quaternion& quat, const float precision) const noexcept
		{
			return ((std::abs(mData[0] - quat.mData[0]) < precision) && (std::abs(mData[1] - quat.mData[1]) < precision) && (std::abs(mData[2] - quat.mData[2]) < precision) && (std::abs(mData[3] - quat.mData[3]) < precision));
		}

		Quaternion& scaleAngle(const TDataType scale) noexcept;
		Quaternion& conjugate() noexcept;
		Quaternion& normalize() noexcept;

		Quaternion getConjugate() const noexcept;

		TDataType getMagnitude() const noexcept;
		TDataType getMagnitudeSquared() const noexcept;
		TDataType getDot(const Quaternion &quat) const noexcept;

		void getAxisAngle(TDataType& vecX, TDataType& vecY, TDataType& vecZ, TDataType& ang) const noexcept;
		void getAxisAngle(Vector3Type& vec, TDataType& ang) const noexcept;
		void getEulerAngles(TDataType& angX, TDataType& angY, TDataType& angZ) const noexcept;

		Vector3Type unitRotate(const Vector3Type& vec) const noexcept;
		void unitRotate(const Vector3Type& vec, Vector3Type& dest) const noexcept;
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
