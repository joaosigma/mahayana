#pragma once

#include "common/timer.hpp"
#include "common/stream.hpp"

#include <array>
#include <mutex>
#include <atomic>
#include <chrono>
#include <cstdint>
#include <unordered_map>
#include <unordered_set>

namespace hr { namespace engine
{
#ifdef HR_PROFILLING

	class Profiler
	{
	public:
		typedef uint_fast64_t uint64;

		static constexpr double QuantizationPeriodMS = 250.0;
		static constexpr size_t QuantizationSamples = 240;

		enum class StatGroup : unsigned int
		{
			Frame = 0x1,
			GPU = 0x2,
			Mem = 0x3
		};

		enum class StatId : unsigned int
		{
			FrameTotal = (static_cast<unsigned int>(StatGroup::Frame) << 16) | 0x1,
			FrameLogic = (static_cast<unsigned int>(StatGroup::Frame) << 16) | 0x2,
			FrameDraw = (static_cast<unsigned int>(StatGroup::Frame) << 16) | 0x3,
			FrameGPU = (static_cast<unsigned int>(StatGroup::Frame) << 16) | 0x4,

			GPUTimeElapsed = (static_cast<unsigned int>(StatGroup::GPU) << 16) | 0x1,
			GPUSamples = (static_cast<unsigned int>(StatGroup::GPU) << 16) | 0x2,
			GPUVerticesSubmitted = (static_cast<unsigned int>(StatGroup::GPU) << 16) | 0x3,
			GPUPrimitivesSubmitted = (static_cast<unsigned int>(StatGroup::GPU) << 16) | 0x4,
			GPUVertexShaderInvocations = (static_cast<unsigned int>(StatGroup::GPU) << 16) | 0x5,
			GPUFragmentShaderInvocations = (static_cast<unsigned int>(StatGroup::GPU) << 16) | 0x6,
			GPUClipInputPrimitives = (static_cast<unsigned int>(StatGroup::GPU) << 16) | 0x7,
			GPUClipOutputPrimitives = (static_cast<unsigned int>(StatGroup::GPU) << 16) | 0x8,

			MemTotal = (static_cast<unsigned int>(StatGroup::Mem) << 16) | 0x1,
			MemScripts = (static_cast<unsigned int>(StatGroup::Mem) << 16) | 0x2,
			MemObjects = (static_cast<unsigned int>(StatGroup::Mem) << 16) | 0x3,
			MemAnimations = (static_cast<unsigned int>(StatGroup::Mem) << 16) | 0x4,
		};

		static constexpr bool isSupported()
		{
			return true;
		}

		static const char* translateStatId(StatId statId)
		{
			switch(statId)
			{
			case StatId::FrameTotal:
				return "frame.total";
			case StatId::FrameLogic:
				return "frame.logic";
			case StatId::FrameDraw:
				return "frame.draw";
			case StatId::FrameGPU:
				return "frame.gpu";
			case StatId::GPUTimeElapsed:
				return "gpu.timeElapsed";
			case StatId::GPUSamples:
				return "gpu.Samples";
			case StatId::GPUVerticesSubmitted:
				return "gpu.verticesSubmitted";
			case StatId::GPUPrimitivesSubmitted:
				return "gpu.primitivesSubmitted";
			case StatId::GPUVertexShaderInvocations:
				return "gpu.vertexShaderInvocations";
			case StatId::GPUFragmentShaderInvocations:
				return "gpu.fragmentShaderInvocations";
			case StatId::GPUClipInputPrimitives:
				return "gpu.clipInputPrimitives";
			case StatId::GPUClipOutputPrimitives:
				return "gpu.clipOutputPrimitives";
			case StatId::MemTotal:
				return "mem.total";
			case StatId::MemScripts:
				return "mem.scripts";
			case StatId::MemObjects:
				return "mem.objects";
			case StatId::MemAnimations:
				return "mem.animations";
			}

			return "unknown stat";
		}

	private:

		class Recorder
		{
			static constexpr size_t BufferSize = 400;
			static_assert(Recorder::BufferSize >= 1, "Buffer size must be at least 1");

			struct SampleInfo {
				StatId statId;
				uint64 sampleId, value;
				std::chrono::high_resolution_clock::time_point timestamp;
			};

			size_t mNumSamples = 0;
			hr::streams::FileStream mStream;
			std::array<SampleInfo, Recorder::BufferSize> mBuffer;

			void dumpBuffer();

		public:
			Recorder(const char* const filePath);
			~Recorder();

			void storeSample(StatId statId, uint64 sampleId, std::chrono::high_resolution_clock::time_point timestamp, uint64 value);
		};

		struct Sample
		{
			uint64 sampleId = 0, value = 0;
			std::chrono::high_resolution_clock::time_point timestamp;

			Sample() = default;

			Sample(uint64 sampleId, uint64 value, std::chrono::high_resolution_clock::time_point timestamp)
				: sampleId(sampleId), value(value), timestamp(timestamp)
			{ }
		};

		struct StatData
		{
			struct {
				uint64 numTotalSamples = 0;
				uint64 lastValue = 0, maxValue = 0, minValue = 0;
			} globalInfo;

			struct {
				uint64 sum = 0;
				size_t numSamples = 0;
			} quantization;

			struct {
				bool arrayFull = false;
				size_t arrayHead = 0;
				std::array<double, Profiler::QuantizationSamples> samples;
			} quantizedSamples;
					
			StatData() = default;
		};

		static constexpr StatGroup extractGroup(StatId statId)
		{
			return static_cast<StatGroup>(static_cast<unsigned int>(statId) >> 16);
		}

		mutable std::mutex mMutex;

		hr::Timer mTimer;
		std::unique_ptr<Recorder> mRecorder;
		std::atomic<unsigned int> mSampleId = 0;
		std::unordered_set<StatGroup> mStatGroups;
		std::unordered_map<StatId, StatData> mStats;

	public:
		Profiler()
		{ }

		void enableStat(StatId statId)
		{
			std::lock_guard<std::mutex> lock(mMutex);

			if (mStats.find(statId) != mStats.end())
				return;

			mStats[statId];
			mStatGroups.insert(Profiler::extractGroup(statId));
		}

		template<typename... Targs>
		void enableStat(StatId statId, Targs... statIds)
		{
			enableStat(statId);
			enableStat(statIds...);
		}

		void disableStat(StatId statId)
		{
			std::lock_guard<std::mutex> lock(mMutex);

			if (mStats.find(statId) == mStats.end())
				return;

			mStats.erase(statId);
			mStatGroups.erase(Profiler::extractGroup(statId));
		}

		template<typename... Targs>
		void disableStat(Targs... statIds)
		{
			disableStat(statIds...);
		}

		void disableAllStats()
		{
			std::lock_guard<std::mutex> lock(mMutex);

			mStatGroups.clear();
			mStats.clear();
		}

		bool isStatEnabled(StatGroup statGroup) const
		{
			std::lock_guard<std::mutex> lock(mMutex);

			return (mStatGroups.find(statGroup) != mStatGroups.end());
		}

		bool isStatEnabled(StatId statId) const
		{
			std::lock_guard<std::mutex> lock(mMutex);

			return (mStats.find(statId) != mStats.end());
		}

		void nextSample();

		void addSample(StatId statId, uint64 value);

		void recordingStart(const char* const filePath);
		void recordingStop();

		double getLastQuantizedSample(StatId statId) const;

		size_t getLastQuantizedSamples(StatId statId, bool normalizeValues, double* const outBuffer, const size_t maxItems) const;
		double getLastQuantizedSamples(StatId statId, std::chrono::milliseconds samplingPeriod) const;
	};

#else

	class Profiler
	{
	public:
		typedef uint_fast64_t uint64;

		static constexpr double QuantizationPeriodMS = 250.0;
		static constexpr unsigned int QuantizationSamples = 240;

		enum class StatGroup : unsigned int
		{
			Frame = 0x1,
			GPU = 0x2,
			Mem = 0x3
		};

		enum class StatId : unsigned int
		{
			FrameTotal = (static_cast<unsigned int>(StatGroup::Frame) << 16) | 0x1,
			FrameLogic = (static_cast<unsigned int>(StatGroup::Frame) << 16) | 0x2,
			FrameDraw = (static_cast<unsigned int>(StatGroup::Frame) << 16) | 0x3,
			FrameGPU = (static_cast<unsigned int>(StatGroup::Frame) << 16) | 0x4,

			GPUTimeElapsed = (static_cast<unsigned int>(StatGroup::GPU) << 16) | 0x1,
			GPUSamples = (static_cast<unsigned int>(StatGroup::GPU) << 16) | 0x2,
			GPUVerticesSubmitted = (static_cast<unsigned int>(StatGroup::GPU) << 16) | 0x3,
			GPUPrimitivesSubmitted = (static_cast<unsigned int>(StatGroup::GPU) << 16) | 0x4,
			GPUVertexShaderInvocations = (static_cast<unsigned int>(StatGroup::GPU) << 16) | 0x5,
			GPUFragmentShaderInvocations = (static_cast<unsigned int>(StatGroup::GPU) << 16) | 0x6,
			GPUClipInputPrimitives = (static_cast<unsigned int>(StatGroup::GPU) << 16) | 0x7,
			GPUClipOutputPrimitives = (static_cast<unsigned int>(StatGroup::GPU) << 16) | 0x8,

			MemTotal = (static_cast<unsigned int>(StatGroup::Mem) << 16) | 0x1,
			MemScripts = (static_cast<unsigned int>(StatGroup::Mem) << 16) | 0x2,
			MemObjects = (static_cast<unsigned int>(StatGroup::Mem) << 16) | 0x3,
			MemAnimations = (static_cast<unsigned int>(StatGroup::Mem) << 16) | 0x4,
		};

		static constexpr bool isSupported()
		{
			return false;
		}

		static const char* translateStatId(StatId statId)
		{
			return "feature disabled";
		}

	public:
		Profiler() = default;

		void enableStat(StatId statId)
		{ }

		template<typename... Targs>
		void enableStat(StatId statId, Targs... statIds)
		{ }

		void disableStat(StatId statId)
		{ }

		template<typename... Targs>
		void disableStat(Targs... statIds)
		{ }

		void disableAllStats()
		{ }

		bool isStatEnabled(StatGroup statGroup) const
		{
			return false;
		}

		bool isStatEnabled(StatId statId) const
		{
			return false;
		}

		void nextSample()
		{ }

		void addSample(StatId statId, uint64 value)
		{ }

		void recordingStart(const char* const filePath)
		{ }

		void recordingStop()
		{ }

		double getLastQuantizedSample(StatId statId) const
		{
			return 0.0;
		}

		size_t getLastQuantizedSamples(StatId statId, bool normalizeValues, double* const outBuffer, const size_t maxItems) const
		{
			return 0;
		}

		double getLastQuantizedSamples(StatId statId, std::chrono::milliseconds samplingPeriod) const
		{
			return 0.0;
		}
	};

#endif
} }
