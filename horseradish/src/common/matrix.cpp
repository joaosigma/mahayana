#include "matrix.hpp"

#include "math.hpp"

#include <cstring>
#include <immintrin.h>
#include <xmmintrin.h>

namespace HorseRadish
{
	void Matrix::asmMat4x4Vec3(float *vecWrite, const float *vecRead, float wCompMul, size_t stride, const float *mat, size_t numVec)
	{
		unsigned int leftOver;
		__m128 mat1, mat2, mat3, mat4, final;

		mat1 = _mm_loadu_ps(mat + 0);
		mat2 = _mm_loadu_ps(mat + 4);
		mat3 = _mm_loadu_ps(mat + 8);
		mat4 = _mm_mul_ps(_mm_loadu_ps(mat + 12), _mm_load_ps1(&wCompMul));

		leftOver = numVec;
		for (; leftOver >= 4; leftOver -= 4)
		{
			final = _mm_mul_ps(_mm_load_ps1(vecRead + 0), mat1);
			final = _mm_add_ps(final, _mm_mul_ps(_mm_load_ps1(vecRead + 1), mat2));
			final = _mm_add_ps(final, _mm_mul_ps(_mm_load_ps1(vecRead + 2), mat3));
			final = _mm_add_ps(final, mat4);
			_mm_storel_pi((__m64 *)vecWrite, final);
			_mm_store_ss(vecWrite + 2, _mm_movehl_ps(final, final));

			vecRead = (float*)(((unsigned char*)vecRead) + stride);
			vecWrite = (float*)(((unsigned char*)vecWrite) + stride);

			final = _mm_mul_ps(_mm_load_ps1(vecRead + 0), mat1);
			final = _mm_add_ps(final, _mm_mul_ps(_mm_load_ps1(vecRead + 1), mat2));
			final = _mm_add_ps(final, _mm_mul_ps(_mm_load_ps1(vecRead + 2), mat3));
			final = _mm_add_ps(final, mat4);
			_mm_storel_pi((__m64 *)vecWrite, final);
			_mm_store_ss(vecWrite + 2, _mm_movehl_ps(final, final));

			vecRead = (float*)(((unsigned char*)vecRead) + stride);
			vecWrite = (float*)(((unsigned char*)vecWrite) + stride);

			final = _mm_mul_ps(_mm_load_ps1(vecRead + 0), mat1);
			final = _mm_add_ps(final, _mm_mul_ps(_mm_load_ps1(vecRead + 1), mat2));
			final = _mm_add_ps(final, _mm_mul_ps(_mm_load_ps1(vecRead + 2), mat3));
			final = _mm_add_ps(final, mat4);
			_mm_storel_pi((__m64 *)vecWrite, final);
			_mm_store_ss(vecWrite + 2, _mm_movehl_ps(final, final));

			vecRead = (float*)(((unsigned char*)vecRead) + stride);
			vecWrite = (float*)(((unsigned char*)vecWrite) + stride);

			final = _mm_mul_ps(_mm_load_ps1(vecRead + 0), mat1);
			final = _mm_add_ps(final, _mm_mul_ps(_mm_load_ps1(vecRead + 1), mat2));
			final = _mm_add_ps(final, _mm_mul_ps(_mm_load_ps1(vecRead + 2), mat3));
			final = _mm_add_ps(final, mat4);
			_mm_storel_pi((__m64 *)vecWrite, final);
			_mm_store_ss(vecWrite + 2, _mm_movehl_ps(final, final));

			vecRead = (float*)(((unsigned char*)vecRead) + stride);
			vecWrite = (float*)(((unsigned char*)vecWrite) + stride);
		}

		for (; leftOver > 0; leftOver--)
		{
			final = _mm_mul_ps(_mm_load_ps1(vecRead + 0), mat1);
			final = _mm_add_ps(final, _mm_mul_ps(_mm_load_ps1(vecRead + 1), mat2));
			final = _mm_add_ps(final, _mm_mul_ps(_mm_load_ps1(vecRead + 2), mat3));
			final = _mm_add_ps(final, mat4);
			_mm_storel_pi((__m64 *)vecWrite, final);
			_mm_store_ss(vecWrite + 2, _mm_movehl_ps(final, final));

			vecRead = (float*)(((unsigned char*)vecRead) + stride);
			vecWrite = (float*)(((unsigned char*)vecWrite) + stride);
		}
	}

	void Matrix::asmMat4x4Vec4(float *vecWrite, const float *vecRead, size_t stride, const float *mat, size_t numVec)
	{
		unsigned int leftOver;
		__m128 mat1, mat2, mat3, mat4, final, curVec;

		mat1 = _mm_loadu_ps(mat + 0);
		mat2 = _mm_loadu_ps(mat + 4);
		mat3 = _mm_loadu_ps(mat + 8);
		mat4 = _mm_loadu_ps(mat + 12);

		leftOver = numVec;

		if ((reinterpret_cast<uintptr_t>(vecWrite) % 16 == 0) && (reinterpret_cast<uintptr_t>(vecRead) % 16 == 0) && (stride % 16 == 0))
		{
			for (; leftOver >= 4; leftOver -= 4)
			{
				curVec = _mm_load_ps(vecRead);
				final = _mm_mul_ps(_mm_shuffle_ps(curVec, curVec, 0x00), mat1);
				final = _mm_add_ps(final, _mm_mul_ps(_mm_shuffle_ps(curVec, curVec, 0x55), mat2));
				final = _mm_add_ps(final, _mm_mul_ps(_mm_shuffle_ps(curVec, curVec, 0xAA), mat3));
				final = _mm_add_ps(final, _mm_mul_ps(_mm_shuffle_ps(curVec, curVec, 0xFF), mat4));
				_mm_store_ps(vecWrite, final);

				vecRead = (float*)(((unsigned char*)vecRead) + stride);
				vecWrite = (float*)(((unsigned char*)vecWrite) + stride);

				curVec = _mm_load_ps(vecRead);
				final = _mm_mul_ps(_mm_shuffle_ps(curVec, curVec, 0x00), mat1);
				final = _mm_add_ps(final, _mm_mul_ps(_mm_shuffle_ps(curVec, curVec, 0x55), mat2));
				final = _mm_add_ps(final, _mm_mul_ps(_mm_shuffle_ps(curVec, curVec, 0xAA), mat3));
				final = _mm_add_ps(final, _mm_mul_ps(_mm_shuffle_ps(curVec, curVec, 0xFF), mat4));
				_mm_store_ps(vecWrite, final);

				vecRead = (float*)(((unsigned char*)vecRead) + stride);
				vecWrite = (float*)(((unsigned char*)vecWrite) + stride);

				curVec = _mm_load_ps(vecRead);
				final = _mm_mul_ps(_mm_shuffle_ps(curVec, curVec, 0x00), mat1);
				final = _mm_add_ps(final, _mm_mul_ps(_mm_shuffle_ps(curVec, curVec, 0x55), mat2));
				final = _mm_add_ps(final, _mm_mul_ps(_mm_shuffle_ps(curVec, curVec, 0xAA), mat3));
				final = _mm_add_ps(final, _mm_mul_ps(_mm_shuffle_ps(curVec, curVec, 0xFF), mat4));
				_mm_store_ps(vecWrite, final);

				vecRead = (float*)(((unsigned char*)vecRead) + stride);
				vecWrite = (float*)(((unsigned char*)vecWrite) + stride);

				curVec = _mm_load_ps(vecRead);
				final = _mm_mul_ps(_mm_shuffle_ps(curVec, curVec, 0x00), mat1);
				final = _mm_add_ps(final, _mm_mul_ps(_mm_shuffle_ps(curVec, curVec, 0x55), mat2));
				final = _mm_add_ps(final, _mm_mul_ps(_mm_shuffle_ps(curVec, curVec, 0xAA), mat3));
				final = _mm_add_ps(final, _mm_mul_ps(_mm_shuffle_ps(curVec, curVec, 0xFF), mat4));
				_mm_store_ps(vecWrite, final);

				vecRead = (float*)(((unsigned char*)vecRead) + stride);
				vecWrite = (float*)(((unsigned char*)vecWrite) + stride);
			}
		}
		else
		{
			for (; leftOver >= 4; leftOver -= 4)
			{
				curVec = _mm_loadu_ps(vecRead);
				final = _mm_mul_ps(_mm_shuffle_ps(curVec, curVec, 0x00), mat1);
				final = _mm_add_ps(final, _mm_mul_ps(_mm_shuffle_ps(curVec, curVec, 0x55), mat2));
				final = _mm_add_ps(final, _mm_mul_ps(_mm_shuffle_ps(curVec, curVec, 0xAA), mat3));
				final = _mm_add_ps(final, _mm_mul_ps(_mm_shuffle_ps(curVec, curVec, 0xFF), mat4));
				_mm_storeu_ps(vecWrite, final);

				vecRead = (float*)(((unsigned char*)vecRead) + stride);
				vecWrite = (float*)(((unsigned char*)vecWrite) + stride);

				curVec = _mm_loadu_ps(vecRead);
				final = _mm_mul_ps(_mm_shuffle_ps(curVec, curVec, 0x00), mat1);
				final = _mm_add_ps(final, _mm_mul_ps(_mm_shuffle_ps(curVec, curVec, 0x55), mat2));
				final = _mm_add_ps(final, _mm_mul_ps(_mm_shuffle_ps(curVec, curVec, 0xAA), mat3));
				final = _mm_add_ps(final, _mm_mul_ps(_mm_shuffle_ps(curVec, curVec, 0xFF), mat4));
				_mm_storeu_ps(vecWrite, final);

				vecRead = (float*)(((unsigned char*)vecRead) + stride);
				vecWrite = (float*)(((unsigned char*)vecWrite) + stride);

				curVec = _mm_loadu_ps(vecRead);
				final = _mm_mul_ps(_mm_shuffle_ps(curVec, curVec, 0x00), mat1);
				final = _mm_add_ps(final, _mm_mul_ps(_mm_shuffle_ps(curVec, curVec, 0x55), mat2));
				final = _mm_add_ps(final, _mm_mul_ps(_mm_shuffle_ps(curVec, curVec, 0xAA), mat3));
				final = _mm_add_ps(final, _mm_mul_ps(_mm_shuffle_ps(curVec, curVec, 0xFF), mat4));
				_mm_storeu_ps(vecWrite, final);

				vecRead = (float*)(((unsigned char*)vecRead) + stride);
				vecWrite = (float*)(((unsigned char*)vecWrite) + stride);

				curVec = _mm_loadu_ps(vecRead);
				final = _mm_mul_ps(_mm_shuffle_ps(curVec, curVec, 0x00), mat1);
				final = _mm_add_ps(final, _mm_mul_ps(_mm_shuffle_ps(curVec, curVec, 0x55), mat2));
				final = _mm_add_ps(final, _mm_mul_ps(_mm_shuffle_ps(curVec, curVec, 0xAA), mat3));
				final = _mm_add_ps(final, _mm_mul_ps(_mm_shuffle_ps(curVec, curVec, 0xFF), mat4));
				_mm_storeu_ps(vecWrite, final);

				vecRead = (float*)(((unsigned char*)vecRead) + stride);
				vecWrite = (float*)(((unsigned char*)vecWrite) + stride);
			}
		}

		for (; leftOver > 0; leftOver--)
		{
			curVec = _mm_loadu_ps(vecRead);
			final = _mm_mul_ps(_mm_shuffle_ps(curVec, curVec, 0x00), mat1);
			final = _mm_add_ps(final, _mm_mul_ps(_mm_shuffle_ps(curVec, curVec, 0x55), mat2));
			final = _mm_add_ps(final, _mm_mul_ps(_mm_shuffle_ps(curVec, curVec, 0xAA), mat3));
			final = _mm_add_ps(final, _mm_mul_ps(_mm_shuffle_ps(curVec, curVec, 0xFF), mat4));
			_mm_storeu_ps(vecWrite, final);

			vecRead = (float*)(((unsigned char*)vecRead) + stride);
			vecWrite = (float*)(((unsigned char*)vecWrite) + stride);
		}
	}
	
	void Matrix::fastMat4x4Mult(float * const result, const float * const mat1, const float * const mat2)
	{
		_mm256_zeroupper();

		__m256 a0, a1;
		a0 = _mm256_loadu_ps(mat2 + 0);
		a1 = _mm256_loadu_ps(mat2 + 8);

		__m128 b0, b1, b2, b3;
		b0 = _mm_loadu_ps(mat1 + 0);
		b1 = _mm_loadu_ps(mat1 + 4);
		b2 = _mm_loadu_ps(mat1 + 8);
		b3 = _mm_loadu_ps(mat1 + 12);

		__m256 out0;
		out0 = _mm256_mul_ps(_mm256_shuffle_ps(a0, a0, 0x00), _mm256_broadcast_ps(&b0));
		out0 = _mm256_add_ps(out0, _mm256_mul_ps(_mm256_shuffle_ps(a0, a0, 0x55), _mm256_broadcast_ps(&b1)));
		out0 = _mm256_add_ps(out0, _mm256_mul_ps(_mm256_shuffle_ps(a0, a0, 0xaa), _mm256_broadcast_ps(&b2)));
		out0 = _mm256_add_ps(out0, _mm256_mul_ps(_mm256_shuffle_ps(a0, a0, 0xff), _mm256_broadcast_ps(&b3)));

		__m256 out1;
		out1 = _mm256_mul_ps(_mm256_shuffle_ps(a1, a1, 0x00), _mm256_broadcast_ps(&b0));
		out1 = _mm256_add_ps(out1, _mm256_mul_ps(_mm256_shuffle_ps(a1, a1, 0x55), _mm256_broadcast_ps(&b1)));
		out1 = _mm256_add_ps(out1, _mm256_mul_ps(_mm256_shuffle_ps(a1, a1, 0xaa), _mm256_broadcast_ps(&b2)));
		out1 = _mm256_add_ps(out1, _mm256_mul_ps(_mm256_shuffle_ps(a1, a1, 0xff), _mm256_broadcast_ps(&b3)));

		_mm256_storeu_ps(result + 0, out0);
		_mm256_storeu_ps(result + 8, out1);
	}

	Matrix::Matrix(const Matrix3 &mat)
	{
		m[0] = mat.m[0];	m[1] = mat.m[1];	m[2] = mat.m[2];
		m[4] = mat.m[3];	m[5] = mat.m[4];	m[6] = mat.m[5];
		m[8] = mat.m[6];	m[9] = mat.m[7];	m[10] = mat.m[8];
		m[3] = m[7] = m[11] = m[12] = m[13] = m[14] = 0.0f;
		m[15] = 1.0f;
	}

	Matrix::Matrix(const float src[16])
	{
		std::memcpy(m, src, sizeof(float) * 16);
	}

	void Matrix::operator*=(const Matrix &s)
	{
		Matrix::fastMat4x4Mult(this->m, this->m, s.m);
	}

	void Matrix::operator*=(const Matrix3 &s)
	{
		__m128 mat1, mat2, mat3, row1, row2, row3;

		mat1 = _mm_loadu_ps(m + 0);
		mat2 = _mm_loadu_ps(m + 4);
		mat3 = _mm_loadu_ps(m + 8);

		row1 = _mm_mul_ps(_mm_load_ps1(s.m + 0), mat1);
		row2 = _mm_mul_ps(_mm_load_ps1(s.m + 1), mat2);
		row3 = _mm_mul_ps(_mm_load_ps1(s.m + 2), mat3);
		_mm_storeu_ps(m + 0, _mm_add_ps(_mm_add_ps(row1, row2), row3));

		row1 = _mm_mul_ps(_mm_load_ps1(s.m + 3), mat1);
		row2 = _mm_mul_ps(_mm_load_ps1(s.m + 4), mat2);
		row3 = _mm_mul_ps(_mm_load_ps1(s.m + 5), mat3);
		_mm_storeu_ps(m + 4, _mm_add_ps(_mm_add_ps(row1, row2), row3));

		row1 = _mm_mul_ps(_mm_load_ps1(s.m + 6), mat1);
		row2 = _mm_mul_ps(_mm_load_ps1(s.m + 7), mat2);
		row3 = _mm_mul_ps(_mm_load_ps1(s.m + 8), mat3);
		_mm_storeu_ps(m + 8, _mm_add_ps(_mm_add_ps(row1, row2), row3));
	}

	void Matrix::operator*=(const float src[16])
	{
		Matrix::fastMat4x4Mult(m, m, src);
	}

	void Matrix::operator+=(const Matrix &s)
	{
		_mm_storeu_ps(m, _mm_add_ps(_mm_loadu_ps(m), _mm_loadu_ps(s.m)));
		_mm_storeu_ps(m + 4, _mm_add_ps(_mm_loadu_ps(m + 4), _mm_loadu_ps(s.m + 4)));
		_mm_storeu_ps(m + 8, _mm_add_ps(_mm_loadu_ps(m + 8), _mm_loadu_ps(s.m + 8)));
		_mm_storeu_ps(m + 12, _mm_add_ps(_mm_loadu_ps(m + 12), _mm_loadu_ps(s.m + 12)));
	}

	void Matrix::operator+=(const float src[16])
	{
		_mm_storeu_ps(m, _mm_add_ps(_mm_loadu_ps(m), _mm_loadu_ps(src)));
		_mm_storeu_ps(m + 4, _mm_add_ps(_mm_loadu_ps(m + 4), _mm_loadu_ps(src + 4)));
		_mm_storeu_ps(m + 8, _mm_add_ps(_mm_loadu_ps(m + 8), _mm_loadu_ps(src + 8)));
		_mm_storeu_ps(m + 12, _mm_add_ps(_mm_loadu_ps(m + 12), _mm_loadu_ps(src + 12)));
	}

	void Matrix::operator-=(const Matrix &s)
	{
		_mm_storeu_ps(m, _mm_sub_ps(_mm_loadu_ps(m), _mm_loadu_ps(s.m)));
		_mm_storeu_ps(m + 4, _mm_sub_ps(_mm_loadu_ps(m + 4), _mm_loadu_ps(s.m + 4)));
		_mm_storeu_ps(m + 8, _mm_sub_ps(_mm_loadu_ps(m + 8), _mm_loadu_ps(s.m + 8)));
		_mm_storeu_ps(m + 12, _mm_sub_ps(_mm_loadu_ps(m + 12), _mm_loadu_ps(s.m + 12)));
	}

	void Matrix::operator-=(const float src[16])
	{
		_mm_storeu_ps(m, _mm_sub_ps(_mm_loadu_ps(m), _mm_loadu_ps(src)));
		_mm_storeu_ps(m + 4, _mm_sub_ps(_mm_loadu_ps(m + 4), _mm_loadu_ps(src + 4)));
		_mm_storeu_ps(m + 8, _mm_sub_ps(_mm_loadu_ps(m + 8), _mm_loadu_ps(src + 8)));
		_mm_storeu_ps(m + 12, _mm_sub_ps(_mm_loadu_ps(m + 12), _mm_loadu_ps(src + 12)));
	}

	Matrix Matrix::operator*(const Matrix &s) const
	{
		Matrix res;
		Matrix::fastMat4x4Mult(res.m, m, s.m);
		return res;
	}

	Matrix Matrix::operator+(const Matrix &s) const
	{
		Matrix res;
		_mm_storeu_ps(res.m, _mm_add_ps(_mm_loadu_ps(m), _mm_loadu_ps(s.m)));
		_mm_storeu_ps(res.m + 4, _mm_add_ps(_mm_loadu_ps(m + 4), _mm_loadu_ps(s.m + 4)));
		_mm_storeu_ps(res.m + 8, _mm_add_ps(_mm_loadu_ps(m + 8), _mm_loadu_ps(s.m + 8)));
		_mm_storeu_ps(res.m + 12, _mm_add_ps(_mm_loadu_ps(m + 12), _mm_loadu_ps(s.m + 12)));
		return res;
	}

	Matrix Matrix::operator-(const Matrix &s) const
	{
		Matrix res;
		_mm_storeu_ps(res.m, _mm_sub_ps(_mm_loadu_ps(m), _mm_loadu_ps(s.m)));
		_mm_storeu_ps(res.m + 4, _mm_sub_ps(_mm_loadu_ps(m + 4), _mm_loadu_ps(s.m + 4)));
		_mm_storeu_ps(res.m + 8, _mm_sub_ps(_mm_loadu_ps(m + 8), _mm_loadu_ps(s.m + 8)));
		_mm_storeu_ps(res.m + 12, _mm_sub_ps(_mm_loadu_ps(m + 12), _mm_loadu_ps(s.m + 12)));
		return res;
	}

	void Matrix::transform(float vec[3]) const
	{
		float vecX = vec[0];
		float vecY = vec[1];
		float vecZ = vec[2];

		vec[0] = vecX*m[0] + vecY*m[4] + vecZ*m[8] + m[12];
		vec[1] = vecX*m[1] + vecY*m[5] + vecZ*m[9] + m[13];
		vec[2] = vecX*m[2] + vecY*m[6] + vecZ*m[10] + m[14];
	}

	void Matrix::transform(Vector3f& vec) const
	{
		__m128 row1, row2, row3;

		row1 = _mm_mul_ps(_mm_load_ps1(&vec[0]), _mm_loadu_ps(m));
		row2 = _mm_mul_ps(_mm_load_ps1(&vec[1]), _mm_loadu_ps(m + 4));
		row3 = _mm_mul_ps(_mm_load_ps1(&vec[2]), _mm_loadu_ps(m + 8));
		_mm_storeu_ps(vec.data(), _mm_add_ps(_mm_add_ps(row1, row2), _mm_add_ps(row3, _mm_loadu_ps(m + 12))));
	}

	void Matrix::transform(const Vector3f& vec, Vector3f& result) const
	{
		__m128 row1, row2, row3;

		row1 = _mm_mul_ps(_mm_load_ps1(&vec[0]), _mm_loadu_ps(m));
		row2 = _mm_mul_ps(_mm_load_ps1(&vec[1]), _mm_loadu_ps(m + 4));
		row3 = _mm_mul_ps(_mm_load_ps1(&vec[2]), _mm_loadu_ps(m + 8));
		_mm_storeu_ps(result.data(), _mm_add_ps(_mm_add_ps(row1, row2), _mm_add_ps(row3, _mm_loadu_ps(m + 12))));
	}

	void Matrix::transform(Vector3f* const vec, size_t numVec) const
	{
		Matrix::asmMat4x4Vec3((float*)vec, (float*)vec, 1.0f, sizeof(Vector3f), m, numVec);
	}

	void Matrix::transform(Vector4f& vec) const
	{
		__m128 row1, row2, row3, row4;

		row1 = _mm_mul_ps(_mm_load_ps1(&vec[0]), _mm_loadu_ps(m));
		row2 = _mm_mul_ps(_mm_load_ps1(&vec[1]), _mm_loadu_ps(m + 4));
		row3 = _mm_mul_ps(_mm_load_ps1(&vec[2]), _mm_loadu_ps(m + 8));
		row4 = _mm_mul_ps(_mm_load_ps1(&vec[3]), _mm_loadu_ps(m + 12));
		_mm_storeu_ps(vec.data(), _mm_add_ps(_mm_add_ps(row1, row2), _mm_add_ps(row3, row4)));
	}

	void Matrix::transform(const Vector4f& vec, Vector4f& result) const
	{
		__m128 row1, row2, row3, row4;

		row1 = _mm_mul_ps(_mm_load_ps1(&vec[0]), _mm_loadu_ps(m));
		row2 = _mm_mul_ps(_mm_load_ps1(&vec[1]), _mm_loadu_ps(m + 4));
		row3 = _mm_mul_ps(_mm_load_ps1(&vec[2]), _mm_loadu_ps(m + 8));
		row4 = _mm_mul_ps(_mm_load_ps1(&vec[3]), _mm_loadu_ps(m + 12));
		_mm_storeu_ps(result.data(), _mm_add_ps(_mm_add_ps(row1, row2), _mm_add_ps(row3, row4)));
	}

	void Matrix::transform(Vector4f* const vec, size_t numVec) const
	{
		Matrix::asmMat4x4Vec4((float*)vec, (float*)vec, sizeof(Vector4f), m, numVec);
	}

	void Matrix::rotateScale(float vec[3]) const
	{
		float vecX = vec[0];
		float vecY = vec[1];
		float vecZ = vec[2];

		vec[0] = vecX*m[0] + vecY*m[4] + vecZ*m[8];
		vec[1] = vecX*m[1] + vecY*m[5] + vecZ*m[9];
		vec[2] = vecX*m[2] + vecY*m[6] + vecZ*m[10];
	}

	void Matrix::rotateScale(Vector3f& vec) const
	{
		__m128 row1, row2, row3;

		row1 = _mm_mul_ps(_mm_load_ps1(&vec[0]), _mm_loadu_ps(m + 0));
		row2 = _mm_mul_ps(_mm_load_ps1(&vec[1]), _mm_loadu_ps(m + 4));
		row3 = _mm_mul_ps(_mm_load_ps1(&vec[2]), _mm_loadu_ps(m + 8));
		_mm_storeu_ps(vec.data(), _mm_add_ps(_mm_add_ps(row1, row2), row3));
	}

	void Matrix::rotateScale(const Vector3f& vec, Vector3f& result) const
	{
		__m128 row1, row2, row3;

		row1 = _mm_mul_ps(_mm_load_ps1(&vec[0]), _mm_loadu_ps(m + 0));
		row2 = _mm_mul_ps(_mm_load_ps1(&vec[1]), _mm_loadu_ps(m + 4));
		row3 = _mm_mul_ps(_mm_load_ps1(&vec[2]), _mm_loadu_ps(m + 8));
		_mm_storeu_ps(result.data(), _mm_add_ps(_mm_add_ps(row1, row2), row3));
	}

	void Matrix::rotateScale(Vector3f* const vec, size_t numVec) const
	{
		Matrix::asmMat4x4Vec3((float*)vec, (float*)vec, 0.0f, sizeof(Vector3f), m, numVec);
	}

	void Matrix::transform(BBox &bbox) const
	{
		Vector3f pts[8];

		bbox.corners(pts);
		transform(pts, 8);

		bbox.reset();
		bbox.merge(pts, 8);
	}

	void Matrix::transform(const BBox &bbox, BBox &bboxDest) const
	{
		Vector3f pts[8];

		bbox.corners(pts);
		transform(pts, 8);

		bboxDest.reset();
		bboxDest.merge(pts, 8);
	}

	Vector4f Matrix::getColumn(size_t columnIndex) const
	{
		columnIndex = columnIndex % 4;
		return Vector4f(m[columnIndex], m[columnIndex + 4], m[columnIndex + 8], m[columnIndex + 12]);
	}

	Vector4f Matrix::getRow(size_t rowIndex) const
	{
		return Vector4f(m + ((rowIndex % 4) * 4));
	}

	void Matrix::getRotation(Vector3f &vec, float &angle) const
	{
		float s, tr, quat[4];
		int op;

		tr = m[0] + m[5] + m[10];

		if (tr > 0.0f)
		{
			s = sqrt(tr + 1.0f);

			quat[3] = s*0.5f;

			s = 0.5f / s;

			quat[0] = (m[6] - m[9])*s;
			quat[1] = (m[8] - m[2])*s;
			quat[2] = (m[1] - m[4])*s;
		}
		else
		{
			op = 0;
			if (m[5] > m[0])
				op = 1;
			if (m[10] > m[op * 5])
				op = 2;

			switch (op){
			case 0:
				s = sqrt((m[0] - (m[5] + m[10])) + 1.0f);

				quat[0] = s*0.5f;
				if (s != 0.0)
					s = 0.5f / s;

				quat[1] = (m[1] + m[4])*s;
				quat[2] = (m[2] + m[8])*s;
				quat[3] = (m[6] - m[9])*s;

				break;

			case 1:
				s = sqrt((m[5] - (m[10] + m[0])) + 1.0f);

				quat[1] = s*0.5f;
				if (s != 0.0f)
					s = 0.5f / s;

				quat[3] = (m[8] - m[2])*s;
				quat[2] = (m[6] + m[9])*s;
				quat[0] = (m[4] + m[1])*s;

				break;

			case 2:
				s = sqrt((m[10] - (m[0] + m[5])) + 1.0f);

				quat[2] = s*0.5f;
				if (s != 0.0f)
					s = 0.5f / s;

				quat[3] = (m[1] - m[4])*s;
				quat[0] = (m[8] + m[2])*s;
				quat[1] = (m[9] + m[6])*s;
				break;
			}
		}

		tr = quat[0] * quat[0] + quat[1] * quat[1] + quat[2] * quat[2];
		if (tr == 0.0f)
		{
			vec[0] = 0.0f;
			vec[1] = 0.0f;
			vec[2] = 1.0f;
			angle = 0.0f;
			return;
		}

		tr = 1.0f / tr;
		vec[0] = quat[0] * tr;
		vec[1] = quat[1] * tr;
		vec[2] = quat[2] * tr;
		vec.normalize();
		angle = ((float)acos(quat[3]))*114.5915590261646417f; // 180/pi=57.295779513082320876f * 2.0f
	}

	void Matrix::getEulerAngles(float &rfYAngle, float &rfPAngle, float &rfRAngle) const
	{
		if (-1.0f < -m[8])
		{
			if (-m[8] < 1.0)
				rfPAngle = asin(-m[8]);
			else
				rfPAngle = -(2.0f*atanf(1.0f));
		}
		else
		{
			rfPAngle = 2.0f*atanf(1.0f);
		}

		if (rfPAngle < (2.0f*atanf(1.0f)))
		{
			if (rfPAngle > -(2.0f*atanf(1.0f)))
			{
				rfYAngle = atan2(m[4], m[0]);
				rfRAngle = atan2(m[9], m[10]);
			}
			else
			{
				float fRmY = atan2(-m[1], m[2]);
				rfRAngle = 0.0f;
				rfYAngle = rfRAngle - fRmY;
			}
		}

		else
		{
			float fRpY = atan2(-m[0], m[2]);
			rfRAngle = 0.0f;
			rfYAngle = fRpY - rfRAngle;
		}
	}

	Matrix3 Matrix::getMat3x3() const
	{
		Matrix3 mat;
		getMat3x3(mat);
		return mat;
	}

	void Matrix::getMat3x3(Matrix3 &mat3) const
	{
		mat3.m[0] = m[0];
		mat3.m[1] = m[1];
		mat3.m[2] = m[2];
		mat3.m[3] = m[4];
		mat3.m[4] = m[5];
		mat3.m[5] = m[6];
		mat3.m[6] = m[8];
		mat3.m[7] = m[9];
		mat3.m[8] = m[10];
	}

	void Matrix::getMat3x3(float dest[9]) const
	{
		dest[0] = m[0];
		dest[1] = m[1];
		dest[2] = m[2];
		dest[3] = m[4];
		dest[4] = m[5];
		dest[5] = m[6];
		dest[6] = m[8];
		dest[7] = m[9];
		dest[8] = m[10];
	}

	void Matrix::getMat2x2(float dest[4]) const
	{
		dest[0] = m[0];
		dest[1] = m[1];
		dest[2] = m[4];
		dest[3] = m[5];
	}

	void Matrix::write(float dest[16]) const
	{
		std::memcpy(dest, m, sizeof(float) * 16);
	}

	Matrix Matrix::transpose() const
	{
		Matrix result;
		__m128 row1, row2, row3, row4;

		row1 = _mm_loadu_ps(m + 0);
		row2 = _mm_loadu_ps(m + 4);
		row3 = _mm_loadu_ps(m + 8);
		row4 = _mm_loadu_ps(m + 12);
		_MM_TRANSPOSE4_PS(row1, row2, row3, row4);
		_mm_storeu_ps(result.m + 0, row1);
		_mm_storeu_ps(result.m + 4, row2);
		_mm_storeu_ps(result.m + 8, row3);
		_mm_storeu_ps(result.m + 12, row4);

		return result;
	}

	void Matrix::transpose()
	{
		__m128 row1, row2, row3, row4;

		row1 = _mm_loadu_ps(m + 0);
		row2 = _mm_loadu_ps(m + 4);
		row3 = _mm_loadu_ps(m + 8);
		row4 = _mm_loadu_ps(m + 12);
		_MM_TRANSPOSE4_PS(row1, row2, row3, row4);
		_mm_storeu_ps(m + 0, row1);
		_mm_storeu_ps(m + 4, row2);
		_mm_storeu_ps(m + 8, row3);
		_mm_storeu_ps(m + 12, row4);
	}

	Matrix Matrix::inverse() const
	{
		Matrix result;
		float tmp[12], det;

		//calculate pairs for first 8 elements (cofactors)
		tmp[0] = m[10] * m[15];	tmp[1] = m[11] * m[14];
		tmp[2] = m[9] * m[15];	tmp[3] = m[11] * m[13];
		tmp[4] = m[9] * m[14];	tmp[5] = m[10] * m[13];
		tmp[6] = m[8] * m[15];	tmp[7] = m[11] * m[12];
		tmp[8] = m[8] * m[14];	tmp[9] = m[10] * m[12];
		tmp[10] = m[8] * m[13];	tmp[11] = m[9] * m[12];

		//calculate first 8 elements (cofactors)
		result.m[0] = tmp[0] * m[5] + tmp[3] * m[6] + tmp[4] * m[7] - tmp[1] * m[5] - tmp[2] * m[6] - tmp[5] * m[7];
		result.m[4] = tmp[1] * m[4] + tmp[6] * m[6] + tmp[9] * m[7] - tmp[0] * m[4] - tmp[7] * m[6] - tmp[8] * m[7];
		result.m[8] = tmp[2] * m[4] + tmp[7] * m[5] + tmp[10] * m[7] - tmp[3] * m[4] - tmp[6] * m[5] - tmp[11] * m[7];
		result.m[12] = tmp[5] * m[4] + tmp[8] * m[5] + tmp[11] * m[6] - tmp[4] * m[4] - tmp[9] * m[5] - tmp[10] * m[6];
		result.m[1] = tmp[1] * m[1] + tmp[2] * m[2] + tmp[5] * m[3] - tmp[0] * m[1] - tmp[3] * m[2] - tmp[4] * m[3];
		result.m[5] = tmp[0] * m[0] + tmp[7] * m[2] + tmp[8] * m[3] - tmp[1] * m[0] - tmp[6] * m[2] - tmp[9] * m[3];
		result.m[9] = tmp[3] * m[0] + tmp[6] * m[1] + tmp[11] * m[3] - tmp[2] * m[0] - tmp[7] * m[1] - tmp[10] * m[3];
		result.m[13] = tmp[4] * m[0] + tmp[9] * m[1] + tmp[10] * m[2] - tmp[5] * m[0] - tmp[8] * m[1] - tmp[11] * m[2];

		//calculate pairs for second 8 elements (cofactors)
		tmp[0] = m[2] * m[7];		tmp[1] = m[3] * m[6];
		tmp[2] = m[1] * m[7];		tmp[3] = m[3] * m[5];
		tmp[4] = m[1] * m[6];		tmp[5] = m[2] * m[5];
		tmp[6] = m[0] * m[7];		tmp[7] = m[3] * m[4];
		tmp[8] = m[0] * m[6];		tmp[9] = m[2] * m[4];
		tmp[10] = m[0] * m[5];		tmp[11] = m[1] * m[4];

		//calculate second 8 elements (cofactors)
		result.m[2] = tmp[0] * m[13] + tmp[3] * m[14] + tmp[4] * m[15] - tmp[1] * m[13] - tmp[2] * m[14] - tmp[5] * m[15];
		result.m[6] = tmp[1] * m[12] + tmp[6] * m[14] + tmp[9] * m[15] - tmp[0] * m[12] - tmp[7] * m[14] - tmp[8] * m[15];
		result.m[10] = tmp[2] * m[12] + tmp[7] * m[13] + tmp[10] * m[15] - tmp[3] * m[12] - tmp[6] * m[13] - tmp[11] * m[15];
		result.m[14] = tmp[5] * m[12] + tmp[8] * m[13] + tmp[11] * m[14] - tmp[4] * m[12] - tmp[9] * m[13] - tmp[10] * m[14];
		result.m[3] = tmp[2] * m[10] + tmp[5] * m[11] + tmp[1] * m[9] - tmp[4] * m[11] - tmp[0] * m[9] - tmp[3] * m[10];
		result.m[7] = tmp[8] * m[11] + tmp[0] * m[8] + tmp[7] * m[10] - tmp[6] * m[10] - tmp[9] * m[11] - tmp[1] * m[8];
		result.m[11] = tmp[6] * m[9] + tmp[11] * m[11] + tmp[3] * m[8] - tmp[10] * m[11] - tmp[2] * m[8] - tmp[7] * m[9];
		result.m[15] = tmp[10] * m[10] + tmp[4] * m[8] + tmp[9] * m[9] - tmp[8] * m[9] - tmp[11] * m[10] - tmp[5] * m[8];

		// calculate determinant
		det = m[0] * result.m[0] + m[1] * result.m[4] + m[2] * result.m[8] + m[3] * result.m[12];
		if (Math::isZero(det))
			return result;

		//multiplicar tudo pelo determinante
		det = 1.0f / det;
		result.m[0] *= det;		result.m[1] *= det;		result.m[2] *= det;		result.m[3] *= det;
		result.m[4] *= det;		result.m[5] *= det;		result.m[6] *= det;		result.m[7] *= det;
		result.m[8] *= det;		result.m[9] *= det;		result.m[10] *= det;	result.m[11] *= det;
		result.m[12] *= det;	result.m[13] *= det;	result.m[14] *= det;	result.m[15] *= det;

		return result;
	}

	void Matrix::inverse()
	{
		float tmp[12], result[16], det;

		//calculate pairs for first 8 elements (cofactors)
		tmp[0] = m[10] * m[15];	tmp[1] = m[11] * m[14];
		tmp[2] = m[9] * m[15];	tmp[3] = m[11] * m[13];
		tmp[4] = m[9] * m[14];	tmp[5] = m[10] * m[13];
		tmp[6] = m[8] * m[15];	tmp[7] = m[11] * m[12];
		tmp[8] = m[8] * m[14];	tmp[9] = m[10] * m[12];
		tmp[10] = m[8] * m[13];	tmp[11] = m[9] * m[12];

		//calculate first 8 elements (cofactors)
		result[0] = tmp[0] * m[5] + tmp[3] * m[6] + tmp[4] * m[7] - tmp[1] * m[5] - tmp[2] * m[6] - tmp[5] * m[7];
		result[4] = tmp[1] * m[4] + tmp[6] * m[6] + tmp[9] * m[7] - tmp[0] * m[4] - tmp[7] * m[6] - tmp[8] * m[7];
		result[8] = tmp[2] * m[4] + tmp[7] * m[5] + tmp[10] * m[7] - tmp[3] * m[4] - tmp[6] * m[5] - tmp[11] * m[7];
		result[12] = tmp[5] * m[4] + tmp[8] * m[5] + tmp[11] * m[6] - tmp[4] * m[4] - tmp[9] * m[5] - tmp[10] * m[6];
		result[1] = tmp[1] * m[1] + tmp[2] * m[2] + tmp[5] * m[3] - tmp[0] * m[1] - tmp[3] * m[2] - tmp[4] * m[3];
		result[5] = tmp[0] * m[0] + tmp[7] * m[2] + tmp[8] * m[3] - tmp[1] * m[0] - tmp[6] * m[2] - tmp[9] * m[3];
		result[9] = tmp[3] * m[0] + tmp[6] * m[1] + tmp[11] * m[3] - tmp[2] * m[0] - tmp[7] * m[1] - tmp[10] * m[3];
		result[13] = tmp[4] * m[0] + tmp[9] * m[1] + tmp[10] * m[2] - tmp[5] * m[0] - tmp[8] * m[1] - tmp[11] * m[2];

		//calculate pairs for second 8 elements (cofactors)
		tmp[0] = m[2] * m[7];		tmp[1] = m[3] * m[6];
		tmp[2] = m[1] * m[7];		tmp[3] = m[3] * m[5];
		tmp[4] = m[1] * m[6];		tmp[5] = m[2] * m[5];
		tmp[6] = m[0] * m[7];		tmp[7] = m[3] * m[4];
		tmp[8] = m[0] * m[6];		tmp[9] = m[2] * m[4];
		tmp[10] = m[0] * m[5];		tmp[11] = m[1] * m[4];

		//calculate second 8 elements (cofactors)
		result[2] = tmp[0] * m[13] + tmp[3] * m[14] + tmp[4] * m[15] - tmp[1] * m[13] - tmp[2] * m[14] - tmp[5] * m[15];
		result[6] = tmp[1] * m[12] + tmp[6] * m[14] + tmp[9] * m[15] - tmp[0] * m[12] - tmp[7] * m[14] - tmp[8] * m[15];
		result[10] = tmp[2] * m[12] + tmp[7] * m[13] + tmp[10] * m[15] - tmp[3] * m[12] - tmp[6] * m[13] - tmp[11] * m[15];
		result[14] = tmp[5] * m[12] + tmp[8] * m[13] + tmp[11] * m[14] - tmp[4] * m[12] - tmp[9] * m[13] - tmp[10] * m[14];
		result[3] = tmp[2] * m[10] + tmp[5] * m[11] + tmp[1] * m[9] - tmp[4] * m[11] - tmp[0] * m[9] - tmp[3] * m[10];
		result[7] = tmp[8] * m[11] + tmp[0] * m[8] + tmp[7] * m[10] - tmp[6] * m[10] - tmp[9] * m[11] - tmp[1] * m[8];
		result[11] = tmp[6] * m[9] + tmp[11] * m[11] + tmp[3] * m[8] - tmp[10] * m[11] - tmp[2] * m[8] - tmp[7] * m[9];
		result[15] = tmp[10] * m[10] + tmp[4] * m[8] + tmp[9] * m[9] - tmp[8] * m[9] - tmp[11] * m[10] - tmp[5] * m[8];

		// calculate determinant
		det = m[0] * result[0] + m[1] * result[4] + m[2] * result[8] + m[3] * result[12];
		if (Math::isZero(det))
			return;

		//multiplicar tudo pelo determinante
		det = 1.0f / det;
		result[0] *= det;		result[1] *= det;		result[2] *= det;		result[3] *= det;
		result[4] *= det;		result[5] *= det;		result[6] *= det;		result[7] *= det;
		result[8] *= det;		result[9] *= det;		result[10] *= det;	result[11] *= det;
		result[12] *= det;	result[13] *= det;	result[14] *= det;	result[15] *= det;

		//basta copiar para mim próprio e pronto
		std::memcpy(m, result, sizeof(float) * 16);
	}

	Matrix Matrix::inverseTranspose() const
	{
		Matrix result;
		float tmp[12], det;

		//calculate pairs for first 8 elements (cofactors)
		tmp[0] = m[10] * m[15];	tmp[1] = m[11] * m[14];
		tmp[2] = m[9] * m[15];	tmp[3] = m[11] * m[13];
		tmp[4] = m[9] * m[14];	tmp[5] = m[10] * m[13];
		tmp[6] = m[8] * m[15];	tmp[7] = m[11] * m[12];
		tmp[8] = m[8] * m[14];	tmp[9] = m[10] * m[12];
		tmp[10] = m[8] * m[13];	tmp[11] = m[9] * m[12];

		//calculate first 8 elements (cofactors)
		result.m[0] = tmp[0] * m[5] + tmp[3] * m[6] + tmp[4] * m[7] - tmp[1] * m[5] - tmp[2] * m[6] - tmp[5] * m[7];
		result.m[1] = tmp[1] * m[4] + tmp[6] * m[6] + tmp[9] * m[7] - tmp[0] * m[4] - tmp[7] * m[6] - tmp[8] * m[7];
		result.m[2] = tmp[2] * m[4] + tmp[7] * m[5] + tmp[10] * m[7] - tmp[3] * m[4] - tmp[6] * m[5] - tmp[11] * m[7];
		result.m[3] = tmp[5] * m[4] + tmp[8] * m[5] + tmp[11] * m[6] - tmp[4] * m[4] - tmp[9] * m[5] - tmp[10] * m[6];
		result.m[4] = tmp[1] * m[1] + tmp[2] * m[2] + tmp[5] * m[3] - tmp[0] * m[1] - tmp[3] * m[2] - tmp[4] * m[3];
		result.m[5] = tmp[0] * m[0] + tmp[7] * m[2] + tmp[8] * m[3] - tmp[1] * m[0] - tmp[6] * m[2] - tmp[9] * m[3];
		result.m[6] = tmp[3] * m[0] + tmp[6] * m[1] + tmp[11] * m[3] - tmp[2] * m[0] - tmp[7] * m[1] - tmp[10] * m[3];
		result.m[7] = tmp[4] * m[0] + tmp[9] * m[1] + tmp[10] * m[2] - tmp[5] * m[0] - tmp[8] * m[1] - tmp[11] * m[2];

		//calculate pairs for second 8 elements (cofactors)
		tmp[0] = m[2] * m[7];		tmp[1] = m[3] * m[6];
		tmp[2] = m[1] * m[7];		tmp[3] = m[3] * m[5];
		tmp[4] = m[1] * m[6];		tmp[5] = m[2] * m[5];
		tmp[6] = m[0] * m[7];		tmp[7] = m[3] * m[4];
		tmp[8] = m[0] * m[6];		tmp[9] = m[2] * m[4];
		tmp[10] = m[0] * m[5];		tmp[11] = m[1] * m[4];

		//calculate second 8 elements (cofactors)
		result.m[8] = tmp[0] * m[13] + tmp[3] * m[14] + tmp[4] * m[15] - tmp[1] * m[13] - tmp[2] * m[14] - tmp[5] * m[15];
		result.m[9] = tmp[1] * m[12] + tmp[6] * m[14] + tmp[9] * m[15] - tmp[0] * m[12] - tmp[7] * m[14] - tmp[8] * m[15];
		result.m[10] = tmp[2] * m[12] + tmp[7] * m[13] + tmp[10] * m[15] - tmp[3] * m[12] - tmp[6] * m[13] - tmp[11] * m[15];
		result.m[11] = tmp[5] * m[12] + tmp[8] * m[13] + tmp[11] * m[14] - tmp[4] * m[12] - tmp[9] * m[13] - tmp[10] * m[14];
		result.m[12] = tmp[2] * m[10] + tmp[5] * m[11] + tmp[1] * m[9] - tmp[4] * m[11] - tmp[0] * m[9] - tmp[3] * m[10];
		result.m[13] = tmp[8] * m[11] + tmp[0] * m[8] + tmp[7] * m[10] - tmp[6] * m[10] - tmp[9] * m[11] - tmp[1] * m[8];
		result.m[14] = tmp[6] * m[9] + tmp[11] * m[11] + tmp[3] * m[8] - tmp[10] * m[11] - tmp[2] * m[8] - tmp[7] * m[9];
		result.m[15] = tmp[10] * m[10] + tmp[4] * m[8] + tmp[9] * m[9] - tmp[8] * m[9] - tmp[11] * m[10] - tmp[5] * m[8];

		// calculate determinant
		det = m[0] * result.m[0] + m[1] * result.m[1] + m[2] * result.m[2] + m[3] * result.m[3];
		if (Math::isZero(det))
			return result;

		//multiplicar tudo pelo determinante
		det = 1.0f / det;
		result.m[0] *= det;		result.m[1] *= det;		result.m[2] *= det;		result.m[3] *= det;
		result.m[4] *= det;		result.m[5] *= det;		result.m[6] *= det;		result.m[7] *= det;
		result.m[8] *= det;		result.m[9] *= det;		result.m[10] *= det;	result.m[11] *= det;
		result.m[12] *= det;	result.m[13] *= det;	result.m[14] *= det;	result.m[15] *= det;

		return result;
	}

	void Matrix::inverseTranspose()
	{
		float tmp[12], result[16], det;

		//calculate pairs for first 8 elements (cofactors)
		tmp[0] = m[10] * m[15];	tmp[1] = m[11] * m[14];
		tmp[2] = m[9] * m[15];	tmp[3] = m[11] * m[13];
		tmp[4] = m[9] * m[14];	tmp[5] = m[10] * m[13];
		tmp[6] = m[8] * m[15];	tmp[7] = m[11] * m[12];
		tmp[8] = m[8] * m[14];	tmp[9] = m[10] * m[12];
		tmp[10] = m[8] * m[13];	tmp[11] = m[9] * m[12];

		//calculate first 8 elements (cofactors)
		result[0] = tmp[0] * m[5] + tmp[3] * m[6] + tmp[4] * m[7] - tmp[1] * m[5] - tmp[2] * m[6] - tmp[5] * m[7];
		result[1] = tmp[1] * m[4] + tmp[6] * m[6] + tmp[9] * m[7] - tmp[0] * m[4] - tmp[7] * m[6] - tmp[8] * m[7];
		result[2] = tmp[2] * m[4] + tmp[7] * m[5] + tmp[10] * m[7] - tmp[3] * m[4] - tmp[6] * m[5] - tmp[11] * m[7];
		result[3] = tmp[5] * m[4] + tmp[8] * m[5] + tmp[11] * m[6] - tmp[4] * m[4] - tmp[9] * m[5] - tmp[10] * m[6];
		result[4] = tmp[1] * m[1] + tmp[2] * m[2] + tmp[5] * m[3] - tmp[0] * m[1] - tmp[3] * m[2] - tmp[4] * m[3];
		result[5] = tmp[0] * m[0] + tmp[7] * m[2] + tmp[8] * m[3] - tmp[1] * m[0] - tmp[6] * m[2] - tmp[9] * m[3];
		result[6] = tmp[3] * m[0] + tmp[6] * m[1] + tmp[11] * m[3] - tmp[2] * m[0] - tmp[7] * m[1] - tmp[10] * m[3];
		result[7] = tmp[4] * m[0] + tmp[9] * m[1] + tmp[10] * m[2] - tmp[5] * m[0] - tmp[8] * m[1] - tmp[11] * m[2];

		//calculate pairs for second 8 elements (cofactors)
		tmp[0] = m[2] * m[7];		tmp[1] = m[3] * m[6];
		tmp[2] = m[1] * m[7];		tmp[3] = m[3] * m[5];
		tmp[4] = m[1] * m[6];		tmp[5] = m[2] * m[5];
		tmp[6] = m[0] * m[7];		tmp[7] = m[3] * m[4];
		tmp[8] = m[0] * m[6];		tmp[9] = m[2] * m[4];
		tmp[10] = m[0] * m[5];		tmp[11] = m[1] * m[4];

		//calculate second 8 elements (cofactors)
		result[8] = tmp[0] * m[13] + tmp[3] * m[14] + tmp[4] * m[15] - tmp[1] * m[13] - tmp[2] * m[14] - tmp[5] * m[15];
		result[9] = tmp[1] * m[12] + tmp[6] * m[14] + tmp[9] * m[15] - tmp[0] * m[12] - tmp[7] * m[14] - tmp[8] * m[15];
		result[10] = tmp[2] * m[12] + tmp[7] * m[13] + tmp[10] * m[15] - tmp[3] * m[12] - tmp[6] * m[13] - tmp[11] * m[15];
		result[11] = tmp[5] * m[12] + tmp[8] * m[13] + tmp[11] * m[14] - tmp[4] * m[12] - tmp[9] * m[13] - tmp[10] * m[14];
		result[12] = tmp[2] * m[10] + tmp[5] * m[11] + tmp[1] * m[9] - tmp[4] * m[11] - tmp[0] * m[9] - tmp[3] * m[10];
		result[13] = tmp[8] * m[11] + tmp[0] * m[8] + tmp[7] * m[10] - tmp[6] * m[10] - tmp[9] * m[11] - tmp[1] * m[8];
		result[14] = tmp[6] * m[9] + tmp[11] * m[11] + tmp[3] * m[8] - tmp[10] * m[11] - tmp[2] * m[8] - tmp[7] * m[9];
		result[15] = tmp[10] * m[10] + tmp[4] * m[8] + tmp[9] * m[9] - tmp[8] * m[9] - tmp[11] * m[10] - tmp[5] * m[8];

		// calculate determinant
		det = m[0] * result[0] + m[1] * result[1] + m[2] * result[2] + m[3] * result[3];
		if (Math::isZero(det))
			return;

		//multiplicar tudo pelo determinante
		det = 1.0f / det;
		result[0] *= det;		result[1] *= det;		result[2] *= det;		result[3] *= det;
		result[4] *= det;		result[5] *= det;		result[6] *= det;		result[7] *= det;
		result[8] *= det;		result[9] *= det;		result[10] *= det;	result[11] *= det;
		result[12] *= det;	result[13] *= det;	result[14] *= det;	result[15] *= det;

		//basta copiar para mim próprio e pronto
		std::memcpy(m, result, sizeof(float) * 16);
	}

	Matrix Matrix::inverseHomogenous() const
	{
		Matrix result;
		float aux1, aux2;

		std::memcpy(result.m, m, sizeof(float) * 16);

		aux1 = result.m[1];	result.m[1] = result.m[4];	result.m[4] = aux1;
		aux1 = result.m[2];	result.m[2] = result.m[8];	result.m[8] = aux1;
		aux1 = result.m[6];	result.m[6] = result.m[9];	result.m[9] = aux1;

		aux1 = -(result.m[0] * result.m[12] + result.m[4] * result.m[13] + result.m[8] * result.m[14]);
		aux2 = -(result.m[1] * result.m[12] + result.m[5] * result.m[13] + result.m[9] * result.m[14]);
		result.m[14] = -(result.m[2] * result.m[12] + result.m[6] * result.m[13] + result.m[10] * result.m[14]);
		result.m[13] = aux2;
		result.m[12] = aux1;

		return result;
	}

	void Matrix::inverseHomogenous()
	{
		float aux1, aux2;

		aux1 = m[1];	m[1] = m[4];	m[4] = aux1;
		aux1 = m[2];	m[2] = m[8];	m[8] = aux1;
		aux1 = m[6];	m[6] = m[9];	m[9] = aux1;

		aux1 = -(m[0] * m[12] + m[4] * m[13] + m[8] * m[14]);
		aux2 = -(m[1] * m[12] + m[5] * m[13] + m[9] * m[14]);
		m[14] = -(m[2] * m[12] + m[6] * m[13] + m[10] * m[14]);
		m[13] = aux2;
		m[12] = aux1;
	}

	Matrix& Matrix::mulTranslation(float x, float y, float z)
	{
		__m128 row1, row2, row3;

		row1 = _mm_mul_ps(_mm_load_ps1(&x), _mm_loadu_ps(m));
		row2 = _mm_mul_ps(_mm_load_ps1(&y), _mm_loadu_ps(m + 4));
		row3 = _mm_mul_ps(_mm_load_ps1(&z), _mm_loadu_ps(m + 8));
		_mm_storeu_ps(m + 12, _mm_add_ps(_mm_add_ps(row1, row2), _mm_add_ps(row3, _mm_loadu_ps(m + 12))));

		return *this;
	}

	Matrix& Matrix::mulTranslation(const float * const vec)
	{
		__m128 row1, row2, row3;

		row1 = _mm_mul_ps(_mm_load_ps1(vec + 0), _mm_loadu_ps(m));
		row2 = _mm_mul_ps(_mm_load_ps1(vec + 1), _mm_loadu_ps(m + 4));
		row3 = _mm_mul_ps(_mm_load_ps1(vec + 2), _mm_loadu_ps(m + 8));
		_mm_storeu_ps(m + 12, _mm_add_ps(_mm_add_ps(row1, row2), _mm_add_ps(row3, _mm_loadu_ps(m + 12))));

		return *this;
	}

	Matrix& Matrix::mulScale(float x, float y, float z)
	{
		_mm_storeu_ps(m + 0, _mm_mul_ps(_mm_load_ps1(&x), _mm_loadu_ps(m + 0)));
		_mm_storeu_ps(m + 4, _mm_mul_ps(_mm_load_ps1(&y), _mm_loadu_ps(m + 4)));
		_mm_storeu_ps(m + 8, _mm_mul_ps(_mm_load_ps1(&z), _mm_loadu_ps(m + 8)));

		return *this;
	}

	Matrix& Matrix::mulScale(const float * const vec)
	{
		_mm_storeu_ps(m + 0, _mm_mul_ps(_mm_load_ps1(vec + 0), _mm_loadu_ps(m + 0)));
		_mm_storeu_ps(m + 4, _mm_mul_ps(_mm_load_ps1(vec + 1), _mm_loadu_ps(m + 4)));
		_mm_storeu_ps(m + 8, _mm_mul_ps(_mm_load_ps1(vec + 2), _mm_loadu_ps(m + 8)));

		return *this;
	}

	Matrix& Matrix::mulRotationX(float angleDeg)
	{
		float c, s, p1, p2, p3, p4;

		Math::sinCos(Math::convDeg2Rad(angleDeg), s, c);

		p1 = m[4];
		p2 = m[5];
		p3 = m[6];
		p4 = m[7];

		m[4] = c * m[4] + s * m[8];
		m[5] = c * m[5] + s * m[9];
		m[6] = c * m[6] + s * m[10];
		m[7] = c * m[7] + s * m[11];

		m[8] = -s * p1 + c * m[8];
		m[9] = -s * p2 + c * m[9];
		m[10] = -s * p3 + c * m[10];
		m[11] = -s * p4 + c * m[11];

		return *this;
	}

	Matrix& Matrix::mulRotationY(float angleDeg)
	{
		float c, s, p1, p2, p3, p4;

		Math::sinCos(Math::convDeg2Rad(angleDeg), s, c);

		p1 = m[0];
		p2 = m[1];
		p3 = m[2];
		p4 = m[3];

		m[0] = c * m[0] + -s * m[8];
		m[1] = c * m[1] + -s * m[9];
		m[2] = c * m[2] + -s * m[10];
		m[3] = c * m[3] + -s * m[11];

		m[8] = s * p1 + c * m[8];
		m[9] = s * p2 + c * m[9];
		m[10] = s * p3 + c * m[10];
		m[11] = s * p4 + c * m[11];

		return *this;
	}

	Matrix& Matrix::mulRotationZ(float angleDeg)
	{
		float c, s, p1, p2, p3, p4;

		Math::sinCos(Math::convDeg2Rad(angleDeg), s, c);

		p1 = m[0];
		p2 = m[1];
		p3 = m[2];
		p4 = m[3];

		m[0] = c * m[0] + s * m[4];
		m[1] = c * m[1] + s * m[5];
		m[2] = c * m[2] + s * m[6];
		m[3] = c * m[3] + s * m[7];

		m[4] = -s * p1 + c * m[4];
		m[5] = -s * p2 + c * m[5];
		m[6] = -s * p3 + c * m[6];
		m[7] = -s * p4 + c * m[7];

		return *this;
	}

	Matrix& Matrix::mul(const Matrix &s)
	{
		Matrix::fastMat4x4Mult(this->m, this->m, s.m);
		return *this;
	}

	Matrix& Matrix::mul(const float src[16])
	{
		Matrix::fastMat4x4Mult(m, m, src);
		return *this;
	}

	Matrix& Matrix::mulReverseOrder(const Matrix &s)
	{
		Matrix::fastMat4x4Mult(this->m, s.m, this->m);
		return *this;
	}

	Matrix& Matrix::mulReverseOrder(const float src[16])
	{
		Matrix::fastMat4x4Mult(m, src, m);
		return *this;
	}

	void Matrix::set(float value)
	{
		__m128 scalar;

		scalar = _mm_load_ps1(&value);
		_mm_storeu_ps(m + 0, scalar);
		_mm_storeu_ps(m + 4, scalar);
		_mm_storeu_ps(m + 8, scalar);
		_mm_storeu_ps(m + 12, scalar);
	}

	void Matrix::set(const float src[16])
	{
		std::memcpy(m, src, sizeof(float) * 16);
	}

	void Matrix::set(const Matrix &mat)
	{
		std::memcpy(m, mat.m, sizeof(float) * 16);
	}

	void Matrix::setZero(void)
	{
		std::memset(m, 0, sizeof(float) * 16);
	}

	void Matrix::setIdentity(void)
	{
		std::memset(m, 0, sizeof(float) * 16);
		m[0] = m[5] = m[10] = m[15] = 1.0f;
	}

	void Matrix::setFrom3x3(const Matrix3 &mat3)
	{
		m[0] = mat3.m[0];
		m[1] = mat3.m[1];
		m[2] = mat3.m[2];
		m[4] = mat3.m[3];
		m[5] = mat3.m[4];
		m[6] = mat3.m[5];
		m[8] = mat3.m[6];
		m[9] = mat3.m[7];
		m[10] = mat3.m[8];

		m[3] = m[7] = m[11] = m[12] = m[13] = m[14] = 0.0f;
		m[15] = 1.0f;
	}

	void Matrix::setFrom3x3(const float src[9])
	{
		m[0] = src[0];
		m[1] = src[1];
		m[2] = src[2];
		m[4] = src[3];
		m[5] = src[4];
		m[6] = src[5];
		m[8] = src[6];
		m[9] = src[7];
		m[10] = src[8];

		m[3] = m[7] = m[11] = m[12] = m[13] = m[14] = 0.0f;
		m[15] = 1.0f;
	}

	void Matrix::setFrom2x2(const float src[4])
	{
		m[0] = src[0];
		m[1] = src[1];
		m[4] = src[2];
		m[5] = src[3];

		m[2] = m[3] = m[6] = m[7] = m[8] = m[9] = m[11] = m[12] = m[13] = m[14] = 0.0f;
		m[10] = m[15] = 1.0f;
	}

	void Matrix::setTranspose(const float src[16])
	{
		__m128 row1, row2, row3, row4;

		row1 = _mm_loadu_ps(src + 0);
		row2 = _mm_loadu_ps(src + 4);
		row3 = _mm_loadu_ps(src + 8);
		row4 = _mm_loadu_ps(src + 12);
		_MM_TRANSPOSE4_PS(row1, row2, row3, row4);
		_mm_storeu_ps(m + 0, row1);
		_mm_storeu_ps(m + 4, row2);
		_mm_storeu_ps(m + 8, row3);
		_mm_storeu_ps(m + 12, row4);
	}

	void Matrix::setTranspose(const Matrix &mat)
	{
		__m128 row1, row2, row3, row4;

		row1 = _mm_loadu_ps(mat.m + 0);
		row2 = _mm_loadu_ps(mat.m + 4);
		row3 = _mm_loadu_ps(mat.m + 8);
		row4 = _mm_loadu_ps(mat.m + 12);
		_MM_TRANSPOSE4_PS(row1, row2, row3, row4);
		_mm_storeu_ps(m + 0, row1);
		_mm_storeu_ps(m + 4, row2);
		_mm_storeu_ps(m + 8, row3);
		_mm_storeu_ps(m + 12, row4);
	}

	void Matrix::setTranslation(float x, float y, float z)
	{
		m[0] = m[5] = m[10] = m[15] = 1.0f;
		m[1] = m[2] = m[3] = m[4] = m[6] = m[7] = m[8] = m[9] = m[11] = 0.0f;
		m[12] = x;
		m[13] = y;
		m[14] = z;
	}

	void Matrix::setTranslation(const float vec[3])
	{
		m[0] = m[5] = m[10] = m[15] = 1.0f;
		m[1] = m[2] = m[3] = m[4] = m[6] = m[7] = m[8] = m[9] = m[11] = 0.0f;
		m[12] = vec[0];
		m[13] = vec[1];
		m[14] = vec[2];
	}

	void Matrix::setTranslation(const Vector3f& vec)
	{
		m[0] = m[5] = m[10] = m[15] = 1.0f;
		m[1] = m[2] = m[3] = m[4] = m[6] = m[7] = m[8] = m[9] = m[11] = 0.0f;
		m[12] = vec[0];
		m[13] = vec[1];
		m[14] = vec[2];
	}

	void Matrix::setScale(float scale)
	{
		memset(m, 0, sizeof(float) * 16);
		m[0] = m[5] = m[10] = scale;
		m[15] = 1.0f;
	}

	void Matrix::setScale(float x, float y, float z)
	{
		memset(m, 0, sizeof(float) * 16);
		m[0] = x;
		m[5] = y;
		m[10] = z;
		m[15] = 1.0f;
	}

	void Matrix::setScale(const Vector3f& vec)
	{
		memset(m, 0, sizeof(float) * 16);
		m[0] = vec[0];
		m[5] = vec[1];
		m[10] = vec[2];
		m[15] = 1.0f;
	}

	void Matrix::setReflect(const Plane &plane)
	{
		Vector3f pNormal = plane.normal();
		float d = plane.d();
		pNormal.normalize();

		m[0] = -2.0f * pNormal[0] * pNormal[0] + 1.0f;
		m[1] = -2.0f * pNormal[1] * pNormal[0];
		m[2] = -2.0f * pNormal[2] * pNormal[0];
		m[3] = 0.0f;

		m[4] = -2.0f * pNormal[0] * pNormal[1];
		m[5] = -2.0f * pNormal[1] * pNormal[1] + 1.0f;
		m[6] = -2.0f * pNormal[2] * pNormal[1];
		m[7] = 0.0f;

		m[8] = -2.0f * pNormal[0] * pNormal[2];
		m[9] = -2.0f * pNormal[1] * pNormal[2];
		m[10] = -2.0f * pNormal[2] * pNormal[2] + 1.0f;
		m[11] = 0.0f;

		m[12] = -2.0f * pNormal[0] * d;
		m[13] = -2.0f * pNormal[1] * d;
		m[14] = -2.0f * pNormal[2] * d;
		m[15] = 1.0f;
	}

	void Matrix::setReflect(float a, float b, float c, float d)
	{
		m[0] = -2.0f * a * a + 1.0f;
		m[1] = -2.0f * b * a;
		m[2] = -2.0f * c * a;
		m[3] = 0.0f;

		m[4] = -2.0f * a * b;
		m[5] = -2.0f * b * b + 1.0f;
		m[6] = -2.0f * c * b;
		m[7] = 0.0f;

		m[8] = -2.0f * a * c;
		m[9] = -2.0f * b * c;
		m[10] = -2.0f * c * c + 1.0f;
		m[11] = 0.0f;

		m[12] = -2.0f * a * d;
		m[13] = -2.0f * b * d;
		m[14] = -2.0f * c * d;
		m[15] = 1.0f;
	}

	void Matrix::setRotationX(float angleDeg)
	{
		float c, s;

		Math::sinCos(Math::convDeg2Rad(angleDeg), s, c);

		m[9] = -s;
		m[10] = c;
		m[5] = c;
		m[6] = s;

		m[1] = m[2] = m[3] = m[4] = m[7] = m[8] = m[11] = m[12] = m[13] = m[14] = 0.0f;
		m[0] = m[15] = 1.0f;
	}

	void Matrix::setRotationY(float angleDeg)
	{
		float c, s;

		Math::sinCos(Math::convDeg2Rad(angleDeg), s, c);

		m[0] = c;
		m[2] = -s;
		m[8] = s;
		m[10] = c;

		m[1] = m[3] = m[4] = m[6] = m[7] = m[9] = m[11] = m[12] = m[13] = m[14] = 0.0f;
		m[5] = m[15] = 1.0f;
	}

	void Matrix::setRotationZ(float angleDeg)
	{
		float c, s;

		Math::sinCos(Math::convDeg2Rad(angleDeg), s, c);

		m[0] = c;
		m[1] = s;
		m[4] = -s;
		m[5] = c;

		m[2] = m[3] = m[6] = m[7] = m[8] = m[9] = m[11] = m[12] = m[13] = m[14] = 0.0f;
		m[10] = m[15] = 1.0f;
	}

	void Matrix::setRotation(float angleDeg, const Vector3f& vec)
	{
		Vector3f aux;
		float c, s, t, txx, tyy, tzz, txy, txz, tyz, sx, sy, sz;

		aux = vec;
		aux.normalize();

		Math::sinCos(Math::convDeg2Rad(angleDeg), s, c);

		t = 1.0f - c;
		txx = aux[0] * aux[0] * t;
		tyy = aux[1] * aux[1] * t;
		tzz = aux[2] * aux[2] * t;
		txy = aux[0] * aux[1] * t;
		txz = aux[0] * aux[2] * t;
		tyz = aux[1] * aux[2] * t;
		sx = s * aux[0];
		sy = s * aux[1];
		sz = s * aux[2];

		m[0] = txx + c;
		m[1] = txy + sz;
		m[2] = txz - sy;

		m[4] = txy - sz;
		m[5] = tyy + c;
		m[6] = tyz + sx;

		m[8] = txz + sy;
		m[9] = tyz - sx;
		m[10] = tzz + c;

		m[3] = m[7] = m[11] = m[12] = m[13] = m[14] = 0.0f;
		m[15] = 1.0f;
	}

	void Matrix::setRotation(float angleDegX, float angleDegY, float angleDegZ)
	{
		float cx, cy, cz, sx, sy, sz;

		Math::sinCos(Math::convDeg2Rad(angleDegX), sx, cx);
		Math::sinCos(Math::convDeg2Rad(angleDegY), sy, cy);
		Math::sinCos(Math::convDeg2Rad(angleDegZ), sz, cz);

		m[0] = cy * cz;
		m[1] = -cy * sz;
		m[2] = sy;

		m[4] = cz * sx * sy + cx * sz;
		m[5] = cx * cz - sx * sy * sz;
		m[6] = -cy * sx;

		m[8] = -cx * cz * sy + sx * sz;
		m[9] = cz * sx + cx * sy + sz;
		m[10] = cx * cy;

		m[15] = 1.0f;
		m[3] = m[7] = m[11] = m[12] = m[13] = m[14] = 0.0f;
	}

	void Matrix::setSaturation(float sat)
	{
		float minusS, posS;

		posS = Math::fClamp(sat, -1.0f, 1.0f);
		minusS = 1.0f - posS;

		m[0] = minusS*0.3086f + posS;
		m[1] = m[2] = minusS*0.3086f;
		m[4] = m[6] = minusS*0.6094f;
		m[5] = minusS*0.6094f + posS;
		m[8] = m[9] = minusS*0.0820f;
		m[10] = minusS*0.0820f + posS;

		m[3] = m[7] = m[11] = m[12] = m[13] = m[14] = 0.0f;
		m[15] = 1.0f;
	}

	void Matrix::setRotationFromTo(const Vector3f& from, const Vector3f& to)
	{
		Vector3f v;
		float e, h, f;

		v.storeCrossProduct(from, to);
		f = e = from.getDot(to);
		if (e < 0.0f)
			f = -e;

		if (f > (1.0f - 0.000000001))
		{
			float u[3], v[3], x[3], c1, c2, c3;
			int i, j;

			x[0] = (from[0] > 0.0) ? from[0] : -from[0];
			x[1] = (from[1] > 0.0) ? from[1] : -from[1];
			x[2] = (from[2] > 0.0) ? from[2] : -from[2];

			if (x[0] < x[1])
			{
				if (x[0] < x[2])
				{
					x[0] = 1.0f;
					x[1] = x[2] = 0.0f;
				}
				else
				{
					x[2] = 1.0f;
					x[0] = x[1] = 0.0f;
				}
			}
			else
			{
				if (x[1] < x[2])
				{
					x[1] = 1.0f;
					x[0] = x[2] = 0.0f;
				}
				else
				{
					x[2] = 1.0f;
					x[0] = x[1] = 0.0f;
				}
			}

			u[0] = x[0] - from[0];
			u[1] = x[1] - from[1];
			u[2] = x[2] - from[2];
			v[0] = x[0] - to[0];
			v[1] = x[1] - to[1];
			v[2] = x[2] - to[2];


			c1 = 2.0f / (u[0] * u[0] + u[1] * u[1] + u[2] * u[2]);
			c2 = 2.0f / (v[0] * v[0] + v[1] * v[1] + v[2] * v[2]);
			c3 = c1 * c2  * (u[0] * v[0] + u[1] * v[1] + u[2] * v[2]);

			for (i = 0; i < 3; i++)
			{
				for (j = 0; j < 3; j++)
					m[j * 4 + i] = -c1 * u[i] * u[j] - c2 * v[i] * v[j] + c3 * v[i] * u[j];
				m[i * 4 + i] += 1.0f;
			}
			m[3] = m[7] = m[11] = m[12] = m[13] = m[14] = 0.0f;
			m[15] = 1.0f;
			return;
		}

		float hvx, hvz, hvxy, hvxz, hvyz;

		h = 1.0f / (1.0f + e);
		hvx = h * v[0];
		hvz = h * v[2];
		hvxy = hvx * v[1];
		hvxz = hvx * v[2];
		hvyz = hvz * v[1];

		m[0] = e + hvx * v[0];
		m[4] = hvxy - v[2];
		m[8] = hvxz + v[1];

		m[1] = hvxy + v[2];
		m[5] = e + h * v[1] * v[1];
		m[9] = hvyz - v[0];

		m[2] = hvxz - v[1];
		m[6] = hvyz + v[0];
		m[10] = e + hvz * v[2];

		m[3] = m[7] = m[11] = m[12] = m[13] = m[14] = 0.0f;
		m[15] = 1.0f;
	}

	void Matrix::setGLModelView(const Vector3f& pos, const Vector3f& target, const Vector3f& up)
	{
		Vector3f x, y, z;

		z.set(pos);
		z -= target;
		z.normalize();

		x.storeCrossProduct(up, z);
		y.storeCrossProduct(z, x);

		x.normalize();
		y.normalize();

		m[0] = x[0];
		m[1] = y[0];
		m[2] = z[0];
		m[4] = x[1];
		m[5] = y[1];
		m[6] = z[1];
		m[8] = x[2];
		m[9] = y[2];
		m[10] = z[2];

		x *= -1.0f;
		y *= -1.0f;
		z *= -1.0f;

		m[12] = x[0] * pos[0] + x[1] * pos[1] + x[2] * pos[2];
		m[13] = y[0] * pos[0] + y[1] * pos[1] + y[2] * pos[2];
		m[14] = z[0] * pos[0] + z[1] * pos[1] + z[2] * pos[2];

		m[3] = m[7] = m[11] = 0.0f;
		m[15] = 1.0f;
	}

	void Matrix::setGLModelView(const Vector3f& pos, const Vector3f& target)
	{
		Vector3f up(0.0f, 1.0f, 0.0f);
		setGLModelView(pos, target, up);
	}

	void Matrix::setGLModelView(const Vector3f& pos, float angleDegX, float angleDegY, const Vector3f& up)
	{
		Vector3f target, x, y, z;
		float sx, sy, cx, cy;

		Math::sinCos(Math::convDeg2Rad(angleDegX), sx, cx);
		Math::sinCos(Math::convDeg2Rad(angleDegY), sy, cy);

		target[0] = pos[0] + sx*cy;
		target[1] = pos[1] + sy;
		target[2] = pos[2] - (cx*cy);

		z.set(pos);
		z -= target;
		z.normalize();

		x.storeCrossProduct(up, z);
		y.storeCrossProduct(z, x);

		x.normalize();
		y.normalize();

		m[0] = x[0];
		m[1] = y[0];
		m[2] = z[0];
		m[4] = x[1];
		m[5] = y[1];
		m[6] = z[1];
		m[8] = x[2];
		m[9] = y[2];
		m[10] = z[2];

		x *= -1.0f;
		y *= -1.0f;
		z *= -1.0f;

		m[12] = x[0] * pos[0] + x[1] * pos[1] + x[2] * pos[2];
		m[13] = y[0] * pos[0] + y[1] * pos[1] + y[2] * pos[2];
		m[14] = z[0] * pos[0] + z[1] * pos[1] + z[2] * pos[2];

		m[3] = m[7] = m[11] = 0.0f;
		m[15] = 1.0f;
	}

	void Matrix::setGLModelView(int cubemapFace, const Vector3f& centerCube)
	{
		std::memset(m, 0, sizeof(float) * 16);

		//positive X
		if (cubemapFace == 0)
		{
			m[2] = m[5] = m[8] = -1.0f;
			m[12] = centerCube[2];
			m[13] = centerCube[1];
			m[14] = centerCube[0];
			m[15] = 1.0f;
			return;
		}

		//negative X
		if (cubemapFace == 1)
		{
			m[2] = m[8] = 1.0f;
			m[5] = -1.0f;
			m[12] = -centerCube[2];
			m[13] = centerCube[1];
			m[14] = -centerCube[0];
			m[15] = 1.0f;
			return;
		}

		//positive Y
		if (cubemapFace == 2)
		{
			m[0] = m[9] = 1.0f;
			m[6] = -1.0f;
			m[12] = -centerCube[0];
			m[13] = -centerCube[2];
			m[14] = centerCube[1];
			m[15] = 1.0f;
			return;
		}

		//negative Y
		if (cubemapFace == 3)
		{
			m[0] = m[6] = 1.0f;
			m[9] = -1.0f;
			m[12] = -centerCube[0];
			m[13] = centerCube[2];
			m[14] = -centerCube[1];
			m[15] = 1.0f;
			return;
		}

		//positive Z
		if (cubemapFace == 4)
		{
			m[5] = m[10] = -1.0f;
			m[0] = 1.0f;
			m[12] = -centerCube[0];
			m[13] = centerCube[1];
			m[14] = centerCube[2];
			m[15] = 1.0f;
			return;
		}

		//negative Z
		if (cubemapFace == 5)
		{
			m[0] = m[5] = -1.0f;
			m[10] = 1.0f;
			m[12] = centerCube[0];
			m[13] = centerCube[1];
			m[14] = -centerCube[2];
			m[15] = 1.0f;
			return;
		}

		m[0] = m[5] = m[10] = m[15] = 1.0f;
	}

	void Matrix::setGLProjection3D(float fovy, float aspect, float zNear, float zFar)
	{
		double ymin, ymax, xmin, xmax, n, f;

		n = (double)zNear;
		f = (double)zFar;
		ymax = n * tan(((double)fovy) * 0.00872664625997164788461845384);
		ymin = -ymax;
		xmin = ymin * ((double)aspect);
		xmax = ymax * ((double)aspect);

		std::memset(m, 0, sizeof(float) * 16);
		m[0] = (float)((2.0*n) / (xmax - xmin));
		m[5] = (float)((2.0*n) / (ymax - ymin));
		m[8] = (float)((xmax + xmin) / (xmax - xmin));
		m[9] = (float)((ymax + ymin) / (ymax - ymin));
		m[10] = -(float)((f + n) / (f - n));
		m[11] = -1.0f;
		m[14] = -(float)((2.0*f*n) / (f - n));
	}

	void Matrix::setGLProjection3D(float fovy, float aspect, float zNear)
	{
		double ymin, ymax, xmin, xmax, n;

		n = (double)zNear;
		ymax = n * tan(((double)fovy) * 0.00872664625997164788461845384);
		ymin = -ymax;
		xmin = ymin * ((double)aspect);
		xmax = ymax * ((double)aspect);

		std::memset(m, 0, sizeof(float) * 16);
		m[0] = (float)((2.0*n) / (xmax - xmin));
		m[5] = (float)((2.0*n) / (ymax - ymin));
		m[8] = (float)((xmax + xmin) / (xmax - xmin));
		m[9] = (float)((ymax + ymin) / (ymax - ymin));
		m[10] = m[11] = -1.0f;
		m[14] = -(float)(2.0*n);
	}

	void Matrix::setGLProjection2D(float width, float height)
	{
		std::memset(m, 0, sizeof(float) * 16);

		m[0] = 2.0f / width;
		m[5] = 2.0f / height;
		m[10] = m[12] = m[13] = -1.0f;
		m[15] = 1.0f;
	}

	void Matrix::setGLProjectionOrtho(float left, float right, float bottom, float top, float zNear, float zFar)
	{
		std::memset(m, 0, sizeof(float) * 16);

		m[0] = 2.0f / (right - left);
		m[5] = 2.0f / (top - bottom);
		m[10] = -2.0f / (zFar - zNear);
		m[15] = 1.0f;

		m[12] = -(right + left) / (right - left);
		m[13] = -(top + bottom) / (top - bottom);
		m[14] = -(zFar + zNear) / (zFar - zNear);
	}

	void Matrix::setGLProjectionOrtho(const BBox &bbox)
	{
		Vector3f bmin, bmax;
		bbox.minMax(bmin, bmax);

		float xDist = (bmax[0] - bmin[0])*0.5f;
		float yDist = (bmax[1] - bmin[1])*0.5f;
		float zDist = bmax[2] - bmin[2];

		std::memset(m, 0, sizeof(float) * 16);

		m[0] = 2.0f / (xDist + xDist);
		m[5] = 2.0f / (yDist + yDist);
		m[10] = -2.0f / (zDist);
		m[15] = 1.0f;

		m[12] = -(xDist - xDist) / (xDist + xDist);
		m[13] = -(yDist - yDist) / (yDist + yDist);
		m[14] = -1.0f;
	}

	Matrix3::Matrix3(const Matrix &mat)
	{
		m[0] = mat.m[0];		m[1] = mat.m[1];		m[2] = mat.m[2];
		m[3] = mat.m[4];		m[4] = mat.m[5];		m[5] = mat.m[6];
		m[6] = mat.m[8];		m[7] = mat.m[9];		m[8] = mat.m[10];
	}

	Matrix3::Matrix3(const float src[9])
	{
		std::memcpy(m, src, sizeof(float) * 9);
	}

	void Matrix3::operator*=(const Matrix &s)
	{
		float matAux[9];
		std::memcpy(matAux, m, sizeof(float) * 9);

		m[0] = s.m[0] * matAux[0] + s.m[1] * matAux[3] + s.m[2] * matAux[6];
		m[1] = s.m[0] * matAux[1] + s.m[1] * matAux[4] + s.m[2] * matAux[7];
		m[2] = s.m[0] * matAux[2] + s.m[1] * matAux[5] + s.m[2] * matAux[8];

		m[3] = s.m[4] * matAux[0] + s.m[5] * matAux[3] + s.m[6] * matAux[6];
		m[4] = s.m[4] * matAux[1] + s.m[5] * matAux[4] + s.m[6] * matAux[7];
		m[5] = s.m[4] * matAux[2] + s.m[5] * matAux[5] + s.m[6] * matAux[8];

		m[6] = s.m[8] * matAux[0] + s.m[9] * matAux[3] + s.m[10] * matAux[6];
		m[7] = s.m[8] * matAux[1] + s.m[9] * matAux[4] + s.m[10] * matAux[7];
		m[8] = s.m[8] * matAux[2] + s.m[9] * matAux[5] + s.m[10] * matAux[8];
	}

	void Matrix3::operator*=(const Matrix3 &s)
	{
		float matAux[9];
		std::memcpy(matAux, m, sizeof(float) * 9);

		m[0] = s.m[0] * matAux[0] + s.m[1] * matAux[3] + s.m[2] * matAux[6];
		m[1] = s.m[0] * matAux[1] + s.m[1] * matAux[4] + s.m[2] * matAux[7];
		m[2] = s.m[0] * matAux[2] + s.m[1] * matAux[5] + s.m[2] * matAux[8];

		m[3] = s.m[3] * matAux[0] + s.m[4] * matAux[3] + s.m[5] * matAux[6];
		m[4] = s.m[3] * matAux[1] + s.m[4] * matAux[4] + s.m[5] * matAux[7];
		m[5] = s.m[3] * matAux[2] + s.m[4] * matAux[5] + s.m[5] * matAux[8];

		m[6] = s.m[6] * matAux[0] + s.m[7] * matAux[3] + s.m[8] * matAux[6];
		m[7] = s.m[6] * matAux[1] + s.m[7] * matAux[4] + s.m[8] * matAux[7];
		m[8] = s.m[6] * matAux[2] + s.m[7] * matAux[5] + s.m[8] * matAux[8];
	}


	void Matrix3::operator*=(const float src[9])
	{
		float matAux[9];
		std::memcpy(matAux, m, sizeof(float) * 9);

		m[0] = src[0] * matAux[0] + src[1] * matAux[3] + src[2] * matAux[6];
		m[1] = src[0] * matAux[1] + src[1] * matAux[4] + src[2] * matAux[7];
		m[2] = src[0] * matAux[2] + src[1] * matAux[5] + src[2] * matAux[8];

		m[3] = src[3] * matAux[0] + src[4] * matAux[3] + src[5] * matAux[6];
		m[4] = src[3] * matAux[1] + src[4] * matAux[4] + src[5] * matAux[7];
		m[5] = src[3] * matAux[2] + src[4] * matAux[5] + src[5] * matAux[8];

		m[6] = src[6] * matAux[0] + src[7] * matAux[3] + src[8] * matAux[6];
		m[7] = src[6] * matAux[1] + src[7] * matAux[4] + src[8] * matAux[7];
		m[8] = src[6] * matAux[2] + src[7] * matAux[5] + src[8] * matAux[8];
	}

	void Matrix3::operator+=(const Matrix3 &s)
	{
		m[0] += s.m[0];
		m[1] += s.m[1];
		m[2] += s.m[2];

		m[3] += s.m[3];
		m[4] += s.m[4];
		m[5] += s.m[5];

		m[6] += s.m[6];
		m[7] += s.m[7];
		m[8] += s.m[8];
	}

	void Matrix3::operator+=(const float src[9])
	{
		m[0] += src[0];
		m[1] += src[1];
		m[2] += src[2];

		m[3] += src[3];
		m[4] += src[4];
		m[5] += src[5];

		m[6] += src[6];
		m[7] += src[7];
		m[8] += src[8];
	}

	void Matrix3::operator-=(const Matrix3 &s)
	{
		m[0] -= s.m[0];
		m[1] -= s.m[1];
		m[2] -= s.m[2];

		m[3] -= s.m[3];
		m[4] -= s.m[4];
		m[5] -= s.m[5];

		m[6] -= s.m[6];
		m[7] -= s.m[7];
		m[8] -= s.m[8];
	}

	void Matrix3::operator-=(const float src[9])
	{
		m[0] -= src[0];
		m[1] -= src[1];
		m[2] -= src[2];

		m[3] -= src[3];
		m[4] -= src[4];
		m[5] -= src[5];

		m[6] -= src[6];
		m[7] -= src[7];
		m[8] -= src[8];
	}

	Matrix3 Matrix3::operator*(const Matrix3 &s) const
	{
		Matrix3 res(*this);
		res *= s;
		return res;
	}

	Matrix3 Matrix3::operator+(const Matrix3 &s) const
	{
		Matrix3 res(*this);
		res += s;
		return res;
	}

	Matrix3 Matrix3::operator-(const Matrix3 &s) const
	{
		Matrix3 res(*this);
		res -= s;
		return res;
	}

	void Matrix3::transform(float vec[3]) const
	{
		float vecX = vec[0];
		float vecY = vec[1];
		float vecZ = vec[2];

		vec[0] = vecX*m[0] + vecY*m[3] + vecZ*m[6];
		vec[1] = vecX*m[1] + vecY*m[4] + vecZ*m[7];
		vec[2] = vecX*m[2] + vecY*m[5] + vecZ*m[8];
	}

	void Matrix3::transform(Vector3f& vec) const
	{
		float result[3];

		result[0] = vec[0]*m[0] + vec[1]*m[3] + vec[2]*m[6];
		result[1] = vec[0]*m[1] + vec[1]*m[4] + vec[2]*m[7];
		result[2] = vec[0]*m[2] + vec[1]*m[5] + vec[2]*m[8];

		vec[0] = result[0];
		vec[1] = result[1];
		vec[2] = result[2];
	}

	void Matrix3::transform(const Vector3f& vec, Vector3f& result) const
	{
		result[0] = vec[0]*m[0] + vec[1]*m[3] + vec[2]*m[6];
		result[1] = vec[0]*m[1] + vec[1]*m[4] + vec[2]*m[7];
		result[2] = vec[0]*m[2] + vec[1]*m[5] + vec[2]*m[8];
	}

	void Matrix3::transform(Vector3f* const vec, size_t numVec) const
	{
		float result[3];

		for (size_t i = 0; i < numVec; i++)
		{
			result[0] = vec[i][0]*m[0] + vec[i][1]*m[3] + vec[i][2]*m[6];
			result[1] = vec[i][0]*m[1] + vec[i][1]*m[4] + vec[i][2]*m[7];
			result[2] = vec[i][0]*m[2] + vec[i][1]*m[5] + vec[i][2]*m[8];

			vec[i][0] = result[0];
			vec[i][1] = result[1];
			vec[i][2] = result[2];
		}
	}

	Vector3f Matrix3::getColumn(size_t columnIndex) const
	{
		columnIndex = columnIndex % 3;
		return Vector3f(m[columnIndex], m[columnIndex + 3], m[columnIndex + 6]);
	}

	Vector3f Matrix3::getRow(size_t rowIndex) const
	{
		return Vector3f(m + ((rowIndex % 3) * 3));
	}

	void Matrix3::getRotation(Vector3f& vec, float &angle) const
	{
		float s, quat[4];
		int op;

		float tr = m[0] + m[4] + m[8];

		if (tr > 0.0f)
		{
			s = sqrt(tr + 1.0f);

			quat[3] = s*0.5f;

			s = 0.5f / s;

			quat[0] = (m[5] - m[7])*s;
			quat[1] = (m[6] - m[2])*s;
			quat[2] = (m[1] - m[3])*s;
		}
		else
		{
			op = 0;
			if (m[4] > m[0])
				op = 1;
			if (m[8] > m[op * 4])
				op = 2;

			switch (op){
			case 0:
				s = sqrt((m[0] - (m[4] + m[8])) + 1.0f);

				quat[0] = s*0.5f;
				if (s != 0.0f)
					s = 0.5f / s;

				quat[1] = (m[1] + m[3])*s;
				quat[2] = (m[2] + m[6])*s;
				quat[3] = (m[5] - m[7])*s;

				break;

			case 1:
				s = sqrt((m[4] - (m[8] + m[0])) + 1.0f);

				quat[1] = s*0.5f;
				if (s != 0.0f)
					s = 0.5f / s;

				quat[3] = (m[6] - m[2])*s;
				quat[2] = (m[5] + m[7])*s;
				quat[0] = (m[3] + m[1])*s;

				break;

			case 2:
				s = sqrt((m[8] - (m[0] + m[4])) + 1.0f);

				quat[2] = s*0.5f;
				if (s != 0.0f)
					s = 0.5f / s;

				quat[3] = (m[1] - m[3])*s;
				quat[0] = (m[6] + m[2])*s;
				quat[1] = (m[7] + m[5])*s;
				break;
			}
		}

		tr = quat[0] * quat[0] + quat[1] * quat[1] + quat[2] * quat[2];
		if (tr == 0.0f)
		{
			vec[0] = 0.0f;
			vec[1] = 0.0f;
			vec[2] = 1.0f;
			angle = 0.0f;
			return;
		}

		tr = 1.0f / tr;
		vec[0] = quat[0] * tr;
		vec[1] = quat[1] * tr;
		vec[2] = quat[2] * tr;
		vec.normalize();
		angle = ((float)acos(quat[3]))*114.5915590261646417f; // 180/pi=57.295779513082320876f * 2.0f
	}

	void Matrix3::getEulerAngles(float &rfYAngle, float &rfPAngle, float &rfRAngle) const
	{
		if (-1.0f < -m[6])
		{
			if (-m[6] < 1.0)
				rfPAngle = asin(-m[6]);
			else
				rfPAngle = -(2.0f*atanf(1.0f));
		}
		else
		{
			rfPAngle = 2.0f*atanf(1.0f);
		}

		if (rfPAngle < (2.0f*atanf(1.0f)))
		{
			if (rfPAngle > -(2.0f*atanf(1.0f)))
			{
				rfYAngle = atan2(m[3], m[0]);
				rfRAngle = atan2(m[7], m[8]);
			}
			else
			{
				float fRmY = atan2(-m[1], m[2]);
				rfRAngle = 0.0f;
				rfYAngle = rfRAngle - fRmY;
			}
		}

		else
		{
			float fRpY = atan2(-m[0], m[2]);
			rfRAngle = 0.0f;
			rfYAngle = fRpY - rfRAngle;
		}
	}

	void Matrix3::write(float dest[9]) const
	{
		std::memcpy(dest, m, sizeof(float) * 9);
	}

	void Matrix3::transpose(Matrix3 &dest) const
	{
		dest.m[1] = m[3];
		dest.m[2] = m[6];
		dest.m[3] = m[1];
		dest.m[5] = m[7];
		dest.m[6] = m[2];
		dest.m[7] = m[5];

		dest.m[0] = m[0];
		dest.m[4] = m[4];
		dest.m[8] = m[8];
	}

	void Matrix3::transpose()
	{
		std::swap(m[1], m[3]);
		std::swap(m[2], m[6]);
		std::swap(m[5], m[7]);
	}

	void Matrix3::mulRotationX(float angleDeg)
	{
		float c, s;
		Math::sinCos(Math::convDeg2Rad(angleDeg), s, c);

		float p1 = m[3];
		float p2 = m[4];
		float p3 = m[5];

		m[3] = (c * p1) + (s * m[6]);
		m[4] = (c * p2) + (s * m[7]);
		m[5] = (c * p3) + (s * m[8]);

		m[6] = (-s * p1) + (c * m[6]);
		m[7] = (-s * p2) + (c * m[7]);
		m[8] = (-s * p3) + (c * m[8]);
	}

	void Matrix3::mulRotationY(float angleDeg)
	{
		float c, s;
		Math::sinCos(Math::convDeg2Rad(angleDeg), s, c);

		float p1 = m[0];
		float p2 = m[1];
		float p3 = m[2];

		m[0] = (c * p1) + (-s * m[6]);
		m[1] = (c * p2) + (-s * m[7]);
		m[2] = (c * p3) + (-s * m[8]);

		m[6] = (s * p1) + (c * m[6]);
		m[7] = (s * p2) + (c * m[7]);
		m[8] = (s * p3) + (c * m[8]);
	}

	void Matrix3::mulRotationZ(float angleDeg)
	{
		float c, s;
		Math::sinCos(Math::convDeg2Rad(angleDeg), s, c);

		float p1 = m[0];
		float p2 = m[1];
		float p3 = m[2];

		m[0] = (c * p1) + (s * m[3]);
		m[1] = (c * p2) + (s * m[4]);
		m[2] = (c * p3) + (s * m[5]);

		m[3] = (-s * p1) + (c * m[3]);
		m[4] = (-s * p2) + (c * m[4]);
		m[5] = (-s * p3) + (c * m[5]);
	}

	void Matrix3::set(float value)
	{
		m[0] = m[1] = m[2] = value;
		m[3] = m[4] = m[5] = value;
		m[6] = m[7] = m[8] = value;
	}

	void Matrix3::set(const float src[9])
	{
		std::memcpy(m, src, sizeof(float) * 9);
	}

	void Matrix3::set(const Matrix &mat)
	{
		m[0] = mat.m[0];		m[1] = mat.m[1];		m[2] = mat.m[2];
		m[3] = mat.m[4];		m[4] = mat.m[5];		m[5] = mat.m[6];
		m[6] = mat.m[8];		m[7] = mat.m[9];		m[8] = mat.m[10];
	}

	void Matrix3::set(const Matrix3 &mat)
	{
		std::memcpy(m, mat.m, sizeof(float) * 9);
	}

	void Matrix3::setZero(void)
	{
		std::memset(m, 0, sizeof(float) * 9);
	}

	void Matrix3::setIdentity(void)
	{
		std::memset(m, 0, sizeof(float) * 9);
		m[0] = m[4] = m[8] = 1.0f;
	}

	void Matrix3::setRotationX(float angleDeg)
	{
		float c, s;
		Math::sinCos(Math::convDeg2Rad(angleDeg), s, c);

		m[7] = -s;
		m[8] = c;
		m[4] = c;
		m[5] = s;

		m[1] = m[2] = m[3] = m[6] = 0.0f;
		m[0] = 1.0f;
	}

	void Matrix3::setRotationY(float angleDeg)
	{
		float c, s;
		Math::sinCos(Math::convDeg2Rad(angleDeg), s, c);

		m[0] = c;
		m[2] = -s;
		m[6] = s;
		m[8] = c;

		m[1] = m[3] = m[5] = m[7] = 0.0f;
		m[4] = 1.0f;
	}

	void Matrix3::setRotationZ(float angleDeg)
	{
		float c, s;
		Math::sinCos(Math::convDeg2Rad(angleDeg), s, c);

		m[0] = c;
		m[1] = s;
		m[3] = -s;
		m[4] = c;

		m[2] = m[5] = m[6] = m[7] = 0.0f;
		m[8] = 1.0f;
	}

	void Matrix3::setRotation(float angleDeg, const Vector3f& vec)
	{
		float c, s, t, txx, tyy, tzz, txy, txz, tyz;

		Vector3f aux = vec;
		aux.normalize();

		Math::sinCos(Math::convDeg2Rad(angleDeg), s, c);

		t = 1.0f - c;
		txx = aux[0] * aux[0] * t;
		tyy = aux[1] * aux[1] * t;
		tzz = aux[2] * aux[2] * t;
		txy = aux[0] * aux[1] * t;
		txz = aux[0] * aux[2] * t;
		tyz = aux[1] * aux[2] * t;
		aux *= s;

		m[0] = txx + c;
		m[1] = txy + aux[2];
		m[2] = txz - aux[1];

		m[3] = txy - aux[2];
		m[4] = tyy + c;
		m[5] = tyz + aux[0];

		m[6] = txz + aux[1];
		m[7] = tyz - aux[0];
		m[8] = tzz + c;
	}

	void Matrix3::setRotation(float angleDegX, float angleDegY, float angleDegZ)
	{
		float cx, cy, cz, sx, sy, sz;

		Math::sinCos(Math::convDeg2Rad(angleDegX), sx, cx);
		Math::sinCos(Math::convDeg2Rad(angleDegY), sy, cy);
		Math::sinCos(Math::convDeg2Rad(angleDegZ), sz, cz);

		m[0] = cy * cz;
		m[1] = -cy * sz;
		m[2] = sy;

		m[3] = cz * sx * sy + cx * sz;
		m[4] = cx * cz - sx * sy * sz;
		m[5] = -cy * sx;

		m[6] = -cx * cz * sy + sx * sz;
		m[7] = cz * sx + cx * sy + sz;
		m[8] = cx * cy;
	}

	void Matrix3::setRotation(float angleDeg, float x, float y, float z)
	{
		setRotation(angleDeg, Vector3f(x, y, z));
	}

	void Matrix3::setRotationFromTo(const Vector3f& from, const Vector3f& to)
	{
		Vector3f v;
		float e, h, f;

		v.storeCrossProduct(from, to);
		f = e = from.getDot(to);
		if (e < 0.0f)
			f = -e;

		if (f > (1.0f - 0.000000001))
		{
			float u[3], v[3], x[3], c1, c2, c3;
			int i, j;

			x[0] = (from[0] > 0.0) ? from[0] : -from[0];
			x[1] = (from[1] > 0.0) ? from[1] : -from[1];
			x[2] = (from[2] > 0.0) ? from[2] : -from[2];

			if (x[0] < x[1])
			{
				if (x[0] < x[2])
				{
					x[0] = 1.0f;
					x[1] = x[2] = 0.0f;
				}
				else
				{
					x[2] = 1.0f;
					x[0] = x[1] = 0.0f;
				}
			}
			else
			{
				if (x[1] < x[2])
				{
					x[1] = 1.0f;
					x[0] = x[2] = 0.0f;
				}
				else
				{
					x[2] = 1.0f;
					x[0] = x[1] = 0.0f;
				}
			}

			u[0] = x[0] - from[0];
			u[1] = x[1] - from[1];
			u[2] = x[2] - from[2];
			v[0] = x[0] - to[0];
			v[1] = x[1] - to[1];
			v[2] = x[2] - to[2];


			c1 = 2.0f / (u[0] * u[0] + u[1] * u[1] + u[2] * u[2]);
			c2 = 2.0f / (v[0] * v[0] + v[1] * v[1] + v[2] * v[2]);
			c3 = c1 * c2  * (u[0] * v[0] + u[1] * v[1] + u[2] * v[2]);

			for (i = 0; i < 3; i++)
			{
				for (j = 0; j < 3; j++)
					m[j * 3 + i] = -c1 * u[i] * u[j] - c2 * v[i] * v[j] + c3 * v[i] * u[j];
				m[i * 3 + i] += 1.0f;
			}
			return;
		}

		float hvx, hvz, hvxy, hvxz, hvyz;

		h = 1.0f / (1.0f + e);
		hvx = h * v[0];
		hvz = h * v[2];
		hvxy = hvx * v[1];
		hvxz = hvx * v[2];
		hvyz = hvz * v[1];

		m[0] = e + hvx * v[0];
		m[3] = hvxy - v[2];
		m[6] = hvxz + v[1];

		m[1] = hvxy + v[2];
		m[4] = e + h * v[1] * v[1];
		m[7] = hvyz - v[0];

		m[2] = hvxz - v[1];
		m[5] = hvyz + v[0];
		m[8] = e + hvz * v[2];
	}
}