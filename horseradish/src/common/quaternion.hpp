#pragma once

#include "vector.hpp"

namespace hr
{
	class Quaternion
	{
		float mData[4];

	public:
		Quaternion()
		{
			mData[0] = mData[1] = mData[2] = 0.0f;
			mData[3] = 1.0f;
		}

		explicit Quaternion(const float qx, const float qy, const float qz, const float qw)
		{
			mData[0] = qx;
			mData[1] = qy;
			mData[2] = qz;
			mData[3] = qw;
		}

		float* data()
		{
			return mData;
		}

		const float* data() const
		{
			return mData;
		}

		float& operator[] (const size_t index)
		{
			return mData[index % 4];
		}

		const float& operator[] (const size_t index) const
		{
			return mData[index % 4];
		}

		void operator+=(const Quaternion &quat);
		void operator-=(const Quaternion &quat);
		void operator*=(const Quaternion &quat);
		void operator*=(const float &scalar);
		void operator/=(const Quaternion &quat);

		Quaternion& setAxisAngle(const float &vx, const float &vy, const float &vz, const float &angleDeg);
		Quaternion& setAxisAngle(const Vector3f &unitVec, const float &angleDeg);
		Quaternion& setFromMatrix3x3(const float * const matrix);
		Quaternion& setFromMatrix4x4(const float * const matrix);
		Quaternion& setFromEuler(const float &angX, const float &angY, const float &angZ);
		Quaternion& setSLerp(const Quaternion &from, const Quaternion &to, float t);
		Quaternion& setNLerp(const Quaternion &from, const Quaternion &to, float t);
		Quaternion& set(const float &nx, const float &ny, const float &nz, const float &nw);
		Quaternion& set(const Vector3f &vec, const float &nw);
		Quaternion& set(const Quaternion &quat);
		Quaternion& setAngle(const float &nx, const float &ny, const float &nz, const float &angleDeg);
		Quaternion& setAngle(const Vector3f &vec, const float &angleDeg);
		Quaternion& setFromVectors(const Vector3f &v1, const Vector3f &v2);
		Quaternion& setIdentity();

		Quaternion& scaleAngle(float scale);
		Quaternion& conjugate();
		Quaternion& normalize();
		Quaternion& expandWNormalized();

		float getMagnitude() const;
		float getMagnitudeSquared() const;
		float getDot(const Quaternion &quat) const;

		void getAxisAngle(float* const vecX, float* const vecY, float* const vecZ, float* const ang) const;
		void getAxisAngle(Vector3f &vec, float * const ang) const;
		void getEulerAngles(float * const angX, float * const angY, float * const angZ) const;

		Vector3f unitRotate(const Vector3f &vec) const;
		void unitRotate(const Vector3f &vec, Vector3f &dest) const;
	};
}
