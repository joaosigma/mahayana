#include "Ray.hpp"

#include "Math.hpp"

namespace HorseRadish
{
	void Ray::setOrigin(const float &x, const float &y, const float &z)
	{
		mOrigin[0] = x;
		mOrigin[1] = y;
		mOrigin[2] = z;
	}

	void Ray::setOrigin(const Vector3f &nOrigin)
	{
		mOrigin = nOrigin;
	}

	void Ray::setDirection(const float &x, const float &y, const float &z)
	{
		mDirection[0] = x;
		mDirection[1] = y;
		mDirection[2] = z;
	}

	void Ray::setDirection(const Vector3f &nDirection)
	{
		mDirection[0] = nDirection[0];
		mDirection[1] = nDirection[1];
		mDirection[2] = nDirection[2];
	}

	void Ray::setLength(const float &nLength)
	{
		mLength = std::abs(nLength);
	}

	void Ray::setSizedDirection(const float &x, const float &y, const float &z)
	{
		mDirection.set(x, y, z);
		mLength = mDirection.getMagnitude();
		mDirection.normalize();
	}

	void Ray::setSizedDirection(const Vector3f &nSizedDirection)
	{
		mDirection = nSizedDirection;
		mDirection.normalize();

		mLength = nSizedDirection.getMagnitude();
	}

} //HorseRadish