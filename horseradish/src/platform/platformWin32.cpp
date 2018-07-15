#include "platform.hpp"

#include "../common/stringUtils.hpp"
#include "../common/scopedAction.hpp"

#include <libs/cppformat/format.h>

#if defined(_WIN32)

#include <io.h>
#include <regex>
#include <fcntl.h>

#include <windows.h>
#include <shellapi.h>

struct RedirectData {

	struct PipeHandles{
		HANDLE read, write;
	}pipeIn, pipeOut, pipeErr;

	int osHandlePipeIn, osHandlePipeOut, osHandlePipeErr;
	bool redirected;

} redirectData = { { nullptr, nullptr }, { nullptr, nullptr }, { nullptr, nullptr }, -1, -1, -1, false };

static
void pipeClear(HANDLE pipeHandle)
{
	DWORD bytesRead, bytesAvailable;

	while (true)
	{
		PeekNamedPipe(pipeHandle, nullptr, 0, nullptr, &bytesAvailable, nullptr);
		if (bytesAvailable <= 0)
			break;

		char tempBuffer[256];
		if (ReadFile(pipeHandle, tempBuffer, sizeof(tempBuffer), &bytesRead, nullptr) != TRUE)
			break;

		if (bytesRead < sizeof(tempBuffer))
			break;
	}
}

static
int pipeRead(HANDLE pipeHandle, void *outBuffer, const int outBufferSize)
{
	DWORD bytesRead, bytesAvailable;

	PeekNamedPipe(pipeHandle, nullptr, 0, nullptr, &bytesAvailable, nullptr);
	if (bytesAvailable <= 0)
		return 0;

	if (ReadFile(pipeHandle, outBuffer, outBufferSize, &bytesRead, nullptr) != TRUE)
		return -1;

	return bytesRead;
}

namespace hr { namespace platform
{
	const char* Platform::NewLine = "\r\n";
	const size_t Platform::NewLineSize = 2;

	const unsigned int Platform::DirectorySeparatorChar = '\\';
	const unsigned int Platform::VolumeSeparatorChar = ':';

	Platform::SingleInstance::SingleInstance()
	{
		mGlobalData = CreateMutex(NULL, FALSE, L"HorseRadish global mutex");
		mIsAnotherRunning = (GetLastError() == ERROR_ALREADY_EXISTS);
	}

	Platform::SingleInstance::~SingleInstance()
	{
		if (mGlobalData)
			CloseHandle(static_cast<HANDLE>(mGlobalData));

		mGlobalData = nullptr;
		mIsAnotherRunning = false;
	}

	bool Platform::setProcessPriority(PriorityType priorityType)
	{
		switch (priorityType)
		{
		case Platform::PriorityType::Normal:
			return (::SetPriorityClass(GetCurrentProcess(), NORMAL_PRIORITY_CLASS) == TRUE);
			break;
		case Platform::PriorityType::High:
			return (::SetPriorityClass(GetCurrentProcess(), ABOVE_NORMAL_PRIORITY_CLASS) == TRUE);
			break;
		case Platform::PriorityType::Highest:
			return (::SetPriorityClass(GetCurrentProcess(), HIGH_PRIORITY_CLASS) == TRUE);
			break;
		default:
			break;
		}

		return false;
	}

	bool Platform::setThreadPriority(PriorityType priorityType)
	{
		switch (priorityType)
		{
		case Platform::PriorityType::Normal:
			return (::SetThreadPriority(GetCurrentThread(), THREAD_PRIORITY_NORMAL) == TRUE);
			break;
		case Platform::PriorityType::High:
			return (::SetThreadPriority(GetCurrentThread(), THREAD_PRIORITY_ABOVE_NORMAL) == TRUE);
			break;
		case Platform::PriorityType::Highest:
			return (::SetThreadPriority(GetCurrentThread(), THREAD_PRIORITY_HIGHEST) == TRUE);
			break;
		default:
			break;
		}

		return false;
	}

	Platform::OperatingSystemType Platform::getOS()
	{
		return OperatingSystemType::Windows;
	}

	bool Platform::isOS(OperatingSystemType operatingSystemType)
	{
		return (operatingSystemType == OperatingSystemType::Windows);
	}

	bool Platform::isArch64()
	{
#if defined(_WIN64)
		return true;
#else
		return false;
#endif
	}

	bool Platform::cpuGetVendorID(std::string& outputValue)
	{
		int cpuInfo[4];
		char cpuString[128];

		memset(cpuInfo, 0, sizeof(cpuInfo));
		__cpuid(cpuInfo, 0x0);

		memset(cpuString, 0, sizeof(cpuString));
		memcpy(cpuString + 0, cpuInfo + 1, sizeof(int));
		memcpy(cpuString + 4, cpuInfo + 3, sizeof(int));
		memcpy(cpuString + 8, cpuInfo + 2, sizeof(int));

		outputValue = cpuString;
		return true;
	}

	bool Platform::cpuGetProcessorName(std::string& outputValue)
	{
		int cpuInfo[4];
		char cpuString[128];

		memset(cpuInfo, 0, sizeof(cpuInfo));
		__cpuid(cpuInfo, 0x80000000);

		if (cpuInfo[0] < 0x80000004)
		{
			outputValue = "<empty>";
			return true;
		}

		memset(cpuString, 0, sizeof(cpuString));

		__cpuid(cpuInfo, 0x80000002);
		memcpy(cpuString + 0, cpuInfo + 0, sizeof(int));
		memcpy(cpuString + 4, cpuInfo + 1, sizeof(int));
		memcpy(cpuString + 8, cpuInfo + 2, sizeof(int));
		memcpy(cpuString + 12, cpuInfo + 3, sizeof(int));
		__cpuid(cpuInfo, 0x80000003);
		memcpy(cpuString + 16, cpuInfo + 0, sizeof(int));
		memcpy(cpuString + 20, cpuInfo + 1, sizeof(int));
		memcpy(cpuString + 24, cpuInfo + 2, sizeof(int));
		memcpy(cpuString + 28, cpuInfo + 3, sizeof(int));
		__cpuid(cpuInfo, 0x80000004);
		memcpy(cpuString + 32, cpuInfo + 0, sizeof(int));
		memcpy(cpuString + 36, cpuInfo + 1, sizeof(int));
		memcpy(cpuString + 40, cpuInfo + 2, sizeof(int));
		memcpy(cpuString + 44, cpuInfo + 3, sizeof(int));

		outputValue = cpuString;
		hr::StringUtils::trim(outputValue);

		return true;
	}

	bool Platform::cpuCheckFeatures(CPUFeature featuresCheck)
	{
		int cpuInfo[4];

		memset(cpuInfo, 0, sizeof(cpuInfo));
		__cpuid(cpuInfo, 0x1);

		if ((featuresCheck & CPUFeature::SSE) == CPUFeature::SSE)
		{
			if ((cpuInfo[3] & 0x2000000) != 0x2000000)
				return false;
		}

		if ((featuresCheck & CPUFeature::SSE2) == CPUFeature::SSE2)
		{
			if ((cpuInfo[3] & 0x4000000) != 0x4000000)
				return false;
		}

		if ((featuresCheck & CPUFeature::HyperThreading) == CPUFeature::HyperThreading)
		{
			if ((cpuInfo[3] & 0x10000000) != 0x10000000)
				return false;
		}

		if ((featuresCheck & CPUFeature::CMov) == CPUFeature::CMov)
		{
			if ((cpuInfo[3] & 0x8000) != 0x8000)
				return false;
		}

		return true;
	}

	bool Platform::systemInfo(SystemInfo systemInfo, std::string& infoValue)
	{
		TCHAR bufferAux[32767];
		DWORD bufferAuxCharCount;

		bufferAux[0] = '\0';
		bufferAuxCharCount = sizeof(bufferAux) / sizeof(TCHAR);

		if (systemInfo == Platform::SystemInfo::ExecutableFullPath)
		{
			auto result = GetModuleFileName(0, bufferAux, bufferAuxCharCount);
			if ((result == 0) || (result > bufferAuxCharCount))
				return false;

			infoValue = hr::StringUtils::conv2UTF8(bufferAux);
			return true;
		}

		if (systemInfo == Platform::SystemInfo::CurrentFolder)
		{
			auto result = GetCurrentDirectory(bufferAuxCharCount, bufferAux);
			if ((result == 0) || (result > bufferAuxCharCount))
				return false;

			infoValue = hr::StringUtils::conv2UTF8(bufferAux);
			return true;
		}

		if (systemInfo == Platform::SystemInfo::SystemFolder)
		{
			auto result = GetSystemDirectory(bufferAux, bufferAuxCharCount);
			if ((result == 0) || (result > bufferAuxCharCount))
				return false;

			infoValue = hr::StringUtils::conv2UTF8(bufferAux);
			return true;
		}

		if (systemInfo == Platform::SystemInfo::MachineName)
		{
			if (GetComputerName(bufferAux, &bufferAuxCharCount) == FALSE)
				return false;

			infoValue = hr::StringUtils::conv2UTF8(bufferAux);
			return true;
		}

		if (systemInfo == Platform::SystemInfo::CurrentUsername)
		{
			if (GetUserName(bufferAux, &bufferAuxCharCount) == FALSE)
				return false;

			infoValue = hr::StringUtils::conv2UTF8(bufferAux);
			return true;
		}

		if (systemInfo == Platform::SystemInfo::OperatingSystemName)
		{
			OSVERSIONINFOEX versionInfo;

			memset(&versionInfo, 0, sizeof(OSVERSIONINFOEX));
			versionInfo.dwOSVersionInfoSize = sizeof(OSVERSIONINFOEX);
			if (GetVersionEx((LPOSVERSIONINFOW)&versionInfo) == FALSE)
				return false;

			if ((versionInfo.dwMajorVersion == 6) && (versionInfo.dwMinorVersion == 1) && (versionInfo.wProductType == VER_NT_WORKSTATION))
				infoValue = fmt::format("Windows 7 ({0}.{1})", versionInfo.dwMajorVersion, versionInfo.dwMinorVersion);
			else if ((versionInfo.dwMajorVersion == 6) && (versionInfo.dwMinorVersion == 0) && (versionInfo.wProductType != VER_NT_WORKSTATION))
				infoValue = fmt::format("Windows Vista ({0}.{1})", versionInfo.dwMajorVersion, versionInfo.dwMinorVersion);
			else if ((versionInfo.dwMajorVersion == 5) && (versionInfo.dwMinorVersion == 1))
				infoValue = fmt::format("Windows XP ({0}.{1})", versionInfo.dwMajorVersion, versionInfo.dwMinorVersion);
			else
				infoValue = fmt::format("Windows ({0}.{1})", versionInfo.dwMajorVersion, versionInfo.dwMinorVersion);

			if (versionInfo.wServicePackMajor > 0)
			{
				if (versionInfo.wServicePackMinor > 0)
					infoValue += fmt::format(" SP{0}.{1}", versionInfo.wServicePackMajor, versionInfo.wServicePackMinor);
				else
					infoValue += fmt::format(" SP{0}", versionInfo.wServicePackMajor);
			}

			return true;
		}

		return false;
	}

	bool Platform::systemInfo(SystemInfo systemInfo, int &infoValue)
	{
		infoValue = -1;

		if ((systemInfo == SystemInfo::MemoryTotal) || (systemInfo == SystemInfo::MemoryFree))
		{
			MEMORYSTATUS memoryStatus;
			GlobalMemoryStatus(&memoryStatus);

			if (systemInfo == SystemInfo::MemoryTotal)
			{
				infoValue = memoryStatus.dwTotalPhys;
				return true;
			}
			if (systemInfo == SystemInfo::MemoryFree)
			{
				infoValue = memoryStatus.dwAvailPhys;
				return true;
			}

			return false;
		}

		if ((systemInfo == SystemInfo::DisplayWidth) || (systemInfo == SystemInfo::DisplayHeight) || (systemInfo == SystemInfo::DisplayColorBits) || (systemInfo == SystemInfo::DisplayFrequency))
		{
			DEVMODE deviceMode;

			memset(&deviceMode, 0, sizeof(DEVMODE));
			deviceMode.dmSize = sizeof(DEVMODE);
			if (EnumDisplaySettingsEx(nullptr, ENUM_REGISTRY_SETTINGS, &deviceMode, 0) == FALSE)
				return false;

			if (systemInfo == SystemInfo::DisplayWidth)
			{
				infoValue = deviceMode.dmPelsWidth;
				return true;
			}
			if (systemInfo == SystemInfo::DisplayHeight)
			{
				infoValue = deviceMode.dmPelsHeight;
				return true;
			}
			if (systemInfo == SystemInfo::DisplayColorBits)
			{
				infoValue = deviceMode.dmBitsPerPel;
				return true;
			}
			if (systemInfo == SystemInfo::DisplayFrequency)
			{
				infoValue = deviceMode.dmDisplayFrequency;
				return true;
			}

			return false;
		}

		if (systemInfo == SystemInfo::CleanBoot)
		{
			infoValue = (GetSystemMetrics(SM_CLEANBOOT) == 0) ? 1 : 0;
			return true;
		}

		return false;
	}

	bool Platform::spawnSelf()
	{
		STARTUPINFO startInfo;
		PROCESS_INFORMATION processInfo;

		wchar_t szFileName[MAX_PATH];
		if (!GetModuleFileName(nullptr, szFileName, MAX_PATH))
			return false;

		memset(&startInfo, 0, sizeof(STARTUPINFO));
		memset(&processInfo, 0, sizeof(PROCESS_INFORMATION));
		startInfo.cb = sizeof(STARTUPINFO);
		startInfo.lpDesktop = L"";

		CreateProcess(nullptr, szFileName, nullptr, nullptr, FALSE, 0, nullptr, nullptr, &startInfo, &processInfo);

		CloseHandle(processInfo.hProcess);
		CloseHandle(processInfo.hThread);

		return true;
	}

	bool Platform::clipboardGetStrings(std::function<bool(const std::string&)> funcCallback)
	{
		if (!funcCallback)
			return false;

		if (OpenClipboard(nullptr) == FALSE)
			return false;

		ScopedAction scopedAction([&]()
		{
			CloseClipboard();
		});

		if (IsClipboardFormatAvailable(CF_UNICODETEXT) == FALSE)
			return false;

		auto clipData = GetClipboardData(CF_UNICODETEXT);
		if (clipData == nullptr)
			return false;

		auto clipDataUTF8 = hr::StringUtils::conv2UTF8(static_cast<const wchar_t*>(clipData));

		funcCallback(clipDataUTF8);

		/*
		TODO: VS2015 doesn't support this!!! :/

		std::sregex_token_iterator first(clipDataUTF8.begin(), clipDataUTF8.end(), std::regex("\\n+"), -1), last;
		for (; first != last; first++)
		{
			if (!funcCallback(*first))
				break;
		}*/

		return true;
	}

	bool Platform::clipboardGetFiles(std::function<bool(const std::string&)> funcCallback)
	{
		if (!funcCallback)
			return false;

		if (OpenClipboard(nullptr) == FALSE)
			return false;

		ScopedAction scopedAction([&]()
		{
			CloseClipboard();
		});

		if (IsClipboardFormatAvailable(CF_HDROP) == FALSE)
			return false;

		auto clipData = (HDROP)GetClipboardData(CF_HDROP);
		if (clipData == nullptr)
			return false;

		auto numFiles = DragQueryFile(clipData, 0xFFFFFFFF, NULL, 0);
		if (numFiles <= 0)
			return false;

		wchar_t fileBufferWChar[512];
		for (unsigned int i = 0; i < numFiles; i++)
		{
			if (DragQueryFile(clipData, i, fileBufferWChar, sizeof(fileBufferWChar) / sizeof(wchar_t)) == 0)
				continue;

			auto curFileUTF8 = hr::StringUtils::conv2UTF8(fileBufferWChar);
			if (!funcCallback(curFileUTF8))
				break;
		}

		return true;
	}

	bool Platform::stdInOutErrRedirect()
	{
		if (redirectData.redirected)
			return true;

		Platform::stdInOutErrClose();

		if (CreatePipe(&redirectData.pipeIn.read, &redirectData.pipeIn.write, nullptr, 0) == 0)
			return false;
		if (CreatePipe(&redirectData.pipeOut.read, &redirectData.pipeOut.write, nullptr, 0) == 0)
			return false;
		if (CreatePipe(&redirectData.pipeErr.read, &redirectData.pipeErr.write, nullptr, 0) == 0)
			return false;

		redirectData.osHandlePipeIn = _open_osfhandle((intptr_t)redirectData.pipeIn.read, _O_RDONLY);
		if (redirectData.osHandlePipeIn == -1)
			return false;
		redirectData.osHandlePipeOut = _open_osfhandle((intptr_t)redirectData.pipeOut.write, 0);
		if (redirectData.osHandlePipeOut == -1)
			return false;
		redirectData.osHandlePipeErr = _open_osfhandle((intptr_t)redirectData.pipeErr.write, 0);
		if (redirectData.osHandlePipeErr == -1)
			return false;

		if (_dup2(redirectData.osHandlePipeIn, 0) != 0) //stdin
			return false;
		if (_dup2(redirectData.osHandlePipeOut, 1) != 0) //stdout
			return false;
		if (_dup2(redirectData.osHandlePipeErr, 2) != 0) //stderr
			return false;

		redirectData.redirected = true;
		return false;
	}

	void Platform::stdInOutErrClose()
	{
		if (redirectData.osHandlePipeIn != -1)
			_close(redirectData.osHandlePipeIn);
		if (redirectData.osHandlePipeOut != -1)
			_close(redirectData.osHandlePipeOut);
		if (redirectData.osHandlePipeErr != -1)
			_close(redirectData.osHandlePipeErr);

		if (redirectData.pipeIn.write != nullptr)
			CloseHandle(redirectData.pipeIn.write);
		if (redirectData.pipeOut.read != nullptr)
			CloseHandle(redirectData.pipeOut.read);
		if (redirectData.pipeErr.read != nullptr)
			CloseHandle(redirectData.pipeErr.read);

		memset(&redirectData, 0, sizeof(RedirectData));
		redirectData.osHandlePipeIn = -1;
		redirectData.osHandlePipeOut = -1;
		redirectData.osHandlePipeErr = -1;
	}

	void Platform::stdErrClear()
	{
		if (!redirectData.redirected)
			return;

		pipeClear(redirectData.pipeErr.read);
	}

	void Platform::stdOutClear()
	{
		if (!redirectData.redirected)
			return;

		pipeClear(redirectData.pipeOut.read);
	}

	bool Platform::stdErrRead(void *outBuffer, int outBufferSize, int &bytesWritten)
	{
		bytesWritten = 0;

		if (!redirectData.redirected || !outBuffer || outBufferSize <= 0)
			return false;

		bytesWritten = pipeRead(redirectData.pipeErr.read, outBuffer, outBufferSize);
		if (bytesWritten >= 0)
			return true;

		bytesWritten = 0;
		return false;
	}

	bool Platform::stdOutRead(void *outBuffer, int outBufferSize, int &bytesWritten)
	{
		bytesWritten = 0;

		if (!redirectData.redirected || !outBuffer || outBufferSize <= 0)
			return false;

		bytesWritten = pipeRead(redirectData.pipeOut.read, outBuffer, outBufferSize);
		if (bytesWritten >= 0)
			return true;

		bytesWritten = 0;
		return false;
	}
} }

#endif