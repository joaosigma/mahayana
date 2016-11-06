#include "renderer2D.hpp"

#include "../common/imageFactory.hpp"

namespace hr { namespace render
{
	void Renderer2D::initialize(size_t renderWidth, size_t renderHeight, hr::io::FileSystem * const fileSystem, const char* const textFont, size_t  textSize)
	{
		mRenderWidth = renderWidth;
		mRenderHeight = renderHeight;

		mShaders.drawNoTex.progVertex.init(hr::gl::objects::ShaderProgram::Type::Vertex, fileSystem->readFileAsString("shaders/2dDraw.vshader"));
		mShaders.drawNoTex.progFragment.init(hr::gl::objects::ShaderProgram::Type::Fragment, fileSystem->readFileAsString("shaders/2dDraw.fshader"));

		mShaders.drawNoTex.progPipeline.init();
		mShaders.drawNoTex.progPipeline.setStage(mShaders.drawNoTex.progVertex);
		mShaders.drawNoTex.progPipeline.setStage(mShaders.drawNoTex.progFragment);

		mShaders.text.progVertex.init(hr::gl::objects::ShaderProgram::Type::Vertex, fileSystem->readFileAsString("shaders/2dText.vshader"));
		mShaders.text.progFragment.init(hr::gl::objects::ShaderProgram::Type::Fragment, fileSystem->readFileAsString("shaders/2dText.fshader"));

		mShaders.text.progPipeline.init();
		mShaders.text.progPipeline.setStage(mShaders.text.progVertex);
		mShaders.text.progPipeline.setStage(mShaders.text.progFragment);

		mGui.font = std::make_unique<tools::Font>(textSize, textFont, mShaders.text.progVertex.getId(), mShaders.text.progFragment.getId(), mShaders.text.progPipeline.getId());
	}
} }
