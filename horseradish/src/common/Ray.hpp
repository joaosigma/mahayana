#pragma once

#include "Vector.hpp"

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

		explicit Ray(const Vector3f &nOrigin, const Vector3f &nDirection)
		{
			mOrigin = nOrigin;
			
			mDirection = nDirection;
			mDirection.normalize();

			mLength = nDirection.getMagnitude();
		}

		const Vector3f& GetOrigin() const
		{
			return mOrigin;
		}

		const Vector3f& GetDirection() const
		{
			return mDirection;
		}

		float GetLength() const
		{
			return mLength;
		}

		void GetPointOn(const float &t, Vector3f &write) const
		{
			write = mOrigin + (mDirection * t);
		}

		Vector3f GetPointOn(const float &t) const
		{
			return (mOrigin + (mDirection * t));
		}

		float GetDotOrigin(const float &x, const float &y, const float &z) const
		{
			return mOrigin.getDot(x, y, z);
		}

		float GetDotOrigin(const Vector3f &vec) const
		{
			return mOrigin.getDot(vec);
		}

		float GetDotDirection(const float &x, const float &y, const float &z) const
		{
			mDirection.getDot(x, y, z);
		}

		float GetDotDirection(const Vector3f &vec) const
		{
			return vec.getDot(mDirection);
		}

		void setOrigin(const float &x, const float &y, const float &z);
		void setOrigin(const Vector3f &nOrigin);
		void setDirection(const float &x, const float &y, const float &z);
		void setDirection(const Vector3f &nDirection);
		void setLength(const float &nLength);
		void setSizedDirection(const float &x, const float &y, const float &z);
		void setSizedDirection(const Vector3f &nSizedDirection);

		void negateDir()
		{
			mDirection *= -1.0f;
		}
	};

} //HorseRadish
