#pragma once
#ifndef __HPLATFORM__
#define __HPLATFORM__

#include "String.hpp"

#include <functional>

#include "PlatformWin32.hpp"

namespace HorseRadish
{
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
			enum PriorityType { Normal, High, Highest };
			enum OperatingSystemType { Windows, Linux };
			enum SystemInfo { DisplayWidth, DisplayHeight, DisplayColorBits, DisplayFrequency, OperatingSystemName, SystemFolder, MachineName, CurrentUsername, MemoryTotal, MemoryFree, CleanBoot, CurrentFolder, ExecutableFullPath };

		public:
			static const HorseRadish::hChar NewLine[];
			static const int NewLineSize;

			static const unsigned int DirectorySeparatorChar;
			static const unsigned int VolumeSeparatorChar;

			static const int KiloByte;
			static const int MegaByte;
			static const int GigaByte;

			static bool SetProcessPriority(const PriorityType &priorityType);
			static bool SetThreadPriority(const PriorityType &priorityType);

			static OperatingSystemType GetOS();
			static bool IsOS(const OperatingSystemType &operatingSystemType);

			static bool IsArch64();

			static bool GetSystemInfo(const SystemInfo &systemInfo, HorseRadish::String &infoValue);
			static bool GetSystemInfo(const SystemInfo &systemInfo, int &infoValue);

			static bool InstanciateProcess(const char * const commandLine);

			static bool ClipboardGetStrings(std::function<bool (const HorseRadish::String &)> funcCallback);
			static bool ClipboardGetFiles(std::function<bool (const HorseRadish::String &)> funcCallback);

			static bool StdInOutErrRedirect();
			static void StdInOutErrClose();
			static void StdErrClear();
			static void StdOutClear();
			static bool StdErrRead(void *outBuffer, const int outBufferSize, int &bytesWritten);
			static bool StdOutRead(void *outBuffer, const int outBufferSize, int &bytesWritten);
	};
}

#endif