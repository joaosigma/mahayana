#include "Platform.hpp"

#include "stringUtils.hpp"
#include "ScopedAction.hpp"

#include <libs\cppformat\format.h>

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

namespace HorseRadish
{
	const char* Platform::NewLine = "\r\n\0";
	const int Platform::NewLineSize = 2;

	const unsigned int Platform::DirectorySeparatorChar = '\\';
	const unsigned int Platform::VolumeSeparatorChar = ':';

	const unsigned int Platform::KiloByte = 1024;
	const unsigned int Platform::MegaByte = 1048576;
	const unsigned int Platform::GigaByte = 1073741824;

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
		HorseRadish::StringUtils::trim(outputValue);

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

			infoValue = HorseRadish::StringUtils::conv2UTF8(bufferAux);
			return true;
		}

		if (systemInfo == Platform::SystemInfo::CurrentFolder)
		{
			auto result = GetCurrentDirectory(bufferAuxCharCount, bufferAux);
			if ((result == 0) || (result > bufferAuxCharCount))
				return false;

			infoValue = HorseRadish::StringUtils::conv2UTF8(bufferAux);
			return true;
		}

		if (systemInfo == Platform::SystemInfo::SystemFolder)
		{
			auto result = GetSystemDirectory(bufferAux, bufferAuxCharCount);
			if ((result == 0) || (result > bufferAuxCharCount))
				return false;

			infoValue = HorseRadish::StringUtils::conv2UTF8(bufferAux);
			return true;
		}

		if (systemInfo == Platform::SystemInfo::MachineName)
		{
			if (GetComputerName(bufferAux, &bufferAuxCharCount) == FALSE)
				return false;

			infoValue = HorseRadish::StringUtils::conv2UTF8(bufferAux);
			return true;
		}

		if (systemInfo == Platform::SystemInfo::CurrentUsername)
		{
			if (GetUserName(bufferAux, &bufferAuxCharCount) == FALSE)
				return false;

			infoValue = HorseRadish::StringUtils::conv2UTF8(bufferAux);
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

		auto clipDataUTF8 = HorseRadish::StringUtils::conv2UTF8(static_cast<const wchar_t*>(clipData));

		funcCallback(clipDataUTF8);

		/*
		TODO: VS205 doesn't support this!!! :/

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

			auto curFileUTF8 = HorseRadish::StringUtils::conv2UTF8(fileBufferWChar);
			if (funcCallback(curFileUTF8) == false)
				break;
		}

		return true;
	}

	void Platform::asmBufferClear(void* dest, size_t bytes)
	{

#ifdef _M_X64

		memset(dest, 0, bytes);

#else

		__asm
		{
			mov		ebx, [bytes]
				mov		edi, [dest]

				cmp		ebx, 128
				jb		$memclr_the_end

				xor		edx, edx
				mov		eax, edi
				mov		ecx, 16
				div		ecx

				pxor	xmm0, xmm0
				pxor	xmm1, xmm1
				pxor	xmm2, xmm2
				pxor	xmm3, xmm3

				cmp		edx, 0
				je		$memclr_align_done

				sub		ecx, edx
				sub		ebx, ecx
				xor		eax, eax

				rep		stos al

			$memclr_align_done :

			cmp		ebx, 128
				jb		$memclr_the_end

				sub		ebx, 128
				movntdq[edi + 0], xmm0
				movntdq[edi + 16], xmm1
				movntdq[edi + 32], xmm2
				movntdq[edi + 48], xmm3
				movntdq[edi + 64], xmm0
				movntdq[edi + 80], xmm1
				movntdq[edi + 96], xmm2
				movntdq[edi + 112], xmm3
				add		edi, 128

				jmp		$memclr_align_done

			$memclr_the_end :

			mov		ecx, ebx
				xor		eax, eax
				rep		stos al
		}

#endif

	}

	void Platform::asmBufferCopy(void* dest, const void* src, size_t bytes)
	{

#ifdef _M_X64

		memcpy(dest, src, bytes);

#else

		__asm {

			// Copyright (C) 2009  Jan Boon (Kaetemi)
			// optimized on Intel Core 2 Duo T7500

			mov         ecx, bytes
				mov         edi, dest
				mov         esi, src
				add         ecx, edi

				prefetchnta[esi]
				prefetchnta[esi + 32]
				prefetchnta[esi + 64]
				prefetchnta[esi + 96]

				// handle nBytes lower than 128
				cmp         bytes, 512
				jge         fast
			slow :
			mov         bl, [esi]
				mov[edi], bl
				inc         edi
				inc         esi
				cmp         ecx, edi
				jnz         slow
				jmp         end

			fast :
			// align dstEnd to 128 bytes
			and         ecx, 0xFFFFFF80

				// get srcEnd aligned to dstEnd aligned to 128 bytes
				mov         ebx, esi
				sub         ebx, edi
				add         ebx, ecx

				// skip unaligned copy if dst is aligned
				mov         eax, edi
				and         edi, 0xFFFFFF80
				cmp         eax, edi
				jne         first
				jmp         more

			first :
			// copy the first 128 bytes unaligned
			movdqu      xmm0, [esi]
				movdqu      xmm1, [esi + 16]
				movdqu      xmm2, [esi + 32]
				movdqu      xmm3, [esi + 48]

				movdqu      xmm4, [esi + 64]
				movdqu      xmm5, [esi + 80]
				movdqu      xmm6, [esi + 96]
				movdqu      xmm7, [esi + 112]

				movdqu[eax], xmm0
				movdqu[eax + 16], xmm1
				movdqu[eax + 32], xmm2
				movdqu[eax + 48], xmm3

				movdqu[eax + 64], xmm4
				movdqu[eax + 80], xmm5
				movdqu[eax + 96], xmm6
				movdqu[eax + 112], xmm7

				// add 128 bytes to edi aligned earlier
				add         edi, 128

				// offset esi by the same value
				sub         eax, edi
				sub         esi, eax

				// last bytes if dst at dstEnd
				cmp         ecx, edi
				jnz         more
				jmp         last

			more :
			// handle equally aligned arrays
			mov         eax, esi
				and         eax, 0xFFFFFF80
				cmp         eax, esi
				jne         unaligned4k

			aligned4k :
			mov         eax, esi
				add         eax, 4096
				cmp         eax, ebx
				jle         aligned4kin
				cmp         ecx, edi
				jne         alignedlast
				jmp         last

			aligned4kin :
			prefetchnta[esi]
				prefetchnta[esi + 32]
				prefetchnta[esi + 64]
				prefetchnta[esi + 96]

				add         esi, 128

				cmp         eax, esi
				jne         aligned4kin

				sub         esi, 4096

			alinged4kout:
			movdqa      xmm0, [esi]
				movdqa      xmm1, [esi + 16]
				movdqa      xmm2, [esi + 32]
				movdqa      xmm3, [esi + 48]

				movdqa      xmm4, [esi + 64]
				movdqa      xmm5, [esi + 80]
				movdqa      xmm6, [esi + 96]
				movdqa      xmm7, [esi + 112]

				movntdq[edi], xmm0
				movntdq[edi + 16], xmm1
				movntdq[edi + 32], xmm2
				movntdq[edi + 48], xmm3

				movntdq[edi + 64], xmm4
				movntdq[edi + 80], xmm5
				movntdq[edi + 96], xmm6
				movntdq[edi + 112], xmm7

				add         esi, 128
				add         edi, 128

				cmp         eax, esi
				jne         alinged4kout
				jmp         aligned4k

			alignedlast :
			mov         eax, esi

			alignedlastin :
			prefetchnta[esi]
				prefetchnta[esi + 32]
				prefetchnta[esi + 64]
				prefetchnta[esi + 96]

				add         esi, 128

				cmp         ebx, esi
				jne         alignedlastin

				mov         esi, eax

			alignedlastout :
			movdqa      xmm0, [esi]
				movdqa      xmm1, [esi + 16]
				movdqa      xmm2, [esi + 32]
				movdqa      xmm3, [esi + 48]

				movdqa      xmm4, [esi + 64]
				movdqa      xmm5, [esi + 80]
				movdqa      xmm6, [esi + 96]
				movdqa      xmm7, [esi + 112]

				movntdq[edi], xmm0
				movntdq[edi + 16], xmm1
				movntdq[edi + 32], xmm2
				movntdq[edi + 48], xmm3

				movntdq[edi + 64], xmm4
				movntdq[edi + 80], xmm5
				movntdq[edi + 96], xmm6
				movntdq[edi + 112], xmm7

				add         esi, 128
				add         edi, 128

				cmp         ecx, edi
				jne         alignedlastout
				jmp         last

			unaligned4k :
			mov         eax, esi
				add         eax, 4096
				cmp         eax, ebx
				jle         unaligned4kin
				cmp         ecx, edi
				jne         unalignedlast
				jmp         last

			unaligned4kin :
			prefetchnta[esi]
				prefetchnta[esi + 32]
				prefetchnta[esi + 64]
				prefetchnta[esi + 96]

				add         esi, 128

				cmp         eax, esi
				jne         unaligned4kin

				sub         esi, 4096

			unalinged4kout:
			movdqu      xmm0, [esi]
				movdqu      xmm1, [esi + 16]
				movdqu      xmm2, [esi + 32]
				movdqu      xmm3, [esi + 48]

				movdqu      xmm4, [esi + 64]
				movdqu      xmm5, [esi + 80]
				movdqu      xmm6, [esi + 96]
				movdqu      xmm7, [esi + 112]

				movntdq[edi], xmm0
				movntdq[edi + 16], xmm1
				movntdq[edi + 32], xmm2
				movntdq[edi + 48], xmm3

				movntdq[edi + 64], xmm4
				movntdq[edi + 80], xmm5
				movntdq[edi + 96], xmm6
				movntdq[edi + 112], xmm7

				add         esi, 128
				add         edi, 128

				cmp         eax, esi
				jne         unalinged4kout
				jmp         unaligned4k

			unalignedlast :
			mov         eax, esi

			unalignedlastin :
			prefetchnta[esi]
				prefetchnta[esi + 32]
				prefetchnta[esi + 64]
				prefetchnta[esi + 96]

				add         esi, 128

				cmp         ebx, esi
				jne         unalignedlastin

				mov         esi, eax

			unalignedlastout :
			movdqu      xmm0, [esi]
				movdqu      xmm1, [esi + 16]
				movdqu      xmm2, [esi + 32]
				movdqu      xmm3, [esi + 48]

				movdqu      xmm4, [esi + 64]
				movdqu      xmm5, [esi + 80]
				movdqu      xmm6, [esi + 96]
				movdqu      xmm7, [esi + 112]

				movntdq[edi], xmm0
				movntdq[edi + 16], xmm1
				movntdq[edi + 32], xmm2
				movntdq[edi + 48], xmm3

				movntdq[edi + 64], xmm4
				movntdq[edi + 80], xmm5
				movntdq[edi + 96], xmm6
				movntdq[edi + 112], xmm7

				add         esi, 128
				add         edi, 128

				cmp         ecx, edi
				jne         unalignedlastout
				jmp         last

			last :
			// get the last 128 bytes
			mov         ecx, bytes
				mov         edi, dest
				mov         esi, src
				add         edi, ecx
				add         esi, ecx
				sub         edi, 128
				sub         esi, 128

				// copy the last 128 bytes unaligned
				movdqu      xmm0, [esi]
				movdqu      xmm1, [esi + 16]
				movdqu      xmm2, [esi + 32]
				movdqu      xmm3, [esi + 48]

				movdqu      xmm4, [esi + 64]
				movdqu      xmm5, [esi + 80]
				movdqu      xmm6, [esi + 96]
				movdqu      xmm7, [esi + 112]

				movdqu[edi], xmm0
				movdqu[edi + 16], xmm1
				movdqu[edi + 32], xmm2
				movdqu[edi + 48], xmm3

				movdqu[edi + 64], xmm4
				movdqu[edi + 80], xmm5
				movdqu[edi + 96], xmm6
				movdqu[edi + 112], xmm7

			end :
		}

#endif

	}

	void Platform::asmBufferCopyAligned(void* dest, const void* src, size_t multiple128Bytes)
	{

#ifdef _M_X64

		memcpy(dest, src, multiple128Bytes * 128);

#else

		__asm
		{
			mov esi, src;    //src pointer
			mov edi, dest;   //dest pointer

			mov ebx, multiple128Bytes; //ebx is our counter 

		loop_copy:
			prefetchnta 128[ESI]; //SSE2 prefetch
			prefetchnta 160[ESI];
			prefetchnta 192[ESI];
			prefetchnta 224[ESI];

			movdqa xmm0, 0[ESI]; //move data from src to registers
			movdqa xmm1, 16[ESI];
			movdqa xmm2, 32[ESI];
			movdqa xmm3, 48[ESI];
			movdqa xmm4, 64[ESI];
			movdqa xmm5, 80[ESI];
			movdqa xmm6, 96[ESI];
			movdqa xmm7, 112[ESI];

			movntdq 0[EDI], xmm0; //move data from registers to dest
			movntdq 16[EDI], xmm1;
			movntdq 32[EDI], xmm2;
			movntdq 48[EDI], xmm3;
			movntdq 64[EDI], xmm4;
			movntdq 80[EDI], xmm5;
			movntdq 96[EDI], xmm6;
			movntdq 112[EDI], xmm7;

			add esi, 128;
			add edi, 128;
			dec ebx;

			jnz loop_copy; //loop please
		}

#endif

	}

	void Platform::asmBufferSetUBYTE(void* dest, unsigned char val, size_t bytes)
	{

#ifdef _M_X64

		memset(dest, val, bytes);

#else

		__asm
		{

			mov		ebx, bytes
				mov		edi, dest

				cmp		ebx, 128
				jb		$memset_the_end

				mov		al, val
				mov		ah, al
				shl		eax, 16
				mov		al, val
				mov		ah, al
				movd	xmm0, eax
				shufps	xmm0, xmm0, 0
				movdqa	xmm1, xmm0
				movdqa	xmm2, xmm0
				movdqa	xmm3, xmm0

				xor		edx, edx
				mov		eax, edi
				mov		ecx, 16
				div		ecx

				cmp		edx, 0
				je		$memset_align_done

				sub		ecx, edx
				sub		ebx, ecx
				mov		al, val

				rep		stos al

			$memset_align_done :

			cmp		ebx, 128
				jb		$memset_the_end

				sub		ebx, 128
				movntdq[edi + 0], xmm0
				movntdq[edi + 16], xmm1
				movntdq[edi + 32], xmm2
				movntdq[edi + 48], xmm3
				movntdq[edi + 64], xmm0
				movntdq[edi + 80], xmm1
				movntdq[edi + 96], xmm2
				movntdq[edi + 112], xmm3
				add		edi, 128
				jmp		$memset_align_done

			$memset_the_end :
			mov		ecx, ebx
				mov		al, val
				rep		stos al
		}

#endif

	}

	void Platform::asmBufferSetUI32(void* dest, unsigned int val, size_t bytes)
	{

#ifdef _M_X64

		for (size_t multiplosInts = bytes / sizeof(unsigned int); multiplosInts > 0; multiplosInts--, bytes -= sizeof(unsigned int), dest = ((unsigned int*)dest) + 1)
			*((unsigned int*)dest) = val;

		auto resto = bytes % sizeof(unsigned int);
		if (resto > 0)
			memcpy(dest, &val, resto);

#else

		__asm
		{
			mov		ebx, bytes
				mov		edi, dest

				cmp		ebx, 128
				jb		$memset_i32_the_end

				xor		edx, edx
				mov		eax, edi
				mov		ecx, 8
				div		ecx

				movd	xmm0, val
				shufps	xmm0, xmm0, 0
				movdqa	xmm1, xmm0
				movdqa	xmm2, xmm0
				movdqa	xmm3, xmm0

				cmp		edx, 0
				je		$memset_i32_align

			$memset_i32_no_align :

			cmp		ebx, 128
				jb		$memset_i32_the_end

				sub		ebx, 128
				movdqu[edi + 0], xmm0
				movdqu[edi + 16], xmm1
				movdqu[edi + 32], xmm2
				movdqu[edi + 48], xmm3
				movdqu[edi + 64], xmm0
				movdqu[edi + 80], xmm1
				movdqu[edi + 96], xmm2
				movdqu[edi + 112], xmm3
				add		edi, 128
				jmp		$memset_i32_no_align

			$memset_i32_align :

			cmp		ebx, 128
				jb		$memset_i32_the_end

				sub		ebx, 128
				movntdq[edi + 0], xmm0
				movntdq[edi + 16], xmm1
				movntdq[edi + 32], xmm2
				movntdq[edi + 48], xmm3
				movntdq[edi + 64], xmm0
				movntdq[edi + 80], xmm1
				movntdq[edi + 96], xmm2
				movntdq[edi + 112], xmm3
				add		edi, 128
				jmp		$memset_i32_align

			$memset_i32_the_end :

			mov		ecx, ebx
				shr		ecx, 2
				mov		eax, val
				rep		stos eax

				and		ebx, 0x3

				cmp		ebx, 0
				je		$memset_i32_finally
				mov		eax, 0xff
				shl		eax, 0
				and		eax, val
				shr		eax, 0
				and		eax, 0xff
				mov		BYTE PTR[edi], al
				dec		ebx

				cmp		ebx, 0
				je		$memset_i32_finally
				mov		eax, 0xff
				shl		eax, 8
				and		eax, val
				shr		eax, 8
				and		eax, 0xff
				mov		BYTE PTR[edi + 1], al
				dec		ebx

				cmp		ebx, 0
				je		$memset_i32_finally
				mov		eax, 0xff
				shl		eax, 16
				and		eax, val
				shr		eax, 16
				and		eax, 0xff
				mov		BYTE PTR[edi + 2], al

			$memset_i32_finally :
		}

#endif

	}

	void Platform::asmFloat2UByte(unsigned char *dest, const float *src, size_t num, const float mulVal, const float addVal)
	{

#ifdef _M_X64

		for (int i = 0; i < num; i++)
			dest[i] = std::min(std::max((Math::ftoi(src[i]) * mulVal) + addVal, 0), 255);

#else

		static const __declspec(align(16)) float _255 = 255.0f;
		int intVal;

		__asm
		{
			mov ebx, num
				mov edi, dest
				mov esi, src

				cmp		ebx, 32
				jbe		$convF_just_last

				xor		edx, edx
				mov		eax, esi
				mov		ecx, 16
				div		ecx
				cmp		edx, 0
				je		$convF_align_done

				cmp		edx, 4
				je		$convF_align_cando
				cmp		edx, 8
				je		$convF_align_cando
				cmp		edx, 12
				je		$convF_align_cando

				movd	xmm4, mulVal
				movd	xmm5, addVal

				shufps	xmm4, xmm4, 0x00
				shufps	xmm5, xmm5, 0x00

				jmp		$convF_loopUA_16

			$convF_align_cando :
			shr		edx, 2
				sub		ecx, edx
				sub		ebx, ecx

				fld		mulVal
				fld		addVal
				fldz
				fld		_255

			$convF_align_loop :
			cmp		ecx, 0
				je		$convF_align_done_pop

				fld[esi]
				fmul	st(0), st(4)
				fadd	st(0), st(3)
				fcomi	st(0), st(1)
				fcmovnb	st(0), st(1)
				fcomi	st(0), st(2)
				fcmovb	st(0), st(2)
				fistp[edi]

				sub		ecx, 1
				add		esi, 4
				add		edi, 1
				jmp		$convF_align_loop

			$convF_align_done_pop :

			ffreep st(0)
				ffreep st(0)
				ffreep st(0)
				ffreep st(0)

			$convF_align_done :

							  movd	xmm4, mulVal
							  movd	xmm5, addVal

							  shufps	xmm4, xmm4, 0x00
							  shufps	xmm5, xmm5, 0x00

						  $convF_loopA_16:
			cmp ebx, 16
				jb $convF_loop_1

				prefetchnta[esi + 64]

				movaps xmm0, [esi]
				movaps xmm1, [esi + 16]
				movaps xmm2, [esi + 32]
				movaps xmm3, [esi + 48]
				mulps xmm0, xmm4
				mulps xmm1, xmm4
				mulps xmm2, xmm4
				mulps xmm3, xmm4
				addps xmm0, xmm5
				addps xmm1, xmm5
				addps xmm2, xmm5
				addps xmm3, xmm5

				cvtps2pi mm0, xmm0
				cvtps2pi mm2, xmm1
				movhlps xmm0, xmm0
				movhlps xmm1, xmm1
				cvtps2pi mm1, xmm0
				cvtps2pi mm3, xmm1

				packssdw mm0, mm1
				packuswb mm0, mm0

				packssdw mm2, mm3
				packuswb mm2, mm2

				movd[edi + 0], mm0
				movd[edi + 4], mm2

				cvtps2pi mm0, xmm2
				cvtps2pi mm2, xmm3
				movhlps xmm2, xmm2
				movhlps xmm3, xmm3
				cvtps2pi mm1, xmm2
				cvtps2pi mm3, xmm3

				packssdw mm0, mm1
				packuswb mm0, mm0

				packssdw mm2, mm3
				packuswb mm2, mm2

				movd[edi + 8], mm0
				movd[edi + 12], mm2

				sub ebx, 16
				add esi, 64
				add edi, 16
				jmp $convF_loopA_16

			$convF_loopUA_16 :
			cmp ebx, 16
				jb $convF_just_last_usedMMX

				prefetchnta[esi + 64]

				movups xmm0, [esi]
				movups xmm1, [esi + 16]
				movups xmm2, [esi + 32]
				movups xmm3, [esi + 48]
				mulps xmm0, xmm4
				mulps xmm1, xmm4
				mulps xmm2, xmm4
				mulps xmm3, xmm4
				addps xmm0, xmm5
				addps xmm1, xmm5
				addps xmm2, xmm5
				addps xmm3, xmm5

				cvtps2pi mm0, xmm0
				cvtps2pi mm2, xmm1
				movhlps xmm0, xmm0
				movhlps xmm1, xmm1
				cvtps2pi mm1, xmm0
				cvtps2pi mm3, xmm1

				packssdw mm0, mm1
				packuswb mm0, mm0

				packssdw mm2, mm3
				packuswb mm2, mm2

				movd[edi + 0], mm0
				movd[edi + 4], mm2

				cvtps2pi mm0, xmm2
				cvtps2pi mm2, xmm3
				movhlps xmm2, xmm2
				movhlps xmm3, xmm3
				cvtps2pi mm1, xmm2
				cvtps2pi mm3, xmm3

				packssdw mm0, mm1
				packuswb mm0, mm0

				packssdw mm2, mm3
				packuswb mm2, mm2

				movd[edi + 8], mm0
				movd[edi + 12], mm2

				sub ebx, 16
				add esi, 64
				add edi, 16
				jmp $convF_loopUA_16

			$convF_loop_1 :
			cmp ebx, 4
				jb $convF_just_last_usedMMX

				movaps xmm0, [esi]
				mulps xmm0, xmm4
				addps xmm0, xmm5
				cvtps2pi mm0, xmm0
				movhlps xmm0, xmm0
				cvtps2pi mm1, xmm0

				packssdw mm0, mm1
				packuswb mm0, mm0

				movd[edi], mm0

				sub ebx, 4
				add esi, 16
				add edi, 4
				jmp	$convF_loop_1

			$convF_just_last_usedMMX :
			emms

			$convF_just_last :
			cmp ebx, 0
				je $convF_finalEnd

				fld		mulVal
				fld		addVal
				fldz
				fld		_255

			$convF_just_last_do1 :
			cmp		ebx, 0
				je		$convF_just_last_do1_end

				fld[esi]
				fmul	st(0), st(4)
				fadd	st(0), st(3)
				fcomi	st(0), st(1)
				fcmovnb	st(0), st(1)
				fcomi	st(0), st(2)
				fcmovb	st(0), st(2)
				fistp	intVal
				mov		eax, intVal
				mov[edi], al

				sub		ebx, 1
				add		esi, 4
				add		edi, 1
				jmp		$convF_just_last_do1

			$convF_just_last_do1_end :
			ffreep st(0)
				ffreep st(0)
				ffreep st(0)
				ffreep st(0)

			$convF_finalEnd :
		}

#endif

	}

	void Platform::asmUByte2Float(float *dest, const unsigned char *src, size_t num, const float mulVal, const float addVal)
	{

#ifdef _M_X64

		for (int i = 0; i < num; i++)
			dest[i] = ((float)src[i]) * mulVal + addVal;

#else

		static __declspec(align(16)) int auxBuffer;

		__asm
		{
			mov ebx, num
				mov edi, dest
				mov esi, src

				cmp		ebx, 32
				jbe		$convF_just_last

				xor		edx, edx
				mov		eax, edi
				mov		ecx, 16
				div		ecx
				cmp		edx, 0
				je		$convF_align_done

				cmp		edx, 4
				je		$convF_align_cando
				cmp		edx, 8
				je		$convF_align_cando
				cmp		edx, 12
				je		$convF_align_cando

				pxor	mm7, mm7
				pxor	xmm5, xmm5
				movd	xmm6, mulVal
				movd	xmm7, addVal
				shufps	xmm6, xmm6, 0x00
				shufps	xmm7, xmm7, 0x00

				jmp		$convF_loopUA_16

			$convF_align_cando :
			shr		edx, 2
				sub		ecx, edx
				sub		ebx, ecx

				fld		addVal
				fld		mulVal

			$convF_align_loop :
			cmp		ecx, 0
				je		$convF_align_done_pop

				movzx	eax, BYTE PTR[esi]
				mov		DWORD PTR[auxBuffer], eax
				fild[auxBuffer]
				fmul	st(0), st(1)
				fadd	st(0), st(2)
				fstp[edi]

				sub		ecx, 1
				add		esi, 1
				add		edi, 4
				jmp		$convF_align_loop

			$convF_align_done_pop :

			ffreep st(0)
				ffreep st(0)

			$convF_align_done :

							  pxor	mm7, mm7
							  pxor	xmm5, xmm5
							  movd	xmm6, mulVal
							  movd	xmm7, addVal
							  shufps	xmm6, xmm6, 0x00
							  shufps	xmm7, xmm7, 0x00

						  $convF_loopA_16:
			cmp ebx, 20
				jb $convF_just_last_usedMMX

				prefetchnta[esi + 24]

				movd	mm0, [esi + 0]
				movd	mm1, [esi + 4]
				movd	mm2, [esi + 8]
				movd	mm3, [esi + 12]
				movd	mm4, [esi + 16]

				punpcklbw	mm0, mm7
				punpcklbw	mm1, mm7
				punpcklbw	mm2, mm7
				punpcklbw	mm3, mm7
				punpcklbw	mm4, mm7

				movq2dq		xmm0, mm0
				movq2dq		xmm1, mm1
				movq2dq		xmm2, mm2
				movq2dq		xmm3, mm3
				movq2dq		xmm4, mm4

				punpcklwd	xmm0, xmm5
				punpcklwd	xmm1, xmm5
				punpcklwd	xmm2, xmm5
				punpcklwd	xmm3, xmm5
				punpcklwd	xmm4, xmm5

				cvtdq2ps xmm0, xmm0
				cvtdq2ps xmm1, xmm1
				cvtdq2ps xmm2, xmm2
				cvtdq2ps xmm3, xmm3
				cvtdq2ps xmm4, xmm4

				mulps	xmm0, xmm6
				mulps	xmm1, xmm6
				mulps	xmm2, xmm6
				mulps	xmm3, xmm6
				mulps	xmm4, xmm6

				addps	xmm0, xmm7
				addps	xmm1, xmm7
				addps	xmm2, xmm7
				addps	xmm3, xmm7
				addps	xmm4, xmm7

				movntps[edi + 0], xmm0
				movntps[edi + 16], xmm1
				movntps[edi + 32], xmm2
				movntps[edi + 48], xmm3
				movntps[edi + 64], xmm4

				sub ebx, 20
				add esi, 20
				add edi, 80
				jmp $convF_loopA_16

			$convF_loopUA_16 :
			cmp ebx, 20
				jb $convF_just_last_usedMMX

				prefetchnta[esi + 24]

				movd	mm0, [esi + 0]
				movd	mm1, [esi + 4]
				movd	mm2, [esi + 8]
				movd	mm3, [esi + 12]
				movd	mm4, [esi + 16]

				punpcklbw	mm0, mm7
				punpcklbw	mm1, mm7
				punpcklbw	mm2, mm7
				punpcklbw	mm3, mm7
				punpcklbw	mm4, mm7

				movq2dq		xmm0, mm0
				movq2dq		xmm1, mm1
				movq2dq		xmm2, mm2
				movq2dq		xmm3, mm3
				movq2dq		xmm4, mm4

				punpcklwd	xmm0, xmm5
				punpcklwd	xmm1, xmm5
				punpcklwd	xmm2, xmm5
				punpcklwd	xmm3, xmm5
				punpcklwd	xmm4, xmm5

				cvtdq2ps xmm0, xmm0
				cvtdq2ps xmm1, xmm1
				cvtdq2ps xmm2, xmm2
				cvtdq2ps xmm3, xmm3
				cvtdq2ps xmm4, xmm4

				mulps	xmm0, xmm6
				mulps	xmm1, xmm6
				mulps	xmm2, xmm6
				mulps	xmm3, xmm6
				mulps	xmm4, xmm6

				addps	xmm0, xmm7
				addps	xmm1, xmm7
				addps	xmm2, xmm7
				addps	xmm3, xmm7
				addps	xmm4, xmm7

				movups[edi + 0], xmm0
				movups[edi + 16], xmm1
				movups[edi + 32], xmm2
				movups[edi + 48], xmm3
				movups[edi + 64], xmm4

				sub ebx, 20
				add esi, 20
				add edi, 80
				jmp $convF_loopUA_16

			$convF_just_last_usedMMX :
			emms

			$convF_just_last :
			cmp ebx, 0
				je $convF_finalEnd

				fld		addVal
				fld		mulVal

			$convF_just_last_do1 :
			cmp		ebx, 0
				je		$convF_just_last_do1_end

				movzx	eax, BYTE PTR[esi]
				mov		DWORD PTR[auxBuffer], eax
				fild[auxBuffer]
				fmul	st(0), st(1)
				fadd	st(0), st(2)
				fstp[edi]

				sub		ebx, 1
				add		esi, 1
				add		edi, 4
				jmp		$convF_just_last_do1

			$convF_just_last_do1_end :
			ffreep st(0)
				ffreep st(0)

			$convF_finalEnd :
		}

#endif

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
}

#endif