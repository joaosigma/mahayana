#include "Timer.hpp"

namespace HorseRadish
{
	Timer::Timer()
	{
		this->ReStart();
	}

	void Timer::ReStart()
	{
		this->timepoint = std::chrono::high_resolution_clock::now();
	}

	double Timer::GetTimeS(const bool restart)
	{
		auto curTime = std::chrono::high_resolution_clock::now();
		auto elapsed = std::chrono::duration<double, std::chrono::seconds::period>(curTime - this->timepoint).count();

		if (restart)
			this->timepoint = curTime;

		return elapsed;
	}

	double Timer::GetTimeMS(const bool restart)
	{
		auto curTime = std::chrono::high_resolution_clock::now();
		auto elapsed = std::chrono::duration<double, std::chrono::milliseconds::period>(curTime - this->timepoint).count();

		if (restart)
			this->timepoint = curTime;

		return elapsed;
	}

	hUInt64 Timer::GetTimeIntS(const bool restart)
	{
		auto curTime = std::chrono::high_resolution_clock::now();
		auto elapsed = std::chrono::duration_cast<std::chrono::seconds>(curTime - this->timepoint).count();

		if (restart)
			this->timepoint = curTime;

		return static_cast<hUInt64>(elapsed);
	}

	hUInt64 Timer::GetTimeIntMS(const bool restart)
	{
		auto curTime = std::chrono::high_resolution_clock::now();
		auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(curTime - this->timepoint).count();

		if (restart)
			this->timepoint = curTime;

		return static_cast<hUInt64>(elapsed);
	}

	void Timer::SetS(const hUInt64 seconds)
	{
		this->timepoint = std::chrono::high_resolution_clock::now();
		this->timepoint -= std::chrono::seconds(seconds);
	}

	void Timer::SetS(const double seconds)
	{
		this->timepoint = std::chrono::high_resolution_clock::now();
		this->timepoint -= std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::duration<double, std::chrono::seconds::period>(seconds));
	}

	void Timer::SetMS(const hUInt64 miliseconds)
	{
		this->timepoint = std::chrono::high_resolution_clock::now();
		this->timepoint -= std::chrono::milliseconds(miliseconds);
	}

	void Timer::SetMS(const double miliseconds)
	{
		this->timepoint = std::chrono::high_resolution_clock::now();
		this->timepoint -= std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::duration<double, std::chrono::milliseconds::period>(miliseconds));
	}

	void Timer::AddS(const hInt64 seconds)
	{
		this->timepoint += std::chrono::seconds(seconds);
	}

	void Timer::AddS(const double seconds)
	{
		this->timepoint += std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::duration<double, std::chrono::seconds::period>(seconds));
	}

	void Timer::AddMS(const hInt64 miliseconds)
	{
		this->timepoint += std::chrono::milliseconds(miliseconds);
	}

	void Timer::AddMS(const double miliseconds)
	{
		this->timepoint += std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::duration<double, std::chrono::milliseconds::period>(miliseconds));
	}

} //namespace HorseRadish