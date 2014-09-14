#include "BAreas.hpp"

namespace HorseRadish
{
	BRect::BRect(const Vector * const points, const unsigned int numVec)
	{
		minX = minY = Math::INFINITY;
		maxX = maxY = -Math::INFINITY;

		Merge(points, numVec);
	}

	BRect::BRect(const BRect * const brects, const unsigned int numBRect)
	{
		minX = minY = Math::INFINITY;
		maxX = maxY = -Math::INFINITY;

		for (unsigned int i = 0; i < numBRect; i++)
		{
			Merge(brects[i].minX, brects[i].minY);
			Merge(brects[i].maxX, brects[i].maxY);
		}
	}

	BRect& BRect::operator=(const BRect& brect)
	{
		minX = brect.minX;
		minY = brect.minY;
		maxX = brect.maxX;
		maxY = brect.maxY;
		return *this;
	}

	void BRect::operator+=(const BRect& brect)
	{
		Merge(brect.maxX, brect.maxY);
		Merge(brect.minX, brect.minY);
	}

	void BRect::operator+=(const Vector& pt)
	{
		Merge(pt.x, pt.y);
	}

	float BRect::GetArea(void) const
	{
		return ((maxX - minX) * (maxY - minY));
	}

	/*
	3-------------2
	|			  |
	|			  |
	|	  front	  |
	|			  |
	0-------------1
	*/
	void BRect::GetCorners(Vector points[4]) const
	{
		points[0].Set(minX, minY, 0.0f);
		points[1].Set(maxX, minY, 0.0f);
		points[2].Set(maxX, maxY, 0.0f);
		points[3].Set(minX, maxY, 0.0f);
	}

	void BRect::Merge(const Vector &pt)
	{
		minX = Math::fMin(minX, pt.x);
		minY = Math::fMin(minY, pt.y);
		maxX = Math::fMax(maxX, pt.x);
		maxY = Math::fMax(maxY, pt.y);
	}

	void BRect::Merge(const float * const pt)
	{
		minX = Math::fMin(minX, pt[0]);
		minY = Math::fMin(minY, pt[1]);
		maxX = Math::fMax(maxX, pt[0]);
		maxY = Math::fMax(maxY, pt[1]);
	}

	void BRect::Merge(const Vector * const pts, const int numPts)
	{
		for (int i = 0; i < numPts; i++)
		{
			minX = Math::fMin(minX, pts[i].x);
			minY = Math::fMin(minY, pts[i].y);
			maxX = Math::fMax(maxX, pts[i].x);
			maxY = Math::fMax(maxY, pts[i].y);
		}
	}

	void BRect::Merge(const float &x, const float &y)
	{
		minX = Math::fMin(minX, x);
		minY = Math::fMin(minY, y);
		maxX = Math::fMax(maxX, x);
		maxY = Math::fMax(maxY, y);
	}

	void BRect::Translate(const Vector &translation)
	{
		minX += translation.x;
		minY += translation.y;
		maxX += translation.x;
		maxY += translation.y;
	}

	void BRect::Expand(const float amount)
	{
		minX -= amount;
		minY -= amount;
		maxX += amount;
		maxY += amount;
	}

	void BRect::Expand(const float amountX, const float amountY)
	{
		minX -= amountX;
		minY -= amountY;
		maxX += amountX;
		maxY += amountY;
	}

	void BRect::CrossSection(const BRect& brect, BRect& brectResult) const
	{
		brectResult.minX = Math::fMax(minX, brect.minX);
		brectResult.minY = Math::fMax(minY, brect.minY);
		brectResult.maxX = Math::fMin(maxX, brect.maxX);
		brectResult.maxY = Math::fMin(maxY, brect.maxY);

		if ((brectResult.minX <= brectResult.maxX) && (brectResult.minY <= brectResult.maxY))
			return;

		brectResult.Reset();
	}

	bool BRect::ContainsPoint(const Vector &point) const
	{
		if (point.x<minX || point.y<minY || point.x>maxX || point.y>maxY)
			return false;
		return true;
	}

	bool BRect::ContainsPoint(const float &x, const float &y) const
	{
		if (x<minX || y<minY || x>maxX || y>maxY)
			return false;
		return true;
	}

	bool BRect::ContainsX(const float &x) const
	{
		if (x<minX || x>maxX)
			return false;
		return true;
	}

	bool BRect::ContainsY(const float &y) const
	{
		if (y<minY || y>maxY)
			return false;
		return true;
	}

} //HorseRadish