#pragma once

#include "vector.hpp"

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
	class alignas(16) Quaternion
	{
		float mData[4]{ 0.0f, 0.0f, 0.0f, 1.0f };

	public:
		enum class AxisOrder { XYZ, XZY, YXZ, YZX, ZXY, ZYX };

	public:
		static Quaternion genAxisAngle(const Vector3f& unitVec, const float angleDeg);
		static Quaternion genAxisAngle(const float unitVecX, const float unitVecY, const float unitVecZ, const float angleDeg);

	public:
		constexpr Quaternion() = default;
		constexpr Quaternion(const Quaternion&) = default;
		constexpr Quaternion& operator=(const Quaternion&) = default;
		constexpr Quaternion(Quaternion&&) = default;
		constexpr Quaternion& operator=(Quaternion&&) = default;

		explicit constexpr Quaternion(const float quat[4]) noexcept
			: mData{ quat[0], quat[1], quat[2], quat[3] }
		{ }
		
		explicit constexpr Quaternion(const double quat[4]) noexcept
			: mData{ static_cast<float>(quat[0]), static_cast<float>(quat[1]), static_cast<float>(quat[2]), static_cast<float>(quat[3]) }
		{ }

		explicit constexpr Quaternion(const float qx, const float qy, const float qz, const float qw) noexcept
			: mData{ qx, qy, qz, qw }
		{ }

		explicit constexpr Quaternion(const Vector3f& vec, const float qw) noexcept
			: mData{vec[0], vec[1], vec[2], qw}
		{ }

		explicit constexpr Quaternion(const double qx, const double qy, const double qz, const double qw) noexcept
			: mData{ static_cast<float>(qx), static_cast<float>(qy), static_cast<float>(qz), static_cast<float>(qw) }
		{ }

		explicit constexpr Quaternion(const Vector3d& vec, const double qw) noexcept
			: mData{ static_cast<float>(vec[0]), static_cast<float>(vec[1]), static_cast<float>(vec[2]), static_cast<float>(qw) }
		{ }

		constexpr float* data() noexcept
		{
			return mData;
		}

		constexpr const float* data() const noexcept
		{
			return mData;
		}

		constexpr float& operator[] (const size_t index) noexcept
		{
			return mData[index % 4];
		}

		constexpr const float& operator[] (const size_t index) const noexcept
		{
			return mData[index % 4];
		}

		Quaternion& operator+=(const Quaternion &quat) noexcept;
		Quaternion& operator-=(const Quaternion &quat) noexcept;
		Quaternion& operator*=(const Quaternion &quat) noexcept;
		Quaternion& operator*=(float scalar) noexcept;
		Quaternion& operator/=(const Quaternion &quat) noexcept;

		Quaternion operator+(const Quaternion& quat) const noexcept;
		Quaternion operator-(const Quaternion &quat) const noexcept;
		Quaternion operator*(const Quaternion &quat) const noexcept;
		Quaternion operator*(float scalar) const noexcept;
		Quaternion operator/(const Quaternion &quat) const noexcept;

		bool isEqual(const Quaternion& quat, const float precision) const noexcept
		{
			return ((std::abs(mData[0] - quat.mData[0]) < precision) && (std::abs(mData[1] - quat.mData[1]) < precision) && (std::abs(mData[2] - quat.mData[2]) < precision) && (std::abs(mData[3] - quat.mData[3]) < precision));
		}

		Quaternion& setAxisAngle(const float unitVecX, const float unitVecY, const float unitVecZ, const float angleDeg) noexcept;
		Quaternion& setAxisAngle(const Vector3f &unitVec, const float angleDeg) noexcept;
		Quaternion& setFromMatrix3x3(const float * const matrix) noexcept;
		Quaternion& setFromMatrix4x4(const float * const matrix) noexcept;
		Quaternion& setFromEuler(const float angX, const float angY, const float angZ, AxisOrder axisOrder) noexcept;
		Quaternion& setSLerp(const Quaternion &from, const Quaternion &to, float t) noexcept;
		Quaternion& setNLerp(const Quaternion &from, const Quaternion &to, float t) noexcept;
		Quaternion& set(const Quaternion &quat) noexcept;
		Quaternion& setFromVectors(const Vector3f& from, const Vector3f& to) noexcept;
		Quaternion& setIdentity() noexcept;

		Quaternion& scaleAngle(float scale) noexcept;
		Quaternion& conjugate() noexcept;
		Quaternion& normalize() noexcept;

		Quaternion getConjugate() const noexcept;

		float getMagnitude() const noexcept;
		float getMagnitudeSquared() const noexcept;
		float getDot(const Quaternion &quat) const noexcept;

		void getAxisAngle(float& vecX, float& vecY, float& vecZ, float& ang) const noexcept;
		void getAxisAngle(Vector3f& vec, float& ang) const noexcept;
		void getEulerAngles(float& angX, float& angY, float& angZ) const noexcept;

		Vector3f unitRotate(const Vector3f &vec) const noexcept;
		void unitRotate(const Vector3f &vec, Vector3f &dest) const noexcept;
	};
}
