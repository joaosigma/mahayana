#pragma once

#include "matrix.hpp"
#include "quaternion.hpp"
#include "vector.hpp"

namespace hr
{
    template<>
    struct serialize::Support<Vector3f>
    {
        template<class TArchiveWriter>
        static void save(TArchiveWriter& writer, const Vector3f& vec)
        {
            writer << vec.data();
        }

        template<class TArchiveReader>
        static void load(TArchiveReader& reader, Vector3f& vec)
        {
            reader >> vec.data();
        }
    };

    template<>
    struct serialize::Support<Vector3d>
    {
        template<class TArchiveWriter>
        static void save(TArchiveWriter& writer, const Vector3d& vec)
        {
            writer << vec.data();
        }

        template<class TArchiveReader>
        static void load(TArchiveReader& reader, Vector3d& vec)
        {
            reader >> vec.data();
        }
    };

    template<>
    struct serialize::Support<Vector4f>
    {
        template<class TArchiveWriter>
        static void save(TArchiveWriter& writer, const Vector4f& vec)
        {
            writer << vec.data();
        }

        template<class TArchiveReader>
        static void load(TArchiveReader& reader, Vector4f& vec)
        {
            reader >> vec.data();
        }
    };

    template<>
    struct serialize::Support<Vector4d>
    {
        template<class TArchiveWriter>
        static void save(TArchiveWriter& writer, const Vector4d& vec)
        {
            writer << vec.data();
        }

        template<class TArchiveReader>
        static void load(TArchiveReader& reader, Vector4d& vec)
        {
            reader >> vec.data();
        }
    };

    template<>
    struct serialize::Support<Quaternionf>
    {
        template<class TArchiveWriter>
        static void save(TArchiveWriter& writer, const Quaternionf& quat)
        {
            writer << std::span<const float>(quat.data(), 4);
        }

        template<class TArchiveReader>
        static void load(TArchiveReader& reader, Quaternionf& quat)
        {
            reader >> std::span<float>(quat.data(), 4);
        }
    };

    template<>
    struct serialize::Support<Quaterniond>
    {
        template<class TArchiveWriter>
        static void save(TArchiveWriter& writer, const Quaterniond& quat)
        {
            writer << std::span<const double>(quat.data(), 4);
        }

        template<class TArchiveReader>
        static void load(TArchiveReader& reader, Quaterniond& quat)
        {
            reader >> std::span<double>(quat.data(), 4);
        }
    };

    template<>
    struct serialize::Support<Matrix3f>
    {
        template<class TArchiveWriter>
        static void save(TArchiveWriter& writer, const Matrix3f& matrix)
        {
            writer << matrix.data();
        }

        template<class TArchiveReader>
        static void load(TArchiveReader& reader, Matrix3f& matrix)
        {
            reader >> matrix.data();
        }
    };

    template<>
    struct serialize::Support<Matrix3d>
    {
        template<class TArchiveWriter>
        static void save(TArchiveWriter& writer, const Matrix3d& matrix)
        {
            writer << matrix.data();
        }

        template<class TArchiveReader>
        static void load(TArchiveReader& reader, Matrix3d& matrix)
        {
            reader >> matrix.data();
        }
    };

    template<>
    struct serialize::Support<Matrix4f>
    {
        template<class TArchiveWriter>
        static void save(TArchiveWriter& writer, const Matrix4f& matrix)
        {
            writer << matrix.data();
        }

        template<class TArchiveReader>
        static void load(TArchiveReader& reader, Matrix4f& matrix)
        {
            reader >> matrix.data();
        }
    };

    template<>
    struct serialize::Support<Matrix4d>
    {
        template<class TArchiveWriter>
        static void save(TArchiveWriter& writer, const Matrix4d& matrix)
        {
            writer << matrix.data();
        }

        template<class TArchiveReader>
        static void load(TArchiveReader& reader, Matrix4d& matrix)
        {
            reader >> matrix.data();
        }
    };
}
