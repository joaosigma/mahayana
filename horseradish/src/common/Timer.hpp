#pragma once
#ifndef __HTIMER__
#define __HTIMER__

#include "Types.hpp"
#include "Platform.hpp"
#include "Vector.hpp"
#include <windows.h>

namespace HorseRadish
{

class Timer
{
	static LARGE_INTEGER freqTimerS;
	static LARGE_INTEGER freqTimerMS;
	static double invFreqS;
	static double invFreqMS;
	static bool isInitialized;

	LARGE_INTEGER qptStartTime;

public:
	Timer();

	void ReStart();

	double GetTimeS(const bool reStart = false);
	double GetTimeMS(const bool reStart = false);
	hUInt32 GetTimeIntS(const bool reStart = false);
	hUInt32 GetTimeIntS(hUInt64 &seconds, const bool reStart = false);
	hUInt32 GetTimeIntMS(const bool reStart = false);
	hUInt32 GetTimeIntMS(hUInt64 &miliseconds, const bool reStart = false);

	void SetS(const hUInt32 seconds);
	void SetS(const hUInt64 seconds);
	void SetS(const double seconds);
	void SetMS(const hUInt32 miliseconds);
	void SetMS(const hUInt64 miliseconds);
	void SetMS(const double miliseconds);

	void AddS(const hInt32 seconds);
	void AddS(const hInt64 seconds);
	void AddS(const double seconds);
	void AddMS(const hInt32 miliseconds);
	void AddMS(const hInt64 miliseconds);
	void AddMS(const double miliseconds);
};

}//namespace HorseRadish

#endif