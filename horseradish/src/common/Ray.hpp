#pragma once

#include "Vector.hpp"

namespace HorseRadish
{
	class Ray
	{
		Vector3f origin;
		float direction[3], length;

	public:

		Ray();
		explicit Ray(const Vector3f &nOrigin, const Vector3f &nDirection);
		explicit Ray(const Ray &ray);

		Ray& operator=(const Ray& ray) { this->origin = ray.origin; this->direction[0] = ray.direction[0]; this->direction[1] = ray.direction[1]; this->direction[2] = ray.direction[2]; this->length = ray.length; return *this; }

		void SetOrigin(const float &x, const float &y, const float &z);
		void SetOrigin(const Vector3f &nOrigin);
		void SetDirection(const float &x, const float &y, const float &z);
		void SetDirection(const Vector3f &nDirection);
		void SetLength(const float &nLength);
		void SetSizedDirection(const float &x, const float &y, const float &z);
		void SetSizedDirection(const Vector3f &nSizedDirection);

		void  GetOrigin(Vector3f &rOrigin) const { rOrigin.set(this->origin); }
		Vector3f GetOrigin() const { return Vector3f(this->origin); }
		void  GetDirection(Vector3f &rDirection) const { rDirection.set(this->direction); }
		Vector3f GetDirection() const { return Vector3f(this->direction); }
		float GetLength() const { return this->length; }
		void  GetPointOn(const float &t, Vector3f &write) const;
		Vector3f GetPointOn(const float &t) const;

		float GetDotOrigin(const float &x, const float &y, const float &z) const { return this->origin.getDot(x, y, z); }
		float GetDotOrigin(const Vector3f &vec) const { return origin.getDot(vec); }
		float GetDotDirection(const float &x, const float &y, const float &z) const { return ((this->direction[0] * x) + (this->direction[1] * y) + (this->direction[2] * z)); }
		float GetDotDirection(const Vector3f &vec) const { return vec.getDot(this->direction); }

		void NegateDir() { this->direction[0] *= -1.0f; this->direction[1] *= -1.0f; this->direction[2] *= -1.0f; }
		void Normalize();
	};

} //HorseRadish
