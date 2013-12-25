#pragma once
#ifndef __HMEM_CONTAINERS__
#define __HMEM_CONTAINERS__

#include <new>
#include <stdlib.h>
#include <string.h>

#include "Types.hpp"

namespace HorseRadish
{

namespace Memory
{

//§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§
//§§§§§§ Classe PoolGrow		§§
//§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§
class PoolGrow
{
	void **pools, *curPool, *curAllocEnd, *curAlloc;
	unsigned int numPools,numAllocs,growSize;

	bool addNewPool()
	{
		void **newPool;

		//mais um pool
		newPool=(void**)realloc(pools,sizeof(void*)*(numPools+1));
		if (newPool==nullptr)
			return false;
		
		//espaço nesse pool
		pools=newPool;
		pools[numPools]=malloc(growSize);
		if (pools[numPools]==nullptr)
			return false;

		//limpo os dados
		memset(pools[numPools],0,growSize);

		//deu tudo bem, arranjo só os ponteiros
		curPool = curAlloc = pools[numPools];
		curAllocEnd = ((unsigned char*)curPool) + growSize;

		//posso avançar com isto
		numPools++;
		return true;
	}

public:
	PoolGrow(unsigned int growthSize)
	{
		//limpo tudo
		pools=nullptr;
		curPool=curAllocEnd=curAlloc=nullptr;
		numPools=numAllocs=0;
		growSize=growthSize;

		//quero pelo menos um pool
		pools=(void**)malloc(sizeof(void*));
		if (pools==nullptr)
			return;

		//já tenho um pool
		numPools=1;
		pools[0]=malloc(growSize);
		if (pools[0]==nullptr)
			return;

		//limpo os dados
		memset(pools[0], 0, growSize);

		//deu tudo bem, arranjo só os ponteiros
		curPool = curAlloc = pools[0];
		curAllocEnd = ((unsigned char*)curPool) + growSize;
	}
	~PoolGrow()
	{
		while(numPools)
		{
			numPools--;
			free(pools[numPools]);
		}
		if (pools)
			free(pools);
		pools=nullptr;
		curAllocEnd=curAlloc=nullptr;
		numPools=numAllocs=0;
	}

	bool isReadyToPerform() const
	{
		if (pools!=nullptr && pools[0]!=nullptr)
			return true;
		return false;
	}

	unsigned int getNumAllocs() const
	{	return numAllocs;	}

	HFUNC_RESTRICT void* alloc(unsigned int size)
	{
		//esta classe não foi usada para isto
		if (size >= growSize)
			return nullptr;

		//se já acabou, tenho de arranjar mais pools
		if ( ((((unsigned char*)curAlloc)+size)>=curAllocEnd) && (addNewPool()==false))
			return nullptr;

		//basta devolver
		numAllocs++;
		curAlloc=((unsigned char*)curAlloc)+size;
		return (((unsigned char*)curAlloc)-size);
	}

	void clearPool()
	{
		//apago tudo menos um
		while(numPools>1)
		{
			numPools--;
			free(pools[numPools]);
		}

		//arranjo os ponteiros e alguns valores
		curPool = curAlloc = pools[0];
		curAllocEnd = ((unsigned char*)curPool) + growSize;
		numPools=1;
		numAllocs=0;
	}
};

}//namespace Memory
}//namespace HorseRadish

#endif