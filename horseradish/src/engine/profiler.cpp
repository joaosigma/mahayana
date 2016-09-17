#include "profiler.hpp"

#include <cassert>

namespace HorseRadish { namespace Engine {

#ifdef HR_PROFILLING

void Profiler::Recorder::dumpBuffer()
{
	if (mNumSamples == 0)
		return;

	HorseRadish::Streams::StreamWriter writer(mStream);

	for (unsigned int curSample = 0; curSample < mNumSamples; curSample++)
	{
		auto& sample = mBuffer[curSample];
		writer.write(sample.statId);
		writer.write(sample.sampleId);
		writer.write(sample.timestamp);
		writer.write(sample.value);
	}

	mNumSamples = 0;
}

Profiler::Recorder::Recorder(const char* const filePath)
	: mStream(filePath, false, true)
{}

Profiler::Recorder::~Recorder()
{
	dumpBuffer();
}

void Profiler::Recorder::storeSample(StatId statId, uint64 sampleId, std::chrono::high_resolution_clock::time_point timestamp, uint64 value)
{
	if (mNumSamples == mBuffer.size())
		dumpBuffer();

	assert(mNumSamples < mBuffer.size());

	auto& sample = mBuffer[mNumSamples];
	sample.sampleId = sampleId;
	sample.statId = statId;
	sample.timestamp = timestamp;
	sample.value = value;

	mNumSamples++;
}

void Profiler::nextSample()
{
	mSampleId++;

	std::lock_guard<std::mutex> lock(mMutex);

	auto elapsedTime = mTimer.getTimeMS();
	if (elapsedTime < Profiler::QuantizationPeriodMS)
		return;

	mTimer.reStart();

	double elapsedTimeScale = Profiler::QuantizationPeriodMS / elapsedTime;

	for (auto& it : mStats)
	{
		StatData& statData = it.second;

		double avg = (statData.quantization.numSamples == 0) ? 0.0 : (static_cast<double>(statData.quantization.sum) / static_cast<double>(statData.quantization.numSamples));
		avg *= elapsedTimeScale;

		statData.quantization.sum = 0;
		statData.quantization.numSamples = 0;

		statData.quantizedSamples.samples[statData.quantizedSamples.arrayHead] = avg;
		statData.quantizedSamples.arrayHead++;
		if (statData.quantizedSamples.arrayHead >= statData.quantizedSamples.samples.size())
		{
			statData.quantizedSamples.arrayHead = 0;
			statData.quantizedSamples.arrayFull = true;
		}
	}
}

void Profiler::addSample(StatId statId, uint64 value)
{
	auto sampleId = mSampleId.load();
	auto sampleTimestamp = std::chrono::high_resolution_clock::now();

	std::lock_guard<std::mutex> lock(mMutex);

	std::unordered_map<StatId, StatData>::iterator it = mStats.find(statId);
	if (it == mStats.end())
		return;

	StatData& statData = it->second;

	statData.globalInfo.numTotalSamples++;
	statData.globalInfo.lastValue = value;
	statData.globalInfo.maxValue = (value > statData.globalInfo.maxValue) ? value : statData.globalInfo.maxValue;
	statData.globalInfo.minValue = (value < statData.globalInfo.minValue) ? value : statData.globalInfo.minValue;

	statData.quantization.numSamples++;
	statData.quantization.sum += value;

	if (mRecorder)
		mRecorder->storeSample(statId, sampleId, sampleTimestamp, value);
}

void Profiler::recordingStart(const char* const filePath)
{
	if (filePath == nullptr || *filePath == '\0')
		return;

	std::lock_guard<std::mutex> lock(mMutex);

	if (mRecorder)
		recordingStop();

	assert(!mRecorder);
	mRecorder = std::make_unique<Profiler::Recorder>(filePath);
}

void Profiler::recordingStop()
{
	std::lock_guard<std::mutex> lock(mMutex);

	if (mRecorder)
		mRecorder.reset();
}

double Profiler::getLastQuantizedSample(StatId statId) const
{
	std::lock_guard<std::mutex> lock(mMutex);

	std::unordered_map<StatId, StatData>::const_iterator it = mStats.find(statId);
	if (it == mStats.end())
		return 0.0;

	const auto& statData = it->second;
	if ((statData.quantizedSamples.arrayHead == 0) && !statData.quantizedSamples.arrayFull)
		return 0.0;

	auto posRead = statData.quantizedSamples.arrayHead;
	posRead = (posRead == 0) ? (statData.quantizedSamples.samples.size() - 1) : (posRead - 1);

	return statData.quantizedSamples.samples[posRead];
}

size_t Profiler::getLastQuantizedSamples(StatId statId, bool normalizeValues, double* const outBuffer, const size_t maxItems) const
{
	if (!outBuffer || maxItems == 0)
		return 0;

	std::lock_guard<std::mutex> lock(mMutex);

	std::unordered_map<StatId, StatData>::const_iterator it = mStats.find(statId);
	if (it == mStats.end())
		return 0;

	const auto& statData = it->second;
	if ((statData.quantizedSamples.arrayHead == 0) && !statData.quantizedSamples.arrayFull)
		return 0;

	size_t sampledCount = 0;
	double sampleDist = 1.0 / static_cast<double>(statData.globalInfo.maxValue - statData.globalInfo.minValue);

	if (statData.quantizedSamples.arrayFull)
	{
		auto posRead = statData.quantizedSamples.arrayHead;

		if (normalizeValues)
		{
			do
			{
				posRead = (posRead == 0) ? (statData.quantizedSamples.samples.size() - 1) : (posRead - 1);
				outBuffer[sampledCount++] = (statData.quantizedSamples.samples[posRead] - statData.globalInfo.minValue) * sampleDist;

			} while ((sampledCount < maxItems) && (posRead != statData.quantizedSamples.arrayHead));
		}
		else
		{
			do
			{
				posRead = (posRead == 0) ? (statData.quantizedSamples.samples.size() - 1) : (posRead - 1);
				outBuffer[sampledCount++] = statData.quantizedSamples.samples[posRead];

			} while ((sampledCount < maxItems) && (posRead != statData.quantizedSamples.arrayHead));
		}
	}
	else
	{
		auto posRead = statData.quantizedSamples.arrayHead;

		if (normalizeValues)
		{
			do
			{
				outBuffer[sampledCount++] = (statData.quantizedSamples.samples[--posRead] - statData.globalInfo.minValue) * sampleDist;
			} while ((sampledCount < maxItems) && (posRead > 0));
		}
		else
		{
			do
			{
				outBuffer[sampledCount++] = statData.quantizedSamples.samples[--posRead];
			} while ((sampledCount < maxItems) && (posRead > 0));
		}
	}

	return sampledCount;
}

double Profiler::getLastQuantizedSamples(StatId statId, std::chrono::milliseconds samplingPeriod) const
{
	std::lock_guard<std::mutex> lock(mMutex);

	std::unordered_map<StatId, StatData>::const_iterator it = mStats.find(statId);
	if (it == mStats.end())
		return 0.0;

	const auto& statData = it->second;
	if ((statData.quantizedSamples.arrayHead == 0) && !statData.quantizedSamples.arrayFull)
		return 0.0;

	double sampleSum = 0.0;
	double sampleTime = 0.0f;
	double sampleTargetTime = static_cast<double>(samplingPeriod.count());
	size_t sampledCount = 0;

	if (statData.quantizedSamples.arrayFull)
	{
		auto posRead = statData.quantizedSamples.arrayHead;
		do
		{
			posRead = (posRead == 0) ? (statData.quantizedSamples.samples.size() - 1) : (posRead - 1);

			sampleSum += statData.quantizedSamples.samples[posRead];
			sampleTime += Profiler::QuantizationPeriodMS;
			sampledCount++;

		} while ((sampleTime < sampleTargetTime) && (posRead != statData.quantizedSamples.arrayHead));
	}
	else
	{
		auto posRead = statData.quantizedSamples.arrayHead;
		do
		{
			sampleSum += statData.quantizedSamples.samples[--posRead];
			sampleTime += Profiler::QuantizationPeriodMS;
			sampledCount++;

		} while ((sampleTime < sampleTargetTime) && (posRead > 0));
	}

	auto value = sampleSum / static_cast<double>(sampledCount);
	return (value * sampleTargetTime / sampleTime);
}

#else

#endif

} }