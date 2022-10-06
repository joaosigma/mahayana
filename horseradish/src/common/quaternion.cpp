#include "quaternion.hpp"

#include "math.hpp"

#include <type_traits>

namespace hr
{
	static_assert(std::is_trivially_copyable<Quaternion>::value);

	Quaternion Quaternion::genAxisAngle(const Vector3f& unitVec, const float angleDeg)
	{
		return Quaternion::genAxisAngle(unitVec[0], unitVec[1], unitVec[2], angleDeg);
	}

	Quaternion Quaternion::genAxisAngle(const float unitVecX, const float unitVecY, const float unitVecZ, const float angleDeg)
	{
		Quaternion ret;

		auto angleRad = Math::Deg2Rad<float> * angleDeg * 0.5f;

		float sin;
		Math::sinCos(angleRad, sin, ret.mData[3]);

		ret.mData[0] = unitVecX * sin;
		ret.mData[1] = unitVecY * sin;
		ret.mData[2] = unitVecZ * sin;

		return ret;
	}

	Quaternion& Quaternion::operator+=(const Quaternion &quat) noexcept
	{
		_mm_store_ps(mData, _mm_add_ps(_mm_load_ps(mData), _mm_load_ps(quat.mData)));
		return *this;
	}

	Quaternion& Quaternion::operator-=(const Quaternion &quat) noexcept
	{
		_mm_store_ps(mData, _mm_sub_ps(_mm_load_ps(mData), _mm_load_ps(quat.mData)));
		return *this;
	}

	Quaternion& Quaternion::operator*=(const Quaternion& quat) noexcept
	{
		Quaternion thiz{*this};

		mData[0] = (quat.mData[3] * thiz.mData[0]) + (quat.mData[0] * thiz.mData[3]) - (quat.mData[1] * thiz.mData[2]) + (quat.mData[2] * thiz.mData[1]);
		mData[1] = (quat.mData[3] * thiz.mData[1]) + (quat.mData[0] * thiz.mData[2]) + (quat.mData[1] * thiz.mData[3]) - (quat.mData[2] * thiz.mData[0]);
		mData[2] = (quat.mData[3] * thiz.mData[2]) - (quat.mData[0] * thiz.mData[1]) + (quat.mData[1] * thiz.mData[0]) + (quat.mData[2] * thiz.mData[3]);
		mData[3] = (quat.mData[3] * thiz.mData[3]) - (quat.mData[0] * thiz.mData[0]) - (quat.mData[1] * thiz.mData[1]) - (quat.mData[2] * thiz.mData[2]);

		return *this;
	}

	Quaternion& Quaternion::operator*=(float scalar) noexcept
	{
		_mm_store_ps(mData, _mm_mul_ps(_mm_load_ps(mData), _mm_load_ps1(&scalar)));
		return *this;
	}

	Quaternion& Quaternion::operator/=(const Quaternion &quat) noexcept
	{
		auto mag = 1.0f / quat.getMagnitudeSquared();

		Quaternion inv{ -quat[0] * mag, -quat[1] * mag, -quat[2] * mag, quat[3] * mag };

		return operator*=(inv);
	}

	Quaternion Quaternion::operator+(const Quaternion& quat) const noexcept
	{
		auto res = *this;
		res += quat;
		return res;
	}

	Quaternion Quaternion::operator-(const Quaternion& quat) const noexcept
	{
		auto res = *this;
		res -= quat;
		return res;
	}

	Quaternion Quaternion::operator*(const Quaternion& quat) const noexcept
	{
		auto res = *this;
		res *= quat;
		return res;
	}

	Quaternion Quaternion::operator*(float scalar) const noexcept
	{
		auto res = *this;
		res *= scalar;
		return res;
	}

	Quaternion Quaternion::operator/(const Quaternion& quat) const noexcept
	{
		auto res = *this;
		res /= quat;
		return res;
	}

	Quaternion& Quaternion::setAxisAngle(const float unitVecX, const float unitVecY, const float unitVecZ, const float angleDeg) noexcept
	{
		*this = Quaternion::genAxisAngle(unitVecX, unitVecY, unitVecZ, angleDeg);
		return *this;
	}

	Quaternion& Quaternion::setAxisAngle(const Vector3f &unitVec, const float angleDeg) noexcept
	{
		*this = Quaternion::genAxisAngle(unitVec, angleDeg);
		return *this;
	}

	Quaternion& Quaternion::setFromMatrix3x3(const float * const matrix) noexcept
	{
		float s = matrix[0] + matrix[4] + matrix[8];
		if (s > 0.0f)
		{
			s = Math::sqrt(s + 1.0f);

			mData[3] = s*0.5f;
			s = 0.5f / s;
			mData[0] = (matrix[5] - matrix[7])*s;
			mData[1] = (matrix[6] - matrix[2])*s;
			mData[2] = (matrix[1] - matrix[3])*s;
			
			return *this;
		}

		if ((matrix[4] <= matrix[0]) && (matrix[8] <= matrix[0]))
		{
			s = Math::sqrt((matrix[0] - (matrix[4] + matrix[8])) + 1.0f);

			mData[0] = s*0.5f;
			s = 0.5f / s;
			mData[1] = (matrix[1] + matrix[3])*s;
			mData[2] = (matrix[2] + matrix[6])*s;
			mData[3] = (matrix[5] - matrix[7])*s;

			return *this;
		}

		if ((matrix[4] > matrix[0]) && (matrix[8] <= matrix[4]))
		{
			s = Math::sqrt((matrix[4] - (matrix[8] + matrix[0])) + 1.0f);

			mData[1] = s*0.5f;
			s = 0.5f / s;
			mData[3] = (matrix[6] - matrix[2])*s;
			mData[2] = (matrix[5] + matrix[7])*s;
			mData[0] = (matrix[3] + matrix[1])*s;

			return *this;
		}

		s = Math::sqrt((matrix[8] - (matrix[0] + matrix[4])) + 1.0f);

		mData[2] = s*0.5f;
		s = 0.5f / s;
		mData[3] = (matrix[1] - matrix[3])*s;
		mData[0] = (matrix[6] + matrix[2])*s;
		mData[1] = (matrix[7] + matrix[5])*s;

		return *this;
	}

	Quaternion& Quaternion::setFromMatrix4x4(const float * const matrix) noexcept
	{
		float s = matrix[0] + matrix[5] + matrix[10];
		if (s > 0.0f)
		{
			s = Math::sqrt(s + 1.0f);

			mData[3] = s*0.5f;
			s = 0.5f / s;
			mData[0] = (matrix[6] - matrix[9])*s;
			mData[1] = (matrix[8] - matrix[2])*s;
			mData[2] = (matrix[1] - matrix[4])*s;
			
			return *this;
		}

		if ((matrix[5] <= matrix[0]) && (matrix[10] <= matrix[0]))
		{
			s = Math::sqrt((matrix[0] - (matrix[5] + matrix[10])) + 1.0f);

			mData[0] = s*0.5f;
			s = 0.5f / s;
			mData[1] = (matrix[1] + matrix[4])*s;
			mData[2] = (matrix[2] + matrix[8])*s;
			mData[3] = (matrix[6] - matrix[9])*s;
			
			return *this;
		}

		if ((matrix[5] > matrix[0]) && (matrix[10] <= matrix[5]))
		{
			s = Math::sqrt((matrix[5] - (matrix[10] + matrix[0])) + 1.0f);

			mData[1] = s*0.5f;
			s = 0.5f / s;
			mData[3] = (matrix[8] - matrix[2])*s;
			mData[2] = (matrix[6] + matrix[9])*s;
			mData[0] = (matrix[4] + matrix[1])*s;
			
			return *this;
		}

		s = Math::sqrt((matrix[10] - (matrix[0] + matrix[5])) + 1.0f);

		mData[2] = s*0.5f;
		s = 0.5f / s;
		mData[3] = (matrix[1] - matrix[4])*s;
		mData[0] = (matrix[8] + matrix[2])*s;
		mData[1] = (matrix[9] + matrix[6])*s;

		return *this;
	}

	Quaternion& Quaternion::setFromEuler(const float angX, const float angY, const float angZ, AxisOrder axisOrder) noexcept
	{
		auto xRot = Quaternion::genAxisAngle(1.0f, 0.0f, 0.0f, angX);
		auto yRot = Quaternion::genAxisAngle(0.0f, 1.0f, 0.0f, angY);
		auto zRot = Quaternion::genAxisAngle(0.0f, 0.0f, 1.0f, angZ);

		switch (axisOrder)
		{
		case AxisOrder::XYZ:
			*this = xRot;
			*this *= yRot;
			*this *= zRot;
			return *this;
		case AxisOrder::XZY:
			*this = xRot;
			*this *= zRot;
			*this *= yRot;
			return *this;
		case AxisOrder::YXZ:
			*this = yRot;
			*this *= xRot;
			*this *= zRot;
			return *this;
		case AxisOrder::YZX:
			*this = yRot;
			*this *= zRot;
			*this *= xRot;
			return *this;
		case AxisOrder::ZXY:
			*this = zRot;
			*this *= xRot;
			*this *= yRot;
			return *this;
		case AxisOrder::ZYX:
			*this = zRot;
			*this *= yRot;
			*this *= xRot;
			return *this;
		default:
			break;
		}

		setIdentity();
		return *this;
	}

	Quaternion& Quaternion::setSLerp(const Quaternion &from, const Quaternion &to, float t) noexcept
	{
		//input quaternions must be normalized

		if (t <= 0.0f)
		{
			*this = from;
			return *this;
		}
		if (t >= 1.0f)
		{
			*this = to;
			return *this;
		}

		double n2;
		double n0;
		double dot = from.getDot(to);

		// if the dot product is negative, slerp won't take the shorter path. Since a quaternion is equivalent if
		// all compoents are negated, fix by reversing one quaternion.
		bool flag = (dot < 0.0);
		if (flag) dot = -dot;

		if (dot > 0.999999) //to close, degenerate to a linear interpolation
		{
			n0 = 1.0 - t;
			n2 = flag ? -t : t;
		}
		else
		{
			double n4 = std::acos(dot);
			double n5 = 1.0 / std::sin(n4);
			n0 = std::sin((1.0 - t) * n4) * n5;
			n2 = flag ? -std::sin(t * n4) * n5 : std::sin(t * n4) * n5;
		}

		mData[0] = static_cast<float>((n0 * from.mData[0]) + (n2 * to.mData[0]));
		mData[1] = static_cast<float>((n0 * from.mData[1]) + (n2 * to.mData[1]));
		mData[2] = static_cast<float>((n0 * from.mData[2]) + (n2 * to.mData[2]));
		mData[3] = static_cast<float>((n0 * from.mData[3]) + (n2 * to.mData[3]));

		normalize();
		return *this;
	}

	Quaternion& Quaternion::setNLerp(const Quaternion &from, const Quaternion &to, float t) noexcept
	{
		if (t <= 0.0f)
		{
			*this = from;
			return *this;
		}
		if (t >= 1.0f)
		{
			*this = to;
			return *this;
		}

		if (from.getDot(to) >= 0.0f)
			*this = (from * (1.0f - t)) + (to * t);
		else
			*this = (from * (1.0f - t)) - (to * t);

		normalize();
		return *this;
	}

	Quaternion& Quaternion::set(const Quaternion& quat) noexcept
	{
		std::memcpy(mData, quat.mData, sizeof(float) * 4);
		return *this;
	}

	Quaternion& Quaternion::setFromVectors(const Vector3f& from, const Vector3f& to) noexcept
	{
		//vectors don't need to be normalized

		/*
		* This is a faster version, but doesn't take into account opposite vectors...
		* 
		* Quaternion q(from.crossProduct(to), from.getDot(to));
		* q[3] += q.getMagnitude();
		* q.normalize();
		* 
		* return q;
		* 
		*/

		float norm_u_norm_v = std::sqrt(from.getDot(from) * to.getDot(to));
		float real_part = norm_u_norm_v + from.getDot(to);
		Vector3f axis;

		if (real_part < (1.e-6f * norm_u_norm_v))
		{
			// if vectors are exactly opposite, rotate 180º around an arbitrary orthogonal axis (axis normalisation can happen later, when we normalise the quaternion)
			real_part = 0.0f;
			axis = std::abs(from[0]) > std::abs(from[2]) ? Vector3f(-from[1], from[0], 0.0f) : Vector3f(0.0f, -from[2], from[1]);
		}
		else
		{
			// otherwise, build quaternion the standard way...
			axis = from.crossProduct(to);
		}

		mData[0] = axis[0];
		mData[1] = axis[1];
		mData[2] = axis[2];
		mData[3] = real_part;
		normalize();

		return *this;
	}

	Quaternion& Quaternion::setIdentity() noexcept
	{
		mData[0] = mData[1] = mData[2] = 0.0f;
		mData[3] = 1.0f;

		return *this;
	}

	Quaternion& Quaternion::scaleAngle(float scale) noexcept
	{
		mData[3] *= scale;

		return *this;
	}

	Quaternion& Quaternion::conjugate() noexcept
	{
		//since we're dealing with unit quaternions, the conjugate is the same as the inverse
		mData[0] = -mData[0];
		mData[1] = -mData[1];
		mData[2] = -mData[2];

		return *this;
	}

	Quaternion& Quaternion::normalize() noexcept
	{
		__m128 vecTmp = _mm_load_ps(mData);
		__m128 vecMag = _mm_rsqrt_ps(_mm_dp_ps(vecTmp, vecTmp, 0xF0 | 0xF));
		_mm_store_ps(mData, _mm_mul_ps(vecTmp, vecMag));

		return *this;
	}

	Quaternion Quaternion::getConjugate() const noexcept
	{
		Quaternion res{ *this };
		res.conjugate();

		return res;
	}

	float Quaternion::getMagnitude() const noexcept
	{
		float mag;

		__m128 vecTmp = _mm_load_ps(mData);
		_mm_store_ss(&mag, _mm_sqrt_ss(_mm_dp_ps(vecTmp, vecTmp, 0xF0 | 0xF)));

		return mag;
	}

	float Quaternion::getMagnitudeSquared() const noexcept
	{
		float mag;

		__m128 vecTmp = _mm_load_ps(mData);
		_mm_store_ss(&mag, _mm_dp_ps(vecTmp, vecTmp, 0xF0 | 0xF));

		return mag;
	}

	float Quaternion::getDot(const Quaternion &quat) const noexcept
	{
		float dot;

		_mm_store_ss(&dot, _mm_dp_ps(_mm_load_ps(mData), _mm_load_ps(quat.mData), 0xF0 | 0xF));
		return dot;
	}

	void Quaternion::getAxisAngle(float& vecX, float& vecY, float& vecZ, float& ang) const noexcept
	{
		Vector3f vec;
		getAxisAngle(vec, ang);

		vecX = vec[0];
		vecY = vec[1];
		vecZ = vec[2];
	}

	void Quaternion::getAxisAngle(Vector3f &vec, float& ang) const noexcept
	{
		float len = mData[0] * mData[0] + mData[1] * mData[1] + mData[2] * mData[2];
		if (len == 0.0f)
		{
			vec[0] = 1.0f;
			vec[1] = 0.0f;
			vec[2] = 0.0f;
			ang = 0.0f;
			return;
		}

		vec.set(mData);
		vec *= (1.0f / len);
		vec.normalize();

		ang = std::acos(mData[3]) * 114.5915590261646417f; // 180/pi=57.295779513082320876f * 2.0f
	}

	void Quaternion::getEulerAngles(float& angX, float& angY, float& angZ) const noexcept
	{
		double test = (mData[0] * mData[3]) - (mData[1] * mData[2]);
		if (test > 0.4995f) // singularity at north pole
		{
			angY = 2.0f * std::atan2(mData[1], mData[0]);
			angX = Math::Pi<float> * 2.0f;
			angZ = 0.0f;
			return;
		}
		if (test < -0.4995f) // singularity at south pole
		{
			angY = -2.0f * std::atan2(mData[1], mData[0]);
			angX = -Math::Pi<float> * 2.0f;
			angZ = 0.0f;
			return;
		}

		// yaw
		angY = std::atan2((2.0f * mData[3] * mData[1]) + (2.0f * mData[2] * mData[0]), 1.0f - (2.0f * (mData[0] * mData[0] + mData[1] * mData[1])));

		// pitch
		angX = std::asin(2.0f * (mData[3] * mData[0] - mData[1] * mData[2]));

		// roll
		angZ = std::atan2((2.0f * mData[3] * mData[2]) + (2.0f * mData[0] * mData[1]), 1.0f - (2.0f * (mData[2] * mData[2] + mData[0] * mData[0])));
	}

	Vector3f Quaternion::unitRotate(const Vector3f& vec) const noexcept
	{
		//this only works if this quaternion is normalized (a unit quaternion)

		Vector3f u{ mData[0], mData[1], mData[2] };

		Vector3f res;
		res = u * 2.0f * u.getDot(vec);
		res += vec * ((mData[3] * mData[3]) - u.getDot(u));
		res += u.crossProduct(vec) * (2.0f * mData[3]);

		return res;
	}

	void Quaternion::unitRotate(const Vector3f &vec, Vector3f &dest) const noexcept
	{
		dest = unitRotate(vec);
	}
}
