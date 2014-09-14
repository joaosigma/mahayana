#include "String.hpp"

#include "Math.hpp"
#include "Encoders.hpp"
#include "Platform.hpp"

#include <time.h>
#include <assert.h>

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

	bufferWrite[0]='\0';
	return 0;
}

bool String::validateChar(const HorseRadish::hChar * const charPtr)
{
	int length;
	HorseRadish::hUInt8 a;
	const HorseRadish::hChar *srcptr;

	length = String::trailingBytesUTF8[(HorseRadish::hUInt8)*charPtr]+1;
	srcptr = charPtr + length;

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

	numChar = 0;
	for(walker = stringData; *walker!='\0'; walker += String::trailingBytesUTF8[(HorseRadish::hUInt8)*walker] + 1)
	{
		if (String::validateChar(walker)==false)
			return false;

		numChar++;
	}

	if (dataCharCount != nullptr)
		*dataCharCount = numChar;
	if (dataByteCount != nullptr)
		*dataByteCount = (unsigned short)(walker - stringData);

	return true;
}

bool String::isutf8(const HorseRadish::hChar &value)
{
	return ((value & 0xC0) != 0x80);
}

void String::calcStrParams()
{
	if (*this->actualBuffer=='\0')
	{
		this->numBytes = this->numChars = 0;
		return;
	}

	this->numChars = 0;

	HorseRadish::hChar *walker = this->actualBuffer;
	for(; *walker != '\0'; walker += String::trailingBytesUTF8[(HorseRadish::hUInt8)*walker] + 1)
		this->numChars++;

	this->numBytes = (unsigned short)(walker - this->actualBuffer);
}

bool String::adjustRequiredBuffer(const unsigned int &numBytesRequired, const bool &copyData)
{
	if (this->numBytes == numBytesRequired)
		return true;

	if (this->numBytes > numBytesRequired)
		return true;

	if ((numBytesRequired + 1) > sizeof(this->bufferLocal))
	{
		this->bufferHeap.resize(numBytesRequired + 1);
		
		if ((copyData == true) && (this->numBytes > 0) && (this->actualBuffer == this->bufferLocal))
			memcpy(this->bufferHeap.data(), this->bufferLocal, this->numBytes + 1);

		this->bufferLocal[0] = '\0';
		this->actualBuffer = this->bufferHeap.data();
	}

	return true;
}

String::String()
	: numBytes(0), numChars(0)
{
	this->bufferLocal[0] = '\0';
	this->actualBuffer = this->bufferLocal;
}

String::String(const String &s)
	: String()
{
	this->Set(s);
}

String::String(String &&s)
{
	this->bufferHeap = std::move(s.bufferHeap);
	this->numBytes = s.numBytes;
	this->numChars = s.numChars;

	if (s.actualBuffer == s.bufferLocal)
	{
		this->actualBuffer = this->bufferLocal;
		memcpy(this->bufferLocal, s.bufferLocal, s.numBytes);
	}
	else
	{
		this->actualBuffer = this->bufferHeap.data();
		this->bufferLocal[0] = '\0';
	}

	s.actualBuffer = nullptr;
	s.numBytes = s.numChars = 0;
}

String::String(const char *fmt, ...)
	: String()
{
	va_list ap;
	int bytesWritten;
	char auxBuffer[2048];

	if (fmt == nullptr)
		return;

	va_start(ap, fmt);
		bytesWritten = vsnprintf_s(auxBuffer, sizeof(auxBuffer), _TRUNCATE, fmt, ap);
	va_end(ap);

	if (bytesWritten <= 0)
		return;

	if (this->adjustRequiredBuffer(bytesWritten, false) == false)
		return;

	memcpy(this->actualBuffer, auxBuffer, bytesWritten + 1);

	if (String::validateString(this->actualBuffer, &this->numChars, &this->numBytes) == false)
		this->SetEmpty();
}

String::~String()
{
	this->bufferLocal[0] = '\0';
	this->bufferHeap.clear();
	this->actualBuffer = nullptr;;
	this->numBytes = this->numChars = 0;
}

int String::Convert(const Encoding flagFormat, void * const outBuffer, const unsigned int outSize) const
{
	if ((outBuffer == nullptr) || (outSize == 0))
		return 0;

	if (flagFormat == String::Encoding::ASCII)
	{
		unsigned int curChar;
		int countChars, bufferLeft;
		HorseRadish::hChar *writer;

		countChars = 0;
		writer = (HorseRadish::hChar*)outBuffer;
		bufferLeft = outSize;

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

		writer[countChars] = '\0';
		return countChars;
	}

	if (flagFormat == String::Encoding::UTF32)
	{
		unsigned int *writer, curChar, countChars;

		writer = (unsigned int*)outBuffer;
		countChars = 0;

		Iterator it(*this);
		while( ((curChar=it.Read()) != 0) && ((countChars*4) < outSize) )
			writer[countChars++] = curChar;

		writer[countChars] = '\0';
		return countChars;
	}

	if (flagFormat == String::Encoding::Windows)
	{
		return MultiByteToWideChar(CP_UTF8, 0, (LPCSTR)this->actualBuffer, -1, (LPWSTR)outBuffer, outSize);
	}

	return 0;
}


void String::Trim(const unsigned int unicodeChar)
{
	Iterator itStart(*this);
	for(; itStart.IsLast() == false; ++itStart)
	{
		if (*itStart != unicodeChar)
			break;
	}

	if (itStart.IsLast() == true)
	{
		this->SetEmpty();
		return;
	}

	Iterator itEnd(*this);
	itEnd.Last();
	for(; itEnd.IsFirst() == false; --itEnd)
	{
		if (*itEnd != unicodeChar)
			break;
		if (itEnd <= itStart)
			break;
	}

	if (itEnd < itStart)
	{
		this->SetEmpty();
		return;
	}
	
	++itEnd;

	if ((itStart.IsFirst() == true) && (itEnd.IsLast() == true))
		return;

	int i = 0;
	for(int curPos = itStart.byteIndex; curPos < itEnd.byteIndex; i++,curPos++)
		this->actualBuffer[i] = this->actualBuffer[curPos];
	this->actualBuffer[i] = '\0';

	calcStrParams();
}

void String::Reverse()
{
	HorseRadish::hChar *walker;
	String strTmp;

	strTmp.Set(*this);

	walker = this->actualBuffer;
	for(Iterator it(strTmp); true; --it)
	{
		walker += String::unicode2utf8(*it, walker);
		if (it.IsFirst() == true)
			break;
	}

	assert(*walker == '\0');
}

void String::MakeLowerCase()
{
	for(int i=0; i<this->numBytes; i++)
	{
		if ((this->actualBuffer[i] >= 'A') && (this->actualBuffer[i] <= 'Z'))
			this->actualBuffer[i] += 'a'-'A';
	}
}

void String::MakeUpperCase()
{
	for(int i=0; i<this->numBytes; i++)
	{
		if ((this->actualBuffer[i] >= 'a') && (this->actualBuffer[i] <= 'z'))
			this->actualBuffer[i] -= 'a'-'A';
	}
}

void String::Replace(const unsigned int unicodeOldChar, const unsigned int unicodeNewChar)
{
	if (String::lengthUTF8(unicodeOldChar) == String::lengthUTF8(unicodeNewChar))
	{
		HorseRadish::hChar bufferNew[8];
		unsigned int charNumBytes;

		charNumBytes = String::unicode2utf8(unicodeNewChar, bufferNew);

		for(Iterator it(*this); it.IsLast() == false; ++it)
		{
			if (*it != unicodeOldChar)
				continue;
			
			memcpy(this->actualBuffer + it.byteIndex, bufferNew, charNumBytes);
		}

		return;
	}

	String novaStr;
	unsigned int curChar;

	for(Iterator it(*this); (curChar = it.Read()) != '\0'; )
	{
		if (curChar == unicodeOldChar)
		{
			novaStr += unicodeNewChar;
			continue;
		}
	
		novaStr += curChar;
	}

	this->Set(novaStr);
}

void String::GetSubStr(String &subStr, const unsigned int startChar, const unsigned int numChar) const
{
	subStr.SetEmpty();

	if ((startChar >= this->numChars) || (numChar == 0))
		return;

	auto walker = this->actualBuffer;
	for(unsigned int i=0; i<startChar; i++)
		walker += String::trailingBytesUTF8[(HorseRadish::hUInt8)*walker] + 1;

	int numCharsCopied = 0;
	auto walkerEnd = walker;
	for(unsigned int i=0; (i < numChar) && (*walkerEnd != '\0'); i++, numCharsCopied++)
		walkerEnd += String::trailingBytesUTF8[(HorseRadish::hUInt8)*walkerEnd] + 1;

	if (subStr.adjustRequiredBuffer((unsigned int)(walkerEnd - walker), false) == false)
		return;

	memcpy(subStr.actualBuffer, walker, walkerEnd - walker);

	subStr.numChars = numCharsCopied;
	subStr.numBytes = (unsigned short)(walkerEnd - walker);
	subStr.actualBuffer[subStr.numBytes] = '\0';
}

String String::GetSubStr(const unsigned int startChar, const unsigned int numChar) const
{
	String subStr;
	
	this->GetSubStr(subStr, startChar, numChar);
	return subStr;
}

int String::GetCharByteIndex(const unsigned int pos) const
{
	if (pos == 0)
		return 0;

	for(Iterator it(*this); it.IsLast() == false; ++it)
	{
		if (it.charIndex == pos)
			return it.byteIndex;
	}

	return -1;
}

int String::GetCharPosIndex(const unsigned int byte) const
{
	if (byte == 0)
		return 0;

	for(Iterator it(*this); it.IsLast() == false; ++it)
	{
		if (it.byteIndex == byte)
			return it.charIndex;
	}

	return -1;
}

String& String::operator=(const String& s)
{
	this->Set(s);

	return *this;
}

const String String::operator+(const String& s) const
{
	String aux;

	if (aux.adjustRequiredBuffer(this->numBytes + s.numBytes, false) == false)
		return aux;

	memcpy(aux.actualBuffer, this->actualBuffer, this->numBytes);
	memcpy(aux.actualBuffer + this->numBytes, s.actualBuffer, s.numBytes + 1);
	
	aux.numBytes = this->numBytes + s.numBytes;
	aux.numChars = this->numChars + s.numChars;
	return aux;
}

String& String::operator+=(const String& s)
{
	if (s.numBytes <= 0)
		return *this;

	if (this->adjustRequiredBuffer(this->numBytes + s.numBytes, true) == false)
		return *this;

	memcpy(this->actualBuffer + this->numBytes, s.actualBuffer, s.numBytes + 1);
	this->numBytes += s.numBytes;
	this->numChars += s.numChars;

	return *this;
}

String& String::operator+=(const HorseRadish::hChar * const s)
{
	String strAux;

	if ((s == nullptr) || (*s == '\0'))
		return *this;

	strAux.Set(Encoding::UTF8, s);

	if (this->adjustRequiredBuffer(this->numBytes + strAux.numBytes, true) == false)
		return *this;

	memcpy(this->actualBuffer + this->numBytes, strAux.actualBuffer, strAux.numBytes + 1);
	this->numBytes += strAux.numBytes;
	this->numChars += strAux.numChars;

	return *this;
}

String& String::operator+=(const unsigned int unicodeChar)
{
	if (unicodeChar <= 0)
		return *this;

	if (this->adjustRequiredBuffer(this->numBytes + String::lengthUTF8(unicodeChar), true) == false)
		return *this;

	HorseRadish::hChar *writer = this->actualBuffer + this->numBytes;

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

	*writer='\0';

	return *this;
}

bool String::operator==(const String& s) const
{
	if ((this->numBytes != s.numBytes) || (this->numChars != s.numChars))
		return false;

	for(Iterator it1(*this), it2(s); it1.IsLast() == false; ++it1, ++it2)
	{
		if (*it1 != *it2)
			return false;
	}

	return true;
}

bool String::operator!=(const String& s) const
{
	return (!((*this) == s));
}

bool String::operator==(const HorseRadish::hChar * const s) const
{
	if (s == nullptr)
		return false;

	return (strcmp((const char*)this->actualBuffer, (const char*)s) == 0);
}

bool String::operator!=(const HorseRadish::hChar * const s) const
{
	return (!((*this) == s));
}

bool String::operator==(const char * const s) const
{
	if (s == nullptr)
		return false;

	return (strcmp((const char*)this->actualBuffer, (const char*)s) == 0);
}

bool String::operator!=(const char * const s) const
{
	return (!((*this) == s));
}

unsigned int String::operator[](int index) const
{
	return this->GetUnicodeAt(index);
}

bool String::StartsWith(const String &compareTo) const
{
	if (this->numBytes < compareTo.numBytes)
		return false;

	for(unsigned short i=0; i<compareTo.numBytes && compareTo.actualBuffer[i]!='\0'; i++)
	{
		if (this->actualBuffer[i] != compareTo.actualBuffer[i])
			return false;
	}

	return true;
}

bool String::StartsWith(const HorseRadish::hChar * const compareTo) const
{
	if ((compareTo == nullptr) || (*compareTo == '\0'))
		return false;

	for(unsigned short i = 0; ((i < this->numBytes) && (this->actualBuffer[i] != '\0')); i++)
	{
		if (compareTo[i] == '\0')
			return true;

		if (this->actualBuffer[i] != compareTo[i])
			return false;
	}

	return false;
}

bool String::StartsWith(const unsigned int &unicodeChar) const
{
	Iterator it(*this);

	return (it.Read() == unicodeChar);
}

bool String::EndsWith(const String &compareTo) const
{
	if (this->numBytes < compareTo.numBytes)
		return false;

	for(int i = compareTo.numBytes - 1, j = this->numBytes - 1; i >= 0; i--, j--)
	{
		if (this->actualBuffer[j] != compareTo.actualBuffer[i])
			return false;
	}

	return true;
}

bool String::EndsWith(const unsigned int &unicodeChar) const
{
	Iterator it(*this);
	it--;

	return (it.Read() == unicodeChar);
}

void String::SetEmpty()
{
	this->bufferLocal[0]='\0';
	this->bufferHeap.clear();
	this->actualBuffer = this->bufferLocal;
	this->numBytes = this->numChars = 0;
}

int String::Set(const Encoding &flagFormat, const void * const nova, const int &numCharToEncode)
{
	unsigned int metaNumChar;

	this->numBytes = this->numChars = 0;
	this->bufferLocal[0]='\0';
	this->bufferHeap.clear();
	this->actualBuffer = this->bufferLocal;

	if ((nova == nullptr) || ((*((const char *)nova)) == '\0') || (numCharToEncode == 0))
		return 0;

	metaNumChar = (numCharToEncode < 0) ? 0xFFFFFFFF : numCharToEncode;
	metaNumChar = (metaNumChar > 0xFFFFFFFF) ? 0xFFFFFFFF : metaNumChar;

	if (flagFormat == String::Encoding::UTF8)
	{
		auto srcLength = strlen((const char*)nova);
		if (this->adjustRequiredBuffer(srcLength, false) == false)
			return 0;

		memcpy(this->actualBuffer, nova, srcLength + 1);

		if (String::validateString(this->actualBuffer, &this->numChars,&this->numBytes)==false)
		{
			this->SetEmpty();
			return -1;
		}

		if (metaNumChar < 0xFFFFFFFF)
			this->CloseAt(metaNumChar);

		return this->numChars;
	}

	if (flagFormat == String::Encoding::ASCII)
	{
		for (auto walker = reinterpret_cast<const unsigned char*>(nova); *walker != '\0'; walker++)
		{
			if (this->numChars >= metaNumChar)
				break;

			if ((this->numBytes + 3) > sizeof(this->bufferLocal))
			{
				if (this->adjustRequiredBuffer(this->numBytes + 2, true) == false)
				{
					this->SetEmpty();
					return -1;
				}
			}

			auto curChar = *walker;
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

		this->actualBuffer[this->numBytes] = '\0';
		return this->numChars;
	}
	
	if (flagFormat == String::Encoding::UTF16)
	{
		for(auto walker = reinterpret_cast<const unsigned short*>(nova); *walker != '\0'; walker++)
		{
			if (this->numChars >= metaNumChar)
				break;

			if ((this->numBytes + 4) > sizeof(this->bufferLocal))
			{
				if (this->adjustRequiredBuffer(this->numBytes + 3, true) == false)
				{
					this->SetEmpty();
					return -1;
				}
			}

			auto curChar = *walker;
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

		this->actualBuffer[this->numBytes] = '\0';
		return this->numChars;
	}

	if (flagFormat == String::Encoding::UTF32)
	{
		for (auto walker = reinterpret_cast<const unsigned int*>(nova); *walker != '\0'; walker++)
		{
			if (this->numChars >= metaNumChar)
				break;

			if ((this->numBytes + 5) > sizeof(this->bufferLocal))
			{
				if (this->adjustRequiredBuffer(this->numBytes + 4, true) == false)
				{
					this->SetEmpty();
					return -1;
				}
			}

			auto curChar = *walker;
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

		this->actualBuffer[this->numBytes] = '\0';
		return this->numChars;
	}

	if (flagFormat == String::Encoding::Windows)
	{
		int bytesNecessarios;

		bytesNecessarios = WideCharToMultiByte(CP_UTF8, 0, (LPCWSTR)nova, -1, nullptr, 0, nullptr, nullptr);
		if (bytesNecessarios <= 0)
			return -1;

		if (this->adjustRequiredBuffer(bytesNecessarios, false) == false)
			return -1;

		WideCharToMultiByte(CP_UTF8, 0, (LPCWSTR)nova, -1, (LPSTR)this->actualBuffer, bytesNecessarios, nullptr, nullptr);

		if (String::validateString(this->actualBuffer, &this->numChars, &this->numBytes) == false)
		{
			this->SetEmpty();
			return -1;
		}

		return this->numChars;
	}

	return 0;
}

int String::Set(const HorseRadish::hChar * const nova)
{
	this->numBytes = this->numChars = 0;
	this->bufferLocal[0]='\0';
	this->bufferHeap.clear();
	this->actualBuffer = this->bufferLocal;

	if ((nova == nullptr) || (*nova == '\0'))
		return 0;
	
	auto srcLength = strlen((const char*)nova);
	if (this->adjustRequiredBuffer(srcLength, false) == false)
		return 0;

	memcpy(this->actualBuffer, nova, srcLength + 1);

	if (String::validateString(this->actualBuffer, &this->numChars,&this->numBytes)==false)
	{
		this->SetEmpty();
		return -1;
	}

	return this->numChars;
}

int String::Set(const String &nova)
{
	if (&nova == this)
		return this->numChars;

	this->numBytes = this->numChars = 0;
	this->bufferLocal[0]='\0';
	this->bufferHeap.clear();
	this->actualBuffer = this->bufferLocal;

	if (nova.numBytes < 0)
		return 0;

	if (this->adjustRequiredBuffer(nova.numBytes, false) == false)
		return 0;

	this->numBytes = nova.numBytes;
	this->numChars = nova.numChars;

	memcpy(this->actualBuffer, nova.actualBuffer, this->numBytes + 1);

	return this->numChars;
}

int String::SetPrintf(const Encoding flagFormat, const char *fmt, ...)
{
	va_list ap;
	int bytesWritten;
	char auxBuffer[1024];

	this->numBytes = this->numChars = 0;
	this->bufferLocal[0]='\0';
	this->bufferHeap.clear();
	this->actualBuffer = this->bufferLocal;

	if (fmt == nullptr)
		return 0;

	va_start(ap, fmt);
		bytesWritten = vsnprintf_s(auxBuffer, sizeof(auxBuffer), _TRUNCATE, fmt, ap);
	va_end(ap);

	if (bytesWritten <= 0)
		return 0;

	return this->Set(flagFormat, auxBuffer);
}

int String::SetLine(const Encoding flagFormat, const void * const from)
{
	unsigned int curChar;

	this->Set(flagFormat, from);

	Iterator it(*this);
	for(; (curChar = *it) != '\0'; ++it)
	{
		if (curChar==10 || curChar==13)
			break;
	}

	this->numBytes = it.byteIndex;
	this->numChars = it.charIndex;
	this->actualBuffer[this->numBytes] = '\0';

	return this->numChars;
}

void String::SetTime(const unsigned int seconds)
{
	unsigned int horas, min, sec;

	horas = min = 0;
	sec = seconds;
	while (sec >= 3600)
	{
		horas++;
		sec -= 3600;
	}
	while (sec >= 60)
	{
		min++;
		sec -= 60;
	}

	this->SetPrintf(String::Encoding::ASCII, "%d:%02d:%02d", horas, min, sec);
}

void String::SetHexDump(const void * const buffer, const size_t bufferSize)
{
	char auxBuffer[1024];

	this->SetEmpty();

	if ((buffer == nullptr) || (bufferSize == 0))
		return;

	auto bufferWalker = reinterpret_cast<const unsigned char*>(buffer);

	size_t numBytesEncoded = 0;

	while (true)
	{
		sprintf_s(auxBuffer, sizeof(auxBuffer), "%016X", numBytesEncoded);
		*this += (const HorseRadish::hChar*)auxBuffer;

		*this += ' ';

		while (numBytesEncoded < bufferSize)
		{
			*this += ' ';
			Encoders::EncodeHex(bufferWalker, 1, true, *this);
		
			numBytesEncoded++;
			bufferWalker++;

			if ((numBytesEncoded % 16) == 0)
				break;
		}

		if (numBytesEncoded >= bufferSize)
			break;

		*this += HorseRadish::Platform::NewLine;
	}
}

void String::SetMemory(const unsigned int bytes, const bool useMetric)
{
	auto bytesDouble = (double)bytes;

	double aux = bytesDouble * (useMetric ? 0.000000001 : 0.000000000931322574615478515625);		//1.0 / 1000000000.0 or 1.0 / 1073741824.0
	if (aux >= 1.0f)
	{
		this->SetPrintf(String::Encoding::ASCII, "%.2f GB", aux);
		return;
	}

	aux = bytesDouble * (useMetric ? 0.000001 : 0.00000095367431640625);		//1.0 / 1000000.0 or 1.0 / 1048576.0;
	if (aux >= 1.0f)
	{
		this->SetPrintf(String::Encoding::ASCII, "%.2f MB", aux);
		return;
	}

	aux = bytesDouble * (useMetric ? 0.001 : 0.0009765625);		//1.0 / 1000.0 or 1.0 / 1024.0
	if (aux >= 1.0f)
	{
		this->SetPrintf(String::Encoding::ASCII, "%.2f KB", aux);
		return;
	}
	
	this->SetPrintf(String::Encoding::ASCII, "%d bytes", bytes);
}

void String::SetCurrentTime()
{
	tm newtime;
	__time32_t aclock;

	_time32(&aclock);
	_localtime32_s(&newtime,&aclock);

	this->SetPrintf(String::Encoding::ASCII, "%d:%.2d:%.2d", newtime.tm_hour, newtime.tm_min, newtime.tm_sec);
}

void String::SetCurrentDate()
{
	tm newtime;
	__time32_t aclock;

	_time32(&aclock);
	_localtime32_s(&newtime,&aclock);

	this->SetPrintf(String::Encoding::ASCII, "%.2d/%.2d/%d", newtime.tm_mday, newtime.tm_mon + 1, newtime.tm_year + 1900);
}

void String::RemoveAllChars(const unsigned int unicodeChar)
{
	unsigned int curChar, curCharSize, newBytes, newChars;

	newBytes = newChars = 0;
	for(Iterator it(*this); (curChar = it.Read(curCharSize)) != '\0'; )
	{
		if (curChar == unicodeChar)
			continue;

		if ((newBytes + curCharSize) >= it.byteIndex)
		{
			newBytes = it.byteIndex;
			newChars++;
			continue;
		}

		memcpy(this->actualBuffer + newBytes, this->actualBuffer + it.byteIndex - curCharSize, curCharSize);
		newBytes += curCharSize;
		newChars++;
	}

	this->actualBuffer[newBytes]='\0';
	this->numBytes = newBytes;
	this->numChars = newChars;
}

void String::RemoveDoubles(const unsigned int unicodeChar)
{
	unsigned int curChar, lastChar, curCharSize, newBytes, newChars;

	newBytes = newChars = 0;
	lastChar = 0;
	for(Iterator it(*this); (curChar = it.Read(curCharSize)) != '\0'; )
	{
		if ((curChar == unicodeChar) && (lastChar == unicodeChar))
			continue;

		lastChar = curChar;

		if ((newBytes + curCharSize) >= it.byteIndex)
		{
			newBytes = it.byteIndex;
			newChars++;
			continue;
		}

		memcpy(this->actualBuffer + newBytes, this->actualBuffer + it.byteIndex - curCharSize, curCharSize);
		newBytes += curCharSize;
		newChars++;
	}

	this->actualBuffer[newBytes] = '\0';
	this->numBytes = newBytes;
	this->numChars = newChars;
}

void String::RemoveAt(const unsigned int posChar)
{
	unsigned int charByteIndex, charByteSize;

	if (posChar >= this->numChars)
		return;

	charByteIndex = charByteSize = 0;

	for(Iterator it(*this); it.IsLast() == false; ++it)
	{
		if (it.charIndex != posChar)
			continue;
		
		charByteIndex = it.byteIndex;
		charByteSize = String::lengthUTF8(*it);
		break;
	}

	if (charByteSize == 0)
		return;

	for(; this->actualBuffer[charByteIndex - 1] != '\0'; charByteIndex++)
		this->actualBuffer[charByteIndex] = this->actualBuffer[charByteIndex + charByteSize];

	this->numBytes -= charByteSize;
	this->numChars--;
}

void String::RemoveAt(const unsigned int posChar, const unsigned int numToDelete)
{
	unsigned int charByteIndex, charsByteSize, charsDeleted;

	if ((posChar >= numChars) || (numToDelete <= 0))
		return;

	charByteIndex = charsByteSize = charsDeleted = 0;

	for(Iterator it(*this); it.IsLast() == false; ++it)
	{
		if (it.charIndex != posChar)
			continue;

		charByteIndex = it.byteIndex;

		for(unsigned int i=0; (i<numToDelete) && (it.IsLast() == false); i++, ++it)
		{
			charsByteSize += String::lengthUTF8(*it);
			charsDeleted++;
		}

		break;
	}

	if ((charsByteSize == 0) || (charsDeleted == 0))
		return;

	for(; this->actualBuffer[charByteIndex - 1] != '\0'; charByteIndex++)
		this->actualBuffer[charByteIndex] = this->actualBuffer[charByteIndex + charsByteSize];

	this->numBytes -= charsByteSize;
	this->numChars -= charsDeleted;
}

void String::Remove(const unsigned int numberChar, const Position flagPosition)
{
	if (numberChar==0)
		return;

	if (flagPosition == String::Position::End)
	{
		if (numberChar >= this->numChars)
		{
			this->SetEmpty();
			return;
		}

		auto walker = this->actualBuffer;
		for(unsigned int curChar = 0; curChar < (this->numChars - numberChar); curChar++)
			walker += String::trailingBytesUTF8[(HorseRadish::hUInt8)*walker] + 1;

		auto numBytesRemovidos = (unsigned int)(this->actualBuffer + this->numBytes - walker);

		*walker = '\0';

		this->numChars -= numberChar;
		this->numBytes -= numBytesRemovidos;
		return;
	}

	if (flagPosition == String::Position::Start)
	{
		if (numberChar >= this->numChars)
		{
			this->SetEmpty();
			return;
		}

		auto walker = this->actualBuffer;
		for (unsigned int i = 0; i < numberChar; i++)
			walker += String::trailingBytesUTF8[(HorseRadish::hUInt8)*walker] + 1;

		unsigned int i;
		for (i = 0; walker[i] != '\0'; i++)
			this->actualBuffer[i] = walker[i];
		this->actualBuffer[i] = '\0';

		auto numBytesRemovidos = (unsigned int)(walker - this->actualBuffer);

		this->numChars -= numberChar;
		this->numBytes -= numBytesRemovidos;
		return;
	}

	if (flagPosition == String::Position::Stend)
	{
		if ((numberChar * 2) >= numChars)
		{
			this->SetEmpty();
			return;
		}

		this->Remove(numberChar, String::Position::End);
		this->Remove(numberChar, String::Position::Start);
		return;
	}
}

void String::AppendAtStart(const String &appendStr)
{
	if (appendStr.numBytes == 0)
		return;

	if (this->adjustRequiredBuffer(this->numBytes + appendStr.numBytes, true) == false)
		return;

	auto walkerWrite = this->actualBuffer + this->numBytes + appendStr.numBytes;
	auto walkerRead = this->actualBuffer + this->numBytes;
	for(; walkerRead >= this->actualBuffer; walkerRead--, walkerWrite--)
		*walkerWrite = *walkerRead;

	memcpy(this->actualBuffer, appendStr.actualBuffer, appendStr.numBytes);

	this->numBytes += appendStr.numBytes;
	this->numChars += appendStr.numChars;
}

void String::AppendAtStart(const unsigned int unicodeNewChar)
{
	if (unicodeNewChar == 0)
		return;

	auto newCharSize = String::lengthUTF8(unicodeNewChar);

	if (this->adjustRequiredBuffer(this->numBytes + newCharSize, true) == false)
		return;

	auto walkerWrite = this->actualBuffer + this->numBytes + newCharSize;
	auto walkerRead = this->actualBuffer + this->numBytes;
	for(;walkerRead >= this->actualBuffer; walkerRead--, walkerWrite--)
		*walkerWrite = *walkerRead;

	String::unicode2utf8(unicodeNewChar, this->actualBuffer);

	this->numBytes += newCharSize;
	this->numChars++;
}

void String::AppendAtEnd(const String &appendStr)
{
	if (appendStr.numBytes == 0)
		return;

	if (this->adjustRequiredBuffer(this->numBytes + appendStr.numBytes, true) == false)
		return;

	memcpy(this->actualBuffer + this->numBytes, appendStr.actualBuffer, appendStr.numBytes + 1);
	
	this->numBytes += appendStr.numBytes;
	this->numChars += appendStr.numChars;
}

void String::AppendAtEnd(const unsigned int unicodeNewChar)
{
	if (unicodeNewChar==0)
		return;

	auto newCharSize = String::lengthUTF8(unicodeNewChar);

	if (this->adjustRequiredBuffer(this->numBytes + newCharSize, true) == false)
		return;

	String::unicode2utf8(unicodeNewChar, this->actualBuffer + this->numBytes);
	this->actualBuffer[this->numBytes + newCharSize] = '\0';
	
	this->numBytes += newCharSize;
	this->numChars++;
}

void String::AppendAtPos(const String &appendStr, const unsigned int pos)
{
	if (appendStr.numBytes == 0)
		return;

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

	if (this->adjustRequiredBuffer(this->numBytes + appendStr.numBytes, true) == false)
		return;

	Iterator it(*this);
	while(it.charIndex < pos)
		++it;

	for(int endBack = this->numBytes; endBack >= it.byteIndex; endBack--)
		this->actualBuffer[endBack + appendStr.numBytes] = this->actualBuffer[endBack];

	memcpy(this->actualBuffer + it.byteIndex, appendStr.actualBuffer, appendStr.numBytes);

	this->numBytes += appendStr.numBytes;
	this->numChars += appendStr.numChars;
}

void String::AppendAtPos(const unsigned int unicodeNewChar, const unsigned int pos)
{
	if (unicodeNewChar == 0)
		return;

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

	Iterator it(*this);
	while(it.charIndex < pos)
		++it;

	auto newCharSize = String::lengthUTF8(unicodeNewChar);

	for(int endBack = this->numBytes; endBack >= it.byteIndex; endBack--)
		this->actualBuffer[endBack + newCharSize] = this->actualBuffer[endBack];

	String::unicode2utf8(unicodeNewChar, this->actualBuffer + it.byteIndex);

	this->numBytes += newCharSize;
	this->numChars++;
}

void String::CloseAt(const unsigned int charPos)
{
	if (charPos == 0)
	{
		this->SetEmpty();
		return;
	}

	auto walker = this->actualBuffer;
	for(unsigned int i=0; i<charPos; i++)
		walker += String::trailingBytesUTF8[(HorseRadish::hUInt8)*walker] + 1;

	*walker = '\0';

	this->numBytes = (unsigned short)(walker - this->actualBuffer);
	this->numChars = charPos;
}

bool String::IsEmpty(const bool checkOnlyWhiteSpaces) const
{
	if (this->numBytes <= 0)
		return true;

	if (checkOnlyWhiteSpaces == false)
		return false;

	for(const HorseRadish::hChar *walker = this->actualBuffer; *walker != '\0'; walker++)
	{
		if (*walker != ' ')
			return false;
	}

	return true;
}

unsigned int String::Capacity(const unsigned int capacityNeeded)
{
	this->adjustRequiredBuffer(capacityNeeded, true);
	return this->numBytes;
}

float String::ToFloat() const
{
	bool exponent, expMinus;
	int casasExp;
	float up, down, resultado, casasDecimais;

	auto walker = this->actualBuffer;
	while (*walker == ' ' && *walker != '\0') walker++;

	auto minus = false;
	if (*walker == '-' || *walker == '+')
	{
		if (*walker == '-')
			minus = true;
		walker++;
	}

	casasExp = 0;
	up = down = 0.0f;
	casasDecimais = 1.0f;

	while (*walker >= '0' && *walker <= '9')
	{
		up *= 10.0f;
		up += (float)((*walker) - '0');
		walker++;
	}

	if (*walker == '.')
	{
		walker++;
		while (*walker >= '0' && *walker <= '9')
		{
			down *= 10.0f;
			down += (float)((*walker) - '0');
			casasDecimais *= 10.0f;
			walker++;
		}
	}

	exponent = expMinus = false;
	if (*walker == 'd' || *walker == 'D' || *walker == 'e' || *walker == 'E')
		exponent = true;

	if (exponent)
	{
		walker++;
		if (*walker == '-' || *walker == '+')
		{
			if (*walker == '-')
				expMinus = true;
			walker++;
		}

		while (*walker >= '0' && *walker <= '9')
		{
			casasExp *= 10;
			casasExp += (*walker) - '0';
			walker++;
		}
	}

	if (!exponent)
	{
		resultado = down / casasDecimais;
		resultado += up;
		if (minus)
			resultado = -resultado;
		return resultado;
	}

	resultado = down / casasDecimais;
	resultado += up;
	if (minus)
		resultado = -resultado;
	if (expMinus)
	{
		while ((casasExp--) > 0)	resultado /= 10;
		return resultado;
	}
	while ((casasExp--) > 0)	resultado *= 10;
	return resultado;
}

int String::ToInt() const
{
	auto walker = this->actualBuffer;
	while (*walker == ' ' && *walker != '\0') walker++;

	auto minus = false;
	if (*walker == '-' || *walker == '+')
	{
		if (*walker == '-')
			minus = true;
		walker++;
	}

	int val = 0;

	while (*walker >= '0' && *walker <= '9')
	{
		val *= 10;
		val += (*walker) - '0';
		walker++;
	}

	return (minus ? -val : val);
}

int String::ToHex() const
{
	auto walker = this->actualBuffer;
	while (*walker == ' ' && *walker != '\0') walker++;

	auto minus = false;
	if (*walker == '-' || *walker == '+')
	{
		if (*walker == '-')
			minus = true;
		walker++;
	}

	int val = 0;

	while ((*walker >= '0' && *walker <= '9') || (*walker >= 'a' && *walker <= 'f') || (*walker >= 'A' && *walker <= 'F'))
	{
		val *= 16;
		if (*walker >= 'a' && *walker <= 'f')
			val += (*walker) - 'a' + 10;
		else if (*walker >= 'A' && *walker <= 'F')
			val += (*walker) - 'A' + 10;
		else
			val += (*walker) - '0';
		walker++;
	}

	return (minus ? -val : val);
}

unsigned int String::GetUnicodeAt(const unsigned int &charPos) const
{
	if (charPos >= this->numChars)
		return 0;

	for(Iterator it(*this); it.IsLast() == false; ++it)
	{
		if (it.charIndex == charPos)
			return *it;
	}

	assert(false);
	return 0;
}

} //HorseRadish