#include "MemManager.hpp"
#include "String.hpp"
#include "Machine.hpp"

#include <string.h>

namespace HorseRadish
{

namespace Memory
{

/*§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§
§§§§§§   -= Variáveis locais =-   §§§§§
§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§*/
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

/*§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§
§§§§§§   -= Coisas para toda a gente =-   §§§§§
§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§*/
static
void copiaString(const char * __restrict from, char * __restrict to)
{
	if (from==nullptr || to==nullptr)
		return;

	int i=1;
	while((*from)!='\0' && ((++i)<FILE_SIZE))
		*(to++)=*(from++);
	*to='\0';
}

static
int intMax(const int a, const int b)
{
	int aux=a-b;
    return a-(aux & (aux>>31));
}

static
int intAbs(const int a)
{    return a - ((a+a) & (a>>31));}

static
bool checkForPattern(void * const pointer, const unsigned int size, const unsigned int pattern)
{
	unsigned int i,shiftCount,*iPtr,length;
	unsigned char *cPtr;

	//verificar o maior
	iPtr = (unsigned int *) pointer;
	length = size;
	for(i=0; i<(length >> 2); i++,iPtr++)
		{
		if (*iPtr != pattern)
			return false;
		}

	//verificar o que resta
	shiftCount = 0;
	cPtr = (unsigned char *) iPtr;
	for(i=0; i<(length & 0x3); i++,cPtr++,shiftCount+=8)
		{
		if (*cPtr != ((unsigned char)((( pattern & ( 0xff << shiftCount ) ) >> shiftCount) & 0xff)) )
			return false;
		}

	//está tudo correcto
	return true;
}

/*§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§
§§§§§§   -= Funcoes para a arvore AVL =-   §§§§§
§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§*/
int MemoryManager::avlCalcPeso(AVL * const tree)
{
	if (tree==nullptr)
		return -1;
	return (intMax(tree->left?tree->left->height:-1, tree->right?tree->right->height:-1) + 1);
}

bool MemoryManager::avlIsBalanced(AVL * const tree)
{
	int l,r;

	l=r=-1;
	if (tree->left)		l=tree->left->height;
	if (tree->right)	r=tree->right->height;

	if (intAbs(l-r)>=2)
		return false;
	return true;
}

void MemoryManager::avlRotateLL(AVL * const tree)
{
	AVL *novoRoot;

	//o meu pai passa a ter o meu filho (tenho de ver onde estou no meu pai)
	if (tree->pai)
		{
		if (tree->pai->left==tree)
			tree->pai->left=tree->left;
		else
			tree->pai->right=tree->left;
		}
	else
		{
		FreeNodes=tree->left;
		}

	//não esquecer que o filho tem um ponteiro para o pai, logo, tem de actualizar isso tambem
	tree->left->pai=tree->pai;

	//mas eu deixo de ter um filho à esquerda (que é o meu filho que agora perdi), mas ganho os deles
	novoRoot=tree->left;
	tree->left=novoRoot->right;
	if (novoRoot->right)
		novoRoot->right->pai=tree;

	//como eu agora vou passar a ser filho do meu filho sei que tenho um valor maior que o dele, logo
	//tenho de ser o seu filho da direita, logo o meu pai também passa a ser ele
	novoRoot->right=tree;
	tree->pai=novoRoot;

	//recalculo o meu peso e o mesmo para o meu antigo filho
	tree->height=avlCalcPeso(tree);
	novoRoot->height=avlCalcPeso(novoRoot);
}

void MemoryManager::avlRotateRR(AVL * const tree)
{
	AVL *novoRoot;

	//o meu pai passa a ter o meu filho (tenho de ver onde estou no meu pai)
	if (tree->pai)
		{
		if (tree->pai->left==tree)
			tree->pai->left=tree->right;
		else
			tree->pai->right=tree->right;
		}
	else
		{
		FreeNodes=tree->right;
		}

	//não esquecer que o filho tem um ponteiro para o pai, logo, tem de actualizar isso tambem
	tree->right->pai=tree->pai;

	//mas eu deixo de ter um filho à esquerda (que é o meu filho que agora perdi), mas ganho os deles
	novoRoot=tree->right;
	tree->right=novoRoot->left;
	if (novoRoot->left)
		novoRoot->left->pai=tree;

	//como eu agora vou passar a ser filho do meu filho sei que tenho um valor maior que o dele, logo
	//tenho de ser o seu filho da direita, logo o meu pai também passa a ser ele
	novoRoot->left=tree;
	tree->pai=novoRoot;

	//recalculo o meu peso e o mesmo para o meu antigo filho
	tree->height=avlCalcPeso(tree);
	novoRoot->height=avlCalcPeso(novoRoot);
}

void MemoryManager::avlRotateLR(AVL * const tree)
{
	AVL *novoRoot,*lRoot,*rRoot;

	//qual o novo root
	novoRoot=tree->left->right;
	novoRoot->pai=tree->pai;

	//o meu pai passa a ter o meu neto (tenho de ver onde estou no meu pai)
	if (tree->pai)
		{
		if (tree->pai->left==tree)
			tree->pai->left=novoRoot;
		else
			tree->pai->right=novoRoot;
		}
	else
		{
		FreeNodes=novoRoot;
		}

	//guardo os lados do novo root
	lRoot=novoRoot->left;
	rRoot=novoRoot->right;

	//os novos filhos do root
	novoRoot->left=tree->left;
	novoRoot->right=tree;
	tree->pai=novoRoot;
	tree->left->pai=novoRoot;

	//arranjos os outros dois
	novoRoot->left->right=lRoot;
	novoRoot->right->left=rRoot;
	if (lRoot)	lRoot->pai=novoRoot->left;
	if (rRoot)	rRoot->pai=novoRoot->right;
	
	//recalculo o meu peso e o mesmo para o meu antigo filho
	novoRoot->height=avlCalcPeso(novoRoot);
	novoRoot->left->height=avlCalcPeso(novoRoot->left);
	novoRoot->right->height=avlCalcPeso(novoRoot->right);
}

void MemoryManager::avlRotateRL(AVL * const tree)
{
	AVL *novoRoot,*lRoot,*rRoot;

	//qual o novo root
	novoRoot=tree->right->left;
	novoRoot->pai=tree->pai;

	//o meu pai passa a ter o meu filho (tenho de ver onde estou no meu pai)
	if (tree->pai)
		{
		if (tree->pai->left==tree)
			tree->pai->left=novoRoot;
		else
			tree->pai->right=novoRoot;
		}
	else
		{
		FreeNodes=novoRoot;
		}

	//guardo os lados do novo root
	lRoot=novoRoot->left;
	rRoot=novoRoot->right;

	//os novos filhos do root
	novoRoot->left=tree;
	novoRoot->right=tree->right;
	tree->pai=novoRoot;
	tree->right->pai=novoRoot;

	//arranjos os outros dois
	novoRoot->left->right=lRoot;
	novoRoot->right->left=rRoot;
	if (lRoot)	lRoot->pai=novoRoot->left;
	if (rRoot)	rRoot->pai=novoRoot->right;
	
	//recalculo o meu peso e o mesmo para o meu antigo filho
	novoRoot->height=avlCalcPeso(novoRoot);
	novoRoot->left->height=avlCalcPeso(novoRoot->left);
	novoRoot->right->height=avlCalcPeso(novoRoot->right);
}

MemoryManager::AVL* MemoryManager::avlFindBest(AVL * const tree, const int valor)
{
	//se achei, devolvo o que encontrei mas só se tiver livres
	if (tree->ID==valor)
		{
		if (tree->dlList==nullptr)
			return nullptr;
		return tree;
		}

	//se for para ir para a esquerda
	if (valor<tree->ID)
		{
		//se nao tenho mais à esquerda eu sou o valor optimo, logo devolve se tiver livres
		if (tree->left==nullptr)
			{
			if (tree->dlList==nullptr || tree->ID<valor)
				return nullptr;
			return tree;
			}

		//há melhores, logo ando pra baixo na lista e se esta devolver, porreiro
		AVL *novo;
		if ( (novo=avlFindBest(tree->left,valor))!=nullptr)
			return novo;

		//nao existem nenhuns para baixo, logo se eu tiver mando os meus
		if (tree->dlList==nullptr || tree->ID<valor)
			return nullptr;
		return tree;
		}

	//chegando aqui tenho mesmo de ir para a direita

	//se nao tenho mais à direita eu sou o valor optimo, logo devolve se tiver livres
	if (tree->right==nullptr)
		{
		if (tree->dlList==nullptr || tree->ID<valor)
			return nullptr;
		return tree;
		}

	//há melhores, logo ando pra baixo na lista e se esta devolver, porreiro
	AVL *novo;
	if ( (novo=avlFindBest(tree->right,valor))!=nullptr)
		return novo;

	//nao existem nenhuns para baixo, logo se eu tiver mando os meus
	if (tree->dlList==nullptr || tree->ID<valor)
		return nullptr;
	return tree;
}

bool MemoryManager::avlLeftHeavier(const AVL * const tree)
{
	//assumo que tem de haver pelo menos um filho e por isso nao verifico se right==left==nullptr
	if (tree->right==nullptr)
		return true;
	if (tree->left==nullptr)
		return false;

	if (tree->left->height > tree->right->height)
		return true;
	return false;
}

MemoryManager::AVL* MemoryManager::avlInsert(AVL * const tree, const int valor, bool &nodeExisted)
{
	//se achei o valor que queria
	if (valor==tree->ID)
		{
		nodeExisted=true;
		return tree;
		}

	//para guardar o novo no
	AVL *novoNo;

	//se o valor a guardar for mais pequeno
	if (valor<tree->ID)
		{
		//se não tiver lá nada, crio o filho
		if (tree->left==nullptr)
			{
			//crio o novo filho e preencho os campos
			nodeExisted=false;
			tree->left=novoNo=poolAVL.Alloc();
			if (novoNo!=nullptr)
				{
				novoNo->height=0;
				novoNo->left=novoNo->right=nullptr;
				novoNo->ID=valor;
				novoNo->pai=tree;
				novoNo->dlList=nullptr;
				}
			}
		else
			{
			//senão, desco pelo meu filho
			novoNo=avlInsert(tree->left,valor,nodeExisted);

			//se já existia, tá-se bem
			if (nodeExisted)
				return novoNo;
			}
		}
	//se o valor não é igual nem menor de onde estou, tem de ser maior
	else
		{
		//se não tiver lá nada, crio o filho
		if (tree->right==nullptr)
			{
			//crio o novo filho e preencho os campos
			nodeExisted=false;
			tree->right=novoNo=(AVL*)poolAVL.Alloc();
			if (novoNo!=nullptr)
				{
				novoNo->height=0;
				novoNo->left=novoNo->right=nullptr;
				novoNo->ID=valor;
				novoNo->pai=tree;
				novoNo->dlList=nullptr;
				}
			}
		else
			{
			//senão, desco pelo meu filho
			novoNo=avlInsert(tree->right,valor,nodeExisted);

			//se já existia, tá-se bem
			if (nodeExisted)
				return novoNo;
			}
		}

	//se deu barraca ou se não foi criado nenhum
	if (novoNo==nullptr)
		return nullptr;

	//chegando aqui já tá inserido, mas pode não estar balanceado
	//calculo o novo peso e vejo se tenho de fazer alguma coisa
	tree->height=avlCalcPeso(tree);
	if (avlIsBalanced(tree)==true)
		return novoNo;

	//Chegando aqui tenho mesmo de balancear.

	//para os casos LL e LR
	if (avlLeftHeavier(tree))
		{
		if (avlLeftHeavier(tree->left))
			avlRotateLL(tree);
		else
			avlRotateLR(tree);
		}
	//para os casos RR e RL
	else
		{
		if (avlLeftHeavier(tree->right))
			avlRotateRL(tree);
		else
			avlRotateRR(tree);
		}

	//e já tá
	return novoNo;
}

bool MemoryManager::avlCheckBalance(AVL * const tree)
{
	if (tree==nullptr)
		return false;

	if (tree->left)
		{
		if (avlCheckBalance(tree->left)==false)
			return false;
		}
	if (tree->right)
		{
		if (avlCheckBalance(tree->right)==false)
			return false;
		}
	return true;
}

void MemoryManager::avlRemoveDataFromNode(AVL * const avl, AVL_DATA *avlTrash)
{
	if (avl->dlList==avlTrash)
		{
		avl->dlList=avlTrash->prox;
		if (avl->dlList)
			avl->dlList->prev=nullptr;
		free(avlTrash);
		return;
		}
			
	avlTrash->prev->prox=avlTrash->prox;
	if (avlTrash->prox)
		avlTrash->prox->prev=avlTrash->prev;
	free(avlTrash);
}

void MemoryManager::avlCreateForNode(NO * const node)
{
	AVL_DATA *novoData;
	AVL *novoAVL;
	bool newCreated;

	//isto tem de estar limpo por sao passados como referencias
	newCreated=false;

	//crio uma nova entrada
	if (FreeNodes!=nullptr)
		{
		novoAVL=avlInsert(FreeNodes,getUserDSize(node),newCreated);
		}
	else
		{
		FreeNodes=novoAVL=(AVL*)poolAVL.Alloc();
		if (FreeNodes==nullptr)
			return;
		FreeNodes->height=0;
		FreeNodes->left=FreeNodes->right=nullptr;
		FreeNodes->ID=getUserDSize(node);
		FreeNodes->pai=nullptr;
		FreeNodes->dlList=nullptr;
		}

	//se deu barraca
	if (novoAVL==nullptr)
		return;

	//preencho
	if (novoAVL->dlList==nullptr)
		{
		novoAVL->dlList=(AVL_DATA*)malloc(sizeof(AVL_DATA));
		if (novoAVL->dlList==nullptr)
			return;
		novoAVL->dlList->prox=novoAVL->dlList->prev=nullptr;
		novoAVL->dlList->no=node;

		node->avl=novoAVL;
		node->avlData=novoAVL->dlList;
		node->checkSum=calcCheckSum(node);
		return;
		}

	//incluo na lista duplamente ligada
	novoData=(AVL_DATA*)malloc(sizeof(AVL_DATA));
	if (novoData==nullptr)
		return;

	//arranjo os ponteiros
	novoData->no=node;
	novoData->prox=novoAVL->dlList;
	novoData->prev=nullptr;
	novoAVL->dlList->prev=novoData;
	novoAVL->dlList=novoData;

	node->avl=novoAVL;
	node->avlData=novoAVL->dlList;
	node->checkSum=calcCheckSum(node);
}

void MemoryManager::avlDelete(AVL * const tree)
{
	AVL_DATA *walk,*lixo;

	if (tree==nullptr)
		return;

	//apagar a lista
	walk=tree->dlList;
	while(walk!=nullptr)
		{
		lixo=walk;
		walk=walk->prox;
		free(lixo);
		}

	//nunca esquecer que ponteiros AVL nunca se apagam
	//porque foram allocados com poolAVL
	avlDelete(tree->right);
	avlDelete(tree->left);
}

void MemoryManager::avlGetInfo(const AVL * const tree, unsigned int &nNodes, unsigned int &nBPointers, unsigned int &nLevels, unsigned int &sumFBlocks, const int curLevel)
{
	if (tree==nullptr)
		return;

	//conto no número de ponteiros para os blocos
	{
	for(const AVL_DATA * walk=tree->dlList; walk!=nullptr; walk=walk->prox)
		{
		sumFBlocks+=tree->ID;
		nBPointers++;
		}
	}

	//se tiver algum filho, tenho de juntar

	//seguir pelos filhos
	if (tree->right)	avlGetInfo(tree->right,nNodes,nBPointers,nLevels,sumFBlocks,curLevel+1);
	if (tree->left)		avlGetInfo(tree->left,nNodes,nBPointers,nLevels,sumFBlocks,curLevel+1);

	//se deve actualizar o nivel maximo
	if (nLevels<((unsigned int)curLevel))
		nLevels=(unsigned int)curLevel;

	//estou num nó, conta isto
	nNodes++;
}

/*§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§
§§§§§§   -= Funcoes auxiliares =-   §§§§§
§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§*/
int MemoryManager::writeLeaks()
{
	NO *walk;
	int count;
	MEM_POOL *walkerM;
	HorseRadish::String strBytesSize;

	//para cada pool
	count=0;
	for(walkerM=MemPools; walkerM!=nullptr; walkerM=walkerM->prox)
	{
		//para cada bloco
		for(walk=(NO*)walkerM->memPool; walk<=walkerM->memPoolEnd; walk=getNextNO(walk))
		{
			//se tiver livre cago nele
			if (walk->avlData)
				continue;

			//tenho um leak
			strBytesSize.SetMemory(getUserDSize(walk));
			myLogInTAB(logID, "[%s]\t- \"%s\" - %d", strBytesSize.GetData(), walk->infoSource, walk->infoLine);
			count++;
		}
	}

	//devolvo quantos achei
	return count;
}

MemoryManager::MEM_POOL* MemoryManager::procuraPool(const unsigned char* const pointer)
{
	MEM_POOL *walkerM;

	//tenho sempre de ter alguma coisa
	if (MemPools==nullptr)
		return nullptr;

	//caso muito usual, logo aproveito
	if (pointer>=MemPools->memPool && pointer<(((unsigned char*)MemPools->memPool)+MemPools->memTOTAL))
		return MemPools;

	//passo por todos à procura
	for(walkerM=MemPools->prox; walkerM!=nullptr; walkerM=walkerM->prox)
	{
		if (pointer>=walkerM->memPool && pointer<(((unsigned char*)walkerM->memPool)+walkerM->memTOTAL))
			return walkerM;
	}
	return nullptr;
}

void MemoryManager::verificaEstado(int * const erros, const unsigned int nNodesInAVL)
{
	NO *walk,*next;
	MEM_POOL *walkerM;
	
	//por defeito nao existem erros
	*erros=0;

	//***********
	//toca a passar por eles todos
	for(walkerM=MemPools; walkerM!=nullptr; walkerM=walkerM->prox)
	{
		//verificar se o último tá livre
		if (((NO*)walkerM->memPoolEnd)->avlData!=nullptr)
			(*erros)|=ERRO_LAST_FREE;

		//para cada allocação
		for(walk=(NO*)walkerM->memPool; walk<=walkerM->memPoolEnd; walk=getNextNO(walk))
		{
			//o próximo
			next=nullptr;
			if (walk!=walkerM->memPoolEnd)
				next=getNextNO(walk);

			//estes campos não podem estar a zero
			if ( (walk->noSize<=(sizeof(NO)+walk->pad)) || (walk!=walkerM->memPool && (walk->prevNoSize<=sizeof(NO))) )
				(*erros)|=ERRO_WRONG_DATA;

			//verificar o checksum
			if (walk->checkSum!=calcCheckSum(walk))
				(*erros)|=ERRO_CHECKSUM;

			//verificar se o pad está bem escrito
			if ((getUserPTR(walk))[-1]!=walk->pad)
				(*erros)|=ERRO_PAD_TAG;

			//daqui pra frente preciso sempre do next, por isso se nao o tiver sigo
			if (next==nullptr)
				continue;

			//o prévio tem de ter o meu valor correcto
			if (next->prevNoSize!=walk->noSize)
				(*erros)|=ERRO_PREV_SIZE;

			//agora não pode tambem haver grupinhos de ponteiros livres
			if (walk->avlData!=nullptr && next->avlData!=nullptr)
				(*erros)|=ERRO_GROUP_FREE;

			//se está livre, cá dentro tem de estar o pattern para libertado
			/*if (walk->avlData!=nullptr && checkForPattern(getUserPTR(walk),getUserDSize(walk),patternReleased)==false)
				(*erros)|=ERRO_PATTERN_RELEASE;*/
		}
	}

	//vou só verificar se o número de nós entre a árvore e o fixed pool está correcto
	if (poolAVL.GetNumAllocs()!=nNodesInAVL)
		(*erros)|=ERRO_WRONG_AVL_NODES;
}

void MemoryManager::destroyPool(MEM_POOL * const lixo)
{
	//verifico parametro
	if (lixo==nullptr)
		return;

	//limpo a entrada
	if (lixo->memPool)
		VirtualFree((void *)lixo->memPool,0,MEM_RELEASE);

	//retiro da lista
	//se for o primeiro
	if (lixo==MemPools)
		{
		MemPools=MemPools->prox;
		if (MemPools)
			MemPools->prev=nullptr;
		free(lixo);
		return;
		}

	//se estiver a meio ou for o ultimo
	lixo->prev->prox=lixo->prox;
	if (lixo->prox)
		lixo->prox->prev=lixo->prev;
	free(lixo);
}

MemoryManager::MEM_POOL* MemoryManager::createPool(const int tamanho)
{
	MEM_POOL *walkerM;

	//crio nova entrada
	if (MemPools==nullptr)
		{
		MemPools=(MEM_POOL*)malloc(sizeof(MEM_POOL));
		if (MemPools==nullptr)
			return nullptr;
		MemPools->prox=nullptr;
		MemPools->prev=nullptr;
		walkerM=MemPools;
		}
	else
		{
		for(walkerM=MemPools; walkerM->prox!=nullptr; walkerM=walkerM->prox);
		walkerM->prox=(MEM_POOL*)malloc(sizeof(MEM_POOL));
		if (walkerM->prox==nullptr)
			return nullptr;
		walkerM->prox->prox=nullptr;
		walkerM->prox->prev=walkerM;
		walkerM=walkerM->prox;
		}

	//arranjo parametros
	walkerM->numNos=0;
	walkerM->memPool=nullptr;
	walkerM->memPoolEnd=nullptr;
	walkerM->memTOTAL=0;

	//se o tamanho for maior que o pedido
	if (tamanho>=mngPoolSize)
		{
		walkerM->memTOTAL=tamanho;
		walkerM->memPool=(unsigned char *)VirtualAlloc(nullptr,walkerM->memTOTAL,MEM_COMMIT,PAGE_READWRITE);
		walkerM->memPoolEnd=walkerM->memPool;
		if (walkerM->memPool==nullptr)
			{
			destroyPool(walkerM);
			return nullptr;
			}
		}
	//tento criar com o tamanho indicado, senao divido por 2 e tento outra vez, a ultima tentativa
	else
		{
		walkerM->memTOTAL=mngPoolSize;
		walkerM->memPool=(unsigned char *)VirtualAlloc(nullptr,walkerM->memTOTAL,MEM_COMMIT,PAGE_READWRITE);
		walkerM->memPoolEnd=walkerM->memPool;
		if (walkerM->memPool==nullptr)
			{
			walkerM->memTOTAL/=2;
			if (walkerM->memTOTAL<tamanho)
				walkerM->memTOTAL=tamanho;
			walkerM->memPool=(unsigned char *)VirtualAlloc(nullptr,walkerM->memTOTAL,MEM_COMMIT,PAGE_READWRITE);
			walkerM->memPoolEnd=walkerM->memPool;
			if (walkerM->memPool==nullptr)
				{
				destroyPool(walkerM);
				return nullptr;
				}
			}
		}

	//limpo o pool com o pattern de não usado
	HorseRadish::Machine::AsmBufferSetUI32(walkerM->memPool,patternUnused,walkerM->memTOTAL);

	//tudo OK
	return walkerM;
}

int MemoryManager::getPoolFreeSpace(const MEM_POOL * const pool)
{
	unsigned char *last;
		
	last=((unsigned char*)pool->memPoolEnd);
	last+=((NO*)pool->memPoolEnd)->noSize;

	return (pool->memTOTAL - (last-((unsigned char*)pool->memPool)));
}


/*§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§
§§§§§§   -= A classe MemoryManager =-   §§§§§
§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§*/
int MemoryManager::procStat(const int flag)
{
	//se a flag é rasca, devolvo ERRO
	if (flag<=0)
		return -1;

	//saber o número de pools
	if (flag==SMEM_MANAGER_NUMBER_OF_POOLS)
		{
		int count=0;
		for(const MEM_POOL *walkerM=MemPools; walkerM!=nullptr; walkerM=walkerM->prox)
			count++;
		return count;
		}

	//saber o número de ponteiros (total)
	if (flag==SMEM_MANAGER_NUMBER_OF_POINTERS)
		{
		int count=0;
		for(const MEM_POOL *walkerM=MemPools; walkerM!=nullptr; walkerM=walkerM->prox)
			{
			for(NO *walk=(NO*)walkerM->memPool; walk<=walkerM->memPoolEnd; walk=getNextNO(walk))
				count++;
			}
		return count;
		}

	//saber o número de ponteiros livres
	if (flag==SMEM_MANAGER_NUMBER_OF_POINTERS_FREE)
		{
		int count=0;
		for(const MEM_POOL *walkerM=MemPools; walkerM!=nullptr; walkerM=walkerM->prox)
			{
			for(NO *walk=(NO*)walkerM->memPool; walk<=walkerM->memPoolEnd; walk=getNextNO(walk))
				{
				if (walk->avlData)
					count++;
				}
			}
		return count;
		}

	//saber o número de ponteiros allocados
	if (flag==SMEM_MANAGER_NUMBER_OF_POINTERS_IN_USE)
		{
		int count=0;
		for(const MEM_POOL *walkerM=MemPools; walkerM!=nullptr; walkerM=walkerM->prox)
			{
			for(NO *walk=(NO*)walkerM->memPool; walk<=walkerM->memPoolEnd; walk=getNextNO(walk))
				{
				if (walk->avlData==nullptr)
					count++;
				}
			}
		return count;
		}

	//*********
	//saber a memória total reservada
	if (flag==SMEM_MANAGER_MEMORY_POOL)
		{
		int count=0;
		for(const MEM_POOL *walkerM=MemPools; walkerM!=nullptr; walkerM=walkerM->prox)
			count+=walkerM->memTOTAL;
		return count;
		}

	//saber a memória allocada pelo utilizador
	if (flag==SMEM_MANAGER_MEMORY_POOL_ALLOCATED)
		{
		int count=0;
		for(const MEM_POOL *walkerM=MemPools; walkerM!=nullptr; walkerM=walkerM->prox)
			{
			for(NO *walk=(NO*)walkerM->memPool; walk<=walkerM->memPoolEnd; walk=getNextNO(walk))
				{
				if (walk->avlData==nullptr)
					count+=getUserDSize(walk);
				}
			}
		return count;
		}

	//saber a memória total allocado livre
	if (flag==SMEM_MANAGER_MEMORY_POOL_FREE)
		{
		int count=0;
		for(const MEM_POOL *walkerM=MemPools; walkerM!=nullptr; walkerM=walkerM->prox)
			{
			for(NO *walk=(NO*)walkerM->memPool; walk<=walkerM->memPoolEnd; walk=getNextNO(walk))
				{
				if (walk->avlData)
					count+=getUserDSize(walk);
				}
			}
		return count;
		}

	//saber o pico de memória allocada
	if (flag==SMEM_MANAGER_MEMORY_POOL_PEAK)
		{
		return InfoData.memPeak;
		}

	//saber a memória em excesso (necessária ao gestor)
	if (flag==SMEM_MANAGER_MEMORY_EXCESS)
		{
		int count;
		unsigned int nNodes,nBPointers,nLevels,sumFBlocks;

		//tiro os valores
		nNodes=nBPointers=nLevels=sumFBlocks=0;
		avlGetInfo(FreeNodes,nNodes,nBPointers,nLevels,sumFBlocks,1);
		
		//faço as contas e já tá
		count=nNodes*sizeof(AVL) + nBPointers*sizeof(AVL_DATA);
		count+=this->Stat(SMEM_MANAGER_NUMBER_OF_POINTERS)*sizeof(NO);
		return count;
		}

	//*********
	//saber o número total de nós da árvore AVL
	if (flag==SMEM_MANAGER_AVL_NUM_NODES)
		{
		unsigned int nNodes,nBPointers,nLevels,sumFBlocks;

		//inicio a zero, tiro os valores e devolvo
		nNodes=nBPointers=nLevels=sumFBlocks=0;
		avlGetInfo(FreeNodes,nNodes,nBPointers,nLevels,sumFBlocks,1);
		return nNodes;
		}

	//saber o número de níveis da árvore AVL
	if (flag==SMEM_MANAGER_AVL_NUM_LEVELS)
		{
		unsigned int nNodes,nBPointers,nLevels,sumFBlocks;

		//inicio a zero, tiro os valores e devolvo
		nNodes=nBPointers=nLevels=sumFBlocks=0;
		avlGetInfo(FreeNodes,nNodes,nBPointers,nLevels,sumFBlocks,1);
		return nLevels;
		}

	//chegando aqui é barraca
	return -1;
}

void MemoryManager::procSnapshot()
{
	const MEM_POOL *walkerM;
	NO *walk;

	//inicio
	myLogIn(logID," ");
	myLogIn(logID,".:Listing of information regarding every allocation in the manager:.");
	myLogIn(logID,"   Addr       Size       Addr       Size     Flags");
	myLogIn(logID," Reported   Reported    Actual     Actual    C F P  Allocated by");
	myLogIn(logID,"---------- ---------- ---------- ---------- ------- ---------------------------------------------------");

	//para cada mempool
	for(walkerM=MemPools; walkerM!=nullptr; walkerM=walkerM->prox)
		{
		//para cada allocado
		for(walk=(NO*)walkerM->memPool; walk<=walkerM->memPoolEnd; walk=getNextNO(walk))
			{
			myLogIn(logID,"0x%08X 0x%08X 0x%08X 0x%08X  %c %c %c  %s (%d)",
				getUserPTR(walk),
				getUserDSize(walk),
				walk,
				walk->noSize,
				checkCheckSum(walk) ? ' ':'X',
				walk->avlData==nullptr ? ' ':'X',
				' ',//(walk->avlData==nullptr || checkForPattern(getUserPTR(walk),getUserDSize(walk),patternReleased)) ? ' ':'X',
				walk->infoSource,
				walk->infoLine);
			}
		}

	//e termino
	myLogIn(logID,".:Listing information finished:.");
	myLogIn(logID," ");
}

void* MemoryManager::procAlloc(const int bytes, const char * const file, const int line)
{
	unsigned char *markPad;
	NO *no,*last,*newNo,*next;
	MEM_POOL *walkerM;
	AVL *bestFree;
	int oldNoSize;

	//se nao precisar
	if (bytes<=0)
		{
		myLogIn(logID,"Returning nullptr when asked for 0 bytes of memory!");
		myLogIn(logID,"Request made at \"%s\" in line %d.",file,line);
		return nullptr;
		}

	//procuro por espaço livre nos pool
	for(walkerM=MemPools; walkerM!=nullptr; walkerM=walkerM->prox)
		{
		//vou procurar um espaço livre na lista, se esta existir
		//depois se não tiver nada na lista, sigo
		if (FreeNodes!=nullptr && (bestFree=avlFindBest(FreeNodes,bytes))!=nullptr)
			{
			//verifico só se não é muito grande
			if ( checkSplit(bestFree->dlList->no,bytes+BYTE_SLACK)==true)
				{
				//tiro o novo no e a ligacao à lista
				no=bestFree->dlList->no;
				avlRemoveDataFromNode(bestFree,bestFree->dlList);

				//vou buscar o próximo (tem de ser agora pq a seguir mudo o tamanho dele)
				next=getNextNO(no);

				//guardar o tamanho total antigo
				oldNoSize=no->noSize;

				//preencher o novo campo
				no->infoSource[0]='\0';
				copiaString(file,no->infoSource);
				no->infoLine=line;
				no->avl=nullptr;
				no->avlData=nullptr;
				no->noSize=sizeof(NO)+ bytes + no->pad;
				no->checkSum=calcCheckSum(no);

				//estatisticas
				InfoData.memTOTAL+=bytes;
				InfoData.memPeak=intMax(InfoData.memTOTAL,InfoData.memPeak);

				//o novo livre mais à frente
				newNo=(NO*)(((unsigned char*)no)+no->noSize);

				//preencho o livre
				newNo->infoSource[0]='\0';
				newNo->infoLine=-1;
				newNo->avl=nullptr;
				newNo->avlData=nullptr;
				newNo->pad=calcPad((unsigned char*)newNo + sizeof(NO));
				newNo->noSize=oldNoSize-no->noSize;
				newNo->prevNoSize=no->noSize;
				newNo->checkSum=calcCheckSum(newNo);
				
				//o da frente
				next->prevNoSize=newNo->noSize;

				//este que criei tem de ser livre!
				avlCreateForNode(newNo);

				//não esquecer de escrever o pad
				markPad=getUserPTR(newNo);
				markPad[-1]=newNo->pad;
				
				//e já tá
				return (void*)getUserPTR(no);
				}
				
			//chegando aqui pode ir todo tal qual como está
			//logo, tiro o nó correspondente, apago a entrada e arranjo o ponteiro
			newNo=bestFree->dlList->no;
			avlRemoveDataFromNode(bestFree,bestFree->dlList);

			//só para dizer que já não estou livre
			newNo->avlData=nullptr;
			newNo->avl=nullptr;

			//estatisticas
			InfoData.memTOTAL+=bytes;
			InfoData.memPeak=intMax(InfoData.memTOTAL,InfoData.memPeak);

			//e já está
			newNo->checkSum=calcCheckSum(newNo);
			return (void*)getUserPTR(newNo);
			}

		//vou buscar o ultimo e verifico se tenho espaço no que resta da pool para o pedido
		//somo 256 que é por causa do pad que pode ter para ficar alinhado
		last=(NO*)walkerM->memPoolEnd;
		if ( ((unsigned char*)last-(unsigned char*)walkerM->memPool+last->noSize+sizeof(NO)+bytes+256)>=walkerM->memTOTAL)
			continue;
			
		//chegando aqui posso descançar pois o meu pedido cabe num pool
		//sei que alguma coisa já foi allocada pois, se nao assim fosse, este pool nao existia
	
		//vou escrever neste
		newNo=getNextNO(last);

		//arranjo o novo NO
		newNo->infoSource[0]='\0';
		copiaString(file,newNo->infoSource);
		newNo->infoLine=line;
		newNo->avl=nullptr;
		newNo->avlData=nullptr;
		newNo->prevNoSize=last->noSize;
		newNo->pad=calcPad((unsigned char*)newNo + sizeof(NO));
		newNo->noSize=sizeof(NO)+ bytes + newNo->pad;
		newNo->checkSum=calcCheckSum(newNo);

		//arranja o pool
		walkerM->numNos++;
		walkerM->memPoolEnd=newNo;

		//estatistica
		InfoData.memTOTAL+=bytes;
		InfoData.memPeak=intMax(InfoData.memTOTAL,InfoData.memPeak);

		//vou buscar o ponteiro que vou mandar e marco o pad um byte antes
		//o pad mesmo que tenha sido zero, vai calhar em cima do pad na estructura, logo nao há crise
		markPad=getUserPTR(newNo);
		markPad[-1]=newNo->pad;

		return (void*)markPad;
		}

	//nao encontrei espaço livre, logo, tenho de o criar
	//se quero mais memoria que o poolSize (nao esquecer o espaco para a estrcutura e pad
	walkerM=createPool(bytes+sizeof(NO)+256);
	if (walkerM==nullptr)
		{
		myLogIn(logID,"Returning nullptr when asked for %d bytes of memory!",bytes);
		myLogIn(logID,"Request made at \"%s\" in line %d.",file,line);
		return nullptr;
		}

	//vou escrever neste
	newNo=(NO*)walkerM->memPool;

	//arranjo o novo NO
	newNo->infoSource[0]='\0';
	copiaString(file,newNo->infoSource);
	newNo->infoLine=line;
	newNo->avl=nullptr;
	newNo->avlData=nullptr;
	newNo->prevNoSize=0;
	newNo->pad=calcPad((unsigned char*)newNo + sizeof(NO));
	newNo->noSize=sizeof(NO)+ bytes + newNo->pad;
	newNo->checkSum=calcCheckSum(newNo);

	//arranja o pool
	walkerM->numNos++;
	walkerM->memPoolEnd=newNo;

	//estatisticas
	InfoData.memTOTAL+=bytes;
	InfoData.memPeak=intMax(InfoData.memTOTAL,InfoData.memPeak);

	//vou buscar o ponteiro que vou mandar e marco o pad um byte antes
	//o pad mesmo que tenha sido zero, vai calhar em cima do pad na estructura, logo nao há crise
	markPad=getUserPTR(newNo);
	markPad[-1]=newNo->pad;

	return (void*)markPad;
}

void MemoryManager::procFree(void * const pointer)
{
	NO *no,*behind,*front;
	MEM_POOL *walkerM;

	//procuro apontador nos pools
	walkerM=procuraPool((unsigned char*)pointer);
	if (walkerM==nullptr)
		{
		InfoData.freeError++;
		return;
		}

	//posso buscar o ponteiro assim e depois verifico o checksum
	no=(NO*)( ((unsigned char*)pointer) - sizeof(NO) - ((unsigned char*)pointer)[-1] );
	if (checkCheckSum(no)==false)
		{
		myLogIn(logID,"Checksum of pointer is incorrect!");
		myLogInTAB(logID,"pointer: %p",pointer);
		return;
		}

	//sei que vou libertar este pedaço de memoria, logo, posso subtrai-lo
	walkerM->numNos--;
	InfoData.memTOTAL-=getUserDSize(no);

	//meter a zero
	//SAsmBufferClear(pointer,getUserDSize(no));
	
	//<<<<<<<<<<<<
	//OPTIMIZAÇÕES
	behind=front=nullptr;
	if (no!=walkerM->memPool)		behind=getPrevNO(no);
	if (no!=walkerM->memPoolEnd)	front=getNextNO(no);

	//***********
	//se estiver sozinho
	if (walkerM->memPool==no && walkerM->memPoolEnd==no)
		{
		destroyPool(walkerM);
		return;
		}

	//***********
	//se for o ultimo (escuso de criar um espaco vazio)
	if (front==nullptr)
		{
		//se o anterior estiver livre, levo-o comigo
		if (behind->avlData)
			{
			//se ele é o primeiro, então isto vai à vida
			if (behind==walkerM->memPool)
				{
				avlRemoveDataFromNode(behind->avl,behind->avlData);
				destroyPool(walkerM);
				return;
				}

			//tenho de tirar a entrada nos livres
			avlRemoveDataFromNode(behind->avl,behind->avlData);
			behind->avlData=nullptr;
			behind->avl=nullptr;
			behind->checkSum=calcCheckSum(behind);

			//elimino, actualizo as coisas e out of here
			walkerM->numNos--;
			walkerM->memPoolEnd=getPrevNO(behind);
			return;
			}

		//actualizo o ultimo e xau
		walkerM->memPoolEnd=behind;
		return;
		}

	//***********
	//se for o primeiro
	if (behind==nullptr)
		{
		//se o da frente está livre, posso-o levar comigo
		if (front->avlData)
			{
			//se é o ultimo da pool, a pool vai com os cães
			if (front==walkerM->memPoolEnd)
				{
				avlRemoveDataFromNode(front->avl,front->avlData);
				destroyPool(walkerM);
				return;
				}

			//apago o da frente (nao posso usar a entrada pq os tamanhos são fixos)
			avlRemoveDataFromNode(front->avl,front->avlData);
			front->avlData=nullptr;
			front->checkSum=calcCheckSum(front);
	
			//somo o valor do próximo a mim e actualizo o tamanho meu marcado no a seguir
			no->noSize+=front->noSize;
			(getNextNO(no))->prevNoSize=no->noSize;

			//preencho com o pattern
			//SAsmBufferSetUI32(getUserPTR(no),patternReleased,getUserDSize(no));
			
			//fico então livre, crio entrada, actualizo algumas coisas e prontos
			avlCreateForNode(no);
			walkerM->numNos--;
			return;
			}

		//crio uma entrada nova na lista, preencho com o pattern
		avlCreateForNode(no);
		//SAsmBufferSetUI32(getUserPTR(no),patternReleased,getUserDSize(no));
		return;
		}

	//>>>> chegando aqui estou sempre no meio de coisas

	//*********
	//se tenho os dois ao lado livres
	if ( (behind->avlData) && (front->avlData) )
		{
		//nunca pode haver só 3 por isso implicava que o front era o ultimo e livre ao mesmo tempo
		//e no fim da lista nunca existe nenhum livre

		//apago o da frente e do detrás (nao os posso usar pq os tamanhos são fixos)
		avlRemoveDataFromNode(behind->avl,behind->avlData);
		avlRemoveDataFromNode(front->avl,front->avlData);
		behind->avlData=front->avlData=nullptr;
		behind->checkSum=calcCheckSum(behind);
		front->checkSum=calcCheckSum(front);

		//posso já eliminar este free
		behind->noSize+=no->noSize+front->noSize;
		(getNextNO(behind))->prevNoSize=behind->noSize;

		//preencho com o pattern
		//SAsmBufferSetUI32(getUserPTR(behind),patternReleased,getUserDSize(behind));

		//fico livre, crio a entrada, actualizo e adeus!
		avlCreateForNode(behind);
		walkerM->numNos-=2;
		return;
		}

	//*********
	//se o da frente está livre
	if (front->avlData)
		{
		//apago o da frente (nao o posso usar pq os tamanhos são fixos)
		avlRemoveDataFromNode(front->avl,front->avlData);
		front->avlData=nullptr;
		front->checkSum=calcCheckSum(front);

		//posso já eliminar este free
		no->noSize+=front->noSize;
		(getNextNO(no))->prevNoSize=no->noSize;

		//preencho com o pattern
		//SAsmBufferSetUI32(getUserPTR(no),patternReleased,getUserDSize(no));

		//marco como livre, crio entrada, actualizo e adeus!
		avlCreateForNode(no);
		walkerM->numNos--;
		return;
		}

	//*********
	//se o da trás está livre
	if (behind->avlData)
		{
		//apago o da detrás (nao o posso usar pq os tamanhos são fixos)
		avlRemoveDataFromNode(behind->avl,behind->avlData);
		behind->avlData=nullptr;

		//posso já eliminar este free
		behind->noSize+=no->noSize;
		(getNextNO(behind))->prevNoSize=behind->noSize;

		//preencho com o pattern
		//SAsmBufferSetUI32(getUserPTR(behind),patternReleased,getUserDSize(behind));

		//marco como livre, crio entrada, actualizo e adeus!
		avlCreateForNode(behind);
		walkerM->numNos--;
		return;
		}

	//***********
	//não dá pra optimizar, logo tem direito a uma entrada, pattern e prontos
	avlCreateForNode(no);
	//SAsmBufferSetUI32(getUserPTR(no),patternReleased,getUserDSize(no));
}

void* MemoryManager::procRealloc(const void * const pointer, const int bytes, const char * const file, const int line)
{
	void *novo;
	NO *no,*front;
	MEM_POOL *walkerM;

	//verificar parametros
	if (MemPools==nullptr || bytes<=0)
		{
		myLogInTAB(logID,"Returning nullptr when asked to reallocate %d bytes of memory!",bytes);
		return nullptr;
		}

	//se tiver um ponteiro a nullptr é como se tivesse nada, faço tudo normalmente
	if (pointer==nullptr)
		return procAlloc(bytes,file,line);
		
	//procuro apontador nos pools
	walkerM=procuraPool((unsigned char*)pointer);
	if (walkerM==nullptr)
		{
		InfoData.reallocError++;
		myLogIn(logID,"Returning nullptr when asked to reallocate %d bytes of memory!",bytes);
		myLogInTAB(logID,"could not find pointer: %p",pointer);
		return nullptr;
		}

	//ou posso procurar assim e verificar depois o checksum
	no=(NO*)( ((unsigned char*)pointer) - sizeof(NO) - ((unsigned char*)pointer)[-1] );
	if (checkCheckSum(no)==false)
		{
		myLogIn(logID,"Checksum of pointer is incorrect!");
		myLogInTAB(logID,"pointer: %p",pointer);
		return nullptr;
		}

	//se ja estava libertado
	if (no->avlData)
		{
		InfoData.reallocError++;
		myLogIn(logID,"Returning nullptr when asked to reallocate %d bytes of memory!",bytes);
		myLogInTAB(logID,"pointer to reallocate is not in use: %p",pointer);
		return nullptr;
		}

	//se o tamanho já for maior, devolvo o proprio ponteiro
	//isto pode acontecer, pois no ultimo realloc posso ter juntado um bloco muito grande
	if (getUserDSize(no)>=bytes)
		return (void*)getUserPTR(no);

	//primeira coisa é ver se este ponteiro é o ultimo e se tiver espaço em frente
	if ((no==walkerM->memPoolEnd) && ((getPosNO(walkerM,no)+sizeof(NO)+bytes+no->pad)<walkerM->memTOTAL) )
		{
		//já posso fazer isto
		InfoData.memTOTAL+=bytes-getUserDSize(no);
		InfoData.memPeak=intMax(InfoData.memTOTAL,InfoData.memPeak);

		//actualizo novo ponteiro, arranjo bounds e devolvo
		no->noSize=sizeof(NO)+no->pad+bytes;
		no->checkSum=calcCheckSum(no);
		return (void*)getUserPTR(no);
		}

	//segunda tentativa é ver se tenho pessoal livre à frente
	if ( (no!=walkerM->memPoolEnd) )
		{
		//o próximo
		front=getNextNO(no);
			
		//se tiver e se o espaço dele mais o de agora der para a encomenda
		if ( (front->avlData) && (getUserDSize(no)+getUserDSize(front)>=bytes))
			{
			//já posso fazer isto
			InfoData.memTOTAL+=getUserDSize(front);
			InfoData.memPeak=intMax(InfoData.memTOTAL,InfoData.memPeak);

			//tiro o da frente dos livres
			avlRemoveDataFromNode(front->avl,front->avlData);
			front->avl=nullptr;
			front->avlData=nullptr;

			//posso juntar o próximo
			no->noSize+=front->noSize;
			(getNextNO(no))->prevNoSize=no->noSize;

			//devolvo finalmente
			no->checkSum=calcCheckSum(no);
			return (void*)getUserPTR(no);
			}
		}

	//nao consegui optimizar, faço tudo na mesma
	novo=procAlloc(bytes,file,line);
	if (novo==nullptr)
		{
		myLogInTAB(logID,"Returning nullptr when asked to reallocate %d bytes of memory!",bytes);
		return nullptr;
		}

	//copio para la as coisas
	HorseRadish::Machine::AsmBufferCopy(novo,getUserPTR(no),getUserDSize(no));
	procFree(getUserPTR(no));
	return novo;
}

void MemoryManager::procCleanEverything(const bool writeStatus)
{
	//se for para escrever alguma coisa
	if (writeStatus)
	{
		unsigned int leaks,nNodes,nBPointers,nLevels,sumFBlocks;
		HorseRadish::String strFormatted;
		int erros;

		//escrevo alguma informacao
		SConsolePLog(logID," ");
		strFormatted.SetCurrentTime();
		myLogIn(logID,".:Dumping information started (%s):.", strFormatted.GetData());

		//Mostar como está agora
		SConsolePLog(logID,"Current state:");
		strFormatted.SetMemory(this->Stat(SMEM_MANAGER_MEMORY_POOL));
		myLogInTAB(logID,"memory pool total: %s", strFormatted.GetData());
		strFormatted.SetMemory(this->Stat(SMEM_MANAGER_MEMORY_POOL_FREE));
		myLogInTAB(logID,"memory pool free: %s", strFormatted.GetData());
		myLogInTAB(logID,"number of pools: %d", this->Stat(SMEM_MANAGER_NUMBER_OF_POOLS));
		myLogInTAB(logID,"number of blocks (total): %d", this->Stat(SMEM_MANAGER_NUMBER_OF_POINTERS));
		myLogInTAB(logID,"number of blocks (free): %d", this->Stat(SMEM_MANAGER_NUMBER_OF_POINTERS_FREE));
		myLogInTAB(logID,"number of blocks (in use): %d", this->Stat(SMEM_MANAGER_NUMBER_OF_POINTERS_IN_USE));
		strFormatted.SetMemory(this->Stat(SMEM_MANAGER_NUMBER_OF_POINTERS)*sizeof(NO));
		myLogInTAB(logID,"memory excess: %s", strFormatted.GetData());

		//algumas informaçõe da árvore
		SConsolePLog(logID,"Tree of free nodes:");
		nNodes=nBPointers=nLevels=sumFBlocks=0;
		avlGetInfo(FreeNodes,nNodes,nBPointers,nLevels,sumFBlocks,1);
		myLogInTAB(logID,"number of nodes: %d",nNodes);
		myLogInTAB(logID,"number of block pointers: %d",nBPointers);
		myLogInTAB(logID,"number of levels: %d",nLevels);
		strFormatted.SetMemory(sumFBlocks);
		myLogInTAB(logID,"memory reserved in tree: %s", strFormatted.GetData());
		strFormatted.SetMemory(nNodes*sizeof(AVL) + nBPointers*sizeof(AVL_DATA));
		myLogInTAB(logID,"memory excess: %s", strFormatted.GetData());

		//Mostrar informações globais
		SConsolePLog(logID,"Overall statistics:");
		strFormatted.SetMemory(InfoData.memPeak);
		myLogInTAB(logID,"memory peak: %s", strFormatted.GetData());
		myLogInTAB(logID,"number of Alloc calls: %d",InfoData.numAllocCalls);
		myLogInTAB(logID,"number of AllocDebug calls: %d",InfoData.numAllocDebugCalls);
		myLogInTAB(logID,"number of Free calls: %d",InfoData.numFreeCalls);
		myLogInTAB(logID,"number of Free calls with wrong pointers: %d",InfoData.freeError);
		myLogInTAB(logID,"number of Realloc calls: %d",InfoData.numReallocCalls);
		myLogInTAB(logID,"number of ReallocDebug calls: %d",InfoData.numReallocDebugCalls);
		myLogInTAB(logID,"number of Realloc calls with wrong pointers: %d",InfoData.reallocError);
		myLogInTAB(logID,"number of BlockMelt calls: %d",InfoData.numBlockMeltCalls);
		myLogInTAB(logID,"number of BlockAlloc calls: %d",InfoData.numBlockAllocCalls);
		myLogInTAB(logID,"number of BlockGrow calls: %d",InfoData.numBlockGrowCalls);
		myLogInTAB(logID,"number of BlockGrow calls with wrong pointers: %d",InfoData.blockGrowError);

		//verifico se existem error de implementacao
		SConsolePLog(logID,"Internal error checks:");
		verificaEstado(&erros,nNodes);
		if (erros & ERRO_CHECKSUM)			SConsolePLogTab(logID,"a node checksum was incorrect!",2);
		if (erros & ERRO_LAST_FREE)			SConsolePLogTab(logID,"the last node of a pool was free!",2);
		if (erros & ERRO_GROUP_FREE)		SConsolePLogTab(logID,"there is at least a pair of free nodes!",2);
		if (erros & ERRO_PREV_SIZE)			SConsolePLogTab(logID,"the previous size of a node was incorrect!",2);
		if (erros & ERRO_PAD_TAG)			SConsolePLogTab(logID,"the pad field had incorrect values!",2);
		if (erros & ERRO_WRONG_DATA)		SConsolePLogTab(logID,"a node had impossible values!",2);
		if (erros & ERRO_WRONG_AVL_NODES)	SConsolePLogTab(logID,"wrong number of nodes between AVL tree and fixed pool!",2);
		if (erros & ERRO_PATTERN_RELEASE)	SConsolePLogTab(logID,"a free node has corrupted release pattern!",2);
		if (erros==0)
			SConsolePLogTab(logID,"manager passed all internal error checks :)",2);

		//escrevo os leak que encontrar e escrevo quantos encontrei
		SConsolePLog(logID,"Current leaks:");
		leaks=writeLeaks();
		if (leaks==0)
			SConsolePLogTab(logID,"no leaks",2);
		else
			myLogInTAB(logID,"number of leaks: %d",leaks);

		//fecho
		SConsolePLogInfo(logID,".:Dumping information finished:.");
		SConsolePLog(logID," ");
	}

	//vou às pools
	while(MemPools!=nullptr)
		destroyPool(MemPools);

	//apagar a árvore e tudo o que tem lá dentro
	avlDelete(FreeNodes);
	FreeNodes=nullptr;

	//também agora já posso apagar os pools fixos para os nós AVL
	poolAVL.Clear();
	
	//variaveis finais
	memset(&InfoData,0,sizeof(STATS));
	MemPools=nullptr;
}

MemoryManager::MemoryManager(const int MBytes, const int alignBytes, const int flags) : poolAVL(50)
{
	HorseRadish::String strBytesSize;

	//verificar isto
	if (alignBytes==2 || alignBytes==4 || alignBytes==8 || alignBytes==16 || alignBytes==32 || alignBytes==64 || alignBytes==128)
		mngAlign=alignBytes;
	if (MBytes>0)
		mngPoolSize=MBytes*1024*1024;

	//tudo o resto a zero
	memset(&InfoData,0,sizeof(STATS));
	MemPools=nullptr;
	workFlags=flags;

	//formato o tamanho da pool
	strBytesSize.SetMemory(mngPoolSize);

	//falar ao mundo
	myLogIn(logID, "Memory manager inicialized:");
	myLogInTAB(logID, "statictics cleaned");
	myLogInTAB(logID, "memory pool size: %s", strBytesSize.GetData());
	myLogInTAB(logID, "align bytes: %d",mngAlign);
	if (workFlags & SMEM_MANAGER_KEEP_FIRST_POOL)		myLogInTAB(logID, "keeping first pool");
	else	myLogInTAB(logID, "destroy first pool");
	if (workFlags & SMEM_MANAGER_THREAD_UNPROTECTED)	myLogInTAB(logID, "thread unprotected");
	else	myLogInTAB(logID, "thread protected");
	myLogIn(logID," ");

	//se for para ter sempre um bloco, aloco um ja e fica cá sempre
	if (workFlags & SMEM_MANAGER_KEEP_FIRST_POOL)
		this->AllocDEBUG(1,"!block for preserving first pool!",0);
}

MemoryManager::~MemoryManager()
{
	SConsolePLog(logID,"#250,255,102. !! - !!");
	SConsolePLog(logID,"About to reset the memory manager!");

	//vou às pools
	while(MemPools!=nullptr)
		destroyPool(MemPools);
	MemPools=nullptr;

	//apagar a árvore e tudo o que tem lá dentro
	avlDelete(FreeNodes);
	FreeNodes=nullptr;

	//também agora já posso apagar os pools fixos para os nós AVL
	poolAVL.Clear();

	//e pronto
	SConsolePLog(logID,"    done.");
	SConsolePLog(logID,"#250,255,102. !! - !!");
}

int MemoryManager::Stat(const int flag)
{
	//se não é preciso estar protegido
	if (workFlags & SMEM_MANAGER_THREAD_UNPROTECTED){
		return procStat(flag);
		}

	//as coisas normais
	int resultado;

	//tenho de entrar na seccao
	EnterCriticalSection(&criticalSection);

	//fao o que tenho de fazer
	resultado=procStat(flag);

	//tenho de sair da seccao
	LeaveCriticalSection(&criticalSection);

	//devolvo a vontade
	return resultado;
}

void MemoryManager::Snapshot()
{
	//se não é preciso estar protegido
	if (workFlags & SMEM_MANAGER_THREAD_UNPROTECTED)
		{
		procSnapshot();
		return;
		}

	//tenho de entrar na seccao
	EnterCriticalSection(&criticalSection);

	//fao o que tenho de fazer
	procSnapshot();

	//tenho de sair da seccao
	LeaveCriticalSection(&criticalSection);
}

void* MemoryManager::Alloc(const int bytes)
{
	//contar uma chamada
	InfoData.numAllocCalls++;

	//se não é preciso estar protegido
	if (workFlags & SMEM_MANAGER_THREAD_UNPROTECTED){
		return procAlloc(bytes,"<sem informação de debug>",0);
		}

	//as coisas normais
	void* resultado;

	//tenho de entrar na seccao
	EnterCriticalSection(&criticalSection);

	//fao o que tenho de fazer
	resultado=procAlloc(bytes,"<sem informação de debug>",0);

	//tenho de sair da seccao
	LeaveCriticalSection(&criticalSection);
    
	//devolvo a vontade
	return resultado;
}

void* MemoryManager::AllocDEBUG(const int bytes, const char * const file, const int line)
{
	//contar uma chamada
	InfoData.numAllocDebugCalls++;

	//se não é preciso estar protegido
	if (workFlags & SMEM_MANAGER_THREAD_UNPROTECTED){
		return procAlloc(bytes,file,line);
		}

	//as coisas normais
	void* resultado;

	//tenho de entrar na seccao
	EnterCriticalSection(&criticalSection);

	//fao o que tenho de fazer
	resultado=procAlloc(bytes,file,line);

	//tenho de sair da seccao
	LeaveCriticalSection(&criticalSection);

	//devolvo a vontade
	return resultado;
}

void* MemoryManager::AllocUnmanaged(const int bytes)
{
	//não preciso de fazer nada, basta devolver isto
	return malloc(bytes);
}

void MemoryManager::Free(void * const pointer)
{
	//contar uma chamada
	InfoData.numFreeCalls++;

	//se não é preciso estar protegido
	if (workFlags & SMEM_MANAGER_THREAD_UNPROTECTED){
		procFree(pointer);
		return;
		}

	//tenho de entrar na seccao
	EnterCriticalSection(&criticalSection);

	//faco o que tenho de fazer
	procFree(pointer);

	//tenho de sair da seccao
	LeaveCriticalSection(&criticalSection);
}

void* MemoryManager::Realloc(const void * const pointer, const int bytes)
{
	//contar uma chamada
	InfoData.numReallocCalls++;

	//se não é preciso estar protegido
	if (workFlags & SMEM_MANAGER_THREAD_UNPROTECTED){
		return procRealloc(pointer,bytes,"<sem informação de debug>",0);
		}

	//as coisas normais
	void* resultado;

	//tenho de entrar na seccao
	EnterCriticalSection(&criticalSection);

	//fao o que tenho de fazer
	resultado=procRealloc(pointer,bytes,"<sem informação de debug>",0);

	//tenho de sair da seccao
	LeaveCriticalSection(&criticalSection);

	//devolvo a vontade
	return resultado;
}

void* MemoryManager::ReallocDEBUG(const void * const pointer, const int bytes, const char * const file, const int line)
{
	//contar uma chamada
	InfoData.numReallocDebugCalls++;

	//se não é preciso estar protegido
	if (workFlags & SMEM_MANAGER_THREAD_UNPROTECTED){
		return procRealloc(pointer,bytes,file,line);
		}

	//as coisas normais
	void* resultado;

	//tenho de entrar na seccao
	EnterCriticalSection(&criticalSection);

	//fao o que tenho de fazer
	resultado=procRealloc(pointer,bytes,file,line);

	//tenho de sair da seccao
	LeaveCriticalSection(&criticalSection);

	//devolvo a vontade
	return resultado;
}

}//namespace Memory
}//namespace HorseRadish