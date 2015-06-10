#pragma once

#include <functional>

namespace HorseRadish {

	class Platform
	{
	public:
		class SingleInstance
		{
		private:
			bool isAnotherRunning;
			void *globalData;
		public:
			SingleInstance();
			~SingleInstance();

			bool IsAnotherRunning() const { return this->isAnotherRunning; };
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

		static bool SetProcessPriority(const PriorityType &priorityType);
		static bool SetThreadPriority(const PriorityType &priorityType);

		static OperatingSystemType GetOS();
		static bool IsOS(const OperatingSystemType &operatingSystemType);

		static bool IsArch64();

		static bool CPUGetVendorID(std::string& outputValue);
		static bool CPUGetProcessorName(std::string& outputValue);
		static bool CPUCheckFeatures(const CPUFeature &featuresCheck);

		static bool GetSystemInfo(const SystemInfo &systemInfo, std::string& infoValue);
		static bool GetSystemInfo(const SystemInfo &systemInfo, int &infoValue);

		static bool InstanciateProcess(const std::string& commandLine);

		static bool ClipboardGetStrings(std::function<bool(const std::string&)> funcCallback);
		static bool ClipboardGetFiles(std::function<bool(const std::string&)> funcCallback);

		static void AsmBufferClear(void* dest, size_t bytes);
		static void AsmBufferCopy(void* dest, const void* src, size_t bytes);
		static void AsmBufferCopyAligned(void* dest, const void* src, size_t multiple128Bytes);
		static void AsmBufferSetUBYTE(void* dest, unsigned char val, size_t bytes);
		static void AsmBufferSetUI32(void* dest, unsigned int val, size_t bytes);
		static void AsmFloat2UByte(unsigned char *dest, const float *src, const unsigned int num, const float mulVal, const float addVal);
		static void AsmUByte2Float(float *dest, const unsigned char *src, const unsigned int num, const float mulVal, const float addVal);

		static bool StdInOutErrRedirect();
		static void StdInOutErrClose();
		static void StdErrClear();
		static void StdOutClear();
		static bool StdErrRead(void *outBuffer, const int outBufferSize, int &bytesWritten);
		static bool StdOutRead(void *outBuffer, const int outBufferSize, int &bytesWritten);
	};
}
