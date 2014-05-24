#include "UTF.hpp"
#include "Platform.hpp"
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

} redirectData = { { nullptr, nullptr }, { nullptr, nullptr }, { nullptr, nullptr }, -1, -1, -1, false };;

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

//preciso de inicializar estas contantes
const HorseRadish::hChar Platform::NewLine[] = "\r\n\0";
const int Platform::NewLineSize = 2;

const unsigned int Platform::DirectorySeparatorChar = '\\';
const unsigned int Platform::VolumeSeparatorChar = ':';

const int Platform::KiloByte = 1024;
const int Platform::MegaByte = 1048576;
const int Platform::GigaByte = 1073741824;

Platform::SingleInstance::SingleInstance()
{
	//tento criar um mutex e verifico se ele foi criado
	this->globalData = CreateMutex(NULL, FALSE, L"HorseRadish global mutex");
	this->isAnotherRunning = (GetLastError() == ERROR_ALREADY_EXISTS);
}

Platform::SingleInstance::~SingleInstance()
{
	//se for preciso limpar alguma coisa
	if (this->globalData != nullptr)
		CloseHandle((HANDLE)this->globalData);

	//limpo tudo
	this->globalData = NULL;
	this->isAnotherRunning = false;
}

bool Platform::SetProcessPriority(const PriorityType &priorityType)
{
	HANDLE hProcess;

	//o processo actual
	hProcess = GetCurrentProcess();

	//conforme o tipo de prioridade
	if (priorityType == Platform::Normal)
		return (::SetPriorityClass(hProcess, NORMAL_PRIORITY_CLASS) == TRUE);
	if (priorityType == Platform::High)
		return (::SetPriorityClass(hProcess, ABOVE_NORMAL_PRIORITY_CLASS) == TRUE);
	if (priorityType == Platform::Highest)
		return (::SetPriorityClass(hProcess, HIGH_PRIORITY_CLASS) == TRUE);

	//chegando aqui alguma coisa correu mal
	return false;
}

bool Platform::SetThreadPriority(const PriorityType &priorityType)
{
	HANDLE hThread;

	//a thread actual
	hThread = GetCurrentThread();

	//conforme o tipo de prioridade
	if (priorityType == Platform::Normal)
		return (::SetThreadPriority(hThread, THREAD_PRIORITY_NORMAL) == TRUE);
	if (priorityType == Platform::High)
		return (::SetThreadPriority(hThread, THREAD_PRIORITY_ABOVE_NORMAL) == TRUE);
	if (priorityType == Platform::Highest)
		return (::SetThreadPriority(hThread, THREAD_PRIORITY_HIGHEST) == TRUE);

	//chegando aqui alguma coisa correu mal
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
	//basta seleccionar
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

	//por omissão
	infoValue.SetEmpty();

	//inicio estas coisas
	bufferAux[0] = '\0';
	bufferAuxCharCount = sizeof(bufferAux) / sizeof(TCHAR);

	//se for para o caminho completo deste executável
	if (systemInfo == Platform::ExecutableFullPath)
	{
		DWORD resultado;

		//tento obter os dados
		resultado = GetModuleFileName(0, bufferAux, bufferAuxCharCount);
		if ((resultado == 0) || (resultado > bufferAuxCharCount))
			return false;

		//guardo na string e já tá
		infoValue.Set(HorseRadish::String::Windows, bufferAux);
		return true;
	}

	//se for para a pasta actual
	if (systemInfo == Platform::CurrentFolder)
	{
		DWORD resultado;

		//tento obter os dados
		resultado = GetCurrentDirectory(bufferAuxCharCount, bufferAux);
		if ((resultado == 0) || (resultado > bufferAuxCharCount))
			return false;

		//guardo na string e já tá
		infoValue.Set(HorseRadish::String::Windows, bufferAux);
		return true;
	}

	//se for para a pasta do sistema
	if (systemInfo == Platform::SystemFolder)
	{
		DWORD resultado;

		//tento obter os dados
		resultado = GetSystemDirectory(bufferAux, bufferAuxCharCount);
		if ((resultado == 0) || (resultado > bufferAuxCharCount))
			return false;

		//guardo na string e já tá
		infoValue.Set(HorseRadish::String::Windows, bufferAux);
		return true;
	}

	//se for para o nome do computador
	if (systemInfo == Platform::MachineName)
	{
		//tento obter os dados
		if (GetComputerName(bufferAux, &bufferAuxCharCount) == FALSE)
			return false;

		//guardo na string e já tá
		infoValue.Set(HorseRadish::String::Windows, bufferAux);
		return true;
	}

	//se for para o nome do utilizador actual
	if (systemInfo == Platform::CurrentUsername)
	{
		//tento obter os dados
		if (GetUserName(bufferAux, &bufferAuxCharCount) == FALSE)
			return false;

		//guardo na string e já tá
		infoValue.Set(HorseRadish::String::Windows, bufferAux);
		return true;
	}

	//se for para saber o nome do SO
	if (systemInfo == Platform::OperatingSystemName)
	{
		OSVERSIONINFOEX versionInfo;

		//tento obter os dados
		memset(&versionInfo, 0, sizeof(OSVERSIONINFOEX));
		versionInfo.dwOSVersionInfoSize = sizeof(OSVERSIONINFOEX);
		if (GetVersionEx((LPOSVERSIONINFOW)&versionInfo) == FALSE)
			return false;

		//conforme o tipo
		if ((versionInfo.dwMajorVersion == 6) && (versionInfo.dwMinorVersion == 1) && (versionInfo.wProductType == VER_NT_WORKSTATION))
			infoValue.SetPrintf(HorseRadish::String::UTF8, "Windows 7 (%d.%d)", versionInfo.dwMajorVersion, versionInfo.dwMinorVersion);
		else if ((versionInfo.dwMajorVersion == 6) && (versionInfo.dwMinorVersion == 0) && (versionInfo.wProductType != VER_NT_WORKSTATION))
			infoValue.SetPrintf(HorseRadish::String::UTF8, "Windows Vista (%d.%d)", versionInfo.dwMajorVersion, versionInfo.dwMinorVersion);
		else if ((versionInfo.dwMajorVersion == 5) && (versionInfo.dwMinorVersion == 1))
			infoValue.SetPrintf(HorseRadish::String::UTF8, "Windows XP (%d.%d)", versionInfo.dwMajorVersion, versionInfo.dwMinorVersion);
		else
			infoValue.SetPrintf(HorseRadish::String::UTF8, "Windows (%d.%d)", versionInfo.dwMajorVersion, versionInfo.dwMinorVersion);

		//informação acerca do SP
		if (versionInfo.wServicePackMajor > 0)
		{
			//se tiver uma versão menor
			if (versionInfo.wServicePackMinor > 0)
				infoValue += HorseRadish::String(" SP%d.%d", versionInfo.wServicePackMajor, versionInfo.wServicePackMinor);
			else
				infoValue += HorseRadish::String(" SP%d", versionInfo.wServicePackMajor);
		}

		//já tá
		return true;
	}

	//chegando aqui é mau
	return false;
}

bool Platform::GetSystemInfo(const SystemInfo &systemInfo, int &infoValue)
{
	//por omissão
	infoValue = -1;

	//se for para a memória
	if ((systemInfo == SystemInfo::MemoryTotal) || (systemInfo == SystemInfo::MemoryFree))
	{
		MEMORYSTATUS memoryStatus;

		//leio os dados
		GlobalMemoryStatus(&memoryStatus);

		//conforme o que foi pedido
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

		//nao devia vir para aqui
		return false;
	}

	//se for para o display
	if ((systemInfo == SystemInfo::DisplayWidth) || (systemInfo == SystemInfo::DisplayHeight) || (systemInfo == SystemInfo::DisplayColorBits) || (systemInfo == SystemInfo::DisplayFrequency))
	{
		DEVMODE deviceMode;

		//leio os dados
		memset(&deviceMode, 0, sizeof(DEVMODE));
		deviceMode.dmSize = sizeof(DEVMODE);
		if (EnumDisplaySettingsEx(nullptr, ENUM_REGISTRY_SETTINGS, &deviceMode, 0) == FALSE)
			return false;

		//conforme o que foi pedido
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

		//nao devia vir para aqui
		return false;
	}

	//se for para saber se o SO está normal
	if (systemInfo == SystemInfo::CleanBoot)
	{
		//basta verificar isto
		infoValue = (GetSystemMetrics(SM_CLEANBOOT) == 0) ? 1 : 0;
		return true;
	}

	//nao devia vir para aqui
	return false;
}

bool Platform::InstanciateProcess(const char * const commandLine)
{
	STARTUPINFO startInfo;
	PROCESS_INFORMATION processInfo;
	wchar_t commandLineWChar[1024];

	//se não tenho nada
	if ((commandLine == NULL) || (*commandLine == '\0'))
		return false;

	//limpar tudinho
	memset(&startInfo, 0, sizeof(STARTUPINFO));
	memset(&processInfo, 0, sizeof(PROCESS_INFORMATION));
	startInfo.cb = sizeof(STARTUPINFO);
	startInfo.lpDesktop = L"";

	//tenho de converter a linha de comandos para WideChar
	HorseRadish::UTF::ConvertUTF8To(commandLine, HorseRadish::UTF::Windows, commandLineWChar, sizeof(commandLineWChar));

	//crio de novo o processo
	CreateProcess(nullptr, commandLineWChar, nullptr, nullptr, FALSE, 0, nullptr, nullptr, &startInfo, &processInfo);

	//toca a fechar os handles
	CloseHandle(processInfo.hProcess);
	CloseHandle(processInfo.hThread);

	//correu tudo bem
	return true;
}

bool Platform::ClipboardGetStrings(std::function<bool (const HorseRadish::String &)> funcCallback)
{
	HorseRadish::String clipDataUTF8, curToken;

	//se não tenho nada
	if (funcCallback == nullptr)
		return false;

	//abro o clipboard
	if (OpenClipboard(nullptr) == FALSE)
		return false;

	//como tenho de fechar o clipboard quando sair desta função
	ScopedAction scopedAction([&]()
	{
		CloseClipboard();
	});

	//se não tenho nada no clipboard
	if (IsClipboardFormatAvailable(CF_UNICODETEXT) == FALSE)
		return false;

	//tiro o texto
	auto clipData = GetClipboardData(CF_UNICODETEXT);
	if (clipData == nullptr)
		return false;

	//converto para UTF8
	clipDataUTF8.Set(HorseRadish::String::Encoding::Windows, clipData);

	//os dados consistem de várias linhas separadas por \r\n
	for(HorseRadish::String::Tokenizer tokenizer(clipDataUTF8, '\n'); tokenizer.IsLast() == false; )
	{
		//tiro o token e se não tenho nada
		tokenizer.Read(curToken);
		if (curToken.GetSizeBytes() == 0)
			continue;

		//devolvo este token e se não é preciso continuar
		if (funcCallback(curToken) == false)
			break;
	}

	//chegando aqui havia dados no clipboard
	return true;
}

bool Platform::ClipboardGetFiles(std::function<bool (const HorseRadish::String &)> funcCallback)
{
	HorseRadish::String curFileUTF8;
	wchar_t fileBufferWChar[512];
	int numFiles;

	//se não tenho nada
	if (funcCallback == nullptr)
		return false;

	//abro o clipboard
	if (OpenClipboard(nullptr) == FALSE)
		return false;

	//como tenho de fechar o clipboard quando sair desta função
	ScopedAction scopedAction([&]()
	{
		CloseClipboard();
	});

	//se não tenho nada no clipboard
	if (IsClipboardFormatAvailable(CF_HDROP) == FALSE)
		return false;

	//tiro os dados
	auto clipData = (HDROP)GetClipboardData(CF_HDROP);
	if (clipData == nullptr)
		return false;

	//quantos ficheiros tenho
	numFiles = DragQueryFile(clipData, 0xFFFFFFFF, NULL, 0);
	if (numFiles <= 0)
		return false;

	//para cada ficheiro
	for(int i = 0; i < numFiles; i++)
	{
		//tiro o ficheiro
		if (DragQueryFile(clipData, i, fileBufferWChar, sizeof(fileBufferWChar) / sizeof(wchar_t)) == 0)
			continue;

		//converto o ficheiro
		curFileUTF8.Set(HorseRadish::String::Encoding::Windows, fileBufferWChar);

		//devolvo este ficheiro e se não é preciso continuar
		if (funcCallback(curFileUTF8) == false)
			break;
	}

	//chegando aqui havia dados no clipboard
	return true;
}

bool Platform::StdInOutErrRedirect()
{
	if (redirectData.redirected)
		return true;

	//close anything previous
	Platform::StdInOutErrClose();

	//create a pipe for stdout, stdin and stderr
	if (CreatePipe(&redirectData.pipeIn.read, &redirectData.pipeIn.write, nullptr, 0) == 0)
		return false;
	if (CreatePipe(&redirectData.pipeOut.read, &redirectData.pipeOut.write, nullptr, 0) == 0)
		return false;
	if (CreatePipe(&redirectData.pipeErr.read, &redirectData.pipeErr.write, nullptr, 0) == 0)
		return false;
	
	//convert the corresponding pipe handle to an OS handle
	redirectData.osHandlePipeIn = _open_osfhandle((intptr_t)redirectData.pipeIn.read, _O_RDONLY);
	if (redirectData.osHandlePipeIn == -1)
		return false;
	redirectData.osHandlePipeOut = _open_osfhandle((intptr_t)redirectData.pipeOut.write, 0);
	if (redirectData.osHandlePipeOut == -1)
		return false;
	redirectData.osHandlePipeErr = _open_osfhandle((intptr_t)redirectData.pipeErr.write, 0);
	if (redirectData.osHandlePipeErr == -1)
		return false;

	//redirecting
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