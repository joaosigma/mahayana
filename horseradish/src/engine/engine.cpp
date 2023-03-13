#include "engine.hpp"

#include "render/raytracer.hpp"
#include "common/imageFactory.hpp"

#include "../build.hpp"

#include <format>
#include <cassert>

namespace hr::engine
{
	void Engine::exit(ExitAction exitAction, std::optional<std::string_view> errorDesc)
	{
		std::lock_guard<std::mutex> lock(mSyncLock);

		if ((mCurState != State::Initializing) && (mCurState != State::Running))
			return;

		auto hasMsg = errorDesc && !errorDesc.value().empty();

		mCurState = (mCurState == State::Initializing) ? State::Stopped : State::Stopping;
		mExitAction = exitAction;

		if (hasMsg)
		{
			mErrorDesc = std::string{*errorDesc};
			mLoggerRuntimeCtx->error(*errorDesc);
		}

		if (mWindow)
			mWindow->sendMessageClose();
	}

	void Engine::initParseCmdLine(std::string_view cmdLine)
	{
		if (cmdLine.find("--debugRaytracer") != std::string_view::npos)
			mCmdLineOptions.dbgRayTrace = true;
	}

	void Engine::initFileSystem()
	{
		mFileSystem = std::make_shared<hr::io::FileSystem>(10);

		//mount current and previous directory
		{
			auto currentPath = std::filesystem::current_path();
		
			mFileSystem->mountPath(currentPath, {});
			mFileSystem->mountPath(currentPath.parent_path(), {});
		}

		//mount main game resource directory
		mFileSystem->mountPath(std::filesystem::path{"d:/jogos/doom3/base/"}, {});
		mLoggerRuntimeCtx->info("${olive}->${default}Path set to: \"d:/jogos/doom3/base/\"");

		size_t totalFich = 0;
		size_t totalPacks = 0;

		//for every pack/zip/7zip file
		hr::io::FileSystem::findFiles("d:/jogos/doom3/base/pak*.pk4", true, [&](const std::filesystem::path &path, const uint64_t&)
		{
			size_t numFilesZip;

			//mount the zip file as a directoty
			if (mFileSystem->mountZip(path, {}, &numFilesZip))
			{
				totalPacks += 1;
				totalFich += numFilesZip;
			}

			return true;
		});

		mLoggerRuntimeCtx->info("   loaded {0} archives with a total of {1} files", totalPacks, totalFich);
	}

	void Engine::runtimeFuncVarRegister(const std::string &funcName, Runtime::FunctionReturnContext &ctx)
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

		mVars.findAllWithKeys([&](const std::string& key, const std::shared_ptr<IVariable>&)
		{
			finalList.push_back(key);
		});

		ctx.setReturnValue(finalList);
	}

	void Engine::runtimeFuncRuntime(const std::string &funcName, Runtime::FunctionReturnContext &)
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
		//misc info
		mLoggerRuntimeCtx->info("${olive}->${default}System information:");

		if (auto info = hr::platform::Platform::cpuGetVendorID(); info)
			mLoggerRuntimeCtx->info("   CPU vendor ID: {}", *info);
		if (auto info = hr::platform::Platform::cpuGetProcessorName(); info)
			mLoggerRuntimeCtx->info("   CPU processor name: {}", *info);

		{
			auto memTotal = hr::platform::Platform::systemInfoInt(hr::platform::Platform::SystemInfo::MemoryTotal);
			auto memFree = hr::platform::Platform::systemInfoInt(hr::platform::Platform::SystemInfo::MemoryFree);

			mLoggerRuntimeCtx->info("   Total physical memory: {0}", hr::StringUtils::formatSize(memTotal.value_or(0)));
			mLoggerRuntimeCtx->info("   Free physical memory: {0}", hr::StringUtils::formatSize(memFree.value_or(0)));
		}

		{
			auto displayWidth = hr::platform::Platform::systemInfoInt(hr::platform::Platform::SystemInfo::DisplayWidth);
			auto displayHeight = hr::platform::Platform::systemInfoInt(hr::platform::Platform::SystemInfo::DisplayHeight);
			auto displayColorBits = hr::platform::Platform::systemInfoInt(hr::platform::Platform::SystemInfo::DisplayColorBits);
			auto displayFrequency = hr::platform::Platform::systemInfoInt(hr::platform::Platform::SystemInfo::DisplayFrequency);
			mLoggerRuntimeCtx->info("   Desktop resolution: {0}x{1}x{2}@{3}", displayWidth.value_or(0), displayHeight.value_or(0), displayColorBits.value_or(0), displayFrequency.value_or(0));
		}


		mLoggerRuntimeCtx->info("   Operating system: {0}", hr::platform::Platform::systemInfoStr(hr::platform::Platform::SystemInfo::OperatingSystemName).value_or(""));
		mLoggerRuntimeCtx->info(hr::platform::Platform::isArch64() ? "   Build type: x86 64bit" : "   Build type: x86 32bit");
		mLoggerRuntimeCtx->info("   Machine name: {0}", hr::platform::Platform::systemInfoStr(hr::platform::Platform::SystemInfo::MachineName).value_or(""));
		mLoggerRuntimeCtx->info("   User name: {0}", hr::platform::Platform::systemInfoStr(hr::platform::Platform::SystemInfo::CurrentUsername).value_or(""));

		//test UTF8
		mLoggerRuntimeCtx->info("${olive}->${default}UTF8 text test:");
		mLoggerRuntimeCtx->info("   Hello!");
		mLoggerRuntimeCtx->info("   Olá!");
		mLoggerRuntimeCtx->info("   Grüß Gott");
		mLoggerRuntimeCtx->info("   Здравствуйте");
		mLoggerRuntimeCtx->info("   Γειά σου");
		mLoggerRuntimeCtx->info("   مرحبا");
	}

	Engine::Engine(std::string_view cmdLine, bool devMode)
		: mDevMode(devMode)
		, mAsyncDispatcher{mAsyncScheduler}
	{
		initParseCmdLine(cmdLine);

		mLogger = std::make_shared<Logger>(0, 0, "../logs/log.txt");
		mLoggerRenderCtx = std::make_shared<Logger::Context>(*mLogger, Logger::ModuleType::Graphics);
		mLoggerRuntimeCtx = std::make_shared<Logger::Context>(*mLogger, Logger::ModuleType::SysRuntime);

		{
			auto maxThreads = std::min<size_t>(std::thread::hardware_concurrency() * 2, 10);
			
			mAsyncThreads.reserve(maxThreads);
			for (size_t curThread = 0; curThread < maxThreads; curThread++)
				mAsyncThreads.push_back(std::jthread([this]() { mAsyncScheduler.run(); }));
		}

		mRuntime = std::make_shared<Runtime>(*mLoggerRuntimeCtx);

		mRuntime->registerFunc("runtime.varCreate", std::bind(&Engine::runtimeFuncVarRegister, this, std::placeholders::_1, std::placeholders::_2));
		mRuntime->registerFunc("runtime.varGet", std::bind(&Engine::runtimeFuncVarGet, this, std::placeholders::_1, std::placeholders::_2));
		mRuntime->registerFunc("runtime.varSet", std::bind(&Engine::runtimeFuncVarSet, this, std::placeholders::_1, std::placeholders::_2));
		mRuntime->registerFunc("runtime.varList", std::bind(&Engine::runtimeFuncVarList, this, std::placeholders::_1, std::placeholders::_2));

		mRuntime->registerFunc("runtime.exec", std::bind(&Engine::runtimeFuncRuntime, this, std::placeholders::_1, std::placeholders::_2));
		mRuntime->registerFunc("runtime.quit", std::bind(&Engine::runtimeFuncRuntime, this, std::placeholders::_1, std::placeholders::_2));
		mRuntime->registerFunc("runtime.restart", std::bind(&Engine::runtimeFuncRuntime, this, std::placeholders::_1, std::placeholders::_2));

#if !defined(NDEBUG)
		mRuntime->registerFunc("runtime.forceCrash", std::bind(&Engine::runtimeFuncRuntime, this, std::placeholders::_1, std::placeholders::_2));
#endif

		mRuntime->runScriptFile("../engine.runtime.nut");

		mStats[StatSampleType::Fps] = std::unique_ptr<StatSeries<3000>>(new StatSeries<3000>(" FPS"));
		mStats[StatSampleType::NumTris] = std::unique_ptr<StatSeries<3000>>(new StatSeries<3000>(" Ktris/s"));

		//some vars can already be set
		{
			std::string strAux;

			if (auto info = hr::platform::Platform::cpuGetVendorID(); info)
				var<std::string>("sys.info.cpuVendor", *info);
			if (auto info = hr::platform::Platform::cpuGetProcessorName(); info)
				var<std::string>("sys.info.cpuName", *info);
			var<std::string>("sys.info.build", std::format("Horseradish v1.0.0 ({})", hr::build::Hash));
		}

		//first entries
		mLoggerRuntimeCtx->info("====== Engine created!");
		mLoggerRuntimeCtx->info("======   build: " __DATE__ " " __TIME__);
		mLoggerRuntimeCtx->info("");
	}

	Engine::~Engine()
	{
		mRuntime.reset();

		mAsyncDispatcher.stopAndWait();
		mAsyncScheduler.stop();
		for (auto& thread : mAsyncThreads)
			thread.join();
		mAsyncThreads.clear();

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

		//if we want to simply run the raytracer
		if (mCmdLineOptions.dbgRayTrace)
		{
			auto raytracer = hr::render::Raytracer(mAsyncDispatcher, 1024, 768);

			hr::render::tools::Camera cam;
			raytracer.trace(cam, hr::gl::tools::Viewport{1024, 768});

			{
				auto srcImg = raytracer.buffer().clone();
				srcImg.transform(
				  [](hr::Colorf& pixel)
				  {
					  pixel[0] = hr::Colorf::convertLinear2SRGB(pixel[0]);
					  pixel[1] = hr::Colorf::convertLinear2SRGB(pixel[1]);
					  pixel[2] = hr::Colorf::convertLinear2SRGB(pixel[2]);
					  return true;
				  });

				auto finalImg = srcImg.convert<uint8_t, hr::imaging::ImageFormatRGB>(1.0f, 0.0f);

				hr::streams::FileStream fs(R"(../raytrace.png)", false, true);
				hr::streams::StreamWriter swriter{fs};
				hr::imaging::Factory::savePNG(swriter, finalImg);
			}

			//leave
			mExitCode = 0;
			mCurState = State::Stopped;
			return true;
		}

		//initiate every system
		mCurState = State::Initializing;
		{
			mRuntime->runScriptFile("../engine.initd.nut");
	
			logSysInfo();
			mLoggerRuntimeCtx->info("");

			initFileSystem();
			mLoggerRuntimeCtx->info("");

			mWindow = std::make_shared<platform::Window>(*mLogger);

			auto windowStyle = platform::Window::WindowStyle::StyleWindow;
			bool onSecondary = false;
			size_t targetWidth = var<int>("renderer.dims.width");
			size_t targetHeight = var<int>("renderer.dims.height");

			if (!mWindow->windowInit("Horseradish engine v1.0", windowStyle, onSecondary, targetWidth, targetHeight))
			{
				mWindow.reset();

				const auto msg = mWindow->getErrorMsg();
				if (msg.empty())
					exit(ExitAction::Nothing, "Unable to create main window");
				else
					exit(ExitAction::Nothing, std::format("Unable to create main window: {}", msg));

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
			std::jthread threadRender(&Engine::renderLoop, this);

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
		}

		//so long, and thanks for all the fish
		mCurState = State::Stopped;
		return true;
	}

	int Engine::getExitCode() const
	{
		if (mCurState != State::Stopped)
			return -1;
		return mExitCode;
	}

	Engine::ExitAction Engine::getExitAction() const
	{
		return mExitAction;
	}

	std::string_view Engine::getErrorDesc() const
	{
		return mErrorDesc;
	}
}
