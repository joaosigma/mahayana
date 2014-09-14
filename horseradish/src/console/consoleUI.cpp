#include "consoleUI.hpp"

#include "common\Platform.hpp"
#include "common\Timer.hpp"
#include "common\ImageFactory.hpp"
#include "common\opengl\openGL.hpp"
#include "common\opengl\openGLext.hpp"
#include "common\opengl\objects.hpp"
#include "common\openGL\tools.hpp"

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
				tabTextAreaWidth = 0;
			}
			ConsoleGUI::ConsoleTab::~ConsoleTab()
			{
			}

			const HorseRadish::String* ConsoleGUI::ConsoleTab::GetText()
			{
				return &tabName;
			}
			int ConsoleGUI::ConsoleTab::GetTextAreaWidth()
			{
				return tabTextAreaWidth;
			}
			void ConsoleGUI::ConsoleTab::SetTextAreaWidth(const int &width)
			{
				tabTextAreaWidth = width;
			}
			void ConsoleGUI::ConsoleTab::SetContentAreaSize(const int &width, const int &height)
			{
				tabContentAreaSize.Set(width, height);
			}
			void ConsoleGUI::ConsoleTab::SetContentAreaOrigin(const int &x, const int &y)
			{
				tabContentAreaPos.Set(x, y);
			}
			void ConsoleGUI::ConsoleTab::SetContentScreenDelta(const int &x, const int &y)
			{
				tabContentScreenDelta.Set(x, y);
			}

			const int ConsoleGUI::tabContentMargin = 4;
			const int ConsoleGUI::infoVarSlack = 4;
			const int ConsoleGUI::smsMessageSlack = 5;
			const float ConsoleGUI::smsTimeVisible = 5.0f;
			const float ConsoleGUI::smsTime2Hide = 1.0f;

			void ConsoleGUI::infoCheckWidth(const HorseRadish::OpenGL::Tools::Font * const guiFont)
			{
				int curWidth, maxWidth;
				char bufferOut[256];
				HorseRadish::String unicodeStr;

				maxWidth = 0;
				for (const auto &curVar : this->infoArrayVars)
				{
					unicodeStr.Set(HorseRadish::String::Encoding::UTF8, mainConsole->VarPrint(curVar.GetData(), bufferOut, 256));
					curWidth = guiFont->getStringWidth(unicodeStr);
					if (curWidth > maxWidth)
						maxWidth = curWidth;
				}

				if (maxWidth > this->infoMaxWidth)
					this->infoMaxWidth = maxWidth;
			}

			void ConsoleGUI::drawConsoleBackground(HorseRadish::Render::Renderer2D * const renderData, const HorseRadish::Matrix &transformMatrix, const float &consoleAlpha)
			{
				int selectedTabX, selectedTabWidth;

				selectedTabX = 0;
				for (int i = 0; i < this->consoleSelectedTab; i++)
					selectedTabX += this->consoleTabPages[i]->GetTextAreaWidth();

				selectedTabWidth = this->consoleTabPages[this->consoleSelectedTab]->GetTextAreaWidth();

				renderData->shaders.prog2DDrawNoTex->Bind();
				HorseRadish::OpenGL::glUniformMatrix4fv(renderData->glUniformCache->GetUniformPos(renderData->shaders.prog2DDrawNoTex->glID, "transformationMatrix"), 1, GL_FALSE, transformMatrix);

				auto glImmediateMode = renderData->glImmediateMode;
				auto guiFont = renderData->gui.font;

				glImmediateMode->BeginDraw(HorseRadish::OpenGL::Tools::ImmediateMode::GeometryType::Quads);
				glImmediateMode->AddColor(0, 0, 0, HorseRadish::Color::ConvertColor(consoleAlpha));
				glImmediateMode->AddQuad(0.0f, 0.0f, this->consoleRect.width, this->consoleRect.height - this->consoleTabSpace.height);
				glImmediateMode->AddQuad(0.0f, this->consoleRect.height - this->consoleTabSpace.height, this->consoleTabSpace.width, this->consoleTabSpace.height);
				glImmediateMode->EndDraw();

				HorseRadish::OpenGL::glLineWidth(2.0f);
				HorseRadish::OpenGL::glEnable(GL_LINE_SMOOTH);

				glImmediateMode->BeginDraw(HorseRadish::OpenGL::Tools::ImmediateMode::GeometryType::Lines);

				if (this->consoleSelectedTab == 0)
				{
					if (this->consoleTabPages.size() > 1)
					{
						glImmediateMode->AddColor(68, 68, 68, 255);

						glImmediateMode->AddLineH(selectedTabWidth, this->consoleTabSpace.width, this->consoleRect.height - 2.0f);

						auto curTabX = selectedTabWidth;
						for (int i = 1; i < consoleTabPages.size(); i++)
						{
							curTabX += this->consoleTabPages[i]->GetTextAreaWidth();
							glImmediateMode->AddLineV(curTabX, this->consoleRect.height - this->consoleTabSpace.height, this->consoleRect.height - 2.0);
						}
					}

					glImmediateMode->AddColor(128, 128, 128, 255);
					glImmediateMode->AddLineV(0.0f, this->consoleRect.height - this->consoleTabSpace.height, this->consoleRect.height);
					glImmediateMode->AddLineH(0.0f, selectedTabWidth, this->consoleRect.height);
					glImmediateMode->AddLineV(selectedTabWidth, this->consoleRect.height - this->consoleTabSpace.height, this->consoleRect.height);
					glImmediateMode->AddLineH(selectedTabWidth, this->consoleRect.width, this->consoleRect.height - this->consoleTabSpace.height);
				}
				else
				{
					int curTabX;

					glImmediateMode->AddColor(68, 68, 68, 255);

					glImmediateMode->AddLineH(0.0f, selectedTabX, this->consoleRect.height - 2.0f);
					if (this->consoleSelectedTab < (this->consoleTabPages.size() - 1))
						glImmediateMode->AddLineH(selectedTabX + selectedTabWidth, this->consoleTabSpace.width, this->consoleRect.height - 2.0f);

					curTabX = 0;
					for (int i = 0; i < this->consoleSelectedTab; i++)
					{
						glImmediateMode->AddLineV(curTabX, this->consoleRect.height - this->consoleTabSpace.height, this->consoleRect.height - 2.0f);
						curTabX += this->consoleTabPages[i]->GetTextAreaWidth();
					}

					curTabX = selectedTabX + selectedTabWidth;
					for (int i = this->consoleSelectedTab + 1; i < this->consoleTabPages.size(); i++)
					{
						curTabX += this->consoleTabPages[i]->GetTextAreaWidth();
						glImmediateMode->AddLineV(curTabX, this->consoleRect.height - this->consoleTabSpace.height, this->consoleRect.height - 2.0f);
					}

					glImmediateMode->AddColor(128, 128, 128, 255);
					glImmediateMode->AddLineH(0.0f, selectedTabX, this->consoleRect.height - this->consoleTabSpace.height);
					glImmediateMode->AddLineV(selectedTabX, this->consoleRect.height - this->consoleTabSpace.height, this->consoleRect.height);
					glImmediateMode->AddLineH(selectedTabX, selectedTabX + selectedTabWidth, this->consoleRect.height);
					glImmediateMode->AddLineV(selectedTabX + selectedTabWidth, this->consoleRect.height - this->consoleTabSpace.height, this->consoleRect.height);
					glImmediateMode->AddLineH(selectedTabX + selectedTabWidth, this->consoleRect.width, this->consoleRect.height - this->consoleTabSpace.height);
				}

				glImmediateMode->AddColor(128, 128, 128, 255);
				glImmediateMode->AddLineV(0.0f, 0.0f, this->consoleRect.height - this->consoleTabSpace.height);
				glImmediateMode->AddLineH(0.0f, this->consoleRect.width, 0.0f);
				glImmediateMode->AddLineV(this->consoleRect.width, 0.0f, this->consoleRect.height - this->consoleTabSpace.height);
				glImmediateMode->EndDraw();

				HorseRadish::OpenGL::glLineWidth(1.0f);
				HorseRadish::OpenGL::glDisable(GL_LINE_SMOOTH);

				if (texConsoleLogo != nullptr)
				{
					HorseRadish::OpenGL::glBindTextureUnit(0, texConsoleLogo->glID);

					renderData->shaders.prog2DDrawTex->Bind();
					HorseRadish::OpenGL::glUniform1i(renderData->glUniformCache->GetUniformPos(renderData->shaders.prog2DDrawTex->glID, "texColorSampler"), 0);
					HorseRadish::OpenGL::glUniformMatrix4fv(renderData->glUniformCache->GetUniformPos(renderData->shaders.prog2DDrawTex->glID, "transformationMatrix"), 1, GL_FALSE, transformMatrix);

					glImmediateMode->BeginDraw(HorseRadish::OpenGL::Tools::ImmediateMode::GeometryType::Quads);
					glImmediateMode->AddColor(255, 255, 255, 255);
					glImmediateMode->AddTexCoord(0.0f, 0.0f);
					glImmediateMode->AddPosition(this->consoleRect.width - this->consoleLogoSize.width, this->consoleRect.height - this->consoleLogoSize.height);
					glImmediateMode->AddTexCoord(1.0f, 0.0f);
					glImmediateMode->AddPosition(this->consoleRect.width, this->consoleRect.height - this->consoleLogoSize.height);
					glImmediateMode->AddTexCoord(1.0f, 1.0f);
					glImmediateMode->AddPosition(this->consoleRect.width, this->consoleRect.height);
					glImmediateMode->AddTexCoord(0.0f, 1.0f);
					glImmediateMode->AddPosition(this->consoleRect.width - this->consoleLogoSize.width, this->consoleRect.height);
					glImmediateMode->EndDraw();
				}

				guiFont->paintBegin(transformMatrix);
				guiFont->setColor(1.0f, 1.0f, 1.0f);

				for (int i = 0, curStartX = 0; i < this->consoleTabPages.size(); i++)
				{
					int posX, posY;

					if (i == this->consoleSelectedTab)
						guiFont->setColor(1.0f, 1.0f, 1.0f);
					else
						guiFont->setColor(0.7f, 0.7f, 0.7f);

					posX = (this->consoleTabPages[i]->GetTextAreaWidth() - guiFont->getStringWidth(*this->consoleTabPages[i]->GetText())) / 2;
					posX += curStartX;
					posY = this->consoleRect.height - this->consoleTabSpace.height;
					posY += (this->consoleTabSpace.height - guiFont->getMaxHeight()) / 2;
					curStartX += this->consoleTabPages[i]->GetTextAreaWidth();
					guiFont->write(posX, posY, *this->consoleTabPages[i]->GetText());
				}

				guiFont->draw();
				guiFont->paintEnd();

				renderData->shaders.prog2DDrawNoTex->Bind();
				HorseRadish::OpenGL::glUniformMatrix4fv(renderData->glUniformCache->GetUniformPos(renderData->shaders.prog2DDrawNoTex->glID, "transformationMatrix"), 1, GL_FALSE, transformMatrix);
			}

			void ConsoleGUI::drawConsole(HorseRadish::Render::Renderer2D * const renderData, const HorseRadish::OpenGL::Tools::Viewport * const hrViewport)
			{
				if (this->consoleVisible == false)
					return;

				auto consoleAlpha = mainConsole->VarGetDataF("sys_consoleAlpha");

				HorseRadish::Matrix transformMatrix;
				transformMatrix.Set(hrViewport->getProj2D());
				transformMatrix.MultTranslate(this->consoleRect.x, this->consoleRect.y, 0.0f);

				auto glImmediateMode = renderData->glImmediateMode;

				drawConsoleBackground(renderData, transformMatrix, consoleAlpha);

				this->consoleTabPages[this->consoleSelectedTab]->DrawContent(renderData, transformMatrix);
			}

			void ConsoleGUI::drawInfo(HorseRadish::Render::Renderer2D * const renderData, const HorseRadish::OpenGL::Tools::Viewport * const hrViewport)
			{
				float lineY;
				char bufferOut[256];
				HorseRadish::String unicodeStr;

				if (this->infoVisible == false)
					return;

				auto consoleAlpha = mainConsole->VarGetDataF("sys_consoleAlpha");

				infoCheckWidth(renderData->gui.font);

				auto glImmediateMode = renderData->glImmediateMode;
				auto guiFont = renderData->gui.font;

				renderData->shaders.prog2DDrawNoTex->Bind();
				HorseRadish::OpenGL::glUniformMatrix4fv(renderData->glUniformCache->GetUniformPos(renderData->shaders.prog2DDrawNoTex->glID, "transformationMatrix"), 1, GL_FALSE, hrViewport->getProj2D());

				glImmediateMode->BeginDraw(HorseRadish::OpenGL::Tools::ImmediateMode::GeometryType::Quads);
				glImmediateMode->AddColor(0, 0, 0, HorseRadish::Color::ConvertColor(consoleAlpha));
				glImmediateMode->AddQuad(0.0f, 0.0f, this->infoMaxWidth + 10, (guiFont->getMaxHeight() + 3) * this->infoArrayVars.size() + 5);
				glImmediateMode->EndDraw();

				lineY = (guiFont->getMaxHeight() + 3) * this->infoArrayVars.size() + 5;
				HorseRadish::OpenGL::glLineWidth(2.0f);

				glImmediateMode->BeginDraw(HorseRadish::OpenGL::Tools::ImmediateMode::GeometryType::Lines);
				glImmediateMode->AddColor(128, 128, 128, 255);
				glImmediateMode->AddLineH(0.0f, this->infoMaxWidth + 10, lineY);
				glImmediateMode->AddLineV(this->infoMaxWidth + 10, lineY, 0.0f);
				glImmediateMode->EndDraw();

				HorseRadish::OpenGL::glLineWidth(1.0f);

				guiFont->paintBegin(hrViewport->getProj2D());
				guiFont->setColor(1.0f, 1.0f, 1.0f, 1.0f);

				lineY = 3.0f;
				for (const auto &curVar : this->infoArrayVars)
				{
					unicodeStr.Set(HorseRadish::String::Encoding::UTF8, mainConsole->VarPrint(curVar.GetData(), bufferOut, 256));
					guiFont->write(5.0f, lineY, unicodeStr);
					lineY += guiFont->getMaxHeight() + ConsoleGUI::infoVarSlack;
				}

				guiFont->draw();
				guiFont->paintEnd();
			}

			void ConsoleGUI::drawSMS(HorseRadish::Render::Renderer2D * const renderData, const HorseRadish::OpenGL::Tools::Viewport * const hrViewport)
			{
				int winHeight;
				float textY, textH;

				if ((this->smsNumEntries <= 0) || (this->smsVisible == false))
					return;

				auto guiFont = renderData->gui.font;

				winHeight = mainConsole->VarGetDataI("r_winHeight");
				textH = (float)guiFont->getMaxHeight();

				guiFont->paintBegin(hrViewport->getProj2D());

				textY = ((float)winHeight) - textH - ConsoleGUI::smsMessageSlack;
				for (int i = 0; i < this->smsNumEntries; i++, textY -= textH)
				{
					HorseRadish::String unicodeStr;

					unicodeStr.Set(HorseRadish::String::Encoding::UTF8, this->smsEntries[i].texto.GetData());

					guiFont->setColor(0.0f, 0.0f, 0.0f, this->smsEntries[i].alpha * 0.8f);
					guiFont->write(ConsoleGUI::smsMessageSlack + 1.0f, textY - 1.0f, unicodeStr);

					guiFont->setColor(this->smsEntries[i].cor[0], this->smsEntries[i].cor[1], this->smsEntries[i].cor[2], this->smsEntries[i].alpha);
					guiFont->write(ConsoleGUI::smsMessageSlack, textY, unicodeStr);
				}

				guiFont->draw();
				guiFont->paintEnd();

				for (int i = this->smsNumEntries - 1; i >= 0; i--)
				{
					if (this->smsEntries[i].type == ConsoleGUI::SMSFrame)
					{
						for (int j = i; j < this->smsNumEntries - 1; j++)
							memcpy(this->smsEntries + j, this->smsEntries + j + 1, sizeof(ConteudoSMS));
						this->smsNumEntries--;
						continue;
					}

					if (this->smsEntries[i].type == ConsoleGUI::SMSTime)
					{
						if ((this->smsEntries[i].fading == false) && (this->smsEntries[i].tempo.GetTimeS() >= ConsoleGUI::smsTimeVisible))
						{
							this->smsEntries[i].tempo.ReStart();
							this->smsEntries[i].fading = true;
						}

						if (this->smsEntries[i].fading == false)
							continue;

						if (this->smsEntries[i].tempo.GetTimeS() >= ConsoleGUI::smsTime2Hide)
						{
							this->smsNumEntries--;
							continue;
						}

						this->smsEntries[i].alpha = 1.0f - (this->smsEntries[i].tempo.GetTimeS() / ConsoleGUI::smsTime2Hide);
						continue;
					}

					break;
				}
			}

			ConsoleGUI::ConsoleGUI(Console *console, HorseRadish::Render::Renderer2D* const renderData, HorseRadish::OpenGL::Objects::ObjectsManager* const textureManager)
			{
				HRSRC recursoH;
				int winWidth, winHeight;

				this->mainConsole = console;
				this->renderData = renderData;

				this->texConsoleLogo = nullptr;

				this->consoleVisible = false;
				this->consoleSelectedTab = 0;

				this->infoMaxWidth = 0;
				this->infoVisible = false;

				this->smsMaxEntries = this->smsNumEntries = 0;
				this->smsEntries = nullptr;
				this->smsVisible = false;

				//logo
				/*{
					HorseRadish::Imaging::Image *imagem;

					//crio o memory stream
					HorseRadish::Streams::MemoryStream memStream(ficheiro, ficheiroSize);

					//tento ler a imagem
					imagem = HorseRadish::Imaging::Factory::ReadPNG(&HorseRadish::Streams::StreamReader(&memStream));
					if (imagem != nullptr)
					{
					//crio a textura e posso logo apagar a imagem
					texConsoleLogo = textureManager->Create2D(imagem, HRTexturesManager::RGBA32, STEXTURE_COMPRESS);
					if (texConsoleLogo != nullptr)
					textureManager->Parami(texConsoleLogo, STEXTURE_WRAP, STEXTURE_REPEAT);

					//termino com a imagem
					delete imagem;
					imagem = nullptr;
					}
					}*/

				winWidth = mainConsole->VarGetDataI("r_winWidth");
				winHeight = mainConsole->VarGetDataI("r_winHeight");

				auto guiFont = renderData->gui.font;

				this->consoleRect.width = winWidth - 60;
				this->consoleRect.height = winHeight - 60;
				this->consoleTabSpace.width = 0;
				this->consoleTabSpace.height = guiFont->getMaxHeight() + ConsoleGUI::tabContentMargin * 2;
				this->consoleRect.x = (winWidth - this->consoleRect.width) / 2;
				this->consoleRect.y = (winHeight - this->consoleRect.height) / 2;

				this->consoleLogoSize.width = this->consoleRect.height / 4;
				this->consoleLogoSize.height = this->consoleRect.height / 4;

				this->smsMaxEntries = this->consoleRect.height / guiFont->getMaxHeight();
				this->smsEntries = new ConteudoSMS[this->smsMaxEntries];
				this->smsNumEntries = 0;
			}

			ConsoleGUI::~ConsoleGUI()
			{
				for (auto& curTabPage : this->consoleTabPages)
					delete curTabPage;

				this->consoleTabPages.clear();
				this->consoleSelectedTab = 0;

				this->infoMaxWidth = 0;
				this->infoVisible = false;
				this->infoArrayVars.clear();

				delete[] this->smsEntries;
				this->smsMaxEntries = this->smsNumEntries = 0;
				this->smsVisible = false;
				this->smsEntries = nullptr;
			}

			void ConsoleGUI::Draw(const HorseRadish::OpenGL::Tools::Viewport * const hrViewport)
			{
				this->drawSMS(this->renderData, hrViewport);
				this->drawInfo(this->renderData, hrViewport);
				this->drawConsole(this->renderData, hrViewport);
			}

			bool ConsoleGUI::GUIVisivel(const TypeFunction &function) const
			{
				if (function & ConsoleGUI::FunctionConsole)
				{
					if (this->consoleVisible == true)
						return true;
				}

				if (function & ConsoleGUI::FunctionInfo)
				{
					if ((this->infoVisible == true) && (this->infoArrayVars.size() > 0))
						return true;
				}

				if (function & ConsoleGUI::FunctionSMS)
				{
					if ((this->smsVisible == true) && (this->smsNumEntries > 0))
						return true;
				}

				return false;
			}

			ConsoleGUI::ConsoleTab* ConsoleGUI::ConsoleGetSelectedTab() const
			{
				if ((this->consoleSelectedTab < 0) || (this->consoleSelectedTab >= this->consoleTabPages.size()))
					return nullptr;

				return this->consoleTabPages[this->consoleSelectedTab];
			}

			void ConsoleGUI::ConsoleAddTab(ConsoleTab * const newConsoleTab)
			{
				if (newConsoleTab == nullptr)
					return;

				this->consoleTabPages.push_back(newConsoleTab);

				newConsoleTab->SetTextAreaWidth(renderData->gui.font->getStringWidth(*newConsoleTab->GetText()) + 12.0f);
				newConsoleTab->SetContentAreaOrigin(ConsoleGUI::tabContentMargin, ConsoleGUI::tabContentMargin);
				newConsoleTab->SetContentScreenDelta(this->consoleRect.x, this->consoleRect.y);
				newConsoleTab->SetContentAreaSize(this->consoleRect.width - ConsoleGUI::tabContentMargin * 2, this->consoleRect.height - this->consoleTabSpace.height - ConsoleGUI::tabContentMargin * 2);

				this->consoleTabSpace.width += newConsoleTab->GetTextAreaWidth();
			}

			void ConsoleGUI::ConsoleProcessMSG(const MSG * const msg)
			{
				if (this->GUIVisivel(HorseRadish::Console::UI::ConsoleGUI::FunctionConsole))
				{
					auto curTab = this->ConsoleGetSelectedTab();
					if (curTab)
						curTab->ProcessMSG(msg);
				}

				if ((msg->message == WM_KEYDOWN) && (msg->wParam == 220))
				{
					this->consoleVisible = !this->consoleVisible;
					return;
				}

				if (this->consoleVisible == false)
					return;

				if ((msg->message == WM_KEYDOWN) && (msg->wParam >= VK_F1) && (msg->wParam <= VK_F24))
				{
					int novoIndiceTab;

					novoIndiceTab = msg->wParam - VK_F1;
					if ((novoIndiceTab < 0) || (novoIndiceTab >= this->consoleTabPages.size()))
						return;

					this->consoleSelectedTab = novoIndiceTab;
					return;
				}
			}

			void ConsoleGUI::ConsoleVisible(const bool visible)
			{
				this->consoleVisible = visible;
			}

			bool ConsoleGUI::ConsoleConsumesInput()
			{
				return ((this->consoleVisible == true) && (this->consoleSelectedTab == 0));
			}

			void ConsoleGUI::InfoAddVar(const char * const varName)
			{
				char bufferOut[128];

				if (varName == nullptr || varName[0] == '\0')
					return;
				if (mainConsole->VarPrint(varName, bufferOut, 128) == nullptr)
					return;

				for (auto& curVar : this->infoArrayVars)
				{
					if (curVar == varName)
						return;
				}

				this->infoArrayVars.push_back(HorseRadish::String());
				this->infoArrayVars[this->infoArrayVars.size() - 1].Set(HorseRadish::String::Encoding::UTF8, varName);
			}

			void ConsoleGUI::InfoVisible(const bool visible)
			{
				if (visible)
				{
					if (this->infoArrayVars.empty() || this->infoVisible == true)
						return;

					this->infoVisible = true;
					return;
				}

				this->infoVisible = false;
			}

			void ConsoleGUI::SMSAdd(const char * const what, const TypeSMS &type, const float &r, const float &g, const float &b)
			{
				if (this->smsVisible == false)
					return;

				if ((what == nullptr) || (what[0] == '\0'))
					return;

				for (int i = this->smsNumEntries; i > 0; i--)
					memcpy(this->smsEntries + i, this->smsEntries + i - 1, sizeof(ConteudoSMS));

				this->smsEntries[0].type = type;
				this->smsEntries[0].cor[0] = HorseRadish::Math::fClamp(r, 0.0f, 1.0f);
				this->smsEntries[0].cor[1] = HorseRadish::Math::fClamp(g, 0.0f, 1.0f);
				this->smsEntries[0].cor[2] = HorseRadish::Math::fClamp(b, 0.0f, 1.0f);
				this->smsEntries[0].alpha = 1.0f;
				this->smsEntries[0].texto.Set(HorseRadish::String::Encoding::UTF8, what);
				this->smsEntries[0].fading = false;
				this->smsEntries[0].tempo.ReStart();

				this->smsNumEntries++;
				if (this->smsNumEntries >= this->smsMaxEntries)
					this->smsNumEntries = this->smsMaxEntries - 1;
			}

			void ConsoleGUI::SMSAdd(const char * const what, const TypeSMS &type)
			{
				this->SMSAdd(what, type, 1.0f, 1.0f, 1.0f);
			}

			void ConsoleGUI::SMSVisible(const bool visible)
			{
				this->smsVisible = visible;
			}

		} //UI
	} //Console
} //HorseRadish