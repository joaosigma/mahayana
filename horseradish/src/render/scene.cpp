#include "scene.hpp"

#include "../common/opengl/openGL.hpp"
#include "../common/opengl/tools/viewport.hpp"

#include "../misc/videoStream.hpp"

namespace hr { namespace render
{
	Scene::Scene(engine::Runtime& runtime, engine::Logger::Context& logger, hr::io::FileSystem& fileSystem, hr::gl::objects::Context &glCtx, const std::string& name, const std::string& filePath, size_t renderWidth, size_t renderHeight)
		: mName(name), mFilePath(filePath)
		, mRuntime(runtime), mLogger(logger), mGlCtx(glCtx)
	{
		misc::VideoStream::Initialize();

		mVideoData.stream = std::make_unique<misc::VideoStream>(3, PixelFormat::PIX_FMT_BGR24, mFilePath.c_str());
		if (!mVideoData.stream->isValid())
		{
			mVideoData.stream.reset();
			return;
		}

		mVideoData.stream->getVideoDims(mVideoData.frameSize.width, mVideoData.frameSize.height);

		mRenderData.texVideo.init(hr::gl::objects::Texture::Type::TexRectangle, hr::gl::objects::Texture::StorageType::RGBA_8, mVideoData.frameSize.width, mVideoData.frameSize.height);

		mRenderData.bufferPBO.init(hr::gl::objects::Buffer::Type::PixelUnpackBuffer, mVideoData.stream->getVideoFrameDataSize(), hr::gl::objects::Buffer::UsageType::OnlyWrite);

		mRenderData.sampler.init(hr::gl::objects::Sampler::FilterType::Linear, hr::gl::objects::Sampler::FilterType::Linear, hr::gl::objects::Sampler::WrapType::ClampEdge);

		mRenderData.progVertex.init(hr::gl::objects::ShaderProgram::Type::Vertex, fileSystem.readFileAsString("shaders/stage.vshader"));
		mRenderData.progFragment.init(hr::gl::objects::ShaderProgram::Type::Fragment, fileSystem.readFileAsString("shaders/stage.fshader"));
		//std::string infoLog = mRenderData.progVertex.getInfoLog();
		//infoLog += mRenderData.progFragment.getInfoLog();

		auto matrixProj2D = hr::gl::tools::Viewport::genMatrix2DProj(renderWidth, renderHeight);

		hr::gl::glProgramUniform1i(mRenderData.progFragment.getId(), mRenderData.progFragment.getUniformLocation("texSampler"), 0);
		hr::gl::glProgramUniformMatrix4fv(mRenderData.progVertex.getId(), mRenderData.progVertex.getUniformLocation("transformationMatrix"), 1, false, matrixProj2D.data());

		mRenderData.progPipeline.init();
		mRenderData.progPipeline.setStage(mRenderData.progVertex);
		mRenderData.progPipeline.setStage(mRenderData.progFragment);

		mRenderData.windowSize.reset(renderWidth, renderHeight);
		mRenderData.proj2D = hr::gl::tools::Viewport::genMatrix2DProj(mRenderData.windowSize.width, mRenderData.windowSize.height);
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
			hr::hInt64 frameID;
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
				mRenderData.texVideo.uploadData(0, 0, 0, mVideoData.frameSize.width, mVideoData.frameSize.height, hr::gl::objects::Texture::DataFormat::BGR, hr::gl::objects::Texture::DataType::UBYTE, nullptr);
				mRenderData.bufferPBO.unbind();

				mVideoData.frameLastID = frameID;
			}

			mVideoData.frameTimer.reStart();
			mVideoData.waitDuration = std::chrono::milliseconds(hr::Math::ftoi(frameDurationS * 1000.0));
		}

		if (mRenderData.fading || (mVideoData.frameLastID >= 0))
		{
			auto viewRect = mVideoData.stream->getVideoRect(mRenderData.windowSize.width, mRenderData.windowSize.height, true);

			hr::gl::glEnable(GL_BLEND);
			hr::gl::glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

			hr::gl::glBindProgramPipeline(mRenderData.progPipeline.getId());

			mRenderData.sampler.bind(0);
			mRenderData.texVideo.bind(0);

			mRenderData.imode.beginDraw(hr::gl::tools::ImmediateMode::GeometryType::Quads);
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

			hr::gl::glDisable(GL_BLEND);
			hr::gl::glBindProgramPipeline(0);
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