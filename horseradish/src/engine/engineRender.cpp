#include "engine.hpp"

#include <format>

#include "profiler.hpp"

#include "../common/timer.hpp"
#include "../common/opengl/openGL.hpp"
#include "../common/opengl/tools/viewport.hpp"

#include "../render/stage.hpp"
#include "../render/world.hpp"
#include "../render/worldEditor.hpp"
#include "../render/consoleUI.hpp"
#include "../render/profilerUI.hpp"
#include "../render/rendererMain.hpp"
#include "../render/rendererDebug.hpp"
#include "../render/renderer2D.hpp"

namespace hr::engine
{
	namespace
	{
		class Timestep
		{
			double mLag{ 0 };
			hr::Timer mTimer;
			hr::Timer mTimerTotal;
			const double mSimStep{ 0 };

		public:
			explicit Timestep(size_t simulationHertz) noexcept
				: mSimStep{ 1.0 / static_cast<double>((simulationHertz <= 0) ? 30 : simulationHertz) * 1000.0 }
			{ }

			void reset() noexcept
			{
				mTimer.reStart();
				mTimerTotal.reStart();
			}

			std::chrono::milliseconds totalElapsed() noexcept
			{
				return mTimerTotal.getTime();
			}

			std::chrono::milliseconds update() noexcept
			{
				auto elapsed = mTimer.getTime(true);
				mLag += std::chrono::duration<double, std::chrono::seconds::period>(elapsed).count();

				return elapsed;
			}

			template<class TCallback>
			void processSim(TCallback&& cb) noexcept
			{
				while (mLag >= mSimStep)
				{
					cb(mSimStep);
					mLag -= mSimStep;
				}
			}

			double renderLag() const noexcept
			{
				return (mLag / mSimStep); //[0, 1], where 0.5 means that we are drawing between the current state and the next state
			}
		};

		void openglInitialize()
		{
			hr::gl::glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
			hr::gl::glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
			hr::gl::glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
			hr::gl::glPolygonOffset(1.0f, 1.0f);
			hr::gl::glDisable(GL_DEPTH_TEST);
			hr::gl::glDisable(GL_BLEND);
			hr::gl::glDisable(GL_SCISSOR_TEST);
			hr::gl::glDisable(GL_STENCIL_TEST);
			hr::gl::glEnable(GL_CULL_FACE);
			hr::gl::glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);
			hr::gl::glHint(GL_LINE_SMOOTH_HINT, GL_NICEST);
			hr::gl::glCullFace(GL_BACK);
			hr::gl::glFrontFace(GL_CCW);

			hr::gl::glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

			//we are using Reverse-Z (https://nlguillemot.wordpress.com/2016/12/07/reversed-z-in-opengl/)
			hr::gl::glClipControl(GL_LOWER_LEFT, GL_ZERO_TO_ONE);
			hr::gl::glDepthFunc(GL_GREATER);
			hr::gl::glDepthMask(GL_TRUE);
			hr::gl::glClearDepth(0.0f);
			hr::gl::glClearStencil(0);
		}

		void openGLWriteInfo(hr::engine::Logger &logger, const hr::gl::objects::Context &glContext)
		{
			std::string infoValueString;
			int infoValueInt;

			hr::engine::Logger::Context ctx(logger, hr::engine::Logger::ModuleType::Graphics);

			//driver info
			ctx.info("${olive}->${default}OpenGL driver info:");
			glContext.info(hr::gl::objects::Context::InformationType::Version, infoValueString);
			ctx.info("   OpenGL version: " + infoValueString);
			glContext.info(hr::gl::objects::Context::InformationType::Vendor, infoValueString);
			ctx.info("   OpenGL vendor: " + infoValueString);
			glContext.info(hr::gl::objects::Context::InformationType::Renderer, infoValueString);
			ctx.info("   OpenGL renderer: " + infoValueString);

			//extensions available
			ctx.info("${olive}->${default}OpenGL extensions available:");
			hr::gl::glGetIntegerv(GL_NUM_EXTENSIONS, &infoValueInt);
			for (int curExt = 0; curExt < infoValueInt; curExt++)
				ctx.info((const char*)hr::gl::glGetStringi(GL_EXTENSIONS, curExt));

			//other stuff
			ctx.info("${olive}->${default}OpenGL extended information:");

			glContext.info(hr::gl::objects::Context::InformationType::GLSLVersion, infoValueString);
			ctx.info("   GLSL version: " + infoValueString);

			glContext.info(hr::gl::objects::Context::InformationType::MaxDrawBuffers, infoValueInt);
			ctx.info("   Maximum number of draw buffers: {0}", infoValueInt);

			glContext.info(hr::gl::objects::Context::InformationType::MaxColorAttachments, infoValueInt);
			ctx.info("   Maximum number of color attachments in FBOs: {0}", infoValueInt);

			glContext.info(hr::gl::objects::Context::InformationType::MaxTextureSize, infoValueInt);
			ctx.info("   Maximum 1D/2D texture size: {0}x{0}", infoValueInt);
			glContext.info(hr::gl::objects::Context::InformationType::MaxTexture3DSize, infoValueInt);
			ctx.info("   Maximum 3D texture size: {0}x{0}x{0}", infoValueInt);
			glContext.info(hr::gl::objects::Context::InformationType::MaxTextureCubemapSize, infoValueInt);
			ctx.info("   Maximum cubemap texture size: {0}x{0}", infoValueInt);
			glContext.info(hr::gl::objects::Context::InformationType::MaxTextureRectSize, infoValueInt);
			ctx.info("   Maximum rectangle texture size: {0}x{0}", infoValueInt);
		}

		void CALLBACK openglDebugMessagesCallback(GLenum source, GLenum type, GLuint, GLenum severity, GLsizei length, const GLchar *message, const void *userParam)
		{
			if (length <= 0)
				return;

			const char *glSource = "", *glType = "", *glSeverity = "";

			auto logger = const_cast<hr::engine::Logger*>(reinterpret_cast<const hr::engine::Logger*>(userParam));

			switch (source)
			{
			case GL_DEBUG_SOURCE_API:
				glSource = "source(api)";
				break;
			case GL_DEBUG_SOURCE_SHADER_COMPILER:
				glSource = "source(glsl)";
				break;
			case GL_DEBUG_SOURCE_WINDOW_SYSTEM:
				glSource = "source(window system)";
				break;
			case GL_DEBUG_SOURCE_THIRD_PARTY:
				glSource = "source(third party)";
				break;
			case GL_DEBUG_SOURCE_APPLICATION:
				glSource = "source(application)";
				break;
			case GL_DEBUG_SOURCE_OTHER:
				glSource = "source(other)";
				break;
			default:
				glSource = "source(unknown)";
				break;
			}

			switch (type)
			{
			case GL_DEBUG_TYPE_ERROR:
				glType = "type(error)";
				break;
			case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR:
				glType = "type(deprecated)";
				break;
			case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR:
				glType = "type(undefined)";
				break;
			case GL_DEBUG_TYPE_PERFORMANCE:
				glType = "type(performance)";
				break;
			case GL_DEBUG_TYPE_PORTABILITY:
				glType = "type(portability)";
				break;
			case GL_DEBUG_TYPE_OTHER:
				glType = "type(other)";
				break;
			default:
				glType = "type(unknown)";
				break;
			}

			auto entryType = hr::engine::Logger::EntryType::Info;
			switch (severity)
			{
			case GL_DEBUG_SEVERITY_HIGH:
				glSeverity = "severity(high)";
				entryType = hr::engine::Logger::EntryType::Warning;
				break;
			case GL_DEBUG_SEVERITY_MEDIUM:
				glSeverity = "severity(medium)";
				break;
			case GL_DEBUG_SEVERITY_LOW:
				glSeverity = "severity(low)";
				break;
			default:
				glSeverity = "severity(unknown)";
				break;
			}

			logger->log(entryType, hr::engine::Logger::ModuleType::Graphics, "OpenGL [{0} - {1} - {2}]:", glSource, glType, glSeverity);
			logger->log(entryType, hr::engine::Logger::ModuleType::Graphics, "     {0}", message);
		}
	}

	void Engine::renderLoop()
	{
		std::unique_ptr<Profiler> profiler;
		std::unique_ptr<render::Stage> stage;
		std::unique_ptr<hr::render::RendererMain> rendererMain;
		std::unique_ptr<hr::render::RendererDebug> rendererDebug;
		std::unique_ptr<hr::render::Renderer2D> renderer2D;
		std::unique_ptr<hr::render::World> renderData;
		std::unique_ptr<platform::OpenglContext> glContext;
		std::unique_ptr<hr::render::ConsoleUI> consoleUI;
		std::unique_ptr<hr::render::ProfilerUI> profilerUI;

		mLoggerRenderCtx->info(" ");
		mLoggerRenderCtx->info("${olive}->${default}Render thread initialized.");

		auto isDevMove = var<bool>("sys.developer");

		//start a profiler if enabled
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

			glContext = std::make_unique<platform::OpenglContext>(*mWindow, "OpenGL32.dll", 4, 6, var<bool>("renderer.glDebug"), true);
			if (!glContext->isValid())
			{
				auto errorMsg = glContext->getErrorMsg();
				if (errorMsg.empty())
					exit(ExitAction::Nothing, "Unable to create OpenGL context");
				else
					exit(ExitAction::Nothing, std::format("Unable to create OpenGL context: {0}", errorMsg).c_str());

				return;
			}

			hr::gl::glGetIntegerv(GL_MAJOR_VERSION, &glMajorVersion);
			hr::gl::glGetIntegerv(GL_MINOR_VERSION, &glMinorVersion);
			if ((glMajorVersion < 4) || ((glMajorVersion == 4) && (glMinorVersion < 6)))
			{
				exit(ExitAction::Nothing, "OpenGL version 4.6 or higher is required (try updating your drivers).");
				return;
			}

			if (!glContext->isExtPresent("GL_EXT_texture_compression_s3tc"))
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
				hr::gl::glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DONT_CARE, 0, nullptr, GL_TRUE);
				hr::gl::glDebugMessageCallback(openglDebugMessagesCallback, mLogger.get());
				hr::gl::glEnable(GL_DEBUG_OUTPUT);
				hr::gl::glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);

				glContext->dispatchDebugMessages();
			}
		}

		//start everything related to the renderers (worl, deferred renderer, etc.)
		{
			size_t displayWidth = var<int>("renderer.dims.width");
			size_t displayHeight = var<int>("renderer.dims.height");

			renderer2D = std::make_unique<hr::render::Renderer2D>(*glContext);
			renderer2D->initialize(displayWidth, displayHeight, mFileSystem.get(), var<std::string>("sys.console.text.font").c_str());

			rendererDebug = std::make_unique<hr::render::RendererDebug>(*glContext, *mFileSystem, *renderer2D);

			rendererMain = std::make_unique<hr::render::RendererMain>(*glContext, *mFileSystem, displayWidth, displayHeight);

			stage = std::make_unique<render::Stage>(*mRuntime, *mLoggerRuntimeCtx, *mFileSystem, *glContext, displayWidth, displayHeight);

			//!!!!!!!!!!!!!!!! dev
			{
				{
					hr::render::WorldEditor editor;
					
					auto newArea = editor.newArea("c:/Users/Sigma/Desktop/xeno.hscene", "c:/Users/Sigma/Desktop/xeno.hbin");

					editor.importGLTF(newArea, R"(C:\Users\Sigma\Desktop\xeno\scene.gltf)");

					/*editor.processMesh(newArea, { }, [](hr::geom::Mesh& mesh)
					{
						Matrix trans;

						trans.setRotation(0.0f, 180.0f, 0.0f);
						mesh.transform(trans, Matrix3(trans));
					});*/

					//editor.importMD5(newArea, R"(C:\Users\Sigma\Desktop\network_guardian\mesh.md5mesh)", "network_guardian");
					//editor.removeObjects(newArea, { "network_guardian/ng_lo_collision" });
					//editor.importMD5Anim(newArea, "network_guardian", R"(C:\Users\Sigma\Desktop\network_guardian\idle.md5anim)", "idle");
					//editor.importMD5Anim(newArea, "hellknight", R"(C:\Users\Sigma\Desktop\hellknight\stand.md5anim)", "stand");
				}

				renderData = std::make_unique<hr::render::World>();
				//renderData->loadArea(*rendererMain, "../scenes/spheres.hscene", "../scenes/spheres.hbin");
				//renderData->loadArea(*rendererMain, "../scenes/wood-log.hscene", "../scenes/wood-log.hbin");
				//renderData->loadArea(*rendererMain, "../scenes/sandstone1.hscene", "../scenes/sandstone1.hbin");
				//renderData->loadArea(*rendererMain, "../scenes/sandstone2.hscene", "../scenes/sandstone2.hbin");
				//renderData->loadArea(*rendererMain, "../scenes/volund.hscene", "../scenes/volund.hbin");				
				//renderData->loadArea(*rendererMain, "../scenes/makron.hscene", "../scenes/makron.hbin");
				//renderData->loadArea(*rendererMain, "../scenes/hellknight.hscene", "../scenes/hellknight.hbin");
				//renderData->loadArea(*rendererMain, "../scenes/guardian.hscene", "../scenes/guardian.hbin");
				
				renderData->loadArea(*rendererMain, "c:/Users/Sigma/Desktop/xeno.hscene", "c:/Users/Sigma/Desktop/xeno.hbin");
			}
		}
			
		//start other renderers (misc) related stuff
		{
			if (Profiler::isSupported())
			{
				profilerUI = std::make_unique<hr::render::ProfilerUI>(*profiler, *renderer2D);
				profilerUI->setInfoState(isDevMove);
			}

			if (isDevMove)
				consoleUI = std::make_unique<hr::render::ConsoleUI>(*mLogger, *renderer2D, 20);
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

		hr::gl::glEnable(GL_FRAMEBUFFER_SRGB);

		//we are about to enter the main render loop
		{
			//setup camera
			hr::render::tools::CameraFPS camera;
			camera.setPos(0.0f, 0.0f, 1.0f);
			camera.setTarget(0.0f, 0.0f, 0.0f);
			camera.setMovementScale(hr::render::tools::CameraFPS::CameraInput::Keyboard, 10.0f);

			hr::gl::tools::Viewport viewportRender(45.0f, var<int>("renderer.dims.width"), var<int>("renderer.dims.height"), 0.05f);
			hr::gl::glViewport(0, 0, viewportRender.width(), viewportRender.height());

			hr::gl::objects::Query::Group<8> renderGlQueryGroup = {
				hr::gl::objects::Query::Type::TimeElapsed, hr::gl::objects::Query::Type::SamplesPassed,
				hr::gl::objects::Query::Type::VerticesSubmitted, hr::gl::objects::Query::Type::PrimitivesSubmitted,
				hr::gl::objects::Query::Type::VertexShaderInvocations, hr::gl::objects::Query::Type::FragmentShaderInvocations,
				hr::gl::objects::Query::Type::ClippingInputPrimitives, hr::gl::objects::Query::Type::ClippingOutputPrimitives
			};

			Timestep timestep(50);

			while (mCurState == State::Running)
			{
				hr::Timer timerFrame;
				double lastFrameTimeS = std::chrono::duration_cast<std::chrono::duration<double, std::chrono::seconds::period>>(timestep.update()).count();

				profiler->nextSample();

				//--------------------
				//Start frame rendering requests to queue stuff onto the GPU
				//--------------------

				//in case nothing is drawn
				hr::gl::glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
				hr::gl::glDepthMask(GL_TRUE);
				hr::gl::glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

				if (Profiler::isSupported())
					renderGlQueryGroup.queriesBegin();

				//draw main, deferred scene
				rendererMain->render(camera, viewportRender);
				rendererMain->renderDebug(*rendererDebug, camera, viewportRender);
				rendererMain->renderComposite(viewportRender);

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
					auto fontSize = static_cast<size_t>(var<int>("sys.console.text.size"));

					hr::gl::glEnable(GL_BLEND);
					hr::gl::glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

					if (profilerUI && profilerUI->isVisible())
						profilerUI->draw(fontSize, viewportRender);

					if (consoleUI && consoleUI->isVisible())
						consoleUI->draw(fontSize, viewportRender);

					hr::gl::glDisable(GL_BLEND);
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

					hr::render::tools::CameraFPS::CameraAction cameraActions = hr::render::tools::CameraFPS::None;
					if (mWindow->rawInputGetKeyStatus(platform::Window::VirtualKeys::Up) || mWindow->rawInputGetKeyStatus('W'))
						cameraActions = (hr::render::tools::CameraFPS::CameraAction)(cameraActions | hr::render::tools::CameraFPS::Forward);
					if (mWindow->rawInputGetKeyStatus(platform::Window::VirtualKeys::Down) || mWindow->rawInputGetKeyStatus('S'))
						cameraActions = (hr::render::tools::CameraFPS::CameraAction)(cameraActions | hr::render::tools::CameraFPS::Backward);
					if (mWindow->rawInputGetKeyStatus(platform::Window::VirtualKeys::Left) || mWindow->rawInputGetKeyStatus('A'))
						cameraActions = (hr::render::tools::CameraFPS::CameraAction)(cameraActions | hr::render::tools::CameraFPS::StrifeLeft);
					if (mWindow->rawInputGetKeyStatus(platform::Window::VirtualKeys::Right) || mWindow->rawInputGetKeyStatus('D'))
						cameraActions = (hr::render::tools::CameraFPS::CameraAction)(cameraActions | hr::render::tools::CameraFPS::StrifeRight);
					if (mWindow->rawInputGetKeyStatus(platform::Window::VirtualKeys::Space))
						cameraActions = (hr::render::tools::CameraFPS::CameraAction)(cameraActions | hr::render::tools::CameraFPS::Up);
					if (mWindow->rawInputGetKeyStatus(platform::Window::VirtualKeys::Control))
						cameraActions = (hr::render::tools::CameraFPS::CameraAction)(cameraActions | hr::render::tools::CameraFPS::Down);
					if (mWindow->rawInputGetKeyStatus(platform::Window::VirtualKeys::Shift))
						cameraActions = (hr::render::tools::CameraFPS::CameraAction)(cameraActions | hr::render::tools::CameraFPS::Run);

					camera.commitInput(cameraActions, mousePosition[0], mousePosition[1], true, lastFrameTimeS);
				}

				//process window messages
				mWindow->processMessages([this, &stage, &consoleUI](const platform::Window::Message &msg)
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

				//run any simulations that require a fixed timestep
				timestep.processSim([](double deltaTimeMS)
				{
				});

				//process step in the render data, stage and console
				{
					hr::render::World::Timestep wTimestep;
					wTimestep.t = std::chrono::duration<float, std::chrono::seconds::period>(timestep.totalElapsed()).count();

					renderData->prepareNextFrame(wTimestep, *rendererMain, camera, viewportRender);
					stage->processStep();
				}

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

					hr::streams::FileStream fileStream("screenshot.bmp", false, true);

					//rendererMain //blit from the final framebuffer
				}

				//only limit FPS if not developing
				if (!isDevMove)
				{
					auto frameTotalTimeMS = timerFrame.getTimeMS();
					if (frameTotalTimeMS < 16.5) //cap to 60fps
					{
						std::this_thread::sleep_for(std::chrono::milliseconds(hr::Math::ftoi(16.0 - frameTotalTimeMS)));
						while (timerFrame.getTimeMS() < 16.5);
					}
				}

				if (Profiler::isSupported())
					profiler->addSample(Profiler::StatId::FrameTotal, timerFrame.getTimeIntMS());
			}
		}

		stage.reset();

		profilerUI.reset();
		profiler.reset();

		consoleUI.reset();

		rendererMain.reset();
		rendererDebug.reset();
		renderer2D.reset();

		renderData.reset();

		glContext.reset();
	}
}
