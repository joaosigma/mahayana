#pragma once

#include "vector.hpp"

namespace HorseRadish
{
	class Ray
	{
		Vector3f mOrigin, mDirection;
		float mLength;

	public:
		Ray()
			: mOrigin(0.0f), mDirection(0.0f), mLength(0.0f)
		{ }

		explicit Ray(const Vector3f& origin, const Vector3f& direction)
		{
			mOrigin = origin;
			
			mDirection = direction;
			mDirection.normalize();

			mLength = direction.getMagnitude();
		}

		const Vector3f& origin() const
		{
			return mOrigin;
		}

		const Vector3f& direction() const
		{
			return mDirection;
		}

		float length() const
		{
			return mLength;
		}

		Vector3f pointOn(float t) const
		{
			return (mOrigin + (mDirection * t));
		}

		float getDotOrigin(float x, float y, float z) const
		{
			return mOrigin.getDot(x, y, z);
		}

		float getDotOrigin(const Vector3f& vec) const
		{
			return mOrigin.getDot(vec);
		}

		float getDotDirection(float x, float y, float z) const
		{
			mDirection.getDot(x, y, z);
		}

		float getDotDirection(const Vector3f& vec) const
		{
			return vec.getDot(mDirection);
		}

		void setOrigin(float x, float y, float z);
		void setOrigin(const Vector3f& origin);
		void setDirection(float x, float y, float z);
		void setDirection(const Vector3f& direction);
		void setLength(float length);
		void setSizedDirection(float x, float y, float z);
		void setSizedDirection(const Vector3f& sizedDirection);

		void negateDir()
		{
			mDirection *= -1.0f;
		}
	};

} //HorseRadish
