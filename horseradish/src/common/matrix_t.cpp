#pragma once

#include "utils_t.hpp"

#include "matrix.hpp"
#include "random.hpp"

#include <catch2/catch_test_macros.hpp>
#include <catch2/benchmark/catch_benchmark.hpp>

namespace hr::utests
{
	namespace
	{
		float randomScalarf()
		{
			return static_cast<float>(Random{}.nextDouble(-10.0, 10.0));
		}

		float randomScalarf(float min, float max)
		{
			return static_cast<float>(Random{}.nextDouble(min, max));
		}

		double randomScalard()
		{
			return Random{}.nextDouble(-10.0, 10.0);
		}

		double randomScalard(double min, double max)
		{
			return Random{}.nextDouble(min, max);
		}

		Matrix3f randomMatrix3f()
		{
			Random r;

			auto mat = Matrix3f::naked();
			for (int i = 0; i < 9; i++)
				mat[i] = static_cast<float>(r.nextDouble(-10.0, 10.0));

			return mat;
		}

		Matrix3d randomMatrix3d()
		{
			Random r;

			auto mat = Matrix3d::naked();
			for (int i = 0; i < 9; i++)
				mat[i] = r.nextDouble(-10.0, 10.0);

			return mat;
		}

		Matrix4f randomMatrix4f()
		{
			Random r;

			auto mat = Matrix4f::naked();
			for (int i = 0; i < 16; i++)
				mat[i] = static_cast<float>(r.nextDouble(-10.0, 10.0));

			return mat;
		}

		Matrix4d randomMatrix4d()
		{
			Random r;

			auto mat = Matrix4d::naked();
			for (int i = 0; i < 16; i++)
				mat[i] = r.nextDouble(-10.0, 10.0);

			return mat;
		}

		void randomVectors(std::vector<Vector3f> vectors)
		{
			Random r;

			for (auto& vec : vectors)
				vec = Vector3f{static_cast<float>(r.nextDouble(-10.0, 10.0)), static_cast<float>(r.nextDouble(-10.0, 10.0)), static_cast<float>(r.nextDouble(-10.0, 10.0))};
		}

		void randomVectors(std::vector<Vector4f> vectors)
		{
			Random r;

			for (auto& vec : vectors)
				vec = Vector4f{static_cast<float>(r.nextDouble(-10.0, 10.0)), static_cast<float>(r.nextDouble(-10.0, 10.0)), static_cast<float>(r.nextDouble(-10.0, 10.0)), static_cast<float>(r.nextDouble(-10.0, 10.0))};
		}

		void randomVectors(std::vector<Vector3d> vectors)
		{
			Random r;

			for (auto& vec : vectors)
				vec = Vector3d{r.nextDouble(-10.0, 10.0), r.nextDouble(-10.0, 10.0), r.nextDouble(-10.0, 10.0)};
		}

		void randomVectors(std::vector<Vector4d> vectors)
		{
			Random r;

			for (auto& vec : vectors)
				vec = Vector4d{r.nextDouble(-10.0, 10.0), r.nextDouble(-10.0, 10.0), r.nextDouble(-10.0, 10.0), r.nextDouble(-10.0, 10.0)};
		}
	}

	TEST_CASE("Matrix 4x4 of type float", "[common][matrix][matrix4x4f]")
	{
		SECTION("init")
		{
			const float matIdentity[]{1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f};
			REQUIRE_THAT(Matrix4f::identity(), Matrix4Equals(matIdentity));

			const float matZero[]{0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f};
			REQUIRE_THAT(Matrix4f::zero(), Matrix4Equals(matZero));

			const float matScalar[]{1.12345f, 1.12345f, 1.12345f, 1.12345f, 1.12345f, 1.12345f, 1.12345f, 1.12345f, 1.12345f, 1.12345f, 1.12345f, 1.12345f, 1.12345f, 1.12345f, 1.12345f, 1.12345f};
			REQUIRE_THAT(Matrix4f::from(1.12345f), Matrix4Equals(matScalar));

			const float matRandom[]{2.0f, -5.0f, 3.0f, 7.0f, 1.0f, -6.0f, -9.0f, 4.0f, 8.0f, -90.0f, 60.0f, -40.0f, 25.0f, 12.0f, -91.0f, -48.0f};
			REQUIRE_THAT(Matrix4f::from({2.0f, -5.0f, 3.0f, 7.0f, 1.0f, -6.0f, -9.0f, 4.0f, 8.0f, -90.0f, 60.0f, -40.0f, 25.0f, 12.0f, -91.0f, -48.0f}), Matrix4Equals(matRandom));

			const float matF[]{0.11679f, 12.1245f, -12.0f, 91.0f, 48.0f, -133.0f, 67.0f, 90.0f, -60.0f, 40.0f, -25.0f, 0.0f, 0.5911f, 9.1244f, 0.0004224f, 22.6677f};
			const double matD[]{-60.0, 40.0, -25.0, 0.0, 0.5911, 9.1244, 0.0004224, 22.6677, -90.0, 60.0, -40.0, 25.0, 12.0, -91.0, -48.0, 133.0};
			const float matDasF[]{-60.0f, 40.0f, -25.0f, 0.0f, 0.5911f, 9.1244f, 0.0004224f, 22.6677f, -90.0f, 60.0f, -40.0f, 25.0f, 12.0f, -91.0f, -48.0f, 133.0f};
			REQUIRE_THAT(Matrix4f::from<float>(matF), Matrix4Equals(matF));
			REQUIRE_THAT(Matrix4f::from<double>(matD), Matrix4Equals(matDasF));
		}

		SECTION("access")
		{
			const float matRandom[]{2.0f, -5.0f, 3.0f, 7.0f, 1.0f, -6.0f, -9.0f, 4.0f, 8.0f, -90.0f, 60.0f, -40.0f, 25.0f, 12.0f, -91.0f, -48.0f};
			auto mat = Matrix4f::from({2.0f, -5.0f, 3.0f, 7.0f, 1.0f, -6.0f, -9.0f, 4.0f, 8.0f, -90.0f, 60.0f, -40.0f, 25.0f, 12.0f, -91.0f, -48.0f});

			for (int i = 0; i < 16; i++)
				REQUIRE(mat[i] == Catch::Approx(matRandom[i]));

			for (int i = 0; i < 16; i++)
				REQUIRE(mat[i + 16] == Catch::Approx(matRandom[i]));

			for (int i = 0; i < 16; i++)
				REQUIRE(mat[i + 32] == Catch::Approx(matRandom[i]));

			for (int i = 0; i < 16; i++)
				REQUIRE(mat.data()[i] == Catch::Approx(matRandom[i]));

			REQUIRE_THAT(mat.getRow(0), VectorEquals(2.0f, -5.0f, 3.0f, 7.0f));
			REQUIRE_THAT(mat.getRow(1), VectorEquals(1.0f, -6.0f, -9.0f, 4.0f));
			REQUIRE_THAT(mat.getRow(2), VectorEquals(8.0f, -90.0f, 60.0f, -40.0f));
			REQUIRE_THAT(mat.getRow(3), VectorEquals(25.0f, 12.0f, -91.0f, -48.0f));
			REQUIRE_THAT(mat.getRow(4), VectorEquals(2.0f, -5.0f, 3.0f, 7.0f));
			REQUIRE_THAT(mat.getRow(5), VectorEquals(1.0f, -6.0f, -9.0f, 4.0f));
			REQUIRE_THAT(mat.getRow(6), VectorEquals(8.0f, -90.0f, 60.0f, -40.0f));
			REQUIRE_THAT(mat.getRow(7), VectorEquals(25.0f, 12.0f, -91.0f, -48.0f));
			REQUIRE_THAT(mat.getRow(11), VectorEquals(25.0f, 12.0f, -91.0f, -48.0f));

			REQUIRE_THAT(mat.getColumn(0), VectorEquals(2.0f, 1.0f, 8.0f, 25.0f));
			REQUIRE_THAT(mat.getColumn(1), VectorEquals(-5.0f, -6.0f, -90.0f, 12.0f));
			REQUIRE_THAT(mat.getColumn(2), VectorEquals(3.0f, -9.0f, 60.0f, -91.0f));
			REQUIRE_THAT(mat.getColumn(3), VectorEquals(7.0f, 4.0f, -40.0f, -48.0f));
		}

		SECTION("misc")
		{
			auto m = randomMatrix4f();

			float buffer[16];
			m.write(buffer);
			REQUIRE_THAT(m, Matrix4Equals(buffer));
		}

		SECTION("convert")
		{
			const float matData3f[]{0.11679f, 12.1245f, -12.0f, 48.0f, -133.0f, 67.0f, -60.0f, 40.0f, -25.0f};
			const double matData3d[]{0.11679, 12.1245, -12.0, 48.0, -133.0, 67.0, -60.0, 40.0, -25.0};
			const float matData4f[]{0.11679f, 12.1245f, -12.0f, 91.0f, 48.0f, -133.0f, 67.0f, 90.0f, -60.0f, 40.0f, -25.0f, 0.0f, 0.5911f, 9.1244f, 0.0004224f, 22.6677f};
			const double matData4d[]{0.11679, 12.1245, -12.0, 91.0, 48.0, -133.0, 67.0, 90.0, -60.0, 40.0, -25.0, 0.0, 0.5911, 9.1244, 0.0004224, 22.6677};

			auto m4 = Matrix4f::from(std::span<const float>(matData4f, 16));
			REQUIRE_THAT(m4, Matrix4Equals(matData4f));

			auto m3f = m4.convert<Matrix3, float>();
			auto m3d = m4.convert<Matrix3, double>();
			REQUIRE_THAT(m3f, Matrix3Equals(matData3f));
			REQUIRE_THAT(m3d, Matrix3Equals(matData3d));

			auto m4f = m4.convert<Matrix4, float>();
			auto m4d = m4.convert<Matrix4, double>();
			REQUIRE_THAT(m4f, Matrix4Equals(matData4f));
			REQUIRE_THAT(m4d, Matrix4Equals(matData4d));
		}

		SECTION("matrix plus matrix")
		{
			{
				auto m1 = randomMatrix4f();
				auto m2 = randomMatrix4f();
				auto mRes = Matrix4f::naked();
				for (int i = 0; i < 16; i++)
					mRes[i] = m1[i] + m2[i];

				auto m3 = m1 + m2;
				REQUIRE_THAT(m3, Matrix4Equals(mRes));
				m3 = m1;
				m3 += m2;
				REQUIRE_THAT(m3, Matrix4Equals(mRes));
				m3 = m1;
				m3 += m2.data();
				REQUIRE_THAT(m3, Matrix4Equals(mRes));
			}

			{
				auto m1 = randomMatrix4f();
				auto m2 = randomMatrix4f();
				auto mRes = Matrix4f::naked();
				for (int i = 0; i < 16; i++)
					mRes[i] = m1[i] + m2[i];

				auto m3 = m1 + m2;
				REQUIRE_THAT(m3, Matrix4Equals(mRes));
				m3 = m1;
				m3 += m2;
				REQUIRE_THAT(m3, Matrix4Equals(mRes));
				m3 = m1;
				m3 += m2.data();
				REQUIRE_THAT(m3, Matrix4Equals(mRes));
			}
		}

		SECTION("matrix minus matrix")
		{
			{
				auto m1 = randomMatrix4f();
				auto m2 = randomMatrix4f();
				auto mRes = Matrix4f::naked();
				for (int i = 0; i < 16; i++)
					mRes[i] = m1[i] - m2[i];

				auto m3 = m1 - m2;
				REQUIRE_THAT(m3, Matrix4Equals(mRes));
				m3 = m1;
				m3 -= m2;
				REQUIRE_THAT(m3, Matrix4Equals(mRes));
				m3 = m1;
				m3 -= m2.data();
				REQUIRE_THAT(m3, Matrix4Equals(mRes));
			}

			{
				auto m1 = randomMatrix4f();
				auto m2 = randomMatrix4f();
				auto mRes = Matrix4f::naked();
				for (int i = 0; i < 16; i++)
					mRes[i] = m1[i] - m2[i];

				auto m3 = m1 - m2;
				REQUIRE_THAT(m3, Matrix4Equals(mRes));
				m3 = m1;
				m3 -= m2;
				REQUIRE_THAT(m3, Matrix4Equals(mRes));
				m3 = m1;
				m3 -= m2.data();
				REQUIRE_THAT(m3, Matrix4Equals(mRes));
			}
		}

		SECTION("matrix op scalar")
		{
			{
				auto m = randomMatrix4f();
				auto scalar = randomScalarf();
				auto mRes = Matrix4f::naked();
				for (int i = 0; i < 16; i++)
					mRes[i] = m[i] * scalar;

				auto m2 = m * scalar;
				REQUIRE_THAT(m2, Matrix4Equals(mRes));
				m2 = m;
				m2 *= scalar;
				REQUIRE_THAT(m2, Matrix4Equals(mRes));
			}

			{
				auto m = randomMatrix4f();
				auto scalar = randomScalarf();
				auto mRes = Matrix4f::naked();
				for (int i = 0; i < 16; i++)
					mRes[i] = m[i] * scalar;

				auto m2 = m * scalar;
				REQUIRE_THAT(m2, Matrix4Equals(mRes));
				m2 = m;
				m2 *= scalar;
				REQUIRE_THAT(m2, Matrix4Equals(mRes));
			}
		}

		SECTION("matrix times matrix")
		{
			{
				auto m1 = Matrix4f::from({1.0f, 2.0f, 3.0f, 4.0f, 5.0f, 6.0f, 7.0f, 8.0f, 1.0f, 2.0f, 3.0f, 4.0f, 5.0f, 6.0f, 7.0f, 8.0f});
				auto m2 = Matrix4f::from({1.0f, 2.0f, 3.0f, 4.0f, 5.0f, 6.0f, 7.0f, 8.0f, 1.0f, 2.0f, 3.0f, 4.0f, 5.0f, 6.0f, 7.0f, 8.0f});
				auto mRes = Matrix4f::from({34.0f, 44.0f, 54.0f, 64.0f, 82.0f, 108.0f, 134.0f, 160.0f, 34.0f, 44.0f, 54.0f, 64.0f, 82.0f, 108.0f, 134.0f, 160.0f});

				auto m3 = m1 * m2;
				REQUIRE_THAT(m3, Matrix4Equals(mRes));
				m3 = m1;
				m3 *= m2;
				REQUIRE_THAT(m3, Matrix4Equals(mRes));
				m3 = m1;
				m3 *= m2.data();
				REQUIRE_THAT(m3, Matrix4Equals(mRes));
			}

			{
				auto m = Matrix4f::from({1.0f, 2.0f, 3.0f, 4.0f, 5.0f, 6.0f, 7.0f, 8.0f, 9.0f, 10.0f, 11.0f, 12.0f, 13.0f, 14.0f, 15.0f, 16.0f});

				auto mRes = Matrix4f::naked();
				{
					const auto& a = m;
					const auto& b = m;

					mRes[0] = (a[0] * b[0] + a[1] * b[4] + a[2] * b[8] + a[3] * b[12]);
					mRes[1] = (a[0] * b[1] + a[1] * b[5] + a[2] * b[9] + a[3] * b[13]);
					mRes[2] = (a[0] * b[2] + a[1] * b[6] + a[2] * b[10] + a[3] * b[14]);
					mRes[3] = (a[0] * b[3] + a[1] * b[7] + a[2] * b[11] + a[3] * b[15]);

					mRes[4] = (a[4] * b[0] + a[5] * b[4] + a[6] * b[8] + a[7] * b[12]);
					mRes[5] = (a[4] * b[1] + a[5] * b[5] + a[6] * b[9] + a[7] * b[13]);
					mRes[6] = (a[4] * b[2] + a[5] * b[6] + a[6] * b[10] + a[7] * b[14]);
					mRes[7] = (a[4] * b[3] + a[5] * b[7] + a[6] * b[11] + a[7] * b[15]);

					mRes[8] = (a[8] * b[0] + a[9] * b[4] + a[10] * b[8] + a[11] * b[12]);
					mRes[9] = (a[8] * b[1] + a[9] * b[5] + a[10] * b[9] + a[11] * b[13]);
					mRes[10] = (a[8] * b[2] + a[9] * b[6] + a[10] * b[10] + a[11] * b[14]);
					mRes[11] = (a[8] * b[3] + a[9] * b[7] + a[10] * b[11] + a[11] * b[15]);

					mRes[12] = (a[12] * b[0] + a[13] * b[4] + a[14] * b[8] + a[15] * b[12]);
					mRes[13] = (a[12] * b[1] + a[13] * b[5] + a[14] * b[9] + a[15] * b[13]);
					mRes[14] = (a[12] * b[2] + a[13] * b[6] + a[14] * b[10] + a[15] * b[14]);
					mRes[15] = (a[12] * b[3] + a[13] * b[7] + a[14] * b[11] + a[15] * b[15]);
				}

				auto m2 = m * m;
				REQUIRE_THAT(m2, Matrix4Equals(mRes));
				m2 = m;
				m2 *= m;
				REQUIRE_THAT(m2, Matrix4Equals(mRes));
				m2 = m;
				m2 *= m.data();
				REQUIRE_THAT(m2, Matrix4Equals(mRes));
			}
		}

		SECTION("matrix times misc")
		{
			auto m = randomMatrix4f();

			//matrix3
			{
				auto m3 = randomMatrix3f();

				auto mActual = m;
				mActual *= m3;

				auto mExpected = m;
				mExpected *= m3.convert<Matrix4, float>();

				REQUIRE_THAT(mActual, Matrix4Equals(mExpected));
			}

			//span of a 4x4 matrix
			{
				auto m4 = randomMatrix4f();

				auto mActual = m;
				mActual *= m4.data();

				auto mExpected = m;
				mExpected *= m4.convert<Matrix4, float>();

				REQUIRE_THAT(mActual, Matrix4Equals(mExpected));
			}

			//quaternion
			{
				auto q = Quaternionf::fromEuler(11.0f, 22.0f, 33.0f, Quaternionf::AxisOrder::YZX);

				auto mActual = m;
				mActual *= q;

				auto mExpected = m;
				mExpected *= Matrix4f::rotation(q);

				REQUIRE_THAT(mActual, Matrix4Equals(mExpected));
			}
		}

		SECTION("determinant")
		{
			REQUIRE(Matrix4f::identity().determinant() == Catch::Approx(1.0f));

			{
				auto m = Matrix4f::rotationX(30.0f);
				m *= Matrix4f::rotationY(30.0f);
				m *= Matrix4f::rotationZ(30.0f);
				REQUIRE(m.determinant() == Catch::Approx(1.0f));
			}

			{
				auto m = Matrix4f::from({6.0f, 1.0f, 1.0f, 0.0f, 4.0f, -2.0f, 5.0f, 0.0f, 2.0f, 8.0f, 7.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f});
				REQUIRE(m.determinant() == Catch::Approx(-306.0f));
			}

			{
				auto m = Matrix4f::from({2.0f, 3.0f, 4.0f, 0.0f, 1.0f, 2.0f, -3.0f, 0.0f, 1.0f, 1.0f, 5.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f});
				REQUIRE(m.determinant() == Catch::Approx(-2.0f));
			}

			{
				// determinant(A) = 1 / determinant(inverse(A))

				auto m = Matrix4f::from({5.0f, 2.0f, 8.25f, 1.0f, 12.0f, 6.8f, 2.14f, 9.6f, 6.5f, 1.0f, 3.14f, 2.22f, 0.0f, 0.86f, 4.0f, 1.0f});

				auto mI = m;
				mI.inverse();

				auto dM = m.determinant();
				auto dMI = mI.determinant();

				REQUIRE(dM == Catch::Approx(1.0f / dMI));
			}
		}

		SECTION("transpose")
		{
			{
				auto m = Matrix4f::identity();
				m.inverse();
				REQUIRE_THAT(m, Matrix4Equals(Matrix4f::identity()));
			}

			{
				auto m = randomMatrix4f();

				auto mT = m;
				mT.transpose();

				REQUIRE(m[0] == Catch::Approx(mT[0]));
				REQUIRE(m[1] == Catch::Approx(mT[4]));
				REQUIRE(m[2] == Catch::Approx(mT[8]));
				REQUIRE(m[3] == Catch::Approx(mT[12]));
				REQUIRE(m[4] == Catch::Approx(mT[1]));
				REQUIRE(m[5] == Catch::Approx(mT[5]));
				REQUIRE(m[6] == Catch::Approx(mT[9]));
				REQUIRE(m[7] == Catch::Approx(mT[13]));
				REQUIRE(m[8] == Catch::Approx(mT[2]));
				REQUIRE(m[9] == Catch::Approx(mT[6]));
				REQUIRE(m[10] == Catch::Approx(mT[10]));
				REQUIRE(m[11] == Catch::Approx(mT[14]));
				REQUIRE(m[12] == Catch::Approx(mT[3]));
				REQUIRE(m[13] == Catch::Approx(mT[7]));
				REQUIRE(m[14] == Catch::Approx(mT[11]));
				REQUIRE(m[15] == Catch::Approx(mT[15]));
			}
		}

		SECTION("matrix translation")
		{
			{
				auto mRes = Matrix4f::from({1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 2.0f, -3.0f, 4.0f, 1.0f});

				REQUIRE_THAT(Matrix4f::translation(2.0f, -3.0f, 4.0f), Matrix4Equals(mRes));
				REQUIRE_THAT(Matrix4f::translation(Vector3f{2.0f, -3.0f, 4.0f}), Matrix4Equals(mRes));
			}
		}

		SECTION("matrix scale")
		{
			{
				auto mRes = Matrix4f::from({-2.0f, 0.0f, 0.0f, 0.0f, 0.0f, -2.0f, 0.0f, 0.0f, 0.0f, 0.0f, -2.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f});

				REQUIRE_THAT(Matrix4f::scale(-2.0f), Matrix4Equals(mRes));
				REQUIRE_THAT(Matrix4f::scale(-2.0f, -2.0f, -2.0f), Matrix4Equals(mRes));
				REQUIRE_THAT(Matrix4f::scale(Vector3f{-2.0f, -2.0f, -2.0f}), Matrix4Equals(mRes));
			}

			{
				auto mRes = Matrix4f::from({-2.0f, 0.0f, 0.0f, 0.0f, 0.0f, 3.0f, 0.0f, 0.0f, 0.0f, 0.0f, -4.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f});

				REQUIRE_THAT(Matrix4f::scale(-2.0f, 3.0f, -4.0f), Matrix4Equals(mRes));
				REQUIRE_THAT(Matrix4f::scale(Vector3f{-2.0f, 3.0f, -4.0f}), Matrix4Equals(mRes));
			}
		}

		SECTION("matrix rotation")
		{
			//along the X axis
			auto m = Matrix4f::rotation(Vector3f{1.0f, 0.0f, 0.0f}, 90.0f);
			auto m2 = Matrix4f::rotationX(90.0f);
			REQUIRE_THAT(m, Matrix4Equals(m2));
			REQUIRE_THAT(m.transformCopy(Vector3f{0.0f, 1.0f, 0.0f}), VectorEquals(0.0f, 0.0f, 1.0f));
			REQUIRE_THAT(Matrix4f::rotationX(30.0f), Matrix4Equals(Matrix4f::from({1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.8660254f, 0.5f, 0.0f, 0.0f, -0.5f, 0.8660254f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f})));

			m = Matrix4f::rotation(Vector3f{1.0f, 0.0f, 0.0f}, 180.0f);
			m2 = Matrix4f::rotationX(180.0f);
			REQUIRE_THAT(m, Matrix4Equals(m2));
			REQUIRE_THAT(m.transformCopy(Vector3f{0.0f, 1.0f, 0.0f}), VectorEquals(0.0f, -1.0f, 0.0f));

			m = Matrix4f::rotation(Vector3f{1.0f, 0.0f, 0.0f}, 270.0f);
			m2 = Matrix4f::rotationX(270.0f);
			REQUIRE_THAT(m, Matrix4Equals(m2));
			REQUIRE_THAT(m.transformCopy(Vector3f{0.0f, 1.0f, 0.0f}), VectorEquals(0.0f, 0.0f, -1.0f));

			//along the Y axis
			m = Matrix4f::rotation(Vector3f{0.0f, 1.0f, 0.0f}, 90.0f);
			m2 = Matrix4f::rotationY(90.0f);
			REQUIRE_THAT(m, Matrix4Equals(m2));
			REQUIRE_THAT(m.transformCopy(Vector3f{1.0f, 0.0f, 0.0f}), VectorEquals(0.0f, 0.0f, -1.0f));
			REQUIRE_THAT(Matrix4f::rotationY(60.0f), Matrix4Equals(Matrix4f::from({0.49999997f, 0.0f, -0.866025448f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.866025448f, 0.0f, 0.49999997f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f})));

			m = Matrix4f::rotation(Vector3f{0.0f, 1.0f, 0.0f}, 180.0f);
			m2 = Matrix4f::rotationY(180.0f);
			REQUIRE_THAT(m, Matrix4Equals(m2));
			REQUIRE_THAT(m.transformCopy(Vector3f{1.0f, 0.0f, 0.0f}), VectorEquals(-1.0f, 0.0f, 0.0f));

			m = Matrix4f::rotation(Vector3f{0.0f, 1.0f, 0.0f}, 270.0f);
			m2 = Matrix4f::rotationY(270.0f);
			REQUIRE_THAT(m, Matrix4Equals(m2));
			REQUIRE_THAT(m.transformCopy(Vector3f{1.0f, 0.0f, 0.0f}), VectorEquals(0.0f, 0.0f, 1.0f));

			//along the Z axis
			m = Matrix4f::rotation(Vector3f{0.0f, 0.0f, 1.0f}, 90.0f);
			m2 = Matrix4f::rotationZ(90.0f);
			REQUIRE_THAT(m, Matrix4Equals(m2));
			REQUIRE_THAT(m.transformCopy(Vector3f{0.0f, 1.0f, 0.0f}), VectorEquals(-1.0f, 0.0f, 0.0f));
			REQUIRE_THAT(Matrix4f::rotationZ(50.0f), Matrix4Equals(Matrix4f::from({0.642787635f, 0.766044438f, 0.0f, 0.0f, -0.766044438f, 0.642787635f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f})));

			m = Matrix4f::rotation(Vector3f{0.0f, 0.0f, 1.0f}, 180.0f);
			m2 = Matrix4f::rotationZ(180.0f);
			REQUIRE_THAT(m, Matrix4Equals(m2));
			REQUIRE_THAT(m.transformCopy(Vector3f{0.0f, 1.0f, 0.0f}), VectorEquals(0.0f, -1.0f, 0.0f));

			m = Matrix4f::rotation(Vector3f{0.0f, 0.0f, 1.0f}, 270.0f);
			m2 = Matrix4f::rotationZ(270.0f);
			REQUIRE_THAT(m, Matrix4Equals(m2));
			REQUIRE_THAT(m.transformCopy(Vector3f{0.0f, 1.0f, 0.0f}), VectorEquals(1.0f, 0.0f, 0.0f));
		}

		SECTION("matrix rotation (quaternion)")
		{
			//identity
			auto q = Quaternionf::identity();
			auto m = Matrix4f::rotation(q);
			REQUIRE_THAT(q.unitRotate(Vector3f{1.0f, 1.0f, 1.0f}), VectorEquals(1.0f, 1.0f, 1.0f));
			REQUIRE_THAT(m.transformCopy(Vector3f{1.0f, 1.0f, 1.0f}), VectorEquals(1.0f, 1.0f, 1.0f));

			//along the X axis
			q = Quaternionf::fromAxisAngle(Vector3f{1.0f, 0.0f, 0.0f}, 90.0f);
			m = Matrix4f::rotation(q);
			REQUIRE_THAT(q.unitRotate(Vector3f{0.0f, 1.0f, 0.0f}), VectorEquals(0.0f, 0.0f, 1.0f));
			REQUIRE_THAT(m.transformCopy(Vector3f{0.0f, 1.0f, 0.0f}), VectorEquals(0.0f, 0.0f, 1.0f));

			q = Quaternionf::fromAxisAngle(Vector3f{1.0f, 0.0f, 0.0f}, 180.0f);
			m = Matrix4f::rotation(q);
			REQUIRE_THAT(q.unitRotate(Vector3f{0.0f, 1.0f, 0.0f}), VectorEquals(0.0f, -1.0f, 0.0f));
			REQUIRE_THAT(m.transformCopy(Vector3f{0.0f, 1.0f, 0.0f}), VectorEquals(0.0f, -1.0f, 0.0f));

			q = Quaternionf::fromAxisAngle(Vector3f{1.0f, 0.0f, 0.0f}, 270.0f);
			m = Matrix4f::rotation(q);
			REQUIRE_THAT(q.unitRotate(Vector3f{0.0f, 1.0f, 0.0f}), VectorEquals(0.0f, 0.0f, -1.0f));
			REQUIRE_THAT(m.transformCopy(Vector3f{0.0f, 1.0f, 0.0f}), VectorEquals(0.0f, 0.0f, -1.0f));

			//along the Y axis
			q = Quaternionf::fromAxisAngle(Vector3f{0.0f, 1.0f, 0.0f}, 90.0f);
			m = Matrix4f::rotation(q);
			REQUIRE_THAT(q.unitRotate(Vector3f{1.0f, 0.0f, 0.0f}), VectorEquals(0.0f, 0.0f, -1.0f));
			REQUIRE_THAT(m.transformCopy(Vector3f{1.0f, 0.0f, 0.0f}), VectorEquals(0.0f, 0.0f, -1.0f));

			q = Quaternionf::fromAxisAngle(Vector3f{0.0f, 1.0f, 0.0f}, 180.0f);
			m = Matrix4f::rotation(q);
			REQUIRE_THAT(q.unitRotate(Vector3f{1.0f, 0.0f, 0.0f}), VectorEquals(-1.0f, 0.0f, 0.0f));
			REQUIRE_THAT(m.transformCopy(Vector3f{1.0f, 0.0f, 0.0f}), VectorEquals(-1.0f, 0.0f, 0.0f));

			q = Quaternionf::fromAxisAngle(Vector3f{0.0f, 1.0f, 0.0f}, 270.0f);
			m = Matrix4f::rotation(q);
			REQUIRE_THAT(q.unitRotate(Vector3f{1.0f, 0.0f, 0.0f}), VectorEquals(0.0f, 0.0f, 1.0f));
			REQUIRE_THAT(m.transformCopy(Vector3f{1.0f, 0.0f, 0.0f}), VectorEquals(0.0f, 0.0f, 1.0f));

			//along the Z axis
			q = Quaternionf::fromAxisAngle(Vector3f{0.0f, 0.0f, 1.0f}, 90.0f);
			m = Matrix4f::rotation(q);
			REQUIRE_THAT(q.unitRotate(Vector3f{0.0f, 1.0f, 0.0f}), VectorEquals(-1.0f, 0.0f, 0.0f));
			REQUIRE_THAT(m.transformCopy(Vector3f{0.0f, 1.0f, 0.0f}), VectorEquals(-1.0f, 0.0f, 0.0f));

			q = Quaternionf::fromAxisAngle(Vector3f{0.0f, 0.0f, 1.0f}, 180.0f);
			m = Matrix4f::rotation(q);
			REQUIRE_THAT(q.unitRotate(Vector3f{0.0f, 1.0f, 0.0f}), VectorEquals(0.0f, -1.0f, 0.0f));
			REQUIRE_THAT(m.transformCopy(Vector3f{0.0f, 1.0f, 0.0f}), VectorEquals(0.0f, -1.0f, 0.0f));

			q = Quaternionf::fromAxisAngle(Vector3f{0.0f, 0.0f, 1.0f}, 270.0f);
			m = Matrix4f::rotation(q);
			REQUIRE_THAT(q.unitRotate(Vector3f{0.0f, 1.0f, 0.0f}), VectorEquals(1.0f, 0.0f, 0.0f));
			REQUIRE_THAT(m.transformCopy(Vector3f{0.0f, 1.0f, 0.0f}), VectorEquals(1.0f, 0.0f, 0.0f));

			//generic
			{
				const float mRes[]{0.875595033f, 0.420031041f, -0.2385524f, 0.0f, -0.38175258f, 0.904303849f, 0.1910483f, 0.0f, 0.295970082f, -0.07621294f, 0.952151954f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f};

				q = Quaternionf::fromAxisAngle(Vector3f{1.0f, 2.0f, 3.0f}.normalize(), 30.0f);
				REQUIRE_THAT(Matrix4f::rotation(q), Matrix4Equals(mRes));
			}
		}

		SECTION("inverse")
		{
			{
				auto m = Matrix4f::identity();
				m.inverse();

				REQUIRE_THAT(m, Matrix4Equals(Matrix4f::identity()));
			}

			{
				auto m = Matrix4f::from({2.0f, 3.0f, 4.0f, 0.0f, 1.0f, 2.0f, -3.0f, 0.0f, 1.0f, 1.0f, 5.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f});
				const float mRes[]{-6.5f, 5.5f, 8.5f, 0.0f, 4.0f, -3.0f, -5.0f, 0.0f, 0.5f, -0.5f, -0.5f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f};

				auto mI = m;
				mI.inverse();
				REQUIRE_THAT(mI, Matrix4Equals(mRes));

				mI *= m; //must produce identity
				REQUIRE_THAT(Matrix4f::identity(), Matrix4Equals(mI));
			}

			{
				auto m = Matrix4f::identity();
				m *= Quaternionf::fromAxisAngle(Vector3f{1.0f, 0.0f, 0.0f}, 30.0f);
				m *= Quaternionf::fromAxisAngle(Vector3f{0.0f, 1.0f, 0.0f}, 30.0f);
				m *= Quaternionf::fromAxisAngle(Vector3f{0.0f, 0.0f, 1.0f}, 30.0f);
				m.inverse();

				const float mRes[]{0.74999994f, -0.216506317f, 0.62499994f, 0.0f, 0.433012635f, 0.87499994f, -0.216506317f, 0.0f, -0.49999997f, 0.433012635f,
				  0.74999994f, 0.0f, 0.0f, 0.0f, 0.0f, 0.99999994f};

				REQUIRE_THAT(m, Matrix4Equals(mRes));
			}

			{
				auto m = Matrix4f::translation(23.0f, 42.0f, 666.0f);

				auto mI = m;
				mI.inverse();
				mI *= m;

				REQUIRE_THAT(mI, Matrix4Equals(Matrix4f::identity()));
			}

			{
				auto m = Matrix4f::scale(23.0f, 42.0f, -666.0f);

				auto mI = m;
				mI.inverse();
				mI *= m;

				REQUIRE_THAT(mI, Matrix4Equals(Matrix4f::identity()));
			}
		}

		SECTION("clone")
		{
			auto m1 = randomMatrix4f();
			REQUIRE_THAT(m1.clone(Matrix4f::CloneTransform::None), Matrix4Equals(m1));

			auto m2 = m1;
			m2.transpose();
			REQUIRE_THAT(m1.clone(Matrix4f::CloneTransform::Transpose), Matrix4Equals(m2));

			m2 = m1;
			m2.inverse();
			REQUIRE_THAT(m1.clone(Matrix4f::CloneTransform::Inverse), Matrix4Equals(m2));

			m2 = m1;
			m2.inverseHomogenous();
			REQUIRE_THAT(m1.clone(Matrix4f::CloneTransform::InverseHomogenous), Matrix4Equals(m2));

			m2 = m1;
			m2.inverseTranspose();
			REQUIRE_THAT(m1.clone(Matrix4f::CloneTransform::InverseTranspose), Matrix4Equals(m2));
		}

		SECTION("transform vector3")
		{
			auto runVariants = [](const Matrix4f& m, const Vector3f& vecSource, const Vector3f& vecExpected)
			{
				//return a copy of vector
				REQUIRE_THAT(m.transformCopy(vecSource), VectorEquals(vecExpected));

				//a vector
				{
					auto vecTmp = vecSource;
					m.transform(vecTmp);

					REQUIRE_THAT(vecTmp, VectorEquals(vecExpected));
				}

				//a span of 3
				{
					auto vecTmp = vecSource;
					m.transform(std::span<float, 3>{vecTmp.data(), 3});

					REQUIRE_THAT(vecTmp, VectorEquals(vecExpected));
				}

				//multiple vectors
				{
					Vector3f vecTmp[19];
					for (auto& vec : vecTmp)
						vec = vecSource;

					m.transform(vecTmp);
					for (auto& vec : vecTmp)
						REQUIRE_THAT(vec, VectorEquals(vecExpected));
				}
			};

			{
				auto m = Matrix4f::identity();

				Vector3f vecSource{1.0f, -2.0f, 3.0f};
				Vector3f vecExpected{1.0f, -2.0f, 3.0f};

				INFO("matrix identity");
				runVariants(m, vecSource, vecExpected);
			}

			{
				auto m = Matrix4f::from({10.0f, 10.0f, 10.0f, 0.0f, 20.0f, 20.0f, 20.0f, 0.0f, 30.0f, 30.0f, 30.0f, 0.0f, 5.0f, 10.0f, 15.0f, 1.0f});

				Vector3f vecSource{20.0f, 30.0f, 40.0f};
				Vector3f vecExpected{2005.0f, 2010.0f, 2015.0f};

				INFO("matrix random");
				runVariants(m, vecSource, vecExpected);
			}

			{
				auto m = Matrix4f::rotationX(30.0f);
				m *= Matrix4f::rotationY(30.0f);
				m *= Matrix4f::rotationZ(30.0f);

				m[12] = 10.0f;
				m[13] = 20.0f;
				m[14] = 30.0f;

				Vector3f vecSource{1.0f, 2.0f, 3.0f};
				Vector3f vecExpected{12.19198728f, 21.53349376f, 32.61602545f};

				INFO("matrix rotation and translation");
				runVariants(m, vecSource, vecExpected);
			}
		}

		SECTION("transform vector4")
		{
			auto runVariants = [](const Matrix4f& m, const Vector4f& vecSource, const Vector4f& vecExpected)
			{
				//return a copy of vector
				REQUIRE_THAT(m.transformCopy(vecSource), VectorEquals(vecExpected));

				//a vector
				{
					auto vecTmp = vecSource;
					m.transform(vecTmp);

					REQUIRE_THAT(vecTmp, VectorEquals(vecExpected));
				}

				//a span of 4
				{
					auto vecTmp = vecSource;
					m.transform(std::span<float, 4>{vecTmp.data(), 4});

					REQUIRE_THAT(vecTmp, VectorEquals(vecExpected));
				}

				//multiple vectors
				{
					Vector4f vecTmp[19];
					for (auto& vec : vecTmp)
						vec = vecSource;

					m.transform(vecTmp);
					for (auto& vec : vecTmp)
						REQUIRE_THAT(vec, VectorEquals(vecExpected));
				}
			};

			{
				auto m = Matrix4f::identity();

				Vector4f vecSource{1.0f, -2.0f, 3.0f, -4.0f};
				Vector4f vecExpected{1.0f, -2.0f, 3.0f, -4.0f};

				INFO("matrix identity");
				runVariants(m, vecSource, vecExpected);
			}

			{
				auto m = Matrix4f::from({10.0f, 10.0f, 10.0f, 0.0f, 20.0f, 20.0f, 20.0f, 0.0f, 30.0f, 30.0f, 30.0f, 0.0f, 5.0f, 10.0f, 15.0f, 1.0f});

				Vector4f vecSource{10.0f, 5.0f, 1.0f, 4.0f};
				Vector4f vecExpected{250.0f, 270.0f, 290.0f, 4.0f};

				INFO("matrix random");
				runVariants(m, vecSource, vecExpected);
			}

			{
				auto m = Matrix4f::rotationX(30.0f);
				m *= Matrix4f::rotationY(30.0f);
				m *= Matrix4f::rotationZ(30.0f);

				m[12] = 10.0f;
				m[13] = 20.0f;
				m[14] = 30.0f;

				Vector4f vecSource1{1.0f, 2.0f, 3.0f, 1.0f};
				Vector4f vecSource2{1.0f, 2.0f, 3.0f, 0.0f};
				Vector4f vecExpected1{12.19198728f, 21.53349376f, 32.61602545f, 1.0f};
				Vector4f vecExpected2{2.19198728f, 1.53349376f, 2.61602545f, 0.0f};

				INFO("matrix rotation and translation (w=1.0)");
				runVariants(m, vecSource1, vecExpected1);

				INFO("matrix rotation and translation (w=0.0)");
				runVariants(m, vecSource2, vecExpected2);
			}
		}

		SECTION("matrix multiplication order")
		{
			auto m = Matrix4f::rotationY(90.0f) * Matrix4f::rotationX(90.0f);
			REQUIRE_THAT(m.transformCopy(Vector3f{1.0f, 0.0f, 0.0f}), VectorEquals(0.0f, 1.0f, 0.0f));
			REQUIRE_THAT(m.transformCopy(Vector3f{-0.9f, 0.0f, 0.0f}), VectorEquals(0.0f, -0.9f, 0.0f));

			m = Matrix4f::rotationY(-90.0f) * Matrix4f::rotationX(90.0f) * Matrix4f::translation(0.5f, 0.0f, 0.5f);
			REQUIRE_THAT(m.transformCopy(Vector3f{1.0f, 0.0f, 0.0f}), VectorEquals(0.5f, -1.0f, 0.5f));
			REQUIRE_THAT(m.transformCopy(Vector3f{-0.9f, 0.0f, 0.0f}), VectorEquals(0.5f, 0.9f, 0.5f));

			m = Matrix4f::translation(1.0f, 0.0f, 0.0f) * Matrix4f::rotationY(90.0f) * Matrix4f::rotationX(90.0f);
			REQUIRE_THAT(m.transformCopy(Vector3f{0.0f, 0.0f, 0.0f}), VectorEquals(0.0f, 1.0f, 0.0f));

			m = Matrix4f::translation(1.0f, 0.0f, 0.0f) * Matrix4f::scale(0.33f) * Matrix4f::rotationY(90.0f) * Matrix4f::rotationX(90.0f);
			REQUIRE_THAT(m.transformCopy(Vector3f{0.0f, 0.0f, 0.0f}), VectorEquals(0.0f, 0.33f, 0.0f));
		}

		//SECTION("lookAt")
		//{
		//	auto m = Matrix4f::glModelView(Vector3f{0.0f, 0.0f, -10.0f}, Vector3f{0.0f, 0.0f, 0.0f}, Vector3f{0.0f, 1.0f, 0.0f});
		//	REQUIRE_THAT(m.transformCopy(Vector3f{0.0f, 0.0f, 1.0f}), VectorEquals(0.0f, 0.0f, -11.0f));
		//	REQUIRE_THAT(m.transformCopy(Vector3f{0.0f, 0.0f, 0.0f}), VectorEquals(0.0f, 0.0f, -10.0f));
		//	REQUIRE_THAT(m.transformCopy(Vector3f{0.0f, 0.0f, -10.0f}), VectorEquals(0.0f, 0.0f, 0.0f));
		//	
		//
		//	m = Matrix4f::glModelView(Vector3f{1.0f, 0.0f, -10.0f}, Vector3f{1.0f, 0.0f, 0.0f}, Vector3f{0.0f, 1.0f, 0.0f});
		//	auto vecView = m.transformCopy(Vector3f{0.0f, 0.0f, 0.0f});
		//	REQUIRE_THAT(vecView, VectorEquals(1.0f, 0.0f, -10.0f));
		//}

		

		SECTION("matrix benchmark")
		{
			constexpr size_t NumVectors{ 5000 };

			BENCHMARK_ADVANCED("vec3 not optimized")(Catch::Benchmark::Chronometer meter)
			{
				auto m = Matrix4f::rotationY(randomScalarf(45.0f, 92.0f)) * Matrix4f::rotationX(randomScalarf(93.0f, 148.0f)) * Matrix4f::translation(0.5f, 0.0f, 0.5f);

				std::vector<Vector3f> vectors;
				vectors.resize(NumVectors);
				randomVectors(vectors);

				meter.measure([&m, &vectors]
				{
					auto mData = m.data();
					float x, y, z;

					for (auto& vec : vectors)
					{
						x = vec[0];
						y = vec[1];
						z = vec[2];

						vec[0] = (x * mData[0]) + (y * mData[4]) + (z * mData[8]) + mData[12];
						vec[1] = (x * mData[1]) + (y * mData[5]) + (z * mData[9]) + mData[13];
						vec[2] = (x * mData[2]) + (y * mData[6]) + (z * mData[10]) + mData[14];
					}

					return vectors.back();
				});
			};

			BENCHMARK_ADVANCED("vec3 matrix optimized")(Catch::Benchmark::Chronometer meter)
			{
				auto m = Matrix4f::rotationY(randomScalarf(45.0f, 92.0f)) * Matrix4f::rotationX(randomScalarf(93.0f, 148.0f)) * Matrix4f::translation(0.5f, 0.0f, 0.5f);

				std::vector<Vector3f> vectors;
				vectors.resize(NumVectors);
				randomVectors(vectors);

				meter.measure([&m, &vectors]
				{
					m.transform(vectors);
					return vectors.back();
				});
			};

			BENCHMARK_ADVANCED("vec4 not optimized")(Catch::Benchmark::Chronometer meter)
			{
				auto m = Matrix4f::rotationY(randomScalarf(45.0f, 92.0f)) * Matrix4f::rotationX(randomScalarf(93.0f, 148.0f)) * Matrix4f::translation(0.5f, 0.0f, 0.5f);

				std::vector<Vector4f> vectors;
				vectors.resize(NumVectors);
				randomVectors(vectors);

				meter.measure([&m, &vectors]
				{
					auto mData = m.data();
					float x, y, z, w;

					for (auto& vec : vectors)
					{
						x = vec[0];
						y = vec[1];
						z = vec[2];
						w = vec[3];

						vec[0] = (x * mData[0]) + (y * mData[4]) + (z * mData[8]) + (w * mData[12]);
						vec[1] = (x * mData[1]) + (y * mData[5]) + (z * mData[9]) + (w * mData[13]);
						vec[2] = (x * mData[2]) + (y * mData[6]) + (z * mData[10]) + (w * mData[14]);
						vec[3] = (x * mData[3]) + (y * mData[7]) + (z * mData[11]) + (w * mData[15]);
					}

					return vectors.back();
				});
			};

			BENCHMARK_ADVANCED("vec4 matrix optimized")(Catch::Benchmark::Chronometer meter)
			{
				auto m = Matrix4f::rotationY(randomScalarf(45.0f, 92.0f)) * Matrix4f::rotationX(randomScalarf(93.0f, 148.0f)) * Matrix4f::translation(0.5f, 0.0f, 0.5f);

				std::vector<Vector4f> vectors;
				vectors.resize(NumVectors);
				randomVectors(vectors);

				meter.measure([&m, &vectors]
				{
					m.transform(vectors);
					return vectors.back();
				});
			};
		}

		/************
		* NOTE: missing tests:
		*  - Matrix4f::scaleAngle
		*  - Matrix4f::reflection
		*  - Matrix4f::saturation
		*  - Matrix4f::glModelView*
		*  - Matrix4f::transform(BBox)
		*  - Matrix4f::inverseTranspose
		*  - Matrix4f::inverseHomogenous
		*  - Matrix4f::extractTranslation
		*  - Matrix4f::extractScale
		*  - Matrix4f::extractRotation
		*/
	}

	TEST_CASE("Matrix 4x4 of type double", "[common][matrix][matrix4x4d]")
	{
		SECTION("init")
		{
			const double matIdentity[]{1.0, 0.0, 0.0, 0.0, 0.0, 1.0, 0.0, 0.0, 0.0, 0.0, 1.0, 0.0, 0.0, 0.0, 0.0, 1.0};
			REQUIRE_THAT(Matrix4d::identity(), Matrix4Equals(matIdentity));

			const double matZero[]{0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0};
			REQUIRE_THAT(Matrix4d::zero(), Matrix4Equals(matZero));

			const double matScalar[]{1.12345, 1.12345, 1.12345, 1.12345, 1.12345, 1.12345, 1.12345, 1.12345, 1.12345, 1.12345, 1.12345, 1.12345, 1.12345, 1.12345, 1.12345, 1.12345};
			REQUIRE_THAT(Matrix4d::from(1.12345), Matrix4Equals(matScalar));

			const double matRandom[]{2.0, -5.0, 3.0, 7.0, 1.0, -6.0, -9.0, 4.0, 8.0, -90.0, 60.0, -40.0, 25.0, 12.0, -91.0, -48.0};
			REQUIRE_THAT(Matrix4d::from({2.0, -5.0, 3.0, 7.0, 1.0, -6.0, -9.0, 4.0, 8.0, -90.0, 60.0, -40.0, 25.0, 12.0, -91.0, -48.0}), Matrix4Equals(matRandom));

			const double matD[]{-60.0, 40.0, -25.0, 0.0, 0.5911, 9.1244, 0.0004224, 22.6677, -90.0, 60.0, -40.0, 25.0, 12.0, -91.0, -48.0, 133.0};
			const float matF[]{0.11679f, 12.1245f, -12.0f, 91.0f, 48.0f, -133.0f, 67.0f, 90.0f, -60.0f, 40.0f, -25.0f, 0.0f, 0.5911f, 9.1244f, 0.0004224f, 22.6677f};
			const double matFasD[]{0.11679, 12.1245, -12.0, 91.0, 48.0, -133.0, 67.0, 90.0, -60.0, 40.0, -25.0, 0.0, 0.5911, 9.1244, 0.0004224, 22.6677};
			REQUIRE_THAT(Matrix4d::from<double>(matD), Matrix4Equals(matD));
			REQUIRE_THAT(Matrix4d::from<float>(matF), Matrix4Equals(matFasD));
		}

		SECTION("access")
		{
			const double matRandom[]{2.0, -5.0, 3.0, 7.0, 1.0, -6.0, -9.0, 4.0, 8.0, -90.0, 60.0, -40.0, 25.0, 12.0, -91.0, -48.0};
			auto mat = Matrix4d::from({2.0, -5.0, 3.0, 7.0, 1.0, -6.0, -9.0, 4.0, 8.0, -90.0, 60.0, -40.0, 25.0, 12.0, -91.0, -48.0});

			for (int i = 0; i < 16; i++)
				REQUIRE(mat[i] == Catch::Approx(matRandom[i]));

			for (int i = 0; i < 16; i++)
				REQUIRE(mat[i + 16] == Catch::Approx(matRandom[i]));

			for (int i = 0; i < 16; i++)
				REQUIRE(mat[i + 32] == Catch::Approx(matRandom[i]));

			for (int i = 0; i < 16; i++)
				REQUIRE(mat.data()[i] == Catch::Approx(matRandom[i]));

			REQUIRE_THAT(mat.getRow(0), VectorEquals(2.0, -5.0, 3.0, 7.0));
			REQUIRE_THAT(mat.getRow(1), VectorEquals(1.0, -6.0, -9.0, 4.0));
			REQUIRE_THAT(mat.getRow(2), VectorEquals(8.0, -90.0, 60.0, -40.0));
			REQUIRE_THAT(mat.getRow(3), VectorEquals(25.0, 12.0, -91.0, -48.0));
			REQUIRE_THAT(mat.getRow(4), VectorEquals(2.0, -5.0, 3.0, 7.0));
			REQUIRE_THAT(mat.getRow(5), VectorEquals(1.0, -6.0, -9.0, 4.0));
			REQUIRE_THAT(mat.getRow(6), VectorEquals(8.0, -90.0, 60.0, -40.0));
			REQUIRE_THAT(mat.getRow(7), VectorEquals(25.0, 12.0, -91.0, -48.0));
			REQUIRE_THAT(mat.getRow(11), VectorEquals(25.0, 12.0, -91.0, -48.0));

			REQUIRE_THAT(mat.getColumn(0), VectorEquals(2.0, 1.0, 8.0, 25.0));
			REQUIRE_THAT(mat.getColumn(1), VectorEquals(-5.0, -6.0, -90.0, 12.0));
			REQUIRE_THAT(mat.getColumn(2), VectorEquals(3.0, -9.0, 60.0, -91.0));
			REQUIRE_THAT(mat.getColumn(3), VectorEquals(7.0, 4.0, -40.0, -48.0));
		}

		SECTION("misc")
		{
			auto m = randomMatrix4d();

			double buffer[16];
			m.write(buffer);
			REQUIRE_THAT(m, Matrix4Equals(buffer));
		}

		SECTION("convert")
		{
			const float matData3f[]{0.11679f, 12.1245f, -12.0f, 48.0f, -133.0f, 67.0f, -60.0f, 40.0f, -25.0f};
			const double matData3d[]{0.11679, 12.1245, -12.0, 48.0, -133.0, 67.0, -60.0, 40.0, -25.0};
			const float matData4f[]{0.11679f, 12.1245f, -12.0f, 91.0f, 48.0f, -133.0f, 67.0f, 90.0f, -60.0f, 40.0f, -25.0f, 0.0f, 0.5911f, 9.1244f, 0.0004224f, 22.6677f};
			const double matData4d[]{0.11679, 12.1245, -12.0, 91.0, 48.0, -133.0, 67.0, 90.0, -60.0, 40.0, -25.0, 0.0, 0.5911, 9.1244, 0.0004224, 22.6677};

			auto m4 = Matrix4d::from(std::span<const double>(matData4d, 16));
			REQUIRE_THAT(m4, Matrix4Equals(matData4d));

			auto m3f = m4.convert<Matrix3, float>();
			auto m3d = m4.convert<Matrix3, double>();
			REQUIRE_THAT(m3f, Matrix3Equals(matData3f));
			REQUIRE_THAT(m3d, Matrix3Equals(matData3d));

			auto m4f = m4.convert<Matrix4, float>();
			auto m4d = m4.convert<Matrix4, double>();
			REQUIRE_THAT(m4f, Matrix4Equals(matData4f));
			REQUIRE_THAT(m4d, Matrix4Equals(matData4d));
		}

		SECTION("matrix plus matrix")
		{
			{
				auto m1 = randomMatrix4d();
				auto m2 = randomMatrix4d();
				auto mRes = Matrix4d::naked();
				for (int i = 0; i < 16; i++)
					mRes[i] = m1[i] + m2[i];

				auto m3 = m1 + m2;
				REQUIRE_THAT(m3, Matrix4Equals(mRes));
				m3 = m1;
				m3 += m2;
				REQUIRE_THAT(m3, Matrix4Equals(mRes));
				m3 = m1;
				m3 += m2.data();
				REQUIRE_THAT(m3, Matrix4Equals(mRes));
			}

			{
				auto m1 = randomMatrix4d();
				auto m2 = randomMatrix4d();
				auto mRes = Matrix4d::naked();
				for (int i = 0; i < 16; i++)
					mRes[i] = m1[i] + m2[i];

				auto m3 = m1 + m2;
				REQUIRE_THAT(m3, Matrix4Equals(mRes));
				m3 = m1;
				m3 += m2;
				REQUIRE_THAT(m3, Matrix4Equals(mRes));
				m3 = m1;
				m3 += m2.data();
				REQUIRE_THAT(m3, Matrix4Equals(mRes));
			}
		}

		SECTION("matrix minus matrix")
		{
			{
				auto m1 = randomMatrix4d();
				auto m2 = randomMatrix4d();
				auto mRes = Matrix4d::naked();
				for (int i = 0; i < 16; i++)
					mRes[i] = m1[i] - m2[i];

				auto m3 = m1 - m2;
				REQUIRE_THAT(m3, Matrix4Equals(mRes));
				m3 = m1;
				m3 -= m2;
				REQUIRE_THAT(m3, Matrix4Equals(mRes));
				m3 = m1;
				m3 -= m2.data();
				REQUIRE_THAT(m3, Matrix4Equals(mRes));
			}

			{
				auto m1 = randomMatrix4d();
				auto m2 = randomMatrix4d();
				auto mRes = Matrix4d::naked();
				for (int i = 0; i < 16; i++)
					mRes[i] = m1[i] - m2[i];

				auto m3 = m1 - m2;
				REQUIRE_THAT(m3, Matrix4Equals(mRes));
				m3 = m1;
				m3 -= m2;
				REQUIRE_THAT(m3, Matrix4Equals(mRes));
				m3 = m1;
				m3 -= m2.data();
				REQUIRE_THAT(m3, Matrix4Equals(mRes));
			}
		}

		SECTION("matrix op scalar")
		{
			{
				auto m = randomMatrix4d();
				auto scalar = randomScalard();
				auto mRes = Matrix4d::naked();
				for (int i = 0; i < 16; i++)
					mRes[i] = m[i] * scalar;

				auto m2 = m * scalar;
				REQUIRE_THAT(m2, Matrix4Equals(mRes));
				m2 = m;
				m2 *= scalar;
				REQUIRE_THAT(m2, Matrix4Equals(mRes));
			}

			{
				auto m = randomMatrix4d();
				auto scalar = randomScalard();
				auto mRes = Matrix4d::naked();
				for (int i = 0; i < 16; i++)
					mRes[i] = m[i] * scalar;

				auto m2 = m * scalar;
				REQUIRE_THAT(m2, Matrix4Equals(mRes));
				m2 = m;
				m2 *= scalar;
				REQUIRE_THAT(m2, Matrix4Equals(mRes));
			}
		}

		SECTION("matrix times matrix")
		{
			{
				auto m1 = Matrix4d::from({1.0, 2.0, 3.0, 4.0, 5.0, 6.0, 7.0, 8.0, 1.0, 2.0, 3.0, 4.0, 5.0, 6.0, 7.0, 8.0});
				auto m2 = Matrix4d::from({1.0, 2.0, 3.0, 4.0, 5.0, 6.0, 7.0, 8.0, 1.0, 2.0, 3.0, 4.0, 5.0, 6.0, 7.0, 8.0});
				auto mRes = Matrix4d::from({34.0, 44.0, 54.0, 64.0, 82.0, 108.0, 134.0, 160.0, 34.0, 44.0, 54.0, 64.0, 82.0, 108.0, 134.0, 160.0});

				auto m3 = m1 * m2;
				REQUIRE_THAT(m3, Matrix4Equals(mRes));
				m3 = m1;
				m3 *= m2;
				REQUIRE_THAT(m3, Matrix4Equals(mRes));
				m3 = m1;
				m3 *= m2.data();
				REQUIRE_THAT(m3, Matrix4Equals(mRes));
			}

			{
				auto m = Matrix4d::from({1.0, 2.0, 3.0, 4.0, 5.0, 6.0, 7.0, 8.0, 9.0, 10.0, 11.0, 12.0, 13.0, 14.0, 15.0, 16.0});

				auto mRes = Matrix4d::naked();
				{
					const auto& a = m;
					const auto& b = m;

					mRes[0] = (a[0] * b[0] + a[1] * b[4] + a[2] * b[8] + a[3] * b[12]);
					mRes[1] = (a[0] * b[1] + a[1] * b[5] + a[2] * b[9] + a[3] * b[13]);
					mRes[2] = (a[0] * b[2] + a[1] * b[6] + a[2] * b[10] + a[3] * b[14]);
					mRes[3] = (a[0] * b[3] + a[1] * b[7] + a[2] * b[11] + a[3] * b[15]);

					mRes[4] = (a[4] * b[0] + a[5] * b[4] + a[6] * b[8] + a[7] * b[12]);
					mRes[5] = (a[4] * b[1] + a[5] * b[5] + a[6] * b[9] + a[7] * b[13]);
					mRes[6] = (a[4] * b[2] + a[5] * b[6] + a[6] * b[10] + a[7] * b[14]);
					mRes[7] = (a[4] * b[3] + a[5] * b[7] + a[6] * b[11] + a[7] * b[15]);

					mRes[8] = (a[8] * b[0] + a[9] * b[4] + a[10] * b[8] + a[11] * b[12]);
					mRes[9] = (a[8] * b[1] + a[9] * b[5] + a[10] * b[9] + a[11] * b[13]);
					mRes[10] = (a[8] * b[2] + a[9] * b[6] + a[10] * b[10] + a[11] * b[14]);
					mRes[11] = (a[8] * b[3] + a[9] * b[7] + a[10] * b[11] + a[11] * b[15]);

					mRes[12] = (a[12] * b[0] + a[13] * b[4] + a[14] * b[8] + a[15] * b[12]);
					mRes[13] = (a[12] * b[1] + a[13] * b[5] + a[14] * b[9] + a[15] * b[13]);
					mRes[14] = (a[12] * b[2] + a[13] * b[6] + a[14] * b[10] + a[15] * b[14]);
					mRes[15] = (a[12] * b[3] + a[13] * b[7] + a[14] * b[11] + a[15] * b[15]);
				}

				auto m2 = m * m;
				REQUIRE_THAT(m2, Matrix4Equals(mRes));
				m2 = m;
				m2 *= m;
				REQUIRE_THAT(m2, Matrix4Equals(mRes));
				m2 = m;
				m2 *= m.data();
				REQUIRE_THAT(m2, Matrix4Equals(mRes));
			}
		}

		SECTION("matrix times misc")
		{
			auto m = randomMatrix4d();

			//matrix3
			{
				auto m3 = randomMatrix3d();

				auto mActual = m;
				mActual *= m3;

				auto mExpected = m;
				mExpected *= m3.convert<Matrix4, double>();

				REQUIRE_THAT(mActual, Matrix4Equals(mExpected));
			}

			//span of a 4x4 matrix
			{
				auto m4 = randomMatrix4d();

				auto mActual = m;
				mActual *= m4.data();

				auto mExpected = m;
				mExpected *= m4.convert<Matrix4, double>();

				REQUIRE_THAT(mActual, Matrix4Equals(mExpected));
			}

			//quaternion
			{
				auto q = Quaterniond::fromEuler(11.0, 22.0, 33.0, Quaterniond::AxisOrder::YZX);

				auto mActual = m;
				mActual *= q;

				auto mExpected = m;
				mExpected *= Matrix4d::rotation(q);

				REQUIRE_THAT(mActual, Matrix4Equals(mExpected));
			}
		}

		SECTION("determinant")
		{
			REQUIRE(Matrix4d::identity().determinant() == Catch::Approx(1.0));

			{
				auto m = Matrix4d::rotationX(30.0);
				m *= Matrix4d::rotationY(30.0);
				m *= Matrix4d::rotationZ(30.0);
				REQUIRE(m.determinant() == Catch::Approx(1.0));
			}

			{
				auto m = Matrix4d::from({6.0, 1.0, 1.0, 0.0, 4.0, -2.0, 5.0, 0.0, 2.0, 8.0, 7.0, 0.0, 0.0, 0.0, 0.0, 1.0});
				REQUIRE(m.determinant() == Catch::Approx(-306.0));
			}

			{
				auto m = Matrix4d::from({2.0, 3.0, 4.0, 0.0, 1.0, 2.0, -3.0, 0.0, 1.0, 1.0, 5.0, 0.0, 0.0, 0.0, 0.0, 1.0});
				REQUIRE(m.determinant() == Catch::Approx(-2.0));
			}

			{
				// determinant(A) = 1 / determinant(inverse(A))

				auto m = Matrix4d::from({5.0, 2.0, 8.25, 1.0, 12.0, 6.8, 2.14, 9.6, 6.5, 1.0, 3.14, 2.22, 0.0, 0.86, 4.0, 1.0});

				auto mI = m;
				mI.inverse();

				auto dM = m.determinant();
				auto dMI = mI.determinant();

				REQUIRE(dM == Catch::Approx(1.0 / dMI));
			}
		}

		SECTION("transpose")
		{
			{
				auto m = Matrix4d::identity();
				m.inverse();
				REQUIRE_THAT(m, Matrix4Equals(Matrix4d::identity()));
			}

			{
				auto m = randomMatrix4d();

				auto mT = m;
				mT.transpose();

				REQUIRE(m[0] == Catch::Approx(mT[0]));
				REQUIRE(m[1] == Catch::Approx(mT[4]));
				REQUIRE(m[2] == Catch::Approx(mT[8]));
				REQUIRE(m[3] == Catch::Approx(mT[12]));
				REQUIRE(m[4] == Catch::Approx(mT[1]));
				REQUIRE(m[5] == Catch::Approx(mT[5]));
				REQUIRE(m[6] == Catch::Approx(mT[9]));
				REQUIRE(m[7] == Catch::Approx(mT[13]));
				REQUIRE(m[8] == Catch::Approx(mT[2]));
				REQUIRE(m[9] == Catch::Approx(mT[6]));
				REQUIRE(m[10] == Catch::Approx(mT[10]));
				REQUIRE(m[11] == Catch::Approx(mT[14]));
				REQUIRE(m[12] == Catch::Approx(mT[3]));
				REQUIRE(m[13] == Catch::Approx(mT[7]));
				REQUIRE(m[14] == Catch::Approx(mT[11]));
				REQUIRE(m[15] == Catch::Approx(mT[15]));
			}
		}

		SECTION("matrix translation")
		{
			{
				auto mRes = Matrix4d::from({1.0, 0.0, 0.0, 0.0, 0.0, 1.0, 0.0, 0.0, 0.0, 0.0, 1.0, 0.0, 2.0, -3.0, 4.0, 1.0});

				REQUIRE_THAT(Matrix4d::translation(2.0, -3.0, 4.0), Matrix4Equals(mRes));
				REQUIRE_THAT(Matrix4d::translation(Vector3d{2.0, -3.0, 4.0}), Matrix4Equals(mRes));
			}
		}

		SECTION("matrix scale")
		{
			{
				auto mRes = Matrix4d::from({-2.0, 0.0, 0.0, 0.0, 0.0, -2.0, 0.0, 0.0, 0.0, 0.0, -2.0, 0.0, 0.0, 0.0, 0.0, 1.0});

				REQUIRE_THAT(Matrix4d::scale(-2.0), Matrix4Equals(mRes));
				REQUIRE_THAT(Matrix4d::scale(-2.0, -2.0, -2.0), Matrix4Equals(mRes));
				REQUIRE_THAT(Matrix4d::scale(Vector3d{-2.0, -2.0, -2.0}), Matrix4Equals(mRes));
			}

			{
				auto mRes = Matrix4d::from({-2.0, 0.0, 0.0, 0.0, 0.0, 3.0, 0.0, 0.0, 0.0, 0.0, -4.0, 0.0, 0.0, 0.0, 0.0, 1.0});

				REQUIRE_THAT(Matrix4d::scale(-2.0, 3.0, -4.0), Matrix4Equals(mRes));
				REQUIRE_THAT(Matrix4d::scale(Vector3d{-2.0, 3.0, -4.0}), Matrix4Equals(mRes));
			}
		}

		SECTION("matrix rotation")
		{
			//along the X axis
			auto m = Matrix4d::rotation(Vector3d{1.0, 0.0, 0.0}, 90.0);
			auto m2 = Matrix4d::rotationX(90.0);
			REQUIRE_THAT(m, Matrix4Equals(m2));
			REQUIRE_THAT(m.transformCopy(Vector3d{0.0, 1.0, 0.0}), VectorEquals(0.0, 0.0, 1.0));
			REQUIRE_THAT(Matrix4d::rotationX(30.0), Matrix4Equals(Matrix4d::from({1.0, 0.0, 0.0, 0.0, 0.0, 0.8660254, 0.5, 0.0, 0.0, -0.5, 0.8660254, 0.0, 0.0, 0.0, 0.0, 1.0})));

			m = Matrix4d::rotation(Vector3d{1.0, 0.0, 0.0}, 180.0);
			m2 = Matrix4d::rotationX(180.0);
			REQUIRE_THAT(m, Matrix4Equals(m2));
			REQUIRE_THAT(m.transformCopy(Vector3d{0.0, 1.0, 0.0}), VectorEquals(0.0, -1.0, 0.0));

			m = Matrix4d::rotation(Vector3d{1.0, 0.0, 0.0}, 270.0);
			m2 = Matrix4d::rotationX(270.0);
			REQUIRE_THAT(m, Matrix4Equals(m2));
			REQUIRE_THAT(m.transformCopy(Vector3d{0.0, 1.0, 0.0}), VectorEquals(0.0, 0.0, -1.0));

			//along the Y axis
			m = Matrix4d::rotation(Vector3d{0.0, 1.0, 0.0}, 90.0);
			m2 = Matrix4d::rotationY(90.0);
			REQUIRE_THAT(m, Matrix4Equals(m2));
			REQUIRE_THAT(m.transformCopy(Vector3d{1.0, 0.0, 0.0}), VectorEquals(0.0, 0.0, -1.0));
			REQUIRE_THAT(Matrix4d::rotationY(60.0), Matrix4Equals(Matrix4d::from({0.49999997, 0.0, -0.866025448, 0.0, 0.0, 1.0, 0.0, 0.0, 0.866025448, 0.0, 0.49999997, 0.0, 0.0, 0.0, 0.0, 1.0})));

			m = Matrix4d::rotation(Vector3d{0.0, 1.0, 0.0}, 180.0);
			m2 = Matrix4d::rotationY(180.0);
			REQUIRE_THAT(m, Matrix4Equals(m2));
			REQUIRE_THAT(m.transformCopy(Vector3d{1.0, 0.0, 0.0}), VectorEquals(-1.0, 0.0, 0.0));

			m = Matrix4d::rotation(Vector3d{0.0, 1.0, 0.0}, 270.0);
			m2 = Matrix4d::rotationY(270.0);
			REQUIRE_THAT(m, Matrix4Equals(m2));
			REQUIRE_THAT(m.transformCopy(Vector3d{1.0, 0.0, 0.0}), VectorEquals(0.0, 0.0, 1.0));

			//along the Z axis
			m = Matrix4d::rotation(Vector3d{0.0, 0.0, 1.0}, 90.0);
			m2 = Matrix4d::rotationZ(90.0);
			REQUIRE_THAT(m, Matrix4Equals(m2));
			REQUIRE_THAT(m.transformCopy(Vector3d{0.0, 1.0, 0.0}), VectorEquals(-1.0, 0.0, 0.0));
			REQUIRE_THAT(Matrix4d::rotationZ(50.0), Matrix4Equals(Matrix4d::from({0.642787635, 0.766044438, 0.0, 0.0, -0.766044438, 0.642787635, 0.0, 0.0, 0.0, 0.0, 1.0, 0.0, 0.0, 0.0, 0.0, 1.0})));

			m = Matrix4d::rotation(Vector3d{0.0, 0.0, 1.0}, 180.0);
			m2 = Matrix4d::rotationZ(180.0);
			REQUIRE_THAT(m, Matrix4Equals(m2));
			REQUIRE_THAT(m.transformCopy(Vector3d{0.0, 1.0, 0.0}), VectorEquals(0.0, -1.0, 0.0));

			m = Matrix4d::rotation(Vector3d{0.0, 0.0, 1.0}, 270.0);
			m2 = Matrix4d::rotationZ(270.0);
			REQUIRE_THAT(m, Matrix4Equals(m2));
			REQUIRE_THAT(m.transformCopy(Vector3d{0.0, 1.0, 0.0}), VectorEquals(1.0, 0.0, 0.0));
		}

		SECTION("matrix rotation (quaternion)")
		{
			//identity
			auto q = Quaterniond::identity();
			auto m = Matrix4d::rotation(q);
			REQUIRE_THAT(q.unitRotate(Vector3d{1.0, 1.0, 1.0}), VectorEquals(1.0, 1.0, 1.0));
			REQUIRE_THAT(m.transformCopy(Vector3d{1.0, 1.0, 1.0}), VectorEquals(1.0, 1.0, 1.0));

			//along the X axis
			q = Quaterniond::fromAxisAngle(Vector3d{1.0, 0.0, 0.0}, 90.0);
			m = Matrix4d::rotation(q);
			REQUIRE_THAT(q.unitRotate(Vector3d{0.0, 1.0, 0.0}), VectorEquals(0.0, 0.0, 1.0));
			REQUIRE_THAT(m.transformCopy(Vector3d{0.0, 1.0, 0.0}), VectorEquals(0.0, 0.0, 1.0));

			q = Quaterniond::fromAxisAngle(Vector3d{1.0, 0.0, 0.0}, 180.0);
			m = Matrix4d::rotation(q);
			REQUIRE_THAT(q.unitRotate(Vector3d{0.0, 1.0, 0.0}), VectorEquals(0.0, -1.0, 0.0));
			REQUIRE_THAT(m.transformCopy(Vector3d{0.0, 1.0, 0.0}), VectorEquals(0.0, -1.0, 0.0));

			q = Quaterniond::fromAxisAngle(Vector3d{1.0, 0.0, 0.0}, 270.0);
			m = Matrix4d::rotation(q);
			REQUIRE_THAT(q.unitRotate(Vector3d{0.0, 1.0, 0.0}), VectorEquals(0.0, 0.0, -1.0));
			REQUIRE_THAT(m.transformCopy(Vector3d{0.0, 1.0, 0.0}), VectorEquals(0.0, 0.0, -1.0));

			//along the Y axis
			q = Quaterniond::fromAxisAngle(Vector3d{0.0, 1.0, 0.0}, 90.0);
			m = Matrix4d::rotation(q);
			REQUIRE_THAT(q.unitRotate(Vector3d{1.0, 0.0, 0.0}), VectorEquals(0.0, 0.0, -1.0));
			REQUIRE_THAT(m.transformCopy(Vector3d{1.0, 0.0, 0.0}), VectorEquals(0.0, 0.0, -1.0));

			q = Quaterniond::fromAxisAngle(Vector3d{0.0, 1.0, 0.0}, 180.0);
			m = Matrix4d::rotation(q);
			REQUIRE_THAT(q.unitRotate(Vector3d{1.0, 0.0, 0.0}), VectorEquals(-1.0, 0.0, 0.0));
			REQUIRE_THAT(m.transformCopy(Vector3d{1.0, 0.0, 0.0}), VectorEquals(-1.0, 0.0, 0.0));

			q = Quaterniond::fromAxisAngle(Vector3d{0.0, 1.0, 0.0}, 270.0);
			m = Matrix4d::rotation(q);
			REQUIRE_THAT(q.unitRotate(Vector3d{1.0, 0.0, 0.0}), VectorEquals(0.0, 0.0, 1.0));
			REQUIRE_THAT(m.transformCopy(Vector3d{1.0, 0.0, 0.0}), VectorEquals(0.0, 0.0, 1.0));

			//along the Z axis
			q = Quaterniond::fromAxisAngle(Vector3d{0.0, 0.0, 1.0}, 90.0);
			m = Matrix4d::rotation(q);
			REQUIRE_THAT(q.unitRotate(Vector3d{0.0, 1.0, 0.0}), VectorEquals(-1.0, 0.0, 0.0));
			REQUIRE_THAT(m.transformCopy(Vector3d{0.0, 1.0, 0.0}), VectorEquals(-1.0, 0.0, 0.0));

			q = Quaterniond::fromAxisAngle(Vector3d{0.0, 0.0, 1.0}, 180.0);
			m = Matrix4d::rotation(q);
			REQUIRE_THAT(q.unitRotate(Vector3d{0.0, 1.0, 0.0}), VectorEquals(0.0, -1.0, 0.0));
			REQUIRE_THAT(m.transformCopy(Vector3d{0.0, 1.0, 0.0}), VectorEquals(0.0, -1.0, 0.0));

			q = Quaterniond::fromAxisAngle(Vector3d{0.0, 0.0, 1.0}, 270.0);
			m = Matrix4d::rotation(q);
			REQUIRE_THAT(q.unitRotate(Vector3d{0.0, 1.0, 0.0}), VectorEquals(1.0, 0.0, 0.0));
			REQUIRE_THAT(m.transformCopy(Vector3d{0.0, 1.0, 0.0}), VectorEquals(1.0, 0.0, 0.0));

			//generic
			{
				const double mRes[]{0.875595033, 0.420031041, -0.2385524, 0.0, -0.38175258, 0.904303849, 0.1910483, 0.0, 0.295970082, -0.07621294, 0.952151954, 0.0, 0.0, 0.0, 0.0, 1.0};

				q = Quaterniond::fromAxisAngle(Vector3d{1.0, 2.0, 3.0}.normalize(), 30.0);
				REQUIRE_THAT(Matrix4d::rotation(q), Matrix4Equals(mRes));
			}
		}

		SECTION("inverse")
		{
			{
				auto m = Matrix4d::identity();
				m.inverse();

				REQUIRE_THAT(m, Matrix4Equals(Matrix4d::identity()));
			}

			{
				auto m = Matrix4d::from({2.0, 3.0, 4.0, 0.0, 1.0, 2.0, -3.0, 0.0, 1.0, 1.0, 5.0, 0.0, 0.0, 0.0, 0.0, 1.0});
				const double mRes[]{-6.5, 5.5, 8.5, 0.0, 4.0, -3.0, -5.0, 0.0, 0.5, -0.5, -0.5, 0.0, 0.0, 0.0, 0.0, 1.0};

				auto mI = m;
				mI.inverse();
				REQUIRE_THAT(mI, Matrix4Equals(mRes));

				mI *= m; //must produce identity
				REQUIRE_THAT(Matrix4d::identity(), Matrix4Equals(mI));
			}

			{
				auto m = Matrix4d::identity();
				m *= Quaterniond::fromAxisAngle(Vector3d{1.0, 0.0, 0.0}, 30.0);
				m *= Quaterniond::fromAxisAngle(Vector3d{0.0, 1.0, 0.0}, 30.0);
				m *= Quaterniond::fromAxisAngle(Vector3d{0.0, 0.0, 1.0}, 30.0);
				m.inverse();

				const double mRes[]{0.74999994, -0.216506317, 0.62499994, 0.0, 0.433012635, 0.87499994, -0.216506317, 0.0, -0.49999997, 0.433012635,
				  0.74999994, 0.0, 0.0, 0.0, 0.0, 0.99999994};

				REQUIRE_THAT(m, Matrix4Equals(mRes));
			}

			{
				auto m = Matrix4d::translation(23.0, 42.0, 666.0);

				auto mI = m;
				mI.inverse();
				mI *= m;

				REQUIRE_THAT(mI, Matrix4Equals(Matrix4d::identity()));
			}

			{
				auto m = Matrix4d::scale(23.0, 42.0, -666.0);

				auto mI = m;
				mI.inverse();
				mI *= m;

				REQUIRE_THAT(mI, Matrix4Equals(Matrix4d::identity()));
			}
		}

		SECTION("clone")
		{
			auto m1 = randomMatrix4d();
			REQUIRE_THAT(m1.clone(Matrix4d::CloneTransform::None), Matrix4Equals(m1));

			auto m2 = m1;
			m2.transpose();
			REQUIRE_THAT(m1.clone(Matrix4d::CloneTransform::Transpose), Matrix4Equals(m2));

			m2 = m1;
			m2.inverse();
			REQUIRE_THAT(m1.clone(Matrix4d::CloneTransform::Inverse), Matrix4Equals(m2));

			m2 = m1;
			m2.inverseHomogenous();
			REQUIRE_THAT(m1.clone(Matrix4d::CloneTransform::InverseHomogenous), Matrix4Equals(m2));

			m2 = m1;
			m2.inverseTranspose();
			REQUIRE_THAT(m1.clone(Matrix4d::CloneTransform::InverseTranspose), Matrix4Equals(m2));
		}

		SECTION("transform vector3")
		{
			auto runVariants = [](const Matrix4d& m, const Vector3d& vecSource, const Vector3d& vecExpected)
			{
				//return a copy of vector
				REQUIRE_THAT(m.transformCopy(vecSource), VectorEquals(vecExpected));

				//a vector
				{
					auto vecTmp = vecSource;
					m.transform(vecTmp);

					REQUIRE_THAT(vecTmp, VectorEquals(vecExpected));
				}

				//a span of 3
				{
					auto vecTmp = vecSource;
					m.transform(std::span<double, 3>{vecTmp.data(), 3});

					REQUIRE_THAT(vecTmp, VectorEquals(vecExpected));
				}

				//multiple vectors
				{
					Vector3d vecTmp[19];
					for (auto& vec : vecTmp)
						vec = vecSource;

					m.transform(vecTmp);
					for (auto& vec : vecTmp)
						REQUIRE_THAT(vec, VectorEquals(vecExpected));
				}
			};

			{
				auto m = Matrix4d::identity();

				Vector3d vecSource{1.0, -2.0, 3.0};
				Vector3d vecExpected{1.0, -2.0, 3.0};

				INFO("matrix identity");
				runVariants(m, vecSource, vecExpected);
			}

			{
				auto m = Matrix4d::from({10.0, 10.0, 10.0, 0.0, 20.0, 20.0, 20.0, 0.0, 30.0, 30.0, 30.0, 0.0, 5.0, 10.0, 15.0, 1.0});

				Vector3d vecSource{20.0, 30.0, 40.0};
				Vector3d vecExpected{2005.0, 2010.0, 2015.0};

				INFO("matrix random");
				runVariants(m, vecSource, vecExpected);
			}

			{
				auto m = Matrix4d::rotationX(30.0);
				m *= Matrix4d::rotationY(30.0);
				m *= Matrix4d::rotationZ(30.0);

				m[12] = 10.0;
				m[13] = 20.0;
				m[14] = 30.0;

				Vector3d vecSource{1.0, 2.0, 3.0};
				Vector3d vecExpected{12.19198728, 21.53349376, 32.61602545};

				INFO("matrix rotation and translation");
				runVariants(m, vecSource, vecExpected);
			}
		}

		SECTION("transform vector4")
		{
			auto runVariants = [](const Matrix4d& m, const Vector4d& vecSource, const Vector4d& vecExpected)
			{
				//return a copy of vector
				REQUIRE_THAT(m.transformCopy(vecSource), VectorEquals(vecExpected));

				//a vector
				{
					auto vecTmp = vecSource;
					m.transform(vecTmp);

					REQUIRE_THAT(vecTmp, VectorEquals(vecExpected));
				}

				//a span of 4
				{
					auto vecTmp = vecSource;
					m.transform(std::span<double, 4>{vecTmp.data(), 4});

					REQUIRE_THAT(vecTmp, VectorEquals(vecExpected));
				}

				//multiple vectors
				{
					Vector4d vecTmp[19];
					for (auto& vec : vecTmp)
						vec = vecSource;

					m.transform(vecTmp);
					for (auto& vec : vecTmp)
						REQUIRE_THAT(vec, VectorEquals(vecExpected));
				}
			};

			{
				auto m = Matrix4d::identity();

				Vector4d vecSource{1.0, -2.0, 3.0, -4.0};
				Vector4d vecExpected{1.0, -2.0, 3.0, -4.0};

				INFO("matrix identity");
				runVariants(m, vecSource, vecExpected);
			}

			{
				auto m = Matrix4d::from({10.0, 10.0, 10.0, 0.0, 20.0, 20.0, 20.0, 0.0, 30.0, 30.0, 30.0, 0.0, 5.0, 10.0, 15.0, 1.0});

				Vector4d vecSource{10.0, 5.0, 1.0, 4.0};
				Vector4d vecExpected{250.0, 270.0, 290.0, 4.0};

				INFO("matrix random");
				runVariants(m, vecSource, vecExpected);
			}

			{
				auto m = Matrix4d::rotationX(30.0);
				m *= Matrix4d::rotationY(30.0);
				m *= Matrix4d::rotationZ(30.0);

				m[12] = 10.0;
				m[13] = 20.0;
				m[14] = 30.0;

				Vector4d vecSource1{1.0, 2.0, 3.0, 1.0};
				Vector4d vecSource2{1.0, 2.0, 3.0, 0.0};
				Vector4d vecExpected1{12.19198728, 21.53349376, 32.61602545, 1.0};
				Vector4d vecExpected2{2.19198728, 1.53349376, 2.61602545, 0.0};

				INFO("matrix rotation and translation (w=1.0)");
				runVariants(m, vecSource1, vecExpected1);

				INFO("matrix rotation and translation (w=0.0)");
				runVariants(m, vecSource2, vecExpected2);
			}
		}

		SECTION("matrix multiplication order")
		{
			auto m = Matrix4d::rotationY(90.0) * Matrix4d::rotationX(90.0);
			REQUIRE_THAT(m.transformCopy(Vector3d{1.0, 0.0, 0.0}), VectorEquals(0.0, 1.0, 0.0));
			REQUIRE_THAT(m.transformCopy(Vector3d{-0.9, 0.0, 0.0}), VectorEquals(0.0, -0.9, 0.0));

			m = Matrix4d::rotationY(-90.0) * Matrix4d::rotationX(90.0) * Matrix4d::translation(0.5, 0.0, 0.5);
			REQUIRE_THAT(m.transformCopy(Vector3d{1.0, 0.0, 0.0}), VectorEquals(0.5, -1.0, 0.5));
			REQUIRE_THAT(m.transformCopy(Vector3d{-0.9, 0.0, 0.0}), VectorEquals(0.5, 0.9, 0.5));

			m = Matrix4d::translation(1.0, 0.0, 0.0) * Matrix4d::rotationY(90.0) * Matrix4d::rotationX(90.0);
			REQUIRE_THAT(m.transformCopy(Vector3d{0.0, 0.0, 0.0}), VectorEquals(0.0, 1.0, 0.0));

			m = Matrix4d::translation(1.0, 0.0, 0.0) * Matrix4d::scale(0.33) * Matrix4d::rotationY(90.0) * Matrix4d::rotationX(90.0);
			REQUIRE_THAT(m.transformCopy(Vector3d{0.0, 0.0, 0.0}), VectorEquals(0.0, 0.33, 0.0));
		}

		SECTION("matrix benchmark")
		{
			constexpr size_t NumVectors{ 5000 };

			BENCHMARK_ADVANCED("vec3 not optimized")(Catch::Benchmark::Chronometer meter)
			{
				auto m = Matrix4d::rotationY(randomScalarf(45.0, 92.0)) * Matrix4d::rotationX(randomScalarf(93.0, 148.0)) * Matrix4d::translation(0.5, 0.0, 0.5);

				std::vector<Vector3d> vectors;
				vectors.resize(NumVectors);
				randomVectors(vectors);

				meter.measure([&m, &vectors]
				{
					auto mData = m.data();
					double x, y, z;

					for (auto& vec : vectors)
					{
						x = vec[0];
						y = vec[1];
						z = vec[2];

						vec[0] = (x * mData[0]) + (y * mData[4]) + (z * mData[8]) + mData[12];
						vec[1] = (x * mData[1]) + (y * mData[5]) + (z * mData[9]) + mData[13];
						vec[2] = (x * mData[2]) + (y * mData[6]) + (z * mData[10]) + mData[14];
					}

					return vectors.back();
				});
			};

			BENCHMARK_ADVANCED("vec3 matrix optimized")(Catch::Benchmark::Chronometer meter)
			{
				auto m = Matrix4d::rotationY(randomScalarf(45.0, 92.0)) * Matrix4d::rotationX(randomScalarf(93.0, 148.0)) * Matrix4d::translation(0.5, 0.0, 0.5);

				std::vector<Vector3d> vectors;
				vectors.resize(NumVectors);
				randomVectors(vectors);

				meter.measure([&m, &vectors]
				{
					m.transform(vectors);
					return vectors.back();
				});
			};

			BENCHMARK_ADVANCED("vec4 not optimized")(Catch::Benchmark::Chronometer meter)
			{
				auto m = Matrix4d::rotationY(randomScalarf(45.0, 92.0)) * Matrix4d::rotationX(randomScalarf(93.0, 148.0)) * Matrix4d::translation(0.5, 0.0, 0.5);

				std::vector<Vector4d> vectors;
				vectors.resize(NumVectors);
				randomVectors(vectors);

				meter.measure([&m, &vectors]
				{
					auto mData = m.data();
					double x, y, z, w;

					for (auto& vec : vectors)
					{
						x = vec[0];
						y = vec[1];
						z = vec[2];
						w = vec[3];

						vec[0] = (x * mData[0]) + (y * mData[4]) + (z * mData[8]) + (w * mData[12]);
						vec[1] = (x * mData[1]) + (y * mData[5]) + (z * mData[9]) + (w * mData[13]);
						vec[2] = (x * mData[2]) + (y * mData[6]) + (z * mData[10]) + (w * mData[14]);
						vec[3] = (x * mData[3]) + (y * mData[7]) + (z * mData[11]) + (w * mData[15]);
					}

					return vectors.back();
				});
			};

			BENCHMARK_ADVANCED("vec4 matrix optimized")(Catch::Benchmark::Chronometer meter)
			{
				auto m = Matrix4d::rotationY(randomScalarf(45.0, 92.0)) * Matrix4d::rotationX(randomScalarf(93.0, 148.0)) * Matrix4d::translation(0.5, 0.0, 0.5);

				std::vector<Vector4d> vectors;
				vectors.resize(NumVectors);
				randomVectors(vectors);

				meter.measure([&m, &vectors]
				{
					m.transform(vectors);
					return vectors.back();
				});
			};
		}

		/************
		* NOTE: missing tests:
		*  - Matrix4d::scaleAngle
		*  - Matrix4d::reflection
		*  - Matrix4d::saturation
		*  - Matrix4d::glModelView*
		*  - Matrix4d::transform(BBox)
		*  - Matrix4d::inverseTranspose
		*  - Matrix4d::inverseHomogenous
		*  - Matrix4d::extractTranslation
		*  - Matrix4d::extractScale
		*  - Matrix4d::extractRotation
		*/
	}

	TEST_CASE("Matrix 3x3 of type float", "[common][matrix][matrix3x3f]")
	{
		SECTION("init")
		{
			const float matIdentity[]{ 1.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 1.0f };
			REQUIRE_THAT(Matrix3f::identity(), Matrix3Equals(matIdentity));

			const float matZero[]{0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f};
			REQUIRE_THAT(Matrix3f::zero(), Matrix3Equals(matZero));

			const float matScalar[]{1.12345f, 1.12345f, 1.12345f, 1.12345f, 1.12345f, 1.12345f, 1.12345f, 1.12345f, 1.12345f};
			REQUIRE_THAT(Matrix3f::from(1.12345f), Matrix3Equals(matScalar));

			const float matRandom[]{2.0f, -5.0f, 3.0f, 7.0f, 1.0f, -6.0f, -9.0f, 4.0f, 8.0f};
			REQUIRE_THAT(Matrix3f::from({2.0f, -5.0f, 3.0f, 7.0f, 1.0f, -6.0f, -9.0f, 4.0f, 8.0f}), Matrix3Equals(matRandom));

			const float matF[]{-12.0f, 91.0f, 48.0f, -133.0f, 67.0f, 90.0f, -60.0f, 40.0f, -25.0f};
			const double matD[]{-90.0, 60.0, -40.0, 25.0, 12.0, -91.0, -48.0, 133.0, -67.0};
			const float matDasF[]{-90.0f, 60.0f, -40.0f, 25.0f, 12.0f, -91.0f, -48.0f, 133.0f, -67.0f};
			REQUIRE_THAT(Matrix3f::from<float>(matF), Matrix3Equals(matF));
			REQUIRE_THAT(Matrix3f::from<double>(matD), Matrix3Equals(matDasF));
		}

		SECTION("access")
		{
			auto mat = Matrix3f::from({2.0f, -5.0f, 3.0f, 7.0f, 1.0f, -6.0f, -9.0f, 4.0f, 8.0f});

			REQUIRE(mat[0] == Catch::Approx(2.0f));
			REQUIRE(mat[1] == Catch::Approx(-5.0f));
			REQUIRE(mat[2] == Catch::Approx(3.0f));
			REQUIRE(mat[3] == Catch::Approx(7.0f));
			REQUIRE(mat[4] == Catch::Approx(1.0f));
			REQUIRE(mat[5] == Catch::Approx(-6.0f));
			REQUIRE(mat[6] == Catch::Approx(-9.0f));
			REQUIRE(mat[7] == Catch::Approx(4.0f));
			REQUIRE(mat[8] == Catch::Approx(8.0f));

			REQUIRE(mat[9] == Catch::Approx(2.0f));
			REQUIRE(mat[10] == Catch::Approx(-5.0f));
			REQUIRE(mat[11] == Catch::Approx(3.0f));
			REQUIRE(mat[12] == Catch::Approx(7.0f));
			REQUIRE(mat[13] == Catch::Approx(1.0f));
			REQUIRE(mat[14] == Catch::Approx(-6.0f));
			REQUIRE(mat[15] == Catch::Approx(-9.0f));
			REQUIRE(mat[16] == Catch::Approx(4.0f));
			REQUIRE(mat[17] == Catch::Approx(8.0f));

			REQUIRE(mat[27] == Catch::Approx(2.0f));
			REQUIRE(mat[28] == Catch::Approx(-5.0f));
			REQUIRE(mat[29] == Catch::Approx(3.0f));
			REQUIRE(mat[30] == Catch::Approx(7.0f));
			REQUIRE(mat[31] == Catch::Approx(1.0f));
			REQUIRE(mat[32] == Catch::Approx(-6.0f));
			REQUIRE(mat[33] == Catch::Approx(-9.0f));
			REQUIRE(mat[34] == Catch::Approx(4.0f));
			REQUIRE(mat[35] == Catch::Approx(8.0f));

			REQUIRE(mat.data()[0] == Catch::Approx(2.0f));
			REQUIRE(mat.data()[1] == Catch::Approx(-5.0f));
			REQUIRE(mat.data()[2] == Catch::Approx(3.0f));
			REQUIRE(mat.data()[3] == Catch::Approx(7.0f));
			REQUIRE(mat.data()[4] == Catch::Approx(1.0f));
			REQUIRE(mat.data()[5] == Catch::Approx(-6.0f));
			REQUIRE(mat.data()[6] == Catch::Approx(-9.0f));
			REQUIRE(mat.data()[7] == Catch::Approx(4.0f));
			REQUIRE(mat.data()[8] == Catch::Approx(8.0f));

			REQUIRE_THAT(mat.getRow(0), VectorEquals(2.0f, -5.0f, 3.0f));
			REQUIRE_THAT(mat.getRow(1), VectorEquals(7.0f, 1.0f, -6.0f));
			REQUIRE_THAT(mat.getRow(2), VectorEquals(-9.0f, 4.0f, 8.0f));
			REQUIRE_THAT(mat.getRow(3), VectorEquals(2.0f, -5.0f, 3.0f));
			REQUIRE_THAT(mat.getRow(4), VectorEquals(7.0f, 1.0f, -6.0f));
			REQUIRE_THAT(mat.getRow(5), VectorEquals(-9.0f, 4.0f, 8.0f));
			REQUIRE_THAT(mat.getRow(11), VectorEquals(-9.0f, 4.0f, 8.0f));

			REQUIRE_THAT(mat.getColumn(0), VectorEquals(2.0f, 7.0f, -9.0f));
			REQUIRE_THAT(mat.getColumn(1), VectorEquals(-5.0f, 1.0f, 4.0f));
			REQUIRE_THAT(mat.getColumn(2), VectorEquals(3.0f, -6.0f, 8.0f));
			REQUIRE_THAT(mat.getColumn(3), VectorEquals(2.0f, 7.0f, -9.0f));
			REQUIRE_THAT(mat.getColumn(4), VectorEquals(-5.0f, 1.0f, 4.0f));
			REQUIRE_THAT(mat.getColumn(5), VectorEquals(3.0f, -6.0f, 8.0f));
			REQUIRE_THAT(mat.getColumn(10), VectorEquals(-5.0f, 1.0f, 4.0f));
		}

		SECTION("misc")
		{
			auto m = Matrix3f::from({-14.0f, 96.0f, 45.0f, -140.0f, 66.0f, 96.0f, -51.0f, 36.0f, -33.0f});

			float buffer[9];
			m.write(buffer);
			REQUIRE_THAT(m, Matrix3Equals(buffer));
		}

		SECTION("convert")
		{
			const float matData3f[]{0.6f, 0.2f, 0.3f, 0.2f, 0.7f, 0.5f, 0.3f, 0.5f, 0.7f};
			const double matData3d[]{0.6, 0.2, 0.3, 0.2, 0.7, 0.5, 0.3, 0.5, 0.7};
			const float matData4f[]{0.6f, 0.2f, 0.3f, 0.0f, 0.2f, 0.7f, 0.5f, 0.0f, 0.3f, 0.5f, 0.7f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f};
			const double matData4d[]{0.6, 0.2, 0.3, 0.0, 0.2, 0.7, 0.5, 0.0, 0.3, 0.5, 0.7, 0.0, 0.0, 0.0, 0.0, 1.0};

			auto m3 = Matrix3f::from(std::span<const float>(matData3f, 9));
			REQUIRE_THAT(m3, Matrix3Equals(matData3f));

			auto m3f = m3.convert<Matrix3, float>();
			auto m3d = m3.convert<Matrix3, double>();
			REQUIRE_THAT(m3f, Matrix3Equals(matData3f));
			REQUIRE_THAT(m3d, Matrix3Equals(matData3d));

			auto m4f = m3.convert<Matrix4, float>();
			auto m4d = m3.convert<Matrix4, double>();
			REQUIRE_THAT(m4f, Matrix4Equals(matData4f));
			REQUIRE_THAT(m4d, Matrix4Equals(matData4d));
		}

		SECTION("matrix plus matrix")
		{
			auto m1 = Matrix3f::from({2.0f, -5.0f, 3.0f, 7.0f, 1.0f, -6.0f, -9.0f, 4.0f, 8.0f});
			auto m2 = Matrix3f::from({-14.0f, 96.0f, 45.0f, -140.0f, 66.0f, 96.0f, -51.0f, 36.0f, -33.0f});
			auto m4 = Matrix3f::from({-12.0f, 91.0f, 48.0f, -133.0f, 67.0f, 90.0f, -60.0f, 40.0f, -25.0f});
			auto m3 = m1 + m2;
			REQUIRE_THAT(m3, Matrix3Equals(m4));
			m3 = m1;
			m3 += m2;
			REQUIRE_THAT(m3, Matrix3Equals(m4));
			m3 = m1;
			m3 += m2.data();
			REQUIRE_THAT(m3, Matrix3Equals(m4));

			m1 = Matrix3f::from({0.24f, 0.0082f, -0.3f, 0.6f, -1.4f, 0.73f, 0.38f, 0.096f, -0.16f});
			m2 = Matrix3f::from({1.7f, 0.003f, -5.6f, 0.44f, 0.2082f, -0.1f, 0.8f, -1.2f, 0.93f});
			m4 = Matrix3f::from({1.94f, 0.0112f, -5.9f, 1.04f, -1.1918f, 0.63f, 1.18f, -1.104f, 0.77f});
			m3 = m1 + m2;
			REQUIRE_THAT(m3, Matrix3Equals(m4));
			m3 = m1;
			m3 += m2;
			REQUIRE_THAT(m3, Matrix3Equals(m4));
			m3 = m1;
			m3 += m2.data();
			REQUIRE_THAT(m3, Matrix3Equals(m4));

			m1 = Matrix3f::from({-27.0f, 83.0f, 32.0f, -153.0f, 53.0f, 83.0f, -64.0f, 23.0f, -46.0f});
			m2 = Matrix3f::from({5.0f, -2.0f, 6.0f, 10.0f, 4.0f, -3.0f, -6.0f, 7.0f, 11.0f});
			m4 = Matrix3f::from({-22.0f, 81.0f, 38.0f, -143.0f, 57.0f, 80.0f, -70.0f, 30.0f, -35.0f});
			m3 = m1 + m2;
			REQUIRE_THAT(m3, Matrix3Equals(m4));
			m3 = m1;
			m3 += m2;
			REQUIRE_THAT(m3, Matrix3Equals(m4));
			m3 = m1;
			m3 += m2.data();
			REQUIRE_THAT(m3, Matrix3Equals(m4));
		}

		SECTION("matrix minus matrix")
		{
			auto m1 = Matrix3f::from({2.0f, -5.0f, 3.0f, 7.0f, 1.0f, -6.0f, -9.0f, 4.0f, 8.0f});
			auto m2 = Matrix3f::from({-14.0f, 96.0f, 45.0f, -140.0f, 66.0f, 96.0f, -51.0f, 36.0f, -33.0f});
			auto m4 = Matrix3f::from({16.0f, -101.0f, -42.0f, 147.0f, -65.0f, -102.0f, 42.0f, -32.0f, 41.0f});
			auto m3 = m1 - m2;
			REQUIRE_THAT(m3, Matrix3Equals(m4));
			m3 = m1;
			m3 -= m2;
			REQUIRE_THAT(m3, Matrix3Equals(m4));
			m3 = m1;
			m3 -= m2.data();
			REQUIRE_THAT(m3, Matrix3Equals(m4));

			m1 = Matrix3f::from({0.24f, 0.0082f, -0.3f, 0.6f, -1.4f, 0.73f, 0.38f, 0.096f, -0.16f});
			m2 = Matrix3f::from({1.7f, 0.003f, -5.6f, 0.44f, 0.2082f, -0.1f, 0.8f, -1.2f, 0.93f});
			m4 = Matrix3f::from({-1.46f, 0.0052f, 5.3f, 0.16f, -1.6082f, 0.83f, -0.42f, 1.296f, -1.09f});
			m3 = m1 - m2;
			REQUIRE_THAT(m3, Matrix3Equals(m4));
			m3 = m1;
			m3 -= m2;
			REQUIRE_THAT(m3, Matrix3Equals(m4));
			m3 = m1;
			m3 -= m2.data();
			REQUIRE_THAT(m3, Matrix3Equals(m4));

			m1 = Matrix3f::from({-27.0f, 83.0f, 32.0f, -153.0f, 53.0f, 83.0f, -64.0f, 23.0f, -46.0f});
			m2 = Matrix3f::from({5.0f, -2.0f, 6.0f, 10.0f, 4.0f, -3.0f, -6.0f, 7.0f, 11.0f});
			m4 = Matrix3f::from({-32.0f, 85.0f, 26.0f, -163.0f, 49.0f, 86.0f, -58.0f, 16.0f, -57.0f});
			m3 = m1 - m2;
			REQUIRE_THAT(m3, Matrix3Equals(m4));
			m3 = m1;
			m3 -= m2;
			REQUIRE_THAT(m3, Matrix3Equals(m4));
			m3 = m1;
			m3 -= m2.data();
			REQUIRE_THAT(m3, Matrix3Equals(m4));
		}

		SECTION("determinant")
		{
			REQUIRE(Matrix3f::identity().determinant() == Catch::Approx(1.0f));

			{
				auto m = Matrix3f::rotationX(30.0f);
				m *= Matrix3f::rotationY(30.0f);
				m *= Matrix3f::rotationZ(30.0f);
				REQUIRE(m.determinant() == Catch::Approx(1.0f));
			}

			{
				auto m = Matrix3f::from({6.0f, 1.0f, 1.0f, 4.0f, -2.0f, 5.0f, 2.0f, 8.0f, 7.0f});
				REQUIRE(m.determinant() == Catch::Approx(-306.0f));
			}
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
			m3 = m1;
			m3 *= m2;
			REQUIRE_THAT(m3, Matrix3Equals(m4));
			m3 = m1;
			m3 *= m2.data();
			REQUIRE_THAT(m3, Matrix3Equals(m4));

			m1 = Matrix3f::from({0.24f, 0.0082f, -0.3f, 0.6f, -1.4f, 0.73f, 0.38f, 0.096f, -0.16f});
			m2 = Matrix3f::from({1.7f, 0.003f, -5.6f, 0.44f, 0.2082f, -0.1f, 0.8f, -1.2f, 0.93f});
			m4 = Matrix3f::from({0.1716080f, 0.3624272f, -1.6238200f, 0.9880000f, -1.1656800f, -2.5411000f, 0.5602400f, 0.2131272f, -2.2864000f});
			m3 = m1 * m2;
			REQUIRE_THAT(m3, Matrix3Equals(m4));
			m3 = m1;
			m3 *= m2;
			REQUIRE_THAT(m3, Matrix3Equals(m4));
			m3 = m1;
			m3 *= m2.data();
			REQUIRE_THAT(m3, Matrix3Equals(m4));

			m1 = Matrix3f::from({-27.0f, 83.0f, 32.0f, -153.0f, 53.0f, 83.0f, -64.0f, 23.0f, -46.0f});
			m2 = Matrix3f::from({5.0f, -2.0f, 6.0f, 10.0f, 4.0f, -3.0f, -6.0f, 7.0f, 11.0f});
			m4 = Matrix3f::from({503.0f, 610.0f, -59.0f, -733.0f, 1099.0f, -164.0f, 186.0f, -102.0f, -959.0f});
			m3 = m1 * m2;
			REQUIRE_THAT(m3, Matrix3Equals(m4));
			m3 = m1;
			m3 *= m2;
			REQUIRE_THAT(m3, Matrix3Equals(m4));
			m3 = m1;
			m3 *= m2.data();
			REQUIRE_THAT(m3, Matrix3Equals(m4));
		}

		SECTION("matrix rotation")
		{
			//along the X axis
			auto m = Matrix3f::rotation(Vector3f{1.0f, 0.0f, 0.0f}, 90.0f);
			auto m2 = Matrix3f::rotationX(90.0f);
			REQUIRE_THAT(m, Matrix3Equals(m2));
			REQUIRE_THAT(m.transformCopy(Vector3f{0.0f, 1.0f, 0.0f}), VectorEquals(0.0f, 0.0f, 1.0f));

			m = Matrix3f::rotation(Vector3f{1.0f, 0.0f, 0.0f}, 180.0f);
			m2 = Matrix3f::rotationX(180.0f);
			REQUIRE_THAT(m, Matrix3Equals(m2));
			REQUIRE_THAT(m.transformCopy(Vector3f{0.0f, 1.0f, 0.0f}), VectorEquals(0.0f, -1.0f, 0.0f));

			m = Matrix3f::rotation(Vector3f{1.0f, 0.0f, 0.0f}, 270.0f);
			m2 = Matrix3f::rotationX(270.0f);
			REQUIRE_THAT(m, Matrix3Equals(m2));
			REQUIRE_THAT(m.transformCopy(Vector3f{0.0f, 1.0f, 0.0f}), VectorEquals(0.0f, 0.0f, -1.0f));

			//along the Y axis
			m = Matrix3f::rotation(Vector3f{0.0f, 1.0f, 0.0f}, 90.0f);
			m2 = Matrix3f::rotationY(90.0f);
			REQUIRE_THAT(m, Matrix3Equals(m2));
			REQUIRE_THAT(m.transformCopy(Vector3f{1.0f, 0.0f, 0.0f}), VectorEquals(0.0f, 0.0f, -1.0f));

			m = Matrix3f::rotation(Vector3f{0.0f, 1.0f, 0.0f}, 180.0f);
			m2 = Matrix3f::rotationY(180.0f);
			REQUIRE_THAT(m, Matrix3Equals(m2));
			REQUIRE_THAT(m.transformCopy(Vector3f{1.0f, 0.0f, 0.0f}), VectorEquals(-1.0f, 0.0f, 0.0f));

			m = Matrix3f::rotation(Vector3f{0.0f, 1.0f, 0.0f}, 270.0f);
			m2 = Matrix3f::rotationY(270.0f);
			REQUIRE_THAT(m, Matrix3Equals(m2));
			REQUIRE_THAT(m.transformCopy(Vector3f{1.0f, 0.0f, 0.0f}), VectorEquals(0.0f, 0.0f, 1.0f));

			//along the Z axis
			m = Matrix3f::rotation(Vector3f{0.0f, 0.0f, 1.0f}, 90.0f);
			m2 = Matrix3f::rotationZ(90.0f);
			REQUIRE_THAT(m, Matrix3Equals(m2));
			REQUIRE_THAT(m.transformCopy(Vector3f{0.0f, 1.0f, 0.0f}), VectorEquals(-1.0f, 0.0f, 0.0f));

			m = Matrix3f::rotation(Vector3f{0.0f, 0.0f, 1.0f}, 180.0f);
			m2 = Matrix3f::rotationZ(180.0f);
			REQUIRE_THAT(m, Matrix3Equals(m2));
			REQUIRE_THAT(m.transformCopy(Vector3f{0.0f, 1.0f, 0.0f}), VectorEquals(0.0f, -1.0f, 0.0f));

			m = Matrix3f::rotation(Vector3f{0.0f, 0.0f, 1.0f}, 270.0f);
			m2 = Matrix3f::rotationZ(270.0f);
			REQUIRE_THAT(m, Matrix3Equals(m2));
			REQUIRE_THAT(m.transformCopy(Vector3f{0.0f, 1.0f, 0.0f}), VectorEquals(1.0f, 0.0f, 0.0f));
		}

		SECTION("matrix rotation (quaternion)")
		{
			//identity
			auto q = Quaternionf::identity();
			auto m = Matrix3f::rotation(q);
			REQUIRE_THAT(q.unitRotate(Vector3f{1.0f, 1.0f, 1.0f}), VectorEquals(1.0f, 1.0f, 1.0f));
			REQUIRE_THAT(m.transformCopy(Vector3f{1.0f, 1.0f, 1.0f}), VectorEquals(1.0f, 1.0f, 1.0f));

			//along the X axis
			q = Quaternionf::fromAxisAngle(Vector3f{1.0f, 0.0f, 0.0f}, 90.0f);
			m = Matrix3f::rotation(q);
			REQUIRE_THAT(q.unitRotate(Vector3f{0.0f, 1.0f, 0.0f}), VectorEquals(0.0f, 0.0f, 1.0f));
			REQUIRE_THAT(m.transformCopy(Vector3f{0.0f, 1.0f, 0.0f}), VectorEquals(0.0f, 0.0f, 1.0f));

			q = Quaternionf::fromAxisAngle(Vector3f{1.0f, 0.0f, 0.0f}, 180.0f);
			m = Matrix3f::rotation(q);
			REQUIRE_THAT(q.unitRotate(Vector3f{0.0f, 1.0f, 0.0f}), VectorEquals(0.0f, -1.0f, 0.0f));
			REQUIRE_THAT(m.transformCopy(Vector3f{0.0f, 1.0f, 0.0f}), VectorEquals(0.0f, -1.0f, 0.0f));

			q = Quaternionf::fromAxisAngle(Vector3f{1.0f, 0.0f, 0.0f}, 270.0f);
			m = Matrix3f::rotation(q);
			REQUIRE_THAT(q.unitRotate(Vector3f{0.0f, 1.0f, 0.0f}), VectorEquals(0.0f, 0.0f, -1.0f));
			REQUIRE_THAT(m.transformCopy(Vector3f{0.0f, 1.0f, 0.0f}), VectorEquals(0.0f, 0.0f, -1.0f));

			//along the Y axis
			q = Quaternionf::fromAxisAngle(Vector3f{0.0f, 1.0f, 0.0f}, 90.0f);
			m = Matrix3f::rotation(q);
			REQUIRE_THAT(q.unitRotate(Vector3f{1.0f, 0.0f, 0.0f}), VectorEquals(0.0f, 0.0f, -1.0f));
			REQUIRE_THAT(m.transformCopy(Vector3f{1.0f, 0.0f, 0.0f}), VectorEquals(0.0f, 0.0f, -1.0f));

			q = Quaternionf::fromAxisAngle(Vector3f{0.0f, 1.0f, 0.0f}, 180.0f);
			m = Matrix3f::rotation(q);
			REQUIRE_THAT(q.unitRotate(Vector3f{1.0f, 0.0f, 0.0f}), VectorEquals(-1.0f, 0.0f, 0.0f));
			REQUIRE_THAT(m.transformCopy(Vector3f{1.0f, 0.0f, 0.0f}), VectorEquals(-1.0f, 0.0f, 0.0f));

			q = Quaternionf::fromAxisAngle(Vector3f{0.0f, 1.0f, 0.0f}, 270.0f);
			m = Matrix3f::rotation(q);
			REQUIRE_THAT(q.unitRotate(Vector3f{1.0f, 0.0f, 0.0f}), VectorEquals(0.0f, 0.0f, 1.0f));
			REQUIRE_THAT(m.transformCopy(Vector3f{1.0f, 0.0f, 0.0f}), VectorEquals(0.0f, 0.0f, 1.0f));

			//along the Z axis
			q = Quaternionf::fromAxisAngle(Vector3f{0.0f, 0.0f, 1.0f}, 90.0f);
			m = Matrix3f::rotation(q);
			REQUIRE_THAT(q.unitRotate(Vector3f{0.0f, 1.0f, 0.0f}), VectorEquals(-1.0f, 0.0f, 0.0f));
			REQUIRE_THAT(m.transformCopy(Vector3f{0.0f, 1.0f, 0.0f}), VectorEquals(-1.0f, 0.0f, 0.0f));

			q = Quaternionf::fromAxisAngle(Vector3f{0.0f, 0.0f, 1.0f}, 180.0f);
			m = Matrix3f::rotation(q);
			REQUIRE_THAT(q.unitRotate(Vector3f{0.0f, 1.0f, 0.0f}), VectorEquals(0.0f, -1.0f, 0.0f));
			REQUIRE_THAT(m.transformCopy(Vector3f{0.0f, 1.0f, 0.0f}), VectorEquals(0.0f, -1.0f, 0.0f));

			q = Quaternionf::fromAxisAngle(Vector3f{0.0f, 0.0f, 1.0f}, 270.0f);
			m = Matrix3f::rotation(q);
			REQUIRE_THAT(q.unitRotate(Vector3f{0.0f, 1.0f, 0.0f}), VectorEquals(1.0f, 0.0f, 0.0f));
			REQUIRE_THAT(m.transformCopy(Vector3f{0.0f, 1.0f, 0.0f}), VectorEquals(1.0f, 0.0f, 0.0f));
		}

		SECTION("clone")
		{
			auto m1 = Matrix3f::from({0.24f, 0.0082f, -0.3f, 0.6f, -1.4f, 0.73f, 0.38f, 0.096f, -0.16f});

			REQUIRE_THAT(m1.clone(Matrix3f::CloneTransform::None), Matrix3Equals(m1));

			auto m2 = m1;
			m2.transpose();
			REQUIRE_THAT(m1.clone(Matrix3f::CloneTransform::Transpose), Matrix3Equals(m2));
		}

		SECTION("transform vector")
		{
			auto q = Quaternionf::fromEuler(145.55f, 49.917f, 264.22f, Quaternionf::AxisOrder::XZY);
			Vector3f vecSource{1.0f, 1.0f, 1.0f};
			auto vecRotated = q.unitRotate(vecSource);

			auto m = Matrix3f::rotation(q);

			REQUIRE_THAT(vecRotated, VectorEquals(m.transformCopy(vecSource)));

			auto res1 = vecSource;
			m.transform(res1);
			REQUIRE_THAT(vecRotated, VectorEquals(res1));

			auto res2 = vecSource;
			m.transform(std::span<float, 3>{res2.data(), 3});
			REQUIRE_THAT(vecRotated, VectorEquals(res2));

			Vector3f res3[19];
			for (auto& vec : res3)
				vec = vecSource;

			m.transform(res3);
			for (auto& vec : res3)
				REQUIRE_THAT(vec, VectorEquals(vecRotated));
		}

		SECTION("matrix multiplication order")
		{
			auto q1 = Quaternionf::fromAxisAngle(Vector3f{0.0f, 1.0f, 0.0f}, 90.0f);
			auto q2 = Quaternionf::fromAxisAngle(Vector3f{1.0f, 0.0f, 0.0f}, 90.0f);

			auto m = Matrix3f::rotation(q1) * Matrix3f::rotation(q2);
			REQUIRE_THAT(m.transformCopy(Vector3f{1.0f, 0.0f, 0.0f}), VectorEquals(0.0f, 1.0f, 0.0f));
			REQUIRE_THAT(m.transformCopy(Vector3f{-0.9f, 0.0f, 0.0f}), VectorEquals(0.0f, -0.9f, 0.0f));
		}
	}

	TEST_CASE("Matrix 3x3 of type double", "[common][matrix][matrix3x3d]")
	{
		SECTION("init")
		{
			const double matIdentity[]{1.0, 0.0, 0.0, 0.0, 1.0, 0.0, 0.0, 0.0, 1.0};
			REQUIRE_THAT(Matrix3d::identity(), Matrix3Equals(matIdentity));

			const double matZero[]{0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0};
			REQUIRE_THAT(Matrix3d::zero(), Matrix3Equals(matZero));

			const double matScalar[]{1.12345, 1.12345, 1.12345, 1.12345, 1.12345, 1.12345, 1.12345, 1.12345, 1.12345};
			REQUIRE_THAT(Matrix3d::from(1.12345), Matrix3Equals(matScalar));

			const double matRandom[]{2.0, -5.0, 3.0, 7.0, 1.0, -6.0, -9.0, 4.0, 8.0};
			REQUIRE_THAT(Matrix3d::from({2.0, -5.0, 3.0, 7.0, 1.0, -6.0, -9.0, 4.0, 8.0}), Matrix3Equals(matRandom));

			const double matD[]{-12.0, 91.0, 48.0, -133.0, 67.0, 90.0, -60.0, 40.0, -25.0};
			const float matF[]{-90.0f, 60.0f, -40.0f, 25.0f, 12.0f, -91.0f, -48.0f, 133.0f, -67.0f};
			const double matFasD[]{-90.0, 60.0, -40.0, 25.0, 12.0, -91.0, -48.0, 133.0, -67.0};
			REQUIRE_THAT(Matrix3d::from<double>(matD), Matrix3Equals(matD));
			REQUIRE_THAT(Matrix3d::from<float>(matF), Matrix3Equals(matFasD));
		}

		SECTION("access")
		{
			auto mat = Matrix3d::from({2.0, -5.0, 3.0, 7.0, 1.0, -6.0, -9.0, 4.0, 8.0});

			REQUIRE(mat[0] == Catch::Approx(2.0));
			REQUIRE(mat[1] == Catch::Approx(-5.0));
			REQUIRE(mat[2] == Catch::Approx(3.0));
			REQUIRE(mat[3] == Catch::Approx(7.0));
			REQUIRE(mat[4] == Catch::Approx(1.0));
			REQUIRE(mat[5] == Catch::Approx(-6.0));
			REQUIRE(mat[6] == Catch::Approx(-9.0));
			REQUIRE(mat[7] == Catch::Approx(4.0));
			REQUIRE(mat[8] == Catch::Approx(8.0));

			REQUIRE(mat[9] == Catch::Approx(2.0));
			REQUIRE(mat[10] == Catch::Approx(-5.0));
			REQUIRE(mat[11] == Catch::Approx(3.0));
			REQUIRE(mat[12] == Catch::Approx(7.0));
			REQUIRE(mat[13] == Catch::Approx(1.0));
			REQUIRE(mat[14] == Catch::Approx(-6.0));
			REQUIRE(mat[15] == Catch::Approx(-9.0));
			REQUIRE(mat[16] == Catch::Approx(4.0));
			REQUIRE(mat[17] == Catch::Approx(8.0));

			REQUIRE(mat[27] == Catch::Approx(2.0));
			REQUIRE(mat[28] == Catch::Approx(-5.0));
			REQUIRE(mat[29] == Catch::Approx(3.0));
			REQUIRE(mat[30] == Catch::Approx(7.0));
			REQUIRE(mat[31] == Catch::Approx(1.0));
			REQUIRE(mat[32] == Catch::Approx(-6.0));
			REQUIRE(mat[33] == Catch::Approx(-9.0));
			REQUIRE(mat[34] == Catch::Approx(4.0));
			REQUIRE(mat[35] == Catch::Approx(8.0));

			REQUIRE(mat.data()[0] == Catch::Approx(2.0));
			REQUIRE(mat.data()[1] == Catch::Approx(-5.0));
			REQUIRE(mat.data()[2] == Catch::Approx(3.0));
			REQUIRE(mat.data()[3] == Catch::Approx(7.0));
			REQUIRE(mat.data()[4] == Catch::Approx(1.0));
			REQUIRE(mat.data()[5] == Catch::Approx(-6.0));
			REQUIRE(mat.data()[6] == Catch::Approx(-9.0));
			REQUIRE(mat.data()[7] == Catch::Approx(4.0));
			REQUIRE(mat.data()[8] == Catch::Approx(8.0));

			REQUIRE_THAT(mat.getRow(0), VectorEquals(2.0, -5.0, 3.0));
			REQUIRE_THAT(mat.getRow(1), VectorEquals(7.0, 1.0, -6.0));
			REQUIRE_THAT(mat.getRow(2), VectorEquals(-9.0, 4.0, 8.0));
			REQUIRE_THAT(mat.getRow(3), VectorEquals(2.0, -5.0, 3.0));
			REQUIRE_THAT(mat.getRow(4), VectorEquals(7.0, 1.0, -6.0));
			REQUIRE_THAT(mat.getRow(5), VectorEquals(-9.0, 4.0, 8.0));
			REQUIRE_THAT(mat.getRow(11), VectorEquals(-9.0, 4.0, 8.0));

			REQUIRE_THAT(mat.getColumn(0), VectorEquals(2.0, 7.0, -9.0));
			REQUIRE_THAT(mat.getColumn(1), VectorEquals(-5.0, 1.0, 4.0));
			REQUIRE_THAT(mat.getColumn(2), VectorEquals(3.0, -6.0, 8.0));
			REQUIRE_THAT(mat.getColumn(3), VectorEquals(2.0, 7.0, -9.0));
			REQUIRE_THAT(mat.getColumn(4), VectorEquals(-5.0, 1.0, 4.0));
			REQUIRE_THAT(mat.getColumn(5), VectorEquals(3.0, -6.0, 8.0));
			REQUIRE_THAT(mat.getColumn(10), VectorEquals(-5.0, 1.0, 4.0));
		}

		SECTION("misc")
		{
			auto m = Matrix3d::from({-14.0, 96.0, 45.0, -140.0, 66.0, 96.0, -51.0, 36.0, -33.0});

			double buffer[9];
			m.write(buffer);
			REQUIRE_THAT(m, Matrix3Equals(buffer));
		}

		SECTION("convert")
		{
			const double matData3d[]{0.6, 0.2, 0.3, 0.2, 0.7, 0.5, 0.3, 0.5, 0.7};
			const float matData3f[]{0.6f, 0.2f, 0.3f, 0.2f, 0.7f, 0.5f, 0.3f, 0.5f, 0.7f};
			const double matData4d[]{0.6, 0.2, 0.3, 0.0, 0.2, 0.7, 0.5, 0.0, 0.3, 0.5, 0.7, 0.0, 0.0, 0.0, 0.0, 1.0};
			const float matData4f[]{0.6f, 0.2f, 0.3f, 0.0f, 0.2f, 0.7f, 0.5f, 0.0f, 0.3f, 0.5f, 0.7f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f};

			auto m3 = Matrix3d::from(std::span<const double>(matData3d, 9));
			REQUIRE_THAT(m3, Matrix3Equals(matData3d));

			auto m3f = m3.convert<Matrix3, float>();
			auto m3d = m3.convert<Matrix3, double>();
			REQUIRE_THAT(m3f, Matrix3Equals(matData3f));
			REQUIRE_THAT(m3d, Matrix3Equals(matData3d));

			auto m4f = m3.convert<Matrix4, float>();
			auto m4d = m3.convert<Matrix4, double>();
			REQUIRE_THAT(m4f, Matrix4Equals(matData4f));
			REQUIRE_THAT(m4d, Matrix4Equals(matData4d));
		}

		SECTION("matrix plus matrix")
		{
			auto m1 = Matrix3d::from({2.0, -5.0, 3.0, 7.0, 1.0, -6.0, -9.0, 4.0, 8.0});
			auto m2 = Matrix3d::from({-14.0, 96.0, 45.0, -140.0, 66.0, 96.0, -51.0, 36.0, -33.0});
			auto m4 = Matrix3d::from({-12.0, 91.0, 48.0, -133.0, 67.0, 90.0, -60.0, 40.0, -25.0});
			auto m3 = m1 + m2;
			REQUIRE_THAT(m3, Matrix3Equals(m4));
			m3 = m1;
			m3 += m2;
			REQUIRE_THAT(m3, Matrix3Equals(m4));
			m3 = m1;
			m3 += m2.data();
			REQUIRE_THAT(m3, Matrix3Equals(m4));

			m1 = Matrix3d::from({0.24, 0.0082, -0.3, 0.6, -1.4, 0.73, 0.38, 0.096, -0.16});
			m2 = Matrix3d::from({1.7, 0.003, -5.6, 0.44, 0.2082, -0.1, 0.8, -1.2, 0.93});
			m4 = Matrix3d::from({1.94, 0.0112, -5.9, 1.04, -1.1918, 0.63, 1.18, -1.104, 0.77});
			m3 = m1 + m2;
			REQUIRE_THAT(m3, Matrix3Equals(m4));
			m3 = m1;
			m3 += m2;
			REQUIRE_THAT(m3, Matrix3Equals(m4));
			m3 = m1;
			m3 += m2.data();
			REQUIRE_THAT(m3, Matrix3Equals(m4));

			m1 = Matrix3d::from({-27.0, 83.0, 32.0, -153.0, 53.0, 83.0, -64.0, 23.0, -46.0});
			m2 = Matrix3d::from({5.0, -2.0, 6.0, 10.0, 4.0, -3.0, -6.0, 7.0, 11.0});
			m4 = Matrix3d::from({-22.0, 81.0, 38.0, -143.0, 57.0, 80.0, -70.0, 30.0, -35.0});
			m3 = m1 + m2;
			REQUIRE_THAT(m3, Matrix3Equals(m4));
			m3 = m1;
			m3 += m2;
			REQUIRE_THAT(m3, Matrix3Equals(m4));
			m3 = m1;
			m3 += m2.data();
			REQUIRE_THAT(m3, Matrix3Equals(m4));
		}

		SECTION("matrix minus matrix")
		{
			auto m1 = Matrix3d::from({2.0, -5.0, 3.0, 7.0, 1.0, -6.0, -9.0, 4.0, 8.0});
			auto m2 = Matrix3d::from({-14.0, 96.0, 45.0, -140.0, 66.0, 96.0, -51.0, 36.0, -33.0});
			auto m4 = Matrix3d::from({16.0, -101.0, -42.0, 147.0, -65.0, -102.0, 42.0, -32.0, 41.0});
			auto m3 = m1 - m2;
			REQUIRE_THAT(m3, Matrix3Equals(m4));
			m3 = m1;
			m3 -= m2;
			REQUIRE_THAT(m3, Matrix3Equals(m4));
			m3 = m1;
			m3 -= m2.data();
			REQUIRE_THAT(m3, Matrix3Equals(m4));

			m1 = Matrix3d::from({0.24, 0.0082, -0.3, 0.6, -1.4, 0.73, 0.38, 0.096, -0.16});
			m2 = Matrix3d::from({1.7, 0.003, -5.6, 0.44, 0.2082, -0.1, 0.8, -1.2, 0.93});
			m4 = Matrix3d::from({-1.46, 0.0052, 5.3, 0.16, -1.6082, 0.83, -0.42, 1.296, -1.09});
			m3 = m1 - m2;
			REQUIRE_THAT(m3, Matrix3Equals(m4));
			m3 = m1;
			m3 -= m2;
			REQUIRE_THAT(m3, Matrix3Equals(m4));
			m3 = m1;
			m3 -= m2.data();
			REQUIRE_THAT(m3, Matrix3Equals(m4));

			m1 = Matrix3d::from({-27.0, 83.0, 32.0, -153.0, 53.0, 83.0, -64.0, 23.0, -46.0});
			m2 = Matrix3d::from({5.0, -2.0, 6.0, 10.0, 4.0, -3.0, -6.0, 7.0, 11.0});
			m4 = Matrix3d::from({-32.0, 85.0, 26.0, -163.0, 49.0, 86.0, -58.0, 16.0, -57.0});
			m3 = m1 - m2;
			REQUIRE_THAT(m3, Matrix3Equals(m4));
			m3 = m1;
			m3 -= m2;
			REQUIRE_THAT(m3, Matrix3Equals(m4));
			m3 = m1;
			m3 -= m2.data();
			REQUIRE_THAT(m3, Matrix3Equals(m4));
		}

		SECTION("determinant")
		{
			REQUIRE(Matrix3d::identity().determinant() == Catch::Approx(1.0));

			{
				auto m = Matrix3d::rotationX(30.0);
				m *= Matrix3d::rotationY(30.0);
				m *= Matrix3d::rotationZ(30.0);
				REQUIRE(m.determinant() == Catch::Approx(1.0));
			}

			{
				auto m = Matrix3d::from({6.0, 1.0, 1.0, 4.0, -2.0, 5.0, 2.0, 8.0, 7.0});
				REQUIRE(m.determinant() == Catch::Approx(-306.0));
			}
		}

		SECTION("transpose")
		{
			auto m1 = Matrix3d::from({2.0, -5.0, 3.0, 7.0, 1.0, -6.0, -9.0, 4.0, 8.0});
			m1.transpose();
			REQUIRE_THAT(m1, Matrix3Equals(Matrix3d::from({2.0, 7.0, -9.0, -5.0, 1.0, 4.0, 3.0, -6.0, 8.0})));

			m1 = Matrix3d::from({0.24, 0.0082, -0.3, 0.6, -1.4, 0.73, 0.38, 0.096, -0.16});
			m1.transpose();
			REQUIRE_THAT(m1, Matrix3Equals(Matrix3d::from({0.24, 0.6, 0.38, 0.0082, -1.4, 0.096, -0.3, 0.73, -0.16})));

			m1 = Matrix3d::from({-27.0, 83.0, 32.0, -153.0, 53.0, 83.0, -64.0, 23.0, -46.0});
			m1.transpose();
			REQUIRE_THAT(m1, Matrix3Equals(Matrix3d::from({-27.0, -153.0, -64.0, 83.0, 53.0, 23.0, 32.0, 83.0, -46.0})));
		}

		SECTION("matrix times matrix")
		{
			auto m1 = Matrix3d::from({2.0, -5.0, 3.0, 7.0, 1.0, -6.0, -9.0, 4.0, 8.0});
			auto m2 = Matrix3d::from({-14.0, 96.0, 45.0, -140.0, 66.0, 96.0, -51.0, 36.0, -33.0});
			auto m4 = Matrix3d::from({519.0, -30.0, -489.0, 68.0, 522.0, 609.0, -842.0, -312.0, -285.0});

			auto m3 = m1 * m2;
			REQUIRE_THAT(m3, Matrix3Equals(m4));
			m3 = m1;
			m3 *= m2;
			REQUIRE_THAT(m3, Matrix3Equals(m4));
			m3 = m1;
			m3 *= m2.data();
			REQUIRE_THAT(m3, Matrix3Equals(m4));

			m1 = Matrix3d::from({0.24, 0.0082, -0.3, 0.6, -1.4, 0.73, 0.38, 0.096, -0.16});
			m2 = Matrix3d::from({1.7, 0.003, -5.6, 0.44, 0.2082, -0.1, 0.8, -1.2, 0.93});
			m4 = Matrix3d::from({0.1716080, 0.3624272, -1.6238200, 0.9880000, -1.1656800, -2.5411000, 0.5602400, 0.2131272, -2.2864000});
			m3 = m1 * m2;
			REQUIRE_THAT(m3, Matrix3Equals(m4));
			m3 = m1;
			m3 *= m2;
			REQUIRE_THAT(m3, Matrix3Equals(m4));
			m3 = m1;
			m3 *= m2.data();
			REQUIRE_THAT(m3, Matrix3Equals(m4));

			m1 = Matrix3d::from({-27.0, 83.0, 32.0, -153.0, 53.0, 83.0, -64.0, 23.0, -46.0});
			m2 = Matrix3d::from({5.0, -2.0, 6.0, 10.0, 4.0, -3.0, -6.0, 7.0, 11.0});
			m4 = Matrix3d::from({503.0, 610.0, -59.0, -733.0, 1099.0, -164.0, 186.0, -102.0, -959.0});
			m3 = m1 * m2;
			REQUIRE_THAT(m3, Matrix3Equals(m4));
			m3 = m1;
			m3 *= m2;
			REQUIRE_THAT(m3, Matrix3Equals(m4));
			m3 = m1;
			m3 *= m2.data();
			REQUIRE_THAT(m3, Matrix3Equals(m4));
		}

		SECTION("matrix rotation")
		{
			//along the X axis
			auto m = Matrix3d::rotation(Vector3d{1.0, 0.0, 0.0}, 90.0);
			auto m2 = Matrix3d::rotationX(90.0);
			REQUIRE_THAT(m, Matrix3Equals(m2));
			REQUIRE_THAT(m.transformCopy(Vector3d{0.0, 1.0, 0.0}), VectorEquals(0.0, 0.0, 1.0));

			m = Matrix3d::rotation(Vector3d{1.0, 0.0, 0.0}, 180.0);
			m2 = Matrix3d::rotationX(180.0);
			REQUIRE_THAT(m, Matrix3Equals(m2));
			REQUIRE_THAT(m.transformCopy(Vector3d{0.0, 1.0, 0.0}), VectorEquals(0.0, -1.0, 0.0));

			m = Matrix3d::rotation(Vector3d{1.0, 0.0, 0.0}, 270.0);
			m2 = Matrix3d::rotationX(270.0);
			REQUIRE_THAT(m, Matrix3Equals(m2));
			REQUIRE_THAT(m.transformCopy(Vector3d{0.0, 1.0, 0.0}), VectorEquals(0.0, 0.0, -1.0));

			//along the Y axis
			m = Matrix3d::rotation(Vector3d{0.0, 1.0, 0.0}, 90.0);
			m2 = Matrix3d::rotationY(90.0);
			REQUIRE_THAT(m, Matrix3Equals(m2));
			REQUIRE_THAT(m.transformCopy(Vector3d{1.0, 0.0, 0.0}), VectorEquals(0.0, 0.0, -1.0));

			m = Matrix3d::rotation(Vector3d{0.0, 1.0, 0.0}, 180.0);
			m2 = Matrix3d::rotationY(180.0);
			REQUIRE_THAT(m, Matrix3Equals(m2));
			REQUIRE_THAT(m.transformCopy(Vector3d{1.0, 0.0, 0.0}), VectorEquals(-1.0, 0.0, 0.0));

			m = Matrix3d::rotation(Vector3d{0.0, 1.0, 0.0}, 270.0);
			m2 = Matrix3d::rotationY(270.0);
			REQUIRE_THAT(m, Matrix3Equals(m2));
			REQUIRE_THAT(m.transformCopy(Vector3d{1.0, 0.0, 0.0}), VectorEquals(0.0, 0.0, 1.0));

			//along the Z axis
			m = Matrix3d::rotation(Vector3d{0.0, 0.0, 1.0}, 90.0);
			m2 = Matrix3d::rotationZ(90.0);
			REQUIRE_THAT(m, Matrix3Equals(m2));
			REQUIRE_THAT(m.transformCopy(Vector3d{0.0, 1.0, 0.0}), VectorEquals(-1.0, 0.0, 0.0));

			m = Matrix3d::rotation(Vector3d{0.0, 0.0, 1.0}, 180.0);
			m2 = Matrix3d::rotationZ(180.0);
			REQUIRE_THAT(m, Matrix3Equals(m2));
			REQUIRE_THAT(m.transformCopy(Vector3d{0.0, 1.0, 0.0}), VectorEquals(0.0, -1.0, 0.0));

			m = Matrix3d::rotation(Vector3d{0.0, 0.0, 1.0}, 270.0);
			m2 = Matrix3d::rotationZ(270.0);
			REQUIRE_THAT(m, Matrix3Equals(m2));
			REQUIRE_THAT(m.transformCopy(Vector3d{0.0, 1.0, 0.0}), VectorEquals(1.0, 0.0, 0.0));
		}

		SECTION("matrix rotation (quaternion)")
		{
			//identity
			auto q = Quaterniond::identity();
			auto m = Matrix3d::rotation(q);
			REQUIRE_THAT(q.unitRotate(Vector3d{1.0, 1.0, 1.0}), VectorEquals(1.0, 1.0, 1.0));
			REQUIRE_THAT(m.transformCopy(Vector3d{1.0, 1.0, 1.0}), VectorEquals(1.0, 1.0, 1.0));

			//along the X axis
			q = Quaterniond::fromAxisAngle(Vector3d{1.0, 0.0, 0.0}, 90.0);
			m = Matrix3d::rotation(q);
			REQUIRE_THAT(q.unitRotate(Vector3d{0.0, 1.0, 0.0}), VectorEquals(0.0, 0.0, 1.0));
			REQUIRE_THAT(m.transformCopy(Vector3d{0.0, 1.0, 0.0}), VectorEquals(0.0, 0.0, 1.0));

			q = Quaterniond::fromAxisAngle(Vector3d{1.0, 0.0, 0.0}, 180.0);
			m = Matrix3d::rotation(q);
			REQUIRE_THAT(q.unitRotate(Vector3d{0.0, 1.0, 0.0}), VectorEquals(0.0, -1.0, 0.0));
			REQUIRE_THAT(m.transformCopy(Vector3d{0.0, 1.0, 0.0}), VectorEquals(0.0, -1.0, 0.0));

			q = Quaterniond::fromAxisAngle(Vector3d{1.0, 0.0, 0.0}, 270.0);
			m = Matrix3d::rotation(q);
			REQUIRE_THAT(q.unitRotate(Vector3d{0.0, 1.0, 0.0}), VectorEquals(0.0, 0.0, -1.0));
			REQUIRE_THAT(m.transformCopy(Vector3d{0.0, 1.0, 0.0}), VectorEquals(0.0, 0.0, -1.0));

			//along the Y axis
			q = Quaterniond::fromAxisAngle(Vector3d{0.0, 1.0, 0.0}, 90.0);
			m = Matrix3d::rotation(q);
			REQUIRE_THAT(q.unitRotate(Vector3d{1.0, 0.0, 0.0}), VectorEquals(0.0, 0.0, -1.0));
			REQUIRE_THAT(m.transformCopy(Vector3d{1.0, 0.0, 0.0}), VectorEquals(0.0, 0.0, -1.0));

			q = Quaterniond::fromAxisAngle(Vector3d{0.0, 1.0, 0.0}, 180.0);
			m = Matrix3d::rotation(q);
			REQUIRE_THAT(q.unitRotate(Vector3d{1.0, 0.0, 0.0}), VectorEquals(-1.0, 0.0, 0.0));
			REQUIRE_THAT(m.transformCopy(Vector3d{1.0, 0.0, 0.0}), VectorEquals(-1.0, 0.0, 0.0));

			q = Quaterniond::fromAxisAngle(Vector3d{0.0, 1.0, 0.0}, 270.0);
			m = Matrix3d::rotation(q);
			REQUIRE_THAT(q.unitRotate(Vector3d{1.0, 0.0, 0.0}), VectorEquals(0.0, 0.0, 1.0));
			REQUIRE_THAT(m.transformCopy(Vector3d{1.0, 0.0, 0.0}), VectorEquals(0.0, 0.0, 1.0));

			//along the Z axis
			q = Quaterniond::fromAxisAngle(Vector3d{0.0, 0.0, 1.0}, 90.0);
			m = Matrix3d::rotation(q);
			REQUIRE_THAT(q.unitRotate(Vector3d{0.0, 1.0, 0.0}), VectorEquals(-1.0, 0.0, 0.0));
			REQUIRE_THAT(m.transformCopy(Vector3d{0.0, 1.0, 0.0}), VectorEquals(-1.0, 0.0, 0.0));

			q = Quaterniond::fromAxisAngle(Vector3d{0.0, 0.0, 1.0}, 180.0);
			m = Matrix3d::rotation(q);
			REQUIRE_THAT(q.unitRotate(Vector3d{0.0, 1.0, 0.0}), VectorEquals(0.0, -1.0, 0.0));
			REQUIRE_THAT(m.transformCopy(Vector3d{0.0, 1.0, 0.0}), VectorEquals(0.0, -1.0, 0.0));

			q = Quaterniond::fromAxisAngle(Vector3d{0.0, 0.0, 1.0}, 270.0);
			m = Matrix3d::rotation(q);
			REQUIRE_THAT(q.unitRotate(Vector3d{0.0, 1.0, 0.0}), VectorEquals(1.0, 0.0, 0.0));
			REQUIRE_THAT(m.transformCopy(Vector3d{0.0, 1.0, 0.0}), VectorEquals(1.0, 0.0, 0.0));
		}

		SECTION("clone")
		{
			auto m1 = Matrix3d::from({0.24, 0.0082, -0.3, 0.6, -1.4, 0.73, 0.38, 0.096, -0.16});

			REQUIRE_THAT(m1.clone(Matrix3d::CloneTransform::None), Matrix3Equals(m1));

			auto m2 = m1;
			m2.transpose();
			REQUIRE_THAT(m1.clone(Matrix3d::CloneTransform::Transpose), Matrix3Equals(m2));
		}

		SECTION("transform vector")
		{
			auto q = Quaterniond::fromEuler(145.55, 49.917, 264.22, Quaterniond::AxisOrder::XZY);
			Vector3d vecSource{1.0, 1.0, 1.0};
			auto vecRotated = q.unitRotate(vecSource);

			auto m = Matrix3d::rotation(q);

			REQUIRE_THAT(vecRotated, VectorEquals(m.transformCopy(vecSource)));

			auto res1 = vecSource;
			m.transform(res1);
			REQUIRE_THAT(vecRotated, VectorEquals(res1));

			auto res2 = vecSource;
			m.transform(std::span<double, 3>{res2.data(), 3});
			REQUIRE_THAT(vecRotated, VectorEquals(res2));

			Vector3d res3[19];
			for (auto& vec : res3)
				vec = vecSource;

			m.transform(res3);
			for (auto& vec : res3)
				REQUIRE_THAT(vec, VectorEquals(vecRotated));
		}

		SECTION("matrix multiplication order")
		{
			auto q1 = Quaterniond::fromAxisAngle(Vector3d{0.0, 1.0, 0.0}, 90.0);
			auto q2 = Quaterniond::fromAxisAngle(Vector3d{1.0, 0.0, 0.0}, 90.0);

			auto m = Matrix3d::rotation(q1) * Matrix3d::rotation(q2);
			REQUIRE_THAT(m.transformCopy(Vector3d{1.0, 0.0, 0.0}), VectorEquals(0.0, 1.0, 0.0));
			REQUIRE_THAT(m.transformCopy(Vector3d{-0.9, 0.0, 0.0}), VectorEquals(0.0, -0.9, 0.0));
		}
	}
}
