#pragma once

#include "math.hpp"

#include "random.hpp"

#include <catch2/benchmark/catch_benchmark.hpp>
#include <catch2/catch_approx.hpp>
#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_vector.hpp>

namespace hr::utests
{
    namespace
    {
        template<typename T>
        struct MyApprox: public Catch::Approx
        {
            MyApprox(T value)
              : Catch::Approx(value)
            {
                epsilon(0.001);
                margin(0.0000001);
            }

            MyApprox(T value, double custom_epsilon)
              : Catch::Approx(value)
            {
                epsilon(custom_epsilon);
                margin(0.0000001);
            }
        };

        void randomVector(std::vector<float>& vec)
        {
            Random r;

            for (auto& value : vec)
                value = static_cast<float>(r.nextDouble(-10.0f, 10.0f));
        }

        void randomVector(std::vector<double>& vec)
        {
            Random r;

            for (auto& value : vec)
                value = r.nextDouble(-10.0f, 10.0f);
        }
    }

    TEST_CASE("Math", "[common][math]")
    {
        SECTION("simd")
        {
            {
                std::vector<float> valsf;
                valsf.resize(103);
                randomVector(valsf);

                auto targets = valsf;
                for (auto& val : targets)
                    val = (val * 0.46f) + 0.27f;

                hr::Math::SIMD::mad(valsf, 0.46f, 0.27f);
                REQUIRE_THAT(valsf, Catch::Matchers::Approx(targets));
            }

            {
                std::vector<double> valsd;
                valsd.resize(99);
                randomVector(valsd);

                auto targets = valsd;
                for (auto& val : targets)
                    val = (val * 0.69) - 0.17;

                hr::Math::SIMD::mad(valsd, 0.69f, -0.17f);
                REQUIRE_THAT(valsd, Catch::Matchers::Approx(targets));
            }

            constexpr size_t NumValue{5000};

            BENCHMARK_ADVANCED("mad not optimized (float)")(Catch::Benchmark::Chronometer meter)
            {
                std::vector<float> vals;
                vals.resize(NumValue);
                randomVector(vals);

                meter.measure(
                  [&vals]
                  {
                      for (auto& val : vals)
                          val = (val * 0.46f) + 0.27f;

                      return vals.back();
                  });
            };

            BENCHMARK_ADVANCED("mad optimized (float)")(Catch::Benchmark::Chronometer meter)
            {
                std::vector<float> vals;
                vals.resize(NumValue);
                randomVector(vals);

                meter.measure(
                  [&vals]
                  {
                      hr::Math::SIMD::mad(vals, 0.46f, 0.27f);
                      return vals.back();
                  });
            };

            BENCHMARK_ADVANCED("mad not optimized (double)")(Catch::Benchmark::Chronometer meter)
            {
                std::vector<double> vals;
                vals.resize(NumValue);
                randomVector(vals);

                meter.measure(
                  [&vals]
                  {
                      for (auto& val : vals)
                          val = (val * 0.46) + 0.27;

                      return vals.back();
                  });
            };

            BENCHMARK_ADVANCED("mad optimized (double)")(Catch::Benchmark::Chronometer meter)
            {
                std::vector<double> vals;
                vals.resize(NumValue);
                randomVector(vals);

                meter.measure(
                  [&vals]
                  {
                      hr::Math::SIMD::mad(vals, 0.46, 0.27);
                      return vals.back();
                  });
            };
        }

        SECTION("sqrt")
        {
            REQUIRE(hr::Math::sqrt(1.0f) == Catch::Approx(std::sqrt(1.0f)));
            REQUIRE(hr::Math::sqrt(1.1f) == Catch::Approx(std::sqrt(1.1f)));
            REQUIRE(hr::Math::sqrt(0.34f) == Catch::Approx(std::sqrt(0.34f)));
            REQUIRE(hr::Math::sqrt(2.2f) == Catch::Approx(std::sqrt(2.2f)));

            REQUIRE(hr::Math::sqrt(1.0) == Catch::Approx(std::sqrt(1.0)));
            REQUIRE(hr::Math::sqrt(1.1) == Catch::Approx(std::sqrt(1.1)));
            REQUIRE(hr::Math::sqrt(0.34) == Catch::Approx(std::sqrt(0.34)));
            REQUIRE(hr::Math::sqrt(2.2) == Catch::Approx(std::sqrt(2.2)));

            REQUIRE(hr::Math::sqrtInv(1.0f) == MyApprox(1.0f / std::sqrt(1.0f)));
            REQUIRE(hr::Math::sqrtInv(1.1f) == MyApprox(1.0f / std::sqrt(1.1f)));
            REQUIRE(hr::Math::sqrtInv(0.34f) == MyApprox(1.0f / std::sqrt(0.34f)));
            REQUIRE(hr::Math::sqrtInv(2.2f) == MyApprox(1.0f / std::sqrt(2.2f)));

            REQUIRE(hr::Math::sqrtInv(1.0) == Catch::Approx(1.0 / std::sqrt(1.0)));
            REQUIRE(hr::Math::sqrtInv(1.1) == Catch::Approx(1.0 / std::sqrt(1.1)));
            REQUIRE(hr::Math::sqrtInv(0.34) == Catch::Approx(1.0 / std::sqrt(0.34)));
            REQUIRE(hr::Math::sqrtInv(2.2) == Catch::Approx(1.0 / std::sqrt(2.2)));
        }

        SECTION("sin / cos")
        {
            // single calls

            REQUIRE(hr::Math::sin(0.0f) == MyApprox(std::sin(0.0f)));
            REQUIRE(hr::Math::sin(3.14159f) == MyApprox(std::sin(3.14159f)));
            REQUIRE(hr::Math::sin(6.0f) == MyApprox(std::sin(6.0f)));
            REQUIRE(hr::Math::sin(243.0f) == MyApprox(std::sin(243.0f)));
            REQUIRE(hr::Math::sin(-0.42f) == MyApprox(std::sin(-0.42f)));
            REQUIRE(hr::Math::sin(6.3f) == MyApprox(std::sin(6.3f)));
            REQUIRE(hr::Math::sin(-2219.0f) == MyApprox(std::sin(-2219.0f)));
            REQUIRE(hr::Math::sin(-15.678f) == MyApprox(std::sin(-15.678f)));

            REQUIRE(hr::Math::cos(0.0f) == MyApprox(std::cos(0.0f)));
            REQUIRE(hr::Math::cos(3.14159f) == MyApprox(std::cos(3.14159f)));
            REQUIRE(hr::Math::cos(6.0f) == MyApprox(std::cos(6.0f)));
            REQUIRE(hr::Math::cos(243.0f) == MyApprox(std::cos(243.0f)));
            REQUIRE(hr::Math::cos(-0.42f) == MyApprox(std::cos(-0.42f)));
            REQUIRE(hr::Math::cos(6.3f) == MyApprox(std::cos(6.3f)));
            REQUIRE(hr::Math::cos(-2219.0f) == MyApprox(std::cos(-2219.0f)));
            REQUIRE(hr::Math::cos(-15.678f) == MyApprox(std::cos(-15.678f)));

            REQUIRE(hr::Math::sin(0.0) == MyApprox(std::sin(0.0)));
            REQUIRE(hr::Math::sin(3.14159) == MyApprox(std::sin(3.14159)));
            REQUIRE(hr::Math::sin(6.0) == MyApprox(std::sin(6.0)));
            REQUIRE(hr::Math::sin(243.0) == MyApprox(std::sin(243.0)));
            REQUIRE(hr::Math::sin(-0.42) == MyApprox(std::sin(-0.42)));
            REQUIRE(hr::Math::sin(6.3) == MyApprox(std::sin(6.3)));
            REQUIRE(hr::Math::sin(-2219.0) == MyApprox(std::sin(-2219.0)));
            REQUIRE(hr::Math::sin(-15.678) == MyApprox(std::sin(-15.678)));

            REQUIRE(hr::Math::cos(0.0) == MyApprox(std::cos(0.0)));
            REQUIRE(hr::Math::cos(3.14159) == MyApprox(std::cos(3.14159)));
            REQUIRE(hr::Math::cos(6.0) == MyApprox(std::cos(6.0)));
            REQUIRE(hr::Math::cos(243.0) == MyApprox(std::cos(243.0)));
            REQUIRE(hr::Math::cos(-0.42) == MyApprox(std::cos(-0.42)));
            REQUIRE(hr::Math::cos(6.3) == MyApprox(std::cos(6.3)));
            REQUIRE(hr::Math::cos(-2219.0) == MyApprox(std::cos(-2219.0)));
            REQUIRE(hr::Math::cos(-15.678) == MyApprox(std::cos(-15.678)));

            // composite calls
            {
                float s, c;
                auto res = hr::Math::sinCos(2.389f);
                hr::Math::sinCos(2.389f, s, c);
                REQUIRE(std::get<0>(res) == MyApprox(s));
                REQUIRE(std::get<1>(res) == MyApprox(c));

                res = hr::Math::sinCos(0.0f);
                REQUIRE(std::get<0>(res) == MyApprox(std::sin(0.0f)));
                REQUIRE(std::get<1>(res) == MyApprox(std::cos(0.0f)));
                res = hr::Math::sinCos(3.14159f);
                REQUIRE(std::get<0>(res) == MyApprox(std::sin(3.14159f)));
                REQUIRE(std::get<1>(res) == MyApprox(std::cos(3.14159f)));
                res = hr::Math::sinCos(6.0f);
                REQUIRE(std::get<0>(res) == MyApprox(std::sin(6.0f)));
                REQUIRE(std::get<1>(res) == MyApprox(std::cos(6.0f)));
                res = hr::Math::sinCos(243.0f);
                REQUIRE(std::get<0>(res) == MyApprox(std::sin(243.0f)));
                REQUIRE(std::get<1>(res) == MyApprox(std::cos(243.0f)));
                res = hr::Math::sinCos(-0.42f);
                REQUIRE(std::get<0>(res) == MyApprox(std::sin(-0.42f)));
                REQUIRE(std::get<1>(res) == MyApprox(std::cos(-0.42f)));
                res = hr::Math::sinCos(6.3f);
                REQUIRE(std::get<0>(res) == MyApprox(std::sin(6.3f)));
                REQUIRE(std::get<1>(res) == MyApprox(std::cos(6.3f)));
                res = hr::Math::sinCos(-2219.0f);
                REQUIRE(std::get<0>(res) == MyApprox(std::sin(-2219.0f)));
                REQUIRE(std::get<1>(res) == MyApprox(std::cos(-2219.0f)));
                res = hr::Math::sinCos(-15.678f);
                REQUIRE(std::get<0>(res) == MyApprox(std::sin(-15.678f)));
                REQUIRE(std::get<1>(res) == MyApprox(std::cos(-15.678f)));
            }

            {
                double s, c;
                auto res = hr::Math::sinCos(-2.389);
                hr::Math::sinCos(-2.389, s, c);
                REQUIRE(std::get<0>(res) == MyApprox(s));
                REQUIRE(std::get<1>(res) == MyApprox(c));

                res = hr::Math::sinCos(0.0);
                REQUIRE(std::get<0>(res) == MyApprox(std::sin(0.0)));
                REQUIRE(std::get<1>(res) == MyApprox(std::cos(0.0)));
                res = hr::Math::sinCos(3.14159);
                REQUIRE(std::get<0>(res) == MyApprox(std::sin(3.14159)));
                REQUIRE(std::get<1>(res) == MyApprox(std::cos(3.14159)));
                res = hr::Math::sinCos(6.0);
                REQUIRE(std::get<0>(res) == MyApprox(std::sin(6.0)));
                REQUIRE(std::get<1>(res) == MyApprox(std::cos(6.0)));
                res = hr::Math::sinCos(243.0);
                REQUIRE(std::get<0>(res) == MyApprox(std::sin(243.0)));
                REQUIRE(std::get<1>(res) == MyApprox(std::cos(243.0)));
                res = hr::Math::sinCos(-0.42);
                REQUIRE(std::get<0>(res) == MyApprox(std::sin(-0.42)));
                REQUIRE(std::get<1>(res) == MyApprox(std::cos(-0.42)));
                res = hr::Math::sinCos(6.3);
                REQUIRE(std::get<0>(res) == MyApprox(std::sin(6.3)));
                REQUIRE(std::get<1>(res) == MyApprox(std::cos(6.3)));
                res = hr::Math::sinCos(-2219.0);
                REQUIRE(std::get<0>(res) == MyApprox(std::sin(-2219.0)));
                REQUIRE(std::get<1>(res) == MyApprox(std::cos(-2219.0)));
                res = hr::Math::sinCos(-15.678);
                REQUIRE(std::get<0>(res) == MyApprox(std::sin(-15.678)));
                REQUIRE(std::get<1>(res) == MyApprox(std::cos(-15.678)));
            }
        }

        SECTION("misc")
        {
            REQUIRE(hr::Math::floor(0.9f) == Catch::Approx(0.0f));
            REQUIRE(hr::Math::floor(1.1f) == Catch::Approx(1.0f));
            REQUIRE(hr::Math::floor(1.9f) == Catch::Approx(1.0f));
            REQUIRE(hr::Math::floor(2.0f) == Catch::Approx(2.0f));
            REQUIRE(hr::Math::floor(2.1f) == Catch::Approx(2.0f));
            REQUIRE(hr::Math::floor(-1.2f) == Catch::Approx(-2.0f));
            REQUIRE(hr::Math::floor(-1.9f) == Catch::Approx(-2.0f));
            REQUIRE(hr::Math::floor(-2.1f) == Catch::Approx(-3.0f));

            REQUIRE(hr::Math::floor(0.9) == Catch::Approx(0.0));
            REQUIRE(hr::Math::floor(1.1) == Catch::Approx(1.0));
            REQUIRE(hr::Math::floor(1.9) == Catch::Approx(1.0));
            REQUIRE(hr::Math::floor(2.0) == Catch::Approx(2.0));
            REQUIRE(hr::Math::floor(2.1) == Catch::Approx(2.0));
            REQUIRE(hr::Math::floor(-1.2) == Catch::Approx(-2.0));
            REQUIRE(hr::Math::floor(-1.9) == Catch::Approx(-2.0));
            REQUIRE(hr::Math::floor(-2.1) == Catch::Approx(-3.0));

            REQUIRE(hr::Math::ceil(0.9f) == Catch::Approx(1.0f));
            REQUIRE(hr::Math::ceil(1.1f) == Catch::Approx(2.0f));
            REQUIRE(hr::Math::ceil(1.9f) == Catch::Approx(2.0f));
            REQUIRE(hr::Math::ceil(2.0f) == Catch::Approx(2.0f));
            REQUIRE(hr::Math::ceil(2.1f) == Catch::Approx(3.0f));
            REQUIRE(hr::Math::ceil(-1.2f) == Catch::Approx(-1.0f));
            REQUIRE(hr::Math::ceil(-1.9f) == Catch::Approx(-1.0f));
            REQUIRE(hr::Math::ceil(-2.1f) == Catch::Approx(-2.0f));

            REQUIRE(hr::Math::ceil(0.9) == Catch::Approx(1.0));
            REQUIRE(hr::Math::ceil(1.1) == Catch::Approx(2.0));
            REQUIRE(hr::Math::ceil(1.9) == Catch::Approx(2.0));
            REQUIRE(hr::Math::ceil(2.0) == Catch::Approx(2.0));
            REQUIRE(hr::Math::ceil(2.1) == Catch::Approx(3.0));
            REQUIRE(hr::Math::ceil(-1.2) == Catch::Approx(-1.0));
            REQUIRE(hr::Math::ceil(-1.9) == Catch::Approx(-1.0));
            REQUIRE(hr::Math::ceil(-2.1) == Catch::Approx(-2.0));

            REQUIRE(hr::Math::nearestInt(0.4f) == Catch::Approx(0.0f));
            REQUIRE(hr::Math::nearestInt(0.5f) == Catch::Approx(1.0f));
            REQUIRE(hr::Math::nearestInt(0.6f) == Catch::Approx(1.0f));
            REQUIRE(hr::Math::nearestInt(-0.4f) == Catch::Approx(0.0f));
            REQUIRE(hr::Math::nearestInt(-0.5f) == Catch::Approx(0.0f));
            REQUIRE(hr::Math::nearestInt(-0.6f) == Catch::Approx(-1.0f));

            REQUIRE(hr::Math::nearestInt(0.4) == Catch::Approx(0.0));
            REQUIRE(hr::Math::nearestInt(0.5) == Catch::Approx(1.0));
            REQUIRE(hr::Math::nearestInt(0.6) == Catch::Approx(1.0));
            REQUIRE(hr::Math::nearestInt(-0.4) == Catch::Approx(0.0));
            REQUIRE(hr::Math::nearestInt(-0.5) == Catch::Approx(0.0));
            REQUIRE(hr::Math::nearestInt(-0.6) == Catch::Approx(-1.0));

            REQUIRE(hr::Math::ftoi<int32_t>(0.4f) == Catch::Approx(0));
            REQUIRE(hr::Math::ftoi<int32_t>(0.5f) == Catch::Approx(0));
            REQUIRE(hr::Math::ftoi<int32_t>(0.6f) == Catch::Approx(1));
            REQUIRE(hr::Math::ftoi<int32_t>(-0.4f) == Catch::Approx(0));
            REQUIRE(hr::Math::ftoi<int32_t>(-0.5f) == Catch::Approx(0));
            REQUIRE(hr::Math::ftoi<int32_t>(-0.6f) == Catch::Approx(-1));

            REQUIRE(hr::Math::ftoi(0.4) == Catch::Approx(0));
            REQUIRE(hr::Math::ftoi(0.5) == Catch::Approx(1));
            REQUIRE(hr::Math::ftoi(0.6) == Catch::Approx(1));
            REQUIRE(hr::Math::ftoi(-0.4) == Catch::Approx(0));
            REQUIRE(hr::Math::ftoi(-0.5) == Catch::Approx(-1));
            REQUIRE(hr::Math::ftoi(-0.6) == Catch::Approx(-1));

            REQUIRE(hr::Math::isZero(0.00001) == false);
            REQUIRE(hr::Math::isZero(0.0000000000001) == false);
            REQUIRE(hr::Math::isZero(std::numeric_limits<double>::epsilon()) == true);

            REQUIRE(hr::Math::iPrevPowerOfTwo(0) == 0);
            REQUIRE(hr::Math::iPrevPowerOfTwo(2) == 1);
            REQUIRE(hr::Math::iPrevPowerOfTwo(3) == 2);
            REQUIRE(hr::Math::iPrevPowerOfTwo(4) == 2);
            REQUIRE(hr::Math::iPrevPowerOfTwo(5) == 4);
            REQUIRE(hr::Math::iPrevPowerOfTwo(513) == 512);

            REQUIRE(hr::Math::iProxPowerOfTwo(0) == 0);
            REQUIRE(hr::Math::iProxPowerOfTwo(2) == 2);
            REQUIRE(hr::Math::iProxPowerOfTwo(3) == 4);
            REQUIRE(hr::Math::iProxPowerOfTwo(4) == 4);
            REQUIRE(hr::Math::iProxPowerOfTwo(5) == 8);
            REQUIRE(hr::Math::iProxPowerOfTwo(513) == 1024);

            REQUIRE(hr::Math::fClamp(0.5f, -1.0f, 1.0f) == Catch::Approx(0.5f));
            REQUIRE(hr::Math::fClamp(-2.5f, -1.0f, 1.0f) == Catch::Approx(-1.0f));
            REQUIRE(hr::Math::fClamp(-1.0f, -1.0f, 1.0f) == Catch::Approx(-1.0f));
            REQUIRE(hr::Math::fClamp(1.0f, -1.0f, 1.0f) == Catch::Approx(1.0f));
            REQUIRE(hr::Math::fClamp(1.3f, -1.0f, 1.0f) == Catch::Approx(1.0f));

            REQUIRE(hr::Math::fClamp(0.5, -1.0, 1.0) == Catch::Approx(0.5));
            REQUIRE(hr::Math::fClamp(-2.5, -1.0, 1.0) == Catch::Approx(-1.0));
            REQUIRE(hr::Math::fClamp(-1.0, -1.0, 1.0) == Catch::Approx(-1.0));
            REQUIRE(hr::Math::fClamp(1.0, -1.0, 1.0) == Catch::Approx(1.0));
            REQUIRE(hr::Math::fClamp(1.3, -1.0, 1.0) == Catch::Approx(1.0));

            REQUIRE(hr::Math::fAlmostEqual(1.3, 2.6) == false);
            REQUIRE(hr::Math::fAlmostEqual(1.3, 1.2) == false);
            REQUIRE(hr::Math::fAlmostEqual(1.3, 2.4) == false);
            REQUIRE(hr::Math::fAlmostEqual(1.33, 1.34) == false);
            REQUIRE(hr::Math::fAlmostEqual(1.333, 1.334) == false);
            REQUIRE(hr::Math::fAlmostEqual(1.3333, 1.3334) == false);
            REQUIRE(hr::Math::fAlmostEqual(1.33333, 1.33334) == false);
            REQUIRE(hr::Math::fAlmostEqual(1.333333, 1.333334) == false);
            REQUIRE(hr::Math::fAlmostEqual(1.3333333, 1.3333334) == true);
        }
    }
}
