#pragma once

#include "types.hpp"
#include "vector.hpp"
#include "plane.hpp"
#include "bvolumes.hpp"
#include "quaternion.hpp"

#include <span>

namespace hr
{
	template<typename TDataType> class Matrix4;
	template<typename TDataType> class Matrix3;

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

		template<typename TDataType>
		friend class Matrix3;

		using Vector3Type = typename Vector<TDataType, 3>;
		using Vector4Type = typename Vector<TDataType, 4>;

		template<class T>
		static constexpr T kZero = T(0.0L);
		template<class T>
		static constexpr T kOne = T(1.0L);
		template<class T>
		static constexpr T kTwo = T(2.0L);

	public:
		using DataType = typename TDataType;

		enum class CloneTransform{ None, Transpose, Inverse, InverseTranspose, InverseHomogenous };

		static constexpr Matrix4 identity() noexcept
		{
			Matrix4 mat;

			mat.m[0] = mat.m[5] = mat.m[10] = mat.m[15] = kOne<TDataType>;
			mat.m[1] = mat.m[2] = mat.m[3] = mat.m[4] = mat.m[6] = mat.m[7] = mat.m[8] = mat.m[9] = mat.m[11] = mat.m[12] = mat.m[13] = mat.m[14] = kZero<TDataType>;

			return mat;
		}

		static constexpr Matrix4 zero()
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
		static constexpr Matrix4 from(std::initializer_list<TType> il)
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

		static constexpr Matrix4 from(const TDataType scalar)
		{
			Matrix4 mat;

			for (int i = 0; i < 16; i++)
				mat.m[i] = scalar;

			return mat;
		}

		static constexpr Matrix4 translation(TDataType x, TDataType y, TDataType z) noexcept
		{
			Matrix4 mat;

			mat.m[0] = mat.m[5] = mat.m[10] = mat.m[15] = kOne<TDataType>;
			mat.m[1] = mat.m[2] = mat.m[3] = mat.m[4] = mat.m[6] = mat.m[7] = mat.m[8] = mat.m[9] = mat.m[11] = kZero<TDataType>;
			mat.m[12] = x;
			mat.m[13] = y;
			mat.m[14] = z;

			return mat;
		}

		static constexpr Matrix4 translation(const Vector3Type &amount) noexcept
		{
			return Matrix4::translation(amount[0], amount[1], amount[2]);
		}

		static constexpr Matrix4 from(const Quaternion<TDataType> &unitQuaternion) noexcept
		{
			Matrix4 mat;

			mat.m[0] = kOne<TDataType> - kTwo<TDataType> * (unitQuaternion[1] * unitQuaternion[1] + unitQuaternion[2] * unitQuaternion[2]);
			mat.m[4] = kTwo<TDataType> * (unitQuaternion[0] * unitQuaternion[1] - unitQuaternion[2] * unitQuaternion[3]);
			mat.m[8] = kTwo<TDataType> * (unitQuaternion[0] * unitQuaternion[2] + unitQuaternion[1] * unitQuaternion[3]);

			mat.m[1] = kTwo<TDataType> * (unitQuaternion[0] * unitQuaternion[1] + unitQuaternion[2] * unitQuaternion[3]);
			mat.m[5] = kOne<TDataType> - kTwo<TDataType> * (unitQuaternion[0] * unitQuaternion[0] + unitQuaternion[2] * unitQuaternion[2]);
			mat.m[9] = kTwo<TDataType> * (unitQuaternion[1] * unitQuaternion[2] - unitQuaternion[0] * unitQuaternion[3]);

			mat.m[2] = kTwo<TDataType> * (unitQuaternion[0] * unitQuaternion[2] - unitQuaternion[1] * unitQuaternion[3]);
			mat.m[6] = kTwo<TDataType> * (unitQuaternion[1] * unitQuaternion[2] + unitQuaternion[0] * unitQuaternion[3]);
			mat.m[10] = kOne<TDataType> - kTwo<TDataType> * (unitQuaternion[0] * unitQuaternion[0] + unitQuaternion[1] * unitQuaternion[1]);

			mat.m[3] = mat.m[7] = mat.m[11] = mat.m[12] = mat.m[13] = mat.m[14] = kZero<TDataType>;
			mat.m[15] = kOne<TDataType>;

			return mat;
		}

		static constexpr Matrix4 rotation(const Quaternion<TDataType> &unitQuaternion) noexcept
		{
			return Matrix4::from(unitQuaternion);
		}

		static constexpr Matrix4 scale(TDataType x, TDataType y, TDataType z) noexcept
		{
			Matrix4 mat;

			mat.m[0] = x;
			mat.m[5] = y;
			mat.m[10] = z;
			mat.m[15] = kOne<TDataType>;
			mat.m[1] = mat.m[2] = mat.m[3] = mat.m[4] = mat.m[6] = mat.m[7] = mat.m[8] = mat.m[9] = mat.m[11] = mat.m[12] = mat.m[13] = mat.m[14] = kZero<TDataType>;

			return mat;
		}

		static constexpr Matrix4 scale(const Vector3Type& scale) noexcept
		{
			return Matrix4::scale(scale[0], scale[1], scale[2]);
		}

		static Matrix4 reflection(const Plane<TDataType> &plane) noexcept
		{
			Matrix4 mat;

			auto pNormal = plane.normal();
			pNormal.normalize();

			auto pNormalAux = pNormal * (-kTwo<TDataType> * pNormal[0]);
			mat.m[0] = pNormalAux[0] + kOne<TDataType>;
			mat.m[1] = pNormalAux[1];
			mat.m[2] = pNormalAux[2];
			mat.m[3] = kZero<TDataType>;

			pNormalAux = pNormal * (-kTwo<TDataType> * pNormal[1]);
			mat.m[4] = pNormalAux[0];
			mat.m[5] = pNormalAux[1] + kOne<TDataType>;
			mat.m[6] = pNormalAux[2];
			mat.m[7] = kZero<TDataType>;

			pNormalAux = pNormal * (-kTwo<TDataType> * pNormal[2]);
			mat.m[8] = pNormalAux[0];
			mat.m[9] = pNormalAux[1];
			mat.m[10] = pNormalAux[2] + kOne<TDataType>;
			mat.m[11] = kZero<TDataType>;

			pNormalAux = pNormal * (-kTwo<TDataType> * plane.d());
			mat.m[12] = pNormalAux[0];
			mat.m[13] = pNormalAux[1];
			mat.m[14] = pNormalAux[2];
			mat.m[15] = kOne<TDataType>;

			return mat;
		}

		static Matrix4 saturation(const TDataType sat) noexcept
		{
			Matrix4 mat;

			if constexpr (std::is_same_v<TDataType, float>)
			{
				auto posS = Math::fClamp(sat, -1.0f, 1.0f);
				auto minusS = 1.0f - posS;

				mat.m[0] = minusS * 0.3086f + posS;
				mat.m[1] = mat.m[2] = minusS * 0.3086f;
				mat.m[4] = mat.m[6] = minusS * 0.6094f;
				mat.m[5] = minusS * 0.6094f + posS;
				mat.m[8] = mat.m[9] = minusS * 0.0820f;
				mat.m[10] = minusS * 0.0820f + posS;

				mat.m[3] = mat.m[7] = mat.m[11] = mat.m[12] = mat.m[13] = mat.m[14] = 0.0f;
				mat.m[15] = 1.0f;
			}
			else
			{
				auto posS = Math::fClamp(sat, -1.0, 1.0);
				auto minusS = 1.0 - posS;

				mat.m[0] = minusS * 0.3086 + posS;
				mat.m[1] = mat.m[2] = minusS * 0.3086;
				mat.m[4] = mat.m[6] = minusS * 0.6094;
				mat.m[5] = minusS * 0.6094 + posS;
				mat.m[8] = mat.m[9] = minusS * 0.0820;
				mat.m[10] = minusS * 0.0820 + posS;

				mat.m[3] = mat.m[7] = mat.m[11] = mat.m[12] = mat.m[13] = mat.m[14] = 0.0;
				mat.m[15] = 1.0;
			}

			return mat;
		}

		static Matrix4 glModelView(const Vector3Type &pos, const Vector3Type &target, const Vector3Type &up) noexcept
		{
			auto z = pos;
			z -= target;
			z.normalize();

			auto x = Vector3Type::calcCrossProduct(up, z);
			auto y = Vector3Type::calcCrossProduct(z, x);

			x.normalize();
			y.normalize();

			Matrix4 mat;
			mat.m[0] = x[0];
			mat.m[1] = y[0];
			mat.m[2] = z[0];
			mat.m[3] = kZero<TDataType>;
			mat.m[4] = x[1];
			mat.m[5] = y[1];
			mat.m[6] = z[1];
			mat.m[7] = kZero<TDataType>;
			mat.m[8] = x[2];
			mat.m[9] = y[2];
			mat.m[10] = z[2];
			mat.m[11] = kZero<TDataType>;

			x *= -kOne<TDataType>;
			y *= -kOne<TDataType>;
			z *= -kOne<TDataType>;

			mat.m[12] = x[0] * pos[0] + x[1] * pos[1] + x[2] * pos[2];
			mat.m[13] = y[0] * pos[0] + y[1] * pos[1] + y[2] * pos[2];
			mat.m[14] = z[0] * pos[0] + z[1] * pos[1] + z[2] * pos[2];
			mat.m[15] = kOne<TDataType>;

			return mat;
		}

		static Matrix4 glModelView(const Vector3Type &pos, const Vector3Type &target) noexcept
		{
			return Matrix4::glModelView(pos, target, Vector3Type{kZero<TDataType>, kOne<TDataType>, kZero<TDataType>});
		}

		static Matrix4 glModelView(const Vector3Type &pos, TDataType angleDegX, TDataType angleDegY, const Vector3Type &up) noexcept
		{
			TDataType sx, sy, cx, cy;
			Math::sinCos(Math::Deg2Rad<TDataType> * angleDegX, sx, cx);
			Math::sinCos(Math::Deg2Rad<TDataType> * angleDegY, sy, cy);

			Vector3Type target;
			target[0] = pos[0] + sx * cy;
			target[1] = pos[1] + sy;
			target[2] = pos[2] - (cx * cy);

			return Matrix4::glModelView(pos, target, up);
		}

		static Matrix4 glModelView(int cubemapFace, const Vector3Type &centerCube) noexcept
		{
			auto mat = Matrix4::identity();

			//positive X
			if (cubemapFace == 0)
			{
				mat.m[2] = mat.m[5] = mat.m[8] = -1.;
				mat.m[12] = centerCube[2];
				mat.m[13] = centerCube[1];
				mat.m[14] = centerCube[0];
				return mat;
			}

			//negative X
			if (cubemapFace == 1)
			{
				mat.m[2] = mat.m[8] = 1.;
				mat.m[5] = -1.;
				mat.m[12] = -centerCube[2];
				mat.m[13] = centerCube[1];
				mat.m[14] = -centerCube[0];
				return mat;
			}

			//positive Y
			if (cubemapFace == 2)
			{
				mat.m[0] = mat.m[9] = 1.;
				mat.m[6] = -1.;
				mat.m[12] = -centerCube[0];
				mat.m[13] = -centerCube[2];
				mat.m[14] = centerCube[1];
				return mat;
			}

			//negative Y
			if (cubemapFace == 3)
			{
				mat.m[0] = mat.m[6] = 1.;
				mat.m[9] = -1.;
				mat.m[12] = -centerCube[0];
				mat.m[13] = centerCube[2];
				mat.m[14] = -centerCube[1];
				return mat;
			}

			//positive Z
			if (cubemapFace == 4)
			{
				mat.m[5] = mat.m[10] = -1.;
				mat.m[0] = 1.;
				mat.m[12] = -centerCube[0];
				mat.m[13] = centerCube[1];
				mat.m[14] = centerCube[2];
				return mat;
			}

			//negative Z
			if (cubemapFace == 5)
			{
				mat.m[0] = mat.m[5] = -1.;
				mat.m[10] = 1.;
				mat.m[12] = centerCube[0];
				mat.m[13] = centerCube[1];
				mat.m[14] = -centerCube[2];
				return mat;
			}

			return mat;
		}

	private:
		constexpr Matrix4() = default;

	public:
		constexpr Matrix4(const Matrix4&) = default;
		constexpr Matrix4& operator=(const Matrix4&) = default;
		constexpr Matrix4(Matrix4&&) = default;
		constexpr Matrix4& operator=(Matrix4&&) = default;

		void operator*=(const TDataType s) noexcept;
		void operator*=(const Matrix4 &mat) noexcept;
		void operator*=(const Matrix3<TDataType> &mat) noexcept;
		void operator*=(std::span<const TDataType> mat) noexcept;
		void operator*=(const Quaternion<TDataType> &unitQuaternion) noexcept;

		void operator+=(const Matrix4 &mat) noexcept;
		void operator+=(std::span<const TDataType> mat) noexcept;

		void operator-=(const Matrix4 &mat) noexcept;
		void operator-=(std::span<const TDataType> mat) noexcept;

		Matrix4 operator*(const Matrix4 &mat) const noexcept;
		Matrix4 operator+(const Matrix4 &mat) const noexcept;
		Matrix4 operator-(const Matrix4 &mat) const noexcept;
		Matrix4 operator*(const TDataType s) const noexcept;

		constexpr TDataType &operator[](size_t index) noexcept
		{
			return m[index % 16];
		}

		constexpr const TDataType &operator[](size_t index) const noexcept
		{
			return m[index % 16];
		}

		constexpr TDataType *data() noexcept
		{
			return m;
		}

		constexpr const TDataType *data() const noexcept
		{
			return m;
		}

		void transform(std::span<TDataType> vec) const noexcept;

		void transform(Vector3Type &vec) const noexcept;
		void transform(const Vector3Type &vec, Vector3Type &result) const noexcept;
		void transform(std::span<Vector3Type> vecs) const noexcept;

		void transform(Vector4Type &vec) const noexcept;
		void transform(const Vector4Type &vec, Vector4Type &result) const noexcept;
		void transform(std::span<Vector4Type> vecs) const noexcept;

		void transform(BBox<Vector3Type> &bbox) const noexcept;
		void transform(const BBox<Vector3Type> &bbox, BBox<Vector3Type> &bboxDest) const noexcept;

		Vector4Type getColumn(size_t columnIndex) const noexcept;
		Vector4Type getRow(size_t rowIndex) const noexcept;

		void write(TDataType dest[16]) const noexcept;

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
			static_assert(std::is_same_v<TNewType<TNewDataType>, Matrix4<TNewDataType>> || std::is_same_v<TNewType<TNewDataType>, Matrix3<TNewDataType>>, "New type must be either Matrix4 or Matrix3");

			if constexpr (std::is_same_v<TNewType<TNewDataType>, Matrix4<TNewDataType>>)
			{
				if constexpr (std::is_same_v<TNewDataType, TDataType>)
				{
					//it's just a copy
					return Matrix4{*this};
				}
				else
				{
					//same class (Matrix4) but with a different data type
					Matrix4<TNewDataType> res;
					for (int i = 0; i < 16; i++)
						res.m[i] = static_cast<TNewDataType>(m[i]);
					return res;
				}
			}
			else
			{
				//convert to Matrix3 and to the data type at the same time
				Matrix3<TNewDataType> mat3;

				mat3.m[0] = static_cast<TNewDataType>(m[0]);
				mat3.m[1] = static_cast<TNewDataType>(m[1]);
				mat3.m[2] = static_cast<TNewDataType>(m[2]);
				mat3.m[3] = static_cast<TNewDataType>(m[4]);
				mat3.m[4] = static_cast<TNewDataType>(m[5]);
				mat3.m[5] = static_cast<TNewDataType>(m[6]);
				mat3.m[6] = static_cast<TNewDataType>(m[8]);
				mat3.m[7] = static_cast<TNewDataType>(m[9]);
				mat3.m[8] = static_cast<TNewDataType>(m[10]);

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

		template<typename TDataType>
		friend class Matrix4;

		using Vector3Type = typename Vector<TDataType, 3>;

		template<class T>
		static constexpr T kZero = T(0.0L);
		template<class T>
		static constexpr T kOne = T(1.0L);
		template<class T>
		static constexpr T kTwo = T(2.0L);

	public:
		using DataType = typename TDataType;

		enum class CloneTransform{ None, Transpose };

		static constexpr Matrix3 identity() noexcept
		{
			Matrix3 mat;

			mat.m[0] = 1.;
			mat.m[1] = mat.m[2] = mat.m[3] = 0.;
			mat.m[4] = 1.;
			mat.m[5] = mat.m[6] = mat.m[7] = 0.;
			mat.m[8] = 1.;

			return mat;
		}

		static constexpr Matrix3 zero() noexcept
		{
			Matrix3 mat;

			for (int i = 0; i < 9; i++)
				mat.m[i] = 0.;

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

		static constexpr Matrix3 from(const Quaternion<TDataType> &unitQuaternion) noexcept
		{
			Matrix3 mat;

			mat.m[0] = kOne<TDataType> - kTwo<TDataType> * (unitQuaternion[1] * unitQuaternion[1] + unitQuaternion[2] * unitQuaternion[2]);
			mat.m[3] = kTwo<TDataType> * (unitQuaternion[0] * unitQuaternion[1] - unitQuaternion[2] * unitQuaternion[3]);
			mat.m[6] = kTwo<TDataType> * (unitQuaternion[0] * unitQuaternion[2] + unitQuaternion[1] * unitQuaternion[3]);

			mat.m[1] = kTwo<TDataType> * (unitQuaternion[0] * unitQuaternion[1] + unitQuaternion[2] * unitQuaternion[3]);
			mat.m[4] = kOne<TDataType> - kTwo<TDataType> * (unitQuaternion[0] * unitQuaternion[0] + unitQuaternion[2] * unitQuaternion[2]);
			mat.m[7] = kTwo<TDataType> * (unitQuaternion[1] * unitQuaternion[2] - unitQuaternion[0] * unitQuaternion[3]);

			mat.m[2] = kTwo<TDataType> * (unitQuaternion[0] * unitQuaternion[2] - unitQuaternion[1] * unitQuaternion[3]);
			mat.m[5] = kTwo<TDataType> * (unitQuaternion[1] * unitQuaternion[2] + unitQuaternion[0] * unitQuaternion[3]);
			mat.m[8] = kOne<TDataType> - kTwo<TDataType> * (unitQuaternion[0] * unitQuaternion[0] + unitQuaternion[1] * unitQuaternion[1]);

			return mat;
		}

		static constexpr Matrix3 rotation(const Quaternion<TDataType> &unitQuaternion) noexcept
		{
			return Matrix3::from(unitQuaternion);
		}

	private:
		constexpr Matrix3() = default;

	public:
		constexpr Matrix3(const Matrix3&) = default;
		constexpr Matrix3& operator=(const Matrix3&) = default;
		constexpr Matrix3(Matrix3&&) = default;
		constexpr Matrix3& operator=(Matrix3&&) = default;

		void operator*=(const Matrix3 &mat) noexcept;
		void operator*=(std::span<const TDataType> mat) noexcept;
		void operator+=(const Matrix3 &mat) noexcept;
		void operator+=(std::span<const TDataType> mat) noexcept;
		void operator-=(const Matrix3 &mat) noexcept;
		void operator-=(std::span<const TDataType> mat) noexcept;

		Matrix3 operator*(const Matrix3 &mat) const noexcept;
		Matrix3 operator+(const Matrix3 &mat) const noexcept;
		Matrix3 operator-(const Matrix3 &mat) const noexcept;

		constexpr TDataType &operator[](const size_t index) noexcept
		{
			return m[index % 9];
		}

		constexpr const TDataType &operator[](const size_t index) const noexcept
		{
			return m[index % 9];
		}

		constexpr TDataType *data() noexcept
		{
			return m;
		}

		constexpr const TDataType *data() const noexcept
		{
			return m;
		}

		void transform(TDataType vec[3]) const noexcept;

		void transform(Vector3Type &vec) const noexcept;
		Vector3Type transform(const Vector3Type &vec) const noexcept;
		void transform(const Vector3Type &vec, Vector3Type &result) const noexcept;

		void transform(std::span<Vector3Type> vecs) const noexcept;

		Vector3Type getColumn(size_t columnIndex) const noexcept;
		Vector3Type getRow(size_t rowIndex) const noexcept;

		void write(TDataType dest[9]) const noexcept;

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
					//it's just a copy
					return Matrix3{*this};
				}
				else
				{
					//same class (Matrix3) but with a different data type
					Matrix3<TNewDataType> res;
					for (int i = 0; i < 9; i++)
						res.m[i] = static_cast<TNewDataType>(m[i]);
					return res;
				}
			}
			else
			{
				//convert to Matrix4 and to the data type at the same time
				Matrix4<TNewDataType> mat4;

				mat4.m[0] = static_cast<TNewDataType>(m[0]);
				mat4.m[1] = static_cast<TNewDataType>(m[1]);
				mat4.m[2] = static_cast<TNewDataType>(m[2]);
				mat4.m[3] = kZero<TNewDataType>;
				mat4.m[4] = static_cast<TNewDataType>(m[3]);
				mat4.m[5] = static_cast<TNewDataType>(m[4]);
				mat4.m[6] = static_cast<TNewDataType>(m[5]);
				mat4.m[7] = kZero<TNewDataType>;
				mat4.m[8] = static_cast<TNewDataType>(m[6]);
				mat4.m[9] = static_cast<TNewDataType>(m[7]);
				mat4.m[10] = static_cast<TNewDataType>(m[8]);
				mat4.m[11] = mat4.m[12] = mat4.m[13] = mat4.m[14] = kZero<TNewDataType>;
				mat4.m[15] = kOne<TNewDataType>;

				return mat4;
			}
		}
	};

	static_assert(std::is_trivially_copyable_v<Matrix3f>, "For performance reasons, this class should be trivially copyable");
	static_assert(std::is_trivially_copyable_v<Matrix3d>, "For performance reasons, this class should be trivially copyable");

	extern template class Matrix3<float>;
	extern template class Matrix3<double>;
}
