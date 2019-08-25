#include "timer.hpp"

namespace hr
{
	Timer::Timer() noexcept
	{
		reStart();
	}

	void Timer::reStart() noexcept
	{
		mTimepoint = std::chrono::high_resolution_clock::now();
	}

	std::chrono::milliseconds Timer::getTime() const noexcept
	{
		return std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::high_resolution_clock::now() - mTimepoint);
	}

	std::chrono::milliseconds Timer::getTime(const bool restart) noexcept
	{
		auto curTime = std::chrono::high_resolution_clock::now();
		auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(curTime - mTimepoint);

		if (restart)
			mTimepoint = curTime;

		return elapsed;
	}

	double Timer::getTimeS() const noexcept
	{
		auto curTime = std::chrono::high_resolution_clock::now();
		auto elapsed = std::chrono::duration<double, std::chrono::seconds::period>(curTime - mTimepoint).count();

		return elapsed;
	}

	double Timer::getTimeS(const bool restart) noexcept
	{
		auto curTime = std::chrono::high_resolution_clock::now();
		auto elapsed = std::chrono::duration<double, std::chrono::seconds::period>(curTime - mTimepoint).count();

		if (restart)
			mTimepoint = curTime;

		return elapsed;
	}

	double Timer::getTimeMS() const noexcept
	{
		auto curTime = std::chrono::high_resolution_clock::now();
		auto elapsed = std::chrono::duration<double, std::chrono::milliseconds::period>(curTime - mTimepoint).count();

		return elapsed;
	}

	double Timer::getTimeMS(const bool restart) noexcept
	{
		auto curTime = std::chrono::high_resolution_clock::now();
		auto elapsed = std::chrono::duration<double, std::chrono::milliseconds::period>(curTime - mTimepoint).count();

		if (restart)
			mTimepoint = curTime;

		return elapsed;
	}

	hUInt64 Timer::getTimeIntS() const noexcept
	{
		auto curTime = std::chrono::high_resolution_clock::now();
		auto elapsed = std::chrono::duration_cast<std::chrono::seconds>(curTime - mTimepoint).count();

		return static_cast<hUInt64>(elapsed);
	}

	hUInt64 Timer::getTimeIntS(const bool restart) noexcept
	{
		auto curTime = std::chrono::high_resolution_clock::now();
		auto elapsed = std::chrono::duration_cast<std::chrono::seconds>(curTime - mTimepoint).count();

		if (restart)
			mTimepoint = curTime;

		return static_cast<hUInt64>(elapsed);
	}

	hUInt64 Timer::getTimeIntMS() const noexcept
	{
		auto curTime = std::chrono::high_resolution_clock::now();
		auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(curTime - mTimepoint).count();

		return static_cast<hUInt64>(elapsed);
	}

	hUInt64 Timer::getTimeIntMS(const bool restart) noexcept
	{
		auto curTime = std::chrono::high_resolution_clock::now();
		auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(curTime - mTimepoint).count();

		if (restart)
			mTimepoint = curTime;

		return static_cast<hUInt64>(elapsed);
	}

	void Timer::setS(const hUInt64 seconds) noexcept
	{
		mTimepoint = std::chrono::high_resolution_clock::now();
		mTimepoint -= std::chrono::seconds(seconds);
	}

	void Timer::setS(const double seconds) noexcept
	{
		mTimepoint = std::chrono::high_resolution_clock::now();
		mTimepoint -= std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::duration<double, std::chrono::seconds::period>(seconds));
	}

	void Timer::setMS(const hUInt64 miliseconds) noexcept
	{
		mTimepoint = std::chrono::high_resolution_clock::now();
		mTimepoint -= std::chrono::milliseconds(miliseconds);
	}

	void Timer::setMS(const double miliseconds) noexcept
	{
		mTimepoint = std::chrono::high_resolution_clock::now();
		mTimepoint -= std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::duration<double, std::chrono::milliseconds::period>(miliseconds));
	}

	void Timer::addS(const hInt64 seconds) noexcept
	{
		mTimepoint += std::chrono::seconds(seconds);
	}

	void Timer::addS(const double seconds) noexcept
	{
		mTimepoint += std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::duration<double, std::chrono::seconds::period>(seconds));
	}

	void Timer::addMS(const hInt64 miliseconds) noexcept
	{
		mTimepoint += std::chrono::milliseconds(miliseconds);
	}

	void Timer::addMS(const double miliseconds) noexcept
	{
		mTimepoint += std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::duration<double, std::chrono::milliseconds::period>(miliseconds));
	}
}