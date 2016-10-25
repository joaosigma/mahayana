#pragma once

#include "types.hpp"

#include <chrono>

namespace HorseRadish
{
	class Timer
	{
		std::chrono::high_resolution_clock::time_point mTimepoint;

	public:
		Timer();

		void reStart();

		std::chrono::milliseconds getTime() const;

		double getTimeS() const;
		double getTimeS(const bool restart);
		double getTimeMS() const;
		double getTimeMS(const bool restart);
		hUInt64 getTimeIntS() const;
		hUInt64 getTimeIntS(const bool restart);
		hUInt64 getTimeIntMS() const;
		hUInt64 getTimeIntMS(const bool restart);

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
