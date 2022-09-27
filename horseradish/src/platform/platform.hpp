#pragma once

#include <functional>

#if defined(_WIN32) && !defined(_WIN64)
	#define HR_BUILD_WINDOWS
	#define HR_BUILD_WINDOWS32
#elif defined(_WIN32) && defined(_WIN64)
	#define HR_BUILD_WINDOWS
	#define HR_BUILD_WINDOWS64
#endif

namespace hr::platform
{
	class Platform
	{
	public:
		class SingleInstance
		{
			bool mIsAnotherRunning;
			void *mGlobalData;

		public:
			SingleInstance();
			~SingleInstance();

			bool isAnotherRunning() const
			{
				return mIsAnotherRunning;
			};
		};

	public:
		enum CPUFeature { SSE = (1 << 0), SSE2 = (1 << 1), HyperThreading = (1 << 2), CMov = (1 << 3) };

		enum class PriorityType { Normal, High, Highest };
		enum class OperatingSystemType { Windows, Linux, Android, iOS, OSX };
		enum class SystemInfo { DisplayWidth, DisplayHeight, DisplayColorBits, DisplayFrequency, OperatingSystemName, MachineName, CurrentUsername, MemoryTotal, MemoryFree, CleanBoot, ExecutableFullPath };

	public:
		static const char* NewLine;
		static const size_t NewLineSize;

		static const unsigned int DirectorySeparatorChar;
		static const unsigned int VolumeSeparatorChar;

		static bool setProcessPriority(PriorityType priorityType);
		static bool setThreadPriority(PriorityType priorityType);

		static OperatingSystemType getOS();
		static bool isOS(OperatingSystemType operatingSystemType);

		static bool isArch64();

		static bool cpuGetVendorID(std::string& outputValue);
		static bool cpuGetProcessorName(std::string& outputValue);
		static bool cpuCheckFeatures(CPUFeature featuresCheck);

		static bool systemInfo(SystemInfo systemInfo, std::string& infoValue);
		static bool systemInfo(SystemInfo systemInfo, int64_t& infoValue);

		static bool spawnSelf();

		static bool clipboardGetStrings(std::function<bool(const std::string&)> funcCallback);
		static bool clipboardGetFiles(std::function<bool(const std::string&)> funcCallback);

		static bool stdInOutErrRedirect();
		static void stdInOutErrClose();
		static void stdErrClear();
		static void stdOutClear();
		static bool stdErrRead(void *outBuffer, int outBufferSize, int &bytesWritten);
		static bool stdOutRead(void *outBuffer, int outBufferSize, int &bytesWritten);
	};
}
