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
		float m[16];

		friend class Matrix3;

	public:
		Matrix() noexcept
		{
			std::memset(m, 0, sizeof(float) * 16);
			m[0] = m[5] = m[10] = m[15] = 1.0f;
		}

		Matrix(const Matrix& mat) noexcept
		{
			std::memcpy(m, mat.m, sizeof(float) * 16);
		}

		explicit Matrix(const Matrix3 &mat) noexcept;
		explicit Matrix(const float src[16]) noexcept;
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

		float& operator[](size_t index)
		{
			return m[index % 16];
		}

		const float& operator[](size_t index) const
		{
			return m[index % 16];
		}

		float* data()
		{
			return m;
		}

		const float* data() const
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
		float m[9];

		friend class Matrix;

	public:
		Matrix3()
		{
			std::memset(m, 0, sizeof(float) * 9);
			m[0] = m[4] = m[8] = 1.0f;
		}

		explicit Matrix3(const Matrix &mat);
		explicit Matrix3(const float src[9]);
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

		float& operator[](const size_t index)
		{
			return m[index % 9];
		}

		const float& operator[](const size_t index) const
		{
			return m[index % 9];
		}

		float* data()
		{
			return m;
		}

		const float* data() const
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
