module;

#include <cassert>

export module core:matrix;

import std;
import :vector;
import :plane;
import :bvolumes;
import :quaternion;

export namespace hr
{
    template<typename TDataType>
    class Matrix4;
    template<typename TDataType>
    class Matrix3;

    using Matrix4f = Matrix4<float>;
    using Matrix4d = Matrix4<double>;

    using Matrix3f = Matrix3<float>;
    using Matrix3d = Matrix3<double>;

    /*
     * The matrix classes uses a row-major mathematical convention and layout
     *   row 0 is indices [0 ... 3]
     *   row 1 is indices [4 ... 7]
     *   ...
     * This means that the layout reads as: [Xx Xy Xz 0.0 Yx Yy Yz 0.0 Zx Zy Zz 0.0 Tx Ty Tz 1.0]
     *
     * So in order for multiplication with vectors to work, they must be read as a "column with 4 rows" and are transformed like: v*M (left of pre-multiplication)
     * This also means that product (multiplication) order, if you want to translate and *then* rotate, is: res = translation * rotation
     *
     * Rotations are left-handed, which means positive rotation is clockwise about the axis of rotation (as looking to the origin of axis)
     *   - plus X points right, plus Y points up and plus Z points forward (to the horizon)
     *   - this is the same as in quaternions
     */

    template<typename TDataType>
    class alignas(alignof(TDataType) * 4) Matrix4
    {
        TDataType m[16];

        using Vector3Type = typename Vector<TDataType, 3>;
        using Vector4Type = typename Vector<TDataType, 4>;

        template<class T>
        static constexpr T kZero = T(0.0);
        template<class T>
        static constexpr T kOne = T(1.0);
        template<class T>
        static constexpr T kTwo = T(2.0);

    public:
        using DataType = typename TDataType;

        enum class CloneTransform
        {
            None,
            Transpose,
            Inverse,
            InverseTranspose,
            InverseHomogenous
        };

        static constexpr Matrix4 naked() noexcept
        {
            return {};
        }

        static constexpr Matrix4 identity() noexcept
        {
            Matrix4 mat;

            mat.m[0] = mat.m[5] = mat.m[10] = mat.m[15] = kOne<TDataType>;
            mat.m[1] = mat.m[2] = mat.m[3] = mat.m[4] = mat.m[6] = mat.m[7] = mat.m[8] = mat.m[9] = mat.m[11] = mat.m[12] = mat.m[13] = mat.m[14] = kZero<TDataType>;

            return mat;
        }

        static constexpr Matrix4 zero() noexcept
        {
            Matrix4 mat;

            for (int i = 0; i < 16; i++)
                mat.m[i] = kZero<TDataType>;

            return mat;
        }

        template<typename TType>
        static constexpr Matrix4 from(std::span<const TType> data) noexcept
        {
            static_assert(std::is_same_v<TType, float> || std::is_same_v<TType, double>, "Type must be either float or double");

            assert(data.size() == 16);
            if (data.size() != 16)
                return Matrix4::identity();

            Matrix4 mat;

            if constexpr (std::is_same_v<TDataType, TType>)
            {
                for (int i = 0; i < 16; i++)
                    mat.m[i] = data[i];
            }
            else
            {
                for (int i = 0; i < 16; i++)
                    mat.m[i] = static_cast<TDataType>(data[i]);
            }

            return mat;
        }

        template<typename TType>
        static constexpr Matrix4 from(std::initializer_list<TType> il) noexcept
        {
            assert(il.size() == 16);
            if (il.size() != 16)
                return Matrix4::identity();

            Matrix4 mat;

            auto src = std::data(il);
            if constexpr (std::is_same_v<TDataType, TType>)
            {
                for (int i = 0; i < 16; i++)
                    mat.m[i] = src[i];
            }
            else
            {
                for (int i = 0; i < 16; i++)
                    mat.m[i] = static_cast<TDataType>(src[i]);
            }

            return mat;
        }

        static constexpr Matrix4 from(const TDataType scalar) noexcept
        {
            Matrix4 mat;

            for (int i = 0; i < 16; i++)
                mat.m[i] = scalar;

            return mat;
        }

        static Matrix4 translation(TDataType x, TDataType y, TDataType z) noexcept;
        static Matrix4 translation(const Vector3Type& amount) noexcept;

        static Matrix4 rotation(const Quaternion<TDataType>& unitQuaternion) noexcept;
        static Matrix4 rotation(const Vector3Type& unitVec, const TDataType angleDeg) noexcept;
        static Matrix4 rotation(const TDataType unitVecX, const TDataType unitVecY, const TDataType unitVecZ, const TDataType angleDeg) noexcept;
        static Matrix4 rotationX(const TDataType angleDeg) noexcept;
        static Matrix4 rotationY(const TDataType angleDeg) noexcept;
        static Matrix4 rotationZ(const TDataType angleDeg) noexcept;

        static Matrix4 scale(TDataType scale) noexcept;
        static Matrix4 scale(TDataType x, TDataType y, TDataType z) noexcept;
        static Matrix4 scale(const Vector3Type& scale) noexcept;

        static Matrix4 reflection(const Plane<TDataType>& plane) noexcept;

        static Matrix4 saturation(const TDataType sat) noexcept;

        static Matrix4 glModelView(const Vector3Type& pos, const Vector3Type& target, const Vector3Type& up) noexcept;
        static Matrix4 glModelView(const Vector3Type& pos, const Vector3Type& target) noexcept;
        static Matrix4 glModelView(const Vector3Type& pos, TDataType angleDegX, TDataType angleDegY, const Vector3Type& up) noexcept;
        static Matrix4 glModelView(int cubemapFace, const Vector3Type& centerCube) noexcept;

    private:
        constexpr Matrix4() = default;

    public:
        constexpr Matrix4(const Matrix4&) = default;
        constexpr Matrix4& operator=(const Matrix4&) = default;
        constexpr Matrix4(Matrix4&&) = default;
        constexpr Matrix4& operator=(Matrix4&&) = default;

        void operator*=(const Matrix4& mat) noexcept;
        void operator*=(const TDataType scalar) noexcept;
        void operator*=(const Matrix3<TDataType>& mat) noexcept;
        void operator*=(std::span<const TDataType> mat) noexcept;
        void operator*=(const Quaternion<TDataType>& unitQuaternion) noexcept;

        void operator+=(const Matrix4& mat) noexcept;
        void operator+=(std::span<const TDataType> mat) noexcept;

        void operator-=(const Matrix4& mat) noexcept;
        void operator-=(std::span<const TDataType> mat) noexcept;

        Matrix4 operator*(const Matrix4& mat) const noexcept;
        Matrix4 operator*(const TDataType scalar) const noexcept;
        Matrix4 operator+(const Matrix4& mat) const noexcept;
        Matrix4 operator-(const Matrix4& mat) const noexcept;

        template<typename Self>
        constexpr auto& operator[](this Self&& self, const size_t index) noexcept
        {
            return self.m[index % 16];
        }

        template<typename Self>
        constexpr auto data(this Self&& self) noexcept
        {
            return std::span{self.m}.first<16>();
        }

        void transform(std::span<TDataType, 3> vec) const noexcept;
        void transform(std::span<TDataType, 4> vec) const noexcept;

        void transform(Vector3Type& vec) const noexcept;
        [[nodiscard]] Vector3Type transformCopy(const Vector3Type& vec) const noexcept;

        void transform(std::span<Vector3Type> vecs) const noexcept;

        void transform(Vector4Type& vec) const noexcept;
        [[nodiscard]] Vector4Type transformCopy(const Vector4Type& vec) const noexcept;

        void transform(std::span<Vector4Type> vecs) const noexcept;

        void transform(BBox<Vector3Type>& bbox) const noexcept;
        [[nodiscard]] BBox<Vector3Type> transformCopy(const BBox<Vector3Type>& bbox) const noexcept;

        Vector4Type getColumn(size_t columnIndex) const noexcept;
        Vector4Type getRow(size_t rowIndex) const noexcept;

        TDataType determinant() const noexcept;

        void write(std::span<TDataType, 16> dest) const noexcept;

        Matrix4 clone(CloneTransform transform = CloneTransform::None) const noexcept;

        Matrix4& transpose() noexcept;
        Matrix4& inverse() noexcept;
        Matrix4& inverseTranspose() noexcept;
        Matrix4& inverseHomogenous() noexcept;

        Vector3Type extractTranslation() const noexcept;
        Vector3Type extractScale() const noexcept;
        Quaternion<TDataType> extractRotation() const noexcept;

        template<template<typename> typename TNewType, typename TNewDataType>
        TNewType<TNewDataType> convert() const noexcept
        {
            static_assert(std::is_same_v<TNewDataType, float> || std::is_same_v<TNewDataType, double>, "New data type must be either float or double");
            static_assert(std::is_same_v<TNewType<TNewDataType>, Matrix4<TNewDataType>> || std::is_same_v<TNewType<TNewDataType>, Matrix3<TNewDataType>>,
                          "New type must be either Matrix4 or Matrix3");

            if constexpr (std::is_same_v<TNewType<TNewDataType>, Matrix4<TNewDataType>>)
            {
                if constexpr (std::is_same_v<TNewDataType, TDataType>)
                {
                    // it's just a copy
                    return Matrix4{*this};
                }
                else
                {
                    // same class (Matrix4) but with a different data type
                    auto res = Matrix4<TNewDataType>::naked();
                    for (int i = 0; i < 16; i++)
                        res[i] = static_cast<TNewDataType>(m[i]);
                    return res;
                }
            }
            else
            {
                // convert to Matrix3 and to the data type at the same time
                auto mat3 = Matrix3<TNewDataType>::naked();

                mat3[0] = static_cast<TNewDataType>(m[0]);
                mat3[1] = static_cast<TNewDataType>(m[1]);
                mat3[2] = static_cast<TNewDataType>(m[2]);
                mat3[3] = static_cast<TNewDataType>(m[4]);
                mat3[4] = static_cast<TNewDataType>(m[5]);
                mat3[5] = static_cast<TNewDataType>(m[6]);
                mat3[6] = static_cast<TNewDataType>(m[8]);
                mat3[7] = static_cast<TNewDataType>(m[9]);
                mat3[8] = static_cast<TNewDataType>(m[10]);

                return mat3;
            }
        }
    };

    static_assert(alignof(Matrix4f) == 16, "For performance reasons, this class must be aligned to a 16 byte boundary");
    static_assert(alignof(Matrix4d) == 32, "For performance reasons, this class must be aligned to a 32 byte boundary");
    static_assert(std::is_trivially_copyable_v<Matrix4f>, "For performance reasons, this class should be trivially copyable");
    static_assert(std::is_trivially_copyable_v<Matrix4d>, "For performance reasons, this class should be trivially copyable");

    extern template class Matrix4<float>;
    extern template class Matrix4<double>;

    template<typename TDataType>
    class Matrix3
    {
        static_assert(std::is_same_v<TDataType, float> || std::is_same_v<TDataType, double>, "Base type must be either float or double");

        TDataType m[9];

        using Vector3Type = typename Vector<TDataType, 3>;

        template<class T>
        static constexpr T kZero = T(0.0L);
        template<class T>
        static constexpr T kOne = T(1.0L);
        template<class T>
        static constexpr T kTwo = T(2.0L);

    public:
        using DataType = typename TDataType;

        enum class CloneTransform
        {
            None,
            Transpose
        };

        static constexpr Matrix3 naked() noexcept
        {
            return {};
        }

        static constexpr Matrix3 identity() noexcept
        {
            Matrix3 mat;

            mat.m[0] = kOne<TDataType>;
            mat.m[1] = mat.m[2] = mat.m[3] = kZero<TDataType>;
            mat.m[4] = kOne<TDataType>;
            mat.m[5] = mat.m[6] = mat.m[7] = kZero<TDataType>;
            mat.m[8] = kOne<TDataType>;

            return mat;
        }

        static constexpr Matrix3 zero() noexcept
        {
            Matrix3 mat;

            for (int i = 0; i < 9; i++)
                mat.m[i] = kZero<TDataType>;

            return mat;
        }

        template<typename TType>
        static constexpr Matrix3 from(std::span<const TType> data) noexcept
        {
            static_assert(std::is_same_v<TType, float> || std::is_same_v<TType, double>, "Type must be either float or double");

            assert(data.size() == 9);
            if (data.size() != 9)
                return Matrix3::identity();

            Matrix3 mat;

            if constexpr (std::is_same_v<TDataType, TType>)
            {
                for (int i = 0; i < 9; i++)
                    mat.m[i] = data[i];
            }
            else
            {
                for (int i = 0; i < 9; i++)
                    mat.m[i] = static_cast<TDataType>(data[i]);
            }

            return mat;
        }

        template<typename TType>
        static constexpr Matrix3 from(std::initializer_list<TType> il) noexcept
        {
            assert(il.size() == 9);
            if (il.size() != 9)
                return Matrix3::identity();

            Matrix3 mat;

            auto src = std::data(il);
            if constexpr (std::is_same_v<TDataType, TType>)
            {
                for (int i = 0; i < 9; i++)
                    mat.m[i] = src[i];
            }
            else
            {
                for (int i = 0; i < 9; i++)
                    mat.m[i] = static_cast<TDataType>(src[i]);
            }

            return mat;
        }

        static constexpr Matrix3 from(const TDataType scalar) noexcept
        {
            Matrix3 mat;

            for (int i = 0; i < 9; i++)
                mat.m[i] = scalar;

            return mat;
        }

        static Matrix3 rotation(const Quaternion<TDataType>& unitQuaternion) noexcept;
        static Matrix3 rotation(const Vector3Type& unitVec, const TDataType angleDeg) noexcept;
        static Matrix3 rotation(const TDataType unitVecX, const TDataType unitVecY, const TDataType unitVecZ, const TDataType angleDeg) noexcept;
        static Matrix3 rotationX(const TDataType angleDeg) noexcept;
        static Matrix3 rotationY(const TDataType angleDeg) noexcept;
        static Matrix3 rotationZ(const TDataType angleDeg) noexcept;

    private:
        constexpr Matrix3() = default;

    public:
        constexpr Matrix3(const Matrix3&) = default;
        constexpr Matrix3& operator=(const Matrix3&) = default;
        constexpr Matrix3(Matrix3&&) = default;
        constexpr Matrix3& operator=(Matrix3&&) = default;

        void operator*=(const Matrix3& mat) noexcept;
        void operator*=(std::span<const TDataType> mat) noexcept;

        void operator+=(const Matrix3& mat) noexcept;
        void operator+=(std::span<const TDataType> mat) noexcept;
        void operator-=(const Matrix3& mat) noexcept;
        void operator-=(std::span<const TDataType> mat) noexcept;

        Matrix3 operator*(const Matrix3& mat) const noexcept;
        Matrix3 operator+(const Matrix3& mat) const noexcept;
        Matrix3 operator-(const Matrix3& mat) const noexcept;

        template<typename Self>
        constexpr auto& operator[](this Self&& self, const size_t index) noexcept
        {
            return self.m[index % 9];
        }

        template<typename Self>
        constexpr auto data(this Self&& self) noexcept
        {
            return std::span{self.m}.first<9>();
        }

        void transform(std::span<TDataType, 3> vec) const noexcept;

        void transform(Vector3Type& vec) const noexcept;
        [[nodiscard]] Vector3Type transformCopy(const Vector3Type& vec) const noexcept;

        void transform(std::span<Vector3Type> vecs) const noexcept;

        Vector3Type getColumn(size_t columnIndex) const noexcept;
        Vector3Type getRow(size_t rowIndex) const noexcept;

        TDataType determinant() const noexcept;

        void write(std::span<TDataType, 9> dest) const noexcept;

        Matrix3 clone(CloneTransform transform = CloneTransform::None) const noexcept;

        Matrix3& transpose() noexcept
        {
            std::swap(m[1], m[3]);
            std::swap(m[2], m[6]);
            std::swap(m[5], m[7]);

            return *this;
        }

        template<template<typename> typename TNewType, typename TNewDataType>
        TNewType<TNewDataType> convert() const noexcept
        {
            static_assert(std::is_same_v<TNewDataType, float> || std::is_same_v<TNewDataType, double>, "New data type must be either float or double");
            static_assert(std::is_same_v<TNewType<TNewDataType>, Matrix4<TNewDataType>> || std::is_same_v<TNewType<TNewDataType>, Matrix3<TNewDataType>>,
                          "New type must be either Matrix4 or Matrix3");

            if constexpr (std::is_same_v<TNewType<TNewDataType>, Matrix3<TNewDataType>>)
            {
                if constexpr (std::is_same_v<TNewDataType, TDataType>)
                {
                    // it's just a copy
                    return Matrix3{*this};
                }
                else
                {
                    // same class (Matrix3) but with a different data type
                    auto res = Matrix3<TNewDataType>::naked();
                    for (int i = 0; i < 9; i++)
                        res[i] = static_cast<TNewDataType>(m[i]);
                    return res;
                }
            }
            else
            {
                // convert to Matrix4 and to the data type at the same time
                auto mat4 = Matrix4<TNewDataType>::naked();

                mat4[0] = static_cast<TNewDataType>(m[0]);
                mat4[1] = static_cast<TNewDataType>(m[1]);
                mat4[2] = static_cast<TNewDataType>(m[2]);
                mat4[3] = kZero<TNewDataType>;
                mat4[4] = static_cast<TNewDataType>(m[3]);
                mat4[5] = static_cast<TNewDataType>(m[4]);
                mat4[6] = static_cast<TNewDataType>(m[5]);
                mat4[7] = kZero<TNewDataType>;
                mat4[8] = static_cast<TNewDataType>(m[6]);
                mat4[9] = static_cast<TNewDataType>(m[7]);
                mat4[10] = static_cast<TNewDataType>(m[8]);
                mat4[11] = mat4[12] = mat4[13] = mat4[14] = kZero<TNewDataType>;
                mat4[15] = kOne<TNewDataType>;

                return mat4;
            }
        }
    };

    static_assert(std::is_trivially_copyable_v<Matrix3f>, "For performance reasons, this class should be trivially copyable");
    static_assert(std::is_trivially_copyable_v<Matrix3d>, "For performance reasons, this class should be trivially copyable");

    extern template class Matrix3<float>;
    extern template class Matrix3<double>;
}
