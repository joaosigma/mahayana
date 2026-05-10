module;

#include <cassert>
#include <fcntl.h>
#include <intrin.h>
#include <io.h>

// clang-format off
#include <windows.h>
#include <shellapi.h>
// clang-format on

#if !defined(_WIN64)
#error "Only x64 (Win64) builds are supported"
#endif

module core:platform.impl;

import std;
import :platform;
import :stringUtils;
import :scopedAction;

namespace hr::platform
{
    namespace
    {
        struct RedirectData
        {
            struct PipeHandles
            {
                HANDLE read, write;
            } pipeIn, pipeOut, pipeErr;

            int osHandlePipeIn, osHandlePipeOut, osHandlePipeErr;
            bool redirected;

        } redirectData = {{nullptr, nullptr}, {nullptr, nullptr}, {nullptr, nullptr}, -1, -1, -1, false};

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

        int pipeRead(HANDLE pipeHandle, void* outBuffer, const int outBufferSize)
        {
            DWORD bytesRead, bytesAvailable;

            PeekNamedPipe(pipeHandle, nullptr, 0, nullptr, &bytesAvailable, nullptr);
            if (bytesAvailable <= 0)
                return 0;

            if (ReadFile(pipeHandle, outBuffer, outBufferSize, &bytesRead, nullptr) != TRUE)
                return -1;

            return bytesRead;
        }
    }

    const std::string_view Platform::NewLine{"\r\n"};

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

    std::optional<std::string> Platform::cpuGetVendorID()
    {
        int cpuInfo[4];
        char cpuString[128];

        std::memset(cpuInfo, 0, sizeof(cpuInfo));
        __cpuid(cpuInfo, 0x0);

        std::memset(cpuString, 0, sizeof(cpuString));
        std::memcpy(cpuString + 0, cpuInfo + 1, sizeof(int));
        std::memcpy(cpuString + 4, cpuInfo + 3, sizeof(int));
        std::memcpy(cpuString + 8, cpuInfo + 2, sizeof(int));

        return std::string{cpuString};
    }

    std::optional<std::string> Platform::cpuGetProcessorName()
    {
        int cpuInfo[4];
        std::memset(cpuInfo, 0, sizeof(cpuInfo));
        __cpuid(cpuInfo, 0x80000000);

        if (cpuInfo[0] < 0x80000004)
            return {};

        char cpuString[128];
        std::memset(cpuString, 0, sizeof(cpuString));

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

        return hr::StringUtils::trimCopy(cpuString);
    }

    bool Platform::cpuCheckFeatures(CPUFeature featuresCheck)
    {
        int cpuInfo[4];

        std::memset(cpuInfo, 0, sizeof(cpuInfo));
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

    std::optional<std::string> Platform::systemInfoStr(SystemInfo systemInfo)
    {
        switch (systemInfo)
        {
            case Platform::SystemInfo::ExecutableFullPath:
            {
                std::array<TCHAR, 16338> bufferAux;

                auto result = GetModuleFileName(0, bufferAux.data(), bufferAux.size());
                if (result <= 0)
                    return {};

                return hr::StringUtils::conv2UTF8({bufferAux.data(), result});
            }

            case Platform::SystemInfo::MachineName:
            {
                std::array<TCHAR, 16338> bufferAux;
                DWORD bufferAuxCharCount;

                bufferAuxCharCount = bufferAux.size();
                if (GetComputerName(bufferAux.data(), &bufferAuxCharCount) == FALSE)
                    return {};

                return hr::StringUtils::conv2UTF8({bufferAux.data(), bufferAuxCharCount});
            }

            case Platform::SystemInfo::CurrentUsername:
            {
                std::array<TCHAR, 16338> bufferAux;
                DWORD bufferAuxCharCount;

                bufferAuxCharCount = bufferAux.size();
                if (GetUserName(bufferAux.data(), &bufferAuxCharCount) == FALSE)
                    return {};

                return hr::StringUtils::conv2UTF8({bufferAux.data(), bufferAuxCharCount});
            }

            case Platform::SystemInfo::OperatingSystemName:
            {
                OSVERSIONINFOEX versionInfo;

                std::memset(&versionInfo, 0, sizeof(OSVERSIONINFOEX));
                versionInfo.dwOSVersionInfoSize = sizeof(OSVERSIONINFOEX);
                if (GetVersionEx((LPOSVERSIONINFOW)&versionInfo) == FALSE)
                    return {};

                std::string name;
                if ((versionInfo.dwMajorVersion == 6) && (versionInfo.dwMinorVersion == 1) && (versionInfo.wProductType == VER_NT_WORKSTATION))
                    name = std::format("Windows 7 ({0}.{1})", versionInfo.dwMajorVersion, versionInfo.dwMinorVersion);
                else if ((versionInfo.dwMajorVersion == 6) && (versionInfo.dwMinorVersion == 0) && (versionInfo.wProductType != VER_NT_WORKSTATION))
                    name = std::format("Windows Vista ({0}.{1})", versionInfo.dwMajorVersion, versionInfo.dwMinorVersion);
                else if ((versionInfo.dwMajorVersion == 5) && (versionInfo.dwMinorVersion == 1))
                    name = std::format("Windows XP ({0}.{1})", versionInfo.dwMajorVersion, versionInfo.dwMinorVersion);
                else
                    name = std::format("Windows ({0}.{1})", versionInfo.dwMajorVersion, versionInfo.dwMinorVersion);

                if (versionInfo.wServicePackMajor > 0)
                {
                    if (versionInfo.wServicePackMinor > 0)
                        name += std::format(" SP{0}.{1}", versionInfo.wServicePackMajor, versionInfo.wServicePackMinor);
                    else
                        name += std::format(" SP{0}", versionInfo.wServicePackMajor);
                }

                return name;
            }
            default:
                break;
        }

        return {};
    }

    std::optional<int64_t> Platform::systemInfoInt(SystemInfo systemInfo)
    {
        switch (systemInfo)
        {
            case SystemInfo::MemoryTotal:
            case SystemInfo::MemoryFree:
            {
                MEMORYSTATUSEX memoryStatus;
                memoryStatus.dwLength = sizeof(memoryStatus);
                GlobalMemoryStatusEx(&memoryStatus);

                switch (systemInfo)
                {
                    case SystemInfo::MemoryTotal:
                        return memoryStatus.ullTotalPhys;
                    case SystemInfo::MemoryFree:
                        return memoryStatus.ullAvailPhys;
                    default:
                        break;
                }

                return {};
            }

            case SystemInfo::DisplayWidth:
            case SystemInfo::DisplayHeight:
            case SystemInfo::DisplayColorBits:
            case SystemInfo::DisplayFrequency:
            {
                DEVMODE deviceMode;

                std::memset(&deviceMode, 0, sizeof(DEVMODE));
                deviceMode.dmSize = sizeof(DEVMODE);
                if (EnumDisplaySettingsEx(nullptr, ENUM_REGISTRY_SETTINGS, &deviceMode, 0) == FALSE)
                    return {};

                switch (systemInfo)
                {
                    case SystemInfo::DisplayWidth:
                        return deviceMode.dmPelsWidth;
                    case SystemInfo::DisplayHeight:
                        return deviceMode.dmPelsHeight;
                    case SystemInfo::DisplayColorBits:
                        return deviceMode.dmBitsPerPel;
                    case SystemInfo::DisplayFrequency:
                        return deviceMode.dmDisplayFrequency;
                    default:
                        break;
                }

                return {};
            }

            case SystemInfo::CleanBoot:
                return (GetSystemMetrics(SM_CLEANBOOT) == 0) ? 1 : 0;

            default:
                break;
        }

        return {};
    }

    bool Platform::spawnSelf()
    {
        STARTUPINFO startInfo;
        PROCESS_INFORMATION processInfo;

        wchar_t szFileName[MAX_PATH];
        if (!GetModuleFileName(nullptr, szFileName, MAX_PATH))
            return false;

        std::memset(&startInfo, 0, sizeof(STARTUPINFO));
        std::memset(&processInfo, 0, sizeof(PROCESS_INFORMATION));
        startInfo.cb = sizeof(STARTUPINFO);

        CreateProcess(nullptr, szFileName, nullptr, nullptr, FALSE, NORMAL_PRIORITY_CLASS, nullptr, nullptr, &startInfo, &processInfo);

        CloseHandle(processInfo.hProcess);
        CloseHandle(processInfo.hThread);

        return true;
    }

    bool Platform::clipboardGetStrings(const std::function<bool(std::string)>& funcCallback)
    {
        if (!funcCallback)
            return false;

        if (OpenClipboard(nullptr) == FALSE)
            return false;

        ScopedAction scopedAction([&]() { CloseClipboard(); });

        if (IsClipboardFormatAvailable(CF_UNICODETEXT) == FALSE)
            return false;

        auto clipData = GetClipboardData(CF_UNICODETEXT);
        if (clipData == nullptr)
            return false;

        auto clipDataUTF8 = hr::StringUtils::conv2UTF8(static_cast<const wchar_t*>(clipData));
        funcCallback(std::move(clipDataUTF8));

        return true;
    }

    bool Platform::clipboardGetFiles(const std::function<bool(std::string)>& funcCallback)
    {
        if (!funcCallback)
            return false;

        if (OpenClipboard(nullptr) == FALSE)
            return false;

        ScopedAction scopedAction([&]() { CloseClipboard(); });

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
            if (!funcCallback(std::move(curFileUTF8)))
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

        if (_dup2(redirectData.osHandlePipeIn, 0) != 0) // stdin
            return false;
        if (_dup2(redirectData.osHandlePipeOut, 1) != 0) // stdout
            return false;
        if (_dup2(redirectData.osHandlePipeErr, 2) != 0) // stderr
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

        std::memset(&redirectData, 0, sizeof(RedirectData));
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

    bool Platform::stdErrRead(void* outBuffer, int outBufferSize, int& bytesWritten)
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

    bool Platform::stdOutRead(void* outBuffer, int outBufferSize, int& bytesWritten)
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

    std::optional<int> Platform::execute(std::string_view execPathArgs, std::optional<std::string>& output)
    {
        if (execPathArgs.empty())
            return std::nullopt;

        // create pipes to read stdout and stderr

        HANDLE stdOutErrRead{nullptr};
        HANDLE stdOutErrWrite{nullptr};
        if (output.has_value())
        {
            SECURITY_ATTRIBUTES sa;
            sa.nLength = sizeof(SECURITY_ATTRIBUTES);
            sa.bInheritHandle = TRUE;
            sa.lpSecurityDescriptor = NULL;

            if (!CreatePipe(&stdOutErrRead, &stdOutErrWrite, &sa, 0))
                return std::nullopt;

            if (!SetHandleInformation(stdOutErrRead, HANDLE_FLAG_INHERIT, 0))
                return std::nullopt;
        }

        // run the process

        STARTUPINFO startInfo;
        PROCESS_INFORMATION processInfo;
        {
            std::memset(&startInfo, 0, sizeof(STARTUPINFO));
            std::memset(&processInfo, 0, sizeof(PROCESS_INFORMATION));
            startInfo.cb = sizeof(STARTUPINFO);
            startInfo.dwFlags = STARTF_USESHOWWINDOW;
            startInfo.wShowWindow = SW_HIDE;
            if (stdOutErrWrite)
            {
                startInfo.hStdError = stdOutErrWrite;
                startInfo.hStdOutput = stdOutErrWrite;
                startInfo.dwFlags |= STARTF_USESTDHANDLES;
            }

            {
                auto nativePathArgs = hr::StringUtils::conv2Native(execPathArgs);

                auto nativePathArgsWritable = std::make_unique_for_overwrite<wchar_t[]>(nativePathArgs.size() + 1);
                std::memcpy(nativePathArgsWritable.get(), nativePathArgs.data(), sizeof(wchar_t) * nativePathArgs.size());
                nativePathArgsWritable[nativePathArgs.size()] = '\0';

                auto res = CreateProcess(nullptr, nativePathArgsWritable.get(), nullptr, nullptr, TRUE, 0, nullptr, nullptr, &startInfo, &processInfo);

                if (stdOutErrWrite)
                    CloseHandle(stdOutErrWrite);

                if (!res)
                {
                    if (stdOutErrRead)
                        CloseHandle(stdOutErrRead);

                    return std::nullopt;
                }
            }

            WaitForSingleObject(processInfo.hProcess, INFINITE);
        }

        std::optional<int> exitCode;
        {
            DWORD nativeExitCode;
            if (GetExitCodeProcess(processInfo.hProcess, &nativeExitCode))
                exitCode = static_cast<int>(nativeExitCode);
        }

        // read output from process
        if (stdOutErrRead)
        {
            std::array<CHAR, 4096> buffer;
            while (true)
            {
                DWORD bytesRead;

                auto res = ReadFile(stdOutErrRead, buffer.data(), buffer.size(), &bytesRead, NULL);
                if (!res || (bytesRead == 0))
                    break;

                assert(output.has_value());
                (*output).append(buffer.data(), bytesRead);
            }

            CloseHandle(stdOutErrRead);
        }

        // all done

        CloseHandle(processInfo.hProcess);
        CloseHandle(processInfo.hThread);

        return exitCode;
    }
}
