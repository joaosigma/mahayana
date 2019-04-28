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
	This means that the layout reads as: [Xx Xy Xz 0 Yx Yy Yz 0 Zx Zy Zz 0 Tx Ty Tz 0]

	So in order for multiplication with vectors to work, they must be read as a "column with 4 rows" and are transformed like: v*M (left of pre-multiplication)
	So, matrix-matrix multiplication happens with a pre-multiple of the transpose:
		Res = Mat1.operator*(Mat2) means Res = Mat2^T * Mat1 (as opposed to Res = Mat1 * Mat2)

	(Unreal4 matrix behaves the same as this one, although this one predates it!)
	*/

	class Matrix
	{
		float m[16]{ 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f };

		friend class Matrix3;

	public:
		static constexpr Matrix genMatTranslate(float x, float y, float z)
		{
			Matrix m;
			m[12] = x;
			m[13] = y;
			m[14] = z;

			return m;
		}

		static constexpr void genMatTranslate(Matrix& mat, float x, float y, float z)
		{
			mat[12] = x;
			mat[13] = y;
			mat[14] = z;
			mat[0] = mat[5] = mat[10] = mat[15] = 1.0f;
			mat[1] = mat[2] = mat[3] = mat[4] = mat[6] = mat[7] = mat[8] = mat[9] = mat[11] = 0.0f;
		}

		static constexpr Matrix genMatScale(float x, float y, float z)
		{
			Matrix m;
			m[0] = x;
			m[5] = y;
			m[10] = z;

			return m;
		}

		static constexpr void genMatScale(Matrix& mat, float x, float y, float z)
		{
			mat[0] = x;
			mat[5] = y;
			mat[10] = z;
			mat[15] = 1.0f;
			mat[1] = mat[2] = mat[3] = mat[4] = mat[6] = mat[7] = mat[8] = mat[9] = mat[11] = mat[12] = mat[13] = mat[14] = 0.0f;
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
		
		void operator*=(const float s);
		void operator*=(const Matrix &s);
		void operator*=(const Matrix3 &s);
		void operator*=(const float src[16]);
		void operator*=(const Quaternion &unitQuaternion);
		void operator+=(const Matrix &s);
		void operator+=(const float src[16]);
		void operator-=(const Matrix &s);
		void operator-=(const float src[16]);

		Matrix operator*(const Matrix &s) const;
		Matrix operator+(const Matrix &s) const;
		Matrix operator-(const Matrix &s) const;
		Matrix operator*(const float s) const;

		constexpr float& operator[](size_t index)
		{
			return m[index % 16];
		}

		constexpr const float& operator[](size_t index) const
		{
			return m[index % 16];
		}

		constexpr float* data()
		{
			return m;
		}

		constexpr const float* data() const
		{
			return m;
		}

		void transform(float vec[3]) const;
		void transform(Vector3f &vec) const;
		void transform(const Vector3f &vec, Vector3f &result) const;
		void transform(Vector3f * const vec, size_t numVec) const;

		void transform(Vector4f &vec) const;
		void transform(const Vector4f &vec, Vector4f &result) const;
		void transform(Vector4f * const vec, size_t numVec) const;

		void transform(BBox &bbox) const;
		void transform(const BBox &bbox, BBox &bboxDest) const;

		Vector4f getColumn(size_t columnIndex) const;
		Vector4f getRow(size_t rowIndex) const;

		void getRotation(Vector3f &vec, float &angle) const;
		void getEulerAngles(float &rfYAngle, float &rfPAngle, float &rfRAngle) const;

		Matrix3 getMat3x3() const;
		void getMat3x3(Matrix3 &mat3) const;
		void getMat3x3(float dest[9]) const;
		void getMat2x2(float dest[4]) const;

		void write(float dest[16]) const;

		Matrix getTranspose() const;
		Matrix& transpose();

		Matrix getInverse() const;
		Matrix& inverse();

		Matrix getInverseTranspose() const;
		Matrix& inverseTranspose();

		Matrix getInverseHomogenous() const;
		Matrix& inverseHomogenous();

		Matrix& mulTranslation(float x, float y, float z);
		Matrix& mulTranslation(const float * const vec);
		Matrix& mulScale(float x, float y, float z);
		Matrix& mulScale(const float * const vec);
		Matrix& mul(const Matrix &s);
		Matrix& mul(const float src[16]);
		Matrix& mulReverseOrder(const Matrix &s);
		Matrix& mulReverseOrder(const float src[16]);

		void set(float value);
		void set(const float src[16]);
		void set(const Matrix &mat);
		void setIdentity(void);
		void setFrom3x3(const Matrix3 &mat3);
		void setFrom3x3(const float src[9]);
		void setFrom2x2(const float src[4]);
		void setTranspose(const float src[16]);
		void setTranspose(const Matrix &mat);
		void setTranslation(float x, float y, float z);
		void setTranslation(const float vec[3]);
		void setTranslation(const Vector3f &vec);
		void setScale(float scale);
		void setScale(float x, float y, float z);
		void setScale(const Vector3f &vec);
		void setReflect(const Plane &plane);
		void setRotationX(float angleDeg);
		void setRotationY(float angleDeg);
		void setRotationZ(float angleDeg);
		void setRotation(float angleDeg, const Vector3f &vec);
		void setRotation(float angleDegX, float angleDegY, float angleDegZ);
		void setSaturation(float sat);
		void setRotationFromTo(const Vector3f &from, const Vector3f &to);

		void setGLModelView(const Vector3f &pos, const Vector3f &target, const Vector3f &up);
		void setGLModelView(const Vector3f &pos, const Vector3f &target);
		void setGLModelView(const Vector3f &pos, float angleDegX, float angleDegY, const Vector3f &up);
		void setGLModelView(int cubemapFace, const Vector3f &centerCube);
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

		explicit constexpr Matrix3(const Matrix &mat)
			: m{ mat.m[0], mat.m[1], mat.m[2], mat.m[4], mat.m[5], mat.m[6], mat.m[8], mat.m[9], mat.m[10] }
		{ }

		explicit constexpr Matrix3(const float src[9])
			: m{ src[0], src[1], src[2], src[3], src[4], src[5], src[6], src[7], src[8] }
		{ }

		explicit Matrix3(const Quaternion &unitQuaternion);
		
		void operator*=(const Matrix &s);
		void operator*=(const Matrix3 &s);
		void operator*=(const float src[9]);
		void operator+=(const Matrix3 &s);
		void operator+=(const float src[9]);
		void operator-=(const Matrix3 &s);
		void operator-=(const float src[9]);

		Matrix3 operator*(const Matrix3 &s) const;
		Matrix3 operator+(const Matrix3 &s) const;
		Matrix3 operator-(const Matrix3 &s) const;

		constexpr float& operator[](const size_t index)
		{
			return m[index % 9];
		}

		constexpr const float& operator[](const size_t index) const
		{
			return m[index % 9];
		}

		constexpr float* data()
		{
			return m;
		}

		constexpr const float* data() const
		{
			return m;
		}

		void transform(float vec[3]) const;
		void transform(Vector3f &vec) const;
		void transform(const Vector3f &vec, Vector3f &result) const;
		void transform(Vector3f * const vec, size_t numVec) const;

		Vector3f getColumn(size_t columnIndex) const;
		Vector3f getRow(size_t rowIndex) const;

		void getRotation(Vector3f &vec, float &angle) const;
		void getEulerAngles(float &rfYAngle, float &rfPAngle, float &rfRAngle) const;

		void write(float dest[9]) const;

		Matrix3 getTranspose() const;
		Matrix3& transpose(void);

		Matrix3& set(float value);
		Matrix3& set(const float src[9]);
		Matrix3& set(const Matrix &mat);
		Matrix3& set(const Matrix3 &mat);
		Matrix3& setZero(void);
		Matrix3& setIdentity(void);
		Matrix3& setRotationX(float angleDeg);
		Matrix3& setRotationY(float angleDeg);
		Matrix3& setRotationZ(float angleDeg);
		Matrix3& setRotation(float angleDeg, const Vector3f &vec);
		Matrix3& setRotation(float angleDegX, float angleDegY, float angleDegZ);
		Matrix3& setRotation(float angleDeg, float x, float y, float z);
		Matrix3& setRotationFromTo(const Vector3f &from, const Vector3f &to);
	};
}
