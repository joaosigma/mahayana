#pragma once

#include "utils_t.hpp"

#include "triangle.hpp"

#include <catch2/catch_test_macros.hpp>

namespace hr::utests
{
	TEST_CASE("triangle vec3f", "[common][triangle][triangle3f]")
	{
		SECTION("init and access")
		{
			Triangle<Vector3f> tri{Vector3f{1.0f, -2.0f, 3.0f}, Vector3f{-4.0f, 5.0f, -6.0f}, Vector3f{7.0f, -8.0f, 9.0f}};

			REQUIRE_THAT(tri[0], VectorEquals(1.0f, -2.0f, 3.0f));
			REQUIRE_THAT(tri[1], VectorEquals(-4.0f, 5.0f, -6.0f));
			REQUIRE_THAT(tri[2], VectorEquals(7.0f, -8.0f, 9.0f));
		}

		SECTION("calc normal")
		{
			Vector3f p1{0.0f, 0.0f, 0.0f};
			Vector3f p2{0.5f, 0.0f, 0.5f};
			Vector3f p3{1.0f, 0.0f, 0.0f};

			REQUIRE_THAT(Triangle<Vector3f>::calcNormal(p1, p2, p3), VectorEquals(0.0f, 1.0f, 0.0f));
			REQUIRE_THAT(Triangle<Vector3f>::calcNormal(p1.data(), p2.data(), p3.data()), VectorEquals(0.0f, 1.0f, 0.0f));

			REQUIRE_THAT(Triangle<Vector3f>::calcNormal(p1, p3, p2), VectorEquals(0.0f, -1.0f, 0.0f));
			REQUIRE_THAT(Triangle<Vector3f>::calcNormal(p1.data(), p3.data(), p2.data()), VectorEquals(0.0f, -1.0f, 0.0f));
		}
	}
}
