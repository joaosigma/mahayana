#pragma once
#ifndef __HR_RENDER_RENDERER_2D__
#define __HR_RENDER_RENDERER_2D__

#include "renderer.hpp"

#include "console\console.hpp"

namespace HorseRadish
{

namespace Render
{

//§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§
//§§§§§§   Classe Renderer	§§§§§
//§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§
class Renderer2D : public Renderer
{
public:
	struct Shaders{
		const HorseRadish::OpenGL::Objects::Program *progDeferredGBuffer;
		const HorseRadish::OpenGL::Objects::Program *progMainDebug;
		const HorseRadish::OpenGL::Objects::Program *progPerVertexLightDir;

		const HorseRadish::OpenGL::Objects::Program *prog2DDrawNoTex, *prog2DDrawTex;
		const HorseRadish::OpenGL::Objects::Program *prog2DText;

		const HorseRadish::OpenGL::Objects::Program *progDeferredDebug;

		const HorseRadish::OpenGL::Objects::Program *progPPSimpleColor;

		const HorseRadish::OpenGL::Objects::Program *renderZPass;

		const HorseRadish::OpenGL::Objects::Program *progDebugTex;

	}shaders;

	struct GUI{
		HorseRadish::OpenGL::Tools::Font *font, *fontConsole;
		HorseRadish::OpenGL::Tools::FontManager *fontManager;

		GUI() : font(nullptr), fontConsole(0), fontManager(nullptr) { }
	}gui;

	struct AuxTools{
		HorseRadish::OpenGL::Tools::UniformCache *uniformCache;
		HorseRadish::OpenGL::Tools::ImmediateMode *glImmediateMode;
		float curTimeS, lastTimeS;

		AuxTools(){ this->uniformCache = nullptr; this->glImmediateMode = nullptr; this->curTimeS = this->lastTimeS = 0.0f; }
	}auxTools;

	HorseRadish::OpenGL::Objects::ObjectsManager *glObjectManager;
	HorseRadish::OpenGL::Tools::UniformCache *glUniformCache;
	HorseRadish::OpenGL::Tools::ImmediateMode *glImmediateMode;
	int renderWidth, renderHeight;
	

public:
	Renderer2D(HorseRadish::OpenGL::Objects::Context * const glContext);
	~Renderer2D();

	void Initialize(const int &renderWidth, const int &renderHeight, HorseRadish::IO::FileSystem * const fileSystem, HorseRadish::Console::Console *mainConsole);
};

}//namespace Render
}//namespace HorseRadish

#endif