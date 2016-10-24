#pragma once

#include "types.hpp"
#include "vector.hpp"
#include "plane.hpp"
#include "bvolumes.hpp"

namespace HorseRadish
{
	class Matrix
	{
		float m[16];

		friend class Matrix3;

		static void asmMat4x4Vec3(float *vecWrite, const float *vecRead, float wCompMul, size_t stride, const float *mat, size_t numVec);
		static void asmMat4x4Vec4(float *vecWrite, const float *vecRead, size_t stride, const float *mat, size_t numVec);
		static void fastMat4x4Mult(float * const result, const float * const mat1, const float * const mat2);

	public:
		Matrix()
		{
			std::memset(m, 0, sizeof(float) * 16);
			m[0] = m[5] = m[10] = m[15] = 1.0f;
		}

		explicit Matrix(const Matrix3 &mat);
		explicit Matrix(const float src[16]);
		
		void operator*=(const Matrix &s);
		void operator*=(const Matrix3 &s);
		void operator*=(const float src[16]);
		void operator+=(const Matrix &s);
		void operator+=(const float src[16]);
		void operator-=(const Matrix &s);
		void operator-=(const float src[16]);

		Matrix operator*(const Matrix &s) const;
		Matrix operator+(const Matrix &s) const;
		Matrix operator-(const Matrix &s) const;

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

		void rotateScale(float vec[3]) const;
		void rotateScale(Vector3f &vec) const;
		void rotateScale(const Vector3f &vec, Vector3f &result) const;
		void rotateScale(Vector3f * const vec, size_t numVec) const;

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

		Matrix transpose() const;
		void transpose();

		Matrix inverse() const;
		void inverse();

		Matrix inverseTranspose() const;
		void inverseTranspose();

		Matrix inverseHomogenous() const;
		void inverseHomogenous();

		Matrix& mulTranslation(float x, float y, float z);
		Matrix& mulTranslation(const float * const vec);
		Matrix& mulScale(float x, float y, float z);
		Matrix& mulScale(const float * const vec);
		Matrix& mulRotationX(float angleDeg);
		Matrix& mulRotationY(float angleDeg);
		Matrix& mulRotationZ(float angleDeg);
		Matrix& mul(const Matrix &s);
		Matrix& mul(const float src[16]);
		Matrix& mulReverseOrder(const Matrix &s);
		Matrix& mulReverseOrder(const float src[16]);

		void set(float value);
		void set(const float src[16]);
		void set(const Matrix &mat);
		void setZero(void);
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
		void setReflect(float a, float b, float c, float d);
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
		void setGLProjection3D(float fovy, float aspect, float zNear, float zFar);
		void setGLProjection3D(float fovy, float aspect, float zNear);
		void setGLProjection2D(float width, float height);
		void setGLProjectionOrtho(float left, float right, float bottom, float top, float zNear, float zFar);
		void setGLProjectionOrtho(const BBox &bbox);
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

		void transpose(Matrix3 &dest) const;
		void transpose(void);

		void mulRotationX(float angleDeg);
		void mulRotationY(float angleDeg);
		void mulRotationZ(float angleDeg);

		void set(float value);
		void set(const float src[9]);
		void set(const Matrix &mat);
		void set(const Matrix3 &mat);
		void setZero(void);
		void setIdentity(void);
		void setRotationX(float angleDeg);
		void setRotationY(float angleDeg);
		void setRotationZ(float angleDeg);
		void setRotation(float angleDeg, const Vector3f &vec);
		void setRotation(float angleDegX, float angleDegY, float angleDegZ);
		void setRotation(float angleDeg, float x, float y, float z);
		void setRotationFromTo(const Vector3f &from, const Vector3f &to);
	};

} //HorseRadish
