#pragma once

#include "matrix.hpp"
#include "quaternion.hpp"
#include "vector.hpp"

#include <format>
#include <type_traits>

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

namespace hr::utests
{
    template<typename TType>
    struct Vector3EqualsMatcher: Catch::Matchers::MatcherGenericBase
    {
        static_assert(std::is_same_v<float, TType> || std::is_same_v<double, TType>);

        Vector3EqualsMatcher(TType x, TType y, TType z)
          : m_x{x}, m_y{y}, m_z{z}
        {}

        template<typename TVector>
        bool match(const TVector& v) const
        {
            static_assert(std::is_same_v<TVector, hr::Vector3f> || std::is_same_v<TVector, hr::Vector3d>);
            static_assert(std::is_same_v<typename TVector::DataType, TType>);
            static_assert(TVector::NumComponents == 3);

            // set epsilon to allowed a 0.1% difference and a margin to allow (0.0f == -0.0f) to pass

            return (v[0] == Catch::Approx(m_x).epsilon(0.001).margin(0.0000001)) && (v[1] == Catch::Approx(m_y).epsilon(0.001).margin(0.0000001)) &&
                   (v[2] == Catch::Approx(m_z).epsilon(0.001).margin(0.0000001));
        }

        std::string describe() const override
        {
            return std::format("Vector is: {{{}, {}, {}}}", m_x, m_y, m_z);
        }

    private:
        TType m_x, m_y, m_z;
    };

    template<typename TType>
    struct Vector4EqualsMatcher: Catch::Matchers::MatcherGenericBase
    {
        static_assert(std::is_same_v<float, TType> || std::is_same_v<double, TType>);

        Vector4EqualsMatcher(TType x, TType y, TType z, TType w)
          : m_x{x}, m_y{y}, m_z{z}, m_w{w}
        {}

        template<typename TVector>
        bool match(const TVector& v) const
        {
            static_assert(std::is_same_v<TVector, hr::Vector4f> || std::is_same_v<TVector, hr::Vector4d>);
            static_assert(std::is_same_v<typename TVector::DataType, TType>);
            static_assert(TVector::NumComponents == 4);

            // set epsilon to allowed a 0.1% difference and a margin to allow (0.0f == -0.0f) to pass

            return (v[0] == Catch::Approx(m_x).epsilon(0.001).margin(0.0000001)) && (v[1] == Catch::Approx(m_y).epsilon(0.001).margin(0.0000001)) &&
                   (v[2] == Catch::Approx(m_z).epsilon(0.001).margin(0.0000001)) && (v[3] == Catch::Approx(m_w).epsilon(0.001).margin(0.0000001));
        }

        std::string describe() const override
        {
            return std::format("Components are: {{{}, {}, {}, {}}}", m_x, m_y, m_z, m_w);
        }

    private:
        TType m_x, m_y, m_z, m_w;
    };

    template<typename TType>
    auto VectorEquals(TType x, TType y, TType z)
    {
        return Vector3EqualsMatcher<TType>{x, y, z};
    }

    inline auto VectorEquals(const hr::Vector3f& vec)
    {
        return Vector3EqualsMatcher<float>{vec[0], vec[1], vec[2]};
    }

    inline auto VectorEquals(const hr::Vector3d& vec)
    {
        return Vector3EqualsMatcher<double>{vec[0], vec[1], vec[2]};
    }

    template<typename TType>
    auto VectorEquals(TType x, TType y, TType z, TType w)
    {
        return Vector4EqualsMatcher<TType>{x, y, z, w};
    }

    inline auto VectorEquals(const hr::Vector4f& vec)
    {
        return Vector4EqualsMatcher<float>{vec[0], vec[1], vec[2], vec[3]};
    }

    inline auto VectorEquals(const hr::Vector4d& vec)
    {
        return Vector4EqualsMatcher<double>{vec[0], vec[1], vec[2], vec[3]};
    }

    template<typename TType>
    struct QuaternionEqualsMatcher: Catch::Matchers::MatcherGenericBase
    {
        static_assert(std::is_same_v<float, TType> || std::is_same_v<double, TType>);

        QuaternionEqualsMatcher(Quaternion<TType> q)
          : m_quat{std::move(q)}
        {}

        QuaternionEqualsMatcher(const TType qx, const TType qy, const TType qz, const TType qw)
          : m_quat{Quaternion<TType>::from(qx, qy, qz, qw)}
        {}

        bool match(const Quaternion<TType>& q) const
        {
            // set epsilon to allowed a 0.1% difference and a margin to allow (0.0f == -0.0f) to pass

            return (q[0] == Catch::Approx(m_quat[0]).epsilon(0.001).margin(0.0000001)) && (q[1] == Catch::Approx(m_quat[1]).epsilon(0.001).margin(0.0000001)) &&
                   (q[2] == Catch::Approx(m_quat[2]).epsilon(0.001).margin(0.0000001)) && (q[3] == Catch::Approx(m_quat[3]).epsilon(0.001).margin(0.0000001));
        }

        std::string describe() const override
        {
            return std::format("Quaternion is: {{{}, {}, {}, {}}}", m_quat[0], m_quat[1], m_quat[2], m_quat[3]);
        }

    private:
        Quaternion<TType> m_quat;
    };

    template<typename TType>
    inline auto QuaternionEquals(const TType qx, const TType qy, const TType qz, const TType qw)
    {
        return QuaternionEqualsMatcher<TType>{qx, qy, qz, qw};
    }

    inline auto QuaternionEquals(Quaternionf q)
    {
        return QuaternionEqualsMatcher<float>{std::move(q)};
    }

    inline auto QuaternionEquals(Quaterniond q)
    {
        return QuaternionEqualsMatcher<double>{std::move(q)};
    }

    template<typename TType>
    struct Matrix4EqualsMatcher: Catch::Matchers::MatcherGenericBase
    {
        static_assert(std::is_same_v<float, TType> || std::is_same_v<double, TType>);

        Matrix4EqualsMatcher(std::span<const TType, 16> mat)
        {
            std::memcpy(m_mat, mat.data(), mat.size_bytes());
        }

        bool match(const hr::Matrix4<TType>& m) const
        {
            // set epsilon to allowed a 0.1% difference and a margin to allow (0.0f == -0.0f) to pass

            for (int i = 0; i < 16; i++)
            {
                if (m[i] != Catch::Approx(m_mat[i]).epsilon(0.001).margin(0.0000001))
                    return false;
            }

            return true;
        }

        std::string describe() const override
        {
            return std::format("Matrix4x4 is: {{{{{}, {}, {}, {}}}, {{{}, {}, {}, {}}}, {{{}, {}, {}, {}}}, {{{}, {}, {}, {}}}}}", m_mat[0], m_mat[1], m_mat[2], m_mat[3], m_mat[4],
                               m_mat[5], m_mat[6], m_mat[7], m_mat[8], m_mat[9], m_mat[10], m_mat[11], m_mat[12], m_mat[13], m_mat[14], m_mat[15]);
        }

    private:
        TType m_mat[16];
    };

    template<typename TType>
    auto Matrix4Equals(const TType mat[16])
    {
        return Matrix4EqualsMatcher<TType>(std::span<const TType, 16>{mat, 16});
    }

    inline auto Matrix4Equals(const hr::Matrix4f& mat)
    {
        return Matrix4EqualsMatcher<float>{mat.data()};
    }

    inline auto Matrix4Equals(const hr::Matrix4d& mat)
    {
        return Matrix4EqualsMatcher<double>{mat.data()};
    }

    template<typename TType>
    struct Matrix3EqualsMatcher: Catch::Matchers::MatcherGenericBase
    {
        static_assert(std::is_same_v<float, TType> || std::is_same_v<double, TType>);

        Matrix3EqualsMatcher(std::span<const TType, 9> mat)
        {
            std::memcpy(m_mat, mat.data(), mat.size_bytes());
        }

        bool match(const hr::Matrix3<TType>& m) const
        {
            // set epsilon to allowed a 0.1% difference and a margin to allow (0.0f == -0.0f) to pass

            for (int i = 0; i < 9; i++)
            {
                if (m[i] != Catch::Approx(m_mat[i]).epsilon(0.001).margin(0.0000001))
                    return false;
            }

            return true;
        }

        std::string describe() const override
        {
            return std::format("Matrix3x3 is: {{{{{}, {}, {}}}, {{{}, {}, {}}}, {{{}, {}, {}}}}}", m_mat[0], m_mat[1], m_mat[2], m_mat[3], m_mat[4], m_mat[5], m_mat[6], m_mat[7],
                               m_mat[8]);
        }

    private:
        TType m_mat[9];
    };

    template<typename TType>
    auto Matrix3Equals(const TType mat[9])
    {
        return Matrix3EqualsMatcher<TType>{std::span<const TType, 9>{mat, 9}};
    }

    inline auto Matrix3Equals(const hr::Matrix3f& mat)
    {
        return Matrix3EqualsMatcher<float>{mat.data()};
    }

    inline auto Matrix3Equals(const hr::Matrix3d& mat)
    {
        return Matrix3EqualsMatcher<double>{mat.data()};
    }
}
