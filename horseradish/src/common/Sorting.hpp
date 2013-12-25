#pragma once
#ifndef __HSORTING__
#define __HSORTING__

#include "Platform.hpp"
#include "Types.hpp"

#include <malloc.h>

namespace HorseRadish
{

class Sorting
{
	static void radixByte0(const int &numero, const int * const HRESTRICT source, int * const HRESTRICT dest);
	static void radixByte1(const int &numero, const int * const HRESTRICT source, int * const HRESTRICT dest);
	static void radixByte2(const int &numero, const int * const HRESTRICT source, int * const HRESTRICT dest);
	static void radixByte3(const int &numero, const int * const HRESTRICT source, int * const HRESTRICT dest);

	template<typename T>
	static unsigned qsPartition(T* const baseArray, unsigned f, unsigned l, const T &pivot)
	{
		T* temp;

		//tenho de allocar espaço para um elemento
		temp = (T*)_malloca(sizeof(T));

		unsigned i = f-1, j = l+1;
		while(true)
		{
			while(pivot < baseArray[--j]);
			while(baseArray[++i] < pivot);
			if(i < j)
			{
				memcpy(temp, baseArray + i, sizeof(T));
				memcpy(baseArray + i, baseArray + j, sizeof(T));
				memcpy(baseArray + j, temp, sizeof(T));
			}
			else
				return j;
		}

		//liberto o espaço temporário
		_freea(temp);
	}
	template<typename T, typename U>
	static unsigned qsPartition(T* const baseArray, unsigned f, unsigned l, const T &pivot, U compareFunc)
	{
		T* temp;

		//tenho de allocar espaço para um elemento
		temp = (T*)_malloca(sizeof(T));

		unsigned i = f-1, j = l+1;
		while(true)
		{
			while(compareFunc(pivot, baseArray[--j]) < 0);
			while(compareFunc(baseArray[++i], pivot) < 0);
			if(i < j)
			{
				memcpy(temp, baseArray + i, sizeof(T));
				memcpy(baseArray + i, baseArray + j, sizeof(T));
				memcpy(baseArray + j, temp, sizeof(T));
			}
			else
				return j;
		}

		//liberto o espaço temporário
		_freea(temp);
	}

	template<typename T, typename U>
	static const T& qsMinElement(const T &elementA, const T &elementB, U compareFunc)
	{
		return ((compareFunc(elementA, elementB) < 0) ? elementA : elementB);
	}

	template<typename T>
	static void qsMedianHybrid(T* const baseArray, unsigned f, unsigned l)
	{
		while(f+16 < l)
		{
			const T &v1 = baseArray[f];
			const T &v2 = baseArray[l];
			const T &v3 = baseArray[(f + l) / 2];

			const T &median = (v1 < v2) ? ( (v3 < v1) ? v1 : ((v2 < v3) ? v2: v3)) : ((v3 < v2) ? v2 : ((v1 < v3) ? v1 : v3));

			unsigned m = qsPartition(baseArray, f, l, median);
			qsMedianHybrid(baseArray, f, m);
			f = m+1;
		}
	}
	template<typename T, typename U>
	static void qsMedianHybrid(T* const baseArray, unsigned f, unsigned l, U compareFunc)
	{
		while(f+16 < l)
		{
			const T &v1 = baseArray[f];
			const T &v2 = baseArray[l];
			const T &v3 = baseArray[(f + l) / 2];

			const T &median = (compareFunc(v1, v2) < 0) ? ( (compareFunc(v3, v1) < 0) ? v1 : qsMinElement(v2, v3, compareFunc)) : ((compareFunc(v3, v2)< 0) ? v2 : qsMinElement(v1, v3, compareFunc));
			
			unsigned m = qsPartition(baseArray, f, l, median, compareFunc);
			qsMedianHybrid(baseArray, f, m, compareFunc);
			f = m+1;
		}
	}

public:

	//§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§
	//§§§§§§ Radix sort (declarações)		§§
	//§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§
	static void RadixSort(int *baseArray, int *tempArray, const unsigned int numElements);
	static void RadixQueue(float *baseArray, float *tempArray, int *orderOut, int *orderTemp, const unsigned int numElements);

	//§§§§§§§§§§§§§§§§§§§§§§§§
	//§§§§§§ QuickSort		§§
	//§§§§§§§§§§§§§§§§§§§§§§§§
	template<typename T>
	static void QuickSort(T* const baseArray, const unsigned int numElements)
	{
		//verifico os parametros
		if ((baseArray == nullptr) || (numElements <= 1))
			return;

		//função recursiva do qsort
		Sorting::qsMedianHybrid<T>(baseArray, 0, numElements - 1);

		//o resto (16 elementos) vai por insertion sort (é mais rápido)
		Sorting::InsertionSort<T>(baseArray, numElements);
	}

	template<typename T, typename U>
	static void QuickSort(T* const baseArray, const unsigned int numElements, U compareFunc)
	{
		//verifico os parametros
		if ((baseArray == nullptr) || (numElements <= 1))
			return;

		//função recursiva do qsort
		Sorting::qsMedianHybrid<T>(baseArray, 0, numElements - 1, compareFunc);

		//o resto (16 elementos) vai por insertion sort (é mais rápido)
		Sorting::InsertionSort<T>(baseArray, numElements, compareFunc);
	}
	
	//§§§§§§§§§§§§§§§§§§§§§§§§§§§§
	//§§§§§§ InsertionSort		§§
	//§§§§§§§§§§§§§§§§§§§§§§§§§§§§
	template<typename T>
	static void InsertionSort(T* const baseArray, const unsigned int numElements)
	{
		T *temp;

		//verifico os parametros
		if ((baseArray == nullptr) || (numElements <= 1))
			return;

		//tenho de allocar espaço para um elemento
		temp = (T*)_malloca(sizeof(T));

		for(int i = 1; i < numElements; ++i)
		{
			memcpy(temp, baseArray + i, sizeof(T));
			
			int j = i;
			while((j > 0) && (*temp < baseArray[j - 1]))
			{
				memcpy(baseArray + j, baseArray + j - 1, sizeof(T));
				--j;
			}

			memcpy(baseArray + j, temp, sizeof(T));
		}

		//liberto o espaço temporário
		_freea(temp);
	}

	template<typename T, typename U>
	static void InsertionSort(T* const baseArray, const unsigned int numElements, U compareFunc)
	{
		T *temp;

		//verifico os parametros
		if ((baseArray == nullptr) || (numElements <= 1) || (compareFunc == nullptr))
			return;

		//tenho de allocar espaço para um elemento
		temp = (T*)_malloca(sizeof(T));

		for(int i = 1; i < numElements; ++i)
		{
			memcpy(temp, baseArray + i, sizeof(T));
			
			int j = i;
			while((j > 0) && (compareFunc(*temp, baseArray[j - 1]) < 0))
			{
				memcpy(baseArray + j, baseArray + j - 1, sizeof(T));
				--j;
			}

			memcpy(baseArray + j, temp, sizeof(T));
		}

		//liberto o espaço temporário
		_freea(temp);
	}

	//§§§§§§§§§§§§§§§§§§§§§§§§
	//§§§§§§ ShellSort		§§
	//§§§§§§§§§§§§§§§§§§§§§§§§
	template <typename T>
	static void ShellSort(T* const baseArray, const unsigned int numElements)
	{
		unsigned int j, increment;
		T *temp;

		//verifico os parametros
		if ((baseArray == nullptr) || (numElements <= 1))
			return;
		
		//tenho de allocar espaço para um elemento
		temp = (T*)_malloca(sizeof(T));

		//o algoritmo começa aqui
		increment = 3;
		while(increment>0)
		{
			for(unsigned int i=0; i<numElements; i++)
			{
				j = i;
				memcpy(temp, baseArray + i,sizeof(T));
				while ((j >= increment) && (baseArray[j - increment] > *temp))
				{
					memcpy(baseArray + j, baseArray + j - increment, sizeof(T));
					j = j - increment;
				}
				memcpy(baseArray + j, temp, sizeof(T));
			}

			if (increment/2 != 0)
				increment = increment/2;
			else if (increment == 1)
				increment = 0;
			else
				increment = 1;
		}

		//liberto o espaço temporário
		_freea(temp);
	}

	template <typename T, typename U>
	static void ShellSort(T* const baseArray, const unsigned int numElements, U compareFunc)
	{
		unsigned int j, increment;
		T *temp;

		//verifico os parametros
		if ((baseArray == nullptr) || (numElements <= 1) || (compareFunc == nullptr))
			return;
		
		//tenho de allocar espaço para um elemento
		temp = (T*)_malloca(sizeof(T));

		//o algoritmo começa aqui
		increment = 3;
		while(increment>0)
		{
			for(unsigned int i=0; i<numElements; i++)
			{
				j = i;
				memcpy(temp, baseArray + i,sizeof(T));
				while ((j >= increment) && (compareFunc(baseArray[j - increment], *temp) > 0))
				{
					memcpy(baseArray + j, baseArray + j - increment, sizeof(T));
					j = j - increment;
				}
				memcpy(baseArray + j, temp, sizeof(T));
			}

			if (increment/2 != 0)
				increment = increment/2;
			else if (increment == 1)
				increment = 0;
			else
				increment = 1;
		}

		//liberto o espaço temporário
		_freea(temp);
	}

	//§§§§§§§§§§§§§§§§§§§§§§§§§§§§
	//§§§§§§ Binary search		§§
	//§§§§§§§§§§§§§§§§§§§§§§§§§§§§
	template <typename T>
	static int BinarySearch(T* const baseArray, const unsigned int numElements, const T& compareElement)
	{
		T *left, *right, *middle;

		//verifico os parametros
		if ((baseArray == nullptr) || (numElements <= 0))
			return -1;

		//posso já comparar com o primeiro e com o último
		if (compareElement == *baseArray)
			return 0;
		if (compareElement < *baseArray)
			return -1;
		if (compareElement == baseArray[numElements - 1])
			return (numElements - 1);
		if (compareElement > baseArray[numElements - 1])
			return -1;

		//arranjo ponteiros
		left = baseArray;
		right = baseArray + numElements;
		
		//procuro
		while(left < right)
		{
			//calculo o meio
			middle = left + (right - left) / 2;

			//se achei
			if (compareElement == *middle)
				return (middle - baseArray);

			//ou avança para um lado ou para o outro
			if (compareElement > *middle)
				left = middle + 1;
			else
				right = middle;
		}

		//é pena, nao existe
		return -1;
	}

	template <typename T, typename U>
	static int BinarySearch(T* const baseArray, const unsigned int numElements, int (*compareFunc)(const U&, const T&), const U& compareElement)
	{
		T *left, *right, *middle;
		int resultado;

		//verifico os parametros
		if ((baseArray == nullptr) || (numElements <= 0) || (compareFunc == nullptr))
			return -1;

		//posso já comparar com o primeiro e com o último
		resultado = compareFunc(compareElement, *baseArray);
		if (resultado == 0)
			return 0;
		if (resultado < 0)
			return -1;
		resultado = compareFunc(compareElement, baseArray[numElements - 1]);
		if (resultado == 0)
			return (numElements - 1);
		if (resultado > 0)
			return -1;

		//arranjo ponteiros
		left = baseArray;
		right = baseArray + numElements;
		
		//procuro
		while(left < right)
		{
			//calculo o meio
			middle = left + (right - left) / 2;

			//testo com o do meio e se achei
			resultado = compareFunc(compareElement, *middle);
			if (resultado==0)
				return (middle - baseArray);

			//ou avança para um lado ou para o outro
			if (resultado >0)
				left = middle + 1;
			else
				right = middle;
		}

		//é pena, nao existe
		return -1;
	}
};

}//namespace HorseRadish

#endif