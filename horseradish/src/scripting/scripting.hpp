#pragma once

#include "console\console.hpp"

#include <libs\squirrel\squirrel.h>

namespace HorseRadish
{
	class Scripting
	{
	public:
		static Console::Console *console;
		static bool initialized;

	public:
		static void Initialize(Console::Console *console, const bool devMode = false);
		static void Terminate();

		static bool ExecuteScript(const char * const script);
	};

} //HorseRadish
