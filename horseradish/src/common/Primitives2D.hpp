#pragma once

#include "Math.hpp"

namespace HorseRadish
{
	namespace Primitives2D
	{
		template <typename T>
		struct Rectangle{
			T x, y, width, height;

			inline Rectangle()
			{
				x = y = width = height = 0;
			}
			inline Rectangle(const Rectangle &r)
			{
				x = r.x; y = r.y; width = r.width; height = r.height;
			}
			explicit inline Rectangle(const T &x, const T &y, const T &width, const T &height)
			{
				this->x = x; this->y = y; this->width = width; this->height = height;
			}

			inline ~Rectangle()
			{
				return;
			}

			inline operator T *(void)
			{
				return &x;
			}
			inline operator const T *(void) const
			{
				return &x;
			}

			inline Rectangle& operator=(const Rectangle &r)
			{
				x = r.x; y = r.y; width = r.width; height = r.height;
				return *this;
			}

			inline void operator+=(const Rectangle &r)
			{
				x += r.x; y += r.y; width += r.width; height += r.height;
			}
			inline void operator-=(const Rectangle &r)
			{
				x -= r.x; y -= r.y; width -= r.width; height -= r.height;
			}

			inline Rectangle operator+(const Rectangle &r) const
			{
				return Rectangle(x + r.x, y + r.y, width + r.width, height + r.height);
			}
			inline Rectangle operator-(const Rectangle &r) const
			{
				return Rectangle(x - r.x, y - r.y, width - r.width, height - r.height);
			}

			inline void Reset()
			{
				x = y = width = height = 0;
			}
			inline void Set(const Rectangle &r)
			{
				x = r.x; y = r.y; width = r.width; height = r.height;
			}
			inline void Set(const Rectangle * const r)
			{
				x = r->x; y = r->y; width = r->width; height = r->height;
			}
			inline void Set(const T &x, const T &y, const T &width, const T &height)
			{
				this->x = x; this->y = y; this->width = width; this->height = height;
			}

			void Move(const T &amount)
			{
				x += amount; y += amount;
			}
			void Move(const T &x, const T &y)
			{
				this->x += x; this->y += y;
			}
			void Grow(const T &amount)
			{
				width += amount; height += amount;
			}
			void Grow(const T &width, const T &height)
			{
				this->width += width; this->height += height;
			}

			T GetArea() const
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
		struct Size{
			T width, height;

			inline Size()
			{
				width = height = 0;
			}
			inline Size(const Size &s)
			{
				width = s.width; height = s.height;
			}
			explicit inline Size(const T &width, const T &height)
			{
				this->width = width; this->height = height;
			}

			inline ~Size()
			{
				return;
			}

			inline operator T *(void)
			{
				return &x;
			}
			inline operator const T *(void) const
			{
				return &x;
			}

			inline Size& operator=(const Size &s)
			{
				width = s.width; height = s.height;
				return *this;
			}

			inline void operator+=(const Size &s)
			{
				width += s.width; height += s.height;
			}
			inline void operator-=(const Size &s)
			{
				width -= s.width; height -= s.height;
			}

			inline Size operator+(const Size &s) const
			{
				return Size(width + s.width, height + s.height);
			}
			inline Size operator-(const Size &s) const
			{
				return Size(width - s.width, height - s.height);
			}

			inline void Reset()
			{
				width = height = 0;
			}
			inline void Set(const Size &s)
			{
				width = s.width; height = s.height;
			}
			inline void Set(const Size * const s)
			{
				width = s->width; height = s->height;
			}
			inline void Set(const T &width, const T &height)
			{
				this->width = width; this->height = height;
			}

			void Grow(const T &amount)
			{
				width += amount; height += amount;
			}
			void Grow(const T &width, const T &height)
			{
				this->width += width; this->height += height;
			}

			T GetArea() const
			{
				return (width * height);
			}
		};

		template <typename T>
		struct Point{
			T x, y;

			inline Point()
			{
				x = y = 0;
			}
			inline Point(const Point &p)
			{
				x = p.x; y = p.y;
			}
			explicit inline Point(const T &x, const T &y)
			{
				this->x = x; this->y = y;
			}

			inline ~Point()
			{
				return;
			}

			inline operator T *(void)
			{
				return &x;
			}
			inline operator const T *(void) const
			{
				return &x;
			}

			inline Point& operator=(const Point &p)
			{
				x = p.x; y = p.y;
				return *this;
			}

			inline void operator+=(const Point &p)
			{
				x += p.x; y += p.y;
			}
			inline void operator-=(const Point &p)
			{
				x -= p.x; y -= p.y;
			}

			inline Point operator+(const Point &p) const
			{
				return Point(x + p.x, y + p.y);
			}
			inline Point operator-(const Point &p) const
			{
				return Point(x - p.x, y - p.y);
			}

			inline void Reset()
			{
				x = y = 0;
			}
			inline void Set(const Point &p)
			{
				x = p.x; y = p.y;
			}
			inline void Set(const Point * const p)
			{
				x = p->x; y = p->y;
			}
			inline void Set(const T &x, const T &y)
			{
				this->x = x; this->y = y;
			}

			void Move(const T &amount)
			{
				x += amount; y += amount;
			}
			void Move(const T &x, const T &y)
			{
				this->x += x; this->y += y;
			}
		};

	} //Primitives2D
} //HorseRadish

