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
			std::memset(mData, 0, sizeof(float) * 4);
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

		void operator+=(const Quaternion &quat);
		void operator-=(const Quaternion &quat);
		void operator*=(const Quaternion &quat);
		void operator*=(const float &scalar);
		void operator/=(const Quaternion &quat);

		void setAxisAngle(const float &vx, const float &vy, const float &vz, const float &angleDeg);
		void setAxisAngle(const Vector3f &vec, const float &angleDeg);
		void setFromMatrix3x3(const float * const matrix);
		void setFromMatrix4x4(const float * const matrix);
		void setFromEuler(const float &angX, const float &angY, const float &angZ);
		void setSLerp(const Quaternion &from, const Quaternion &to, const float &t);
		void setLerp(const Quaternion &from, const Quaternion &to, const float &t);
		void set(const float &nx, const float &ny, const float &nz, const float &nw);
		void set(const Vector3f &vec, const float &nw);
		void set(const Quaternion &quat);
		void setAngle(const float &nx, const float &ny, const float &nz, const float &angleDeg);
		void setAngle(const Vector3f &vec, const float &angleDeg);
		void setFromVectors(const Vector3f &v1, const Vector3f &v2);
		void setIdentity();

		void scaleAngle(float scale);
		void invert();
		void normalize();
		void mulEulerAngles(float angX, float angY, float angZ);
		void expandW();
		void expandWNormalize();

		float getDot(const Quaternion &quat) const;
		void getVector(float * const vec) const;
		void getVector(Vector3f &vec) const;
		void getMatrix3x3(float* const matrix) const;
		void getMatrix4x4(float* const matrix) const;
		void getAxisAngle(float* const vecX, float* const vecY, float* const vecZ, float* const ang) const;
		void getAxisAngle(Vector3f &vec, float * const ang) const;
		void getEulerAngles(float * const angX, float * const angY, float * const angZ) const;

		void rotateVector3(Vector3f &vec) const;
		void rotateVector3(const Vector3f &vec, Vector3f &dest) const;
		void rotateVector3(const float * const vec, float * const dest) const;
		void rotateVector3(const float &vx, const float &vy, const float &vz, Vector3f &dest) const;
	};
}
