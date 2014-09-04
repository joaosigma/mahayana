#include "common\Platform.hpp"
#include "common\Timer.hpp"
#include "common\ImageFactory.hpp"
#include "common\Common.hpp"
#include "common\Containers.hpp"
#include "common\opengl\openGL.hpp"
#include "common\opengl\openGLext.hpp"
#include "common\opengl\objects.hpp"
#include "common\openGL\tools.hpp"

#include <windows.h>
#include <ZMouse.h>
#include <math.h>
#include <shellapi.h>

#include "consoleUI.hpp"

namespace HorseRadish
{
namespace Console
{
namespace UI
{

//§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§
//§§§§   -= Implementação de ConsoleGUI::ConsoleTab  =-   §§§§
//§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§
ConsoleGUI::ConsoleTab::ConsoleTab()
{
	tabTextAreaWidth = 0;
}
ConsoleGUI::ConsoleTab::~ConsoleTab()
{
}

const HorseRadish::String* ConsoleGUI::ConsoleTab::GetText()
{	return &tabName;	}
int ConsoleGUI::ConsoleTab::GetTextAreaWidth()
{	return tabTextAreaWidth;	}
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

//§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§
//§§§§   -= Implementação de ConsoleGUI  =-   §§§§
//§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§
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

	maxWidth=0;
	for(int i=0; i<this->infoNumVars; i++)
	{
		unicodeStr.Set(HorseRadish::String::Encoding::UTF8, mainConsole->VarPrint(this->infoArrayVars[i].GetData(), bufferOut, 256));
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
	HorseRadish::OpenGL::Tools::ImmediateMode *glImmediateMode;
	HorseRadish::OpenGL::Tools::Font *guiFont;

	//preciso de calcular onde começa o X da tabPage seleccionada
	selectedTabX = 0;
	for(int i=0; i<this->consoleSelectedTab; i++)
		selectedTabX += this->consoleTabPages[i]->GetTextAreaWidth();

	//e já agora o tamanho dela
	selectedTabWidth = this->consoleTabPages[this->consoleSelectedTab]->GetTextAreaWidth();

	//o programa em causa e todos os uniforms que preciso
	renderData->shaders.prog2DDrawNoTex->Bind();
	HorseRadish::OpenGL::glUniformMatrix4fv(renderData->glUniformCache->GetUniformPos(renderData->shaders.prog2DDrawNoTex->glID, "transformationMatrix"),1,GL_FALSE, transformMatrix);

	//para ajudar
	glImmediateMode = renderData->glImmediateMode;
	guiFont = renderData->gui.font;

	//o quadrado de fundo e por debaixo do texto das tabs
	glImmediateMode->BeginDraw(HorseRadish::OpenGL::Tools::ImmediateMode::Quads);
		glImmediateMode->AddColor(0, 0, 0, HorseRadish::Color::ConvertColor(consoleAlpha));
		glImmediateMode->AddQuad(0.0f, 0.0f, this->consoleRect.width, this->consoleRect.height - this->consoleTabSpace.height);
		glImmediateMode->AddQuad(0.0f, this->consoleRect.height - this->consoleTabSpace.height, this->consoleTabSpace.width, this->consoleTabSpace.height);
	glImmediateMode->EndDraw();

	//preparo as coisas para desenhar as linhas de bordo da consola
	HorseRadish::OpenGL::glLineWidth(2.0f);
	HorseRadish::OpenGL::glEnable(GL_LINE_SMOOTH);

	//tenho de desenhar agora as linhas
	glImmediateMode->BeginDraw(HorseRadish::OpenGL::Tools::ImmediateMode::Lines);

		//se tiver a primeira tab seleccionada
		if (this->consoleSelectedTab == 0)
		{
			//se tiver mais tabs além da primeira
			if (this->consoleTabPages.GetNumElements() > 1)
			{
				int curTabX;

				//a cor para as restantes é mais escura
				glImmediateMode->AddColor(68, 68, 68, 255);

				//a linha em baixo até à última tab
				glImmediateMode->AddLineH(selectedTabWidth, this->consoleTabSpace.width, this->consoleRect.height - 2.0f);

				//para todas as restantes tabs
				curTabX = selectedTabWidth;
				for(int i = 1; i < consoleTabPages.GetNumElements(); i++)
				{
					curTabX += this->consoleTabPages[i]->GetTextAreaWidth();
					glImmediateMode->AddLineV(curTabX, this->consoleRect.height - this->consoleTabSpace.height, this->consoleRect.height - 2.0);
				}
			}

			//as linhas à volta da tab seleccionada
			glImmediateMode->AddColor(128, 128, 128, 255);
			glImmediateMode->AddLineV(0.0f, this->consoleRect.height - this->consoleTabSpace.height, this->consoleRect.height);
			glImmediateMode->AddLineH(0.0f, selectedTabWidth, this->consoleRect.height);
			glImmediateMode->AddLineV(selectedTabWidth, this->consoleRect.height - this->consoleTabSpace.height, this->consoleRect.height);
			glImmediateMode->AddLineH(selectedTabWidth, this->consoleRect.width, this->consoleRect.height - this->consoleTabSpace.height);
		}
		//é qualquer outra tab que está seleccionada
		else
		{
			int curTabX;

			//a primeira coisa a desenhar é as linhas das tabs não seleccionadas (que ficam com esta cor)
			glImmediateMode->AddColor(68, 68, 68, 255);

			//a linha em cima até à tabSeleccionada e depois desde essa até à última (esta só vale a pena se não for a ultima tab)
			glImmediateMode->AddLineH(0.0f, selectedTabX, this->consoleRect.height - 2.0f);
			if (this->consoleSelectedTab < (this->consoleTabPages.GetNumElements() - 1))
				glImmediateMode->AddLineH(selectedTabX + selectedTabWidth, this->consoleTabSpace.width, this->consoleRect.height - 2.0f);

			//agora tenho de desenhar as barras individuais de cata tab até à seleccionada
			curTabX = 0;
			for(int i=0; i<this->consoleSelectedTab; i++)
			{
				glImmediateMode->AddLineV(curTabX, this->consoleRect.height - this->consoleTabSpace.height, this->consoleRect.height - 2.0f);
				curTabX += this->consoleTabPages[i]->GetTextAreaWidth();
			}

			//para todas as restantes tabs (à direita da seleccionada)
			curTabX = selectedTabX + selectedTabWidth;
			for(int i=this->consoleSelectedTab+1; i<this->consoleTabPages.GetNumElements(); i++)
			{
				curTabX += this->consoleTabPages[i]->GetTextAreaWidth();
				glImmediateMode->AddLineV(curTabX, this->consoleRect.height - this->consoleTabSpace.height, this->consoleRect.height - 2.0f);
			}

			//as linhas à volta da tab seleccionada
			glImmediateMode->AddColor(128, 128, 128, 255);
			glImmediateMode->AddLineH(0.0f, selectedTabX, this->consoleRect.height - this->consoleTabSpace.height);
			glImmediateMode->AddLineV(selectedTabX, this->consoleRect.height - this->consoleTabSpace.height, this->consoleRect.height);
			glImmediateMode->AddLineH(selectedTabX, selectedTabX+selectedTabWidth, this->consoleRect.height);
			glImmediateMode->AddLineV(selectedTabX+selectedTabWidth, this->consoleRect.height - this->consoleTabSpace.height, this->consoleRect.height);
			glImmediateMode->AddLineH(selectedTabX+selectedTabWidth, this->consoleRect.width, this->consoleRect.height - this->consoleTabSpace.height);
		}

		//a linha à esquerda, cima e à direita
		glImmediateMode->AddColor(128, 128, 128, 255);
		glImmediateMode->AddLineV(0.0f, 0.0f, this->consoleRect.height - this->consoleTabSpace.height);
		glImmediateMode->AddLineH(0.0f, this->consoleRect.width, 0.0f);
		glImmediateMode->AddLineV(this->consoleRect.width, 0.0f, this->consoleRect.height - this->consoleTabSpace.height);
	glImmediateMode->EndDraw();

	//reponho estado do GL
	HorseRadish::OpenGL::glLineWidth(1.0f);
	HorseRadish::OpenGL::glDisable(GL_LINE_SMOOTH);

	//se tenho logo
	if (texConsoleLogo != nullptr)
	{
		//a textura para o logo
		HorseRadish::OpenGL::glBindTextureUnit(0, texConsoleLogo->glID);

		//o programa em causa e todos os uniforms que preciso
		renderData->shaders.prog2DDrawTex->Bind();
		HorseRadish::OpenGL::glUniform1i(renderData->glUniformCache->GetUniformPos(renderData->shaders.prog2DDrawTex->glID, "texColorSampler"), 0);
		HorseRadish::OpenGL::glUniformMatrix4fv(renderData->glUniformCache->GetUniformPos(renderData->shaders.prog2DDrawTex->glID, "transformationMatrix"),1,GL_FALSE, transformMatrix);

		//basta desenhar este quadrado
		glImmediateMode->BeginDraw(HorseRadish::OpenGL::Tools::ImmediateMode::Quads);
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

	//não me posso esquecer de desenhar o texto de cada tab
	guiFont->paintBegin(transformMatrix);
	guiFont->setColor(1.0f,1.0f,1.0f);

	//para cada tab
	for(int i=0, curStartX = 0; i<this->consoleTabPages.GetNumElements(); i++)
		{
		int posX, posY;

		if (i == this->consoleSelectedTab)
			guiFont->setColor(1.0f,1.0f,1.0f);
		else
			guiFont->setColor(0.7f,0.7f,0.7f);
		
		posX = (this->consoleTabPages[i]->GetTextAreaWidth() - guiFont->getStringWidth(*this->consoleTabPages[i]->GetText())) / 2;
		posX += curStartX;
		posY = this->consoleRect.height - this->consoleTabSpace.height;
		posY += (this->consoleTabSpace.height - guiFont->getMaxHeight()) / 2;
		curStartX += this->consoleTabPages[i]->GetTextAreaWidth();
		guiFont->write(posX, posY, *this->consoleTabPages[i]->GetText());
		}

	//digo à fonte para desenhar o que falta e depois desactivar as coisas que precisa
	guiFont->draw();
	guiFont->paintEnd();

	//reponho isto (devido ao programa usado para escrever)
	renderData->shaders.prog2DDrawNoTex->Bind();
	HorseRadish::OpenGL::glUniformMatrix4fv(renderData->glUniformCache->GetUniformPos(renderData->shaders.prog2DDrawNoTex->glID, "transformationMatrix"),1,GL_FALSE, transformMatrix);
}

void ConsoleGUI::drawConsole(HorseRadish::Render::Renderer2D * const renderData, const HorseRadish::OpenGL::Tools::Viewport * const hrViewport)
{
	float consoleAlpha;
	HorseRadish::OpenGL::Tools::ImmediateMode *glImmediateMode;
	HorseRadish::Matrix transformMatrix;

	//se não está visivel
	if (this->consoleVisible == false)
		return;

	//preciso disto
	consoleAlpha = mainConsole->VarGetDataF("sys_consoleAlpha");

	//pego na matrix de projecção e multiplico-a pela modelview (neste caso, uma simples translação)
	transformMatrix.Set(hrViewport->getProj2D());
	transformMatrix.MultTranslate(this->consoleRect.x, this->consoleRect.y, 0.0f);

	//para ajudar
	glImmediateMode = renderData->glImmediateMode;

	//o fundo da consola
	drawConsoleBackground(renderData, transformMatrix, consoleAlpha);

	//agora mando desenhar a tabPage correcta
	this->consoleTabPages[this->consoleSelectedTab]->DrawContent(renderData, transformMatrix);
}

void ConsoleGUI::drawInfo(HorseRadish::Render::Renderer2D * const renderData, const HorseRadish::OpenGL::Tools::Viewport * const hrViewport)
{
	float lineY, consoleAlpha;
	char bufferOut[256];
	HorseRadish::String unicodeStr;
	HorseRadish::OpenGL::Tools::ImmediateMode *glImmediateMode;
	HorseRadish::OpenGL::Tools::Font *guiFont;

	//se não estou a ser visto
	if (this->infoVisible == false)
		return;

	//preciso disto
	consoleAlpha = mainConsole->VarGetDataF("sys_consoleAlpha");

	//vejo se estou bem de comprimentos
	infoCheckWidth(renderData->gui.font);

	//para ajudar
	glImmediateMode = renderData->glImmediateMode;
	guiFont = renderData->gui.font;

	//o programa em causa e todos os uniforms que preciso
	renderData->shaders.prog2DDrawNoTex->Bind();
	HorseRadish::OpenGL::glUniformMatrix4fv(renderData->glUniformCache->GetUniformPos(renderData->shaders.prog2DDrawNoTex->glID, "transformationMatrix"),1,GL_FALSE, hrViewport->getProj2D());

	//desenho o quadrado de fundo
	glImmediateMode->BeginDraw(HorseRadish::OpenGL::Tools::ImmediateMode::Quads);
		glImmediateMode->AddColor(0, 0, 0, HorseRadish::Color::ConvertColor(consoleAlpha));
		glImmediateMode->AddQuad(0.0f, 0.0f, this->infoMaxWidth + 10, (guiFont->getMaxHeight() + 3) * this->infoNumVars + 5);
	glImmediateMode->EndDraw();

	//algumas coisas para começar a desenhar as linhas de borda
	lineY = (guiFont->getMaxHeight() + 3) * this->infoNumVars + 5;
	HorseRadish::OpenGL::glLineWidth(2.0f);

	//tenho de desenhar agora as linhas
	glImmediateMode->BeginDraw(HorseRadish::OpenGL::Tools::ImmediateMode::Lines);
		glImmediateMode->AddColor(128, 128, 128, 255);
		glImmediateMode->AddLineH(0.0f, this->infoMaxWidth + 10, lineY);
		glImmediateMode->AddLineV(this->infoMaxWidth + 10, lineY, 0.0f);
	glImmediateMode->EndDraw();
	
	//reponho este estado
	HorseRadish::OpenGL::glLineWidth(1.0f);

	//preparo para escrever as variáveis todas
	guiFont->paintBegin(hrViewport->getProj2D());
	guiFont->setColor(1.0f,1.0f,1.0f,1.0f);

	//desenho cada variável
	lineY = 3.0f;
	for(int i=0; i<this->infoNumVars; i++)
	{
		unicodeStr.Set(HorseRadish::String::Encoding::UTF8, mainConsole->VarPrint(this->infoArrayVars[i].GetData(), bufferOut, 256)); 
		guiFont->write(5.0f, lineY, unicodeStr);
		lineY += guiFont->getMaxHeight() + ConsoleGUI::infoVarSlack;
	}

	//digo à fonte para acabar de desenhar o texto que falta e para desactivar os estados que alterou
	guiFont->draw();
	guiFont->paintEnd();
}

void ConsoleGUI::drawSMS(HorseRadish::Render::Renderer2D * const renderData, const HorseRadish::OpenGL::Tools::Viewport * const hrViewport)
{
	int winHeight;
	float textY,textH;
	HorseRadish::OpenGL::Tools::Font *guiFont;

	//se não tenho nada para mostrar ou não quero mostrar
	if ((this->smsNumEntries <= 0) || (this->smsVisible == false))
		return;

	//isto ajuda
	guiFont = renderData->gui.font;

	//dá jeito o tamanho máximo do ecran e da fonte a usar
	winHeight = mainConsole->VarGetDataI("r_winHeight");
	textH = (float)guiFont->getMaxHeight();

	//digo à fonte para ligar tudo o que precisa para desenhar com a minima mudança de estados
	guiFont->paintBegin(hrViewport->getProj2D());

	//desenho cada linha de texto
	textY = ((float)winHeight) - textH - ConsoleGUI::smsMessageSlack;
	for(int i = 0; i < this->smsNumEntries; i++, textY -= textH)
	{
		HorseRadish::String unicodeStr;

		//o texto a escrever
		unicodeStr.Set(HorseRadish::String::Encoding::UTF8, this->smsEntries[i].texto.GetData());

		//desenho a sombra do texto
		guiFont->setColor(0.0f, 0.0f, 0.0f, this->smsEntries[i].alpha * 0.8f);
		guiFont->write(ConsoleGUI::smsMessageSlack + 1.0f, textY - 1.0f, unicodeStr);

		//desenho o texto normalmente
		guiFont->setColor(this->smsEntries[i].cor[0], this->smsEntries[i].cor[1], this->smsEntries[i].cor[2], this->smsEntries[i].alpha);
		guiFont->write(ConsoleGUI::smsMessageSlack, textY, unicodeStr);
	}

	//digo à fonte para acabar de desenhar o texto que falta e para desactivar os estados que alterou
	guiFont->draw();
	guiFont->paintEnd();

	//agora só passo por cada linha para ver o que é preciso apagar
	for(int i = this->smsNumEntries - 1; i >= 0; i--)
	{
		//se for pra apagar, apago simplesmente (não esquecer que pode haver mais para a frente)
		if (this->smsEntries[i].type == ConsoleGUI::SMSFrame)
		{
			for(int j=i; j<this->smsNumEntries-1; j++)
				memcpy(this->smsEntries + j, this->smsEntries + j + 1, sizeof(ConteudoSMS));
			this->smsNumEntries--;
			continue;
		}

		//se for para ir apagando de acordo com o tempo
		if (this->smsEntries[i].type == ConsoleGUI::SMSTime)
		{
			//se ainda não comecei a fader fade
			if ((this->smsEntries[i].fading==false) && (this->smsEntries[i].tempo.GetTimeS() >= ConsoleGUI::smsTimeVisible) )
			{
				this->smsEntries[i].tempo.ReStart();
				this->smsEntries[i].fading = true;
			}

			//chegando aqui, se não estou ainda em fade, escuso de seguir
			if (this->smsEntries[i].fading == false)
				continue;

			//chegando aqui estou a fazer fade de certeza
			//verificar se já acabou, e se acabou quer dizer
			if (this->smsEntries[i].tempo.GetTimeS() >= ConsoleGUI::smsTime2Hide)
			{
				this->smsNumEntries--;
				continue;
			}

			//estou ainda a fazer fade, logo calculo-o e posso sair, sem continuar pra o proximo pq
			//este ainda não acabou
			this->smsEntries[i].alpha = 1.0f - (this->smsEntries[i].tempo.GetTimeS() / ConsoleGUI::smsTime2Hide);
			continue;
		}
		
		//chegando aqui, não é para fazer nada, logo posso sair
		break;
	}
}

ConsoleGUI::ConsoleGUI(Console *console, HorseRadish::Render::Renderer2D* const renderData, HorseRadish::OpenGL::Objects::ObjectsManager* const textureManager)
{
	HRSRC recursoH;
	HorseRadish::OpenGL::Tools::Font *guiFont;
	int winWidth, winHeight;

	//guardo isto
	this->mainConsole = console;
	this->renderData = renderData;

	//por omissão não tenho logo
	this->texConsoleLogo = nullptr;

	//limpo as coisas da consola
	this->consoleVisible = false;
	this->consoleSelectedTab = 0;

	//limpo as coisas do info
	this->infoNumVars = this->infoMaxWidth = 0;
	this->infoVisible = false;

	//limpo as coisas do SMS
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

	//preciso destas variáveis
	winWidth = mainConsole->VarGetDataI("r_winWidth");
	winHeight = mainConsole->VarGetDataI("r_winHeight");

	//para ajudar
	guiFont = renderData->gui.font;
	
	//preparo alguns parametros para a consola
	this->consoleRect.width = winWidth - 60;
	this->consoleRect.height = winHeight - 60;
	this->consoleTabSpace.width = 0;
	this->consoleTabSpace.height = guiFont->getMaxHeight() + ConsoleGUI::tabContentMargin * 2;
	this->consoleRect.x = (winWidth - this->consoleRect.width) / 2;
	this->consoleRect.y = (winHeight - this->consoleRect.height) / 2;
	
	//as dimensões do logo
	this->consoleLogoSize.width = this->consoleRect.height / 4;
	this->consoleLogoSize.height = this->consoleRect.height / 4;

	//posso criar espaço para os sms
	this->smsMaxEntries = this->consoleRect.height / guiFont->getMaxHeight();
	this->smsEntries = new ConteudoSMS[this->smsMaxEntries];
	this->smsNumEntries = 0;
}

ConsoleGUI::~ConsoleGUI()
{
	//apago cada um das tabs
	for(int curTabPageIndex = 0; curTabPageIndex < this->consoleTabPages.GetNumElements(); curTabPageIndex++)
		delete this->consoleTabPages[curTabPageIndex];

	//e as restantes coisas das tabs
	this->consoleTabPages.Clear();
	this->consoleSelectedTab = 0;

	//posso mandar limpar o info
	this->infoNumVars = this->infoMaxWidth = 0;
	this->infoVisible = false;
	this->infoArrayVars.Clear();

	//em relação ao SMS
	delete[] this->smsEntries;
	this->smsMaxEntries = this->smsNumEntries = 0;
	this->smsVisible = false;
	this->smsEntries = nullptr;
}

void ConsoleGUI::Draw(const HorseRadish::OpenGL::Tools::Viewport * const hrViewport)
{
	//basta chamar estas funções (atenção à ordem)
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
		if ((this->infoVisible == true) && (this->infoArrayVars.GetNumElements() > 0))
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
	//se não tenho nada seleccionado
	if ((this->consoleSelectedTab < 0) || (this->consoleSelectedTab >= this->consoleTabPages.GetNumElements()))
		return nullptr;

	//devolvo a tab seleccionada
	return this->consoleTabPages[this->consoleSelectedTab];
}

void ConsoleGUI::ConsoleAddTab(ConsoleTab * const newConsoleTab)
{
	//preciso de ter alguma coisa
	if (newConsoleTab == nullptr)
		return;

	//tenho estas tabPages
	this->consoleTabPages.Add(newConsoleTab);

	//posso calcular todos estes valores
	newConsoleTab->SetTextAreaWidth(renderData->gui.font->getStringWidth(*newConsoleTab->GetText()) + 12.0f);
	newConsoleTab->SetContentAreaOrigin(ConsoleGUI::tabContentMargin, ConsoleGUI::tabContentMargin);
	newConsoleTab->SetContentScreenDelta(this->consoleRect.x, this->consoleRect.y);
	newConsoleTab->SetContentAreaSize(this->consoleRect.width - ConsoleGUI::tabContentMargin * 2, this->consoleRect.height - this->consoleTabSpace.height - ConsoleGUI::tabContentMargin * 2);

	//aproveito e acumulo o comprimento total para as tabs
	this->consoleTabSpace.width += newConsoleTab->GetTextAreaWidth();
}

void ConsoleGUI::ConsoleProcessMSG(const MSG * const msg)
{
	//se a consola está visível
	if (this->GUIVisivel(HorseRadish::Console::UI::ConsoleGUI::FunctionConsole))
	{
		auto curTab = this->ConsoleGetSelectedTab();
		if (curTab)
			curTab->ProcessMSG(msg);
	}

	//se for um tecla e for a tecla '\'
	if ( (msg->message == WM_KEYDOWN) && (msg->wParam == 220) )
	{
		this->consoleVisible = !this->consoleVisible;
		return;
	}

	//se não estiver visivel, não é preciso fazer mais nada
	if (this->consoleVisible == false)
		return;

	//se for algum dos Fs (mudo a tab da consola)
	if ((msg->message == WM_KEYDOWN) && (msg->wParam >= VK_F1) && (msg->wParam <= VK_F24))
	{
		int novoIndiceTab;

		//tiro o novo indice e se está fora do intervalo, ignoro-o
		novoIndiceTab = msg->wParam - VK_F1;
		if ((novoIndiceTab < 0) || (novoIndiceTab >= this->consoleTabPages.GetNumElements()))
			return;

		//posso usar este indice
		this->consoleSelectedTab = novoIndiceTab;
		return;
	}
}

void ConsoleGUI::ConsoleVisible(const bool visible)
{
	//basta guardar isto
	this->consoleVisible = visible;
}

bool ConsoleGUI::ConsoleConsumesInput()
{
	//a consola consome input se estiver visivel e for a primeira tab a seleccionada (a própria consola)
	return ((this->consoleVisible == true) && (this->consoleSelectedTab == 0));
}

void ConsoleGUI::InfoAddVar(const char * const varName)
{	
	HorseRadish::String *novaLista;
	char bufferOut[128];

	//verificar se a variável é válida
	if (varName==nullptr || varName[0]=='\0')
		return;
	if (mainConsole->VarPrint(varName, bufferOut, 128)==nullptr)
		return;

	//se já tiver inserida, não insiro
	for(int i=0; i < this->infoNumVars; i++)
	{
		if (this->infoArrayVars[i] == varName)
			return;
	}

	//crio uma nova entrada
	this->infoArrayVars.Add();

	//copio e arranjo as coisas
	this->infoArrayVars[this->infoNumVars].Set(HorseRadish::String::UTF8, varName);
	this->infoNumVars++;
}

void ConsoleGUI::InfoVisible(const bool visible)
{	
	//se for para activar
	if (visible)
	{
		if ((this->infoArrayVars.GetNumElements() <= 0) || (this->infoVisible == true))
			return;

		this->infoVisible=true;
		return;
	}

	//chegando aqui é pra desactivar (muito simples)
	this->infoVisible=false;
}

void ConsoleGUI::SMSAdd(const char * const what, const TypeSMS &type, const float &r, const float &g, const float &b)
{
	//não adiciono nada se não estiver visivel
	if (this->smsVisible == false)
		return;

	//verificar dados
	if ((what == nullptr) || (what[0] == '\0'))
		return;

	//movo as frases para a frente
	for(int i = this->smsNumEntries; i > 0; i--)
		memcpy(this->smsEntries + i, this->smsEntries + i - 1, sizeof(ConteudoSMS));

	//posso preencher (é sempre o primeiro)
	this->smsEntries[0].type = type;
	this->smsEntries[0].cor[0] = HorseRadish::Math::fClamp(r, 0.0f, 1.0f);
	this->smsEntries[0].cor[1] = HorseRadish::Math::fClamp(g, 0.0f, 1.0f);
	this->smsEntries[0].cor[2] = HorseRadish::Math::fClamp(b, 0.0f, 1.0f);
	this->smsEntries[0].alpha = 1.0f;
	this->smsEntries[0].texto.Set(HorseRadish::String::UTF8, what);
	this->smsEntries[0].fading = false;
	this->smsEntries[0].tempo.ReStart();

	//estou a usar mais um
	this->smsNumEntries++;
	if (this->smsNumEntries >= this->smsMaxEntries)
		this->smsNumEntries = this->smsMaxEntries - 1;
}

void ConsoleGUI::SMSAdd(const char * const what, const TypeSMS &type)
{	this->SMSAdd(what, type, 1.0f, 1.0f, 1.0f);}

void ConsoleGUI::SMSVisible(const bool visible)
{	this->smsVisible = visible;}

}//namespace UI
}//namespace Console
}//namespace HorseRadish