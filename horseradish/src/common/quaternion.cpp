#include "quaternion.hpp"

#include "math.hpp"

namespace hr
{
	void Quaternion::operator+=(const Quaternion &quat)
	{
		_mm_storeu_ps(mData, _mm_add_ps(_mm_loadu_ps(mData), _mm_loadu_ps(quat.mData)));
	}

	void Quaternion::operator-=(const Quaternion &quat)
	{
		_mm_storeu_ps(mData, _mm_sub_ps(_mm_loadu_ps(mData), _mm_loadu_ps(quat.mData)));
	}

	void Quaternion::operator*=(const Quaternion &quat)
	{
		float qX = mData[0];
		float qY = mData[1];
		float qZ = mData[2];

		mData[0] = (mData[3] * quat.mData[0]) + (qX * quat.mData[3]) + (qY * quat.mData[2]) - (qZ * quat.mData[1]);
		mData[1] = (mData[3] * quat.mData[1]) - (qX * quat.mData[2]) + (qY * quat.mData[3]) + (qZ * quat.mData[0]);
		mData[2] = (mData[3] * quat.mData[2]) + (qX * quat.mData[1]) - (qY * quat.mData[0]) + (qZ * quat.mData[3]);
		mData[3] = (mData[3] * quat.mData[3]) - (qX * quat.mData[0]) - (qY * quat.mData[1]) - (qZ * quat.mData[2]);
	}

	void Quaternion::operator*=(const float &scalar)
	{
		_mm_storeu_ps(mData, _mm_mul_ps(_mm_loadu_ps(mData), _mm_load_ps1(&scalar)));
	}

	void Quaternion::operator/=(const Quaternion &quat)
	{
		Quaternion q, r, s;

		q.mData[0] = -quat.mData[0];
		q.mData[1] = -quat.mData[1];
		q.mData[2] = -quat.mData[2];
		q.mData[3] = quat.mData[3];

		r.mData[0] = mData[0];
		r.mData[1] = mData[1];
		r.mData[2] = mData[2];
		r.mData[3] = mData[3];
		r *= q;

		s.mData[0] = q.mData[0];
		s.mData[1] = q.mData[1];
		s.mData[2] = q.mData[2];
		s.mData[3] = q.mData[3];
		s *= q;

		mData[0] = r.mData[0] / s.mData[3];
		mData[1] = r.mData[1] / s.mData[3];
		mData[2] = r.mData[2] / s.mData[3];
		mData[3] = r.mData[3] / s.mData[3];
	}

	void Quaternion::setAxisAngle(const float &vx, const float &vy, const float &vz, const float &angleDeg)
	{
		float angleRad = (angleDeg*0.017453292519943295f)*0.5f;

		float resSin;
		Math::sinCos(angleRad, resSin, mData[3]);

		mData[0] = vx*resSin;
		mData[1] = vy*resSin;
		mData[2] = vz*resSin;
	}

	void Quaternion::setAxisAngle(const Vector3f &vec, const float &angleDeg)
	{
		float angleRad = (angleDeg*0.017453292519943295f)*0.5f;

		float resSin;
		Math::sinCos(angleRad, resSin, mData[3]);

		mData[0] = vec[0] * resSin;
		mData[1] = vec[1] * resSin;
		mData[2] = vec[2] * resSin;
	}

	void Quaternion::setFromMatrix3x3(const float * const matrix)
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
			return;
		}

		if ((matrix[4] <= matrix[0]) && (matrix[8] <= matrix[0]))
		{
			s = Math::sqrt((matrix[0] - (matrix[4] + matrix[8])) + 1.0f);

			mData[0] = s*0.5f;
			s = 0.5f / s;
			mData[1] = (matrix[1] + matrix[3])*s;
			mData[2] = (matrix[2] + matrix[6])*s;
			mData[3] = (matrix[5] - matrix[7])*s;
			return;
		}

		if ((matrix[4] > matrix[0]) && (matrix[8] <= matrix[4]))
		{
			s = Math::sqrt((matrix[4] - (matrix[8] + matrix[0])) + 1.0f);

			mData[1] = s*0.5f;
			s = 0.5f / s;
			mData[3] = (matrix[6] - matrix[2])*s;
			mData[2] = (matrix[5] + matrix[7])*s;
			mData[0] = (matrix[3] + matrix[1])*s;
			return;
		}

		s = Math::sqrt((matrix[8] - (matrix[0] + matrix[4])) + 1.0f);

		mData[2] = s*0.5f;
		s = 0.5f / s;
		mData[3] = (matrix[1] - matrix[3])*s;
		mData[0] = (matrix[6] + matrix[2])*s;
		mData[1] = (matrix[7] + matrix[5])*s;
	}

	void Quaternion::setFromMatrix4x4(const float * const matrix)
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
			return;
		}

		if ((matrix[5] <= matrix[0]) && (matrix[10] <= matrix[0]))
		{
			s = Math::sqrt((matrix[0] - (matrix[5] + matrix[10])) + 1.0f);

			mData[0] = s*0.5f;
			s = 0.5f / s;
			mData[1] = (matrix[1] + matrix[4])*s;
			mData[2] = (matrix[2] + matrix[8])*s;
			mData[3] = (matrix[6] - matrix[9])*s;
			return;
		}

		if ((matrix[5] > matrix[0]) && (matrix[10] <= matrix[5]))
		{
			s = Math::sqrt((matrix[5] - (matrix[10] + matrix[0])) + 1.0f);

			mData[1] = s*0.5f;
			s = 0.5f / s;
			mData[3] = (matrix[8] - matrix[2])*s;
			mData[2] = (matrix[6] + matrix[9])*s;
			mData[0] = (matrix[4] + matrix[1])*s;
			return;
		}

		s = Math::sqrt((matrix[10] - (matrix[0] + matrix[5])) + 1.0f);

		mData[2] = s*0.5f;
		s = 0.5f / s;
		mData[3] = (matrix[1] - matrix[4])*s;
		mData[0] = (matrix[8] + matrix[2])*s;
		mData[1] = (matrix[9] + matrix[6])*s;
	}

	void Quaternion::setFromEuler(const float &angX, const float &angY, const float &angZ)
	{
		float cosR, cosP, cosY, sinR, sinP, sinY, cpcy, spsy;

		Math::sinCos(angX*0.0087266462599716478846184f, sinR, cosR);
		Math::sinCos(angY*0.0087266462599716478846184f, sinP, cosP);
		Math::sinCos(angZ*0.0087266462599716478846184f, sinY, cosY);
		spsy = sinP * sinY;
		cpcy = cosP * cosY;

		mData[3] = cosR * cpcy + sinR * spsy;
		mData[0] = sinR * cpcy - cosR * spsy;
		mData[1] = cosR * sinP * cosY + sinR * cosP * sinY;
		mData[2] = cosR * cosP * sinY - sinR * sinP * cosY;

		normalize();
	}

	void Quaternion::setSLerp(const Quaternion &from, const Quaternion &to, const float &t)
	{
		float c, s;

		float dot = from.getDot(to);
		if (dot > 0.99999f)
		{
			mData[0] = from.mData[0] + (to.mData[0] + from.mData[0]) * t;
			mData[1] = from.mData[1] + (to.mData[1] + from.mData[1]) * t;
			mData[2] = from.mData[2] + (to.mData[2] + from.mData[2]) * t;
			mData[3] = from.mData[3] + (to.mData[3] + from.mData[3]) * t;
			normalize();
			return;
		}

		dot = Math::fClamp(dot, -1.0f, 1.0f);
		Math::sinCos(acosf(dot)*t, s, c);

		Quaternion qAux;
		qAux.mData[0] = to.mData[0] - from.mData[0] * dot;
		qAux.mData[1] = to.mData[1] - from.mData[1] * dot;
		qAux.mData[2] = to.mData[2] - from.mData[2] * dot;
		qAux.mData[3] = to.mData[3] - from.mData[3] * dot;
		qAux.normalize();

		mData[0] = from.mData[0] * c + qAux.mData[0] * s;
		mData[1] = from.mData[1] * c + qAux.mData[1] * s;
		mData[2] = from.mData[2] * c + qAux.mData[2] * s;
		mData[3] = from.mData[3] * c + qAux.mData[3] * s;
	}

	void Quaternion::setLerp(const Quaternion &from, const Quaternion &to, const float &t)
	{
		float to1[4], cosom, scale0, scale1;

		cosom = from.mData[0] * to.mData[0] + from.mData[1] * to.mData[1] + from.mData[2] * to.mData[2] + from.mData[3] * to.mData[3];

		if (cosom < 0.0f)
		{
			to1[0] = -to.mData[0];
			to1[1] = -to.mData[1];
			to1[2] = -to.mData[2];
			to1[3] = -to.mData[3];
		}
		else
		{
			to1[0] = to.mData[0];
			to1[1] = to.mData[1];
			to1[2] = to.mData[2];
			to1[3] = to.mData[3];
		}

		scale0 = 1.0f - t;
		scale1 = t;

		mData[0] = scale0*from.mData[0] + scale1*to1[0];
		mData[1] = scale0*from.mData[1] + scale1*to1[1];
		mData[2] = scale0*from.mData[2] + scale1*to1[2];
		mData[3] = scale0*from.mData[3] + scale1*to1[3];
	}

	void Quaternion::set(const float &nx, const float &ny, const float &nz, const float &nw)
	{
		mData[0] = nx;
		mData[1] = ny;
		mData[2] = nz;
		mData[3] = nw;
	}

	void Quaternion::set(const Vector3f &vec, const float &nw)
	{
		mData[0] = vec[0];
		mData[1] = vec[1];
		mData[2] = vec[2];
		mData[3] = nw;
	}

	void Quaternion::setAngle(const float &nx, const float &ny, const float &nz, const float &angleDeg)
	{
		mData[0] = nx;
		mData[1] = ny;
		mData[2] = nz;
		mData[3] = cos(angleDeg*0.00872664625997164788461845384f);	//also divides by 2
	}

	void Quaternion::setAngle(const Vector3f &vec, const float &angleDeg)
	{
		mData[0] = vec[0];
		mData[1] = vec[1];
		mData[2] = vec[2];
		mData[3] = cos(angleDeg*0.00872664625997164788461845384f);	//also divides by 2
	}

	void Quaternion::set(const Quaternion &quat)
	{
		std::memcpy(mData, quat.mData, sizeof(float) * 4);
	}

	void Quaternion::scaleAngle(float scale)
	{
		mData[3] *= scale;
	}

	void Quaternion::invert()
	{
		mData[0] = -mData[0];
		mData[1] = -mData[1];
		mData[2] = -mData[2];
	}

	void Quaternion::normalize()
	{
		__m128 vecTmp = _mm_loadu_ps(mData);
		__m128 vecMag = _mm_rsqrt_ps(_mm_dp_ps(vecTmp, vecTmp, 0xF0 | 0xF));
		_mm_storeu_ps(mData, _mm_mul_ps(vecTmp, vecMag));
	}

	void Quaternion::mulEulerAngles(float angX, float angY, float angZ)
	{
		float auxX, auxY, auxZ, auxW;
		float degX, degY, degZ;
		float cosR, cosP, cosY, sinR, sinP, sinY, cpcy, spsy;

		Math::sinCos(angX*0.0087266462599716478846184f, sinR, cosR);
		Math::sinCos(angY*0.0087266462599716478846184f, sinP, cosP);
		Math::sinCos(angZ*0.0087266462599716478846184f, sinY, cosY);
		spsy = sinP * sinY;
		cpcy = cosP * cosY;

		auxW = cosR * cpcy + sinR * spsy;
		auxX = sinR * cpcy - cosR * spsy;
		auxY = cosR * sinP * cosY + sinR * cosP * sinY;
		auxZ = cosR * cosP * sinY - sinR * sinP * cosY;

		spsy = 1.0f / sqrt(auxX*auxX + auxY*auxY + auxZ*auxZ + auxW*auxW);
		auxX *= spsy;
		auxY *= spsy;
		auxZ *= spsy;
		auxW *= spsy;

		degX = mData[0];
		degY = mData[1];
		degZ = mData[2];
		cosP = mData[3];

		mData[0] = cosP*auxX + degX*auxW + degY*auxZ - degZ*auxY;
		mData[1] = cosP*auxY - degX*auxZ + degY*auxW + degZ*auxX;
		mData[2] = cosP*auxZ + degX*auxY - degY*auxX + degZ*auxW;
		mData[3] = cosP*auxW - degX*auxX - degY*auxY - degZ*auxZ;
	}

	void Quaternion::expandW()
	{
		float term;

		term = 1.0f - (mData[0] * mData[0]) - (mData[1] * mData[1]) - (mData[2] * mData[2]);
		mData[3] = 0.0f;
		if (term >= 0.0f)
			mData[3] = -(float)sqrt(term);
	}

	void Quaternion::expandWNormalize()
	{
		float auxF;

		auxF = 1.0f - (mData[0] * mData[0]) - (mData[1] * mData[1]) - (mData[2] * mData[2]);
		mData[3] = 0.0f;
		if (auxF >= 0.0f)
			mData[3] = -(float)sqrt(auxF);

		auxF = 1.0f / sqrt((auxF - 1.0f)*(-1.0f) + mData[3] * mData[3]);
		mData[0] *= auxF;
		mData[1] *= auxF;
		mData[2] *= auxF;
		mData[3] *= auxF;
	}

	float Quaternion::getDot(const Quaternion &quat) const
	{
		float final;

		__m128 qTmp = _mm_loadu_ps(mData);
		_mm_store_ss(&final, _mm_dp_ps(qTmp, qTmp, 0xF0 | 0xF));
		return final;
	}

	void Quaternion::getVector(float * const vec) const
	{
		vec[0] = mData[0];
		vec[1] = mData[1];
		vec[2] = mData[2];
	}

	void Quaternion::getVector(Vector3f &vec) const
	{
		vec.set(mData);
	}

	void Quaternion::getMatrix3x3(float* const matrix) const
	{
		float xx, yy, zz, wx, wy, wz, xy, xz, yz;

		xx = mData[0] * mData[0];
		yy = mData[1] * mData[1];
		zz = mData[2] * mData[2];
		wx = mData[3] * mData[3];
		wy = mData[3] * mData[1];
		wz = mData[3] * mData[2];
		xy = mData[0] * mData[1];
		xz = mData[0] * mData[2];
		yz = mData[1] * mData[2];

		matrix[0] = 1.0f - 2.0f * (yy + zz);
		matrix[1] = 2.0f * (xy - wz);
		matrix[2] = 2.0f * (xz + wy);

		matrix[3] = 2.0f * (xy + wz);
		matrix[4] = 1.0f - 2.0f * (xx + zz);
		matrix[5] = 2.0f * (yz - wx);

		matrix[6] = 2.0f * (xz - wy);
		matrix[7] = 2.0f * (yz + wx);
		matrix[8] = 1.0f - 2.0f * (xx + yy);
	}

	void Quaternion::getMatrix4x4(float* const matrix) const
	{
		matrix[0] = 1.0f - 2.0f * (mData[1] * mData[1] + mData[2] * mData[2]);
		matrix[1] = 2.0f * (mData[0] * mData[1] - mData[3] * mData[2]);
		matrix[2] = 2.0f * (mData[0] * mData[2] + mData[3] * mData[1]);
		matrix[3] = 0.0f;

		matrix[4] = 2.0f * (mData[0] * mData[1] + mData[3] * mData[2]);
		matrix[5] = 1.0f - 2.0f * (mData[0] * mData[0] + mData[2] * mData[2]);
		matrix[6] = 2.0f * (mData[1] * mData[2] - mData[3] * mData[0]);
		matrix[7] = 0.0f;

		matrix[8] = 2.0f * (mData[0] * mData[2] - mData[3] * mData[1]);
		matrix[9] = 2.0f * (mData[1] * mData[2] + mData[3] * mData[0]);
		matrix[10] = 1.0f - 2.0f * (mData[0] * mData[0] + mData[1] * mData[1]);
		matrix[11] = 0.0f;

		matrix[12] = 0.0f;
		matrix[13] = 0.0f;
		matrix[14] = 0.0f;
		matrix[15] = 1.0f;
	}

	void Quaternion::getAxisAngle(float* const vecX, float* const vecY, float* const vecZ, float* const ang) const
	{
		float auxX, auxY, auxZ;

		float len = mData[0] * mData[0] + mData[1] * mData[1] + mData[2] * mData[2];
		if (len == 0.0f)
		{
			*vecX = 0.0f;
			*vecY = 0.0f;
			*vecZ = 1.0f;
			*ang = 0.0f;
			return;
		}

		len = 1.0f / len;
		auxX = mData[0] * len;
		auxY = mData[1] * len;
		auxZ = mData[2] * len;

		len = 1.0f / sqrt(auxX*auxX + auxY*auxY + auxZ*auxZ);
		*vecX = auxX*len;
		*vecY = auxY*len;
		*vecZ = auxZ*len;

		*ang = ((float)acos(mData[3]))*114.5915590261646417f; // 180/pi=57.295779513082320876f * 2.0f
	}

	void Quaternion::getAxisAngle(Vector3f &vec, float * const ang) const
	{
		float len = mData[0] * mData[0] + mData[1] * mData[1] + mData[2] * mData[2];
		if (len == 0.0f)
		{
			vec[0] = 0.0f;
			vec[1] = 0.0f;
			vec[2] = 1.0f;
			*ang = 0.0f;
			return;
		}

		vec.set(mData);
		vec *= (1.0f / len);
		vec.normalize();

		*ang = ((float)acos(mData[3]))*114.5915590261646417f; // 180/pi=57.295779513082320876f * 2.0f
	}

	void Quaternion::getEulerAngles(float * const angX, float * const angY, float * const angZ) const
	{
		//TODO
	}

	void Quaternion::setFromVectors(const Vector3f &v1, const Vector3f &v2)
	{
		Vector3f t;

		// get dot product of two vectors
		float cost = Vector3f::calcDot(v1, v2);

		// check if parallel
		if (cost > 0.99999f)
		{
			mData[0] = mData[1] = mData[2] = 0.0f;
			mData[3] = 1.0f;
			return;
		}
		// check if opposite
		else if (cost < -0.99999f)
		{
			// check if we can use cross product of from vector with [1, 0, 0]
			t.set(0.0, v1[0], -v1[1]);

			if (t.getDot() < 1e-6) // nope! we need cross product of from vector with [0, 1, 0]
				t.set(-v1[2], 0.0, v1[0]);

			// normalize
			t.normalize();

			mData[0] = t[0];
			mData[1] = t[1];
			mData[2] = t[2];
			mData[3] = 0.0;
			return;
		}

		// ... else we can just cross two vectors
		t = v1.crossProduct(v2);
		t.normalize();

		// we have to use half-angle formulae (sin^2 t = ( 1 - cos (2t) ) /2)
		t *= sqrt(0.5f * (1.0f - cost));

		// scale the axis to get the normalized quaternion
		mData[0] = t[0];
		mData[1] = t[1];
		mData[2] = t[2];

		// cos^2 t = ( 1 + cos (2t) ) / 2
		// w part is cosine of half the rotation angle
		mData[3] = sqrt(0.5f * (1.0f + cost));
	}

	void Quaternion::setIdentity()
	{
		mData[0] = mData[1] = mData[2] = 0.0f;
		mData[3] = 1.0f;
	}

	void Quaternion::rotateVector3(Vector3f &vec) const
	{
		rotateVector3(vec, vec);
	}

	void Quaternion::rotateVector3(const Vector3f &vec, Vector3f &dest) const
	{
		float auxX, auxY, auxZ, auxW, W;

		auxX = -(mData[3] * vec[0] + mData[1] * vec[2] - mData[2] * vec[1]);
		auxY = -(mData[3] * vec[1] - mData[0] * vec[2] + mData[2] * vec[0]);
		auxZ = -(mData[3] * vec[2] + mData[0] * vec[1] - mData[1] * vec[0]);
		auxW = -(mData[0] * vec[0] - mData[1] * vec[1] - mData[2] * vec[2]);
		auxW = mData[0] * vec[0] + mData[1] * vec[1] + mData[2] * vec[2];

		W = -mData[3];
		dest[0] = auxW*mData[0] + auxX*W + auxY*mData[2] - auxZ*mData[1];
		dest[1] = auxW*mData[1] - auxX*mData[2] + auxY*W + auxZ*mData[0];
		dest[2] = auxW*mData[2] + auxX*mData[1] - auxY*mData[0] + auxZ*W;
	}

	void Quaternion::rotateVector3(const float * const vec, float * const dest) const
	{
		float auxX, auxY, auxZ, auxW, W;

		auxX = -(mData[3] * vec[0] + mData[1] * vec[2] - mData[2] * vec[1]);
		auxY = -(mData[3] * vec[1] - mData[0] * vec[2] + mData[2] * vec[0]);
		auxZ = -(mData[3] * vec[2] + mData[0] * vec[1] - mData[1] * vec[0]);
		auxW = -(mData[0] * vec[0] - mData[1] * vec[1] - mData[2] * vec[2]);
		auxW = mData[0] * vec[0] + mData[1] * vec[1] + mData[2] * vec[2];

		W = -mData[3];
		dest[0] = auxW*mData[0] + auxX*W + auxY*mData[2] - auxZ*mData[1];
		dest[1] = auxW*mData[1] - auxX*mData[2] + auxY*W + auxZ*mData[0];
		dest[2] = auxW*mData[2] + auxX*mData[1] - auxY*mData[0] + auxZ*W;
	}

	void Quaternion::rotateVector3(const float &vx, const float &vy, const float &vz, Vector3f &dest) const
	{
		rotateVector3(Vector3f(vx, vy, vz), dest);
	}
}