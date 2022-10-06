#pragma once

#include "quaternion.hpp"

#include <format>
#include <optional>

#include "libs/catch2/catch.hpp"

namespace Catch
{
	template<>
	struct StringMaker<hr::Quaternion>
	{
		static std::string convert(hr::Quaternion const& q)
		{
			return std::format("Quaternion is: {{{}, {}, {}, {}}}", q[0], q[1], q[2], q[3]);
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
		struct QuaternionEqualsMatcher : Catch::Matchers::MatcherGenericBase
		{
			QuaternionEqualsMatcher(Quaternion q)
			{
				m_quat = std::move(q);
			}

			QuaternionEqualsMatcher(const float qx, const float qy, const float qz, const float qw)
			{ 
				m_quat = Quaternion{qx, qy, qz, qw};
			}

			bool match(const Quaternion& q) const
			{
				//set epsilon to allowed a 0.1% difference and a margin to allow (0.0f == -0.0f) to pass
				return (q[0] == Catch::Approx(m_quat[0]).epsilon(0.001).margin(0.0000001)) 
					&& (q[1] == Catch::Approx(m_quat[1]).epsilon(0.001).margin(0.0000001))
					&& (q[2] == Catch::Approx(m_quat[2]).epsilon(0.001).margin(0.0000001))
					&& (q[3] == Catch::Approx(m_quat[3]).epsilon(0.001).margin(0.0000001));
			}

			std::string describe() const override
			{
				return std::format("Quaternion is: {{{}, {}, {}, {}}}", m_quat[0], m_quat[1], m_quat[2], m_quat[3]);
			}

		private:
			Quaternion m_quat;
		};

		auto QuaternionEquals(const float qx, const float qy, const float qz, const float qw) -> QuaternionEqualsMatcher
		{
			return QuaternionEqualsMatcher{qx, qy, qz, qw};
		}

		auto QuaternionEquals(Quaternion q) -> QuaternionEqualsMatcher
		{
			return QuaternionEqualsMatcher{std::move(q)};
		}

		struct VectorEqualsMatcher : Catch::Matchers::MatcherGenericBase
		{
			VectorEqualsMatcher(float x, float y, float z)
			  : m_x{x}, m_y{y}, m_z{z}
			{ }

			bool match(const Vector3f& v) const
			{
				//set epsilon to allowed a 0.1% difference and a margin to allow (0.0f == -0.0f) to pass
				return (v[0] == Catch::Approx(m_x).epsilon(0.001).margin(0.0000001))
					&& (v[1] == Catch::Approx(m_y).epsilon(0.001).margin(0.0000001))
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
	}

	TEST_CASE("Quaternion init", "[common][quaternion]")
	{
		SECTION("init")
		{
			REQUIRE_THAT(Quaternion(), QuaternionEquals(0.0f, 0.0f, 0.0f, 1.0f));
			REQUIRE_THAT(Quaternion(1.0f, 2.0f, 3.0f, 4.0f), QuaternionEquals(1.0f, 2.0f, 3.0f, 4.0f));
			REQUIRE_THAT(Quaternion(1.0, 2.0, 3.0, 4.0), QuaternionEquals(1.0f, 2.0f, 3.0f, 4.0f));
		}

		SECTION("access")
		{
			Quaternion q(1.0f, 2.0f, 3.0f, 4.0f);

			REQUIRE(q[0] == Catch::Approx(1.0f));
			REQUIRE(q[1] == Catch::Approx(2.0f));
			REQUIRE(q[2] == Catch::Approx(3.0f));
			REQUIRE(q[3] == Catch::Approx(4.0f));
			REQUIRE(q[4] == Catch::Approx(1.0f));
			REQUIRE(q[5] == Catch::Approx(2.0f));
			REQUIRE(q[6] == Catch::Approx(3.0f));
			REQUIRE(q[7] == Catch::Approx(4.0f));
		}

		SECTION("basic operators")
		{
			Quaternion qa(1.0f, 2.0f, 3.0f, 4.0f);
			Quaternion qb(0.1f, -0.2f, 0.3f, -0.4f);

			/*
			* NOTES:
			*  - operation "*" is quaternion multiplication, not per-element multiplication
			*  - operation "/" is actually the inverse / conjugate
			*/

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
			Quaternion q(1.0, 2.0, 3.0, 4.0);

			q = q * 2.0;
			REQUIRE_THAT(q, QuaternionEquals(2.0f, 4.0f, 6.0f, 8.0f));

			q *= 0.5;
			REQUIRE_THAT(q, QuaternionEquals(1.0f, 2.0f, 3.0f, 4.0f));
		}

		SECTION("normalize")
		{
			Quaternion qa(1.0f, 1.0f, 1.0f, 1.0f);
			qa.normalize();
			REQUIRE_THAT(qa, QuaternionEquals(0.5f, 0.5f, 0.5f, 0.5f));

			auto qb = qa;
			qb.normalize();
			REQUIRE_THAT(qb, QuaternionEquals(qa));
		}

		SECTION("inverse / conjugate")
		{
			Quaternion qa(1.0f, -2.0f, 1.0f, 3.0f);
			Quaternion qb(-1.0f, 2.0f, 3.0f, 2.0f);
			qa.normalize();
			qb.normalize();

			REQUIRE_THAT(qa * qa.getConjugate(), QuaternionEquals(0.0f, 0.0f, 0.0f, 1.0f));
			REQUIRE_THAT(qa / qa, QuaternionEquals(0.0f, 0.0f, 0.0f, 1.0f));

			REQUIRE_THAT(qa * qa.getConjugate() * qb, QuaternionEquals(qb));
			REQUIRE_THAT(qa / qa * qb, QuaternionEquals(qb));
		}

		SECTION("axis angle X")
		{
			REQUIRE_THAT(Quaternion::genAxisAngle(Vector3f{1.0f, 0.0f, 0.0f}, 120.0f), QuaternionEquals(0.866025f, 0.0f, 0.0f, 0.5f));
			REQUIRE_THAT(Quaternion::genAxisAngle(1.0f, 0.0f, 0.0f, 120.0f), QuaternionEquals(0.866025f, 0.0f, 0.0f, 0.5f));
		}

		SECTION("axis angle Y")
		{
			REQUIRE_THAT(Quaternion::genAxisAngle(Vector3f{0.0f, 1.0f, 0.0f}, 30.0f), QuaternionEquals(0.0f, 0.258819f, 0.0f, 0.965926f));
			REQUIRE_THAT(Quaternion::genAxisAngle(0.0f, 1.0f, 0.0f, 30.0f), QuaternionEquals(0.0f, 0.258819f, 0.0f, 0.965926f));
		}

		SECTION("axis angle Z")
		{
			REQUIRE_THAT(Quaternion::genAxisAngle(Vector3f{0.0f, 0.0f, 1.0f}, 60.0f), QuaternionEquals(0.0f, 0.0f, 0.5f, 0.866025f));
			REQUIRE_THAT(Quaternion::genAxisAngle(0.0f, 0.0f, 1.0f, 60.0f), QuaternionEquals(0.0f, 0.0f, 0.5f, 0.866025f));
		}

		SECTION("axis angle all")
		{
			Vector3f vec{1.0, 2.0, 0.5};
			vec.normalize();

			REQUIRE_THAT(Quaternion::genAxisAngle(vec, 35.0f), QuaternionEquals(0.131239f, 0.262478f, 0.0656194f, 0.953717f));
			REQUIRE_THAT(Quaternion::genAxisAngle(vec[0], vec[1], vec[2], 35.0f), QuaternionEquals(0.131239f, 0.262478f, 0.0656194f, 0.953717f));
		}

		SECTION("euler angles")
		{
			Quaternion q_yaw, q_pitch, q_roll, q_ypr;

			q_yaw.setFromEuler(0.0f, 45.0f, 0.0f, Quaternion::AxisOrder::XYZ);
			REQUIRE_THAT(q_yaw, QuaternionEquals(0.0f, 0.382684f, 0.0f, 0.923879f));

			q_pitch.setFromEuler(30.0f, 0.0f, 0.0f, Quaternion::AxisOrder::XYZ);
			REQUIRE_THAT(q_pitch, QuaternionEquals(0.258819f, 0.0f, 0.0f, 0.965926f));

			q_roll.setFromEuler(0.0f, 0.0f, 10.0f, Quaternion::AxisOrder::XYZ);
			REQUIRE_THAT(q_roll, QuaternionEquals(0.0f, 0.0f, 0.0871558f, 0.996195f));

			q_ypr.setFromEuler(30.0f, 45.0f, 10.0f, Quaternion::AxisOrder::YXZ);
			REQUIRE_THAT(q_yaw * q_pitch * q_roll, QuaternionEquals(q_ypr));
		}

		SECTION("vector rotation")
		{
			Quaternion qa{0.3919183f, 0.3196269f, -0.8430416f, -0.1830837f};
			REQUIRE(qa.getMagnitude() == Catch::Approx(1.0f));
			REQUIRE_THAT(qa.unitRotate(Vector3f{0.3535534f, -0.1464466f, 0.3535534f}), VectorEquals(-0.487732f, 0.1646256f, 0.08039001f));

			Quaternion qb(0.336838, 0.0115086, 0.421048, 0.842096);
			REQUIRE(qb.getMagnitude() == Catch::Approx(1.0f));
			REQUIRE_THAT(qb.unitRotate(Vector3f{0.53f, -0.0532f, 0.22f}), VectorEquals(0.4459215f, 0.2350067f, 0.2793851f));

			Quaternion qc(0, 0.7071068, 0, 0.7071068);
			REQUIRE(qc.getMagnitude() == Catch::Approx(1.0f));
			REQUIRE_THAT(qc.unitRotate(Vector3f{13.0f, -1.23f, 3.4f}), VectorEquals(3.4f, -1.23f, -13.0f));
		}

		SECTION("angles between vectors")
		{
			Quaternion q;

			q.setFromVectors(Vector3f::calcNormalize(Vector3f{0.8f, 1.4f, 2.6f}), Vector3f::calcNormalize(Vector3f{1.2f, 0.3f, -2.9f}));
			REQUIRE_THAT(q, QuaternionEquals(-0.590233f, 0.6634023f, -0.1756065f, 0.4250704f));

			q.setFromVectors(Vector3f{0.0f, 1.0f, 0.0f}, Vector3f{0.0f, 0.0f, 1.0f});
			REQUIRE_THAT(q, QuaternionEquals(0.7071068f, 0.0f, 0.0f, 0.7071068f));

			q.setFromVectors(Vector3f::calcNormalize(Vector3f{-1.0f, 0.4f, 2.9f}), Vector3f::calcNormalize(Vector3f{0.8f, 1.4f, 2.6f}));
			REQUIRE_THAT(q, QuaternionEquals(-0.1695316f, 0.2761906f, -0.09655445f, 0.9410924f));

		    q.setFromVectors(Vector3f::calcNormalize(Vector3f{0.8f, -1.4f, 2.6f}), Vector3f::calcNormalize(Vector3f{0.8f, -1.4f, 2.6f}));
		    REQUIRE_THAT(q, QuaternionEquals(0.0f, 0.0f, 0.0f, 1.0f));

			//opposite vectors
		    auto v1 = Vector3f{-0.8f, 1.4f, -2.6f};
		    auto v2 = Vector3f{0.8f, -1.4f, 2.6f};
		    q.setFromVectors(v1, v2);
		    REQUIRE_THAT(q.unitRotate(v1), VectorEquals(v2));
		}

		SECTION("dot")
		{
			Quaternion qa{-27.0f, 83.0f, 32.0f, -153.0f};
			Quaternion qb{36.0f, -64.0f, 12.0f, 24.0f};

			REQUIRE(qa.getDot(qb) == Catch::Approx(-9572.0f));
			REQUIRE(qb.getDot(qa) == Catch::Approx(-9572.0f));
			REQUIRE(qa.getMagnitudeSquared() == Catch::Approx(qa.getDot(qa)));
			
			qa = Quaternion{0.3535534f, -0.1464466f, 0.3535534f, 0.8535535f};
			qb = Quaternion{0.3919183f, 0.3196269f, -0.8430416f, -0.1830837f};
			REQUIRE(qa.getDot(qb) == Catch::Approx(-0.362576186656952));

			qa = Quaternion{0.0f, 0.7071068f, 0.0f, 0.7071068f};
			qb = Quaternion{0.0f, 0.7071068f, 0.0f, 0.7071068f};
			REQUIRE(qa.getDot(qb) == Catch::Approx(1.0f));
		}

		SECTION("rotation")
		{
			//identity
			Quaternion q;
			REQUIRE_THAT(q.unitRotate(Vector3f{1.0f, 1.0f, 1.0f}), VectorEquals(1.0f, 1.0f, 1.0f));

			//along the X axis
			q = Quaternion::genAxisAngle(Vector3f{1.0f, 0.0f, 0.0f}, 90.0f);
			REQUIRE_THAT(q.unitRotate(Vector3f{0.0f, 1.0f, 0.0f}), VectorEquals(0.0f, 0.0f, 1.0f));

			q = Quaternion::genAxisAngle(Vector3f{1.0f, 0.0f, 0.0f}, 180.0f);
			REQUIRE_THAT(q.unitRotate(Vector3f{0.0f, 1.0f, 0.0f}), VectorEquals(0.0f, -1.0f, 0.0f));

			q = Quaternion::genAxisAngle(Vector3f{1.0f, 0.0f, 0.0f}, 270.0f);
			REQUIRE_THAT(q.unitRotate(Vector3f{0.0f, 1.0f, 0.0f}), VectorEquals(0.0f, 0.0f, -1.0f));

			//along the Y axis
			q = Quaternion::genAxisAngle(Vector3f{0.0f, 1.0f, 0.0f}, 90.0f);
			REQUIRE_THAT(q.unitRotate(Vector3f{1.0f, 0.0f, 0.0f}), VectorEquals(0.0f, 0.0f, -1.0f));

			q = Quaternion::genAxisAngle(Vector3f{0.0f, 1.0f, 0.0f}, 180.0f);
			REQUIRE_THAT(q.unitRotate(Vector3f{1.0f, 0.0f, 0.0f}), VectorEquals(-1.0f, 0.0f, 0.0f));

			q = Quaternion::genAxisAngle(Vector3f{0.0f, 1.0f, 0.0f}, 270.0f);
			REQUIRE_THAT(q.unitRotate(Vector3f{1.0f, 0.0f, 0.0f}), VectorEquals(0.0f, 0.0f, 1.0f));

			//along the Z axis
			q = Quaternion::genAxisAngle(Vector3f{0.0f, 0.0f, 1.0f}, 90.0f);
			REQUIRE_THAT(q.unitRotate(Vector3f{0.0f, 1.0f, 0.0f}), VectorEquals(-1.0f, 0.0f, 0.0f));

			q = Quaternion::genAxisAngle(Vector3f{0.0f, 0.0f, 1.0f}, 180.0f);
			REQUIRE_THAT(q.unitRotate(Vector3f{0.0f, 1.0f, 0.0f}), VectorEquals(0.0f, -1.0f, 0.0f));

			q = Quaternion::genAxisAngle(Vector3f{0.0f, 0.0f, 1.0f}, 270.0f);
			REQUIRE_THAT(q.unitRotate(Vector3f{0.0f, 1.0f, 0.0f}), VectorEquals(1.0f, 0.0f, 0.0f));
		}

		SECTION("slerp")
		{
			{
				Quaternion qa(-0.270598f, 0.270598f, 0.6532815f, 0.6532815f);
				Quaternion qb(0.5f, 0.5f, 0.5f, 0.5f);
				Quaternion q;

				q.setSLerp(qa, qb, -0.1f);
				REQUIRE_THAT(q, QuaternionEquals(qa));
				q.setSLerp(qa, qb, 0.0f);
				REQUIRE_THAT(q, QuaternionEquals(qa));
				q.setSLerp(qa, qb, 1.0f);
				REQUIRE_THAT(q, QuaternionEquals(qb));
				q.setSLerp(qa, qb, 1.1f);
				REQUIRE_THAT(q, QuaternionEquals(qb));

				q.setSLerp(qa, qb, 0.62f);
				REQUIRE_THAT(q, QuaternionEquals(0.22069444274723088f, 0.4498729015909088f, 0.6119266025696755f, 0.6119266025696755f));
			}

			{
				auto qa = Quaternion::genAxisAngle(Vector3f{1.0f, 0.0f, 0.0f}, 0.0f);
				auto qb = Quaternion::genAxisAngle(Vector3f{1.0f, 0.0f, 0.0f}, 90.0f);
				Quaternion q;

				REQUIRE(qa.getMagnitude() == Catch::Approx(1.0f));
				REQUIRE(qb.getMagnitude() == Catch::Approx(1.0f));

				q.setSLerp(qa, qb, -0.1f);
				REQUIRE_THAT(q, QuaternionEquals(qa));
				q.setSLerp(qa, qb, 0.0f);
				REQUIRE_THAT(q, QuaternionEquals(qa));
				q.setSLerp(qa, qb, 1.0f);
				REQUIRE_THAT(q, QuaternionEquals(qb));
				q.setSLerp(qa, qb, 1.1f);
				REQUIRE_THAT(q, QuaternionEquals(qb));

				q.setSLerp(qa, qb, 0.5f);
				REQUIRE_THAT(q, QuaternionEquals(Quaternion::genAxisAngle(Vector3f{1.0f, 0.0f, 0.0f}, 45.0f)));
			}
		}

		SECTION("nlerp")
		{
			Quaternion qa(-1.0f, 2.0f, -3.0f, 4.0f);
			Quaternion qb(1.0f, 3.0f, 0.0f, 4.8f);
			Quaternion q;

			q.setNLerp(qa, qb, -0.1f);
			REQUIRE_THAT(q, QuaternionEquals(qa));
			q.setNLerp(qa, qb, 0.0f);
			REQUIRE_THAT(q, QuaternionEquals(qa));
			q.setNLerp(qa, qb, 1.0f);
			REQUIRE_THAT(q, QuaternionEquals(qb));
			q.setNLerp(qa, qb, 1.1f);
			REQUIRE_THAT(q, QuaternionEquals(qb));

			q.setNLerp(qa, qb, 0.5f);
			
			Quaternion res(0.0f, 2.5f, -1.5f, 4.4f);
			res.normalize();
			REQUIRE_THAT(q, QuaternionEquals(res));
		}

		SECTION("to angle axis")
		{
			{
				Quaternion q(0.6514133f, -0.1282655f, 0.6116868f, 0.430172f);

				float angle;
				Vector3f axis;
				q.getAxisAngle(axis, angle);

				REQUIRE(angle == Catch::Approx(hr::Math::Rad2Deg<float> * 2.25223f));
				REQUIRE_THAT(axis, VectorEquals(0.7215902f, -0.1420836f, 0.6775839f));
			}

			{
				Quaternion q(0.0f, 0.0f, 0.0f, 1.0f);

				float angle;
				Vector3f axis;
				q.getAxisAngle(axis, angle);

				REQUIRE(angle == Catch::Approx(hr::Math::Rad2Deg<float> * 0.0f));
				REQUIRE_THAT(axis, VectorEquals(1.0f, 0.0f, 0.0f));
			}

			{
				Quaternion q(0.1164578f, 0.4874545f, 0.8652994f, 0.009090029f);

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
				Quaternion q(0.6514133, -0.1282655, 0.6116868, 0.430172);

				float angles[3];
				q.getEulerAngles(angles[0], angles[1], angles[2]);

				REQUIRE(angles[0] == Catch::Approx(0.8f));
				REQUIRE(angles[1] == Catch::Approx(1.4f));
				REQUIRE(angles[2] == Catch::Approx(2.6f));
			}

			{
				Quaternion q(0.0f, 0.0f, 1.2f, 0.0f);

				float angles[3];
				q.getEulerAngles(angles[0], angles[1], angles[2]);

				REQUIRE(angles[0] == Catch::Approx(0.0f));
				REQUIRE(angles[1] == Catch::Approx(0.0f));
				REQUIRE(angles[2] == Catch::Approx(3.14159265f));
			}

			{
				Quaternion q(0.1164578f, 0.4874545f, 0.8652994f, 0.009090029f);

				float angles[3];
				q.getEulerAngles(angles[0], angles[1], angles[2]);

				REQUIRE(angles[0] == Catch::Approx(-1.0f));
				REQUIRE(angles[1] == Catch::Approx(0.4f));
				REQUIRE(angles[2] == Catch::Approx(2.9f));
			}
		}
	}
}
