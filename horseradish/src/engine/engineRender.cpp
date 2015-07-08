#include "engine.hpp"

#include "common/opengl/openGL.hpp"
#include "common/opengl/tools/viewport.hpp"
#include "console/consoleUI.hpp"

#include "render/stage.hpp"
#include "render/world.hpp"
#include "render/rendererDeferred.hpp"
#include "render/renderer2D.hpp"

#include "../videoStream.hpp"

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
		glContext.getInfo(HorseRadish::OpenGL::Objects::Context::InformationType::Version, infoValueString);
		ctx.info("   OpenGL version: " + infoValueString);
		glContext.getInfo(HorseRadish::OpenGL::Objects::Context::InformationType::Vendor, infoValueString);
		ctx.info("   OpenGL vendor: " + infoValueString);
		glContext.getInfo(HorseRadish::OpenGL::Objects::Context::InformationType::Renderer, infoValueString);
		ctx.info("   OpenGL renderer: " + infoValueString);

		//extensions available
		ctx.info("${olive}->${default}OpenGL extensions available:");
		HorseRadish::OpenGL::glGetIntegerv(GL_NUM_EXTENSIONS, &infoValueInt);
		for (int curExt = 0; curExt < infoValueInt; curExt++)
			ctx.info((const char*)HorseRadish::OpenGL::glGetStringi(GL_EXTENSIONS, curExt));

		//other stuff
		ctx.info("${olive}->${default}OpenGL extended information:");

		glContext.getInfo(HorseRadish::OpenGL::Objects::Context::InformationType::GLSLVersion, infoValueString);
		ctx.info("   GLSL version: " + infoValueString);

		glContext.getInfo(HorseRadish::OpenGL::Objects::Context::InformationType::MaxDrawBuffers, infoValueInt);
		ctx.info(fmt::format("   Maximum number of draw buffers: {0}", infoValueInt));

		glContext.getInfo(HorseRadish::OpenGL::Objects::Context::InformationType::MaxColorAttachments, infoValueInt);
		ctx.info(fmt::format("   Maximum number of color attachments in FBOs: {0}", infoValueInt));

		glContext.getInfo(HorseRadish::OpenGL::Objects::Context::InformationType::MaxTextureSize, infoValueInt);
		ctx.info(fmt::format("   Maximum 1D/2D texture size: {0}x{0}", infoValueInt));
		glContext.getInfo(HorseRadish::OpenGL::Objects::Context::InformationType::MaxTexture3DSize, infoValueInt);
		ctx.info(fmt::format("   Maximum 3D texture size: {0}x{0}x{0}", infoValueInt));
		glContext.getInfo(HorseRadish::OpenGL::Objects::Context::InformationType::MaxTextureCubemapSize, infoValueInt);
		ctx.info(fmt::format("   Maximum cubemap texture size: {0}x{0}", infoValueInt));
		glContext.getInfo(HorseRadish::OpenGL::Objects::Context::InformationType::MaxTextureRectSize, infoValueInt);
		ctx.info(fmt::format("   Maximum rectangle texture size: {0}x{0}", infoValueInt));
	}

	static
	void CALLBACK openglDebugMessagesCallback(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar *message, void *userParam)
	{
		return;
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
		logger->AddLog(entryType, HorseRadish::Engine::Logger::ModuleType::Graphics, fmt::format("OpenGL [{0} - {1} - {2}]:", glSource, glType, glSeverity));
		logger->AddLog(entryType, HorseRadish::Engine::Logger::ModuleType::Graphics, fmt::format("     {0}", message));
	}

	namespace Engine
	{
		void Engine::renderLoop()
		{
			float timeSpentDrawing, timeSpentProcessing, timeSpentIdle;
			HorseRadish::Timer timerSecond, timerFrame, timerTotal;
			HorseRadish::Render::RendererDeferred *rendererDeferred;
			HorseRadish::Render::Renderer2D *renderer2D;
			HorseRadish::Render::World *renderData;
			HorseRadish::OpenGL::Tools::Viewport *viewport;
			HorseRadish::Render::Tools::Camera *camera;
			Console::UI::ConsoleGUI *consolaGUI;
			OpenglContext *glContext;

			mLoggerRenderCtx->info(" ");
			mLoggerRenderCtx->info("${olive}->${default}Render thread initialized.");

			//start everything related to OpenGL
			{
				int glMajorVersion, glMinorVersion;

				glContext = new OpenglContext(*mWindow, "OpenGL32.dll", 4, 5, this->VarGet<bool>("renderer.glDebug"), false);
				if ((glContext == nullptr) || !glContext->IsValid())
				{
					std::string errorMsg = glContext->GetErrorMsg();
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

				mLoggerRenderCtx->info("${olive}->${default}OpenGL system initialized.");
			}

			//prepare some debug stuff
			if (this->VarGet<bool>("renderer.glDebug"))
			{
				HorseRadish::OpenGL::glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DONT_CARE, 0, nullptr, true);
				HorseRadish::OpenGL::glDebugMessageCallback(openglDebugMessagesCallback, mLogger.get());
				HorseRadish::OpenGL::glEnable(GL_DEBUG_OUTPUT);
				HorseRadish::OpenGL::glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);

				glContext->dispatchDebugMessages();
			}

			//mostro a primeira/segunda frame
			glContext->SwapBuffers();
			glContext->SwapBuffers();

			//inicio agora o renderer (fbos, vbos, shaders, etc)
			renderData = new HorseRadish::Render::World();

			renderer2D = new HorseRadish::Render::Renderer2D(*glContext);
			renderer2D->Initialize(this->VarGet<int>("renderer.winWidth"), this->VarGet<int>("renderer.winHeight"), mFileSystem.get(), this->VarGet<std::string>("sys.console.text.font").c_str(), this->VarGet<int>("sys.console.text.size"));

			rendererDeferred = new HorseRadish::Render::RendererDeferred(*glContext, renderData);
			rendererDeferred->Initialize(this->VarGet<int>("renderer.winWidth"), this->VarGet<int>("renderer.winHeight"), mFileSystem.get());

			//escrevo alguma informação acerda do GL
			openGLWriteInfo(*mLogger, *glContext);

			//a camera
			camera = new HorseRadish::Render::Tools::Camera();

			//coloco alguns valores por defeito na camera
			camera->SetPos(0.0f, 0.0f, 1.0f);
			camera->SetTarget(0.0f, 0.0f, 0.0f);
			camera->SetSensitivity(HorseRadish::Render::Tools::Camera::Keyboard, 10.0f);

			//o viewport
			viewport = new HorseRadish::OpenGL::Tools::Viewport(90.0f, this->VarGet<int>("renderer.winWidth"), this->VarGet<int>("renderer.winHeight"), 1.0f, 500.0f);
			HorseRadish::OpenGL::glViewport(0, 0, viewport->getWidth(), viewport->getHeight());

			glContext->SetSwapInterval(this->VarGet<int>("renderer.winSwapInterval"));

			//inicio o UI da consola (e abro-a se for developer)
			consolaGUI = new Console::UI::ConsoleGUI(renderer2D);
			if (this->VarGet<bool>("sys.developer"))
				consolaGUI->ConsoleVisible(true);
			//SOverlayInit();

			std::shared_ptr<Render::Stage> stage = std::make_shared<Render::Stage>(*mRuntime, *mLoggerRuntimeCtx, *mFileSystem, *glContext, this->VarGet<int>("renderer.winWidth"), this->VarGet<int>("renderer.winHeight"));

			//crio as tabs necessárias para o UI da consola
			auto consoleUIMain = new Console::UI::ConsoleTabConsole([&](const char * const newInput)
			{
				mRuntime->runScript(newInput);
			}, this->mLogger);
			consolaGUI->ConsoleAddTab(consoleUIMain);

			//preparo o texto da consola propriamente dita
			consoleUIMain->CriaTextoConsola(renderer2D);
			consoleUIMain->ActualizaTextoConsola();

			//se sou developer preciso de fazer algumas coisas
			if (this->VarGet<bool>("sys.developer") != 0)
			{
				//também quero um contador
				/*renderData->stats.gpuCounter = new SGPUCounter(250);

				//por omissão tenho estas variáveis
				renderData->stats.gpuCounter->addCounter("timeSlot_draw3D", true, 1.0f, 0.0f, 0.0f);
				renderData->stats.gpuCounter->addCounter("timeSlot_process", true, 0.0f, 0.2f, 0.9f);
				renderData->stats.gpuCounter->addCounter("timeSlot_idle", true, 0.5f, 0.15f, 0.5f);*/
			}

			//tudo pronto a ir, só tenho de imprimir alguma informacao
			mLoggerRenderCtx->info(" ");
			mLoggerRenderCtx->info("${#FF9B00}Aplic${#FF8800}ation ${#FF8000}ready ${#FF6A00}to ${#FF6300}go...");
			mLoggerRenderCtx->info("   type \"help\" to print information regarding this console (or optionally for a command or variable)");
			mLoggerRenderCtx->info("   type \"com_list\" for a list of commands and variables (or \"com_listBind\" for a list of binds)");
			mLoggerRenderCtx->info(" ");
			mLoggerRenderCtx->info("===============================================");
			mLoggerRenderCtx->info(" ");

			//***************
			//**********
			{
				
				//HorseRadish::Streams::FileStream readStream("c:/Users/Sigma/Desktop/doom3.json", true, false);
				//HorseRadish::Streams::FileStream readStream("c:/Users/Sigma/Desktop/test_scene.json", true, false);
				HorseRadish::Streams::FileStream readStream("c:/Users/Sigma/Desktop/volund.json", true, false);

				renderData->Cleanup();
				if (!renderData->ImportJSON(HorseRadish::Streams::StreamReader(readStream)))
					renderData->Cleanup();

				//renderData->importObj("C:\\Users\\Sigma\\Desktop\\", "volund.obj");

				renderData->LoadData(mFileSystem.get());

				rendererDeferred->LoadWorld(*mFileSystem);
				
				/*HorseRadish::Streams::FileStream writeStream("c:/Users/Sigma/Desktop/volund2.json", false, true);
				renderData->ExportJSON(HorseRadish::Streams::StreamWriter(writeStream));*/
			}
			//**********
			//***************

			mRuntime->callVoidMethod("events.ready");

			//antes de começar a desenhar, tenho de iniciar os contadores
			timerSecond.ReStart();
			timerFrame.ReStart();
			timerTotal.ReStart();

			HorseRadish::OpenGL::glEnable(GL_FRAMEBUFFER_SRGB);

			{
				HorseRadish::OpenGL::Objects::Query::Group<8> renderGlQueryGroup(
					{ HorseRadish::OpenGL::Objects::Query::Type::TimeElapsed, HorseRadish::OpenGL::Objects::Query::Type::SamplesPassed,
					HorseRadish::OpenGL::Objects::Query::Type::VerticesSubmitted, HorseRadish::OpenGL::Objects::Query::Type::PrimitivesSubmitted,
					HorseRadish::OpenGL::Objects::Query::Type::VertexShaderInvocations, HorseRadish::OpenGL::Objects::Query::Type::FragmentShaderInvocations,
					HorseRadish::OpenGL::Objects::Query::Type::ClippingInputPrimitives, HorseRadish::OpenGL::Objects::Query::Type::ClippingOutputPrimitives });

				while (mCurState == State::Running)
				{
					//a primeira coisa é acertar os tempos
					renderer2D->auxTools.lastTimeS = renderer2D->auxTools.curTimeS;
					renderer2D->auxTools.curTimeS = timerTotal.GetTimeS();

					//caso nao desenhe nada
					HorseRadish::OpenGL::glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
					HorseRadish::OpenGL::glDepthMask(GL_TRUE);
					HorseRadish::OpenGL::glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);


					renderGlQueryGroup.queriesBegin();

					//desenho a cena normalmente
					//renderData->RenderFrame(*camera, viewport);
					rendererDeferred->Render(*camera, *viewport);

					renderGlQueryGroup.queriesEnd();

					//desenho algum debug se existir
					//renderData->RenderDebug(gbMainConsole, camera, viewport);

					//finalmente (e porque isto foi tudo pra um FBO) faço o render final seguido de algum debug se existir
					//renderData->RenderComposite(gbMainConsole, camera, viewport);
					//renderData->RenderDebugComposite(gbMainConsole, camera, viewport);

					{
						stage->drawScenes();

						HorseRadish::OpenGL::glDisable(GL_BLEND);

						stage->drawComposite();
					}

					//leio o tempo que estive à espera para desenhar o 3D
					timeSpentDrawing = timerFrame.GetTimeMS();

					//se for para tirar algum screenshot
					if (this->VarGet<int>("sys.screenshot") > 0)
					{
						//diminuo o número de imagens a tirar e espero que todos os comandos do OpenGL acabei
						this->VarSet("sys.screenshot", this->VarGet<int>("sys.screenshot") - 1);
						HorseRadish::OpenGL::glFinish();

						//crio um ficheiro (ao sair do scope o ficheiro é fechado)
						HorseRadish::Streams::FileStream fileStream("screenshot.bmp", false, true);

						//basta mandar tirar o screenshot
						//glContext->TakeScreenshot(fileStream); //should come from the framebuffers
					}

					//se alguma coisa da consola precisar de ser desenhado
					if (consolaGUI->GUIVisivel())
					{
						//o opengl (não esquecer: não tenho depth buffer por defeito)
						HorseRadish::OpenGL::glEnable(GL_BLEND);
						HorseRadish::OpenGL::glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

						//se for preciso, actualizo o texto da consola propriamente dita
						//if (gbMainConsole->AsChanged() == true)
						consoleUIMain->ActualizaTextoConsola();

						//mando desenhar a consola
						//SOverlayDraw(gbMainConsole);
						consolaGUI->Draw(viewport);

						//reponho o opengl
						HorseRadish::OpenGL::glDisable(GL_BLEND);
					}

					//se houver uma camera e a consola não estiver a consumir input
					if ((camera != nullptr) && (consolaGUI->ConsoleConsumesInput() == false))
					{
						HorseRadish::Render::Tools::Camera::CameraAction cameraActions;

						//tiro as coisas como estão agora
						mWindow->RawInputSnapshot();

						//por omissão
						cameraActions = HorseRadish::Render::Tools::Camera::None;

						//preciso de saber o que ando a fazer
						if (mWindow->RawInputGetKeyStatus(Window::VirtualKeys::Up) || mWindow->RawInputGetKeyStatus('W'))
							cameraActions = (HorseRadish::Render::Tools::Camera::CameraAction)(cameraActions | HorseRadish::Render::Tools::Camera::Forward);
						if (mWindow->RawInputGetKeyStatus(Window::VirtualKeys::Down) || mWindow->RawInputGetKeyStatus('S'))
							cameraActions = (HorseRadish::Render::Tools::Camera::CameraAction)(cameraActions | HorseRadish::Render::Tools::Camera::Backward);
						if (mWindow->RawInputGetKeyStatus(Window::VirtualKeys::Left) || mWindow->RawInputGetKeyStatus('A'))
							cameraActions = (HorseRadish::Render::Tools::Camera::CameraAction)(cameraActions | HorseRadish::Render::Tools::Camera::StrifeLeft);
						if (mWindow->RawInputGetKeyStatus(Window::VirtualKeys::Right) || mWindow->RawInputGetKeyStatus('D'))
							cameraActions = (HorseRadish::Render::Tools::Camera::CameraAction)(cameraActions | HorseRadish::Render::Tools::Camera::StrifeRight);
						if (mWindow->RawInputGetKeyStatus(Window::VirtualKeys::Space))
							cameraActions = (HorseRadish::Render::Tools::Camera::CameraAction)(cameraActions | HorseRadish::Render::Tools::Camera::Up);
						if (mWindow->RawInputGetKeyStatus(Window::VirtualKeys::Control))
							cameraActions = (HorseRadish::Render::Tools::Camera::CameraAction)(cameraActions | HorseRadish::Render::Tools::Camera::Down);
						if (mWindow->RawInputGetKeyStatus(Window::VirtualKeys::Shift))
							cameraActions = (HorseRadish::Render::Tools::Camera::CameraAction)(cameraActions | HorseRadish::Render::Tools::Camera::Run);

						//posso actualizar a camera
						auto mousePosition = mWindow->RawInputGetMouseStatus();
						camera->CommitInput(cameraActions, mousePosition[0], mousePosition[1], true, renderer2D->auxTools.curTimeS - renderer2D->auxTools.lastTimeS);
					}

					mWindow->ProcessMessages([&](const Window::Message &msg)
					{
						if (msg.isType(Window::Message::MessageType::CharacterKey))
							mRuntime->callVoidMethod("events.onKeyPress", msg.getParam());

						stage->processMessage(msg);

						consolaGUI->ConsoleProcessMSG(msg);
					}, true);

					//se já passou um segundo
					if (timerSecond.GetTimeMS() > 1000.0)
					{
						//para ajudar nos cálculos
						double tempoAux = 1.0 / timerSecond.GetTimeS(true);

						//ajusto o valor das variáveis
						//this->VarSet("sys.infoFPS", HorseRadish::Math::ftoi(((double)glContext->counterGetValue(HorseRadish::OpenGL::Objects::Context::CounterType::Frames)) * tempoAux));
						//this->VarSet("renderer.infoMTRIS", ((double)glContext->counterGetValue(HorseRadish::OpenGL::Objects::Context::CounterType::Triangles)) * tempoAux);
						//this->VarSet("renderer.infoMVERTS", ((double)glContext->counterGetValue(HorseRadish::OpenGL::Objects::Context::CounterType::Vertices)) * tempoAux);

						VarSet("renderer.stats.timeElapsed", static_cast<int64_t>(renderGlQueryGroup.getResultI64<0>() / 1000));
						VarSet("renderer.stats.samples", static_cast<int64_t>(renderGlQueryGroup.getResultI64<1>()));
						VarSet("renderer.stats.vertices", static_cast<int64_t>(renderGlQueryGroup.getResultI64<2>()));
						VarSet("renderer.stats.primitives", static_cast<int64_t>(renderGlQueryGroup.getResultI64<3>()));
						VarSet("renderer.stats.vertexShader", static_cast<int64_t>(renderGlQueryGroup.getResultI64<4>()));
						VarSet("renderer.stats.fragmentShader", static_cast<int64_t>(renderGlQueryGroup.getResultI64<5>()));
						VarSet("renderer.stats.clipInputPrimitives", static_cast<int64_t>(renderGlQueryGroup.getResultI64<6>()));
						VarSet("renderer.stats.clipOutputPrimitives", static_cast<int64_t>(renderGlQueryGroup.getResultI64<7>()));
					}

					//mando o renderer preparar a próxima frame
					renderData->PrepareNextFrame(*camera, *viewport);
					stage->processStep();

					//leio o tempo que estive à espera de processar as coisas do motor
					timeSpentProcessing = timerFrame.GetTimeMS() - timeSpentDrawing;

					//mostro o que desenhei, isto tá no fim pra ajudar no paralelismo entre CPU e GPU
					glContext->SwapBuffers();

					//leio o tempo que estive à espera de acabar de fazer o swap
					timeSpentIdle = timerFrame.GetTimeMS() - timeSpentProcessing;

					//se tiver coisas para ler do GPU, leio
					//if (renderData->stats.gpuCounter != nullptr)
					//{
					//	float slotTotalInv;

					//	//preciso de calcular o máximo dos tempos para poder fazer as percentagens correctas
					//	slotTotalInv = 1.0f / (timeSpentDrawing + timeSpentProcessing + timeSpentIdle) * 100.0f;

					//	//os contadores a usar
					//	renderData->stats.gpuCounter->sampleCounter("timeSlot_draw3D", timeSpentDrawing * slotTotalInv);
					//	renderData->stats.gpuCounter->sampleCounter("timeSlot_process", timeSpentProcessing * slotTotalInv);
					//	renderData->stats.gpuCounter->sampleCounter("timeSlot_idle", timeSpentIdle * slotTotalInv);
					//	renderData->stats.gpuCounter->sampleCounter("render_numTris", renderData->stats.numTris);
					//	renderData->stats.gpuCounter->sampleCounter("render_glDrawElements", renderData->stats.numGlDrawElements);

					//	//posso avançar com o sample
					//	renderData->stats.gpuCounter->sampleMoveNext();
					//}

					//vejo o estado da musica e quando para, coloco outra
					//if (SAudio::SAudioMusicStatus()==SAUDIO_STATUS_STOPPED)
					//	{
					//	SAudio::SAudioMusicLoad("~/test2.ogg");
					//	SAudio::SAudioMusicAction(SAUDIO_ACTION_PLAY);
					//	}

					//only limit FPS if not developing
					if (this->VarGet<bool>("sys.developer") == 0)
					{
						auto frameTotalTimeMS = timerFrame.GetTimeMS();
						if (frameTotalTimeMS < 16.5) //cap to 60fps
						{
							std::this_thread::sleep_for(std::chrono::milliseconds(HorseRadish::Math::ftoi(16 - frameTotalTimeMS)));
							while (timerFrame.GetTimeMS() < 16.5);
						}
					}

					//agora sim, faço restart do timer
					timerFrame.ReStart();
				}
			}

			stage.reset();

			delete consolaGUI;
			consolaGUI = nullptr;

			delete rendererDeferred;
			rendererDeferred = nullptr;

			delete renderer2D;
			renderer2D = nullptr;

			renderData->Cleanup();

			//delete renderData->stats.gpuCounter;
			//renderData->stats.gpuCounter = nullptr;

			delete camera;
			delete viewport;
			camera = nullptr;
			viewport = nullptr;

			delete renderData;
			renderData = nullptr;

			delete glContext;
			glContext = nullptr;
		}

	} //Engine
} //HorseRadish

