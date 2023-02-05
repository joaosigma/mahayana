#pragma once

#include "math.hpp"

namespace hr
{
	template <typename T>
	struct Rectangle
	{
		static_assert(std::is_arithmetic<T>::value, "Data type must be arithmetic (e.g.: float, unsigned char, etc.)");

		T x{};
		T y{};
		T width{};
		T height{};

		Rectangle() = default;
		Rectangle(const Rectangle&) = default;
		Rectangle& operator=(const Rectangle&) = default;
		Rectangle(Rectangle&&) = default;
		Rectangle& operator=(Rectangle&&) = default;

		Rectangle(T x, T y, T width, T height) noexcept
		  : x{x}, y{y}, width{width}, height{height}
		{ }

		void operator+=(const Rectangle &r) noexcept
		{
			x += r.x;
			y += r.y;
			width += r.width;
			height += r.height;
		}

		void operator-=(const Rectangle &r) noexcept
		{
			x -= r.x;
			y -= r.y;
			width -= r.width;
			height -= r.height;
		}

		Rectangle operator+(const Rectangle &r) const noexcept
		{
			return Rectangle(x + r.x, y + r.y, width + r.width, height + r.height);
		}

		Rectangle operator-(const Rectangle &r) const noexcept
		{
			return Rectangle(x - r.x, y - r.y, width - r.width, height - r.height);
		}

		void move(T amount) noexcept
		{
			x += amount;
			y += amount;
		}

		void move(T x, T y) noexcept
		{
			this->x += x;
			this->y += y;
		}

		void grow(T amount) noexcept
		{
			width += amount;
			height += amount;
		}

		void grow(T width, T height) noexcept
		{
			this->width += width;
			this->height += height;
		}

		T area() const noexcept
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

		T width{};
		T height{};

		Size() = default;
		Size(const Size&) = default;
		Size& operator=(const Size&) = default;
		Size(Size&&) = default;
		Size& operator=(Size&&) = default;

		Size(T width, T height) noexcept
			: width{ width }, height{ height }
		{ }

		void operator+=(const Size &s) noexcept
		{
			width += s.width;
			height += s.height;
		}

		void operator-=(const Size &s) noexcept
		{
			width -= s.width;
			height -= s.height;
		}

		Size operator+(const Size &s) const noexcept
		{
			return Size(width + s.width, height + s.height);
		}

		Size operator-(const Size &s) const noexcept
		{
			return Size(width - s.width, height - s.height);
		}

		void operator+=(T amount) noexcept
		{
			width += amount;
			height += amount;
		}

		void operator-=(T amount) noexcept
		{
			width -= amount;
			height -= amount;
		}

		Size operator+(T amount) const noexcept
		{
			return Size(width + amount, height + amount);
		}

		Size operator-(T amount) const noexcept
		{
			return Size(width - amount, height - amount);
		}

		void grow(T amount) noexcept
		{
			width += amount;
			height += amount;
		}

		void grow(T width, T height) noexcept
		{
			this->width += width;
			this->height += height;
		}

		T area() const noexcept
		{
			return (width * height);
		}
	};

	template <typename T>
	struct Point
	{
		static_assert(std::is_arithmetic<T>::value, "Data type must be arithmetic (e.g.: float, unsigned char, etc.)");

		T x{};
		T y{};

		Point() = default;
		Point(const Point&) = default;
		Point& operator=(const Point&) = default;
		Point(Point&&) = default;
		Point& operator=(Point&&) = default;

		Point(T x, T y) noexcept
			: x{ x } , y{ y }
		{ }

		void operator+=(const Point &p) noexcept
		{
			x += p.x;
			y += p.y;
		}

		void operator-=(const Point &p) noexcept
		{
			x -= p.x;
			y -= p.y;
		}

		Point operator+(const Point &p) const noexcept
		{
			return Point(x + p.x, y + p.y);
		}

		Point operator-(const Point &p) const noexcept
		{
			return Point(x - p.x, y - p.y);
		}

		void operator+=(T amount) noexcept
		{
			x += amount;
			y += amount;
		}

		void operator-=(T amount) noexcept
		{
			x -= amount;
			y -= amount;
		}

		Point operator+(T amount) const noexcept
		{
			return Point(x + amount, y + amount);
		}

		Point operator-(T amount) const noexcept
		{
			return Point(x - amount, y - amount);
		}

		void move(T amount) noexcept
		{
			x += amount;
			y += amount;
		}

		void move(T x, T y) noexcept
		{
			this->x += x;
			this->y += y;
		}
	};
}
