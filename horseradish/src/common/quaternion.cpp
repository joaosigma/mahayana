#include "quaternion.hpp"

#include "math.hpp"

#include <type_traits>

namespace hr
{
	namespace
	{
		double calcDot(const double* quat)
		{
			__m256d data = _mm256_load_pd(quat);
			__m256d tmp = _mm256_mul_pd(data, data);
			tmp = _mm256_hadd_pd(tmp, tmp);
			__m128d dot = _mm_add_pd(_mm256_castpd256_pd128(tmp), _mm256_extractf128_pd(tmp, 1));
			return _mm_cvtsd_f64(dot);
		}

		double calcDot(const double* quat1, const double* quat2)
		{
			__m256d tmp = _mm256_mul_pd(_mm256_load_pd(quat1), _mm256_load_pd(quat2));
			tmp = _mm256_hadd_pd(tmp, tmp);
			__m128d dot = _mm_add_pd(_mm256_castpd256_pd128(tmp), _mm256_extractf128_pd(tmp, 1));
			return _mm_cvtsd_f64(dot);
		}
	}

	template<typename TDataType>
	Quaternion<TDataType> Quaternion<TDataType>::fromAxisAngle(const Vector3Type& unitVec, const TDataType angleDeg)
	{
		return Quaternion::fromAxisAngle(unitVec[0], unitVec[1], unitVec[2], angleDeg);
	}

	template<typename TDataType>
	Quaternion<TDataType> Quaternion<TDataType>::fromAxisAngle(const TDataType unitVecX, const TDataType unitVecY, const TDataType unitVecZ, const TDataType angleDeg)
	{
		auto angleRad = Math::Deg2Rad<TDataType> * angleDeg * kHalf<TDataType>;

		TDataType sin, cos;
		Math::sinCos(angleRad, sin, cos);

		Quaternion ret;
		ret.mData[0] = unitVecX * sin;
		ret.mData[1] = unitVecY * sin;
		ret.mData[2] = unitVecZ * sin;
		ret.mData[3] = cos;

		return ret;
	}

	template<typename TDataType>
	Quaternion<TDataType> Quaternion<TDataType>::fromMatrix3x3(const TDataType* const matrix) noexcept
	{
		auto s = matrix[0] + matrix[4] + matrix[8];
		if (s > kZero<TDataType>)
		{
			s = Math::sqrt(s + kOne<TDataType>);

			Quaternion ret;

			ret.mData[3] = s * kHalf<TDataType>;
			s = kHalf<TDataType> / s;
			ret.mData[0] = (matrix[5] - matrix[7]) * s;
			ret.mData[1] = (matrix[6] - matrix[2]) * s;
			ret.mData[2] = (matrix[1] - matrix[3]) * s;

			return ret;
		}

		if ((matrix[4] <= matrix[0]) && (matrix[8] <= matrix[0]))
		{
			s = Math::sqrt((matrix[0] - (matrix[4] + matrix[8])) + kOne<TDataType>);

			Quaternion ret;

			ret.mData[0] = s * kHalf<TDataType>;
			s = kHalf<TDataType> / s;
			ret.mData[1] = (matrix[1] + matrix[3]) * s;
			ret.mData[2] = (matrix[2] + matrix[6]) * s;
			ret.mData[3] = (matrix[5] - matrix[7]) * s;

			return ret;
		}

		if ((matrix[4] > matrix[0]) && (matrix[8] <= matrix[4]))
		{
			s = Math::sqrt((matrix[4] - (matrix[8] + matrix[0])) + kOne<TDataType>);

			Quaternion ret;

			ret.mData[1] = s * kHalf<TDataType>;
			s = kHalf<TDataType> / s;
			ret.mData[3] = (matrix[6] - matrix[2]) * s;
			ret.mData[2] = (matrix[5] + matrix[7]) * s;
			ret.mData[0] = (matrix[3] + matrix[1]) * s;

			return ret;
		}

		s = Math::sqrt((matrix[8] - (matrix[0] + matrix[4])) + kOne<TDataType>);

		Quaternion ret;

		ret.mData[2] = s * kHalf<TDataType>;
		s = kHalf<TDataType> / s;
		ret.mData[3] = (matrix[1] - matrix[3]) * s;
		ret.mData[0] = (matrix[6] + matrix[2]) * s;
		ret.mData[1] = (matrix[7] + matrix[5]) * s;

		return ret;
	}

	template<typename TDataType>
	Quaternion<TDataType> Quaternion<TDataType>::fromMatrix4x4(const TDataType* const matrix) noexcept
	{
		auto s = matrix[0] + matrix[5] + matrix[10];
		if (s > kZero<TDataType>)
		{
			s = Math::sqrt(s + kOne<TDataType>);

			Quaternion ret;

			ret.mData[3] = s * kHalf<TDataType>;
			s = kHalf<TDataType> / s;
			ret.mData[0] = (matrix[6] - matrix[9]) * s;
			ret.mData[1] = (matrix[8] - matrix[2]) * s;
			ret.mData[2] = (matrix[1] - matrix[4]) * s;

			return ret;
		}

		if ((matrix[5] <= matrix[0]) && (matrix[10] <= matrix[0]))
		{
			s = Math::sqrt((matrix[0] - (matrix[5] + matrix[10])) + kOne<TDataType>);

			Quaternion ret;

			ret.mData[0] = s * kHalf<TDataType>;
			s = kHalf<TDataType> / s;
			ret.mData[1] = (matrix[1] + matrix[4]) * s;
			ret.mData[2] = (matrix[2] + matrix[8]) * s;
			ret.mData[3] = (matrix[6] - matrix[9]) * s;

			return ret;
		}

		if ((matrix[5] > matrix[0]) && (matrix[10] <= matrix[5]))
		{
			s = Math::sqrt((matrix[5] - (matrix[10] + matrix[0])) + kOne<TDataType>);

			Quaternion ret;

			ret.mData[1] = s * kHalf<TDataType>;
			s = kHalf<TDataType> / s;
			ret.mData[3] = (matrix[8] - matrix[2]) * s;
			ret.mData[2] = (matrix[6] + matrix[9]) * s;
			ret.mData[0] = (matrix[4] + matrix[1]) * s;

			return ret;
		}

		s = Math::sqrt((matrix[10] - (matrix[0] + matrix[5])) + kOne<TDataType>);

		Quaternion ret;

		ret.mData[2] = s * kHalf<TDataType>;
		s = kHalf<TDataType> / s;
		ret.mData[3] = (matrix[1] - matrix[4]) * s;
		ret.mData[0] = (matrix[8] + matrix[2]) * s;
		ret.mData[1] = (matrix[9] + matrix[6]) * s;

		return ret;
	}

	template<typename TDataType>
	Quaternion<TDataType> Quaternion<TDataType>::fromEuler(const TDataType angX, const TDataType angY, const TDataType angZ, AxisOrder axisOrder) noexcept
	{
		auto xRot = Quaternion::fromAxisAngle(kOne<TDataType>, kZero<TDataType>, kZero<TDataType>, angX);
		auto yRot = Quaternion::fromAxisAngle(kZero<TDataType>, kOne<TDataType>, kZero<TDataType>, angY);
		auto zRot = Quaternion::fromAxisAngle(kZero<TDataType>, kZero<TDataType>, kOne<TDataType>, angZ);

		switch (axisOrder)
		{
			case AxisOrder::XYZ:
				xRot *= yRot;
				xRot *= zRot;
				return xRot;
			case AxisOrder::XZY:
				xRot *= zRot;
				xRot *= yRot;
				return xRot;
			case AxisOrder::YXZ:
				yRot *= xRot;
				yRot *= zRot;
				return yRot;
			case AxisOrder::YZX:
				yRot *= zRot;
				yRot *= xRot;
				return yRot;
			case AxisOrder::ZXY:
				zRot *= xRot;
				zRot *= yRot;
				return zRot;
			case AxisOrder::ZYX:
				zRot *= yRot;
				zRot *= xRot;
				return zRot;
			default:
				break;
		}

		return Quaternion::identity();
	}

	template<typename TDataType>
	Quaternion<TDataType> Quaternion<TDataType>::fromVectors(const Vector3Type& from, const Vector3Type& to) noexcept
	{
		//vectors don't need to be normalized

		/*
		* This is a faster version, but doesn't take into account opposite vectors...
		* 
		* Quaternion q(from.crossProduct(to), from.dot(to));
		* q[3] += q.magnitude();
		* q.normalize();
		* 
		* return q;
		* 
		*/

		TDataType norm_u_norm_v = std::sqrt(from.dot(from) * to.dot(to));
		TDataType real_part = norm_u_norm_v + from.dot(to);
		Vector3Type axis;

		if (real_part < (1.e-6f * norm_u_norm_v))
		{
			// if vectors are exactly opposite, rotate 180º around an arbitrary orthogonal axis (axis normalisation can happen later, when we normalise the quaternion)
			real_part = kZero<TDataType>;
			axis = std::abs(from[0]) > std::abs(from[2]) ? Vector3Type(-from[1], from[0], kZero<TDataType>) : Vector3Type(kZero<TDataType>, -from[2], from[1]);
		}
		else
		{
			// otherwise, build quaternion the standard way...
			axis = from.crossProduct(to);
		}

		Quaternion ret;
		ret.mData[0] = axis[0];
		ret.mData[1] = axis[1];
		ret.mData[2] = axis[2];
		ret.mData[3] = real_part;
		ret.normalize();

		return ret;
	}

	template<typename TDataType>
	Quaternion<TDataType> Quaternion<TDataType>::sLerp(const Quaternion& from, const Quaternion& to, const TDataType t) noexcept
	{
		//input quaternions must be normalized

		if (t <= kZero<TDataType>) return from;
		if (t >= kOne<TDataType>) return to;

		double n2;
		double n0;
		double dot = from.dot(to);

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

		Quaternion ret;
		ret.mData[0] = static_cast<TDataType>((n0 * from.mData[0]) + (n2 * to.mData[0]));
		ret.mData[1] = static_cast<TDataType>((n0 * from.mData[1]) + (n2 * to.mData[1]));
		ret.mData[2] = static_cast<TDataType>((n0 * from.mData[2]) + (n2 * to.mData[2]));
		ret.mData[3] = static_cast<TDataType>((n0 * from.mData[3]) + (n2 * to.mData[3]));

		ret.normalize();

		return ret;
	}

	template<typename TDataType>
	Quaternion<TDataType> Quaternion<TDataType>::nLerp(const Quaternion& from, const Quaternion& to, const TDataType t) noexcept
	{
		if (t <= kZero<TDataType>) return from;
		if (t >= kOne<TDataType>) return to;

		Quaternion ret;
		if (from.dot(to) >= kZero<TDataType>)
			ret = (from * (kOne<TDataType> - t)) + (to * t);
		else
			ret = (from * (kOne<TDataType> - t)) - (to * t);

		ret.normalize();

		return ret;
	}

	template<typename TDataType>
	Quaternion<TDataType>& Quaternion<TDataType>::operator+=(const Quaternion &quat) noexcept
	{
		if constexpr (std::is_same_v<TDataType, float>)
		{
			_mm_store_ps(mData, _mm_add_ps(_mm_load_ps(mData), _mm_load_ps(quat.mData)));
		}
		else
		{
			_mm256_store_pd(mData, _mm256_add_pd(_mm256_load_pd(mData), _mm256_load_pd(quat.mData)));
		}

		return *this;
	}

	template<typename TDataType>
	Quaternion<TDataType>& Quaternion<TDataType>::operator-=(const Quaternion &quat) noexcept
	{
		if constexpr (std::is_same_v<TDataType, float>)
		{
			_mm_store_ps(mData, _mm_sub_ps(_mm_load_ps(mData), _mm_load_ps(quat.mData)));
		}
		else
		{
			_mm256_store_pd(mData, _mm256_sub_pd(_mm256_load_pd(mData), _mm256_load_pd(quat.mData)));
		}
		return *this;
	}

	template<typename TDataType>
	Quaternion<TDataType>& Quaternion<TDataType>::operator*=(const Quaternion& quat) noexcept
	{
		Quaternion thiz{*this};

		mData[0] = (quat.mData[3] * thiz.mData[0]) + (quat.mData[0] * thiz.mData[3]) - (quat.mData[1] * thiz.mData[2]) + (quat.mData[2] * thiz.mData[1]);
		mData[1] = (quat.mData[3] * thiz.mData[1]) + (quat.mData[0] * thiz.mData[2]) + (quat.mData[1] * thiz.mData[3]) - (quat.mData[2] * thiz.mData[0]);
		mData[2] = (quat.mData[3] * thiz.mData[2]) - (quat.mData[0] * thiz.mData[1]) + (quat.mData[1] * thiz.mData[0]) + (quat.mData[2] * thiz.mData[3]);
		mData[3] = (quat.mData[3] * thiz.mData[3]) - (quat.mData[0] * thiz.mData[0]) - (quat.mData[1] * thiz.mData[1]) - (quat.mData[2] * thiz.mData[2]);

		return *this;
	}

	template<typename TDataType>
	Quaternion<TDataType>& Quaternion<TDataType>::operator*=(const TDataType scalar) noexcept
	{
		if constexpr (std::is_same_v<TDataType, float>)
		{
			_mm_store_ps(mData, _mm_mul_ps(_mm_load_ps(mData), _mm_load_ps1(&scalar)));
		}
		else
		{
			_mm256_store_pd(mData, _mm256_mul_pd(_mm256_load_pd(mData), _mm256_broadcast_sd(&scalar)));
		}

		return *this;
	}

	template<typename TDataType>
	Quaternion<TDataType>& Quaternion<TDataType>::operator/=(const Quaternion& quat) noexcept
	{
		auto mag = kOne<TDataType> / quat.magnitudeSquared();

		Quaternion inv;
		inv[0] = -quat[0] * mag;
		inv[1] = -quat[1] * mag;
		inv[2] = -quat[2] * mag;
		inv[3] = quat[3] * mag;

		return operator*=(inv);
	}

	template<typename TDataType>
	Quaternion<TDataType> Quaternion<TDataType>::operator+(const Quaternion& quat) const noexcept
	{
		auto res = *this;
		res += quat;
		return res;
	}

	template<typename TDataType>
	Quaternion<TDataType> Quaternion<TDataType>::operator-(const Quaternion& quat) const noexcept
	{
		auto res = *this;
		res -= quat;
		return res;
	}

	template<typename TDataType>
	Quaternion<TDataType> Quaternion<TDataType>::operator*(const Quaternion& quat) const noexcept
	{
		auto res = *this;
		res *= quat;
		return res;
	}

	template<typename TDataType>
	Quaternion<TDataType> Quaternion<TDataType>::operator*(TDataType scalar) const noexcept
	{
		auto res = *this;
		res *= scalar;
		return res;
	}

	template<typename TDataType>
	Quaternion<TDataType> Quaternion<TDataType>::operator/(const Quaternion& quat) const noexcept
	{
		auto res = *this;
		res /= quat;
		return res;
	}

	template<typename TDataType>
	Quaternion<TDataType>& Quaternion<TDataType>::scaleAngle(const TDataType scale) noexcept
	{
		mData[3] *= scale;
		return *this;
	}

	template<typename TDataType>
	Quaternion<TDataType>& Quaternion<TDataType>::conjugate() noexcept
	{
		//since we're dealing with unit quaternions, the conjugate is the same as the inverse
		mData[0] = -mData[0];
		mData[1] = -mData[1];
		mData[2] = -mData[2];

		return *this;
	}

	template<typename TDataType>
	Quaternion<TDataType>& Quaternion<TDataType>::normalize() noexcept
	{
		if constexpr (std::is_same_v<TDataType, float>)
		{
			__m128 vecTmp = _mm_load_ps(mData);
			__m128 vecMag = _mm_rsqrt_ps(_mm_dp_ps(vecTmp, vecTmp, 0xF0 | 0xF));
			_mm_store_ps(mData, _mm_mul_ps(vecTmp, vecMag));
		}
		else
		{
			__m128d temp = _mm_invsqrt_pd(_mm_set1_pd(calcDot(mData)));
			__m256d invMag = _mm256_permute2f128_pd(_mm256_castpd128_pd256(temp), _mm256_castpd128_pd256(temp), 0x20);

			_mm256_store_pd(mData, _mm256_mul_pd(_mm256_load_pd(mData), invMag));
		}

		return *this;
	}

	template<typename TDataType>
	Quaternion<TDataType> Quaternion<TDataType>::getConjugate() const noexcept
	{
		Quaternion res{ *this };
		res.conjugate();

		return res;
	}

	template<typename TDataType>
	TDataType Quaternion<TDataType>::magnitude() const noexcept
	{
		if constexpr (std::is_same_v<TDataType, float>)
		{
			float mag;

			__m128 vecTmp = _mm_load_ps(mData);
			_mm_store_ss(&mag, _mm_sqrt_ss(_mm_dp_ps(vecTmp, vecTmp, 0xF0 | 0xF)));

			return mag;
		}
		else
		{
			__m128d temp = _mm_set1_pd(calcDot(mData));
			return _mm_cvtsd_f64(_mm_sqrt_pd(temp));
		}
	}

	template<typename TDataType>
	TDataType Quaternion<TDataType>::magnitudeSquared() const noexcept
	{
		if constexpr (std::is_same_v<TDataType, float>)
		{
			float mag;

			__m128 vecTmp = _mm_load_ps(mData);
			_mm_store_ss(&mag, _mm_dp_ps(vecTmp, vecTmp, 0xF0 | 0xF));

			return mag;
		}
		else
		{
			return calcDot(mData);
		}
	}

	template<typename TDataType>
	TDataType Quaternion<TDataType>::dot(const Quaternion& quat) const noexcept
	{
		if constexpr (std::is_same_v<TDataType, float>)
		{
			float dot;

			_mm_store_ss(&dot, _mm_dp_ps(_mm_load_ps(mData), _mm_load_ps(quat.mData), 0xF0 | 0xF));
			return dot;
		}
		else
		{
			return calcDot(mData, quat.mData);
		}
	}

	template<typename TDataType>
	void Quaternion<TDataType>::getAxisAngle(TDataType& vecX, TDataType& vecY, TDataType& vecZ, TDataType& ang) const noexcept
	{
		Vector3Type vec;
		getAxisAngle(vec, ang);

		vecX = vec[0];
		vecY = vec[1];
		vecZ = vec[2];
	}

	template<typename TDataType>
	void Quaternion<TDataType>::getAxisAngle(Vector3Type& vec, TDataType& ang) const noexcept
	{
		auto len = mData[0] * mData[0] + mData[1] * mData[1] + mData[2] * mData[2];
		if (Math::isZero(len))
		{
			vec[0] = kOne<TDataType>;
			vec[1] = kZero<TDataType>;
			vec[2] = kZero<TDataType>;
			ang = kZero<TDataType>;
			return;
		}

		vec = Vector3Type{mData[0], mData[1], mData[2]};
		vec *= (kOne<TDataType> / len);
		vec.normalize();

		ang = std::acos(mData[3]) * kTwo<TDataType> * Math::Rad2Deg<TDataType>;
	}

	template<typename TDataType>
	void Quaternion<TDataType>::getEulerAngles(TDataType& angX, TDataType& angY, TDataType& angZ) const noexcept
	{
		if constexpr (std::is_same_v<TDataType, float>)
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
		else
		{
			double test = (mData[0] * mData[3]) - (mData[1] * mData[2]);
			if (test > 0.4995) // singularity at north pole
			{
				angY = 2.0 * std::atan2(mData[1], mData[0]);
				angX = Math::Pi<double> * 2.0f;
				angZ = 0.0;
				return;
			}
			if (test < -0.4995) // singularity at south pole
			{
				angY = -2.0 * std::atan2(mData[1], mData[0]);
				angX = -Math::Pi<double> * 2.0f;
				angZ = 0.0;
				return;
			}

			// yaw
			angY = std::atan2((2.0 * mData[3] * mData[1]) + (2.0 * mData[2] * mData[0]), 1.0 - (2.0 * (mData[0] * mData[0] + mData[1] * mData[1])));

			// pitch
			angX = std::asin(2.0 * (mData[3] * mData[0] - mData[1] * mData[2]));

			// roll
			angZ = std::atan2((2.0 * mData[3] * mData[2]) + (2.0 * mData[0] * mData[1]), 1.0 - (2.0 * (mData[2] * mData[2] + mData[0] * mData[0])));
		}
	}

	template<typename TDataType>
	Quaternion<TDataType>::Vector3Type Quaternion<TDataType>::unitRotate(const Vector3Type& vec) const noexcept
	{
		//this only works if this quaternion is normalized (a unit quaternion)

		Vector3Type u{mData[0], mData[1], mData[2]};

		Vector3Type res;
		res = u * kTwo<TDataType> * u.dot(vec);
		res += vec * ((mData[3] * mData[3]) - u.dot(u));
		res += u.crossProduct(vec) * (kTwo<TDataType> * mData[3]);

		return res;
	}

	template<typename TDataType>
	void Quaternion<TDataType>::unitRotate(const Vector3Type& vec, Vector3Type& dest) const noexcept
	{
		dest = unitRotate(vec);
	}

	template class Quaternion<float>;
	template class Quaternion<double>;
}
