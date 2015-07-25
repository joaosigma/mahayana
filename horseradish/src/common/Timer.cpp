#include "Timer.hpp"

namespace HorseRadish
{
	Timer::Timer()
	{
		reStart();
	}

	void Timer::reStart()
	{
		mTimepoint = std::chrono::high_resolution_clock::now();
	}

	double Timer::getTimeS(const bool restart)
	{
		auto curTime = std::chrono::high_resolution_clock::now();
		auto elapsed = std::chrono::duration<double, std::chrono::seconds::period>(curTime - mTimepoint).count();

		if (restart)
			mTimepoint = curTime;

		return elapsed;
	}

	double Timer::getTimeMS(const bool restart)
	{
		auto curTime = std::chrono::high_resolution_clock::now();
		auto elapsed = std::chrono::duration<double, std::chrono::milliseconds::period>(curTime - mTimepoint).count();

		if (restart)
			mTimepoint = curTime;

		return elapsed;
	}

	hUInt64 Timer::getTimeIntS(const bool restart)
	{
		auto curTime = std::chrono::high_resolution_clock::now();
		auto elapsed = std::chrono::duration_cast<std::chrono::seconds>(curTime - mTimepoint).count();

		if (restart)
			mTimepoint = curTime;

		return static_cast<hUInt64>(elapsed);
	}

	hUInt64 Timer::getTimeIntMS(const bool restart)
	{
		auto curTime = std::chrono::high_resolution_clock::now();
		auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(curTime - mTimepoint).count();

		if (restart)
			mTimepoint = curTime;

		return static_cast<hUInt64>(elapsed);
	}

	void Timer::setS(const hUInt64 seconds)
	{
		mTimepoint = std::chrono::high_resolution_clock::now();
		mTimepoint -= std::chrono::seconds(seconds);
	}

	void Timer::setS(const double seconds)
	{
		mTimepoint = std::chrono::high_resolution_clock::now();
		mTimepoint -= std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::duration<double, std::chrono::seconds::period>(seconds));
	}

	void Timer::setMS(const hUInt64 miliseconds)
	{
		mTimepoint = std::chrono::high_resolution_clock::now();
		mTimepoint -= std::chrono::milliseconds(miliseconds);
	}

	void Timer::setMS(const double miliseconds)
	{
		mTimepoint = std::chrono::high_resolution_clock::now();
		mTimepoint -= std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::duration<double, std::chrono::milliseconds::period>(miliseconds));
	}

	void Timer::addS(const hInt64 seconds)
	{
		mTimepoint += std::chrono::seconds(seconds);
	}

	void Timer::addS(const double seconds)
	{
		mTimepoint += std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::duration<double, std::chrono::seconds::period>(seconds));
	}

	void Timer::addMS(const hInt64 miliseconds)
	{
		mTimepoint += std::chrono::milliseconds(miliseconds);
	}

	void Timer::addMS(const double miliseconds)
	{
		mTimepoint += std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::duration<double, std::chrono::milliseconds::period>(miliseconds));
	}

} //HorseRadish