#include "consoleUI.hpp"

#include "common\Platform.hpp"
#include "common\Timer.hpp"
#include "common\ImageFactory.hpp"
#include "common\opengl\openGL.hpp"
#include "common\opengl\openGLext.hpp"
#include "common\opengl\objects.hpp"

#include <ZMouse.h>
#include <math.h>
#include <shellapi.h>

namespace HorseRadish
{
	namespace Console
	{
		namespace UI
		{
			ConsoleGUI::ConsoleTab::ConsoleTab()
			{
			}
			ConsoleGUI::ConsoleTab::~ConsoleTab()
			{
			}

			void ConsoleGUI::ConsoleTab::SetContentAreaSize(const int &width, const int &height)
			{
				tabContentAreaSize.Set(width, height);
			}
			void ConsoleGUI::ConsoleTab::SetContentAreaOrigin(const int &x, const int &y)
			{
				tabContentAreaPos.Set(x, y);
			}

			void ConsoleGUI::drawConsoleBackground(HorseRadish::Render::Renderer2D * const renderData, const HorseRadish::Matrix &transformMatrix, const float &consoleAlpha)
			{
				HorseRadish::OpenGL::glUseProgram(0);
				HorseRadish::OpenGL::glBindProgramPipeline(renderData->shaders.drawNoTex.progFragment.getId());
				HorseRadish::OpenGL::glProgramUniformMatrix4fv(renderData->shaders.drawNoTex.progVertex.getId(), renderData->shaders.drawNoTex.progVertex.getUniformLocation("transformationMatrix"), 1, false, transformMatrix);

				auto& glImmediateMode = renderData->glImmediateMode;
				auto& guiFont = renderData->gui.font;

				glImmediateMode->beginDraw(HorseRadish::OpenGL::Tools::ImmediateMode::GeometryType::Quads);
					glImmediateMode->setColor(0, 0, 0, HorseRadish::Color::ConvertColor(consoleAlpha));
					glImmediateMode->addQuad(0.0f, 0.0f, this->consoleRect.width, this->consoleRect.height);
				glImmediateMode->endDraw();

				HorseRadish::OpenGL::glLineWidth(2.0f);
				HorseRadish::OpenGL::glEnable(GL_LINE_SMOOTH);

				glImmediateMode->beginDraw(HorseRadish::OpenGL::Tools::ImmediateMode::GeometryType::Lines);
					glImmediateMode->setColor(128, 128, 128, 255);
					glImmediateMode->addLineV(0.0f, 0.0f, this->consoleRect.height);
					glImmediateMode->addLineH(0.0f, this->consoleRect.width, 0.0f);
					glImmediateMode->addLineV(this->consoleRect.width, 0.0f, this->consoleRect.height);
					glImmediateMode->addLineH(0.0f, this->consoleRect.width, this->consoleRect.height);
				glImmediateMode->endDraw();

				HorseRadish::OpenGL::glLineWidth(1.0f);
				HorseRadish::OpenGL::glDisable(GL_LINE_SMOOTH);

				HorseRadish::OpenGL::glUseProgram(0);
				HorseRadish::OpenGL::glBindProgramPipeline(renderData->shaders.drawNoTex.progFragment.getId());
				HorseRadish::OpenGL::glProgramUniformMatrix4fv(renderData->shaders.drawNoTex.progVertex.getId(), renderData->shaders.drawNoTex.progVertex.getUniformLocation("transformationMatrix"), 1, false, transformMatrix);
			}

			void ConsoleGUI::drawConsole(HorseRadish::Render::Renderer2D * const renderData, const HorseRadish::OpenGL::Tools::Viewport * const hrViewport)
			{
				if (this->consoleVisible == false)
					return;

				auto consoleAlpha = 0.6f;

				HorseRadish::Matrix transformMatrix = hrViewport->getProjection(HorseRadish::OpenGL::Tools::Viewport::ProjectionType::Proj2D);

				auto& glImmediateMode = renderData->glImmediateMode;

				drawConsoleBackground(renderData, transformMatrix, consoleAlpha);

				mainTab->DrawContent(renderData, transformMatrix);
			}

			ConsoleGUI::ConsoleGUI(HorseRadish::Render::Renderer2D* const renderData)
			{
				this->mainTab = nullptr;
				this->renderData = renderData;

				this->consoleVisible = false;

				auto& guiFont = renderData->gui.font;

				this->consoleRect.width = renderData->renderWidth - 60;
				this->consoleRect.height = renderData->renderHeight - 60;
				this->consoleRect.x = (renderData->renderWidth - this->consoleRect.width) / 2;
				this->consoleRect.y = (renderData->renderHeight - this->consoleRect.height) / 2;
			}

			ConsoleGUI::~ConsoleGUI()
			{
			}

			void ConsoleGUI::Draw(const HorseRadish::OpenGL::Tools::Viewport * const hrViewport)
			{
				this->drawConsole(this->renderData, hrViewport);
			}

			bool ConsoleGUI::GUIVisivel() const
			{
				return this->consoleVisible;
			}

			void ConsoleGUI::ConsoleAddTab(ConsoleTab * const newConsoleTab)
			{
				if (newConsoleTab == nullptr)
					return;

				this->mainTab = newConsoleTab;

				newConsoleTab->SetContentAreaOrigin(0, 0);
				newConsoleTab->SetContentAreaSize(this->consoleRect.width - 0 * 2, this->consoleRect.height - 0 * 2);
			}

			void ConsoleGUI::ConsoleProcessMSG(const Window::Message &msg)
			{
				if (this->consoleVisible && mainTab)
					mainTab->ProcessMSG(msg);

				if ((msg.getType() == Window::Message::MessageType::VirtualKey) && (msg.getParam() == static_cast<int>(Window::VirtualKeys::Escape)))
				{
					this->consoleVisible = !this->consoleVisible;
					return;
				}
			}

			void ConsoleGUI::ConsoleVisible(const bool visible)
			{
				this->consoleVisible = visible;
			}

			bool ConsoleGUI::ConsoleConsumesInput()
			{
				return this->consoleVisible;
			}

		} //UI
	} //Console
} //HorseRadish