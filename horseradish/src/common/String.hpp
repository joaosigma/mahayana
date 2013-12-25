#pragma once
#ifndef __HSTRING__
#define __HSTRING__

#include <Memory.h>
#include "Containers.hpp"

namespace HorseRadish
{

class String
{

public:
	class Iterator
	{
		unsigned short byteIndex, charIndex;
		const String &string;

		friend class String;

		unsigned int readChar(unsigned int &charNumBytes) const
		{
			unsigned int finalChar;
			unsigned short tempIterator;

			//por omissão
			finalChar = charNumBytes = 0;

			//se estou no fim
			if (this->byteIndex >= this->string.numBytes)
				return finalChar;

			//guardo a posição actual
			tempIterator = this->byteIndex;

			//leio o próximo caracter
			do {
				finalChar <<= 6;
				finalChar += (unsigned char)this->string.actualBuffer[tempIterator++];
				charNumBytes++;
			} while(this->string.actualBuffer[tempIterator] && !String::isutf8(this->string.actualBuffer[tempIterator]));
			finalChar -= String::offsetsFromUTF8[charNumBytes - 1];

			//já tá
			return finalChar;
		}
		unsigned int readCharMove(unsigned int &charNumBytes)
		{
			unsigned int finalChar;

			//esta função trata de tudo
			finalChar = this->readChar(charNumBytes);

			//se li realmente alguma coisa
			if (charNumBytes > 0)
			{
				this->byteIndex += charNumBytes;
				this->charIndex++;
			}

			//posso devolver o caracter
			return finalChar;
		}
	public:
		Iterator(const String &string) : string(string)
		{
			//por omissão está sempre no início
			this->byteIndex = 0;
			this->charIndex = 0;
		}
		Iterator(const Iterator &iterator) : string(iterator.string)
		{
			//copio estes parametros
			this->byteIndex = iterator.byteIndex;
			this->charIndex = iterator.charIndex;
		}
		void First()
		{
			//vai pra o início
			this->byteIndex = 0;
			this->charIndex = 0;
		}
		bool IsFirst() const
		{
			//basta verificar se está no inicio
			return (this->byteIndex == 0);
		}
		void Last()
		{
			//vai pra o fim
			this->byteIndex = this->string.numBytes;
			this->charIndex = this->string.numChars - 1;

			//tenho de recuar para o caracter anterior
			(String::isutf8(this->string.actualBuffer[--this->byteIndex]) || String::isutf8(this->string.actualBuffer[--this->byteIndex]) || String::isutf8(this->string.actualBuffer[--this->byteIndex]) || --this->byteIndex);
		}
		bool IsLast() const
		{
			//basta verificar se está no fim
			return (this->byteIndex >= this->string.numBytes);
		}
		unsigned int GetBytePosition() const
		{
			//basta devolver este parametro
			return (this->byteIndex);
		}
		unsigned int GetCaracterPosition() const
		{
			//basta devolver este parametro
			return (this->charIndex);
		}
		unsigned int Read()
		{
			unsigned int numBytes;

			//esta função trata de tudo
			return this->readCharMove(numBytes);
		}
		unsigned int Read(unsigned int &charByteSize)
		{
			//esta função trata de tudo
			return this->readCharMove(charByteSize);
		}
		Iterator& operator++()
		{
			//se não posso avançar mais
			if (this->byteIndex >= this->string.numBytes)
				return *this;

			//avanço
			this->byteIndex += String::trailingBytesUTF8[(HorseRadish::hUInt8)this->string.actualBuffer[this->byteIndex]] + 1;
			this->charIndex++;

			//devolvo-me
			return *this;
		}
		Iterator& operator+=(int numCharaters)
		{
			//enquanto for para avançar, avanço
			while (((numCharaters--) > 0) && (this->byteIndex < this->string.numBytes))
			{
				this->byteIndex += String::trailingBytesUTF8[(HorseRadish::hUInt8)this->string.actualBuffer[this->byteIndex]] + 1;
				this->charIndex++;
			}

			//devolvo-me
			return *this;
		}
		Iterator& operator--()
		{
			//se não posso recuar mais
			if (this->byteIndex == 0)
				return *this;

			//recuo
			(String::isutf8(this->string.actualBuffer[--this->byteIndex]) || String::isutf8(this->string.actualBuffer[--this->byteIndex]) || String::isutf8(this->string.actualBuffer[--this->byteIndex]) || --this->byteIndex);
			this->charIndex--;

			//devolvo-me
			return *this;
		}
		Iterator& operator-=(int numCharaters)
		{
			//enquanto for para recuar, avanço
			while (((numCharaters--) > 0) && (this->byteIndex > 0))
			{
				(String::isutf8(this->string.actualBuffer[--this->byteIndex]) || String::isutf8(this->string.actualBuffer[--this->byteIndex]) || String::isutf8(this->string.actualBuffer[--this->byteIndex]) || --this->byteIndex);
				this->charIndex--;
			}

			//devolvo-me
			return *this;
		}
		unsigned int operator*() const
		{
			unsigned int numBytes;

			//esta função trata de tudo
			return this->readChar(numBytes);
		}

		bool operator ==(const Iterator &it) const { return (this->byteIndex == it.byteIndex); }
		bool operator !=(const Iterator &it) const { return (this->byteIndex != it.byteIndex); }
		bool operator >(const Iterator &it) const { return (this->byteIndex > it.byteIndex); }
		bool operator <(const Iterator &it) const { return (this->byteIndex < it.byteIndex); }
		bool operator >=(const Iterator &it) const { return (this->byteIndex >= it.byteIndex); }
		bool operator <=(const Iterator &it) const { return (this->byteIndex <= it.byteIndex); }
	};

	class Tokenizer
	{
		unsigned short byteIndex;
		unsigned int tokenUnicode;
		const String &string;

		friend class String;

		unsigned int readChar(unsigned int &charNumBytes) const
		{
			unsigned int finalChar;
			unsigned short tempIterator;

			//por omissão
			finalChar = charNumBytes = 0;

			//se estou no fim
			if (this->byteIndex >= this->string.numBytes)
				return finalChar;

			//guardo a posição actual
			tempIterator = this->byteIndex;

			//leio o próximo caracter
			do {
				finalChar <<= 6;
				finalChar += (unsigned char)this->string.actualBuffer[tempIterator++];
				charNumBytes++;
			} while(this->string.actualBuffer[tempIterator] && !String::isutf8(this->string.actualBuffer[tempIterator]));
			finalChar -= String::offsetsFromUTF8[charNumBytes - 1];

			//já tá
			return finalChar;
		}
		unsigned int readCharMove(unsigned int &charNumBytes)
		{
			unsigned int finalChar;

			//esta função trata de tudo
			finalChar = this->readChar(charNumBytes);

			//se li realmente alguma coisa
			if (charNumBytes > 0)
				this->byteIndex += charNumBytes;

			//posso devolver o caracter
			return finalChar;
		}
	public:
		Tokenizer(const String &string, unsigned int tokenUnicode) : string(string)
		{
			//por omissão está sempre no início
			this->byteIndex = 0;

			//guardo o token
			this->tokenUnicode = tokenUnicode;
		}
		Tokenizer(const Tokenizer &tokenizer) : string(tokenizer.string)
		{
			//copio estes parametros
			this->byteIndex = tokenizer.byteIndex;
			this->tokenUnicode = tokenizer.tokenUnicode;
		}
		void First()
		{
			//vai pra o início
			this->byteIndex = 0;
		}
		bool IsFirst() const
		{
			//basta verificar se está no inicio
			return (this->byteIndex == 0);
		}
		void Last()
		{
			//vai pra o fim
			this->byteIndex = this->string.numBytes;

			//tenho de recuar para o caracter anterior
			(String::isutf8(this->string.actualBuffer[--this->byteIndex]) || String::isutf8(this->string.actualBuffer[--this->byteIndex]) || String::isutf8(this->string.actualBuffer[--this->byteIndex]) || --this->byteIndex);
		}
		bool IsLast() const
		{
			//basta verificar se está no fim
			return (this->byteIndex >= this->string.numBytes);
		}
		unsigned int GetBytePosition() const
		{
			//basta devolver este parametro
			return (this->byteIndex);
		}
		String Read()
		{
			String resultado;

			//leio e devolvo a string lida
			this->Read(resultado);
			return resultado;
		}
		bool Read(String &result)
		{
			unsigned int curChar, numBytes;

			//limpo a string pra onde vou escrever
			result.SetEmpty();

			//se estou no fim
			if (this->byteIndex >= this->string.numBytes)
				return false;

			//enquanto não chegar ao token
			curChar = 0;
			while(true)
			{
				//leio o próximo caracter
				curChar = readCharMove(numBytes);
				
				//se o caracter for o token ou se cheguei ao fim da string
				if ((curChar == this->tokenUnicode) || (curChar == '\0'))
					break;

				//acumulo este caracter
				result += curChar;
			}

			//li alguma coisa
			return true;
		}
		Tokenizer& operator++()
		{
			unsigned int curChar, numBytes;

			//se estou no fim
			if (this->byteIndex >= this->string.numBytes)
				return *this;

			//enquanto não chegar ao token
			curChar = 0;
			while(true)
			{
				//leio o próximo caracter
				curChar = readCharMove(numBytes);
				
				//se o caracter for o token ou se cheguei ao fim da string
				if ((curChar == this->tokenUnicode) || (curChar == '\0'))
					break;
			}

			//devolvo-me
			return *this;
		}
		Tokenizer& operator+=(int numCharaters)
		{
			//enquanto for para avançar, avanço
			while (((numCharaters--) > 0) && (this->byteIndex < this->string.numBytes))
			{
				unsigned int curChar, numBytes;

				//enquanto não chegar ao token
				curChar = 0;
				while(true)
				{
					//leio o próximo caracter
					curChar = readCharMove(numBytes);
					
					//se o caracter for o token ou se cheguei ao fim da string
					if ((curChar == this->tokenUnicode) || (curChar == '\0'))
						break;
				}
			}

			//devolvo-me
			return *this;
		}

		bool operator ==(const Iterator &it) const { return (this->byteIndex == it.byteIndex); }
		bool operator !=(const Iterator &it) const { return (this->byteIndex != it.byteIndex); }
		bool operator >(const Iterator &it) const { return (this->byteIndex > it.byteIndex); }
		bool operator <(const Iterator &it) const { return (this->byteIndex < it.byteIndex); }
		bool operator >=(const Iterator &it) const { return (this->byteIndex >= it.byteIndex); }
		bool operator <=(const Iterator &it) const { return (this->byteIndex <= it.byteIndex); }
	};

private:
	HorseRadish::hChar *actualBuffer;
	HorseRadish::Containers::Array<HorseRadish::hChar> *bufferHeap;
	unsigned short numBytes, numChars;
	HorseRadish::hChar bufferLocal[64 - (sizeof(void*) * 2) - (sizeof(short) * 2)];

	static unsigned int lengthUTF8(const unsigned int &unicodeChar);
	static unsigned int unicode2utf8(const unsigned int &unicodeChar, HorseRadish::hChar * const bufferWrite);
	static bool validateChar(const HorseRadish::hChar * const charPtr);
	static bool validateString(const HorseRadish::hChar * const stringData, unsigned short * const dataCharCount, unsigned short * const dataByteCount);
	static inline bool isutf8(const HorseRadish::hChar &value);

	static const unsigned __int32 offsetsFromUTF8[6];
	static const char trailingBytesUTF8[256];

	void calcStrParams();
	bool adjustRequiredBuffer(const unsigned int &numBytesRequired, const bool &copyData);

public:
	enum Position{
		Start = 0xdea1,
		End = 0xdea2,
		Stend = 0xdea3,
		};

	enum Encoding{
		UTF8 = 0xdeb1,
		ASCII = 0xdeb2,
		UTF16 = 0xdeb3,
		UTF32 = 0xdeb4,
		Windows = 0xdeb5
		};

	String();
	String(const String &nova);
	explicit String(const char *fmt, ...);
	~String();

	void SetEmpty();
	int Set(const Encoding &flagFormat, const void * const nova, const int &numCharToEncode=-1);
	int Set(const HorseRadish::hChar * const nova);
	int Set(const String &nova);
	int SetPrintf(const Encoding flagFormat, const char *fmt, ...);
	int SetLine(const Encoding flagFormat, const void * const from);
	void SetTime(const unsigned int seconds);
	void SetMemory(const unsigned int bytes, const bool useMetric = false);
	void SetHexDump(const void * const buffer, const size_t bufferSize);
	void SetCurrentTime();
	void SetCurrentDate();

	void RemoveAllChars(const unsigned int unicodeChar);
	void RemoveDoubles(const unsigned int unicodeChar);
	void RemoveAt(const unsigned int posChar);
	void RemoveAt(const unsigned int posChar, const unsigned int numToDelete);
	void Remove(const unsigned int numberChar, const Position flagPosition);
	
	void Trim(const unsigned int unicodeChar = ' ');
	void Reverse();
	void MakeLowerCase();
	void MakeUpperCase();
	void Replace(const unsigned int unicodeOldChar, const unsigned int unicodeNewChar);
	void AppendAtStart(const String &appendStr);
	void AppendAtStart(const unsigned int unicodeNewChar);
	void AppendAtEnd(const String &appendStr);
	void AppendAtEnd(const unsigned int unicodeNewChar);
	void AppendAtPos(const String &appendStr, const unsigned int pos);
	void AppendAtPos(const unsigned int unicodeNewChar, const unsigned int pos);
	void CloseAt(const unsigned int charPos);

	bool IsEmpty(const bool checkOnlyWhiteSpaces = false) const;

	unsigned int Capacity(const unsigned int capacityNeeded);

	void CopyTo(char * const outputBuffer) const { if (outputBuffer != nullptr) memcpy(outputBuffer, this->actualBuffer, this->numBytes + 1);}
	bool StartsWith(const String &compareTo) const;
	bool StartsWith(const HorseRadish::hChar * const compareTo) const;
	bool StartsWith(const unsigned int &unicodeChar) const;
	bool EndsWith(const String &compareTo) const;
	bool EndsWith(const unsigned int &unicodeChar) const;
	int Convert(const Encoding flagFormat, void * const outBuffer, const unsigned int outSize) const;
	float ToFloat() const;
	int  ToInt() const;
	int  ToHex() const;
	
	unsigned int GetSizeBytes() const { return this->numBytes; }
	unsigned int GetSizeChars() const { return this->numChars; }
	const char * GetData() const { return ((const char *)this->actualBuffer); }
	void GetSubStr(String &subStr, const unsigned int startChar, const unsigned int numChar) const;
	String GetSubStr(const unsigned int startChar, const unsigned int numChar) const;
	unsigned int GetUnicodeAt(const unsigned int &charPos) const;
	int GetCharByteIndex(const unsigned int pos) const;
	int GetCharPosIndex(const unsigned int byte) const;

	String& operator=(const String& s);
	const String operator+(const String& s) const;
	String& operator+=(const String& s);
	String& operator+=(const HorseRadish::hChar * const s);
	String& operator+=(const unsigned int unicodeChar);
	bool operator==(const String& s) const;
	bool operator!=(const String& s) const;
	bool operator==(const HorseRadish::hChar * const s) const;
	bool operator!=(const HorseRadish::hChar * const s) const;
	bool operator==(const char * const s) const;
	bool operator!=(const char * const s) const;
	unsigned int operator[](int index) const;
	unsigned int& operator[](int index);
};

}//namespace HorseRadish

#endif