#pragma once

#include "Types.hpp"

#include <chrono>

namespace HorseRadish
{
	class Timer
	{
		std::chrono::high_resolution_clock::time_point mTimepoint;

	public:
		Timer();

		void reStart();

		double getTimeS(const bool restart = false);
		double getTimeMS(const bool restart = false);
		hUInt64 getTimeIntS(const bool restart = false);
		hUInt64 getTimeIntMS(const bool restart = false);

		void setS(const hUInt64 seconds);
		void setS(const double seconds);
		void setMS(const hUInt64 miliseconds);
		void setMS(const double miliseconds);

		void addS(const hInt64 seconds);
		void addS(const double seconds);
		void addMS(const hInt64 miliseconds);
		void addMS(const double miliseconds);
	};

} //HorseRadish
