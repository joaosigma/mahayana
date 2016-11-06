#include "engine.hpp"

#include "profiler.hpp"

#include "../common/opengl/openGL.hpp"
#include "../common/opengl/tools/viewport.hpp"

#include "../render/stage.hpp"
#include "../render/world.hpp"
#include "../render/consoleUI.hpp"
#include "../render/profilerUI.hpp"
#include "../render/rendererDeferred.hpp"
#include "../render/renderer2D.hpp"

#include <libs/cppformat/format.h>

namespace HorseRadish
{
	static
	void openglInitialize()
	{
		HorseRadish::OpenGL::glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
		HorseRadish::OpenGL::glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
		HorseRadish::OpenGL::glDepthFunc(GL_LEQUAL);
		HorseRadish::OpenGL::glDepthMask(GL_TRUE);
		HorseRadish::OpenGL::glClearDepth(1.0f);
		HorseRadish::OpenGL::glClearStencil(0);
		HorseRadish::OpenGL::glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		HorseRadish::OpenGL::glPolygonOffset(1.0f, 1.0f);
		HorseRadish::OpenGL::glDisable(GL_DEPTH_TEST);
		HorseRadish::OpenGL::glDisable(GL_BLEND);
		HorseRadish::OpenGL::glDisable(GL_SCISSOR_TEST);
		HorseRadish::OpenGL::glDisable(GL_STENCIL_TEST);
		HorseRadish::OpenGL::glEnable(GL_CULL_FACE);
		HorseRadish::OpenGL::glHint(GL_LINE_SMOOTH_HINT, GL_NICEST);
		HorseRadish::OpenGL::glCullFace(GL_BACK);
		HorseRadish::OpenGL::glFrontFace(GL_CCW);

		HorseRadish::OpenGL::glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
	}

	static
	void openGLWriteInfo(HorseRadish::Engine::Logger &logger, const HorseRadish::OpenGL::Objects::Context &glContext)
	{
		std::string infoValueString;
		int infoValueInt;

		HorseRadish::Engine::Logger::Context ctx(logger, HorseRadish::Engine::Logger::ModuleType::Graphics);

		//driver info
		ctx.info("${olive}->${default}OpenGL driver info:");
		glContext.info(HorseRadish::OpenGL::Objects::Context::InformationType::Version, infoValueString);
		ctx.info("   OpenGL version: " + infoValueString);
		glContext.info(HorseRadish::OpenGL::Objects::Context::InformationType::Vendor, infoValueString);
		ctx.info("   OpenGL vendor: " + infoValueString);
		glContext.info(HorseRadish::OpenGL::Objects::Context::InformationType::Renderer, infoValueString);
		ctx.info("   OpenGL renderer: " + infoValueString);

		//extensions available
		ctx.info("${olive}->${default}OpenGL extensions available:");
		HorseRadish::OpenGL::glGetIntegerv(GL_NUM_EXTENSIONS, &infoValueInt);
		for (int curExt = 0; curExt < infoValueInt; curExt++)
			ctx.info((const char*)HorseRadish::OpenGL::glGetStringi(GL_EXTENSIONS, curExt));

		//other stuff
		ctx.info("${olive}->${default}OpenGL extended information:");

		glContext.info(HorseRadish::OpenGL::Objects::Context::InformationType::GLSLVersion, infoValueString);
		ctx.info("   GLSL version: " + infoValueString);

		glContext.info(HorseRadish::OpenGL::Objects::Context::InformationType::MaxDrawBuffers, infoValueInt);
		ctx.info("   Maximum number of draw buffers: {0}", infoValueInt);

		glContext.info(HorseRadish::OpenGL::Objects::Context::InformationType::MaxColorAttachments, infoValueInt);
		ctx.info("   Maximum number of color attachments in FBOs: {0}", infoValueInt);

		glContext.info(HorseRadish::OpenGL::Objects::Context::InformationType::MaxTextureSize, infoValueInt);
		ctx.info("   Maximum 1D/2D texture size: {0}x{0}", infoValueInt);
		glContext.info(HorseRadish::OpenGL::Objects::Context::InformationType::MaxTexture3DSize, infoValueInt);
		ctx.info("   Maximum 3D texture size: {0}x{0}x{0}", infoValueInt);
		glContext.info(HorseRadish::OpenGL::Objects::Context::InformationType::MaxTextureCubemapSize, infoValueInt);
		ctx.info("   Maximum cubemap texture size: {0}x{0}", infoValueInt);
		glContext.info(HorseRadish::OpenGL::Objects::Context::InformationType::MaxTextureRectSize, infoValueInt);
		ctx.info("   Maximum rectangle texture size: {0}x{0}", infoValueInt);
	}

	static
	void CALLBACK openglDebugMessagesCallback(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar *message, void *userParam)
	{
		const char *glSource, *glType, *glSeverity;

		auto logger = reinterpret_cast<HorseRadish::Engine::Logger*>(userParam);

		//por omissão
		glSource = glType = glSeverity = "";

		//escolho o source correcto
		switch (source)
		{
		case GL_DEBUG_SOURCE_API_ARB:
			glSource = "source(api)";
			break;
		case GL_DEBUG_SOURCE_SHADER_COMPILER_ARB:
			glSource = "source(glsl)";
			break;
		case GL_DEBUG_SOURCE_WINDOW_SYSTEM_ARB:
			glSource = "source(window system)";
			break;
		case GL_DEBUG_SOURCE_THIRD_PARTY_ARB:
			glSource = "source(third party)";
			break;
		case GL_DEBUG_SOURCE_APPLICATION_ARB:
			glSource = "source(application)";
			break;
		case GL_DEBUG_SOURCE_OTHER_ARB:
			glSource = "source(other)";
			break;
		default:
			glSource = "source(unknown)";
			break;
		}

		//escolho o tipo correcto
		switch (type)
		{
		case GL_DEBUG_TYPE_ERROR_ARB:
			glType = "type(error)";
			break;
		case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR_ARB:
			glType = "type(deprecated)";
			break;
		case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR_ARB:
			glType = "type(undefined)";
			break;
		case GL_DEBUG_TYPE_PERFORMANCE_ARB:
			glType = "type(performance)";
			break;
		case GL_DEBUG_TYPE_PORTABILITY_ARB:
			glType = "type(portability)";
			break;
		case GL_DEBUG_TYPE_OTHER_ARB:
			glType = "type(other)";
			break;
		default:
			glType = "type(unknown)";
			break;
		}

		//escolho o grau correcto
		Engine::Logger::EntryType entryType = Engine::Logger::EntryType::Info;
		switch (severity)
		{
		case GL_DEBUG_SEVERITY_HIGH_ARB:
			glSeverity = "severity(high)";
			entryType = Engine::Logger::EntryType::Warning;
			break;
		case GL_DEBUG_SEVERITY_MEDIUM_ARB:
			glSeverity = "severity(medium)";
			break;
		case GL_DEBUG_SEVERITY_LOW_ARB:
			glSeverity = "severity(low)";
			break;
		default:
			glSeverity = "severity(unknown)";
			break;
		}

		//faço log do que preciso
		logger->log(entryType, HorseRadish::Engine::Logger::ModuleType::Graphics, "OpenGL [{0} - {1} - {2}]:", glSource, glType, glSeverity);
		logger->log(entryType, HorseRadish::Engine::Logger::ModuleType::Graphics, "     {0}", message);
	}

	namespace Engine
	{
		void Engine::renderLoop()
		{
			HorseRadish::Timer timerFrame;
			std::unique_ptr<Profiler> profiler;
			std::unique_ptr<Render::Stage> stage;
			std::unique_ptr<HorseRadish::Render::RendererDeferred> rendererDeferred;
			std::unique_ptr<HorseRadish::Render::Renderer2D> renderer2D;
			std::unique_ptr<HorseRadish::Render::World> renderData;
			std::unique_ptr<platform::OpenglContext> glContext;
			std::unique_ptr<HorseRadish::Render::ConsoleUI> consoleUI;
			std::unique_ptr<HorseRadish::Render::ProfilerUI> profilerUI;

			mLoggerRenderCtx->info(" ");
			mLoggerRenderCtx->info("${olive}->${default}Render thread initialized.");

			auto isDevMove = var<bool>("sys.developer");

			//start a profile if enabled
			if (Profiler::isSupported())
			{
				profiler = std::make_unique<Profiler>();
				profiler->enableStat(Profiler::StatId::FrameTotal, Profiler::StatId::FrameDraw, Profiler::StatId::FrameGPU, Profiler::StatId::FrameLogic);

				profiler->enableStat(Profiler::StatId::GPUTimeElapsed, Profiler::StatId::GPUSamples,
					Profiler::StatId::GPUVerticesSubmitted, Profiler::StatId::GPUPrimitivesSubmitted, Profiler::StatId::GPUVertexShaderInvocations,
					Profiler::StatId::GPUFragmentShaderInvocations, Profiler::StatId::GPUClipInputPrimitives, Profiler::StatId::GPUClipOutputPrimitives);
			}

			//start everything related to OpenGL
			{
				int glMajorVersion, glMinorVersion;

				glContext = std::make_unique<platform::OpenglContext>(*mWindow, "OpenGL32.dll", 4, 5, var<bool>("renderer.glDebug"), true);
				if (!glContext->isValid())
				{
					auto errorMsg = glContext->getErrorMsg();
					if (errorMsg.empty())
						exit(ExitAction::Nothing, "Unable to create OpenGL context");
					else
						exit(ExitAction::Nothing, fmt::format("Unable to create OpenGL context: {0}", errorMsg).c_str());

					return;
				}

				HorseRadish::OpenGL::glGetIntegerv(GL_MAJOR_VERSION, &glMajorVersion);
				HorseRadish::OpenGL::glGetIntegerv(GL_MINOR_VERSION, &glMinorVersion);
				if ((glMajorVersion < 4) || ((glMajorVersion == 4) && (glMinorVersion < 5)))
				{
					exit(ExitAction::Nothing, "OpenGL version 4.5 or higher is required (try updating your drivers).");
					return;
				}

				if (!glContext->isExtPresent("GL_ARB_pipeline_statistics_query"))
				{
					exit(ExitAction::Nothing, "Required extensions are not present");
					return;
				}

				openglInitialize();
				glContext->swapBuffers(); //to force a black screen
				glContext->setSwapInterval(var<int>("renderer.winSwapInterval"));

				mLoggerRenderCtx->info("${olive}->${default}OpenGL system initialized.");
				openGLWriteInfo(*mLogger, *glContext);

				if (var<bool>("renderer.glDebug"))
				{
					HorseRadish::OpenGL::glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DONT_CARE, 0, nullptr, GL_TRUE);
					HorseRadish::OpenGL::glDebugMessageCallback(openglDebugMessagesCallback, mLogger.get());
					HorseRadish::OpenGL::glEnable(GL_DEBUG_OUTPUT);
					HorseRadish::OpenGL::glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);

					glContext->dispatchDebugMessages();
				}
			}

			//start everything related to the renderers (worl, deferred renderer, etc.)
			{
				size_t displayWidth = var<int>("renderer.dims.width");
				size_t displayHeight = var<int>("renderer.dims.height");

				renderData = std::make_unique<HorseRadish::Render::World>();

				renderer2D = std::make_unique<HorseRadish::Render::Renderer2D>(*glContext);
				renderer2D->initialize(displayWidth, displayHeight,
										mFileSystem.get(), var<std::string>("sys.console.text.font").c_str(),
										var<int>("sys.console.text.size"));

				rendererDeferred = std::make_unique<HorseRadish::Render::RendererDeferred>(*glContext, *mFileSystem, *renderData, displayWidth, displayHeight);

				stage = std::make_unique<Render::Stage>(*mRuntime, *mLoggerRuntimeCtx, *mFileSystem, *glContext, displayWidth, displayHeight);
			}
			
			//start other render (misc) related stuff
			{
				if (Profiler::isSupported())
				{
					profilerUI = std::make_unique<HorseRadish::Render::ProfilerUI>(*profiler, *renderer2D);
					profilerUI->setInfoState(isDevMove);
				}

				if (isDevMove)
					consoleUI = std::make_unique<HorseRadish::Render::ConsoleUI>(*mLogger, *renderer2D, 20);
			}
			
			//initialization finished
			mLoggerRenderCtx->info(" ");
			mLoggerRenderCtx->info("${#FF9B00}Aplic${#FF8800}ation ${#FF8000}ready ${#FF6A00}to ${#FF6300}go...");
			mLoggerRenderCtx->info("   type \"help\" to print information regarding this console (or optionally for a command or variable)");
			mLoggerRenderCtx->info("   type \"com_list\" for a list of commands and variables (or \"com_listBind\" for a list of binds)");
			mLoggerRenderCtx->info(" ");
			mLoggerRenderCtx->info("===============================================");
			mLoggerRenderCtx->info(" ");

			mRuntime->callVoidMethod("events.ready");

			HorseRadish::OpenGL::glEnable(GL_FRAMEBUFFER_SRGB);

			//!!!!!!!!!!!!!!!! dev
			{

				//HorseRadish::Streams::FileStream readStream("c:/Users/Sigma/Desktop/doom3.json", true, false);
				HorseRadish::Streams::FileStream readStream("c:/Users/Sigma/Desktop/test_scene.json", true, false);
				//HorseRadish::Streams::FileStream readStream("c:/Users/Sigma/Desktop/volund.json", true, false);

				renderData->cleanup();
				if (!renderData->importJSON(HorseRadish::Streams::StreamReader(readStream)))
					renderData->cleanup();

				//renderData->importObj("C:\\Users\\Sigma\\Desktop\\", "volund.obj");

				renderData->loadData(*mFileSystem);

				rendererDeferred->loadWorld();

				/*HorseRadish::Streams::FileStream writeStream("c:/Users/Sigma/Desktop/volund2.json", false, true);
				renderData->ExportJSON(HorseRadish::Streams::StreamWriter(writeStream));*/
			}

			//we are about to enter the main render loop
			{
				//a camera
				HorseRadish::Render::Tools::CameraFPS camera;
				camera.setPos(0.0f, 0.0f, 1.0f);
				camera.setTarget(0.0f, 0.0f, 0.0f);
				camera.setMovementScale(HorseRadish::Render::Tools::CameraFPS::CameraInput::Keyboard, 10.0f);

				HorseRadish::OpenGL::Tools::Viewport viewportRender(90.0f, var<int>("renderer.dims.width"), var<int>("renderer.dims.height"), 1.0f, 500.0f);
				HorseRadish::OpenGL::glViewport(0, 0, viewportRender.width(), viewportRender.height());

				HorseRadish::OpenGL::Objects::Query::Group<8> renderGlQueryGroup = {
					HorseRadish::OpenGL::Objects::Query::Type::TimeElapsed, HorseRadish::OpenGL::Objects::Query::Type::SamplesPassed,
					HorseRadish::OpenGL::Objects::Query::Type::VerticesSubmitted, HorseRadish::OpenGL::Objects::Query::Type::PrimitivesSubmitted,
					HorseRadish::OpenGL::Objects::Query::Type::VertexShaderInvocations, HorseRadish::OpenGL::Objects::Query::Type::FragmentShaderInvocations,
					HorseRadish::OpenGL::Objects::Query::Type::ClippingInputPrimitives, HorseRadish::OpenGL::Objects::Query::Type::ClippingOutputPrimitives
					};

				float lastDeltaTimeS = 0.0f;

				while (mCurState == State::Running)
				{
					timerFrame.reStart();
					profiler->nextSample();

					//--------------------
					//Start frame rendering requests to queue stuff on the GPU
					//--------------------

					//in case nothing is drawn
					HorseRadish::OpenGL::glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
					HorseRadish::OpenGL::glDepthMask(GL_TRUE);
					HorseRadish::OpenGL::glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

					if (Profiler::isSupported())
						renderGlQueryGroup.queriesBegin();

					//draw main, deferred scene
					rendererDeferred->render(camera, viewportRender);
					rendererDeferred->renderComposite(viewportRender);

					if (Profiler::isSupported())
						renderGlQueryGroup.queriesEnd();

					//draw stage
					stage->drawScenes();
					stage->drawComposite(viewportRender);

					if (Profiler::isSupported())
						profiler->addSample(Profiler::StatId::FrameDraw, timerFrame.getTimeIntMS());

					//draw console and/or profiler
					if ((profilerUI && profilerUI->isVisible()) || (consoleUI && consoleUI->isVisible()))
					{
						HorseRadish::OpenGL::glEnable(GL_BLEND);
						HorseRadish::OpenGL::glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

						if (profilerUI && profilerUI->isVisible())
							profilerUI->draw(viewportRender); //sys.profiler.draw

						if (consoleUI && consoleUI->isVisible())
							consoleUI->draw(viewportRender);

						HorseRadish::OpenGL::glDisable(GL_BLEND);
					}

					//--------------------
					//Frame rendering request is finished
					//While the GPU is working, we process any input and/or simulations as required
					//--------------------

					//process input
					if (!consoleUI || !consoleUI->isVisible())
					{
						mWindow->rawInputSnapshot();
						auto mousePosition = mWindow->rawInputGetMouseStatus();

						HorseRadish::Render::Tools::CameraFPS::CameraAction cameraActions = HorseRadish::Render::Tools::CameraFPS::None;
						if (mWindow->rawInputGetKeyStatus(platform::Window::VirtualKeys::Up) || mWindow->rawInputGetKeyStatus('W'))
							cameraActions = (HorseRadish::Render::Tools::CameraFPS::CameraAction)(cameraActions | HorseRadish::Render::Tools::CameraFPS::Forward);
						if (mWindow->rawInputGetKeyStatus(platform::Window::VirtualKeys::Down) || mWindow->rawInputGetKeyStatus('S'))
							cameraActions = (HorseRadish::Render::Tools::CameraFPS::CameraAction)(cameraActions | HorseRadish::Render::Tools::CameraFPS::Backward);
						if (mWindow->rawInputGetKeyStatus(platform::Window::VirtualKeys::Left) || mWindow->rawInputGetKeyStatus('A'))
							cameraActions = (HorseRadish::Render::Tools::CameraFPS::CameraAction)(cameraActions | HorseRadish::Render::Tools::CameraFPS::StrifeLeft);
						if (mWindow->rawInputGetKeyStatus(platform::Window::VirtualKeys::Right) || mWindow->rawInputGetKeyStatus('D'))
							cameraActions = (HorseRadish::Render::Tools::CameraFPS::CameraAction)(cameraActions | HorseRadish::Render::Tools::CameraFPS::StrifeRight);
						if (mWindow->rawInputGetKeyStatus(platform::Window::VirtualKeys::Space))
							cameraActions = (HorseRadish::Render::Tools::CameraFPS::CameraAction)(cameraActions | HorseRadish::Render::Tools::CameraFPS::Up);
						if (mWindow->rawInputGetKeyStatus(platform::Window::VirtualKeys::Control))
							cameraActions = (HorseRadish::Render::Tools::CameraFPS::CameraAction)(cameraActions | HorseRadish::Render::Tools::CameraFPS::Down);
						if (mWindow->rawInputGetKeyStatus(platform::Window::VirtualKeys::Shift))
							cameraActions = (HorseRadish::Render::Tools::CameraFPS::CameraAction)(cameraActions | HorseRadish::Render::Tools::CameraFPS::Run);

						camera.commitInput(cameraActions, mousePosition[0], mousePosition[1], true, lastDeltaTimeS);
					}

					//process window messages
					mWindow->processMessages([&](const platform::Window::Message &msg)
					{
						if (msg.isType(platform::Window::Message::MessageType::CharacterKey))
							mRuntime->callVoidMethod("events.onKeyPress", msg.getParam());

						stage->processMessage(msg);

						if (consoleUI)
							consoleUI->processMsg(msg, [&](const char * const newInput) { mRuntime->runScript(newInput); });

					}, true);

					if (Profiler::isSupported())
					{
						profiler->addSample(Profiler::StatId::GPUTimeElapsed, static_cast<int64_t>(renderGlQueryGroup.getResultI64<0>() / 1000));
						profiler->addSample(Profiler::StatId::GPUSamples, static_cast<int64_t>(renderGlQueryGroup.getResultI64<1>()));
						profiler->addSample(Profiler::StatId::GPUVerticesSubmitted, static_cast<int64_t>(renderGlQueryGroup.getResultI64<2>()));
						profiler->addSample(Profiler::StatId::GPUPrimitivesSubmitted, static_cast<int64_t>(renderGlQueryGroup.getResultI64<3>()));
						profiler->addSample(Profiler::StatId::GPUVertexShaderInvocations, static_cast<int64_t>(renderGlQueryGroup.getResultI64<4>()));
						profiler->addSample(Profiler::StatId::GPUFragmentShaderInvocations, static_cast<int64_t>(renderGlQueryGroup.getResultI64<5>()));
						profiler->addSample(Profiler::StatId::GPUClipInputPrimitives, static_cast<int64_t>(renderGlQueryGroup.getResultI64<6>()));
						profiler->addSample(Profiler::StatId::GPUClipOutputPrimitives, static_cast<int64_t>(renderGlQueryGroup.getResultI64<7>()));
					}

					//process step in the render data, stage and console
					renderData->prepareNextFrame(camera, viewportRender);
					stage->processStep();

					if (consoleUI)
						consoleUI->processStep();
					
					if (profilerUI)
						profilerUI->processStats();

					if (Profiler::isSupported())
						profiler->addSample(Profiler::StatId::FrameGPU, timerFrame.getTimeIntMS());

					//--------------------
					//Simulations are finished, we can swap GPU buffers and move on
					//--------------------

					glContext->swapBuffers();

					//if we must take a screenshot
					if (var<int>("sys.screenshot") > 0)
					{
						var<int>("sys.screenshot", var<int>("sys.screenshot") - 1); //several screenshots can be taken

						HorseRadish::Streams::FileStream fileStream("screenshot.bmp", false, true);

						//rendererDeferred //blit from the final framebuffer
					}

					//only limit FPS if not developing
					if (!isDevMove)
					{
						auto frameTotalTimeMS = timerFrame.getTimeMS();
						if (frameTotalTimeMS < 16.5) //cap to 60fps
						{
							std::this_thread::sleep_for(std::chrono::milliseconds(HorseRadish::Math::ftoi(16 - frameTotalTimeMS)));
							while (timerFrame.getTimeMS() < 16.5);
						}
					}

					if (Profiler::isSupported())
						profiler->addSample(Profiler::StatId::FrameTotal, timerFrame.getTimeIntMS());

					lastDeltaTimeS = static_cast<float>(timerFrame.getTimeS());
				}
			}

			stage.reset();

			profilerUI.reset();
			profiler.reset();

			consoleUI.reset();

			rendererDeferred.reset();
			renderer2D.reset();

			renderData.reset();

			glContext.reset();
		}

	} //Engine
} //HorseRadish

