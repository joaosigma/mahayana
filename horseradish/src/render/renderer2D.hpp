#pragma once

#include "renderer.hpp"
#include "tools\font.hpp"
#include "common\openGL\tools\immediateMode.hpp"

namespace HorseRadish
{
	namespace Render
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

			}shaders;

			struct GUI{
				std::unique_ptr<Tools::Font> font;
			}gui;

			struct AuxTools{
				HorseRadish::OpenGL::Tools::ImmediateMode *glImmediateMode;
				float curTimeS, lastTimeS;

				AuxTools(){ this->glImmediateMode = nullptr; this->curTimeS = this->lastTimeS = 0.0f; }
			}auxTools;

			std::unique_ptr<HorseRadish::OpenGL::Tools::ImmediateMode> glImmediateMode;
			int renderWidth, renderHeight;

		public:
			Renderer2D(const HorseRadish::OpenGL::Objects::Context& glContext);

			void Initialize(const int &renderWidth, const int &renderHeight, HorseRadish::IO::FileSystem * const fileSystem, const char* const textFont, int textSize);
		};

	}
}
