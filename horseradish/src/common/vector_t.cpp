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

				return (v[0] == Catch::Approx(m_x)) && (v[1] == Catch::Approx(m_y)) && (v[2] == Catch::Approx(m_z));
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

				return (v[0] == Catch::Approx(m_x)) && (v[1] == Catch::Approx(m_y)) && (v[2] == Catch::Approx(m_z)) && (v[3] == Catch::Approx(m_w));
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

		template<typename TType>
		auto VEquals(TType x, TType y, TType z, TType w) -> V4EqualsMatcher<TType>
		{
			return V4EqualsMatcher<TType>{x, y, z, w};
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
			REQUIRE_THAT(v3, VEquals(v1[0], v1[1], v1[2]));

			v1 = v2[0];
			REQUIRE_THAT(v1, VEquals(v2[0], v2[0], v2[0]));

			v3.set(v2);
			REQUIRE_THAT(v3, VEquals(v2[0], v2[1], v2[2]));

			v1 = randomVector3f();
			v3.set(v1.data());
			REQUIRE_THAT(v3, VEquals(v1[0], v1[1], v1[2]));

			v1.set(9.3904f);
			REQUIRE_THAT(v1, VEquals(9.3904f, 9.3904f, 9.3904f));
		}

		SECTION("basic operations")
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

		SECTION("operations")
		{
		}

		SECTION("other operations")
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

			auto v3 = v2.convert<double>();
			REQUIRE_THAT(v3, VEquals(static_cast<double>(v2[0]), static_cast<double>(v2[1]), static_cast<double>(v2[2])));

			float buffer[3];
			v1.write(buffer);
			REQUIRE_THAT(v1, VEquals(buffer[0], buffer[1], buffer[2]));
		}
	}
}
