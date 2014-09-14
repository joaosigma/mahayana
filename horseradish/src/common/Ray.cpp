#include "Ray.hpp"

#include "Math.hpp"

namespace HorseRadish
{
	Ray::Ray()
	{
		origin.Set(0.0f);
		direction[0] = direction[1] = direction[2] = 0.0f;
		length = 0.0f;
	}

	Ray::Ray(const Vector &nOrigin, const Vector &nDirection)
	{
		origin = nOrigin;
		direction[0] = nDirection.x;
		direction[1] = nDirection.y;
		direction[2] = nDirection.z;
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
		origin.x = x;
		origin.y = y;
		origin.z = z;
	}

	void Ray::SetOrigin(const Vector &nOrigin)
	{
		origin = nOrigin;
	}

	void Ray::SetDirection(const float &x, const float &y, const float &z)
	{
		direction[0] = x;
		direction[1] = y;
		direction[2] = z;
	}

	void Ray::SetDirection(const Vector &nDirection)
	{
		direction[0] = nDirection.x;
		direction[1] = nDirection.y;
		direction[2] = nDirection.z;
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

	void Ray::SetSizedDirection(const Vector &nSizedDirection)
	{
		direction[0] = nSizedDirection.x;
		direction[1] = nSizedDirection.y;
		direction[2] = nSizedDirection.z;
		length = Math::normalizeRet(direction);
	}

	void Ray::GetPointOn(const float &t, Vector &write) const
	{
		write.x = origin.x + (direction[0] * t);
		write.y = origin.y + (direction[1] * t);
		write.z = origin.z + (direction[2] * t);
	}

	void Ray::GetPointOn(const float &t, Vector * const write) const
	{
		write->x = origin.x + (direction[0] * t);
		write->y = origin.y + (direction[1] * t);
		write->z = origin.z + (direction[2] * t);
	}

	Vector Ray::GetPointOn(const float &t) const
	{
		return Vector(origin.x + (direction[0] * t), origin.y + (direction[1] * t), origin.z + (direction[2] * t));
	}

	void Ray::Normalize()
	{
		length = Math::normalizeRet(direction);
	}

} //HorseRadish