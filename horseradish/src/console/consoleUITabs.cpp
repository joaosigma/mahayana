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
				HorseRadish::String localData;
				unsigned int cursorPos;
				bool changeOccured;

			public:
				ConsoleUserPrompt()
				{
					localData.SetEmpty();
					cursorPos = 0;
					changeOccured = true;
				}
				~ConsoleUserPrompt(){ }

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
				const char *getInput() const { return localData.GetData(); }

				void setInput(const char *novaString)
				{
					if (novaString == nullptr)
						return;

					changeOccured = true;

					if (*novaString == '\0')
					{
						cursorPos = 0;
						localData.SetEmpty();
						return;
					}

					localData.Set(HorseRadish::String::Encoding::UTF8, novaString);
					cursorPos = localData.GetSizeChars();
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
								if (localData.GetUnicodeAt(cursorPos - 1) == ' ' && localData.GetUnicodeAt(cursorPos) != ' ')
									return;
							}
							return;
						}
						if (virtualKey == Window::VirtualKeys::Right && cursorPos < localData.GetSizeChars())
						{
							changeOccured = true;
							for (cursorPos++; cursorPos < localData.GetSizeChars(); cursorPos++)
							{
								if (localData.GetUnicodeAt(cursorPos - 1) == ' ' && localData.GetUnicodeAt(cursorPos) != ' ')
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
						if (cursorPos >= localData.GetSizeChars())
							return;
						cursorPos++;
						changeOccured = true;
						break;
					case Window::VirtualKeys::Home:
						cursorPos = 0;
						changeOccured = true;
						break;
					case Window::VirtualKeys::End:
						cursorPos = localData.GetSizeChars();
						changeOccured = true;
						break;
					case Window::VirtualKeys::Delete:
						if (cursorPos >= localData.GetSizeChars())
							return;
						localData.RemoveAt(cursorPos);
						changeOccured = true;
						break;
					}
				}

				void  setUserInputCHAR(const Window::Message &msg)
				{
					auto keyVal = msg.getParam();

					if (keyVal == '#' || keyVal == 9 || keyVal == '\\' || keyVal == 27)
						return;

					if (keyVal == 127)
					{
						const char *delStart, *delEnd;
						unsigned int startCountIndex;

						if (localData.GetSizeChars() == 0 || cursorPos == 0)
							return;

						delEnd = localData.GetData() + localData.GetCharByteIndex(cursorPos - 1);
						delStart = delEnd - 1;
						while ((delStart > localData.GetData()) && (*delStart == ' '))
							delStart--;
						while ((delStart > localData.GetData()) && (*delStart != ' '))
							delStart--;
						while ((delStart > localData.GetData()) && (*delStart == ' '))
							delStart--;

						if (delStart != localData.GetData())
							delStart += 2;

						startCountIndex = localData.GetCharPosIndex(delStart - localData.GetData());

						if (cursorPos >= localData.GetSizeChars())
						{
							localData.CloseAt(startCountIndex);
							cursorPos = localData.GetSizeChars();
							changeOccured = true;
							return;
						}

						localData.RemoveAt(startCountIndex, cursorPos - startCountIndex);
						cursorPos = localData.GetCharPosIndex(startCountIndex);

						changeOccured = true;
						return;
					}

					if (keyVal == 22)
					{
						HorseRadish::String strUTF8;

						if (HorseRadish::Platform::ClipboardGetStrings([&](const HorseRadish::String &curString) -> bool
						{
							strUTF8.Set(curString);
							return false;
						}) == false)
						{
							HorseRadish::Platform::ClipboardGetFiles([&](const HorseRadish::String &curString) -> bool
							{
								strUTF8.Set(curString);
								return false;
							});
						}

						if (cursorPos >= localData.GetSizeChars())
							localData += strUTF8;
						else
							localData.AppendAtPos(strUTF8, cursorPos);
						cursorPos += strUTF8.GetSizeChars();
						changeOccured = true;
						return;
					}

					if (keyVal == 8)
					{
						if (localData.GetSizeChars() == 0)
							return;

						if (cursorPos == 0)
							return;

						changeOccured = true;
						cursorPos--;

						if ((cursorPos + 1) >= localData.GetSizeChars())
						{
							localData.Remove(1, HorseRadish::String::Position::End);
							return;
						}

						localData.RemoveAt(cursorPos);
						return;
					}

					localData.AppendAtPos(keyVal, cursorPos);
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

				this->cursorTimer.ReStart();
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
				HorseRadish::OpenGL::glProgramUniformMatrix4fv(render2D->shaders.drawNoTex.progVertex.getId(), render2D->shaders.drawNoTex.progVertex.getUniformLocation("transformationMatrix"), 1, false, transformMatrix);

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

				guiFont->paintBegin(transformMatrix);
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

					if (curLinha->texto.IsEmpty(true) == true)
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
					HorseRadish::String::Iterator it(curLinha->texto);
					for (; *it != '\0'; it++)
					{
						auto walker = curLinha->texto.GetData() + it.GetBytePosition();
						if ((walker[0] == '$') && (walker[1] == '{'))
						{
							if ((walker == curLinha->texto.GetData()) || (walker[-1] != '$'))
							{
								walker += 2;
								auto walkerEnd = walker;
								for (; *walkerEnd != '}'; walkerEnd++);

								it += (walkerEnd - walker) + 2;

								if ((*walker == '#') && ((walkerEnd - walker) == 7))
									guiFont->setColor(Color::ParseColorFromHTML(walker));
								else if (strncmp(walker, "red", walkerEnd - walker) == 0)
									guiFont->setColor(1.0f, 0.0f, 0.0f, 1.0f);
								else if (strncmp(walker, "green", walkerEnd - walker) == 0)
									guiFont->setColor(0.0f, 1.0f, 0.0f, 1.0f);
								else if (strncmp(walker, "bgreen", walkerEnd - walker) == 0)
									guiFont->setColor(0.08f, 1.0f, 0.39f, 1.0f);
								else if (strncmp(walker, "olive", walkerEnd - walker) == 0)
									guiFont->setColor(0.59f, 0.59f, 0.0f, 1.0f);
								else if (strncmp(walker, "default", walkerEnd - walker) == 0)
									guiFont->setColor(1.0f, 1.0f, 1.0f, 1.0f);

								continue;
							}
						}

						curX += guiFont->writeChar(curX, textY, *it);
					}

					guiFont->write(curX, textY, it);
					guiFont->setColor(1.0f, 1.0f, 1.0f, 1.0f);
				}

				HorseRadish::String unicodeStr;

				if (userPrompt->getCursorState(true))
				{
					cursorVisivel = true;
					cursorTimer.ReStart();
				}

				unicodeStr.Set(HorseRadish::String::Encoding::UTF8, userPrompt->getInput());
				guiFont->write(this->tabContentAreaPos.x + 15.0f, userTextY, unicodeStr);
				if (cursorVisivel == true)
				{
					int cursorPos, strLength;
					unsigned int cursorChar;

					cursorPos = userPrompt->getInputCursorPos();
					strLength = guiFont->getStringWidth(unicodeStr, cursorPos);

					cursorChar = unicodeStr.GetUnicodeAt(cursorPos);

					guiFont->writeChar(this->tabContentAreaPos.x + 15.0f + strLength, userTextY, '_');
					if (cursorChar != '\0' && cursorChar != ' ')
					{
						guiFont->setColor(1.0f, 0.0f, 0.0f);
						guiFont->writeChar(this->tabContentAreaPos.x + 15.0f + strLength, userTextY, cursorChar);
						guiFont->setColor(1.0f, 1.0f, 1.0f);
					}
				}

				guiFont->paintEnd();

				if (cursorTimer.GetTimeMS() > 750.0f)
				{
					cursorVisivel = !cursorVisivel;
					cursorTimer.ReStart();
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
						const char *string;
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
						/*mainConsole->TABComplete(string, dest, sizeof(dest), [&](const HorseRadish::String &hit)
						{
							if (primeiroHit == true)
								mainConsole->LogInfo(HorseRadish::String(">%s", dest).GetData());
							primeiroHit = false;
							mainConsole->LogTab(hit.GetData(), 2);
						});*/

						if (dest[0] != '\0')
							userPrompt->setInput(dest);
						return;
					}
				}

				if (msg.getType() == Window::Message::MessageType::CharacterKey && msg.getParam() == 13)
				{
					const char *texto;

					texto = userPrompt->getInput();
					if (*texto == '\0')
						return;

					userHistory->AddPhrase(texto);

					if (mInputCb)
						mInputCb(texto);

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

				mLogger->IterateLast([&](const Engine::Logger::EntryType entryType, const Engine::Logger::ModuleType moduleType, const bool isFormatted, const char * const log) -> bool
				{
					if (curLine >= this->numMaxLinhasTexto)
						return false;
					
					auto curLinha = this->listaTexto + curLine;
					curLine++;
					
					curLinha->active = true;
					curLinha->type = entryType;
					curLinha->texto.Set(HorseRadish::String::Encoding::UTF8, log);
					curLinha->formatted = isFormatted;
					return true;

				}, this->textoOffset);

				for (; curLine < this->numMaxLinhasTexto; curLine++)
					this->listaTexto[curLine].active = false;
			}

		} //UI
	} //Console
} //HorseRadish