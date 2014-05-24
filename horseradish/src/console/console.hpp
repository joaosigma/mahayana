#pragma once
#ifndef __CONSOLE__
#define __CONSOLE__

#include "common\Logger.hpp"
#include "common\Containers.hpp"
#include "common\String.hpp"
#include "common\Path.hpp"
#include "common\Avl-tree.hpp"

#include <functional>

namespace HorseRadish
{
namespace Console
{

//o máximo de string que se pode fazer log
#define MAX_LOG_SIZE	512

//as keys que se podem usar (é preferivel criar umas à mão)
#define SCONSOLE_NUMKEYS		57
#define SCONSOLE_KEY_A			0xa0a0
#define SCONSOLE_KEY_B			0xa0a1
#define SCONSOLE_KEY_C			0xa0a2
#define SCONSOLE_KEY_D			0xa0a3
#define SCONSOLE_KEY_E			0xa0a4
#define SCONSOLE_KEY_F			0xa0a5
#define SCONSOLE_KEY_G			0xa0a6
#define SCONSOLE_KEY_H			0xa0a7
#define SCONSOLE_KEY_I			0xa0a8
#define SCONSOLE_KEY_J			0xa0a9
#define SCONSOLE_KEY_K			0xa0aa
#define SCONSOLE_KEY_L			0xa0ab
#define SCONSOLE_KEY_M			0xa0ac
#define SCONSOLE_KEY_N			0xa0ad
#define SCONSOLE_KEY_O			0xa0ae
#define SCONSOLE_KEY_P			0xa0af
#define SCONSOLE_KEY_Q			0xa0b0
#define SCONSOLE_KEY_R			0xa0b1
#define SCONSOLE_KEY_S			0xa0b2
#define SCONSOLE_KEY_T			0xa0b3
#define SCONSOLE_KEY_U			0xa0b4
#define SCONSOLE_KEY_V			0xa0b5
#define SCONSOLE_KEY_W			0xa0b6
#define SCONSOLE_KEY_X			0xa0b7
#define SCONSOLE_KEY_Y			0xa0b8
#define SCONSOLE_KEY_Z			0xa0b9
#define SCONSOLE_KEY_F1			0xa0ba
#define SCONSOLE_KEY_F2			0xa0bb
#define SCONSOLE_KEY_F3			0xa0bc
#define SCONSOLE_KEY_F4			0xa0bd
#define SCONSOLE_KEY_F5			0xa0be
#define SCONSOLE_KEY_F6			0xa0bf
#define SCONSOLE_KEY_F7			0xa0c0
#define SCONSOLE_KEY_F8			0xa0c1
#define SCONSOLE_KEY_F9			0xa0c2
#define SCONSOLE_KEY_F10		0xa0c3
#define SCONSOLE_KEY_F11		0xa0c4
#define SCONSOLE_KEY_F12		0xa0c5
#define SCONSOLE_KEY_ESCAPE		0xa0c6
#define SCONSOLE_KEY_ENTER		0xa0c7
#define SCONSOLE_KEY_SPACE		0xa0c8
#define SCONSOLE_KEY_TAB		0xa0c9
#define SCONSOLE_KEY_PAUSE		0xa0ca
#define SCONSOLE_KEY_UP			0xa0cb
#define SCONSOLE_KEY_DOWN		0xa0cc
#define SCONSOLE_KEY_LEFT		0xa0cd
#define SCONSOLE_KEY_RIGHT		0xa0ce
#define SCONSOLE_KEY_0			0xa0cf
#define SCONSOLE_KEY_1			0xa0d0
#define SCONSOLE_KEY_2			0xa0d1
#define SCONSOLE_KEY_3			0xa0d2
#define SCONSOLE_KEY_4			0xa0d3
#define SCONSOLE_KEY_5			0xa0d4
#define SCONSOLE_KEY_6			0xa0d5
#define SCONSOLE_KEY_7			0xa0d6
#define SCONSOLE_KEY_8			0xa0d7
#define SCONSOLE_KEY_9			0xa0d8

#define SCONSOLE_KEY_MSHIFT		(1<<5)
#define SCONSOLE_KEY_MALT		(1<<6)
#define SCONSOLE_KEY_MCONTROL	(1<<7)

class Console

{
public:
	
	enum class VarType : int
	{
		Integer = 0xcca7,
		Float = 0xcca8,
		String = 0xcca9
	};

	enum VarFlags
	{
		None = 0,
		ReadOnly = (1 << 0), //the value can't be change through the console, only commands
		Clamp = (1 << 1), //the value is always clamped (ints: [0, +inf]; floats: [0, 1])
		Constant = (1 << 2), //the value can NEVER be changed
		Archive = (1 << 3), //the value is saved whenever it's changed (it's persistent)
		Server = (1 << 4), //the value is shared with the server
		Cheat = (1 << 5) //the value can only be changed when cheats are enabled
	};

	typedef std::function<void(Console &console, const unsigned int msgID, const unsigned int numParam, const char **param)> CallbackCommand;
	typedef std::function<void(Console &console, const unsigned int varID, const VarType type, void * const curVal, const void * const oldVal)> CallbackVariable;
	typedef std::function<void(Console &console, const char * msg)> CallbackLoop;

	HorseRadish::Logging::Logger *logger;
private:
	struct BINDS{
		char cmd[116];
		int key, mod;
	};

	struct ARG_DATA{
		char data[MAX_LOG_SIZE];
	};
	//é melhor definir estas coisas aqui
	struct COMANDO{
		Console::CallbackCommand callback;
	};
	struct VARIABLE{
		VarType tipo;
		VarFlags flags;
		bool condition;
		Console::CallbackVariable callback;

		union{
		 float numeric;
		 __int32 integer;
		 char string[128];
		} value;
	};

	struct DATA_NO{
		unsigned int id;
		bool isCommand;
		char nome[32],desc[128],printStr[64];

		VARIABLE varData;
		COMANDO cmdData;
	};
	
	CallbackLoop loopbackFuncError;
	bool changeOccured, doEchoLog, doEchoError;
	HorseRadish::AVLTree<DATA_NO> *treeConsole;
	HorseRadish::Containers::Array<BINDS> binds;
	ARG_DATA *argList;

	static const int tabNumSpaces;
	
	static bool checkValue(const char *value, unsigned char &realValue);
	static bool checkSentence(const char *sentence, int &numSpecialChar);
	static bool isForcedNoEcho(const char * const command);
	static bool isSeveralExpressions(const char * const command);

	bool addPhrase(const char *Frase, int nespacos = 0);
	void addToLogger(const char *sentence, int colorCount);
	//void finishTABComplete(const NO * const arvore, const char * const string, char * const bufferOut);
	bool parseCommand(const char * const command, ARG_DATA * const argList, int &argNumber);
	void doThreeArgOperation(DATA_NO * const spec, const ARG_DATA * const argList);
	void doThreeArgSetCondition(DATA_NO * const spec, const ARG_DATA * const argList);
	void doThreeArgCheckCondition(DATA_NO * const spec, const ARG_DATA * const argList);
	void doThreeArgVarCommand(DATA_NO * const spec, const ARG_DATA * const argList);
	void doFiveArgVarCommand(DATA_NO * const spec, const ARG_DATA * const argList);
	bool divideCommand(const char * const command);
	bool setVarI(DATA_NO * const parametros, const int data);
	bool setVarF(DATA_NO * const parametros, const float data);
	bool setVarS(DATA_NO * const parametros, const char *data);

public:
	Console(const HorseRadish::IO::Path &fileOutputPath);
	~Console();

	void Terminate();
	void SetEcho(const bool echoLog, const bool echoErrors);
	void PrintHelp(const char *what);

	bool RegisterBind(const int key, const int flagMod, const char *toWhat);
	void RegisterCommand(const unsigned int ID, const char *cmdName, const char *cmdDescription, CallbackCommand callbackFunc);
	bool RegisterVariable(const unsigned int ID, const char *varName, const VarType type, const char *varDescription, const char *printString, CallbackVariable callbackFunc);

	void BindEmitKey(const int key, const int flagMod);
	void BindClear();
	void FormatPrint(const char *bufferIn, char *bufferOut, const unsigned int bufferOutSize);
	void TABComplete(const char *bufferIn, char *bufferOut, const unsigned int bufferOutSize, std::function<void (const HorseRadish::String &hit)> callbackHitAction);
	
	bool VarSetDataI(const char *name, const int data);
	bool VarSetDataF(const char *name, const float data);
	bool VarSetDataS(const char *name, const char *data);
	bool VarGetType(const char *name, Console::VarType &varType);
	void VarSetAttrib(const char *name, const VarFlags flagGrant, const VarFlags flagDeny);
	int VarGetDataI(const char *name);
	float VarGetDataF(const char *name);
	const char* VarGetDataS(const char *name);
	const char* VarPrint(const char *varName, char *bufferOut, const int bufferSize);

	bool AsChanged();

	void PrintCommands(bool fullDescription);
	void PrintVars(bool fullDescription);
	void PrintBinds();

	bool Process(const char *command);

	void LogLoopback(CallbackLoop errorLoopbackFunc);
	bool Log(const char *s);
	bool LogError(const char *s);
	bool LogInfo(const char *s);
	bool LogTab(const char *s, const unsigned short numTabs);
	bool LogTabColor(const char *s, const unsigned short numTabs, const unsigned char colorR, const unsigned char colorG, const unsigned char colorB);
};

}//namespace Console
}//namespace HorseRadish

#endif
