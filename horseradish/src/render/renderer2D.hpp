#pragma once

#include "renderer.hpp"
#include "tools/font.hpp"
#include "../common/fileSystem.hpp"
#include "../common/openGL/tools/immediateMode.hpp"

namespace HorseRadish { namespace Render
{
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

		size_t mRenderWidth = 0, mRenderHeight = 0;
		HorseRadish::OpenGL::Tools::ImmediateMode mGlImmediateMode;

	public:
		Renderer2D(const HorseRadish::OpenGL::Objects::Context& glContext)
			: Renderer(glContext)
		{ }

		void initialize(size_t renderWidth, size_t renderHeight, HorseRadish::IO::FileSystem * const fileSystem, const char* const textFont, size_t textSize);
	};
} }
