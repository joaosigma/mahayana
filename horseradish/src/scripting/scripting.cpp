#include "Scripting.hpp"

#include <common\UTF.hpp>
#include <common\String.hpp>
#include <common\FileSystem.hpp>

namespace HorseRadish
{
	Console::Console* Scripting::console = nullptr;
	bool Scripting::initialized = false;

	HSQUIRRELVM squirrelVM;

	void Scripting::Initialize(Console::Console *console, const bool devMode)
	{
		if (Scripting::initialized)
			return;

		Scripting::console = console;

		squirrelVM = sq_open(1024);

		Scripting::initialized = true;
	}

	void Scripting::Terminate()
	{
		if (Scripting::initialized == false)
			return;

		sq_close(squirrelVM);
		squirrelVM = nullptr;

		Scripting::console = nullptr;
		Scripting::initialized = false;
	}

	bool Scripting::ExecuteScript(const char * const script)
	{
		if (script == nullptr || script[0] == '\0')
			return true;


		return false;
	}

} //HorseRadish