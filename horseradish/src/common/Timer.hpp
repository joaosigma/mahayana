#pragma once
#ifndef __HTIMER__
#define __HTIMER__

#include "Types.hpp"

#include <chrono>

namespace HorseRadish
{
	class Timer
	{
		std::chrono::high_resolution_clock::time_point timepoint;

	public:
		Timer();

		void ReStart();

		double GetTimeS(const bool restart = false);
		double GetTimeMS(const bool restart = false);
		hUInt64 GetTimeIntS(const bool restart = false);
		hUInt64 GetTimeIntMS(const bool restart = false);

		void SetS(const hUInt64 seconds);
		void SetS(const double seconds);
		void SetMS(const hUInt64 miliseconds);
		void SetMS(const double miliseconds);

		void AddS(const hInt64 seconds);
		void AddS(const double seconds);
		void AddMS(const hInt64 miliseconds);
		void AddMS(const double miliseconds);
	};

} //namespace HorseRadish

#endif