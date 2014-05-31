#pragma once
#ifndef __HMEM_MANAGER__
#define __HMEM_MANAGER__

#include "Types.hpp"
#include "Containers.hpp"
#include "Hashing.hpp"

#include <mutex>
#include <stdlib.h>

namespace HorseRadish
{

namespace Memory
{

#define BYTE_SLACK	128
#define FILE_SIZE	64

class MemoryManager
{
	struct AVL_DATA;
	struct AVL;

	struct NO{
		AVL_DATA *avlData;
		AVL *avl;
		unsigned __int32 prevNoSize;
		unsigned __int32 noSize;
		__int32 infoLine;
		__int8 infoSource[FILE_SIZE];
		unsigned char checkSum;
		unsigned char pad;
	};

	struct MEM_POOL{
		void *memPool,*memPoolEnd;
		int memTOTAL,numNos;

		MEM_POOL *prox,*prev;
	};

	struct STATS{
		int freeError,reallocError,blockGrowError,memPeak,memTOTAL;
		int numAllocCalls,numAllocDebugCalls,numFreeCalls,numReallocCalls,numReallocDebugCalls;
		int numBlockMeltCalls,numBlockAllocCalls,numBlockGrowCalls;
	};

	struct AVL_DATA{
		NO *no;
		AVL_DATA *prox,*prev;
	};

	struct AVL{
		int ID,height;
		AVL *left,*right,*pai;

		AVL_DATA *dlList;
	};

	MEM_POOL *MemPools;
	AVL *FreeNodes;
	std::mutex mutex;
	int mngPoolSize,mngAlign,workFlags;
	int logID;
	HorseRadish::Containers::Pool<AVL> poolAVL;
	STATS InfoData;

	static
	unsigned char calcCheckSum(const NO * const no)
	{
		return HorseRadish::Hashing::CalculateCRC8(no, 16);
		/*unsigned char resultado;

		const unsigned char *walk;

		//inicio
		resultado = 0x0000;

		//agora faço isto só estas vezes que é para incluir os campos: avlData,avl,noSize e pad
		walk=(const unsigned char*)(&no->avl);
		resultado = crcTable[resultado ^ *walk++];
		resultado = crcTable[resultado ^ *walk++];
		resultado = crcTable[resultado ^ *walk++];
		resultado = crcTable[resultado ^ *walk];

		walk=(const unsigned char*)(&no->avlData);
		resultado = crcTable[resultado ^ *walk++];
		resultado = crcTable[resultado ^ *walk++];
		resultado = crcTable[resultado ^ *walk++];
		resultado = crcTable[resultado ^ *walk];

		walk=(const unsigned char*)(&no->noSize);
		resultado = crcTable[resultado ^ *walk++];
		resultado = crcTable[resultado ^ *walk++];
		resultado = crcTable[resultado ^ *walk++];
		resultado = crcTable[resultado ^ *walk];

		walk=(const unsigned char*)(&no->pad);
		resultado = crcTable[resultado ^ *walk];

		//faço o xor final e depois devolvo
		resultado^= 0x0000;
		return resultado;*/
	}

	static
	bool __fastcall checkCheckSum(const NO * const no)
	{
		if (calcCheckSum(no)!=no->checkSum)
			return false;
		return true;
	}

	static
	unsigned char* __fastcall getUserPTR(NO * const no)
	{	return ((unsigned char*)no)+sizeof(NO)+no->pad;}

	static
	NO* __fastcall getNextNO(NO * const no)
	{	return (NO*)((unsigned char*)no+no->noSize);}

	static
	NO* __fastcall getPrevNO(NO * const no)
	{	return (NO*)((unsigned char*)no-no->prevNoSize);}

	static
	int __fastcall getPosNO(const MEM_POOL * const pool, const NO * const no)
	{	return (((unsigned char*)no) - ((unsigned char*)pool->memPool));}

	static
	int __fastcall getUserDSize(const NO * const no)
	{	return (no->noSize-no->pad-sizeof(NO));}

	static
	bool __fastcall checkSplit(const NO * const no, const int bytesMinimum)
	{
		int dataSize;
		
		//o tamanho que tenho e tiro o tamanho do NO que tenho de criar e o maximo de pad possivel
		dataSize=getUserDSize(no)-sizeof(NO)-256;
		
		//se for maior, cabe cá dentro
		if (dataSize > bytesMinimum)
			return true;
		return false;
	}

	unsigned char __fastcall calcPad(const unsigned char * const data)
	{	return (mngAlign - ((int)data % mngAlign) );}

	int avlCalcPeso(AVL * const tree);
	bool avlIsBalanced(AVL * const tree);
	void avlRotateLL(AVL * const tree);
	void avlRotateRR(AVL * const tree);
	void avlRotateLR(AVL * const tree);
	void avlRotateRL(AVL * const tree);
	AVL* avlFindBest(AVL * const tree, const int valor);
	bool avlLeftHeavier(const AVL * const tree);
	AVL* avlInsert(AVL * const tree, const int valor, bool &nodeExisted);
	bool avlCheckBalance(AVL * const tree);
	void avlRemoveDataFromNode(AVL * const avl, AVL_DATA *avlTrash);
	void avlCreateForNode(NO * const node);
	void avlDelete(AVL * const tree);
	void avlGetInfo(const AVL * const tree, unsigned int &nNodes, unsigned int &nBPointers, unsigned int &nLevels, unsigned int &sumFBlocks, const int curLevel);

	int writeLeaks();
	MEM_POOL* procuraPool(const unsigned char* const pointer);
	void verificaEstado(int * const erros, const unsigned int nNodesInAVL);
	void destroyPool(MEM_POOL * const lixo);
	MEM_POOL* createPool(const int tamanho);
	int getPoolFreeSpace(const MEM_POOL * const pool);

	int procStat(const int flag);
	void procSnapshot();
	void* procAlloc(const int bytes, const char * const file, const int line);
	void procFree(void * const pointer);
	void* procRealloc(const void * const pointer, const int bytes, const char * const file, const int line);
	void procCleanEverything(const bool writeStatus);

public:
	MemoryManager(const int MBytes, const int alignBytes, const int flags);
	~MemoryManager();

	int Stat(const int flag);
	void Snapshot();
	void* Alloc(const int bytes);
	void* AllocDEBUG(const int bytes, const char * const file, const int line);
	void* AllocUnmanaged(const int bytes);
	void Free(void * const pointer);
	void* Realloc(const void * const pointer, const int bytes);
	void* ReallocDEBUG(const void * const pointer, const int bytes, const char * const file, const int line);
	
};

}//namespace Memory
}//namespace HorseRadish

#endif