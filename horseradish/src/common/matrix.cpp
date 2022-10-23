#include "matrix.hpp"

#include "math.hpp"

#include <array>
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
			__m128 mat1 = _mm_load_ps(mat + 0);
			__m128 mat2 = _mm_load_ps(mat + 4);
			__m128 mat3 = _mm_load_ps(mat + 8);
			__m128 mat4 = _mm_mul_ps(_mm_load_ps(mat + 12), _mm_load_ps1(&wCompMul));
			__m128 final;

			auto leftOver = numVec;
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

		void asmMat4x4Vec3(double* vecWrite, const double* vecRead, double wCompMul, size_t stride, const double* mat, size_t numVec)
		{
			__m256d mat1 = _mm256_load_pd(mat + 0);
			__m256d mat2 = _mm256_load_pd(mat + 4);
			__m256d mat3 = _mm256_load_pd(mat + 8);
			__m256d mat4 = _mm256_mul_pd(_mm256_load_pd(mat + 12), _mm256_broadcast_sd(&wCompMul));
			__m256d final;
			double alignas(32) res[4];

			auto leftOver = numVec;
			for (; leftOver >= 4; leftOver -= 4)
			{
				final = _mm256_mul_pd(_mm256_broadcast_sd(vecRead + 0), mat1);
				final = _mm256_add_pd(final, _mm256_mul_pd(_mm256_broadcast_sd(vecRead + 1), mat2));
				final = _mm256_add_pd(final, _mm256_mul_pd(_mm256_broadcast_sd(vecRead + 2), mat3));
				final = _mm256_add_pd(final, mat4);
				_mm256_storeu_pd(res, final);
				vecWrite[0] = res[0];
				vecWrite[1] = res[1];
				vecWrite[2] = res[2];

				vecRead = (double*)(((unsigned char*)vecRead) + stride);
				vecWrite = (double*)(((unsigned char*)vecWrite) + stride);

				final = _mm256_mul_pd(_mm256_broadcast_sd(vecRead + 0), mat1);
				final = _mm256_add_pd(final, _mm256_mul_pd(_mm256_broadcast_sd(vecRead + 1), mat2));
				final = _mm256_add_pd(final, _mm256_mul_pd(_mm256_broadcast_sd(vecRead + 2), mat3));
				final = _mm256_add_pd(final, mat4);
				_mm256_storeu_pd(res, final);
				vecWrite[0] = res[0];
				vecWrite[1] = res[1];
				vecWrite[2] = res[2];

				vecRead = (double*)(((unsigned char*)vecRead) + stride);
				vecWrite = (double*)(((unsigned char*)vecWrite) + stride);

				final = _mm256_mul_pd(_mm256_broadcast_sd(vecRead + 0), mat1);
				final = _mm256_add_pd(final, _mm256_mul_pd(_mm256_broadcast_sd(vecRead + 1), mat2));
				final = _mm256_add_pd(final, _mm256_mul_pd(_mm256_broadcast_sd(vecRead + 2), mat3));
				final = _mm256_add_pd(final, mat4);
				_mm256_storeu_pd(res, final);
				vecWrite[0] = res[0];
				vecWrite[1] = res[1];
				vecWrite[2] = res[2];

				vecRead = (double*)(((unsigned char*)vecRead) + stride);
				vecWrite = (double*)(((unsigned char*)vecWrite) + stride);

				final = _mm256_mul_pd(_mm256_broadcast_sd(vecRead + 0), mat1);
				final = _mm256_add_pd(final, _mm256_mul_pd(_mm256_broadcast_sd(vecRead + 1), mat2));
				final = _mm256_add_pd(final, _mm256_mul_pd(_mm256_broadcast_sd(vecRead + 2), mat3));
				final = _mm256_add_pd(final, mat4);
				_mm256_storeu_pd(res, final);
				vecWrite[0] = res[0];
				vecWrite[1] = res[1];
				vecWrite[2] = res[2];

				vecRead = (double*)(((unsigned char*)vecRead) + stride);
				vecWrite = (double*)(((unsigned char*)vecWrite) + stride);
			}

			for (; leftOver > 0; leftOver--)
			{
				final = _mm256_mul_pd(_mm256_broadcast_sd(vecRead + 0), mat1);
				final = _mm256_add_pd(final, _mm256_mul_pd(_mm256_broadcast_sd(vecRead + 1), mat2));
				final = _mm256_add_pd(final, _mm256_mul_pd(_mm256_broadcast_sd(vecRead + 2), mat3));
				final = _mm256_add_pd(final, mat4);
				_mm256_storeu_pd(res, final);
				vecWrite[0] = res[0];
				vecWrite[1] = res[1];
				vecWrite[2] = res[2];

				vecRead = (double*)(((unsigned char*)vecRead) + stride);
				vecWrite = (double*)(((unsigned char*)vecWrite) + stride);
			}
		}

		void asmMat4x4Vec4(float *vecWrite, const float *vecRead, size_t stride, const float *mat, size_t numVec)
		{
			__m128 mat1 = _mm_load_ps(mat + 0);
			__m128 mat2 = _mm_load_ps(mat + 4);
			__m128 mat3 = _mm_load_ps(mat + 8);
			__m128 mat4 = _mm_load_ps(mat + 12);
			__m128 final, curVec;

			auto leftOver = numVec;

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

				for (; leftOver > 0; leftOver--)
				{
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
		}

		void asmMat4x4Vec4(double* vecWrite, const double* vecRead, size_t stride, const double* mat, size_t numVec)
		{
			__m256d mat1 = _mm256_load_pd(mat + 0);
			__m256d mat2 = _mm256_load_pd(mat + 4);
			__m256d mat3 = _mm256_load_pd(mat + 8);
			__m256d mat4 = _mm256_load_pd(mat + 12);
			__m256d final, curVec;

			auto leftOver = numVec;

			if ((reinterpret_cast<uintptr_t>(vecWrite) % 32 == 0) && (reinterpret_cast<uintptr_t>(vecRead) % 32 == 0) && (stride % 32 == 0))
			{
				for (; leftOver >= 4; leftOver -= 4)
				{
					curVec = _mm256_load_pd(vecRead);
					final = _mm256_mul_pd(_mm256_shuffle_pd(curVec, curVec, 0x00), mat1);
					final = _mm256_add_pd(final, _mm256_mul_pd(_mm256_shuffle_pd(curVec, curVec, 0x55), mat2));
					final = _mm256_add_pd(final, _mm256_mul_pd(_mm256_shuffle_pd(curVec, curVec, 0xAA), mat3));
					final = _mm256_add_pd(final, _mm256_mul_pd(_mm256_shuffle_pd(curVec, curVec, 0xFF), mat4));
					_mm256_store_pd(vecWrite, final);

					vecRead = (double*)(((unsigned char*)vecRead) + stride);
					vecWrite = (double*)(((unsigned char*)vecWrite) + stride);

					curVec = _mm256_load_pd(vecRead);
					final = _mm256_mul_pd(_mm256_shuffle_pd(curVec, curVec, 0x00), mat1);
					final = _mm256_add_pd(final, _mm256_mul_pd(_mm256_shuffle_pd(curVec, curVec, 0x55), mat2));
					final = _mm256_add_pd(final, _mm256_mul_pd(_mm256_shuffle_pd(curVec, curVec, 0xAA), mat3));
					final = _mm256_add_pd(final, _mm256_mul_pd(_mm256_shuffle_pd(curVec, curVec, 0xFF), mat4));
					_mm256_store_pd(vecWrite, final);

					vecRead = (double*)(((unsigned char*)vecRead) + stride);
					vecWrite = (double*)(((unsigned char*)vecWrite) + stride);

					curVec = _mm256_load_pd(vecRead);
					final = _mm256_mul_pd(_mm256_shuffle_pd(curVec, curVec, 0x00), mat1);
					final = _mm256_add_pd(final, _mm256_mul_pd(_mm256_shuffle_pd(curVec, curVec, 0x55), mat2));
					final = _mm256_add_pd(final, _mm256_mul_pd(_mm256_shuffle_pd(curVec, curVec, 0xAA), mat3));
					final = _mm256_add_pd(final, _mm256_mul_pd(_mm256_shuffle_pd(curVec, curVec, 0xFF), mat4));
					_mm256_store_pd(vecWrite, final);

					vecRead = (double*)(((unsigned char*)vecRead) + stride);
					vecWrite = (double*)(((unsigned char*)vecWrite) + stride);

					curVec = _mm256_load_pd(vecRead);
					final = _mm256_mul_pd(_mm256_shuffle_pd(curVec, curVec, 0x00), mat1);
					final = _mm256_add_pd(final, _mm256_mul_pd(_mm256_shuffle_pd(curVec, curVec, 0x55), mat2));
					final = _mm256_add_pd(final, _mm256_mul_pd(_mm256_shuffle_pd(curVec, curVec, 0xAA), mat3));
					final = _mm256_add_pd(final, _mm256_mul_pd(_mm256_shuffle_pd(curVec, curVec, 0xFF), mat4));
					_mm256_store_pd(vecWrite, final);

					vecRead = (double*)(((unsigned char*)vecRead) + stride);
					vecWrite = (double*)(((unsigned char*)vecWrite) + stride);
				}
			}
			else
			{
				for (; leftOver >= 4; leftOver -= 4)
				{
					curVec = _mm256_load_pd(vecRead);
					final = _mm256_mul_pd(_mm256_shuffle_pd(curVec, curVec, 0x00), mat1);
					final = _mm256_add_pd(final, _mm256_mul_pd(_mm256_shuffle_pd(curVec, curVec, 0x55), mat2));
					final = _mm256_add_pd(final, _mm256_mul_pd(_mm256_shuffle_pd(curVec, curVec, 0xAA), mat3));
					final = _mm256_add_pd(final, _mm256_mul_pd(_mm256_shuffle_pd(curVec, curVec, 0xFF), mat4));
					_mm256_store_pd(vecWrite, final);

					vecRead = (double*)(((unsigned char*)vecRead) + stride);
					vecWrite = (double*)(((unsigned char*)vecWrite) + stride);

					curVec = _mm256_load_pd(vecRead);
					final = _mm256_mul_pd(_mm256_shuffle_pd(curVec, curVec, 0x00), mat1);
					final = _mm256_add_pd(final, _mm256_mul_pd(_mm256_shuffle_pd(curVec, curVec, 0x55), mat2));
					final = _mm256_add_pd(final, _mm256_mul_pd(_mm256_shuffle_pd(curVec, curVec, 0xAA), mat3));
					final = _mm256_add_pd(final, _mm256_mul_pd(_mm256_shuffle_pd(curVec, curVec, 0xFF), mat4));
					_mm256_store_pd(vecWrite, final);

					vecRead = (double*)(((unsigned char*)vecRead) + stride);
					vecWrite = (double*)(((unsigned char*)vecWrite) + stride);

					curVec = _mm256_load_pd(vecRead);
					final = _mm256_mul_pd(_mm256_shuffle_pd(curVec, curVec, 0x00), mat1);
					final = _mm256_add_pd(final, _mm256_mul_pd(_mm256_shuffle_pd(curVec, curVec, 0x55), mat2));
					final = _mm256_add_pd(final, _mm256_mul_pd(_mm256_shuffle_pd(curVec, curVec, 0xAA), mat3));
					final = _mm256_add_pd(final, _mm256_mul_pd(_mm256_shuffle_pd(curVec, curVec, 0xFF), mat4));
					_mm256_store_pd(vecWrite, final);

					vecRead = (double*)(((unsigned char*)vecRead) + stride);
					vecWrite = (double*)(((unsigned char*)vecWrite) + stride);

					curVec = _mm256_load_pd(vecRead);
					final = _mm256_mul_pd(_mm256_shuffle_pd(curVec, curVec, 0x00), mat1);
					final = _mm256_add_pd(final, _mm256_mul_pd(_mm256_shuffle_pd(curVec, curVec, 0x55), mat2));
					final = _mm256_add_pd(final, _mm256_mul_pd(_mm256_shuffle_pd(curVec, curVec, 0xAA), mat3));
					final = _mm256_add_pd(final, _mm256_mul_pd(_mm256_shuffle_pd(curVec, curVec, 0xFF), mat4));
					_mm256_store_pd(vecWrite, final);

					vecRead = (double*)(((unsigned char*)vecRead) + stride);
					vecWrite = (double*)(((unsigned char*)vecWrite) + stride);
				}
			}

			for (; leftOver > 0; leftOver--)
			{
				curVec = _mm256_load_pd(vecRead);
				final = _mm256_mul_pd(_mm256_shuffle_pd(curVec, curVec, 0x00), mat1);
				final = _mm256_add_pd(final, _mm256_mul_pd(_mm256_shuffle_pd(curVec, curVec, 0x55), mat2));
				final = _mm256_add_pd(final, _mm256_mul_pd(_mm256_shuffle_pd(curVec, curVec, 0xAA), mat3));
				final = _mm256_add_pd(final, _mm256_mul_pd(_mm256_shuffle_pd(curVec, curVec, 0xFF), mat4));
				_mm256_store_pd(vecWrite, final);

				vecRead = (double*)(((unsigned char*)vecRead) + stride);
				vecWrite = (double*)(((unsigned char*)vecWrite) + stride);
			}
		}
		
		void fastMat4x4Mult(float * const result, const float * const mat1, const float * const mat2)
		{
			_mm256_zeroupper();

			__m256 a0 = _mm256_load_ps(mat1 + 0);
			__m256 a1 = _mm256_load_ps(mat1 + 8);

			__m128 b0 = _mm_load_ps(mat2 + 0);
			__m128 b1 = _mm_load_ps(mat2 + 4);
			__m128 b2 = _mm_load_ps(mat2 + 8);
			__m128 b3 = _mm_load_ps(mat2 + 12);

			__m256 out0 = _mm256_mul_ps(_mm256_shuffle_ps(a0, a0, 0x00), _mm256_broadcast_ps(&b0));
			out0 = _mm256_add_ps(out0, _mm256_mul_ps(_mm256_shuffle_ps(a0, a0, 0x55), _mm256_broadcast_ps(&b1)));
			out0 = _mm256_add_ps(out0, _mm256_mul_ps(_mm256_shuffle_ps(a0, a0, 0xaa), _mm256_broadcast_ps(&b2)));
			out0 = _mm256_add_ps(out0, _mm256_mul_ps(_mm256_shuffle_ps(a0, a0, 0xff), _mm256_broadcast_ps(&b3)));

			__m256 out1 = _mm256_mul_ps(_mm256_shuffle_ps(a1, a1, 0x00), _mm256_broadcast_ps(&b0));
			out1 = _mm256_add_ps(out1, _mm256_mul_ps(_mm256_shuffle_ps(a1, a1, 0x55), _mm256_broadcast_ps(&b1)));
			out1 = _mm256_add_ps(out1, _mm256_mul_ps(_mm256_shuffle_ps(a1, a1, 0xaa), _mm256_broadcast_ps(&b2)));
			out1 = _mm256_add_ps(out1, _mm256_mul_ps(_mm256_shuffle_ps(a1, a1, 0xff), _mm256_broadcast_ps(&b3)));

			_mm256_store_ps(result + 0, out0);
			_mm256_store_ps(result + 8, out1);
		}

		void fastMat4x4Mult(double* const result, const double* const mat1, const double* const mat2)
		{
			__m256d ymm[4];

			__m256d mat2Row0 = _mm256_load_pd(mat2 + 0);
			__m256d mat2Row1 = _mm256_load_pd(mat2 + 4);
			__m256d mat2Row2 = _mm256_load_pd(mat2 + 8);
			__m256d mat2Row3 = _mm256_load_pd(mat2 + 12);

			for (int i = 0; i < 4; i++)
			{
				ymm[0] = _mm256_broadcast_sd(mat1 + (i * 4) + 0);
				ymm[1] = _mm256_broadcast_sd(mat1 + (i * 4) + 1);
				ymm[2] = _mm256_broadcast_sd(mat1 + (i * 4) + 2);
				ymm[3] = _mm256_broadcast_sd(mat1 + (i * 4) + 3);

				ymm[0] = _mm256_mul_pd(ymm[0], mat2Row0);
				ymm[1] = _mm256_mul_pd(ymm[1], mat2Row1);
				ymm[0] = _mm256_add_pd(ymm[0], ymm[1]);
				ymm[2] = _mm256_mul_pd(ymm[2], mat2Row2);
				ymm[3] = _mm256_mul_pd(ymm[3], mat2Row3);
				ymm[2] = _mm256_add_pd(ymm[2], ymm[3]);
				_mm256_store_pd(result + (i * 4), _mm256_add_pd(ymm[0], ymm[2]));
			}
		}

		void fastMat4x4Transpose(float * const result, const float * const mat)
		{
			__m128 row1 = _mm_load_ps(mat + 0);
			__m128 row2 = _mm_load_ps(mat + 4);
			__m128 row3 = _mm_load_ps(mat + 8);
			__m128 row4 = _mm_load_ps(mat + 12);
			_MM_TRANSPOSE4_PS(row1, row2, row3, row4);
			_mm_store_ps(result + 0, row1);
			_mm_store_ps(result + 4, row2);
			_mm_store_ps(result + 8, row3);
			_mm_store_ps(result + 12, row4);
		}

		void fastMat4x4Transpose(double* const result, const double* const mat)
		{
			__m256d row0 = _mm256_load_pd(mat + 0);
			__m256d row1 = _mm256_load_pd(mat + 4);
			__m256d row2 = _mm256_load_pd(mat + 8);
			__m256d row3 = _mm256_load_pd(mat + 12);

			{
				__m256d tmp0 = _mm256_shuffle_pd(row0, row1, 0x0);
				__m256d tmp2 = _mm256_shuffle_pd(row0, row1, 0xF);
				__m256d tmp1 = _mm256_shuffle_pd(row2, row3, 0x0);
				__m256d tmp3 = _mm256_shuffle_pd(row2, row3, 0xF);

				row0 = _mm256_permute2f128_pd(tmp0, tmp1, 0x20);
				row1 = _mm256_permute2f128_pd(tmp2, tmp3, 0x20);
				row2 = _mm256_permute2f128_pd(tmp0, tmp1, 0x31);
				row3 = _mm256_permute2f128_pd(tmp2, tmp3, 0x31);
			}

			_mm256_store_pd(result + 0, row0);
			_mm256_store_pd(result + 4, row1);
			_mm256_store_pd(result + 8, row2);
			_mm256_store_pd(result + 12, row3);
		}

		template<typename T>
		void mat3x3Mult(std::span<T> m1, std::span<const T> m2) noexcept
		{
			static_assert(std::is_same_v<T, float> || std::is_same_v<T, double>, "Type must be either float or double");

			T matAux[9];
			std::memcpy(matAux, m1.data(), sizeof(T) * 9);

			m1[0] = (matAux[0] * m2[0]) + (matAux[1] * m2[3]) + (matAux[2] * m2[6]);
			m1[1] = (matAux[0] * m2[1]) + (matAux[1] * m2[4]) + (matAux[2] * m2[7]);
			m1[2] = (matAux[0] * m2[2]) + (matAux[1] * m2[5]) + (matAux[2] * m2[8]);

			m1[3] = (matAux[3] * m2[0]) + (matAux[4] * m2[3]) + (matAux[5] * m2[6]);
			m1[4] = (matAux[3] * m2[1]) + (matAux[4] * m2[4]) + (matAux[5] * m2[7]);
			m1[5] = (matAux[3] * m2[2]) + (matAux[4] * m2[5]) + (matAux[5] * m2[8]);

			m1[6] = (matAux[6] * m2[0]) + (matAux[7] * m2[3]) + (matAux[8] * m2[6]);
			m1[7] = (matAux[6] * m2[1]) + (matAux[7] * m2[4]) + (matAux[8] * m2[7]);
			m1[8] = (matAux[6] * m2[2]) + (matAux[7] * m2[5]) + (matAux[8] * m2[8]);
		}

		template<typename T>
		void mat3x3Mult(std::span<T> result, std::span<const T> m1, std::span<const T> m2) noexcept
		{
			static_assert(std::is_same_v<T, float> || std::is_same_v<T, double>, "Type must be either float or double");

			result[0] = (m1[0] * m2[0]) + (m1[1] * m2[3]) + (m1[2] * m2[6]);
			result[1] = (m1[0] * m2[1]) + (m1[1] * m2[4]) + (m1[2] * m2[7]);
			result[2] = (m1[0] * m2[2]) + (m1[1] * m2[5]) + (m1[2] * m2[8]);

			result[3] = (m1[3] * m2[0]) + (m1[4] * m2[3]) + (m1[5] * m2[6]);
			result[4] = (m1[3] * m2[1]) + (m1[4] * m2[4]) + (m1[5] * m2[7]);
			result[5] = (m1[3] * m2[2]) + (m1[4] * m2[5]) + (m1[5] * m2[8]);

			result[6] = (m1[6] * m2[0]) + (m1[7] * m2[3]) + (m1[8] * m2[6]);
			result[7] = (m1[6] * m2[1]) + (m1[7] * m2[4]) + (m1[8] * m2[7]);
			result[8] = (m1[6] * m2[2]) + (m1[7] * m2[5]) + (m1[8] * m2[8]);
		}
	}

	template<typename TDataType>
	void Matrix4<TDataType>::operator*=(const TDataType s) noexcept
	{
		if constexpr (std::is_same_v<TDataType, float>)
		{
			__m128 scalar = _mm_load_ps1(&s);

			_mm_store_ps(m + 0, _mm_mul_ps(_mm_load_ps(m + 0), scalar));
			_mm_store_ps(m + 4, _mm_mul_ps(_mm_load_ps(m + 4), scalar));
			_mm_store_ps(m + 8, _mm_mul_ps(_mm_load_ps(m + 8), scalar));
			_mm_store_ps(m + 12, _mm_mul_ps(_mm_load_ps(m + 12), scalar));
		}
		else
		{
			__m256d scalar = _mm256_broadcast_sd(&s);

			_mm256_store_pd(m + 0, _mm256_mul_pd(_mm256_load_pd(m + 0), scalar));
			_mm256_store_pd(m + 4, _mm256_mul_pd(_mm256_load_pd(m + 4), scalar));
			_mm256_store_pd(m + 8, _mm256_mul_pd(_mm256_load_pd(m + 8), scalar));
			_mm256_store_pd(m + 12, _mm256_mul_pd(_mm256_load_pd(m + 12), scalar));
		}
	}

	template<typename TDataType>
	void Matrix4<TDataType>::operator*=(const Matrix4& mat) noexcept
	{
		fastMat4x4Mult(this->m, this->m, mat.m);
	}

	template<typename TDataType>
	void Matrix4<TDataType>::operator*=(const Matrix3<TDataType>& mat) noexcept
	{
		auto mat4 = mat.convert<Matrix4, TDataType>();
		fastMat4x4Mult(this->m, this->m, mat4.m);
	}

	template<typename TDataType>
	void Matrix4<TDataType>::operator*=(std::span<const TDataType> mat) noexcept
	{
		assert(mat.size() == 16);
		fastMat4x4Mult(m, m, mat.data());
	}

	template<typename TDataType>
	void Matrix4<TDataType>::operator*=(const Quaternion<TDataType>& unitQuaternion) noexcept
	{
		auto mat = Matrix4::from(unitQuaternion);
		fastMat4x4Mult(m, m, mat.m);
	}

	template<typename TDataType>
	void Matrix4<TDataType>::operator+=(const Matrix4& mat) noexcept
	{
		if constexpr (std::is_same_v<TDataType, float>)
		{
			_mm_store_ps(m + 0, _mm_add_ps(_mm_load_ps(m + 0), _mm_load_ps(mat.m + 0)));
			_mm_store_ps(m + 4, _mm_add_ps(_mm_load_ps(m + 4), _mm_load_ps(mat.m + 4)));
			_mm_store_ps(m + 8, _mm_add_ps(_mm_load_ps(m + 8), _mm_load_ps(mat.m + 8)));
			_mm_store_ps(m + 12, _mm_add_ps(_mm_load_ps(m + 12), _mm_load_ps(mat.m + 12)));
		}
		else
		{
			_mm256_store_pd(m + 0, _mm256_add_pd(_mm256_load_pd(m + 0), _mm256_load_pd(mat.m + 0)));
			_mm256_store_pd(m + 4, _mm256_add_pd(_mm256_load_pd(m + 4), _mm256_load_pd(mat.m + 4)));
			_mm256_store_pd(m + 8, _mm256_add_pd(_mm256_load_pd(m + 8), _mm256_load_pd(mat.m + 8)));
			_mm256_store_pd(m + 12, _mm256_add_pd(_mm256_load_pd(m + 12), _mm256_load_pd(mat.m + 12)));
		}
	}

	template<typename TDataType>
	void Matrix4<TDataType>::operator+=(std::span<const TDataType> mat) noexcept
	{
		if constexpr (std::is_same_v<TDataType, float>)
		{
			assert(mat.size() == 16);
			_mm_store_ps(m +  0, _mm_add_ps(_mm_load_ps(m +  0), _mm_load_ps(mat.data() + 0)));
			_mm_store_ps(m +  4, _mm_add_ps(_mm_load_ps(m +  4), _mm_load_ps(mat.data() + 4)));
			_mm_store_ps(m +  8, _mm_add_ps(_mm_load_ps(m +  8), _mm_load_ps(mat.data() + 8)));
			_mm_store_ps(m + 12, _mm_add_ps(_mm_load_ps(m + 12), _mm_load_ps(mat.data() + 12)));
		}
		else
		{
			assert(mat.size() == 16);
			_mm256_store_pd(m + 0, _mm256_add_pd(_mm256_load_pd(m + 0), _mm256_load_pd(mat.data() + 0)));
			_mm256_store_pd(m + 4, _mm256_add_pd(_mm256_load_pd(m + 4), _mm256_load_pd(mat.data() + 4)));
			_mm256_store_pd(m + 8, _mm256_add_pd(_mm256_load_pd(m + 8), _mm256_load_pd(mat.data() + 8)));
			_mm256_store_pd(m + 12, _mm256_add_pd(_mm256_load_pd(m + 12), _mm256_load_pd(mat.data() + 12)));
		}
	}

	template<typename TDataType>
	void Matrix4<TDataType>::operator-=(const Matrix4& mat) noexcept
	{
		if constexpr (std::is_same_v<TDataType, float>)
		{
			_mm_store_ps(m +  0, _mm_sub_ps(_mm_load_ps(m +  0), _mm_load_ps(mat.m + 0)));
			_mm_store_ps(m +  4, _mm_sub_ps(_mm_load_ps(m +  4), _mm_load_ps(mat.m + 4)));
			_mm_store_ps(m +  8, _mm_sub_ps(_mm_load_ps(m +  8), _mm_load_ps(mat.m + 8)));
			_mm_store_ps(m + 12, _mm_sub_ps(_mm_load_ps(m + 12), _mm_load_ps(mat.m + 12)));
		}
		else
		{
			_mm256_store_pd(m + 0, _mm256_sub_pd(_mm256_load_pd(m + 0), _mm256_load_pd(mat.m + 0)));
			_mm256_store_pd(m + 4, _mm256_sub_pd(_mm256_load_pd(m + 4), _mm256_load_pd(mat.m + 4)));
			_mm256_store_pd(m + 8, _mm256_sub_pd(_mm256_load_pd(m + 8), _mm256_load_pd(mat.m + 8)));
			_mm256_store_pd(m + 12, _mm256_sub_pd(_mm256_load_pd(m + 12), _mm256_load_pd(mat.m + 12)));
		}
	}

	template<typename TDataType>
	void Matrix4<TDataType>::operator-=(std::span<const TDataType> mat) noexcept
	{
		if constexpr (std::is_same_v<TDataType, float>)
		{
			assert(mat.size() == 16);
			_mm_store_ps(m + 0, _mm_sub_ps(_mm_load_ps(m + 0), _mm_load_ps(mat.data() + 0)));
			_mm_store_ps(m + 4, _mm_sub_ps(_mm_load_ps(m + 4), _mm_load_ps(mat.data() + 4)));
			_mm_store_ps(m + 8, _mm_sub_ps(_mm_load_ps(m + 8), _mm_load_ps(mat.data() + 8)));
			_mm_store_ps(m + 12, _mm_sub_ps(_mm_load_ps(m + 12), _mm_load_ps(mat.data() + 12)));
		}
		else
		{
			assert(mat.size() == 16);
			_mm256_store_pd(m + 0, _mm256_sub_pd(_mm256_load_pd(m + 0), _mm256_load_pd(mat.data() + 0)));
			_mm256_store_pd(m + 4, _mm256_sub_pd(_mm256_load_pd(m + 4), _mm256_load_pd(mat.data() + 4)));
			_mm256_store_pd(m + 8, _mm256_sub_pd(_mm256_load_pd(m + 8), _mm256_load_pd(mat.data() + 8)));
			_mm256_store_pd(m + 12, _mm256_sub_pd(_mm256_load_pd(m + 12), _mm256_load_pd(mat.data() + 12)));
		}
	}

	template<typename TDataType>
	Matrix4<TDataType> Matrix4<TDataType>::operator*(const Matrix4& mat) const noexcept
	{
		Matrix4 res;
		fastMat4x4Mult(res.m, m, mat.m);
		return res;
	}

	template<typename TDataType>
	Matrix4<TDataType> Matrix4<TDataType>::operator+(const Matrix4& mat) const noexcept
	{
		Matrix4 res;

		if constexpr (std::is_same_v<TDataType, float>)
		{
			_mm_store_ps(res.m + 0, _mm_add_ps(_mm_load_ps(m + 0), _mm_load_ps(mat.m + 0)));
			_mm_store_ps(res.m + 4, _mm_add_ps(_mm_load_ps(m + 4), _mm_load_ps(mat.m + 4)));
			_mm_store_ps(res.m + 8, _mm_add_ps(_mm_load_ps(m + 8), _mm_load_ps(mat.m + 8)));
			_mm_store_ps(res.m + 12, _mm_add_ps(_mm_load_ps(m + 12), _mm_load_ps(mat.m + 12)));
		}
		else
		{
			_mm256_store_pd(res.m + 0, _mm256_sub_pd(_mm256_load_pd(m + 0), _mm256_load_pd(mat.m + 0)));
			_mm256_store_pd(res.m + 4, _mm256_sub_pd(_mm256_load_pd(m + 4), _mm256_load_pd(mat.m + 4)));
			_mm256_store_pd(res.m + 8, _mm256_sub_pd(_mm256_load_pd(m + 8), _mm256_load_pd(mat.m + 8)));
			_mm256_store_pd(res.m + 12, _mm256_sub_pd(_mm256_load_pd(m + 12), _mm256_load_pd(mat.m + 12)));
		}

		return res;
	}

	template<typename TDataType>
	Matrix4<TDataType> Matrix4<TDataType>::operator-(const Matrix4& mat) const noexcept
	{
		Matrix4 res;

		if constexpr (std::is_same_v<TDataType, float>)
		{
			_mm_store_ps(res.m + 0, _mm_sub_ps(_mm_load_ps(m + 0), _mm_load_ps(mat.m + 0)));
			_mm_store_ps(res.m + 4, _mm_sub_ps(_mm_load_ps(m + 4), _mm_load_ps(mat.m + 4)));
			_mm_store_ps(res.m + 8, _mm_sub_ps(_mm_load_ps(m + 8), _mm_load_ps(mat.m + 8)));
			_mm_store_ps(res.m + 12, _mm_sub_ps(_mm_load_ps(m + 12), _mm_load_ps(mat.m + 12)));
		}
		else
		{
			_mm256_store_pd(res.m + 0, _mm256_sub_pd(_mm256_load_pd(m + 0), _mm256_load_pd(mat.m + 0)));
			_mm256_store_pd(res.m + 4, _mm256_sub_pd(_mm256_load_pd(m + 4), _mm256_load_pd(mat.m + 4)));
			_mm256_store_pd(res.m + 8, _mm256_sub_pd(_mm256_load_pd(m + 8), _mm256_load_pd(mat.m + 8)));
			_mm256_store_pd(res.m + 12, _mm256_sub_pd(_mm256_load_pd(m + 12), _mm256_load_pd(mat.m + 12)));
		}

		return res;
	}

	template<typename TDataType>
	Matrix4<TDataType> Matrix4<TDataType>::operator*(const TDataType s) const noexcept
	{
		Matrix4 res;

		if constexpr (std::is_same_v<TDataType, float>)
		{
			__m128 scalar = _mm_load_ps1(&s);
			_mm_store_ps(res.m + 0, _mm_mul_ps(_mm_load_ps(m + 0), scalar));
			_mm_store_ps(res.m + 4, _mm_mul_ps(_mm_load_ps(m + 4), scalar));
			_mm_store_ps(res.m + 8, _mm_mul_ps(_mm_load_ps(m + 8), scalar));
			_mm_store_ps(res.m + 12, _mm_mul_ps(_mm_load_ps(m + 12), scalar));
		}
		else
		{
			__m256d scalar = _mm256_broadcast_sd(&s);
			_mm256_store_pd(res.m + 0, _mm256_mul_pd(_mm256_load_pd(m + 0), scalar));
			_mm256_store_pd(res.m + 4, _mm256_mul_pd(_mm256_load_pd(m + 4), scalar));
			_mm256_store_pd(res.m + 8, _mm256_mul_pd(_mm256_load_pd(m + 8), scalar));
			_mm256_store_pd(res.m + 12, _mm256_mul_pd(_mm256_load_pd(m + 12), scalar));
		}

		return res;
	}

	template<typename TDataType>
	void Matrix4<TDataType>::transform(std::span<TDataType> vec) const noexcept
	{
		assert((vec.size() == 3) || (vec.size() == 4));

		if (vec.size() == 3)
		{
			auto vecX = vec[0];
			auto vecY = vec[1];
			auto vecZ = vec[2];

			vec[0] = vecX * m[0] + vecY * m[4] + vecZ * m[8] + m[12];
			vec[1] = vecX * m[1] + vecY * m[5] + vecZ * m[9] + m[13];
			vec[2] = vecX * m[2] + vecY * m[6] + vecZ * m[10] + m[14];
		}
		else
		{
			auto vecData = vec.data();

			if constexpr (std::is_same_v<TDataType, float>)
			{
				__m128 row1 = _mm_mul_ps(_mm_load_ps1(vecData + 0), _mm_load_ps(m));
				__m128 row2 = _mm_mul_ps(_mm_load_ps1(vecData + 1), _mm_load_ps(m + 4));
				__m128 row3 = _mm_mul_ps(_mm_load_ps1(vecData + 2), _mm_load_ps(m + 8));
				__m128 row4 = _mm_mul_ps(_mm_load_ps1(vecData + 3), _mm_load_ps(m + 12));
				_mm_store_ps(vec.data(), _mm_add_ps(_mm_add_ps(row1, row2), _mm_add_ps(row3, row4)));
			}
			else
			{
				__m256d row1 = _mm256_mul_pd(_mm256_broadcast_sd(vecData + 0), _mm256_load_pd(m));
				__m256d row2 = _mm256_mul_pd(_mm256_broadcast_sd(vecData + 1), _mm256_load_pd(m + 4));
				__m256d row3 = _mm256_mul_pd(_mm256_broadcast_sd(vecData + 2), _mm256_load_pd(m + 8));
				__m256d row4 = _mm256_mul_pd(_mm256_broadcast_sd(vecData + 3), _mm256_load_pd(m + 12));
				_mm256_store_pd(vec.data(), _mm256_add_pd(_mm256_add_pd(row1, row2), _mm256_add_pd(row3, row4)));
			}
		}
	}

	template<typename TDataType>
	void Matrix4<TDataType>::transform(Vector3Type& vec) const noexcept
	{
		if constexpr (std::is_same_v<TDataType, float>)
		{
			__m128 row1 = _mm_mul_ps(_mm_load_ps1(&vec[0]), _mm_load_ps(m));
			__m128 row2 = _mm_mul_ps(_mm_load_ps1(&vec[1]), _mm_load_ps(m + 4));
			__m128 row3 = _mm_mul_ps(_mm_load_ps1(&vec[2]), _mm_load_ps(m + 8));
			_mm_store_ps(vec.data(), _mm_add_ps(_mm_add_ps(row1, row2), _mm_add_ps(row3, _mm_load_ps(m + 12))));
		}
		else
		{
			__m256d row1 = _mm256_mul_pd(_mm256_broadcast_sd(&vec[0]), _mm256_load_pd(m));
			__m256d row2 = _mm256_mul_pd(_mm256_broadcast_sd(&vec[1]), _mm256_load_pd(m + 4));
			__m256d row3 = _mm256_mul_pd(_mm256_broadcast_sd(&vec[2]), _mm256_load_pd(m + 8));
			_mm256_store_pd(vec.data(), _mm256_add_pd(_mm256_add_pd(row1, row2), _mm256_add_pd(row3, _mm256_load_pd(m + 12))));
		}
	}

	template<typename TDataType>
	void Matrix4<TDataType>::transform(const Vector3Type& vec, Vector3Type& result) const noexcept
	{
		if constexpr (std::is_same_v<TDataType, float>)
		{
			__m128 row1 = _mm_mul_ps(_mm_load_ps1(&vec[0]), _mm_load_ps(m));
			__m128 row2 = _mm_mul_ps(_mm_load_ps1(&vec[1]), _mm_load_ps(m + 4));
			__m128 row3 = _mm_mul_ps(_mm_load_ps1(&vec[2]), _mm_load_ps(m + 8));
			_mm_store_ps(result.data(), _mm_add_ps(_mm_add_ps(row1, row2), _mm_add_ps(row3, _mm_load_ps(m + 12))));
		}
		else
		{
			__m256d row1 = _mm256_mul_pd(_mm256_broadcast_sd(&vec[0]), _mm256_load_pd(m));
			__m256d row2 = _mm256_mul_pd(_mm256_broadcast_sd(&vec[1]), _mm256_load_pd(m + 4));
			__m256d row3 = _mm256_mul_pd(_mm256_broadcast_sd(&vec[2]), _mm256_load_pd(m + 8));
			_mm256_store_pd(result.data(), _mm256_add_pd(_mm256_add_pd(row1, row2), _mm256_add_pd(row3, _mm256_load_pd(m + 12))));
		}
	}

	template<typename TDataType>
	void Matrix4<TDataType>::transform(std::span<Vector3Type> vecs) const noexcept
	{
		asmMat4x4Vec3(vecs.data()->data(), vecs.data()->data(), kOne<TDataType>, sizeof(Vector3Type), m, vecs.size());
	}

	template<typename TDataType>
	void Matrix4<TDataType>::transform(Vector4Type& vec) const noexcept
	{
		transform({vec.data(), 3});
	}

	template<typename TDataType>
	void Matrix4<TDataType>::transform(const Vector4Type& vec, Vector4Type& result) const noexcept
	{
		if constexpr (std::is_same_v<TDataType, float>)
		{
			__m128 row1 = _mm_mul_ps(_mm_load_ps1(&vec[0]), _mm_load_ps(m));
			__m128 row2 = _mm_mul_ps(_mm_load_ps1(&vec[1]), _mm_load_ps(m + 4));
			__m128 row3 = _mm_mul_ps(_mm_load_ps1(&vec[2]), _mm_load_ps(m + 8));
			__m128 row4 = _mm_mul_ps(_mm_load_ps1(&vec[3]), _mm_load_ps(m + 12));
			_mm_store_ps(result.data(), _mm_add_ps(_mm_add_ps(row1, row2), _mm_add_ps(row3, row4)));
		}
		else
		{
			__m256d row1 = _mm256_mul_pd(_mm256_broadcast_sd(&vec[0]), _mm256_load_pd(m));
			__m256d row2 = _mm256_mul_pd(_mm256_broadcast_sd(&vec[1]), _mm256_load_pd(m + 4));
			__m256d row3 = _mm256_mul_pd(_mm256_broadcast_sd(&vec[2]), _mm256_load_pd(m + 8));
			__m256d row4 = _mm256_mul_pd(_mm256_broadcast_sd(&vec[3]), _mm256_load_pd(m + 12));
			_mm256_store_pd(result.data(), _mm256_add_pd(_mm256_add_pd(row1, row2), _mm256_add_pd(row3, row4)));
		}
	}

	template<typename TDataType>
	void Matrix4<TDataType>::transform(std::span<Vector4Type> vecs) const noexcept
	{
		asmMat4x4Vec4(vecs.data()->data(), vecs.data()->data(), sizeof(Vector4Type), m, vecs.size());
	}

	template<typename TDataType>
	void Matrix4<TDataType>::transform(BBox<Vector3Type>& bbox) const noexcept
	{
		std::array<Vector3Type, 8> pts;

		bbox.corners(pts.data());
		transform(pts);

		bbox.reset();
		bbox.merge(pts.data(), 8);
	}

	template<typename TDataType>
	void Matrix4<TDataType>::transform(const BBox<Vector3Type>& bbox, BBox<Vector3Type>& bboxDest) const noexcept
	{
		std::array<Vector3Type, 8> pts;

		bbox.corners(pts.data());
		transform(pts);

		bboxDest.reset();
		bboxDest.merge(pts.data(), 8);
	}

	template<typename TDataType>
	Matrix4<TDataType>::Vector4Type Matrix4<TDataType>::getColumn(size_t columnIndex) const noexcept
	{
		columnIndex = columnIndex % 4;
		if constexpr (std::is_same_v<TDataType, float>)
		{
			return Vector4Type(m[columnIndex], m[columnIndex + 4], m[columnIndex + 8], m[columnIndex + 12]);
		}
		else
		{
			return {};
		}
	}

	template<typename TDataType>
	Matrix4<TDataType>::Vector4Type Matrix4<TDataType>::getRow(size_t rowIndex) const noexcept
	{
		if constexpr (std::is_same_v<TDataType, float>)
		{
			return Vector4Type(m + ((rowIndex % 4) * 4));
		}
		else
		{
			return {};
		}
	}

	template<typename TDataType>
	void Matrix4<TDataType>::write(TDataType dest[16]) const noexcept
	{
		std::memcpy(dest, m, sizeof(TDataType) * 16);
	}

	template<typename TDataType>
	Matrix4<TDataType> Matrix4<TDataType>::clone(CloneTransform transform) const noexcept
	{
		switch (transform)
		{
			case CloneTransform::Transpose:
			{
				Matrix4 mat;
				fastMat4x4Transpose(mat.m, m);
				return mat;
			}
			case CloneTransform::Inverse:
			{
				Matrix4 mat(*this);
				mat.inverse();
				return mat;
			}
			case CloneTransform::InverseTranspose:
			{
				Matrix4 mat(*this);
				mat.inverseTranspose();
				return mat;
			}
			case CloneTransform::InverseHomogenous:
			{
				Matrix4 mat(*this);
				mat.inverseHomogenous();
				return mat;
			}
			case CloneTransform::None:
			default:
				break;
		}

		//just copy
		return Matrix4{*this};
	}

	template<typename TDataType>
	Matrix4<TDataType>& Matrix4<TDataType>::transpose() noexcept
	{
		fastMat4x4Transpose(m, m);
		return *this;
	}

	template<typename TDataType>
	Matrix4<TDataType>& Matrix4<TDataType>::inverse() noexcept
	{
		if constexpr (std::is_same_v<TDataType, float>)
		{
			float tmp[12], result[16], det;

			//calculate pairs for first 8 elements (cofactors)
			tmp[0] = m[10] * m[15];
			tmp[1] = m[11] * m[14];
			tmp[2] = m[9] * m[15];
			tmp[3] = m[11] * m[13];
			tmp[4] = m[9] * m[14];
			tmp[5] = m[10] * m[13];
			tmp[6] = m[8] * m[15];
			tmp[7] = m[11] * m[12];
			tmp[8] = m[8] * m[14];
			tmp[9] = m[10] * m[12];
			tmp[10] = m[8] * m[13];
			tmp[11] = m[9] * m[12];

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
			tmp[0] = m[2] * m[7];
			tmp[1] = m[3] * m[6];
			tmp[2] = m[1] * m[7];
			tmp[3] = m[3] * m[5];
			tmp[4] = m[1] * m[6];
			tmp[5] = m[2] * m[5];
			tmp[6] = m[0] * m[7];
			tmp[7] = m[3] * m[4];
			tmp[8] = m[0] * m[6];
			tmp[9] = m[2] * m[4];
			tmp[10] = m[0] * m[5];
			tmp[11] = m[1] * m[4];

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
			if (Math::isZero(det)) return *this;

			//multiplicar tudo pelo determinante
			det = 1.0f / det;
			result[0] *= det;
			result[1] *= det;
			result[2] *= det;
			result[3] *= det;
			result[4] *= det;
			result[5] *= det;
			result[6] *= det;
			result[7] *= det;
			result[8] *= det;
			result[9] *= det;
			result[10] *= det;
			result[11] *= det;
			result[12] *= det;
			result[13] *= det;
			result[14] *= det;
			result[15] *= det;

			std::memcpy(m, result, sizeof(float) * 16);
		}
		else
		{
		}

		return *this;
	}

	template<typename TDataType>
	Matrix4<TDataType>& Matrix4<TDataType>::inverseTranspose() noexcept
	{
		inverse();
		transpose();
		return *this;
	}

	template<typename TDataType>
	Matrix4<TDataType>& Matrix4<TDataType>::inverseHomogenous() noexcept
	{
		std::swap(m[1], m[4]);
		std::swap(m[2], m[8]);
		std::swap(m[6], m[9]);

		auto aux1 = -(m[0] * m[12] + m[4] * m[13] + m[8] * m[14]);
		auto aux2 = -(m[1] * m[12] + m[5] * m[13] + m[9] * m[14]);
		m[14] = -(m[2] * m[12] + m[6] * m[13] + m[10] * m[14]);
		m[13] = aux2;
		m[12] = aux1;

		return *this;
	}

	template<typename TDataType>
	Matrix4<TDataType>::Vector3Type Matrix4<TDataType>::extractTranslation() const noexcept
	{
		return Vector3Type{m[12], m[13], m[14]};
	}

	template<typename TDataType>
	Matrix4<TDataType>::Vector3Type Matrix4<TDataType>::extractScale() const noexcept
	{
		Vector3Type scaling;
		scaling[0] = m[15] * std::sqrt((m[0] * m[0]) + (m[1] * m[1]) + (m[2] * m[2]));
		scaling[1] = m[15] * std::sqrt((m[4] * m[4]) + (m[5] * m[5]) + (m[6] * m[6]));
		scaling[2] = m[15] * std::sqrt((m[8] * m[8]) + (m[9] * m[9]) + (m[10] * m[10]));

		return scaling;
	}

	template<typename TDataType>
	Quaternion<TDataType> Matrix4<TDataType>::extractRotation() const noexcept
	{
		Matrix4 tmp{ *this };

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

		return Quaternion<TDataType>::fromMatrix4x4(tmp.data());
	}

	template<typename TDataType>
	void Matrix3<TDataType>::operator*=(const Matrix3& s) noexcept
	{
		mat3x3Mult<TDataType>(m, s.m);
	}
	
	template<typename TDataType>
	void Matrix3<TDataType>::operator*=(std::span<const TDataType> mat) noexcept
	{
		mat3x3Mult<TDataType>(m, mat);
	}

	template<typename TDataType>
	void Matrix3<TDataType>::operator+=(const Matrix3& mat) noexcept
	{
		for (int i = 0; i < 9; i++)
			m[i] += mat.m[i];
	}

	template<typename TDataType>
	void Matrix3<TDataType>::operator+=(std::span<const TDataType> mat) noexcept
	{
		assert(mat.size() == 9);

		for (int i = 0; i < 9; i++)
			m[i] += mat[i];
	}

	template<typename TDataType>
	void Matrix3<TDataType>::operator-=(const Matrix3& mat) noexcept
	{
		for (int i = 0; i < 9; i++)
			m[i] -= mat.m[i];
	}

	template<typename TDataType>
	void Matrix3<TDataType>::operator-=(std::span<const TDataType> mat) noexcept
	{
		assert(mat.size() == 9);

		for (int i = 0; i < 9; i++)
			m[i] += mat[i];
	}

	template<typename TDataType>
	Matrix3<TDataType> Matrix3<TDataType>::operator*(const Matrix3& mat) const noexcept
	{
		Matrix3 res;
		mat3x3Mult<TDataType>(res.m, m, mat.m);
		return res;
	}

	template<typename TDataType>
	Matrix3<TDataType> Matrix3<TDataType>::operator+(const Matrix3& mat) const noexcept
	{
		Matrix3 res(*this);
		res += mat;
		return res;
	}

	template<typename TDataType>
	Matrix3<TDataType> Matrix3<TDataType>::operator-(const Matrix3& mat) const noexcept
	{
		Matrix3 res(*this);
		res -= mat;
		return res;
	}

	template<typename TDataType>
	void Matrix3<TDataType>::transform(TDataType vec[3]) const noexcept
	{
		auto vecX = vec[0];
		auto vecY = vec[1];
		auto vecZ = vec[2];

		vec[0] = vecX*m[0] + vecY*m[3] + vecZ*m[6];
		vec[1] = vecX*m[1] + vecY*m[4] + vecZ*m[7];
		vec[2] = vecX*m[2] + vecY*m[5] + vecZ*m[8];
	}

	template<typename TDataType>
	void Matrix3<TDataType>::transform(Vector3Type& vec) const noexcept
	{
		transform(Vector3Type{vec}, vec);
	}

	template<typename TDataType>
	void Matrix3<TDataType>::transform(const Vector3Type& vec, Vector3Type& result) const noexcept
	{
		result[0] = vec[0]*m[0] + vec[1]*m[3] + vec[2]*m[6];
		result[1] = vec[0]*m[1] + vec[1]*m[4] + vec[2]*m[7];
		result[2] = vec[0]*m[2] + vec[1]*m[5] + vec[2]*m[8];
	}

	template<typename TDataType>
	Matrix3<TDataType>::Vector3Type Matrix3<TDataType>::transform(const Vector3Type& vec) const noexcept
	{
		Vector3Type res;
		transform(vec, res);
		return res;
	}

	template<typename TDataType>
	void Matrix3<TDataType>::transform(std::span<Vector3Type> vecs) const noexcept
	{
		for (auto& vec : vecs)
			transform(vec);
	}

	template<typename TDataType>
	Matrix3<TDataType>::Vector3Type Matrix3<TDataType>::getColumn(size_t columnIndex) const noexcept
	{
		columnIndex = columnIndex % 3;
		return Vector3Type(m[columnIndex], m[columnIndex + 3], m[columnIndex + 6]);
	}

	template<typename TDataType>
	Matrix3<TDataType>::Vector3Type Matrix3<TDataType>::getRow(size_t rowIndex) const noexcept
	{
		return Vector3Type(m + ((rowIndex % 3) * 3));
	}

	template<typename TDataType>
	void Matrix3<TDataType>::write(TDataType dest[9]) const noexcept
	{
		std::memcpy(dest, m, sizeof(TDataType) * 9);
	}

	template<typename TDataType>
	Matrix3<TDataType> Matrix3<TDataType>::clone(CloneTransform transform) const noexcept
	{
		switch (transform)
		{
			case CloneTransform::Transpose:
			{
				Matrix3 mat;
				mat.m[0] = m[0];
				mat.m[1] = m[3];
				mat.m[2] = m[6];
				mat.m[3] = m[1];
				mat.m[4] = m[4];
				mat.m[5] = m[7];
				mat.m[6] = m[2];
				mat.m[7] = m[5];
				mat.m[8] = m[8];
				return mat;
			}
			case CloneTransform::None:
			default:
				break;
		}

		//just copy
		return Matrix3{*this};
	}

	template class Matrix4<float>;
	template class Matrix4<double>;

	template class Matrix3<float>;
	template class Matrix3<double>;
}
