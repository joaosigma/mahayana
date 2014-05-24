#include "Platform.hpp"

#include <windows.h>
#include <stdio.h>
#include <memory.h>
#include <stdarg.h>
#include <time.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include "Math.hpp"
#include "String.hpp"
#include "Encoders.hpp"

namespace HorseRadish
{

const unsigned __int32 String::offsetsFromUTF8[6] = { 0x00000000UL, 0x00003080UL, 0x000E2080UL, 0x03C82080UL, 0xFA082080UL, 0x82082080UL};

const char String::trailingBytesUTF8[256] = {
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
	1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1, 1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
	2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2, 3,3,3,3,3,3,3,3,4,4,4,4,5,5,5,5
};

unsigned int String::lengthUTF8(const unsigned int &unicodeChar)
{
	if (unicodeChar<0x80)
		return 1;
	if (unicodeChar<0x800)
		return 2;
	if (unicodeChar<0x10000)
		return 3;
	if (unicodeChar<0x110000)
		return 4;
	return 0;
}

unsigned int String::unicode2utf8(const unsigned int &unicodeChar, HorseRadish::hChar * const bufferWrite)
{
	//escreve o caracter se for válido
	if (unicodeChar < 0x80)
	{
		bufferWrite[0] = (char)unicodeChar;
		return 1;
	}
	if (unicodeChar < 0x800)
	{
		bufferWrite[0] = (unicodeChar>>6) | 0xC0;
		bufferWrite[1] = (unicodeChar & 0x3F) | 0x80;
		return 2;
	}
	if (unicodeChar < 0x10000)
	{
		bufferWrite[0] = (unicodeChar>>12) | 0xE0;
		bufferWrite[1] = ((unicodeChar>>6) & 0x3F) | 0x80;
		bufferWrite[2] = (unicodeChar & 0x3F) | 0x80;
		return 3;
	}
	if (unicodeChar < 0x110000)
	{
		bufferWrite[0] = (unicodeChar>>18) | 0xF0;
		bufferWrite[1] = ((unicodeChar>>12) & 0x3F) | 0x80;
		bufferWrite[2] = ((unicodeChar>>6) & 0x3F) | 0x80;
		bufferWrite[3] = (unicodeChar & 0x3F) | 0x80;
		return 4;
	}

	//oops
	bufferWrite[0]='\0';
	return 0;
}

bool String::validateChar(const HorseRadish::hChar * const charPtr)
{
	int length;
	HorseRadish::hUInt8 a;
	const HorseRadish::hChar *srcptr;

	//inicio isto
	length = String::trailingBytesUTF8[(HorseRadish::hUInt8)*charPtr]+1;
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

    if (*charPtr > 0xF4)
		return false;
    return true;
}

bool String::validateString(const HorseRadish::hChar * const stringData, unsigned short * const dataCharCount, unsigned short * const dataByteCount)
{
	unsigned short numChar;
	const HorseRadish::hChar *walker;

	//basta passar pela string toda e verificar se algum caracter está errado
	numChar = 0;
	for(walker = stringData; *walker!='\0'; walker += String::trailingBytesUTF8[(HorseRadish::hUInt8)*walker] + 1)
	{
		//se nao for válido
		if (String::validateChar(walker)==false)
			return false;

		//mais um caracter
		numChar++;
	}

	//se for para guardar o numero de caracteres da string e/ou o numero de bytes
	if (dataCharCount != nullptr)
		*dataCharCount = numChar;
	if (dataByteCount != nullptr)
		*dataByteCount = (unsigned short)(walker - stringData);

	//chegando aqui a string está correcta
	return true;
}

inline
bool String::isutf8(const HorseRadish::hChar &value)
{
	return ((value & 0xC0) != 0x80);
}

void String::calcStrParams()
{
	//verifico o caso mais óbvio
	if (*this->actualBuffer=='\0')
	{
		this->numBytes = this->numChars = 0;
		return;
	}

	//por omissão
	this->numChars = 0;

	//percorro toda a string (e posso já calcular o número de caracteres)
	HorseRadish::hChar *walker = this->actualBuffer;
	for(; *walker != '\0'; walker += String::trailingBytesUTF8[(HorseRadish::hUInt8)*walker] + 1)
		this->numChars++;

	//guardo os dados
	this->numBytes = (unsigned short)(walker - this->actualBuffer);

	//ajusto o número de bytes se tiver um buffer "externo"
	if (this->bufferHeap != nullptr)
		this->bufferHeap->Shrink(this->bufferHeap->GetNumElements() - this->numBytes - 1);
}

bool String::adjustRequiredBuffer(const unsigned int &numBytesRequired, const bool &copyData)
{
	//se não há nada a fazer
	if (this->numBytes == numBytesRequired)
		return true;

	//se for para acrescentar
	if (this->numBytes < numBytesRequired)
	{
		//se não poder colocar tudo no buffer local
		if ((numBytesRequired + 1) > sizeof(this->bufferLocal))
		{
			//se ainda não tenho um buffer externo, crio-o
			if (this->bufferHeap == nullptr)
				this->bufferHeap = new HorseRadish::Containers::Array<HorseRadish::hChar>(numBytesRequired + 1);

			//verifico se tenho buffer e espaço para o que preciso
			if ((this->bufferHeap == nullptr) || (this->bufferHeap->Reserve(numBytesRequired + 1) == false))
				return false;

			//se for para copiar as coisas de um lado para o outro
			if ((copyData == true) && (this->numBytes > 0) && (this->actualBuffer == this->bufferLocal))
				memcpy(this->bufferHeap->GetMainPointer(), this->bufferLocal, this->numBytes + 1);

			//passo a usar o buffer externo
			this->bufferLocal[0] = '\0';
			this->actualBuffer = this->bufferHeap->GetMainPointer();
			return true;
		}

		//chegando aqui cabe tudo no buffer local, mas é melhor não alterar nada
		return true;
	}

	//chegando aqui é para diminuir, mas não faço nada a não ser libertar bytes do buffer externo se ele existir
	if (this->bufferHeap != nullptr)
		this->bufferHeap->Shrink(this->bufferHeap->GetNumElements() - numBytesRequired - 1);

	//correu sempre tudo bem
	return true;
}

String::String()
{
	//por omissão a string está vazia
	this->bufferLocal[0] = '\0';
	this->bufferHeap = nullptr;
	this->actualBuffer = this->bufferLocal;
	this->numBytes = this->numChars = 0;
}

String::String(const String &s)
	: String()
{
	//esta função trata de tudo
	this->Set(s);
}

String::String(String &&s)
{
	this->bufferHeap = s.bufferHeap;
	this->numBytes = s.numBytes;
	this->numChars = s.numChars;

	if (s.actualBuffer == s.bufferLocal)
	{
		this->actualBuffer = this->bufferLocal;
		memcpy(this->bufferLocal, s.bufferLocal, s.numBytes);
	}
	else
	{
		this->actualBuffer = this->bufferHeap->GetMainPointer();
		this->bufferLocal[0] = '\0';
	}

	s.actualBuffer = nullptr;
	s.bufferHeap = nullptr;
	s.numBytes = s.numChars = 0;
}

String::String(const char *fmt, ...)
	: String()
{
	va_list ap;
	int bytesWritten;
	char auxBuffer[2048];

	//tenho de ter alguma coisa
	if (fmt == nullptr)
		return;

	//fazer o parse
	va_start(ap, fmt);
		bytesWritten = vsnprintf_s(auxBuffer, sizeof(auxBuffer), _TRUNCATE, fmt, ap);
	va_end(ap);

	//se não tenho nada
	if (bytesWritten <= 0)
		return;

	//mando ajustar o buffer a copiar
	if (this->adjustRequiredBuffer(bytesWritten, false) == false)
		return;

	//basta copiar os dados
	memcpy(this->actualBuffer, auxBuffer, bytesWritten + 1);

	//verifico se a string é válida e se não for limpo-a
	if (String::validateString(this->actualBuffer, &this->numChars, &this->numBytes) == false)
		this->SetEmpty();
}

String::~String()
{
	//se tiver um buffer externo
	if (this->bufferHeap != nullptr)
		delete this->bufferHeap;

	//basta limpar tudo
	this->bufferLocal[0] = '\0';
	this->bufferHeap = nullptr;
	this->actualBuffer = nullptr;;
	this->numBytes = this->numChars = 0;
}

int String::Convert(const Encoding flagFormat, void * const outBuffer, const unsigned int outSize) const
{
	//se nada tenho onde escrever, facilita a vida
	if ((outBuffer == nullptr) || (outSize == 0))
		return 0;

	//********
	//se for para converter para ASCII
	if (flagFormat == String::ASCII)
	{
		unsigned int curChar;
		int countChars, bufferLeft;
		HorseRadish::hChar *writer;

		//preparo algumas coisas
		countChars = 0;
		writer = (HorseRadish::hChar*)outBuffer;
		bufferLeft = outSize;

		//enquanto tiver coisas para ler e espaço para as escrever
		Iterator it(*this);
		while (((curChar = it.Read()) != 0) && (bufferLeft > 0))
		{
			if (curChar<=0x7F)
			{
				writer[countChars] = (char)curChar;
				countChars++;
				bufferLeft--;
				continue;
			}
			if (curChar<=0xC2BF)
			{
				writer[countChars] = (char)(curChar - 0xC280 + 128);
				countChars++;
				bufferLeft--;
				continue;
			}
			if (curChar<=0xC3BF)
			{
				writer[countChars] = (char)(curChar - 0xC380 + 192);
				countChars++;
				bufferLeft--;
				continue;
			}
		}

		//fecho e basta devolver quantos caracteres consegui converter
		writer[countChars] = '\0';
		return countChars;
	}

	//********
	//se for para converter em widechar de 4bytes 
	if (flagFormat == String::UTF32)
	{
		unsigned int *writer, curChar, countChars;

		//preparo algumas coisas
		writer = (unsigned int*)outBuffer;
		countChars = 0;

		//enquanto tiver coisas para ler e espaço para as escrever
		Iterator it(*this);
		while( ((curChar=it.Read()) != 0) && ((countChars*4) < outSize) )
			writer[countChars++] = curChar;

		//fecho e basta devolver quantos caracteres consegui converter
		writer[countChars] = '\0';
		return countChars;
	}

	//se o encoding for widechar do windows
	if (flagFormat == String::Windows)
	{
		//converto e pronto, tá tudo (a função já devolve o numero de caracteres convertidos)
		return MultiByteToWideChar(CP_UTF8, 0, (LPCSTR)this->actualBuffer, -1, (LPWSTR)outBuffer, outSize);
	}

	//chegando aqui é barraca
	return 0;
}


void String::Trim(const unsigned int unicodeChar)
{
	//avanço até ao fim da string ou enquanto tiver caracteres que quero eliminar
	Iterator itStart(*this);
	for(; itStart.IsLast() == false; ++itStart)
	{
		if (*itStart != unicodeChar)
			break;
	}

	//se cheguei ao fim da string é para limpar tudo
	if (itStart.IsLast() == true)
	{
		this->SetEmpty();
		return;
	}

	//faço o mesmo mas começo no fim da string e recuo
	Iterator itEnd(*this);
	itEnd.Last();
	for(; itEnd.IsFirst() == false; --itEnd)
	{
		if (*itEnd != unicodeChar)
			break;
		if (itEnd <= itStart)
			break;
	}

	//se ultrapassei ou cheguei ao inicio, é pra limpar a string
	if (itEnd < itStart)
	{
		this->SetEmpty();
		return;
	}

	//avanço um porque quero escrever aquele para onde estou a apontar
	++itEnd;

	//se não é preciso fazer nada
	if ((itStart.IsFirst() == true) && (itEnd.IsLast() == true))
		return;

	//basta copiar as coisas entre itStart e itEnd
	int i = 0;
	for(int curPos = itStart.byteIndex; curPos < itEnd.byteIndex; i++,curPos++)
		this->actualBuffer[i] = this->actualBuffer[curPos];
	this->actualBuffer[i] = '\0';

	//calculo alguns parametros
	calcStrParams();
}

void String::Reverse()
{
	HorseRadish::hChar *walker;
	String strTmp;

	//faço uma cópia de mim mesmo para poder começar a copiar as coisas
	strTmp.Set(*this);

	//basta copiar cada um dos caracteres
	walker = this->actualBuffer;
	for(Iterator it(strTmp); true; --it)
	{
		walker += String::unicode2utf8(*it, walker);
		if (it.IsFirst() == true)
			break;
	}

	//isto tem de se verificar (o fim da string tem de ser o mesmo)
	assert(*walker == '\0');
}

void String::MakeLowerCase()
{
	//para todos os bytes
	for(int i=0; i<this->numBytes; i++)
	{
		if ((this->actualBuffer[i] >= 'A') && (this->actualBuffer[i] <= 'Z'))
			this->actualBuffer[i] += 'a'-'A';
	}
}

void String::MakeUpperCase()
{
	//para todos os bytes
	for(int i=0; i<this->numBytes; i++)
	{
		if ((this->actualBuffer[i] >= 'a') && (this->actualBuffer[i] <= 'z'))
			this->actualBuffer[i] -= 'a'-'A';
	}
}

void String::Replace(const unsigned int unicodeOldChar, const unsigned int unicodeNewChar)
{
	//se ambos os caracteres tiverem o mesmo tamanho em byes quando convertidos para utf8
	if (String::lengthUTF8(unicodeOldChar) == String::lengthUTF8(unicodeNewChar))
	{
		HorseRadish::hChar bufferNew[8];
		unsigned int charNumBytes;

		//converto o novo caracter para utf8
		charNumBytes = String::unicode2utf8(unicodeNewChar, bufferNew);

		//agora passo por todos os caracteres e mudo
		for(Iterator it(*this); it.IsLast() == false; ++it)
		{
			//se não for este
			if (*it != unicodeOldChar)
				continue;
			
			//basta escrever em cima do outro
			memcpy(this->actualBuffer + it.byteIndex, bufferNew, charNumBytes);
		}

		//e já tá tudo
		return;
	}

	//tenho mesmo de ter uma strAuxiliar
	String novaStr;
	unsigned int curChar;

	//não tenho hipotese, tenho de comecar a passar por todos os caracteres e verificar um a um
	for(Iterator it(*this); (curChar = it.Read()) != '\0'; )
	{
		//se for o que é pra trocar
		if (curChar == unicodeOldChar)
		{
			novaStr += unicodeNewChar;
			continue;
		}
	
		//basta acrescentar
		novaStr += curChar;
	}

	//basta guardar esta string
	this->Set(novaStr);
}

void String::GetSubStr(String &subStr, const unsigned int startChar, const unsigned int numChar) const
{
	int numCharsCopied;
	const HorseRadish::hChar *walker, *walkerEnd;

	//por omissão limpo a string
	subStr.SetEmpty();

	//o start e numChar têm de ser válidos
	if ((startChar >= this->numChars) || (numChar == 0))
		return;

	//avanço até onde tenho de avançar
	walker = this->actualBuffer;
	for(unsigned int i=0; i<startChar; i++)
		walker += String::trailingBytesUTF8[(HorseRadish::hUInt8)*walker] + 1;

	//agora avanço até ao fim da string ou ao número de caracteres
	numCharsCopied = 0;
	walkerEnd = walker;
	for(unsigned int i=0; (i < numChar) && (*walkerEnd != '\0'); i++, numCharsCopied++)
		walkerEnd += String::trailingBytesUTF8[(HorseRadish::hUInt8)*walkerEnd] + 1;

	//preciso de espaço para copiar as coisas
	if (subStr.adjustRequiredBuffer((unsigned int)(walkerEnd - walker), false) == false)
		return;

	//copio o conteúdo
	memcpy(subStr.actualBuffer, walker, walkerEnd - walker);

	//e só arranjar os número
	subStr.numChars = numCharsCopied;
	subStr.numBytes = (unsigned short)(walkerEnd - walker);
	subStr.actualBuffer[subStr.numBytes] = '\0';
}

String String::GetSubStr(const unsigned int startChar, const unsigned int numChar) const
{
	String subStr;
	
	//chamo a função e devolvo a string
	this->GetSubStr(subStr, startChar, numChar);
	return subStr;
}

int String::GetCharByteIndex(const unsigned int pos) const
{
	//se for o inicio
	if (pos == 0)
		return 0;

	//basta ler cada caracter e quando chegar à posição correcta, posso sair
	for(Iterator it(*this); it.IsLast() == false; ++it)
	{
		if (it.charIndex == pos)
			return it.byteIndex;
	}

	//nada feito, a posicao é inválida
	return -1;
}

int String::GetCharPosIndex(const unsigned int byte) const
{
	//se for o inicio
	if (byte == 0)
		return 0;

	//basta ler cada caracter e quando chegar à posição correcta, posso sair
	for(Iterator it(*this); it.IsLast() == false; ++it)
	{
		if (it.byteIndex == byte)
			return it.charIndex;
	}

	//nada feito, o byte é inválido
	return -1;
}

String& String::operator=(const String& s)
{
	//basta chamar esta função
	this->Set(s);

	//devolvo-me
	return *this;
}

const String String::operator+(const String& s) const
{
	String aux;

	//ajusto o buffer da string para o número de bytes pedidos
	if (aux.adjustRequiredBuffer(this->numBytes + s.numBytes, false) == false)
		return aux;

	//copio a string
	memcpy(aux.actualBuffer, this->actualBuffer, this->numBytes);
	memcpy(aux.actualBuffer + this->numBytes, s.actualBuffer, s.numBytes + 1);
	
	//ajusto os parametros
	aux.numBytes = this->numBytes + s.numBytes;
	aux.numChars = this->numChars + s.numChars;
	return aux;
}

String& String::operator+=(const String& s)
{
	//se não tenho nada a fazer
	if (s.numBytes <= 0)
		return *this;

	//ajusto o buffer desta string para o número de bytes pedidos
	if (this->adjustRequiredBuffer(this->numBytes + s.numBytes, true) == false)
		return *this;

	//basta copiar os dados da outra string para esta
	memcpy(this->actualBuffer + this->numBytes, s.actualBuffer, s.numBytes + 1);
	this->numBytes += s.numBytes;
	this->numChars += s.numChars;

	//devolvo-me
	return *this;
}

String& String::operator+=(const HorseRadish::hChar * const s)
{
	String strAux;

	//se não tenho nada a fazer
	if ((s == nullptr) || (*s == '\0'))
		return *this;

	//para simplificar
	strAux.Set(UTF8, s);

	//ajusto o buffer desta string para o número de bytes pedidos
	if (this->adjustRequiredBuffer(this->numBytes + strAux.numBytes, true) == false)
		return *this;

	//basta copiar os dados da outra string para esta
	memcpy(this->actualBuffer + this->numBytes, strAux.actualBuffer, strAux.numBytes + 1);
	this->numBytes += strAux.numBytes;
	this->numChars += strAux.numChars;

	//devolvo-me
	return *this;
}

String& String::operator+=(const unsigned int unicodeChar)
{
	//se não tenho nada a fazer
	if (unicodeChar <= 0)
		return *this;

	//ajusto o buffer desta string para o número de bytes necessários
	if (this->adjustRequiredBuffer(this->numBytes + String::lengthUTF8(unicodeChar), true) == false)
		return *this;

	//onde devo começar a escrever
	HorseRadish::hChar *writer = this->actualBuffer + this->numBytes;

	//de acordo com o caracter
	if (unicodeChar<0x80)
	{
		*writer++ = (char)unicodeChar;
		this->numBytes++;
		this->numChars++;
	}
	else if (unicodeChar<0x800)
	{
		*writer++ = (unicodeChar>>6) | 0xC0;
		*writer++ = (unicodeChar & 0x3F) | 0x80;
		this->numBytes+=2;
		this->numChars++;
	}
	else if (unicodeChar<0x10000)
	{
		*writer++ = (unicodeChar>>12) | 0xE0;
		*writer++ = ((unicodeChar>>6) & 0x3F) | 0x80;
		*writer++ = (unicodeChar & 0x3F) | 0x80;
		this->numBytes+=3;
		this->numChars++;
	}
	else if (unicodeChar<0x110000)
	{
		*writer++ = (unicodeChar>>18) | 0xF0;
		*writer++ = ((unicodeChar>>12) & 0x3F) | 0x80;
		*writer++ = ((unicodeChar>>6) & 0x3F) | 0x80;
		*writer++ = (unicodeChar & 0x3F) | 0x80;
		this->numBytes+=4;
		this->numChars++;
	}

	//fecho a string
	*writer='\0';

	//devolvo-me
	return *this;
}

bool String::operator==(const String& s) const
{
	//se tiverem caracteres ou bytes diferentes, são diferentes de certeza
	if ((this->numBytes != s.numBytes) || (this->numChars != s.numChars))
		return false;

	//toca a avancar até ler tudo
	for(Iterator it1(*this), it2(s); it1.IsLast() == false; ++it1, ++it2)
	{
		//se os caracteres forem diferentes
		if (*it1 != *it2)
			return false;
	}

	//chegando aqui são iguais de certeza
	return true;
}

bool String::operator!=(const String& s) const
{
	//basta chamar o == e negar o resultado
	return (!((*this) == s));
}

bool String::operator==(const HorseRadish::hChar * const s) const
{
	//assumo sempre que nullptr não é igual
	if (s == nullptr)
		return false;

	//basta devolver a comparação
	return (strcmp((const char*)this->actualBuffer, (const char*)s) == 0);
}

bool String::operator!=(const HorseRadish::hChar * const s) const
{
	//basta chamar o == e negar o resultado
	return (!((*this) == s));
}

bool String::operator==(const char * const s) const
{
	//assumo sempre que nullptr não é igual
	if (s == nullptr)
		return false;

	//basta devolver a comparação
	return (strcmp((const char*)this->actualBuffer, (const char*)s) == 0);
}

bool String::operator!=(const char * const s) const
{
	//basta chamar o == e negar o resultado
	return (!((*this) == s));
}

unsigned int String::operator[](int index) const
{
	return this->GetUnicodeAt(index);
}

unsigned int& String::operator[](int index)
{
	auto charAt = this->GetUnicodeAt(index);
	return charAt;
}

bool String::StartsWith(const String &compareTo) const
{
	//se eu como string sou mais pequena, de certeza que não começo com a string indicada
	if (this->numBytes < compareTo.numBytes)
		return false;

	//toca a procurar
	for(unsigned short i=0; i<compareTo.numBytes && compareTo.actualBuffer[i]!='\0'; i++)
	{
		if (this->actualBuffer[i] != compareTo.actualBuffer[i])
			return false;
	}

	//chegando aqui tá tudo bem
	return true;
}

bool String::StartsWith(const HorseRadish::hChar * const compareTo) const
{
	//se não há nada a comparar
	if ((compareTo == nullptr) || (*compareTo == '\0'))
		return false;

	//toca a procurar
	for(unsigned short i = 0; ((i < this->numBytes) && (this->actualBuffer[i] != '\0')); i++)
	{
		//se já cheguei ao fim da string
		if (compareTo[i] == '\0')
			return true;

		//se o carácter é diferente
		if (this->actualBuffer[i] != compareTo[i])
			return false;
	}

	//chegando aqui a string a comparar é maior do que a existente portanto não começa
	return false;
}

bool String::StartsWith(const unsigned int &unicodeChar) const
{
	//preciso de um iterator
	Iterator it(*this);

	//basta ler o primeiro caracter e verificar se é o indicado
	return (it.Read() == unicodeChar);
}

bool String::EndsWith(const String &compareTo) const
{
	//se eu como string sou mais pequena, de certeza que não começo com a string indicada
	if (this->numBytes < compareTo.numBytes)
		return false;

	//toca a procurar
	for(int i = compareTo.numBytes - 1, j = this->numBytes - 1; i >= 0; i--, j--)
	{
		if (this->actualBuffer[j] != compareTo.actualBuffer[i])
			return false;
	}

	//chegando aqui tá tudo bem
	return true;
}

bool String::EndsWith(const unsigned int &unicodeChar) const
{
	//preciso de um iterator
	Iterator it(*this);

	//ando para trás
	it--;

	//basta ler o caracter e verificar se é o indicado
	return (it.Read() == unicodeChar);
}

void String::SetEmpty()
{
	//basta limpar tudo
	this->bufferLocal[0]='\0';
	if (this->bufferHeap != nullptr)
		delete this->bufferHeap;
	this->bufferHeap = nullptr;
	this->actualBuffer = this->bufferLocal;
	this->numBytes = this->numChars = 0;
}

int String::Set(const Encoding &flagFormat, const void * const nova, const int &numCharToEncode)
{
	unsigned int metaNumChar;

	//limpo sempre tudo
	this->numBytes = this->numChars = 0;
	this->bufferLocal[0]='\0';
	if (this->bufferHeap != nullptr)
		this->bufferHeap->Clear();
	this->actualBuffer = this->bufferLocal;

	//se não tenho nada
	if ((nova == nullptr) || ((*((const char *)nova)) == '\0') || (numCharToEncode == 0))
		return 0;

	//se tiver valores negativos é para fazer tudo
	metaNumChar = (numCharToEncode < 0) ? 0xFFFFFFFF : numCharToEncode;
	metaNumChar = (metaNumChar > 0xFFFFFFFF) ? 0xFFFFFFFF : metaNumChar;

	//se o encoding for UTF8
	if (flagFormat == String::UTF8)
	{
		int srcLength;

		//verifico o tamanho em bytes da nova string e tento ajustar o espaço necessário
		srcLength = strlen((const char*)nova);
		if (this->adjustRequiredBuffer(srcLength, false) == false)
			return 0;

		//copio os dados
		memcpy(this->actualBuffer, nova, srcLength + 1);

		//verifico se a string é válida
		if (String::validateString(this->actualBuffer, &this->numChars,&this->numBytes)==false)
		{
			this->SetEmpty();
			return -1;
		}

		//se for para fechar a string numa dada posição
		if (metaNumChar < 0xFFFFFFFF)
			this->CloseAt(metaNumChar);

		//tá tudo correcto, basta
		return this->numChars;
	}

	//se o encoding for ASCII
	if (flagFormat == String::ASCII)
	{
		HorseRadish::hChar curChar;

		//enquanto não copiar tudo
		for(unsigned char *walker = (unsigned char*)nova; *walker != '\0'; walker++)
		{
			//se já cheguei ao limite
			if (this->numChars >= metaNumChar)
				break;

			//enquanto tiver abaixo do nível do buffer local
			if ((this->numBytes + 3) > sizeof(this->bufferLocal))
			{
				//tenho de mandar ajustar o tamanho
				if (this->adjustRequiredBuffer(this->numBytes + 2, true) == false)
				{
					this->SetEmpty();
					return -1;
				}
			}

			//leio o caracter e faço os ajustes de acordo
			curChar = *walker;
			if (curChar <= 127)
			{
				this->actualBuffer[this->numBytes] = (char)curChar;
				this->numBytes++;
				this->numChars++;
				continue;
			}
			if (curChar <= 191)
			{
				this->actualBuffer[this->numBytes + 0] = 0xC280;
				this->actualBuffer[this->numBytes + 1] = curChar - 128;
				this->numBytes += 2;
				this->numChars++;
				continue;
			}
			this->actualBuffer[this->numBytes + 0] = 0xC380;
			this->actualBuffer[this->numBytes + 1] = curChar - 192;
			this->numBytes += 2;
			this->numChars++;
		}

		//fecho a string e devolvo quantos caracteres li
		this->actualBuffer[this->numBytes] = '\0';
		return this->numChars;
	}
	
	//se o encoding for widechar de 2bytes
	if (flagFormat == String::UTF16)
	{
		unsigned short curChar;

		//enquanto não copiar tudo
		for(unsigned short *walker = (unsigned short*)nova; *walker != '\0'; walker++)
		{
			//se já cheguei ao limite
			if (this->numChars >= metaNumChar)
				break;

			//enquanto tiver abaixo do nível do buffer local
			if ((this->numBytes + 4) > sizeof(this->bufferLocal))
			{
				//tenho de mandar ajustar o tamanho
				if (this->adjustRequiredBuffer(this->numBytes + 3, true) == false)
				{
					this->SetEmpty();
					return -1;
				}
			}

			//leio o caracter e faço os ajustes de acordo
			curChar = *walker;
			if (curChar < 0x80)
			{
				this->actualBuffer[this->numBytes] = (char)curChar;
				this->numBytes++;
				this->numChars++;
				continue;
			}
			if (curChar < 0x800)
			{
				this->actualBuffer[this->numBytes + 0] = (curChar>>6) | 0xC0;
				this->actualBuffer[this->numBytes + 1] = (curChar & 0x3F) | 0x80;
				this->numBytes += 2;
				this->numChars++;
				continue;
			}
			this->actualBuffer[this->numBytes + 0] = (curChar>>12) | 0xE0;
			this->actualBuffer[this->numBytes + 1] = ((curChar>>6) & 0x3F) | 0x80;
			this->actualBuffer[this->numBytes + 2] = (curChar & 0x3F) | 0x80;
			this->numBytes += 3;
			this->numChars++;
		}

		//fecho a string e devolvo quantos caracteres li
		this->actualBuffer[this->numBytes] = '\0';
		return this->numChars;
	}

	//se o encoding for widechar de 4bytes
	if (flagFormat == String::UTF32)
	{
		unsigned int curChar;

		//enquanto não copiar tudo
		for(unsigned int *walker = (unsigned int*)nova; *walker != '\0'; walker++)
		{
			//se já cheguei ao limite
			if (this->numChars >= metaNumChar)
				break;

			//enquanto tiver abaixo do nível do buffer local
			if ((this->numBytes + 5) > sizeof(this->bufferLocal))
			{
				//tenho de mandar ajustar o tamanho
				if (this->adjustRequiredBuffer(this->numBytes + 4, true) == false)
				{
					this->SetEmpty();
					return -1;
				}
			}

			//leio o caracter e faço os ajustes de acordo
			curChar = *walker;
			if (curChar < 0x80)
			{
				this->actualBuffer[this->numBytes] = (char)curChar;
				this->numBytes++;
				this->numChars++;
			}
			else if (curChar < 0x800)
			{
				this->actualBuffer[this->numBytes + 0] = (curChar>>6) | 0xC0;
				this->actualBuffer[this->numBytes + 1] = (curChar & 0x3F) | 0x80;
				this->numBytes+=2;
				this->numChars++;
			}
			else if (curChar < 0x10000)
			{
				this->actualBuffer[this->numBytes + 0] = (curChar>>12) | 0xE0;
				this->actualBuffer[this->numBytes + 1] = ((curChar>>6) & 0x3F) | 0x80;
				this->actualBuffer[this->numBytes + 2] = (curChar & 0x3F) | 0x80;
				this->numBytes+=3;
				this->numChars++;
			}
			else if (curChar < 0x110000)
			{
				this->actualBuffer[this->numBytes + 0] = (curChar>>18) | 0xF0;
				this->actualBuffer[this->numBytes + 1] = ((curChar>>12) & 0x3F) | 0x80;
				this->actualBuffer[this->numBytes + 2] = ((curChar>>6) & 0x3F) | 0x80;
				this->actualBuffer[this->numBytes + 3] = (curChar & 0x3F) | 0x80;
				this->numBytes+=4;
				this->numChars++;
			}
		}

		//fecho a string e devolvo quantos caracteres li
		this->actualBuffer[this->numBytes] = '\0';
		return this->numChars;
	}

	//se o encoding for widechar do windows
	if (flagFormat == String::Windows)
	{
		int bytesNecessarios;

		//verifico quantos bytes são necessários
		bytesNecessarios = WideCharToMultiByte(CP_UTF8, 0, (LPCWSTR)nova, -1, nullptr, 0, nullptr, nullptr);
		if (bytesNecessarios <= 0)
			return -1;

		//tento criar o espaço necessário
		if (this->adjustRequiredBuffer(bytesNecessarios, false) == false)
			return -1;

		//converto o encode para UTF8
		WideCharToMultiByte(CP_UTF8, 0, (LPCWSTR)nova, -1, (LPSTR)this->actualBuffer, bytesNecessarios, nullptr, nullptr);

		//verifico sempre a string
		if (String::validateString(this->actualBuffer, &this->numChars, &this->numBytes) == false)
		{
			this->SetEmpty();
			return -1;
		}

		//devolvo quantos caracteres li
		return this->numChars;
	}

	//chegando aqui o encoding não é conhecido
	return 0;
}

int String::Set(const HorseRadish::hChar * const nova)
{
	int srcLength;

	//limpo a string actual
	this->numBytes = this->numChars = 0;
	this->bufferLocal[0]='\0';
	if (this->bufferHeap != nullptr)
		this->bufferHeap->Clear();
	this->actualBuffer = this->bufferLocal;

	//se não tenho de fazer nada
	if ((nova == nullptr) || (*nova == '\0'))
		return 0;
	
	//verifico o tamanho em bytes da nova string e tento ajustar o espaço necessário
	srcLength = strlen((const char*)nova);
	if (this->adjustRequiredBuffer(srcLength, false) == false)
		return 0;

	//copio os dados
	memcpy(this->actualBuffer, nova, srcLength + 1);

	//verifico se a string é válida
	if (String::validateString(this->actualBuffer, &this->numChars,&this->numBytes)==false)
	{
		this->SetEmpty();
		return -1;
	}

	//e devolvo o número de caracteres
	return this->numChars;
}

int String::Set(const String &nova)
{
	//estúpido mas enfim
	if (&nova == this)
		return this->numChars;

	//limpo a string actual
	this->numBytes = this->numChars = 0;
	this->bufferLocal[0]='\0';
	if (this->bufferHeap != nullptr)
		this->bufferHeap->Clear();
	this->actualBuffer = this->bufferLocal;

	//se não tenho de fazer nada
	if (nova.numBytes < 0)
		return 0;

	//tento arranjar espaço
	if (this->adjustRequiredBuffer(nova.numBytes, false) == false)
		return 0;

	//posso gravar estes dados
	this->numBytes = nova.numBytes;
	this->numChars = nova.numChars;

	//copio os dados
	memcpy(this->actualBuffer, nova.actualBuffer, this->numBytes + 1);

	//e devolvo o número de caracteres
	return this->numChars;
}

int String::SetPrintf(const Encoding flagFormat, const char *fmt, ...)
{
	va_list ap;
	int bytesWritten;
	char auxBuffer[1024];

	//limpo a string actual
	this->numBytes = this->numChars = 0;
	this->bufferLocal[0]='\0';
	if (this->bufferHeap != nullptr)
		this->bufferHeap->Clear();
	this->actualBuffer = this->bufferLocal;

	//tenho de ter alguma coisa
	if (fmt == nullptr)
		return 0;

	//fazer o parse
	va_start(ap, fmt);
		bytesWritten = vsnprintf_s(auxBuffer, sizeof(auxBuffer), _TRUNCATE, fmt, ap);
	va_end(ap);

	//se não tenho nada
	if (bytesWritten <= 0)
		return 0;

	//basta colocar este buffer
	return this->Set(flagFormat, auxBuffer);
}

int String::SetLine(const Encoding flagFormat, const void * const from)
{
	unsigned int curChar;

	//antes de fazer qualquer coisa, copio toda a string
	this->Set(flagFormat, from);

	//começo a avançar pela string e quando encontrar 13 ou 10 fecho-a
	Iterator it(*this);
	for(; (curChar = *it) != '\0'; ++it)
	{
		//se for para sair
		if (curChar==10 || curChar==13)
			break;
	}

	//guardo os valores e fecho a string
	this->numBytes = it.byteIndex;
	this->numChars = it.charIndex;
	this->actualBuffer[this->numBytes] = '\0';

	//já tá
	return this->numChars;
}

void String::SetTime(const unsigned int seconds)
{
	unsigned int horas,min,sec;

	horas=min=0;
	sec=seconds;
	while(sec>=3600)
	{
		horas++;
		sec-=3600;
	}
	while(sec>=60)
	{
		min++;
		sec-=60;
	}

	this->SetPrintf(String::ASCII, "%d:%02d:%02d", horas, min, sec);
}

void String::SetHexDump(const void * const buffer, const size_t bufferSize)
{
	char auxBuffer[1024];

	//por omissão
	this->SetEmpty();

	//preciso disto
	if ((buffer == nullptr) || (bufferSize == 0))
		return;

	//isto dá jeito
	auto bufferWalker = (const unsigned char*)buffer;

	//para controlar o número de bytes escritos
	auto numBytesEncoded = 0;

	//para diferenciar linhas
	while (true)
	{
		//escrever morada
		sprintf_s(auxBuffer, sizeof(auxBuffer), "%016X", numBytesEncoded);
		*this += (const HorseRadish::hChar*)auxBuffer;

		//mais um espaço
		*this += ' ';

		//enquanto tiver para escrever e ainda não for para mudar de linha
		while (numBytesEncoded < bufferSize)
		{
			//espaço e codifico mais um caracter
			*this += ' ';
			Encoders::EncodeHex(bufferWalker, 1, true, *this);
		
			//mais um byte que foi traduzido
			numBytesEncoded++;
			bufferWalker++;

			//se é para mudar de linha
			if ((numBytesEncoded % 16) == 0)
				break;
		}

		//se for para sair
		if (numBytesEncoded >= bufferSize)
			break;

		//mudo de linha
		*this += HorseRadish::Platform::NewLine;
	}
}

void String::SetMemory(const unsigned int bytes, const bool useMetric)
{
	double aux, bytesDouble;

	//preciso de converter para double
	bytesDouble = (double)bytes;

	//gigabytes
	aux = bytesDouble * (useMetric ? 0.000000001 : 0.000000000931322574615478515625);		//1.0 / 1000000000.0 ou 1.0 / 1073741824.0
	if (aux >= 1.0f)
	{
		this->SetPrintf(String::ASCII, "%.2f GB", aux);
		return;
	}

	//megabytes
	aux = bytesDouble * (useMetric ? 0.000001 : 0.00000095367431640625);		//1.0 / 1000000.0 ou 1.0 / 1048576.0;
	if (aux >= 1.0f)
	{
		this->SetPrintf(String::ASCII, "%.2f MB", aux);
		return;
	}

	//kilobytes
	aux = bytesDouble * (useMetric ? 0.001 : 0.0009765625);		//1.0 / 1000.0 ou 1.0 / 1024.0
	if (aux >= 1.0f)
	{
		this->SetPrintf(String::ASCII, "%.2f KB", aux);
		return;
	}
	
	//são de certeza só bytes
	this->SetPrintf(String::ASCII, "%d bytes", bytes);
}

void String::SetCurrentTime()
{
	tm newtime;
	__time32_t aclock;

	_time32(&aclock);
	_localtime32_s(&newtime,&aclock);

	this->SetPrintf(String::ASCII,"%d:%.2d:%.2d",newtime.tm_hour,newtime.tm_min,newtime.tm_sec);
}

void String::SetCurrentDate()
{
	tm newtime;
	__time32_t aclock;

	_time32(&aclock);
	_localtime32_s(&newtime,&aclock);

	this->SetPrintf(String::ASCII,"%.2d/%.2d/%d",newtime.tm_mday,newtime.tm_mon+1,newtime.tm_year+1900);
}

void String::RemoveAllChars(const unsigned int unicodeChar)
{
	unsigned int curChar, curCharSize, newBytes, newChars;

	//percorro a string toda
	newBytes = newChars = 0;
	for(Iterator it(*this); (curChar = it.Read(curCharSize)) != '\0'; )
	{
		//se este caracter for para eliminar, ignoro-o
		if (curChar == unicodeChar)
			continue;

		//se não tiver nada pra gravar para trás
		if ((newBytes + curCharSize) >= it.byteIndex)
		{
			newBytes = it.byteIndex;
			newChars++;
			continue;
		}

		//chegando aqui é tenho espaço para gravar anterior
		memcpy(this->actualBuffer + newBytes, this->actualBuffer + it.byteIndex - curCharSize, curCharSize);
		newBytes += curCharSize;
		newChars++;
	}

	//fecho a string e guardo os parametros
	this->actualBuffer[newBytes]='\0';
	this->numBytes = newBytes;
	this->numChars = newChars;
}

void String::RemoveDoubles(const unsigned int unicodeChar)
{
	unsigned int curChar, lastChar, curCharSize, newBytes, newChars;

	//percorro a string toda
	newBytes = newChars = 0;
	lastChar = 0;
	for(Iterator it(*this); (curChar = it.Read(curCharSize)) != '\0'; )
	{
		//se este caracter não for para eliminar, basta seguir
		if ((curChar == unicodeChar) && (lastChar == unicodeChar))
			continue;

		//guardo este caracter
		lastChar = curChar;

		//se não tiver nada pra gravar para trás
		if ((newBytes + curCharSize) >= it.byteIndex)
		{
			newBytes = it.byteIndex;
			newChars++;
			continue;
		}

		//chegando aqui é tenho espaço para gravar anterior
		memcpy(this->actualBuffer + newBytes, this->actualBuffer + it.byteIndex - curCharSize, curCharSize);
		newBytes += curCharSize;
		newChars++;
	}

	//para aproveitar memória
	if (this->bufferHeap != nullptr)
		this->bufferHeap->Shrink(this->numBytes - newBytes);

	//fecho a string e guardo os parametros
	this->actualBuffer[newBytes] = '\0';
	this->numBytes = newBytes;
	this->numChars = newChars;
}

void String::RemoveAt(const unsigned int posChar)
{
	unsigned int charByteIndex, charByteSize;

	//não pode passar o limite da string
	if (posChar >= this->numChars)
		return;

	//por omissão
	charByteIndex = charByteSize = 0;

	//basta ler cada caracter e quando chegar à posição correcta, posso sair
	for(Iterator it(*this); it.IsLast() == false; ++it)
	{
		//se ainda não cheguei à posição
		if (it.charIndex != posChar)
			continue;
		
		//guardo alguns dados
		charByteIndex = it.byteIndex;
		charByteSize = String::lengthUTF8(*it);
		break;
	}

	//se não é pra apagar nada
	if (charByteSize == 0)
		return;

	//copio tudo até ao fim
	for(; this->actualBuffer[charByteIndex - 1] != '\0'; charByteIndex++)
		this->actualBuffer[charByteIndex] = this->actualBuffer[charByteIndex + charByteSize];

	//para aproveitar memória
	if (this->bufferHeap != nullptr)
		this->bufferHeap->Shrink(charByteSize);

	//ajusto os novos valores desta string
	this->numBytes -= charByteSize;
	this->numChars--;
}

void String::RemoveAt(const unsigned int posChar, const unsigned int numToDelete)
{
	unsigned int charByteIndex, charsByteSize, charsDeleted;

	//não pode passar o limite da string
	if ((posChar >= numChars) || (numToDelete <= 0))
		return;

	//por omissão
	charByteIndex = charsByteSize = charsDeleted = 0;

	//basta ler cada caracter e quando chegar à posição correcta, posso sair
	for(Iterator it(*this); it.IsLast() == false; ++it)
	{
		//se ainda não cheguei à posição
		if (it.charIndex != posChar)
			continue;

		//guardo onde é pra começar
		charByteIndex = it.byteIndex;

		//para o número de caracteres a apagar
		for(unsigned int i=0; (i<numToDelete) && (it.IsLast() == false); i++, ++it)
		{
			//conto quantos bytes é pra apagar e o número de caracteres a apagar
			charsByteSize += String::lengthUTF8(*it);
			charsDeleted++;
		}

		//posso sair
		break;
	}

	//se não é pra apagar nada
	if ((charsByteSize == 0) || (charsDeleted == 0))
		return;

	//copio tudo até ao fim
	for(; this->actualBuffer[charByteIndex - 1] != '\0'; charByteIndex++)
		this->actualBuffer[charByteIndex] = this->actualBuffer[charByteIndex + charsByteSize];

	//para aproveitar memória
	if (this->bufferHeap != nullptr)
		this->bufferHeap->Shrink(charsByteSize);

	//ajusto os novos valores desta string
	this->numBytes -= charsByteSize;
	this->numChars -= charsDeleted;
}

void String::Remove(const unsigned int numberChar, const Position flagPosition)
{
	//preciso de alguma coisa
	if (numberChar==0)
		return;

	//se for para apagar a partir do fim
	if (flagPosition == String::End)
	{
		HorseRadish::hChar *walker;
		unsigned int numBytesRemovidos;

		//se é pra apagar tudo facilita a vida
		if (numberChar >= this->numChars)
		{
			this->SetEmpty();
			return;
		}

		//tenho de ver onde apagar
		walker = this->actualBuffer;
		for(unsigned int curChar = 0; curChar < (this->numChars - numberChar); curChar++)
			walker += String::trailingBytesUTF8[(HorseRadish::hUInt8)*walker] + 1;

		//posso saber quantos bytes removi
		numBytesRemovidos = (unsigned int)(this->actualBuffer + this->numBytes - walker);

		//fecho a string
		*walker = '\0';

		//para aproveitar memória
		if (this->bufferHeap != nullptr)
			this->bufferHeap->Shrink(numBytesRemovidos);
		
		//ajusto os parametros da string
		this->numChars -= numberChar;
		this->numBytes -= numBytesRemovidos;
		return;
	}

	//se for para apagar a partir do início
	if (flagPosition == String::Start)
	{
		HorseRadish::hChar *walker;
		unsigned int i, numBytesRemovidos;

		//se é pra apagar tudo facilita a vida
		if (numberChar >= this->numChars)
		{
			this->SetEmpty();
			return;
		}

		//avanço até onde tenho de avançar
		walker = this->actualBuffer;
		for(i=0; i<numberChar; i++)
			walker += String::trailingBytesUTF8[(HorseRadish::hUInt8)*walker] + 1;

		//daqui para a frente, basta copiar tudo até '\0'
		for(i=0; walker[i]!='\0'; i++)
			this->actualBuffer[i] = walker[i];
		this->actualBuffer[i]='\0';

		//posso saber quantos bytes removi
		numBytesRemovidos = (unsigned int)(walker - this->actualBuffer);

		//para aproveitar memória
		if (this->bufferHeap != nullptr)
			this->bufferHeap->Shrink(numBytesRemovidos);

		//ajusto os parametros da string
		this->numChars -= numberChar;
		this->numBytes -= numBytesRemovidos;
		return;
	}

	//se for para apagar a partir do PRINCIPIO e do FIM
	if (flagPosition == String::Stend)
	{
		//se é pra apagar tudo, facilita a vida
		if ((numberChar * 2) >= numChars)
		{
			this->SetEmpty();
			return;
		}

		//basta apagar de um lado e do outro
		this->Remove(numberChar, String::End);
		this->Remove(numberChar, String::Start);
		return;
	}
}

void String::AppendAtStart(const String &appendStr)
{
	HorseRadish::hChar *walkerWrite;
	const HorseRadish::hChar *walkerRead;

	//tenho de ter alguma coisa
	if (appendStr.numBytes == 0)
		return;

	//preciso de espaço
	if (this->adjustRequiredBuffer(this->numBytes + appendStr.numBytes, true) == false)
		return;

	//chego o antigo para a frente
	walkerWrite = this->actualBuffer + this->numBytes + appendStr.numBytes;
	walkerRead = this->actualBuffer + this->numBytes;
	for(; walkerRead >= this->actualBuffer; walkerRead--, walkerWrite--)
		*walkerWrite = *walkerRead;

	//agora posso simplesmente copiar o novo para o principio
	memcpy(this->actualBuffer, appendStr.actualBuffer, appendStr.numBytes);

	//ajusto os parametros da string
	this->numBytes += appendStr.numBytes;
	this->numChars += appendStr.numChars;
}

void String::AppendAtStart(const unsigned int unicodeNewChar)
{
	unsigned int newCharSize;
	HorseRadish::hChar *walkerWrite;
	const HorseRadish::hChar *walkerRead;

	//tenho de ter alguma coisa
	if (unicodeNewChar == 0)
		return;

	//quantos bytes ocupa este novo caracter
	newCharSize = String::lengthUTF8(unicodeNewChar);

	//preciso de espaço
	if (this->adjustRequiredBuffer(this->numBytes + newCharSize, true) == false)
		return;

	//chego o antigo para a frente
	walkerWrite = this->actualBuffer + this->numBytes + newCharSize;
	walkerRead = this->actualBuffer + this->numBytes;
	for(;walkerRead >= this->actualBuffer; walkerRead--, walkerWrite--)
		*walkerWrite = *walkerRead;

	//agora posso simplesmente copiar o novo para o principio
	String::unicode2utf8(unicodeNewChar, this->actualBuffer);

	//ajusto os parametros da string
	this->numBytes += newCharSize;
	this->numChars++;
}

void String::AppendAtEnd(const String &appendStr)
{
	//tenho de ter alguma coisa
	if (appendStr.numBytes == 0)
		return;

	//preciso de espaço
	if (this->adjustRequiredBuffer(this->numBytes + appendStr.numBytes, true) == false)
		return;

	//basta acrescentar a nova string ao fim da lista
	memcpy(this->actualBuffer + this->numBytes, appendStr.actualBuffer, appendStr.numBytes + 1);
	
	//ajusto os parametros da string
	this->numBytes += appendStr.numBytes;
	this->numChars += appendStr.numChars;
}

void String::AppendAtEnd(const unsigned int unicodeNewChar)
{
	unsigned int newCharSize;

	//tenho de ter alguma coisa
	if (unicodeNewChar==0)
		return;

	//quantos bytes ocupa este novo caracter
	newCharSize = String::lengthUTF8(unicodeNewChar);

	//preciso de espaço
	if (this->adjustRequiredBuffer(this->numBytes + newCharSize, true) == false)
		return;

	//basta acrescentar a nova string ao fim da lista
	String::unicode2utf8(unicodeNewChar, this->actualBuffer + this->numBytes);
	this->actualBuffer[this->numBytes + newCharSize] = '\0';
	
	//ajusto os parametros da string
	this->numBytes += newCharSize;
	this->numChars++;
}

void String::AppendAtPos(const String &appendStr, const unsigned int pos)
{
	//tenho de ter alguma coisa
	if (appendStr.numBytes == 0)
		return;

	//verificar os casos especiais que se podem resumir a inserir no principio ou no fim
	if (pos == 0)
	{
		this->AppendAtStart(appendStr);
		return;
	}
	if (pos >= numChars)
	{
		this->AppendAtEnd(appendStr);
		return;
	}

	//preciso de espaço
	if (this->adjustRequiredBuffer(this->numBytes + appendStr.numBytes, true) == false)
		return;

	//avanço até à posição indicada
	Iterator it(*this);
	while(it.charIndex < pos)
		++it;

	//copio tudo desde o fim da string até onde estou
	for(int endBack = this->numBytes; endBack >= it.byteIndex; endBack--)
		this->actualBuffer[endBack + appendStr.numBytes] = this->actualBuffer[endBack];

	//agora posso simplesmente copiar a string indicada
	memcpy(this->actualBuffer + it.byteIndex, appendStr.actualBuffer, appendStr.numBytes);

	//ajusto os parametros da string
	this->numBytes += appendStr.numBytes;
	this->numChars += appendStr.numChars;
}

void String::AppendAtPos(const unsigned int unicodeNewChar, const unsigned int pos)
{
	unsigned int newCharSize;

	//tenho de ter alguma coisa
	if (unicodeNewChar == 0)
		return;

	//verificar os casos especiais que se podem resumir a inserir no principio ou no fim
	if (pos == 0)
	{
		this->AppendAtStart(unicodeNewChar);
		return;
	}
	if (pos >= numChars)
	{
		this->AppendAtEnd(unicodeNewChar);
		return;
	}

	//avanço até à posição indicada
	Iterator it(*this);
	while(it.charIndex < pos)
		++it;

	//quantos bytes ocupa este novo caracter
	newCharSize = String::lengthUTF8(unicodeNewChar);

	//copio tudo desde o fim da string até onde estou
	for(int endBack = this->numBytes; endBack >= it.byteIndex; endBack--)
		this->actualBuffer[endBack + newCharSize] = this->actualBuffer[endBack];

	//agora posso simplesmente copiar a string indicada
	String::unicode2utf8(unicodeNewChar, this->actualBuffer + it.byteIndex);

	//ajusto os parametros da string
	this->numBytes += newCharSize;
	this->numChars++;
}

void String::CloseAt(const unsigned int charPos)
{
	HorseRadish::hChar *walker;

	//se for para fechar no inicio
	if (charPos == 0)
	{
		this->SetEmpty();
		return;
	}

	//avanço até onde tenho de avançar
	walker = this->actualBuffer;
	for(unsigned int i=0; i<charPos; i++)
		walker += String::trailingBytesUTF8[(HorseRadish::hUInt8)*walker] + 1;

	//fecho a string
	*walker = '\0';

	//ajusto os parametros da string
	this->numBytes = (unsigned short)(walker - this->actualBuffer);
	this->numChars = charPos;
}

bool String::IsEmpty(const bool checkOnlyWhiteSpaces) const
{
	//se não tenho está vazio de certeza
	if (this->numBytes <= 0)
		return true;

	//se não é preciso verificar por espaços
	if (checkOnlyWhiteSpaces == false)
		return false;

	//percorro todos os caracteres
	for(const HorseRadish::hChar *walker = this->actualBuffer; *walker != '\0'; walker++)
	{
		//se tenho alguma coisa pra além de espaços
		if (*walker != ' ')
			return false;
	}

	//chegando aqui, só tenho espaços, portanto considero a string vazia
	return true;
}

unsigned int String::Capacity(const unsigned int capacityNeeded)
{
	//mando aumentar e devolvo o novo tamanho
	this->adjustRequiredBuffer(capacityNeeded, true);
	return this->numBytes;
}

float String::ToFloat() const
{
	bool minus,exponent,expMinus;
	int casasExp;
	float up,down,resultado,casasDecimais;
	const HorseRadish::hChar *walker;

	//avanço os brancos
	walker = this->actualBuffer;
	while(*walker==' ' && *walker!='\0') walker++;

	//vejo se tenho sinal
	minus=false;
	if (*walker=='-' || *walker=='+')
	{
		if (*walker=='-')
			minus=true;
		walker++;
	}

	//tenho de por isto a zero
	casasExp=0;
	up=down=0.0f;
	casasDecimais=1.0f;

	//enquanto tiver digitos para ler (parte nao fracionaria)
	while(*walker>='0' && *walker<='9')
	{
		up*=10.0f;
		up+=(float)((*walker)-'0');
		walker++;
	}

	//enquanto tiver digitos para ler (parte fracionaria)
	if (*walker=='.')
	{
		walker++;
		while(*walker>='0' && *walker<='9')
		{
			down*=10.0f;
			down+=(float)((*walker)-'0');
			casasDecimais*=10.0f;
			walker++;
		}
	}

	//se tiver sinal para marcar expoente
	exponent=expMinus=false;
	if (*walker=='d' || *walker=='D' || *walker=='e' || *walker=='E')
		exponent=true;

	//se tiver expoente
	if (exponent)
	{
		//vejo se tenho sinal no expoente
		walker++;
		if (*walker=='-' || *walker=='+')
		{
			if (*walker=='-')
				expMinus=true;
			walker++;
		}

		//leio o numero
		while(*walker>='0' && *walker<='9')
		{
			casasExp*=10;
			casasExp+=(*walker)-'0';
			walker++;
		}
	}

	//agora só falta formar o numero
	if (!exponent)
	{
		resultado=down/casasDecimais;
		resultado+=up;
		if (minus)
			resultado=-resultado;
		return resultado;
	}

	resultado=down/casasDecimais;
	resultado+=up;
	if (minus)
		resultado=-resultado;
	if (expMinus)
	{
		while((casasExp--)>0)	resultado/=10;
		return resultado;
	}
	while((casasExp--)>0)	resultado*=10;
	return resultado;
}

int String::ToInt() const
{
	int val;
	bool minus;
	const HorseRadish::hChar *walker;

	//avanço os brancos
	walker=this->actualBuffer;
	while(*walker==' ' && *walker!='\0') walker++;

	//vejo se tenho sinal
	minus=false;
	if (*walker=='-' || *walker=='+')
	{
		if (*walker=='-')
			minus=true;
		walker++;
	}

	//tenho de por isto a zero
	val=0;

	//enquanto tiver digitos para ler (parte nao fracionaria)
	while(*walker>='0' && *walker<='9')
	{
		val*=10;
		val+=(*walker)-'0';
		walker++;
	}

	//arranjo o numero e mais nada
	return (minus ? -val : val);
}

int String::ToHex() const
{
	int val;
	bool minus;
	const HorseRadish::hChar *walker;

	//avanço os brancos
	walker=this->actualBuffer;
	while(*walker==' ' && *walker!='\0') walker++;

	//vejo se tenho sinal
	minus=false;
	if (*walker=='-' || *walker=='+')
	{
		if (*walker=='-')
			minus=true;
		walker++;
	}

	//tenho de por isto a zero
	val=0;

	//enquanto tiver digitos para ler (parte nao fracionaria)
	while( (*walker>='0' && *walker<='9') || (*walker>='a' && *walker<='f') || (*walker>='A' && *walker<='F') ) 
	{
		val*=16;
		if (*walker>='a' && *walker<='f')
			val+=(*walker)-'a'+10;
		else if (*walker>='A' && *walker<='F')
			val+=(*walker)-'A'+10;
		else
			val+=(*walker)-'0';
		walker++;
	}

	//arranjo o numero e mais nada
	return (minus ? -val : val);
}

unsigned int String::GetUnicodeAt(const unsigned int &charPos) const
{
	//verifico isto
	if (charPos >= this->numChars)
		return 0;

	//precorro toda a string
	for(Iterator it(*this); it.IsLast() == false; ++it)
	{
		//se for esta a posição, devolvo o caracter
		if (it.charIndex == charPos)
			return *it;
	}

	//algo não correu bem (nunca devia chegar aqui)
	assert(false);
	return 0;
}

}//namespace HorseRadish