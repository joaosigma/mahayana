#include "SGPUCounter.hpp"

#include "common\platform.hpp"

#include <limits>

SGPUCounter::SGPUCounter(unsigned int maxSaveSamples)
	: numMaxSamples(maxSaveSamples), curSample(0)
{
	if (numMaxSamples == 0)
		numMaxSamples = 1;
}

SGPUCounter::~SGPUCounter()
{	reset();}

bool SGPUCounter::addCounter(const std::string& counterName)
{	return addCounter(counterName, false, 1.0f, 1.0f, 1.0f);}

bool SGPUCounter::addCounter(const std::string& counterName, bool isPercent)
{	return addCounter(counterName, isPercent, 1.0f, 1.0f, 1.0f);}

bool SGPUCounter::addCounter(const std::string& counterName, const float r, const float g, const float b)
{	return addCounter(counterName, false, 1.0f, 1.0f, 1.0f);}

bool SGPUCounter::addCounter(const std::string& counterName, bool isPercent, const float r, const float g, const float b)
{
	if (counterName.empty())
		return false;

	auto valuesArray = new float[numMaxSamples];

	this->arrayCounters.push_back(CounterData());
	auto& newCounter = this->arrayCounters[this->arrayCounters.size() - 1];

	newCounter.isPercent = isPercent;
	newCounter.index = 0;
	newCounter.name = counterName;
	newCounter.values = valuesArray;
	newCounter.minValue = std::numeric_limits<float>::infinity();
	newCounter.maxValue = -std::numeric_limits<float>::infinity();
	newCounter.color[0] = r;
	newCounter.color[1] = g;
	newCounter.color[2] = b;
	newCounter.color[3] = 1.0f;
	memset(newCounter.values, 0, sizeof(float)*numMaxSamples);
	return true;
}

void SGPUCounter::reset()
{
	for (auto &counter : this->arrayCounters)
		delete[] counter.values;
	this->arrayCounters.clear();
	
	this->curSample = 0;
}

void SGPUCounter::sampleCounter(const std::string& counterName, const float &newSample)
{
	if (counterName.empty())
		return;

	for (auto &counter : this->arrayCounters)
	{
		if (counter.name != counterName)
			continue;

		counter.values[curSample] = newSample;
		counter.minValue = std::fmin(counter.minValue, counter.values[curSample]);
		counter.maxValue = std::fmax(counter.maxValue, counter.values[curSample]);
		break;
	}
}

void SGPUCounter::sampleMoveNext()
{
	this->curSample++;

	if (this->curSample >= this->numMaxSamples)
	{
		this->curSample = 0;

		for (auto &counter : this->arrayCounters)
		{
			counter.minValue = std::numeric_limits<float>::infinity();
			counter.maxValue = -std::numeric_limits<float>::infinity();
		}
	}
}

std::string SGPUCounter::getCounterName(const unsigned int counterNumber) const
{
	if (counterNumber==0 || counterNumber>this->arrayCounters.size())
		return nullptr;

	return this->arrayCounters[counterNumber-1].name;
}

unsigned int SGPUCounter::getCounterNumber(const std::string& counterName) const
{
	if (counterName.empty())
		return 0;

	for(unsigned int i = 0; i < this->arrayCounters.size(); i++)
	{
		if (this->arrayCounters[i].name == counterName)
			return (i+1);
	}

	return 0;
}

bool SGPUCounter::getCounterIsPercent(const unsigned int counterNumber) const
{
	if (counterNumber==0 || counterNumber>this->arrayCounters.size())
		return nullptr;

	return this->arrayCounters[counterNumber-1].isPercent;
}

const float* SGPUCounter::getCounterColor(const unsigned int counterNumber) const
{
	if (counterNumber == 0 || counterNumber>this->arrayCounters.size())
		return nullptr;

	return this->arrayCounters[counterNumber-1].color;
}

int SGPUCounter::getNumCounters() const
{	return this->arrayCounters.size();}

int SGPUCounter::getMaxNumSamples() const
{	return numMaxSamples;}

int SGPUCounter::getCurrentSample() const
{	return curSample;}

float SGPUCounter::getValue(const unsigned int counterNumber, const unsigned int sample) const
{
	if (counterNumber == 0 || counterNumber>this->arrayCounters.size() || sample == 0 || sample>numMaxSamples)
		return (0.0f);

	return this->arrayCounters[counterNumber-1].values[sample-1];
}

float SGPUCounter::getValueCurrent(const unsigned int counterNumber) const
{
	if (counterNumber == 0 || counterNumber>this->arrayCounters.size())
		return (0.0f);

	if (curSample>0)
		return this->arrayCounters[counterNumber-1].values[curSample-1];
	return this->arrayCounters[counterNumber-1].values[0];
}

float SGPUCounter::getValueAvg(const unsigned int counterNumber) const
{
	if (counterNumber==0 || counterNumber>this->arrayCounters.size())
		return (0.0f);

	float numSamplesInv = 1.0f/((float)curSample);
	float sampleVal = 0.0f;
	for(unsigned int i=0; i<curSample; i++)
		sampleVal += this->arrayCounters[counterNumber-1].values[i]*numSamplesInv;

	return sampleVal;
}

void SGPUCounter::getValuesMinMax(const unsigned int counterNumber, float * const valueMin, float * const valueMax) const
{
	if (counterNumber == 0 || counterNumber>this->arrayCounters.size())
		return;

	if (valueMin != nullptr)
		*valueMin = this->arrayCounters[counterNumber-1].minValue;
	if (valueMax != nullptr)
		*valueMax = this->arrayCounters[counterNumber-1].maxValue;
}