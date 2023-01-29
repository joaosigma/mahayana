#include "engine/engine.hpp"
#include "platform/platform.hpp"
#include "common/stringUtils.hpp"

int WINAPI wWinMain(_In_ HINSTANCE, _In_opt_ HINSTANCE, _In_ PWSTR lpCmdLine, _In_ int)
{
	//architecture assumptions
	static_assert(sizeof(unsigned char) == 1);
	static_assert(sizeof(unsigned short) == 2);
	static_assert(sizeof(unsigned int) == 4);
	static_assert(sizeof(float) == 4);
	static_assert(sizeof(double) == 8);

	//minimal checks
	{
		if (!hr::platform::Platform::cpuCheckFeatures((hr::platform::Platform::CPUFeature)(hr::platform::Platform::SSE | hr::platform::Platform::SSE2 | hr::platform::Platform::CMov)))
		{
			hr::platform::Window::MsgBoxWarn("The CPU doesn't have the minimum required features.\nThe application cannot proceed.");
			return 0;
		}

		if (hr::platform::Platform::systemInfoInt(hr::platform::Platform::SystemInfo::CleanBoot).value_or(0) == 0)
		{
			hr::platform::Window::MsgBoxWarn("The OS did not boot normally!\nFor security reasons the application will now exit.");
			return 0;
		}

		if (hr::platform::Platform::SingleInstance().isAnotherRunning())
		{
			hr::platform::Window::MsgBoxError("Another instance of this application is already running.");
			return 0;
		}
	}

#if !defined(_M_X64)
	unsigned int curControlWord;

	_controlfp_s(&curControlWord, _PC_24, _MCW_PC);	//set precision control to 24bit
	_controlfp_s(&curControlWord, _DN_FLUSH, _MCW_DN); //convert denorms to zero
	_controlfp_s(&curControlWord, _RC_NEAR, _MCW_RC); //round to nearest (e.g.: 1.5 > 2.0)
	_controlfp_s(&curControlWord, _EM_INVALID | _EM_DENORMAL | _EM_ZERODIVIDE | _EM_OVERFLOW | _EM_UNDERFLOW | _EM_INEXACT, _MCW_EM); //ignore exceptions
	_clearfp(); //clear previous exceptions
#endif

	auto cmdLine = hr::StringUtils::conv2UTF8(lpCmdLine);

	hr::engine::Engine engine(cmdLine);
	auto success = engine.mainLoop();

#if !defined(_M_X64)
	_controlfp_s(&curControlWord, _CW_DEFAULT, 0xfffff);
#endif

	if (!success)
	{
		hr::platform::Window::MsgBoxError("Unable to start engine (invalid state call)");
		return 0;
	}

	auto engineError = engine.getErrorDesc();
	if (!engineError.empty())
		hr::platform::Window::MsgBoxError(engineError.c_str());

	switch (engine.getExitAction())
	{
	case hr::engine::Engine::ExitAction::Restart:
		hr::platform::Platform::spawnSelf();
		break;
	default:
		break;
	}

	return engine.getExitCode();
}
