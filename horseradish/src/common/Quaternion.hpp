#pragma once
#ifndef __HQUATERNION__
#define __HQUATERNION__

#include "Types.hpp"
#include "Vector.hpp"

namespace HorseRadish
{

HALIGN_16BYTES
class Quaternion{

	float x,y,z,w;

public:

	Quaternion() { x = y = z = 0.0f; w = 1.0f; }
	~Quaternion() { return; }

	explicit Quaternion(Quaternion const &quat)
		{ x = quat.x; y = quat.y; z = quat.z; w = quat.w; }
	explicit Quaternion(const float qx, const float qy, const float qz, const float qw) 
		{ x = qx; y = qy; z = qz; w = qw; }

	inline operator const float *(void) const { return &x; }

	void operator+=(const Quaternion &quat);
	void operator-=(const Quaternion &quat);
	void operator*=(const Quaternion &quat);
	void operator*=(const float &scalar);
	void operator/=(const Quaternion &quat);

	void SetAxisAngle(const float &vx, const float &vy, const float &vz, const float &angulo);
	void SetAxisAngle(const Vector &vec, const float &angulo);
	void SetFromMatrix3x3(const float * const matrix);
	void SetFromMatrix4x4(const float * const matrix);
	void SetFromEuler(const float &angX, const float &angY, const float &angZ);
	void SetSLerp(const Quaternion &from, const Quaternion &to, const float &t);
	void SetLerp(const Quaternion &from, const Quaternion &to, const float &t);
	void Set(const float &nx, const float &ny, const float &nz, const float &nw);
	void Set(const Vector &vec, const float &nw);
	void Set(const Quaternion &quat);
	void SetAngle(const float &nx, const float &ny, const float &nz, const float &angulo);
	void SetAngle(const Vector &vec, const float &angulo);
	void SetFromVectors(const Vector &v1, const Vector &v2);
	void SetX(const float &nx) { x = nx; }
	void SetY(const float &ny) { y = ny; }
	void SetZ(const float &nz) { z = nz; }

	void  ScaleAngle(const float &scale);
	void  Invert();
	void  Normaliza();
	void  RotateVector(Vector &vec) const;
	void  RotateVector(const Vector &vec, Vector &dest) const;
	void  RotateVector(const float * const vec, float * const dest) const;
	void  RotateVector(const float &vx, const float &vy, const float &vz, Vector &dest) const;
	void  RotateVectorAdd(Vector &vec, const Vector &vecAdd) const;
	void  RotateVectorAdd(Vector &dest, const Vector &vec, const Vector &vecAdd) const;
	float Dot(const Quaternion &quat) const;
	void  Identidade();
	void  MulEulerAngles(const float &angX, const float &angY, const float &angZ);
	void  ExpandW();
	void  ExpandWNormaliza();

	void GetVector(float * const vec) const;
	void GetVector(Vector &vec) const;
	void GetMatrix3x3(float * const matrix) const;
	void GetMatrix4x4(float * const matrix) const;
	void GetAxisAngle(float * const vecX, float * const vecY, float * const vecZ, float * const ang) const;
	void GetAxisAngle(Vector &vec, float * const ang) const;
	void GetEulerAngles(float * const angX, float * const angY, float * const angZ) const;
};

}//namespace HorseRadish

#endif