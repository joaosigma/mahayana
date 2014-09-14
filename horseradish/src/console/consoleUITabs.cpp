#include "common\Platform.hpp"
#include "common\Timer.hpp"
#include "common\ImageFactory.hpp"
#include "common\Logger.hpp"
#include "common\opengl\openGL.hpp"
#include "common\opengl\objects.hpp"
#include "common\openGL\tools.hpp"

#include "consoleUI.hpp"

namespace HorseRadish
{
	namespace Console
	{
		namespace UI
		{

			static
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

			static
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
				~ConsoleUserPrompt(){ return; };

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
				void  setUserInputKEYDOWN(const unsigned int keyVal)
				{
					if (HIBYTE((WORD)GetKeyState(VK_RCONTROL)))
					{
						if (keyVal == VK_LEFT && cursorPos != 0)
						{
							changeOccured = true;
							for (cursorPos--; cursorPos != 0; cursorPos--)
							{
								if (localData.GetUnicodeAt(cursorPos - 1) == ' ' && localData.GetUnicodeAt(cursorPos) != ' ')
									return;
							}
							return;
						}
						if (keyVal == VK_RIGHT && cursorPos < localData.GetSizeChars())
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

					switch (keyVal){
					case VK_LEFT:
						if (cursorPos == 0)
							return;
						cursorPos--;
						changeOccured = true;
						break;

					case VK_RIGHT:
						if (cursorPos >= localData.GetSizeChars())
							return;
						cursorPos++;
						changeOccured = true;
						break;
					case VK_HOME:
						cursorPos = 0;
						changeOccured = true;
						break;
					case VK_END:
						cursorPos = localData.GetSizeChars();
						changeOccured = true;
						break;

					case VK_DELETE:
						if (cursorPos >= localData.GetSizeChars())
							return;
						localData.RemoveAt(cursorPos);
						changeOccured = true;
						break;
					}
				}
				void  setUserInputCHAR(const unsigned int keyVal)
				{
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

			ConsoleTabConsole::ConsoleTabConsole(Console *mainConsole) : ConsoleGUI::ConsoleTab()
			{
				this->mainConsole = mainConsole;

				this->tabName.Set(HorseRadish::String::Encoding::ASCII, "Console");

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
				unsigned int charActual;
				int contentAreaLimits[2], numCores, corActual;
				float curX, userTextY, textH, textY, textX;
				HorseRadish::OpenGL::Tools::ImmediateMode *glImmediateMode;
				HorseRadish::OpenGL::Tools::Font *guiFont;

#pragma pack (push)
#pragma pack (1)
				struct DataRGB{
					unsigned int pos;
					unsigned char r, g, b;
				}*dataCores;
#pragma pack (pop)

				contentAreaLimits[0] = this->tabContentAreaPos.x + this->tabContentAreaSize.width;
				contentAreaLimits[1] = this->tabContentAreaPos.y + this->tabContentAreaSize.height;

				render2D->shaders.prog2DDrawNoTex->Bind();
				HorseRadish::OpenGL::glUniformMatrix4fv(render2D->glUniformCache->GetUniformPos(render2D->shaders.prog2DDrawNoTex->glID, "transformationMatrix"), 1, GL_FALSE, transformMatrix);

				glImmediateMode = render2D->glImmediateMode;
				guiFont = render2D->gui.fontConsole;

				glImmediateMode->BeginDraw(HorseRadish::OpenGL::Tools::ImmediateMode::GeometryType::Quads);
				glImmediateMode->AddColor(128, 128, 128, 255);
				glImmediateMode->AddQuad(this->tabContentAreaPos.x, this->tabContentAreaPos.y, 10.0f, 10 + guiFont->getMaxHeight());
				glImmediateMode->EndDraw();

				HorseRadish::OpenGL::glLineWidth(2.0f);
				HorseRadish::OpenGL::glEnable(GL_LINE_SMOOTH);

				glImmediateMode->BeginDraw(HorseRadish::OpenGL::Tools::ImmediateMode::GeometryType::Lines);
				glImmediateMode->AddColor(128, 128, 128, 255);
				//glImmediateMode->AddLineH(this->tabContentAreaPos.x, contentAreaLimits[0], this->tabContentAreaPos.y);
				glImmediateMode->AddLineH(this->tabContentAreaPos.x, contentAreaLimits[0], this->tabContentAreaPos.y + 10 + guiFont->getMaxHeight());
				glImmediateMode->EndDraw();

				HorseRadish::OpenGL::glLineWidth(1.0f);
				HorseRadish::OpenGL::glDisable(GL_LINE_SMOOTH);

				guiFont->paintBegin(transformMatrix);
				guiFont->setColor(1.0f, 1.0f, 1.0f);

				userTextY = this->tabContentAreaPos.y + 5.0f;

				textH = (float)guiFont->getMaxHeight();
				textX = this->tabContentAreaPos.x + 20.0f;
				textY = floorf(userTextY + textH);

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

					numCores = 0;
					if (curLinha->metadata != nullptr)
					{
						numCores = *((int*)curLinha->metadata);
						dataCores = (DataRGB*)(((const char*)curLinha->metadata) + sizeof(int));
					}

					if (numCores <= 0)
					{
						guiFont->write(textX, textY, curLinha->texto);
						continue;
					}

					if ((numCores == 1) && (dataCores[0].pos == 0))
					{
						guiFont->setColor(HorseRadish::Color::ConvertColor(dataCores[0].r), HorseRadish::Color::ConvertColor(dataCores[0].g), HorseRadish::Color::ConvertColor(dataCores[0].b), 1.0f);
						guiFont->write(textX, textY, curLinha->texto);
						guiFont->setColor(1.0f, 1.0f, 1.0f, 1.0f);
						continue;
					}

					curX = textX;
					corActual = 0;

					HorseRadish::String::Iterator it(curLinha->texto);
					for (; (charActual = *it) != '\0'; it++)
					{
						if (it.GetBytePosition() == dataCores[corActual].pos)
						{
							guiFont->setColor(HorseRadish::Color::ConvertColor(dataCores[corActual].r), HorseRadish::Color::ConvertColor(dataCores[corActual].g), HorseRadish::Color::ConvertColor(dataCores[corActual].b), 1.0f);

							corActual++;
							if (corActual >= numCores)
								break;
						}

						curX += guiFont->writeChar(curX, textY, charActual);
					}

					guiFont->write(curX, textY, it);
					guiFont->setColor(1.0f, 1.0f, 1.0f, 1.0f);
				}

				HorseRadish::String unicodeStr;

				unicodeStr.Set(HorseRadish::String::Encoding::UTF8, mainConsole->VarGetDataS("sys_Version"));
				guiFont->setColor(1.0f, 1.0f, 1.0f, 0.7f);
				guiFont->write(contentAreaLimits[0] - guiFont->getStringWidth(unicodeStr) - 5.0f, userTextY + textH + 4.0f, unicodeStr);
				guiFont->setColor(1.0f, 1.0f, 1.0f, 1.0f);

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

				guiFont->draw();
				guiFont->paintEnd();

				if (cursorTimer.GetTimeMS() > 750.0f)
				{
					cursorVisivel = !cursorVisivel;
					cursorTimer.ReStart();
				}
			}

			void ConsoleTabConsole::ProcessMSG(const MSG * const msg)
			{
				if (msg->message == WM_MOUSEWHEEL)
				{
					if ((int)msg->wParam > 0)
						this->textoOffset = HorseRadish::Math::iMin(this->textoOffset + 1, this->mainConsole->logger->GetNumberCurrentEntries() - 5);
					else
						this->textoOffset = HorseRadish::Math::iClampZero(this->textoOffset - 1);

					this->ActualizaTextoConsola();
					return;
				}

				if ((msg->message != WM_CHAR) && (msg->message != WM_KEYDOWN))
					return;

				if (msg->message == WM_KEYDOWN)
				{
					if (msg->wParam == 33)
					{
						this->textoOffset = HorseRadish::Math::iMin(this->textoOffset + 1, this->mainConsole->logger->GetNumberCurrentEntries() - 5);
						this->ActualizaTextoConsola();
						return;
					}
					if (msg->wParam == 34)
					{
						this->textoOffset = HorseRadish::Math::iClampZero(this->textoOffset - 1);
						this->ActualizaTextoConsola();
						return;
					}

					if (msg->wParam == VK_HOME && HIBYTE(GetKeyState(VK_RCONTROL)))
					{
						this->textoOffset = mainConsole->logger->GetNumberCurrentEntries() - 5;
						this->ActualizaTextoConsola();
						return;
					}
					if (msg->wParam == VK_END && HIBYTE(GetKeyState(VK_RCONTROL)))
					{
						this->textoOffset = 0;
						this->ActualizaTextoConsola();
						return;
					}
					if (msg->wParam == VK_UP)
					{
						userHistory->PointerGoUp();
						userPrompt->setInput(userHistory->PointerFrase());
						return;
					}
					if (msg->wParam == VK_DOWN)
					{
						userHistory->PointerGoDown();
						userPrompt->setInput(userHistory->PointerFrase());
						return;
					}

					if (msg->wParam == VK_TAB)
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
						mainConsole->TABComplete(string, dest, sizeof(dest), [&](const HorseRadish::String &hit)
						{
							if (primeiroHit == true)
								mainConsole->LogInfo(HorseRadish::String(">%s", dest).GetData());
							primeiroHit = false;
							mainConsole->LogTab(hit.GetData(), 2);
						});

						if (dest[0] != '\0')
							userPrompt->setInput(dest);
						return;
					}
				}

				if (msg->message == WM_CHAR && msg->wParam == 13)
				{
					const char *texto;

					texto = userPrompt->getInput();
					if (*texto == '\0')
						return;

					userHistory->AddPhrase(texto);

					mainConsole->Process(texto);

					userPrompt->setInput("");
					return;
				}

				if (msg->message == WM_KEYDOWN)
					userPrompt->setUserInputKEYDOWN(msg->wParam);
				else if (msg->message == WM_CHAR)
					userPrompt->setUserInputCHAR(msg->wParam);
			}

			bool ConsoleTabConsole::CriaTextoConsola(HorseRadish::Render::Renderer2D* const renderData)
			{
				int posY, meta, textH;

				textH = renderData->gui.fontConsole->getMaxHeight();

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
				for (int i = 0; i < this->numMaxLinhasTexto; i++)
				{
					this->listaTexto[i].active = false;
					this->listaTexto[i].metadata = nullptr;
				}

				std::function<bool(const void * const metadata, const int metadataSize, const char * const data, const int dataSize)> iterator = [&](const void * const metadata, const int metadataSize, const char * const data, const int dataSize) -> bool
				{
					for (int indexLinha = 0; indexLinha < this->numMaxLinhasTexto; indexLinha++)
					{
						auto curLinha = this->listaTexto + indexLinha;
						if (curLinha->active == true)
							continue;

						curLinha->active = true;
						curLinha->texto.Set(HorseRadish::String::Encoding::UTF8, data);
						curLinha->metadata = (metadataSize <= 0) ? nullptr : metadata;
						return true;
					}

					return false;
				};

				mainConsole->logger->Iterate(true, iterator, this->textoOffset);
			}


			ConsoleTabStats::ConsoleTabStats() : ConsoleGUI::ConsoleTab()
			{
				tabName.Set(HorseRadish::String::Encoding::ASCII, "Stats");
			}

			ConsoleTabStats::~ConsoleTabStats()
			{
			}

			void ConsoleTabStats::DrawContent(HorseRadish::Render::Renderer2D* const render2D, const HorseRadish::Matrix &transformMatrix)
			{
				/*SGPUCounter *gpuCounter;
				int inboxX,inboxY,inboxWidth,inboxHeight;
				int numMaxSamples,maxLegendWidth,textH;
				float convX, posLegendaY;
				HorseRadish::OpenGL::Tools::ImmediateMode *glImmediateMode;
				HorseRadish::OpenGL::Tools::Font *guiFont;

				//se não nada de contadores
				if ((renderData->stats.gpuCounter==nullptr) || (renderData->stats.gpuCounter->getNumCounters()<=0))
				return;

				//para facilitar a vida
				gpuCounter = renderData->stats.gpuCounter;

				//o programa em causa e todos os uniforms que preciso
				renderData->shaders.prog2DDrawNoTex->Bind();
				HorseRadish::OpenGL::glUniformMatrix4fv(renderData->glUniformCache->GetUniformPos(renderData->shaders.prog2DDrawNoTex->glID, "transformationMatrix"),1,GL_FALSE, transformMatrix);

				//para ajudar
				glImmediateMode = renderData->glImmediateMode;
				guiFont = renderData->gui.font;

				//a altura máxima do texto
				textH = guiFont->getMaxHeight();

				//preciso de calcular o tamanho máximo do nome dos counters
				maxLegendWidth = 0;
				for(int i=1; i<=gpuCounter->getNumCounters(); i++)
				{
				HorseRadish::String unicodeStr;

				//tiro a string e vejo o tamanho máximo
				unicodeStr.Set(HorseRadish::String::Encoding::UTF8, gpuCounter->getCounterName(i));
				maxLegendWidth = HorseRadish::Math::iMax(maxLegendWidth, guiFont->getStringWidth(unicodeStr));
				}

				//calculo isto pra a área de gráfico
				inboxX = this->tabContentAreaPos.x + 15 + maxLegendWidth;
				inboxY = this->tabContentAreaPos.y + 5;
				inboxWidth = this->tabContentAreaSize.width - 20 - maxLegendWidth;
				inboxHeight = this->tabContentAreaSize.height - 5*2;

				//posso já saber o máximo de samples a usar e como transformar pra pixel
				numMaxSamples=gpuCounter->getMaxNumSamples();
				convX=((float)inboxWidth)/((float)numMaxSamples);

				//toca a passar por todos os counters e desenho-os
				for(int i=0; i<gpuCounter->getNumCounters(); i++)
				{
				int curX,curY;
				float valueYScale, valueYTrans;

				//se o contador for em percentagem
				if (gpuCounter->getCounterIsPercent(i+1)==true)
				{
				//como os valores estão em 0 e 100, basta ajustar a escala até cima e não é preciso translação
				valueYScale = ((float)inboxHeight)*0.01f;
				valueYTrans = 0.0f;
				}
				else
				{
				float valorMin, valorMax;

				gpuCounter->getValuesMinMax(i+1, &valorMin, &valorMax);

				valueYScale = ((float)inboxHeight) / (valorMax-valorMin);
				valueYTrans = -valorMin;
				}

				//começo a desenhar a linestrip deste contador
				glImmediateMode->BeginDraw(HorseRadish::OpenGL::Tools::ImmediateMode::LineStrip);
				//a cor do contador
				glImmediateMode->AddColorRGB(gpuCounter->getCounterColor(i+1));

				//para cada sample do contador
				for(int j=1; j<=numMaxSamples; j++)
				{
				//calculo o X e Y a usar
				curX=inboxX + HorseRadish::Math::ftoi(convX*((float)j));
				curY=inboxY + HorseRadish::Math::ftoi((gpuCounter->getValue(i+1,j) + valueYTrans) * valueYScale);

				//posso desenhar
				glImmediateMode->AddPosition(curX,curY);
				}

				//fim da linha
				glImmediateMode->EndDraw();
				}

				//desenho duas linhas a representar as barras do gráfico
				HorseRadish::OpenGL::glLineWidth(2.0f);
				glImmediateMode->BeginDraw(HorseRadish::OpenGL::Tools::ImmediateMode::LineStrip);
				glImmediateMode->AddColor(255, 255, 255, 255);
				glImmediateMode->AddPosition(inboxX,inboxY+inboxHeight);
				glImmediateMode->AddPosition(inboxX,inboxY);
				glImmediateMode->AddPosition(inboxX+inboxWidth,inboxY);
				glImmediateMode->EndDraw();
				HorseRadish::OpenGL::glLineWidth(1.0f);

				//digo à fonte para ligar tudo o que precisa para desenhar com a minima mudança de estados
				guiFont->paintBegin(transformMatrix);

				//onde vou desenhar a legenda
				posLegendaY = (this->tabContentAreaPos.y + this->tabContentAreaSize.height) - (textH + 5);

				//para cada contador
				for(int i=0; i<gpuCounter->getNumCounters(); i++)
				{
				HorseRadish::String unicodeStr;

				//tiro a string
				unicodeStr.Set(HorseRadish::String::Encoding::UTF8, gpuCounter->getCounterName(i+1));

				//ajusto a cor e escrevo o texto
				guiFont->setColor(gpuCounter->getCounterColor(i+1));
				guiFont->write(this->tabContentAreaPos.x + 5, posLegendaY, unicodeStr);

				//isto tem de descer
				posLegendaY -= (textH + 5);
				}

				//digo à fonte para acabar de desenhar o texto que falta e para desactivar os estados que alterou
				guiFont->draw();
				guiFont->paintEnd();*/
			}

			void ConsoleTabStats::ProcessMSG(const MSG * const msg)
			{
			}

			ConsoleTabExtra::ConsoleTabExtra(HorseRadish::Render::RendererDeferred * const renderDeferred) : ConsoleGUI::ConsoleTab()
			{
				tabName.Set(HorseRadish::String::Encoding::ASCII, "Extra large");

				this->renderDeferred = renderDeferred;

				this->currentRT = 0;
			}

			ConsoleTabExtra::~ConsoleTabExtra()
			{
			}

			void ConsoleTabExtra::DrawContent(HorseRadish::Render::Renderer2D * const render2D, const HorseRadish::Matrix &transformMatrix)
			{
				const HorseRadish::OpenGL::Objects::Texture *rtAlvo;
				HorseRadish::Color colorScale;

				colorScale.Set(1.0f);

				rtAlvo = nullptr;
				if (this->currentRT == 0)
				{
					rtAlvo = this->renderDeferred->GetRTTexture(HorseRadish::Render::RendererDeferred::Albedo);
				}
				else if (this->currentRT == 1)
				{
					rtAlvo = this->renderDeferred->GetRTTexture(HorseRadish::Render::RendererDeferred::Normals);
				}
				else if (this->currentRT == 2)
				{
					rtAlvo = this->renderDeferred->GetRTTexture(HorseRadish::Render::RendererDeferred::MiscA);
				}
				else if (this->currentRT == 3)
				{
					rtAlvo = this->renderDeferred->GetRTTexture(HorseRadish::Render::RendererDeferred::MiscB);
				}

				if (rtAlvo == nullptr)
					return;

				render2D->shaders.progDeferredDebug->Bind();
				HorseRadish::OpenGL::glUniformMatrix4fv(render2D->glUniformCache->GetUniformPos(render2D->shaders.progDeferredDebug->glID, "transformationMatrix"), 1, GL_FALSE, transformMatrix);
				HorseRadish::OpenGL::glUniform4fv(render2D->glUniformCache->GetUniformPos(render2D->shaders.progDeferredDebug->glID, "colorScale"), 1, (const float *)colorScale);

				HorseRadish::OpenGL::glBindTextureUnit(0, rtAlvo->glID);

				auto glImmediateMode = render2D->glImmediateMode;

				glImmediateMode->BeginDraw(HorseRadish::OpenGL::Tools::ImmediateMode::GeometryType::Quads);
				glImmediateMode->AddColorF(1.0f);

				glImmediateMode->AddTexCoord(this->tabContentAreaPos.x + this->tabContentScreenDelta.x, this->tabContentAreaPos.y + this->tabContentScreenDelta.y);
				glImmediateMode->AddPosition(this->tabContentAreaPos.x, this->tabContentAreaPos.y);

				glImmediateMode->AddTexCoord(this->tabContentAreaPos.x + this->tabContentAreaSize.width + this->tabContentScreenDelta.x, this->tabContentAreaPos.y + this->tabContentScreenDelta.y);
				glImmediateMode->AddPosition(this->tabContentAreaPos.x + this->tabContentAreaSize.width, this->tabContentAreaPos.y);

				glImmediateMode->AddTexCoord(this->tabContentAreaPos.x + this->tabContentAreaSize.width + this->tabContentScreenDelta.x, this->tabContentAreaPos.y + this->tabContentAreaSize.height + this->tabContentScreenDelta.y);
				glImmediateMode->AddPosition(this->tabContentAreaPos.x + this->tabContentAreaSize.width, this->tabContentAreaPos.y + this->tabContentAreaSize.height);

				glImmediateMode->AddTexCoord(this->tabContentAreaPos.x + this->tabContentScreenDelta.x, this->tabContentAreaPos.y + this->tabContentAreaSize.height + this->tabContentScreenDelta.y);
				glImmediateMode->AddPosition(this->tabContentAreaPos.x, this->tabContentAreaPos.y + this->tabContentAreaSize.height);
				glImmediateMode->EndDraw();

				auto guiFont = render2D->gui.fontConsole;

				guiFont->paintBegin(transformMatrix);
				guiFont->setColor(1.0f, 1.0f, 1.0f);
				if (this->currentRT == 0)
					guiFont->write(10.0f, this->tabContentAreaSize.height - guiFont->getMaxHeight(), HorseRadish::String("RT: albedo"));
				else if (this->currentRT == 1)
					guiFont->write(10.0f, this->tabContentAreaSize.height - guiFont->getMaxHeight(), HorseRadish::String("RT: albedo"));

				guiFont->draw();
				guiFont->paintEnd();
			}

			void ConsoleTabExtra::ProcessMSG(const MSG * const msg)
			{
				if (msg->message == WM_KEYDOWN)
				{
					if (msg->wParam == VK_PRIOR)
					{
						this->currentRT--;
						if (this->currentRT < 0)
							this->currentRT = 3;
						return;
					}
					if (msg->wParam == VK_NEXT)
					{
						this->currentRT++;
						if (this->currentRT > 3)
							this->currentRT = 0;
						return;
					}
				}
			}

		} //UI
	} //Console
} //HorseRadish