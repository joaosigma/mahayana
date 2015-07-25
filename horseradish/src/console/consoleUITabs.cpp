#include "common\Platform.hpp"
#include "common\Timer.hpp"
#include "common\ImageFactory.hpp"

#include "engine\logger.hpp"

#include "common\opengl\openGL.hpp"
#include "common\opengl\objects.hpp"

#include "consoleUI.hpp"
#include "render\tools\font.hpp"

namespace HorseRadish
{
	namespace Console
	{
		namespace UI
		{
			class ConsoleUserHistory
			{
				static const int maxLogSize = 256;
				static const int maxHistorySize = maxLogSize * 20;

				char *dataMain, *dataNext, *dataLast, *dataPointer, *pointerPush;
				char dataReadLocal[maxLogSize];
				unsigned int numEntradas;

			public:
				ConsoleUserHistory()
				{
					dataMain = dataNext = dataLast = dataPointer = pointerPush = nullptr;
					numEntradas = 0;

					dataMain = new char[this->maxHistorySize];
					if (dataMain == nullptr)
						return;

					memset(dataMain, 0, this->maxHistorySize);
					memset(dataReadLocal, 0, this->maxLogSize);
					dataNext = dataLast = dataMain;
				}
				~ConsoleUserHistory()
				{
					delete[] dataMain;

					dataMain = dataNext = dataLast = dataPointer = pointerPush = nullptr;
					numEntradas = 0;
				}

				bool AddPhrase(const char * const phrase)
				{
					int strLength;

					if ((dataMain == nullptr) || (phrase == nullptr) || (*phrase == '\0'))
						return false;

					strLength = strlen(phrase);
					if ((strLength + 1) >= this->maxLogSize)
						return false;

					if ((this->maxHistorySize - (dataNext - dataMain)) <= strLength)
					{
						memset(dataMain, 0, this->maxHistorySize);
						memset(dataReadLocal, 0, this->maxLogSize);
						dataNext = dataLast = dataMain;
						numEntradas = 0;
					}

					dataLast = dataNext;
					dataNext += strLength;
					memcpy(dataLast, phrase, strLength);
					*dataNext = '\0';
					dataNext++;
					dataPointer = dataNext;

					numEntradas++;
					return true;
				}
				void PointerGoUp()
				{
					if (dataMain == nullptr || dataPointer == dataMain || dataPointer == nullptr)
						return;

					dataPointer -= 2;
					for (; *dataPointer != '\0'; dataPointer--)
					{
						if (dataPointer == dataMain)
							return;
					}

					dataPointer++;
					return;
				}
				void PointerGoDown()
				{
					if (dataMain == nullptr || dataPointer == dataNext || dataPointer == nullptr)
						return;

					dataPointer++;
					while (*dataPointer != '\0')
						dataPointer++;
					dataPointer++;
					return;
				}
				const char *PointerFrase()
				{
					if (dataMain == nullptr || dataPointer == nullptr || dataPointer == dataNext)
						return nullptr;

					strcpy_s(dataReadLocal, this->maxLogSize, dataPointer);
					return dataReadLocal;
				}
			};

			class ConsoleUserPrompt
			{
				std::vector<unsigned int> unicodeStr;
				unsigned int cursorPos;
				bool changeOccured;

			public:
				ConsoleUserPrompt()
					: cursorPos(0), changeOccured(false)
				{ }

				bool getCursorState(const bool reset)
				{
					if (reset == false)
						return changeOccured;

					bool aux;

					aux = changeOccured;
					changeOccured = false;
					return aux;
				}

				int getInputCursorPos() const { return cursorPos; }

				std::string getInput() const
				{
					return HorseRadish::StringUtils::conv2UTF8(unicodeStr);
				}

				void setInput(const std::string& str)
				{
					changeOccured = true;

					unicodeStr.clear();
					for (const auto& curUnicode : HorseRadish::StringUtils::utf8Wrapper(str))
						unicodeStr.push_back(curUnicode);

					cursorPos = unicodeStr.size();
				}

				void setUserInputKEYDOWN(const Window::Message &msg)
				{
					auto virtualKey = static_cast<Window::VirtualKeys>(msg.getParam());

					if ((msg.getFlags().piecesShort.short1 & static_cast<int>(Window::Message::MessageFlags::ControlKey)) != 0)
					{
						if (virtualKey == Window::VirtualKeys::Left && cursorPos != 0)
						{
							changeOccured = true;
							for (cursorPos--; cursorPos != 0; cursorPos--)
							{
								if (unicodeStr[cursorPos - 1] == ' ' && unicodeStr[cursorPos] != ' ')
									return;
							}
							return;
						}
						if (virtualKey == Window::VirtualKeys::Right && cursorPos < unicodeStr.size())
						{
							changeOccured = true;
							for (cursorPos++; cursorPos < unicodeStr.size(); cursorPos++)
							{
								if (unicodeStr[cursorPos - 1] == ' ' && unicodeStr[cursorPos] != ' ')
									return;
							}
							return;
						}

						return;
					}

					switch (virtualKey)
					{
					case Window::VirtualKeys::Left:
						if (cursorPos == 0)
							return;
						cursorPos--;
						changeOccured = true;
						break;

					case Window::VirtualKeys::Right:
						if (cursorPos >= unicodeStr.size())
							return;
						cursorPos++;
						changeOccured = true;
						break;
					case Window::VirtualKeys::Home:
						cursorPos = 0;
						changeOccured = true;
						break;
					case Window::VirtualKeys::End:
						cursorPos = unicodeStr.size();
						changeOccured = true;
						break;
					case Window::VirtualKeys::Delete:
						if (cursorPos >= unicodeStr.size())
							return;

						unicodeStr.erase(unicodeStr.begin() + cursorPos);
						changeOccured = true;
						break;
					}
				}

				void  setUserInputCHAR(const Window::Message &msg)
				{
					auto keyVal = msg.getParam();

					if (keyVal == '#' || keyVal == 9 || keyVal == '\\' || keyVal == 27)
						return;

					if (keyVal == 22)
					{
						std::string strUTF8;

						if (HorseRadish::Platform::ClipboardGetStrings([&](const std::string& curString) -> bool
						{
							strUTF8 = curString;
							return false;
						}) == false)
						{
							HorseRadish::Platform::ClipboardGetFiles([&](const std::string& curString) -> bool
							{
								strUTF8 = curString;
								return false;
							});
						}

						for (const auto& curUnicode : HorseRadish::StringUtils::utf8Wrapper(strUTF8))
						{
							unicodeStr.insert(unicodeStr.begin() + cursorPos, curUnicode);
							cursorPos++;
						}

						changeOccured = true;
						return;
					}

					if (keyVal == 8)
					{
						if (unicodeStr.empty())
							return;

						if (cursorPos == 0)
							return;

						changeOccured = true;
						cursorPos--;

						unicodeStr.erase(unicodeStr.begin() + cursorPos);
						return;
					}

					unicodeStr.insert(unicodeStr.begin() + cursorPos, keyVal);
					cursorPos++;
					changeOccured = true;
				}
			};

			ConsoleTabConsole::ConsoleTabConsole(std::function<void(const char * const)> inputCb, std::shared_ptr<Engine::Logger> logger) : ConsoleGUI::ConsoleTab()
			{
				this->mInputCb = inputCb;
				this->mLogger = logger;

				this->userHistory = new ConsoleUserHistory();
				this->userPrompt = new ConsoleUserPrompt();

				this->cursorVisivel = false;
				this->numMaxLinhasTexto = 0;
				this->textoOffset = 0;
				this->listaTexto = nullptr;

				this->cursorTimer.reStart();
			}

			ConsoleTabConsole::~ConsoleTabConsole()
			{
				delete[] this->listaTexto;
				delete this->userHistory;
				delete this->userPrompt;

				this->numMaxLinhasTexto = 0;
				this->textoOffset = 0;
				this->listaTexto = nullptr;
				this->userHistory = nullptr;
				this->userPrompt = nullptr;
			}

			void ConsoleTabConsole::DrawContent(HorseRadish::Render::Renderer2D* const render2D, const HorseRadish::Matrix &transformMatrix)
			{
				int contentAreaLimits[2];
				float userTextY, textH, textY, textX;

				contentAreaLimits[0] = this->tabContentAreaPos.x + this->tabContentAreaSize.width;
				contentAreaLimits[1] = this->tabContentAreaPos.y + this->tabContentAreaSize.height;

				HorseRadish::OpenGL::glUseProgram(0);
				HorseRadish::OpenGL::glBindProgramPipeline(render2D->shaders.drawNoTex.progFragment.getId());
				HorseRadish::OpenGL::glProgramUniformMatrix4fv(render2D->shaders.drawNoTex.progVertex.getId(), render2D->shaders.drawNoTex.progVertex.getUniformLocation("transformationMatrix"), 1, false, transformMatrix.data());

				auto& glImmediateMode = render2D->glImmediateMode;
				auto& guiFont = render2D->gui.font;

				glImmediateMode->beginDraw(HorseRadish::OpenGL::Tools::ImmediateMode::GeometryType::Quads);
					glImmediateMode->setColor(128, 128, 128, 255);
					glImmediateMode->addQuad(this->tabContentAreaPos.x, this->tabContentAreaPos.y, 10.0f, 10 + guiFont->getMaxHeight());
				glImmediateMode->endDraw();

				HorseRadish::OpenGL::glLineWidth(2.0f);
				HorseRadish::OpenGL::glEnable(GL_LINE_SMOOTH);

				glImmediateMode->beginDraw(HorseRadish::OpenGL::Tools::ImmediateMode::GeometryType::Lines);
					glImmediateMode->setColor(128, 128, 128, 255);
					//glImmediateMode->addLineH(this->tabContentAreaPos.x, contentAreaLimits[0], this->tabContentAreaPos.y);
					glImmediateMode->addLineH(this->tabContentAreaPos.x, contentAreaLimits[0], this->tabContentAreaPos.y + 10 + guiFont->getMaxHeight());
				glImmediateMode->endDraw();

				HorseRadish::OpenGL::glLineWidth(1.0f);
				HorseRadish::OpenGL::glDisable(GL_LINE_SMOOTH);

				guiFont->paintBegin(transformMatrix.data());
				guiFont->setColor(1.0f, 1.0f, 1.0f);

				userTextY = this->tabContentAreaPos.y + 5.0f;

				textH = (float)guiFont->getMaxHeight();
				textX = this->tabContentAreaPos.x + 20.0f;
				textY = floorf(userTextY + textH);

				guiFont->setColor(1.0f, 1.0f, 1.0f, 1.0f);
				for (int indexLinha = 0; indexLinha < numMaxLinhasTexto; indexLinha++)
				{
					auto curLinha = this->listaTexto + indexLinha;
					if (curLinha->active == false)
						break;

					textY += textH;
					if (textY >= contentAreaLimits[1])
						break;

					if (curLinha->texto.empty())
						continue;

					if (!curLinha->formatted)
					{
						if (curLinha->type == Engine::Logger::EntryType::Error)
							guiFont->setColor(1.0f, 0.0f, 0.0f, 1.0f);
						else if (curLinha->type == Engine::Logger::EntryType::Warning)
							guiFont->setColor(1.0f, 0.42f, 0.17f, 1.0f);
						else
							guiFont->setColor(1.0f, 1.0f, 1.0f, 1.0f);
						guiFont->write(textX, textY, curLinha->texto);
						continue;
					}

					auto curX = textX;

					unsigned int lastChar = 0;
					HorseRadish::StringUtils::utf8Wrapper textWrapper(curLinha->texto);
					for (HorseRadish::StringUtils::utf8Wrapper::const_iterator it = textWrapper.begin(), itEnd = textWrapper.end(); it != itEnd; ++it)
					{
						auto curChar = *it;

						HorseRadish::StringUtils::utf8Wrapper::const_iterator itNext(it);
						itNext++;
						
						if ((curChar == '$') && (lastChar != '$'))
						{
							HorseRadish::StringUtils::utf8Wrapper::const_iterator itNext(it);
							itNext++;

							if ((*itNext == '{') )
							{
								it++;
								it++;

								std::string value;
								value.reserve(10);

								for (; (*it != '}') && (it != itEnd); it++)
									value += *it;

								HorseRadish::StringUtils::trim(value);
								if (value.empty())
									continue;

								if (value[0] == '#')
									guiFont->setColor(Color::parseColorFromHTML(value.c_str()));
								else if (value == "red")
									guiFont->setColor(1.0f, 0.0f, 0.0f, 1.0f);
								else if (value == "green")
									guiFont->setColor(0.0f, 1.0f, 0.0f, 1.0f);
								else if (value == "bgreen")
									guiFont->setColor(0.08f, 1.0f, 0.39f, 1.0f);
								else if (value == "olive")
									guiFont->setColor(0.59f, 0.59f, 0.0f, 1.0f);
								else if (value == "default")
									guiFont->setColor(1.0f, 1.0f, 1.0f, 1.0f);

								continue;
							}
						}

						lastChar = *it;
						curX += guiFont->writeChar(curX, textY, lastChar);
					}

					guiFont->setColor(1.0f, 1.0f, 1.0f, 1.0f);
				}

				std::string unicodeStr;

				if (userPrompt->getCursorState(true))
				{
					cursorVisivel = true;
					cursorTimer.reStart();
				}

				unicodeStr = userPrompt->getInput();
				guiFont->write(this->tabContentAreaPos.x + 15.0f, userTextY, unicodeStr);
				if (cursorVisivel)
				{
					auto cursorPos = userPrompt->getInputCursorPos();
					auto strLength = guiFont->getStringWidth(unicodeStr, cursorPos);

					auto cursorChar = HorseRadish::StringUtils::getUnicodeAt(unicodeStr, cursorPos);

					guiFont->writeChar(this->tabContentAreaPos.x + 15.0f + strLength, userTextY, '_');
					if (cursorChar != '\0' && cursorChar != ' ')
					{
						guiFont->setColor(1.0f, 0.0f, 0.0f);
						guiFont->writeChar(this->tabContentAreaPos.x + 15.0f + strLength, userTextY, cursorChar);
						guiFont->setColor(1.0f, 1.0f, 1.0f);
					}
				}

				guiFont->paintEnd();

				if (cursorTimer.getTimeMS() > 750.0f)
				{
					cursorVisivel = !cursorVisivel;
					cursorTimer.reStart();
				}
			}

			void ConsoleTabConsole::ProcessMSG(const Window::Message &msg)
			{
				if (msg.getType() == Window::Message::MessageType::MouseWheel)
				{
					this->textoOffset += static_cast<signed short>(msg.getFlags().piecesShort.short0);
					if (this->textoOffset < 0)
						this->textoOffset = 0;

					this->ActualizaTextoConsola();
					return;
				}

				if ((msg.getType() != Window::Message::MessageType::CharacterKey) && (msg.getType() != Window::Message::MessageType::VirtualKey))
					return;

				if (msg.getType() == Window::Message::MessageType::VirtualKey)
				{
					if (msg.getParam() == static_cast<int>(Window::VirtualKeys::PageUp))
					{
						this->textoOffset++;
						this->ActualizaTextoConsola();
						return;
					}
					if (msg.getParam() == static_cast<int>(Window::VirtualKeys::PageDown))
					{
						this->textoOffset -= (this->textoOffset > 0) ? 1 : 0;
						this->ActualizaTextoConsola();
						return;
					}

					if (msg.getParam() == static_cast<int>(Window::VirtualKeys::Home))
					{
						this->textoOffset = 0;
						this->ActualizaTextoConsola();
						return;
					}
					if (msg.getParam() == static_cast<int>(Window::VirtualKeys::End))
					{
						this->textoOffset = 0;
						this->ActualizaTextoConsola();
						return;
					}
					if (msg.getParam() == static_cast<int>(Window::VirtualKeys::Up))
					{
						userHistory->PointerGoUp();
						userPrompt->setInput(userHistory->PointerFrase());
						return;
					}
					if (msg.getParam() == static_cast<int>(Window::VirtualKeys::Down))
					{
						userHistory->PointerGoDown();
						userPrompt->setInput(userHistory->PointerFrase());
						return;
					}

					if (msg.getParam() == static_cast<int>(Window::VirtualKeys::Tab))
					{
						/*const char *string;
						char dest[256];
						bool primeiroHit;

						string = userPrompt->getInput();

						if (string[0] == '\0')
							return;

						while ((string[0] == ' ') && (string[0] != '\0'))
							string++;

						if (string[0] == '\0')
						{
							userPrompt->setInput(nullptr);
							return;
						}

						primeiroHit = true;
						dest[0] = '\0';
						mainConsole->TABComplete(string, dest, sizeof(dest), [&](const HorseRadish::String &hit)
						{
							if (primeiroHit == true)
								mainConsole->LogInfo(HorseRadish::String(">%s", dest).GetData());
							primeiroHit = false;
							mainConsole->LogTab(hit.GetData(), 2);
						});

						if (dest[0] != '\0')
							userPrompt->setInput(dest);*/
						return;
					}
				}

				if (msg.getType() == Window::Message::MessageType::CharacterKey && msg.getParam() == 13)
				{
					auto inputStr = userPrompt->getInput();
					if (inputStr.empty())
						return;

					userHistory->AddPhrase(inputStr.c_str());

					if (mInputCb)
						mInputCb(inputStr.c_str());

					userPrompt->setInput("");
					return;
				}

				if (msg.getType() == Window::Message::MessageType::VirtualKey)
					userPrompt->setUserInputKEYDOWN(msg);
				else if (msg.getType() == Window::Message::MessageType::CharacterKey)
					userPrompt->setUserInputCHAR(msg);
			}

			bool ConsoleTabConsole::CriaTextoConsola(HorseRadish::Render::Renderer2D* const renderData)
			{
				int posY, meta, textH;

				textH = renderData->gui.font->getMaxHeight();

				posY = this->tabContentAreaPos.y + textH + 5;
				meta = this->tabContentAreaSize.height - textH - 5;
				this->numMaxLinhasTexto = 0;
				for (; posY <= meta; posY += textH)
					this->numMaxLinhasTexto++;

				this->listaTexto = new LINHA_TEXTO[this->numMaxLinhasTexto];

				for (int i = 0; i < this->numMaxLinhasTexto; i++)
					this->listaTexto[i].active = false;

				return true;
			}

			void ConsoleTabConsole::ActualizaTextoConsola()
			{
				int curLine = 0;

				mLogger->iterateBuffer([&](const Engine::Logger::EntryType entryType, const Engine::Logger::ModuleType moduleType, const bool isFormatted, const std::string& msg) -> bool
				{
					if (curLine >= this->numMaxLinhasTexto)
						return false;
					
					auto curLinha = this->listaTexto + curLine;
					curLine++;
					
					curLinha->active = true;
					curLinha->type = entryType;
					curLinha->texto = msg;
					curLinha->formatted = isFormatted;
					return true;

				}, this->textoOffset);

				for (; curLine < this->numMaxLinhasTexto; curLine++)
					this->listaTexto[curLine].active = false;
			}

		} //UI
	} //Console
} //HorseRadish