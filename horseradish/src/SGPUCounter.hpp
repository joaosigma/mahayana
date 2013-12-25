#pragma once
#ifndef __SGPU_COUNTER__
#define __SGPU_COUNTER__

#include "common\common.hpp"
#include "common\containers.hpp"

//§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§
//§§§§   -= Classe SGPUCounter =-	§§§§
//§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§
class SGPUCounter
{
	struct CounterData{
		HorseRadish::String name;
		float *values, minValue, maxValue;
		unsigned int index;
		bool isPercent;
		float color[4];
	};
	unsigned int numMaxSamples;
	mutable unsigned int curSample;
	HorseRadish::Containers::Array<CounterData> arrayCounters;

public:
	SGPUCounter(unsigned int maxSaveSamples);
	~SGPUCounter();

	bool addCounter(const char * const counterName);
	bool addCounter(const char * const counterName, bool isPercent);
	bool addCounter(const char * const counterName, const float r, const float g, const float b);
	bool addCounter(const char * const counterName, bool isPercent, const float r, const float g, const float b);
	bool addGPUCounter(const char * const counterName);
	bool addGPUCounter(const char * const counterName, const float r, const float g, const float b);
	void reset();
	void sampleCounter(const char * const counterName, const float &newSample) const;
	void sampleMoveNext() const;

	const char* getCounterName(const unsigned int counterNumber) const;
	unsigned int getCounterNumber(const char * const counterName) const;
	bool getCounterIsPercent(const unsigned int counterNumber) const;
	const float* getCounterColor(const unsigned int counterNumber) const;
	int getNumCounters() const;
	int getMaxNumSamples() const;
	int getCurrentSample() const;

	float getValue(const unsigned int counterNumber, const unsigned int sample) const;
	float getValueCurrent(const unsigned int counterNumber) const;
	float getValueAvg(const unsigned int counterNumber) const;
	void getValuesMinMax(const unsigned int counterNumber, float * const valueMin, float * const valueMax) const;
};

#endif