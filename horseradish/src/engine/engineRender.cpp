#include "engine.hpp"

#include <format>

#include "profiler.hpp"

#include "../common/timer.hpp"
#include "../common/opengl/openGL.hpp"
#include "../common/opengl/tools/viewport.hpp"
#include "../common/vulkan/app.hpp"
#include "../common/vulkan/build_tools.hpp"

#include "../render/stage.hpp"
#include "../render/world.hpp"
#include "../render/worldEditor.hpp"
#include "../render/consoleUI.hpp"
#include "../render/profilerUI.hpp"
#include "../render/rendererMain.hpp"
#include "../render/rendererDebug.hpp"
#include "../render/renderer2D.hpp"

#include "../common/imageFactory.hpp"

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
			hr::gl::glFrontFace(GL_CW);

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
			hr::engine::Logger::Context ctx(logger, hr::engine::Logger::ModuleType::Graphics);

			//driver info
			ctx.info("${olive}->${default}OpenGL driver info:");
			ctx.info("   OpenGL version: {}", glContext.infoStr(hr::gl::objects::Context::InformationType::Version).value_or(""));
			ctx.info("   OpenGL vendor: {}", glContext.infoStr(hr::gl::objects::Context::InformationType::Vendor).value_or(""));
			ctx.info("   OpenGL renderer: {}", glContext.infoStr(hr::gl::objects::Context::InformationType::Renderer).value_or(""));

			//extensions available
			ctx.info("${olive}->${default}OpenGL extensions available:");
			{
				int infoValueInt;
				hr::gl::glGetIntegerv(GL_NUM_EXTENSIONS, &infoValueInt);
				for (int curExt = 0; curExt < infoValueInt; curExt++)
					ctx.info((const char *)hr::gl::glGetStringi(GL_EXTENSIONS, curExt));
			}

			//other stuff
			ctx.info("${olive}->${default}OpenGL extended information:");
			ctx.info("   GLSL version: {}", glContext.infoStr(hr::gl::objects::Context::InformationType::GLSLVersion).value_or(""));
			ctx.info("   Maximum number of draw buffers: {}", glContext.infoInt(hr::gl::objects::Context::InformationType::MaxDrawBuffers).value_or(0));
			ctx.info("   Maximum number of color attachments in FBOs: {}", glContext.infoInt(hr::gl::objects::Context::InformationType::MaxColorAttachments).value_or(0));
			ctx.info("   Maximum 1D/2D texture size: {0}x{0}", glContext.infoInt(hr::gl::objects::Context::InformationType::MaxTextureSize).value_or(0));
			ctx.info("   Maximum 3D texture size: {0}x{0}x{0}", glContext.infoInt(hr::gl::objects::Context::InformationType::MaxTexture3DSize).value_or(0));
			ctx.info("   Maximum cubemap texture size: {0}x{0}", glContext.infoInt(hr::gl::objects::Context::InformationType::MaxTextureCubemapSize).value_or(0));
			ctx.info("   Maximum rectangle texture size: {0}x{0}", glContext.infoInt(hr::gl::objects::Context::InformationType::MaxTextureRectSize).value_or(0));
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

		//start everything related to Vulkan
		std::unique_ptr<hr::vulkan::App> vulkanApp;
		{
			vulkanApp = std::make_unique<hr::vulkan::App>([this](vulkan::App::DebugMessageSeverity severity, vulkan::App::App::DebugMessageContext context, std::string_view source, std::string_view msg)
			{
				std::string_view severityStr;
				switch (severity)
				{
					case vulkan::App::DebugMessageSeverity::Verbose:
						severityStr = "verbose";
						break;
					case vulkan::App::DebugMessageSeverity::Info:
						severityStr = "info";
						break;
					case vulkan::App::DebugMessageSeverity::Warning:
						severityStr = "warn";
						break;
					case vulkan::App::DebugMessageSeverity::Error:
						severityStr = "error";
						break;
				}

				std::string contextStr;
				if ((context & vulkan::App::App::DebugMessageContext::General) == vulkan::App::App::DebugMessageContext::General)
					contextStr.append("general, ");
				if ((context & vulkan::App::App::DebugMessageContext::Validation) == vulkan::App::App::DebugMessageContext::Validation)
					contextStr.append("validation, ");
				if ((context & vulkan::App::App::DebugMessageContext::Performance) == vulkan::App::App::DebugMessageContext::Performance)
					contextStr.append("performance, ");
				if ((context & vulkan::App::App::DebugMessageContext::DeviceAddressBinding) == vulkan::App::App::DebugMessageContext::DeviceAddressBinding)
					contextStr.append("device address binding, ");

				if (!contextStr.empty())
					contextStr.erase(contextStr.size() - 2);

				mLoggerRenderCtx->info(std::format("VK {} ({}) {}: {}", severityStr, contextStr, source, msg));
			});

			auto setupSurface = [this](auto vulkanInstance)
			{
				return mWindow->setupVulkanSurface(vulkanInstance);
			};

			if (!vulkanApp->init(setupSurface))
			{
				exit(ExitAction::Nothing, std::format("Unable to initialize Vulkan: {}", vulkanApp->lastError()));
				return;
			}

			mLoggerRuntimeCtx->info("${olive}->${default}Vulkan layer initialized.");

			//compile the shaders (from GLSL to SPIR-V)
			if (isDevMove)
			{
				assert(!vulkan::GLSLCPath.empty());
				auto compilerPath = std::filesystem::canonical(std::filesystem::path(vulkan::GLSLCPath));
				compilerPath.make_preferred();

				auto searchPath = std::filesystem::canonical(std::filesystem::current_path() / "../shaders");
				searchPath.make_preferred() /= "*.?shader";

				bool hasErrors{false};
				hr::io::FileSystem::findFiles(searchPath, true, [this, &compilerPath, &hasErrors](const std::filesystem::path &path, const uint64_t &)
				{
					std::vector<std::string> errors;
					auto success = vulkan::Object<VkShaderModule>::compileShader(path, [this, &compilerPath, &errors](const std::filesystem::path &input, const std::filesystem::path &output, vulkan::Object<VkShaderModule>::ShaderType shaderType)
					{
						std::string_view shaderName;
						switch (shaderType)
						{
							case vulkan::Object<VkShaderModule>::ShaderType::Vertex:
								shaderName = "vertex";
								break;
							case vulkan::Object<VkShaderModule>::ShaderType::Fragment:
								shaderName = "fragment";
								break;
							case vulkan::Object<VkShaderModule>::ShaderType::Compute:
								shaderName = "compute";
								break;
							default:
								return false;
						}

						auto args = std::format("{} -fshader-stage={} -x glsl {} -o {}", compilerPath.string(), shaderName, input.string(), output.string());

						std::optional<std::string> error{std::string{}};
						auto res = platform::Platform::execute(args, error);
						if (error.has_value() && (!res.has_value() || (res.value() != 0)))
							errors.push_back(*std::move(error));
							
						return true; //keep going
					});

					if (!errors.empty())
					{
						std::string fullErrorMsg;
						
						fullErrorMsg += std::format("Unable to compile '{0}' to SPIR-V:{1}{1}", path.string(), platform::Platform::NewLine);
						for (const auto &error : errors)
							fullErrorMsg += std::format("{0}{1}", error, platform::Platform::NewLine);

						mLoggerRuntimeCtx->error(fullErrorMsg);
					}

					hasErrors |= !success;
					return true; //keep going
				});

				if (hasErrors)
				{
					exit(ExitAction::Nothing, "Errors detected while compiling the shaders (check the logs for more information).");
					return;
				}
			}
		}

		//start everything related to OpenGL
		//{
		//	int glMajorVersion, glMinorVersion;
		//
		//	glContext = std::make_unique<platform::OpenglContext>(*mWindow, "OpenGL32.dll", 4, 6, var<bool>("renderer.glDebug"), true);
		//	if (!glContext->isValid())
		//	{
		//		if (auto errorMsg = glContext->getErrorMsg(); errorMsg.empty())
		//			exit(ExitAction::Nothing, "Unable to create OpenGL context");
		//		else
		//			exit(ExitAction::Nothing, std::format("Unable to create OpenGL context: {}", errorMsg).c_str());
		//
		//		return;
		//	}
		//
		//	hr::gl::glGetIntegerv(GL_MAJOR_VERSION, &glMajorVersion);
		//	hr::gl::glGetIntegerv(GL_MINOR_VERSION, &glMinorVersion);
		//	if ((glMajorVersion < 4) || ((glMajorVersion == 4) && (glMinorVersion < 6)))
		//	{
		//		exit(ExitAction::Nothing, "OpenGL version 4.6 or higher is required (try updating your drivers).");
		//		return;
		//	}
		//
		//	if (!glContext->isExtPresent("GL_EXT_texture_compression_s3tc"))
		//	{
		//		exit(ExitAction::Nothing, "Required extensions are not present");
		//		return;
		//	}
		//
		//	openglInitialize();
		//	glContext->swapBuffers(); //to force a black screen
		//	glContext->setSwapInterval(var<int>("renderer.winSwapInterval"));
		//
		//	mLoggerRenderCtx->info("${olive}->${default}OpenGL system initialized.");
		//	openGLWriteInfo(*mLogger, *glContext);
		//
		//	if (var<bool>("renderer.glDebug"))
		//	{
		//		hr::gl::glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DONT_CARE, 0, nullptr, GL_TRUE);
		//		hr::gl::glDebugMessageCallback(openglDebugMessagesCallback, mLogger.get());
		//		hr::gl::glEnable(GL_DEBUG_OUTPUT);
		//		hr::gl::glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
		//
		//		glContext->dispatchDebugMessages();
		//	}
		//}

		//start everything related to the renderers (worl, deferred renderer, etc.)
		//{
		//	size_t displayWidth = var<int>("renderer.dims.width");
		//	size_t displayHeight = var<int>("renderer.dims.height");
		//
		//	renderer2D = std::make_unique<hr::render::Renderer2D>(*glContext);
		//	renderer2D->initialize(displayWidth, displayHeight, mFileSystem.get(), var<std::string>("sys.console.text.font").c_str());
		//
		//	rendererDebug = std::make_unique<hr::render::RendererDebug>(*glContext, *mFileSystem, *renderer2D);
		//
		//	rendererMain = std::make_unique<hr::render::RendererMain>(*glContext, *mFileSystem, displayWidth, displayHeight);
		//
		//	stage = std::make_unique<render::Stage>(*mRuntime, *mLoggerRuntimeCtx, *mFileSystem, *glContext, displayWidth, displayHeight);
		//
		//	//!!!!!!!!!!!!!!!! dev
		//	{
		//		renderData = std::make_unique<hr::render::World>();
		//		renderData->loadArea(*rendererMain, "../scenes/sphere_bot.hscene", "../scenes/sphere_bot.hbin");
		//	}
		//}
			
		//start other renderers (misc) related stuff
		//{
		//	if (Profiler::isSupported())
		//	{
		//		profilerUI = std::make_unique<hr::render::ProfilerUI>(*profiler, *renderer2D);
		//		profilerUI->setInfoState(isDevMove);
		//	}
		//
		//	if (isDevMove)
		//		consoleUI = std::make_unique<hr::render::ConsoleUI>(*mLogger, *renderer2D, 20);
		//}
			
		//initialization finished
		mLoggerRenderCtx->info(" ");
		mLoggerRenderCtx->info("${#FF9B00}Aplic${#FF8800}ation ${#FF8000}ready ${#FF6A00}to ${#FF6300}go...");
		mLoggerRenderCtx->info("   type \"help\" to print information regarding this console (or optionally for a command or variable)");
		mLoggerRenderCtx->info("   type \"com_list\" for a list of commands and variables (or \"com_listBind\" for a list of binds)");
		mLoggerRenderCtx->info(" ");
		mLoggerRenderCtx->info("===============================================");
		mLoggerRenderCtx->info(" ");

		mRuntime->callVoidMethod("events.ready");

		//hr::gl::glEnable(GL_FRAMEBUFFER_SRGB);

		//we are about to enter the main render loop
		{
			//setup camera
			hr::render::tools::CameraFPS camera;
			camera.setPos(0.0f, 0.0f, 1.0f);
			camera.setTarget(0.0f, 0.0f, 0.0f);
			camera.setMovementScale(hr::render::tools::CameraFPS::CameraInput::Keyboard, 10.0f);

			hr::gl::tools::Viewport viewportRender(Math::Deg2Rad<float> * 45.0f, var<int>("renderer.dims.width"), var<int>("renderer.dims.height"), 0.05f);
			//hr::gl::glViewport(0, 0, viewportRender.width(), viewportRender.height());

			//hr::gl::objects::Query::Group<8> renderGlQueryGroup = {
			//	hr::gl::objects::Query::Type::TimeElapsed, hr::gl::objects::Query::Type::SamplesPassed,
			//	hr::gl::objects::Query::Type::VerticesSubmitted, hr::gl::objects::Query::Type::PrimitivesSubmitted,
			//	hr::gl::objects::Query::Type::VertexShaderInvocations, hr::gl::objects::Query::Type::FragmentShaderInvocations,
			//	hr::gl::objects::Query::Type::ClippingInputPrimitives, hr::gl::objects::Query::Type::ClippingOutputPrimitives
			//};

			Timestep timestep(50);

			auto vulkanCmdPool = vulkanApp->createCommandPool();

			//texture
			vulkan::Image vulkanTexture;
			vulkan::ImageView vulkanTextureView;
			vulkan::Memory vulkanMemoryTexture;
			{
				size_t imgWidth{0}, imgHeight{0};
				vulkan::Memory stagingMemoryImg;
				vulkan::Buffer stagingBufferImg;
				{
					hr::imaging::Image<uint8_t, hr::imaging::ImageFormatRGBA> imgData;
					{
						auto fstream = mFileSystem->fileRead(R"(C:\Users\sigma\Desktop\texture.jpg)");
						hr::streams::StreamReader reader(*fstream);
						auto img = hr::imaging::Factory::readJPG(reader);
						imgData = img.convert<uint8_t, hr::imaging::ImageFormatRGBA>(0, 255);
					}

					stagingBufferImg = vulkan::Buffer::gen(vulkanApp->device(), imgData.size(), VK_BUFFER_USAGE_TRANSFER_SRC_BIT);
					stagingMemoryImg = vulkanApp->allocateMemory(stagingBufferImg.hostRequiredSize(), stagingBufferImg.hostRequiredMemoryType(),
					  VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

					stagingBufferImg.allocate(stagingMemoryImg);
					stagingMemoryImg.write(std::as_bytes(imgData.asSpan()), 0);

					imgWidth = imgData.width();
					imgHeight = imgData.height();
				}

				vulkanTexture = vulkan::Image::gen2D(vulkanApp->device(), imgWidth, imgHeight, false, VK_FORMAT_R8G8B8A8_SRGB);
				vulkanMemoryTexture =
				  vulkanApp->allocateMemory(vulkanTexture.hostRequiredSize(), vulkanTexture.hostRequiredMemoryType(), VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
				vulkanTexture.allocate(vulkanMemoryTexture);

				vulkanApp->executeOneTimeCommand(vulkanCmdPool,
				  [&vulkanTexture, &stagingBufferImg, &imgWidth, &imgHeight](vulkan::CommandBuffer::Recorder &recorder)
				  {
					  //transition to VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL
					  {
						  VkImageMemoryBarrier barrier{};
						  barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
						  barrier.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
						  barrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
						  barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
						  barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
						  barrier.image = vulkanTexture.native();
						  barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
						  barrier.subresourceRange.baseMipLevel = 0;
						  barrier.subresourceRange.levelCount = 1;
						  barrier.subresourceRange.baseArrayLayer = 0;
						  barrier.subresourceRange.layerCount = 1;
						  barrier.srcAccessMask = 0;
						  barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

						  recorder.pipelineBarrier(VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, barrier);
					  }

					  //actual transfer
					  recorder.copyBufferToImage(vulkanTexture.native(), stagingBufferImg.native(), imgWidth, imgHeight);

					  //transition to VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL
					  {
						  VkImageMemoryBarrier barrier{};
						  barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
						  barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
						  barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
						  barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
						  barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
						  barrier.image = vulkanTexture.native();
						  barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
						  barrier.subresourceRange.baseMipLevel = 0;
						  barrier.subresourceRange.levelCount = 1;
						  barrier.subresourceRange.baseArrayLayer = 0;
						  barrier.subresourceRange.layerCount = 1;
						  barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
						  barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

						  recorder.pipelineBarrier(VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, barrier);
					  }
				  });

				vulkanTextureView = vulkan::ImageView::gen2D(vulkanApp->device(), vulkanTexture.native(), VK_FORMAT_R8G8B8A8_SRGB);
			}

			vulkan::Sampler vulkanSampler = vulkan::Sampler::create(vulkanApp->device(), VK_FILTER_LINEAR, VK_FILTER_LINEAR);

			struct Vertex
			{
				float pos[2];
				float color[3];
				float uv[2];
			};

			//data
			struct UniformBufferObject
			{
				Matrix4f model = Matrix4f::zero();
				Matrix4f view = Matrix4f::zero();
				Matrix4f proj = Matrix4f::zero();
			};

			vulkan::Memory vulkanMemory;
			vulkan::Buffer vulkanBufferVertexIndices;
			vulkan::Buffer vulkanBufferVertexData;

			vulkan::Memory vulkanMemoryUBO;
			vulkan::Buffer vulkanBufferUBO;
			vulkan::DescriptorSetLayout vulkanDescriptorSetLayout;
			vulkan::DescriptorPool vulkanDescriptorPool = vulkan::DescriptorPool::Builder(vulkanApp->device())
			                                                .addDescriptor(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1)
			                                                .addDescriptor(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1)
			                                                .build(true, 1);
			vulkan::DescriptorSet vulkanDescriptorSet;
			{
				const std::vector<Vertex> vertices = {{{-0.5f, -0.5f}, {1.0f, 0.0f, 0.0f}, {1.0f, 0.0f}}, {{0.5f, -0.5f}, {0.0f, 1.0f, 0.0f}, {0.0f, 0.0f}},
				  {{0.5f, 0.5f}, {0.0f, 0.0f, 1.0f}, {0.0f, 1.0f}}, {{-0.5f, 0.5f}, {1.0f, 1.0f, 1.0f}, {1.0f, 1.0f}}};

				const std::vector<uint16_t> indices = {0, 1, 2, 2, 3, 0};

				vulkanBufferVertexData =
				  vulkan::Buffer::gen(vulkanApp->device(), vertices.size() * sizeof(Vertex), VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT);
				vulkanBufferVertexIndices =
				  vulkan::Buffer::gen(vulkanApp->device(), indices.size() * sizeof(uint16_t), VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT);
				assert(vulkanBufferVertexData.hostRequiredMemoryType() == vulkanBufferVertexIndices.hostRequiredMemoryType());

				auto totalSize = vulkanBufferVertexData.hostRequiredSize();
				totalSize += vulkanBufferVertexIndices.hostRequiredSize();
				vulkanMemory = vulkanApp->allocateMemory(totalSize, vulkanBufferVertexData.hostRequiredMemoryType(), VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

				vulkanBufferVertexData.allocate(vulkanMemory);
				vulkanApp->transferData(vulkanBufferVertexData, 0, std::as_bytes(std::span{vertices}), vulkanCmdPool);

				vulkanBufferVertexIndices.allocate(vulkanMemory);
				vulkanApp->transferData(vulkanBufferVertexIndices, 0, std::as_bytes(std::span{indices}), vulkanCmdPool);

				vulkanBufferUBO = vulkan::Buffer::gen(vulkanApp->device(), sizeof(UniformBufferObject), VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT);
				assert(vulkanBufferUBO.size() == vulkanBufferUBO.hostRequiredSize());
				vulkanMemoryUBO = vulkanApp->allocateMemory(vulkanBufferUBO.hostRequiredSize(), vulkanBufferUBO.hostRequiredMemoryType(),
				  VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

				vulkanBufferUBO.allocate(vulkanMemoryUBO);
				vulkanMemoryUBO.memMap(vulkanBufferUBO.size(), 0);

				vulkanDescriptorSetLayout = vulkan::DescriptorSetLayout::Builder(vulkanApp->device())
				                              .addUbo(0, VK_SHADER_STAGE_VERTEX_BIT)
				                              .addSampler(1, VK_SHADER_STAGE_FRAGMENT_BIT)
				                              .build();

				vulkanDescriptorSet = vulkanDescriptorPool.allocateDescriptorSet(vulkanDescriptorSetLayout.native());
				vulkanDescriptorSet.updateUniformBuffer(0, vulkanBufferUBO.native())
				  .updateImageViewSampler(1, vulkanTextureView.native(), vulkanSampler.native())
				  .save();
			}

			auto vulkanFrameFence = hr::vulkan::Fence::gen(vulkanApp->device(), true);
			auto vulkanSwapChainImageReady = hr::vulkan::Semaphore::gen(vulkanApp->device());

			hr::vulkan::RenderPass renderPass;
			{
				auto builder = hr::vulkan::RenderPass::Builder(vulkanApp->device());
				builder
					.addAttachment(vulkanApp->swapChainImageFormat())
					.addSubpass()
					.addSubpassDependency();
				renderPass = builder.build();
			}

			//init the swapchain (i.e.: how to render to the swapchain images)
			vulkanApp->swapChainInit(renderPass.native());

			hr::vulkan::Pipeline vulkanPipeline;
			{
				auto shaderVertex = hr::vulkan::ShaderModule::loadShader(vulkanApp->device(), "../shaders/temp.v_spv");
				auto shaderFragment = hr::vulkan::ShaderModule::loadShader(vulkanApp->device(), "../shaders/temp.f_spv");

				auto builder = hr::vulkan::Pipeline::Builder(vulkanApp->device());

				builder.addVertexBinding(0, sizeof(Vertex))
				  .addVertexAttribute(0, 0, offsetof(Vertex, pos), VK_FORMAT_R32G32_SFLOAT)
				  .addVertexAttribute(0, 1, offsetof(Vertex, color), VK_FORMAT_R32G32B32_SFLOAT)
				  .addVertexAttribute(0, 2, offsetof(Vertex, uv), VK_FORMAT_R32G32_SFLOAT)
				  .addDescriptorSetLayout(vulkanDescriptorSetLayout.native())
				  .setupInputAssembly(VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST, false)
				  .setupViewport(0.0f, 0.0f, static_cast<float>(viewportRender.width()), static_cast<float>(viewportRender.height()), 0.0f, 1.0f)
				  .setupScissor(0, 0, static_cast<uint32_t>(viewportRender.width()), static_cast<uint32_t>(viewportRender.height()))
				  .setupShader(std::move(shaderVertex), hr::vulkan::ShaderModule::ShaderType::Vertex)
				  .setupShader(std::move(shaderFragment), hr::vulkan::ShaderModule::ShaderType::Fragment);

				vulkanPipeline = builder.build(renderPass.native());
			}

			
			auto vulkanCmdBuffer = vulkanCmdPool.allocateBuffer(true);
			auto vulkanCmdBufferDone = hr::vulkan::Semaphore::gen(vulkanApp->device());

			//main render loop
			while (mCurState == State::Running)
			{
				hr::Timer timerFrame;
				double lastFrameTimeS = std::chrono::duration_cast<std::chrono::duration<double, std::chrono::seconds::period>>(timestep.update()).count();

				profiler->nextSample();

				//--------------------
				//Start frame rendering requests to queue stuff onto the GPU
				//--------------------

				//wait for the previous frame to submit and reset for the next frame
				vulkanFrameFence
					.wait()
					.reset();

				//where to draw to
				auto vulkanSwapChainImage = vulkanApp->swapChainAcquireImage(vulkanSwapChainImageReady.native());

				//record command buffer
				{
					vulkanCmdBuffer.reset();
					auto recorder = vulkanCmdBuffer.record(false);
					
					recorder->doRenderPass(renderPass.native(), vulkanApp->swapChainFramebuffer(vulkanSwapChainImage), viewportRender.width(), viewportRender.height(),
					  [&vulkanPipeline, &vulkanBufferVertexData, &vulkanBufferVertexIndices, &vulkanDescriptorSet](auto &recorder)
					  {
						  recorder.bindPipeline(vulkanPipeline.native());
						  recorder.bindDescriptorSets(vulkanPipeline.layout(), vulkanDescriptorSet.native());
						  recorder.bindVertexBuffer(0, vulkanBufferVertexData.native(), 0);
						  recorder.bindIndexBuffer(vulkanBufferVertexIndices.native(), 0, VK_INDEX_TYPE_UINT16);
						  //scissor and viewport were already set at the pipeline
						  recorder.draw(6);
					  });
					
					recorder->finish();
				}

				//update UBO
				{
					UniformBufferObject ubo;

					ubo.model = Matrix4f::identity();
					ubo.view = camera.modelView();
					ubo.proj = viewportRender.getProjection(hr::gl::tools::Viewport::ProjectionType::Proj3D);

					std::memcpy(vulkanMemoryUBO.memMappedPtr(), &ubo, sizeof(UniformBufferObject));
				}

				vulkanApp->graphicsQueueSubmit(vulkanSwapChainImageReady.native(), VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, vulkanCmdBuffer.native(),
				  vulkanCmdBufferDone.native(),
				  vulkanFrameFence.native());

				////in case nothing is drawn
				//hr::gl::glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
				//hr::gl::glDepthMask(GL_TRUE);
				//hr::gl::glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
				//
				//if (Profiler::isSupported())
				//	renderGlQueryGroup.queriesBegin();

				////draw main, deferred scene
				//rendererMain->render(camera, viewportRender);
				//rendererMain->renderDebug(*rendererDebug, *renderData, camera, viewportRender);
				//rendererMain->renderComposite(viewportRender);
				//
				//if (Profiler::isSupported())
				//	renderGlQueryGroup.queriesEnd();

				////draw stage
				//stage->drawScenes();
				//stage->drawComposite(viewportRender);

				if (Profiler::isSupported())
					profiler->addSample(Profiler::StatId::FrameDraw, timerFrame.getTimeIntMS());

				////draw console and/or profiler
				//if ((profilerUI && profilerUI->isVisible()) || (consoleUI && consoleUI->isVisible()))
				//{
				//	auto fontSize = static_cast<size_t>(var<int>("sys.console.text.size"));
				//
				//	hr::gl::glBindFramebuffer(GL_FRAMEBUFFER, 0);
				//
				//	hr::gl::glEnable(GL_BLEND);
				//	hr::gl::glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
				//
				//	if (profilerUI && profilerUI->isVisible())
				//		profilerUI->draw(fontSize, viewportRender);
				//
				//	if (consoleUI && consoleUI->isVisible())
				//		consoleUI->draw(fontSize, viewportRender);
				//
				//	hr::gl::glDisable(GL_BLEND);
				//}

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

					if (stage)
						stage->processMessage(msg);

					if (consoleUI)
						consoleUI->processMsg(msg, [&](const char * const newInput) { mRuntime->runScript(newInput); });

				}, true);

				//if (Profiler::isSupported())
				//{
				//	profiler->addSample(Profiler::StatId::GPUTimeElapsed, static_cast<int64_t>(renderGlQueryGroup.getResultI64<0>() / 1000));
				//	profiler->addSample(Profiler::StatId::GPUSamples, static_cast<int64_t>(renderGlQueryGroup.getResultI64<1>()));
				//	profiler->addSample(Profiler::StatId::GPUVerticesSubmitted, static_cast<int64_t>(renderGlQueryGroup.getResultI64<2>()));
				//	profiler->addSample(Profiler::StatId::GPUPrimitivesSubmitted, static_cast<int64_t>(renderGlQueryGroup.getResultI64<3>()));
				//	profiler->addSample(Profiler::StatId::GPUVertexShaderInvocations, static_cast<int64_t>(renderGlQueryGroup.getResultI64<4>()));
				//	profiler->addSample(Profiler::StatId::GPUFragmentShaderInvocations, static_cast<int64_t>(renderGlQueryGroup.getResultI64<5>()));
				//	profiler->addSample(Profiler::StatId::GPUClipInputPrimitives, static_cast<int64_t>(renderGlQueryGroup.getResultI64<6>()));
				//	profiler->addSample(Profiler::StatId::GPUClipOutputPrimitives, static_cast<int64_t>(renderGlQueryGroup.getResultI64<7>()));
				//}

				//run any simulations that require a fixed timestep
				timestep.processSim([](double deltaTimeMS)
				{
				});

				//process step in the render data, stage and console
				//{
				//	hr::render::World::Timestep wTimestep;
				//	wTimestep.t = std::chrono::duration<float, std::chrono::seconds::period>(timestep.totalElapsed()).count();
				//
				//	renderData->prepareNextFrame(wTimestep, *rendererMain, camera, viewportRender);
				//	stage->processStep();
				//}

				if (consoleUI)
					consoleUI->processStep();
					
				if (profilerUI)
					profilerUI->processStats();

				if (Profiler::isSupported())
					profiler->addSample(Profiler::StatId::FrameGPU, timerFrame.getTimeIntMS());

				//--------------------
				//Simulations are finished, we can swap GPU buffers and move on
				//--------------------

				//glContext->swapBuffers();

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

				vulkanSwapChainImage.present(vulkanCmdBufferDone.native()); //return / present the frame back to the swapchain
			}

			vulkanApp->waitDeviceIdle(); //wait for any remaining work to finish (before we start destroying everything)
		}

		stage.reset();

		profilerUI.reset();
		profiler.reset();

		consoleUI.reset();

		rendererMain.reset();
		rendererDebug.reset();
		renderer2D.reset();

		renderData.reset();

		vulkanApp.reset();
	}
}
