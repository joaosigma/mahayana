#pragma once
#ifndef __CONSOLE_GUI__
#define __CONSOLE_GUI__

#include "common\Platform.hpp"
#include "common\Common.hpp"
#include "common\Containers.hpp"
#include "common\Timer.hpp"
#include "common\String.hpp"
#include "common\Primitives2D.hpp"
#include "common\opengl\objects.hpp"
#include "common\openGL\tools.hpp"
#include "render\renderer2D.hpp"
#include "render\rendererDeferred.hpp"

#include "console.hpp"

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
		HorseRadish::String tabName;
		HorseRadish::Primitives2D::Point<int> tabContentAreaPos, tabContentScreenDelta;
		HorseRadish::Primitives2D::Size<int> tabContentAreaSize;
		int tabTextAreaWidth;

	public:
		ConsoleTab();
		virtual ~ConsoleTab();

		const HorseRadish::String* GetText();
		int GetTextAreaWidth();

		void SetTextAreaWidth(const int &width);
		void SetContentAreaSize(const int &width, const int &height);
		void SetContentAreaOrigin(const int &x, const int &y);
		void SetContentScreenDelta(const int &x, const int &y);
		
		virtual void DrawContent(HorseRadish::Render::Renderer2D* const render2D, const HorseRadish::Matrix &transformMatrix) = 0;
		virtual void ProcessMSG(const MSG * const msg) = 0;
	};

	enum TypeSMS{
		SMSFrame,
		SMSTime
		};
	enum TypeFunction{
		FunctionConsole = (1 << 0),
		FunctionInfo = (1 << 1),
		FunctionSMS = (1 << 2),
		FunctionAll = 0xFFFF
		};

private:
	struct ConteudoSMS{
		HorseRadish::String texto;
		float cor[3], alpha;
		TypeSMS type;
		HorseRadish::Timer tempo;
		bool fading;
	};

	const HorseRadish::OpenGL::Objects::Texture *texConsoleLogo;
	HorseRadish::Render::Renderer2D *renderData;
	Console *mainConsole;

	int consoleSelectedTab;
	bool consoleVisible;
	HorseRadish::Primitives2D::Rectangle<int> consoleRect;
	HorseRadish::Primitives2D::Size<int> consoleLogoSize, consoleTabSpace;
	HorseRadish::Containers::Array<ConsoleTab*> consoleTabPages;
	
	int infoNumVars, infoMaxWidth;
	bool infoVisible;
	HorseRadish::Containers::Array<HorseRadish::String> infoArrayVars;
	
	int smsMaxEntries, smsNumEntries;
	bool smsVisible;
	ConteudoSMS *smsEntries;

	static const int tabContentMargin; //pixels
	static const int infoVarSlack; //pixels
	static const int smsMessageSlack; //pixels
	static const float smsTimeVisible, smsTime2Hide; //segundos

	void infoCheckWidth(const HorseRadish::OpenGL::Tools::Font * const guiFont);
	void drawConsoleBackground(HorseRadish::Render::Renderer2D * const render2D, const HorseRadish::Matrix &transformMatrix, const float &consoleAlpha);
	void drawConsole(HorseRadish::Render::Renderer2D * const render2D, const HorseRadish::OpenGL::Tools::Viewport * const hrViewport);
	void drawInfo(HorseRadish::Render::Renderer2D * const render2D, const HorseRadish::OpenGL::Tools::Viewport * const hrViewport);
	void drawSMS(HorseRadish::Render::Renderer2D * const render2D, const HorseRadish::OpenGL::Tools::Viewport * const hrViewport);
public:
	ConsoleGUI(Console *console, HorseRadish::Render::Renderer2D* const render2D, HorseRadish::OpenGL::Objects::ObjectsManager* const textureManager);
	~ConsoleGUI();

	void Draw(const HorseRadish::OpenGL::Tools::Viewport * const hrViewport);
	bool GUIVisivel(const TypeFunction &function) const;

	ConsoleTab* ConsoleGetSelectedTab() const;
	void ConsoleAddTab(ConsoleTab * const newConsoleTab);
	void ConsoleProcessMSG(const MSG * const msg);
	void ConsoleVisible(const bool visible);
	bool ConsoleConsumesInput();

	void InfoAddVar(const char * const varName);
	void InfoVisible(const bool visible);

	void SMSAdd(const char * const what, const TypeSMS &type, const float &r, const float &g, const float &b);
	void SMSAdd(const char * const what, const TypeSMS &type);
	void SMSVisible(const bool visible);
};

class ConsoleUserHistory;
class ConsoleUserPrompt;

class ConsoleTabConsole : public ConsoleGUI::ConsoleTab
{
	struct LINHA_TEXTO{
		const void *metadata;
		HorseRadish::String texto;
		bool active;
	}*listaTexto;
	int numMaxLinhasTexto;
	HorseRadish::Timer cursorTimer;
	bool cursorVisivel;
	ConsoleUserHistory *userHistory;
	ConsoleUserPrompt *userPrompt;
	Console *mainConsole;
	int textoOffset;

public:
	ConsoleTabConsole(Console *mainConsole);
	~ConsoleTabConsole();

	void DrawContent(HorseRadish::Render::Renderer2D* const render2D, const HorseRadish::Matrix &transformMatrix);
	void ProcessMSG(const MSG * const msg);
	bool CriaTextoConsola(HorseRadish::Render::Renderer2D* const render2D);
	void ActualizaTextoConsola();
};

class ConsoleTabStats : public ConsoleGUI::ConsoleTab
{
public:
	ConsoleTabStats();
	~ConsoleTabStats();

	void DrawContent(HorseRadish::Render::Renderer2D* const render2D, const HorseRadish::Matrix &transformMatrix);
	void ProcessMSG(const MSG * const msg);
};

class ConsoleTabExtra : public ConsoleGUI::ConsoleTab
{
private:
	int currentRT;
	HorseRadish::Render::RendererDeferred *renderDeferred;

public:
	ConsoleTabExtra(HorseRadish::Render::RendererDeferred * const renderDeferred);
	~ConsoleTabExtra();

	void DrawContent(HorseRadish::Render::Renderer2D * const render2D, const HorseRadish::Matrix &transformMatrix);
	void ProcessMSG(const MSG * const msg);
};

}//namespace UI
}//namespace Console
}//namespace HorseRadish

#endif
