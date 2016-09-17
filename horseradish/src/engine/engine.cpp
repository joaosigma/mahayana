#include "engine.hpp"

#include "common\opengl\openGL.hpp"
#include "..\build.hpp"

#include <libs\cppformat\format.h>

#include <cassert>

namespace HorseRadish { namespace Engine {

	void Engine::exit(ExitAction exitAction, const char * const errorDesc)
	{
		std::lock_guard<std::mutex> lock(mSyncLock);

		if ((mCurState != State::Initializing) && (mCurState != State::Running))
			return;

		mCurState = (mCurState == State::Initializing) ? (errorDesc ? State::StoppedError : State::Stopped) : State::Stopping;
		mExitAction = exitAction;

		if (errorDesc)
		{
			mErrorDesc = errorDesc;
			mLoggerRuntimeCtx->error(errorDesc);
		}

		if (mWindow)
			mWindow->sendMessageClose();
	}

	void Engine::initFileSystem()
	{
		mFileSystem = std::make_shared<HorseRadish::IO::FileSystem>(10);

		//mount current and previous directory
		auto currentFolder = HorseRadish::IO::Path(HorseRadish::IO::Path::KnownPath::CurrentFolder);
		mFileSystem->MountPath(currentFolder, nullptr);
		currentFolder.RemoveLastComponent();
		mFileSystem->MountPath(currentFolder, nullptr);

		//mount main game resource directory
		mFileSystem->MountPath(HorseRadish::IO::Path("d:/jogos/doom3/base/"), nullptr);
		mLoggerRuntimeCtx->info("${olive}->${default}Path set to: \"d:/jogos/doom3/base/\"");

		size_t totalFich = 0;
		size_t totalPacks = 0;

		//for every pack/zip/7zip file
		HorseRadish::IO::FileSystem::FindFiles("d:/jogos/doom3/base/pak*.pk4", true, [&](const HorseRadish::IO::Path &filePath, const HorseRadish::hUInt64 &fileSize)
		{
			size_t numFilesZip;

			//mount the zip file as a directoty
			if (mFileSystem->MountZip(filePath, nullptr, &numFilesZip))
			{
				totalPacks += 1;
				totalFich += numFilesZip;
			}
		});

		mLoggerRuntimeCtx->info("   loaded {0} archives with a total of {1} files", totalPacks, totalFich);
	}

	void Engine::runtimeFuncVarCreate(const std::string &funcName, Runtime::FunctionReturnContext &ctx)
	{
		if (ctx.getNumParams() != 3 && ctx.getNumParams() != 4)
		{
			ctx.throwError("Invalid number of arguments");
			return;
		}

		std::string varName, varDesc, varFormat;

		if (!ctx.getParamValue(0, varName))
		{
			ctx.throwError("Invalid var name");
			return;
		}

		if (mVars.hasData(varName.c_str()))
		{
			ctx.throwError("Var name already in use");
			return;
		}

		ctx.getParamValue(2, varDesc);
		ctx.getParamValue(3, varFormat);

		switch (ctx.getParamType(1))
		{
		case Runtime::FunctionContext::ParamType::Bool:
			mVars.addData(varName.c_str(), std::make_shared<VariableBool>(ctx.getParamValue<bool>(1), varDesc));
			return;
		case Runtime::FunctionContext::ParamType::Float:
			mVars.addData(varName.c_str(), std::make_shared<VariableDouble>(ctx.getParamValue<float>(1), varDesc));
			return;
		case Runtime::FunctionContext::ParamType::Integer:
			mVars.addData(varName.c_str(), std::make_shared<VariableInt>(ctx.getParamValue<int>(1), varDesc));
			return;
		case Runtime::FunctionContext::ParamType::String:
			mVars.addData(varName.c_str(), std::make_shared<VariableString>(ctx.getParamValue<std::string>(1), varDesc));
			return;
		default:
			break;
		}

		ctx.throwError("Unsupported var type");
	}

	void Engine::runtimeFuncVarGet(const std::string &funcName, Runtime::FunctionReturnContext &ctx)
	{
		std::string varName = ctx.getParamValue<std::string>(0);

		std::shared_ptr<IVariable> varData;
		if (!mVars.findData(varName.c_str(), varData) || !varData)
		{
			ctx.setReturnValue();
			return;
		}

		switch (varData->type())
		{
		case IVariable::VariableType::Bool:
			ctx.setReturnValue(std::static_pointer_cast<VariableBool>(varData)->value());
			return;
		case IVariable::VariableType::Integer:
			ctx.setReturnValue(static_cast<int>(std::static_pointer_cast<VariableInt>(varData)->value()));
			return;
		case IVariable::VariableType::Double:
			ctx.setReturnValue(static_cast<float>(std::static_pointer_cast<VariableDouble>(varData)->value()));
			return;
		case IVariable::VariableType::String:
			ctx.setReturnValue(std::static_pointer_cast<VariableString>(varData)->value());
			return;
		default:
			break;
		}

		ctx.setReturnValue();
	}

	void Engine::runtimeFuncVarSet(const std::string &funcName, Runtime::FunctionReturnContext &ctx)
	{
		std::string varName = ctx.getParamValue<std::string>(0);

		std::shared_ptr<IVariable> varData;
		if (!mVars.findData(varName.c_str(), varData) || !varData)
			return;

		switch (varData->type())
		{
		case IVariable::VariableType::Bool:
		{
			if (ctx.getParamType(1) != Runtime::FunctionContext::ParamType::Bool)
				ctx.throwError("Invalid value type (expected bool)");
			else
				std::static_pointer_cast<VariableBool>(varData)->value(ctx.getParamValue<bool>(1));
		}
		return;

		case IVariable::VariableType::Integer:
		{
			if (ctx.getParamType(1) != Runtime::FunctionContext::ParamType::Integer)
				ctx.throwError("Invalid value type (expected integer)");
			else
				std::static_pointer_cast<VariableInt>(varData)->value(ctx.getParamValue<int>(1));
		}
		return;

		case IVariable::VariableType::Double:
		{
			if (ctx.getParamType(1) != Runtime::FunctionContext::ParamType::Float)
				ctx.throwError("Invalid value type (expected double)");
			else
				std::static_pointer_cast<VariableDouble>(varData)->value(ctx.getParamValue<float>(1));
		}
		return;

		case IVariable::VariableType::String:
		{
			if (ctx.getParamType(1) != Runtime::FunctionContext::ParamType::String)
				ctx.throwError("Invalid value type (expected string)");
			else
				std::static_pointer_cast<VariableString>(varData)->value(ctx.getParamValue<std::string>(1));
		}
		return;

		default:
			break;
		}

		ctx.throwError("Internal: var type unknown");
	}

	void Engine::runtimeFuncVarList(const std::string &funcName, Runtime::FunctionReturnContext &ctx)
	{
		std::vector<std::string> finalList;

		mVars.findAllWithKeys([&](const std::string& key, const std::shared_ptr<IVariable>& var)
		{
			finalList.push_back(key);
		});

		ctx.setReturnValue(finalList);
	}

	void Engine::runtimeFuncRuntime(const std::string &funcName, Runtime::FunctionReturnContext &ctx)
	{
		if (funcName == "runtime.quit")
		{
			exit(ExitAction::Nothing);
			return;
		}

		if (funcName == "runtime.restart")
		{
			exit(ExitAction::Restart);
			return;
		}

		if (funcName == "runtime.forceCrash")
		{
			abort();
			return;
		}
	}

	void Engine::logSysInfo()
	{
		std::string auxInfo;
		int memTotal, memFree, displayWidth, displayHeight, displayColorBits, displayFrequency;

		//misc info
		mLoggerRuntimeCtx->info("${olive}->${default}System information:");

		if (HorseRadish::Platform::CPUGetVendorID(auxInfo))
			mLoggerRuntimeCtx->info("   CPU vendor ID: {0}", auxInfo);
		if (HorseRadish::Platform::CPUGetProcessorName(auxInfo))
			mLoggerRuntimeCtx->info("   CPU processor name: {0}", auxInfo);

		
		HorseRadish::Platform::GetSystemInfo(HorseRadish::Platform::SystemInfo::MemoryTotal, memTotal);
 		mLoggerRuntimeCtx->info("   Total physical memory: {0}", HorseRadish::StringUtils::formatSize(memTotal));

		HorseRadish::Platform::GetSystemInfo(HorseRadish::Platform::SystemInfo::MemoryFree, memFree);
		mLoggerRuntimeCtx->info("   Free physical memory: {0}", HorseRadish::StringUtils::formatSize(memFree));

		HorseRadish::Platform::GetSystemInfo(HorseRadish::Platform::SystemInfo::DisplayWidth, displayWidth);
		HorseRadish::Platform::GetSystemInfo(HorseRadish::Platform::SystemInfo::DisplayHeight, displayHeight);
		HorseRadish::Platform::GetSystemInfo(HorseRadish::Platform::SystemInfo::DisplayColorBits, displayColorBits);
		HorseRadish::Platform::GetSystemInfo(HorseRadish::Platform::SystemInfo::DisplayFrequency, displayFrequency);
		mLoggerRuntimeCtx->info("   Desktop resolution: {0}x{1}x{2}@{3}", displayWidth, displayHeight, displayColorBits, displayFrequency);

		HorseRadish::Platform::GetSystemInfo(HorseRadish::Platform::SystemInfo::OperatingSystemName, auxInfo);
		mLoggerRuntimeCtx->info("   Operating system: {0}", auxInfo);

		mLoggerRuntimeCtx->info(HorseRadish::Platform::IsArch64() ? "   Build type: x86 64bit" : "   Build type: x86 32bit");

		HorseRadish::Platform::GetSystemInfo(HorseRadish::Platform::SystemInfo::MachineName, auxInfo);
		mLoggerRuntimeCtx->info("   Machine name: {0}", auxInfo);

		HorseRadish::Platform::GetSystemInfo(HorseRadish::Platform::SystemInfo::CurrentUsername, auxInfo);
		mLoggerRuntimeCtx->info("   User name: {0}", auxInfo);

		//test UTF8
		mLoggerRuntimeCtx->info("${olive}->${default}UTF8 text test:");
		mLoggerRuntimeCtx->info("   Hello!");
		mLoggerRuntimeCtx->info("   Olá!");
		mLoggerRuntimeCtx->info("   Grüß Gott");
		mLoggerRuntimeCtx->info("   Здравствуйте");
		mLoggerRuntimeCtx->info("   Γειά σου");
		mLoggerRuntimeCtx->info("   مرحبا");
	}

	Engine::Engine(const std::string &cmdLine, bool devMode)
		: mDevMode(devMode)
	{
		//initiate logger
		mLogger = std::make_shared<Logger>(10, 500, HorseRadish::IO::Path("../logs/log.txt"));
		mLoggerRenderCtx = std::make_shared<Logger::Context>(*mLogger, Logger::ModuleType::Graphics);
		mLoggerRuntimeCtx = std::make_shared<Logger::Context>(*mLogger, Logger::ModuleType::SysRuntime);

		//initiate runtime
		mRuntime = std::make_shared<Runtime>(*mLoggerRuntimeCtx);

		mRuntime->registerFunc("runtime.varCreate", std::bind(&Engine::runtimeFuncVarCreate, this, std::placeholders::_1, std::placeholders::_2));
		mRuntime->registerFunc("runtime.varGet", std::bind(&Engine::runtimeFuncVarGet, this, std::placeholders::_1, std::placeholders::_2));
		mRuntime->registerFunc("runtime.varSet", std::bind(&Engine::runtimeFuncVarSet, this, std::placeholders::_1, std::placeholders::_2));
		mRuntime->registerFunc("runtime.varList", std::bind(&Engine::runtimeFuncVarList, this, std::placeholders::_1, std::placeholders::_2));

		mRuntime->registerFunc("runtime.exec", std::bind(&Engine::runtimeFuncRuntime, this, std::placeholders::_1, std::placeholders::_2));
		mRuntime->registerFunc("runtime.quit", std::bind(&Engine::runtimeFuncRuntime, this, std::placeholders::_1, std::placeholders::_2));
		mRuntime->registerFunc("runtime.restart", std::bind(&Engine::runtimeFuncRuntime, this, std::placeholders::_1, std::placeholders::_2));

#ifndef NDEBUG
		mRuntime->registerFunc("runtime.forceCrash", std::bind(&Engine::runtimeFuncRuntime, this, std::placeholders::_1, std::placeholders::_2));
#endif

		mRuntime->runScriptFile("../engine.runtime.nut");

		mStats[StatSampleType::Fps] = std::unique_ptr<StatSeries<3000>>(new StatSeries<3000>(" FPS"));
		mStats[StatSampleType::NumTris] = std::unique_ptr<StatSeries<3000>>(new StatSeries<3000>(" Ktris/s"));

		//some vars can already be set
		{
			std::string strAux;

			if (HorseRadish::Platform::CPUGetVendorID(strAux))
				var<std::string>("sys.info.cpuVendor", strAux);
			if (HorseRadish::Platform::CPUGetProcessorName(strAux))
				var<std::string>("sys.info.cpuName", strAux);
			var<std::string>("sys.info.build", fmt::format("Horseradish v1.0.0 (alpha build {0})", HorseRadish::Build::BuildNumber));
		}

		//first entries
		mLoggerRuntimeCtx->info("====== Engine created!");
		mLoggerRuntimeCtx->info("======   build: " __DATE__ " " __TIME__);
		mLoggerRuntimeCtx->info("");
	}

	Engine::~Engine()
	{
		mRuntime.reset();

		mLoggerRuntimeCtx.reset();
		mLoggerRenderCtx.reset();
		mLogger.reset();
	}

	template<>
	bool Engine::var<bool>(const char* const name) const
	{
		auto var = mVars.getData(name);
		assert(var && var->isType(IVariable::VariableType::Bool));

		return std::static_pointer_cast<VariableBool>(var)->value();
	}

	template<>
	int32_t Engine::var<int32_t>(const char* const name) const
	{
		auto var = mVars.getData(name);
		assert(var && var->isType(IVariable::VariableType::Integer));

		return static_cast<int32_t>(std::static_pointer_cast<VariableInt>(var)->value());
	}

	template<>
	int64_t Engine::var<int64_t>(const char* const name) const
	{
		auto var = mVars.getData(name);
		assert(var && var->isType(IVariable::VariableType::Integer));

		return std::static_pointer_cast<VariableInt>(var)->value();
	}

	template<>
	float Engine::var<float>(const char* const name) const
	{
		auto var = mVars.getData(name);
		assert(var && var->isType(IVariable::VariableType::Double));

		return static_cast<float>(std::static_pointer_cast<VariableDouble>(var)->value());
	}

	template<>
	double Engine::var<double>(const char* const name) const
	{
		auto var = mVars.getData(name);
		assert(var && var->isType(IVariable::VariableType::Double));

		return std::static_pointer_cast<VariableDouble>(var)->value();
	}

	template<>
	std::string Engine::var<std::string>(const char* const name) const
	{
		auto var = mVars.getData(name);
		assert(var && var->isType(IVariable::VariableType::String));

		return std::static_pointer_cast<VariableString>(var)->value();
	}

	template<>
	void Engine::var(const char* const name, const bool& value)
	{
		auto var = mVars.getData(name);
		assert(var && var->isType(IVariable::VariableType::Bool));

		std::static_pointer_cast<VariableBool>(var)->value(value);
	}

	template<>
	void Engine::var(const char* const name, const int32_t& value)
	{
		auto var = mVars.getData(name);
		assert(var && var->isType(IVariable::VariableType::Integer));

		std::static_pointer_cast<VariableInt>(var)->value(value);
	}

	template<>
	void Engine::var(const char* const name, const int64_t& value)
	{
		auto var = mVars.getData(name);
		assert(var && var->isType(IVariable::VariableType::Integer));

		std::static_pointer_cast<VariableInt>(var)->value(value);
	}

	template<>
	void Engine::var(const char* const name, const float& value)
	{
		auto var = mVars.getData(name);
		assert(var && var->isType(IVariable::VariableType::Double));

		std::static_pointer_cast<VariableDouble>(var)->value(value);
	}

	template<>
	void Engine::var(const char* const name, const double& value)
	{
		auto var = mVars.getData(name);
		assert(var && var->isType(IVariable::VariableType::Double));

		std::static_pointer_cast<VariableDouble>(var)->value(value);
	}

	template<>
	void Engine::var(const char* const name, const std::string& value)
	{
		auto var = mVars.getData(name);
		assert(var && var->isType(IVariable::VariableType::String));

		std::static_pointer_cast<VariableString>(var)->value(value);
	}

	bool Engine::mainLoop()
	{
		assert(mCurState == State::Created);
		if (mCurState != State::Created)
			return false;

		//initiate every system
		mCurState = State::Initializing;
		{
			mRuntime->runScriptFile("../engine.initd.nut");

			if (HorseRadish::OpenGL::OpenGLLoadLibrary("OpenGL32.dll") == false)
			{
				exit(ExitAction::Nothing, "Unable to load OpenGL driver");
				return false;
			}
	
			logSysInfo();
			mLoggerRuntimeCtx->info("");

			initFileSystem();
			mLoggerRuntimeCtx->info("");

			mWindow = std::make_shared<Window>(*mLogger);

			auto windowStyle = Window::WindowStyle::StyleWindow;
			bool onSecondary = false;
			size_t targetWidth = var<int>("renderer.dims.width");
			size_t targetHeight = var<int>("renderer.dims.height");

			if (!mWindow->windowInit("Horseradish engine v1.0", windowStyle, onSecondary, targetWidth, targetHeight))
			{
				mWindow.reset();

				const auto windowErrorMsg = mWindow->getErrorMsg();
				if (windowErrorMsg.empty())
					exit(ExitAction::Nothing, "Unable to create main window");
				else
					exit(ExitAction::Nothing, fmt::format("Unable to create main window: {0}", windowErrorMsg).c_str());

				return false;
			}

			var<int>("display.dims.width", mWindow->getDisplayWidth());
			var<int>("display.dims.height", mWindow->getDisplayHeight());

			mLoggerRuntimeCtx->info("${olive}->${default}Main window initialized.");
		}

		//main render loop
		mCurState = State::Running;
		{
			//render thread
			std::thread threadRender(&Engine::renderLoop, this);

			//this will block until the window message loop ends
			mExitCode = mWindow->messageLoop([&]()
			{
				mCurState = State::Stopping;
				threadRender.join();
			});
		}

		//stop/clean everything
		{
			mWindow.reset();
			mFileSystem.reset();
			HorseRadish::OpenGL::OpenGLUnloadLibrary();
		}

		//so long, and thanks for all the fish
		mCurState = State::Stopped;
		return true;
	}

	int Engine::getExitCode() const
	{
		auto state = mCurState.load();

		if ((state != State::Stopped) && (state != State::StoppedError))
			return -1;
		return mExitCode;
	}

	Engine::ExitAction Engine::getExitAction() const
	{
		return mExitAction;
	}

	std::string Engine::getErrorDesc() const
	{
		return ((mCurState == State::StoppedError) ? mErrorDesc : std::string());
	}
} }

