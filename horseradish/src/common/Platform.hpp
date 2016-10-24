#pragma once

#include <functional>

namespace HorseRadish {

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
		enum class SystemInfo { DisplayWidth, DisplayHeight, DisplayColorBits, DisplayFrequency, OperatingSystemName, SystemFolder, MachineName, CurrentUsername, MemoryTotal, MemoryFree, CleanBoot, CurrentFolder, ExecutableFullPath };

	public:
		static const char* NewLine;
		static const int NewLineSize;

		static const unsigned int DirectorySeparatorChar;
		static const unsigned int VolumeSeparatorChar;

		static const unsigned int KiloByte;
		static const unsigned int MegaByte;
		static const unsigned int GigaByte;

		static bool setProcessPriority(PriorityType priorityType);
		static bool setThreadPriority(PriorityType priorityType);

		static OperatingSystemType getOS();
		static bool isOS(OperatingSystemType operatingSystemType);

		static bool isArch64();

		static bool cpuGetVendorID(std::string& outputValue);
		static bool cpuGetProcessorName(std::string& outputValue);
		static bool cpuCheckFeatures(CPUFeature featuresCheck);

		static bool systemInfo(SystemInfo systemInfo, std::string& infoValue);
		static bool systemInfo(SystemInfo systemInfo, int &infoValue);

		static bool spawnSelf();

		static bool clipboardGetStrings(std::function<bool(const std::string&)> funcCallback);
		static bool clipboardGetFiles(std::function<bool(const std::string&)> funcCallback);

		static void asmBufferClear(void* dest, size_t bytes);
		static void asmBufferCopy(void* dest, const void* src, size_t bytes);
		static void asmBufferCopyAligned(void* dest, const void* src, size_t multiple128Bytes);
		static void asmBufferSetUBYTE(void* dest, unsigned char val, size_t bytes);
		static void asmBufferSetUI32(void* dest, unsigned int val, size_t bytes);
		static void asmFloat2UByte(unsigned char *dest, const float *src, size_t num, const float mulVal, const float addVal);
		static void asmUByte2Float(float *dest, const unsigned char *src, size_t num, const float mulVal, const float addVal);

		static bool stdInOutErrRedirect();
		static void stdInOutErrClose();
		static void stdErrClear();
		static void stdOutClear();
		static bool stdErrRead(void *outBuffer, int outBufferSize, int &bytesWritten);
		static bool stdOutRead(void *outBuffer, int outBufferSize, int &bytesWritten);
	};
}
