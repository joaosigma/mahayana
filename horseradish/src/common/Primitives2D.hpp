#pragma once

#include "Math.hpp"

namespace HorseRadish { namespace Primitives2D {

	template <typename T>
	struct Rectangle
	{
		static_assert(std::is_arithmetic<T>::value, "Data type must be arithmetic (e.g.: float, unsigned char, etc.)");

		T x = {}, y = {}, width = {}, height = {};

		Rectangle()
		{ }

		Rectangle(const Rectangle &r)
			: x(r.x), y(r.y), width(r.width), height(r.height)
		{}

		explicit Rectangle(const T &x, const T &y, const T &width, const T &height)
			: x(x), y(y), width(width), height(height)
		{ }

		Rectangle& operator=(const Rectangle &r)
		{
			x = r.x;
			y = r.y;
			width = r.width;
			height = r.height;
			return *this;
		}

		void operator+=(const Rectangle &r)
		{
			x += r.x;
			y += r.y;
			width += r.width;
			height += r.height;
		}

		void operator-=(const Rectangle &r)
		{
			x -= r.x;
			y -= r.y;
			width -= r.width;
			height -= r.height;
		}

		Rectangle operator+(const Rectangle &r) const
		{
			return Rectangle(x + r.x, y + r.y, width + r.width, height + r.height);
		}

		Rectangle operator-(const Rectangle &r) const
		{
			return Rectangle(x - r.x, y - r.y, width - r.width, height - r.height);
		}

		void reset()
		{
			x = y = width = height = {};
		}
		
		void reset(const T &x, const T &y, const T &width, const T &height)
		{
			this->x = x;
			this->y = y;
			this->width = width;
			this->height = height;
		}

		void move(const T &amount)
		{
			x += amount;
			y += amount;
		}

		void move(const T &x, const T &y)
		{
			this->x += x;
			this->y += y;
		}

		void grow(const T &amount)
		{
			width += amount;
			height += amount;
		}

		void grow(const T &width, const T &height)
		{
			this->width += width;
			this->height += height;
		}

		T area() const
		{
			return (width * height);
		}

		template<typename TNew>
		Rectangle<TNew> convert() const
		{
			return Rectangle<TNew>(static_cast<TNew>(x), static_cast<TNew>(y), static_cast<TNew>(width), static_cast<TNew>(height));
		}
	};

	template <typename T>
	struct Size
	{
		static_assert(std::is_arithmetic<T>::value, "Data type must be arithmetic (e.g.: float, unsigned char, etc.)");

		T width = {}, height = {};

		Size()
		{ }

		Size(const Size &s)
			: width(s.width)
			, height(s.height)
		{ }

		explicit Size(const T &width, const T &height)
			: width(width)
			, height(height)
		{ }

		Size& operator=(const Size &s)
		{
			width = s.width;
			height = s.height;
			return *this;
		}

		void operator+=(const Size &s)
		{
			width += s.width;
			height += s.height;
		}

		void operator-=(const Size &s)
		{
			width -= s.width;
			height -= s.height;
		}

		Size operator+(const Size &s) const
		{
			return Size(width + s.width, height + s.height);
		}

		Size operator-(const Size &s) const
		{
			return Size(width - s.width, height - s.height);
		}

		void operator+=(const T &amount)
		{
			width += amount;
			height += amount;
		}

		void operator-=(const T &amount)
		{
			width -= amount;
			height -= amount;
		}

		Size operator+(const T &amount) const
		{
			return Size(width + amount, height + amount);
		}

		Size operator-(const T &amount) const
		{
			return Size(width - amount, height - amount);
		}

		void reset()
		{
			width = height = {};
		}

		void reset(const T &width, const T &height)
		{
			this->width = width;
			this->height = height;
		}

		void grow(const T &amount)
		{
			width += amount;
			height += amount;
		}

		void grow(const T &width, const T &height)
		{
			this->width += width;
			this->height += height;
		}

		T area() const
		{
			return (width * height);
		}
	};

	template <typename T>
	struct Point
	{
		static_assert(std::is_arithmetic<T>::value, "Data type must be arithmetic (e.g.: float, unsigned char, etc.)");

		T x = {}, y = {};

		Point()
		{ }

		Point(const Point &p)
			: x(p.x)
			, y(p.y)
		{ }

		explicit Point(const T &x, const T &y)
			: x(x)
			, y(y)
		{ }

		Point& operator=(const Point &p)
		{
			x = p.x;
			y = p.y;
			return *this;
		}

		void operator+=(const Point &p)
		{
			x += p.x;
			y += p.y;
		}

		void operator-=(const Point &p)
		{
			x -= p.x;
			y -= p.y;
		}

		Point operator+(const Point &p) const
		{
			return Point(x + p.x, y + p.y);
		}

		Point operator-(const Point &p) const
		{
			return Point(x - p.x, y - p.y);
		}

		void operator+=(const T &amount)
		{
			x += amount;
			y += amount;
		}

		void operator-=(const T &amount)
		{
			x -= amount;
			y -= amount;
		}

		Point operator+(const T &amount) const
		{
			return Point(x + amount, y + amount);
		}

		Point operator-(const T &amount) const
		{
			return Point(x - amount, y - amount);
		}

		void reset()
		{
			x = y = {};
		}
		
		void reset(const T &x, const T &y)
		{
			this->x = x;
			this->y = y;
		}

		void move(const T &amount)
		{
			x += amount;
			y += amount;
		}

		void move(const T &x, const T &y)
		{
			this->x += x;
			this->y += y;
		}
	};
} }

