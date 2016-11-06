#pragma once

#include "renderer.hpp"
#include "tools/font.hpp"
#include "../common/fileSystem.hpp"
#include "../common/openGL/tools/immediateMode.hpp"

namespace hr { namespace render
{
	class Renderer2D : public Renderer
	{
	public:
		struct Shaders{
			struct {
				hr::gl::objects::ShaderProgram progVertex;
				hr::gl::objects::ShaderProgram progFragment;
				hr::gl::objects::ProgramPipeline progPipeline;
			} drawNoTex, text;

		}mShaders;

		struct GUI{
			std::unique_ptr<tools::Font> font;
		}mGui;

		size_t mRenderWidth = 0, mRenderHeight = 0;
		hr::gl::tools::ImmediateMode mGlImmediateMode;

	public:
		Renderer2D(const hr::gl::objects::Context& glContext)
			: Renderer(glContext)
		{ }

		void initialize(size_t renderWidth, size_t renderHeight, hr::io::FileSystem * const fileSystem, const char* const textFont, size_t textSize);
	};
} }
