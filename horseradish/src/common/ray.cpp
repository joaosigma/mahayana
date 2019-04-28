#include "ray.hpp"

#include "math.hpp"

#include <type_traits>

namespace hr
{
	static_assert(std::is_trivially_copyable<Ray>::value);

	Ray& Ray::setOrigin(float x, float y, float z)
	{
		mOrigin[0] = x;
		mOrigin[1] = y;
		mOrigin[2] = z;
		return *this;
	}

	Ray& Ray::setOrigin(const Vector3f& origin)
	{
		mOrigin = origin;
		return *this;
	}

	Ray& Ray::setDirection(float x, float y, float z)
	{
		mDirection[0] = x;
		mDirection[1] = y;
		mDirection[2] = z;
		return *this;
	}

	Ray& Ray::setDirection(const Vector3f& direction)
	{
		mDirection[0] = direction[0];
		mDirection[1] = direction[1];
		mDirection[2] = direction[2];
		return *this;
	}

	Ray& Ray::setLength(float length)
	{
		mLength = std::abs(length);
		return *this;
	}

	Ray& Ray::setSizedDirection(float x, float y, float z)
	{
		mDirection.set(x, y, z);
		mLength = mDirection.getMagnitude();
		mDirection.normalize();
		return *this;
	}

	Ray& Ray::setSizedDirection(const Vector3f& sizedDirection)
	{
		mDirection = sizedDirection;
		mDirection.normalize();

		mLength = sizedDirection.getMagnitude();
		return *this;
	}
}