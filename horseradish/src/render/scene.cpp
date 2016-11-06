#include "scene.hpp"

#include "../common/opengl/openGL.hpp"
#include "../common/opengl/tools/viewport.hpp"

#include "../misc/videoStream.hpp"

namespace HorseRadish {	namespace Render
{
	Scene::Scene(Engine::Runtime& runtime, Engine::Logger::Context& logger, HorseRadish::IO::FileSystem& fileSystem, HorseRadish::OpenGL::Objects::Context &glCtx, const std::string& name, const std::string& filePath, size_t renderWidth, size_t renderHeight)
		: mName(name), mFilePath(filePath)
		, mRuntime(runtime), mLogger(logger), mGlCtx(glCtx)
	{
		Misc::VideoStream::Initialize();

		mVideoData.stream = std::make_unique<Misc::VideoStream>(3, PixelFormat::PIX_FMT_BGR24, mFilePath.c_str());
		if (!mVideoData.stream->isValid())
		{
			mVideoData.stream.reset();
			return;
		}

		mVideoData.stream->getVideoDims(mVideoData.frameSize.width, mVideoData.frameSize.height);

		mRenderData.texVideo.init(HorseRadish::OpenGL::Objects::Texture::Type::TexRectangle, HorseRadish::OpenGL::Objects::Texture::StorageType::RGBA_8, mVideoData.frameSize.width, mVideoData.frameSize.height);

		mRenderData.bufferPBO.init(HorseRadish::OpenGL::Objects::Buffer::Type::PixelUnpackBuffer, mVideoData.stream->getVideoFrameDataSize(), HorseRadish::OpenGL::Objects::Buffer::UsageType::OnlyWrite);

		mRenderData.sampler.init(HorseRadish::OpenGL::Objects::Sampler::FilterType::Linear, HorseRadish::OpenGL::Objects::Sampler::FilterType::Linear, HorseRadish::OpenGL::Objects::Sampler::WrapType::ClampEdge);

		mRenderData.progVertex.init(HorseRadish::OpenGL::Objects::ShaderProgram::Type::Vertex, fileSystem.readFileAsString("shaders/stage.vshader"));
		mRenderData.progFragment.init(HorseRadish::OpenGL::Objects::ShaderProgram::Type::Fragment, fileSystem.readFileAsString("shaders/stage.fshader"));
		//std::string infoLog = mRenderData.progVertex.getInfoLog();
		//infoLog += mRenderData.progFragment.getInfoLog();

		auto matrixProj2D = HorseRadish::OpenGL::Tools::Viewport::genMatrix2DProj(renderWidth, renderHeight);

		HorseRadish::OpenGL::glProgramUniform1i(mRenderData.progFragment.getId(), mRenderData.progFragment.getUniformLocation("texSampler"), 0);
		HorseRadish::OpenGL::glProgramUniformMatrix4fv(mRenderData.progVertex.getId(), mRenderData.progVertex.getUniformLocation("transformationMatrix"), 1, false, matrixProj2D.data());

		mRenderData.progPipeline.init();
		mRenderData.progPipeline.setStage(mRenderData.progVertex);
		mRenderData.progPipeline.setStage(mRenderData.progFragment);

		mRenderData.windowSize.reset(renderWidth, renderHeight);
		mRenderData.proj2D = HorseRadish::OpenGL::Tools::Viewport::genMatrix2DProj(mRenderData.windowSize.width, mRenderData.windowSize.height);
	}

	Scene::~Scene()
	{ }

	bool Scene::processDraw()
	{
		if (!mVideoData.stream || mVideoData.streamEnded)
			return false;

		if (std::chrono::milliseconds(mVideoData.frameTimer.getTimeIntMS()) >= mVideoData.waitDuration)
		{
			bool frameIsAhead;
			HorseRadish::hInt64 frameID;
			double frameDurationS;

			auto frameData = mVideoData.stream->getFrame(frameIsAhead, frameID, frameDurationS);
			if (!frameData && !frameIsAhead)
			{
				mVideoData.streamEnded = true;
				return false;
			}

			if (frameData && (frameID != mVideoData.frameLastID))
			{
				mRenderData.bufferPBO.writeData(frameData, mVideoData.stream->getVideoFrameDataSize(), 0);

				mRenderData.bufferPBO.bind();
				mRenderData.texVideo.uploadData(0, 0, 0, mVideoData.frameSize.width, mVideoData.frameSize.height, HorseRadish::OpenGL::Objects::Texture::DataFormat::BGR, HorseRadish::OpenGL::Objects::Texture::DataType::UBYTE, nullptr);
				mRenderData.bufferPBO.unbind();

				mVideoData.frameLastID = frameID;
			}

			mVideoData.frameTimer.reStart();
			mVideoData.waitDuration = std::chrono::milliseconds(HorseRadish::Math::ftoi(frameDurationS * 1000.0));
		}

		if (mRenderData.fading || (mVideoData.frameLastID >= 0))
		{
			auto viewRect = mVideoData.stream->getVideoRect(mRenderData.windowSize.width, mRenderData.windowSize.height, true);

			HorseRadish::OpenGL::glEnable(GL_BLEND);
			HorseRadish::OpenGL::glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

			HorseRadish::OpenGL::glBindProgramPipeline(mRenderData.progPipeline.getId());

			mRenderData.sampler.bind(0);
			mRenderData.texVideo.bind(0);

			mRenderData.imode.beginDraw(HorseRadish::OpenGL::Tools::ImmediateMode::GeometryType::Quads);
			mRenderData.imode.setColorF(1.0f, 1.0f, 1.0f, mRenderData.fadingAlpha);

			mRenderData.imode.setTexCoord(0.0f, mVideoData.frameSize.height);
			mRenderData.imode.addPosition(viewRect.x, viewRect.y);

			mRenderData.imode.setTexCoord(mVideoData.frameSize.width, mVideoData.frameSize.height);
			mRenderData.imode.addPosition(viewRect.x + viewRect.width, viewRect.y);

			mRenderData.imode.setTexCoord(mVideoData.frameSize.width, 0.0f);
			mRenderData.imode.addPosition(viewRect.x + viewRect.width, viewRect.y + viewRect.height);

			mRenderData.imode.setTexCoord(0.0f, 0.0f);
			mRenderData.imode.addPosition(viewRect.x, viewRect.y + viewRect.height);
			mRenderData.imode.endDraw();

			HorseRadish::OpenGL::glDisable(GL_BLEND);
			HorseRadish::OpenGL::glBindProgramPipeline(0);
		}

		return true;
	}

	void Scene::processMessage(const std::string& msg, const std::string& payload)
	{
		if (msg == "exit")
		{
			if (mVideoData.stream)
				mRenderData.fading = true;
		}
	}

	void Scene::processStep(double stepDeltaMS, double stepTotalMS, std::function<void(const std::string& msg, const std::string& payload)> cbMessages)
	{
		if (mRenderData.fading)
		{
			mRenderData.fadingAlpha -= stepDeltaMS / 1500.0;
			if (mRenderData.fadingAlpha <= 0.0f)
			{
				mVideoData.streamEnded = true;

				mRenderData.fading = false;
				mRenderData.fadingAlpha = 0.0f;
			}
		}

		if (mVideoData.streamEnded)
		{
			mVideoData.streamEnded = false;
			mVideoData.stream.reset();

			cbMessages("finished", "");
			return;
		}

		if (mVideoData.stream)
			mVideoData.stream->process();
	}
} }