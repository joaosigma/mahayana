#pragma once

#include "common\Math.hpp"

#include <vector>

class SGPUCounter
{
	struct CounterData{
		std::string name;
		float *values, minValue, maxValue;
		unsigned int index;
		bool isPercent;
		float color[4];
	};
	unsigned int numMaxSamples;
	mutable unsigned int curSample;
	std::vector<CounterData> arrayCounters;

public:
	SGPUCounter(unsigned int maxSaveSamples);
	~SGPUCounter();

	bool addCounter(const std::string& counterName);
	bool addCounter(const std::string& counterName, bool isPercent);
	bool addCounter(const std::string& counterName, const float r, const float g, const float b);
	bool addCounter(const std::string& counterName, bool isPercent, const float r, const float g, const float b);
	bool addGPUCounter(const std::string& counterName);
	bool addGPUCounter(const std::string& counterName, const float r, const float g, const float b);
	void reset();
	void sampleCounter(const std::string& counterName, const float &newSample);
	void sampleMoveNext();

	std::string getCounterName(const unsigned int counterNumber) const;
	unsigned int getCounterNumber(const std::string& counterName) const;
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
