#ifdef HR_VS_MEMORY_LEAKS
	#define _CRTDBG_MAP_ALLOC
	#define _CRTDBG_MAPALLOC
	#include <stdlib.h>
	#include <crtdbg.h>
#endif

#include "common/platform.hpp"
#include "common/stringUtils.hpp"

#include "engine/engine.hpp"

#include "common/Primitives2D.hpp"

int WINAPI wWinMain(HINSTANCE hInst, HINSTANCE hInstPrev, PWSTR lpCmdLine, int nCmdShow)
{
	UNREFERENCED_PARAMETER(hInstPrev); 
	UNREFERENCED_PARAMETER(nCmdShow);

#ifdef HR_VS_MEMORY_LEAKS
	_CrtSetDbgFlag ( _CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF );
#endif

	//minimal checks
	{
		if (HorseRadish::Platform::cpuCheckFeatures((HorseRadish::Platform::CPUFeature)(HorseRadish::Platform::SSE | HorseRadish::Platform::SSE2 | HorseRadish::Platform::CMov)) == false)
		{
			Window::MsgBoxWarn("The CPU doesn't have the minimum required features.\nThe application cannot proceed.");
			return 0;
		}

		int isCleanBoot;
		if (!HorseRadish::Platform::systemInfo(HorseRadish::Platform::SystemInfo::CleanBoot, isCleanBoot) || !isCleanBoot)
		{
			Window::MsgBoxWarn("The OS did not boot normally!\nFor security reasons the application will now exit.");
			return 0;
		}

		if (HorseRadish::Platform::SingleInstance().isAnotherRunning() == true)
		{
			Window::MsgBoxError("Another instance of this application is already running.");
			return 0;
		}
	}

#ifndef _M_X64
	unsigned int curControlWord;

	_controlfp_s(&curControlWord, _PC_24, _MCW_PC);	//set precision control to 24bit
	_controlfp_s(&curControlWord, _DN_FLUSH, _MCW_DN); //convert denorms to zero
	_controlfp_s(&curControlWord, _RC_NEAR, _MCW_RC); //round to nearest (e.g.: 1.5 > 2.0)
	_controlfp_s(&curControlWord, _EM_INVALID|_EM_DENORMAL|_EM_ZERODIVIDE|_EM_OVERFLOW|_EM_UNDERFLOW|_EM_INEXACT, _MCW_EM); //ignore exceptions
	_clearfp(); //clear previous exceptions
#endif

	auto cmdLine = HorseRadish::StringUtils::conv2UTF8(lpCmdLine);
	
	HorseRadish::Engine::Engine engine(cmdLine);
	auto success = engine.mainLoop();

#ifndef _M_X64
	_controlfp_s(&curControlWord, _CW_DEFAULT, 0xfffff);
#endif

	if (!success)
	{
		Window::MsgBoxError("Unable to start engine (invalid state call)");
		return 0;
	}

	auto engineError = engine.getErrorDesc();
	if (!engineError.empty())
		Window::MsgBoxError(engineError.c_str());

	switch (engine.getExitAction())
	{
	case HorseRadish::Engine::Engine::ExitAction::Restart:
		HorseRadish::Platform::spawnSelf();
		break;
	default:
		break;
	}

	return engine.getExitCode();	
}