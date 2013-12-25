#include "Timer.hpp"

#include <memory.h>

namespace HorseRadish
{

Timer::Timer()
{
	//se ainda não inicializei isto
	if (Timer::isInitialized == false)
	{
		//as frequencias
		QueryPerformanceFrequency(&Timer::freqTimerS);
		Timer::freqTimerMS.QuadPart = Timer::freqTimerS.QuadPart / 1000;

		//e os inversos delas
		Timer::invFreqS = 1.0 / ((double)freqTimerS.QuadPart);
		Timer::invFreqMS = 1.0 / ((double)freqTimerMS.QuadPart);

		//já iniciei
		Timer::isInitialized = true;
	}

	//posso começar a contagem
	this->ReStart();
}

void Timer::ReStart()
{
	QueryPerformanceCounter(&this->qptStartTime);
}

double Timer::GetTimeS(const bool reStart)
{
	LARGE_INTEGER qptQuery;

	//how much time has passed
	QueryPerformanceCounter(&qptQuery);
	auto timePassed = ((double)(qptQuery.QuadPart - this->qptStartTime.QuadPart)) * Timer::invFreqS;

	//restart de timer is requested and return calculated time
	if (reStart == true)
		this->qptStartTime.QuadPart = qptQuery.QuadPart;
	return timePassed;
}

double Timer::GetTimeMS(const bool reStart)
{
	LARGE_INTEGER qptQuery;

	//how much time has passed
	QueryPerformanceCounter(&qptQuery);
	auto timePassed = ((double)(qptQuery.QuadPart - this->qptStartTime.QuadPart)) * Timer::invFreqMS;

	//restart de timer is requested and return calculated time
	if (reStart == true)
		this->qptStartTime.QuadPart = qptQuery.QuadPart;
	return timePassed;
}

hUInt32 Timer::GetTimeIntS(const bool reStart)
{
	LARGE_INTEGER qptQuery;

	//how much time has passed
	QueryPerformanceCounter(&qptQuery);
	auto timePassed = (qptQuery.QuadPart - this->qptStartTime.QuadPart) / Timer::freqTimerS.QuadPart;

	//restart de timer is requested and return calculated time
	if (reStart == true)
		this->qptStartTime.QuadPart = qptQuery.QuadPart;
	return timePassed;
}

hUInt32 Timer::GetTimeIntS(hUInt64 &seconds, const bool reStart)
{
	LARGE_INTEGER qptQuery;

	//how much time has passed
	QueryPerformanceCounter(&qptQuery);
	seconds = (qptQuery.QuadPart - this->qptStartTime.QuadPart) / Timer::freqTimerS.QuadPart;
	
	//restart de timer is requested and return calculated time
	if (reStart == true)
		this->qptStartTime.QuadPart = qptQuery.QuadPart;
	return ((hUInt32)seconds);
}

hUInt32 Timer::GetTimeIntMS(const bool reStart)
{
	LARGE_INTEGER qptQuery;

	//how much time has passed
	QueryPerformanceCounter(&qptQuery);
	auto timePassed = (qptQuery.QuadPart - this->qptStartTime.QuadPart) / Timer::freqTimerMS.QuadPart;

	//restart de timer is requested and return calculated time
	if (reStart == true)
		this->qptStartTime.QuadPart = qptQuery.QuadPart;
	return timePassed;
}

hUInt32 Timer::GetTimeIntMS(hUInt64 &miliseconds, const bool reStart)
{
	LARGE_INTEGER qptQuery;

	//how much time has passed
	QueryPerformanceCounter(&qptQuery);
	miliseconds = (qptQuery.QuadPart - this->qptStartTime.QuadPart) / Timer::freqTimerMS.QuadPart;

	//restart de timer is requested and return calculated time
	if (reStart == true)
		this->qptStartTime.QuadPart = qptQuery.QuadPart;
	return ((hUInt32)miliseconds);
}

void Timer::SetS(const hUInt32 seconds)
{
	QueryPerformanceCounter(&this->qptStartTime);
	this->qptStartTime.QuadPart -= (Timer::freqTimerS.QuadPart * seconds);
}

void Timer::SetS(const hUInt64 seconds)
{
	QueryPerformanceCounter(&this->qptStartTime);
	this->qptStartTime.QuadPart -= (Timer::freqTimerS.QuadPart * seconds);
}

void Timer::SetS(const double seconds)
{
	QueryPerformanceCounter(&this->qptStartTime);
	this->qptStartTime.QuadPart -= (Timer::freqTimerS.QuadPart * seconds);
}

void Timer::SetMS(const hUInt32 miliseconds)
{
	QueryPerformanceCounter(&this->qptStartTime);
	this->qptStartTime.QuadPart -= (Timer::freqTimerMS.QuadPart * miliseconds);
}

void Timer::SetMS(const hUInt64 miliseconds)
{
	QueryPerformanceCounter(&this->qptStartTime);
	this->qptStartTime.QuadPart -= (Timer::freqTimerMS.QuadPart * miliseconds);
}

void Timer::SetMS(const double miliseconds)
{
	QueryPerformanceCounter(&this->qptStartTime);
	this->qptStartTime.QuadPart -= (Timer::freqTimerMS.QuadPart * miliseconds);
}

void Timer::AddS(const hInt32 seconds)
{
	this->qptStartTime.QuadPart -= (Timer::freqTimerS.QuadPart * seconds);
}

void Timer::AddS(const hInt64 seconds)
{
	this->qptStartTime.QuadPart -= (Timer::freqTimerS.QuadPart * seconds);
}

void Timer::AddS(const double seconds)
{
	this->qptStartTime.QuadPart -= (Timer::freqTimerS.QuadPart * seconds);
}

void Timer::AddMS(const hInt32 miliseconds)
{
	this->qptStartTime.QuadPart -= (Timer::freqTimerMS.QuadPart * miliseconds);
}

void Timer::AddMS(const hInt64 miliseconds)
{
	this->qptStartTime.QuadPart -= (Timer::freqTimerMS.QuadPart * miliseconds);
}

void Timer::AddMS(const double miliseconds)
{
	this->qptStartTime.QuadPart -= (Timer::freqTimerMS.QuadPart * miliseconds);
}

LARGE_INTEGER Timer::freqTimerS = {0};
LARGE_INTEGER Timer::freqTimerMS = {0};
double Timer::invFreqS = 0.0;
double Timer::invFreqMS = 0.0;
bool Timer::isInitialized = false;

}//namespace HorseRadish