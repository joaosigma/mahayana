#pragma once

#include "vector.hpp"

namespace hr
{
	class Ray
	{
		Vector3f mOrigin{ 0.0f }, mDirection{ 0.0f };
		float mLength{ 0.0f };

	public:
		constexpr Ray() = default;
		constexpr Ray(const Ray&) = default;
		constexpr Ray& operator=(const Ray&) = default;
		constexpr Ray(Ray&&) = default;
		constexpr Ray& operator=(Ray&&) = default;

		explicit Ray(const Vector3f& origin, const Vector3f& direction)
			: mOrigin{ origin }
			, mDirection{ direction }
		{
			mDirection.normalize();
			mLength = direction.getMagnitude();
		}

		Ray& setOrigin(float x, float y, float z);
		Ray& setOrigin(const Vector3f& origin);
		Ray& setDirection(float x, float y, float z);
		Ray& setDirection(const Vector3f& direction);
		Ray& setLength(float length);
		Ray& setSizedDirection(float x, float y, float z);
		Ray& setSizedDirection(const Vector3f& sizedDirection);

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

		void negateDir()
		{
			mDirection *= -1.0f;
		}
	};
}
