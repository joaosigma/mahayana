#pragma once

#include "types.hpp"
#include "vector.hpp"
#include "plane.hpp"
#include "bvolumes.hpp"
#include "quaternion.hpp"

#include <span>

namespace hr
{
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
	class Matrix3;

	using Matrix3f = Matrix3<float>;
	using Matrix3d = Matrix3<double>;

	class Matrix
	{
		float m[16];

		template<typename TDataType>
		friend class Matrix3;

	public:
		static constexpr Matrix from(const Quaternion &unitQuaternion) noexcept
		{
			Matrix m;

			m[0] = 1.0f - 2.0f * (unitQuaternion[1] * unitQuaternion[1] + unitQuaternion[2] * unitQuaternion[2]);
			m[4] = 2.0f * (unitQuaternion[0] * unitQuaternion[1] - unitQuaternion[2] * unitQuaternion[3]);
			m[8] = 2.0f * (unitQuaternion[0] * unitQuaternion[2] + unitQuaternion[1] * unitQuaternion[3]);

			m[1] = 2.0f * (unitQuaternion[0] * unitQuaternion[1] + unitQuaternion[2] * unitQuaternion[3]);
			m[5] = 1.0f - 2.0f * (unitQuaternion[0] * unitQuaternion[0] + unitQuaternion[2] * unitQuaternion[2]);
			m[9] = 2.0f * (unitQuaternion[1] * unitQuaternion[2] - unitQuaternion[0] * unitQuaternion[3]);

			m[2] = 2.0f * (unitQuaternion[0] * unitQuaternion[2] - unitQuaternion[1] * unitQuaternion[3]);
			m[6] = 2.0f * (unitQuaternion[1] * unitQuaternion[2] + unitQuaternion[0] * unitQuaternion[3]);
			m[10] = 1.0f - 2.0f * (unitQuaternion[0] * unitQuaternion[0] + unitQuaternion[1] * unitQuaternion[1]);

			m[3] = m[7] = m[11] = m[12] = m[13] = m[14] = 0.0f;
			m[15] = 1.0f;

			return m;
		}

		template<typename TType>
		static constexpr Matrix from(std::span<const TType> data) noexcept
		{
			static_assert(std::is_same_v<TType, float> || std::is_same_v<TType, double>, "Type must be either float or double");

			if (data.size() != 16)
				return Matrix::identity();

			Matrix mat;

			if constexpr (std::is_same_v<TType, float>)
			{
				for (int i = 0; i < 16; i++)
					mat.m[i] = data[i];
			}
			else
			{
				for (int i = 0; i < 16; i++)
					mat.m[i] = static_cast<float>(data[i]);
			}

			return mat;
		}

		static constexpr Matrix from(std::initializer_list<float> il)
		{
			assert(il.size() == 16);

			Matrix mat;

			auto src = std::data(il);
			for (int i = 0; i < 16; i++)
				mat.m[i] = src[i];

			return mat;
		}

		static constexpr Matrix from(const float scalar)
		{
			Matrix mat;

			for (int i = 0; i < 16; i++)
				mat.m[i] = scalar;

			return mat;
		}

		static constexpr Matrix identity() noexcept
		{
			Matrix mat;

			mat.m[0] = mat.m[5] = mat.m[10] = mat.m[15] = 1.;
			mat.m[1] = mat.m[2] = mat.m[3] = mat.m[4] = mat.m[6] = mat.m[7] = mat.m[8] = mat.m[9] = mat.m[11] = mat.m[12] = mat.m[13] = mat.m[14] = 0.;

			return mat;
		}

		static constexpr Matrix zero()
		{
			Matrix mat;

			for (int i = 0; i < 16; i++)
				mat.m[i] = 0.;

			return mat;
		}

		static constexpr Matrix translation(float x, float y, float z) noexcept
		{
			Matrix mat;

			mat.m[0] = mat.m[5] = mat.m[10] = mat.m[15] = 1.;
			mat.m[1] = mat.m[2] = mat.m[3] = mat.m[4] = mat.m[6] = mat.m[7] = mat.m[8] = mat.m[9] = mat.m[11] = 0.;
			mat.m[12] = x;
			mat.m[13] = y;
			mat.m[14] = z;

			return mat;
		}

		static constexpr Matrix translation(const Vector3f &amount) noexcept
		{
			return Matrix::translation(amount[0], amount[1], amount[2]);
		}

		static constexpr Matrix rotation(const Quaternion &unitQuaternion) noexcept
		{
			return Matrix::from(unitQuaternion);
		}

		static constexpr Matrix scale(float x, float y, float z) noexcept
		{
			Matrix mat;

			mat.m[0] = x;
			mat.m[5] = y;
			mat.m[10] = z;
			mat.m[15] = 1.;
			mat.m[1] = mat.m[2] = mat.m[3] = mat.m[4] = mat.m[6] = mat.m[7] = mat.m[8] = mat.m[9] = mat.m[11] = mat.m[12] = mat.m[13] = mat.m[14] = 0.;

			return mat;
		}

		static constexpr Matrix scale(const Vector3f& scale) noexcept
		{
			return Matrix::scale(scale[0], scale[1], scale[2]);
		}

		static Matrix glModelView(const Vector3f &pos, const Vector3f &target, const Vector3f &up) noexcept
		{
			Matrix mat;
			Vector3f x, y, z;

			z.set(pos);
			z -= target;
			z.normalize();

			x = Vector3f::calcCrossProduct(up, z);
			y = Vector3f::calcCrossProduct(z, x);

			x.normalize();
			y.normalize();

			mat.m[0] = x[0];
			mat.m[1] = y[0];
			mat.m[2] = z[0];
			mat.m[3] = 0.;
			mat.m[4] = x[1];
			mat.m[5] = y[1];
			mat.m[6] = z[1];
			mat.m[7] = 0.;
			mat.m[8] = x[2];
			mat.m[9] = y[2];
			mat.m[10] = z[2];
			mat.m[11] = 0.;

			x *= -1.0f;
			y *= -1.0f;
			z *= -1.0f;

			mat.m[12] = x[0] * pos[0] + x[1] * pos[1] + x[2] * pos[2];
			mat.m[13] = y[0] * pos[0] + y[1] * pos[1] + y[2] * pos[2];
			mat.m[14] = z[0] * pos[0] + z[1] * pos[1] + z[2] * pos[2];
			mat.m[15] = 1.;

			return mat;
		}

		static Matrix glModelView(const Vector3f &pos, const Vector3f &target) noexcept
		{
			return Matrix::glModelView(pos, target, Vector3f{0.0f, 1.0f, 0.0f});
		}

		static Matrix glModelView(const Vector3f &pos, float angleDegX, float angleDegY, const Vector3f &up) noexcept
		{
			Vector3f target;
			float sx, sy, cx, cy;

			Math::sinCos(Math::Deg2Rad<float> * angleDegX, sx, cx);
			Math::sinCos(Math::Deg2Rad<float> * angleDegY, sy, cy);

			target[0] = pos[0] + sx * cy;
			target[1] = pos[1] + sy;
			target[2] = pos[2] - (cx * cy);

			return Matrix::glModelView(pos, target, up);
		}

		static Matrix glModelView(int cubemapFace, const Vector3f &centerCube) noexcept
		{
			auto mat = Matrix::identity();

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
				mat.m[2] = mat.m[8] = 1.0f;
				mat.m[5] = -1.;
				mat.m[12] = -centerCube[2];
				mat.m[13] = centerCube[1];
				mat.m[14] = -centerCube[0];
				return mat;
			}

			//positive Y
			if (cubemapFace == 2)
			{
				mat.m[0] = mat.m[9] = 1.0f;
				mat.m[6] = -1.;
				mat.m[12] = -centerCube[0];
				mat.m[13] = -centerCube[2];
				mat.m[14] = centerCube[1];
				return mat;
			}

			//negative Y
			if (cubemapFace == 3)
			{
				mat.m[0] = mat.m[6] = 1.0f;
				mat.m[9] = -1.;
				mat.m[12] = -centerCube[0];
				mat.m[13] = centerCube[2];
				mat.m[14] = -centerCube[1];
				return mat;
			}

			//positive Z
			if (cubemapFace == 4)
			{
				mat.m[5] = mat.m[10] = -1.0f;
				mat.m[0] = 1.;
				mat.m[12] = -centerCube[0];
				mat.m[13] = centerCube[1];
				mat.m[14] = centerCube[2];
				return mat;
			}

			//negative Z
			if (cubemapFace == 5)
			{
				mat.m[0] = mat.m[5] = -1.0f;
				mat.m[10] = 1.;
				mat.m[12] = centerCube[0];
				mat.m[13] = centerCube[1];
				mat.m[14] = -centerCube[2];
				return mat;
			}

			return mat;
		}

	private:
		constexpr Matrix() = default;

	public:
		constexpr Matrix(const Matrix&) = default;
		constexpr Matrix& operator=(const Matrix&) = default;
		constexpr Matrix(Matrix&&) = default;
		constexpr Matrix& operator=(Matrix&&) = default;

		void operator*=(const float s) noexcept;
		void operator*=(const Matrix &s) noexcept;
		void operator*=(const Matrix3f &s) noexcept;
		void operator*=(const float src[16]) noexcept;
		void operator*=(const Quaternion &unitQuaternion) noexcept;
		void operator+=(const Matrix &s) noexcept;
		void operator+=(const float src[16]) noexcept;
		void operator-=(const Matrix &s) noexcept;
		void operator-=(const float src[16]) noexcept;

		Matrix operator*(const Matrix &s) const noexcept;
		Matrix operator+(const Matrix &s) const noexcept;
		Matrix operator-(const Matrix &s) const noexcept;
		Matrix operator*(const float s) const noexcept;

		constexpr float& operator[](size_t index) noexcept
		{
			return m[index % 16];
		}

		constexpr const float& operator[](size_t index) const noexcept
		{
			return m[index % 16];
		}

		constexpr float* data() noexcept
		{
			return m;
		}

		constexpr const float* data() const noexcept
		{
			return m;
		}

		void transform(float vec[3]) const noexcept;
		void transform(Vector3f &vec) const noexcept;
		void transform(const Vector3f &vec, Vector3f &result) const noexcept;
		void transform(Vector3f * const vec, size_t numVec) const noexcept;

		void transform(Vector4f &vec) const noexcept;
		void transform(const Vector4f &vec, Vector4f &result) const noexcept;
		void transform(Vector4f * const vec, size_t numVec) const noexcept;

		void transform(BBox<> &bbox) const noexcept;
		void transform(const BBox<>&bbox, BBox<>&bboxDest) const noexcept;

		Vector4f getColumn(size_t columnIndex) const noexcept;
		Vector4f getRow(size_t rowIndex) const noexcept;

		void write(float dest[16]) const noexcept;

		Matrix getTranspose() const noexcept;
		Matrix& transpose() noexcept;

		Matrix getInverse() const noexcept;
		Matrix& inverse() noexcept;

		Matrix getInverseTranspose() const noexcept;
		Matrix& inverseTranspose() noexcept;

		Matrix getInverseHomogenous() const noexcept;
		Matrix& inverseHomogenous() noexcept;

		Vector3f extractTranslation() const noexcept;
		Vector3f extractScale() const noexcept;
		Quaternion extractRotation() const noexcept;

		void setTranspose(const float src[16]) noexcept;
		void setTranspose(const Matrix &mat) noexcept;

		void setReflect(const Plane<float> &plane) noexcept;
		void setSaturation(const float sat) noexcept;
	};

	static_assert(std::is_trivially_copyable_v<Matrix>, "For performance reasons, this class should be trivially copyable");

	template<typename TDataType>
	class Matrix3
	{
		static_assert(std::is_same_v<TDataType, float> || std::is_same_v<TDataType, double>, "Base type must be either float or double");

		TDataType m[9];

		friend class Matrix;

		using Vector3Type = typename Vector<TDataType, 3>;

	public:
		using DataType = typename TDataType;

		static constexpr Matrix3 from(const Matrix &mat4) noexcept
		{
			Matrix3 mat;

			mat.m[0] = mat4.m[0];
			mat.m[1] = mat4.m[1];
			mat.m[2] = mat4.m[2];
			mat.m[3] = mat4.m[4];
			mat.m[4] = mat4.m[5];
			mat.m[5] = mat4.m[6];
			mat.m[6] = mat4.m[8];
			mat.m[7] = mat4.m[9];
			mat.m[8] = mat4.m[10];

			return mat;
		}

		static constexpr Matrix3 from(const Quaternion &unitQuaternion) noexcept
		{
			Matrix3 mat;

			if constexpr (std::is_same_v<TDataType, float>)
			{
				mat.m[0] = 1.0f - 2.0f * (unitQuaternion[1] * unitQuaternion[1] + unitQuaternion[2] * unitQuaternion[2]);
				mat.m[3] = 2.0f * (unitQuaternion[0] * unitQuaternion[1] - unitQuaternion[2] * unitQuaternion[3]);
				mat.m[6] = 2.0f * (unitQuaternion[0] * unitQuaternion[2] + unitQuaternion[1] * unitQuaternion[3]);

				mat.m[1] = 2.0f * (unitQuaternion[0] * unitQuaternion[1] + unitQuaternion[2] * unitQuaternion[3]);
				mat.m[4] = 1.0f - 2.0f * (unitQuaternion[0] * unitQuaternion[0] + unitQuaternion[2] * unitQuaternion[2]);
				mat.m[7] = 2.0f * (unitQuaternion[1] * unitQuaternion[2] - unitQuaternion[0] * unitQuaternion[3]);

				mat.m[2] = 2.0f * (unitQuaternion[0] * unitQuaternion[2] - unitQuaternion[1] * unitQuaternion[3]);
				mat.m[5] = 2.0f * (unitQuaternion[1] * unitQuaternion[2] + unitQuaternion[0] * unitQuaternion[3]);
				mat.m[8] = 1.0f - 2.0f * (unitQuaternion[0] * unitQuaternion[0] + unitQuaternion[1] * unitQuaternion[1]);
			}
			else
			{
				mat.m[0] = 1.0 - 2.0 * (unitQuaternion[1] * unitQuaternion[1] + unitQuaternion[2] * unitQuaternion[2]);
				mat.m[3] = 2.0 * (unitQuaternion[0] * unitQuaternion[1] - unitQuaternion[2] * unitQuaternion[3]);
				mat.m[6] = 2.0 * (unitQuaternion[0] * unitQuaternion[2] + unitQuaternion[1] * unitQuaternion[3]);

				mat.m[1] = 2.0 * (unitQuaternion[0] * unitQuaternion[1] + unitQuaternion[2] * unitQuaternion[3]);
				mat.m[4] = 1.0 - 2.0 * (unitQuaternion[0] * unitQuaternion[0] + unitQuaternion[2] * unitQuaternion[2]);
				mat.m[7] = 2.0 * (unitQuaternion[1] * unitQuaternion[2] - unitQuaternion[0] * unitQuaternion[3]);

				mat.m[2] = 2.0 * (unitQuaternion[0] * unitQuaternion[2] - unitQuaternion[1] * unitQuaternion[3]);
				mat.m[5] = 2.0 * (unitQuaternion[1] * unitQuaternion[2] + unitQuaternion[0] * unitQuaternion[3]);
				mat.m[8] = 1.0 - 2.0 * (unitQuaternion[0] * unitQuaternion[0] + unitQuaternion[1] * unitQuaternion[1]);
			}

			return mat;
		}

		template<typename TType>
		static constexpr Matrix3 from(std::span<const TType> data) noexcept
		{
			static_assert(std::is_same_v<TType, float> || std::is_same_v<TType, double>, "Type must be either float or double");

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

		static constexpr Matrix3 from(std::initializer_list<TDataType> il) noexcept
		{
			assert(il.size() == 9);

			Matrix3 mat;

			auto src = std::data(il);
			for (int i = 0; i < 9; i++)
				mat.m[i] = src[i];

			return mat;
		}

		static constexpr Matrix3 from(const TDataType scalar) noexcept
		{
			Matrix3 mat;

			for (int i = 0; i < 9; i++)
				mat.m[i] = scalar;

			return mat;
		}

		static constexpr Matrix3 rotation(const Quaternion &unitQuaternion) noexcept
		{
			return Matrix3::from(unitQuaternion);
		}

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

	private:
		constexpr Matrix3() = default;

	public:
		constexpr Matrix3(const Matrix3&) = default;
		constexpr Matrix3& operator=(const Matrix3&) = default;
		constexpr Matrix3(Matrix3&&) = default;
		constexpr Matrix3& operator=(Matrix3&&) = default;

		Matrix3& operator=(const Matrix& mat) noexcept;
		
		void operator*=(const Matrix &mat) noexcept;
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

		Matrix3 getTranspose() const noexcept;
		Matrix3& transpose() noexcept
		{
			std::swap(m[1], m[3]);
			std::swap(m[2], m[6]);
			std::swap(m[5], m[7]);

			return *this;
		}

		template<typename TNewDataType>
		Matrix3<TNewDataType> convert() const noexcept
		{
			Matrix3<TNewDataType> res;

			for (int i = 0; i < 9; i++)
				res[i] = static_cast<TNewDataType>(res[i]);

			return res;
		}

		Matrix convertToMat4() const noexcept;
	};

	static_assert(std::is_trivially_copyable_v<Matrix3f>, "For performance reasons, this class should be trivially copyable");
	static_assert(std::is_trivially_copyable_v<Matrix3d>, "For performance reasons, this class should be trivially copyable");

	extern template class Matrix3<float>;
	extern template class Matrix3<double>;
}
