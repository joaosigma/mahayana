#include "common\platform.hpp"
#include "common\math.hpp"

#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <functional>

#include "console.hpp"

namespace HorseRadish
{
namespace Console
{

//define o número máximo de argumentos
#define NUM_ARGUMENTS 10

//quantos espaços tem cada tab
const int Console::tabNumSpaces = 4;

//§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§
//§§§   -= Coisas auxiliares locais =-   §§§
//§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§
bool Console::divideCommand(const char * const command)
{
	bool resultado;
	const char *read;
	char subCommand[128],*write;

	//toca a passar por tudo
	resultado=true;
	write=subCommand;
	for(read=command; (*read)!='\0'; read++)
	{
		//copia (excepto se for um espaço e estiver no inicio
		if (write==subCommand && *read==' ')
			continue;
		*write=*read;
		write++;

		//se encontrar ';' tem vários comandos
		if ((*read)==';')
		{
			*(write-1)='\0';
			if (subCommand[0]!='\0')
				resultado &= this->Process(subCommand);
			write=subCommand;
			continue;
		}

		//dentro de aspas não conta
		if ((*read)=='"')
		{
			for(read++; (*read)!='\0' && (*read)!='"'; read++,write++)
				*write=*read;
			if ((*read)=='"')
				*(write++)=*read;
			continue;
		}
	}

	//fecho e mando
	*write='\0';
	if (subCommand[0]!='\0')
		resultado &= this->Process(subCommand);

	//se um dos comandos falhou, digo que no todo, falharam
	return resultado;
}

bool Console::setVarI(DATA_NO * const parametros, const int data)
{
	//verificar parametros
	if ((parametros == nullptr) || (parametros->isCommand == true) || (parametros->varData.tipo != VarType::Integer))
		return false;

	//se não tenho callback, basta copiar o valor (tendo o cuidado de clampar se for esse o caso)
	if (!parametros->varData.callback)
	{
		parametros->varData.value.integer = data;
		if (parametros->varData.flags & VarFlags::Clamp)
			parametros->varData.value.integer = HorseRadish::Math::iClampZero(parametros->varData.value.integer);
		return true;
	}

	//old value
	auto oldValue = parametros->varData.value.integer;

	//posso copiar o valor (tendo o cuidado de clampar se for esse o caso)
	parametros->varData.value.integer = data;
	if (parametros->varData.flags & VarFlags::Clamp)
		parametros->varData.value.integer = HorseRadish::Math::iClampZero(parametros->varData.value.integer);

	//chamo agora a callback da variável
	parametros->varData.callback(*this, parametros->id, parametros->varData.tipo, &parametros->varData.value.integer, &oldValue);

	//tá tudo bem
	return true;
}

bool Console::setVarF(DATA_NO * const parametros, const float data)
{
	//verificar parametros
	if ((parametros == nullptr) || (parametros->isCommand == true) || (parametros->varData.tipo != VarType::Float))
		return false;

	//se não tenho callback, basta copiar o valor (tendo o cuidado de clampar se for esse o caso)
	if (!parametros->varData.callback)
	{
		parametros->varData.value.numeric = data;
		if (parametros->varData.flags & VarFlags::Clamp)
			parametros->varData.value.numeric = HorseRadish::Math::fClamp(parametros->varData.value.numeric, 0.0f, 1.0f);
		return true;
	}

	//old value
	auto oldValue = parametros->varData.value.numeric;

	//posso copiar o valor (tendo o cuidado de clampar se for esse o caso)
	parametros->varData.value.numeric = data;
	if (parametros->varData.flags & VarFlags::Clamp)
		parametros->varData.value.numeric = HorseRadish::Math::fClamp(parametros->varData.value.numeric, 0.0f, 1.0f);

	//chamo agora a callback da variável
	parametros->varData.callback(*this, parametros->id, parametros->varData.tipo, &parametros->varData.value.numeric, &oldValue);

	//tá tudo bem
	return true;
}

bool Console::setVarS(DATA_NO * const parametros, const char *data)
{
	//verificar parametros
	if ((parametros == nullptr) || (parametros->isCommand == true) || (parametros->varData.tipo != VarType::String) || (data == nullptr))
		return false;

	//se não tenho callback, basta copiar o valor
	if (!parametros->varData.callback)
	{
		strcpy_s(parametros->varData.value.string, 128, data);
		return true;
	}

	//old value
	auto oldValue = parametros->varData.value.string;

	//posso copiar o valor
	strcpy_s(parametros->varData.value.string, 128, data);

	//chamo agora a callback da variável
	parametros->varData.callback(*this, parametros->id, parametros->varData.tipo, (char*)parametros->varData.value.string, (const char*)oldValue);

	//tá tudo bem
	return true;
}

bool Console::parseCommand(const char * const command, ARG_DATA * const argList, int &argNumber)
{
	const char *read;
	char *writer;

	//limpo isto para começar
	argNumber=0;
	argList[0].data[0]='\0';

	//ignoro espaços em branco e verifico se não tenho nada
	for(read=command; (*read)!='\0' && (*read)==' '; read++);
	if (*read=='\0')
		return false;

	//toca a fazer parse
	writer=argList[argNumber].data;
	while(*read!='\0')
	{
		//se for um espaço, movo para o proximo argumento
		if (*read==' ')
		{
			//como todos os espaços
			for(; (*read)!='\0' && (*read)==' '; read++);

			//se terminei
			if (*read=='\0')
			{
				argNumber++;
				return true;
			}

			//toca a andar
			argNumber++;
			writer=argList[argNumber].data;
			*writer='\0';
			continue;
		}

		//se entrar dentro de parentesis
		if (*read=='(')
		{
			//copio até terminar o parentesis
			for(; (*read)!='\0' && (*read)!=')'; read++)
			{
				//se permito aspas dentro de (
				if (*read=='"')
				{
					this->LogError("ConsoleParser: can't use '\"' inside a () pair.");
					return false;
				}

				//não copio espacos em branco
				if (*read==' ')
					continue;

				//posso copiar nas calmas!
				*(writer++)=*read;
				*writer='\0';
			}

			//se terminei é barraca
			if (*read=='\0')
			{
				this->LogError("ConsoleParser: expected ')' at the end of sentence.");
				return false;
			}

			//copio o ultimo ')', mas isso nao implica mudar de argumento
			*(writer++)=*read;
			*writer='\0';
			read++;
			continue;
		}

		//se entrar dentro de aspas
		if (*read=='"')
		{
			//copio até terminar o parentesis (não copio o parentesis)
			for(read++; (*read)!='\0' && (*read)!='"'; read++)
				*(writer++)=*read;

			//fechar
			*writer='\0';

			//se terminei é barraca
			if (*read=='\0')
			{
				this->LogError("ConsoleParser: expected '\"' at the end of sentence.");
				return false;
			}

			//passo o ultimo '"', mas isso nao implica mudar de argumento
			read++;
			continue;
		}

		//adiciono
		*(writer++)=*read;
		*writer='\0';
		read++;
	}

	//tenho de marcar aquele onde estou
	argNumber++;

	//chegando aqui tá tudo bem
	return true;
}

void Console::doThreeArgOperation(DATA_NO * const spec, const ARG_DATA * const argList)
{
	//se for uma string, só pode ter +=
	if (spec->varData.tipo == VarType::String && argList[1].data[0] != '+')
	{
		this->LogError("Error in second argument: string variavels can only accept +=");
		return;
	}

	//se for do tipo string, posso já fazer a operação
	if (spec->varData.tipo == VarType::String)
	{
		char finalResult[128];

		strcpy_s(finalResult, 128, spec->varData.value.string);
		strcat_s(finalResult, 128, argList[2].data);
		Console::setVarS(spec, finalResult);
		return;
	}

	//se for do tipo int
	if (spec->varData.tipo == VarType::Integer)
	{
		if (argList[1].data[0] == '+')
			Console::setVarI(spec, spec->varData.value.integer + atoi(argList[2].data));
		else if (argList[1].data[0] == '-')
			Console::setVarI(spec, spec->varData.value.integer - atoi(argList[2].data));
		else if (argList[1].data[0] == '*')
			Console::setVarI(spec, spec->varData.value.integer * atoi(argList[2].data));
		else
			Console::setVarI(spec, spec->varData.value.integer / atoi(argList[2].data));
		return;
	}

	//se for do tipo float
	if (spec->varData.tipo == VarType::Float)
	{
		if (argList[1].data[0] == '+')
			Console::setVarF(spec, spec->varData.value.numeric + ((float)atof(argList[2].data)));
		else if (argList[1].data[0] == '-')
			Console::setVarF(spec, spec->varData.value.numeric - ((float)atof(argList[2].data)));
		else if (argList[1].data[0] == '*')
			Console::setVarF(spec, spec->varData.value.numeric * ((float)atof(argList[2].data)));
		else
			Console::setVarF(spec, spec->varData.value.numeric / ((float)atof(argList[2].data)));
		return;
	}
}

void Console::doThreeArgSetCondition(DATA_NO * const spec, const ARG_DATA * const argList)
{
	//se for para fazer coisas verdadeiras e a condição estava errada
	if (argList[1].data[0] == '?' && argList[1].data[1] == '\0' && spec->varData.condition == false)
		return;
	//se for para fazer coisas falsas e a condição estava certa, saio
	if (argList[1].data[0] == '?' && argList[1].data[1] == '!' && spec->varData.condition == true)
		return;

	//posso fazer o que tenho a fazer
	if (spec->varData.tipo == VarType::Integer)
	{
		Console::setVarI(spec, atoi(argList[2].data));
		return;
	}
	if (spec->varData.tipo == VarType::Float)
	{
		Console::setVarF(spec, (float)atof(argList[2].data));
		return;
	}
	if (spec->varData.tipo == VarType::String)
	{
		Console::setVarS(spec, argList[2].data);
		return;
	}
}

void Console::doThreeArgCheckCondition(DATA_NO * const spec, const ARG_DATA * const argList)
{
	//se for para verificar se é igual ou diferente
	if ((argList[1].data[0] == '=' || argList[1].data[0] == '!') && argList[1].data[1] == '=')
	{
		//verifico sempre se é igual
		if (spec->varData.tipo == VarType::Integer)
			spec->varData.condition = (spec->varData.value.integer == atoi(argList[2].data));
		else if (spec->varData.tipo == VarType::Float)
			spec->varData.condition = (spec->varData.value.numeric == atof(argList[2].data));
		else if (spec->varData.tipo == VarType::String)
			spec->varData.condition = (strcmp(spec->varData.value.string, argList[2].data) == 0);

		//se pediram pra ver a diferença, basta mudar a condição e prontos
		if (argList[1].data[0] == '!')
			spec->varData.condition = !spec->varData.condition;
		return;
	}

	//se for para verificar se é menor ou maior
	if ((argList[1].data[0] == '<' || argList[1].data[0] == '>') && argList[1].data[1] == '\0')
	{
		//verifico sempre se é menor
		if (spec->varData.tipo == VarType::Integer)
			spec->varData.condition = (spec->varData.value.integer < atoi(argList[2].data));
		else if (spec->varData.tipo == VarType::Float)
			spec->varData.condition = (spec->varData.value.numeric < atof(argList[2].data));
		else if (spec->varData.tipo == VarType::String)
			spec->varData.condition = (strcmp(spec->varData.value.string, argList[2].data) < 0);

		//se pediram pra ver o maior, basta mudar a condição e prontos
		if (argList[1].data[0] == '>')
			spec->varData.condition = !spec->varData.condition;
		return;
	}

	//se for para verificar se é <= (não se pode usar o mesmo if para o >=)
	if (argList[1].data[0] == '<' && argList[1].data[1] == '=')
	{
		if (spec->varData.tipo == VarType::Integer)
			spec->varData.condition = (spec->varData.value.integer <= atoi(argList[2].data));
		else if (spec->varData.tipo == VarType::Float)
			spec->varData.condition = (spec->varData.value.numeric <= atof(argList[2].data));
		else if (spec->varData.tipo == VarType::String)
			spec->varData.condition = (strcmp(spec->varData.value.string, argList[2].data) <= 0);
		return;
	}

	//se for para verificar se é >= (não se pode usar o mesmo if para o <=)
	if (argList[1].data[0] == '>' && argList[1].data[1] == '=')
	{
		if (spec->varData.tipo == VarType::Integer)
			spec->varData.condition = (spec->varData.value.integer >= atoi(argList[2].data));
		else if (spec->varData.tipo == VarType::Float)
			spec->varData.condition = (spec->varData.value.numeric >= atof(argList[2].data));
		else if (spec->varData.tipo == VarType::String)
			spec->varData.condition = (strcmp(spec->varData.value.string, argList[2].data) >= 0);
		return;
	}
}

void Console::doThreeArgVarCommand(DATA_NO * const spec, const ARG_DATA * const argList)
{
	//verifico se o 2 argumento é na verdade ? ou ?!
	if ((argList[1].data[0]=='?' && argList[1].data[1]=='\0') || (argList[1].data[0]=='?' && argList[1].data[1]=='!' && argList[1].data[2]=='\0') )
	{
		//não posso se for readonly
		if (spec->varData.flags & VarFlags::ReadOnly)
		{
			this->LogError("Variable is read-only.");
			return;
		}

		//posso avançar
		doThreeArgSetCondition(spec,argList);
		return;
	}

	//verifico se o 2 argumento é na verdade +=, -=, *= ou /=
	if (argList[1].data[2]=='\0' && argList[1].data[1]=='=' && (argList[1].data[0]=='+' || argList[1].data[0]=='-' || argList[1].data[0]=='*' || argList[1].data[0]=='/') )
		{
		//não posso se for readonly
		if (spec->varData.flags & VarFlags::ReadOnly)
			{
			this->LogError("Variable is read-only.");
			return;
			}

		//posso avançar
		doThreeArgOperation(spec,argList);
		return;
		}

	//verifico se o 2 argumento é na verdade <,>,<=,>=,== ou !=
	if ( (argList[1].data[0]=='<' || argList[1].data[0]=='>') && argList[1].data[0]=='\0')
		{
		doThreeArgCheckCondition(spec,argList);
		return;
		}
	if ( (argList[1].data[1]=='=' && argList[1].data[2]=='\0') && (argList[1].data[0]=='<' || argList[1].data[0]=='>' || argList[1].data[0]=='=' || argList[1].data[0]=='!'))
		{
		doThreeArgCheckCondition(spec,argList);
		return;
		}

	//chegando aqui, deu barraca
	this->LogError("Error in second argument: expected +=, -=, *=, /=, <=, >=, <, >, ==, !=, ? or ?!");
}

void Console::doFiveArgVarCommand(DATA_NO * const spec, const ARG_DATA * const argList)
{
	const char *ptrToUse;

	//o segundo argumento tem de ser ?
	if (argList[1].data[0] != '?' || argList[1].data[1] != '\0')
	{
		this->LogError("Error in second argument: expected ?");
		return;
	}

	//o quarto argumento tem de ser :
	if (argList[3].data[0] != ':' || argList[3].data[1] != '\0')
	{
		this->LogError("Error in fourth argument: expected :");
		return;
	}

	//por defeito uso a condição verdadeira, mas se a condição estava falsa
	ptrToUse = argList[2].data;
	if (spec->varData.condition == false)
		ptrToUse = argList[4].data;

	//posso fazer o que tenho a fazer
	if (spec->varData.tipo == VarType::Integer)
		Console::setVarI(spec, atoi(ptrToUse));
	else if (spec->varData.tipo == VarType::Float)
		Console::setVarF(spec, (float)atof(ptrToUse));
	else if (spec->varData.tipo == VarType::String)
		Console::setVarS(spec, ptrToUse);
}

bool Console::isForcedNoEcho(const char * const command)
{
	const char *read;

	//ignoro espaços em branco
	for (read = command; (*read) != '\0' && (*read) == ' '; read++);

	//se tiver um '}' é para o comando não echoar
	if (*read == '}')
		return true;
	return false;
}

bool Console::isSeveralExpressions(const char * const command)
{
	const char *read;

	//toca a passar por tudo
	for (read = command; (*read) != '\0'; read++)
	{
		//se encontrar ; tem vários comandos
		if ((*read) == ';')
			return true;

		//dentro de aspas não conta
		if ((*read) == '"')
		{
			for (read++; (*read) != '\0' && (*read) != '"'; read++);
			if ((*read) == '"')
				read++;
		}
	}

	//chegando aqui, não encontrei nada, logo não tem varios comandos
	return false;
}

bool Console::checkValue(const char *value, unsigned char &realValue)
{
	unsigned int resultado;

	//iniciar ambos a zero
	resultado=realValue=0;

	//se não for vazio ('\0'), tem de ser um numero
	if (value[2]=='\0')
		return true;
	if (value[2]<'0' || value[2]>'9')
		return false;
	resultado+=value[2]-'0';
	realValue=(unsigned char)resultado;

	//se não for vazio ('\0'), tem de ser um numero
	if (value[1]=='\0')
		return true;
	if (value[1]<'0' || value[1]>'9')
		return false;
	resultado+=(value[1]-'0')*10;
	realValue=(unsigned char)resultado;

	//se não for vazio ('\0'), tem de ser um numero mas só entre [0,2]
	if (value[0]=='\0')
		return true;
	if (value[0]<'0' || value[0]>'2')
		return false;
	resultado+=(value[0]-'0')*100;

	//basta só verificar se não está acima de 255
	if (resultado>255)
		return false;

	//correu tudo bem
	realValue=(unsigned char)resultado;
	return true;
}

bool Console::checkSentence(const char *sentence, int &numSpecialChar)
{
	const char *walkerNext;
	char valR[3],valG[3],valB[3],*curVal;
	unsigned char temp;
	int i, numVirgulas,curValCount;

	//posso iniciar isto a zero
	numSpecialChar=0;

	//verifico os parametros
	if (sentence==nullptr || sentence[0]=='\0')
		return false;

	//para cada caracter da frase
	for(const char *walker = sentence; *walker != '\0'; walker++)
	{
		//se nao for este, sigo
		if (*walker != '#')
			continue;

		//apartir daqui é pq encontrei um caracter #
		numSpecialChar++;

		//guardo esta posição, e vou pra frente
		walkerNext = walker + 1;

		//o caso mínimo é ter #,,. (preto), logo a string não pode chegar 3 bytes à frente
		//nem sequer ter um ponto à frente
		if (walkerNext[0]=='\0' || walkerNext[0]=='.' || walkerNext[1]=='\0' || walkerNext[2]=='\0')
			return false;

		//posso verificar o caso simples: #,,.
		if (walkerNext[0]==',' && walkerNext[1]==',' && walkerNext[2]=='.')
		{
			walker+=3;	//estou num for, logo em cima é adicionado
			continue;
		}

		//conto o numero de virgulas
		for(numVirgulas=i=0; *walkerNext!='.' && *walkerNext!='\0'; i++,walkerNext++)
		{
			if (*walkerNext==',')
				numVirgulas++;
		}

		//agora, só tenho de verificar se tenho 2 virgulas e estou no ponto
		if (numVirgulas!=2 || *walkerNext!='.')
			return false;

		//chegando aqui, sei que tenho um caracter # válido, tirando um pormenor!
		//os valores pra R,G e B têm de estar entre [0,255]
		valR[0]=valR[1]=valR[2]=0;
		valG[0]=valG[1]=valG[2]=0;
		valB[0]=valB[1]=valB[2]=0;

		///posso já avançar com o ponteiro
		walker=walkerNext;
		
		//basta começar a escrever em cada val[R|G|B] (ando de trás pra frente)
		curValCount=2;
		for(walkerNext--,curVal=valB; *walkerNext!='#'; walkerNext--)
		{
			//se tenho de mudar de linha
			if (*walkerNext==',')
			{
				//inicio a contagem
				curValCount=2;
				if (curVal==valB)	{curVal=valG; continue;}
				if (curVal==valG)	{curVal=valR; continue;}
			}

			//se vou copiar um a mais, é porque a frase tá mal feita entre virgulas e o ponto
			if (curValCount<0)
				return false;

			//copio o valor
			curVal[curValCount--]=*walkerNext;
		}

		//basta verificar os valores de cada um dos val[R|G|B]
		if (Console::checkValue(valR, temp)==false)	return false;
		if (Console::checkValue(valG, temp)==false)	return false;
		if (Console::checkValue(valB, temp)==false)	return false;
	}

	//chegando aqui, está tudo bem
	return true;
}

void Console::addToLogger(const char *sentence, int colorCount)
{
	//se não tenho cores, é bastantes simples
	if (colorCount <= 0)
	{
		//basta fazer log
		this->logger->Log(sentence);
		return;
	}

	//chegando aqui tenho cores

	//vou ter de preparar uma segunda frase, por causa dos caracteres especiais da cor
	char *writer, useDataColor[MAX_LOG_SIZE];
	int corVal, curPos;

	#pragma pack (push)
	#pragma pack (1)
	struct DataRGB{
		int numCores;
		struct CheckRGB{
			unsigned int pos;
			unsigned char r, g, b;
		}cores[256];
	}coresData;
	#pragma pack (pop)

	//preparo a useDataColor
	writer = useDataColor;
	curPos = 0;
	coresData.numCores = 0;

	//vou copiando a frase toda, tendo cuidado com o caracter #
	for(const char *walker=sentence; *walker!='\0'; walker++,curPos++)
	{
		*(writer++)=*walker;
		if (*walker != '#')
			continue;

		//para no próximo escrever por cima do #
		writer--;
		
		//qual a posição
		coresData.cores[coresData.numCores].pos = curPos;

		//primeiro o R
		for(corVal=0,walker++; *walker!=','; walker++)
			corVal=corVal*10 +( (int)((*walker)-'0') );
		coresData.cores[coresData.numCores].r = (char)corVal;

		//depois o G
		for(corVal=0,walker++; *walker!=','; walker++)
			corVal=corVal*10 +( (int)((*walker)-'0') );
		coresData.cores[coresData.numCores].g = (char)corVal;

		//e por fim o B
		for(corVal=0,walker++; *walker!='.'; walker++)
			corVal=corVal*10 +( (int)((*walker)-'0') );
		coresData.cores[coresData.numCores].b = (char)corVal;

		//próxima cor
		coresData.numCores++;
		curPos--;
	}

	//fecho a string e faço log
	*writer = '\0';
	this->logger->Log(&coresData, (sizeof(DataRGB::CheckRGB) * coresData.numCores) + sizeof(int), useDataColor);
}

//§§§§§§§§§§§§§§§§§§§§§§§§
//§§§§§§ Classe Console	§§
//§§§§§§§§§§§§§§§§§§§§§§§§
bool Console::addPhrase(const char *Frase, int nespacos)
{
	char useData[MAX_LOG_SIZE];
	const char *useDataPTR;
	int numSpecialChar;

	//se se a frase está corrrecta
	if (strlen(Frase) + nespacos > MAX_LOG_SIZE)
		return false;

	//coloca ou nao espaços (se tem de colocar espaços, uso uma string auxiliar)
	useDataPTR=Frase;
	if (nespacos!=0)
	{
		char *writer;

		writer=useData;
		for(int i=0; i<nespacos; i++,writer++)
			*writer=' ';
		*writer='\0';
		strcat(writer-1,Frase);
		useDataPTR=useData;
	}

	//verifico somente a sintaxe da frase e conto o numero de caracteres #
	if (Console::checkSentence(useDataPTR, numSpecialChar)==false)
		return false;

	//se tiver mais que 255 caracteres #
	if (numSpecialChar > 255)
		return false;

	//gravo para o logger
	addToLogger(useDataPTR, numSpecialChar);

	//e termino em grande
	changeOccured=true;
	return true;
}

Console::Console(const HorseRadish::IO::Path &fileOutputPath)
{
	HorseRadish::String strFormatted;

	//crio o logger para guardar tudo
	logger = new HorseRadish::Logging::Logger(HorseRadish::Logging::Logger::DefaultCapacityKB * 2, fileOutputPath);

	//por omissão faço echo de tudo
	this->doEchoLog = true;
	this->doEchoError = true;

	//não tenho nada para onde fazer callback
	this->loopbackFuncError = nullptr;

	//a árvore não existe
	this->treeConsole = new HorseRadish::AVLTree<DATA_NO>();

	//crio memória para os argumentos
	this->argList = new ARG_DATA[NUM_ARGUMENTS];

	//coloco para lá texto
	this->Log("<<====== CONSOLE INITIALIZED ======>>");
	this->Log(HorseRadish::String("<<====== Compiled: %s %s", __DATE__, __TIME__).GetData());
	strFormatted.SetCurrentDate();
	this->Log(HorseRadish::String("<<====== Date: %s", strFormatted.GetData()).GetData());
	strFormatted.SetCurrentTime();
	this->Log(HorseRadish::String("<<====== Time: %s", strFormatted.GetData()).GetData());

	/*if (consoleThread)
		SConsoleLog(HorseRadish::String("<<====== Thread will fire with intervals of %ds",TRIGGER_THREAD));
	else
		SConsoleLog("<<====== Thread not created (data will be saved only uppon closing)");*/
	this->Log("------------------------------------------------");
	this->Log("------------------------------------------------");
	this->Log(" ");
}

Console::~Console()
{
	//limpo os dados da consola
	if (this->treeConsole != nullptr)
		this->treeConsole->DeleteData();

	//já não preciso de comandos
	delete this->argList;
	this->argList = nullptr;

	//mando apagar a árvore
	delete this->treeConsole;
	this->treeConsole = nullptr;

	//mando terminar
	this->Terminate();
}

void Console::Terminate()
{
	HorseRadish::String strFormatted;

	//se já fechei
	if (logger == nullptr)
		return;

	//ultimas coisas
	this->Log(" ");
	this->Log("----------------------------------------------");
	this->Log("----------------------------------------------");
	strFormatted.SetCurrentDate();
	this->Log(HorseRadish::String("<<====== Date: %s", strFormatted.GetData()).GetData());
	strFormatted.SetCurrentTime();
	this->Log(HorseRadish::String("<<====== Time: %s", strFormatted.GetData()).GetData());
	this->Log("<<====== CONSOLE TERMINATED ======>>");

	//apago o logger
	if (logger != nullptr)
		delete logger;
	logger = nullptr;
}

void Console::SetEcho(const bool echoLog, const bool echoErrors)
{
	this->doEchoLog = echoLog;
	this->doEchoError = echoErrors;
}

void Console::PrintHelp(const char *what)
{
	DATA_NO *spec;
	char strAux[128];

	//se o what tiver vazio, imprimo simplesmente ajuda sobre esta consola
	if ((what == nullptr) || (*what == '\0'))
	{
		this->LogInfo("This console's help:");
		this->LogTab("> commands or variables can be TAB completed", 2);
		this->LogTab("> different expressions can be grouped with ';'", 2);
		this->LogTab("> expressions started with '}' don't echo in the console", 2);
		this->LogTab("> variable operations:", 2);
		this->LogTab("> var [ += | -= | *= | /= ] value", 3);
		this->LogTab("> var [ < | > | <= | >= | == | != ] value", 3);
		this->LogTab("> var ?[!] value", 3);
		this->LogTab("> var ? value : value", 3);
		return;
	}

	//tiro o que tiver a tirar
	spec = treeConsole->FindData(what);
	if (spec == nullptr)
		return;

	//se for um comando
	if (spec->isCommand)
	{
		strcpy_s(strAux, sizeof(strAux), "Command: ");
		strcat_s(strAux, sizeof(strAux), spec->nome);
		this->LogInfo(strAux);

		sprintf_s(strAux, sizeof(strAux), "ID: %d", spec->id);
		this->LogTab(strAux, 2);

		strcpy_s(strAux, sizeof(strAux), "Description: ");
		strcat_s(strAux, sizeof(strAux), spec->desc);
		this->LogTab(strAux, 2);
		return;
	}

	//é uma variável
	strcpy_s(strAux, sizeof(strAux), "Variable: ");
	strcat_s(strAux, sizeof(strAux), spec->nome);
	if (spec->varData.tipo == VarType::Integer)
		strcat_s(strAux, sizeof(strAux), " (integer)");
	else if (spec->varData.tipo == VarType::Float)
		strcat_s(strAux, sizeof(strAux), " (float)");
	else if (spec->varData.tipo == VarType::String)
		strcat_s(strAux, sizeof(strAux), " (string)");
	else
		strcat_s(strAux, sizeof(strAux), " (unkown)");
	this->LogInfo(strAux);

	sprintf_s(strAux, sizeof(strAux), "ID: %d", spec->id);
	this->LogTab(strAux, 2);

	strcpy_s(strAux, sizeof(strAux), "Description: ");
	strcat_s(strAux, sizeof(strAux), spec->desc);
	this->LogTab(strAux, 2);

	if (spec->printStr[0] != '\0')
	{
		sprintf_s(strAux, sizeof(strAux), "Print format: \"%s\"", spec->printStr);
		this->LogTab(strAux, 2);
	}
}

bool Console::RegisterBind(const int key, const int flagMod, const char *toWhat)
{
	BINDS *newBinds;

	//basta verificar isto
	if ( (key<SCONSOLE_KEY_A) || ((key-SCONSOLE_KEY_A)>=SCONSOLE_NUMKEYS) || toWhat==nullptr)
		return false;

	//preciso de alguma coisa pra registar
	if (toWhat[0]=='\0')
		return false;

	//procuro pela tecla
	for(int i=0, numElements = this->binds.GetNumElements(); i< numElements; i++)
	{
		//se encontrei, fixe, posso usar
		if (this->binds[i].key==key && this->binds[i].mod==flagMod)
		{
			strcpy_s(this->binds[i].cmd, 116, toWhat);
			return true;
		}
	}

	//preciso de mais espaço
	newBinds = this->binds.Add();
	if (newBinds == nullptr)
		return false;

	//escrevo o que tenho de escrever e prontos
	newBinds->key = key;
	newBinds->mod = flagMod;
	strcpy_s(newBinds->cmd, 116, toWhat);
	return true;
}

void Console::BindEmitKey(const int key, const int flagMod)
{
	BINDS *walker;

	//tenho de verificar isto
	if ( (key<SCONSOLE_KEY_A) || ((key-SCONSOLE_KEY_A)>=SCONSOLE_NUMKEYS))
		return;

	//procuro
	for(int i=0, numElements = this->binds.GetNumElements(); i< numElements; i++)
	{
		//se não for esta
		if (this->binds[i].key!=key || this->binds[i].mod!=flagMod)
			continue;

		//executo o comando e pronto
		this->Process(this->binds[i].cmd);
		return;
	}
}

void Console::BindClear()
{	
	//basta chamar isto
	this->binds.Clear(true);
}

void Console::FormatPrint(const char *bufferIn, char *bufferOut, const unsigned int bufferOutSize)
{
	const char *read;
	char *write, *end;
	DATA_NO *parametros;
	char auxBuffer[128];
	int i;

	//verificar parametros
	if (bufferIn == nullptr || bufferOut == nullptr || bufferOutSize == 0 || bufferIn[0] == '\0')
		return;

	//arranjar os ponteiros e começar a escrever
	read = bufferIn;
	write = bufferOut;
	end = bufferOut + bufferOutSize - 1;

	//basta ler
	while (write < end && *read != '\0')
	{
		//se não for [, copio e não faço mais nada
		if (*read != '[')
		{
			*(write++) = *(read++);
			continue;
		}

		//se for um duplo [, copio só o próximo
		if (read[1] == '[')
		{
			*(write++) = *(read++);
			read++;
			continue;
		}

		//chegando aqui é para substituir por o que à frente
		//leio o nome da variavel
		i = 0;
		read++;
		while (i < 63 && *read != ']' && *read != '\0')
			auxBuffer[i++] = *(read++);
		auxBuffer[i] = '\0';
		if (*read != '\0')
			read++;

		//se está vazio, passo à frente
		if (auxBuffer[0] == '\0')
			continue;

		//acho a variavel e se não existir, passo à frente
		parametros = treeConsole->FindData(auxBuffer);
		if ((parametros == nullptr) || (parametros->isCommand == true))
			continue;

		//escrevo a variavel e formato de string
		switch (parametros->varData.tipo)
		{
			case VarType::Integer:
				sprintf_s(auxBuffer, sizeof(auxBuffer), "%d", parametros->varData.value.integer);
				break;
			case VarType::Float:
				sprintf_s(auxBuffer, sizeof(auxBuffer), "%f", parametros->varData.value.numeric);
				break;
			case VarType::String:
				strcpy_s(auxBuffer, sizeof(auxBuffer), parametros->varData.value.string);
				break;
		}

		//calculo o tamanho do que tenho de escrever e se não tiver espaço, saio já
		i = strlen(auxBuffer);
		if ((end - write) < i)
			break;

		//escrevo e está tudo tratado
		memcpy(write, auxBuffer, 1);
		write += i;
	}

	//fecho e já não há mais nada a fazer
	*write = '\0';
}

void Console::TABComplete(const char *bufferIn, char *bufferOut, const unsigned int bufferOutSize, std::function<void (const HorseRadish::String &hit)> callbackHitAction)
{
	int hits;

	//se não tiver mais string
	if ((bufferIn == nullptr) || (*bufferIn)=='\0')
		return;

	//obtenho o numero de hits que existem para a string indicada
	hits=0;
	treeConsole->HitCount(bufferIn, hits, nullptr);
	if (hits <= 0)
		return;

	//completo a string no utilizador
	if (bufferOut != nullptr)
		treeConsole->Finish(bufferIn,bufferOut);

	//se tenho mais possibilidades e é para avisar
	if ((hits > 1) && callbackHitAction)
	{
		treeConsole->HitCount(bufferIn, hits, [&](DATA_NO *data)
		{
			callbackHitAction(HorseRadish::String(data->nome));
		});
	}
}

void Console::RegisterCommand(const unsigned int ID, const char *cmdName, const char *cmdDescription, CallbackCommand callbackFunc)
{
	DATA_NO *spec;

	//parametros
	if (cmdName == nullptr || cmdName[0] == '\0' || !callbackFunc)
		return;

	//se ele já existir
	if (treeConsole->FindData(cmdName) != nullptr)
	{
		this->LogError("There's already a comand or variable with that name.");
		this->LogError(HorseRadish::String("Unable to register \"%s\"", cmdName).GetData());
		return;
	}

	//crio uma especificação para o comando
	spec = new DATA_NO;
	spec->id = ID;
	spec->isCommand = true;
	strcpy_s(spec->nome, 32, cmdName);
	strcpy_s(spec->desc, 128, "<no description available>");
	if (cmdDescription != nullptr && cmdDescription[0] != '\0')
		strcpy_s(spec->desc, 128, cmdDescription);
	spec->cmdData.callback = callbackFunc;

	//adiciono à árvore
	treeConsole->Add(cmdName, spec);
}

bool Console::RegisterVariable(const unsigned int ID, const char *varName, const Console::VarType type, const char *varDescription, const char *printString, CallbackVariable callbackFunc)
{
	DATA_NO *parametros;

	//verificar parametros
	if (varName == nullptr || varName[0] == '\0')
		return false;

	//vejo se existe alguma entrada com este nome
	if (treeConsole->FindData(varName) != nullptr)
	{
		this->LogError("There's already a comand or variable with that name.");
		this->LogError(HorseRadish::String("Unable to create \"%s\"", varName).GetData());
		return false;
	}

	//crio uma especificação para a variável
	parametros = new DATA_NO;
	parametros->id = ID;
	parametros->isCommand = false;
	strcpy_s(parametros->nome, 32, varName);
	strcpy_s(parametros->desc, 128, "<no description available>");
	strcpy_s(parametros->printStr, 64, "");
	if (varDescription != nullptr && varDescription[0] != '\0')
		strcpy_s(parametros->desc, 128, varDescription);
	if (printString != nullptr && printString[0] != '\0')
		strcpy_s(parametros->printStr, 64, printString);

	parametros->varData.callback = callbackFunc;
	parametros->varData.tipo = type;
	parametros->varData.value.numeric = 0.0f;
	parametros->varData.value.integer = 0;
	parametros->varData.value.string[0] = '\0';
	parametros->varData.condition = false;
	parametros->varData.flags = (Console::VarFlags)0;

	//adiciono à árvore
	treeConsole->Add(varName, parametros);

	//tá tudo bem
	return true;
}

bool Console::VarSetDataI(const char *name, const int data)
{
	//verificar parametros
	if (name==nullptr || name[0]=='\0')
		return false;

	//acho e tento mudar a variavel
	return setVarI(treeConsole->FindData(name), data);
}

bool Console::VarSetDataF(const char *name, const float data)
{
	//verificar parametros
	if (name==nullptr || name[0]=='\0')
		return false;

	//acho e tento mudar a variavel
	return Console::setVarF(treeConsole->FindData(name), data);
}

bool Console::VarSetDataS(const char *name, const char *data)
{
	//verificar parametros
	if (name==nullptr || name[0]=='\0' || data==nullptr)
		return false;

	//acho e tento mudar a variavel
	return Console::setVarS(treeConsole->FindData(name), data);
}

void Console::VarSetAttrib(const char *name, const VarFlags flagGrant, const VarFlags flagDeny)
{
	DATA_NO *var;

	//verificar parametros
	if (name==nullptr)
		return;

	//acho a variável
	var = treeConsole->FindData(name);
	if ( (var==nullptr) || (var->isCommand==true) )
		return;

	//se tiver para dar ou retirar readonly
	if (flagGrant & VarFlags::Clamp)
		var->varData.flags = (Console::VarFlags)(var->varData.flags | VarFlags::Clamp);
	if (flagGrant & VarFlags::ReadOnly)
		var->varData.flags = (Console::VarFlags)(var->varData.flags | VarFlags::ReadOnly);

	if (flagDeny & VarFlags::Clamp)
		var->varData.flags = (Console::VarFlags)(var->varData.flags & !VarFlags::Clamp);
	if (flagDeny & VarFlags::ReadOnly)
		var->varData.flags = (Console::VarFlags)(var->varData.flags & !VarFlags::ReadOnly);
}

bool Console::VarGetType(const char *name, Console::VarType &varType)
{
	DATA_NO *parametros;

	//verificar parametros
	if (name==nullptr || name[0]=='\0')
		return false;

	//acho a variavel
	parametros = treeConsole->FindData(name);
	if ( (parametros==nullptr) || (parametros->isCommand==true) )
		return false;

	//guardo o tipo e posso sair
	varType = parametros->varData.tipo;
	return true;
}

int Console::VarGetDataI(const char *name)
{
	DATA_NO *parametros;

	//verificar parametros
	if (name==nullptr || name[0]=='\0')
		return -1;

	//acho a variavel
	parametros = treeConsole->FindData(name);
	if ( (parametros==nullptr) || (parametros->isCommand==true) )
		return -1;

	//se a variavel nao for int, nao vale a pena fazer nada
	if (parametros->varData.tipo != VarType::Integer)
		return -1;

	//tá tudo bem
	return parametros->varData.value.integer;
}

float Console::VarGetDataF(const char *name)
{
	DATA_NO *parametros;

	//verificar parametros
	if (name==nullptr || name[0]=='\0')
		return -1.0f;

	//acho a variavel
	parametros = treeConsole->FindData(name);
	if ( (parametros==nullptr) || (parametros->isCommand==true) )
		return -1.0f;

	//se a variavel nao for int, nao vale a pena fazer nada
	if (parametros->varData.tipo != VarType::Float)
		return -1.0f;

	//tá tudo bem
	return parametros->varData.value.numeric;
}

const char* Console::VarGetDataS(const char *name)
{
	DATA_NO *parametros;

	//verificar parametros
	if (name==nullptr || name[0]=='\0')
		return nullptr;

	//acho a variavel
	parametros = treeConsole->FindData(name);
	if ( (parametros==nullptr) || (parametros->isCommand==true) )
		return nullptr;

	//se a variavel nao for int, nao vale a pena fazer nada
	if (parametros->varData.tipo != VarType::String)
		return nullptr;

	//tá tudo bem
	return parametros->varData.value.string;
}

const char* Console::VarPrint(const char *varName, char *bufferOut, const int bufferSize)
{
	DATA_NO *parametros;

	//verificar parametros
	if (bufferOut==nullptr || bufferSize<=0 || varName==nullptr || varName[0]=='\0')
		return nullptr;

	//acho a variavel
	parametros = treeConsole->FindData(varName);
	if ( (parametros==nullptr) || (parametros->isCommand==true) )
		return nullptr;

	//se não tenho nada
	if (parametros->printStr[0]=='\0')
		return nullptr;

	//de acordo com o tipo
	if (parametros->varData.tipo == VarType::Integer)
	{
		_snprintf_s(bufferOut, bufferSize, _TRUNCATE, parametros->printStr, parametros->varData.value.integer);
		return bufferOut;
	}
	if (parametros->varData.tipo == VarType::Float)
	{
		_snprintf_s(bufferOut, bufferSize, _TRUNCATE, parametros->printStr, parametros->varData.value.numeric);
		return bufferOut;
	}
	if (parametros->varData.tipo == VarType::String)
	{
		_snprintf_s(bufferOut, bufferSize, _TRUNCATE, parametros->printStr, parametros->varData.value.string);
		return bufferOut;
	}

	//nunca deveria chegar aqui
	return nullptr;
}

void Console::PrintCommands(bool fullDescription)
{
	int numHits;

	//o que vou fazer
	this->Log("#66,245,10.Commands available:");

	//para cada entrada
	numHits = treeConsole->ForEachData([&](DATA_NO *dados)
	{
		//ignoro o que não é comando
		if (dados->isCommand == false)
			return;

		//vai para a consola
		this->LogTab(HorseRadish::String("%s - %s", dados->nome, dados->desc).GetData(), 2);
	});

	//mas se não escrevi nada
	if (numHits <= 0)
		this->LogTab("<none>", 2);
}

void Console::PrintVars(bool fullDescription)
{
	int numHits;

	//o que vou fazer
	this->Log("#66,245,10.Variables available:");

	//para cada entrada
	numHits = treeConsole->ForEachData([&](DATA_NO *dados)
	{
		//ignoro comandos (ou seja, quero as variáveis)
		if (dados->isCommand == true)
			return;

		//vai para a consola
		this->LogTab(HorseRadish::String("%s - %s", dados->nome, dados->desc).GetData(), 2);
	});

	//mas se não escrevi nada
	if (numHits <= 0)
		this->LogTab("<none>", 2);
}

void Console::PrintBinds()
{
	//o que vou fazer
	this->Log("#66,245,10.Binds available:");

	//se não tenho nada
	if (this->binds.GetNumElements() <= 0)
	{
		this->LogTab("<none>", 2);
		return;
	}

	//para cada bind
	for(int i=0, numElements = this->binds.GetNumElements(); i< numElements; i++)
	{
		//as letras
		if ( (this->binds[i].key>=SCONSOLE_KEY_A) && (this->binds[i].key<=SCONSOLE_KEY_Z) )
		{
			this->LogTab(HorseRadish::String("<key %c> %s",'A'+this->binds[i].key-SCONSOLE_KEY_A,this->binds[i].cmd).GetData(), 2);
			continue;
		}

		//os Fs
		if ( (this->binds[i].key>=SCONSOLE_KEY_F1) && (this->binds[i].key<=SCONSOLE_KEY_F12) )
		{
			this->LogTab(HorseRadish::String("<F%d> %s",this->binds[i].key-SCONSOLE_KEY_F1+1,this->binds[i].cmd).GetData(), 2);
			continue;
		}

		//os numeros
		if ( (this->binds[i].key>=SCONSOLE_KEY_0) && (this->binds[i].key<=SCONSOLE_KEY_9) )
		{
			this->LogTab(HorseRadish::String("<key %d> %s",this->binds[i].key-SCONSOLE_KEY_0,this->binds[i].cmd).GetData(), 2);
			continue;
		}

		//o resto
		switch(this->binds[i].key)
		{
			case SCONSOLE_KEY_ESCAPE:	this->LogTab(HorseRadish::String("<escape> %s", this->binds[i].cmd).GetData(), 2);break;
			case SCONSOLE_KEY_ENTER:	this->LogTab(HorseRadish::String("<enter> %s", this->binds[i].cmd).GetData(), 2);	break;
			case SCONSOLE_KEY_SPACE:	this->LogTab(HorseRadish::String("<space> %s", this->binds[i].cmd).GetData(), 2);	break;
			case SCONSOLE_KEY_TAB:		this->LogTab(HorseRadish::String("<tab> %s", this->binds[i].cmd).GetData(), 2);	break;
			case SCONSOLE_KEY_PAUSE:	this->LogTab(HorseRadish::String("<pause> %s", this->binds[i].cmd).GetData(), 2);	break;
			case SCONSOLE_KEY_UP:		this->LogTab(HorseRadish::String("<up> %s", this->binds[i].cmd).GetData(), 2);	break;
			case SCONSOLE_KEY_DOWN:		this->LogTab(HorseRadish::String("<down> %s", this->binds[i].cmd).GetData(), 2);	break;
			case SCONSOLE_KEY_LEFT:		this->LogTab(HorseRadish::String("<left> %s", this->binds[i].cmd).GetData(), 2);	break;
			case SCONSOLE_KEY_RIGHT:	this->LogTab(HorseRadish::String("<right> %s", this->binds[i].cmd).GetData(), 2);	break;
		}
	}
}

bool Console::Process(const char *command)
{
	const char *listaArg[NUM_ARGUMENTS + 1];
	bool forceNoEcho;
	int nArgumentos;
	DATA_NO *spec;

	//se não tenho nada para fazer
	if (command == nullptr || command[0] == '\0')
		return false;

	//se tiver vários comandos, é preciso dividir
	if (Console::isSeveralExpressions(command))
		return divideCommand(command);

	//verifico se o comando está a pedir para forçar a não ecoar
	forceNoEcho = Console::isForcedNoEcho(command);

	//adiciono esta frase ao log se for caso disso
	if (doEchoLog && !forceNoEcho)
	{
		if (this->addPhrase(command) == false)
			return false;
	}

	//se tive um forced echo, as próximas coias nao podem ver o }, logo...
	if (forceNoEcho)
	{
		while (*command != '}')
			command++;
		command++;
	}

	//mando ler as coisas
	if (parseCommand(command, argList, nArgumentos) == false)
		return false;

	//se está vazio
	if (nArgumentos <= 0 || argList[0].data[0] == '\0')
		return true;

	//procuro o que indicado
	spec = treeConsole->FindData(argList[0].data);
	if (spec == nullptr)
	{
		this->LogError("Command or variable unrecognized.");
		return true;
	}

	//se for um comando
	if (spec->isCommand)
	{
		int i;

		//crio a lista
		for (i = 0; i < nArgumentos && i < NUM_ARGUMENTS; i++)
			listaArg[i] = argList[i].data;

		//posso sempre isto, pq listaArg tem de tamanho (NUM_ARGUMENTS+1)
		listaArg[i] = nullptr;

		//chamo a função
		spec->cmdData.callback(*this, spec->id, nArgumentos, listaArg);
		return true;
	}

	//chegando aqui só pode ser uma variável
	//se nao tiver argumentos, escrevo o valor da variável
	if (nArgumentos == 1)
	{
		switch (spec->varData.tipo)
		{
			case VarType::Integer:
				this->LogInfo(HorseRadish::String("%s: %d", (const char*)spec->nome, spec->varData.value.integer).GetData());
				break;
			case VarType::Float:
				this->LogInfo(HorseRadish::String("%s: %f", (const char*)spec->nome, spec->varData.value.numeric).GetData());
				break;
			case VarType::String:
				if (spec->varData.value.string[0] == '\0')
					this->LogInfo(HorseRadish::String("%s: %s", (const char*)spec->nome, "<empty>").GetData());
				else
					this->LogInfo(HorseRadish::String("%s: %s", (const char*)spec->nome, (const char*)spec->varData.value.string).GetData());
				break;
		}
		return true;
	}

	//se tem 5 argumentos, só pode ser do tipo "var ? valor : valor"
	if (nArgumentos == 5)
	{
		//não posso se for readonly
		if (spec->varData.flags & VarFlags::ReadOnly)
		{
			this->LogError("Variable is read-only.");
			return true;
		}

		//posso fazer a cena
		doFiveArgVarCommand(spec, argList);
		return true;
	}

	//se tem 3 argumentos, pode ser do tipo "var += valor", "var <= valor", "var ?! valor" ou "var /= valor"
	//logo mando tratar dos argumentos (a função manda erros caso seja preciso)
	if (nArgumentos == 3)
	{
		doThreeArgVarCommand(spec, argList);
		return true;
	}

	//se tem 2 argumentos, então posso simplesmente transformar o argumento para o novo valor da variável
	if (nArgumentos == 2)
	{
		//não posso se for readonly
		if (spec->varData.flags & VarFlags::ReadOnly)
		{
			this->LogError("Variable is read-only.");
			return true;
		}

		//posso mudar o valor
		switch (spec->varData.tipo)
		{
			case VarType::Integer:
				Console::setVarI(spec, atoi(argList[1].data));
				break;
			case VarType::Float:
				Console::setVarF(spec, (float)atof(argList[1].data));
				break;
			case VarType::String:
				Console::setVarS(spec, argList[1].data);
				break;
		}
		return true;
	}

	//as coisas só têm 2 ou 3 argumentos (que são tratados em cima), logo chegando aqui é merda
	this->LogError("Error in the number of arguments: expected operation, condition or move to.");
	return true;
}

bool Console::Log(const char *s)
{
	int strLength;

	//verifico algumas coisas e se não for para registar
	if (s == nullptr || s[0] == '\0')
		return false;

	//se não faço log se for demasiado comprido
	strLength = strlen(s);
	if (strLength >= MAX_LOG_SIZE)
		return false;

	//dou entrada do texto
	return this->addPhrase(s);
}

bool Console::LogError(const char *s)
{
	char aux[MAX_LOG_SIZE];
	bool result;
	int strLength;

	//verifico algumas coisas e se não for para registar
	if (s == nullptr || s[0] == '\0')
		return false;

	//se não faço log se for demasiado comprido
	strLength = strlen(s);
	if ((strLength + 10) >= MAX_LOG_SIZE)
		return false;

	//faço a string
	strcpy_s(aux, MAX_LOG_SIZE, "#255,0,0.");
	memcpy(aux + 9, s, strLength + 1);

	//dou entrada do texto
	result = this->addPhrase(aux);

	//se tiver loopback para error, mando
	if (loopbackFuncError != nullptr)
		loopbackFuncError(*this, s);

	//e devolvo o resultado
	return result;
}

bool Console::LogInfo(const char *s)
{
	char aux[MAX_LOG_SIZE];
	int strLength;

	//verifico algumas coisas e se não for para registar
	if (s == nullptr || s[0] == '\0')
		return false;

	//se não faço log se for demasiado comprido
	strLength = strlen(s);
	if ((strLength + 13) >= MAX_LOG_SIZE)
		return false;

	//faço a string
	strcpy_s(aux, MAX_LOG_SIZE, "#20,255,100.");
	memcpy(aux + 12, s, strLength + 1);

	//dou entrada do texto
	return this->addPhrase(aux);
}

bool Console::LogTab(const char *s, const unsigned short numTabs)
{
	char aux[MAX_LOG_SIZE];
	int strLength, espacos;

	//verifico algumas coisas e se não for para registar
	if (s == nullptr || s[0] == '\0')
		return false;

	//quantos espaços vou ter na realidade
	espacos = Console::tabNumSpaces * numTabs;

	//se não faço log se for demasiado comprido
	strLength = strlen(s);
	if ((strLength + espacos + 1) >= MAX_LOG_SIZE)
		return false;

	//faço a string
	memset(aux, ' ', espacos);
	memcpy(aux + espacos, s, strLength + 1);

	//dou entrada do texto
	return this->addPhrase(aux);
}

bool Console::LogTabColor(const char *s, const unsigned short numTabs, const unsigned char colorR, const unsigned char colorG, const unsigned char colorB)
{
	char aux[MAX_LOG_SIZE];
	bool result;
	int strLength, espacos;

	//verifico algumas coisas e se não for para registar
	if (s == nullptr || s[0] == '\0')
		return false;

	//quantos espaços vou ter na realidade
	espacos = Console::tabNumSpaces * numTabs;

	//se não faço log se for demasiado comprido
	strLength = strlen(s);
	if ((strLength + espacos + 14) >= MAX_LOG_SIZE)
		return false;

	//faço a string
	sprintf(aux, "#%d,%d,%d.", colorR, colorG, colorB);
	for (int i = 0; i < espacos; i++)
		strcat(aux, " ");
	strcat(aux, s);

	//dou entrada do texto
	return this->addPhrase(aux);
}

bool Console::AsChanged()
{
	bool aux;

	aux = changeOccured;
	changeOccured = false;
	return aux;
}

void Console::LogLoopback(Console::CallbackLoop errorLoopbackFunc)
{
	//basta guardar a função
	loopbackFuncError = errorLoopbackFunc;
}

}//namespace Console
}//namespace HorseRadish