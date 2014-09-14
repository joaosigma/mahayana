#pragma once

#include "Types.hpp"
#include "Vector.hpp"
#include "Plane.hpp"
#include "BVolumes.hpp"

namespace HorseRadish
{
	HALIGN_16BYTES
	class Matrix
	{
		float m[16];

		friend class Matrix3;

		static void asmMat4x4Vec3(float *vecWrite, const float *vecRead, const float wCompMul, const unsigned int stride, const float *mat, const unsigned int numVec);
		static void asmMat4x4Vec4(float *vecWrite, const float *vecRead, const unsigned int stride, const float *mat, const unsigned int numVec);
		static void asmMult(float * const result, const float * const mat1, const float * const mat2);

	public:
		Matrix(){ return; }
		~Matrix(){ return; }
		explicit Matrix(const float * const s);
		explicit Matrix(const Matrix &mat);
		explicit Matrix(const Matrix3 &mat);

		void operator*=(const Matrix &s);
		void operator*=(const Matrix3 &s);
		void operator*=(const float *s);
		void operator+=(const Matrix &s);
		void operator+=(const float *s);
		void operator-=(const Matrix &s);
		void operator-=(const float *s);

		float& operator[](const int &i){ return m[i % 16]; }
		operator const float *() const { return m; }

		void TransformVector(float *vec) const;
		void TransformVector(Vector &vec) const;
		void TransformVector(const Vector &vec, Vector &result) const;
		void TransformVector(Vector * const vec, const int numVec) const;

		void TransformVector(Vector4 &vec) const;
		void TransformVector(const Vector4 &vec, Vector4 &result) const;
		void TransformVector(Vector4 * const vec, const int numVec) const;

		void TransformVector3D(float *vec) const;
		void TransformVector3D(Vector &vec) const;
		void TransformVector3D(const Vector &vec, Vector &result) const;
		void TransformVector3D(Vector * const vec, const int numVec) const;

		void TransformBBox(BBox &bbox) const;
		void TransformBBox(const BBox &bbox, BBox &bboxDest) const;

		void GetCol1(Vector4 &result) const;
		void GetCol2(Vector4 &result) const;
		void GetCol3(Vector4 &result) const;
		void GetCol4(Vector4 &result) const;
		void GetRow1(Vector4 &result) const;
		void GetRow2(Vector4 &result) const;
		void GetRow3(Vector4 &result) const;
		void GetRow4(Vector4 &result) const;

		void GetCol1(Vector &result) const;
		void GetCol2(Vector &result) const;
		void GetCol3(Vector &result) const;
		void GetCol4(Vector &result) const;
		void GetRow1(Vector &result) const;
		void GetRow2(Vector &result) const;
		void GetRow3(Vector &result) const;
		void GetRow4(Vector &result) const;

		const float* GetRow1() const { return m + 0; }
		const float* GetRow2() const { return m + 4; }
		const float* GetRow3() const { return m + 8; }
		const float* GetRow4() const { return m + 12; }

		float* GetPointer() { return m; }
		void GetRotate(Vector &vec, float &angulo) const;
		void GetEulerAngles(float &rfYAngle, float &rfPAngle, float &rfRAngle) const;
		void GetFrom3x3(Matrix3 &mat3) const;
		void GetFrom3x3(float* const src) const;
		void GetFrom2x2(float* const src) const;

		void Write(float * const s) const;

		void Transpose(Matrix &dest) const;
		void Transpose(void);
		void Inverse(Matrix &dest) const;
		void Inverse(void);
		void InverseTranspose(Matrix &dest) const;
		void InverseTranspose(void);
		void InverseHomogenous(Matrix &dest) const;
		void InverseHomogenous(void);

		void MultTranslate(const float &x, const float &y, const float &z);
		void MultTranslate(const float * const vec);
		void MultScale(const float &x, const float &y, const float &z);
		void MultScale(const float * const vec);
		void MultRotateX(const float &angulo);
		void MultRotateY(const float &angulo);
		void MultRotateZ(const float &angulo);
		void Mult(const Matrix &s);
		void Mult(const float *s);
		void MultInverseOrder(const Matrix &s);
		void MultInverseOrder(const float *s);

		void Set(const float x);
		void Set(const float *src);
		void Set(const Matrix &mat);
		void SetZero(void);
		void SetIdentidade(void);
		void SetFrom3x3(const Matrix3 &mat3);
		void SetFrom3x3(const float *src);
		void SetFrom2x2(const float *src);
		void SetTranspose(const float *src);
		void SetTranspose(const Matrix &mat);
		void SetTranslate(const float &x, const float &y, const float &z);
		void SetTranslate(const float * const vec);
		void SetTranslate(const Vector &vec);
		void SetScale(const float &x, const float &y, const float &z);
		void SetScale(const Vector &vec);
		void SetReflect(const Plane &plane);
		void SetReflect(const float &a, const float &b, const float &c, const float &d);
		void SetRotateX(const float &angulo);
		void SetRotateY(const float &angulo);
		void SetRotateZ(const float &angulo);
		void SetRotate(const float &angulo, const Vector &vec);
		void SetRotate(const float &anguloX, const float &anguloY, const float &anguloZ);
		void SetRotate(const float &angulo, const float &x, const float &y, const float &z);
		void SetSaturation(const float sat);
		void SetRotationFromTo(const float * const from, const float * const to);
		void SetRotationFromTo(const Vector &from, const Vector &to);

		void SetGLModelView(const Vector &pos, const Vector &target, const Vector &up);
		void SetGLModelView(const Vector &pos, const Vector &target);
		void SetGLModelView(const float *pos, const float *target, const float *up);
		void SetGLModelView(const float *pos, const float *target);
		void SetGLModelView(const Vector &pos, const float angX, const float angY, const Vector &up);
		void SetGLModelView(const float *pos, const float angX, const float angY, const float *up);
		void SetGLModelView(const int cubemapFace, const Vector &centerCube);
		void SetGLProjection3D(const float fovy, const float aspect, const float zNear, const float zFar);
		void SetGLProjection3D(const float fovy, const float aspect, const float zNear);
		void SetGLProjection2D(const float width, const float height);
		void SetGLProjectionOrtho(const float left, const float right, const float bottom, const float top, const float zNear, const float zFar);
		void SetGLProjectionOrtho(const BBox &bbox);
	};

	HALIGN_16BYTES
	class Matrix3
	{
		float m[9];

		friend class Matrix;

	public:
		Matrix3(){ return; }
		~Matrix3(){ return; }
		explicit Matrix3(const float * const s);
		explicit Matrix3(const Matrix &mat);
		explicit Matrix3(const Matrix3 &mat);

		void operator*=(const Matrix &s);
		void operator*=(const Matrix3 &s);
		void operator*=(const float *s);
		void operator+=(const Matrix3 &s);
		void operator+=(const float *s);
		void operator-=(const Matrix3 &s);
		void operator-=(const float *s);

		float& operator[](const int &i){ return m[i % 9]; }
		operator const float *() const { return m; }

		void MulVector(float *vec) const;
		void MulVector(Vector &vec) const;
		void MulVector(const Vector &vec, Vector &result) const;
		void MulVector(Vector * const vec, const int numVec) const;

		void GetCol1(Vector &result) const;
		void GetCol2(Vector &result) const;
		void GetCol3(Vector &result) const;
		void GetRow1(Vector &result) const;
		void GetRow2(Vector &result) const;
		void GetRow3(Vector &result) const;

		const float* GetRow1() const { return m + 0; }
		const float* GetRow2() const { return m + 3; }
		const float* GetRow3() const { return m + 6; }

		float* GetPointer() { return m; }
		void GetRotate(Vector &vec, float &angulo) const;
		void GetEulerAngles(float &rfYAngle, float &rfPAngle, float &rfRAngle) const;

		void Write(float * const s) const;

		void Transpose(Matrix3 &dest) const;
		void Transpose(void);

		void MulRotateX(const float &angulo);
		void MulRotateY(const float &angulo);
		void MulRotateZ(const float &angulo);

		void Set(const float x);
		void Set(const float *src);
		void Set(const Matrix &mat);
		void Set(const Matrix3 &mat);
		void SetZero(void);
		void SetIdentidade(void);
		void SetRotateX(const float &angulo);
		void SetRotateY(const float &angulo);
		void SetRotateZ(const float &angulo);
		void SetRotate(const float &angulo, const Vector &vec);
		void SetRotate(const float &anguloX, const float &anguloY, const float &anguloZ);
		void SetRotate(const float &angulo, const float &x, const float &y, const float &z);
		void SetRotationFromTo(const float * const from, const float * const to);
		void SetRotationFromTo(const Vector &from, const Vector &to);
	};

} //HorseRadish
