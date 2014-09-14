#pragma once

#include "Vector.hpp"
#include "Math.hpp"

namespace HorseRadish
{
	class BRect
	{
		float minX, minY, maxX, maxY;

	public:

		BRect() : minX(Math::INFINITY), minY(Math::INFINITY), maxX(-Math::INFINITY), maxY(-Math::INFINITY) { }
		BRect(const BRect& brect) : minX(brect.minX), minY(brect.minY), maxX(brect.maxX), maxY(brect.maxY) { }
		BRect(const float &minX, const float &minY, const float &maxX, const float &maxY) : minX(minX), minY(minY), maxX(maxX), maxY(maxY) { }
		BRect(const float &centerX, const float &centerY, const float &expandAmount) : minX(centerX), minY(centerY), maxX(centerX), maxY(centerY) { this->Expand(expandAmount); }
		explicit BRect(const Vector * const points, const unsigned int numVec);
		explicit BRect(const BRect * const brects, const unsigned int numBRect);
		~BRect(){ return; }

		BRect& operator=(const BRect& brect);
		void operator+=(const BRect& brect);
		void operator+=(const Vector& pt);

		void GetMin(Vector &point) const { point.Set(minX, minY, 0.0f); }
		void GetMax(Vector &point) const { point.Set(maxX, maxY, 0.0f); }
		void GetMin(float * const point) const { point[0] = minX; point[1] = minY; }
		void GetMax(float * const point) const { point[0] = maxX; point[1] = maxY; }
		void GetMinMax(Vector &min, Vector &max) const { min.Set(minX, minY, 0.0f); max.Set(maxX, maxY, 0.0f); }
		void GetMinMax(float * const min, float * const max) const { min[0] = minX; min[1] = minY; max[0] = maxX; max[1] = maxY; }
		void GetCenter(Vector &point) const { point.Set((minX + maxX)*0.5f, (minY + maxY)*0.5f, 0.0f); }
		void GetCenter(float * const point) const { point[0] = (minX + maxX)*0.5f; point[1] = (minY + maxY)*0.5f; }
		void GetCenter(float &x, float &y) const { x = (minX + maxX)*0.5f; y = (minY + maxY)*0.5f; }
		void GetDims(Vector &point) const { point.Set(maxX - minX, maxY - minY, 0.0); }
		void GetDims(float * const point) const { point[0] = maxX - minX; point[1] = maxY - minY; }
		void GetDims(float &width, float &height) const { width = maxX - minX; height = maxY - minY; }
		float GetWidth() const { return (maxX - minX); }
		float GetHeight() const { return (maxY - minY); }
		float GetArea(void) const;
		void GetCorners(Vector points[4]) const;

		void Merge(const Vector &pt);
		void Merge(const float * const pt);
		void Merge(const Vector * const pts, const int numPts);
		void Merge(const float &x, const float &y);

		void Set(const BRect * const brect) { minX = brect->minX; minY = brect->minY; maxX = brect->maxX; maxY = brect->maxY; }
		void Set(const float &minX, const float &minY, const float &maxX, const float &maxY) { this->minX = minX; this->minY = minY; this->maxX = maxX; this->maxY = maxY; }
		void SetExpand(const float &centerX, const float &centerY, const float &expandAmount) { minX = centerX; minY = centerY; maxX = centerX; maxY = centerY; this->Expand(expandAmount); }
		void SetExpand(const float &centerX, const float &centerY, const float &expandX, const float &expandY) { minX = centerX; minY = centerY; maxX = centerX; maxY = centerY; this->Expand(expandX, expandY); }
		void SetMinMax(const Vector &vec) { minX = vec.x; minY = vec.y; maxX = vec.x; maxY = vec.y; }
		void SetMinMax(const float &x, const float &y) { minX = x; minY = y; maxX = x; maxY = y; }
		void SetMin(const Vector &min) { minX = min.x; minY = min.y; }
		void SetMin(const float &x, const float &y) { minX = x; minY = y; }
		void SetMax(const Vector &max) { maxX = max.x; maxY = max.y; }
		void SetMax(const float &x, const float &y) { maxX = x; maxY = y; }
		
		void Reset(){ minX = Math::INFINITY; minY = Math::INFINITY; maxX = -Math::INFINITY; maxY = -Math::INFINITY; }
		
		void Translate(const Vector &translation);
		void Expand(const float amount);
		void Expand(const float amountX, const float amountY);
		
		void CrossSection(const BRect& brect, BRect& brectResult) const;

		bool ContainsPoint(const Vector &point) const;
		bool ContainsPoint(const float &x, const float &y) const;
		bool ContainsX(const float &x) const;
		bool ContainsY(const float &y) const;
	};

} //HorseRadish
