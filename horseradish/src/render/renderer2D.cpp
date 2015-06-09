#include "renderer2D.hpp"

#include "common\ImageFactory.hpp"

namespace HorseRadish
{
	namespace Render
	{
		Renderer2D::Renderer2D(const HorseRadish::OpenGL::Objects::Context& glContext)
			: Renderer(glContext)
		{
			this->glImmediateMode = std::make_unique<HorseRadish::OpenGL::Tools::ImmediateMode>(102);
		}

		void Renderer2D::Initialize(const int &renderWidth, const int &renderHeight, HorseRadish::IO::FileSystem * const fileSystem, const char* const textFont, int textSize)
		{
			this->renderWidth = renderWidth;
			this->renderHeight = renderHeight;

			shaders.drawNoTex.progVertex.init(HorseRadish::OpenGL::Objects::ShaderProgram::Type::Vertex, fileSystem->readFileAsString("shaders/2dDraw.vshader"));
			shaders.drawNoTex.progFragment.init(HorseRadish::OpenGL::Objects::ShaderProgram::Type::Fragment, fileSystem->readFileAsString("shaders/2dDraw.fshader"));

			shaders.drawNoTex.progPipeline.init();
			shaders.drawNoTex.progPipeline.setStage(shaders.drawNoTex.progVertex);
			shaders.drawNoTex.progPipeline.setStage(shaders.drawNoTex.progFragment);

			shaders.text.progVertex.init(HorseRadish::OpenGL::Objects::ShaderProgram::Type::Vertex, fileSystem->readFileAsString("shaders/2dText.vshader"));
			shaders.text.progFragment.init(HorseRadish::OpenGL::Objects::ShaderProgram::Type::Fragment, fileSystem->readFileAsString("shaders/2dText.fshader"));

			shaders.text.progPipeline.init();
			shaders.text.progPipeline.setStage(shaders.text.progVertex);
			shaders.text.progPipeline.setStage(shaders.text.progFragment);

			this->gui.font = std::make_unique<Tools::Font>(textSize, textFont, shaders.text.progVertex.getId(), shaders.text.progFragment.getId(), shaders.text.progPipeline.getId());
		}

	} //Render
} //HorseRadish