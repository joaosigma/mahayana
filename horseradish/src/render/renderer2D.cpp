#include "renderer2D.hpp"

#include "common\ImageFactory.hpp"

namespace HorseRadish
{
	namespace Render
	{
		Renderer2D::Renderer2D(HorseRadish::OpenGL::Objects::Context * const glContext)
			: Renderer(glContext)
		{
			this->glObjectManager = new HorseRadish::OpenGL::Objects::ObjectsManager(glContext);
			//this->glTextureManager = new HorseRadish::OpenGL::Objects::ObjectsManager(glContext);
			this->glUniformCache = new HorseRadish::OpenGL::Tools::UniformCache();
			this->glImmediateMode = new HorseRadish::OpenGL::Tools::ImmediateMode(102);
		}

		Renderer2D::~Renderer2D()
		{
			this->glObjectManager->ObjectDelete(this->shaders.prog2DDrawNoTex);
			this->glObjectManager->ObjectDelete(this->shaders.prog2DDrawTex);
			this->glObjectManager->ObjectDelete(this->shaders.prog2DText);
			this->glObjectManager->ObjectDelete(this->shaders.progDeferredDebug);
			this->glObjectManager->ObjectDelete(this->shaders.progPPSimpleColor);
			this->glObjectManager->ObjectDelete(this->shaders.progDebugTex);
			this->glObjectManager->ObjectDelete(this->shaders.renderZPass);
			this->shaders.prog2DDrawNoTex = nullptr;
			this->shaders.prog2DDrawTex = nullptr;
			this->shaders.progDeferredDebug = nullptr;
			this->shaders.prog2DText = nullptr;
			this->shaders.progPPSimpleColor = nullptr;
			this->shaders.progDebugTex = nullptr;
			this->shaders.renderZPass = nullptr;

			delete this->gui.fontManager;
			this->gui.font = nullptr;
			this->gui.fontConsole = nullptr;
			this->gui.fontManager = nullptr;

			delete this->glObjectManager;
			//delete this->glTextureManager;
			delete this->glUniformCache;
			delete this->glImmediateMode;

			this->glObjectManager = nullptr;
			//this->glTextureManager = nullptr;
			this->glUniformCache = nullptr;
			this->glImmediateMode = nullptr;
		}

		void Renderer2D::Initialize(const int &renderWidth, const int &renderHeight, HorseRadish::IO::FileSystem * const fileSystem, HorseRadish::Console::Console *mainConsole)
		{
			int textSize;
			const char* textFont;

			textFont = mainConsole->VarGetDataS("sys_consoleTextFont");
			textSize = mainConsole->VarGetDataI("sys_consoleTextSize");

			this->renderWidth = renderWidth;
			this->renderHeight = renderHeight;

			this->shaders.prog2DDrawNoTex = (const HorseRadish::OpenGL::Objects::Program*)this->glObjectManager->ObjectCreate(HorseRadish::OpenGL::Objects::ObjectsManager::Program);
			this->shaders.prog2DDrawTex = (const HorseRadish::OpenGL::Objects::Program*)this->glObjectManager->ObjectCreate(HorseRadish::OpenGL::Objects::ObjectsManager::Program);
			this->glObjectManager->ShadersRead(fileSystem, this->shaders.prog2DDrawNoTex, "shaders/2dDraw.vshader", "shaders/2dDraw.fshader", nullptr);
			this->glObjectManager->ShadersRead(fileSystem, this->shaders.prog2DDrawTex, "shaders/2dDraw.vshader", "shaders/2dDrawTex.fshader", nullptr);

			this->shaders.prog2DText = (const HorseRadish::OpenGL::Objects::Program*)this->glObjectManager->ObjectCreate(HorseRadish::OpenGL::Objects::ObjectsManager::Program);
			this->glObjectManager->ShadersRead(fileSystem, this->shaders.prog2DText, "shaders/2dText.vshader", "shaders/2dText.fshader", nullptr);

			this->shaders.progDeferredDebug = (const HorseRadish::OpenGL::Objects::Program*)this->glObjectManager->ObjectCreate(HorseRadish::OpenGL::Objects::ObjectsManager::Program);
			this->glObjectManager->ShadersRead(fileSystem, this->shaders.progDeferredDebug, "shaders/deferred_gbufferDebug.vshader", "shaders/deferred_gbufferDebug.fshader", nullptr);

			this->shaders.progPPSimpleColor = (const HorseRadish::OpenGL::Objects::Program*)this->glObjectManager->ObjectCreate(HorseRadish::OpenGL::Objects::ObjectsManager::Program);
			this->glObjectManager->ShadersRead(fileSystem, this->shaders.progPPSimpleColor, "shaders/ppSimpleColor.vshader", "shaders/ppSimpleColor.fshader", nullptr);

			this->shaders.progDebugTex = (const HorseRadish::OpenGL::Objects::Program*)this->glObjectManager->ObjectCreate(HorseRadish::OpenGL::Objects::ObjectsManager::Program);
			this->glObjectManager->ShadersRead(fileSystem, this->shaders.progDebugTex, "shaders/2dDrawDebug.vshader", "shaders/2dDrawDebug.fshader", nullptr);

			this->shaders.renderZPass = (const HorseRadish::OpenGL::Objects::Program*)this->glObjectManager->ObjectCreate(HorseRadish::OpenGL::Objects::ObjectsManager::Program);
			this->glObjectManager->ShadersRead(fileSystem, this->shaders.renderZPass, "shaders/rZPass.vshader", "shaders/rZPass.fshader", nullptr);

			this->gui.fontManager = new HorseRadish::OpenGL::Tools::FontManager();
			this->gui.font = this->gui.fontManager->CreateFont(textSize, textFont, this->shaders.prog2DText->glID);
			this->gui.fontConsole = this->gui.fontManager->CreateFont(9, "C:\\Windows\\Fonts\\verdanab.ttf", this->shaders.prog2DText->glID);
		}

	} //Render
} //HorseRadish