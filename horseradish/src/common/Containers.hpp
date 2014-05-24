#pragma once
#ifndef __HCONTAINERS__
#define __HCONTAINERS__

#include <new>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <functional>

#include "Types.hpp"

namespace HorseRadish
{

namespace Containers
{

//§§§§§§§§§§§§§§§§§§§§§§§§§§§§
//§§§§§§ Classe Array		§§
//§§§§§§§§§§§§§§§§§§§§§§§§§§§§
template <typename T>
class Array
{
	T *mainArray;
	unsigned int numElements, numMaxElements, numGrowElements;

	bool addNewElements()
	{
		if (this->numGrowElements <= 0)
			return false;

		auto newBuffer = realloc(this->mainArray, sizeof(T) * (this->numMaxElements + this->numGrowElements));
		if (newBuffer == nullptr)
			return false;

		this->mainArray = (T*)newBuffer;
		this->numMaxElements += this->numGrowElements;
		return true;
	}

public:
	Array(int numStartElements = 0, int numGrowElements = 4)
		: mainArray(nullptr)
		, numElements(0)
		, numMaxElements(0)
		, numGrowElements(numGrowElements)
	{
		if (this->numGrowElements < 4)
			this->numGrowElements = 4;

		if (numStartElements > 0)
		{
			this->mainArray = (T*)malloc(sizeof(T) * numStartElements);
			if (this->mainArray == nullptr)
				return;

			this->numMaxElements = numStartElements;
		}
	}

	Array(Array<T> &&a)
		: mainArray(a.mainArray)
		, numElements(a.numElements)
		, numMaxElements(a.numMaxElements)
		, numGrowElements(a.numGrowElements)
	{
		a.mainArray = nullptr;
		a.numElements = a.numMaxElements = a.numGrowElements = 0;
	}

	~Array()
	{
		this->Clear();

		if (this->mainArray != nullptr)
			free(this->mainArray);

		this->mainArray = nullptr;
		this->numElements = this->numMaxElements = this->numGrowElements = 0;
	}

	Array(Array<T> &a) = delete;
	Array(const Array<T> &a) = delete;

	Array<T>& operator =(const Array<T>&) = delete;

	bool SetCapacity(unsigned int numElements)
	{
		if (numElements <= this->numMaxElements)
			return true;

		auto newBuffer = realloc(this->mainArray, sizeof(T) * numElements);
		if (newBuffer == nullptr)
			return false;

		this->mainArray = (T*)newBuffer;
		this->numMaxElements = numElements;
		return true;
	}

	HFUNC_RESTRICT T* Add()
	{
		if (this->numElements >= this->numMaxElements)
		{
			if (addNewElements() == false)
				return nullptr;
		}

		this->numElements++;

		return new(this->mainArray + this->numElements - 1) T;
	}

	bool Reserve(unsigned int numElements)
	{
		if ((this->numElements + numElements) > this->numMaxElements)
		{
			int newCapacity;

			newCapacity = this->numElements + numElements;
			if ((newCapacity % this->numGrowElements) != 0)
				newCapacity += (this->numGrowElements - (newCapacity % this->numGrowElements));

			if (this->SetCapacity(newCapacity) == false)
				return false;
		}

		auto elementWalker = this->mainArray + this->numElements;
		for(int i = 0; i < numElements; i++, elementWalker++)
			new(elementWalker) T;

		this->numElements += numElements;
		return true;
	}

	void SwapData(Array<T> &swapArray)
	{
		auto oldArray = this->mainArray;
		auto oldNumElements = this->numElements;
		auto oldMaxElements = this->numMaxElements;
		auto oldNumGrowElements = this->numGrowElements;

		this->mainArray = swapArray.mainArray;
		this->numElements = swapArray.numElements;
		this->numMaxElements = swapArray.numMaxElements;
		this->numGrowElements = swapArray.numGrowElements;

		swapArray.mainArray = oldArray;
		swapArray.numElements = oldNumElements;
		swapArray.numMaxElements = oldMaxElements;
		swapArray.numGrowElements = oldNumGrowElements;
	}

	void Clear(bool cleanData = false)
	{
		for(int i = 0; i < this->numElements; i++)
			this->mainArray[i].~T();

		this->numElements = 0;

		if (cleanData == true)
			memset(this->mainArray, 0, sizeof(T) * this->numMaxElements);
	}

	void QuickSort()
	{
		if (this->numElements <= 1)
			return;

		HorseRadish::Sorting::QuickSort<T>(this->mainArray, this->numElements);
	}

	void QuickSort(std::function<int (const T&, const T&)> compareFunc)
	{
		if ((this->numElements <= 1) || !compareFunc)
			return;

		HorseRadish::Sorting::QuickSort<T>(this->mainArray, this->numElements, compareFunc);
	}

	int BinarySearch(const T& compareElement) const
	{
		if (this->numElements <= 0)
			return -1;

		return HorseRadish::Sorting::BinarySearch<T>(this->mainArray, this->numElements, compareElement);
	}

	template <typename U>
	int BinarySearch(std::function<int (const T&, const U&)> compareFunc, const U& compareElement) const
	{
		if ((this->numElements <= 0) || !compareFunc)
			return -1;

		return HorseRadish::Sorting::BinarySearch<T, U>(this->mainArray, this->numElements, compareFunc, compareElement);
	}

	inline T& operator[](const hPrtDiff &index) const { assert(index < this->numElements); assert(index >= 0); return this->mainArray[index]; }
	inline T* operator+(const hPrtDiff &index) const { assert(index < this->numElements); assert(index >= 0); return this->mainArray + index; }

	inline T* GetMainPointer(void) const {	return this->mainArray;	}
	inline int GetNumElements(void) const {	return this->numElements; }
	inline int GetSize(void) const { return (sizeof(T) * this->numElements); }
	inline int GetSizeAllocated(void) const { return (sizeof(T) * this->numMaxElements); }
	inline bool IsEmpty(void) const { return (this->numElements <= 0); }
};

//especialização para caracteres (char)
template <>
class Array<char>
{
	char *mainArray;
	unsigned int numElements, numMaxElements, numGrowElements;

	bool addNewElements()
	{
		if (this->numGrowElements <= 0)
			return false;

		auto newBuffer = realloc(this->mainArray, sizeof(char) * (this->numMaxElements + this->numGrowElements));
		if (newBuffer == nullptr)
			return false;

		this->mainArray = (char*)newBuffer;
		this->numMaxElements += this->numGrowElements;
		return true;
	}

public:
	Array(int numStartElements = 0, int numGrowElements = 25)
		: mainArray(nullptr)
		, numElements(0)
		, numMaxElements(0)
		, numGrowElements(numGrowElements)
	{
		if (this->numGrowElements < 4)
			this->numGrowElements = 4;

		if (numStartElements > 0)
		{
			this->mainArray = (char*)malloc(sizeof(char) * numStartElements);
			if (this->mainArray == nullptr)
				return;

			this->numMaxElements = numStartElements;
		}
	}

	Array(Array<char> &&a)
		: mainArray(a.mainArray)
		, numElements(a.numElements)
		, numMaxElements(a.numMaxElements)
		, numGrowElements(a.numGrowElements)
	{
		a.mainArray = nullptr;
		a.numElements = a.numMaxElements = a.numGrowElements = 0;
	}

	~Array()
	{
		if (this->mainArray != nullptr)
			free(this->mainArray);

		this->mainArray = nullptr;
		this->numElements = this->numMaxElements = this->numGrowElements = 0;
	}

	Array(Array<char> &a) = delete;
	Array(const Array<char> &a) = delete;

	Array<char>& operator =(const Array<char>&) = delete;

	bool SetCapacity(unsigned int numElements)
	{
		if (numElements <= this->numMaxElements)
			return true;

		auto newBuffer = realloc(this->mainArray, sizeof(char) * numElements);
		if (newBuffer == nullptr)
			return false;

		this->mainArray = (char*)newBuffer;
		this->numMaxElements = numElements;
		return true;
	}

	HFUNC_RESTRICT char* Add()
	{
		//se não tenho espaço
		if (this->numElements >= this->numMaxElements)
		{
			//tento criar mais espaço
			if (addNewElements() == false)
				return nullptr;
		}

		//ajusto o número de elementos
		this->numElements++;

		//deolo o ponteiro para o elemento
		return (this->mainArray + this->numElements - 1);
	}

	bool Reserve(unsigned int numElements)
	{
		//se precisar de espaço
		if ((this->numElements + numElements) > this->numMaxElements)
		{
			int newCapacity;

			//calculo o que vou precisar
			newCapacity = this->numElements + numElements;
			if ((newCapacity % this->numGrowElements) != 0)
				newCapacity += (this->numGrowElements - (newCapacity % this->numGrowElements));

			//tenho de aumentar o espaço
			if (this->SetCapacity(newCapacity) == false)
				return false;
		}

		//acabei de criar estes elementos
		this->numElements += numElements;

		//correu tudo bem
		return true;
	}

	void SwapData(Array<char> &swapArray)
	{
		char* oldArray;
		int oldNumElements, oldMaxElements, oldNumGrowElements;

		//guardo todos os dados deste
		oldArray = this->mainArray;
		oldNumElements = this->numElements;
		oldMaxElements = this->numMaxElements;
		oldNumGrowElements = this->numGrowElements;

		//guardo os dados do outro array
		this->mainArray = swapArray.mainArray;
		this->numElements = swapArray.numElements;
		this->numMaxElements = swapArray.numMaxElements;
		this->numGrowElements = swapArray.numGrowElements;

		//e coloco no outro os meus dados antigos
		swapArray.mainArray = oldArray;
		swapArray.numElements = oldNumElements;
		swapArray.numMaxElements = oldMaxElements;
		swapArray.numGrowElements = oldNumGrowElements;
	}

	void Shrink(unsigned int numElements)
	{
		//basta limpar o número de elementos que tenho
		this->numElements -= numElements;
		if (this->numElements < 0)
			this->numElements = 0;
	}

	void Clear(bool cleanData = false)
	{
		this->numElements = 0;

		if (cleanData == true)
			memset(this->mainArray, 0, sizeof(char) * this->numMaxElements);
	}

	inline char& operator[](const hPrtDiff &index) const { assert(index < this->numElements); assert(index >= 0); return this->mainArray[index]; }
	inline char* operator+(const hPrtDiff &index) const { assert(index < this->numElements); assert(index >= 0); return this->mainArray + index; }

	inline char* GetMainPointer(void) const {	return this->mainArray;	}
	inline int GetNumElements(void) const {	return this->numElements; }
	inline int GetSize(void) const { return (sizeof(char) * this->numElements); }
	inline int GetSizeAllocated(void) const { return (sizeof(char) * this->numMaxElements); }
	inline bool IsEmpty(void) const { return (this->numElements <= 0); }
};

//especialização para caracteres (HorseRadish::hChar)
template <>
class Array<HorseRadish::hChar>
{
	HorseRadish::hChar *mainArray;
	unsigned int numElements, numMaxElements, numGrowElements;

	bool addNewElements()
	{
		void *newBuffer;

		//se não tiver um número para crescer
		if (this->numGrowElements <= 0)
			return false;

		//preciso de mais espaço
		newBuffer = realloc(this->mainArray, sizeof(HorseRadish::hChar) * (this->numMaxElements + this->numGrowElements));
		if (newBuffer == nullptr)
			return false;

		//guardo os novos dados
		this->mainArray = (HorseRadish::hChar*)newBuffer;
		this->numMaxElements += this->numGrowElements;
		return true;
	}

public:
	Array(int numStartElements = 0, int numGrowElements = 25)
		: mainArray(nullptr)
		, numElements(0)
		, numMaxElements(0)
		, numGrowElements(numGrowElements)
	{
		if (this->numGrowElements < 4)
			this->numGrowElements = 4;

		//se tiver alguma coisa para allocar imediatamente
		if (numStartElements > 0)
		{
			//tento allocar memória
			this->mainArray = (HorseRadish::hChar*)malloc(sizeof(HorseRadish::hChar) * numStartElements);
			if (this->mainArray == nullptr)
				return;

			//tenho estes elementos disponíveis
			this->numMaxElements = numStartElements;
		}
	}

	Array(Array<HorseRadish::hChar> &&a)
		: mainArray(a.mainArray)
		, numElements(a.numElements)
		, numMaxElements(a.numMaxElements)
		, numGrowElements(a.numGrowElements)
	{
		a.mainArray = nullptr;
		a.numElements = a.numMaxElements = a.numGrowElements = 0;
	}

	~Array()
	{
		if (this->mainArray != nullptr)
			free(this->mainArray);

		this->mainArray = nullptr;
		this->numElements = this->numMaxElements = this->numGrowElements = 0;
	}

	Array(Array<HorseRadish::hChar> &a) = delete;
	Array(const Array<HorseRadish::hChar> &a) = delete;

	Array<HorseRadish::hChar>& operator =(const Array<HorseRadish::hChar>&) = delete;

	bool SetCapacity(unsigned int numElements)
	{
		void *newBuffer;

		//se não é preciso fazer nada
		if (numElements <= this->numMaxElements)
			return true;

		//preciso de mais espaço
		newBuffer = realloc(this->mainArray, sizeof(HorseRadish::hChar) * numElements);
		if (newBuffer == nullptr)
			return false;

		//guardo os novos dados
		this->mainArray = (HorseRadish::hChar*)newBuffer;
		this->numMaxElements = numElements;
		return true;
	}

	HFUNC_RESTRICT HorseRadish::hChar* Add()
	{
		//se não tenho espaço
		if (this->numElements >= this->numMaxElements)
		{
			//tento criar mais espaço
			if (addNewElements() == false)
				return nullptr;
		}

		//ajusto o número de elementos
		this->numElements++;

		//deolo o ponteiro para o elemento
		return (this->mainArray + this->numElements - 1);
	}

	bool Reserve(unsigned int numElements)
	{
		//se precisar de espaço
		if ((this->numElements + numElements) > this->numMaxElements)
		{
			int newCapacity;

			//calculo o que vou precisar
			newCapacity = this->numElements + numElements;
			if ((newCapacity % this->numGrowElements) != 0)
				newCapacity += (this->numGrowElements - (newCapacity % this->numGrowElements));

			//tenho de aumentar o espaço
			if (this->SetCapacity(newCapacity) == false)
				return false;
		}

		//acabei de criar estes elementos
		this->numElements += numElements;

		//correu tudo bem
		return true;
	}

	void SwapData(Array<HorseRadish::hChar> &swapArray)
	{
		HorseRadish::hChar* oldArray;
		int oldNumElements, oldMaxElements, oldNumGrowElements;

		//guardo todos os dados deste
		oldArray = this->mainArray;
		oldNumElements = this->numElements;
		oldMaxElements = this->numMaxElements;
		oldNumGrowElements = this->numGrowElements;

		//guardo os dados do outro array
		this->mainArray = swapArray.mainArray;
		this->numElements = swapArray.numElements;
		this->numMaxElements = swapArray.numMaxElements;
		this->numGrowElements = swapArray.numGrowElements;

		//e coloco no outro os meus dados antigos
		swapArray.mainArray = oldArray;
		swapArray.numElements = oldNumElements;
		swapArray.numMaxElements = oldMaxElements;
		swapArray.numGrowElements = oldNumGrowElements;
	}

	void Shrink(unsigned int numElements)
	{
		//basta limpar o número de elementos que tenho
		this->numElements -= numElements;
		if (this->numElements < 0)
			this->numElements = 0;
	}

	void Clear(bool cleanData = false)
	{
		//basta limpar o número de elementos que tenho
		this->numElements = 0;

		//se for para limpar alguma coisa
		if (cleanData == true)
			memset(this->mainArray, 0, sizeof(HorseRadish::hChar) * this->numMaxElements);
	}

	inline HorseRadish::hChar& operator[](const hPrtDiff &index) const { assert(index < this->numElements); assert(index >= 0); return this->mainArray[index]; }
	inline HorseRadish::hChar* operator+(const hPrtDiff &index) const { assert(index < this->numElements); assert(index >= 0); return this->mainArray + index; }

	inline HorseRadish::hChar* GetMainPointer(void) const {	return this->mainArray;	}
	inline int GetNumElements(void) const {	return this->numElements; }
	inline int GetSize(void) const { return (sizeof(HorseRadish::hChar) * this->numElements); }
	inline int GetSizeAllocated(void) const { return (sizeof(HorseRadish::hChar) * this->numMaxElements); }
	inline bool IsEmpty(void) const { return (this->numElements <= 0); }
};

//especialização para caracteres (unsigned char)
template <>
class Array<unsigned char>
{
	unsigned char *mainArray;
	unsigned int numElements, numMaxElements, numGrowElements;

	bool addNewElements()
	{
		void *newBuffer;

		//se não tiver um número para crescer
		if (this->numGrowElements <= 0)
			return false;

		//preciso de mais espaço
		newBuffer = realloc(this->mainArray, sizeof(unsigned char) * (this->numMaxElements + this->numGrowElements));
		if (newBuffer == nullptr)
			return false;

		//guardo os novos dados
		this->mainArray = (unsigned char*)newBuffer;
		this->numMaxElements += this->numGrowElements;
		return true;
	}

public:
	Array(int numStartElements = 0, int numGrowElements = 25)
		: mainArray(nullptr)
		, numElements(0)
		, numMaxElements(0)
		, numGrowElements(numGrowElements)
	{
		if (this->numGrowElements < 4)
			this->numGrowElements = 4;

		//se tiver alguma coisa para allocar imediatamente
		if (numStartElements > 0)
		{
			//tento allocar memória
			this->mainArray = (unsigned char*)malloc(sizeof(unsigned char) * numStartElements);
			if (this->mainArray == nullptr)
				return;

			//tenho estes elementos disponíveis
			this->numMaxElements = numStartElements;
		}
	}

	Array(Array<unsigned char> &&a)
		: mainArray(a.mainArray)
		, numElements(a.numElements)
		, numMaxElements(a.numMaxElements)
		, numGrowElements(a.numGrowElements)
	{
		a.mainArray = nullptr;
		a.numElements = a.numMaxElements = a.numGrowElements = 0;
	}

	~Array()
	{
		if (this->mainArray != nullptr)
			free(this->mainArray);

		this->mainArray = nullptr;
		this->numElements = this->numMaxElements = this->numGrowElements = 0;
	}

	Array(Array<unsigned char> &a) = delete;
	Array(const Array<unsigned char> &a) = delete;

	Array<unsigned char>& operator =(const Array<unsigned char>&) = delete;

	bool SetCapacity(unsigned int numElements)
	{
		void *newBuffer;

		//se não é preciso fazer nada
		if (numElements <= this->numMaxElements)
			return true;

		//preciso de mais espaço
		newBuffer = realloc(this->mainArray, sizeof(unsigned char) * numElements);
		if (newBuffer == nullptr)
			return false;

		//guardo os novos dados
		this->mainArray = (unsigned char*)newBuffer;
		this->numMaxElements = numElements;
		return true;
	}

	HFUNC_RESTRICT unsigned char* Add()
	{
		//se não tenho espaço
		if (this->numElements >= this->numMaxElements)
		{
			//tento criar mais espaço
			if (addNewElements() == false)
				return nullptr;
		}

		//ajusto o número de elementos
		this->numElements++;

		//deolo o ponteiro para o elemento
		return (this->mainArray + this->numElements - 1);
	}

	bool Reserve(unsigned int numElements)
	{
		//se precisar de espaço
		if ((this->numElements + numElements) > this->numMaxElements)
		{
			int newCapacity;

			//calculo o que vou precisar
			newCapacity = this->numElements + numElements;
			if ((newCapacity % this->numGrowElements) != 0)
				newCapacity += (this->numGrowElements - (newCapacity % this->numGrowElements));

			//tenho de aumentar o espaço
			if (this->SetCapacity(newCapacity) == false)
				return false;
		}

		//acabei de criar estes elementos
		this->numElements += numElements;

		//correu tudo bem
		return true;
	}

	void SwapData(Array<unsigned char> &swapArray)
	{
		unsigned char* oldArray;
		int oldNumElements, oldMaxElements, oldNumGrowElements;

		//guardo todos os dados deste
		oldArray = this->mainArray;
		oldNumElements = this->numElements;
		oldMaxElements = this->numMaxElements;
		oldNumGrowElements = this->numGrowElements;

		//guardo os dados do outro array
		this->mainArray = swapArray.mainArray;
		this->numElements = swapArray.numElements;
		this->numMaxElements = swapArray.numMaxElements;
		this->numGrowElements = swapArray.numGrowElements;

		//e coloco no outro os meus dados antigos
		swapArray.mainArray = oldArray;
		swapArray.numElements = oldNumElements;
		swapArray.numMaxElements = oldMaxElements;
		swapArray.numGrowElements = oldNumGrowElements;
	}

	void Shrink(unsigned int numElements)
	{
		//basta limpar o número de elementos que tenho
		this->numElements -= numElements;
		if (this->numElements < 0)
			this->numElements = 0;
	}

	void Clear(bool cleanData = false)
	{
		//basta limpar o número de elementos que tenho
		this->numElements = 0;

		//se for para limpar alguma coisa
		if (cleanData == true)
			memset(this->mainArray, 0, sizeof(unsigned char) * this->numMaxElements);
	}

	inline unsigned char& operator[](const hPrtDiff &index) const { assert(index < this->numElements); assert(index >= 0); return this->mainArray[index]; }
	inline unsigned char* operator+(const hPrtDiff &index) const { assert(index < this->numElements); assert(index >= 0); return this->mainArray + index; }

	inline unsigned char* GetMainPointer(void) const {	return this->mainArray;	}
	inline int GetNumElements(void) const {	return this->numElements; }
	inline int GetSize(void) const { return (sizeof(unsigned char) * this->numElements); }
	inline int GetSizeAllocated(void) const { return (sizeof(unsigned char) * this->numMaxElements); }
	inline bool IsEmpty(void) const { return (this->numElements <= 0); }
};

//especialização para ponteiros
template <typename T>
class Array<T*>
{
	T **mainArray;
	unsigned int numElements, numMaxElements, numGrowElements;

	bool addNewElements()
	{
		void *newBuffer;

		//se não tiver um número para crescer
		if (this->numGrowElements <= 0)
			return false;

		//preciso de mais espaço
		newBuffer = realloc(mainArray, sizeof(T*) * (this->numMaxElements + this->numGrowElements));
		if (newBuffer == nullptr)
			return false;

		//guardo os novos dados
		this->mainArray = (T**)newBuffer;
		this->numMaxElements += this->numGrowElements;
		return true;
	}

public:
	Array(int numStartElements = 0, int numGrowElements = 20)
		: mainArray(nullptr)
		, numElements(0)
		, numMaxElements(0)
		, numGrowElements(numGrowElements)
	{
		if (this->numGrowElements < 10)
			this->numGrowElements = 10;

		//se tiver alguma coisa para allocar imediatamente
		if (numStartElements > 0)
		{
			//tento allocar memória
			this->mainArray = (T**)malloc(sizeof(T*) * numStartElements);
			if (this->mainArray == nullptr)
				return;

			//tenho estes elementos disponíveis
			this->numMaxElements = numStartElements;
		}
	}

	Array(Array<T*> &&a)
		: mainArray(a.mainArray)
		, numElements(a.numElements)
		, numMaxElements(a.numMaxElements)
		, numGrowElements(a.numGrowElements)
	{
		a.mainArray = nullptr;
		a.numElements = a.numMaxElements = a.numGrowElements = 0;
	}

	~Array()
	{
		if (this->mainArray != nullptr)
			free(this->mainArray);

		this->mainArray = nullptr;
		this->numElements = this->numMaxElements = this->numGrowElements = 0;
	}

	Array(Array<T*> &a) = delete;
	Array(const Array<T*> &a) = delete;

	Array<T*>& operator =(const Array<T*>&) = delete;

	bool SetCapacity(unsigned int numElements)
	{
		void *newBuffer;

		//se não é preciso fazer nada
		if (numElements <= this->numMaxElements)
			return true;

		//preciso de mais espaço
		newBuffer = realloc(mainArray, sizeof(T*) * numElements);
		if (newBuffer == nullptr)
			return false;

		//guardo os novos dados
		this->mainArray = (T**)newBuffer;
		this->numMaxElements = numElements;
		return true;
	}

	T* Add(T* const newElement)
	{
		//se não tenho espaço
		if (this->numElements >= this->numMaxElements)
		{
			//tento criar mais espaço
			if (addNewElements() == false)
				return nullptr;
		}

		//guardo o ponteiro e ajusto o número de elementos
		this->mainArray[this->numElements] = newElement;
		this->numElements++;

		//volto a devolver o ponteiro
		return newElement;
	}

	bool Reserve(unsigned int numElements)
	{
		//se precisar de espaço
		if ((this->numElements + numElements) > this->numMaxElements)
		{
			int newCapacity;

			//calculo o que vou precisar
			newCapacity = this->numElements + numElements;
			if ((newCapacity % this->numGrowElements) != 0)
				newCapacity += (this->numGrowElements - (newCapacity % this->numGrowElements));

			//tenho de aumentar o espaço
			if (this->SetCapacity(newCapacity) == false)
				return false;
		}

		//limpo os que foram pedidos (ficam a nullptr)
		memset(this->mainArray + this->numElements, 0, sizeof(T*) * numElements);

		//acabei de criar estes elementos
		this->numElements += numElements;

		//correu tudo bem
		return true;
	}

	HFUNC_RESTRICT T** Relinquish()
	{
		T** oldArray;

		//guardo o array actual
		oldArray = this->mainArray;

		//limpo isto tudo
		this->mainArray = nullptr;
		this->numElements = 0;
		this->numMaxElements = 0;

		//devolvo o array "antigo"
		return oldArray;
	}

	bool Replace(const int index, T* const newElement)
	{
		//vertifico-me que o elemento existe
		if ((index <0) || (index >= this->numElements))
			return false;

		//substitu-o e pronto
		this->mainArray[index] = newElement;
		return true;
	}

	void Clear()
	{
		//basta limpar o número de elementos que tenho
		this->numElements = 0;

		//e posso eliminar tudo
		memset(this->mainArray, 0, sizeof(T*) * this->numMaxElements);
	}

	inline T* operator[](const hPrtDiff &index) const { assert(index < this->numElements); assert(index >= 0); return this->mainArray[index]; }
	inline T* operator+(const hPrtDiff &index) const { assert(index < this->numElements); assert(index >= 0); return this->mainArray + index; }

	inline T** GetMainPointer(void) const {	return this->mainArray; }
	inline int GetNumElements(void) const {	return this->numElements; }
	inline int GetSize(void) const { return (sizeof(T*) * this->numElements); }
	inline int GetSizeAllocated(void) const { return (sizeof(T*) * this->numMaxElements); }
	inline bool IsEmpty(void) const { return (this->numElements <= 0); }
};

//§§§§§§§§§§§§§§§§§§§§§§§§
//§§§§§§ Classe Pool	§§
//§§§§§§§§§§§§§§§§§§§§§§§§
template <class T>
class Pool
{
	void **pools, *curPool, *curAlloc, *curAllocEnd;
	unsigned int numPools, numAllocs, numPoolElements;

	bool addNewPool()
	{
		void **newPool;

		//mais um pool
		newPool = (void**)realloc(this->pools, sizeof(void*) * (this->numPools + 1));
		if (newPool == nullptr)
			return false;
		
		//espaço nesse pool
		this->pools = newPool;
		this->pools[this->numPools] = malloc(sizeof(T) * this->numPoolElements);
		if (this->pools[this->numPools] == nullptr)
			return false;

		//limpo os dados
		memset(this->pools[this->numPools], 0, sizeof(T) * this->numPoolElements);

		//deu tudo bem, arranjo só os ponteiros
		this->curPool = this->curAlloc = this->pools[this->numPools];
		this->curAllocEnd = ((unsigned char*)this->curPool) + (sizeof(T) * this->numPoolElements);

		//posso avançar com isto
		this->numPools++;
		return true;
	}

public:
	Pool(unsigned int poolMinElements)
		: pools(nullptr)
		, curPool(nullptr)
		, curAllocEnd(nullptr)
		, curAlloc(nullptr)
		, numPools(0)
		, numAllocs(0)
		, numPoolElements(poolMinElements)
	{
		this->pools = (void**)malloc(sizeof(void*));
		if (this->pools == nullptr)
			return;

		this->numPools = 1;
		this->pools[0] = malloc(sizeof(T) * this->numPoolElements);
		if (this->pools[0] == nullptr)
			return;

		memset(this->pools[0], 0, sizeof(T) * this->numPoolElements);

		this->curPool = this->curAlloc = this->pools[0];
		this->curAllocEnd = ((unsigned char*)this->curPool) + (sizeof(T) * this->numPoolElements);
	}

	~Pool()
	{
		while(this->numPools)
		{
			this->numPools--;
			free(this->pools[this->numPools]);
		}

		if (this->pools)
			free(pools);

		this->pools = nullptr;
		this->curPool = this->curAllocEnd = this->curAlloc = nullptr;
		this->numPools = this->numAllocs = 0;
	}

	Pool(Pool<T> &a) = delete;
	Pool(const Pool<T> &a) = delete;

	Pool<T>& operator =(const Pool<T>&) = delete;

	HFUNC_RESTRICT T* Alloc()
	{
		if ( (this->curAlloc >= this->curAllocEnd) && (addNewPool() == false))
			return nullptr;

		this->numAllocs++;
		this->curAlloc = ((unsigned char*)this->curAlloc) + sizeof(T);
		return ((T*)(((unsigned char*)this->curAlloc) - sizeof(T)));
	}

	void Clear()
	{
		while(this->numPools > 1)
		{
			this->numPools--;
			free(this->pools[this->numPools]);
		}

		this->curPool = this->curAlloc = this->pools[0];
		this->curAllocEnd = ((unsigned char*)this->curPool) + (sizeof(T) * this->numPoolElements);
		this->numPools = 1;
		this->numAllocs = 0;
	}

	bool IsReady() const
	{	return ((this->pools != nullptr) && (this->pools[0] != nullptr)); }

	unsigned int GetNumAllocs() const
	{	return this->numAllocs; }
};

//§§§§§§§§§§§§§§§§§§§§§§§§§§§§
//§§§§§§ Classe Queue		§§
//§§§§§§§§§§§§§§§§§§§§§§§§§§§§
template <class T>
class Queue
{
private:
	template <class T>
	struct No
	{
		No<T> *prev;
		No<T> *next;
		T targetData;
	};

	No<T> *first, *last, *curr;
	unsigned int count;

	void insertNodeFirst(No<T> *node)
	{
		if (this->first != nullptr)
			this->first->prev = node;
		else
			this->last = node;

		node->next = this->first;
		node->prev = nullptr;

		this->first = node;
	}

	void insertNodeLast(No<T> *node)
	{
		if (this->last != nullptr)
			this->last->next = node;
		else
			this->first = node;

		node->prev = this->last;
		node->next = nullptr;

		this->last = node;
	}

	void insertNodeBefore(No<T> *at, No<T> *node)
	{
		No<T> *prev = at->prev;
		at->prev = node;
		if (prev)
			prev->next = node;
		else
			this->first = node;
		
		node->next = at;
		node->prev = prev;
	}

	void insertNodeAfter(No<T> *at, No<T> *node)
	{
		No<T> *next = at->next;
		at->next = node;
		if (next)
			next->prev = node;
		else
			this->last = node;

		node->prev = at;
		node->next = next;
	}

	void releaseNode(const No<T> *node)
	{
		if (node->prev == nullptr)
			this->first = node->next;
		else
			node->prev->next = node->next;

		if (node->next == nullptr)
			this->last = node->prev;
		else
			node->next->prev = node->prev;
	}

public:
	Queue()
		: count(0)
		, first(nullptr)
		, last(nullptr)
		, curr(nullptr)
	{
	}

	~Queue()
	{
		Clear();
	}

	Queue(Queue<T> &a) = delete;
	Queue(const Queue<T> &a) = delete;

	Queue<T>& operator =(const Queue<T>&) = delete;

	unsigned int GetCount() const { return this->count; }

	void addFirst(const T targetData)
	{
		No<T> *node = new No<T>;
		
		node->targetData = targetData;
		insertNodeFirst(node);
		count++;
	}

	void addLast(const T targetData)
	{
		No<T> *node = new No<T>;

		node->targetData = targetData;
		insertNodeLast(node);
		count++;
	}

	void insertBeforeCurrent(const T targetData)
	{
		No<T> *node = new No<T>;

		node->targetData = targetData;
		insertNodeBefore(this->curr, node);
		count++;
	}

	void insertAfterCurrent(const T targetData)
	{
		No<T> *node = new No<T>;

		node->targetData = targetData;
		insertNodeAfter(this->curr, node);
		count++;
	}

	bool GoToFirst() { return ((this->curr = this->first) != nullptr); }
	bool GoToLast() { return ((this->curr = this->last) != nullptr); }
	bool GoToPrev() { return ((this->curr = this->curr->prev) != nullptr); }
	bool GoToNext() { return ((this->curr = this->curr->next) != nullptr); }

	bool GoToData (const T targetData)
	{
		this->curr = this->first;
		
		while (this->curr != nullptr)
		{
			if (targetData == this->curr->targetData)
				return true;
			this->curr = this->curr->next;
		}

		return false;
	}

	T GetCurrent() const { return this->curr->targetData; }
	void SetCurrent(const T targetData) { this->curr->targetData = targetData; }

	T GetPrev() const { return this->curr->prev->targetData; }
	T GetNext() const { return this->curr->next->targetData; }
	T GetPrevWrap() const { return ((this->curr->prev != nullptr)? this->curr->prev : this->last)->targetData; }
	T GetNextWrap() const { return ((this->curr->next != nullptr)? this->curr->next : this->first)->targetData; }

	void Clear()
	{
		while (this->first != nullptr)
		{
			this->curr = this->first;
			this->first = this->first->next;
			delete this->curr;
		}

		this->first = this->last = this->curr = nullptr;
		this->count = 0;
	}

	void MoveCurrentToTop()
	{
		if (this->curr != nullptr)
		{
			releaseNode(this->curr);
			insertNodeFirst(this->curr);
		}
	}
};

}//namespace Containers
}//namespace HorseRadish

#endif