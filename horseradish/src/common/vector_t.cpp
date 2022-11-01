#pragma once

#include "utils_t.hpp"

#include "vector.hpp"
#include "random.hpp"

#include "libs/catch2/catch.hpp"

namespace hr::utests
{
	namespace
	{
		float randomScalarf()
		{
			return static_cast<float>(Random{}.nextDouble(-10.0, 10.0));
		}

		double randomScalard()
		{
			return Random{}.nextDouble(-10.0, 10.0);
		}

		Vector3f randomVector3f()
		{
			Random r;
			return Vector3f(static_cast<float>(r.nextDouble(-10.0, 10.0)), static_cast<float>(r.nextDouble(-10.0, 10.0)), static_cast<float>(r.nextDouble(-10.0, 10.0)));
		}

		Vector3d randomVector3d()
		{
			Random r;
			return Vector3d(r.nextDouble(-10.0, 10.0), r.nextDouble(-10.0, 10.0), r.nextDouble(-10.0, 10.0));
		}

		Vector4f randomVector4f()
		{
			Random r;
			return Vector4f(static_cast<float>(r.nextDouble(-10.0, 10.0)), static_cast<float>(r.nextDouble(-10.0, 10.0)),
			  static_cast<float>(r.nextDouble(-10.0, 10.0)), static_cast<float>(r.nextDouble(-10.0, 10.0)));
		}

		Vector4d randomVector4d()
		{
			Random r;
			return Vector4d(r.nextDouble(-10.0, 10.0), r.nextDouble(-10.0, 10.0), r.nextDouble(-10.0, 10.0), r.nextDouble(-10.0, 10.0));
		}
	}

	TEST_CASE("3 component vector of type float", "[common][vector][vector3f]")
	{
		SECTION("init")
		{
			float values[3]{-0.21f, 55.11f, -0.0001f};

			REQUIRE_THAT(Vector3f(), VectorEquals(0.0f, 0.0f, 0.0f));
			REQUIRE_THAT(Vector3f(1.2f), VectorEquals(1.2f, 1.2f, 1.2f));
			REQUIRE_THAT(Vector3f(1.2f, -2.5f, 4.123f), VectorEquals(1.2f, -2.5f, 4.123f));
			REQUIRE_THAT(Vector3f(values), VectorEquals(-0.21f, 55.11f, -0.0001f));
			REQUIRE_THAT(Vector3f::zero(), VectorEquals(0.0f, 0.0f, 0.0f));
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

		SECTION("basic operands")
		{
			auto v1 = randomVector3f();
			auto v2 = randomVector3f();

			auto v3 = v2;
			REQUIRE_THAT(v3, VectorEquals(v2[0], v2[1], v2[2]));

			v3 = -v1;
			REQUIRE_THAT(v3, VectorEquals(-v1[0], -v1[1], -v1[2]));

			v3 = v2;
			v3 += v1;
			REQUIRE_THAT(v3, VectorEquals(v2[0] + v1[0], v2[1] + v1[1], v2[2] + v1[2]));

			v3 = v2;
			v3 += 3.33f;
			REQUIRE_THAT(v3, VectorEquals(v2[0] + 3.33f, v2[1] + 3.33f, v2[2] + 3.33f));

			v3 = v2;
			v3 -= v1;
			REQUIRE_THAT(v3, VectorEquals(v2[0] - v1[0], v2[1] - v1[1], v2[2] - v1[2]));

			v3 = v2;
			v3 -= 0.49451f;
			REQUIRE_THAT(v3, VectorEquals(v2[0] - 0.49451f, v2[1] - 0.49451f, v2[2] - 0.49451f));

			v3 = v2;
			v3 *= v1;
			REQUIRE_THAT(v3, VectorEquals(v2[0] * v1[0], v2[1] * v1[1], v2[2] * v1[2]));

			v3 = v2;
			v3 *= -1.4599f;
			REQUIRE_THAT(v3, VectorEquals(v2[0] * (-1.4599f), v2[1] * (-1.4599f), v2[2] * (-1.4599f)));

			v3 = v2;
			v3 /= v1;
			REQUIRE_THAT(v3, VectorEquals(v2[0] / v1[0], v2[1] / v1[1], v2[2] / v1[2]));

			v3 = v2;
			v3 /= 0.8833f;
			REQUIRE_THAT(v3, VectorEquals(v2[0] / 0.8833f, v2[1] / 0.8833f, v2[2] / 0.8833f));

			REQUIRE_THAT(v2 + v1, VectorEquals(v2[0] + v1[0], v2[1] + v1[1], v2[2] + v1[2]));
			REQUIRE_THAT(v2 - v1, VectorEquals(v2[0] - v1[0], v2[1] - v1[1], v2[2] - v1[2]));
			REQUIRE_THAT(v2 * v1, VectorEquals(v2[0] * v1[0], v2[1] * v1[1], v2[2] * v1[2]));
			REQUIRE_THAT(v2 / v1, VectorEquals(v2[0] / v1[0], v2[1] / v1[1], v2[2] / v1[2]));

			REQUIRE_THAT((v2 + v1) / 1.44f, VectorEquals((v2[0] + v1[0]) / 1.44f, (v2[1] + v1[1]) / 1.44f, (v2[2] + v1[2]) / 1.44f));
			REQUIRE_THAT((v2 - v1) * 44.1f, VectorEquals((v2[0] - v1[0]) * 44.1f, (v2[1] - v1[1]) * 44.1f, (v2[2] - v1[2]) * 44.1f));
			REQUIRE_THAT((v2 * v1) - 1.44f, VectorEquals((v2[0] * v1[0]) - 1.44f, (v2[1] * v1[1]) - 1.44f, (v2[2] * v1[2]) - 1.44f));
			REQUIRE_THAT((v2 / v1) + 44.1f, VectorEquals((v2[0] / v1[0]) + 44.1f, (v2[1] / v1[1]) + 44.1f, (v2[2] / v1[2]) + 44.1f));
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

		SECTION("convert")
		{
			auto v1 = randomVector3f();

			{
				auto conv = v1.convert<float, 3>();
				REQUIRE_THAT(conv, VectorEquals(v1[0], v1[1], v1[2]));
			}

			{
				auto conv = v1.convert<double, 3>();
				REQUIRE_THAT(conv, VectorEquals(static_cast<double>(v1[0]), static_cast<double>(v1[1]), static_cast<double>(v1[2])));
			}

			{
				auto conv = v1.convert<float, 4>(1.34f);
				REQUIRE_THAT(conv, VectorEquals(v1[0], v1[1], v1[2], 1.34f));
			}

			{
				auto conv = v1.convert<double, 4>(-2.79);
				REQUIRE_THAT(conv, VectorEquals(static_cast<double>(v1[0]), static_cast<double>(v1[1]), static_cast<double>(v1[2]), -2.79));
			}
		}

		SECTION("misc")
		{
			auto v1 = randomVector3f();

			float buffer[3];
			v1.write(buffer);
			REQUIRE_THAT(v1, VectorEquals(buffer[0], buffer[1], buffer[2]));
		}

		SECTION("cross product")
		{
			Vector3f v1{2.0f, -5.0f, 4.0f};
			Vector3f v2{6.0f, 2.0f, -8.0f};
			REQUIRE_THAT(Vector3f::calcCrossProduct(v1, v2), VectorEquals(32.0f, 40.0f, 34.0f));
			REQUIRE_THAT(v1.crossProduct(v2), VectorEquals(32.0f, 40.0f, 34.0f));

			v1 = Vector3f{0.24f, 0.0082f, -0.03f};
			v2 = Vector3f{0.53f, -0.0532f, -1.53f};
			REQUIRE_THAT(Vector3f::calcCrossProduct(v1, v2), VectorEquals(-0.014142f, 0.3513f, -0.017114f));
			REQUIRE_THAT(v1.crossProduct(v2), VectorEquals(-0.014142f, 0.3513f, -0.017114f));

			v1 = Vector3f{-27.0f, 83.0f, -163.0f};
			v2 = Vector3f{36.0f, -64.0f, 264.0f};
			REQUIRE_THAT(Vector3f::calcCrossProduct(v1, v2), VectorEquals(11480.0f, 1260.0f, -1260.0f));
			REQUIRE_THAT(v1.crossProduct(v2), VectorEquals(11480.0f, 1260.0f, -1260.0f));

			// Cross product of vector and itself is zero
			REQUIRE_THAT(Vector3f::calcCrossProduct(v1, v1), VectorEquals(0.0f, 0.0f, 0.0f));
			REQUIRE_THAT(v1.crossProduct(v1), VectorEquals(0.0f, 0.0f, 0.0f));
		}

		SECTION("distance")
		{
			Vector3f v1{2.0f, -5.0f, 4.0f};
			Vector3f v2{6.0f, 2.0f, -8.0f};
			REQUIRE(v1.distance(v2) == Catch::Approx(14.4568f));

			v1 = Vector3f{0.24f, 0.0082f, -0.03f};
			v2 = Vector3f{0.53f, -0.0532f, -1.53f};
			REQUIRE(v1.distance(v2) == Catch::Approx(1.52901f));

			v1 = Vector3f{-27.0f, 83.0f, -163.0f};
			v2 = Vector3f{36.0f, -64.0f, 264.0f};
			REQUIRE(v1.distance(v2) == Catch::Approx(455.968f));
		}

		SECTION("dot product")
		{
			Vector3f v1{2.0f, -5.0f, 4.0f};
			Vector3f v2{6.0f, 2.0f, -8.0f};
			REQUIRE(v1.dot() == Catch::Approx(45.0f));
			REQUIRE(v1.dot(v2) == Catch::Approx(-30.0f));

			v1 = Vector3f{0.24f, 0.0082f, -0.03f};
			v2 = Vector3f{0.53f, -0.0532f, -1.53f};
			REQUIRE(v1.dot() == Catch::Approx(0.05856724f));
			REQUIRE(v1.dot(v2) == Catch::Approx(0.172663f));

			v1 = Vector3f{-27.0f, 83.0f, -163.0f};
			v2 = Vector3f{36.0f, -64.0f, 264.0f};
			REQUIRE(v1.dot() == Catch::Approx(34187.0f));
			REQUIRE(v1.dot(v2) == Catch::Approx(-49316.0f));
		}

		SECTION("spherical coordinate space")
		{
			float rad = 6.7082f;
			float theta = 0.931931f;
			float phi = -1.19029f;
			auto v = Vector3f::fromSpherical(rad, theta, phi);
			REQUIRE_THAT(v, VectorEquals(2.0f, -5.0f, 4.0f));

			rad = 0.242007f;
			theta = 1.69508f;
			phi = 0.0341533f;
			v = Vector3f::fromSpherical(rad, theta, phi);
			REQUIRE_THAT(v, VectorEquals(0.24f, 0.0082f, -0.03f));

			rad = 184.8972f;
			theta = 2.6499755f;
			phi = 1.8853006f;
			v = Vector3f::fromSpherical(rad, theta, phi);
			REQUIRE_THAT(v, VectorEquals(-27.0f, 83.0f, -163.0f));
		}

		SECTION("interpolate")
		{
			Vector3f v1{2.0f, -5.0f, 4.0f};
			Vector3f v2{6.0f, 2.0f, -8.0f};
			auto v = Vector3f::calcLerp(v1, v2, 0.0f);
			REQUIRE_THAT(v, VectorEquals(2.0f, -5.0f, 4.0f));

			v1 = Vector3f{0.24f, 0.0082f, -0.03f};
			v2 = Vector3f{0.53f, -0.0532f, -1.53f};
			v = Vector3f::calcLerp(v1, v2, 0.33f);
			REQUIRE_THAT(v, VectorEquals(0.3357f, -0.012062f, -0.525f));

			v1 = Vector3f{-27.0f, 83.0f, -163.0f};
			v2 = Vector3f{36.0f, -64.0f, 264.0f};
			v = Vector3f::calcLerp(v1, v2, 1.0f);
			REQUIRE_THAT(v, VectorEquals(36.0f, -64.0f, 264.0f));
		}

		SECTION("magnitude")
		{
			// set epsilon to allowed a 0.1% difference

			Vector3f v{2.0f, -5.0f, 4.0f};
			REQUIRE(v.magnitude() == Catch::Approx(6.70820393f));
			REQUIRE(v.magnitudeInv() == Catch::Approx(1.0f / 6.70820393f).epsilon(0.001));

			v = Vector3f{0.24f, 0.0082f, -0.03f};
			REQUIRE(v.magnitude() == Catch::Approx(0.242007f));
			REQUIRE(v.magnitudeInv() == Catch::Approx(1.0f / 0.242007f).epsilon(0.001));

			v = Vector3f{-27.0f, 83.0f, -163.0f};
			REQUIRE(v.magnitude() == Catch::Approx(184.897269f));
			REQUIRE(v.magnitudeInv() == Catch::Approx(1.0f / 184.897269f).epsilon(0.001));
		}

		SECTION("min")
		{
			Vector3f v1{2.0f, -5.0f, 4.0f};
			Vector3f v2{6.0f, 2.0f, -8.0f};
			auto v = Vector3f::calcMin(v1, v2);
			REQUIRE_THAT(v, VectorEquals(2.0f, -5.0f, -8.0f));

			v1 = Vector3f{0.24f, 0.0082f, -0.03f};
			v2 = Vector3f{0.53f, -0.0532f, -1.53f};
			v = Vector3f::calcMin(v1, v2);
			REQUIRE_THAT(v, VectorEquals(0.24f, -0.0532f, -1.53f));

			v1 = Vector3f{-27.0f, 83.0f, -163.0f};
			v2 = Vector3f{36.0f, -64.0f, 264.0f};
			v = Vector3f::calcMin(v1, v2);
			REQUIRE_THAT(v, VectorEquals(-27.0f, -64.0f, -163.0f));
		}

		SECTION("max")
		{
			Vector3f v1{2.0f, -5.0f, 4.0f};
			Vector3f v2{6.0f, 2.0f, -8.0f};
			auto v = Vector3f::calcMax(v1, v2);
			REQUIRE_THAT(v, VectorEquals(6.0f, 2.0f, 4.0f));

			v1 = Vector3f{0.24f, 0.0082f, -0.03f};
			v2 = Vector3f{0.53f, -0.0532f, -1.53f};
			v = Vector3f::calcMax(v1, v2);
			REQUIRE_THAT(v, VectorEquals(0.53f, 0.0082f, -0.03f));

			v1 = Vector3f{-27.0f, 83.0f, -163.0f};
			v2 = Vector3f{36.0f, -64.0f, 264.0f};
			v = Vector3f::calcMax(v1, v2);
			REQUIRE_THAT(v, VectorEquals(36.0f, 83.0f, 264.0f));
		}

		SECTION("minmax")
		{
			auto v1 = randomVector3f();
			auto v2 = randomVector3f();
			auto v = Vector3f::calcMinMax(v1, v2);
			REQUIRE_THAT(std::get<0>(v), VectorEquals(std::min(v1[0], v2[0]), std::min(v1[1], v2[1]), std::min(v1[2], v2[2])));
			REQUIRE_THAT(std::get<1>(v), VectorEquals(std::max(v1[0], v2[0]), std::max(v1[1], v2[1]), std::max(v1[2], v2[2])));

			v1 = randomVector3f();
			v2 = randomVector3f();
			v = Vector3f::calcMinMax(v1, v2);
			REQUIRE_THAT(std::get<0>(v), VectorEquals(std::min(v1[0], v2[0]), std::min(v1[1], v2[1]), std::min(v1[2], v2[2])));
			REQUIRE_THAT(std::get<1>(v), VectorEquals(std::max(v1[0], v2[0]), std::max(v1[1], v2[1]), std::max(v1[2], v2[2])));
		}

		SECTION("mormalize")
		{
			Vector3f v{2.0f, -5.0f, 4.0f};
			auto n = Vector3f::calcNormalize(v);
			v.normalize();
			REQUIRE_THAT(n, VectorEquals(0.298142f, -0.745356f, 0.596285f));
			REQUIRE_THAT(v, VectorEquals(0.298142f, -0.745356f, 0.596285f));

			v = Vector3f{0.24f, 0.0082f, -0.03f};
			n = Vector3f::calcNormalize(v);
			v.normalize();
			REQUIRE_THAT(n, VectorEquals(0.991708f, 0.0338834f, -0.123964f));
			REQUIRE_THAT(v, VectorEquals(0.991708f, 0.0338834f, -0.123964f));

			v = Vector3f{-27.0f, 83.0f, -163.0f};
			n = Vector3f::calcNormalize(v);
			v.normalize();
			REQUIRE_THAT(n, VectorEquals(-0.146027f, 0.448898f, -0.881571f));
			REQUIRE_THAT(v, VectorEquals(-0.146027f, 0.448898f, -0.881571f));
		}

		SECTION("project")
		{
			Vector3f v1{2.0f, -5.0f, 4.0f};
			Vector3f v2{6.0f, 2.0f, -8.0f};
			REQUIRE_THAT(Vector3f::calcProject(v1, v2), VectorEquals(-1.7307692308f, -0.5769230769f, 2.3076923077f));
			REQUIRE_THAT(v1.project(v2), VectorEquals(-1.7307692308f, -0.5769230769f, 2.3076923077f));

			v1 = Vector3f{0.24f, 0.0082f, -0.03f};
			v2 = Vector3f{0.53f, -0.0532f, -1.53f};
			REQUIRE_THAT(Vector3f::calcProject(v1, v2), VectorEquals(0.03487f, -0.0035f, -0.10065f));
			REQUIRE_THAT(v1.project(v2), VectorEquals(0.03487f, -0.0035f, -0.10065f));

			v1 = Vector3f{-27.0f, 83.0f, -163.0f};
			v2 = Vector3f{36.0f, -64.0f, 264.0f};
			REQUIRE_THAT(Vector3f::calcProject(v1, v2), VectorEquals(-23.6439377797f, 42.0336671639f, -173.3888770509f));
			REQUIRE_THAT(v1.project(v2), VectorEquals(-23.6439377797f, 42.0336671639f, -173.3888770509f));
		}

		SECTION("reflect")
		{
			Vector3f v1{2.0f, -5.0f, 4.0f};
			Vector3f v2{6.0f, 2.0f, -8.0f};
			REQUIRE_THAT(Vector3f::calcReflect(v1, v2), VectorEquals(5.4615384615f, -3.8461538462f, -0.6153846154f));
			REQUIRE_THAT(v1.reflect(v2), VectorEquals(5.4615384615f, -3.8461538462f, -0.6153846154f));

			v1 = Vector3f{0.24f, 0.0082f, -0.03f};
			v2 = Vector3f{0.53f, -0.0532f, -1.53f};
			REQUIRE_THAT(Vector3f::calcReflect(v1, v2), VectorEquals(0.1702669066f, 0.0151996237f, 0.1713049677f));
			REQUIRE_THAT(v1.reflect(v2), VectorEquals(0.1702669066f, 0.0151996237f, 0.1713049677f));

			v1 = Vector3f{-27, 83, -163};
			v2 = Vector3f{36, -64, 264};
			REQUIRE_THAT(Vector3f::calcReflect(v1, v2), VectorEquals(20.2878755593f, -1.0673343277f, 183.7777541019f));
			REQUIRE_THAT(v1.reflect(v2), VectorEquals(20.2878755593f, -1.0673343277f, 183.7777541019f));

			v1 = Vector3f{1.0f, 2.0f, 3.0f};
			v2 = Vector3f{0.0f, 1.0f, 0.0f};
			REQUIRE_THAT(Vector3f::calcReflect(v1, v2), VectorEquals(1.0f, -2.0f, 3.0f));
			REQUIRE_THAT(v1.reflect(v2), VectorEquals(1.0f, -2.0f, 3.0f));
		}

		SECTION("pointAt")
		{
			Vector3f v1{0.0f, 0.0f, 0.0f};
			Vector3f v2{1.0f, 1.0f, 1.0f};
			REQUIRE_THAT(Vector3f::calcPointAt(v1, v2, 0.0f), VectorEquals(0.0f, 0.0f, 0.0f));
			REQUIRE_THAT(Vector3f::calcPointAt(v1, v2, 0.5f), VectorEquals(0.5f, 0.5f, 0.5f));
			REQUIRE_THAT(Vector3f::calcPointAt(v1, v2, 1.0f), VectorEquals(1.0f, 1.0f, 1.0f));

			v1 = Vector3f{0.0f, 0.5f, 0.0f};
			v2 = Vector3f{0.0f, 0.0f, 1.0f};
			REQUIRE_THAT(Vector3f::calcPointAt(v1, v2, 0.0f), VectorEquals(0.0f, 0.5f, 0.0f));
			REQUIRE_THAT(Vector3f::calcPointAt(v1, v2, 0.5f), VectorEquals(0.0f, 0.5f, 0.5f));
			REQUIRE_THAT(Vector3f::calcPointAt(v1, v2, 1.0f), VectorEquals(0.0f, 0.5f, 1.0f));

			v1 = randomVector3f();
			v2 = Vector3f::calcNormalize(randomVector3f());
			REQUIRE_THAT(Vector3f::calcPointAt(v1, v2, 0.83f), VectorEquals(v2[0] * 0.83f + v1[0], v2[1] * 0.83f + v1[1], v2[2] * 0.83f + v1[2]));
		}

		SECTION("clamp")
		{
			{
				auto v1 = randomVector3f();
				auto min = randomScalarf();
				auto max = randomScalarf();
				auto v2 = v1;
				REQUIRE_THAT(v2.clamp(min, max), VectorEquals(std::min(std::max(v1[0], min), max), std::min(std::max(v1[1], min), max), std::min(std::max(v1[2], min), max)));
			}

			{
				auto v1 = randomVector3f();
				auto min = randomScalarf();
				auto max = randomScalarf();
				auto v2 = v1;
				REQUIRE_THAT(v2.clamp(min, max), VectorEquals(std::min(std::max(v1[0], min), max), std::min(std::max(v1[1], min), max), std::min(std::max(v1[2], min), max)));
			}

			{
				Vector3f v1{2.0f, -5.0f, 4.0f};
				REQUIRE_THAT(v1.clamp(0.0f, 0.0f), VectorEquals(0.0f, 0.0f, 0.0f));
				REQUIRE_THAT(v1.clamp(1.1f, 1.1f), VectorEquals(1.1f, 1.1f, 1.1f));
			}
		}

		SECTION("abs")
		{
			auto v1 = randomVector3f();
			auto v2 = v1;
			REQUIRE_THAT(v2.abs(), VectorEquals(std::abs(v1[0]), std::abs(v1[1]), std::abs(v1[2])));

			v1 = Vector3f{0.0f, 0.0f, 0.0f};
			REQUIRE_THAT(v1.abs(), VectorEquals(0.0f, 0.0f, 0.0f));
		}

		SECTION("neg")
		{
			auto v1 = randomVector3f();
			auto v2 = v1;
			REQUIRE_THAT(v2.neg(), VectorEquals(-v1[0], -v1[1], -v1[2]));

			v1 = Vector3f{0.0f, 0.0f, 0.0f};
			REQUIRE_THAT(v1.abs(), VectorEquals(0.0f, 0.0f, 0.0f));
		}

		SECTION("mad")
		{
			auto v1 = randomVector3f();
			auto v2 = v1;
			REQUIRE_THAT(v2.mad(0.5f, 1.3f), VectorEquals(v1[0] * 0.5f + 1.3f, v1[1] * 0.5f + 1.3f, v1[2] * 0.5f + 1.3f));

			v1 = randomVector3f();
			v2 = v1;
			REQUIRE_THAT(v2.mad(0.0f, -4.88f), VectorEquals(-4.88f, -4.88f, -4.88f));
		}

		/************
		* NOTE: missing tests:
		*  - Vector3f::calcClosestInSegment
		*  - Vector3f::fromInterpolateNormals
		*  - Vector3f::evalSplineCatmullRom
		*  - Vector3f::evalSplineHermite
		*/
	}

	TEST_CASE("3 component vector of type double", "[common][vector][vector3d]")
	{
		SECTION("init")
		{
			double values[3]{-0.21, 55.11, -0.0001};

			REQUIRE_THAT(Vector3d(), VectorEquals(0.0, 0.0, 0.0));
			REQUIRE_THAT(Vector3d(1.2), VectorEquals(1.2, 1.2, 1.2));
			REQUIRE_THAT(Vector3d(1.2, -2.5, 4.123), VectorEquals(1.2, -2.5, 4.123));
			REQUIRE_THAT(Vector3d(values), VectorEquals(-0.21, 55.11, -0.0001));
			REQUIRE_THAT(Vector3d::zero(), VectorEquals(0.0, 0.0, 0.0));
		}

		SECTION("access")
		{
			Vector3d v{0.911, 2.610, -7.841};

			REQUIRE(v[0] == Catch::Approx(0.911));
			REQUIRE(v[1] == Catch::Approx(2.610));
			REQUIRE(v[2] == Catch::Approx(-7.841));
			REQUIRE(v[3] == Catch::Approx(0.911));
			REQUIRE(v[4] == Catch::Approx(2.610));
			REQUIRE(v[5] == Catch::Approx(-7.841));
			REQUIRE(v[12] == Catch::Approx(0.911));
			REQUIRE(v[13] == Catch::Approx(2.610));
			REQUIRE(v[14] == Catch::Approx(-7.841));

			REQUIRE(v.data()[0] == Catch::Approx(0.911));
			REQUIRE(v.data()[1] == Catch::Approx(2.610));
			REQUIRE(v.data()[2] == Catch::Approx(-7.841));
		}

		SECTION("basic operands")
		{
			auto v1 = randomVector3d();
			auto v2 = randomVector3d();

			auto v3 = v2;
			REQUIRE_THAT(v3, VectorEquals(v2[0], v2[1], v2[2]));

			v3 = -v1;
			REQUIRE_THAT(v3, VectorEquals(-v1[0], -v1[1], -v1[2]));

			v3 = v2;
			v3 += v1;
			REQUIRE_THAT(v3, VectorEquals(v2[0] + v1[0], v2[1] + v1[1], v2[2] + v1[2]));

			v3 = v2;
			v3 += 3.33;
			REQUIRE_THAT(v3, VectorEquals(v2[0] + 3.33, v2[1] + 3.33, v2[2] + 3.33));

			v3 = v2;
			v3 -= v1;
			REQUIRE_THAT(v3, VectorEquals(v2[0] - v1[0], v2[1] - v1[1], v2[2] - v1[2]));

			v3 = v2;
			v3 -= 0.49451;
			REQUIRE_THAT(v3, VectorEquals(v2[0] - 0.49451, v2[1] - 0.49451, v2[2] - 0.49451));

			v3 = v2;
			v3 *= v1;
			REQUIRE_THAT(v3, VectorEquals(v2[0] * v1[0], v2[1] * v1[1], v2[2] * v1[2]));

			v3 = v2;
			v3 *= -1.4599;
			REQUIRE_THAT(v3, VectorEquals(v2[0] * (-1.4599), v2[1] * (-1.4599), v2[2] * (-1.4599)));

			v3 = v2;
			v3 /= v1;
			REQUIRE_THAT(v3, VectorEquals(v2[0] / v1[0], v2[1] / v1[1], v2[2] / v1[2]));

			v3 = v2;
			v3 /= 0.8833;
			REQUIRE_THAT(v3, VectorEquals(v2[0] / 0.8833, v2[1] / 0.8833, v2[2] / 0.8833));

			REQUIRE_THAT(v2 + v1, VectorEquals(v2[0] + v1[0], v2[1] + v1[1], v2[2] + v1[2]));
			REQUIRE_THAT(v2 - v1, VectorEquals(v2[0] - v1[0], v2[1] - v1[1], v2[2] - v1[2]));
			REQUIRE_THAT(v2 * v1, VectorEquals(v2[0] * v1[0], v2[1] * v1[1], v2[2] * v1[2]));
			REQUIRE_THAT(v2 / v1, VectorEquals(v2[0] / v1[0], v2[1] / v1[1], v2[2] / v1[2]));

			REQUIRE_THAT((v2 + v1) / 1.44, VectorEquals((v2[0] + v1[0]) / 1.44, (v2[1] + v1[1]) / 1.44, (v2[2] + v1[2]) / 1.44));
			REQUIRE_THAT((v2 - v1) * 44.1, VectorEquals((v2[0] - v1[0]) * 44.1, (v2[1] - v1[1]) * 44.1, (v2[2] - v1[2]) * 44.1));
			REQUIRE_THAT((v2 * v1) - 1.44, VectorEquals((v2[0] * v1[0]) - 1.44, (v2[1] * v1[1]) - 1.44, (v2[2] * v1[2]) - 1.44));
			REQUIRE_THAT((v2 / v1) + 44.1, VectorEquals((v2[0] / v1[0]) + 44.1, (v2[1] / v1[1]) + 44.1, (v2[2] / v1[2]) + 44.1));
		}

		SECTION("comparison")
		{
			auto v1 = randomVector3d();
			auto v2 = randomVector3d();

			REQUIRE(v1 != v2);
			REQUIRE(v1 == v1);
			REQUIRE(v2 == v2);

			REQUIRE(Vector3d{}.isZero(0.0000000001));
			REQUIRE((Vector3d{} + 0.000001).isZero(0.0001));
			REQUIRE_FALSE(v2.isZero(0.0000000001));
			REQUIRE_FALSE(v2.isZero(0.1));

			REQUIRE(v2.isEqual(v2, 0.0000000001));
			REQUIRE_FALSE(v2.isEqual(v1, 0.0000000001));
			REQUIRE(Vector3d{0.001, 0.002, 0.003}.isEqual(Vector3d{0.002, 0.003, 0.004}, 0.01));
			REQUIRE_FALSE(Vector3d{0.001, 0.002, 0.003}.isEqual(Vector3d{0.002, 0.003, 0.004}, 0.001));
		}

		SECTION("convert")
		{
			auto v1 = randomVector3d();

			{
				auto conv = v1.convert<float, 3>();
				REQUIRE_THAT(conv, VectorEquals(static_cast<float>(v1[0]), static_cast<float>(v1[1]), static_cast<float>(v1[2])));
			}

			{
				auto conv = v1.convert<double, 3>();
				REQUIRE_THAT(conv, VectorEquals(v1[0], v1[1], v1[2]));
			}

			{
				auto conv = v1.convert<float, 4>(1.34f);
				REQUIRE_THAT(conv, VectorEquals(static_cast<float>(v1[0]), static_cast<float>(v1[1]), static_cast<float>(v1[2]), 1.34f));
			}

			{
				auto conv = v1.convert<double, 4>(-2.79);
				REQUIRE_THAT(conv, VectorEquals(v1[0], v1[1], v1[2], -2.79));
			}
		}

		SECTION("misc")
		{
			auto v1 = randomVector3d();

			double buffer[3];
			v1.write(buffer);
			REQUIRE_THAT(v1, VectorEquals(buffer[0], buffer[1], buffer[2]));
		}

		SECTION("cross product")
		{
			Vector3d v1{2.0, -5.0, 4.0};
			Vector3d v2{6.0, 2.0, -8.0};
			REQUIRE_THAT(Vector3d::calcCrossProduct(v1, v2), VectorEquals(32.0, 40.0, 34.0));
			REQUIRE_THAT(v1.crossProduct(v2), VectorEquals(32.0, 40.0, 34.0));

			v1 = Vector3d{0.24, 0.0082, -0.03};
			v2 = Vector3d{0.53, -0.0532, -1.53};
			REQUIRE_THAT(Vector3d::calcCrossProduct(v1, v2), VectorEquals(-0.014142, 0.3513, -0.017114));
			REQUIRE_THAT(v1.crossProduct(v2), VectorEquals(-0.014142, 0.3513, -0.017114));

			v1 = Vector3d{-27.0, 83.0, -163.0};
			v2 = Vector3d{36.0, -64.0, 264.0};
			REQUIRE_THAT(Vector3d::calcCrossProduct(v1, v2), VectorEquals(11480.0, 1260.0, -1260.0));
			REQUIRE_THAT(v1.crossProduct(v2), VectorEquals(11480.0, 1260.0, -1260.0));

			// Cross product of vector and itself is zero
			REQUIRE_THAT(Vector3d::calcCrossProduct(v1, v1), VectorEquals(0.0, 0.0, 0.0));
			REQUIRE_THAT(v1.crossProduct(v1), VectorEquals(0.0, 0.0, 0.0));
		}

		SECTION("distance")
		{
			Vector3d v1{2.0, -5.0, 4.0};
			Vector3d v2{6.0, 2.0, -8.0};
			REQUIRE(v1.distance(v2) == Catch::Approx(14.4568));

			v1 = Vector3d{0.24, 0.0082, -0.03};
			v2 = Vector3d{0.53, -0.0532, -1.53};
			REQUIRE(v1.distance(v2) == Catch::Approx(1.52901));

			v1 = Vector3d{-27.0, 83.0, -163.0};
			v2 = Vector3d{36.0, -64.0, 264.0};
			REQUIRE(v1.distance(v2) == Catch::Approx(455.968));
		}

		SECTION("dot product")
		{
			Vector3d v1{2.0, -5.0, 4.0};
			Vector3d v2{6.0, 2.0, -8.0};
			REQUIRE(v1.dot() == Catch::Approx(45.0));
			REQUIRE(v1.dot(v2) == Catch::Approx(-30.0));

			v1 = Vector3d{0.24, 0.0082, -0.03};
			v2 = Vector3d{0.53, -0.0532, -1.53};
			REQUIRE(v1.dot() == Catch::Approx(0.05856724));
			REQUIRE(v1.dot(v2) == Catch::Approx(0.172663));

			v1 = Vector3d{-27.0, 83.0, -163.0};
			v2 = Vector3d{36.0, -64.0, 264.0};
			REQUIRE(v1.dot() == Catch::Approx(34187.0));
			REQUIRE(v1.dot(v2) == Catch::Approx(-49316.0));
		}

		SECTION("spherical coordinate space")
		{
			double rad = 6.7082;
			double theta = 0.931931;
			double phi = -1.19029;
			auto v = Vector3d::fromSpherical(rad, theta, phi);
			REQUIRE_THAT(v, VectorEquals(2.0, -5.0, 4.0));

			rad = 0.242007;
			theta = 1.69508;
			phi = 0.0341533;
			v = Vector3d::fromSpherical(rad, theta, phi);
			REQUIRE_THAT(v, VectorEquals(0.24, 0.0082, -0.03));

			rad = 184.8972;
			theta = 2.6499755;
			phi = 1.8853006;
			v = Vector3d::fromSpherical(rad, theta, phi);
			REQUIRE_THAT(v, VectorEquals(-27.0, 83.0, -163.0));
		}

		SECTION("interpolate")
		{
			Vector3d v1{2.0, -5.0, 4.0};
			Vector3d v2{6.0, 2.0, -8.0};
			auto v = Vector3d::calcLerp(v1, v2, 0.0);
			REQUIRE_THAT(v, VectorEquals(2.0, -5.0, 4.0));

			v1 = Vector3d{0.24, 0.0082, -0.03};
			v2 = Vector3d{0.53, -0.0532, -1.53};
			v = Vector3d::calcLerp(v1, v2, 0.33);
			REQUIRE_THAT(v, VectorEquals(0.3357, -0.012062, -0.525));

			v1 = Vector3d{-27.0, 83.0, -163.0};
			v2 = Vector3d{36.0, -64.0, 264.0};
			v = Vector3d::calcLerp(v1, v2, 1.0);
			REQUIRE_THAT(v, VectorEquals(36.0, -64.0, 264.0));
		}

		SECTION("magnitude")
		{
			// set epsilon to allowed a 0.1% difference

			Vector3d v{2.0, -5.0, 4.0};
			REQUIRE(v.magnitude() == Catch::Approx(6.70820393));
			REQUIRE(v.magnitudeInv() == Catch::Approx(1.0 / 6.70820393).epsilon(0.001));

			v = Vector3d{0.24, 0.0082, -0.03};
			REQUIRE(v.magnitude() == Catch::Approx(0.242007));
			REQUIRE(v.magnitudeInv() == Catch::Approx(1.0 / 0.242007).epsilon(0.001));

			v = Vector3d{-27.0, 83.0, -163.0};
			REQUIRE(v.magnitude() == Catch::Approx(184.897269));
			REQUIRE(v.magnitudeInv() == Catch::Approx(1.0 / 184.897269).epsilon(0.001));
		}

		SECTION("min")
		{
			Vector3d v1{2.0, -5.0, 4.0};
			Vector3d v2{6.0, 2.0, -8.0};
			auto v = Vector3d::calcMin(v1, v2);
			REQUIRE_THAT(v, VectorEquals(2.0, -5.0, -8.0));

			v1 = Vector3d{0.24, 0.0082, -0.03};
			v2 = Vector3d{0.53, -0.0532, -1.53};
			v = Vector3d::calcMin(v1, v2);
			REQUIRE_THAT(v, VectorEquals(0.24, -0.0532, -1.53));

			v1 = Vector3d{-27.0, 83.0, -163.0};
			v2 = Vector3d{36.0, -64.0, 264.0};
			v = Vector3d::calcMin(v1, v2);
			REQUIRE_THAT(v, VectorEquals(-27.0, -64.0, -163.0));
		}

		SECTION("max")
		{
			Vector3d v1{2.0, -5.0, 4.0};
			Vector3d v2{6.0, 2.0, -8.0};
			auto v = Vector3d::calcMax(v1, v2);
			REQUIRE_THAT(v, VectorEquals(6.0, 2.0, 4.0));

			v1 = Vector3d{0.24, 0.0082, -0.03};
			v2 = Vector3d{0.53, -0.0532, -1.53};
			v = Vector3d::calcMax(v1, v2);
			REQUIRE_THAT(v, VectorEquals(0.53, 0.0082, -0.03));

			v1 = Vector3d{-27.0, 83.0, -163.0};
			v2 = Vector3d{36.0, -64.0, 264.0};
			v = Vector3d::calcMax(v1, v2);
			REQUIRE_THAT(v, VectorEquals(36.0, 83.0, 264.0));
		}

		SECTION("minmax")
		{
			auto v1 = randomVector3d();
			auto v2 = randomVector3d();
			auto v = Vector3d::calcMinMax(v1, v2);
			REQUIRE_THAT(std::get<0>(v), VectorEquals(std::min(v1[0], v2[0]), std::min(v1[1], v2[1]), std::min(v1[2], v2[2])));
			REQUIRE_THAT(std::get<1>(v), VectorEquals(std::max(v1[0], v2[0]), std::max(v1[1], v2[1]), std::max(v1[2], v2[2])));

			v1 = randomVector3d();
			v2 = randomVector3d();
			v = Vector3d::calcMinMax(v1, v2);
			REQUIRE_THAT(std::get<0>(v), VectorEquals(std::min(v1[0], v2[0]), std::min(v1[1], v2[1]), std::min(v1[2], v2[2])));
			REQUIRE_THAT(std::get<1>(v), VectorEquals(std::max(v1[0], v2[0]), std::max(v1[1], v2[1]), std::max(v1[2], v2[2])));
		}

		SECTION("mormalize")
		{
			Vector3d v{2.0, -5.0, 4.0};
			auto n = Vector3d::calcNormalize(v);
			v.normalize();
			REQUIRE_THAT(n, VectorEquals(0.298142, -0.745356, 0.596285));
			REQUIRE_THAT(v, VectorEquals(0.298142, -0.745356, 0.596285));

			v = Vector3d{0.24, 0.0082, -0.03};
			n = Vector3d::calcNormalize(v);
			v.normalize();
			REQUIRE_THAT(n, VectorEquals(0.991708, 0.0338834, -0.123964));
			REQUIRE_THAT(v, VectorEquals(0.991708, 0.0338834, -0.123964));

			v = Vector3d{-27.0, 83.0, -163.0};
			n = Vector3d::calcNormalize(v);
			v.normalize();
			REQUIRE_THAT(n, VectorEquals(-0.146027, 0.448898, -0.881571));
			REQUIRE_THAT(v, VectorEquals(-0.146027, 0.448898, -0.881571));
		}

		SECTION("project")
		{
			Vector3d v1{2.0, -5.0, 4.0};
			Vector3d v2{6.0, 2.0, -8.0};
			REQUIRE_THAT(Vector3d::calcProject(v1, v2), VectorEquals(-1.7307692308, -0.5769230769, 2.3076923077));
			REQUIRE_THAT(v1.project(v2), VectorEquals(-1.7307692308, -0.5769230769, 2.3076923077));

			v1 = Vector3d{0.24, 0.0082, -0.03};
			v2 = Vector3d{0.53, -0.0532, -1.53};
			REQUIRE_THAT(Vector3d::calcProject(v1, v2), VectorEquals(0.03487, -0.0035, -0.10065));
			REQUIRE_THAT(v1.project(v2), VectorEquals(0.03487, -0.0035, -0.10065));

			v1 = Vector3d{-27.0, 83.0, -163.0};
			v2 = Vector3d{36.0, -64.0, 264.0};
			REQUIRE_THAT(Vector3d::calcProject(v1, v2), VectorEquals(-23.6439377797, 42.0336671639, -173.3888770509));
			REQUIRE_THAT(v1.project(v2), VectorEquals(-23.6439377797, 42.0336671639, -173.3888770509));
		}

		SECTION("reflect")
		{
			Vector3d v1{2.0, -5.0, 4.0};
			Vector3d v2{6.0, 2.0, -8.0};
			REQUIRE_THAT(Vector3d::calcReflect(v1, v2), VectorEquals(5.4615384615, -3.8461538462, -0.6153846154));
			REQUIRE_THAT(v1.reflect(v2), VectorEquals(5.4615384615, -3.8461538462, -0.6153846154));

			v1 = Vector3d{0.24, 0.0082, -0.03};
			v2 = Vector3d{0.53, -0.0532, -1.53};
			REQUIRE_THAT(Vector3d::calcReflect(v1, v2), VectorEquals(0.1702669066, 0.0151996237, 0.1713049677));
			REQUIRE_THAT(v1.reflect(v2), VectorEquals(0.1702669066, 0.0151996237, 0.1713049677));

			v1 = Vector3d{-27, 83, -163};
			v2 = Vector3d{36, -64, 264};
			REQUIRE_THAT(Vector3d::calcReflect(v1, v2), VectorEquals(20.2878755593, -1.0673343277, 183.7777541019));
			REQUIRE_THAT(v1.reflect(v2), VectorEquals(20.2878755593, -1.0673343277, 183.7777541019));

			v1 = Vector3d{1.0, 2.0, 3.0};
			v2 = Vector3d{0.0, 1.0, 0.0};
			REQUIRE_THAT(Vector3d::calcReflect(v1, v2), VectorEquals(1.0, -2.0, 3.0));
			REQUIRE_THAT(v1.reflect(v2), VectorEquals(1.0, -2.0, 3.0));
		}

		SECTION("pointAt")
		{
			Vector3d v1{0.0, 0.0, 0.0};
			Vector3d v2{1.0, 1.0, 1.0};
			REQUIRE_THAT(Vector3d::calcPointAt(v1, v2, 0.0), VectorEquals(0.0, 0.0, 0.0));
			REQUIRE_THAT(Vector3d::calcPointAt(v1, v2, 0.5), VectorEquals(0.5, 0.5, 0.5));
			REQUIRE_THAT(Vector3d::calcPointAt(v1, v2, 1.0), VectorEquals(1.0, 1.0, 1.0));

			v1 = Vector3d{0.0, 0.5, 0.0};
			v2 = Vector3d{0.0, 0.0, 1.0};
			REQUIRE_THAT(Vector3d::calcPointAt(v1, v2, 0.0), VectorEquals(0.0, 0.5, 0.0));
			REQUIRE_THAT(Vector3d::calcPointAt(v1, v2, 0.5), VectorEquals(0.0, 0.5, 0.5));
			REQUIRE_THAT(Vector3d::calcPointAt(v1, v2, 1.0), VectorEquals(0.0, 0.5, 1.0));

			v1 = randomVector3d();
			v2 = Vector3d::calcNormalize(randomVector3d());
			REQUIRE_THAT(Vector3d::calcPointAt(v1, v2, 0.83), VectorEquals(v2[0] * 0.83 + v1[0], v2[1] * 0.83 + v1[1], v2[2] * 0.83 + v1[2]));
		}

		SECTION("clamp")
		{
			{
				auto v1 = randomVector3d();
				auto min = randomScalard();
				auto max = randomScalard();
				auto v2 = v1;
				REQUIRE_THAT(v2.clamp(min, max), VectorEquals(std::min(std::max(v1[0], min), max), std::min(std::max(v1[1], min), max), std::min(std::max(v1[2], min), max)));
			}

			{
				auto v1 = randomVector3d();
				auto min = randomScalard();
				auto max = randomScalard();
				auto v2 = v1;
				REQUIRE_THAT(v2.clamp(min, max), VectorEquals(std::min(std::max(v1[0], min), max), std::min(std::max(v1[1], min), max), std::min(std::max(v1[2], min), max)));
			}

			{
				Vector3d v1{2.0, -5.0, 4.0};
				REQUIRE_THAT(v1.clamp(0.0, 0.0), VectorEquals(0.0, 0.0, 0.0));
				REQUIRE_THAT(v1.clamp(1.1, 1.1), VectorEquals(1.1, 1.1, 1.1));
			}
		}

		SECTION("abs")
		{
			auto v1 = randomVector3d();
			auto v2 = v1;
			REQUIRE_THAT(v2.abs(), VectorEquals(std::abs(v1[0]), std::abs(v1[1]), std::abs(v1[2])));

			v1 = Vector3d{0.0, 0.0, 0.0};
			REQUIRE_THAT(v1.abs(), VectorEquals(0.0, 0.0, 0.0));
		}

		SECTION("neg")
		{
			auto v1 = randomVector3d();
			auto v2 = v1;
			REQUIRE_THAT(v2.neg(), VectorEquals(-v1[0], -v1[1], -v1[2]));

			v1 = Vector3d{0.0, 0.0, 0.0};
			REQUIRE_THAT(v1.abs(), VectorEquals(0.0, 0.0, 0.0));
		}

		SECTION("mad")
		{
			auto v1 = randomVector3d();
			auto v2 = v1;
			REQUIRE_THAT(v2.mad(0.5, 1.3), VectorEquals(v1[0] * 0.5 + 1.3, v1[1] * 0.5 + 1.3, v1[2] * 0.5 + 1.3));

			v1 = randomVector3d();
			v2 = v1;
			REQUIRE_THAT(v2.mad(0.0, -4.88), VectorEquals(-4.88, -4.88, -4.88));
		}

		/************
		* NOTE: missing tests:
		*  - Vector3d::calcClosestInSegment
		*  - Vector3d::fromInterpolateNormals
		*/
	}

	TEST_CASE("4 component vector of type float", "[common][vector][vector4f]")
	{
		SECTION("init")
		{
			float values[4]{-0.21f, 55.11f, -0.0001f, 234.77f};

			REQUIRE_THAT(Vector4f(), VectorEquals(0.0f, 0.0f, 0.0f, 0.0f));
			REQUIRE_THAT(Vector4f(1.2f), VectorEquals(1.2f, 1.2f, 1.2f, 1.2f));
			REQUIRE_THAT(Vector4f(1.2f, -2.5f, 4.123f, -83.11f), VectorEquals(1.2f, -2.5f, 4.123f, -83.11f));
			REQUIRE_THAT(Vector4f(values), VectorEquals(-0.21f, 55.11f, -0.0001f, 234.77f));
			REQUIRE_THAT(Vector4f::zero(), VectorEquals(0.0f, 0.0f, 0.0f, 0.0f));
		}

		SECTION("access")
		{
			Vector4f v{0.911f, 2.610f, -7.841f, 0.42901f};

			REQUIRE(v[0] == Catch::Approx(0.911f));
			REQUIRE(v[1] == Catch::Approx(2.610f));
			REQUIRE(v[2] == Catch::Approx(-7.841f));
			REQUIRE(v[3] == Catch::Approx(0.42901f));
			REQUIRE(v[4] == Catch::Approx(0.911f));
			REQUIRE(v[5] == Catch::Approx(2.610f));
			REQUIRE(v[6] == Catch::Approx(-7.841f));
			REQUIRE(v[7] == Catch::Approx(0.42901f));
			REQUIRE(v[8] == Catch::Approx(0.911f));
			REQUIRE(v[9] == Catch::Approx(2.610f));
			REQUIRE(v[10] == Catch::Approx(-7.841f));
			REQUIRE(v[11] == Catch::Approx(0.42901f));

			REQUIRE(v.data()[0] == Catch::Approx(0.911f));
			REQUIRE(v.data()[1] == Catch::Approx(2.610f));
			REQUIRE(v.data()[2] == Catch::Approx(-7.841f));
			REQUIRE(v.data()[3] == Catch::Approx(0.42901f));
		}

		SECTION("basic operands")
		{
			auto v1 = randomVector4f();
			auto v2 = randomVector4f();

			auto v3 = v2;
			REQUIRE_THAT(v3, VectorEquals(v2[0], v2[1], v2[2], v2[3]));

			v3 = -v1;
			REQUIRE_THAT(v3, VectorEquals(-v1[0], -v1[1], -v1[2], -v1[3]));

			v3 = v2;
			v3 += v1;
			REQUIRE_THAT(v3, VectorEquals(v2[0] + v1[0], v2[1] + v1[1], v2[2] + v1[2], v2[3] + v1[3]));

			v3 = v2;
			v3 += 3.33f;
			REQUIRE_THAT(v3, VectorEquals(v2[0] + 3.33f, v2[1] + 3.33f, v2[2] + 3.33f, v2[3] + 3.33f));

			v3 = v2;
			v3 -= v1;
			REQUIRE_THAT(v3, VectorEquals(v2[0] - v1[0], v2[1] - v1[1], v2[2] - v1[2], v2[3] - v1[3]));

			v3 = v2;
			v3 -= 0.49451f;
			REQUIRE_THAT(v3, VectorEquals(v2[0] - 0.49451f, v2[1] - 0.49451f, v2[2] - 0.49451f, v2[3] - 0.49451f));

			v3 = v2;
			v3 *= v1;
			REQUIRE_THAT(v3, VectorEquals(v2[0] * v1[0], v2[1] * v1[1], v2[2] * v1[2], v2[3] * v1[3]));

			v3 = v2;
			v3 *= -1.4599f;
			REQUIRE_THAT(v3, VectorEquals(v2[0] * (-1.4599f), v2[1] * (-1.4599f), v2[2] * (-1.4599f), v2[3] * (-1.4599f)));

			v3 = v2;
			v3 /= v1;
			REQUIRE_THAT(v3, VectorEquals(v2[0] / v1[0], v2[1] / v1[1], v2[2] / v1[2], v2[3] / v1[3]));

			v3 = v2;
			v3 /= 0.8833f;
			REQUIRE_THAT(v3, VectorEquals(v2[0] / 0.8833f, v2[1] / 0.8833f, v2[2] / 0.8833f, v2[3] / 0.8833f));

			REQUIRE_THAT(v2 + v1, VectorEquals(v2[0] + v1[0], v2[1] + v1[1], v2[2] + v1[2], v2[3] + v1[3]));
			REQUIRE_THAT(v2 - v1, VectorEquals(v2[0] - v1[0], v2[1] - v1[1], v2[2] - v1[2], v2[3] - v1[3]));
			REQUIRE_THAT(v2 * v1, VectorEquals(v2[0] * v1[0], v2[1] * v1[1], v2[2] * v1[2], v2[3] * v1[3]));
			REQUIRE_THAT(v2 / v1, VectorEquals(v2[0] / v1[0], v2[1] / v1[1], v2[2] / v1[2], v2[3] / v1[3]));

			REQUIRE_THAT((v2 + v1) / 1.44f, VectorEquals((v2[0] + v1[0]) / 1.44f, (v2[1] + v1[1]) / 1.44f, (v2[2] + v1[2]) / 1.44f, (v2[3] + v1[3]) / 1.44f));
			REQUIRE_THAT((v2 - v1) * 44.1f, VectorEquals((v2[0] - v1[0]) * 44.1f, (v2[1] - v1[1]) * 44.1f, (v2[2] - v1[2]) * 44.1f, (v2[3] - v1[3]) * 44.1f));
			REQUIRE_THAT((v2 * v1) - 1.44f, VectorEquals((v2[0] * v1[0]) - 1.44f, (v2[1] * v1[1]) - 1.44f, (v2[2] * v1[2]) - 1.44f, (v2[3] * v1[3]) - 1.44f));
			REQUIRE_THAT((v2 / v1) + 44.1f, VectorEquals((v2[0] / v1[0]) + 44.1f, (v2[1] / v1[1]) + 44.1f, (v2[2] / v1[2]) + 44.1f, (v2[3] / v1[3]) + 44.1f));
		}

		SECTION("comparison")
		{
			auto v1 = randomVector4f();
			auto v2 = randomVector4f();

			REQUIRE(v1 != v2);
			REQUIRE(v1 == v1);
			REQUIRE(v2 == v2);

			REQUIRE(Vector4f{}.isZero(0.0000000001f));
			REQUIRE((Vector4f{} + 0.000001f).isZero(0.0001f));
			REQUIRE_FALSE(v2.isZero(0.0000000001f));
			REQUIRE_FALSE(v2.isZero(0.1f));

			REQUIRE(v2.isEqual(v2, 0.0000000001f));
			REQUIRE_FALSE(v2.isEqual(v1, 0.0000000001f));
			REQUIRE(Vector4f{0.001f, 0.002f, 0.003f, 0.004f}.isEqual(Vector4f{0.002f, 0.003f, 0.004f, 0.005f}, 0.01f));
			REQUIRE_FALSE(Vector4f{0.001f, 0.002f, 0.003f, 0.004f}.isEqual(Vector4f{0.002f, 0.003f, 0.004f, 0.005f}, 0.001f));
		}

		SECTION("convert")
		{
			auto v1 = randomVector4f();

			{
				auto conv = v1.convert<float, 3>();
				REQUIRE_THAT(conv, VectorEquals(v1[0], v1[1], v1[2]));
			}

			{
				auto conv = v1.convert<double, 3>();
				REQUIRE_THAT(conv, VectorEquals(static_cast<double>(v1[0]), static_cast<double>(v1[1]), static_cast<double>(v1[2])));
			}

			{
				auto conv = v1.convert<float, 4>();
				REQUIRE_THAT(conv, VectorEquals(v1[0], v1[1], v1[2], v1[3]));
			}

			{
				auto conv = v1.convert<double, 4>();
				REQUIRE_THAT(conv, VectorEquals(static_cast<double>(v1[0]), static_cast<double>(v1[1]), static_cast<double>(v1[2]), static_cast<double>(v1[3])));
			}
		}

		SECTION("misc")
		{
			auto v1 = randomVector4f();

			float buffer[4];
			v1.write(buffer);
			REQUIRE_THAT(v1, VectorEquals(buffer[0], buffer[1], buffer[2], buffer[3]));
		}

		SECTION("distance")
		{
			Vector4f v1{2.0f, -5.0f, 4.0f, -0.6661f};
			Vector4f v2{6.0f, 2.0f, 44.33f, -8.0f};
			REQUIRE(v1.distance(v2) == Catch::Approx(41.77672784230474f));

			v1 = Vector4f{0.24f, 0.0082f, -0.03f, 1.863f};
			v2 = Vector4f{0.53f, 0.9499f, -0.0532f, -1.53f};
			REQUIRE(v1.distance(v2) == Catch::Approx(3.5332543f));

			v1 = Vector4f{-27.0f, 83.0f, -163.0f, 9812.44f};
			v2 = Vector4f{36.0f, -64.0f, 264.0f, 111.11f};
			REQUIRE(v1.distance(v2) == Catch::Approx(9712.03947));
		}

		SECTION("dot product")
		{
			Vector4f v1{2.0f, -5.0f, 4.0f, -0.6661f};
			Vector4f v2{6.0f, 2.0f, 44.33f, -8.0f};
			REQUIRE(v1.dot() == Catch::Approx(45.443689f));
			REQUIRE(v1.dot(v2) == Catch::Approx(184.6488f));

			v1 = Vector4f{0.24f, 0.0082f, -0.03f, 1.863f};
			v2 = Vector4f{0.53f, 0.9499f, -0.0532f, -1.53f};
			REQUIRE(v1.dot() == Catch::Approx(3.529336f));
			REQUIRE(v1.dot(v2) == Catch::Approx(-2.7138048f));

			v1 = Vector4f{-27.0f, 83.0f, -163.0f, 9812.44f};
			v2 = Vector4f{36.0f, -64.0f, 264.0f, 111.11f};
			REQUIRE(v1.dot() == Catch::Approx(96318165.75360002f));
			REQUIRE(v1.dot(v2) == Catch::Approx(1040944.208400f));
		}

		SECTION("interpolate")
		{
			Vector4f v1{2.0f, -5.0f, 4.0f, -0.6661f};
			Vector4f v2{6.0f, 2.0f, 44.33f, -8.0f};
			auto v = Vector4f::calcLerp(v1, v2, 0.0f);
			REQUIRE_THAT(v, VectorEquals(2.0f, -5.0f, 4.0f, -0.6661f));

			v1 = Vector4f{0.24f, 0.0082f, -0.03f, 1.863f};
			v2 = Vector4f{0.53f, 0.9499f, -0.0532f, -1.53f};
			v = Vector4f::calcLerp(v1, v2, 0.33f);
			REQUIRE_THAT(v, VectorEquals(0.3357f, 0.318961f, -0.0376559f, 0.743309f));

			v1 = Vector4f{-27.0f, 83.0f, -163.0f, 9812.44f};
			v2 = Vector4f{36.0f, -64.0f, 264.0f, 111.11f};
			v = Vector4f::calcLerp(v1, v2, 1.0f);
			REQUIRE_THAT(v, VectorEquals(36.0f, -64.0f, 264.0f, 111.11f));
		}

		SECTION("magnitude")
		{
			// set epsilon to allowed a 0.1% difference

			Vector4f v{2.0f, -5.0f, 4.0f, -0.6661f};
			REQUIRE(v.magnitude() == Catch::Approx(6.7411934f));
			REQUIRE(v.magnitudeInv() == Catch::Approx(1.0f / 6.7411934f).epsilon(0.001));

			v = Vector4f{0.24f, 0.0082f, -0.03f, 1.863f};
			REQUIRE(v.magnitude() == Catch::Approx(1.878652f));
			REQUIRE(v.magnitudeInv() == Catch::Approx(1.0f / 1.878652f).epsilon(0.001));

			v = Vector4f{-27.0f, 83.0f, -163.0f, 9812.44f};
			REQUIRE(v.magnitude() == Catch::Approx(9814.1818f));
			REQUIRE(v.magnitudeInv() == Catch::Approx(1.0f / 9814.1818f).epsilon(0.001));
		}

		SECTION("min")
		{
			Vector4f v1{2.0f, -5.0f, 4.0f, 0.496f};
			Vector4f v2{6.0f, -0.9995f, 2.0f, -8.0f};
			auto v = Vector4f::calcMin(v1, v2);
			REQUIRE_THAT(v, VectorEquals(2.0f, -5.0f, 2.0f, -8.0f));

			v1 = Vector4f{0.24f, 0.0082f, -0.03f, 45.22f};
			v2 = Vector4f{0.53f, 10.005f, -0.0532f, -1.53f};
			v = Vector4f::calcMin(v1, v2);
			REQUIRE_THAT(v, VectorEquals(0.24f, 0.0082f, -0.0532f, -1.53f));

			v1 = Vector4f{-27.0f, 0.4499f, 83.0f, -163.0f};
			v2 = Vector4f{36.0f, -64.0f, 264.0f, 0.00001f};
			v = Vector4f::calcMin(v1, v2);
			REQUIRE_THAT(v, VectorEquals(-27.0f, -64.0f, 83.0f, -163.0f));
		}

		SECTION("max")
		{
			Vector4f v1{2.0f, -5.0f, 4.0f, 0.496f};
			Vector4f v2{6.0f, -0.9995f, 2.0f, -8.0f};
			auto v = Vector4f::calcMax(v1, v2);
			REQUIRE_THAT(v, VectorEquals(6.0f, -0.9995f, 4.0f, 0.496f));

			v1 = Vector4f{0.24f, 0.0082f, -0.03f, 45.22f};
			v2 = Vector4f{0.53f, 10.005f, -0.0532f, -1.53f};
			v = Vector4f::calcMax(v1, v2);
			REQUIRE_THAT(v, VectorEquals(0.53f, 10.005f, -0.03f, 45.22f));

			v1 = Vector4f{-27.0f, 0.4499f, 83.0f, -163.0f};
			v2 = Vector4f{36.0f, -64.0f, 264.0f, 0.00001f};
			v = Vector4f::calcMax(v1, v2);
			REQUIRE_THAT(v, VectorEquals(36.0f, 0.4499f, 264.0f, 0.00001f));
		}

		SECTION("minmax")
		{
			auto v1 = randomVector4f();
			auto v2 = randomVector4f();
			auto v = Vector4f::calcMinMax(v1, v2);
			REQUIRE_THAT(std::get<0>(v), VectorEquals(std::min(v1[0], v2[0]), std::min(v1[1], v2[1]), std::min(v1[2], v2[2]), std::min(v1[3], v2[3])));
			REQUIRE_THAT(std::get<1>(v), VectorEquals(std::max(v1[0], v2[0]), std::max(v1[1], v2[1]), std::max(v1[2], v2[2]), std::max(v1[3], v2[3])));

			v1 = randomVector4f();
			v2 = randomVector4f();
			v = Vector4f::calcMinMax(v1, v2);
			REQUIRE_THAT(std::get<0>(v), VectorEquals(std::min(v1[0], v2[0]), std::min(v1[1], v2[1]), std::min(v1[2], v2[2]), std::min(v1[3], v2[3])));
			REQUIRE_THAT(std::get<1>(v), VectorEquals(std::max(v1[0], v2[0]), std::max(v1[1], v2[1]), std::max(v1[2], v2[2]), std::max(v1[3], v2[3])));
		}

		SECTION("mormalize")
		{
			Vector4f v{2.0f, -5.0f, 4.0f, 0.496f};
			auto n = Vector4f::calcNormalize(v);
			v.normalize();
			REQUIRE_THAT(n, VectorEquals(0.2973307f, -0.74332686f, 0.5946614f, 0.073738f));
			REQUIRE_THAT(v, VectorEquals(0.2973307f, -0.74332686f, 0.5946614f, 0.073738f));

			v = Vector4f{0.24f, 0.0082f, -0.03f, 1.863f};
			n = Vector4f::calcNormalize(v);
			v.normalize();
			REQUIRE_THAT(n, VectorEquals(0.127751f, 0.004365f, -0.015969f, 0.991668f));
			REQUIRE_THAT(v, VectorEquals(0.127751f, 0.004365f, -0.015969f, 0.991668f));

			v = Vector4f{-27.0f, 83.0f, -163.0f, 9812.44f};
			n = Vector4f::calcNormalize(v);
			v.normalize();
			REQUIRE_THAT(n, VectorEquals(-0.002751f, 0.008457f, -0.016609f, 0.999823f));
			REQUIRE_THAT(v, VectorEquals(-0.002751f, 0.008457f, -0.016609f, 0.999823f));
		}

		SECTION("pointAt")
		{
			Vector4f v1{0.0f, 0.0f, 0.0f, 0.0f};
			Vector4f v2{1.0f, 1.0f, 1.0f, 1.0f};
			REQUIRE_THAT(Vector4f::calcPointAt(v1, v2, 0.0f), VectorEquals(0.0f, 0.0f, 0.0f, 0.0f));
			REQUIRE_THAT(Vector4f::calcPointAt(v1, v2, 0.5f), VectorEquals(0.5f, 0.5f, 0.5f, 0.5f));
			REQUIRE_THAT(Vector4f::calcPointAt(v1, v2, 1.0f), VectorEquals(1.0f, 1.0f, 1.0f, 1.0f));

			v1 = Vector4f{0.0f, 0.5f, 0.0f, 0.33f};
			v2 = Vector4f{0.0f, 0.0f, 1.0f, 0.0f};
			REQUIRE_THAT(Vector4f::calcPointAt(v1, v2, 0.0f), VectorEquals(0.0f, 0.5f, 0.0f, 0.33f));
			REQUIRE_THAT(Vector4f::calcPointAt(v1, v2, 0.5f), VectorEquals(0.0f, 0.5f, 0.5f, 0.33f));
			REQUIRE_THAT(Vector4f::calcPointAt(v1, v2, 1.0f), VectorEquals(0.0f, 0.5f, 1.0f, 0.33f));

			v1 = randomVector4f();
			v2 = Vector4f::calcNormalize(randomVector4f());
			REQUIRE_THAT(Vector4f::calcPointAt(v1, v2, 0.83f), VectorEquals(v2[0] * 0.83f + v1[0], v2[1] * 0.83f + v1[1], v2[2] * 0.83f + v1[2], v2[3] * 0.83f + v1[3]));
		}

		SECTION("clamp")
		{
			{
				auto v1 = randomVector4f();
				auto min = randomScalarf();
				auto max = randomScalarf();
				auto v2 = v1;
				REQUIRE_THAT(v2.clamp(min, max),
				  VectorEquals(std::min(std::max(v1[0], min), max), std::min(std::max(v1[1], min), max), std::min(std::max(v1[2], min), max), std::min(std::max(v1[3], min), max)));
			}

			{
				auto v1 = randomVector4f();
				auto min = randomScalarf();
				auto max = randomScalarf();
				auto v2 = v1;
				REQUIRE_THAT(v2.clamp(min, max),
				  VectorEquals(std::min(std::max(v1[0], min), max), std::min(std::max(v1[1], min), max), std::min(std::max(v1[2], min), max), std::min(std::max(v1[3], min), max)));
			}

			{
				Vector4f v1{2.0f, -5.0f, 4.0f, -0.55124f};
				REQUIRE_THAT(v1.clamp(0.0f, 0.0f), VectorEquals(0.0f, 0.0f, 0.0f, 0.0f));
				REQUIRE_THAT(v1.clamp(1.1f, 1.1f), VectorEquals(1.1f, 1.1f, 1.1f, 1.1f));
			}
		}

		SECTION("abs")
		{
			auto v1 = randomVector4f();
			auto v2 = v1;
			REQUIRE_THAT(v2.abs(), VectorEquals(std::abs(v1[0]), std::abs(v1[1]), std::abs(v1[2]), std::abs(v1[3])));

			v1 = Vector4f{0.0f, 0.0f, 0.0f, 0.0f};
			REQUIRE_THAT(v1.abs(), VectorEquals(0.0f, 0.0f, 0.0f, 0.0f));
		}

		SECTION("neg")
		{
			auto v1 = randomVector4f();
			auto v2 = v1;
			REQUIRE_THAT(v2.neg(), VectorEquals(-v1[0], -v1[1], -v1[2], -v1[3]));

			v1 = Vector4f{0.0f, 0.0f, 0.0f, 0.0f};
			REQUIRE_THAT(v1.abs(), VectorEquals(0.0f, 0.0f, 0.0f, 0.0f));
		}

		SECTION("mad")
		{
			auto v1 = randomVector4f();
			auto v2 = v1;
			REQUIRE_THAT(v2.mad(0.5f, 1.3f), VectorEquals(v1[0] * 0.5f + 1.3f, v1[1] * 0.5f + 1.3f, v1[2] * 0.5f + 1.3f, v1[3] * 0.5f + 1.3f));

			v1 = randomVector4f();
			v2 = v1;
			REQUIRE_THAT(v2.mad(0.0f, -4.88f), VectorEquals(-4.88f, -4.88f, -4.88f, -4.88f));
		}
	}

	TEST_CASE("4 component vector of type double", "[common][vector][vector4d]")
	{
		SECTION("init")
		{
			double values[4]{-0.21, 55.11, -0.0001, 234.77};

			REQUIRE_THAT(Vector4d(), VectorEquals(0.0, 0.0, 0.0, 0.0));
			REQUIRE_THAT(Vector4d(1.2), VectorEquals(1.2, 1.2, 1.2, 1.2));
			REQUIRE_THAT(Vector4d(1.2, -2.5, 4.123, -83.11), VectorEquals(1.2, -2.5, 4.123, -83.11));
			REQUIRE_THAT(Vector4d(values), VectorEquals(-0.21, 55.11, -0.0001, 234.77));
			REQUIRE_THAT(Vector4d::zero(), VectorEquals(0.0, 0.0, 0.0, 0.0));
		}

		SECTION("access")
		{
			Vector4d v{0.911, 2.610, -7.841, 0.42901};

			REQUIRE(v[0] == Catch::Approx(0.911));
			REQUIRE(v[1] == Catch::Approx(2.610));
			REQUIRE(v[2] == Catch::Approx(-7.841));
			REQUIRE(v[3] == Catch::Approx(0.42901));
			REQUIRE(v[4] == Catch::Approx(0.911));
			REQUIRE(v[5] == Catch::Approx(2.610));
			REQUIRE(v[6] == Catch::Approx(-7.841));
			REQUIRE(v[7] == Catch::Approx(0.42901));
			REQUIRE(v[8] == Catch::Approx(0.911));
			REQUIRE(v[9] == Catch::Approx(2.610));
			REQUIRE(v[10] == Catch::Approx(-7.841));
			REQUIRE(v[11] == Catch::Approx(0.42901));

			REQUIRE(v.data()[0] == Catch::Approx(0.911));
			REQUIRE(v.data()[1] == Catch::Approx(2.610));
			REQUIRE(v.data()[2] == Catch::Approx(-7.841));
			REQUIRE(v.data()[3] == Catch::Approx(0.42901));
		}

		SECTION("basic operands")
		{
			auto v1 = randomVector4d();
			auto v2 = randomVector4d();

			auto v3 = v2;
			REQUIRE_THAT(v3, VectorEquals(v2[0], v2[1], v2[2], v2[3]));

			v3 = -v1;
			REQUIRE_THAT(v3, VectorEquals(-v1[0], -v1[1], -v1[2], -v1[3]));

			v3 = v2;
			v3 += v1;
			REQUIRE_THAT(v3, VectorEquals(v2[0] + v1[0], v2[1] + v1[1], v2[2] + v1[2], v2[3] + v1[3]));

			v3 = v2;
			v3 += 3.33;
			REQUIRE_THAT(v3, VectorEquals(v2[0] + 3.33, v2[1] + 3.33, v2[2] + 3.33, v2[3] + 3.33));

			v3 = v2;
			v3 -= v1;
			REQUIRE_THAT(v3, VectorEquals(v2[0] - v1[0], v2[1] - v1[1], v2[2] - v1[2], v2[3] - v1[3]));

			v3 = v2;
			v3 -= 0.49451;
			REQUIRE_THAT(v3, VectorEquals(v2[0] - 0.49451, v2[1] - 0.49451, v2[2] - 0.49451, v2[3] - 0.49451));

			v3 = v2;
			v3 *= v1;
			REQUIRE_THAT(v3, VectorEquals(v2[0] * v1[0], v2[1] * v1[1], v2[2] * v1[2], v2[3] * v1[3]));

			v3 = v2;
			v3 *= -1.4599;
			REQUIRE_THAT(v3, VectorEquals(v2[0] * (-1.4599), v2[1] * (-1.4599), v2[2] * (-1.4599), v2[3] * (-1.4599)));

			v3 = v2;
			v3 /= v1;
			REQUIRE_THAT(v3, VectorEquals(v2[0] / v1[0], v2[1] / v1[1], v2[2] / v1[2], v2[3] / v1[3]));

			v3 = v2;
			v3 /= 0.8833;
			REQUIRE_THAT(v3, VectorEquals(v2[0] / 0.8833, v2[1] / 0.8833, v2[2] / 0.8833, v2[3] / 0.8833));

			REQUIRE_THAT(v2 + v1, VectorEquals(v2[0] + v1[0], v2[1] + v1[1], v2[2] + v1[2], v2[3] + v1[3]));
			REQUIRE_THAT(v2 - v1, VectorEquals(v2[0] - v1[0], v2[1] - v1[1], v2[2] - v1[2], v2[3] - v1[3]));
			REQUIRE_THAT(v2 * v1, VectorEquals(v2[0] * v1[0], v2[1] * v1[1], v2[2] * v1[2], v2[3] * v1[3]));
			REQUIRE_THAT(v2 / v1, VectorEquals(v2[0] / v1[0], v2[1] / v1[1], v2[2] / v1[2], v2[3] / v1[3]));

			REQUIRE_THAT((v2 + v1) / 1.44, VectorEquals((v2[0] + v1[0]) / 1.44, (v2[1] + v1[1]) / 1.44, (v2[2] + v1[2]) / 1.44, (v2[3] + v1[3]) / 1.44));
			REQUIRE_THAT((v2 - v1) * 44.1, VectorEquals((v2[0] - v1[0]) * 44.1, (v2[1] - v1[1]) * 44.1, (v2[2] - v1[2]) * 44.1, (v2[3] - v1[3]) * 44.1));
			REQUIRE_THAT((v2 * v1) - 1.44, VectorEquals((v2[0] * v1[0]) - 1.44, (v2[1] * v1[1]) - 1.44, (v2[2] * v1[2]) - 1.44, (v2[3] * v1[3]) - 1.44));
			REQUIRE_THAT((v2 / v1) + 44.1, VectorEquals((v2[0] / v1[0]) + 44.1, (v2[1] / v1[1]) + 44.1, (v2[2] / v1[2]) + 44.1, (v2[3] / v1[3]) + 44.1));
		}

		SECTION("comparison")
		{
			auto v1 = randomVector4d();
			auto v2 = randomVector4d();

			REQUIRE(v1 != v2);
			REQUIRE(v1 == v1);
			REQUIRE(v2 == v2);

			REQUIRE(Vector4d{}.isZero(0.0000000001));
			REQUIRE((Vector4d{} + 0.000001f).isZero(0.0001));
			REQUIRE_FALSE(v2.isZero(0.0000000001));
			REQUIRE_FALSE(v2.isZero(0.1));

			REQUIRE(v2.isEqual(v2, 0.0000000001));
			REQUIRE_FALSE(v2.isEqual(v1, 0.0000000001));
			REQUIRE(Vector4d{0.001, 0.002, 0.003, 0.004f}.isEqual(Vector4d{0.002, 0.003, 0.004, 0.005f}, 0.01));
			REQUIRE_FALSE(Vector4d{0.001, 0.002, 0.003, 0.004f}.isEqual(Vector4d{0.002, 0.003, 0.004, 0.005f}, 0.001));
		}

		SECTION("convert")
		{
			auto v1 = randomVector4d();

			{
				auto conv = v1.convert<float, 3>();
				REQUIRE_THAT(conv, VectorEquals(static_cast<float>(v1[0]), static_cast<float>(v1[1]), static_cast<float>(v1[2])));
			}

			{
				auto conv = v1.convert<double, 3>();
				REQUIRE_THAT(conv, VectorEquals(v1[0], v1[1], v1[2]));
			}

			{
				auto conv = v1.convert<float, 4>();
				REQUIRE_THAT(conv, VectorEquals(static_cast<float>(v1[0]), static_cast<float>(v1[1]), static_cast<float>(v1[2]), static_cast<float>(v1[3])));
			}

			{
				auto conv = v1.convert<double, 4>();
				REQUIRE_THAT(conv, VectorEquals(v1[0], v1[1], v1[2], v1[3]));
			}
		}

		SECTION("misc")
		{
			auto v1 = randomVector4d();

			double buffer[4];
			v1.write(buffer);
			REQUIRE_THAT(v1, VectorEquals(buffer[0], buffer[1], buffer[2], buffer[3]));
		}

		SECTION("distance")
		{
			Vector4d v1{2.0, -5.0, 4.0, -0.6661};
			Vector4d v2{6.0, 2.0, 44.33, -8.0};
			REQUIRE(v1.distance(v2) == Catch::Approx(41.77672784230474));

			v1 = Vector4d{0.24, 0.0082, -0.03, 1.863};
			v2 = Vector4d{0.53, 0.9499, -0.0532, -1.53};
			REQUIRE(v1.distance(v2) == Catch::Approx(3.5332543));

			v1 = Vector4d{-27.0, 83.0, -163.0, 9812.44};
			v2 = Vector4d{36.0, -64.0, 264.0, 111.11};
			REQUIRE(v1.distance(v2) == Catch::Approx(9712.03947));
		}

		SECTION("dot product")
		{
			Vector4d v1{2.0, -5.0, 4.0, -0.6661};
			Vector4d v2{6.0, 2.0, 44.33, -8.0};
			REQUIRE(v1.dot() == Catch::Approx(45.443689));
			REQUIRE(v1.dot(v2) == Catch::Approx(184.6488));

			v1 = Vector4d{0.24, 0.0082, -0.03, 1.863};
			v2 = Vector4d{0.53, 0.9499, -0.0532, -1.53};
			REQUIRE(v1.dot() == Catch::Approx(3.529336));
			REQUIRE(v1.dot(v2) == Catch::Approx(-2.7138048));

			v1 = Vector4d{-27.0, 83.0, -163.0, 9812.44};
			v2 = Vector4d{36.0, -64.0, 264.0, 111.11};
			REQUIRE(v1.dot() == Catch::Approx(96318165.75360002));
			REQUIRE(v1.dot(v2) == Catch::Approx(1040944.208400));
		}

		SECTION("interpolate")
		{
			Vector4d v1{2.0, -5.0, 4.0, -0.6661};
			Vector4d v2{6.0, 2.0, 44.33, -8.0};
			auto v = Vector4d::calcLerp(v1, v2, 0.0f);
			REQUIRE_THAT(v, VectorEquals(2.0, -5.0, 4.0, -0.6661));

			v1 = Vector4d{0.24, 0.0082, -0.03, 1.863};
			v2 = Vector4d{0.53, 0.9499, -0.0532, -1.53};
			v = Vector4d::calcLerp(v1, v2, 0.33f);
			REQUIRE_THAT(v, VectorEquals(0.3357, 0.318961, -0.0376559, 0.743309));

			v1 = Vector4d{-27.0, 83.0, -163.0, 9812.44};
			v2 = Vector4d{36.0, -64.0, 264.0, 111.11};
			v = Vector4d::calcLerp(v1, v2, 1.0f);
			REQUIRE_THAT(v, VectorEquals(36.0, -64.0, 264.0, 111.11));
		}

		SECTION("magnitude")
		{
			// set epsilon to allowed a 0.1% difference

			Vector4d v{2.0, -5.0, 4.0, -0.6661};
			REQUIRE(v.magnitude() == Catch::Approx(6.7411934));
			REQUIRE(v.magnitudeInv() == Catch::Approx(1.0f / 6.7411934f).epsilon(0.001));

			v = Vector4d{0.24, 0.0082, -0.03, 1.863};
			REQUIRE(v.magnitude() == Catch::Approx(1.878652));
			REQUIRE(v.magnitudeInv() == Catch::Approx(1.0f / 1.878652f).epsilon(0.001));

			v = Vector4d{-27.0, 83.0, -163.0, 9812.44};
			REQUIRE(v.magnitude() == Catch::Approx(9814.1818));
			REQUIRE(v.magnitudeInv() == Catch::Approx(1.0f / 9814.1818f).epsilon(0.001));
		}

		SECTION("min")
		{
			Vector4d v1{2.0, -5.0, 4.0, 0.496};
			Vector4d v2{6.0, -0.9995, 2.0, -8.0};
			auto v = Vector4d::calcMin(v1, v2);
			REQUIRE_THAT(v, VectorEquals(2.0, -5.0, 2.0, -8.0));

			v1 = Vector4d{0.24, 0.0082, -0.03, 45.22};
			v2 = Vector4d{0.53, 10.005, -0.0532, -1.53};
			v = Vector4d::calcMin(v1, v2);
			REQUIRE_THAT(v, VectorEquals(0.24, 0.0082, -0.0532, -1.53));

			v1 = Vector4d{-27.0, 0.4499, 83.0, -163.0};
			v2 = Vector4d{36.0, -64.0, 264.0, 0.00001};
			v = Vector4d::calcMin(v1, v2);
			REQUIRE_THAT(v, VectorEquals(-27.0, -64.0, 83.0, -163.0));
		}

		SECTION("max")
		{
			Vector4d v1{2.0, -5.0, 4.0, 0.496};
			Vector4d v2{6.0, -0.9995, 2.0, -8.0};
			auto v = Vector4d::calcMax(v1, v2);
			REQUIRE_THAT(v, VectorEquals(6.0, -0.9995, 4.0, 0.496));

			v1 = Vector4d{0.24, 0.0082, -0.03, 45.22};
			v2 = Vector4d{0.53, 10.005, -0.0532, -1.53};
			v = Vector4d::calcMax(v1, v2);
			REQUIRE_THAT(v, VectorEquals(0.53, 10.005, -0.03, 45.22));

			v1 = Vector4d{-27.0, 0.4499, 83.0, -163.0};
			v2 = Vector4d{36.0, -64.0, 264.0, 0.00001};
			v = Vector4d::calcMax(v1, v2);
			REQUIRE_THAT(v, VectorEquals(36.0, 0.4499, 264.0, 0.00001));
		}

		SECTION("minmax")
		{
			auto v1 = randomVector4d();
			auto v2 = randomVector4d();
			auto v = Vector4d::calcMinMax(v1, v2);
			REQUIRE_THAT(std::get<0>(v), VectorEquals(std::min(v1[0], v2[0]), std::min(v1[1], v2[1]), std::min(v1[2], v2[2]), std::min(v1[3], v2[3])));
			REQUIRE_THAT(std::get<1>(v), VectorEquals(std::max(v1[0], v2[0]), std::max(v1[1], v2[1]), std::max(v1[2], v2[2]), std::max(v1[3], v2[3])));

			v1 = randomVector4d();
			v2 = randomVector4d();
			v = Vector4d::calcMinMax(v1, v2);
			REQUIRE_THAT(std::get<0>(v), VectorEquals(std::min(v1[0], v2[0]), std::min(v1[1], v2[1]), std::min(v1[2], v2[2]), std::min(v1[3], v2[3])));
			REQUIRE_THAT(std::get<1>(v), VectorEquals(std::max(v1[0], v2[0]), std::max(v1[1], v2[1]), std::max(v1[2], v2[2]), std::max(v1[3], v2[3])));
		}

		SECTION("mormalize")
		{
			Vector4d v{2.0, -5.0, 4.0, 0.496};
			auto n = Vector4d::calcNormalize(v);
			v.normalize();
			REQUIRE_THAT(n, VectorEquals(0.2973307, -0.74332686, 0.5946614, 0.073738));
			REQUIRE_THAT(v, VectorEquals(0.2973307, -0.74332686, 0.5946614, 0.073738));

			v = Vector4d{0.24, 0.0082, -0.03, 1.863};
			n = Vector4d::calcNormalize(v);
			v.normalize();
			REQUIRE_THAT(n, VectorEquals(0.127751, 0.004365, -0.015969, 0.991668));
			REQUIRE_THAT(v, VectorEquals(0.127751, 0.004365, -0.015969, 0.991668));

			v = Vector4d{-27.0, 83.0, -163.0, 9812.44};
			n = Vector4d::calcNormalize(v);
			v.normalize();
			REQUIRE_THAT(n, VectorEquals(-0.002751, 0.008457, -0.016609, 0.999823));
			REQUIRE_THAT(v, VectorEquals(-0.002751, 0.008457, -0.016609, 0.999823));
		}

		SECTION("pointAt")
		{
			Vector4d v1{0.0, 0.0, 0.0, 0.0};
			Vector4d v2{1.0, 1.0, 1.0, 1.0};
			REQUIRE_THAT(Vector4d::calcPointAt(v1, v2, 0.0), VectorEquals(0.0, 0.0, 0.0, 0.0));
			REQUIRE_THAT(Vector4d::calcPointAt(v1, v2, 0.5), VectorEquals(0.5, 0.5, 0.5, 0.5));
			REQUIRE_THAT(Vector4d::calcPointAt(v1, v2, 1.0), VectorEquals(1.0, 1.0, 1.0, 1.0));

			v1 = Vector4d{0.0, 0.5, 0.0, 0.33};
			v2 = Vector4d{0.0, 0.0, 1.0, 0.0};
			REQUIRE_THAT(Vector4d::calcPointAt(v1, v2, 0.0), VectorEquals(0.0, 0.5, 0.0, 0.33));
			REQUIRE_THAT(Vector4d::calcPointAt(v1, v2, 0.5), VectorEquals(0.0, 0.5, 0.5, 0.33));
			REQUIRE_THAT(Vector4d::calcPointAt(v1, v2, 1.0), VectorEquals(0.0, 0.5, 1.0, 0.33));

			v1 = randomVector4d();
			v2 = Vector4d::calcNormalize(randomVector4d());
			REQUIRE_THAT(Vector4d::calcPointAt(v1, v2, 0.83),
			  VectorEquals(v2[0] * 0.83f + v1[0], v2[1] * 0.83f + v1[1], v2[2] * 0.83f + v1[2], v2[3] * 0.83f + v1[3]));
		}

		SECTION("clamp")
		{
			{
				auto v1 = randomVector4d();
				auto min = randomScalard();
				auto max = randomScalard();
				auto v2 = v1;
				REQUIRE_THAT(v2.clamp(min, max), VectorEquals(std::min(std::max(v1[0], min), max), std::min(std::max(v1[1], min), max), std::min(std::max(v1[2], min), max), std::min(std::max(v1[3], min), max)));
			}

			{
				auto v1 = randomVector4d();
				auto min = randomScalard();
				auto max = randomScalard();
				auto v2 = v1;
				REQUIRE_THAT(v2.clamp(min, max), VectorEquals(std::min(std::max(v1[0], min), max), std::min(std::max(v1[1], min), max), std::min(std::max(v1[2], min), max), std::min(std::max(v1[3], min), max)));
			}

			{
				Vector4d v1{2.0, -5.0, 4.0, -0.55124};
				REQUIRE_THAT(v1.clamp(0.0, 0.0), VectorEquals(0.0, 0.0, 0.0, 0.0));
				REQUIRE_THAT(v1.clamp(1.1, 1.1), VectorEquals(1.1, 1.1, 1.1, 1.1));
			}
		}

		SECTION("abs")
		{
			auto v1 = randomVector4d();
			auto v2 = v1;
			REQUIRE_THAT(v2.abs(), VectorEquals(std::abs(v1[0]), std::abs(v1[1]), std::abs(v1[2]), std::abs(v1[3])));

			v1 = Vector4d{0.0, 0.0, 0.0, 0.0};
			REQUIRE_THAT(v1.abs(), VectorEquals(0.0, 0.0, 0.0, 0.0));
		}

		SECTION("neg")
		{
			auto v1 = randomVector4d();
			auto v2 = v1;
			REQUIRE_THAT(v2.neg(), VectorEquals(-v1[0], -v1[1], -v1[2], -v1[3]));

			v1 = Vector4d{0.0, 0.0, 0.0, 0.0};
			REQUIRE_THAT(v1.abs(), VectorEquals(0.0, 0.0, 0.0, 0.0));
		}

		SECTION("mad")
		{
			auto v1 = randomVector4d();
			auto v2 = v1;
			REQUIRE_THAT(v2.mad(0.5, 1.3), VectorEquals(v1[0] * 0.5 + 1.3, v1[1] * 0.5 + 1.3, v1[2] * 0.5 + 1.3, v1[3] * 0.5 + 1.3));

			v1 = randomVector4d();
			v2 = v1;
			REQUIRE_THAT(v2.mad(0.0, -4.88), VectorEquals(-4.88, -4.88, -4.88, -4.88));
		}
	}
}
