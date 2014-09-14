#pragma once

#include "Vector.hpp"

namespace HorseRadish
{
	class Ray
	{
		Vector origin;
		float direction[3], length;

	public:

		Ray();
		~Ray(){}
		explicit Ray(const Vector &nOrigin, const Vector &nDirection);
		explicit Ray(const Ray &ray);

		Ray& operator=(const Ray& ray) { this->origin = ray.origin; this->direction[0] = ray.direction[0]; this->direction[1] = ray.direction[1]; this->direction[2] = ray.direction[2]; this->length = ray.length; return *this; }

		void SetOrigin(const float &x, const float &y, const float &z);
		void SetOrigin(const Vector &nOrigin);
		void SetDirection(const float &x, const float &y, const float &z);
		void SetDirection(const Vector &nDirection);
		void SetLength(const float &nLength);
		void SetSizedDirection(const float &x, const float &y, const float &z);
		void SetSizedDirection(const Vector &nSizedDirection);

		void  GetOrigin(Vector &rOrigin) const { rOrigin.Set(this->origin); }
		Vector GetOrigin() const { return Vector(this->origin); }
		void  GetDirection(Vector &rDirection) const { rDirection.Set(this->direction); }
		Vector GetDirection() const { return Vector(this->direction); }
		float GetLength() const { return this->length; }
		void  GetPointOn(const float &t, Vector &write) const;
		void  GetPointOn(const float &t, Vector * const write) const;
		Vector GetPointOn(const float &t) const;

		float GetDotOrigin(const float &x, const float &y, const float &z) const { return this->origin.GetDot(x, y, z); }
		float GetDotOrigin(const Vector &vec) const { return origin.GetDot(vec); }
		float GetDotDirection(const float &x, const float &y, const float &z) const { return ((this->direction[0] * x) + (this->direction[1] * y) + (this->direction[2] * z)); }
		float GetDotDirection(const Vector &vec) const { return vec.GetDot(this->direction); }

		void NegateDir() { this->direction[0] *= -1.0f; this->direction[1] *= -1.0f; this->direction[2] *= -1.0f; }
		void Normalize();
	};

} //HorseRadish
