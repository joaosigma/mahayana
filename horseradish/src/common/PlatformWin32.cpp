#include "Platform.hpp"

#include "UTF.hpp"
#include "ScopedAction.hpp"

#if defined(_WIN32)

#include <windows.h>
#include <shellapi.h>
#include <io.h>
#include <fcntl.h>

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

namespace HorseRadish
{
	const HorseRadish::hChar Platform::NewLine[] = "\r\n\0";
	const int Platform::NewLineSize = 2;

	const unsigned int Platform::DirectorySeparatorChar = '\\';
	const unsigned int Platform::VolumeSeparatorChar = ':';

	const int Platform::KiloByte = 1024;
	const int Platform::MegaByte = 1048576;
	const int Platform::GigaByte = 1073741824;

	Platform::SingleInstance::SingleInstance()
	{
		this->globalData = CreateMutex(NULL, FALSE, L"HorseRadish global mutex");
		this->isAnotherRunning = (GetLastError() == ERROR_ALREADY_EXISTS);
	}

	Platform::SingleInstance::~SingleInstance()
	{
		if (this->globalData != nullptr)
			CloseHandle(static_cast<HANDLE>(this->globalData));

		this->globalData = nullptr;
		this->isAnotherRunning = false;
	}

	bool Platform::SetProcessPriority(const PriorityType &priorityType)
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

	bool Platform::SetThreadPriority(const PriorityType &priorityType)
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

	Platform::OperatingSystemType Platform::GetOS()
	{
		return OperatingSystemType::Windows;
	}

	bool Platform::IsOS(const OperatingSystemType &operatingSystemType)
	{
		return (operatingSystemType == OperatingSystemType::Windows);
	}

	bool Platform::IsArch64()
	{
#if defined(_WIN64)
		return true;
#else
		return false;
#endif
	}

	bool Platform::GetSystemInfo(const SystemInfo &systemInfo, HorseRadish::String &infoValue)
	{
		TCHAR bufferAux[32767];
		DWORD bufferAuxCharCount;

		infoValue.SetEmpty();

		bufferAux[0] = '\0';
		bufferAuxCharCount = sizeof(bufferAux) / sizeof(TCHAR);

		if (systemInfo == Platform::SystemInfo::ExecutableFullPath)
		{
			auto resultado = GetModuleFileName(0, bufferAux, bufferAuxCharCount);
			if ((resultado == 0) || (resultado > bufferAuxCharCount))
				return false;

			infoValue.Set(HorseRadish::String::Encoding::Windows, bufferAux);
			return true;
		}

		if (systemInfo == Platform::SystemInfo::CurrentFolder)
		{
			auto resultado = GetCurrentDirectory(bufferAuxCharCount, bufferAux);
			if ((resultado == 0) || (resultado > bufferAuxCharCount))
				return false;

			infoValue.Set(HorseRadish::String::Encoding::Windows, bufferAux);
			return true;
		}

		if (systemInfo == Platform::SystemInfo::SystemFolder)
		{
			auto resultado = GetSystemDirectory(bufferAux, bufferAuxCharCount);
			if ((resultado == 0) || (resultado > bufferAuxCharCount))
				return false;

			infoValue.Set(HorseRadish::String::Encoding::Windows, bufferAux);
			return true;
		}

		if (systemInfo == Platform::SystemInfo::MachineName)
		{
			if (GetComputerName(bufferAux, &bufferAuxCharCount) == FALSE)
				return false;

			infoValue.Set(HorseRadish::String::Encoding::Windows, bufferAux);
			return true;
		}

		if (systemInfo == Platform::SystemInfo::CurrentUsername)
		{
			if (GetUserName(bufferAux, &bufferAuxCharCount) == FALSE)
				return false;

			infoValue.Set(HorseRadish::String::Encoding::Windows, bufferAux);
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
				infoValue.SetPrintf(HorseRadish::String::Encoding::UTF8, "Windows 7 (%d.%d)", versionInfo.dwMajorVersion, versionInfo.dwMinorVersion);
			else if ((versionInfo.dwMajorVersion == 6) && (versionInfo.dwMinorVersion == 0) && (versionInfo.wProductType != VER_NT_WORKSTATION))
				infoValue.SetPrintf(HorseRadish::String::Encoding::UTF8, "Windows Vista (%d.%d)", versionInfo.dwMajorVersion, versionInfo.dwMinorVersion);
			else if ((versionInfo.dwMajorVersion == 5) && (versionInfo.dwMinorVersion == 1))
				infoValue.SetPrintf(HorseRadish::String::Encoding::UTF8, "Windows XP (%d.%d)", versionInfo.dwMajorVersion, versionInfo.dwMinorVersion);
			else
				infoValue.SetPrintf(HorseRadish::String::Encoding::UTF8, "Windows (%d.%d)", versionInfo.dwMajorVersion, versionInfo.dwMinorVersion);

			if (versionInfo.wServicePackMajor > 0)
			{
				if (versionInfo.wServicePackMinor > 0)
					infoValue += HorseRadish::String(" SP%d.%d", versionInfo.wServicePackMajor, versionInfo.wServicePackMinor);
				else
					infoValue += HorseRadish::String(" SP%d", versionInfo.wServicePackMajor);
			}

			return true;
		}

		return false;
	}

	bool Platform::GetSystemInfo(const SystemInfo &systemInfo, int &infoValue)
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

	bool Platform::InstanciateProcess(const char * const commandLine)
	{
		STARTUPINFO startInfo;
		PROCESS_INFORMATION processInfo;
		wchar_t commandLineWChar[1024];

		if ((commandLine == NULL) || (*commandLine == '\0'))
			return false;

		memset(&startInfo, 0, sizeof(STARTUPINFO));
		memset(&processInfo, 0, sizeof(PROCESS_INFORMATION));
		startInfo.cb = sizeof(STARTUPINFO);
		startInfo.lpDesktop = L"";

		HorseRadish::UTF::ConvertUTF8To(commandLine, HorseRadish::UTF::Encoding::Windows, commandLineWChar, sizeof(commandLineWChar));

		CreateProcess(nullptr, commandLineWChar, nullptr, nullptr, FALSE, 0, nullptr, nullptr, &startInfo, &processInfo);

		CloseHandle(processInfo.hProcess);
		CloseHandle(processInfo.hThread);

		return true;
	}

	bool Platform::ClipboardGetStrings(std::function<bool(const HorseRadish::String &)> funcCallback)
	{
		HorseRadish::String clipDataUTF8, curToken;

		if (funcCallback == nullptr)
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

		clipDataUTF8.Set(HorseRadish::String::Encoding::Windows, clipData);

		for (HorseRadish::String::Tokenizer tokenizer(clipDataUTF8, '\n'); tokenizer.IsLast() == false;)
		{
			tokenizer.Read(curToken);
			if (curToken.GetSizeBytes() == 0)
				continue;

			if (funcCallback(curToken) == false)
				break;
		}

		return true;
	}

	bool Platform::ClipboardGetFiles(std::function<bool(const HorseRadish::String &)> funcCallback)
	{
		HorseRadish::String curFileUTF8;
		wchar_t fileBufferWChar[512];

		if (funcCallback == nullptr)
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

		for (unsigned int i = 0; i < numFiles; i++)
		{
			if (DragQueryFile(clipData, i, fileBufferWChar, sizeof(fileBufferWChar) / sizeof(wchar_t)) == 0)
				continue;

			curFileUTF8.Set(HorseRadish::String::Encoding::Windows, fileBufferWChar);

			if (funcCallback(curFileUTF8) == false)
				break;
		}

		return true;
	}

	bool Platform::StdInOutErrRedirect()
	{
		if (redirectData.redirected)
			return true;

		Platform::StdInOutErrClose();

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

	void Platform::StdInOutErrClose()
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

	void Platform::StdErrClear()
	{
		if (!redirectData.redirected)
			return;

		pipeClear(redirectData.pipeErr.read);
	}

	void Platform::StdOutClear()
	{
		if (!redirectData.redirected)
			return;

		pipeClear(redirectData.pipeOut.read);
	}

	bool Platform::StdErrRead(void *outBuffer, const int outBufferSize, int &bytesWritten)
	{
		bytesWritten = 0;

		if (!redirectData.redirected || outBuffer == nullptr || outBufferSize <= 0)
			return false;

		bytesWritten = pipeRead(redirectData.pipeErr.read, outBuffer, outBufferSize);
		if (bytesWritten >= 0)
			return true;

		bytesWritten = 0;
		return false;
	}

	bool Platform::StdOutRead(void *outBuffer, const int outBufferSize, int &bytesWritten)
	{
		bytesWritten = 0;

		if (!redirectData.redirected || outBuffer == nullptr || outBufferSize <= 0)
			return false;

		bytesWritten = pipeRead(redirectData.pipeOut.read, outBuffer, outBufferSize);
		if (bytesWritten >= 0)
			return true;

		bytesWritten = 0;
		return false;
	}
}

#endif