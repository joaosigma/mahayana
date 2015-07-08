#include "Ray.hpp"

#include "Math.hpp"

namespace HorseRadish
{
	Ray::Ray()
	{
		origin.set(0.0f);
		direction[0] = direction[1] = direction[2] = 0.0f;
		length = 0.0f;
	}

	Ray::Ray(const Vector3f &nOrigin, const Vector3f &nDirection)
	{
		origin = nOrigin;
		direction[0] = nDirection[0];
		direction[1] = nDirection[1];
		direction[2] = nDirection[2];
		length = Math::normalizeRet(direction);
	}

	Ray::Ray(const Ray &ray)
	{
		origin = ray.origin;
		direction[0] = ray.direction[0];
		direction[1] = ray.direction[1];
		direction[2] = ray.direction[2];
		length = ray.length;
	}

	void Ray::SetOrigin(const float &x, const float &y, const float &z)
	{
		origin[0] = x;
		origin[1] = y;
		origin[2] = z;
	}

	void Ray::SetOrigin(const Vector3f &nOrigin)
	{
		origin = nOrigin;
	}

	void Ray::SetDirection(const float &x, const float &y, const float &z)
	{
		direction[0] = x;
		direction[1] = y;
		direction[2] = z;
	}

	void Ray::SetDirection(const Vector3f &nDirection)
	{
		direction[0] = nDirection[0];
		direction[1] = nDirection[1];
		direction[2] = nDirection[2];
	}

	void Ray::SetLength(const float &nLength)
	{
		length = Math::fAbs(nLength);
	}

	void Ray::SetSizedDirection(const float &x, const float &y, const float &z)
	{
		direction[0] = x;
		direction[1] = y;
		direction[2] = z;
		length = Math::normalizeRet(direction);
	}

	void Ray::SetSizedDirection(const Vector3f &nSizedDirection)
	{
		direction[0] = nSizedDirection[0];
		direction[1] = nSizedDirection[1];
		direction[2] = nSizedDirection[2];
		length = Math::normalizeRet(direction);
	}

	void Ray::GetPointOn(const float &t, Vector3f &write) const
	{
		write[0] = origin[0] + (direction[0] * t);
		write[1] = origin[1] + (direction[1] * t);
		write[2] = origin[2] + (direction[2] * t);
	}

	Vector3f Ray::GetPointOn(const float &t) const
	{
		return Vector3f(origin[0] + (direction[0] * t), origin[1] + (direction[1] * t), origin[2] + (direction[2] * t));
	}

	void Ray::Normalize()
	{
		length = Math::normalizeRet(direction);
	}

} //HorseRadish