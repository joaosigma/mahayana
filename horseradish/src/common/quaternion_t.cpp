#pragma once

#include "utils_t.hpp"

#include "quaternion.hpp"
#include "matrix.hpp"

#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_predicate.hpp>

namespace hr::utests
{
	TEST_CASE("Quaternion of type float", "[common][quaternion][quaternionf]")
	{
		SECTION("init")
		{
			REQUIRE_THAT(Quaternionf::identity(), QuaternionEquals(0.0f, 0.0f, 0.0f, 1.0f));
			REQUIRE_THAT(Quaternionf::zero(), QuaternionEquals(0.0f, 0.0f, 0.0f, 0.0f));
			REQUIRE_THAT(Quaternionf::from(1.0f, 2.0f, 3.0f, 4.0f), QuaternionEquals(1.0f, 2.0f, 3.0f, 4.0f));
			REQUIRE_THAT(Quaternionf::from(1.0, 2.0, 3.0, 4.0), QuaternionEquals(1.0f, 2.0f, 3.0f, 4.0f));

			float valuesf[4]{-0.21f, 55.11f, -0.0001f, 234.77f};
			double valuesd[4]{2.5632, -24.449, -0.999601, 0.9912902};

			REQUIRE_THAT(Quaternionf::from(std::span<const float>(valuesf, 4)), QuaternionEquals(-0.21f, 55.11f, -0.0001f, 234.77f));
			REQUIRE_THAT(Quaternionf::from(std::span<const double>(valuesd, 4)), QuaternionEquals(2.5632f, -24.449f, -0.999601f, 0.9912902f));
			//REQUIRE_THAT(Quaternionf::from(std::span<const float>(valuesf, 2)), QuaternionEquals(Quaternionf::identity()));
			//REQUIRE_THAT(Quaternionf::from(std::span<const double>(valuesd, 1)), QuaternionEquals(Quaternionf::identity()));

			Vector3f vec3{-12.55f, -0.91175f, 0.123f};
			Vector4f vec4{5.9682f, -12.55f, 0.123f, -0.91175f};
			REQUIRE_THAT(Quaternionf::from(vec3, 0.3389f), QuaternionEquals(-12.55f, -0.91175f, 0.123f, 0.3389f));
			REQUIRE_THAT(Quaternionf::from(vec4), QuaternionEquals(5.9682f, -12.55f, 0.123f, -0.91175f));
		}

		SECTION("access")
		{
			auto q = Quaternionf::from(1.0f, 2.0f, 3.0f, 4.0f);

			REQUIRE(q[0] == Catch::Approx(1.0f));
			REQUIRE(q[1] == Catch::Approx(2.0f));
			REQUIRE(q[2] == Catch::Approx(3.0f));
			REQUIRE(q[3] == Catch::Approx(4.0f));
			REQUIRE(q[4] == Catch::Approx(1.0f));
			REQUIRE(q[5] == Catch::Approx(2.0f));
			REQUIRE(q[6] == Catch::Approx(3.0f));
			REQUIRE(q[7] == Catch::Approx(4.0f));

			REQUIRE(q.data()[0] == Catch::Approx(1.0f));
			REQUIRE(q.data()[1] == Catch::Approx(2.0f));
			REQUIRE(q.data()[2] == Catch::Approx(3.0f));
			REQUIRE(q.data()[3] == Catch::Approx(4.0f));
		}

		SECTION("basic operators")
		{
			auto qa = Quaternionf::from(1.0f, 2.0f, 3.0f, 4.0f);
			auto qb = Quaternionf::from(0.1f, -0.2f, 0.3f, -0.4f);

			/*
			* NOTES:
			*  - operation "*" is quaternion multiplication, not per-element multiplication
			*  - operation "/" is actually the inverse / conjugate
			*/

			REQUIRE_THAT(-qa, QuaternionEquals(-1.0f, -2.0f, -3.0f, -4.0f));
			REQUIRE_THAT(-qb, QuaternionEquals(-0.1f, 0.2f, -0.3f, 0.4f));
			REQUIRE_THAT(qa + qb, QuaternionEquals(1.1f, 1.8f, 3.3f, 3.6f));
			REQUIRE_THAT(qa - qb, QuaternionEquals(0.9f, 2.2f, 2.7f, 4.4f));

			auto qc = qa;
			qc += qb;
			REQUIRE_THAT(qc, QuaternionEquals(1.1f, 1.8f, 3.3f, 3.6f));

			qc = qa;
			qc -= qb;
			REQUIRE_THAT(qc, QuaternionEquals(0.9f, 2.2f, 2.7f, 4.4f));
		}

		SECTION("scaling")
		{
			auto q = Quaternionf::from(1.0, 2.0, 3.0, 4.0);

			q = q * 2.0;
			REQUIRE_THAT(q, QuaternionEquals(2.0f, 4.0f, 6.0f, 8.0f));

			q *= 0.5;
			REQUIRE_THAT(q, QuaternionEquals(1.0f, 2.0f, 3.0f, 4.0f));
		}

		SECTION("normalize")
		{
			auto qa = Quaternionf::from(1.0f, 1.0f, 1.0f, 1.0f);
			qa.normalize();
			REQUIRE_THAT(qa, QuaternionEquals(0.5f, 0.5f, 0.5f, 0.5f));

			auto qb = qa;
			qb.normalize();
			REQUIRE_THAT(qb, QuaternionEquals(qa));
		}

		SECTION("conjugate")
		{
			auto q = Quaternionf::from(0.0f, 0.0f, 0.0f, 1.0f);
			REQUIRE_THAT(q.getConjugate(), QuaternionEquals(0.0f, 0.0f, 0.0f, 1.0f));
			q.conjugate();
			REQUIRE_THAT(q, QuaternionEquals(0.0f, 0.0f, 0.0f, 1.0f));

			q = Quaternionf::from(-1.0f, 2.0f, -3.0f, 2.0f);
			REQUIRE_THAT(q.getConjugate(), QuaternionEquals(1.0f, -2.0f, 3.0f, 2.0f));
			q.conjugate();
			REQUIRE_THAT(q, QuaternionEquals(1.0f, -2.0f, 3.0f, 2.0f));
		}

		SECTION("inverse")
		{
			{
				auto qa = Quaternionf::from(1.0f, -2.0f, 1.0f, 3.0f);
				auto qb = Quaternionf::from(-1.0f, 2.0f, 3.0f, 2.0f);

				REQUIRE_THAT(qa * qa.getInverse<false>(), QuaternionEquals(0.0f, 0.0f, 0.0f, 1.0f));
				REQUIRE_THAT(qa * qa.getInverse<false>() * qb, QuaternionEquals(qb));

				qa.normalize();
				qb.normalize();
				REQUIRE_THAT(qa * qa.getInverse<true>(), QuaternionEquals(0.0f, 0.0f, 0.0f, 1.0f));
				REQUIRE_THAT(qa * qa.getInverse<true>() * qb, QuaternionEquals(qb));
			}

			{
				auto qa = Quaternionf::from(1.0f, -2.0f, 1.0f, 3.0f);
				auto qb = Quaternionf::from(-1.0f, 2.0f, 3.0f, 2.0f);

				auto qx = qa;
				qx.inverse<false>();
				REQUIRE_THAT(qa * qx, QuaternionEquals(0.0f, 0.0f, 0.0f, 1.0f));
				REQUIRE_THAT(qa * qx * qb, QuaternionEquals(qb));

				qa.normalize();
				qb.normalize();

				qx = qa;
				qx.inverse<true>();
				REQUIRE_THAT(qa * qx, QuaternionEquals(0.0f, 0.0f, 0.0f, 1.0f));
				REQUIRE_THAT(qa * qx * qb, QuaternionEquals(qb));
			}
		}

		SECTION("axis angle X")
		{
			REQUIRE_THAT(Quaternionf::fromAxisAngle(Vector3f{1.0f, 0.0f, 0.0f}, 120.0f), QuaternionEquals(0.866025f, 0.0f, 0.0f, 0.5f));
			REQUIRE_THAT(Quaternionf::fromAxisAngle(1.0f, 0.0f, 0.0f, 120.0f), QuaternionEquals(0.866025f, 0.0f, 0.0f, 0.5f));
		}

		SECTION("axis angle Y")
		{
			REQUIRE_THAT(Quaternionf::fromAxisAngle(Vector3f{0.0f, 1.0f, 0.0f}, 30.0f), QuaternionEquals(0.0f, 0.258819f, 0.0f, 0.965926f));
			REQUIRE_THAT(Quaternionf::fromAxisAngle(0.0f, 1.0f, 0.0f, 30.0f), QuaternionEquals(0.0f, 0.258819f, 0.0f, 0.965926f));
		}

		SECTION("axis angle Z")
		{
			REQUIRE_THAT(Quaternionf::fromAxisAngle(Vector3f{0.0f, 0.0f, 1.0f}, 60.0f), QuaternionEquals(0.0f, 0.0f, 0.5f, 0.866025f));
			REQUIRE_THAT(Quaternionf::fromAxisAngle(0.0f, 0.0f, 1.0f, 60.0f), QuaternionEquals(0.0f, 0.0f, 0.5f, 0.866025f));
		}

		SECTION("axis angle all")
		{
			Vector3f vec{1.0, 2.0, 0.5};
			vec.normalize();

			REQUIRE_THAT(Quaternionf::fromAxisAngle(vec, 35.0f), QuaternionEquals(0.131239f, 0.262478f, 0.0656194f, 0.953717f));
			REQUIRE_THAT(Quaternionf::fromAxisAngle(vec[0], vec[1], vec[2], 35.0f), QuaternionEquals(0.131239f, 0.262478f, 0.0656194f, 0.953717f));
		}

		SECTION("euler angles")
		{
			auto q_yaw = Quaternionf::fromEuler(0.0f, 45.0f, 0.0f, Quaternionf::AxisOrder::XYZ);
			REQUIRE_THAT(q_yaw, QuaternionEquals(0.0f, 0.382684f, 0.0f, 0.923879f));

			auto q_pitch = Quaternionf::fromEuler(30.0f, 0.0f, 0.0f, Quaternionf::AxisOrder::XYZ);
			REQUIRE_THAT(q_pitch, QuaternionEquals(0.258819f, 0.0f, 0.0f, 0.965926f));

			auto q_roll = Quaternionf::fromEuler(0.0f, 0.0f, 10.0f, Quaternionf::AxisOrder::XYZ);
			REQUIRE_THAT(q_roll, QuaternionEquals(0.0f, 0.0f, 0.0871558f, 0.996195f));

			auto q_ypr = Quaternionf::fromEuler(30.0f, 45.0f, 10.0f, Quaternionf::AxisOrder::YXZ);
			REQUIRE_THAT(q_yaw * q_pitch * q_roll, QuaternionEquals(q_ypr));
		}

		SECTION("vector rotation")
		{
			auto qa = Quaternionf::from(0.3919183f, 0.3196269f, -0.8430416f, -0.1830837f);
			REQUIRE(qa.magnitude() == Catch::Approx(1.0f));
			{
				Vector3f vec{0.3535534f, -0.1464466f, 0.3535534f}, res;

				qa.unitRotate(vec, res);
				REQUIRE_THAT(qa.unitRotate(vec), VectorEquals(-0.487732f, 0.1646256f, 0.08039001f));
				REQUIRE_THAT(res, VectorEquals(-0.487732f, 0.1646256f, 0.08039001f));
				REQUIRE_THAT(res, VectorEquals(qa.unitRotate(vec)));
			}

			auto qb = Quaternionf::from(0.336838f, 0.0115086f, 0.421048f, 0.842096f);
			REQUIRE(qb.magnitude() == Catch::Approx(1.0f));
			{
				Vector3f vec{0.53f, -0.0532f, 0.22f}, res;

				qb.unitRotate(vec, res);
				REQUIRE_THAT(qb.unitRotate(vec), VectorEquals(0.4459215f, 0.2350067f, 0.2793851f));
				REQUIRE_THAT(res, VectorEquals(0.4459215f, 0.2350067f, 0.2793851f));
				REQUIRE_THAT(res, VectorEquals(qb.unitRotate(vec)));
			}

			auto qc = Quaternionf::from(0.0f, 0.7071068f, 0.0f, 0.7071068f);
			REQUIRE(qc.magnitude() == Catch::Approx(1.0f));
			{
				Vector3f vec{13.0f, -1.23f, 3.4f}, res;

				qc.unitRotate(vec, res);
				REQUIRE_THAT(qc.unitRotate(vec), VectorEquals(3.4f, -1.23f, -13.0f));
				REQUIRE_THAT(res, VectorEquals(3.4f, -1.23f, -13.0f));
				REQUIRE_THAT(res, VectorEquals(qc.unitRotate(vec)));
			}
		}

		SECTION("angles between vectors")
		{
			auto q = Quaternionf::identity();

			q = Quaternionf::fromVectors(Vector3f::calcNormalize(Vector3f{0.8f, 1.4f, 2.6f}), Vector3f::calcNormalize(Vector3f{1.2f, 0.3f, -2.9f}));
			REQUIRE_THAT(q, QuaternionEquals(-0.590233f, 0.6634023f, -0.1756065f, 0.4250704f));

			q = Quaternionf::fromVectors(Vector3f{0.0f, 1.0f, 0.0f}, Vector3f{0.0f, 0.0f, 1.0f});
			REQUIRE_THAT(q, QuaternionEquals(0.7071068f, 0.0f, 0.0f, 0.7071068f));

			q = Quaternionf::fromVectors(Vector3f::calcNormalize(Vector3f{-1.0f, 0.4f, 2.9f}), Vector3f::calcNormalize(Vector3f{0.8f, 1.4f, 2.6f}));
			REQUIRE_THAT(q, QuaternionEquals(-0.1695316f, 0.2761906f, -0.09655445f, 0.9410924f));

		    q = Quaternionf::fromVectors(Vector3f::calcNormalize(Vector3f{0.8f, -1.4f, 2.6f}), Vector3f::calcNormalize(Vector3f{0.8f, -1.4f, 2.6f}));
		    REQUIRE_THAT(q, QuaternionEquals(0.0f, 0.0f, 0.0f, 1.0f));

			//opposite vectors
		    auto v1 = Vector3f{-0.8f, 1.4f, -2.6f};
		    auto v2 = Vector3f{0.8f, -1.4f, 2.6f};
		    q = Quaternionf::fromVectors(v1, v2);
		    REQUIRE_THAT(q.unitRotate(v1), VectorEquals(v2));
		}

		SECTION("dot")
		{
			auto qa = Quaternionf::from(-27.0f, 83.0f, 32.0f, -153.0f);
			auto qb = Quaternionf::from(36.0f, -64.0f, 12.0f, 24.0f);

			REQUIRE(qa.dot(qb) == Catch::Approx(-9572.0f));
			REQUIRE(qb.dot(qa) == Catch::Approx(-9572.0f));
			REQUIRE(qa.magnitude() == Catch::Approx(std::sqrt(qa.dot(qa))));
			REQUIRE(qa.magnitudeSquared() == Catch::Approx(qa.dot(qa)));
			
			qa = Quaternionf::from(0.3535534f, -0.1464466f, 0.3535534f, 0.8535535f);
			qb = Quaternionf::from(0.3919183f, 0.3196269f, -0.8430416f, -0.1830837f);
			REQUIRE(qa.dot(qb) == Catch::Approx(-0.362576186656952));

			qa = Quaternionf::from(0.0f, 0.7071068f, 0.0f, 0.7071068f);
			qb = Quaternionf::from(0.0f, 0.7071068f, 0.0f, 0.7071068f);
			REQUIRE(qa.dot(qb) == Catch::Approx(1.0f));
		}

		SECTION("rotation")
		{
			//identity
			auto q = Quaternionf::identity();
			REQUIRE_THAT(q.unitRotate(Vector3f{1.0f, 1.0f, 1.0f}), VectorEquals(1.0f, 1.0f, 1.0f));

			//along the X axis
			q = Quaternionf::fromAxisAngle(Vector3f{1.0f, 0.0f, 0.0f}, 90.0f);
			REQUIRE_THAT(q.unitRotate(Vector3f{0.0f, 1.0f, 0.0f}), VectorEquals(0.0f, 0.0f, 1.0f));

			q = Quaternionf::fromAxisAngle(Vector3f{1.0f, 0.0f, 0.0f}, 180.0f);
			REQUIRE_THAT(q.unitRotate(Vector3f{0.0f, 1.0f, 0.0f}), VectorEquals(0.0f, -1.0f, 0.0f));

			q = Quaternionf::fromAxisAngle(Vector3f{1.0f, 0.0f, 0.0f}, 270.0f);
			REQUIRE_THAT(q.unitRotate(Vector3f{0.0f, 1.0f, 0.0f}), VectorEquals(0.0f, 0.0f, -1.0f));

			//along the Y axis
			q = Quaternionf::fromAxisAngle(Vector3f{0.0f, 1.0f, 0.0f}, 90.0f);
			REQUIRE_THAT(q.unitRotate(Vector3f{1.0f, 0.0f, 0.0f}), VectorEquals(0.0f, 0.0f, -1.0f));

			q = Quaternionf::fromAxisAngle(Vector3f{0.0f, 1.0f, 0.0f}, 180.0f);
			REQUIRE_THAT(q.unitRotate(Vector3f{1.0f, 0.0f, 0.0f}), VectorEquals(-1.0f, 0.0f, 0.0f));

			q = Quaternionf::fromAxisAngle(Vector3f{0.0f, 1.0f, 0.0f}, 270.0f);
			REQUIRE_THAT(q.unitRotate(Vector3f{1.0f, 0.0f, 0.0f}), VectorEquals(0.0f, 0.0f, 1.0f));

			//along the Z axis
			q = Quaternionf::fromAxisAngle(Vector3f{0.0f, 0.0f, 1.0f}, 90.0f);
			REQUIRE_THAT(q.unitRotate(Vector3f{0.0f, 1.0f, 0.0f}), VectorEquals(-1.0f, 0.0f, 0.0f));

			q = Quaternionf::fromAxisAngle(Vector3f{0.0f, 0.0f, 1.0f}, 180.0f);
			REQUIRE_THAT(q.unitRotate(Vector3f{0.0f, 1.0f, 0.0f}), VectorEquals(0.0f, -1.0f, 0.0f));

			q = Quaternionf::fromAxisAngle(Vector3f{0.0f, 0.0f, 1.0f}, 270.0f);
			REQUIRE_THAT(q.unitRotate(Vector3f{0.0f, 1.0f, 0.0f}), VectorEquals(1.0f, 0.0f, 0.0f));
		}

		SECTION("slerp")
		{
			{
				auto qa = Quaternionf::from(-0.270598f, 0.270598f, 0.6532815f, 0.6532815f);
				auto qb = Quaternionf::from(0.5f, 0.5f, 0.5f, 0.5f);
				auto q = Quaternionf::identity();

				q = Quaternionf::sLerp(qa, qb, -0.1f);
				REQUIRE_THAT(q, QuaternionEquals(qa));
				q = Quaternionf::sLerp(qa, qb, 0.0f);
				REQUIRE_THAT(q, QuaternionEquals(qa));
				q = Quaternionf::sLerp(qa, qb, 1.0f);
				REQUIRE_THAT(q, QuaternionEquals(qb));
				q = Quaternionf::sLerp(qa, qb, 1.1f);
				REQUIRE_THAT(q, QuaternionEquals(qb));

				q = Quaternionf::sLerp(qa, qb, 0.62f);
				REQUIRE_THAT(q, QuaternionEquals(0.22069444274723088f, 0.4498729015909088f, 0.6119266025696755f, 0.6119266025696755f));
			}

			{
				auto qa = Quaternionf::fromAxisAngle(Vector3f{1.0f, 0.0f, 0.0f}, 0.0f);
				auto qb = Quaternionf::fromAxisAngle(Vector3f{1.0f, 0.0f, 0.0f}, 90.0f);
				auto q = Quaternionf::identity();

				REQUIRE(qa.magnitude() == Catch::Approx(1.0f));
				REQUIRE(qb.magnitude() == Catch::Approx(1.0f));

				q = Quaternionf::sLerp(qa, qb, -0.1f);
				REQUIRE_THAT(q, QuaternionEquals(qa));
				q = Quaternionf::sLerp(qa, qb, 0.0f);
				REQUIRE_THAT(q, QuaternionEquals(qa));
				q = Quaternionf::sLerp(qa, qb, 1.0f);
				REQUIRE_THAT(q, QuaternionEquals(qb));
				q = Quaternionf::sLerp(qa, qb, 1.1f);
				REQUIRE_THAT(q, QuaternionEquals(qb));

				q = Quaternionf::sLerp(qa, qb, 0.5f);
				REQUIRE_THAT(q, QuaternionEquals(Quaternionf::fromAxisAngle(Vector3f{1.0f, 0.0f, 0.0f}, 45.0f)));
			}
		}

		SECTION("nlerp")
		{
			auto qa = Quaternionf::from(-1.0f, 2.0f, -3.0f, 4.0f);
			auto qb = Quaternionf::from(1.0f, 3.0f, 0.0f, 4.8f);
			auto q = Quaternionf::identity();

			q = Quaternionf::nLerp(qa, qb, -0.1f);
			REQUIRE_THAT(q, QuaternionEquals(qa));
			q = Quaternionf::nLerp(qa, qb, 0.0f);
			REQUIRE_THAT(q, QuaternionEquals(qa));
			q = Quaternionf::nLerp(qa, qb, 1.0f);
			REQUIRE_THAT(q, QuaternionEquals(qb));
			q = Quaternionf::nLerp(qa, qb, 1.1f);
			REQUIRE_THAT(q, QuaternionEquals(qb));

			q = Quaternionf::nLerp(qa, qb, 0.5f);
			
			auto res = Quaternionf::from(0.0f, 2.5f, -1.5f, 4.4f);
			res.normalize();
			REQUIRE_THAT(q, QuaternionEquals(res));
		}

		SECTION("to angle axis")
		{
			{
				auto q = Quaternionf::from(0.6514133f, -0.1282655f, 0.6116868f, 0.430172f);

				float angle;
				Vector3f axis;
				q.getAxisAngle(axis, angle);

				REQUIRE(angle == Catch::Approx(hr::Math::Rad2Deg<float> * 2.25223f));
				REQUIRE_THAT(axis, VectorEquals(0.7215902f, -0.1420836f, 0.6775839f));
			}

			{
				auto q = Quaternionf::from(0.0f, 0.0f, 0.0f, 1.0f);

				float angle;
				Vector3f axis;
				q.getAxisAngle(axis, angle);

				REQUIRE(angle == Catch::Approx(hr::Math::Rad2Deg<float> * 0.0f));
				REQUIRE_THAT(axis, VectorEquals(1.0f, 0.0f, 0.0f));
			}

			{
				auto q = Quaternionf::from(0.1164578f, 0.4874545f, 0.8652994f, 0.009090029f);

				float angle;
				Vector3f axis;
				q.getAxisAngle(axis, angle);

				REQUIRE(angle == Catch::Approx(hr::Math::Rad2Deg<float> * 3.12341f));
				REQUIRE_THAT(axis, VectorEquals(0.1164626f, 0.4874747f, 0.8653352f));
			}
		}

		SECTION("to euler angles")
		{
			{
				auto q = Quaternionf::from(0.6514133f, -0.1282655f, 0.6116868f, 0.430172f);

				float angles[3];
				q.getEulerAngles(angles[0], angles[1], angles[2]);

				REQUIRE(angles[0] == Catch::Approx(0.8f));
				REQUIRE(angles[1] == Catch::Approx(1.4f));
				REQUIRE(angles[2] == Catch::Approx(2.6f));
			}

			{
				auto q = Quaternionf::from(0.0f, 0.0f, 1.2f, 0.0f);

				float angles[3];
				q.getEulerAngles(angles[0], angles[1], angles[2]);

				REQUIRE(angles[0] == Catch::Approx(0.0f));
				REQUIRE(angles[1] == Catch::Approx(0.0f));
				REQUIRE(angles[2] == Catch::Approx(3.14159265f));
			}

			{
				auto q = Quaternionf::from(0.1164578f, 0.4874545f, 0.8652994f, 0.009090029f);

				float angles[3];
				q.getEulerAngles(angles[0], angles[1], angles[2]);

				REQUIRE(angles[0] == Catch::Approx(-1.0f));
				REQUIRE(angles[1] == Catch::Approx(0.4f));
				REQUIRE(angles[2] == Catch::Approx(2.9f));
			}
		}

		SECTION("comparison")
		{
			auto qa = Quaternionf::from(0.6514133f, -0.1282655f, 0.6116868f, 0.430172f);
			auto qb = Quaternionf::from(-0.21f, 55.11f, -0.0001f, 234.77f);

			REQUIRE(qb.isEqual(qb, 0.0000000001f));
			REQUIRE_FALSE(qb.isEqual(qa, 0.0000000001f));
			REQUIRE(Quaternionf::from(0.001f, 0.002f, 0.003f, 0.004f).isEqual(Quaternionf::from(0.002f, 0.003f, 0.004f, 0.005f), 0.009f));
			REQUIRE_FALSE(Quaternionf::from(0.001f, 0.002f, 0.003f, 0.004f).isEqual(Quaternionf::from(0.002f, 0.003f, 0.004f, 0.005f), 0.0009f));
		}

		SECTION("convert")
		{
			auto q = Quaternionf::from(0.6514133f, -0.1282655f, 0.6116868f, 0.430172f);
			auto qf = q.convert<float>();
			auto qd = q.convert<double>();

			REQUIRE_THAT(qf, QuaternionEquals(0.6514133f, -0.1282655f, 0.6116868f, 0.430172f));
			REQUIRE_THAT(qd, QuaternionEquals(0.6514133, -0.1282655, 0.6116868, 0.430172));
		}

		SECTION("multiplication order")
		{
			{
				auto q1 = Quaternionf::from(1.0f, 2.0f, 3.0f, 4.0f);
				auto q2 = Quaternionf::from(5.0f, 6.0f, 7.0f, 8.0f);
				auto qr = q1 * q2;
				REQUIRE_THAT(qr, QuaternionEquals(24.0f, 48.0f, 48.0f, -6.0f));
			}

			{
				auto q1 = Quaternionf::fromAxisAngle(Vector3f{0.0f, 1.0f, 0.0f}, 90.0f);
				auto q2 = Quaternionf::fromAxisAngle(Vector3f{1.0f, 0.0f, 0.0f}, 90.0f);
				auto qFinal = q2 * q1;
				REQUIRE_THAT(qFinal.unitRotate(Vector3f{1.0f, 0.0f, 0.0f}), VectorEquals(0.0f, 1.0f, 0.0f));
				REQUIRE_THAT(qFinal.unitRotate(Vector3f{-0.9f, 0.0f, 0.0f}), VectorEquals(0.0f, -0.9f, 0.0f));
			}
		}

		SECTION("from matrix")
		{
			{
				const float matIdentity[]{1.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 1.0f};
				auto q = Quaternionf::fromMatrix3x3(matIdentity);
				REQUIRE_THAT(q, QuaternionEquals(0.0f, 0.0f, 0.0f, 1.0f));
			}

			{
				const float matIdentity[]{1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f};
				auto q = Quaternionf::fromMatrix4x4(matIdentity);
				REQUIRE_THAT(q, QuaternionEquals(0.0f, 0.0f, 0.0f, 1.0f));
			}

			for (float angle = 0.0f; angle < 720.0f; angle += 10.0f)
			{
				auto qOriginal = Quaternionf::fromAxisAngle(Vector3f{1.0f, 0.0f, 0.0f}, angle);

				auto pred = [qOriginal](const Quaternionf& q) -> bool
				{
					auto matcher = QuaternionEquals(qOriginal);
					return matcher.match(q) || matcher.match(-q);
				};

				auto mat3 = Matrix3f::rotation(qOriginal);
				REQUIRE_THAT(Quaternionf::fromMatrix3x3(mat3.data()), Catch::Matchers::Predicate<Quaternionf>(pred));

				auto mat4 = Matrix4f::rotation(qOriginal);
				REQUIRE_THAT(Quaternionf::fromMatrix4x4(mat4.data()), Catch::Matchers::Predicate<Quaternionf>(pred));
			}

			for (float angle = 0.0f; angle < 720.0f; angle += 10.0f)
			{
				auto qOriginal = Quaternionf::fromAxisAngle(Vector3f{0.0f, 1.0f, 0.0f}, angle);

				auto pred = [qOriginal](const Quaternionf& q) -> bool
				{
					auto matcher = QuaternionEquals(qOriginal);
					return matcher.match(q) || matcher.match(-q);
				};

				auto mat3 = Matrix3f::rotation(qOriginal);
				REQUIRE_THAT(Quaternionf::fromMatrix3x3(mat3.data()), Catch::Matchers::Predicate<Quaternionf>(pred));

				auto mat4 = Matrix4f::rotation(qOriginal);
				REQUIRE_THAT(Quaternionf::fromMatrix4x4(mat4.data()), Catch::Matchers::Predicate<Quaternionf>(pred));
			}

			for (float angle = 0.0f; angle < 720.0f; angle += 10.0f)
			{
				auto qOriginal = Quaternionf::fromAxisAngle(Vector3f{0.0f, 0.0f, 1.0f}, angle);

				auto pred = [qOriginal](const Quaternionf& q) -> bool
				{
					auto matcher = QuaternionEquals(qOriginal);
					return matcher.match(q) || matcher.match(-q);
				};

				auto mat3 = Matrix3f::rotation(qOriginal);
				REQUIRE_THAT(Quaternionf::fromMatrix3x3(mat3.data()), Catch::Matchers::Predicate<Quaternionf>(pred));

				auto mat4 = Matrix4f::rotation(qOriginal);
				REQUIRE_THAT(Quaternionf::fromMatrix4x4(mat4.data()), Catch::Matchers::Predicate<Quaternionf>(pred));
			}

			for (float angle = 0.0f; angle < 720.0f; angle += 10.0f)
			{
				auto qOriginal = Quaternionf::fromAxisAngle(Vector3f{0.0f, 0.0f, 1.0f}, angle);
				qOriginal *= Quaternionf::fromAxisAngle(Vector3f{0.0f, 1.0f, 0.0f}, angle);
				qOriginal *= Quaternionf::fromAxisAngle(Vector3f{1.0f, 0.0f, 0.0f}, angle);

				auto pred = [qOriginal](const Quaternionf& q) -> bool
				{
					auto matcher = QuaternionEquals(qOriginal);
					return matcher.match(q) || matcher.match(-q);
				};

				auto mat3 = Matrix3f::rotationX(angle);
				mat3 *= Matrix3f::rotationY(angle);
				mat3 *= Matrix3f::rotationZ(angle);
				REQUIRE_THAT(Quaternionf::fromMatrix3x3(mat3.data()), Catch::Matchers::Predicate<Quaternionf>(pred));

				auto mat4 = Matrix4f::rotationX(angle);
				mat4 *= Matrix4f::rotationY(angle);
				mat4 *= Matrix4f::rotationZ(angle);
				REQUIRE_THAT(Quaternionf::fromMatrix4x4(mat4.data()), Catch::Matchers::Predicate<Quaternionf>(pred));
			}
		}

		/************
		* NOTE: missing tests:
		*  - Quaternionf::scaleAngle
		*/
	}

	TEST_CASE("Quaternion of type double", "[common][quaternion][quaterniond]")
	{
		SECTION("init")
		{
			REQUIRE_THAT(Quaterniond::identity(), QuaternionEquals(0.0, 0.0, 0.0, 1.0));
			REQUIRE_THAT(Quaterniond::zero(), QuaternionEquals(0.0, 0.0, 0.0, 0.0));
			REQUIRE_THAT(Quaterniond::from(1.0, 2.0, 3.0, 4.0), QuaternionEquals(1.0, 2.0, 3.0, 4.0));
			REQUIRE_THAT(Quaterniond::from(1.0, 2.0, 3.0, 4.0), QuaternionEquals(1.0, 2.0, 3.0, 4.0));

			float valuesf[4]{-0.21f, 55.11f, -0.0001f, 234.77f};
			double valuesd[4]{2.5632, -24.449, -0.999601, 0.9912902};

			REQUIRE_THAT(Quaterniond::from(std::span<const float>(valuesf, 4)), QuaternionEquals(-0.21, 55.11, -0.0001, 234.77));
			REQUIRE_THAT(Quaterniond::from(std::span<const double>(valuesd, 4)), QuaternionEquals(2.5632, -24.449, -0.999601, 0.9912902));
			//REQUIRE_THAT(Quaterniond::from(std::span<const float>(valuesf, 2)), QuaternionEquals(Quaterniond::identity()));
			//REQUIRE_THAT(Quaterniond::from(std::span<const double>(valuesd, 1)), QuaternionEquals(Quaterniond::identity()));

			Vector3d vec3{-12.55, -0.91175, 0.123};
			Vector4d vec4{5.9682, -12.55, 0.123, -0.91175};
			REQUIRE_THAT(Quaterniond::from(vec3, 0.3389), QuaternionEquals(-12.55, -0.91175, 0.123, 0.3389));
			REQUIRE_THAT(Quaterniond::from(vec4), QuaternionEquals(5.9682, -12.55, 0.123, -0.91175));
		}

		SECTION("access")
		{
			auto q = Quaterniond::from(1.0, 2.0, 3.0, 4.0);

			REQUIRE(q[0] == Catch::Approx(1.0));
			REQUIRE(q[1] == Catch::Approx(2.0));
			REQUIRE(q[2] == Catch::Approx(3.0));
			REQUIRE(q[3] == Catch::Approx(4.0));
			REQUIRE(q[4] == Catch::Approx(1.0));
			REQUIRE(q[5] == Catch::Approx(2.0));
			REQUIRE(q[6] == Catch::Approx(3.0));
			REQUIRE(q[7] == Catch::Approx(4.0));

			REQUIRE(q.data()[0] == Catch::Approx(1.0));
			REQUIRE(q.data()[1] == Catch::Approx(2.0));
			REQUIRE(q.data()[2] == Catch::Approx(3.0));
			REQUIRE(q.data()[3] == Catch::Approx(4.0));
		}

		SECTION("basic operators")
		{
			auto qa = Quaterniond::from(1.0, 2.0, 3.0, 4.0);
			auto qb = Quaterniond::from(0.1, -0.2, 0.3, -0.4);

			/*
			* NOTES:
			*  - operation "*" is quaternion multiplication, not per-element multiplication
			*  - operation "/" is actually the inverse / conjugate
			*/

			REQUIRE_THAT(-qa, QuaternionEquals(-1.0, -2.0, -3.0, -4.0));
			REQUIRE_THAT(-qb, QuaternionEquals(-0.1, 0.2, -0.3, 0.4));
			REQUIRE_THAT(qa + qb, QuaternionEquals(1.1, 1.8, 3.3, 3.6));
			REQUIRE_THAT(qa - qb, QuaternionEquals(0.9, 2.2, 2.7, 4.4));

			auto qc = qa;
			qc += qb;
			REQUIRE_THAT(qc, QuaternionEquals(1.1, 1.8, 3.3, 3.6));

			qc = qa;
			qc -= qb;
			REQUIRE_THAT(qc, QuaternionEquals(0.9, 2.2, 2.7, 4.4));
		}

		SECTION("scaling")
		{
			auto q = Quaterniond::from(1.0, 2.0, 3.0, 4.0);

			q = q * 2.0;
			REQUIRE_THAT(q, QuaternionEquals(2.0, 4.0, 6.0, 8.0));

			q *= 0.5;
			REQUIRE_THAT(q, QuaternionEquals(1.0, 2.0, 3.0, 4.0));
		}

		SECTION("normalize")
		{
			auto qa = Quaterniond::from(1.0, 1.0, 1.0, 1.0);
			qa.normalize();
			REQUIRE_THAT(qa, QuaternionEquals(0.5, 0.5, 0.5, 0.5));

			auto qb = qa;
			qb.normalize();
			REQUIRE_THAT(qb, QuaternionEquals(qa));
		}

		SECTION("conjugate")
		{
			auto q = Quaterniond::from(0.0, 0.0, 0.0, 1.0);
			REQUIRE_THAT(q.getConjugate(), QuaternionEquals(0.0, 0.0, 0.0, 1.0));
			q.conjugate();
			REQUIRE_THAT(q, QuaternionEquals(0.0, 0.0, 0.0, 1.0));

			q = Quaterniond::from(-1.0, 2.0, -3.0, 2.0);
			REQUIRE_THAT(q.getConjugate(), QuaternionEquals(1.0, -2.0, 3.0, 2.0));
			q.conjugate();
			REQUIRE_THAT(q, QuaternionEquals(1.0, -2.0, 3.0, 2.0));
		}

		SECTION("inverse")
		{
			{
				auto qa = Quaterniond::from(1.0, -2.0, 1.0, 3.0);
				auto qb = Quaterniond::from(-1.0, 2.0, 3.0, 2.0);

				REQUIRE_THAT(qa * qa.getInverse<false>(), QuaternionEquals(0.0, 0.0, 0.0, 1.0));
				REQUIRE_THAT(qa * qa.getInverse<false>() * qb, QuaternionEquals(qb));

				qa.normalize();
				qb.normalize();
				REQUIRE_THAT(qa * qa.getInverse<true>(), QuaternionEquals(0.0, 0.0, 0.0, 1.0));
				REQUIRE_THAT(qa * qa.getInverse<true>() * qb, QuaternionEquals(qb));
			}

			{
				auto qa = Quaterniond::from(1.0, -2.0, 1.0, 3.0);
				auto qb = Quaterniond::from(-1.0, 2.0, 3.0, 2.0);

				auto qx = qa;
				qx.inverse<false>();
				REQUIRE_THAT(qa * qx, QuaternionEquals(0.0, 0.0, 0.0, 1.0));
				REQUIRE_THAT(qa * qx * qb, QuaternionEquals(qb));

				qa.normalize();
				qb.normalize();

				qx = qa;
				qx.inverse<true>();
				REQUIRE_THAT(qa * qx, QuaternionEquals(0.0, 0.0, 0.0, 1.0));
				REQUIRE_THAT(qa * qx * qb, QuaternionEquals(qb));
			}
		}

		SECTION("axis angle X")
		{
			REQUIRE_THAT(Quaterniond::fromAxisAngle(Vector3d{1.0, 0.0, 0.0}, 120.0), QuaternionEquals(0.866025, 0.0, 0.0, 0.5));
			REQUIRE_THAT(Quaterniond::fromAxisAngle(1.0, 0.0, 0.0, 120.0), QuaternionEquals(0.866025, 0.0, 0.0, 0.5));
		}

		SECTION("axis angle Y")
		{
			REQUIRE_THAT(Quaterniond::fromAxisAngle(Vector3d{0.0, 1.0, 0.0}, 30.0), QuaternionEquals(0.0, 0.258819, 0.0, 0.965926));
			REQUIRE_THAT(Quaterniond::fromAxisAngle(0.0, 1.0, 0.0, 30.0), QuaternionEquals(0.0, 0.258819, 0.0, 0.965926));
		}

		SECTION("axis angle Z")
		{
			REQUIRE_THAT(Quaterniond::fromAxisAngle(Vector3d{0.0, 0.0, 1.0}, 60.0), QuaternionEquals(0.0, 0.0, 0.5, 0.866025));
			REQUIRE_THAT(Quaterniond::fromAxisAngle(0.0, 0.0, 1.0, 60.0), QuaternionEquals(0.0, 0.0, 0.5, 0.866025));
		}

		SECTION("axis angle all")
		{
			Vector3d vec{1.0, 2.0, 0.5};
			vec.normalize();

			REQUIRE_THAT(Quaterniond::fromAxisAngle(vec, 35.0), QuaternionEquals(0.131239, 0.262478, 0.0656194, 0.953717));
			REQUIRE_THAT(Quaterniond::fromAxisAngle(vec[0], vec[1], vec[2], 35.0), QuaternionEquals(0.131239, 0.262478, 0.0656194, 0.953717));
		}

		SECTION("euler angles")
		{
			auto q_yaw = Quaterniond::fromEuler(0.0, 45.0, 0.0, Quaterniond::AxisOrder::XYZ);
			REQUIRE_THAT(q_yaw, QuaternionEquals(0.0, 0.382684, 0.0, 0.923879));

			auto q_pitch = Quaterniond::fromEuler(30.0, 0.0, 0.0, Quaterniond::AxisOrder::XYZ);
			REQUIRE_THAT(q_pitch, QuaternionEquals(0.258819, 0.0, 0.0, 0.965926));

			auto q_roll = Quaterniond::fromEuler(0.0, 0.0, 10.0, Quaterniond::AxisOrder::XYZ);
			REQUIRE_THAT(q_roll, QuaternionEquals(0.0, 0.0, 0.0871558, 0.996195));

			auto q_ypr = Quaterniond::fromEuler(30.0, 45.0, 10.0, Quaterniond::AxisOrder::YXZ);
			REQUIRE_THAT(q_yaw * q_pitch * q_roll, QuaternionEquals(q_ypr));
		}

		SECTION("vector rotation")
		{
			auto qa = Quaterniond::from(0.3919183, 0.3196269, -0.8430416, -0.1830837);
			REQUIRE(qa.magnitude() == Catch::Approx(1.0));
			{
				Vector3d vec{0.3535534, -0.1464466, 0.3535534}, res;

				qa.unitRotate(vec, res);
				REQUIRE_THAT(qa.unitRotate(vec), VectorEquals(-0.487732, 0.1646256, 0.08039001));
				REQUIRE_THAT(res, VectorEquals(-0.487732, 0.1646256, 0.08039001));
				REQUIRE_THAT(res, VectorEquals(qa.unitRotate(vec)));
			}

			auto qb = Quaterniond::from(0.336838, 0.0115086, 0.421048, 0.842096);
			REQUIRE(qb.magnitude() == Catch::Approx(1.0));
			{
				Vector3d vec{0.53, -0.0532, 0.22}, res;

				qb.unitRotate(vec, res);
				REQUIRE_THAT(qb.unitRotate(vec), VectorEquals(0.4459215, 0.2350067, 0.2793851));
				REQUIRE_THAT(res, VectorEquals(0.4459215, 0.2350067, 0.2793851));
				REQUIRE_THAT(res, VectorEquals(qb.unitRotate(vec)));
			}

			auto qc = Quaterniond::from(0.0, 0.7071068, 0.0, 0.7071068);
			REQUIRE(qc.magnitude() == Catch::Approx(1.0));
			{
				Vector3d vec{13.0, -1.23, 3.4}, res;

				qc.unitRotate(vec, res);
				REQUIRE_THAT(qc.unitRotate(vec), VectorEquals(3.4, -1.23, -13.0));
				REQUIRE_THAT(res, VectorEquals(3.4, -1.23, -13.0));
				REQUIRE_THAT(res, VectorEquals(qc.unitRotate(vec)));
			}
		}

		SECTION("angles between vectors")
		{
			auto q = Quaterniond::identity();

			q = Quaterniond::fromVectors(Vector3d::calcNormalize(Vector3d{0.8, 1.4, 2.6}), Vector3d::calcNormalize(Vector3d{1.2, 0.3, -2.9}));
			REQUIRE_THAT(q, QuaternionEquals(-0.590233, 0.6634023, -0.1756065, 0.4250704));

			q = Quaterniond::fromVectors(Vector3d{0.0, 1.0, 0.0}, Vector3d{0.0, 0.0, 1.0});
			REQUIRE_THAT(q, QuaternionEquals(0.7071068, 0.0, 0.0, 0.7071068));

			q = Quaterniond::fromVectors(Vector3d::calcNormalize(Vector3d{-1.0, 0.4, 2.9}), Vector3d::calcNormalize(Vector3d{0.8, 1.4, 2.6}));
			REQUIRE_THAT(q, QuaternionEquals(-0.1695316, 0.2761906, -0.09655445, 0.9410924));

			q = Quaterniond::fromVectors(Vector3d::calcNormalize(Vector3d{0.8, -1.4, 2.6}), Vector3d::calcNormalize(Vector3d{0.8, -1.4, 2.6}));
			REQUIRE_THAT(q, QuaternionEquals(0.0, 0.0, 0.0, 1.0));

			//opposite vectors
			auto v1 = Vector3d{-0.8, 1.4, -2.6};
			auto v2 = Vector3d{0.8, -1.4, 2.6};
			q = Quaterniond::fromVectors(v1, v2);
			REQUIRE_THAT(q.unitRotate(v1), VectorEquals(v2));
		}

		SECTION("dot")
		{
			auto qa = Quaterniond::from(-27.0, 83.0, 32.0, -153.0);
			auto qb = Quaterniond::from(36.0, -64.0, 12.0, 24.0);

			REQUIRE(qa.dot(qb) == Catch::Approx(-9572.0));
			REQUIRE(qb.dot(qa) == Catch::Approx(-9572.0));
			REQUIRE(qa.magnitude() == Catch::Approx(std::sqrt(qa.dot(qa))));
			REQUIRE(qa.magnitudeSquared() == Catch::Approx(qa.dot(qa)));

			qa = Quaterniond::from(0.3535534, -0.1464466, 0.3535534, 0.8535535);
			qb = Quaterniond::from(0.3919183, 0.3196269, -0.8430416, -0.1830837);
			REQUIRE(qa.dot(qb) == Catch::Approx(-0.362576186656952));

			qa = Quaterniond::from(0.0, 0.7071068, 0.0, 0.7071068);
			qb = Quaterniond::from(0.0, 0.7071068, 0.0, 0.7071068);
			REQUIRE(qa.dot(qb) == Catch::Approx(1.0));
		}

		SECTION("rotation")
		{
			//identity
			auto q = Quaterniond::identity();
			REQUIRE_THAT(q.unitRotate(Vector3d{1.0, 1.0, 1.0}), VectorEquals(1.0, 1.0, 1.0));

			//along the X axis
			q = Quaterniond::fromAxisAngle(Vector3d{1.0, 0.0, 0.0}, 90.0);
			REQUIRE_THAT(q.unitRotate(Vector3d{0.0, 1.0, 0.0}), VectorEquals(0.0, 0.0, 1.0));

			q = Quaterniond::fromAxisAngle(Vector3d{1.0, 0.0, 0.0}, 180.0);
			REQUIRE_THAT(q.unitRotate(Vector3d{0.0, 1.0, 0.0}), VectorEquals(0.0, -1.0, 0.0));

			q = Quaterniond::fromAxisAngle(Vector3d{1.0, 0.0, 0.0}, 270.0);
			REQUIRE_THAT(q.unitRotate(Vector3d{0.0, 1.0, 0.0}), VectorEquals(0.0, 0.0, -1.0));

			//along the Y axis
			q = Quaterniond::fromAxisAngle(Vector3d{0.0, 1.0, 0.0}, 90.0);
			REQUIRE_THAT(q.unitRotate(Vector3d{1.0, 0.0, 0.0}), VectorEquals(0.0, 0.0, -1.0));

			q = Quaterniond::fromAxisAngle(Vector3d{0.0, 1.0, 0.0}, 180.0);
			REQUIRE_THAT(q.unitRotate(Vector3d{1.0, 0.0, 0.0}), VectorEquals(-1.0, 0.0, 0.0));

			q = Quaterniond::fromAxisAngle(Vector3d{0.0, 1.0, 0.0}, 270.0);
			REQUIRE_THAT(q.unitRotate(Vector3d{1.0, 0.0, 0.0}), VectorEquals(0.0, 0.0, 1.0));

			//along the Z axis
			q = Quaterniond::fromAxisAngle(Vector3d{0.0, 0.0, 1.0}, 90.0);
			REQUIRE_THAT(q.unitRotate(Vector3d{0.0, 1.0, 0.0}), VectorEquals(-1.0, 0.0, 0.0));

			q = Quaterniond::fromAxisAngle(Vector3d{0.0, 0.0, 1.0}, 180.0);
			REQUIRE_THAT(q.unitRotate(Vector3d{0.0, 1.0, 0.0}), VectorEquals(0.0, -1.0, 0.0));

			q = Quaterniond::fromAxisAngle(Vector3d{0.0, 0.0, 1.0}, 270.0);
			REQUIRE_THAT(q.unitRotate(Vector3d{0.0, 1.0, 0.0}), VectorEquals(1.0, 0.0, 0.0));
		}

		SECTION("slerp")
		{
			{
				auto qa = Quaterniond::from(-0.270598, 0.270598, 0.6532815, 0.6532815);
				auto qb = Quaterniond::from(0.5, 0.5, 0.5, 0.5);
				auto q = Quaterniond::identity();

				q = Quaterniond::sLerp(qa, qb, -0.1);
				REQUIRE_THAT(q, QuaternionEquals(qa));
				q = Quaterniond::sLerp(qa, qb, 0.0);
				REQUIRE_THAT(q, QuaternionEquals(qa));
				q = Quaterniond::sLerp(qa, qb, 1.0);
				REQUIRE_THAT(q, QuaternionEquals(qb));
				q = Quaterniond::sLerp(qa, qb, 1.1);
				REQUIRE_THAT(q, QuaternionEquals(qb));

				q = Quaterniond::sLerp(qa, qb, 0.62);
				REQUIRE_THAT(q, QuaternionEquals(0.22069444274723088, 0.4498729015909088, 0.6119266025696755, 0.6119266025696755));
			}

			{
				auto qa = Quaterniond::fromAxisAngle(Vector3d{1.0, 0.0, 0.0}, 0.0);
				auto qb = Quaterniond::fromAxisAngle(Vector3d{1.0, 0.0, 0.0}, 90.0);
				auto q = Quaterniond::identity();

				REQUIRE(qa.magnitude() == Catch::Approx(1.0));
				REQUIRE(qb.magnitude() == Catch::Approx(1.0));

				q = Quaterniond::sLerp(qa, qb, -0.1);
				REQUIRE_THAT(q, QuaternionEquals(qa));
				q = Quaterniond::sLerp(qa, qb, 0.0);
				REQUIRE_THAT(q, QuaternionEquals(qa));
				q = Quaterniond::sLerp(qa, qb, 1.0);
				REQUIRE_THAT(q, QuaternionEquals(qb));
				q = Quaterniond::sLerp(qa, qb, 1.1);
				REQUIRE_THAT(q, QuaternionEquals(qb));

				q = Quaterniond::sLerp(qa, qb, 0.5);
				REQUIRE_THAT(q, QuaternionEquals(Quaterniond::fromAxisAngle(Vector3d{1.0, 0.0, 0.0}, 45.0)));
			}
		}

		SECTION("nlerp")
		{
			auto qa = Quaterniond::from(-1.0, 2.0, -3.0, 4.0);
			auto qb = Quaterniond::from(1.0, 3.0, 0.0, 4.8);
			auto q = Quaterniond::identity();

			q = Quaterniond::nLerp(qa, qb, -0.1);
			REQUIRE_THAT(q, QuaternionEquals(qa));
			q = Quaterniond::nLerp(qa, qb, 0.0);
			REQUIRE_THAT(q, QuaternionEquals(qa));
			q = Quaterniond::nLerp(qa, qb, 1.0);
			REQUIRE_THAT(q, QuaternionEquals(qb));
			q = Quaterniond::nLerp(qa, qb, 1.1);
			REQUIRE_THAT(q, QuaternionEquals(qb));

			q = Quaterniond::nLerp(qa, qb, 0.5);

			auto res = Quaterniond::from(0.0, 2.5, -1.5, 4.4);
			res.normalize();
			REQUIRE_THAT(q, QuaternionEquals(res));
		}

		SECTION("to angle axis")
		{
			{
				auto q = Quaterniond::from(0.6514133, -0.1282655, 0.6116868, 0.430172);

				double angle;
				Vector3d axis;
				q.getAxisAngle(axis, angle);

				REQUIRE(angle == Catch::Approx(hr::Math::Rad2Deg<double> * 2.25223));
				REQUIRE_THAT(axis, VectorEquals(0.7215902, -0.1420836, 0.6775839));
			}

			{
				auto q = Quaterniond::from(0.0, 0.0, 0.0, 1.0);

				double angle;
				Vector3d axis;
				q.getAxisAngle(axis, angle);

				REQUIRE(angle == Catch::Approx(hr::Math::Rad2Deg<double> * 0.0));
				REQUIRE_THAT(axis, VectorEquals(1.0, 0.0, 0.0));
			}

			{
				auto q = Quaterniond::from(0.1164578, 0.4874545, 0.8652994, 0.009090029);

				double angle;
				Vector3d axis;
				q.getAxisAngle(axis, angle);

				REQUIRE(angle == Catch::Approx(hr::Math::Rad2Deg<double> * 3.12341));
				REQUIRE_THAT(axis, VectorEquals(0.1164626, 0.4874747, 0.8653352));
			}
		}

		SECTION("to euler angles")
		{
			{
				auto q = Quaterniond::from(0.6514133, -0.1282655, 0.6116868, 0.430172);

				double angles[3];
				q.getEulerAngles(angles[0], angles[1], angles[2]);

				REQUIRE(angles[0] == Catch::Approx(0.8));
				REQUIRE(angles[1] == Catch::Approx(1.4));
				REQUIRE(angles[2] == Catch::Approx(2.6));
			}

			{
				auto q = Quaterniond::from(0.0, 0.0, 1.2, 0.0);

				double angles[3];
				q.getEulerAngles(angles[0], angles[1], angles[2]);

				REQUIRE(angles[0] == Catch::Approx(0.0));
				REQUIRE(angles[1] == Catch::Approx(0.0));
				REQUIRE(angles[2] == Catch::Approx(3.14159265));
			}

			{
				auto q = Quaterniond::from(0.1164578, 0.4874545, 0.8652994, 0.009090029);

				double angles[3];
				q.getEulerAngles(angles[0], angles[1], angles[2]);

				REQUIRE(angles[0] == Catch::Approx(-1.0));
				REQUIRE(angles[1] == Catch::Approx(0.4));
				REQUIRE(angles[2] == Catch::Approx(2.9));
			}
		}

		SECTION("comparison")
		{
			auto qa = Quaterniond::from(0.6514133, -0.1282655, 0.6116868, 0.430172);
			auto qb = Quaterniond::from(-0.21, 55.11, -0.0001, 234.77);

			REQUIRE(qb.isEqual(qb, 0.0000000001));
			REQUIRE_FALSE(qb.isEqual(qa, 0.0000000001));
			REQUIRE(Quaterniond::from(0.001, 0.002, 0.003, 0.004).isEqual(Quaterniond::from(0.002, 0.003, 0.004, 0.005), 0.009));
			REQUIRE_FALSE(Quaterniond::from(0.001, 0.002, 0.003, 0.004).isEqual(Quaterniond::from(0.002, 0.003, 0.004, 0.005), 0.0009));
		}

		SECTION("convert")
		{
			auto q = Quaterniond::from(0.6514133, -0.1282655, 0.6116868, 0.430172);
			auto qf = q.convert<float>();
			auto qd = q.convert<double>();

			REQUIRE_THAT(qf, QuaternionEquals(0.6514133f, -0.1282655f, 0.6116868f, 0.430172f));
			REQUIRE_THAT(qd, QuaternionEquals(0.6514133, -0.1282655, 0.6116868, 0.430172));
		}

		SECTION("multiplication order")
		{
			{
				auto q1 = Quaterniond::from(1.0, 2.0, 3.0, 4.0);
				auto q2 = Quaterniond::from(5.0, 6.0, 7.0, 8.0);
				auto qr = q1 * q2;
				REQUIRE_THAT(qr, QuaternionEquals(24.0, 48.0, 48.0, -6.0));
			}

			{
				auto q1 = Quaterniond::fromAxisAngle(Vector3d{0.0, 1.0, 0.0}, 90.0);
				auto q2 = Quaterniond::fromAxisAngle(Vector3d{1.0, 0.0, 0.0}, 90.0);
				auto qFinal = q2 * q1;
				REQUIRE_THAT(qFinal.unitRotate(Vector3d{1.0, 0.0, 0.0}), VectorEquals(0.0, 1.0, 0.0));
				REQUIRE_THAT(qFinal.unitRotate(Vector3d{-0.9, 0.0, 0.0}), VectorEquals(0.0, -0.9, 0.0));
			}
		}

		SECTION("from matrix")
		{
			{
				const double matIdentity[]{1.0, 0.0, 0.0, 0.0, 1.0, 0.0, 0.0, 0.0, 1.0};
				auto q = Quaterniond::fromMatrix3x3(matIdentity);
				REQUIRE_THAT(q, QuaternionEquals(0.0, 0.0, 0.0, 1.0));
			}

			{
				const double matIdentity[]{1.0, 0.0, 0.0, 0.0, 0.0, 1.0, 0.0, 0.0, 0.0, 0.0, 1.0, 0.0, 0.0, 0.0, 0.0, 1.0};
				auto q = Quaterniond::fromMatrix4x4(matIdentity);
				REQUIRE_THAT(q, QuaternionEquals(0.0, 0.0, 0.0, 1.0));
			}

			for (double angle = 0.0; angle < 720.0; angle += 10.0)
			{
				auto qOriginal = Quaterniond::fromAxisAngle(Vector3d{1.0, 0.0, 0.0}, angle);

				auto pred = [qOriginal](const Quaterniond& q) -> bool
				{
					auto matcher = QuaternionEquals(qOriginal);
					return matcher.match(q) || matcher.match(-q);
				};

				auto mat3 = Matrix3d::rotation(qOriginal);
				REQUIRE_THAT(Quaterniond::fromMatrix3x3(mat3.data()), Catch::Matchers::Predicate<Quaterniond>(pred));

				auto mat4 = Matrix4d::rotation(qOriginal);
				REQUIRE_THAT(Quaterniond::fromMatrix4x4(mat4.data()), Catch::Matchers::Predicate<Quaterniond>(pred));
			}

			for (double angle = 0.0; angle < 720.0; angle += 10.0)
			{
				auto qOriginal = Quaterniond::fromAxisAngle(Vector3d{0.0, 1.0, 0.0}, angle);

				auto pred = [qOriginal](const Quaterniond& q) -> bool
				{
					auto matcher = QuaternionEquals(qOriginal);
					return matcher.match(q) || matcher.match(-q);
				};

				auto mat3 = Matrix3d::rotation(qOriginal);
				REQUIRE_THAT(Quaterniond::fromMatrix3x3(mat3.data()), Catch::Matchers::Predicate<Quaterniond>(pred));

				auto mat4 = Matrix4d::rotation(qOriginal);
				REQUIRE_THAT(Quaterniond::fromMatrix4x4(mat4.data()), Catch::Matchers::Predicate<Quaterniond>(pred));
			}

			for (double angle = 0.0; angle < 720.0; angle += 10.0)
			{
				auto qOriginal = Quaterniond::fromAxisAngle(Vector3d{0.0, 0.0, 1.0}, angle);

				auto pred = [qOriginal](const Quaterniond& q) -> bool
				{
					auto matcher = QuaternionEquals(qOriginal);
					return matcher.match(q) || matcher.match(-q);
				};

				auto mat3 = Matrix3d::rotation(qOriginal);
				REQUIRE_THAT(Quaterniond::fromMatrix3x3(mat3.data()), Catch::Matchers::Predicate<Quaterniond>(pred));

				auto mat4 = Matrix4d::rotation(qOriginal);
				REQUIRE_THAT(Quaterniond::fromMatrix4x4(mat4.data()), Catch::Matchers::Predicate<Quaterniond>(pred));
			}

			for (double angle = 0.0; angle < 720.0; angle += 10.0)
			{
				auto qOriginal = Quaterniond::fromAxisAngle(Vector3d{0.0, 0.0, 1.0}, angle);
				qOriginal *= Quaterniond::fromAxisAngle(Vector3d{0.0, 1.0, 0.0}, angle);
				qOriginal *= Quaterniond::fromAxisAngle(Vector3d{1.0, 0.0, 0.0}, angle);

				auto pred = [qOriginal](const Quaterniond& q) -> bool
				{
					auto matcher = QuaternionEquals(qOriginal);
					return matcher.match(q) || matcher.match(-q);
				};

				auto mat3 = Matrix3d::rotationX(angle);
				mat3 *= Matrix3d::rotationY(angle);
				mat3 *= Matrix3d::rotationZ(angle);
				REQUIRE_THAT(Quaterniond::fromMatrix3x3(mat3.data()), Catch::Matchers::Predicate<Quaterniond>(pred));

				auto mat4 = Matrix4d::rotationX(angle);
				mat4 *= Matrix4d::rotationY(angle);
				mat4 *= Matrix4d::rotationZ(angle);
				REQUIRE_THAT(Quaterniond::fromMatrix4x4(mat4.data()), Catch::Matchers::Predicate<Quaterniond>(pred));
			}
		}

		/************
		* NOTE: missing tests:
		*  - Quaterniond::scaleAngle
		*/
	}
}
