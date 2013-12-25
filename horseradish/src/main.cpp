#include "common\platform.hpp"

//definir algumas coisas
//#define HR_VS_MEMORY_LEAKS
#define THREAD_PARAM_NUM_MSG	100

///§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§
//§§§§§§   -= Includes que necessito =-   §§§§§
//§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§

#ifdef HR_VS_MEMORY_LEAKS
	#define _CRTDBG_MAP_ALLOC
	#define _CRTDBG_MAPALLOC
	#include <stdlib.h>
	#include <crtdbg.h>
#endif

#include <time.h>
#include <float.h>
#include <process.h>
#include <windows.h>

#include "build.hpp"

#include "console\consoleUI.hpp"
#include "videoStream.hpp"
#include "platform\winApp.hpp"
#include "SGPUCounter.hpp"

#include "console\console.hpp"

#include "common\types.hpp"
#include "common\Encoders.hpp"
#include "common\hashing.hpp"
#include "common\common.hpp"
#include "common\memcontainers.hpp"
#include "common\timer.hpp"
#include "common\random.hpp"
#include "common\utf.hpp"
#include "common\machine.hpp"
#include "common\image.hpp"
#include "common\imageFactory.hpp"
#include "common\stream.hpp"
#include "common\platform.hpp"
#include "common\fileSystem.hpp"
#include "common\path.hpp"
#include "common\logger.hpp"
#include "common\sorting.hpp"
#include "common\math.hpp"
#include "common\smartpointers.hpp"
#include "common\opengl\openGL.h"
#include "common\opengl\openGLext.hpp"
#include "common\opengl\objects.hpp"
#include "render\world.hpp"
#include "render\rendererDeferred.hpp"
#include "render\renderer2D.hpp"

#define HR_EXIT_NOT_YET				0xdea0
#define HR_EXIT_QUIT				0xdead
#define HR_EXIT_RESTART				0xdeae
#define HR_EXIT_RESTART_VID			0xdeaf
#define HR_EXIT_RESTART_EDITOR		0xdea1
#define HR_EXIT_RENDER_INIT_ERROR	0xdea2

using namespace HorseRadish::Console;

//§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§
//§§§§§§   -= Todas as variáveis globais =-   §§§§§
//§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§
static
struct EDITOR_PARAMETERS{
	unsigned int editorWinW, editorWinH;
	HWND editorWinHandle;
	void *mappedBuffer;
	char *mappedBufferChar;
	HANDLE hMapObject;
	HANDLE hMutex;
	HANDLE hEventEditorWrote,hEventAppWrote;
	unsigned int msgID;
};
struct THREAD_RENDER_PARAMETERS{
	MSG listaMain[THREAD_PARAM_NUM_MSG];
	MSG listaAux[THREAD_PARAM_NUM_MSG];

	unsigned int numMsgListaMain, numMsgListaAux;
};

static DWORD gbAppExitCode=0;
static RawInput *gbRawInput=nullptr;
static Window *gbWindow=nullptr;
static HANDLE gbThreadRenderEventExit=nullptr;
static HANDLE gbThreadRenderEventReady=nullptr;
static HorseRadish::Timer gbTotalTimer;
static int gbAppExitReason=HR_EXIT_NOT_YET;
static EDITOR_PARAMETERS *gbEditorParam = nullptr;
static THREAD_RENDER_PARAMETERS *gbThreadParams=nullptr;
static HorseRadish::IO::FileSystem *gbFileSystem=nullptr;
static Console *gbMainConsole;

void renderThread(void *threadData);
void editorProcessMemory();
void processWindowsKeyUp(WPARAM wParam);
void writeSystemInfo();
void systemExit(const int exitCause);
void parseAppConfFile(const HorseRadish::IO::Path &filePath);
LRESULT CALLBACK windowsMessages(HWND hWnd, UINT Message, WPARAM wParam, LPARAM lParam);

//§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§
//§§§§§§   -= As funções de callback para os comandos e variáveis da consola =-	§§§§§
//§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§
static
int CALLBACK consoleCallbackCommands(const unsigned int cmdID, const unsigned int numParam, const char **param)
{
	//*****
	//com_createVar
	if (cmdID==1)
	{		
		return 0;
	}

	//*****
	//com_execCfg
	if (cmdID==2)
	{
		//tenho de ter 1 argumentos (o nome do ficheiro que devo ler)
		if (numParam!=2)
		{
			gbMainConsole->LogError("Error in arguments: com_execCfg <file to read>");
			return 0;
		}

		//faço parse e prontos
		parseAppConfFile(HorseRadish::IO::Path(param[1]));
		return 0;
	}

	//*****
	//com_unBind
	if (cmdID==3)
	{
		return 0;
	}

	//*****
	//com_unBindAll
	if (cmdID==4)
	{
		gbMainConsole->BindClear();
		return 0;
	}

	//*****
	//com_list
	if (cmdID==5)
	{
		gbMainConsole->PrintCommands(false);
		gbMainConsole->PrintVars(false);
		return 0;
	}

	//*****
	//com_listBind
	if (cmdID==6)
	{
		gbMainConsole->PrintBinds();
		return 0;
	}

	//*****
	//com_echoOff
	if (cmdID==7)
	{
		gbMainConsole->SetEcho(false,true);
		return 0;
	}

	//*****
	//com_echoOn
	if (cmdID==8)
	{
		gbMainConsole->SetEcho(true,true);
		return 0;
	}

	//*****
	//com_console
	/*if (cmdID==9)
	{
		if (numParam==1)
			{
			if (GUIVisivel(CONSOLE_GUI_CONSOLE))
				GUIConsoleVisible(false);
			else
				GUIConsoleVisible(true);
			return 1;
			}
		if (numParam!=2)
			return -1;

		if (!strcmp("open",param[1]))
			{
			GUIConsoleVisible(true);
			return 1;
			}
		if (!strcmp("close",param[1]))
			{
			GUIConsoleVisible(false);
			return 1;
			}
		return -1;
	}*/

	//*****
	//com_log
	if (cmdID==10)
	{
		if (numParam!=2)
			return -1;
		gbMainConsole->Log(param[1]);
		return 1;
	}

	//*****
	//com_sms
	/*if (cmdID==11)
	{
		if (numParam!=2)
			return -1;
		GUISMSAdd(param[1], CONSOLE_SMS_TIME, 1.0f, 1.0f, 1.0f);
		return 1;
	}*/

	//*****
	//sys_restart
	if (cmdID==30)
	{
		systemExit(HR_EXIT_RESTART);
		return 0;
	}

	//*****
	//sys_forceCrash
	if (cmdID==31)
	{
		//provoco um abort e mais nada
		abort();
		return 0;
	}

	//*****
	//ed_begin
	if (cmdID==40)
	{
		systemExit(HR_EXIT_RESTART_EDITOR);
		return 0;
	}

	//*****
	//ed_exportHRF
	if (cmdID==41)
	{
		//tenho de ter pelo menos 1 argumento (onde salvar o ficheiro)
		if (numParam<2)
		{
			gbMainConsole->LogError("Error in arguments: ed_exportHRF <path to file>");
			return 0;
		}

		//basta gravar para o sitio indicado
		/*if (RenderExportHRF(renderData, param[1]) == 0)
			gbMainConsole->LogInfo(SStringUTF8("Scene exported to: \"%s\"", param[1]));
		else
			gbMainConsole->LogError("error exporting scene.");*/
		return 0;
	}

	//*****
	//quit
	if (cmdID==50)
	{
		systemExit(HR_EXIT_QUIT);
		return 0;
	}

	//*****
	//help
	if (cmdID==51)
	{
		//se não tenho argumentos, imprimo a ajuda da própria consola
		if (numParam==1)
		{
			gbMainConsole->PrintHelp(nullptr);
			return 0;
		}

		//se tenho um argumento, imprimo ajuda sobre ele
		if (numParam==2)
		{
			gbMainConsole->PrintHelp(param[1]);
			return 0;
		}

		//chegando aqui é barraca
		gbMainConsole->LogError("Error in arguments: help [\"command\" | \"variable\"]");
		return 0;
	}
		
	//*****
	//bind
	if (cmdID==52)
		{
		//tenho de ter 2 argumentos
		if (numParam!=3)
		{
			gbMainConsole->LogError("Error in arguments: bind <key> <\"command\">");
			return 0;
		}
		
		//se for uma letra
		if (param[1][0]>='A' && param[1][0]<='Z' && param[1][1]=='\0'){
			gbMainConsole->RegisterBind(param[1][0]-'A'+SCONSOLE_KEY_A,0,param[2]);
			return 0;
		}
		//se for um numero
		if (param[1][0]>='0' && param[1][0]<='9' && param[1][1]=='\0'){
			gbMainConsole->RegisterBind(param[1][0]-'0'+SCONSOLE_KEY_0,0,param[2]);
			return 0;
		}
		//se for um F
		if (param[1][0]=='F' && param[1][1]>='1' && (param[1][2]=='\0' || (param[1][2]>='0' && param[1][2]<='2' && param[1][3]=='\0'))){
			gbMainConsole->RegisterBind(atoi(param[1]+1)+SCONSOLE_KEY_F1-1,0,param[2]);
			return 0;
		}
		//agora vejo os outros casos todos
		if (strcmp(param[1],"ESCAPE")==0){
			gbMainConsole->RegisterBind(SCONSOLE_KEY_ESCAPE,0,param[2]);
			return 0;
		}
		if (strcmp(param[1],"ENTER")==0){
			gbMainConsole->RegisterBind(SCONSOLE_KEY_ENTER,0,param[2]);
			return 0;
		}
		if (strcmp(param[1],"SPACE")==0){
			gbMainConsole->RegisterBind(SCONSOLE_KEY_SPACE,0,param[2]);
			return 0;
		}
		if (strcmp(param[1],"TAB")==0){
			gbMainConsole->RegisterBind(SCONSOLE_KEY_TAB,0,param[2]);
			return 0;
		}
		if (strcmp(param[1],"PAUSE")==0){
			gbMainConsole->RegisterBind(SCONSOLE_KEY_PAUSE,0,param[2]);
			return 0;
		}
		if (strcmp(param[1],"UP")==0){
			gbMainConsole->RegisterBind(SCONSOLE_KEY_UP,0,param[2]);
			return 0;
		}
		if (strcmp(param[1],"DOWN")==0){
			gbMainConsole->RegisterBind(SCONSOLE_KEY_DOWN,0,param[2]);
			return 0;
		}
		if (strcmp(param[1],"LEFT")==0){
			gbMainConsole->RegisterBind(SCONSOLE_KEY_LEFT,0,param[2]);
			return 0;
		}
		if (strcmp(param[1],"RIGHT")==0){
			gbMainConsole->RegisterBind(SCONSOLE_KEY_RIGHT,0,param[2]);
			return 0;
		}

		//chegando aqui, deu barraca e eu nao conheco o bind
		gbMainConsole->LogError("Error in arguments: bind key unrecognized.");
		return 0;
	}
}

static
int CALLBACK consoleCallbackVariables(const unsigned int varID, const int type, const void *curVal, const void *oldVal)
{
	//r_varAuxF1
	/*if (varID == 20)
	{
		HorseRadish::String dadosMostrar;

		dadosMostrar.SetPrintf(HorseRadish::String::UTF8, "r_varAuxF1: %f", *((const float*)curVal));
		GUISMSAdd((const char*)dadosMostrar, CONSOLE_SMS_TIME, 1.0f,0.0f,0.0f);
		return 0;
	}

	//r_varAuxF2
	if (varID == 21)
	{
		HorseRadish::String dadosMostrar;

		dadosMostrar.SetPrintf(HorseRadish::String::UTF8, "r_varAuxF2: %f", *((const float*)curVal));
		GUISMSAdd((const char*)dadosMostrar, CONSOLE_SMS_TIME, 1.0f,0.0f,0.0f);
		return 0;
	}*/

	//sys_consoleTextFont
	if (varID == 31)
	{
		const char* valorTexto;
		valorTexto = (const char*)curVal;
		if ((valorTexto == nullptr) || (*valorTexto == '\0'))
			return -1;
		return 0;
	}

	//sys_consoleTextSize
	if (varID == 32)
	{
		*((int*)curVal) = HorseRadish::Math::iClamp( *((const int*)curVal), 3, 50);
		return 0;
	}

	//*****
	//r_winSwapInterval
	/*if (varID==1)
	{
		int interval;
		interval = *((int*)curVal);

		//tenho de ter isto, senão nada feito
		if (interval<0 || interval>10)
			return -1;

		//basta meter o valor e já tá
		if (HorseRadish::OpenGL::Windows::wglSwapIntervalEXT != nullptr)
			HorseRadish::OpenGL::Windows::wglSwapIntervalEXT(interval);
		return 0;
	}*/

	//*****
	//r_winFOV
	/*if (varID==5)
	{
		float fov;
		fov = *((float*)curVal);

		//tenho de ter isto, senão nada feito
		if (fov<=0.0f || fov>=180.0f)
			return -1;

		//basta meter o valor e já tá
		if (rendererData)
			rendererData->viewport.setFOV(fov);
		return 0;
	}*/

	//*****
	//r_drawMode
	/*if (varID==40)
	{
		int drawMode;
		
		//tiro o valor e verifico o intervalo onde está
		drawMode = *((int*)curVal);
		if (drawMode<0 || drawMode>7)
			return -1;

		//só escrevo alguma coisa se realmente mudei de modo
		if (drawMode != *((int*)oldVal))
		{
			//de acordo com o valor que fica
			switch(drawMode)
			{
				case 7: GUISMSAdd("Modo de desenho: produzem sombras",SCONSOLE_SMS_TIME);		break;
				case 6: GUISMSAdd("Modo de desenho: sólido",SCONSOLE_SMS_TIME);		break;
				case 5: GUISMSAdd("Modo de desenho: ambiente",SCONSOLE_SMS_TIME);	break;
				case 4: GUISMSAdd("Modo de desenho: bump",SCONSOLE_SMS_TIME);		break;
				case 3: GUISMSAdd("Modo de desenho: especular",SCONSOLE_SMS_TIME);	break;
				case 2: GUISMSAdd("Modo de desenho: normais",SCONSOLE_SMS_TIME);	break;
				case 1: GUISMSAdd("Modo de desenho: diffuso",SCONSOLE_SMS_TIME);	break;

				default:
				case 0: GUISMSAdd("Modo de desenho: completo",SCONSOLE_SMS_TIME);	break;
			}
		}

		//tá tudo bem
		return 0;
	}

	//*****
	//r_postProcess
	if (varID==41)
	{
		ENGINE_PPROCESS_MATERIAL *pprocess;

		//tiro o postprocess a usar e se for igual a que já cá tá (pode incluir nullptr), não faço nada
		pprocess = PProcessFind(libraryPProcess,libraryNumPProcess,(const char*)curVal);
		if (rendererData->opengl.pprocessMaterial==pprocess)
			return 0;

		//se tiver algo anterior, limpo
		if (rendererData->opengl.pprocessMaterial)
			PProcessUnLoad(rendererData->opengl.pprocessMaterial);

		//se tiver algo escolhido, tento carregar
		if ( (pprocess!=nullptr) && (PProcessLoad(pprocess)==false))
		{
			PProcessUnLoad(pprocess);
			pprocess=nullptr;
		}

		//coloco o novo e mando o renderer refazer algumas coisas
		rendererData->opengl.pprocessMaterial=pprocess;
		RendererUpdatePostProcess(rendererData);
		return 0;
	}

	//*****
	//tex_diffuseFilter
	if (varID==50)
	{
		SString filterName;
		
		//se o nome do filtro for válido
		filterName=(const char*)curVal;
		if (filterName=="none" || filterName=="linear" || filterName=="bilinear" || filterName=="trilinear")
		{
			//passo por cada material e mudo os parametros
			if (rendererData)
			{
				for(int i=0; i<rendererData->numMaterials; i++)
					MaterialSetTexParam(rendererData->materials+i);
			}
			return 0;
		}
	
		//chegando aqui o valor é inválido
		return -1;
	}

	//*****
	//tex_diffuseAnisotropic
	if (varID==51)
	{
		float anisoVal;
		
		//tiro o valor e não aceito valores inferiores a 1.0f
		anisoVal = *((float*)curVal);
		if (anisoVal<1.0f)
			return -1;

		//tá tudo bem, logo passo por cada material, mudo os parametros e posso sair
		if (rendererData)
		{
			for(int i=0; i<rendererData->numMaterials; i++)
				MaterialSetTexParam(rendererData->materials+i);
		}
		return 0;
	}*/

	//tudo normal
	return 0;
}

static
void registerConsoleCommands()
{
	gbMainConsole->RegisterCommand(1,"r_reloadMaterial","Faz reload do material especificado", nullptr);
	gbMainConsole->RegisterCommand(4,"r_reloadScene","Faz reload da cena actual carregada", nullptr);
	gbMainConsole->RegisterCommand(5,"r_printMaterial","Mostra a informação acerca do material especificado", nullptr);
	gbMainConsole->RegisterCommand(6,"r_printLight","Mostra a informação acerca da luz especificada", nullptr);
	gbMainConsole->RegisterCommand(7,"r_showInfo","Mostra ou adiciona variáveis de informação", nullptr);
	
	gbMainConsole->RegisterCommand(20,"r_camSetPos","Indica a nova posição da camera", nullptr);
	gbMainConsole->RegisterCommand(21,"r_camSetDir","Indica a nova direcção da camera", nullptr);
	gbMainConsole->RegisterCommand(22,"r_camLoad","Carrega uma camera cinemática", nullptr);
	gbMainConsole->RegisterCommand(23,"r_camStop","Pára uma camera cinemática (caso exista)", nullptr);
	gbMainConsole->RegisterCommand(24,"r_camInfo","Imprime dados relativos a uma camera", nullptr);
	
	gbMainConsole->RegisterCommand(1,"com_createVar","Cria uma nova variável na consola",consoleCallbackCommands);
	gbMainConsole->RegisterCommand(2,"com_execCfg","Executa um ficheiro de configuração",consoleCallbackCommands);
	gbMainConsole->RegisterCommand(3,"com_unBind","Termina uma ligação de uma tecla a um comando",consoleCallbackCommands);
	gbMainConsole->RegisterCommand(4,"com_unBindAll","Termina todas as ligações de uma tecla a um comando",consoleCallbackCommands);
	gbMainConsole->RegisterCommand(5,"com_list","Forneçe uma lista de todos os comandos e variáveis disponiveis",consoleCallbackCommands);
	gbMainConsole->RegisterCommand(6,"com_listBind","Fornece uma lista de todos os binds disponiveis",consoleCallbackCommands);
	gbMainConsole->RegisterCommand(7,"com_echoOff","Comandos deixam de fazer echo na consola (excepto erros)",consoleCallbackCommands);
	gbMainConsole->RegisterCommand(8,"com_echoOn","Comandos tornam a fazer echo na consola",consoleCallbackCommands);
	gbMainConsole->RegisterCommand(9,"com_console","Abrir/fechar a consola",consoleCallbackCommands);
	gbMainConsole->RegisterCommand(10,"com_log","Sends text to the output of the console.",consoleCallbackCommands);
	gbMainConsole->RegisterCommand(11,"com_sms","Sends a sms to the screen.",consoleCallbackCommands);

	gbMainConsole->RegisterCommand(30,"sys_restart","Reinicia toda a aplicação", consoleCallbackCommands);
	gbMainConsole->RegisterCommand(31,"sys_forceCrash","Força a aplicação a crashar imediatamente", consoleCallbackCommands);
	
	gbMainConsole->RegisterCommand(40,"ed_begin","Calls the editor.", consoleCallbackCommands);
	gbMainConsole->RegisterCommand(41,"ed_exportHRF","Exports the entire scene contents to a HRF file",consoleCallbackCommands);
	
	gbMainConsole->RegisterCommand(50,"quit","Termina a aplicação (fechando tudo normalmente)", consoleCallbackCommands);
	gbMainConsole->RegisterCommand(51,"help","Displays help information of a command or variable or the help of the console", consoleCallbackCommands);
	gbMainConsole->RegisterCommand(52,"bind","Cria uma ligação de uma tecla a um comando.", consoleCallbackCommands);
}

static
void registerConsoleVariables()
{
	HorseRadish::String stringAux;

	gbMainConsole->RegisterVariable(0,"r_infoMTRIS",Console::Float,"Milhões de triangulos por segundo (por frame)","MTRIS: %.4f",nullptr);
	gbMainConsole->RegisterVariable(0,"r_infoMVERTS",Console::Float,"Keeps the current million of vertices per frame value.","MVERTS: %.3f",nullptr);
	gbMainConsole->RegisterVariable(0,"r_infoMaterials",Console::Integer,"Número de materiais usados na última frame","Número de materiais: %d",nullptr);
	gbMainConsole->RegisterVariable(0,"r_infoTextures",Console::Integer,"Número de texturas diferentes usadas na última frame","Número de texturas: %d",nullptr);
	gbMainConsole->RegisterVariable(0,"r_infoObjectos",Console::Integer,"Número de objectos usados na última frame","Número de objectos: %d",nullptr);
	gbMainConsole->RegisterVariable(0,"r_infoLights",Console::Integer,"Número de luzes usadas na última frame","Número de luzes: %d",nullptr);
	gbMainConsole->RegisterVariable(0,"r_infoPosition",Console::String,"Posição da camera que controla a vista (por frame)","Posição: %s",nullptr);
	gbMainConsole->RegisterVariable(0,"r_infoGLError",Console::String,"Último erro reportado pelo OpenGL","OpenGL error: %s",nullptr);
	gbMainConsole->RegisterVariable(0,"r_infoGLDrawElements",Console::Integer,"Número de chamadas a glDrawElements (por frame)","GLDrawElements: %d",nullptr);
	gbMainConsole->RegisterVariable(0,"r_infoViewDir",Console::String,"Direcção da camera (por frame)","View direction: %s",nullptr);

	gbMainConsole->RegisterVariable(0,"r_showTris",Console::Integer,"Mostra os triângulos que compoem a cena",nullptr,nullptr);
	gbMainConsole->RegisterVariable(0,"r_showBBox",Console::Integer,"Mostra as bounding boxes dos objectos",nullptr,nullptr);
	gbMainConsole->RegisterVariable(0,"r_showBSphere",Console::Integer,"Mostra as bounding spheres dos objectos",nullptr,nullptr);
	gbMainConsole->RegisterVariable(0,"r_showSurfInfo",Console::Integer,"Mostra info da superfície que intersecta o rato",nullptr,nullptr);
	gbMainConsole->RegisterVariable(0,"r_showLightFrustum",Console::Integer,"Mostra os frustums das luzes",nullptr,nullptr);
	gbMainConsole->RegisterVariable(0,"r_showLightBBox",Console::Integer,"Mostra as bounding box das superfícies para as luzes",nullptr,nullptr);
	gbMainConsole->RegisterVariable(0,"r_showNormals",Console::Integer,"Mostra as normais de cada objecto",nullptr,nullptr);
	gbMainConsole->RegisterVariable(0,"r_showOrthoSpace",Console::Integer,"Mostra o espaço ortonormal de cada objecto",nullptr,nullptr);
	gbMainConsole->RegisterVariable(0,"r_showOverdraw",Console::Integer,"Mostra o overdraw dos passes (vermelho – azul)",nullptr,nullptr);
	gbMainConsole->RegisterVariable(0,"r_showSkeletons",Console::Integer,"Mostra os esqueletos dos modelos animados",nullptr,nullptr);
	gbMainConsole->RegisterVariable(0,"r_showCrosshair",Console::Integer,"Mostra a mira do rato",nullptr,nullptr);
	gbMainConsole->RegisterVariable(0,"r_showColliders",Console::Integer,"Mostra os objectos testados contra colisão",nullptr,nullptr);
	gbMainConsole->RegisterVariable(0,"r_showTexture",Console::String,"Mostra uma dada textura no canto inferior direito",nullptr,nullptr);

	gbMainConsole->RegisterVariable(1,"r_winSwapInterval",Console::Integer,"O valor para o swapinterval",nullptr,consoleCallbackVariables);
	gbMainConsole->RegisterVariable(2,"r_winWidth",Console::Integer,"O comprimento da janela de visualização",nullptr,consoleCallbackVariables);
	gbMainConsole->RegisterVariable(3,"r_winHeight",Console::Integer,"A altura da janela de visualização",nullptr,consoleCallbackVariables);
	gbMainConsole->RegisterVariable(4,"r_winFullscreen",Console::Integer,"Se a janela está ou não fullscreen",nullptr,consoleCallbackVariables);
	gbMainConsole->RegisterVariable(5,"r_winFOV",Console::Float,"O field of view da janela",nullptr,consoleCallbackVariables);
	gbMainConsole->RegisterVariable(6,"r_winRefresh",Console::Integer,"O refresh rate quando está em fullscreen",nullptr,consoleCallbackVariables);
	
	gbMainConsole->RegisterVariable(0,"r_useVBOIndex",Console::Integer,"Usa ou não VBO's para guardar os indices",nullptr,nullptr);
	gbMainConsole->RegisterVariable(0,"r_useLightScissor",Console::Integer,"Usa ou não scissor test para as luzes",nullptr,nullptr);
	gbMainConsole->RegisterVariable(0,"r_useParallax",Console::Integer,"Usa ou não o efeito de parallax (global)",nullptr,nullptr);
	gbMainConsole->RegisterVariable(0,"r_useShadows",Console::Integer,"Usa ou não sombras (global)",nullptr,nullptr);
	gbMainConsole->RegisterVariable(0,"r_useAmbient",Console::Integer,"Faz ou não o passe ambiente (global)",nullptr,nullptr);
	gbMainConsole->RegisterVariable(0,"r_useGlow",Console::Integer,"Liga ou não o uso de glare / bloom na cena",nullptr,nullptr);
		
	gbMainConsole->RegisterVariable(0,"r_glDriver",Console::String,"Qual o driver de OpenGL a usar",nullptr,nullptr);
	gbMainConsole->RegisterVariable(0,"r_glDebug",Console::Integer,"Indica se é ou não para colocar o OpenGL em modo de debug",nullptr,nullptr);
	gbMainConsole->RegisterVariable(0,"r_orthoSpaceScale",Console::Float,"A escala a usar para mostrar normais, tangentes e binormais",nullptr,nullptr);
	gbMainConsole->RegisterVariable(0,"r_filterShadows",Console::Integer,"Se filtra ou não as sombras",nullptr,nullptr);
	gbMainConsole->RegisterVariable(0,"r_drawInfinites",Console::Integer,"Se deverá ou não desenhar infinitos",nullptr,nullptr);
	gbMainConsole->RegisterVariable(0,"r_drawNoLighting",Console::Integer,"Se deverá ou não desenhar materiais não iluminados",nullptr,nullptr);
    gbMainConsole->RegisterVariable(40,"r_drawMode",Console::Integer,"Que tipo de desenho deverá ser feito",nullptr, consoleCallbackVariables);
	
	gbMainConsole->RegisterVariable(20,"r_varAuxF1",Console::Float,"A generic float variable for debugging purposes",nullptr, consoleCallbackVariables);
	gbMainConsole->RegisterVariable(21,"r_varAuxF2",Console::Float,"A generic float variable for debugging porpuses",nullptr, consoleCallbackVariables);

	gbMainConsole->RegisterVariable(41,"r_ppEffect",Console::String,"The postprocessing effect to use",nullptr, consoleCallbackVariables);
	gbMainConsole->RegisterVariable(0,"r_ppTone",Console::Float,"Amount of brightness to use in postprocessing",nullptr,nullptr);
	gbMainConsole->RegisterVariable(0,"r_ppBrightness",Console::Float,"Amount of tone to use in postprocessing",nullptr,nullptr);
	gbMainConsole->RegisterVariable(0,"r_ppAO",Console::Integer,"------",nullptr,nullptr);

	gbMainConsole->RegisterVariable(0,"r_hdriGlowThreshold",Console::Float,"A quantidade a subtrair à cor para obter o glow",nullptr,nullptr);

	gbMainConsole->RegisterVariable(50,"tex_diffuseFilter",Console::String,"O tipo de filtro a aplicar às texturas difusas",nullptr, consoleCallbackVariables);
	gbMainConsole->RegisterVariable(51,"tex_diffuseAnisotropic",Console::Float,"A quantidade de anisotrópico a aplicar às texturas difusas",nullptr, consoleCallbackVariables);

	gbMainConsole->RegisterVariable(0,"p_infoVelocity",Console::String,"A velocidade do player",nullptr,nullptr);
	gbMainConsole->RegisterVariable(0,"p_gravity",Console::Float,"A gravidade a aplicar ao jogador",nullptr,nullptr);
	gbMainConsole->RegisterVariable(0,"p_noClip",Console::Integer,"Se o jogador pode ou não voar e reagir com o ambiente",nullptr,nullptr);

	gbMainConsole->RegisterVariable(0,"sys_infoCPUVendorID",Console::String,"The CPU vendor ID string","CPU vendor ID: %s",nullptr);
	gbMainConsole->RegisterVariable(0,"sys_infoCPUProcessorName",Console::String,"The CPU processor name string","CPU processor name: %s",nullptr);
	gbMainConsole->RegisterVariable(0,"sys_infoFPS",Console::Integer,"Average number of frames per second (updated every second)","FPS: %d",nullptr);
	gbMainConsole->RegisterVariable(0,"sys_screenshot",Console::Integer,"Número de screenshot (um por frame) a tirar",nullptr,nullptr);
	gbMainConsole->RegisterVariable(0,"sys_version",Console::String,"A versão do build do HorseRadish em uso",nullptr,nullptr);
	gbMainConsole->RegisterVariable(30,"sys_consoleAlpha",Console::Float,"The console's alpha value (to make it a little transparent)",nullptr, consoleCallbackVariables);
	gbMainConsole->RegisterVariable(31,"sys_consoleTextFont",Console::String,"The console's font file used to render the text",nullptr, consoleCallbackVariables);
	gbMainConsole->RegisterVariable(32,"sys_consoleTextSize",Console::Integer,"The console's font size used to render the text",nullptr, consoleCallbackVariables);

	gbMainConsole->RegisterVariable(0,"developer",Console::Integer,"This should be diferent than 0 if the engine is being developed (shows more errors, further test, etc)",nullptr,nullptr);

	//algums valores por defeito
	gbMainConsole->VarSetDataI("r_winSwapInterval",1);
	gbMainConsole->VarSetDataI("r_winWidth",800);
	gbMainConsole->VarSetDataI("r_winHeight",600);
	gbMainConsole->VarSetDataI("r_winFullscreen",0);
	gbMainConsole->VarSetDataF("r_winFOV",90.0f);
	gbMainConsole->VarSetDataI("r_winRefresh",0);
	gbMainConsole->VarSetDataS("r_showTexture","");
	gbMainConsole->VarSetDataI("r_useLightScissor",1);
	gbMainConsole->VarSetDataI("r_useParallax",1);
	gbMainConsole->VarSetDataI("r_useShadows",1);
	gbMainConsole->VarSetDataI("r_drawInfinites",1);
	gbMainConsole->VarSetDataI("r_drawNoLighting",1);
	gbMainConsole->VarSetDataI("r_useAmbient",1);
	gbMainConsole->VarSetDataI("r_useGlow",1);
	gbMainConsole->VarSetDataS("r_glDriver", "OpenGL32.dll");
	gbMainConsole->VarSetDataI("r_glDebug", 0);
	gbMainConsole->VarSetDataF("r_orthoSpaceScale",1.0f);
	gbMainConsole->VarSetDataI("r_filterShadows",1);
	gbMainConsole->VarSetDataI("r_drawMode",0);
	gbMainConsole->VarSetDataF("r_varAuxF1",1.0f);
	gbMainConsole->VarSetDataF("r_varAuxF2",0.1f);
	gbMainConsole->VarSetDataF("r_ppTone",1.0f);
	gbMainConsole->VarSetDataF("r_ppBrightness",0.0f);
	gbMainConsole->VarSetDataI("r_ppAO",0);
	gbMainConsole->VarSetDataF("r_hdriGlowThreshold",0.9f);
	gbMainConsole->VarSetDataS("tex_diffuseFilter","bilinear");
	gbMainConsole->VarSetDataF("tex_diffuseAnisotropic",1.0f);
	gbMainConsole->VarSetDataI("p_noClip",1);
	gbMainConsole->VarSetDataF("sys_consoleAlpha",0.8f);
	gbMainConsole->VarSetDataS("sys_consoleTextFont","C:\\WINDOWS\\Fonts\\TAHOMAbd.TTF");
	gbMainConsole->VarSetDataI("sys_consoleTextSize",9);
	gbMainConsole->VarSetDataS("sys_version", HorseRadish::String("Horseradish v1.0.0 (alpha build 1052)", HorseRadish::Build::BuildNumber).GetData());

	//gravo algumas coisas para algumas variáveis
	if (HorseRadish::Machine::CPUGetVendorID(stringAux) == true)
		gbMainConsole->VarSetDataS("sys_infoCPUVendorID", stringAux.GetData());
	if (HorseRadish::Machine::CPUGetProcessorName(stringAux) == true)
		gbMainConsole->VarSetDataS("sys_infoCPUProcessorName", stringAux.GetData());

	//algumas das variáveis são read-only
	/*gbMainConsole->VarSetAttrib("r_infoMTRIS",SCONSOLE_VAR_READONLY,0);
	gbMainConsole->VarSetAttrib("r_infoMaterials",SCONSOLE_VAR_READONLY,0);
	gbMainConsole->VarSetAttrib("r_infoTextures",SCONSOLE_VAR_READONLY,0);
	gbMainConsole->VarSetAttrib("r_infoObjectos",SCONSOLE_VAR_READONLY,0);
	gbMainConsole->VarSetAttrib("r_infoLights",SCONSOLE_VAR_READONLY,0);
	gbMainConsole->VarSetAttrib("r_infoPosition",SCONSOLE_VAR_READONLY,0);
	gbMainConsole->VarSetAttrib("r_infoGLError",SCONSOLE_VAR_READONLY,0);
	gbMainConsole->VarSetAttrib("r_infoGLDrawElements",SCONSOLE_VAR_READONLY,0);*/
	gbMainConsole->VarSetAttrib("r_glDriver", Console::ReadOnly, Console::VarFlags::None);
	gbMainConsole->VarSetAttrib("sys_consoleAlpha", Console::Clamp, Console::VarFlags::None);
	gbMainConsole->VarSetAttrib("sys_infoCPUVendorID", Console::ReadOnly, Console::VarFlags::None);
	gbMainConsole->VarSetAttrib("sys_infoCPUProcessorName", Console::ReadOnly, Console::VarFlags::None);
	gbMainConsole->VarSetAttrib("sys_version", Console::ReadOnly, (Console::VarFlags)0);
}

static
void CALLBACK callbackConsoleErrorLoopback(const char *errorInfo)
{
	/*if (errorInfo!=nullptr && errorInfo[0]!='\0')
		consolaGUI->GUISMSAdd(errorInfo,CONSOLE_SMS_TIME,1.0f,0.0f,0.0f);*/
}

static
void parseAppConfFile(const HorseRadish::IO::Path &filePath)
{
	HorseRadish::String linhaDados;

	//abro o ficheiro
	HorseRadish::Streams::FileStream fileStream(filePath, true, false);

	//crio um stream para poder mais facilmente ler cada linha individualmente
	HorseRadish::Streams::StreamReader streamReader(&fileStream);

	//enquanto não chegar ao fim do documento
	while(streamReader.CanRead() == true)
	{
		//leio a linha
		if (streamReader.ReadLineString(linhaDados, HorseRadish::String::UTF8) <= 0)
			continue;

		//se for um comentário
		if (linhaDados.StartsWith((HorseRadish::hChar*)"//") == true)
			continue;

		//mando executar o comando
		gbMainConsole->Process(linhaDados.GetData());
	}
}

static
bool systemInitialize(const HINSTANCE hInstance, const PWSTR lpCmdLine)
{
	int processPriority, totalFich, totalPacks;
	HorseRadish::String openglDriverName, systemConfOverride;

	//a primeira coisa que faço é criar a consola
	gbMainConsole = new Console(HorseRadish::IO::Path("./logs/appLog.txt"));

	//registo os comandos e variáveis na consola
	registerConsoleCommands();
	registerConsoleVariables();

	//se alguém indicou que um ficheiro deve ser executado para fazer override às configurações de sistema
	if (Window::CommandLineGetParam(lpCmdLine, (const HorseRadish::hChar*)"-sysConf", systemConfOverride) == true)
		parseAppConfFile(HorseRadish::IO::Path(systemConfOverride));

	//se alguém indicou um driver diferente para o OpenGL
	if (Window::CommandLineGetParam(lpCmdLine, (const HorseRadish::hChar*)"-glDriver", openglDriverName) == true)
		gbMainConsole->VarSetDataS("r_glDriver", openglDriverName.GetData());	

	//mando carregar a biblioteca do OpenGL
	if (HorseRadish::OpenGL::OpenGLLoadLibrary((const HorseRadish::hChar*)gbMainConsole->VarGetDataS("r_glDriver")) == false)
	{
		gbMainConsole->LogError("Error linking OpenGL driver!!");
		Window::MsgBoxErro("Unable to load OpenGL driver!\nApplication cannot proceed.");
		return false;
	}

	//posso criar a janela da aplicação
	gbWindow = new Window(hInstance);

	//escrevo informação acerca do sistema
	writeSystemInfo();
	gbMainConsole->Log(" ");

	//pequenas coisas que tem de ser activadas e preparadas
	srand((unsigned)time(nullptr));
	gbTotalTimer.ReStart();

	//se é preciso mudar a prioridade do processo
	if (Window::CommandLineGetParam(lpCmdLine, (const HorseRadish::hChar*)"-priority", processPriority) == true)
	{
		//só aceito estes dois valores
		if ((processPriority == 1) || (processPriority == 2))
			HorseRadish::Platform::SetProcessPriority( (processPriority == 1) ? HorseRadish::Platform::High : HorseRadish::Platform::Highest );
	}

	//verifico se foi pedido para ficar em modo de editor
	/*if ((cmdParams.editorWinHandle != 0) && (cmdParams.editorWinW != 0) && (cmdParams.editorWinH != 0))
	{
		//crio espaço para o editor
		gbEditorParam = new EDITOR_PARAMETERS;

		//limpo as coisas lá dentro e devo gravar isto
		memset(gbEditorParam, 0, sizeof(EDITOR_PARAMETERS));
		gbEditorParam->editorWinH = cmdParams.editorWinW;
		gbEditorParam->editorWinH = cmdParams.editorWinH;
		gbEditorParam->editorWinHandle = (HWND)cmdParams.editorWinHandle;
	}*/

	//ler o ficheiro de configuração por omissão da aplicação
	parseAppConfFile(HorseRadish::IO::Path("hr_app.cfg"));

	//para controlar o acesso a ficheiros
	gbFileSystem = new HorseRadish::IO::FileSystem(10);
	if (gbFileSystem == nullptr)
		return false;

	//monto a directoria actual e a anterior
	auto currentFolder = HorseRadish::IO::Path(HorseRadish::IO::Path::CurrentFolder);
	gbFileSystem->MountPath(currentFolder, nullptr);
	currentFolder.RemoveLastComponent();
	gbFileSystem->MountPath(currentFolder, nullptr);

	//temporariamente, leio as coisas daqui
	gbFileSystem->MountPath(HorseRadish::IO::Path("d:/jogos/doom3/base/"), nullptr);
	gbMainConsole->LogInfo(HorseRadish::String("#150,150,0.->#255,255,255.Path set to: \"%s\"", "d:/jogos/doom3/base/").GetData());
	
	//inicio isto a zero
	totalFich = totalPacks = 0;

	//tenho de pesquisar por todos os ficheiros
	HorseRadish::IO::FileSystem::FindFiles((HorseRadish::hChar*)"d:/jogos/doom3/base/pak*.pk4", true, [&](const HorseRadish::IO::Path &filePath, const HorseRadish::hUInt64 &fileSize)
	{
		int numFilesZip;

		//carrego o ZIP
		if (gbFileSystem->MountZip(filePath, nullptr, &numFilesZip) == true)
		{
			//mais um pak e acumulo o número de ficheiros
			totalPacks += 1;
			totalFich += numFilesZip;
		}
	});

	//aviso o que fiz
	gbMainConsole->LogTab(HorseRadish::String("loaded %d archives with a total of %d files", totalPacks, totalFich).GetData(), 2);
		
	//correu tudo bem
	return true;
}

static
void systemShutdown()
{
	//se tiver um sistema de ficheiros
	delete gbFileSystem;
	gbFileSystem = nullptr;

	//fechar o OpenGL
	HorseRadish::OpenGL::OpenGLUnloadLibrary();

	//apago a consola
	delete gbMainConsole;
	gbMainConsole = nullptr;
}

static
void systemExit(const int exitCause)
{
	//fazer restart
	if (exitCause == HR_EXIT_RESTART)
	{
		gbAppExitReason = HR_EXIT_RESTART;
		gbWindow->SendMessageClose();
		return;
	}

	//fazer restart só ao video
	if (exitCause == HR_EXIT_RESTART)
	{
		gbAppExitReason = HR_EXIT_RESTART_VID;
		gbWindow->SendMessageClose();
		return;
	}

	//fazer um quit normal
	if (exitCause == HR_EXIT_QUIT)
	{
		gbAppExitReason = HR_EXIT_QUIT;
		gbWindow->SendMessageClose();
		return;
	}

	//tenho de sair e iniciar o editor
	if (exitCause == HR_EXIT_RESTART_EDITOR)
	{
		gbAppExitReason = HR_EXIT_RESTART_EDITOR;
		gbWindow->SendMessageClose();
		return;
	}
}

static
void writeSystemInfo()
{
	HorseRadish::String auxInfo;
	int memTotal, memFree, displayWidth, displayHeight, displayColorBits, displayFrequency;

	//informação geral
	gbMainConsole->Log("#150,150,0.->#255,255,255.System information:");
	
		if (HorseRadish::Machine::CPUGetVendorID(auxInfo) == true)
			gbMainConsole->LogTab(HorseRadish::String("CPU vendor ID: %s.", auxInfo.GetData()).GetData(), 2);
		if (HorseRadish::Machine::CPUGetProcessorName(auxInfo) == true)
			gbMainConsole->LogTab(HorseRadish::String("CPU processor name: %s.", auxInfo.GetData()).GetData(), 2);

		HorseRadish::Platform::GetSystemInfo(HorseRadish::Platform::MemoryTotal, memTotal);
		auxInfo.SetMemory(memTotal);
		gbMainConsole->LogTab(HorseRadish::String("Total physical memory: %s", auxInfo.GetData()).GetData(), 2);

		HorseRadish::Platform::GetSystemInfo(HorseRadish::Platform::MemoryFree, memFree);
		auxInfo.SetMemory(memFree);
		gbMainConsole->LogTab(HorseRadish::String("Free physical memory: %s", auxInfo.GetData()).GetData(), 2);

		HorseRadish::Platform::GetSystemInfo(HorseRadish::Platform::DisplayWidth, displayWidth);
		HorseRadish::Platform::GetSystemInfo(HorseRadish::Platform::DisplayHeight, displayHeight);
		HorseRadish::Platform::GetSystemInfo(HorseRadish::Platform::DisplayColorBits, displayColorBits);
		HorseRadish::Platform::GetSystemInfo(HorseRadish::Platform::DisplayFrequency, displayFrequency);
		gbMainConsole->LogTab(HorseRadish::String("Desktop resolution: %dx%dx%d@%d", displayWidth, displayHeight, displayColorBits, displayFrequency).GetData(), 2);

		HorseRadish::Platform::GetSystemInfo(HorseRadish::Platform::OperatingSystemName, auxInfo);
		gbMainConsole->LogTab(HorseRadish::String("Operating system: %s", auxInfo.GetData()).GetData(), 2);

		gbMainConsole->LogTab(HorseRadish::Platform::IsArch64() ? "Build type: x86 64bit" : "Build type: x86 32bit", 2);

		HorseRadish::Platform::GetSystemInfo(HorseRadish::Platform::MachineName, auxInfo);
		gbMainConsole->LogTab(HorseRadish::String("Machine name: %s", auxInfo.GetData()).GetData(), 2);

		HorseRadish::Platform::GetSystemInfo(HorseRadish::Platform::CurrentUsername, auxInfo);
		gbMainConsole->LogTab(HorseRadish::String("User name: %s", auxInfo.GetData()).GetData(), 2);

	//para testar UTF8
	gbMainConsole->Log("#150,150,0.->#255,255,255.UTF8 text test:");
		gbMainConsole->LogTab("Hello!", 2);
		gbMainConsole->LogTab("Olá!", 2);
		gbMainConsole->LogTab("Grüß Gott", 2);
		gbMainConsole->LogTab("Здравствуйте", 2);
		gbMainConsole->LogTab("Γειά σου", 2);
		gbMainConsole->LogTab("مرحبا", 2);
}

static
void writeOpenGLInfo(const HorseRadish::OpenGL::Objects::Context &glContext)
{
	HorseRadish::String infoValueString;
	int infoValueInt;

	//info acerca da drive
	gbMainConsole->Log("#150,150,0.->#255,255,255.OpenGL driver info:");
		glContext.GetInformation(HorseRadish::OpenGL::Objects::Context::Version, infoValueString);
		gbMainConsole->LogTab(HorseRadish::String("OpenGL version: %s.", infoValueString.GetData()).GetData(), 2);
		glContext.GetInformation(HorseRadish::OpenGL::Objects::Context::Vendor, infoValueString);
		gbMainConsole->LogTab(HorseRadish::String("OpenGL vendor: %s.", infoValueString.GetData()).GetData(), 2);
		glContext.GetInformation(HorseRadish::OpenGL::Objects::Context::Renderer, infoValueString);
		gbMainConsole->LogTab(HorseRadish::String("OpenGL renderer: %s.", infoValueString.GetData()).GetData(), 2);

	//info acerca das extensões
	gbMainConsole->Log("#150,150,0.->#255,255,255.OpenGL extensions available:");
	HorseRadish::OpenGL::glGetIntegerv(GL_NUM_EXTENSIONS, &infoValueInt);
	for(int curExt = 0; curExt < infoValueInt; curExt++)
		gbMainConsole->LogTab((const char*)HorseRadish::OpenGL::glGetStringi(GL_EXTENSIONS, curExt), 2);

	//info de outras coisas
	gbMainConsole->Log("#150,150,0.->#255,255,255.OpenGL extended information:");
	
		glContext.GetInformation(HorseRadish::OpenGL::Objects::Context::GLSLVersion, infoValueString);
		gbMainConsole->LogTab(HorseRadish::String("GLSL version: %s", infoValueString.GetData()).GetData(), 2);

		glContext.GetInformation(HorseRadish::OpenGL::Objects::Context::MaxDrawBuffers, infoValueInt);
		gbMainConsole->LogTab(HorseRadish::String("Maximum number of draw buffers: %d", infoValueInt).GetData(), 2);

		glContext.GetInformation(HorseRadish::OpenGL::Objects::Context::MaxColorAttachments, infoValueInt);
		gbMainConsole->LogTab(HorseRadish::String("Maximum number of color attachments in FBOs: %d", infoValueInt).GetData(), 2);

		glContext.GetInformation(HorseRadish::OpenGL::Objects::Context::MaxTextureSize, infoValueInt);
		gbMainConsole->LogTab(HorseRadish::String("Maximum 1D/2D texture size: %dx%d", infoValueInt, infoValueInt).GetData(), 2);
		glContext.GetInformation(HorseRadish::OpenGL::Objects::Context::MaxTexture3DSize, infoValueInt);
		gbMainConsole->LogTab(HorseRadish::String("Maximum 3D texture size: %dx%dx%d", infoValueInt, infoValueInt, infoValueInt).GetData(), 2);
		glContext.GetInformation(HorseRadish::OpenGL::Objects::Context::MaxTextureCubemapSize, infoValueInt);
		gbMainConsole->LogTab(HorseRadish::String("Maximum cubemap texture size: %dx%d", infoValueInt, infoValueInt).GetData(), 2);
		glContext.GetInformation(HorseRadish::OpenGL::Objects::Context::MaxTextureRectSize, infoValueInt);
		gbMainConsole->LogTab(HorseRadish::String("Maximum rectangle texture size: %dx%d", infoValueInt, infoValueInt).GetData(), 2);
}

void CALLBACK openglDebugMessagesCallback(HorseRadish::OpenGL::GLenum source, HorseRadish::OpenGL::GLenum type, HorseRadish::OpenGL::GLuint id, HorseRadish::OpenGL::GLenum severity, HorseRadish::OpenGL::GLsizei length, const HorseRadish::OpenGL::GLchar* message, HorseRadish::OpenGL::GLvoid* userParam)
{
	Console *appConsole;
	const char *glSource, *glType, *glSeverity;

	//isto dá jeito
	appConsole = (Console*)userParam;

	//por omissão
	glSource = glType = glSeverity = "";

	//escolho o source correcto
	switch(source)
	{
		case GL_DEBUG_SOURCE_API_ARB:
			glSource = "source(api)";
			break;
		case GL_DEBUG_SOURCE_SHADER_COMPILER_ARB:
			glSource = "source(glsl)";
			break;
		case GL_DEBUG_SOURCE_WINDOW_SYSTEM_ARB:
			glSource = "source(window system)";
			break;
		case GL_DEBUG_SOURCE_THIRD_PARTY_ARB:
			glSource = "source(third party)";
			break;
		case GL_DEBUG_SOURCE_APPLICATION_ARB:
			glSource = "source(application)";
			break;
		case GL_DEBUG_SOURCE_OTHER_ARB:
			glSource = "source(other)";
			break;
	}

	//escolho o tipo correcto
	switch(type)
	{
		case GL_DEBUG_TYPE_ERROR_ARB:
			glType = "type(error)";
			break;
		case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR_ARB:
			glType = "type(deprecated)";
			break;
		case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR_ARB:
			glType = "type(undefined)";
			break;
		case GL_DEBUG_TYPE_PERFORMANCE_ARB:
			glType = "type(performance)";
			break;
		case GL_DEBUG_TYPE_PORTABILITY_ARB:
			glType = "type(portability)";
			break;
		case GL_DEBUG_TYPE_OTHER_ARB:
			glType = "type(other)";
			break;
	}

	//escolho o grau correcto
	switch(severity)
	{
		case GL_DEBUG_SEVERITY_HIGH_ARB:
			glSeverity = "severity(high)";
			break;
		case GL_DEBUG_SEVERITY_MEDIUM_ARB:
			glSeverity = "severity(medium)";
			break;
		case GL_DEBUG_SEVERITY_LOW_ARB:
			glSeverity = "severity(low)";
			break;
	}

	//faço log do que preciso
	appConsole->LogError(HorseRadish::String("OpenGL [%s - %s - %s]:", glSource, glType, glSeverity).GetData());
	appConsole->LogError(HorseRadish::String("     %s", message).GetData());
}

static
bool openglInitialize(const HorseRadish::OpenGL::Objects::Context &glContext)
{
	int glMajorVersion, glMinorVersion;

	//inicio todas as extensões necessárias de base (do OpenGL 3.2)
	if (HorseRadish::OpenGL::OpenGLGetProcs() == false)
	{
		gbMainConsole->LogError("Unable to load required functions to work with OpenGL 3.2 (try updating your drivers).");
		gbWindow->WindowKill();
		return false;
	}

	//inicio extensões auxiliares
	HorseRadish::OpenGL::Extensions::ExtensionsLoad((const HorseRadish::hChar*)gbMainConsole->VarGetDataS("r_glDriver"));

	//certifico-me que estou com OpenGL 3
	HorseRadish::OpenGL::glGetIntegerv(GL_MAJOR_VERSION, &glMajorVersion);
	HorseRadish::OpenGL::glGetIntegerv(GL_MINOR_VERSION, &glMinorVersion);
	if ((glMajorVersion < 3) || ((glMajorVersion == 3) && (glMinorVersion < 2)))
	{
		gbMainConsole->LogError("OpenGL version 3.2 or higher is required (try updating your drivers).");
		gbWindow->WindowKill();
		return false;
	}

	//ponho o GL como eu quero e inicio extensoes
	HorseRadish::OpenGL::glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
	HorseRadish::OpenGL::glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	HorseRadish::OpenGL::glDepthFunc(GL_LEQUAL);
	HorseRadish::OpenGL::glDepthMask(GL_TRUE);
	HorseRadish::OpenGL::glClearDepth(1.0f);
	HorseRadish::OpenGL::glClearStencil(0);
	HorseRadish::OpenGL::glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	HorseRadish::OpenGL::glPolygonOffset(1.0f, 1.0f);
	HorseRadish::OpenGL::glDisable(GL_DEPTH_TEST);
	HorseRadish::OpenGL::glDisable(GL_BLEND);
	HorseRadish::OpenGL::glDisable(GL_SCISSOR_TEST);
	HorseRadish::OpenGL::glDisable(GL_STENCIL_TEST);
	HorseRadish::OpenGL::glEnable(GL_CULL_FACE);
	HorseRadish::OpenGL::glHint(GL_LINE_SMOOTH_HINT,GL_NICEST);
	HorseRadish::OpenGL::glCullFace(GL_BACK);
	HorseRadish::OpenGL::glFrontFace(GL_CCW);

	//limpo os buffers todos
	HorseRadish::OpenGL::glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

	//correu tudo bem
	return true;
}

static
bool windowInitialize()
{
	//se estiver em modo de edicao, tenho
	if (gbEditorParam != nullptr)
	{
		int winWidth,winHeight;
		HorseRadish::String baseName;

		//coloco o tamanho da janela nas variáveis indicadas
		gbMainConsole->VarSetDataI("r_winWidth", gbEditorParam->editorWinW);
		gbMainConsole->VarSetDataI("r_winHeight", gbEditorParam->editorWinH);
		
		//inicio opcoes
		winWidth = gbMainConsole->VarGetDataI("r_winWidth");
		winHeight = gbMainConsole->VarGetDataI("r_winHeight");

		//inicio janela
		/*if (gbWindow->WindowEditorInit(windowsMessages,"Horseradish engine v1.0",winWidth,winHeight,gbEditorParam->editorWinHandle,glDriver)==false)
		{
			Window::MsgBoxErro("Unable to create rendering context (editor)!\nApplication cannot proceed.");
			return false;
		}
		gbMainConsole->Log("#150,150,0.->#255,255,255.Windows system (editor) initialized.");*/

		//qual o nome base a usar para criar a string
		baseName.SetPrintf(HorseRadish::String::UTF8, "horseradish_%d", GetCurrentProcessId());

		//TODO: isto não deveria estr em ANSI
		//registo uma mensagem para falar com o editor
		/*gbEditorParam->msgID = RegisterWindowMessageA((baseName + HorseRadish::String("_msg")).GetData());

		//TODO: isto não deveria estr em ANSI
		//crio o handle para o file mapping
		gbEditorParam->hMapObject = CreateFileMappingA(INVALID_HANDLE_VALUE, nullptr, PAGE_READWRITE, 0, 4096, (baseName + HorseRadish::String("_fm")).GetData());
		if (gbEditorParam->hMapObject == nullptr)
			return false;

		//tiro agora o ponteiro para o buffer criado
		gbEditorParam->mappedBuffer = MapViewOfFile(gbEditorParam->hMapObject, FILE_MAP_WRITE, 0, 0, 0);
		if (gbEditorParam->mappedBuffer == nullptr) 
			return false;

		//limpo o buffer
		gbEditorParam->mappedBufferChar = (char*)gbEditorParam->mappedBuffer;
		memset(gbEditorParam->mappedBuffer, 0, 4096);

		//TODO: isto não deveria estr em ANSI
		//e finalmente o mutex e os eventos para controlar o acesso à memória partilhada
		gbEditorParam->hMutex = CreateMutexA(nullptr, false, (baseName + HorseRadish::String("_mutex")).GetData());
		if (gbEditorParam->hMutex == nullptr)
			return false;
		gbEditorParam->hEventEditorWrote = CreateEventA(nullptr, TRUE, FALSE, (baseName + HorseRadish::String("_eventE")).GetData());
		if (gbEditorParam->hEventEditorWrote == nullptr)
			return false;
		gbEditorParam->hEventAppWrote = CreateEventA(nullptr, TRUE, FALSE, (baseName + HorseRadish::String("_eventA")).GetData());
		if (gbEditorParam->hEventAppWrote == nullptr)
			return false;*/
	}
	else
	{
		int winWidth,winHeight;
		bool winFullscreen;

		//inicio opcoes de acordo com o utilizador
		winWidth = gbMainConsole->VarGetDataI("r_winWidth");
		winHeight = gbMainConsole->VarGetDataI("r_winHeight");
		winFullscreen = (gbMainConsole->VarGetDataI("r_winFullscreen")) ? true:false;

		//inicio janela
		if (gbWindow->WindowInit(windowsMessages, (const HorseRadish::hChar*)"Horseradish engine v1.0", winWidth, winHeight, winFullscreen)==false)
		{
			Window::MsgBoxErro("Unable to create rendering window!\nApplication cannot proceed.");
			return false;
		}

		//aviso
		gbMainConsole->Log("#150,150,0.->#255,255,255.Windows system initialized.");
	}

	//para ler as posições do rato e teclas (faz as vezes do DInput)
	gbRawInput = new RawInput(gbWindow);
	gbMainConsole->Log("#150,150,0.->#255,255,255.Direct Input initialized.");

	//**************
	//OpenAL
	if (gbMainConsole->VarGetDataI("snd_quality")!=0)
	{
		/*if (SAudio::SAudioInit("OpenAL32.dll")==false)
			gbMainConsole->LogError("Falhou criar instância de DirectAudio.");
		else
			gbMainConsole->Log("#150,150,0.->#255,255,255.Direct Audio initialized.");*/
	}

	//vou por um pouco de movimento
	gbMainConsole->VarSetDataF("sys_mouseSensitivity", 50.0f);
	gbMainConsole->VarSetDataF("sys_keyboardSensitivity", 50.0f);

	//posso começar a mostrar erros que vão sendo logados
	gbMainConsole->LogLoopback(callbackConsoleErrorLoopback);

	//correu tudo bem
	return true;
}

static
void windowShutdown()
{
	//limpo isto tudo
	//SAudio::SAudioClose();

	//apagar algumas coisas
	delete gbRawInput;
	gbRawInput = nullptr;

	//basta fechar a janela
	gbWindow->WindowKill();

	//se estou no editor tenho de fechar algumas coisas
	if (gbEditorParam != nullptr)
	{
		//espero pelo mutex
		WaitForSingleObject(gbEditorParam->hMutex, INFINITE);

		//fecho os handles todos que for preciso
		if (gbEditorParam->mappedBuffer != nullptr)
			UnmapViewOfFile(gbEditorParam->mappedBuffer); 
		if (gbEditorParam->hMapObject != nullptr)
			CloseHandle(gbEditorParam->hMapObject);
		if (gbEditorParam->hEventAppWrote != nullptr)
			CloseHandle(gbEditorParam->hEventAppWrote);
		if (gbEditorParam->hEventEditorWrote != nullptr)
			CloseHandle(gbEditorParam->hEventEditorWrote);
		if (gbEditorParam->hMutex != nullptr)
			CloseHandle(gbEditorParam->hMutex);
		
		//limpo tudo agora
		delete gbEditorParam;
		gbEditorParam = nullptr;
	}

	//não esqueçer a janela
	delete gbWindow;
	gbWindow = nullptr;
}

static
void showInitialCredits(HorseRadish::Render::Renderer2D * const renderData, OpenglContext * const glContext, HorseRadish::OpenGL::Tools::Viewport * const renderViewport)
{
	VideoStream *videoStream;
	HorseRadish::OpenGL::Tools::ImmediateMode *glImmediateMode;
	HorseRadish::OpenGL::Objects::ObjectsManager *glObjects;
	const HorseRadish::OpenGL::Objects::Texture *texVideo;
	const HorseRadish::OpenGL::Objects::PixelBuffer *pboVideo;
	float videoColor;
	int videoWidth, videoHeight;
	HorseRadish::hInt64 frameLastID;
	HorseRadish::Primitives2D::Rectangle<int> videoRect;
	bool videoExiting, texUpdated;
	HorseRadish::Timer timerSaida;

	//preciso de inicializar a componente do video
	VideoStream::Initialize();

	//preciso de ficar em projecção 2D
	renderViewport->setProjection(HorseRadish::OpenGL::Tools::Viewport::Proj2D);
	renderViewport->updateGL();

	//preparo algumas coisas
	HorseRadish::OpenGL::glBindFramebuffer(GL_FRAMEBUFFER, 0);
	HorseRadish::OpenGL::glDisable(GL_BLEND);
	HorseRadish::OpenGL::glDisable(GL_DEPTH_TEST);
	HorseRadish::OpenGL::glDepthMask(GL_FALSE);
	HorseRadish::OpenGL::glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
	HorseRadish::OpenGL::glClearColor(0.0f, 0.0f, 0.0f, 1.0f);

	//mostro tudo a preto
	HorseRadish::OpenGL::glClear(GL_COLOR_BUFFER_BIT);
	glContext->SwapBuffers();

	//abro o ficheiro com o video
	videoStream = new VideoStream(3, PixelFormat::PIX_FMT_BGR24, "..\\media\\icredits.mkv");
	if ((videoStream == nullptr) || (videoStream->IsValid() == false))
	{
		//ele pode existir (ou seja, está inválido)
		if (videoStream != nullptr)
			delete videoStream;

		//posso sair
		videoStream = nullptr;
		return;
	}

	//preciso das dimensões do video
	videoStream->GetVideoDims(videoWidth, videoHeight);

	//para ajudar
	glImmediateMode = renderData->glImmediateMode;
	glObjects = new HorseRadish::OpenGL::Objects::ObjectsManager(glContext);

	//crio a textura que vai mostrar o video
	texVideo = glObjects->CreateRect(true, nullptr, videoWidth, videoHeight, HorseRadish::OpenGL::Objects::ObjectsManager::RGBA32);
	texVideo->Bind(0);
	
	//o PBO que vou usar para transferir as coisas para a textura criada
	pboVideo = (HorseRadish::OpenGL::Objects::PixelBuffer*)glObjects->ObjectCreate(HorseRadish::OpenGL::Objects::ObjectsManager::PixelUnpackBuffer);
	if (pboVideo != nullptr)
	{
		//crio espaço no PBO
		pboVideo->Bind();
		pboVideo->LoadBuffer(nullptr, videoStream->GetVideoFrameDataSize(), HorseRadish::OpenGL::Objects::PixelBuffer::Stream);
	}

	//posso usar o programa do texto para mostrar a textura
	HorseRadish::OpenGL::glUseProgram(renderData->shaders.prog2DText->glID);
	HorseRadish::OpenGL::glUniform1i(HorseRadish::OpenGL::glGetUniformLocation(renderData->shaders.prog2DText->glID, "texTextSampler"), 0);
	HorseRadish::OpenGL::glUniformMatrix4fv(HorseRadish::OpenGL::glGetUniformLocation(renderData->shaders.prog2DText->glID, "transformationMatrix"), 1, GL_FALSE, renderViewport->getProj2D());

	//preciso de calcular o rectangulo onde devo mostrar o video
	videoStream->GetVideoRect(renderViewport->getWidth(), renderViewport->getHeight(), true, videoRect);

	//por omissão
	frameLastID = -1;
	texUpdated = false;
	videoExiting = false;
	videoColor = 1.0f;

	//ciclo infinito da thread
	while(true)
	{
		bool frameIsAhead;
		HorseRadish::hInt64 frameID;
		double frameDurationS;
		const void* frameData;

		//limpo qualquer coisa antes
		HorseRadish::OpenGL::glClear(GL_COLOR_BUFFER_BIT);

		//se já estou de saída
		if (videoExiting == true)
		{
			//se já acabei
			if (videoColor <= 0.0f)
				break;

			//ajusto a cor
			videoColor = 1.0f - timerSaida.GetTimeS();
		}

		//se for para sair
		if (WaitForSingleObject(gbThreadRenderEventExit, 0) == WAIT_OBJECT_0)
			break;

		//tiro o ponteiro para o buffer da frame (se já não existe, ou o relógio se atrasou ou cheguei ao fim do video)
		frameData = videoStream->GetFrame(frameIsAhead, frameID, frameDurationS);
		if ((frameData == nullptr) && (frameIsAhead == false))
			break;

		//se a frame devolvida é outra
		if ((frameData != nullptr) && (frameID != frameLastID))
		{
			//actualizo a textura com os novos dados (verifico se estou ou não a usar PBO)
			if (pboVideo != nullptr)
			{
				//actualizo o PBO
				pboVideo->UpdateBuffer(frameData, videoStream->GetVideoFrameDataSize(), 0);

				//actualizo a textura com os dados do PBO
				HorseRadish::OpenGL::glTexSubImage2D(GL_TEXTURE_RECTANGLE, 0, 0, 0, videoWidth, videoHeight, GL_BGR, GL_UNSIGNED_BYTE, (void*)0);
			}
			else
			{
				//coloco os dados directamente na textura
				HorseRadish::OpenGL::glTexSubImage2D(GL_TEXTURE_RECTANGLE, 0, 0, 0, videoWidth, videoHeight, GL_BGR, GL_UNSIGNED_BYTE, frameData);
			}

			//já a posso usar
			texUpdated = true;

			//guardo o novo ID
			frameLastID = frameID;
		}

		//se já posso usar a textura
		if (texUpdated == true)
		{
			//desenho o quadrado
			glImmediateMode->BeginDraw(HorseRadish::OpenGL::Tools::ImmediateMode::Quads);
				glImmediateMode->AddColorF((videoColor < 0.0f) ? 0.0f : videoColor);

				glImmediateMode->AddTexCoord(0.0f, videoHeight);
				glImmediateMode->AddPosition(videoRect.x, videoRect.y);

				glImmediateMode->AddTexCoord(videoWidth, videoHeight);
				glImmediateMode->AddPosition(videoRect.x + videoRect.width, videoRect.y);

				glImmediateMode->AddTexCoord(videoWidth, 0.0f);
				glImmediateMode->AddPosition(videoRect.x + videoRect.width, videoRect.y + videoRect.height);

				glImmediateMode->AddTexCoord(0.0f, 0.0f);
				glImmediateMode->AddPosition(videoRect.x, videoRect.y + videoRect.height);
			glImmediateMode->EndDraw();
		}

		//espero pelo mutex (variável numMsgListaMain), mando a mensagem para o GUI da consola e liberto o mutex
		if (gbThreadParams->numMsgListaMain > 0)
		{
			//tranco a consola e passo por todas as mensagens e actualizo a consola e o overlay
			for(unsigned int curMsgIndex=0; curMsgIndex<gbThreadParams->numMsgListaMain; curMsgIndex++)
			{
				//mando processar as teclas
				if (gbThreadParams->listaMain[curMsgIndex].message == WM_KEYUP)
				{
					//digo que quero começar a sair do vídeo
					timerSaida.ReStart();
					videoExiting = true;
				}
			}

			//já não tenho nada na lista
			gbThreadParams->numMsgListaMain = 0;
		}

		//mando processar a próxima frame do video (não é necessário, mas dá sempre jeito fazê-lo aqui)
		videoStream->Process();

		//mostro o que desenhei
		glContext->SwapBuffers();

		//só faz sentido recalcular a duração da frame se ela já existir
		frameDurationS = 0.0;
		if (frameIsAhead == false)
		{
			//ao chamar videoStream->Process, posso tirar a duração da frame com mais precisão
			frameDurationS = videoStream->GetFrameDuration(frameID);
			if (frameDurationS < 0.01)
				frameDurationS = 0.0;
		}

		//posso esperar o tempo que esta frame demora a ser mostrada
		Sleep(HorseRadish::Math::ftoi(frameDurationS * 1000.0));
	}

	//limpar o último conteúdo
	HorseRadish::OpenGL::glClear(GL_COLOR_BUFFER_BIT);
	glContext->SwapBuffers();

	//se tenho um PBO
	if (pboVideo != nullptr)
		pboVideo->Unbind();

	//apago tudo o que tiver de apagar
	delete glObjects;
	delete videoStream;
	glObjects = nullptr;
	videoStream = nullptr;
}

static
void renderThread(void *threadData)
{
	float timeSpentDrawing, timeSpentProcessing, timeSpentIdle;
	HorseRadish::Timer timerSecond, timerFrame, timerTotal;
	HorseRadish::Render::RendererDeferred *rendererDeferred;
	HorseRadish::Render::Renderer2D *renderer2D;
	HorseRadish::Render::World *renderData;
	HorseRadish::OpenGL::Objects::ObjectsManager *texManagerGUI, *texManagerRender, *texManagerMain;
	HorseRadish::OpenGL::Tools::Viewport *viewport;
	HorseRadish::OpenGL::Tools::Camera *camera;
	UI::ConsoleGUI *consolaGUI;
	HorseRadish::OpenGL::Objects::ObjectsManager *glObjectManager;
	OpenglContext *glContext;

	//aviso que a thread de render foi inicializada
	gbMainConsole->Log(" ");
	gbMainConsole->Log("#150,150,0.->#255,255,255.Render thread initialized.");

	//crio o contexto
	glContext = new OpenglContext(gbWindow, (const HorseRadish::hChar*)gbMainConsole->VarGetDataS("r_glDriver"), 3, 3, (gbMainConsole->VarGetDataI("r_glDebug") != 0), false);
	if (glContext == nullptr)
	{
		gbAppExitReason = HR_EXIT_RENDER_INIT_ERROR;
		SetEvent(gbThreadRenderEventReady);
		return;
	}

	//aviso que o OpenGL está pronto
	gbMainConsole->Log("#150,150,0.->#255,255,255.OpenGL system initialized.");

	//e que tal carregar o GL propriamente dito
	if (openglInitialize(*glContext) == false)
	{
		gbAppExitReason = HR_EXIT_RENDER_INIT_ERROR;
		SetEvent(gbThreadRenderEventReady);
		return;
	}

	//inicio o contexto
	glContext->InitializeContext();

	//preciso destas extensões
	if (glContext->IsExtensionPresent((OpenglContext::Extensions)(OpenglContext::TextureStorage | OpenglContext::MapBufferAlignment | OpenglContext::ShadingLanguage420Pack | OpenglContext::DirectStateAccess)) == false)
	{
		Window::MsgBoxErro("The following OpenGL extensions are required:\n   - GL_ARB_texture_storage\n   - GL_ARB_map_buffer_alignmentn\n   - GL_ARB_shading_language_420pack\n   - GL_EXT_direct_state_access\nApplication cannot proceed.");
		gbAppExitReason = HR_EXIT_RENDER_INIT_ERROR;
		SetEvent(gbThreadRenderEventReady);
		return;
	}

	//se estou em modo de debug
	if ((gbMainConsole->VarGetDataI("r_glDebug") != 0) && (glContext->IsExtensionPresent(OpenglContext::DebugOutput) == true))
	{
		//quero receber todo o tipo de mensagens e digo para onde devo recebé-las
		HorseRadish::OpenGL::Extensions::glDebugMessageControlARB(GL_DONT_CARE, GL_DONT_CARE, GL_DONT_CARE, 0, nullptr, true);
		HorseRadish::OpenGL::Extensions::glDebugMessageCallbackARB(openglDebugMessagesCallback, gbMainConsole);

		//como já posso ter mensagens pendentes, mando-as para o callback
		glContext->DispatchDebugMessages();
	}

	//mostro a primeira/segunda frame
	glContext->SwapBuffers();
	glContext->SwapBuffers();

	//inicio o gestor de objectos GL
	glObjectManager = new HorseRadish::OpenGL::Objects::ObjectsManager(glContext);

	//preciso destes gestores de texturas
	texManagerGUI = new HorseRadish::OpenGL::Objects::ObjectsManager(glContext);
	texManagerRender = new HorseRadish::OpenGL::Objects::ObjectsManager(glContext);
	texManagerMain = new HorseRadish::OpenGL::Objects::ObjectsManager(glContext);

	//inicio agora o renderer (fbos, vbos, shaders, etc)
	renderData = new HorseRadish::Render::World();
	
	renderer2D = new HorseRadish::Render::Renderer2D(glContext);
	renderer2D->Initialize(gbMainConsole->VarGetDataI("r_winWidth"), gbMainConsole->VarGetDataI("r_winHeight"), gbFileSystem, gbMainConsole);

	rendererDeferred = new HorseRadish::Render::RendererDeferred(glContext, renderData);
	rendererDeferred->Initialize(gbMainConsole->VarGetDataI("r_winWidth"), gbMainConsole->VarGetDataI("r_winHeight"), gbFileSystem);

	//escrevo alguma informação acerda do GL
	writeOpenGLInfo(*glContext);

	//a camera
	camera = new HorseRadish::OpenGL::Tools::Camera();
	
	//coloco alguns valores por defeito na camera
	camera->SetPos(0.0f,0.0f,1.0f);
	camera->SetTarget(0.0f,0.0f,0.0f);
	camera->SetSensitivity(HorseRadish::OpenGL::Tools::Camera::Keyboard, 10.0f);

	//o viewport
	viewport = new HorseRadish::OpenGL::Tools::Viewport(gbMainConsole->VarGetDataI("r_winWidth"), gbMainConsole->VarGetDataI("r_winHeight"));

	//coisas do viewport
	viewport->setFOV(90.0f);
	viewport->setZRange(1.0f, 500.0f);
	viewport->setProjection(HorseRadish::OpenGL::Tools::Viewport::Proj3D);
	viewport->updateGL();

	//colocar o r_winSwapInterval como estava e abre finalmente a consola
	glContext->SetSwapInterval(gbMainConsole->VarGetDataI("r_winSwapInterval"));

	//inicio o UI da consola (e abro-a se for developer)
	consolaGUI = new UI::ConsoleGUI(gbMainConsole, renderer2D, texManagerGUI);
	if (gbMainConsole->VarGetDataI("developer") != 0)
		consolaGUI->ConsoleVisible(true);
	//SOverlayInit();

	//crio as tabs necessárias para o UI da consola
	auto consoleUIMain = new UI::ConsoleTabConsole(gbMainConsole);
	auto consoleUIStats = new UI::ConsoleTabStats();
	auto consoleUIDeferred = new UI::ConsoleTabExtra(rendererDeferred);
	consolaGUI->ConsoleAddTab(consoleUIMain);
	consolaGUI->ConsoleAddTab(consoleUIStats);
	consolaGUI->ConsoleAddTab(consoleUIDeferred);

	//preparo o texto da consola propriamente dita
	consoleUIMain->CriaTextoConsola(renderer2D);
	consoleUIMain->ActualizaTextoConsola();
	
	//se estou em modo de edição
	if (gbEditorParam != nullptr)
	{
		//tenho de dizer ao meu "pai" que estou pronto
		PostMessage(gbEditorParam->editorWinHandle, gbEditorParam->msgID, 1, 0);
		gbMainConsole->LogInfo("Informing editor that application is ready...");

		//e coloco como SMS pq vou já fechar a consola
		consolaGUI->SMSAdd("Informing editor that application is ready...", UI::ConsoleGUI::SMSTime, 0.0f, 1.0f, 0.0f);
		consolaGUI->ConsoleVisible(false);
	}

	//posso avisar que estou pronto
	SetEvent(gbThreadRenderEventReady);

	//se sou developer preciso de fazer algumas coisas
	if (gbMainConsole->VarGetDataI("developer") != 0)
	{
		//adiciono estas variáveis para serem mostradas no canto inferior esquerdo e poder ver SMSs
		consolaGUI->InfoAddVar("sys_infoFPS");
		consolaGUI->InfoVisible(true);
		consolaGUI->SMSVisible(true);

		//também quero um contador
		/*renderData->stats.gpuCounter = new SGPUCounter(250);

		//por omissão tenho estas variáveis
		renderData->stats.gpuCounter->addCounter("timeSlot_draw3D", true, 1.0f, 0.0f, 0.0f);
		renderData->stats.gpuCounter->addCounter("timeSlot_process", true, 0.0f, 0.2f, 0.9f);
		renderData->stats.gpuCounter->addCounter("timeSlot_idle", true, 0.5f, 0.15f, 0.5f);*/

		//mando um sms a avisar
		consolaGUI->SMSAdd("Developer mode is ON", UI::ConsoleGUI::SMSTime, 1.0f, 0.0f, 0.0f);
	}

	//tudo pronto a ir, só tenho de imprimir alguma informacao
	gbMainConsole->Log(" ");
	gbMainConsole->Log("#255,155,0.Aplic#255,136,0.ation #255,128,0.ready #255,106,0.to #255,99,0.go...");
	gbMainConsole->LogTab("type \"help\" to print information regarding this console (or optionally for a command or variable)", 2);
	gbMainConsole->LogTab("type \"com_list\" for a list of commands and variables (or \"com_listBind\" for a list of binds)", 2);
	gbMainConsole->Log(" ");
	gbMainConsole->Log("===============================================");
	gbMainConsole->Log(" ");

	//mostro os créditos iniciais (se não for developer)
	//if (gbMainConsole->VarGetDataI("developer") == 0)
	{
		//mostro os créditos iniciais
		showInitialCredits(renderer2D, glContext, viewport);

		//reponho a matriz
		viewport->setProjection(HorseRadish::OpenGL::Tools::Viewport::Proj3D);
		viewport->updateGL();
	}

	//***************
	//**********
	{
		//HorseRadish::Streams::FileStream fileStream(HorseRadish::IO::Path("c:/Users/Sigma/Desktop/doom3.hrf"), true, false);
		HorseRadish::Streams::FileStream fileStream(HorseRadish::IO::Path("c:/Users/Sigma/Desktop/test_scene.hrf"), true, false);

		renderData->Cleanup();
		if (renderData->ImportHRF(&fileStream) < 0)
			renderData->Cleanup();
		renderData->LoadData(glObjectManager, gbFileSystem, texManagerMain);

		rendererDeferred->LoadWorld(gbFileSystem);
	}
	//**********
	//***************
	
	//antes de começar a desenhar, tenho de iniciar os contadores
	timerSecond.ReStart();
	timerFrame.ReStart();
	timerTotal.ReStart();

	//ciclo infinito da thread
	while(true)
	{
		//se for para sair
		if (WaitForSingleObject(gbThreadRenderEventExit, 0) == WAIT_OBJECT_0)
			break;

		//a primeira coisa é acertar os tempos
		renderer2D->auxTools.lastTimeS = renderer2D->auxTools.curTimeS;
		renderer2D->auxTools.curTimeS = timerTotal.GetTimeS();

		//caso nao desenhe nada
		HorseRadish::OpenGL::glColorMask(GL_TRUE,GL_TRUE,GL_TRUE,GL_TRUE);
		HorseRadish::OpenGL::glDepthMask(GL_TRUE);
		HorseRadish::OpenGL::glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		//desenho a cena normalmente
		//renderData->RenderFrame(camera, viewport);
		rendererDeferred->Render(camera, viewport);

		//desenho algum debug se existir
		//renderData->RenderDebug(gbMainConsole, camera, viewport);

		//finalmente (e porque isto foi tudo pra um FBO) faço o render final seguido de algum debug se existir
		//renderData->RenderComposite(gbMainConsole, camera, viewport);
		//renderData->RenderDebugComposite(gbMainConsole, camera, viewport);

		//leio o tempo que estive à espera para desenhar o 3D
		timeSpentDrawing = timerFrame.GetTimeMS();

		//se for para tirar algum screenshot
		if (gbMainConsole->VarGetDataI("sys_screenshot") > 0)
		{
			//diminuo o número de imagens a tirar e espero que todos os comandos do OpenGL acabei
			gbMainConsole->VarSetDataI("sys_screenshot", gbMainConsole->VarGetDataI("sys_screenshot") - 1);
			HorseRadish::OpenGL::glFinish();

			//crio um ficheiro (ao sair do scope o ficheiro é fechado)
			HorseRadish::Streams::FileStream fileStream(HorseRadish::IO::Path("screenshot.bmp"), false, true);

			//basta mandar tirar o screenshot
			glContext->TakeScreenshot(fileStream);
		}

		//se alguma coisa da consola precisar de ser desenhado
		if (consolaGUI->GUIVisivel(UI::ConsoleGUI::FunctionAll) == true)
		{			
			//o opengl (não esquecer: não tenho depth buffer por defeito)
			HorseRadish::OpenGL::glEnable(GL_BLEND);
			HorseRadish::OpenGL::glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

			//se for preciso, actualizo o texto da consola propriamente dita
			if (gbMainConsole->AsChanged() == true)
				consoleUIMain->ActualizaTextoConsola();

			//mando desenhar a consola
			//SOverlayDraw(gbMainConsole);
			consolaGUI->Draw(viewport);

			//reponho o opengl
			HorseRadish::OpenGL::glDisable(GL_BLEND);
		}

		//se houver uma camera e a consola não estiver a consumir input
		if ((camera != nullptr) && (consolaGUI->ConsoleConsumesInput() == false))
		{
			HorseRadish::OpenGL::Tools::Camera::CameraAction cameraActions;

			//tiro as coisas como estão agora
			gbRawInput->Snapshot();

			//por omissão
			cameraActions = HorseRadish::OpenGL::Tools::Camera::None;

			//preciso de saber o que ando a fazer
			if (gbRawInput->KStatus(VK_UP) || gbRawInput->KStatus('W'))
				cameraActions = (HorseRadish::OpenGL::Tools::Camera::CameraAction)(cameraActions | HorseRadish::OpenGL::Tools::Camera::Forward);
			if (gbRawInput->KStatus(VK_DOWN) || gbRawInput->KStatus('S'))
				cameraActions = (HorseRadish::OpenGL::Tools::Camera::CameraAction)(cameraActions | HorseRadish::OpenGL::Tools::Camera::Backward);
			if (gbRawInput->KStatus(VK_LEFT) || gbRawInput->KStatus('A'))
				cameraActions = (HorseRadish::OpenGL::Tools::Camera::CameraAction)(cameraActions | HorseRadish::OpenGL::Tools::Camera::StrifeLeft);
			if (gbRawInput->KStatus(VK_RIGHT) || gbRawInput->KStatus('D'))
				cameraActions = (HorseRadish::OpenGL::Tools::Camera::CameraAction)(cameraActions | HorseRadish::OpenGL::Tools::Camera::StrifeRight);
			if (gbRawInput->KStatus(VK_SPACE))
				cameraActions = (HorseRadish::OpenGL::Tools::Camera::CameraAction)(cameraActions | HorseRadish::OpenGL::Tools::Camera::Up);
			if (gbRawInput->KStatus(VK_LCONTROL))
				cameraActions = (HorseRadish::OpenGL::Tools::Camera::CameraAction)(cameraActions | HorseRadish::OpenGL::Tools::Camera::Down);
			if (gbRawInput->KStatus(VK_LSHIFT) || gbRawInput->KStatus(VK_RSHIFT))
				cameraActions = (HorseRadish::OpenGL::Tools::Camera::CameraAction)(cameraActions | HorseRadish::OpenGL::Tools::Camera::Run);

			//posso actualizar a camera
			camera->CommitInput(cameraActions, gbRawInput->MStatusPosX(), gbRawInput->MStatusPosY(), true, renderer2D->auxTools.curTimeS - renderer2D->auxTools.lastTimeS);
		}

		//espero pelo mutex (variável numMsgListaMain), mando a mensagem para o GUI da consola e liberto o mutex
		if (gbThreadParams->numMsgListaMain > 0)
		{
			//tranco a consola e passo por todas as mensagens e actualizo a consola e o overlay
			for(unsigned int curMsgIndex=0; curMsgIndex<gbThreadParams->numMsgListaMain; curMsgIndex++)
			{
				//mando processar as teclas
				if (gbThreadParams->listaMain[curMsgIndex].message == WM_KEYUP)
					processWindowsKeyUp(gbThreadParams->listaMain[curMsgIndex].wParam);

				//se a consola está visível
				if (consolaGUI->GUIVisivel(HorseRadish::Console::UI::ConsoleGUI::FunctionConsole) == true)
				{
					//escolho a tab para onde mandar as mensagens
					if (consolaGUI->ConsoleGetSelectedTab() == consoleUIMain)
						consoleUIMain->ProcessMSG(gbThreadParams->listaMain + curMsgIndex);
					else if (consolaGUI->ConsoleGetSelectedTab() == consoleUIDeferred)
						consoleUIDeferred->ProcessMSG(gbThreadParams->listaMain + curMsgIndex);
				}

				//mando o resto para a consola e para o sistema de overlays
				consolaGUI->ConsoleProcessMSG(gbThreadParams->listaMain + curMsgIndex);
				//SOverlayWndMessage(gbMainConsole, gbThreadParams->listaMain[curMsgIndex].message,gbThreadParams->listaMain[curMsgIndex].wParam,gbThreadParams->listaMain[curMsgIndex].lParam);
			}

			//verifico se o editor me mandou alguma coisa
			if ((gbEditorParam != nullptr) && (WaitForSingleObject(gbEditorParam->hEventEditorWrote, 0) == WAIT_OBJECT_0))
				editorProcessMemory();

			//já não tenho nada na lista
			gbThreadParams->numMsgListaMain = 0;
		}

		//mais uma frame
		glContext->CounterIncrease(HorseRadish::OpenGL::Objects::Context::Frames);

		//se já passou um segundo
		if (timerSecond.GetTimeMS() > 1000.0)
		{
			double tempoAux;

			//para ajudar nos cálculos
			tempoAux = 1.0 / timerSecond.GetTimeS(true);

			//ajusto o valor das variáveis
			gbMainConsole->VarSetDataI("sys_infoFPS", HorseRadish::Math::ftoi(((double)glContext->CounterGetValue(HorseRadish::OpenGL::Objects::Context::Frames)) * tempoAux));
			gbMainConsole->VarSetDataF("r_infoMTRIS", ((double)glContext->CounterGetValue(HorseRadish::OpenGL::Objects::Context::Triangles)) * tempoAux);
			gbMainConsole->VarSetDataF("r_infoMVERTS", ((double)glContext->CounterGetValue(HorseRadish::OpenGL::Objects::Context::Vertices)) * tempoAux);

			//recomeço as contagens
			glContext->CounterReset(HorseRadish::OpenGL::Objects::Context::Frames);
			glContext->CounterReset(HorseRadish::OpenGL::Objects::Context::Triangles);
			glContext->CounterReset(HorseRadish::OpenGL::Objects::Context::Vertices);
		}

		//mando o renderer preparar a próxima frame
		renderData->PrepareNextFrame(camera, viewport);

		//leio o tempo que estive à espera de processar as coisas do motor
		timeSpentProcessing = timerFrame.GetTimeMS() - timeSpentDrawing;

		//mostro o que desenhei, isto tá no fim pra ajudar no paralelismo entre CPU e GPU
		glContext->SwapBuffers();

		//leio o tempo que estive à espera de acabar de fazer o swap
		timeSpentIdle = timerFrame.GetTimeMS() - timeSpentProcessing;

		//se tiver coisas para ler do GPU, leio
		//if (renderData->stats.gpuCounter != nullptr)
		//{
		//	float slotTotalInv;

		//	//preciso de calcular o máximo dos tempos para poder fazer as percentagens correctas
		//	slotTotalInv = 1.0f / (timeSpentDrawing + timeSpentProcessing + timeSpentIdle) * 100.0f;

		//	//os contadores a usar
		//	renderData->stats.gpuCounter->sampleCounter("timeSlot_draw3D", timeSpentDrawing * slotTotalInv);
		//	renderData->stats.gpuCounter->sampleCounter("timeSlot_process", timeSpentProcessing * slotTotalInv);
		//	renderData->stats.gpuCounter->sampleCounter("timeSlot_idle", timeSpentIdle * slotTotalInv);
		//	renderData->stats.gpuCounter->sampleCounter("render_numTris", renderData->stats.numTris);
		//	renderData->stats.gpuCounter->sampleCounter("render_glDrawElements", renderData->stats.numGlDrawElements);

		//	//posso avançar com o sample
		//	renderData->stats.gpuCounter->sampleMoveNext();
		//}
		
		//vejo o estado da musica e quando para, coloco outra
		//if (SAudio::SAudioMusicStatus()==SAUDIO_STATUS_STOPPED)
		//	{
		//	SAudio::SAudioMusicLoad("~/test2.ogg");
		//	SAudio::SAudioMusicAction(SAUDIO_ACTION_PLAY);
		//	}

		//se não for developer, tenho de limitar as FPS
		if (gbMainConsole->VarGetDataI("developer") == 0)
		{
			//calculo o tempo total que a frame demorou e reinicio o timer
			auto frameTotalTimeMS = timerFrame.GetTimeMS();
		
			//se a frame demorou menos do que aquilo que devia (para dar 60fps)
			if (frameTotalTimeMS < 16.5)
			{
				//durmo o tempo necessário
				Sleep(16.5 - frameTotalTimeMS);

				//mas não quer dizer que tenha dormido o tempo necessário
				while (timerFrame.GetTimeMS() < 16.5);
			}
		}

		//agora sim, faço restart do timer
		timerFrame.ReStart();
	}

	//apago o GUI da consola
	delete consolaGUI;
	consolaGUI = nullptr;

	delete rendererDeferred;
	rendererDeferred = nullptr;

	delete renderer2D;
	renderer2D = nullptr;

	//apago tudo o que esteja na cena e dou cabo da própria cena
	renderData->Cleanup();

	//já não preciso de contar nada do GPU
	//delete renderData->stats.gpuCounter;
	//renderData->stats.gpuCounter = nullptr;

	//apago tudo do render

	//já não preciso disto
	delete camera;
	delete viewport;
	camera = nullptr;
	viewport = nullptr;

	//posso apagar o render
	delete renderData;
	renderData = nullptr;

	//posso ver-me livre das texturas
	delete texManagerMain;
	delete texManagerRender;
	delete texManagerGUI;
	texManagerMain = nullptr;
	texManagerRender = nullptr;
	texManagerGUI = nullptr;

	//limpo isto tudo
	delete glObjectManager;
	glObjectManager = nullptr;

	//apago o contexto
	delete glContext;
	glContext = nullptr;
}

static
bool messageLoop()
{
	HANDLE gbThreadRender;

	//preicso de espaço para passar / trocar dados com a thread
	gbThreadParams = new THREAD_RENDER_PARAMETERS;

	//limpo isto
	memset(gbThreadParams, 0, sizeof(THREAD_RENDER_PARAMETERS));

	//inicio a thread dos gráficos e o evento para a fazer sair
	gbThreadRenderEventExit = CreateEvent(nullptr, true, false, nullptr);
	gbThreadRenderEventReady = CreateEvent(nullptr, true, false, nullptr);
	gbThreadRender = (HANDLE)_beginthread(renderThread, 0, nullptr);

	//espero que a thread fiquer pronta
	WaitForSingleObject(gbThreadRenderEventReady, INFINITE);

	//aqui vou eu até o windows me mandar parar
	while (gbAppExitReason == HR_EXIT_NOT_YET)
	{
		//mando ver se há mensagens e se houver, para as mandar
		gbWindow->PeekMessageDispatch(true);

		//para isto não ocupar o CPU por completo
		Sleep(0);
	}

	//assinalo o evento para a thread de render terminar e espero que ela termine
	SetEvent(gbThreadRenderEventExit);
	WaitForSingleObject(gbThreadRender, INFINITE);

	//posso apagar esta memória
	delete gbThreadParams;
	gbThreadParams = nullptr;

	//fecho estes handles
	CloseHandle(gbThreadRenderEventExit);
	CloseHandle(gbThreadRenderEventReady);
	gbThreadRender = nullptr;
	gbThreadRenderEventExit = nullptr;
	gbThreadRenderEventReady = nullptr;

	//correu tudo bem
	return true;
}

static
void processWindowsKeyUp(WPARAM wParam)
{
	//só mando teclas como BIND se a consola não está visivel!
	/*if (consolaGUI->GUIVisivel(CONSOLE_GUI_CONSOLE)==false)
		{
		int flags;

		//crio as flags para modificar a tecla
		flags=(GetKeyState(VK_SHIFT) & 0xFF00)? SCONSOLE_KEY_MSHIFT:0;
		flags|=(GetKeyState(VK_CONTROL) & 0xFF00)? SCONSOLE_KEY_MCONTROL:0;
		flags|=(GetKeyState(VK_MENU) & 0xFF00)? SCONSOLE_KEY_MALT:0;

		//mandar para a consola
		if (wParam>='A' && wParam<='Z')
			gbMainConsole->BindEmitKey((wParam-'A')+SCONSOLE_KEY_A,flags);
		else if (wParam>='0' && wParam<='9')
			gbMainConsole->BindEmitKey((wParam-'0')+SCONSOLE_KEY_0,flags);
		else if (wParam>=VK_F1 && wParam<=VK_F12)
			gbMainConsole->BindEmitKey((wParam-VK_F1)+SCONSOLE_KEY_F1,flags);
		else
			{
			switch(wParam){
				case VK_ESCAPE:		gbMainConsole->BindEmitKey(SCONSOLE_KEY_ESCAPE,flags);break;
				case VK_RETURN:		gbMainConsole->BindEmitKey(SCONSOLE_KEY_ENTER,flags);break;
				case VK_SPACE:		gbMainConsole->BindEmitKey(SCONSOLE_KEY_SPACE,flags);break;
				case VK_TAB:		gbMainConsole->BindEmitKey(SCONSOLE_KEY_TAB,flags);break;
				case VK_PAUSE:		gbMainConsole->BindEmitKey(SCONSOLE_KEY_PAUSE,flags);break;
				case VK_UP:			gbMainConsole->BindEmitKey(SCONSOLE_KEY_UP,flags);break;
				case VK_DOWN:		gbMainConsole->BindEmitKey(SCONSOLE_KEY_DOWN,flags);break;
				case VK_LEFT:		gbMainConsole->BindEmitKey(SCONSOLE_KEY_LEFT,flags);break;
				case VK_RIGHT:		gbMainConsole->BindEmitKey(SCONSOLE_KEY_RIGHT,flags);break;
				}
			}
		}*/

	//if (wParam==VK_F8)
	//{
	//unsigned int scriptID;
	//scriptID = SScript::SScriptLoad(mainScriptKernelID,"~/gmdebug.txt",nullptr);
	//SMemSnapshot();
	//}
}

//funções para processar as mensagens vindas do editor
void editorProcessMemory()
{
	int mensagemTipo;

	//faço reset ào evento e tenho sempre de fazer lock à memória
	ResetEvent(gbEditorParam->hEventEditorWrote);
	WaitForSingleObject(gbEditorParam->hMutex,INFINITE);

	//leio o tipo de mensagem
	mensagemTipo = ((int*)gbEditorParam->mappedBuffer)[0];

	//se for do tipo 2 (para carregar uma geometria)
	if (mensagemTipo == 2)
	{
		HorseRadish::String pathFicheiro;
		int formatoInterno;

		//leio o que me mandaram
		pathFicheiro.Set(HorseRadish::String::Encoding::UTF8, gbEditorParam->mappedBufferChar + sizeof(int));
		formatoInterno = ((int*)(gbEditorParam->mappedBufferChar + sizeof(int) + pathFicheiro.GetSizeBytes() + 1))[0];

		//meto o inicio a zero e posso libertar o mutex
		((int*)gbEditorParam->mappedBufferChar)[0] = 0;
		ReleaseMutex(gbEditorParam->hMutex);

		Window::MsgBoxAviso(HorseRadish::String("%s - %d", pathFicheiro.GetData(), formatoInterno).GetData());
		return;
	}

}

static
void editorProcessMessages(WPARAM wParam, LPARAM lParam)
{
	//se o editor está simplesmente a dizer "Olá"
	if (wParam == 1)
	{
		//basta escrever olá no ecran
		gbMainConsole->LogInfo("Editor said: Hi! I'm alive and well...");
		//GUISMSAdd("Editor said: Hi! I'm alive and well...", CONSOLE_SMS_TIME, 0.0f, 1.0f, 0.0f);
		return;
	}

	//alguem pressionou uma tecla ou o WM_MOUSEWHEEL
	if ((wParam == 2) || (wParam == 3) || (wParam == 4) || (wParam == 5))
	{
		MSG msgAux;

		//se foi o WM_KEYUP
		if (wParam == 2)
			processWindowsKeyUp(lParam);

		//crio uma mensagem auxiliar para mandar para a consola
		memset(&msgAux, 0, sizeof(MSG));
		switch(wParam)
		{
			case 2: msgAux.message = WM_KEYUP;		break;
			case 3: msgAux.message = WM_KEYDOWN;	break;
			case 4: msgAux.message = WM_CHAR;		break;
			case 5: msgAux.message = WM_MOUSEWHEEL;	break;
		}
		msgAux.wParam = lParam;
		
		//mando esta mensagem para a consola
		//GUIConsoleMSG(&msgAux);
		return;
	}

	gbMainConsole->Log(HorseRadish::String("Mensagem id: %d", wParam).GetData());
}

//função principal para receber mensagens do windows
LRESULT CALLBACK windowsMessages(HWND hWnd, UINT messageID, WPARAM wParam, LPARAM lParam)
{
	//esta mensagem é processada directamente aqui
	if (messageID == WM_INPUT)
    {
		//mas só faço alguma coisa se tiver onde possa guardar os dados
		if (gbRawInput != nullptr)
		{
			UINT bufferSize;
			BYTE bufferAux[40];
	    
			//basta ler os dados do buffer (40 é suficiente para o rato e teclado, qualquer outro é preciso saber qual o tamanho necessário)
			bufferSize = sizeof(bufferAux);
			GetRawInputData((HRAWINPUT)lParam, RID_INPUT, bufferAux, &bufferSize, sizeof(RAWINPUTHEADER));
	    
			//basta mandar os dados
			gbRawInput->ProcessRawInput((RAWINPUT*)bufferAux);
		}

		//foi tudo processado
        return 0;
    }

	//se foi uma tecla que foi abaixo ou acima, tenho de mandar isso para o input (se existir)
	if ((gbRawInput != nullptr) && ((messageID == WM_KEYDOWN) || (messageID == WM_KEYUP)))
		gbRawInput->ProcessKey((messageID == WM_KEYDOWN), wParam, lParam);

	//qualquer mensagem é colocada nos parametros da thread
	if (gbThreadParams != nullptr)
	{
		//verifico fila de mensagens, se tiver alguma coisa e tiver espaço onde escrever
		if (gbThreadParams->numMsgListaAux < THREAD_PARAM_NUM_MSG)
		{
			memset(gbThreadParams->listaAux + gbThreadParams->numMsgListaAux, 0, sizeof(MSG));
			gbThreadParams->listaAux[gbThreadParams->numMsgListaAux].hwnd = hWnd;
			gbThreadParams->listaAux[gbThreadParams->numMsgListaAux].message = messageID;
			gbThreadParams->listaAux[gbThreadParams->numMsgListaAux].wParam = wParam;
			gbThreadParams->listaAux[gbThreadParams->numMsgListaAux].lParam = lParam;
			gbThreadParams->numMsgListaAux++;
		}

		//se tiver a lista principal a 0 e alguma coisa na minha lista auxiliar, transfiro as coisas para lá
		if ((gbThreadParams->numMsgListaMain == 0) && (gbThreadParams->numMsgListaAux != 0))
		{
			memcpy(gbThreadParams->listaMain, gbThreadParams->listaAux, sizeof(MSG)*gbThreadParams->numMsgListaAux);
			gbThreadParams->numMsgListaMain = gbThreadParams->numMsgListaAux;
			gbThreadParams->numMsgListaAux = 0;
		}
	}

	//se estou em modo de editor e recebo uma mensagem interna, processo-a
	if ((gbEditorParam != nullptr) && (messageID == gbEditorParam->msgID))
	{
		editorProcessMessages(wParam, lParam);
		return 0;
	}

	//verificar qual é
	switch (messageID){
		case WM_DESTROY:
		case WM_CLOSE:
						//posso simplesmente sair
						if (gbAppExitReason == HR_EXIT_NOT_YET)
							gbAppExitReason = HR_EXIT_QUIT;
						PostQuitMessage(0);
						return 0;
		case WM_QUIT:
						//posso simplesmente sair
						if (gbAppExitReason == HR_EXIT_NOT_YET)
							gbAppExitReason = HR_EXIT_QUIT;
						gbAppExitCode = wParam;
						return 0;
						
		case WM_LBUTTONUP:
		case WM_RBUTTONUP:
						//se estou em modo de editor, ganho o focus
						if (gbEditorParam != nullptr)
						{
							gbWindow->SetFocus();
							return 0;
						}
						break;
						
	}

	//mando para o tratamento por omissão
	return DefWindowProc(hWnd, messageID, wParam, lParam);
}

//função de entrada do windows
int WINAPI wWinMain(HINSTANCE hInst, HINSTANCE hInstPrev, PWSTR lpCmdLine, int nCmdShow)
{
#ifdef HR_VS_MEMORY_LEAKS
	_CrtSetDbgFlag ( _CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF );
#endif

	unsigned int curControlWord;
	int infoValue;

	//antes de tudo, verificar isto
	if ((HorseRadish::Platform::GetSystemInfo(HorseRadish::Platform::CleanBoot, infoValue) == false) || (infoValue == 0))
	{
		Window::MsgBoxAviso("The OS did not boot normally!\nFor security reasons the application will now exit.");
		return 0;
	}

	//quero processadores com, no mínimo, SSE, SSE2 e CMOV
	if (HorseRadish::Machine::CPUCheckFeatures((HorseRadish::Machine::CPUFeature)(HorseRadish::Machine::SSE | HorseRadish::Machine::SSE2 | HorseRadish::Machine::CMov)) == false)
	{
		Window::MsgBoxAviso("The CPU doesn't have the minimum required features.\nThe application cannot proceed.");
		return 0;
	}

	//crio esta classe que me diz se outra aplicação já está a correr
	HorseRadish::Platform::SingleInstance singleInstance;
	if (singleInstance.IsAnotherRunning() == true)
	{
		Window::MsgBoxErro("Another instance of this application is already running.");
		return 0;
	}

#ifndef _M_X64
	//mudo a control word para operações de FP
	_controlfp_s(&curControlWord, _PC_24, _MCW_PC);	//quero uma precisão de 24bits
	_controlfp_s(&curControlWord, _DN_FLUSH, _MCW_DN); //quero que os denormal vão para zero
	_controlfp_s(&curControlWord, _RC_NEAR, _MCW_RC); //arredondar para mais próximo (1.5 > 2.0)
	_controlfp_s(&curControlWord, _EM_INVALID|_EM_DENORMAL|_EM_ZERODIVIDE|_EM_OVERFLOW|_EM_UNDERFLOW|_EM_INEXACT, _MCW_EM); //não quero excepções
	_clearfp(); //limpo quaisquer excepções anteriores
#endif

	//inicio o sistema
	systemInitialize(hInst, lpCmdLine);

	//preparo a janela e o resto dos sistemas
	windowInitialize();

	//inicia o message loop da aplicação e cria também a thread de render
	messageLoop();

	//fecho a janela
	windowShutdown();

	//fecho o sistema
	systemShutdown();

	//se for preciso mandar executar alguma coisa
	if (gbAppExitReason == HR_EXIT_RESTART)
		HorseRadish::Platform::InstanciateProcess("Horseradish.exe");
	else if (gbAppExitReason == HR_EXIT_RESTART_EDITOR)
		HorseRadish::Platform::InstanciateProcess("BloodyMary.exe");

	//reponho a control word para operações de FP e saio
	_controlfp_s(&curControlWord, _CW_DEFAULT, 0xfffff );
	return gbAppExitCode;

	//porcaria dos avisos
	UNREFERENCED_PARAMETER(hInstPrev); 
    UNREFERENCED_PARAMETER(nCmdShow);
}