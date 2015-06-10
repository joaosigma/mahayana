#pragma once

#include "common\Platform.hpp"
#include "common\Timer.hpp"
#include "common\Primitives2D.hpp"
#include "common\opengl\objects.hpp"
#include "common\openGL\tools\viewport.hpp"
#include "render\renderer2D.hpp"
#include "render\rendererDeferred.hpp"
#include "..\platform\window.hpp"

#include "engine\logger.hpp"

#include <string>
#include <vector>

namespace HorseRadish
{
	namespace Console
	{
		namespace UI
		{
			class ConsoleGUI
			{
			public:
				class ConsoleTab
				{
				protected:
					HorseRadish::Primitives2D::Point<int> tabContentAreaPos;
					HorseRadish::Primitives2D::Size<int> tabContentAreaSize;

				public:
					ConsoleTab();
					virtual ~ConsoleTab();

					void SetContentAreaSize(const int &width, const int &height);
					void SetContentAreaOrigin(const int &x, const int &y);

					virtual void DrawContent(HorseRadish::Render::Renderer2D* const render2D, const HorseRadish::Matrix &transformMatrix) = 0;
					virtual void ProcessMSG(const Window::Message &msg) = 0;
				};

			private:
				HorseRadish::Render::Renderer2D *renderData;

				bool consoleVisible;
				ConsoleTab *mainTab;
				HorseRadish::Primitives2D::Rectangle<int> consoleRect;
				
				void drawConsoleBackground(HorseRadish::Render::Renderer2D * const render2D, const HorseRadish::Matrix &transformMatrix, const float &consoleAlpha);
				void drawConsole(HorseRadish::Render::Renderer2D * const render2D, const HorseRadish::OpenGL::Tools::Viewport * const hrViewport);

			public:
				ConsoleGUI(HorseRadish::Render::Renderer2D* const render2D);
				~ConsoleGUI();

				void Draw(const HorseRadish::OpenGL::Tools::Viewport * const hrViewport);
				bool GUIVisivel() const;

				void ConsoleAddTab(ConsoleTab * const newConsoleTab);
				void ConsoleProcessMSG(const Window::Message &msg);
				void ConsoleVisible(const bool visible);
				bool ConsoleConsumesInput();
			};

			class ConsoleUserHistory;
			class ConsoleUserPrompt;

			class ConsoleTabConsole : public ConsoleGUI::ConsoleTab
			{
				struct LINHA_TEXTO{
					std::string texto;
					bool active, formatted;
					Engine::Logger::EntryType type;
				}*listaTexto;

				int numMaxLinhasTexto;
				HorseRadish::Timer cursorTimer;
				bool cursorVisivel;
				ConsoleUserHistory *userHistory;
				ConsoleUserPrompt *userPrompt;
				std::function<void(const char * const)> mInputCb;
				std::shared_ptr<Engine::Logger> mLogger;
				int textoOffset;

			public:
				ConsoleTabConsole(std::function<void(const char * const)> inputCb, std::shared_ptr<Engine::Logger> logger);
				~ConsoleTabConsole();

				void DrawContent(HorseRadish::Render::Renderer2D* const render2D, const HorseRadish::Matrix &transformMatrix);
				void ProcessMSG(const Window::Message &msg);
				bool CriaTextoConsola(HorseRadish::Render::Renderer2D* const render2D);
				void ActualizaTextoConsola();
			};

		} //UI
	} //Console
} //HorseRadish

