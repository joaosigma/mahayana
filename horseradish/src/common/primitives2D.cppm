module;

#include "type_traits"

export module core:primitives2D;

namespace hr
{
    export template<typename T>
    struct Rectangle
    {
        static_assert(std::is_arithmetic_v<T>, "Data type must be arithmetic (e.g.: float, unsigned char, etc.)");

        T x;
        T y;
        T width;
        T height;

    public:
        static constexpr Rectangle zero() noexcept
        {
            return Rectangle{T(), T(), T(), T()};
        }

    public:
        constexpr Rectangle() noexcept = delete;

        constexpr Rectangle(T x, T y, T width, T height) noexcept
          : x{x}, y{y}, width{width}, height{height}
        {}

        void operator+=(const Rectangle& r) noexcept
        {
            x += r.x;
            y += r.y;
            width += r.width;
            height += r.height;
        }

        void operator-=(const Rectangle& r) noexcept
        {
            x -= r.x;
            y -= r.y;
            width -= r.width;
            height -= r.height;
        }

        Rectangle operator+(const Rectangle& r) const noexcept
        {
            return Rectangle(x + r.x, y + r.y, width + r.width, height + r.height);
        }

        Rectangle operator-(const Rectangle& r) const noexcept
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

    static_assert(std::is_trivially_copyable_v<Rectangle<size_t>>, "For performance reasons, the Rectangle<size_t> data type should be trivially copyable");
    static_assert(std::is_trivially_copyable_v<Rectangle<float>>, "For performance reasons, the Rectangle<float> data type should be trivially copyable");

    export template<typename T>
    struct Size
    {
        static_assert(std::is_arithmetic_v<T>, "Data type must be arithmetic (e.g.: float, unsigned char, etc.)");

        T width;
        T height;

    public:
        static constexpr Size zero() noexcept
        {
            return Size{T(), T()};
        }

    public:
        constexpr Size() noexcept = delete;

        constexpr Size(T width, T height) noexcept
          : width{width}, height{height}
        {}

        void operator+=(const Size& s) noexcept
        {
            width += s.width;
            height += s.height;
        }

        void operator-=(const Size& s) noexcept
        {
            width -= s.width;
            height -= s.height;
        }

        Size operator+(const Size& s) const noexcept
        {
            return Size(width + s.width, height + s.height);
        }

        Size operator-(const Size& s) const noexcept
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

    export template<typename T>
    struct Point
    {
        static_assert(std::is_arithmetic_v<T>, "Data type must be arithmetic (e.g.: float, unsigned char, etc.)");

        T x;
        T y;

    public:
        static constexpr Point zero() noexcept
        {
            return Point{T(), T()};
        }

    public:
        constexpr Point() noexcept = delete;

        constexpr Point(T x, T y) noexcept
          : x{x}, y{y}
        {}

        void operator+=(const Point& p) noexcept
        {
            x += p.x;
            y += p.y;
        }

        void operator-=(const Point& p) noexcept
        {
            x -= p.x;
            y -= p.y;
        }

        Point operator+(const Point& p) const noexcept
        {
            return Point(x + p.x, y + p.y);
        }

        Point operator-(const Point& p) const noexcept
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
