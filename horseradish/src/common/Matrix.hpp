#pragma once

#include "Types.hpp"
#include "Vector.hpp"
#include "Plane.hpp"
#include "BVolumes.hpp"

namespace HorseRadish
{
	class Matrix
	{
		float m[16];

		friend class Matrix3;

		static void asmMat4x4Vec3(float *vecWrite, const float *vecRead, const float wCompMul, const unsigned int stride, const float *mat, const unsigned int numVec);
		static void asmMat4x4Vec4(float *vecWrite, const float *vecRead, const unsigned int stride, const float *mat, const unsigned int numVec);
		static void fastMat4x4Mult(float * const result, const float * const mat1, const float * const mat2);

	public:
		Matrix();
		Matrix(const Matrix &mat);
		Matrix(const Matrix3 &mat);
		explicit Matrix(const float * const s);
		
		void operator*=(const Matrix &s);
		void operator*=(const Matrix3 &s);
		void operator*=(const float *s);
		void operator+=(const Matrix &s);
		void operator+=(const float *s);
		void operator-=(const Matrix &s);
		void operator-=(const float *s);

		float& operator[](const size_t index)
		{
			return m[index % 16];
		}

		const float& operator[](const size_t index) const
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

		void transform(float *vec) const;
		void transform(Vector3f &vec) const;
		void transform(const Vector3f &vec, Vector3f &result) const;
		void transform(Vector3f * const vec, const int numVec) const;

		void transform(Vector4f &vec) const;
		void transform(const Vector4f &vec, Vector4f &result) const;
		void transform(Vector4f * const vec, const int numVec) const;

		void rotateScale(float *vec) const;
		void rotateScale(Vector3f &vec) const;
		void rotateScale(const Vector3f &vec, Vector3f &result) const;
		void rotateScale(Vector3f * const vec, const int numVec) const;

		void transform(BBox &bbox) const;
		void transform(const BBox &bbox, BBox &bboxDest) const;

		Vector4f getColumn(unsigned int columnIndex) const;
		Vector4f getRow(unsigned int rowIndex) const;

		void getRotation(Vector3f &vec, float &angle) const;
		void getEulerAngles(float &rfYAngle, float &rfPAngle, float &rfRAngle) const;
		void getMat3x3(Matrix3 &mat3) const;
		void getMat3x3(float* const src) const;
		void getMat2x2(float* const src) const;

		void write(float* const s) const;

		void transpose(Matrix &dest) const;
		void transpose(void);
		void inverse(Matrix &dest) const;
		void inverse(void);
		void inverseTranspose(Matrix &dest) const;
		void inverseTranspose(void);
		void inverseHomogenous(Matrix &dest) const;
		void inverseHomogenous(void);

		Matrix& mulTranslation(const float &x, const float &y, const float &z);
		Matrix& mulTranslation(const float * const vec);
		Matrix& mulScale(const float &x, const float &y, const float &z);
		Matrix& mulScale(const float * const vec);
		Matrix& mulRotationX(const float &angle);
		Matrix& mulRotationY(const float &angle);
		Matrix& mulRotationZ(const float &angle);
		Matrix& mul(const Matrix &s);
		Matrix& mul(const float *s);
		Matrix& mulReverseOrder(const Matrix &s);
		Matrix& mulReverseOrder(const float *s);

		void set(const float x);
		void set(const float *src);
		void set(const Matrix &mat);
		void setZero(void);
		void setIdentity(void);
		void setFrom3x3(const Matrix3 &mat3);
		void setFrom3x3(const float *src);
		void setFrom2x2(const float *src);
		void setTranspose(const float *src);
		void setTranspose(const Matrix &mat);
		void setTranslation(const float &x, const float &y, const float &z);
		void setTranslation(const float * const vec);
		void setTranslation(const Vector3f &vec);
		void setScale(const float scale);
		void setScale(const float &x, const float &y, const float &z);
		void setScale(const Vector3f &vec);
		void setReflect(const Plane &plane);
		void setReflect(const float &a, const float &b, const float &c, const float &d);
		void setRotationX(const float &angle);
		void setRotationY(const float &angle);
		void setRotationZ(const float &angle);
		void setRotation(const float &angle, const Vector3f &vec);
		void setRotation(const float &angleX, const float &angleY, const float &angleZ);
		void setSaturation(const float sat);
		void setRotationFromTo(const Vector3f &from, const Vector3f &to);

		void setGLModelView(const Vector3f &pos, const Vector3f &target, const Vector3f &up);
		void setGLModelView(const Vector3f &pos, const Vector3f &target);
		void setGLModelView(const Vector3f &pos, const float angX, const float angY, const Vector3f &up);
		void setGLModelView(const int cubemapFace, const Vector3f &centerCube);
		void setGLProjection3D(const float fovy, const float aspect, const float zNear, const float zFar);
		void setGLProjection3D(const float fovy, const float aspect, const float zNear);
		void setGLProjection2D(const float width, const float height);
		void setGLProjectionOrtho(const float left, const float right, const float bottom, const float top, const float zNear, const float zFar);
		void setGLProjectionOrtho(const BBox &bbox);
	};

	class Matrix3
	{
		float m[9];

		friend class Matrix;

	public:
		Matrix3();
		Matrix3(const Matrix &mat);
		Matrix3(const Matrix3 &mat);
		explicit Matrix3(const float * const s);

		void operator*=(const Matrix &s);
		void operator*=(const Matrix3 &s);
		void operator*=(const float *s);
		void operator+=(const Matrix3 &s);
		void operator+=(const float *s);
		void operator-=(const Matrix3 &s);
		void operator-=(const float *s);

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

		void transform(float *vec) const;
		void transform(Vector3f &vec) const;
		void transform(const Vector3f &vec, Vector3f &result) const;
		void transform(Vector3f * const vec, const int numVec) const;

		Vector3f getColumn(unsigned int columnIndex) const;
		Vector3f getRow(unsigned int rowIndex) const;

		void getRotation(Vector3f &vec, float &angulo) const;
		void getEulerAngles(float &rfYAngle, float &rfPAngle, float &rfRAngle) const;

		void write(float * const s) const;

		void transpose(Matrix3 &dest) const;
		void transpose(void);

		void mulRotationX(const float &angle);
		void mulRotationY(const float &angle);
		void mulRotationZ(const float &angle);

		void set(const float x);
		void set(const float *src);
		void set(const Matrix &mat);
		void set(const Matrix3 &mat);
		void setZero(void);
		void setIdentity(void);
		void setRotationX(const float &angle);
		void setRotationY(const float &angle);
		void setRotationZ(const float &angle);
		void setRotation(const float &angle, const Vector3f &vec);
		void setRotation(const float &angleX, const float &angleY, const float &angleZ);
		void setRotation(const float &angle, const float &x, const float &y, const float &z);
		void setRotationFromTo(const Vector3f &from, const Vector3f &to);
	};

} //HorseRadish
