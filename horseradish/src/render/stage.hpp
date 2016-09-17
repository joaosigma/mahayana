#pragma once

#include "render/scene.hpp"
#include "engine/logger.hpp"
#include "engine/runtime.hpp"
#include "common/opengl/objects.hpp"
#include "common/opengl/tools/viewport.hpp"

#include "platform/window.hpp"

namespace HorseRadish {	namespace Render {

	class Stage
	{
		class SceneRuntimeProxy;

	private:
		bool mScenesDrawned = false;
		Engine::Runtime& mRuntime;
		Engine::Logger::Context& mLogger;
		HorseRadish::IO::FileSystem& mFileSystem;
		std::vector<SceneRuntimeProxy*> mTempScenes;
		HorseRadish::OpenGL::Objects::Context &mGlCtx;
		std::unordered_map<std::string, SceneRuntimeProxy*> mRuntimeScenes;

		struct RenderData
		{
			HorseRadish::OpenGL::Objects::FrameBuffer fbo;
			HorseRadish::OpenGL::Objects::Sampler sampler;
			HorseRadish::OpenGL::Objects::ShaderProgram progVertex;
			HorseRadish::OpenGL::Objects::ShaderProgram progFragment;
			HorseRadish::OpenGL::Objects::ProgramPipeline progPipeline;
			HorseRadish::OpenGL::Objects::Texture texColor;

			HorseRadish::OpenGL::Tools::ImmediateMode imode;
		} mRenderData;

		void runtimeFuncSceneGet(const std::string &funcName, Engine::Runtime::FunctionReturnContext &ctx);
		void runtimeFuncSceneCreate(const std::string &funcName, Engine::Runtime::FunctionReturnContext &ctx);
		void runtimeFuncSceneDestroy(const std::string &funcName, Engine::Runtime::FunctionReturnContext &ctx);
		void runtimeFuncSceneAddRemove(const std::string &funcName, Engine::Runtime::FunctionReturnContext &ctx);

	public:
		Stage(Engine::Runtime& runtime, Engine::Logger::Context& logger, HorseRadish::IO::FileSystem& fileSystem, HorseRadish::OpenGL::Objects::Context &glCtx, size_t renderWidth, size_t renderHeight);
		~Stage();

		Stage(const Stage&) = delete;
		Stage& operator=(const Stage&) = delete;

		void drawScenes();
		void drawComposite(const HorseRadish::OpenGL::Tools::Viewport& viewport);
		void processStep();
		void processMessage(const Window::Message& msg);
	};

} }