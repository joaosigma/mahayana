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
		_mm_storeu_ps(mData, _mm_add_ps(_mm_loadu_ps(mData), _mm_loadu_ps(quat.mData)));
		return *this;
	}

	Quaternion& Quaternion::operator-=(const Quaternion &quat) noexcept
	{
		_mm_storeu_ps(mData, _mm_sub_ps(_mm_loadu_ps(mData), _mm_loadu_ps(quat.mData)));
		return *this;
	}

	Quaternion& Quaternion::operator*=(const Quaternion &quat) noexcept
	{
		Quaternion thiz{ *this };

		mData[0] = (quat.mData[3] * thiz.mData[0]) + (quat.mData[0] * thiz.mData[3]) + (quat.mData[1] * thiz.mData[2]) - (quat.mData[2] * thiz.mData[1]);
		mData[1] = (quat.mData[3] * thiz.mData[1]) - (quat.mData[0] * thiz.mData[2]) + (quat.mData[1] * thiz.mData[3]) + (quat.mData[2] * thiz.mData[0]);
		mData[2] = (quat.mData[3] * thiz.mData[2]) + (quat.mData[0] * thiz.mData[1]) - (quat.mData[1] * thiz.mData[0]) + (quat.mData[2] * thiz.mData[3]);
		mData[3] = (quat.mData[3] * thiz.mData[3]) - (quat.mData[0] * thiz.mData[0]) - (quat.mData[1] * thiz.mData[1]) - (quat.mData[2] * thiz.mData[2]);

		return *this;
	}

	Quaternion& Quaternion::operator*=(const float &scalar) noexcept
	{
		_mm_storeu_ps(mData, _mm_mul_ps(_mm_loadu_ps(mData), _mm_load_ps1(&scalar)));
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

	Quaternion Quaternion::operator*(const float& scalar) const noexcept
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
		Quaternion v1 = to;

		double dot = from.getDot(v1); // compute the cosine of the angle between the two vectors.

		// If the dot product is negative, slerp won't take
		// the shorter path. Note that v1 and -v1 are equivalent when
		// the negation is applied to all four components. Fix by 
		// reversing one quaternion.
		if (dot < 0.0f)
		{
			v1.mData[0] = -v1.mData[0];
			v1.mData[1] = -v1.mData[1];
			v1.mData[2] = -v1.mData[2];
			v1.mData[3] = -v1.mData[3];
			dot = -dot;
		}
		
		// If the inputs are too close for comfort, linearly interpolate and normalize the result.
		if (dot >= 0.99995)
		{
			setNLerp(from, v1, t);
			return *this;
		}

		// Since dot is in range [0, DOT_THRESHOLD], acos is safe
		double theta_0 = std::acos(dot);        // theta_0 = angle between input vectors
		double theta = theta_0 * t;          // theta = angle between v0 and result
		double sin_theta = std::sin(theta);     // compute this value only once
		double sin_theta_0 = std::sin(theta_0) + 0.0000001; // compute this value only once

		double s0 = std::cos(theta) - dot * sin_theta / sin_theta_0;  // == sin(theta_0 - theta) / sin(theta_0)
		double s1 = sin_theta / sin_theta_0;

		mData[0] = static_cast<float>(from.mData[0] * s0 + v1.mData[0] * s1);
		mData[1] = static_cast<float>(from.mData[1] * s0 + v1.mData[1] * s1);
		mData[2] = static_cast<float>(from.mData[2] * s0 + v1.mData[2] * s1);
		mData[3] = static_cast<float>(from.mData[3] * s0 + v1.mData[3] * s1);

		normalize();
		return *this;
	}

	Quaternion& Quaternion::setNLerp(const Quaternion &from, const Quaternion &to, float t) noexcept
	{
		float t0 = 1.0f - t;

		if (from.getDot(to) < 0.0f)
		{
			mData[0] = (t0 * from.mData[0]) + (t * (-to.mData[0]));
			mData[1] = (t0 * from.mData[1]) + (t * (-to.mData[1]));
			mData[2] = (t0 * from.mData[2]) + (t * (-to.mData[2]));
			mData[3] = (t0 * from.mData[3]) + (t * (-to.mData[3]));
		}
		else
		{
			mData[0] = (t0 * from.mData[0]) + (t * to.mData[0]);
			mData[1] = (t0 * from.mData[1]) + (t * to.mData[1]);
			mData[2] = (t0 * from.mData[2]) + (t * to.mData[2]);
			mData[3] = (t0 * from.mData[3]) + (t * to.mData[3]);
		}

		normalize();
		return *this;
	}

	Quaternion& Quaternion::set(const Quaternion& quat) noexcept
	{
		std::memcpy(mData, quat.mData, sizeof(float) * 4);
		return *this;
	}

	Quaternion& Quaternion::setFromVectors(const Vector3f &from, const Vector3f &to) noexcept
	{
		//!untested!

		//this only works if both vectors are normalized

		float cost = Vector3f::calcDot(from, to);
		if (cost > 0.99999f) //they're parallel
		{
			mData[0] = mData[1] = mData[2] = 0.0f;
			mData[3] = 1.0f;

			return *this;
		}
		else if (cost < -0.99999f) //they're opposite
		{
			//check if we can use cross product of from vector with [1, 0, 0]
			Vector3f t{ 0.0, from[0], -from[1] };

			if (t.getDot() < 1e-6) // nope! we need cross product of from vector with [0, 1, 0]
				t.set(-from[2], 0.0, from[0]);

			//normalize
			t.normalize();

			mData[0] = t[0];
			mData[1] = t[1];
			mData[2] = t[2];
			mData[3] = 0.0;

			return *this;
		}

		//... else we can just cross two vectors
		auto t = from.crossProduct(to);
		t.normalize();

		//we have to use half-angle formulae (sin^2 t = ( 1 - cos (2t) ) /2)
		t *= std::sqrt(0.5f * (1.0f - cost));

		//scale the axis to get the normalized quaternion
		mData[0] = t[0];
		mData[1] = t[1];
		mData[2] = t[2];

		//cos^2 t = ( 1 + cos (2t) ) / 2
		//w part is cosine of half the rotation angle
		mData[3] = std::sqrt(0.5f * (1.0f + cost));

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
		mData[0] = -mData[0];
		mData[1] = -mData[1];
		mData[2] = -mData[2];

		return *this;
	}

	Quaternion& Quaternion::normalize() noexcept
	{
		__m128 vecTmp = _mm_loadu_ps(mData);
		__m128 vecMag = _mm_rsqrt_ps(_mm_dp_ps(vecTmp, vecTmp, 0xF0 | 0xF));
		_mm_storeu_ps(mData, _mm_mul_ps(vecTmp, vecMag));

		return *this;
	}

	Quaternion& Quaternion::expandWNormalized() noexcept
	{
		//we can only expand (calculate) W if we assume the quaternion is of unit length, which means the final quaternion is already normalized

		float term = 1.0f - (mData[0] * mData[0]) - (mData[1] * mData[1]) - (mData[2] * mData[2]);
		mData[3] = (term < 0.0f) ? 0.0f : -sqrt(term);

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

		__m128 vecTmp = _mm_loadu_ps(mData);
		_mm_store_ss(&mag, _mm_sqrt_ss(_mm_dp_ps(vecTmp, vecTmp, 0xF0 | 0xF)));

		return mag;
	}

	float Quaternion::getMagnitudeSquared() const noexcept
	{
		float mag;

		__m128 vecTmp = _mm_loadu_ps(mData);
		_mm_store_ss(&mag, _mm_dp_ps(vecTmp, vecTmp, 0xF0 | 0xF));

		return mag;
	}

	float Quaternion::getDot(const Quaternion &quat) const noexcept
	{
		float dot;

		_mm_store_ss(&dot, _mm_dp_ps(_mm_loadu_ps(mData), _mm_loadu_ps(quat.mData), 0xF0 | 0xF));
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
			vec[0] = 0.0f;
			vec[1] = 0.0f;
			vec[2] = 1.0f;
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
		//TODO
	}

	Vector3f Quaternion::unitRotate(const Vector3f& vec) const noexcept
	{
		//this only works if this quaternion is normalized (a unit quaternion)

		/*Quaternion qrotated{ *this };
		qrotated *= Quaternion{ vec[0], vec[1], vec[2], 0.f };
		qrotated *= getConjugate();

		return Vector3f{ qrotated.mData[0], qrotated.mData[1], qrotated.mData[2] };*/

		Vector3f u{ mData[0], mData[1], mData[2] };

		Vector3f res;
		res = u * 2.0f * u.getDot(vec);
		res += vec * ((mData[3] * mData[3]) - u.getDot(u));
		res += u.crossProduct(vec) * 2.0f * mData[3];

		return res;
	}

	void Quaternion::unitRotate(const Vector3f &vec, Vector3f &dest) const noexcept
	{
		dest = unitRotate(vec);
	}
}