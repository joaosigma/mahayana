#pragma once

#include "../engine/logger.hpp"
#include "../engine/runtime.hpp"
#include "../common/timer.hpp"
#include "../common/matrix.hpp"
#include "../common/fileSystem.hpp"
#include "../common/primitives2D.hpp"
#include "../common/opengl/tools/immediateMode.hpp"
#include "../common/opengl/objects.hpp"

#include <chrono>
#include <string>

namespace hr::misc
{
	class VideoStream;
}

namespace hr::render
{
	class Scene
	{
		friend class Stage;

		std::string mName;
		std::string mFilePath;
		engine::Runtime& mRuntime;
		engine::Logger::Context& mLogger;
		hr::gl::objects::Context& mGlCtx;

		struct VideoData {
			bool streamEnded = false;
			Timer frameTimer;
			std::unique_ptr<misc::VideoStream> stream;
			int64_t frameLastID = -1;
			std::chrono::milliseconds waitDuration = std::chrono::milliseconds::zero();
			hr::Size<size_t> frameSize;
		} mVideoData;
		struct RenderData {
			bool fading = false;
			float fadingAlpha = 1.0f;
			hr::Matrix4f proj2D = hr::Matrix4f::identity();
			hr::Size<int> windowSize;
			hr::gl::tools::ImmediateMode imode;
			hr::gl::objects::Texture texVideo;

			hr::gl::objects::Buffer bufferPBO;
			hr::gl::objects::Sampler sampler;
			hr::gl::objects::ShaderProgram progVertex;
			hr::gl::objects::ShaderProgram progFragment;
			hr::gl::objects::ProgramPipeline progPipeline;
		} mRenderData;


	public:
		Scene(engine::Runtime& runtime, engine::Logger::Context& logger, hr::io::FileSystem& fileSystem, hr::gl::objects::Context& glCtx, const std::string& name, const std::string& filePath, size_t renderWidth, size_t renderHeight);
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
}
