#include "MemManager.hpp"
#include "String.hpp"
#include "Machine.hpp"

#include <string.h>

#include <windows.h>

namespace HorseRadish
{

	namespace Memory
	{

#define SMEM_MANAGER_KEEP_FIRST_POOL			(1<<0)
#define SMEM_MANAGER_THREAD_UNPROTECTED			(1<<1)

#define SMEM_MANAGER_NUMBER_OF_POOLS			0x0ed1
#define SMEM_MANAGER_NUMBER_OF_POINTERS			0x0ed2
#define SMEM_MANAGER_NUMBER_OF_POINTERS_FREE	0x0ed3
#define SMEM_MANAGER_NUMBER_OF_POINTERS_IN_USE	0x0ed4
#define SMEM_MANAGER_MEMORY_POOL				0x0ed5
#define SMEM_MANAGER_MEMORY_POOL_ALLOCATED		0x0ed6
#define SMEM_MANAGER_MEMORY_POOL_FREE			0x0ed7
#define SMEM_MANAGER_MEMORY_POOL_PEAK			0x0ed8
#define SMEM_MANAGER_MEMORY_EXCESS				0x0ed9
#define SMEM_MANAGER_AVL_NUM_NODES				0x0eda
#define SMEM_MANAGER_AVL_NUM_LEVELS				0x0edb

#define ERRO_CHECKSUM			(1<<0)
#define ERRO_LAST_FREE			(1<<1)
#define ERRO_GROUP_FREE			(1<<2)
#define ERRO_PREV_SIZE			(1<<3)
#define ERRO_PAD_TAG			(1<<4)
#define ERRO_WRONG_DATA			(1<<5)
#define ERRO_WRONG_AVL_NODES	(1<<6)
#define ERRO_PATTERN_RELEASE	(1<<7)

		static const unsigned int patternPrefix = 0xbaadf00d; //fill pattern for bytes preceeding allocated blocks
		static const unsigned int patternPostfix = 0xdeadc0de; //fill pattern for bytes following allocated blocks
		static const unsigned int patternUnused = 0xfeedface; //blocos ainda não alocados
		static const unsigned int patternReleased = 0xdeadbeef; //blocos acabados de alocar

		void SConsolePLog(int sad, const char * const asdasd)
		{ }

		void SConsolePLogInfo(int sad, const char * const asdasd)
		{ }

		void SConsolePLogTab(int sad, const char * const asdasd, int sdfsdf)
		{ }

		void myLogInTAB(const int logID, const char *fmt, ...)
		{ }

		void myLogIn(const int logID, const char *fmt, ...)
		{ }

		static
			void copiaString(const char * __restrict from, char * __restrict to)
		{
			if (from == nullptr || to == nullptr)
				return;

			int i = 1;
			while ((*from) != '\0' && ((++i) < FILE_SIZE))
				*(to++) = *(from++);
			*to = '\0';
		}

		static
			int intMax(const int a, const int b)
		{
			int aux = a - b;
			return a - (aux & (aux >> 31));
		}

		static
			int intAbs(const int a)
		{
			return a - ((a + a) & (a >> 31));
		}

		static
			bool checkForPattern(void * const pointer, const unsigned int size, const unsigned int pattern)
		{
			unsigned int i, shiftCount, *iPtr, length;
			unsigned char *cPtr;

			iPtr = (unsigned int *)pointer;
			length = size;
			for (i = 0; i < (length >> 2); i++, iPtr++)
			{
				if (*iPtr != pattern)
					return false;
			}

			shiftCount = 0;
			cPtr = (unsigned char *)iPtr;
			for (i = 0; i < (length & 0x3); i++, cPtr++, shiftCount += 8)
			{
				if (*cPtr != ((unsigned char)(((pattern & (0xff << shiftCount)) >> shiftCount) & 0xff)))
					return false;
			}

			return true;
		}

		int MemoryManager::avlCalcPeso(AVL * const tree)
		{
			if (tree == nullptr)
				return -1;
			return (intMax(tree->left ? tree->left->height : -1, tree->right ? tree->right->height : -1) + 1);
		}

		bool MemoryManager::avlIsBalanced(AVL * const tree)
		{
			int l, r;

			l = r = -1;
			if (tree->left)		l = tree->left->height;
			if (tree->right)	r = tree->right->height;

			if (intAbs(l - r) >= 2)
				return false;
			return true;
		}

		void MemoryManager::avlRotateLL(AVL * const tree)
		{
			AVL *novoRoot;

			if (tree->pai)
			{
				if (tree->pai->left == tree)
					tree->pai->left = tree->left;
				else
					tree->pai->right = tree->left;
			}
			else
			{
				FreeNodes = tree->left;
			}

			tree->left->pai = tree->pai;

			novoRoot = tree->left;
			tree->left = novoRoot->right;
			if (novoRoot->right)
				novoRoot->right->pai = tree;

			novoRoot->right = tree;
			tree->pai = novoRoot;

			tree->height = avlCalcPeso(tree);
			novoRoot->height = avlCalcPeso(novoRoot);
		}

		void MemoryManager::avlRotateRR(AVL * const tree)
		{
			AVL *novoRoot;

			if (tree->pai)
			{
				if (tree->pai->left == tree)
					tree->pai->left = tree->right;
				else
					tree->pai->right = tree->right;
			}
			else
			{
				FreeNodes = tree->right;
			}

			tree->right->pai = tree->pai;

			novoRoot = tree->right;
			tree->right = novoRoot->left;
			if (novoRoot->left)
				novoRoot->left->pai = tree;

			novoRoot->left = tree;
			tree->pai = novoRoot;

			tree->height = avlCalcPeso(tree);
			novoRoot->height = avlCalcPeso(novoRoot);
		}

		void MemoryManager::avlRotateLR(AVL * const tree)
		{
			AVL *novoRoot, *lRoot, *rRoot;

			novoRoot = tree->left->right;
			novoRoot->pai = tree->pai;

			if (tree->pai)
			{
				if (tree->pai->left == tree)
					tree->pai->left = novoRoot;
				else
					tree->pai->right = novoRoot;
			}
			else
			{
				FreeNodes = novoRoot;
			}

			lRoot = novoRoot->left;
			rRoot = novoRoot->right;

			novoRoot->left = tree->left;
			novoRoot->right = tree;
			tree->pai = novoRoot;
			tree->left->pai = novoRoot;

			novoRoot->left->right = lRoot;
			novoRoot->right->left = rRoot;
			if (lRoot)	lRoot->pai = novoRoot->left;
			if (rRoot)	rRoot->pai = novoRoot->right;

			novoRoot->height = avlCalcPeso(novoRoot);
			novoRoot->left->height = avlCalcPeso(novoRoot->left);
			novoRoot->right->height = avlCalcPeso(novoRoot->right);
		}

		void MemoryManager::avlRotateRL(AVL * const tree)
		{
			AVL *novoRoot, *lRoot, *rRoot;

			novoRoot = tree->right->left;
			novoRoot->pai = tree->pai;

			if (tree->pai)
			{
				if (tree->pai->left == tree)
					tree->pai->left = novoRoot;
				else
					tree->pai->right = novoRoot;
			}
			else
			{
				FreeNodes = novoRoot;
			}

			lRoot = novoRoot->left;
			rRoot = novoRoot->right;

			novoRoot->left = tree;
			novoRoot->right = tree->right;
			tree->pai = novoRoot;
			tree->right->pai = novoRoot;

			novoRoot->left->right = lRoot;
			novoRoot->right->left = rRoot;
			if (lRoot)	lRoot->pai = novoRoot->left;
			if (rRoot)	rRoot->pai = novoRoot->right;

			novoRoot->height = avlCalcPeso(novoRoot);
			novoRoot->left->height = avlCalcPeso(novoRoot->left);
			novoRoot->right->height = avlCalcPeso(novoRoot->right);
		}

		MemoryManager::AVL* MemoryManager::avlFindBest(AVL * const tree, const int valor)
		{
			if (tree->ID == valor)
			{
				if (tree->dlList == nullptr)
					return nullptr;
				return tree;
			}

			if (valor < tree->ID)
			{
				if (tree->left == nullptr)
				{
					if (tree->dlList == nullptr || tree->ID < valor)
						return nullptr;
					return tree;
				}

				AVL *novo;
				if ((novo = avlFindBest(tree->left, valor)) != nullptr)
					return novo;

				if (tree->dlList == nullptr || tree->ID < valor)
					return nullptr;
				return tree;
			}

			if (tree->right == nullptr)
			{
				if (tree->dlList == nullptr || tree->ID < valor)
					return nullptr;
				return tree;
			}

			AVL *novo;
			if ((novo = avlFindBest(tree->right, valor)) != nullptr)
				return novo;

			if (tree->dlList == nullptr || tree->ID<valor)
				return nullptr;
			return tree;
		}

		bool MemoryManager::avlLeftHeavier(const AVL * const tree)
		{
			if (tree->right == nullptr)
				return true;
			if (tree->left == nullptr)
				return false;

			if (tree->left->height > tree->right->height)
				return true;
			return false;
		}

		MemoryManager::AVL* MemoryManager::avlInsert(AVL * const tree, const int valor, bool &nodeExisted)
		{
			if (valor == tree->ID)
			{
				nodeExisted = true;
				return tree;
			}

			AVL *novoNo;

			if (valor < tree->ID)
			{
				if (tree->left == nullptr)
				{
					nodeExisted = false;
					tree->left = novoNo = poolAVL.Alloc();
					if (novoNo != nullptr)
					{
						novoNo->height = 0;
						novoNo->left = novoNo->right = nullptr;
						novoNo->ID = valor;
						novoNo->pai = tree;
						novoNo->dlList = nullptr;
					}
				}
				else
				{
					novoNo = avlInsert(tree->left, valor, nodeExisted);

					if (nodeExisted)
						return novoNo;
				}
			}
			else
			{
				if (tree->right == nullptr)
				{
					nodeExisted = false;
					tree->right = novoNo = (AVL*)poolAVL.Alloc();
					if (novoNo != nullptr)
					{
						novoNo->height = 0;
						novoNo->left = novoNo->right = nullptr;
						novoNo->ID = valor;
						novoNo->pai = tree;
						novoNo->dlList = nullptr;
					}
				}
				else
				{
					novoNo = avlInsert(tree->right, valor, nodeExisted);

					if (nodeExisted)
						return novoNo;
				}
			}

			if (novoNo == nullptr)
				return nullptr;

			tree->height = avlCalcPeso(tree);
			if (avlIsBalanced(tree) == true)
				return novoNo;

			if (avlLeftHeavier(tree))
			{
				if (avlLeftHeavier(tree->left))
					avlRotateLL(tree);
				else
					avlRotateLR(tree);
			}
			else
			{
				if (avlLeftHeavier(tree->right))
					avlRotateRL(tree);
				else
					avlRotateRR(tree);
			}

			return novoNo;
		}

		bool MemoryManager::avlCheckBalance(AVL * const tree)
		{
			if (tree == nullptr)
				return false;

			if (tree->left)
			{
				if (avlCheckBalance(tree->left) == false)
					return false;
			}
			if (tree->right)
			{
				if (avlCheckBalance(tree->right) == false)
					return false;
			}
			return true;
		}

		void MemoryManager::avlRemoveDataFromNode(AVL * const avl, AVL_DATA *avlTrash)
		{
			if (avl->dlList == avlTrash)
			{
				avl->dlList = avlTrash->prox;
				if (avl->dlList)
					avl->dlList->prev = nullptr;
				free(avlTrash);
				return;
			}

			avlTrash->prev->prox = avlTrash->prox;
			if (avlTrash->prox)
				avlTrash->prox->prev = avlTrash->prev;
			free(avlTrash);
		}

		void MemoryManager::avlCreateForNode(NO * const node)
		{
			AVL_DATA *novoData;
			AVL *novoAVL;
			bool newCreated;

			newCreated = false;

			if (FreeNodes != nullptr)
			{
				novoAVL = avlInsert(FreeNodes, getUserDSize(node), newCreated);
			}
			else
			{
				FreeNodes = novoAVL = (AVL*)poolAVL.Alloc();
				if (FreeNodes == nullptr)
					return;
				FreeNodes->height = 0;
				FreeNodes->left = FreeNodes->right = nullptr;
				FreeNodes->ID = getUserDSize(node);
				FreeNodes->pai = nullptr;
				FreeNodes->dlList = nullptr;
			}

			if (novoAVL == nullptr)
				return;

			if (novoAVL->dlList == nullptr)
			{
				novoAVL->dlList = (AVL_DATA*)malloc(sizeof(AVL_DATA));
				if (novoAVL->dlList == nullptr)
					return;
				novoAVL->dlList->prox = novoAVL->dlList->prev = nullptr;
				novoAVL->dlList->no = node;

				node->avl = novoAVL;
				node->avlData = novoAVL->dlList;
				node->checkSum = calcCheckSum(node);
				return;
			}

			novoData = (AVL_DATA*)malloc(sizeof(AVL_DATA));
			if (novoData == nullptr)
				return;

			novoData->no = node;
			novoData->prox = novoAVL->dlList;
			novoData->prev = nullptr;
			novoAVL->dlList->prev = novoData;
			novoAVL->dlList = novoData;

			node->avl = novoAVL;
			node->avlData = novoAVL->dlList;
			node->checkSum = calcCheckSum(node);
		}

		void MemoryManager::avlDelete(AVL * const tree)
		{
			AVL_DATA *walk, *lixo;

			if (tree == nullptr)
				return;

			walk = tree->dlList;
			while (walk != nullptr)
			{
				lixo = walk;
				walk = walk->prox;
				free(lixo);
			}

			avlDelete(tree->right);
			avlDelete(tree->left);
		}

		void MemoryManager::avlGetInfo(const AVL * const tree, unsigned int &nNodes, unsigned int &nBPointers, unsigned int &nLevels, unsigned int &sumFBlocks, const int curLevel)
		{
			if (tree == nullptr)
				return;

			{
				for (const AVL_DATA * walk = tree->dlList; walk != nullptr; walk = walk->prox)
				{
					sumFBlocks += tree->ID;
					nBPointers++;
				}
			}

			if (tree->right)	avlGetInfo(tree->right, nNodes, nBPointers, nLevels, sumFBlocks, curLevel + 1);
			if (tree->left)		avlGetInfo(tree->left, nNodes, nBPointers, nLevels, sumFBlocks, curLevel + 1);

			if (nLevels < ((unsigned int)curLevel))
				nLevels = (unsigned int)curLevel;

			nNodes++;
		}

		int MemoryManager::writeLeaks()
		{
			NO *walk;
			int count;
			MEM_POOL *walkerM;
			HorseRadish::String strBytesSize;

			count = 0;
			for (walkerM = MemPools; walkerM != nullptr; walkerM = walkerM->prox)
			{
				for (walk = (NO*)walkerM->memPool; walk <= walkerM->memPoolEnd; walk = getNextNO(walk))
				{
					if (walk->avlData)
						continue;

					strBytesSize.SetMemory(getUserDSize(walk));
					myLogInTAB(logID, "[%s]\t- \"%s\" - %d", strBytesSize.GetData(), walk->infoSource, walk->infoLine);
					count++;
				}
			}

			return count;
		}

		MemoryManager::MEM_POOL* MemoryManager::procuraPool(const unsigned char* const pointer)
		{
			if (MemPools == nullptr)
				return nullptr;

			if (pointer >= MemPools->memPool && pointer < (((unsigned char*)MemPools->memPool) + MemPools->memTOTAL))
				return MemPools;

			for (MEM_POOL *walkerM = MemPools->prox; walkerM != nullptr; walkerM = walkerM->prox)
			{
				if (pointer >= walkerM->memPool && pointer < (((unsigned char*)walkerM->memPool) + walkerM->memTOTAL))
					return walkerM;
			}

			return nullptr;
		}

		void MemoryManager::verificaEstado(int * const erros, const unsigned int nNodesInAVL)
		{
			NO *walk, *next;
			MEM_POOL *walkerM;

			*erros = 0;

			for (walkerM = MemPools; walkerM != nullptr; walkerM = walkerM->prox)
			{
				if (((NO*)walkerM->memPoolEnd)->avlData != nullptr)
					(*erros) |= ERRO_LAST_FREE;

				for (walk = (NO*)walkerM->memPool; walk <= walkerM->memPoolEnd; walk = getNextNO(walk))
				{
					next = nullptr;
					if (walk != walkerM->memPoolEnd)
						next = getNextNO(walk);

					if ((walk->noSize <= (sizeof(NO) + walk->pad)) || (walk != walkerM->memPool && (walk->prevNoSize <= sizeof(NO))))
						(*erros) |= ERRO_WRONG_DATA;

					if (walk->checkSum != calcCheckSum(walk))
						(*erros) |= ERRO_CHECKSUM;

					if ((getUserPTR(walk))[-1] != walk->pad)
						(*erros) |= ERRO_PAD_TAG;

					if (next == nullptr)
						continue;

					if (next->prevNoSize != walk->noSize)
						(*erros) |= ERRO_PREV_SIZE;

					if (walk->avlData != nullptr && next->avlData != nullptr)
						(*erros) |= ERRO_GROUP_FREE;

					/*if (walk->avlData!=nullptr && checkForPattern(getUserPTR(walk),getUserDSize(walk),patternReleased)==false)
						(*erros)|=ERRO_PATTERN_RELEASE;*/
				}
			}

			if (poolAVL.GetNumAllocs() != nNodesInAVL)
				(*erros) |= ERRO_WRONG_AVL_NODES;
		}

		void MemoryManager::destroyPool(MEM_POOL * const lixo)
		{
			if (lixo == nullptr)
				return;

			if (lixo->memPool)
				VirtualFree((void *)lixo->memPool, 0, MEM_RELEASE);

			if (lixo == MemPools)
			{
				MemPools = MemPools->prox;
				if (MemPools)
					MemPools->prev = nullptr;
				free(lixo);
				return;
			}

			lixo->prev->prox = lixo->prox;
			if (lixo->prox)
				lixo->prox->prev = lixo->prev;
			free(lixo);
		}

		MemoryManager::MEM_POOL* MemoryManager::createPool(const int tamanho)
		{
			MEM_POOL *walkerM;

			if (MemPools == nullptr)
			{
				MemPools = (MEM_POOL*)malloc(sizeof(MEM_POOL));
				if (MemPools == nullptr)
					return nullptr;
				MemPools->prox = nullptr;
				MemPools->prev = nullptr;
				walkerM = MemPools;
			}
			else
			{
				for (walkerM = MemPools; walkerM->prox != nullptr; walkerM = walkerM->prox);
				walkerM->prox = (MEM_POOL*)malloc(sizeof(MEM_POOL));
				if (walkerM->prox == nullptr)
					return nullptr;
				walkerM->prox->prox = nullptr;
				walkerM->prox->prev = walkerM;
				walkerM = walkerM->prox;
			}

			walkerM->numNos = 0;
			walkerM->memPool = nullptr;
			walkerM->memPoolEnd = nullptr;
			walkerM->memTOTAL = 0;

			if (tamanho >= mngPoolSize)
			{
				walkerM->memTOTAL = tamanho;
				walkerM->memPool = (unsigned char *)VirtualAlloc(nullptr, walkerM->memTOTAL, MEM_COMMIT, PAGE_READWRITE);
				walkerM->memPoolEnd = walkerM->memPool;
				if (walkerM->memPool == nullptr)
				{
					destroyPool(walkerM);
					return nullptr;
				}
			}
			else
			{
				walkerM->memTOTAL = mngPoolSize;
				walkerM->memPool = (unsigned char *)VirtualAlloc(nullptr, walkerM->memTOTAL, MEM_COMMIT, PAGE_READWRITE);
				walkerM->memPoolEnd = walkerM->memPool;
				if (walkerM->memPool == nullptr)
				{
					walkerM->memTOTAL /= 2;
					if (walkerM->memTOTAL < tamanho)
						walkerM->memTOTAL = tamanho;
					walkerM->memPool = (unsigned char *)VirtualAlloc(nullptr, walkerM->memTOTAL, MEM_COMMIT, PAGE_READWRITE);
					walkerM->memPoolEnd = walkerM->memPool;
					if (walkerM->memPool == nullptr)
					{
						destroyPool(walkerM);
						return nullptr;
					}
				}
			}

			HorseRadish::Machine::AsmBufferSetUI32(walkerM->memPool, patternUnused, walkerM->memTOTAL);

			return walkerM;
		}

		int MemoryManager::getPoolFreeSpace(const MEM_POOL * const pool)
		{
			unsigned char *last;

			last = ((unsigned char*)pool->memPoolEnd);
			last += ((NO*)pool->memPoolEnd)->noSize;

			return (pool->memTOTAL - (last - ((unsigned char*)pool->memPool)));
		}

		int MemoryManager::procStat(const int flag)
		{
			if (flag <= 0)
				return -1;

			if (flag == SMEM_MANAGER_NUMBER_OF_POOLS)
			{
				int count = 0;
				for (const MEM_POOL *walkerM = MemPools; walkerM != nullptr; walkerM = walkerM->prox)
					count++;
				return count;
			}

			if (flag == SMEM_MANAGER_NUMBER_OF_POINTERS)
			{
				int count = 0;
				for (const MEM_POOL *walkerM = MemPools; walkerM != nullptr; walkerM = walkerM->prox)
				{
					for (NO *walk = (NO*)walkerM->memPool; walk <= walkerM->memPoolEnd; walk = getNextNO(walk))
						count++;
				}
				return count;
			}

			if (flag == SMEM_MANAGER_NUMBER_OF_POINTERS_FREE)
			{
				int count = 0;
				for (const MEM_POOL *walkerM = MemPools; walkerM != nullptr; walkerM = walkerM->prox)
				{
					for (NO *walk = (NO*)walkerM->memPool; walk <= walkerM->memPoolEnd; walk = getNextNO(walk))
					{
						if (walk->avlData)
							count++;
					}
				}
				return count;
			}

			if (flag == SMEM_MANAGER_NUMBER_OF_POINTERS_IN_USE)
			{
				int count = 0;
				for (const MEM_POOL *walkerM = MemPools; walkerM != nullptr; walkerM = walkerM->prox)
				{
					for (NO *walk = (NO*)walkerM->memPool; walk <= walkerM->memPoolEnd; walk = getNextNO(walk))
					{
						if (walk->avlData == nullptr)
							count++;
					}
				}
				return count;
			}

			if (flag == SMEM_MANAGER_MEMORY_POOL)
			{
				int count = 0;
				for (const MEM_POOL *walkerM = MemPools; walkerM != nullptr; walkerM = walkerM->prox)
					count += walkerM->memTOTAL;
				return count;
			}

			if (flag == SMEM_MANAGER_MEMORY_POOL_ALLOCATED)
			{
				int count = 0;
				for (const MEM_POOL *walkerM = MemPools; walkerM != nullptr; walkerM = walkerM->prox)
				{
					for (NO *walk = (NO*)walkerM->memPool; walk <= walkerM->memPoolEnd; walk = getNextNO(walk))
					{
						if (walk->avlData == nullptr)
							count += getUserDSize(walk);
					}
				}
				return count;
			}

			if (flag == SMEM_MANAGER_MEMORY_POOL_FREE)
			{
				int count = 0;
				for (const MEM_POOL *walkerM = MemPools; walkerM != nullptr; walkerM = walkerM->prox)
				{
					for (NO *walk = (NO*)walkerM->memPool; walk <= walkerM->memPoolEnd; walk = getNextNO(walk))
					{
						if (walk->avlData)
							count += getUserDSize(walk);
					}
				}
				return count;
			}

			if (flag == SMEM_MANAGER_MEMORY_POOL_PEAK)
			{
				return InfoData.memPeak;
			}

			if (flag == SMEM_MANAGER_MEMORY_EXCESS)
			{
				int count;
				unsigned int nNodes, nBPointers, nLevels, sumFBlocks;

				nNodes = nBPointers = nLevels = sumFBlocks = 0;
				avlGetInfo(FreeNodes, nNodes, nBPointers, nLevels, sumFBlocks, 1);

				count = nNodes*sizeof(AVL) + nBPointers*sizeof(AVL_DATA);
				count += this->Stat(SMEM_MANAGER_NUMBER_OF_POINTERS)*sizeof(NO);
				return count;
			}

			if (flag == SMEM_MANAGER_AVL_NUM_NODES)
			{
				unsigned int nNodes, nBPointers, nLevels, sumFBlocks;

				nNodes = nBPointers = nLevels = sumFBlocks = 0;
				avlGetInfo(FreeNodes, nNodes, nBPointers, nLevels, sumFBlocks, 1);
				return nNodes;
			}

			if (flag == SMEM_MANAGER_AVL_NUM_LEVELS)
			{
				unsigned int nNodes, nBPointers, nLevels, sumFBlocks;

				//inicio a zero, tiro os valores e devolvo
				nNodes = nBPointers = nLevels = sumFBlocks = 0;
				avlGetInfo(FreeNodes, nNodes, nBPointers, nLevels, sumFBlocks, 1);
				return nLevels;
			}

			return -1;
		}

		void MemoryManager::procSnapshot()
		{
			const MEM_POOL *walkerM;
			NO *walk;

			myLogIn(logID, " ");
			myLogIn(logID, ".:Listing of information regarding every allocation in the manager:.");
			myLogIn(logID, "   Addr       Size       Addr       Size     Flags");
			myLogIn(logID, " Reported   Reported    Actual     Actual    C F P  Allocated by");
			myLogIn(logID, "---------- ---------- ---------- ---------- ------- ---------------------------------------------------");

			for (walkerM = MemPools; walkerM != nullptr; walkerM = walkerM->prox)
			{
				for (walk = (NO*)walkerM->memPool; walk <= walkerM->memPoolEnd; walk = getNextNO(walk))
				{
					myLogIn(logID, "0x%08X 0x%08X 0x%08X 0x%08X  %c %c %c  %s (%d)",
						getUserPTR(walk),
						getUserDSize(walk),
						walk,
						walk->noSize,
						checkCheckSum(walk) ? ' ' : 'X',
						walk->avlData == nullptr ? ' ' : 'X',
						' ',//(walk->avlData==nullptr || checkForPattern(getUserPTR(walk),getUserDSize(walk),patternReleased)) ? ' ':'X',
						walk->infoSource,
						walk->infoLine);
				}
			}

			myLogIn(logID, ".:Listing information finished:.");
			myLogIn(logID, " ");
		}

		void* MemoryManager::procAlloc(const int bytes, const char * const file, const int line)
		{
			unsigned char *markPad;
			NO *no, *last, *newNo, *next;
			MEM_POOL *walkerM;
			AVL *bestFree;
			int oldNoSize;

			if (bytes <= 0)
			{
				myLogIn(logID, "Returning nullptr when asked for 0 bytes of memory!");
				myLogIn(logID, "Request made at \"%s\" in line %d.", file, line);
				return nullptr;
			}

			for (walkerM = MemPools; walkerM != nullptr; walkerM = walkerM->prox)
			{
				if (FreeNodes != nullptr && (bestFree = avlFindBest(FreeNodes, bytes)) != nullptr)
				{
					if (checkSplit(bestFree->dlList->no, bytes + BYTE_SLACK) == true)
					{
						no = bestFree->dlList->no;
						avlRemoveDataFromNode(bestFree, bestFree->dlList);

						next = getNextNO(no);

						oldNoSize = no->noSize;

						no->infoSource[0] = '\0';
						copiaString(file, no->infoSource);
						no->infoLine = line;
						no->avl = nullptr;
						no->avlData = nullptr;
						no->noSize = sizeof(NO) + bytes + no->pad;
						no->checkSum = calcCheckSum(no);

						InfoData.memTOTAL += bytes;
						InfoData.memPeak = intMax(InfoData.memTOTAL, InfoData.memPeak);

						newNo = (NO*)(((unsigned char*)no) + no->noSize);

						newNo->infoSource[0] = '\0';
						newNo->infoLine = -1;
						newNo->avl = nullptr;
						newNo->avlData = nullptr;
						newNo->pad = calcPad((unsigned char*)newNo + sizeof(NO));
						newNo->noSize = oldNoSize - no->noSize;
						newNo->prevNoSize = no->noSize;
						newNo->checkSum = calcCheckSum(newNo);

						next->prevNoSize = newNo->noSize;

						avlCreateForNode(newNo);

						markPad = getUserPTR(newNo);
						markPad[-1] = newNo->pad;

						return (void*)getUserPTR(no);
					}

					newNo = bestFree->dlList->no;
					avlRemoveDataFromNode(bestFree, bestFree->dlList);

					newNo->avlData = nullptr;
					newNo->avl = nullptr;

					InfoData.memTOTAL += bytes;
					InfoData.memPeak = intMax(InfoData.memTOTAL, InfoData.memPeak);

					newNo->checkSum = calcCheckSum(newNo);
					return (void*)getUserPTR(newNo);
				}

				last = (NO*)walkerM->memPoolEnd;
				if (((unsigned char*)last - (unsigned char*)walkerM->memPool + last->noSize + sizeof(NO) + bytes + 256) >= walkerM->memTOTAL)
					continue;

				newNo = getNextNO(last);

				newNo->infoSource[0] = '\0';
				copiaString(file, newNo->infoSource);
				newNo->infoLine = line;
				newNo->avl = nullptr;
				newNo->avlData = nullptr;
				newNo->prevNoSize = last->noSize;
				newNo->pad = calcPad((unsigned char*)newNo + sizeof(NO));
				newNo->noSize = sizeof(NO) + bytes + newNo->pad;
				newNo->checkSum = calcCheckSum(newNo);

				walkerM->numNos++;
				walkerM->memPoolEnd = newNo;

				InfoData.memTOTAL += bytes;
				InfoData.memPeak = intMax(InfoData.memTOTAL, InfoData.memPeak);

				markPad = getUserPTR(newNo);
				markPad[-1] = newNo->pad;

				return (void*)markPad;
			}

			walkerM = createPool(bytes + sizeof(NO) + 256);
			if (walkerM == nullptr)
			{
				myLogIn(logID, "Returning nullptr when asked for %d bytes of memory!", bytes);
				myLogIn(logID, "Request made at \"%s\" in line %d.", file, line);
				return nullptr;
			}

			newNo = (NO*)walkerM->memPool;

			newNo->infoSource[0] = '\0';
			copiaString(file, newNo->infoSource);
			newNo->infoLine = line;
			newNo->avl = nullptr;
			newNo->avlData = nullptr;
			newNo->prevNoSize = 0;
			newNo->pad = calcPad((unsigned char*)newNo + sizeof(NO));
			newNo->noSize = sizeof(NO) + bytes + newNo->pad;
			newNo->checkSum = calcCheckSum(newNo);

			walkerM->numNos++;
			walkerM->memPoolEnd = newNo;

			InfoData.memTOTAL += bytes;
			InfoData.memPeak = intMax(InfoData.memTOTAL, InfoData.memPeak);

			markPad = getUserPTR(newNo);
			markPad[-1] = newNo->pad;

			return (void*)markPad;
		}

		void MemoryManager::procFree(void * const pointer)
		{
			NO *no, *behind, *front;
			MEM_POOL *walkerM;

			walkerM = procuraPool((unsigned char*)pointer);
			if (walkerM == nullptr)
			{
				InfoData.freeError++;
				return;
			}

			no = (NO*)(((unsigned char*)pointer) - sizeof(NO) - ((unsigned char*)pointer)[-1]);
			if (checkCheckSum(no) == false)
			{
				myLogIn(logID, "Checksum of pointer is incorrect!");
				myLogInTAB(logID, "pointer: %p", pointer);
				return;
			}

			walkerM->numNos--;
			InfoData.memTOTAL -= getUserDSize(no);

			//SAsmBufferClear(pointer,getUserDSize(no));

			//<<<<<<<<<<<<
			//OPTIMIZAÇÕES
			behind = front = nullptr;
			if (no != walkerM->memPool)		behind = getPrevNO(no);
			if (no != walkerM->memPoolEnd)	front = getNextNO(no);

			if (walkerM->memPool == no && walkerM->memPoolEnd == no)
			{
				destroyPool(walkerM);
				return;
			}

			if (front == nullptr)
			{
				if (behind->avlData)
				{
					if (behind == walkerM->memPool)
					{
						avlRemoveDataFromNode(behind->avl, behind->avlData);
						destroyPool(walkerM);
						return;
					}

					avlRemoveDataFromNode(behind->avl, behind->avlData);
					behind->avlData = nullptr;
					behind->avl = nullptr;
					behind->checkSum = calcCheckSum(behind);

					walkerM->numNos--;
					walkerM->memPoolEnd = getPrevNO(behind);
					return;
				}

				walkerM->memPoolEnd = behind;
				return;
			}

			if (behind == nullptr)
			{
				if (front->avlData)
				{
					if (front == walkerM->memPoolEnd)
					{
						avlRemoveDataFromNode(front->avl, front->avlData);
						destroyPool(walkerM);
						return;
					}

					avlRemoveDataFromNode(front->avl, front->avlData);
					front->avlData = nullptr;
					front->checkSum = calcCheckSum(front);

					no->noSize += front->noSize;
					(getNextNO(no))->prevNoSize = no->noSize;

					//SAsmBufferSetUI32(getUserPTR(no),patternReleased,getUserDSize(no));

					avlCreateForNode(no);
					walkerM->numNos--;
					return;
				}

				avlCreateForNode(no);
				//SAsmBufferSetUI32(getUserPTR(no),patternReleased,getUserDSize(no));
				return;
			}

			if ((behind->avlData) && (front->avlData))
			{
				avlRemoveDataFromNode(behind->avl, behind->avlData);
				avlRemoveDataFromNode(front->avl, front->avlData);
				behind->avlData = front->avlData = nullptr;
				behind->checkSum = calcCheckSum(behind);
				front->checkSum = calcCheckSum(front);

				behind->noSize += no->noSize + front->noSize;
				(getNextNO(behind))->prevNoSize = behind->noSize;

				//SAsmBufferSetUI32(getUserPTR(behind),patternReleased,getUserDSize(behind));

				avlCreateForNode(behind);
				walkerM->numNos -= 2;
				return;
			}

			if (front->avlData)
			{
				avlRemoveDataFromNode(front->avl, front->avlData);
				front->avlData = nullptr;
				front->checkSum = calcCheckSum(front);

				no->noSize += front->noSize;
				(getNextNO(no))->prevNoSize = no->noSize;

				//SAsmBufferSetUI32(getUserPTR(no),patternReleased,getUserDSize(no));

				avlCreateForNode(no);
				walkerM->numNos--;
				return;
			}

			if (behind->avlData)
			{
				avlRemoveDataFromNode(behind->avl, behind->avlData);
				behind->avlData = nullptr;

				behind->noSize += no->noSize;
				(getNextNO(behind))->prevNoSize = behind->noSize;

				//SAsmBufferSetUI32(getUserPTR(behind),patternReleased,getUserDSize(behind));

				avlCreateForNode(behind);
				walkerM->numNos--;
				return;
			}

			avlCreateForNode(no);
			//SAsmBufferSetUI32(getUserPTR(no),patternReleased,getUserDSize(no));
		}

		void* MemoryManager::procRealloc(const void * const pointer, const int bytes, const char * const file, const int line)
		{
			void *novo;
			NO *no, *front;
			MEM_POOL *walkerM;

			if (MemPools == nullptr || bytes <= 0)
			{
				myLogInTAB(logID, "Returning nullptr when asked to reallocate %d bytes of memory!", bytes);
				return nullptr;
			}

			if (pointer == nullptr)
				return procAlloc(bytes, file, line);

			walkerM = procuraPool((unsigned char*)pointer);
			if (walkerM == nullptr)
			{
				InfoData.reallocError++;
				myLogIn(logID, "Returning nullptr when asked to reallocate %d bytes of memory!", bytes);
				myLogInTAB(logID, "could not find pointer: %p", pointer);
				return nullptr;
			}

			no = (NO*)(((unsigned char*)pointer) - sizeof(NO) - ((unsigned char*)pointer)[-1]);
			if (checkCheckSum(no) == false)
			{
				myLogIn(logID, "Checksum of pointer is incorrect!");
				myLogInTAB(logID, "pointer: %p", pointer);
				return nullptr;
			}

			if (no->avlData)
			{
				InfoData.reallocError++;
				myLogIn(logID, "Returning nullptr when asked to reallocate %d bytes of memory!", bytes);
				myLogInTAB(logID, "pointer to reallocate is not in use: %p", pointer);
				return nullptr;
			}

			if (getUserDSize(no) >= bytes)
				return (void*)getUserPTR(no);

			if ((no == walkerM->memPoolEnd) && ((getPosNO(walkerM, no) + sizeof(NO) + bytes + no->pad) < walkerM->memTOTAL))
			{
				InfoData.memTOTAL += bytes - getUserDSize(no);
				InfoData.memPeak = intMax(InfoData.memTOTAL, InfoData.memPeak);

				no->noSize = sizeof(NO) + no->pad + bytes;
				no->checkSum = calcCheckSum(no);
				return (void*)getUserPTR(no);
			}

			if ((no != walkerM->memPoolEnd))
			{
				front = getNextNO(no);

				if ((front->avlData) && (getUserDSize(no) + getUserDSize(front) >= bytes))
				{
					InfoData.memTOTAL += getUserDSize(front);
					InfoData.memPeak = intMax(InfoData.memTOTAL, InfoData.memPeak);

					avlRemoveDataFromNode(front->avl, front->avlData);
					front->avl = nullptr;
					front->avlData = nullptr;

					no->noSize += front->noSize;
					(getNextNO(no))->prevNoSize = no->noSize;

					no->checkSum = calcCheckSum(no);
					return (void*)getUserPTR(no);
				}
			}

			novo = procAlloc(bytes, file, line);
			if (novo == nullptr)
			{
				myLogInTAB(logID, "Returning nullptr when asked to reallocate %d bytes of memory!", bytes);
				return nullptr;
			}

			HorseRadish::Machine::AsmBufferCopy(novo, getUserPTR(no), getUserDSize(no));
			procFree(getUserPTR(no));
			return novo;
		}

		void MemoryManager::procCleanEverything(const bool writeStatus)
		{
			if (writeStatus)
			{
				unsigned int leaks, nNodes, nBPointers, nLevels, sumFBlocks;
				HorseRadish::String strFormatted;
				int erros;

				SConsolePLog(logID, " ");
				strFormatted.SetCurrentTime();
				myLogIn(logID, ".:Dumping information started (%s):.", strFormatted.GetData());

				SConsolePLog(logID, "Current state:");
				strFormatted.SetMemory(this->Stat(SMEM_MANAGER_MEMORY_POOL));
				myLogInTAB(logID, "memory pool total: %s", strFormatted.GetData());
				strFormatted.SetMemory(this->Stat(SMEM_MANAGER_MEMORY_POOL_FREE));
				myLogInTAB(logID, "memory pool free: %s", strFormatted.GetData());
				myLogInTAB(logID, "number of pools: %d", this->Stat(SMEM_MANAGER_NUMBER_OF_POOLS));
				myLogInTAB(logID, "number of blocks (total): %d", this->Stat(SMEM_MANAGER_NUMBER_OF_POINTERS));
				myLogInTAB(logID, "number of blocks (free): %d", this->Stat(SMEM_MANAGER_NUMBER_OF_POINTERS_FREE));
				myLogInTAB(logID, "number of blocks (in use): %d", this->Stat(SMEM_MANAGER_NUMBER_OF_POINTERS_IN_USE));
				strFormatted.SetMemory(this->Stat(SMEM_MANAGER_NUMBER_OF_POINTERS)*sizeof(NO));
				myLogInTAB(logID, "memory excess: %s", strFormatted.GetData());

				SConsolePLog(logID, "Tree of free nodes:");
				nNodes = nBPointers = nLevels = sumFBlocks = 0;
				avlGetInfo(FreeNodes, nNodes, nBPointers, nLevels, sumFBlocks, 1);
				myLogInTAB(logID, "number of nodes: %d", nNodes);
				myLogInTAB(logID, "number of block pointers: %d", nBPointers);
				myLogInTAB(logID, "number of levels: %d", nLevels);
				strFormatted.SetMemory(sumFBlocks);
				myLogInTAB(logID, "memory reserved in tree: %s", strFormatted.GetData());
				strFormatted.SetMemory(nNodes*sizeof(AVL) + nBPointers*sizeof(AVL_DATA));
				myLogInTAB(logID, "memory excess: %s", strFormatted.GetData());

				SConsolePLog(logID, "Overall statistics:");
				strFormatted.SetMemory(InfoData.memPeak);
				myLogInTAB(logID, "memory peak: %s", strFormatted.GetData());
				myLogInTAB(logID, "number of Alloc calls: %d", InfoData.numAllocCalls);
				myLogInTAB(logID, "number of AllocDebug calls: %d", InfoData.numAllocDebugCalls);
				myLogInTAB(logID, "number of Free calls: %d", InfoData.numFreeCalls);
				myLogInTAB(logID, "number of Free calls with wrong pointers: %d", InfoData.freeError);
				myLogInTAB(logID, "number of Realloc calls: %d", InfoData.numReallocCalls);
				myLogInTAB(logID, "number of ReallocDebug calls: %d", InfoData.numReallocDebugCalls);
				myLogInTAB(logID, "number of Realloc calls with wrong pointers: %d", InfoData.reallocError);
				myLogInTAB(logID, "number of BlockMelt calls: %d", InfoData.numBlockMeltCalls);
				myLogInTAB(logID, "number of BlockAlloc calls: %d", InfoData.numBlockAllocCalls);
				myLogInTAB(logID, "number of BlockGrow calls: %d", InfoData.numBlockGrowCalls);
				myLogInTAB(logID, "number of BlockGrow calls with wrong pointers: %d", InfoData.blockGrowError);

				SConsolePLog(logID, "Internal error checks:");
				verificaEstado(&erros, nNodes);
				if (erros & ERRO_CHECKSUM)			SConsolePLogTab(logID, "a node checksum was incorrect!", 2);
				if (erros & ERRO_LAST_FREE)			SConsolePLogTab(logID, "the last node of a pool was free!", 2);
				if (erros & ERRO_GROUP_FREE)		SConsolePLogTab(logID, "there is at least a pair of free nodes!", 2);
				if (erros & ERRO_PREV_SIZE)			SConsolePLogTab(logID, "the previous size of a node was incorrect!", 2);
				if (erros & ERRO_PAD_TAG)			SConsolePLogTab(logID, "the pad field had incorrect values!", 2);
				if (erros & ERRO_WRONG_DATA)		SConsolePLogTab(logID, "a node had impossible values!", 2);
				if (erros & ERRO_WRONG_AVL_NODES)	SConsolePLogTab(logID, "wrong number of nodes between AVL tree and fixed pool!", 2);
				if (erros & ERRO_PATTERN_RELEASE)	SConsolePLogTab(logID, "a free node has corrupted release pattern!", 2);
				if (erros == 0)
					SConsolePLogTab(logID, "manager passed all internal error checks :)", 2);

				SConsolePLog(logID, "Current leaks:");
				leaks = writeLeaks();
				if (leaks == 0)
					SConsolePLogTab(logID, "no leaks", 2);
				else
					myLogInTAB(logID, "number of leaks: %d", leaks);

				SConsolePLogInfo(logID, ".:Dumping information finished:.");
				SConsolePLog(logID, " ");
			}

			while (MemPools != nullptr)
				destroyPool(MemPools);

			avlDelete(FreeNodes);
			FreeNodes = nullptr;

			poolAVL.Clear();

			memset(&InfoData, 0, sizeof(STATS));
			MemPools = nullptr;
		}

		MemoryManager::MemoryManager(const int MBytes, const int alignBytes, const int flags) : poolAVL(50)
		{
			HorseRadish::String strBytesSize;

			if (alignBytes == 2 || alignBytes == 4 || alignBytes == 8 || alignBytes == 16 || alignBytes == 32 || alignBytes == 64 || alignBytes == 128)
				mngAlign = alignBytes;
			if (MBytes > 0)
				mngPoolSize = MBytes * 1024 * 1024;

			memset(&InfoData, 0, sizeof(STATS));
			MemPools = nullptr;
			workFlags = flags;

			strBytesSize.SetMemory(mngPoolSize);

			myLogIn(logID, "Memory manager inicialized:");
			myLogInTAB(logID, "statictics cleaned");
			myLogInTAB(logID, "memory pool size: %s", strBytesSize.GetData());
			myLogInTAB(logID, "align bytes: %d", mngAlign);
			if (workFlags & SMEM_MANAGER_KEEP_FIRST_POOL)		myLogInTAB(logID, "keeping first pool");
			else	myLogInTAB(logID, "destroy first pool");
			if (workFlags & SMEM_MANAGER_THREAD_UNPROTECTED)	myLogInTAB(logID, "thread unprotected");
			else	myLogInTAB(logID, "thread protected");
			myLogIn(logID, " ");

			if (workFlags & SMEM_MANAGER_KEEP_FIRST_POOL)
				this->AllocDEBUG(1, "!block for preserving first pool!", 0);
		}

		MemoryManager::~MemoryManager()
		{
			SConsolePLog(logID, "#250,255,102. !! - !!");
			SConsolePLog(logID, "About to reset the memory manager!");

			while (MemPools != nullptr)
				destroyPool(MemPools);
			MemPools = nullptr;

			avlDelete(FreeNodes);
			FreeNodes = nullptr;

			poolAVL.Clear();

			SConsolePLog(logID, "    done.");
			SConsolePLog(logID, "#250,255,102. !! - !!");
		}

		int MemoryManager::Stat(const int flag)
		{
			if (workFlags & SMEM_MANAGER_THREAD_UNPROTECTED)
				return procStat(flag);

			std::lock_guard<std::mutex> lock(mutex);

			return procStat(flag);
		}

		void MemoryManager::Snapshot()
		{
			if (workFlags & SMEM_MANAGER_THREAD_UNPROTECTED)
			{
				procSnapshot();
				return;
			}

			std::lock_guard<std::mutex> lock(mutex);

			procSnapshot();
		}

		void* MemoryManager::Alloc(const int bytes)
		{
			InfoData.numAllocCalls++;

			if (workFlags & SMEM_MANAGER_THREAD_UNPROTECTED)
				return procAlloc(bytes, "<sem informação de debug>", 0);

			std::lock_guard<std::mutex> lock(mutex);

			return procAlloc(bytes, "<sem informação de debug>", 0);
		}

		void* MemoryManager::AllocDEBUG(const int bytes, const char * const file, const int line)
		{
			InfoData.numAllocDebugCalls++;

			if (workFlags & SMEM_MANAGER_THREAD_UNPROTECTED)
				return procAlloc(bytes, file, line);

			std::lock_guard<std::mutex> lock(mutex);

			return procAlloc(bytes, file, line);
		}

		void* MemoryManager::AllocUnmanaged(const int bytes)
		{
			return malloc(bytes);
		}

		void MemoryManager::Free(void * const pointer)
		{
			InfoData.numFreeCalls++;

			if (workFlags & SMEM_MANAGER_THREAD_UNPROTECTED)
			{
				procFree(pointer);
				return;
			}

			std::lock_guard<std::mutex> lock(mutex);

			procFree(pointer);
		}

		void* MemoryManager::Realloc(const void * const pointer, const int bytes)
		{
			InfoData.numReallocCalls++;

			if (workFlags & SMEM_MANAGER_THREAD_UNPROTECTED)
				return procRealloc(pointer, bytes, "<sem informação de debug>", 0);

			std::lock_guard<std::mutex> lock(mutex);

			return procRealloc(pointer, bytes, "<sem informação de debug>", 0);
		}

		void* MemoryManager::ReallocDEBUG(const void * const pointer, const int bytes, const char * const file, const int line)
		{
			InfoData.numReallocDebugCalls++;

			if (workFlags & SMEM_MANAGER_THREAD_UNPROTECTED)
				return procRealloc(pointer, bytes, file, line);

			std::lock_guard<std::mutex> lock(mutex);

			return procRealloc(pointer, bytes, file, line);
		}

	} //namespace Memory

} //namespace HorseRadish