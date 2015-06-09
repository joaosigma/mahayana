#pragma once

#include "engine/logger.hpp"
#include "engine/runtime.hpp"
#include "common/Timer.hpp"
#include "common/Matrix.hpp"
#include "common\FileSystem.hpp"
#include "common/Primitives2D.hpp"
#include "common/opengl/tools\immediateMode.hpp"
#include "common/opengl/objects.hpp"

#include <chrono>
#include <string>

class VideoStream;

namespace HorseRadish {	namespace Render {

	class Scene
	{
		friend class Stage;

		std::string mName;
		std::string mFilePath;
		Engine::Runtime& mRuntime;
		Engine::Logger::Context& mLogger;
		HorseRadish::OpenGL::Objects::Context& mGlCtx;

		struct VideoData {
			bool streamEnded;
			Timer frameTimer;
			VideoStream *stream;
			HorseRadish::hInt64 frameLastID;
			std::chrono::milliseconds waitDuration;
			HorseRadish::Primitives2D::Size<int> frameSize;
		} mVideoData;
		struct RenderData {
			bool fading;
			float fadingAlpha;
			HorseRadish::Matrix proj2D;
			HorseRadish::Primitives2D::Size<int> windowSize;
			HorseRadish::OpenGL::Tools::ImmediateMode *imode;
			HorseRadish::OpenGL::Objects::Texture texVideo;
			
			HorseRadish::OpenGL::Objects::Buffer bufferPBO;
			HorseRadish::OpenGL::Objects::Sampler sampler;
			HorseRadish::OpenGL::Objects::ShaderProgram progVertex;
			HorseRadish::OpenGL::Objects::ShaderProgram progFragment;
			HorseRadish::OpenGL::Objects::ProgramPipeline progPipeline;
		} mRenderData;
		

	public:
		Scene(Engine::Runtime& runtime, Engine::Logger::Context& logger, HorseRadish::IO::FileSystem& fileSystem, HorseRadish::OpenGL::Objects::Context &glCtx, const std::string& name, const std::string& filePath, unsigned int glRenderWidth, unsigned int glRenderHeight);
		~Scene();

		Scene(const Scene&) = delete;
		Scene& operator=(const Scene&) = delete;

		bool processDraw();
		void processMessage(const std::string& msg, const std::string& payload);
		void processStep(double stepDeltaMS, double stepTotalMS, std::function<void(const std::string&, const std::string&)> cbMessages);

		const std::string& getName() const
		{
			return mName;
		}
	};

} }