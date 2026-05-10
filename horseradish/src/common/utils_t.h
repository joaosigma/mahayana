#include <catch2/matchers/catch_matchers_templated.hpp>
#include <catch2/matchers/catch_matchers_vector.hpp>

namespace Catch
{
    template<>
    struct StringMaker<hr::Vector3f>
    {
        static std::string convert(hr::Vector3f const& v)
        {
            return std::format("Vector is: {{{}, {}, {}}}", v[0], v[1], v[2]);
        }
    };

    template<>
    struct StringMaker<hr::Vector3d>
    {
        static std::string convert(hr::Vector3d const& v)
        {
            return std::format("Vector is: {{{}, {}, {}}}", v[0], v[1], v[2]);
        }
    };

    template<>
    struct StringMaker<hr::Vector4f>
    {
        static std::string convert(hr::Vector4f const& v)
        {
            return std::format("Vector is: {{{}, {}, {}, {}}}", v[0], v[1], v[2], v[3]);
        }
    };

    template<>
    struct StringMaker<hr::Quaternionf>
    {
        static std::string convert(hr::Quaternionf const& q)
        {
            return std::format("Quaternion is: {{{}, {}, {}, {}}}", q[0], q[1], q[2], q[3]);
        }
    };

    template<>
    struct StringMaker<hr::Quaterniond>
    {
        static std::string convert(hr::Quaterniond const& q)
        {
            return std::format("Quaternion is: {{{}, {}, {}, {}}}", q[0], q[1], q[2], q[3]);
        }
    };

    template<>
    struct StringMaker<hr::Matrix4f>
    {
        static std::string convert(hr::Matrix4f const& m)
        {
            return std::format("Matrix is: {{{{{}, {}, {}, {}}}, {{{}, {}, {}, {}}}, {{{}, {}, {}, {}}}, {{{}, {}, {}, {}}}}}", m[0], m[1], m[2], m[3], m[4], m[5], m[6], m[7],
                               m[8], m[9], m[10], m[11], m[12], m[13], m[14], m[15]);
        }
    };

    template<>
    struct StringMaker<hr::Matrix4d>
    {
        static std::string convert(hr::Matrix4d const& m)
        {
            return std::format("Matrix is: {{{{{}, {}, {}, {}}}, {{{}, {}, {}, {}}}, {{{}, {}, {}, {}}}, {{{}, {}, {}, {}}}}}", m[0], m[1], m[2], m[3], m[4], m[5], m[6], m[7],
                               m[8], m[9], m[10], m[11], m[12], m[13], m[14], m[15]);
        }
    };

    template<>
    struct StringMaker<hr::Matrix3f>
    {
        static std::string convert(hr::Matrix3f const& m)
        {
            return std::format("Matrix is: {{{{{}, {}, {}}}, {{{}, {}, {}}}, {{{}, {}, {}}}}}", m[0], m[1], m[2], m[3], m[4], m[5], m[6], m[7], m[8]);
        }
    };

    template<>
    struct StringMaker<hr::Matrix3d>
    {
        static std::string convert(hr::Matrix3d const& m)
        {
            return std::format("Matrix is: {{{{{}, {}, {}}}, {{{}, {}, {}}}, {{{}, {}, {}}}}}", m[0], m[1], m[2], m[3], m[4], m[5], m[6], m[7], m[8]);
        }
    };
}
