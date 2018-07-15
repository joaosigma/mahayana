#pragma once

#include "vector.hpp"
#include "math.hpp"

#include <limits>

namespace hr
{
	class BRect
	{
		float mMinX, mMinY, mMaxX, mMaxY;

	public:
		BRect() noexcept
			: mMinX(std::numeric_limits<float>::infinity())
			, mMinY(std::numeric_limits<float>::infinity())
			, mMaxX(-std::numeric_limits<float>::infinity())
			, mMaxY(-std::numeric_limits<float>::infinity())
		{ }
		
		BRect(const BRect& brect) noexcept
			: mMinX(brect.mMinX), mMinY(brect.mMinY), mMaxX(brect.mMaxX), mMaxY(brect.mMaxY)
		{ }
		BRect(const float &minX, const float &minY, const float &maxX, const float &maxY) noexcept
			: mMinX(minX), mMinY(minY), mMaxX(maxX), mMaxY(maxY)
		{ }
		BRect(const float &centerX, const float &centerY, const float &expandAmount) noexcept
			: mMinX(centerX), mMinY(centerY), mMaxX(centerX), mMaxY(centerY)
		{
			expand(expandAmount);
		}
		
		explicit BRect(const Vector3f * const points, size_t numVec);
		explicit BRect(const BRect * const brects, size_t numBRect);

		BRect& operator=(const BRect& brect)
		{
			mMinX = brect.mMinX;
			mMinY = brect.mMinY;
			mMaxX = brect.mMaxX;
			mMaxY = brect.mMaxY;
			return *this;
		}

		void operator+=(const BRect& brect)
		{
			merge(brect.mMaxX, brect.mMaxY);
			merge(brect.mMinX, brect.mMinY);
		}

		void operator+=(const Vector3f& pt)
		{
			merge(pt[0], pt[1]);
		}

		void min(Vector3f &point) const
		{
			point.set(mMinX, mMinY, 0.0f);
		}

		void min(float &x, float &y) const
		{
			x = mMinX;
			y = mMinY;
		}

		void max(Vector3f &point) const
		{
			point.set(mMaxX, mMaxY, 0.0f);
		}

		void max(float &x, float &y) const
		{
			x = mMaxX;
			y = mMaxY;
		}

		void minMax(Vector3f &min, Vector3f &max) const
		{
			min.set(mMinX, mMinY, 0.0f);
			max.set(mMaxX, mMaxY, 0.0f);
		}

		void minMax(float * const min, float * const max) const
		{
			min[0] = mMinX;
			min[1] = mMinY;
			max[0] = mMaxX;
			max[1] = mMaxY;
		}

		void center(Vector3f &point) const
		{
			point[2] = 0.0f;
			center(point[0], point[1]);
		}

		void center(float &x, float &y) const
		{
			x = (mMinX + mMaxX) * 0.5f;
			y = (mMinY + mMaxY) * 0.5f;
		}

		void dims(float &width, float &height) const
		{
			width = this->width();
			height = this->height();
		}

		float width() const
		{
			return (mMaxX - mMinX);
		}

		float height() const
		{
			return (mMaxY - mMinY);
		}

		float area(void) const
		{
			return ((mMaxX - mMinX) * (mMaxY - mMinY));
		}

		void corners(Vector3f points[4]) const;

		void merge(const Vector3f &pt);
		void merge(const float * const pt);
		void merge(const Vector3f * const pts, size_t numPts);
		void merge(const float &x, const float &y);

		void set(const float &minX, const float &minY, const float &maxX, const float &maxY)
		{
			mMinX = minX;
			mMinY = minY;
			mMaxX = maxX;
			mMaxY = maxY;
		}

		void setExpand(const float &centerX, const float &centerY, const float &expandAmount)
		{
			mMinX = centerX;
			mMinY = centerY;
			mMaxX = centerX;
			mMaxY = centerY;
			expand(expandAmount);
		}

		void setExpand(const float &centerX, const float &centerY, const float &expandX, const float &expandY)
		{
			mMinX = centerX;
			mMinY = centerY;
			mMaxX = centerX;
			mMaxY = centerY;
			expand(expandX, expandY);
		}

		void setMinMax(const Vector3f &vec)
		{
			mMinX = vec[0];
			mMinY = vec[1];
			mMaxX = vec[0];
			mMaxY = vec[1];
		}

		void setMinMax(const float &x, const float &y)
		{
			mMinX = x;
			mMinY = y;
			mMaxX = x;
			mMaxY = y;
		}

		void setMin(const Vector3f &min)
		{
			mMinX = min[0];
			mMinY = min[1];
		}

		void setMin(const float &x, const float &y)
		{
			mMinX = x;
			mMinY = y;
		}

		void setMax(const Vector3f &max)
		{
			mMaxX = max[0];
			mMaxY = max[1];
		}

		void setMax(const float &x, const float &y)
		{
			mMaxX = x;
			mMaxY = y;
		}
		
		void reset()
		{
			mMinX = std::numeric_limits<float>::infinity();
			mMinY = std::numeric_limits<float>::infinity();
			mMaxX = -std::numeric_limits<float>::infinity();
			mMaxY = -std::numeric_limits<float>::infinity();
		}
		
		void translate(const Vector3f &translation);
		void expand(const float amount);
		void expand(const float amountX, const float amountY);
		
		void crossSection(const BRect& brect, BRect& brectResult) const;

		bool containsPoint(const Vector3f &point) const
		{
			return (point[0] >= mMinX && point[1] >= mMinY && point[0] <= mMaxX && point[1] <= mMaxY);
		}

		bool containsPoint(const float &x, const float &y) const
		{
			return (x >= mMinX && y >= mMinY && x <= mMaxX && y <= mMaxY);
		}

		bool containsX(const float &x) const
		{
			return (x >= mMinX && x <= mMaxX);
		}

		bool containsY(const float &y) const
		{
			return (y >= mMinY && y <= mMaxY);
		}
	};
}
