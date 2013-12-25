#include "UTF.hpp"
#include "Types.hpp"

#include <windows.h>

namespace HorseRadish
{

const HorseRadish::hChar UTF::BomUTF8[] = { 0xEF, 0xBB, 0xBF };
const HorseRadish::hChar UTF::BomUTF16BigE[] = { 0xFE, 0xFF };
const HorseRadish::hChar UTF::BomUTF16LittleE[] = { 0xFF, 0xFE };
const HorseRadish::hChar UTF::BomUTF32BigE[] = { 0x00, 0x00, 0xFE, 0xFF };
const HorseRadish::hChar UTF::BomUTF32LittleE[] = { 0xFF, 0xFE, 0x00, 0x00 };

bool UTF::validateChar(const unsigned char * const charPtr)
{
	int length;
	unsigned char a;
	const unsigned char *srcptr;

	//inicio isto
	length = trailingBytesUTF8[*charPtr] + 1;
	srcptr = charPtr + length;

	//toca a verificar
	switch (length)
	{
		default: return false;
	
		case 4: if ((a = (*--srcptr)) < 0x80 || a > 0xBF) return false;
		case 3: if ((a = (*--srcptr)) < 0x80 || a > 0xBF) return false;
		case 2: if ((a = (*--srcptr)) > 0xBF) return false;

		switch (*charPtr)
		{
			case 0xE0: if (a < 0xA0) return false; break;
			case 0xED: if (a > 0x9F) return false; break;
			case 0xF0: if (a < 0x90) return false; break;
			case 0xF4: if (a > 0x8F) return false; break;
			default:   if (a < 0x80) return false;
		}

		case 1: if (*charPtr >= 0x80 && *charPtr < 0xC2) return false;
	}

    return (*charPtr <= 0xF4);
}

int UTF::ValidateUTF8(const void *bufferUTF8, int *numChars, int *numBytes)
{
	unsigned short numChar;
	const unsigned char *walker;

	//não dá jeito acontecer isto
	if (bufferUTF8 == nullptr)
		return -1;

	//basta passar pela string toda e verificar se algum caracter está errado
	numChar=0;
	for(walker=(const unsigned char*)bufferUTF8; *walker!='\0'; walker+=trailingBytesUTF8[*walker]+1)
	{
		//se nao for válido
		if (validateChar(walker)==false)
			return 0;

		//mais um caracter
		numChar++;
	}

	//se for para guardar o numero de caracteres da string ou o numero de bytes
	if (numChars!=nullptr)
		*numChars=numChar;
	if (numBytes!=nullptr)
		*numBytes=walker-((const unsigned char*)bufferUTF8);

	//chegando aqui a string está correcta
	return 1;
}

int UTF::ConvertUTF8To(const String &stringIn, const Encoding targetEncoding, void *outBuffer, const int outBufferSize)
{
	//se não tenho nada pra onde escrever, facilita a vida
	if ((outBuffer == nullptr) || (outBufferSize <= 0))
		return -1;

	//se o encoding for widechar do windows
	if (targetEncoding == UTF::Windows)
	{
		//converto e pronto, tá tudo (a função já devolve o numero de caracteres convertidos)
		return MultiByteToWideChar(CP_UTF8, 0, (LPSTR)stringIn.GetData(), -1, (LPWSTR)outBuffer, outBufferSize);
	}

	//chegando aqui é barraca
	return 0;
}

int UTF::ConvertUTF8To(const void *bufferInUTF8, const Encoding targetEncoding, void *outBuffer, const int outBufferSize)
{
	//se não tenho nada donde ler ou pra onde escrever, facilita a vida
	if ((bufferInUTF8 == nullptr) || (outBuffer == nullptr) || (outBufferSize <= 0))
		return -1;

	//se o encoding for widechar do windows
	if (targetEncoding == UTF::Windows)
	{
		//converto e pronto, tá tudo (a função já devolve o numero de caracteres convertidos)
		return MultiByteToWideChar(CP_UTF8, 0, (LPSTR)bufferInUTF8, -1, (LPWSTR)outBuffer, outBufferSize);
	}

	//chegando aqui é barraca
	return 0;
}

int UTF::ConvertToUTF8(const Encoding inEncoding, const void *inBuffer, void *outBuffer, const int outBufferSize)
{
	//se não tenho nada donde ler ou pra onde escrever, facilita a vida
	if ((inBuffer == nullptr) || (outBuffer == nullptr) || (outBufferSize <= 0))
		return 0;

	//se o encoding for ASCII
	if (inEncoding == UTF::ASCII)
	{
		unsigned char *walker,curChar;
		int numBytes,numChars;
		char *writer;

		//preparo as coisas
		numBytes=numChars=0;
		writer=(char*)outBuffer;
		walker=(unsigned char*)inBuffer;

		//enquanto não copiar tudo
		while(*walker!=0)
		{
			curChar=*(walker++);
			
			//conforme o tipo
			if (curChar<=127)
			{
				*writer++=(char)curChar;
				numBytes++;
				numChars++;
			}
			else if (curChar<=191)
			{
				*writer++=0xC280;
				*writer++=(curChar-128);
				numBytes+=2;
				numChars++;
			}
			else 
			{
				*writer++=0xC380;
				*writer++=(curChar-192);
				numBytes+=2;
				numChars++;
			}
		}

		//fecho e basta devolver quantos caracteres li
		*writer='\0';
		return numChars;
	}
	
	//se o encoding for widechar de 2bytes
	if (inEncoding == UTF::UTF16)
	{
		unsigned short *walker;
		int numBytes,numChars;
		unsigned int curChar;
		char *writer;

		//preparo as coisas
		numBytes=numChars=0;
		writer=(char*)outBuffer;
		walker=(unsigned short*)inBuffer;

		//enquanto não copiar tudo
		while(*walker!=0)
		{
			curChar=(unsigned short)(*(walker++));

			//conforme o tipo
			if (curChar<0x80)
			{
				*writer++=(char)curChar;
				numBytes++;
				numChars++;
			}
			else if (curChar<0x800)
			{
				*writer++ = (curChar>>6) | 0xC0;
				*writer++ = (curChar & 0x3F) | 0x80;
				numBytes+=2;
				numChars++;
			}
			else
			{
				*writer++ = (curChar>>12) | 0xE0;
				*writer++ = ((curChar>>6) & 0x3F) | 0x80;
				*writer++ = (curChar & 0x3F) | 0x80;
				numBytes+=3;
				numChars++;
			}
		}

		//fecho e basta devolver quantos caracteres li
		*writer='\0';
		return numChars;
	}

	//se o encoding for widechar de 4bytes
	if (inEncoding == UTF::UTF32)
	{
		unsigned int *walker,curChar;
		int numBytes,numChars;
		char *writer;

		//preparo as coisas
		numBytes=numChars=0;
		writer=(char*)outBuffer;
		walker=(unsigned int*)inBuffer;

		//enquanto não copiar tudo
		while(*walker!=0)
		{
			curChar=*(walker++);

			//conforme o tipo
			if (curChar<0x80)
			{
				*writer++=(char)curChar;
				numBytes++;
				numChars++;
			}
			else if (curChar<0x800)
			{
				*writer++ = (curChar>>6) | 0xC0;
				*writer++ = (curChar & 0x3F) | 0x80;
				numBytes+=2;
				numChars++;
			}
			else if (curChar<0x10000)
			{
				*writer++ = (curChar>>12) | 0xE0;
				*writer++ = ((curChar>>6) & 0x3F) | 0x80;
				*writer++ = (curChar & 0x3F) | 0x80;
				numBytes+=3;
				numChars++;
			}
			else if (curChar<0x110000)
			{
				*writer++ = (curChar>>18) | 0xF0;
				*writer++ = ((curChar>>12) & 0x3F) | 0x80;
				*writer++ = ((curChar>>6) & 0x3F) | 0x80;
				*writer++ = (curChar & 0x3F) | 0x80;
				numBytes+=4;
				numChars++;
			}
		}

		//fecho e basta devolver quantos caracteres li
		*writer='\0';
		return numChars;
	}

	//se o encoding for widechar do windows
	if (inEncoding == UTF::Windows)
	{
		int numChars;

		//converto o encode para UTF8 e posso logo sair (a função devolve logo o numero de caracteres escritos, menos o do '\0')
		numChars = WideCharToMultiByte(CP_UTF8, 0, (LPCWSTR)inBuffer, -1, (LPSTR)outBuffer, outBufferSize, nullptr, nullptr);
		return (numChars-1);
	}

	//chegando aqui o encoding não é conhecido
	return 0;
}

const char UTF::trailingBytesUTF8[256] = {
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1, 1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
    2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2, 3,3,3,3,3,3,3,3,4,4,4,4,5,5,5,5
};

}//namespace HorseRadish