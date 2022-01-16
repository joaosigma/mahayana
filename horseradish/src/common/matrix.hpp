#pragma once

#include "types.hpp"
#include "vector.hpp"
#include "plane.hpp"
#include "bvolumes.hpp"
#include "quaternion.hpp"

namespace hr
{
	/*
	The matrix classes uses a row-major mathematical convention and layout
		row 0 is indices [0 ... 3]
		row 1 is indices [4 ... 7]
		...
	This means that the layout reads as: [Xx Xy Xz 0.0 Yx Yy Yz 0.0 Zx Zy Zz 0.0 Tx Ty Tz 1.0]

	So in order for multiplication with vectors to work, they must be read as a "column with 4 rows" and are transformed like: v*M (left of pre-multiplication)
	This also means that product (multiplication) order, if you want to translate and *then* rotate, is: res = translation * rotation

	Rotations are left-handed, which means positive rotation is clockwise about the axis of rotation (as pointing towards the negative values):
		- plus X points right, plus Y points up and plus Z points forward (to the horizon)
		- this is the same as in quaternions
	*/

	class Matrix
	{
		float m[16]{ 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f };

		friend class Matrix3;

	public:
		static constexpr Matrix genMatIdentity() noexcept
		{
			Matrix m;
			m.m[0] = m.m[5] = m.m[10] = m.m[15] = 1.0f;
			m.m[1] = m.m[2] = m.m[3] = m.m[4] = m.m[6] = m.m[7] = m.m[8] = m.m[9] = m.m[11] = m.m[12] = m.m[13] = m.m[14] = 0.0f;

			return m;
		}

		static constexpr void genMatIdentity(Matrix& mat) noexcept
		{
			mat.m[0] = mat.m[5] = mat.m[10] = mat.m[15] = 1.0f;
			mat.m[1] = mat.m[2] = mat.m[3] = mat.m[4] = mat.m[6] = mat.m[7] = mat.m[8] = mat.m[9] = mat.m[11] = mat.m[12] = mat.m[13] = mat.m[14] = 0.0f;
		}

		static constexpr Matrix genMatTranslate(float x, float y, float z) noexcept
		{
			Matrix m;
			m.m[0] = m.m[5] = m.m[10] = m.m[15] = 1.0f;
			m.m[1] = m.m[2] = m.m[3] = m.m[4] = m.m[6] = m.m[7] = m.m[8] = m.m[9] = m.m[11] = 0.0f;
			m.m[12] = x;
			m.m[13] = y;
			m.m[14] = z;

			return m;
		}

		static constexpr Matrix genMatTranslate(const Vector3f& translate) noexcept
		{
			return genMatTranslate(translate[0], translate[1], translate[2]);
		}

		static constexpr void genMatTranslate(Matrix& mat, float x, float y, float z) noexcept
		{
			mat.m[0] = mat.m[5] = mat.m[10] = mat.m[15] = 1.0f;
			mat.m[1] = mat.m[2] = mat.m[3] = mat.m[4] = mat.m[6] = mat.m[7] = mat.m[8] = mat.m[9] = mat.m[11] = 0.0f;
			mat.m[12] = x;
			mat.m[13] = y;
			mat.m[14] = z;
		}

		static constexpr void genMatTranslate(Matrix& mat, const Vector3f& translate) noexcept
		{
			genMatTranslate(mat, translate[0], translate[1], translate[2]);
		}

		static constexpr Matrix genMatScale(float x, float y, float z) noexcept
		{
			Matrix m;
			m.m[0] = x;
			m.m[5] = y;
			m.m[10] = z;
			m.m[15] = 1.0f;
			m.m[1] = m.m[2] = m.m[3] = m.m[4] = m.m[6] = m.m[7] = m.m[8] = m.m[9] = m.m[11] = m.m[12] = m.m[13] = m.m[14] = 0.0f;

			return m;
		}

		static constexpr Matrix genMatScale(const Vector3f& scale) noexcept
		{
			return genMatScale(scale[0], scale[1], scale[2]);
		}

		static constexpr void genMatScale(Matrix& mat, float x, float y, float z) noexcept
		{
			mat.m[0] = x;
			mat.m[5] = y;
			mat.m[10] = z;
			mat.m[15] = 1.0f;
			mat.m[1] = mat.m[2] = mat.m[3] = mat.m[4] = mat.m[6] = mat.m[7] = mat.m[8] = mat.m[9] = mat.m[11] = mat.m[12] = mat.m[13] = mat.m[14] = 0.0f;
		}

		static constexpr void genMatScale(Matrix& mat, const Vector3f& scale) noexcept
		{
			genMatScale(mat, scale[0], scale[1], scale[2]);
		}

	public:
		constexpr Matrix() = default;
		constexpr Matrix(const Matrix&) = default;
		constexpr Matrix& operator=(const Matrix&) = default;
		constexpr Matrix(Matrix&&) = default;
		constexpr Matrix& operator=(Matrix&&) = default;

		explicit constexpr Matrix(const float src[16]) noexcept
			: m{ src[0], src[1], src[2], src[3], src[4], src[5], src[6], src[7], src[8], src[9], src[10], src[11], src[12], src[13], src[14], src[15] }
		{ }	

		explicit Matrix(const Matrix3 &mat) noexcept;
		explicit Matrix(const double src[16]) noexcept;
		explicit Matrix(const Quaternion &unitQuaternion) noexcept;
		
		void operator*=(const float s) noexcept;
		void operator*=(const Matrix &s) noexcept;
		void operator*=(const Matrix3 &s) noexcept;
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

		Matrix3 getMat3x3() const noexcept;
		void getMat3x3(Matrix3 &mat3) const noexcept;
		void getMat3x3(float dest[9]) const noexcept;
		void getMat2x2(float dest[4]) const noexcept;

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

		void set(float value) noexcept;
		void set(const float src[16]) noexcept;
		void set(const Matrix &mat) noexcept;
		void setIdentity(void) noexcept;
		void setFrom3x3(const Matrix3 &mat3) noexcept;
		void setFrom3x3(const float src[9]) noexcept;
		void setFrom2x2(const float src[4]) noexcept;
		void setTranspose(const float src[16]) noexcept;
		void setTranspose(const Matrix &mat) noexcept;
		void setTranslation(float x, float y, float z) noexcept;
		void setTranslation(const float vec[3]) noexcept;
		void setTranslation(const Vector3f &vec) noexcept;
		void setScale(float scale) noexcept;
		void setScale(float x, float y, float z) noexcept;
		void setScale(const Vector3f &vec) noexcept;
		void setReflect(const Plane<float> &plane) noexcept;
		void setRotationX(const float angleDeg) noexcept;
		void setRotationY(const float angleDeg) noexcept;
		void setRotationZ(const float angleDeg) noexcept;
		void setRotation(const Vector3f& unitVec, const float angleDeg) noexcept;
		void setSaturation(const float sat) noexcept;

		void setGLModelView(const Vector3f &pos, const Vector3f &target, const Vector3f &up) noexcept;
		void setGLModelView(const Vector3f &pos, const Vector3f &target) noexcept;
		void setGLModelView(const Vector3f &pos, float angleDegX, float angleDegY, const Vector3f &up) noexcept;
		void setGLModelView(int cubemapFace, const Vector3f &centerCube) noexcept;
	};

	class Matrix3
	{
		float m[9]{ 1.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 1.0f };

		friend class Matrix;

	public:
		constexpr Matrix3() = default;
		constexpr Matrix3(const Matrix3&) = default;
		constexpr Matrix3& operator=(const Matrix3&) = default;
		constexpr Matrix3(Matrix3&&) = default;
		constexpr Matrix3& operator=(Matrix3&&) = default;

		explicit constexpr Matrix3(const Matrix &mat) noexcept
			: m{ mat.m[0], mat.m[1], mat.m[2], mat.m[4], mat.m[5], mat.m[6], mat.m[8], mat.m[9], mat.m[10] }
		{ }

		explicit constexpr Matrix3(const float src[9]) noexcept
			: m{ src[0], src[1], src[2], src[3], src[4], src[5], src[6], src[7], src[8] }
		{ }

		explicit Matrix3(const Quaternion &unitQuaternion) noexcept;

		Matrix3& operator=(const Matrix& mat) noexcept;
		
		void operator*=(const Matrix &s) noexcept;
		void operator*=(const Matrix3 &s) noexcept;
		void operator*=(const float src[9]) noexcept;
		void operator+=(const Matrix3 &s) noexcept;
		void operator+=(const float src[9]) noexcept;
		void operator-=(const Matrix3 &s) noexcept;
		void operator-=(const float src[9]) noexcept;

		Matrix3 operator*(const Matrix3 &s) const noexcept;
		Matrix3 operator+(const Matrix3 &s) const noexcept;
		Matrix3 operator-(const Matrix3 &s) const noexcept;

		constexpr float& operator[](const size_t index) noexcept
		{
			return m[index % 9];
		}

		constexpr const float& operator[](const size_t index) const noexcept
		{
			return m[index % 9];
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

		Vector3f getColumn(size_t columnIndex) const noexcept;
		Vector3f getRow(size_t rowIndex) const noexcept;

		void write(float dest[9]) const noexcept;

		Matrix3 getTranspose() const noexcept;
		Matrix3& transpose() noexcept;

		Matrix3& set(float value) noexcept;
		Matrix3& set(const float src[9]) noexcept;
		Matrix3& set(const Matrix &mat) noexcept;
		Matrix3& set(const Matrix3 &mat) noexcept;
		Matrix3& setZero() noexcept;
		Matrix3& setIdentity() noexcept;
		Matrix3& setRotationX(const float angleDeg) noexcept;
		Matrix3& setRotationY(const float angleDeg) noexcept;
		Matrix3& setRotationZ(const float angleDeg) noexcept;
		Matrix3& setRotation(const Vector3f& unitVec, const float angleDeg) noexcept;
	};
}
