#pragma once

#include "utils_t.hpp"

#include "matrix.hpp"

#include "libs/catch2/catch.hpp"

namespace hr::utests
{
	TEST_CASE("Matrix 4x4 of type float", "[common][matrix][matrix4x4f]")
	{
		SECTION("init")
		{
			const float matIdentityF[]{ 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f };
			REQUIRE_THAT(Matrix4f::identity(), Matrix4Equals(matIdentityF));

			const double matIdentityD[]{1.0, 0.0, 0.0, 0.0, 0.0, 1.0, 0.0, 0.0, 0.0, 0.0, 1.0, 0.0, 0.0, 0.0, 0.0, 1.0};
			REQUIRE_THAT(Matrix4d::identity(), Matrix4Equals(matIdentityD));
		}

		SECTION("convert")
		{
			const float matData4[]{0.6f, 0.2f, 0.3f, 0.4f, 0.2f, 0.7f, 0.5f, 0.3f, 0.3f, 0.5f, 0.7f, 0.2f, 0.4f, 0.3f, 0.2f, 0.6f};
			const float matData3[]{0.6f, 0.2f, 0.3f, 0.2f, 0.7f, 0.5f, 0.3f, 0.5f, 0.7f};
			auto m4 = Matrix4f::from(std::span<const float>(matData4, 16));
			auto m3 = m4.convert<Matrix3, float>();

			REQUIRE_THAT(m4, Matrix4Equals(matData4));
			REQUIRE_THAT(m3, Matrix3Equals(matData3));
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

		SECTION("convert")
		{
			const float matData3[]{0.6f, 0.2f, 0.3f, 0.2f, 0.7f, 0.5f, 0.3f, 0.5f, 0.7f};
			const float matData4[]{0.6f, 0.2f, 0.3f, 0.0f, 0.2f, 0.7f, 0.5f, 0.0f, 0.3f, 0.5f, 0.7f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f};

			auto m3 = Matrix3f::from(std::span<const float>(matData3, 9));
			auto m4 = m3.convert<Matrix4, float>();

			REQUIRE_THAT(m3, Matrix3Equals(matData3));
			REQUIRE_THAT(m4, Matrix4Equals(matData4));
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
