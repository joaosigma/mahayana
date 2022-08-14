#include "matrix.hpp"

#include "math.hpp"

#include <cstring>
#include <type_traits>
#include <immintrin.h>
#include <xmmintrin.h>

namespace hr
{
	namespace
	{
		void asmMat4x4Vec3(float *vecWrite, const float *vecRead, float wCompMul, size_t stride, const float *mat, size_t numVec)
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

		void asmMat4x4Vec4(float *vecWrite, const float *vecRead, size_t stride, const float *mat, size_t numVec)
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

		void fastMat4x4Mult(float * const result, const float * const mat1, const float * const mat2)
		{
			_mm256_zeroupper();

			__m256 a0, a1;
			a0 = _mm256_loadu_ps(mat1 + 0);
			a1 = _mm256_loadu_ps(mat1 + 8);

			__m128 b0, b1, b2, b3;
			b0 = _mm_loadu_ps(mat2 + 0);
			b1 = _mm_loadu_ps(mat2 + 4);
			b2 = _mm_loadu_ps(mat2 + 8);
			b3 = _mm_loadu_ps(mat2 + 12);

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

		void fastMat4x4Transpose(float * const result, const float * const mat)
		{
			__m128 row1, row2, row3, row4;

			row1 = _mm_loadu_ps(mat + 0);
			row2 = _mm_loadu_ps(mat + 4);
			row3 = _mm_loadu_ps(mat + 8);
			row4 = _mm_loadu_ps(mat + 12);
			_MM_TRANSPOSE4_PS(row1, row2, row3, row4);
			_mm_storeu_ps(result + 0, row1);
			_mm_storeu_ps(result + 4, row2);
			_mm_storeu_ps(result + 8, row3);
			_mm_storeu_ps(result + 12, row4);
		}
	}

	static_assert(std::is_trivially_copyable<Matrix>::value);
	static_assert(std::is_trivially_copyable<Matrix3>::value);

	Matrix::Matrix(const Matrix3 &mat) noexcept
		: m{ mat.m[0], mat.m[1], mat.m[2], 0.0f, mat.m[3], mat.m[4], mat.m[5], 0.0f, mat.m[6], mat.m[7], mat.m[8], 0.0f, 0.0f, 0.0f, 0.0f, 1.0f }
	{ }

	Matrix::Matrix(const double src[16]) noexcept
	{
		m[0] = static_cast<float>(src[0]);
		m[1] = static_cast<float>(src[1]);
		m[2] = static_cast<float>(src[2]);
		m[3] = static_cast<float>(src[3]);
		m[4] = static_cast<float>(src[4]);
		m[5] = static_cast<float>(src[5]);
		m[6] = static_cast<float>(src[6]);
		m[7] = static_cast<float>(src[7]);
		m[8] = static_cast<float>(src[8]);
		m[9] = static_cast<float>(src[9]);
		m[10] = static_cast<float>(src[10]);
		m[11] = static_cast<float>(src[11]);
		m[12] = static_cast<float>(src[12]);
		m[13] = static_cast<float>(src[13]);
		m[14] = static_cast<float>(src[14]);
		m[15] = static_cast<float>(src[15]);
	}

	Matrix::Matrix(const Quaternion &unitQuaternion) noexcept
	{
		m[0] = 1.0f - 2.0f * (unitQuaternion[1] * unitQuaternion[1] + unitQuaternion[2] * unitQuaternion[2]);
		m[4] =        2.0f * (unitQuaternion[0] * unitQuaternion[1] - unitQuaternion[2] * unitQuaternion[3]);
		m[8] =        2.0f * (unitQuaternion[0] * unitQuaternion[2] + unitQuaternion[1] * unitQuaternion[3]);

		m[1] =        2.0f * (unitQuaternion[0] * unitQuaternion[1] + unitQuaternion[2] * unitQuaternion[3]);
		m[5] = 1.0f - 2.0f * (unitQuaternion[0] * unitQuaternion[0] + unitQuaternion[2] * unitQuaternion[2]);
		m[9] =        2.0f * (unitQuaternion[1] * unitQuaternion[2] - unitQuaternion[0] * unitQuaternion[3]);

		m[2] =         2.0f * (unitQuaternion[0] * unitQuaternion[2] - unitQuaternion[1] * unitQuaternion[3]);
		m[6] =         2.0f * (unitQuaternion[1] * unitQuaternion[2] + unitQuaternion[0] * unitQuaternion[3]);
		m[10] = 1.0f - 2.0f * (unitQuaternion[0] * unitQuaternion[0] + unitQuaternion[1] * unitQuaternion[1]);

		m[3] = m[7] = m[11] = m[12] = m[13] = m[14] = 0.0f;
		m[15] = 1.0f;
	}

	void Matrix::operator*=(const float s) noexcept
	{
		__m128 scalar = _mm_load_ps1(&s);

		_mm_storeu_ps(m +  0, _mm_mul_ps(_mm_loadu_ps(m +  0), scalar));
		_mm_storeu_ps(m +  4, _mm_mul_ps(_mm_loadu_ps(m +  4), scalar));
		_mm_storeu_ps(m +  8, _mm_mul_ps(_mm_loadu_ps(m +  8), scalar));
		_mm_storeu_ps(m + 12, _mm_mul_ps(_mm_loadu_ps(m + 12), scalar));
	}

	void Matrix::operator*=(const Matrix &s) noexcept
	{
		fastMat4x4Mult(this->m, this->m, s.m);
	}

	void Matrix::operator*=(const Matrix3 &s) noexcept
	{
		Matrix m4{ s };
		fastMat4x4Mult(this->m, this->m, m4.m);
	}

	void Matrix::operator*=(const float src[16]) noexcept
	{
		fastMat4x4Mult(m, m, src);
	}

	void Matrix::operator*=(const Quaternion &unitQuaternion) noexcept
	{
		Matrix mat(unitQuaternion);
		fastMat4x4Mult(m, m, mat.m);
	}

	void Matrix::operator+=(const Matrix &s) noexcept
	{
		_mm_storeu_ps(m +  0, _mm_add_ps(_mm_loadu_ps(m +  0), _mm_loadu_ps(s.m +  0)));
		_mm_storeu_ps(m +  4, _mm_add_ps(_mm_loadu_ps(m +  4), _mm_loadu_ps(s.m +  4)));
		_mm_storeu_ps(m +  8, _mm_add_ps(_mm_loadu_ps(m +  8), _mm_loadu_ps(s.m +  8)));
		_mm_storeu_ps(m + 12, _mm_add_ps(_mm_loadu_ps(m + 12), _mm_loadu_ps(s.m + 12)));
	}

	void Matrix::operator+=(const float src[16]) noexcept
	{
		_mm_storeu_ps(m +  0, _mm_add_ps(_mm_loadu_ps(m +  0), _mm_loadu_ps(src +  0)));
		_mm_storeu_ps(m +  4, _mm_add_ps(_mm_loadu_ps(m +  4), _mm_loadu_ps(src +  4)));
		_mm_storeu_ps(m +  8, _mm_add_ps(_mm_loadu_ps(m +  8), _mm_loadu_ps(src +  8)));
		_mm_storeu_ps(m + 12, _mm_add_ps(_mm_loadu_ps(m + 12), _mm_loadu_ps(src + 12)));
	}

	void Matrix::operator-=(const Matrix &s) noexcept
	{
		_mm_storeu_ps(m +  0, _mm_sub_ps(_mm_loadu_ps(m +  0), _mm_loadu_ps(s.m +  0)));
		_mm_storeu_ps(m +  4, _mm_sub_ps(_mm_loadu_ps(m +  4), _mm_loadu_ps(s.m +  4)));
		_mm_storeu_ps(m +  8, _mm_sub_ps(_mm_loadu_ps(m +  8), _mm_loadu_ps(s.m +  8)));
		_mm_storeu_ps(m + 12, _mm_sub_ps(_mm_loadu_ps(m + 12), _mm_loadu_ps(s.m + 12)));
	}

	void Matrix::operator-=(const float src[16]) noexcept
	{
		_mm_storeu_ps(m +  0, _mm_sub_ps(_mm_loadu_ps(m +  0), _mm_loadu_ps(src +  0)));
		_mm_storeu_ps(m +  4, _mm_sub_ps(_mm_loadu_ps(m +  4), _mm_loadu_ps(src +  4)));
		_mm_storeu_ps(m +  8, _mm_sub_ps(_mm_loadu_ps(m +  8), _mm_loadu_ps(src +  8)));
		_mm_storeu_ps(m + 12, _mm_sub_ps(_mm_loadu_ps(m + 12), _mm_loadu_ps(src + 12)));
	}

	Matrix Matrix::operator*(const Matrix &s) const noexcept
	{
		Matrix res;
		fastMat4x4Mult(res.m, m, s.m);
		return res;
	}

	Matrix Matrix::operator+(const Matrix &s) const noexcept
	{
		Matrix res;
		_mm_storeu_ps(res.m +  0, _mm_add_ps(_mm_loadu_ps(m +  0), _mm_loadu_ps(s.m +  0)));
		_mm_storeu_ps(res.m +  4, _mm_add_ps(_mm_loadu_ps(m +  4), _mm_loadu_ps(s.m +  4)));
		_mm_storeu_ps(res.m +  8, _mm_add_ps(_mm_loadu_ps(m +  8), _mm_loadu_ps(s.m +  8)));
		_mm_storeu_ps(res.m + 12, _mm_add_ps(_mm_loadu_ps(m + 12), _mm_loadu_ps(s.m + 12)));
		return res;
	}

	Matrix Matrix::operator-(const Matrix &s) const noexcept
	{
		Matrix res;
		_mm_storeu_ps(res.m +  0, _mm_sub_ps(_mm_loadu_ps(m +  0), _mm_loadu_ps(s.m +  0)));
		_mm_storeu_ps(res.m +  4, _mm_sub_ps(_mm_loadu_ps(m +  4), _mm_loadu_ps(s.m +  4)));
		_mm_storeu_ps(res.m +  8, _mm_sub_ps(_mm_loadu_ps(m +  8), _mm_loadu_ps(s.m +  8)));
		_mm_storeu_ps(res.m + 12, _mm_sub_ps(_mm_loadu_ps(m + 12), _mm_loadu_ps(s.m + 12)));
		return res;
	}

	Matrix Matrix::operator*(const float s) const noexcept
	{
		Matrix res;

		__m128 scalar = _mm_load_ps1(&s);
		_mm_storeu_ps(res.m +  0, _mm_mul_ps(_mm_loadu_ps(m +  0), scalar));
		_mm_storeu_ps(res.m +  4, _mm_mul_ps(_mm_loadu_ps(m +  4), scalar));
		_mm_storeu_ps(res.m +  8, _mm_mul_ps(_mm_loadu_ps(m +  8), scalar));
		_mm_storeu_ps(res.m + 12, _mm_mul_ps(_mm_loadu_ps(m + 12), scalar));

		return res;
	}

	void Matrix::transform(float vec[3]) const noexcept
	{
		float vecX = vec[0];
		float vecY = vec[1];
		float vecZ = vec[2];

		vec[0] = vecX*m[0] + vecY*m[4] + vecZ*m[8] + m[12];
		vec[1] = vecX*m[1] + vecY*m[5] + vecZ*m[9] + m[13];
		vec[2] = vecX*m[2] + vecY*m[6] + vecZ*m[10] + m[14];
	}

	void Matrix::transform(Vector3f& vec) const noexcept
	{
		__m128 row1, row2, row3;

		row1 = _mm_mul_ps(_mm_load_ps1(&vec[0]), _mm_loadu_ps(m));
		row2 = _mm_mul_ps(_mm_load_ps1(&vec[1]), _mm_loadu_ps(m + 4));
		row3 = _mm_mul_ps(_mm_load_ps1(&vec[2]), _mm_loadu_ps(m + 8));
		_mm_storeu_ps(vec.data(), _mm_add_ps(_mm_add_ps(row1, row2), _mm_add_ps(row3, _mm_loadu_ps(m + 12))));
	}

	void Matrix::transform(const Vector3f& vec, Vector3f& result) const noexcept
	{
		__m128 row1, row2, row3;

		row1 = _mm_mul_ps(_mm_load_ps1(&vec[0]), _mm_loadu_ps(m));
		row2 = _mm_mul_ps(_mm_load_ps1(&vec[1]), _mm_loadu_ps(m + 4));
		row3 = _mm_mul_ps(_mm_load_ps1(&vec[2]), _mm_loadu_ps(m + 8));
		_mm_storeu_ps(result.data(), _mm_add_ps(_mm_add_ps(row1, row2), _mm_add_ps(row3, _mm_loadu_ps(m + 12))));
	}

	void Matrix::transform(Vector3f* const vec, size_t numVec) const noexcept
	{
		asmMat4x4Vec3((float*)vec, (float*)vec, 1.0f, sizeof(Vector3f), m, numVec);
	}

	void Matrix::transform(Vector4f& vec) const noexcept
	{
		__m128 row1, row2, row3, row4;

		row1 = _mm_mul_ps(_mm_load_ps1(&vec[0]), _mm_loadu_ps(m));
		row2 = _mm_mul_ps(_mm_load_ps1(&vec[1]), _mm_loadu_ps(m + 4));
		row3 = _mm_mul_ps(_mm_load_ps1(&vec[2]), _mm_loadu_ps(m + 8));
		row4 = _mm_mul_ps(_mm_load_ps1(&vec[3]), _mm_loadu_ps(m + 12));
		_mm_storeu_ps(vec.data(), _mm_add_ps(_mm_add_ps(row1, row2), _mm_add_ps(row3, row4)));
	}

	void Matrix::transform(const Vector4f& vec, Vector4f& result) const noexcept
	{
		__m128 row1, row2, row3, row4;

		row1 = _mm_mul_ps(_mm_load_ps1(&vec[0]), _mm_loadu_ps(m));
		row2 = _mm_mul_ps(_mm_load_ps1(&vec[1]), _mm_loadu_ps(m + 4));
		row3 = _mm_mul_ps(_mm_load_ps1(&vec[2]), _mm_loadu_ps(m + 8));
		row4 = _mm_mul_ps(_mm_load_ps1(&vec[3]), _mm_loadu_ps(m + 12));
		_mm_storeu_ps(result.data(), _mm_add_ps(_mm_add_ps(row1, row2), _mm_add_ps(row3, row4)));
	}

	void Matrix::transform(Vector4f* const vec, size_t numVec) const noexcept
	{
		asmMat4x4Vec4((float*)vec, (float*)vec, sizeof(Vector4f), m, numVec);
	}

	void Matrix::transform(BBox<>& bbox) const noexcept
	{
		Vector3f pts[8];

		bbox.corners(pts);
		transform(pts, 8);

		bbox.reset();
		bbox.merge(pts, 8);
	}

	void Matrix::transform(const BBox<>& bbox, BBox<>& bboxDest) const noexcept
	{
		Vector3f pts[8];

		bbox.corners(pts);
		transform(pts, 8);

		bboxDest.reset();
		bboxDest.merge(pts, 8);
	}

	Vector4f Matrix::getColumn(size_t columnIndex) const noexcept
	{
		columnIndex = columnIndex % 4;
		return Vector4f(m[columnIndex], m[columnIndex + 4], m[columnIndex + 8], m[columnIndex + 12]);
	}

	Vector4f Matrix::getRow(size_t rowIndex) const noexcept
	{
		return Vector4f(m + ((rowIndex % 4) * 4));
	}

	Matrix3 Matrix::getMat3x3() const noexcept
	{
		Matrix3 mat;
		getMat3x3(mat);
		return mat;
	}

	void Matrix::getMat3x3(Matrix3 &mat3) const noexcept
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

	void Matrix::getMat3x3(float dest[9]) const noexcept
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

	void Matrix::getMat2x2(float dest[4]) const noexcept
	{
		dest[0] = m[0];
		dest[1] = m[1];
		dest[2] = m[4];
		dest[3] = m[5];
	}

	void Matrix::write(float dest[16]) const noexcept
	{
		std::memcpy(dest, m, sizeof(float) * 16);
	}

	Matrix Matrix::getTranspose() const noexcept
	{
		Matrix result;
		fastMat4x4Transpose(result.m, m);
		return result;
	}

	Matrix& Matrix::transpose() noexcept
	{
		fastMat4x4Transpose(m, m);
		return *this;
	}

	Matrix Matrix::getInverse() const noexcept
	{
		Matrix result(*this);
		result.inverse();
		return result;
	}

	Matrix& Matrix::inverse() noexcept
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
			return *this;

		//multiplicar tudo pelo determinante
		det = 1.0f / det;
		result[0] *= det;		result[1] *= det;		result[2] *= det;		result[3] *= det;
		result[4] *= det;		result[5] *= det;		result[6] *= det;		result[7] *= det;
		result[8] *= det;		result[9] *= det;		result[10] *= det;	result[11] *= det;
		result[12] *= det;	result[13] *= det;	result[14] *= det;	result[15] *= det;

		std::memcpy(m, result, sizeof(float) * 16);
		return *this;
	}

	Matrix Matrix::getInverseTranspose() const noexcept
	{
		Matrix result(*this);
		result.inverseTranspose();
		return result;
	}

	Matrix& Matrix::inverseTranspose() noexcept
	{
		inverse();
		transpose();
		return *this;
	}

	Matrix Matrix::getInverseHomogenous() const noexcept
	{
		Matrix result(*this);
		result.inverseHomogenous();
		return result;
	}

	Matrix& Matrix::inverseHomogenous() noexcept
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

		return *this;
	}

	Vector3f Matrix::extractTranslation() const noexcept
	{
		return Vector3f{ m[12], m[13], m[14] };
	}

	Vector3f Matrix::extractScale() const noexcept
	{
		Vector3f scaling;
		scaling[0] = m[15] * std::sqrt((m[0] * m[0]) + (m[1] * m[1]) + (m[2] * m[2]));
		scaling[1] = m[15] * std::sqrt((m[4] * m[4]) + (m[5] * m[5]) + (m[6] * m[6]));
		scaling[2] = m[15] * std::sqrt((m[8] * m[8]) + (m[9] * m[9]) + (m[10] * m[10]));

		return scaling;
	}

	Quaternion Matrix::extractRotation() const noexcept
	{
		Matrix tmp{ *this };

		//remove scaling
		{
			auto scaling = extractScale();

			if (!Math::isZero(scaling[0]))
			{
				tmp[0] /= scaling[0];
				tmp[1] /= scaling[0];
				tmp[2] /= scaling[0];
			}
			if (!Math::isZero(scaling[1]))
			{
				tmp[4] /= scaling[1];
				tmp[5] /= scaling[1];
				tmp[6] /= scaling[1];
			}
			if (!Math::isZero(scaling[2]))
			{
				tmp[8] /= scaling[2];
				tmp[9] /= scaling[2];
				tmp[10] /= scaling[2];
			}
		}

		Quaternion quat; 
		quat.setFromMatrix4x4(tmp.data());
		
		return quat;
	}

	void Matrix::set(float value) noexcept
	{
		auto scalar = _mm_load_ps1(&value);
		_mm_storeu_ps(m + 0, scalar);
		_mm_storeu_ps(m + 4, scalar);
		_mm_storeu_ps(m + 8, scalar);
		_mm_storeu_ps(m + 12, scalar);
	}

	void Matrix::set(const float src[16]) noexcept
	{
		std::memcpy(m, src, sizeof(float) * 16);
	}

	void Matrix::set(const Matrix &mat) noexcept
	{
		std::memcpy(m, mat.m, sizeof(float) * 16);
	}

	void Matrix::setIdentity(void) noexcept
	{
		std::memset(m, 0, sizeof(float) * 16);
		m[0] = m[5] = m[10] = m[15] = 1.0f;
	}

	void Matrix::setFrom3x3(const Matrix3 &mat3) noexcept
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

	void Matrix::setFrom3x3(const float src[9]) noexcept
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

	void Matrix::setFrom2x2(const float src[4]) noexcept
	{
		m[0] = src[0];
		m[1] = src[1];
		m[4] = src[2];
		m[5] = src[3];

		m[2] = m[3] = m[6] = m[7] = m[8] = m[9] = m[11] = m[12] = m[13] = m[14] = 0.0f;
		m[10] = m[15] = 1.0f;
	}

	void Matrix::setTranspose(const float src[16]) noexcept
	{
		fastMat4x4Transpose(m, src);
	}

	void Matrix::setTranspose(const Matrix &mat) noexcept
	{
		fastMat4x4Transpose(m, mat.m);
	}

	void Matrix::setTranslation(float x, float y, float z) noexcept
	{
		Matrix::genMatTranslate(*this, x, y, z);
	}

	void Matrix::setTranslation(const float vec[3]) noexcept
	{
		Matrix::genMatTranslate(*this, vec[0], vec[1], vec[2]);
	}

	void Matrix::setTranslation(const Vector3f& vec) noexcept
	{
		Matrix::genMatTranslate(*this, vec[0], vec[1], vec[2]);
	}

	void Matrix::setScale(float scale) noexcept
	{
		Matrix::genMatScale(*this, scale, scale, scale);
	}

	void Matrix::setScale(float x, float y, float z) noexcept
	{
		Matrix::genMatScale(*this, x, y, z);
	}

	void Matrix::setScale(const Vector3f& vec) noexcept
	{
		Matrix::genMatScale(*this, vec[0], vec[1], vec[2]);
	}

	void Matrix::setReflect(const Plane<float> &plane) noexcept
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

	void Matrix::setRotationX(const float angleDeg) noexcept
	{
		float c, s;

		Math::sinCos(Math::Deg2Rad<float> * angleDeg, s, c);

		m[5] = m[10] = c;
		m[6] = s;
		m[9] = -s;

		m[1] = m[2] = m[3] = m[4] = m[7] = m[8] = m[11] = m[12] = m[13] = m[14] = 0.0f;
		m[0] = m[15] = 1.0f;
	}

	void Matrix::setRotationY(const float angleDeg) noexcept
	{
		float c, s;

		Math::sinCos(Math::Deg2Rad<float> * angleDeg, s, c);

		m[0] = m[10] = c;
		m[2] = -s;
		m[8] = s;

		m[1] = m[3] = m[4] = m[6] = m[7] = m[9] = m[11] = m[12] = m[13] = m[14] = 0.0f;
		m[5] = m[15] = 1.0f;
	}

	void Matrix::setRotationZ(const float angleDeg) noexcept
	{
		float c, s;

		Math::sinCos(Math::Deg2Rad<float> * angleDeg, s, c);

		m[0] = m[5] = c;
		m[1] = s;
		m[4] = -s;

		m[2] = m[3] = m[6] = m[7] = m[8] = m[9] = m[11] = m[12] = m[13] = m[14] = 0.0f;
		m[10] = m[15] = 1.0f;
	}

	void Matrix::setRotation(const Vector3f& unitVec, const float angleDeg) noexcept
	{
		float c, s;
		Math::sinCos(Math::Deg2Rad<float> * angleDeg, s, c);

		auto t = 1.0f - c;
		auto txy = unitVec[0] * unitVec[1] * t;
		auto txz = unitVec[0] * unitVec[2] * t;
		auto tyz = unitVec[1] * unitVec[2] * t;
		auto sx = s * unitVec[0];
		auto sy = s * unitVec[1];
		auto sz = s * unitVec[2];

		m[0] = (unitVec[0] * unitVec[0] * t) + c;
		m[1] = txy + sz;
		m[2] = txz - sy;

		m[4] = txy - sz;
		m[5] = (unitVec[1] * unitVec[1] * t) + c;
		m[6] = tyz + sx;

		m[8] = txz + sy;
		m[9] = tyz - sx;
		m[10] = (unitVec[2] * unitVec[2] * t) + c;

		m[3] = m[7] = m[11] = m[12] = m[13] = m[14] = 0.0f;
		m[15] = 1.0f;
	}

	void Matrix::setSaturation(const float sat) noexcept
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

	void Matrix::setGLModelView(const Vector3f& pos, const Vector3f& target, const Vector3f& up) noexcept
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

	void Matrix::setGLModelView(const Vector3f& pos, const Vector3f& target) noexcept
	{
		Vector3f up(0.0f, 1.0f, 0.0f);
		setGLModelView(pos, target, up);
	}

	void Matrix::setGLModelView(const Vector3f& pos, float angleDegX, float angleDegY, const Vector3f& up) noexcept
	{
		Vector3f target, x, y, z;
		float sx, sy, cx, cy;

		Math::sinCos(Math::Deg2Rad<float> * angleDegX, sx, cx);
		Math::sinCos(Math::Deg2Rad<float> * angleDegY, sy, cy);

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

	void Matrix::setGLModelView(int cubemapFace, const Vector3f& centerCube) noexcept
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

	Matrix3::Matrix3(const Quaternion &unitQuaternion) noexcept
		: m{ 0.0f }
	{
		m[0] = 1.0f - 2.0f * (unitQuaternion[1] * unitQuaternion[1] + unitQuaternion[2] * unitQuaternion[2]);
		m[3] =        2.0f * (unitQuaternion[0] * unitQuaternion[1] - unitQuaternion[2] * unitQuaternion[3]);
		m[6] =        2.0f * (unitQuaternion[0] * unitQuaternion[2] + unitQuaternion[1] * unitQuaternion[3]);

		m[1] =        2.0f * (unitQuaternion[0] * unitQuaternion[1] + unitQuaternion[2] * unitQuaternion[3]);
		m[4] = 1.0f - 2.0f * (unitQuaternion[0] * unitQuaternion[0] + unitQuaternion[2] * unitQuaternion[2]);
		m[7] =        2.0f * (unitQuaternion[1] * unitQuaternion[2] - unitQuaternion[0] * unitQuaternion[3]);

		m[2] =        2.0f * (unitQuaternion[0] * unitQuaternion[2] - unitQuaternion[1] * unitQuaternion[3]);
		m[5] =        2.0f * (unitQuaternion[1] * unitQuaternion[2] + unitQuaternion[0] * unitQuaternion[3]);
		m[8] = 1.0f - 2.0f * (unitQuaternion[0] * unitQuaternion[0] + unitQuaternion[1] * unitQuaternion[1]);
	}

	Matrix3& Matrix3::operator=(const Matrix& mat) noexcept
	{
		m[0] = mat.m[0];
		m[1] = mat.m[1];
		m[2] = mat.m[2];
		m[3] = mat.m[4];
		m[4] = mat.m[5];
		m[5] = mat.m[6];
		m[6] = mat.m[8];
		m[7] = mat.m[9];
		m[8] = mat.m[10];
		return *this;
	}

	void Matrix3::operator*=(const Matrix &s) noexcept
	{
		float matAux[9];
		std::memcpy(matAux, m, sizeof(float) * 9);

		m[0] = (matAux[0] * s.m[0]) + (matAux[1] * s.m[4]) + (matAux[2] * s.m[8]);
		m[1] = (matAux[0] * s.m[1]) + (matAux[1] * s.m[5]) + (matAux[2] * s.m[9]);
		m[2] = (matAux[0] * s.m[2]) + (matAux[1] * s.m[6]) + (matAux[2] * s.m[10]);

		m[3] = (matAux[3] * s.m[0]) + (matAux[4] * s.m[4]) + (matAux[5] * s.m[8]);
		m[4] = (matAux[3] * s.m[1]) + (matAux[4] * s.m[5]) + (matAux[5] * s.m[9]);
		m[5] = (matAux[3] * s.m[2]) + (matAux[4] * s.m[6]) + (matAux[5] * s.m[10]);

		m[6] = (matAux[6] * s.m[0]) + (matAux[7] * s.m[4]) + (matAux[8] * s.m[8]);
		m[7] = (matAux[6] * s.m[1]) + (matAux[7] * s.m[5]) + (matAux[8] * s.m[9]);
		m[8] = (matAux[6] * s.m[2]) + (matAux[7] * s.m[6]) + (matAux[8] * s.m[10]);
	}

	void Matrix3::operator*=(const Matrix3 &s) noexcept
	{
		this->operator*=(s.m);
	}
	
	void Matrix3::operator*=(const float src[9]) noexcept
	{
		float matAux[9];
		std::memcpy(matAux, m, sizeof(float) * 9);

		m[0] = (matAux[0] * src[0]) + (matAux[1] * src[3]) + (matAux[2] * src[6]);
		m[1] = (matAux[0] * src[1]) + (matAux[1] * src[4]) + (matAux[2] * src[7]);
		m[2] = (matAux[0] * src[2]) + (matAux[1] * src[5]) + (matAux[2] * src[8]);

		m[3] = (matAux[3] * src[0]) + (matAux[4] * src[3]) + (matAux[5] * src[6]);
		m[4] = (matAux[3] * src[1]) + (matAux[4] * src[4]) + (matAux[5] * src[7]);
		m[5] = (matAux[3] * src[2]) + (matAux[4] * src[5]) + (matAux[5] * src[8]);

		m[6] = (matAux[6] * src[0]) + (matAux[7] * src[3]) + (matAux[8] * src[6]);
		m[7] = (matAux[6] * src[1]) + (matAux[7] * src[4]) + (matAux[8] * src[7]);
		m[8] = (matAux[6] * src[2]) + (matAux[7] * src[5]) + (matAux[8] * src[8]);
	}

	void Matrix3::operator+=(const Matrix3 &s) noexcept
	{
		this->operator+=(s.m);
	}

	void Matrix3::operator+=(const float src[9]) noexcept
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

	void Matrix3::operator-=(const Matrix3 &s) noexcept
	{
		this->operator-=(s.m);
	}

	void Matrix3::operator-=(const float src[9]) noexcept
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

	Matrix3 Matrix3::operator*(const Matrix3 &s) const noexcept
	{
		Matrix3 res(*this);
		res *= s;
		return res;
	}

	Matrix3 Matrix3::operator+(const Matrix3 &s) const noexcept
	{
		Matrix3 res(*this);
		res += s;
		return res;
	}

	Matrix3 Matrix3::operator-(const Matrix3 &s) const noexcept
	{
		Matrix3 res(*this);
		res -= s;
		return res;
	}

	void Matrix3::transform(float vec[3]) const noexcept
	{
		float vecX = vec[0];
		float vecY = vec[1];
		float vecZ = vec[2];

		vec[0] = vecX*m[0] + vecY*m[3] + vecZ*m[6];
		vec[1] = vecX*m[1] + vecY*m[4] + vecZ*m[7];
		vec[2] = vecX*m[2] + vecY*m[5] + vecZ*m[8];
	}

	void Matrix3::transform(Vector3f& vec) const noexcept
	{
		float result[3];

		result[0] = vec[0]*m[0] + vec[1]*m[3] + vec[2]*m[6];
		result[1] = vec[0]*m[1] + vec[1]*m[4] + vec[2]*m[7];
		result[2] = vec[0]*m[2] + vec[1]*m[5] + vec[2]*m[8];

		vec[0] = result[0];
		vec[1] = result[1];
		vec[2] = result[2];
	}

	void Matrix3::transform(const Vector3f& vec, Vector3f& result) const noexcept
	{
		result[0] = vec[0]*m[0] + vec[1]*m[3] + vec[2]*m[6];
		result[1] = vec[0]*m[1] + vec[1]*m[4] + vec[2]*m[7];
		result[2] = vec[0]*m[2] + vec[1]*m[5] + vec[2]*m[8];
	}

	void Matrix3::transform(Vector3f* const vec, size_t numVec) const noexcept
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

	Vector3f Matrix3::getColumn(size_t columnIndex) const noexcept
	{
		columnIndex = columnIndex % 3;
		return Vector3f(m[columnIndex], m[columnIndex + 3], m[columnIndex + 6]);
	}

	Vector3f Matrix3::getRow(size_t rowIndex) const noexcept
	{
		return Vector3f(m + ((rowIndex % 3) * 3));
	}

	void Matrix3::write(float dest[9]) const noexcept
	{
		std::memcpy(dest, m, sizeof(float) * 9);
	}

	Matrix3 Matrix3::getTranspose() const noexcept
	{
		Matrix3 newMat;

		newMat.m[1] = m[3];
		newMat.m[2] = m[6];
		newMat.m[3] = m[1];
		newMat.m[5] = m[7];
		newMat.m[6] = m[2];
		newMat.m[7] = m[5];

		newMat.m[0] = m[0];
		newMat.m[4] = m[4];
		newMat.m[8] = m[8];

		return newMat;
	}

	Matrix3& Matrix3::transpose() noexcept
	{
		std::swap(m[1], m[3]);
		std::swap(m[2], m[6]);
		std::swap(m[5], m[7]);

		return *this;
	}

	Matrix3& Matrix3::set(float value) noexcept
	{
		m[0] = m[1] = m[2] = value;
		m[3] = m[4] = m[5] = value;
		m[6] = m[7] = m[8] = value;

		return *this;
	}

	Matrix3& Matrix3::set(const float src[9]) noexcept
	{
		std::memcpy(m, src, sizeof(float) * 9);
		return *this;
	}

	Matrix3& Matrix3::set(const Matrix &mat) noexcept
	{
		m[0] = mat.m[0];		m[1] = mat.m[1];		m[2] = mat.m[2];
		m[3] = mat.m[4];		m[4] = mat.m[5];		m[5] = mat.m[6];
		m[6] = mat.m[8];		m[7] = mat.m[9];		m[8] = mat.m[10];

		return *this;
	}

	Matrix3& Matrix3::set(const Matrix3 &mat) noexcept
	{
		std::memcpy(m, mat.m, sizeof(float) * 9);
		return *this;
	}

	Matrix3& Matrix3::setZero() noexcept
	{
		std::memset(m, 0, sizeof(float) * 9);
		return *this;
	}

	Matrix3& Matrix3::setIdentity() noexcept
	{
		std::memset(m, 0, sizeof(float) * 9);
		m[0] = m[4] = m[8] = 1.0f;
		return *this;
	}

	Matrix3& Matrix3::setRotationX(const float angleDeg) noexcept
	{
		Matrix mat;
		mat.setRotationX(angleDeg);

		*this = mat;
		return *this;
	}

	Matrix3& Matrix3::setRotationY(const float angleDeg) noexcept
	{
		Matrix mat;
		mat.setRotationY(angleDeg);

		*this = mat;
		return *this;
	}

	Matrix3& Matrix3::setRotationZ(const float angleDeg) noexcept
	{
		Matrix mat;
		mat.setRotationZ(angleDeg);

		*this = mat;
		return *this;
	}

	Matrix3& Matrix3::setRotation(const Vector3f& unitVec, const float angleDeg) noexcept
	{
		Matrix mat;
		mat.setRotation(unitVec, angleDeg);

		*this = mat;
		return *this;
	}
}