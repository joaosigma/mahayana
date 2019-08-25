#pragma once

#include "types.hpp"

#include <chrono>

namespace hr
{
	class Timer final
	{
		std::chrono::high_resolution_clock::time_point mTimepoint;

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
		hUInt64 getTimeIntS() const noexcept;
		hUInt64 getTimeIntS(const bool restart) noexcept;
		hUInt64 getTimeIntMS() const noexcept;
		hUInt64 getTimeIntMS(const bool restart) noexcept;

		void setS(const hUInt64 seconds) noexcept;
		void setS(const double seconds) noexcept;
		void setMS(const hUInt64 miliseconds) noexcept;
		void setMS(const double miliseconds) noexcept;

		void addS(const hInt64 seconds) noexcept;
		void addS(const double seconds) noexcept;
		void addMS(const hInt64 miliseconds) noexcept;
		void addMS(const double miliseconds) noexcept;
	};
}
