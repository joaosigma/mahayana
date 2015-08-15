#pragma once

#include "renderer.hpp"
#include "tools/font.hpp"
#include "common/FileSystem.hpp"
#include "common/openGL/tools/immediateMode.hpp"

namespace HorseRadish { namespace Render {

	class Renderer2D : public Renderer
	{
	public:
		struct Shaders{
			struct {
				HorseRadish::OpenGL::Objects::ShaderProgram progVertex;
				HorseRadish::OpenGL::Objects::ShaderProgram progFragment;
				HorseRadish::OpenGL::Objects::ProgramPipeline progPipeline;
			} drawNoTex, text;

		}mShaders;

		struct GUI{
			std::unique_ptr<Tools::Font> font;
		}mGui;

		int mRenderWidth, mRenderHeight;
		HorseRadish::OpenGL::Tools::ImmediateMode mGlImmediateMode;

	public:
		Renderer2D(const HorseRadish::OpenGL::Objects::Context& glContext);

		void initialize(const int &renderWidth, const int &renderHeight, HorseRadish::IO::FileSystem * const fileSystem, const char* const textFont, int textSize);
	};

} }
