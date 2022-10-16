#pragma once

#include "matrix.hpp"
#include "random.hpp"

#include <format>
#include <optional>

#include "libs/catch2/catch.hpp"

namespace Catch
{
	template<>
	struct StringMaker<hr::Matrix>
	{
		static std::string convert(hr::Matrix const& m)
		{
			return std::format("Matrix is: {{{{{}, {}, {}, {}}}, {{{}, {}, {}, {}}}, {{{}, {}, {}, {}}}, {{{}, {}, {}, {}}}}}", m[0], m[1], m[2], m[3], m[4], m[5], m[6], m[7], m[8], m[9], m[10], m[11], m[12], m[13], m[14], m[15]);
		}
	};

	template<>
	struct StringMaker<hr::Matrix3f>
	{
		static std::string convert(hr::Matrix3f const& m)
		{
			return std::format("Matrix is: {{{{{}, {}, {}}}, {{{}, {}, {}}}, {{{}, {}, {}}}}}", m[0], m[1], m[2], m[3], m[4], m[5], m[6], m[7], m[8]);
		}
	};

	template<>
	struct StringMaker<hr::Matrix3d>
	{
		static std::string convert(hr::Matrix3d const& m)
		{
			return std::format("Matrix is: {{{{{}, {}, {}}}, {{{}, {}, {}}}, {{{}, {}, {}}}}}", m[0], m[1], m[2], m[3], m[4], m[5], m[6], m[7], m[8]);
		}
	};

	template<>
	struct StringMaker<hr::Vector3f>
	{
		static std::string convert(hr::Vector3f const& v)
		{
			return std::format("Vector is: {{{}, {}, {}}}", v[0], v[1], v[2]);
		}
	};
}

namespace hr::utests
{
	namespace
	{
		template<typename TType>
		struct Matrix4EqualsMatcher : Catch::Matchers::MatcherGenericBase
		{
			Matrix4EqualsMatcher(const TType mat[16])
			{ 
				std::memcpy(m_mat, mat, sizeof(TType) * 16);
			}

			bool match(const Matrix& m) const
			{
				for (int i = 0; i < 16; i++)
				{
					if (m[i] != Catch::Approx(m_mat[i])) return false;
				}

				return true;
			}

			std::string describe() const override
			{
				return std::format("Matrix4x4 is: {{{{{}, {}, {}, {}}}, {{{}, {}, {}, {}}}, {{{}, {}, {}, {}}}, {{{}, {}, {}, {}}}}}", m_mat[0], m_mat[1], m_mat[2],
				  m_mat[3], m_mat[4], m_mat[5], m_mat[6], m_mat[7], m_mat[8], m_mat[9], m_mat[10], m_mat[11], m_mat[12], m_mat[13], m_mat[14], m_mat[15]);
			}

		private:
			TType m_mat[16];
		};

		auto Matrix4Equals(const float mat[16]) -> Matrix4EqualsMatcher<float>
		{
			return Matrix4EqualsMatcher<float>{mat};
		}

		auto Matrix4Equals(const Matrix& mat) -> Matrix4EqualsMatcher<float>
		{
			return Matrix4EqualsMatcher<float>{mat.data()};
		}

		template<typename TType>
		struct Matrix3EqualsMatcher : Catch::Matchers::MatcherGenericBase
		{
			Matrix3EqualsMatcher(const TType mat[9])
			{
				std::memcpy(m_mat, mat, sizeof(TType) * 9);
			}

			bool match(const Matrix3<TType> & m) const
			{
				for (int i = 0; i < 9; i++)
				{
					if (m[i] != Catch::Approx(m_mat[i])) return false;
				}

				return true;
			}

			std::string describe() const override
			{
				return std::format("Matrix3x3 is: {{{{{}, {}, {}}}, {{{}, {}, {}}}, {{{}, {}, {}}}}}", m_mat[0], m_mat[1], m_mat[2], m_mat[3], m_mat[4], m_mat[5], m_mat[6], m_mat[7], m_mat[8]);
			}

		private:
			TType m_mat[9];
		};

		auto Matrix3Equals(const float mat[9]) -> Matrix3EqualsMatcher<float>
		{
			return Matrix3EqualsMatcher<float>{mat};
		}

		auto Matrix3Equals(const double mat[9]) -> Matrix3EqualsMatcher<double>
		{
			return Matrix3EqualsMatcher<double>{mat};
		}

		auto Matrix3Equals(const Matrix3f& mat) -> Matrix3EqualsMatcher<float>
		{
			return Matrix3EqualsMatcher<float>{mat.data()};
		}

		auto Matrix3Equals(const Matrix3d& mat) -> Matrix3EqualsMatcher<double>
		{
			return Matrix3EqualsMatcher<double>{mat.data()};
		}

		struct VectorEqualsMatcher : Catch::Matchers::MatcherGenericBase
		{
			VectorEqualsMatcher(float x, float y, float z)
			  : m_x{x} , m_y{y} , m_z{z}
			{ }

			bool match(const Vector3f& v) const
			{
				// set epsilon to allowed a 0.1% difference and a margin to allow (0.0f == -0.0f) to pass

				return (v[0] == Catch::Approx(m_x).epsilon(0.001).margin(0.0000001)) && (v[1] == Catch::Approx(m_y).epsilon(0.001).margin(0.0000001))
				  && (v[2] == Catch::Approx(m_z).epsilon(0.001).margin(0.0000001));
			}

			std::string describe() const override
			{
				return std::format("Vector is: {{{}, {}, {}}}", m_x, m_y, m_z);
			}

		private:
			float m_x, m_y, m_z;
		};

		auto VectorEquals(const float vx, const float vy, const float vz) -> VectorEqualsMatcher
		{
			return VectorEqualsMatcher{vx, vy, vz};
		}

		auto VectorEquals(const Vector3f& v) -> VectorEqualsMatcher
		{
			return VectorEqualsMatcher{v[0], v[1], v[2]};
		}

		Vector3f randomVector3f()
		{
			Random r;
			return Vector3f(r.nextDouble(-10.0f, 10.0f), r.nextDouble(-10.0f, 10.0f), r.nextDouble(-10.0f, 10.0f));
		}
	}

	TEST_CASE("Matrix 4x4 of type float", "[common][matrix][matrix4x4f]")
	{
		SECTION("init")
		{
			const float matIdentity[]{ 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f };

			REQUIRE_THAT(Matrix::identity(), Matrix4Equals(matIdentity));
			
		}

		/*
		SECTION("inverse")
		{
			const float matData[]{0.6f, 0.2f, 0.3f, 0.4f, 0.2f, 0.7f, 0.5f, 0.3f, 0.3f, 0.5f, 0.7f, 0.2f, 0.4f, 0.3f, 0.2f, 0.6f};

			Matrix m{matData};
			auto mInverse = m.getInverse();
			m *= mInverse;
			
			REQUIRE_THAT(Matrix::genMatIdentity(), Matrix4Equals(m));
		}

		SECTION("multiplication")
		{
			const float matData[]{0.0f, 1.0f, 2.0f, 3.0f, 4.0f, 5.0f, 6.0f, 7.0f, 8.0f, 9.0f, 10.0f, 11.0f, 12.0f, 13.0f, 14.0f, 15.0f};
			const float matRes[]{0.0f, 1.0f, 4.0f, 9.0f, 16.0f, 25.0f, 36.0f, 49.0f, 64.0f, 81.0f, 100.0f, 121.0f, 144.0f, 169.0f, 196.0f, 225.0f};

			Matrix m{matData};
			m = m * m;
			REQUIRE_THAT(m, Matrix4Equals(matRes));
		}
		*/
	}

	TEST_CASE("Matrix 3x3 of type float", "[common][matrix][matrix3x3f]")
	{
		SECTION("init")
		{
			const float matIdentity[]{ 1.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 1.0f };
			REQUIRE_THAT(Matrix3f::identity(), Matrix3Equals(matIdentity));

			const float mat1[]{2.0f, -5.0f, 3.0f, 7.0f, 1.0f, -6.0f, -9.0f, 4.0f, 8.0f};
			REQUIRE_THAT(Matrix3f::from({2.0f, -5.0f, 3.0f, 7.0f, 1.0f, -6.0f, -9.0f, 4.0f, 8.0f}), Matrix3Equals(mat1));

			const float mat2[]{-12.0f, 91.0f, 48.0f, -133.0f, 67.0f, 90.0f, -60.0f, 40.0f, -25.0f};
			const float* mat3 = mat2;
			REQUIRE_THAT(Matrix3f::from<float>(mat2), Matrix3Equals(mat2));
			REQUIRE_THAT(Matrix3f::from(std::span{mat3, 9}), Matrix3Equals(mat3));
		}

		SECTION("matrix plus matrix")
		{
			auto m1 = Matrix3f::from({2.0f, -5.0f, 3.0f, 7.0f, 1.0f, -6.0f, -9.0f, 4.0f, 8.0f});
			auto m2 = Matrix3f::from({-14.0f, 96.0f, 45.0f, -140.0f, 66.0f, 96.0f, -51.0f, 36.0f, -33.0f});
			auto m4 = Matrix3f::from({-12.0f, 91.0f, 48.0f, -133.0f, 67.0f, 90.0f, -60.0f, 40.0f, -25.0f});
			auto m3 = m1 + m2;
			REQUIRE_THAT(m3, Matrix3Equals(m4));
			m3 = m1; m3 += m2;
			REQUIRE_THAT(m3, Matrix3Equals(m4));

			m1 = Matrix3f::from({0.24f, 0.0082f, -0.3f, 0.6f, -1.4f, 0.73f, 0.38f, 0.096f, -0.16f});
			m2 = Matrix3f::from({1.7f, 0.003f, -5.6f, 0.44f, 0.2082f, -0.1f, 0.8f, -1.2f, 0.93f});
			m4 = Matrix3f::from({1.94f, 0.0112f, -5.9f, 1.04f, -1.1918f, 0.63f, 1.18f, -1.104f, 0.77f});
			m3 = m1 + m2;
			REQUIRE_THAT(m3, Matrix3Equals(m4));
			m3 = m1; m3 += m2;
			REQUIRE_THAT(m3, Matrix3Equals(m4));

			m1 = Matrix3f::from({-27.0f, 83.0f, 32.0f, -153.0f, 53.0f, 83.0f, -64.0f, 23.0f, -46.0f});
			m2 = Matrix3f::from({5.0f, -2.0f, 6.0f, 10.0f, 4.0f, -3.0f, -6.0f, 7.0f, 11.0f});
			m4 = Matrix3f::from({-22.0f, 81.0f, 38.0f, -143.0f, 57.0f, 80.0f, -70.0f, 30.0f, -35.0f});
			m3 = m1 + m2;
			REQUIRE_THAT(m3, Matrix3Equals(m4));
			m3 = m1; m3 += m2;
			REQUIRE_THAT(m3, Matrix3Equals(m4));
		}

		SECTION("matrix minus matrix")
		{
			auto m1 = Matrix3f::from({2.0f, -5.0f, 3.0f, 7.0f, 1.0f, -6.0f, -9.0f, 4.0f, 8.0f});
			auto m2 = Matrix3f::from({-14.0f, 96.0f, 45.0f, -140.0f, 66.0f, 96.0f, -51.0f, 36.0f, -33.0f});
			auto m4 = Matrix3f::from({16.0f, -101.0f, -42.0f, 147.0f, -65.0f, -102.0f, 42.0f, -32.0f, 41.0f});
			auto m3 = m1 - m2;
			REQUIRE_THAT(m3, Matrix3Equals(m4));
			m3 = m1; m3 -= m2;
			REQUIRE_THAT(m3, Matrix3Equals(m4));

			m1 = Matrix3f::from({0.24f, 0.0082f, -0.3f, 0.6f, -1.4f, 0.73f, 0.38f, 0.096f, -0.16f});
			m2 = Matrix3f::from({1.7f, 0.003f, -5.6f, 0.44f, 0.2082f, -0.1f, 0.8f, -1.2f, 0.93f});
			m4 = Matrix3f::from({-1.46f, 0.0052f, 5.3f, 0.16f, -1.6082f, 0.83f, -0.42f, 1.296f, -1.09f});
			m3 = m1 - m2;
			REQUIRE_THAT(m3, Matrix3Equals(m4));
			m3 = m1; m3 -= m2;
			REQUIRE_THAT(m3, Matrix3Equals(m4));

			m1 = Matrix3f::from({-27.0f, 83.0f, 32.0f, -153.0f, 53.0f, 83.0f, -64.0f, 23.0f, -46.0f});
			m2 = Matrix3f::from({5.0f, -2.0f, 6.0f, 10.0f, 4.0f, -3.0f, -6.0f, 7.0f, 11.0f});
			m4 = Matrix3f::from({-32.0f, 85.0f, 26.0f, -163.0f, 49.0f, 86.0f, -58.0f, 16.0f, -57.0f});
			m3 = m1 - m2;
			REQUIRE_THAT(m3, Matrix3Equals(m4));
			m3 = m1; m3 -= m2;
			REQUIRE_THAT(m3, Matrix3Equals(m4));
		}

		SECTION("transpose")
		{
			auto m1 = Matrix3f::from({2.0f, -5.0f, 3.0f, 7.0f, 1.0f, -6.0f, -9.0f, 4.0f, 8.0f});
			m1.transpose();
			REQUIRE_THAT(m1, Matrix3Equals(Matrix3f::from({2.0f, 7.0f, -9.0f, -5.0f, 1.0f, 4.0f, 3.0f, -6.0f, 8.0f})));

			m1 = Matrix3f::from({0.24f, 0.0082f, -0.3f, 0.6f, -1.4f, 0.73f, 0.38f, 0.096f, -0.16f});
			m1.transpose();
			REQUIRE_THAT(m1, Matrix3Equals(Matrix3f::from({0.24f, 0.6f, 0.38f, 0.0082f, -1.4f, 0.096f, -0.3f, 0.73f, -0.16f})));

			m1 = Matrix3f::from({-27.0f, 83.0f, 32.0f, -153.0f, 53.0f, 83.0f, -64.0f, 23.0f, -46.0f});
			m1.transpose();
			REQUIRE_THAT(m1, Matrix3Equals(Matrix3f::from({-27.0f, -153.0f, -64.0f, 83.0f, 53.0f, 23.0f, 32.0f, 83.0f, -46.0f})));
		}

		SECTION("matrix times matrix")
		{
			auto m1 = Matrix3f::from({2.0f, -5.0f, 3.0f, 7.0f, 1.0f, -6.0f, -9.0f, 4.0f, 8.0f});
			auto m2 = Matrix3f::from({-14.0f, 96.0f, 45.0f, -140.0f, 66.0f, 96.0f, -51.0f, 36.0f, -33.0f});
			auto m4 = Matrix3f::from({519.0f, -30.0f, -489.0f, 68.0f, 522.0f, 609.0f, -842.0f, -312.0f, -285.0f});

			auto m3 = m1 * m2;
			REQUIRE_THAT(m3, Matrix3Equals(m4));
			m3 = m1; m3 *= m2;
			REQUIRE_THAT(m3, Matrix3Equals(m4));

			m1 = Matrix3f::from({0.24f, 0.0082f, -0.3f, 0.6f, -1.4f, 0.73f, 0.38f, 0.096f, -0.16f});
			m2 = Matrix3f::from({1.7f, 0.003f, -5.6f, 0.44f, 0.2082f, -0.1f, 0.8f, -1.2f, 0.93f});
			m4 = Matrix3f::from({0.1716080f, 0.3624272f, -1.6238200f, 0.9880000f, -1.1656800f, -2.5411000f, 0.5602400f, 0.2131272f, -2.2864000f});
			m3 = m1 * m2;
			REQUIRE_THAT(m3, Matrix3Equals(m4));
			m3 = m1; m3 *= m2;
			REQUIRE_THAT(m3, Matrix3Equals(m4));

			m1 = Matrix3f::from({-27.0f, 83.0f, 32.0f, -153.0f, 53.0f, 83.0f, -64.0f, 23.0f, -46.0f});
			m2 = Matrix3f::from({5.0f, -2.0f, 6.0f, 10.0f, 4.0f, -3.0f, -6.0f, 7.0f, 11.0f});
			m4 = Matrix3f::from({503.0f, 610.0f, -59.0f, -733.0f, 1099.0f, -164.0f, 186.0f, -102.0f, -959.0f});
			m3 = m1 * m2;
			REQUIRE_THAT(m3, Matrix3Equals(m4));
			m3 = m1; m3 *= m2;
			REQUIRE_THAT(m3, Matrix3Equals(m4));
		}

		SECTION("matrix from quaternion")
		{
			//identity
			Quaternion q;
			auto m = Matrix3f::from(q);
			REQUIRE_THAT(q.unitRotate(Vector3f{1.0f, 1.0f, 1.0f}), VectorEquals(1.0f, 1.0f, 1.0f));
			REQUIRE_THAT(m.transform(Vector3f{1.0f, 1.0f, 1.0f}), VectorEquals(1.0f, 1.0f, 1.0f));

			//along the X axis
			q = Quaternion::genAxisAngle(Vector3f{1.0f, 0.0f, 0.0f}, 90.0f);
			m = Matrix3f::from(q);
			REQUIRE_THAT(q.unitRotate(Vector3f{0.0f, 1.0f, 0.0f}), VectorEquals(0.0f, 0.0f, 1.0f));
			REQUIRE_THAT(m.transform(Vector3f{0.0f, 1.0f, 0.0f}), VectorEquals(0.0f, 0.0f, 1.0f));

			q = Quaternion::genAxisAngle(Vector3f{1.0f, 0.0f, 0.0f}, 180.0f);
			m = Matrix3f::from(q);
			REQUIRE_THAT(q.unitRotate(Vector3f{0.0f, 1.0f, 0.0f}), VectorEquals(0.0f, -1.0f, 0.0f));
			REQUIRE_THAT(m.transform(Vector3f{0.0f, 1.0f, 0.0f}), VectorEquals(0.0f, -1.0f, 0.0f));

			q = Quaternion::genAxisAngle(Vector3f{1.0f, 0.0f, 0.0f}, 270.0f);
			m = Matrix3f::from(q);
			REQUIRE_THAT(q.unitRotate(Vector3f{0.0f, 1.0f, 0.0f}), VectorEquals(0.0f, 0.0f, -1.0f));
			REQUIRE_THAT(m.transform(Vector3f{0.0f, 1.0f, 0.0f}), VectorEquals(0.0f, 0.0f, -1.0f));

			//along the Y axis
			q = Quaternion::genAxisAngle(Vector3f{0.0f, 1.0f, 0.0f}, 90.0f);
			m = Matrix3f::from(q);
			REQUIRE_THAT(q.unitRotate(Vector3f{1.0f, 0.0f, 0.0f}), VectorEquals(0.0f, 0.0f, -1.0f));
			REQUIRE_THAT(m.transform(Vector3f{1.0f, 0.0f, 0.0f}), VectorEquals(0.0f, 0.0f, -1.0f));

			q = Quaternion::genAxisAngle(Vector3f{0.0f, 1.0f, 0.0f}, 180.0f);
			m = Matrix3f::from(q);
			REQUIRE_THAT(q.unitRotate(Vector3f{1.0f, 0.0f, 0.0f}), VectorEquals(-1.0f, 0.0f, 0.0f));
			REQUIRE_THAT(m.transform(Vector3f{1.0f, 0.0f, 0.0f}), VectorEquals(-1.0f, 0.0f, 0.0f));

			q = Quaternion::genAxisAngle(Vector3f{0.0f, 1.0f, 0.0f}, 270.0f);
			m = Matrix3f::from(q);
			REQUIRE_THAT(q.unitRotate(Vector3f{1.0f, 0.0f, 0.0f}), VectorEquals(0.0f, 0.0f, 1.0f));
			REQUIRE_THAT(m.transform(Vector3f{1.0f, 0.0f, 0.0f}), VectorEquals(0.0f, 0.0f, 1.0f));

			//along the Z axis
			q = Quaternion::genAxisAngle(Vector3f{0.0f, 0.0f, 1.0f}, 90.0f);
			m = Matrix3f::from(q);
			REQUIRE_THAT(q.unitRotate(Vector3f{0.0f, 1.0f, 0.0f}), VectorEquals(-1.0f, 0.0f, 0.0f));
			REQUIRE_THAT(m.transform(Vector3f{0.0f, 1.0f, 0.0f}), VectorEquals(-1.0f, 0.0f, 0.0f));

			q = Quaternion::genAxisAngle(Vector3f{0.0f, 0.0f, 1.0f}, 180.0f);
			m = Matrix3f::from(q);
			REQUIRE_THAT(q.unitRotate(Vector3f{0.0f, 1.0f, 0.0f}), VectorEquals(0.0f, -1.0f, 0.0f));
			REQUIRE_THAT(m.transform(Vector3f{0.0f, 1.0f, 0.0f}), VectorEquals(0.0f, -1.0f, 0.0f));

			q = Quaternion::genAxisAngle(Vector3f{0.0f, 0.0f, 1.0f}, 270.0f);
			m = Matrix3f::from(q);
			REQUIRE_THAT(q.unitRotate(Vector3f{0.0f, 1.0f, 0.0f}), VectorEquals(1.0f, 0.0f, 0.0f));
			REQUIRE_THAT(m.transform(Vector3f{0.0f, 1.0f, 0.0f}), VectorEquals(1.0f, 0.0f, 0.0f));
		}

		SECTION("matrix multiplication order")
		{
			auto q1 = Quaternion::genAxisAngle(Vector3f{0.0f, 1.0f, 0.0f}, 90.0f);
			auto q2 = Quaternion::genAxisAngle(Vector3f{1.0f, 0.0f, 0.0f}, 90.0f);

			auto m = Matrix3f::from(q1) * Matrix3f::from(q2);
			REQUIRE_THAT(m.transform(Vector3f{1.0f, 0.0f, 0.0f}), VectorEquals(0.0f, 1.0f, 0.0f));
			REQUIRE_THAT(m.transform(Vector3f{-0.9f, 0.0f, 0.0f}), VectorEquals(0.0f, -0.9f, 0.0f));
		}
	}
}
