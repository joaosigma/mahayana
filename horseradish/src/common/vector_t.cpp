#pragma once

#include "vector.hpp"
#include "random.hpp"

#include <format>
#include <optional>

#include "libs/catch2/catch.hpp"

namespace Catch
{
	template<>
	struct StringMaker<hr::Vector3f>
	{
		static std::string convert(hr::Vector3f const& v)
		{
			return std::format("Vector is: {{{}, {}, {}}}", v[0], v[1], v[2]);
		}
	};

	template<>
	struct StringMaker<hr::Vector3d>
	{
		static std::string convert(hr::Vector3d const& v)
		{
			return std::format("Vector is: {{{}, {}, {}}}", v[0], v[1], v[2]);
		}
	};

	template<>
	struct StringMaker<hr::Vector4f>
	{
		static std::string convert(hr::Vector4f const& v)
		{
			return std::format("Vector is: {{{}, {}, {}, {}}}", v[0], v[1], v[2], v[3]);
		}
	};
}

namespace hr::utests
{
	namespace
	{
		template<typename TType>
		struct V3EqualsMatcher : Catch::Matchers::MatcherGenericBase
		{
			static_assert(std::is_same_v<float, TType> || std::is_same_v<double, TType>);

			V3EqualsMatcher(TType x, TType y, TType z)
			  : m_x{x}, m_y{y}, m_z{z}
			{ }

			template<typename TVector>
			bool match(const TVector& v) const
			{
				static_assert(std::is_same_v<TVector, Vector3f> || std::is_same_v<TVector, Vector3d>);
				static_assert(TVector::NumComponents == 3);

				// set epsilon to allowed a 0.1% difference

				return (v[0] == Catch::Approx(m_x).epsilon(0.001)) 
					&& (v[1] == Catch::Approx(m_y).epsilon(0.001))
					&& (v[2] == Catch::Approx(m_z).epsilon(0.001));
			}

			std::string describe() const override
			{
				return std::format("Vector is: {{{}, {}, {}}}", m_x, m_y, m_z);
			}

		private:
			TType m_x, m_y, m_z;
		};

		template<typename TType>
		struct V4EqualsMatcher : Catch::Matchers::MatcherGenericBase
		{
			static_assert(std::is_same_v<float, TType> || std::is_same_v<double, TType>);

			V4EqualsMatcher(TType x, TType y, TType z, TType w)
			  : m_x{x}, m_y{y}, m_z{z}, m_w{w}
			{ }

			template<typename TVector>
			bool match(const TVector& v) const
			{
				static_assert(std::is_same_v<TVector, Vector4f>);
				static_assert(TVector::NumComponents == 4);

				// set epsilon to allowed a 0.1% difference

				return (v[0] == Catch::Approx(m_x).epsilon(0.001))
					&& (v[1] == Catch::Approx(m_y).epsilon(0.001))
					&& (v[2] == Catch::Approx(m_z).epsilon(0.001))
					&& (v[3] == Catch::Approx(m_w).epsilon(0.001));
			}

			std::string describe() const override
			{
				return std::format("Components are: {{{}, {}, {}, {}}}", m_x, m_y, m_z, m_w);
			}

		private:
			TType m_x, m_y, m_z, m_w;
		};

		template<typename TType>
		auto VEquals(TType x, TType y, TType z) -> V3EqualsMatcher<TType>
		{
			return V3EqualsMatcher<TType>{x, y, z};
		}

		auto VEquals(const Vector3f& vec) -> V3EqualsMatcher<float>
		{
			return V3EqualsMatcher<float>{vec[0], vec[1], vec[2]};
		}

		auto VEquals(const Vector3d& vec) -> V3EqualsMatcher<double>
		{
			return V3EqualsMatcher<double>{vec[0], vec[1], vec[2]};
		}

		template<typename TType>
		auto VEquals(TType x, TType y, TType z, TType w) -> V4EqualsMatcher<TType>
		{
			return V4EqualsMatcher<TType>{x, y, z, w};
		}

		auto VEquals(const Vector4f& vec) -> V4EqualsMatcher<float>
		{
			return V4EqualsMatcher<float>{vec[0], vec[1], vec[2], vec[3]};
		}

		Vector3f randomVector3f()
		{
			Random r;
			return Vector3f(r.nextDouble(-10.0f, 10.0f), r.nextDouble(-10.0f, 10.0f), r.nextDouble(-10.0f, 10.0f));
		}
	}

	TEST_CASE("3 component vector of type float", "[common][vector][vector3f]")
	{
		SECTION("init")
		{
			float values[3]{-0.21f, 55.11f, -0.0001f};

			REQUIRE_THAT(Vector3f(), VEquals(0.0f, 0.0f, 0.0f));
			REQUIRE_THAT(Vector3f(1.2f), VEquals(1.2f, 1.2f, 1.2f));
			REQUIRE_THAT(Vector3f(1.2f, -2.5f, 4.123f), VEquals(1.2f, -2.5f, 4.123f));
			REQUIRE_THAT(Vector3f(values), VEquals(-0.21f, 55.11f, -0.0001f));
		}

		SECTION("access")
		{
			Vector3f v{0.911f, 2.610f, -7.841f};

			REQUIRE(v[0] == Catch::Approx(0.911f));
			REQUIRE(v[1] == Catch::Approx(2.610f));
			REQUIRE(v[2] == Catch::Approx(-7.841f));
			REQUIRE(v[3] == Catch::Approx(0.911f));
			REQUIRE(v[4] == Catch::Approx(2.610f));
			REQUIRE(v[5] == Catch::Approx(-7.841f));
			REQUIRE(v[12] == Catch::Approx(0.911f));
			REQUIRE(v[13] == Catch::Approx(2.610f));
			REQUIRE(v[14] == Catch::Approx(-7.841f));

			REQUIRE(v.data()[0] == Catch::Approx(0.911f));
			REQUIRE(v.data()[1] == Catch::Approx(2.610f));
			REQUIRE(v.data()[2] == Catch::Approx(-7.841f));
		}

		SECTION("assignment")
		{
			auto v1 = randomVector3f();
			auto v2 = randomVector3f();

			Vector3f v3{};
			REQUIRE_THAT(v3, VEquals(0.0f, 0.0f, 0.0f));

			v3 = v1;
			REQUIRE_THAT(v3, VEquals(v1));

			v1 = v2[0];
			REQUIRE_THAT(v1, VEquals(v2[0], v2[0], v2[0]));

			v3.set(v2);
			REQUIRE_THAT(v3, VEquals(v2));

			v1 = randomVector3f();
			v3.set(v1.data());
			REQUIRE_THAT(v3, VEquals(v1));

			v1.set(9.3904f);
			REQUIRE_THAT(v1, VEquals(9.3904f, 9.3904f, 9.3904f));
		}

		SECTION("basic operands")
		{
			auto v1 = randomVector3f();
			auto v2 = randomVector3f();

			auto v3 = v2;
			REQUIRE_THAT(v3, VEquals(v2[0], v2[1], v2[2]));

			v3 = -v1;
			REQUIRE_THAT(v3, VEquals(-v1[0], -v1[1], -v1[2]));

			v3 = v2;
			v3 += v1;
			REQUIRE_THAT(v3, VEquals(v2[0] + v1[0], v2[1] + v1[1], v2[2] + v1[2]));

			v3 = v2;
			v3 += v1.data();
			REQUIRE_THAT(v3, VEquals(v2[0] + v1[0], v2[1] + v1[1], v2[2] + v1[2]));

			v3 = v2;
			v3 += 3.33f;
			REQUIRE_THAT(v3, VEquals(v2[0] + 3.33f, v2[1] + 3.33f, v2[2] + 3.33f));

			v3 = v2;
			v3 -= v1;
			REQUIRE_THAT(v3, VEquals(v2[0] - v1[0], v2[1] - v1[1], v2[2] - v1[2]));

			v3 = v2;
			v3 -= v1.data();
			REQUIRE_THAT(v3, VEquals(v2[0] - v1[0], v2[1] - v1[1], v2[2] - v1[2]));

			v3 = v2;
			v3 -= 0.49451f;
			REQUIRE_THAT(v3, VEquals(v2[0] - 0.49451f, v2[1] - 0.49451f, v2[2] - 0.49451f));

			v3 = v2;
			v3 *= v1;
			REQUIRE_THAT(v3, VEquals(v2[0] * v1[0], v2[1] * v1[1], v2[2] * v1[2]));

			v3 = v2;
			v3 *= v1.data();
			REQUIRE_THAT(v3, VEquals(v2[0] * v1[0], v2[1] * v1[1], v2[2] * v1[2]));

			v3 = v2;
			v3 *= -1.4599f;
			REQUIRE_THAT(v3, VEquals(v2[0] * (-1.4599f), v2[1] * (-1.4599f), v2[2] * (-1.4599f)));

			v3 = v2;
			v3 /= v1;
			REQUIRE_THAT(v3, VEquals(v2[0] / v1[0], v2[1] / v1[1], v2[2] / v1[2]));

			v3 = v2;
			v3 /= v1.data();
			REQUIRE_THAT(v3, VEquals(v2[0] / v1[0], v2[1] / v1[1], v2[2] / v1[2]));

			v3 = v2;
			v3 /= 0.8833f;
			REQUIRE_THAT(v3, VEquals(v2[0] / 0.8833f, v2[1] / 0.8833f, v2[2] / 0.8833f));

			REQUIRE_THAT(v2 + v1, VEquals(v2[0] + v1[0], v2[1] + v1[1], v2[2] + v1[2]));
			REQUIRE_THAT(v2 - v1, VEquals(v2[0] - v1[0], v2[1] - v1[1], v2[2] - v1[2]));
			REQUIRE_THAT(v2 * v1, VEquals(v2[0] * v1[0], v2[1] * v1[1], v2[2] * v1[2]));
			REQUIRE_THAT(v2 / v1, VEquals(v2[0] / v1[0], v2[1] / v1[1], v2[2] / v1[2]));

			REQUIRE_THAT(v2 + v1.data(), VEquals(v2[0] + v1[0], v2[1] + v1[1], v2[2] + v1[2]));
			REQUIRE_THAT(v2 - v1.data(), VEquals(v2[0] - v1[0], v2[1] - v1[1], v2[2] - v1[2]));
			REQUIRE_THAT(v2 * v1.data(), VEquals(v2[0] * v1[0], v2[1] * v1[1], v2[2] * v1[2]));
			REQUIRE_THAT(v2 / v1.data(), VEquals(v2[0] / v1[0], v2[1] / v1[1], v2[2] / v1[2]));

			REQUIRE_THAT((v2 + v1) / 1.44f, VEquals((v2[0] + v1[0]) / 1.44f, (v2[1] + v1[1]) / 1.44f, (v2[2] + v1[2]) / 1.44f));
			REQUIRE_THAT((v2 - v1) * 44.1f, VEquals((v2[0] - v1[0]) * 44.1f, (v2[1] - v1[1]) * 44.1f, (v2[2] - v1[2]) * 44.1f));
			REQUIRE_THAT((v2 * v1) - 1.44f, VEquals((v2[0] * v1[0]) - 1.44f, (v2[1] * v1[1]) - 1.44f, (v2[2] * v1[2]) - 1.44f));
			REQUIRE_THAT((v2 / v1) + 44.1f, VEquals((v2[0] / v1[0]) + 44.1f, (v2[1] / v1[1]) + 44.1f, (v2[2] / v1[2]) + 44.1f));
		}

		SECTION("comparison")
		{
			auto v1 = randomVector3f();
			auto v2 = randomVector3f();

			REQUIRE(v1 != v2);
			REQUIRE(v1 == v1);
			REQUIRE(v2 == v2);

			REQUIRE(Vector3f{}.isZero(0.0000000001f));
			REQUIRE((Vector3f{} + 0.000001f).isZero(0.0001f));
			REQUIRE_FALSE(v2.isZero(0.0000000001f));
			REQUIRE_FALSE(v2.isZero(0.1f));

			REQUIRE(v2.isEqual(v2, 0.0000000001f));
			REQUIRE_FALSE(v2.isEqual(v1, 0.0000000001f));
			REQUIRE(Vector3f{0.001f, 0.002f, 0.003f}.isEqual(Vector3f{0.002f, 0.003f, 0.004f}, 0.01f));
			REQUIRE_FALSE(Vector3f{0.001f, 0.002f, 0.003f}.isEqual(Vector3f{0.002f, 0.003f, 0.004f}, 0.001f));
		}

		SECTION("misc")
		{
			auto v1 = randomVector3f();
			auto v2 = randomVector3f();

			auto v3 = v2.convert<double>();
			REQUIRE_THAT(v3, VEquals(static_cast<double>(v2[0]), static_cast<double>(v2[1]), static_cast<double>(v2[2])));

			float buffer[3];
			v1.write(buffer);
			REQUIRE_THAT(v1, VEquals(buffer[0], buffer[1], buffer[2]));
		}

		SECTION("cross product")
		{
			Vector3f v1{2.0f, -5.0f, 4.0f};
			Vector3f v2{6.0f, 2.0f, -8.0f};
			auto v = Vector3f::calcCrossProduct(v1, v2);
			REQUIRE_THAT(v, VEquals(32.0f, 40.0f, 34.0f));

			v1 = Vector3f{0.24f, 0.0082f, -0.03};
			v2 = Vector3f{0.53f, -0.0532f, -1.53};
			v = Vector3f::calcCrossProduct(v1, v2);
			REQUIRE_THAT(v, VEquals(-0.014142f, 0.3513f, -0.017114f));

			v1 = Vector3f{-27.0f, 83.0f, -163.0f};
			v2 = Vector3f{36.0f, -64.0f, 264.0f};
			v = Vector3f::calcCrossProduct(v1, v2);
			REQUIRE_THAT(v, VEquals(11480.0f, 1260.0f, -1260.0f));

			// Cross product of vector and itself is zero
			v = Vector3f::calcCrossProduct(v1, v1);
			REQUIRE_THAT(v, VEquals(0.0f, 0.0f, 0.0f));
		}

		SECTION("distance")
		{
			Vector3f v1{2.0f, -5.0f, 4.0f};
			Vector3f v2{6.0f, 2.0f, -8.0f};
			REQUIRE(Vector3f::calcDistance(v1, v2) == Catch::Approx(14.4568f));

			v1 = Vector3f{0.24f, 0.0082f, -0.03};
			v2 = Vector3f{0.53f, -0.0532f, -1.53};
			REQUIRE(Vector3f::calcDistance(v1, v2) == Catch::Approx(1.52901f));

			v1 = Vector3f{-27.0f, 83.0f, -163.0f};
			v2 = Vector3f{36.0f, -64.0f, 264.0f};
			REQUIRE(Vector3f::calcDistance(v1, v2) == Catch::Approx(455.968f));
		}

		SECTION("dot product")
		{
			Vector3f v1{2.0f, -5.0f, 4.0f};
			Vector3f v2{6.0f, 2.0f, -8.0f};
			REQUIRE(Vector3f::calcDot(v1, v2) == Catch::Approx(-30.0f));

			v1 = Vector3f{0.24f, 0.0082f, -0.03};
			v2 = Vector3f{0.53f, -0.0532f, -1.53};
			REQUIRE(Vector3f::calcDot(v1, v2) == Catch::Approx(0.172663f));

			v1 = Vector3f{-27.0f, 83.0f, -163.0f};
			v2 = Vector3f{36.0f, -64.0f, 264.0f};
			REQUIRE(Vector3f::calcDot(v1, v2) == Catch::Approx(-49316.0f));
		}

		SECTION("spherical coordinate space")
		{
			double rad = 6.7082f;
			double theta = 0.931931f;
			double phi = -1.19029f;
			auto v = Vector3f::fromSpherical(rad, theta, phi);
			REQUIRE_THAT(v, VEquals(2.0f, -5.0f, 4.0f));

			rad = 0.242007f;
			theta = 1.69508f;
			phi = 0.0341533f;
			v = Vector3f::fromSpherical(rad, theta, phi);
			REQUIRE_THAT(v, VEquals(0.24f, 0.0082f, -0.03f));

			rad = 184.8972f;
			theta = 2.6499755f;
			phi = 1.8853006f;
			v = Vector3f::fromSpherical(rad, theta, phi);
			REQUIRE_THAT(v, VEquals(-27.0f, 83.0f, -163.0f));
		}

		SECTION("interpolate")
		{
			Vector3f v1{2.0f, -5.0f, 4.0f};
			Vector3f v2{6.0f, 2.0f, -8.0f};
			auto v = Vector3f::calcLerp(v1, v2, 0.0f);
			REQUIRE_THAT(v, VEquals(2.0f, -5.0f, 4.0f));

			v1 = Vector3f{0.24f, 0.0082f, -0.03};
			v2 = Vector3f{0.53f, -0.0532f, -1.53};
			v = Vector3f::calcLerp(v1, v2, 0.33f);
			REQUIRE_THAT(v, VEquals(0.3357f, -0.012062f, -0.525f));

			v1 = Vector3f{-27.0f, 83.0f, -163.0f};
			v2 = Vector3f{36.0f, -64.0f, 264.0f};
			v = Vector3f::calcLerp(v1, v2, 1.0f);
			REQUIRE_THAT(v, VEquals(36.0f, -64.0f, 264.0f));
		}

		SECTION("magnitude")
		{
			// set epsilon to allowed a 0.1% difference

			Vector3f v{2.0f, -5.0f, 4.0f};
			REQUIRE(Vector3f::calcMagnitude(v) == Catch::Approx(6.70820393f));
			REQUIRE(Vector3f::calcMagnitudeInverse(v) == Catch::Approx(1.0f / 6.70820393f).epsilon(0.001));

			v = Vector3f{0.24f, 0.0082f, -0.03f};
			REQUIRE(Vector3f::calcMagnitude(v) == Catch::Approx(0.242007f));
			REQUIRE(Vector3f::calcMagnitudeInverse(v) == Catch::Approx(1.0f / 0.242007f).epsilon(0.001));

			v = Vector3f{-27.0f, 83.0f, -163.0f};
			REQUIRE(Vector3f::calcMagnitude(v) == Catch::Approx(184.897269f));
			REQUIRE(Vector3f::calcMagnitudeInverse(v) == Catch::Approx(1.0f / 184.897269f).epsilon(0.001));
		}

		SECTION("max")
		{
			Vector3f v1{2.0f, -5.0f, 4.0f};
			Vector3f v2{6.0f, 2.0f, -8.0f};
			auto v = Vector3f::calcMax(v1, v2);
			REQUIRE_THAT(v, VEquals(6.0f, 2.0f, 4.0f));

			v1 = Vector3f{0.24f, 0.0082f, -0.03};
			v2 = Vector3f{0.53f, -0.0532f, -1.53};
			v = Vector3f::calcMax(v1, v2);
			REQUIRE_THAT(v, VEquals(0.53f, 0.0082f, -0.03f));

			v1 = Vector3f{-27.0f, 83.0f, -163.0f};
			v2 = Vector3f{36.0f, -64.0f, 264.0f};
			v = Vector3f::calcMax(v1, v2);
			REQUIRE_THAT(v, VEquals(36.0f, 83.0f, 264.0f));
		}

		SECTION("min")
		{
			Vector3f v1{2.0f, -5.0f, 4.0f};
			Vector3f v2{6.0f, 2.0f, -8.0f};
			auto v = Vector3f::calcMin(v1, v2);
			REQUIRE_THAT(v, VEquals(2.0f, -5.0f, -8.0f));

			v1 = Vector3f{0.24f, 0.0082f, -0.03};
			v2 = Vector3f{0.53f, -0.0532f, -1.53};
			v = Vector3f::calcMin(v1, v2);
			REQUIRE_THAT(v, VEquals(0.24f, -0.0532f, -1.53f));

			v1 = Vector3f{-27.0f, 83.0f, -163.0f};
			v2 = Vector3f{36.0f, -64.0f, 264.0f};
			v = Vector3f::calcMin(v1, v2);
			REQUIRE_THAT(v, VEquals(-27.0f, -64.0f, -163.0f));
		}

		SECTION("mormalize")
		{
			Vector3f v{2.0f, -5.0f, 4.0f};
			auto n = Vector3f::calcNormalize(v);
			REQUIRE_THAT(n, VEquals(0.298142f, -0.745356f, 0.596285f));

			v = Vector3f{0.24f, 0.0082f, -0.03f};
			n = Vector3f::calcNormalize(v);
			REQUIRE_THAT(n, VEquals(0.991708f, 0.0338834f, -0.123964f));

			v = Vector3f{-27.0f, 83.0f, -163.0f};
			n = Vector3f::calcNormalize(v);
			REQUIRE_THAT(n, VEquals(-0.146027f, 0.448898f, -0.881571f));
		}

		SECTION("project")
		{
			Vector3f v1{2.0f, -5.0f, 4.0f};
			Vector3f v2{6.0f, 2.0f, -8.0f};
			auto v = Vector3f::calcProject(v1, v2);
			REQUIRE_THAT(v, VEquals(-1.7307692308f, -0.5769230769f, 2.3076923077f));

			v1 = Vector3f{0.24f, 0.0082f, -0.03};
			v2 = Vector3f{0.53f, -0.0532f, -1.53};
			v = Vector3f::calcProject(v1, v2);
			REQUIRE_THAT(v, VEquals(0.03487f, -0.0035f, -0.10065f));

			v1 = Vector3f{-27.0f, 83.0f, -163.0f};
			v2 = Vector3f{36.0f, -64.0f, 264.0f};
			v = Vector3f::calcProject(v1, v2);
			REQUIRE_THAT(v, VEquals(-23.6439377797f, 42.0336671639f, -173.3888770509f));
		}

		SECTION("reflect")
		{
			// Case 1
			Vector3f v1{2.0f, -5.0f, 4.0f};
			Vector3f v2{6.0f, 2.0f, -8.0f};
			auto v = Vector3f::calcReflect(v1, v2);
			REQUIRE_THAT(v, VEquals(5.4615384615f, -3.8461538462f, -0.6153846154f));

			v1 = Vector3f{0.24f, 0.0082f, -0.03};
			v2 = Vector3f{0.53f, -0.0532f, -1.53};
			v = Vector3f::calcReflect(v1, v2);
			REQUIRE_THAT(v, VEquals(0.1702669066f, 0.0151996237f, 0.1713049677f));

			v1 = Vector3f{-27, 83, -163};
			v2 = Vector3f{36, -64, 264};
			v = Vector3f::calcReflect(v1, v2);
			REQUIRE_THAT(v, VEquals(20.2878755593f, -1.0673343277f, 183.7777541019f));

			v1 = Vector3f{1.0f, 2.0f, 3.0f};
			v2 = Vector3f{0.0f, 1.0f, 0.0f};
			v = Vector3f::calcReflect(v1, v2);
			REQUIRE_THAT(v, VEquals(1.0f, -2.0f, 3.0f));
		}
	}
}
