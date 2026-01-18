#pragma once

#include <chrono>
#include <cstdint>
#include <type_traits>

namespace hr
{
	class Timer final
	{
        using ClockType = std::conditional<std::chrono::high_resolution_clock::is_steady && !std::is_same_v<std::chrono::high_resolution_clock, std::chrono::steady_clock>,
                                           std::chrono::high_resolution_clock,
                                           std::chrono::steady_clock>::type;

		ClockType::time_point mTimepoint;

	public:
		Timer() noexcept;
		constexpr Timer(const Timer&) = default;
		constexpr Timer& operator=(const Timer&) = default;
		constexpr Timer(Timer&&) = default;
		constexpr Timer& operator=(Timer&&) = default;

		void reStart() noexcept;

		std::chrono::milliseconds getTime() const noexcept;
		std::chrono::milliseconds getTime(const bool restart) noexcept;

		double getTimeS() const noexcept;
		double getTimeS(const bool restart) noexcept;
		double getTimeMS() const noexcept;
		double getTimeMS(const bool restart) noexcept;
		uint64_t getTimeIntS() const noexcept;
		uint64_t getTimeIntS(const bool restart) noexcept;
		uint64_t getTimeIntMS() const noexcept;
		uint64_t getTimeIntMS(const bool restart) noexcept;

		void setS(const uint64_t seconds) noexcept;
		void setS(const double seconds) noexcept;
		void setMS(const uint64_t miliseconds) noexcept;
		void setMS(const double miliseconds) noexcept;

		void addS(const int64_t seconds) noexcept;
		void addS(const double seconds) noexcept;
		void addMS(const int64_t miliseconds) noexcept;
		void addMS(const double miliseconds) noexcept;
	};
}
