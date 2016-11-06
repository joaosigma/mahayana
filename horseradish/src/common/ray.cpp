#include "ray.hpp"

#include "math.hpp"

namespace HorseRadish
{
	void Ray::setOrigin(float x, float y, float z)
	{
		mOrigin[0] = x;
		mOrigin[1] = y;
		mOrigin[2] = z;
	}

	void Ray::setOrigin(const Vector3f& origin)
	{
		mOrigin = origin;
	}

	void Ray::setDirection(float x, float y, float z)
	{
		mDirection[0] = x;
		mDirection[1] = y;
		mDirection[2] = z;
	}

	void Ray::setDirection(const Vector3f& direction)
	{
		mDirection[0] = direction[0];
		mDirection[1] = direction[1];
		mDirection[2] = direction[2];
	}

	void Ray::setLength(float length)
	{
		mLength = std::abs(length);
	}

	void Ray::setSizedDirection(float x, float y, float z)
	{
		mDirection.set(x, y, z);
		mLength = mDirection.getMagnitude();
		mDirection.normalize();
	}

	void Ray::setSizedDirection(const Vector3f& sizedDirection)
	{
		mDirection = sizedDirection;
		mDirection.normalize();

		mLength = sizedDirection.getMagnitude();
	}
}