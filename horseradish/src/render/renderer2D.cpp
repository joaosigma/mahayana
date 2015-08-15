#include "renderer2D.hpp"

#include "common/ImageFactory.hpp"

namespace HorseRadish
{
	namespace Render
	{
		Renderer2D::Renderer2D(const HorseRadish::OpenGL::Objects::Context& glContext)
			: Renderer(glContext)
		{
		}

		void Renderer2D::initialize(const int &renderWidth, const int &renderHeight, HorseRadish::IO::FileSystem * const fileSystem, const char* const textFont, int textSize)
		{
			mRenderWidth = renderWidth;
			mRenderHeight = renderHeight;

			mShaders.drawNoTex.progVertex.init(HorseRadish::OpenGL::Objects::ShaderProgram::Type::Vertex, fileSystem->readFileAsString("shaders/2dDraw.vshader"));
			mShaders.drawNoTex.progFragment.init(HorseRadish::OpenGL::Objects::ShaderProgram::Type::Fragment, fileSystem->readFileAsString("shaders/2dDraw.fshader"));

			mShaders.drawNoTex.progPipeline.init();
			mShaders.drawNoTex.progPipeline.setStage(mShaders.drawNoTex.progVertex);
			mShaders.drawNoTex.progPipeline.setStage(mShaders.drawNoTex.progFragment);

			mShaders.text.progVertex.init(HorseRadish::OpenGL::Objects::ShaderProgram::Type::Vertex, fileSystem->readFileAsString("shaders/2dText.vshader"));
			mShaders.text.progFragment.init(HorseRadish::OpenGL::Objects::ShaderProgram::Type::Fragment, fileSystem->readFileAsString("shaders/2dText.fshader"));

			mShaders.text.progPipeline.init();
			mShaders.text.progPipeline.setStage(mShaders.text.progVertex);
			mShaders.text.progPipeline.setStage(mShaders.text.progFragment);

			mGui.font = std::make_unique<Tools::Font>(textSize, textFont, mShaders.text.progVertex.getId(), mShaders.text.progFragment.getId(), mShaders.text.progPipeline.getId());
		}

	} //Render
} //HorseRadish