#include "common\Platform.hpp"
#include "common\Mesh.hpp"
#include "common\MeshFactory.hpp"

#include <windows.h>

using namespace HorseRadish;

/*§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§
§§§§§§   -= Estructuras e declarações necessárias =-   §§§§§
§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§*/

#define LWO2_FORM 'MROF'
#define LWO2_LWO2 '2OWL'
#define LWO2_TAGS 'SGAT'
#define LWO2_LAYR 'RYAL'
#define LWO2_PNTS 'STNP'
#define	LWO2_POLS 'SLOP'
#define	LWO2_FACE 'ECAF'
#define	LWO2_VMAP 'PAMV'
#define	LWO2_VMAD 'DAMV'
#define	LWO2_TXUV 'VUXT'
#define	LWO2_SURF 'FRUS'
#define	LWO2_SLST 'SFRS'
#define LWO2_WGHT 'THGW'
#define LWO2_PTAG 'GATP'
#define LWO2_TMAP 'PAMT'

//estructura auxiliar para guardar os meus dados
struct LWO_TEXCOORD_MAD{
	int vert,poly;
	float u,v;
};
struct LWO_POSITION{
	float x,y,z,u,v;
	bool texSet;
};
struct LWO_LISTAGEOM{
	char nome[256];

	LWO_POSITION *listPosition;
	LWO_TEXCOORD_MAD *listTexCoordMAD;
	int numPosition,numTexCoordMAD,numIndices;
	unsigned int *Indices,*tagIndex;
	
	struct LWO_LISTAGEOM *prox;
};
struct LWO_LISTAMATERIAL{
	char nome[256];
	struct LWO_LISTAMATERIAL *prox;
};
struct LWO_TAGSNAMES{
	char nome[256];
	LWO_LISTAMATERIAL *material;
	struct LWO_TAGSNAMES *prox;
};

//a lista de meshes, materiais e tagsNames
static LWO_LISTAGEOM *ListaMeshes;
static LWO_LISTAMATERIAL *ListaMateriais;
static LWO_TAGSNAMES *ListaTagsNames;

static
struct LWO_TAGSNAMES * criaTagsNamesList(const char * const nome)
{
	LWO_TAGSNAMES *walker;

	if (ListaTagsNames==nullptr)
	{
		ListaTagsNames=new LWO_TAGSNAMES;
		if (ListaTagsNames==nullptr)
			return nullptr;
		ListaTagsNames->nome[0]='\0';
		ListaTagsNames->material=nullptr;
		if (nome)
			strcpy(ListaTagsNames->nome,nome);
		ListaTagsNames->prox=nullptr;
		return ListaTagsNames;
	}

	for(walker=ListaTagsNames; walker->prox!=nullptr; walker=walker->prox);

	walker->prox=new LWO_TAGSNAMES;
	if (walker->prox==nullptr)
		return nullptr;
	walker->prox->nome[0]='\0';
	walker->prox->material=nullptr;
	if (nome)
		strcpy(walker->prox->nome,nome);
	walker->prox->prox=nullptr;
	return walker->prox;
}

static
struct LWO_LISTAGEOM * criaMeshList(const char * const nome)
{
	LWO_LISTAGEOM *walker;

	if (ListaMeshes==nullptr)
	{
		ListaMeshes=new LWO_LISTAGEOM;
		if (ListaMeshes==nullptr)
			return nullptr;
		memset(ListaMeshes,0,sizeof(LWO_LISTAGEOM));
		if (nome)
			strcpy(ListaMeshes->nome,nome);
		ListaMeshes->prox=nullptr;
		return ListaMeshes;
	}

	for(walker=ListaMeshes; walker->prox!=nullptr; walker=walker->prox);

	walker->prox=new LWO_LISTAGEOM;
	if (walker->prox==nullptr)
		return nullptr;
	memset(walker->prox,0,sizeof(LWO_LISTAGEOM));
	if (nome)
		strcpy(walker->prox->nome,nome);
	walker->prox->prox=nullptr;
	return walker->prox;
}

static
struct LWO_LISTAMATERIAL * criaMaterialList(const char * const nome)
{
	LWO_LISTAMATERIAL *walker;

	if (ListaMateriais==nullptr)
	{
		ListaMateriais=new LWO_LISTAMATERIAL;
		if (ListaMateriais==nullptr)
			return nullptr;
		ListaMateriais->nome[0]='\0';
		if (nome)
			strcpy(ListaMateriais->nome,nome);
		ListaMateriais->prox=nullptr;
		return ListaMateriais;
	}

	for(walker=ListaMateriais; walker->prox!=nullptr; walker=walker->prox);

	walker->prox=new LWO_LISTAMATERIAL;
	if (walker->prox==nullptr)
		return nullptr;
	walker->prox->nome[0]='\0';
	if (nome)
		strcpy(walker->prox->nome,nome);
	walker->prox->prox=nullptr;
	return walker->prox;
}

static
struct LWO_LISTAGEOM * currentModel()
{
	LWO_LISTAGEOM *walker;

	if (ListaMeshes==nullptr)
		return nullptr;

	for(walker=ListaMeshes; walker->prox!=nullptr; walker=walker->prox);

	return walker;
}

static
void destroiMesh(void)
{
	LWO_LISTAGEOM *walker,*lixo;
	LWO_LISTAMATERIAL *walkerM,*lixoM;
	LWO_TAGSNAMES *walkerT,*lixoT;

	walkerT=ListaTagsNames;
	while(walkerT!=nullptr)
	{
		lixoT=walkerT;
		walkerT=walkerT->prox;
		delete lixoT;
	}

	walker=ListaMeshes;
	while(walker!=nullptr)
	{
		lixo=walker;
		walker=walker->prox;
		if (lixo->listPosition)
			delete [] lixo->listPosition;
		if (lixo->listTexCoordMAD)
			delete [] lixo->listTexCoordMAD;
		if (lixo->Indices)
			delete [] lixo->Indices;
		if (lixo->tagIndex)
			delete [] lixo->tagIndex;
		delete lixo;
	}

	walkerM=ListaMateriais;
	while(walkerM!=nullptr)
	{
		lixoM=walkerM;
		walkerM=walkerM->prox;
		delete lixoM;
	}
}

/*§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§
§§§§§§   -= Funções para gerir tudo =-   §§§§§
§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§*/
static
float toFloat(const char * const input)
{
	union{
		float temp;
		unsigned char byte_4[4];
	}swp;

	swp.byte_4[0] = input[3];
	swp.byte_4[1] = input[2];
	swp.byte_4[2] = input[1];
	swp.byte_4[3] = input[0];

	return swp.temp;
}

static
unsigned int toUInt(const char * const input)
{
	union{
		unsigned int temp;
		unsigned char byte_4[4];
	}swp;

	swp.byte_4[0] = input[3];
	swp.byte_4[1] = input[2];
	swp.byte_4[2] = input[1];
	swp.byte_4[3] = input[0];

	return swp.temp;
}

static
unsigned short toUShort(const char * const input)
{
	union{
		unsigned short temp;
		unsigned char byte_2[2];
	}swp;

	swp.byte_2[0] = input[1];
	swp.byte_2[1] = input[0];

	return swp.temp;
}

static
int getLWOChunk(const char * const buffer)
{
	int chunkWalker,i,j,chunkid,chunklength,aux,numFacesTri,numFacesQuad;
	LWO_LISTAMATERIAL *novoMaterial;
	LWO_LISTAGEOM *modelo;
	char str[8];

	//qual o id do chunck
	chunkid = *((int*)(buffer));

	//inverto estes 4 bytes
	chunklength=toUInt(buffer+4);

	//começo com 8
    chunkWalker=8;

	//trato cada chuck
    switch (chunkid){

		//para ver se é o tipo de ficheiro correcto
		case LWO2_FORM:
				
				//leio os primeiros 4 caracteres
				str[0]=buffer[chunkWalker+0];
				str[1]=buffer[chunkWalker+1];
				str[2]=buffer[chunkWalker+2];
				str[3]=buffer[chunkWalker+3];
				str[4]='\0';
				chunkWalker+=4;
				
				//se a string nao for esta
				if (strcmp(str,"LWO2")!=0) 
					chunkWalker=chunklength;
				break;

		//ler as tags
		case LWO2_TAGS:

				//leio
				while(chunkWalker<chunklength)
					{
					criaTagsNamesList(buffer+chunkWalker);
					for(j=0; buffer[chunkWalker]!='\0'; j++)
						chunkWalker++;
					chunkWalker++;
					if (j%2==0)
						chunkWalker++;
					}
				
				//avanço com o chunck
				chunkWalker=chunklength;

				break;

		//ver todos os pontos
		case LWO2_PNTS:

				//crio mais uma entrada
				modelo=criaMeshList(nullptr);
				if (modelo==nullptr)
					break;

				//crio espaço para a posicao dos vertices
				modelo->numPosition=chunklength/12;
				modelo->listPosition=new LWO_POSITION[modelo->numPosition];
				if (modelo->listPosition==nullptr)
					break;

				//leio os dados
				for (i=0; i<modelo->numPosition; i++)
					{
					//leio os dados
					modelo->listPosition[i].x=toFloat(buffer+chunkWalker+0);
					modelo->listPosition[i].y=toFloat(buffer+chunkWalker+4);
					modelo->listPosition[i].z=toFloat(buffer+chunkWalker+8);

					//coloco isto a zero
					modelo->listPosition[i].u=0.0f;
					modelo->listPosition[i].v=0.0f;
					modelo->listPosition[i].texSet=false;

					//o z neste tipo de ficheiros está invertido
					modelo->listPosition[i].z=-modelo->listPosition[i].z;

					//avanço com ponteiros
					chunkWalker+=12;
					}		
				break;

		//ver a lista de nomes das superficies
		case LWO2_SURF:

				//crio o novo material(=surface) e passo logo o nome dele
				novoMaterial=criaMaterialList(buffer+chunkWalker);

				//avanço por todas as superficies
				/*while(chunkWalker<chunklength)
					{
					

					//o tamanho do chunk
					aux=toUInt(buffer+chunkWalker-4);

					//avança
					chunkWalker+=aux+4;
					}*/
				
				//avanço com o chunck
				chunkWalker=chunklength;

				break;
				
			
		//ler todos os poligonos
		case LWO2_POLS:

				//só quero do tipo faces
				if (*((long*)(buffer+chunkWalker))==LWO2_FACE)
					{
					//vou buscar a mesh com que estou a trabalhar
					modelo=currentModel();
					if (modelo==nullptr)
						break;

					//começo a contagem a zero
					modelo->numIndices=0;
					numFacesTri=numFacesQuad=0;

					//avanço com o ponteiro
					chunkWalker+=4;

					//começo a contar o numero de indices
					for(i=chunkWalker; i<=chunklength;)
						{
						//leio o numero de indices e avanço com o indice
						aux=toUShort(buffer+i);
						i+=2+aux*2;

						//vejo o que para aqui tenho e faço contas
						if (aux==3)
							numFacesTri++;
						else if (aux==4)
							numFacesQuad++;
						}

					//calculo o numero de indices e se mesmo assim dá barraca, saio
					modelo->numIndices=numFacesTri*3 + numFacesQuad*2*3;
					if (modelo->numIndices<=0)
						{
						modelo->numIndices=0;
						chunkWalker=chunklength;
						break;
						}

					//crio espaço para os indices
					modelo->Indices=new unsigned int[modelo->numIndices];
					if (modelo->Indices==nullptr)
						{
						modelo->numIndices=0;
						chunkWalker=chunklength;
						break;
						}

					//leio os indices (os LWO têm as faces trocadas)
					for(j=0,i=chunkWalker; i<=chunklength;)
						{
						//leio o numero de indices desta face e passo à frente
						aux=toUShort(buffer+i);
						i+=2;

						//se não for um tri nem um quad, nao sei ler (vai para a proxima face)
						if (aux!=3 && aux!=4)
							continue;

						//se tiver de ler quadrados
						if (aux==4)
							{
							modelo->Indices[j+0]=toUShort(buffer+i+0);
							modelo->Indices[j+1]=toUShort(buffer+i+2);
							modelo->Indices[j+2]=toUShort(buffer+i+4);

							modelo->Indices[j+3]=modelo->Indices[j+0];
							modelo->Indices[j+4]=modelo->Indices[j+2];
							modelo->Indices[j+5]=toUShort(buffer+i+6);

							i+=8;
							j+=6;
							continue;
							}

						//leio triangulos
						modelo->Indices[j+2]=toUShort(buffer+i+0);
						modelo->Indices[j+1]=toUShort(buffer+i+2);
						modelo->Indices[j+0]=toUShort(buffer+i+4);

						i+=6;
						j+=3;
						}

					//avanço com o ponteiro do chunk
					chunkWalker=chunklength;
					}

				break;

		//ler quais os polygonos e para onde vão
		case LWO2_PTAG:

				//tenho de ter de surperficie
				if (*((long*)(buffer+chunkWalker))==LWO2_SURF)
					{
					//o modelo
					modelo=currentModel();
					if (modelo==nullptr)
						{
						chunkWalker=chunklength;
						break;
						}

					//verificar isto
					if ( ((chunklength-4)/4)!=(modelo->numIndices/3) )
						{
						chunkWalker=chunklength;
						break;
						}

					//crio espaco
					modelo->tagIndex=new unsigned int[modelo->numIndices/3];
					if (modelo->tagIndex==nullptr)
						{
						chunkWalker=chunklength;
						break;
						}

					//até ler tudo
					for(j=0,i=4; i<chunklength; j++)
						{
						//leio os tags
						aux=toUShort(buffer+chunkWalker+i);
						i+=2;

						modelo->tagIndex[aux]=toUShort(buffer+chunkWalker+i);
						i+=2;
						}
					}


				//avanço com o chunck
				chunkWalker=chunklength;

				//próximo
				break;

		//ler coordenadas de textura
		case LWO2_VMAP:

				//ler só do tipo TXUV
				if (*((long*)(buffer+chunkWalker))==LWO2_TXUV)
					{
					//vou buscar a mesh actual
					modelo=currentModel();
					if (modelo==nullptr)
						break;

					//começo a ler, e aqui passo à frente do lixo
					for(j=0,i=6; buffer[chunkWalker+i]!=0; i++,j++);
					i++;
					if (j%2==0)
						i++;

					//posso ler para dentro do vertice em questao, em vez de guardar num dado sitio
					for(j=0; i<chunklength; j++)
						{
						//qual o indice
						aux=toUShort(buffer+chunkWalker+i);
						i+=2;

						//vertice
						modelo->listPosition[aux].u=toFloat(buffer+chunkWalker+i+0);
						modelo->listPosition[aux].v=toFloat(buffer+chunkWalker+i+4);
						modelo->listPosition[aux].texSet=false;
						i+=8;
						}
					}

				chunkWalker=chunklength;
				break;

		case LWO2_VMAD:

				//ler só do tipo TXUV
				if (*((long*)(buffer+chunkWalker))==LWO2_TXUV)
					{
					//vou buscar a mesh actual
					modelo=currentModel();
					if (modelo==nullptr)
						break;

					//começo a ler, e aqui passo à frente do lixo
					for(j=0,i=6; buffer[chunkWalker+i]!=0; i++,j++);
					i++;
					if (j%2==0)
						i++;

					//crio espaço para albergar o que vou ler
					modelo->numTexCoordMAD=(chunklength-i)/12;
					modelo->listTexCoordMAD=new LWO_TEXCOORD_MAD[modelo->numTexCoordMAD];
					if (modelo->listTexCoordMAD==nullptr)
						break;

					//leio tudo
					for(j=0; i<chunklength; j++)
						{
						//indices
						modelo->listTexCoordMAD[j].vert=toUShort(buffer+chunkWalker+i);
						modelo->listTexCoordMAD[j].poly=toUShort(buffer+chunkWalker+i+2);
						i+=4;
						
						//valores propriamente ditos
						modelo->listTexCoordMAD[j].u=toFloat(buffer+chunkWalker+i);
						modelo->listTexCoordMAD[j].v=toFloat(buffer+chunkWalker+i+4);
						i+=8;
						}
					}

				chunkWalker=chunklength;
				break;

		default:
			//ignorar todos os outros chunks
			chunkWalker=chunklength;
			break;
		}

	//avançar para o fim do chunk recursivamente
    while(chunkWalker<chunklength)
		chunkWalker+=getLWOChunk(buffer+chunkWalker)+8;

    return chunklength;
}

static
bool copiaMesh(const LWO_LISTAGEOM * const meshFrom, LWO_LISTAGEOM * const meshTo)
{
	if (meshFrom==nullptr || meshTo==nullptr || meshFrom==meshTo)
		return false;

	//criar as coisas
	meshTo->numIndices=meshFrom->numIndices;
	meshTo->numPosition=meshFrom->numPosition;
	meshTo->Indices=new unsigned int[meshTo->numIndices];
	meshTo->tagIndex=new unsigned int[meshTo->numIndices/3];
	meshTo->listPosition=new LWO_POSITION[meshTo->numPosition];
	if (meshTo->Indices==nullptr || meshTo->tagIndex==nullptr || meshTo->listPosition==nullptr)
	{
		if (meshTo->Indices)	delete [] meshTo->Indices;
		if (meshTo->tagIndex)	delete [] meshTo->tagIndex;
		if (meshTo->listPosition)	delete [] meshTo->listPosition;
		return false;
	}

	//copiar as coisas
	memcpy(meshTo->Indices,meshFrom->Indices,sizeof(unsigned int)*meshTo->numIndices);
	memcpy(meshTo->tagIndex,meshFrom->tagIndex,sizeof(unsigned int)*(meshTo->numIndices/3));
	memcpy(meshTo->listPosition,meshFrom->listPosition,sizeof(LWO_POSITION)*meshTo->numPosition);
	
	//já está
	return true;
}

static
void cleanThisTri(LWO_LISTAGEOM * const mesh, const int indexTri)
{
	if (mesh==nullptr || indexTri<0 || (indexTri>=mesh->numIndices/3) )
		return;

	mesh->tagIndex[indexTri]=-1;
	mesh->Indices[indexTri*3+0]=-1;
	mesh->Indices[indexTri*3+1]=-1;
	mesh->Indices[indexTri*3+2]=-1;
}

static
void cleanUpDeletedTris(LWO_LISTAGEOM * const mesh)
{
	int i,numTri;

	if (mesh==nullptr)
		return;

	numTri=mesh->numIndices/3;
	for(i=0; i<numTri; i++)
	{
		if (!(mesh->tagIndex[i]==-1 || mesh->Indices[i*3+0]==-1))
			continue;

		if (i==(numTri-1))
		{
			mesh->numIndices-=3;
			break;
		}

		mesh->tagIndex[i]=mesh->tagIndex[numTri-1];
		mesh->Indices[i*3+0]=mesh->Indices[mesh->numIndices-3];
		mesh->Indices[i*3+1]=mesh->Indices[mesh->numIndices-2];
		mesh->Indices[i*3+2]=mesh->Indices[mesh->numIndices-1];
		mesh->numIndices-=3;
		numTri--;
		i--;
	}
}

static
void prepareSurfacesMaterial()
{
	int *tagHitCount,numTags,i,contaUsados,usedTagIndex;
	LWO_TAGSNAMES *walkerTags;
	LWO_LISTAGEOM *meshWalker,*meshNova;

	//quantos tags tenho ao todo
	for(numTags=0, walkerTags=ListaTagsNames; walkerTags!=nullptr; walkerTags=walkerTags->prox,numTags++);

	//se nao tenho tags
	if (numTags==0)
		return;

	//crio espaço
	tagHitCount=new int[numTags];
	if (tagHitCount==nullptr)
		return;

	//para cada mesh
	for(meshWalker=ListaMeshes; meshWalker!=nullptr; meshWalker=meshWalker->prox)
	{
		//se nao tenho tagIndex, passo
		if (meshWalker->tagIndex==nullptr)
			continue;

		//pra cada indice, toca a fazer a contagem
		memset(tagHitCount, 0, sizeof(int)*numTags);
		for(i=0; i<meshWalker->numIndices/3; i++)
			tagHitCount[meshWalker->tagIndex[i]]++;

		//vejo quantos uso
		for(contaUsados=i=0; i<numTags; i++)
		{
			if (tagHitCount[i]!=0)
				contaUsados++;
		}

		//se só uso este, fixe
		if (contaUsados==1)
			continue;

		//chegando aqui, nesta mesh existe pelo menos um poligono com uma superficie diferente dos outros todos
		//para contornar este pequeno obstáculo, vamos criar uma nova mesh
		meshNova=criaMeshList(nullptr);
		if (meshNova==nullptr)
			continue;

		//copio todos os elementos
		copiaMesh(meshWalker,meshNova);

		//para todos os elementos da mesh, todo aquele que for diferente do anterior, passa para o novo
		//e todo o que fica nesta mesh sai do novo
		usedTagIndex=meshWalker->tagIndex[0];
		for(i=0; i<meshWalker->numIndices/3; i++)
		{
			//se for igual, sai do novo, porque fica neste
			if (meshWalker->tagIndex[i]==usedTagIndex)
			{
				cleanThisTri(meshNova,i);
				continue;
			}

			//se nao, tenho de retirar do actual e fica no novo
			cleanThisTri(meshWalker,i);
		}

		//pronto, basta somente arranjar as coisas de maneira que os triangulos retirados saiam mesmo de la
		cleanUpDeletedTris(meshWalker);
		cleanUpDeletedTris(meshNova);
	}

	//isto vai à vida e já está
	delete [] tagHitCount;
}

static
void prepareTexVMAD()
{
	LWO_LISTAGEOM *meshWalker;
	int i,vert,poly;

	//pra cada mesh
	for(meshWalker=ListaMeshes; meshWalker!=nullptr; meshWalker=meshWalker->prox)
	{
		//se nao tenho nada
		if (meshWalker->numTexCoordMAD<=0 || meshWalker->listTexCoordMAD==nullptr)
			continue;

		//por cada VMAD
		for(i=0; i<meshWalker->numTexCoordMAD; i++)
		{
			//se o valor já foi colocado, então, posso lá por e não fazer mais nada
			vert=meshWalker->listTexCoordMAD[i].vert;
			poly=meshWalker->listTexCoordMAD[i].poly;
	
			//se ainda nao foi utilizado
			if (meshWalker->listPosition[vert].texSet==false)
			{
				meshWalker->listPosition[vert].u=meshWalker->listTexCoordMAD[i].u;
				meshWalker->listPosition[vert].v=meshWalker->listTexCoordMAD[i].v;
				meshWalker->listPosition[vert].texSet=true;
				continue;
			}

			//se o valor for o mesmo!! é pra cagar nele
			if ( (meshWalker->listPosition[vert].u==meshWalker->listTexCoordMAD[i].u) && (meshWalker->listPosition[vert].v==meshWalker->listTexCoordMAD[i].v) )
				continue;

			//chegando aqui ele já existia, logo vou ter de duplicar o poligono onde ele estava
			//o que significa mais um vertice, e arranjar o indice correspondente

			//crio o novo vertice
			meshWalker->listPosition=(LWO_POSITION*)realloc(meshWalker->listPosition,sizeof(LWO_POSITION)*(meshWalker->numPosition+1));
			meshWalker->numPosition++;

			//o novo vertice fica com a posicao do antigo e a coordenada do anterior
			meshWalker->listPosition[meshWalker->numPosition-1].x=meshWalker->listPosition[vert].x;
			meshWalker->listPosition[meshWalker->numPosition-1].y=meshWalker->listPosition[vert].y;
			meshWalker->listPosition[meshWalker->numPosition-1].z=meshWalker->listPosition[vert].z;
			meshWalker->listPosition[meshWalker->numPosition-1].u=meshWalker->listTexCoordMAD[i].u;
			meshWalker->listPosition[meshWalker->numPosition-1].v=meshWalker->listTexCoordMAD[i].v;
			meshWalker->listPosition[meshWalker->numPosition-1].texSet=true;

			//e agora os novos indices apontam para os dois antigos e para o novo criado e já tá
			if (meshWalker->Indices[poly*3+0]==vert)	meshWalker->Indices[poly*3+0]=meshWalker->numPosition-1;
			if (meshWalker->Indices[poly*3+1]==vert)	meshWalker->Indices[poly*3+1]=meshWalker->numPosition-1;
			if (meshWalker->Indices[poly*3+2]==vert)	meshWalker->Indices[poly*3+2]=meshWalker->numPosition-1;
		}

		//posso apagar
		delete [] meshWalker->listTexCoordMAD;
		meshWalker->listTexCoordMAD=nullptr;
		meshWalker->numTexCoordMAD=0;
	}
}


static
Geometry::Model *criaModelo()
{
	LWO_LISTAMATERIAL *materialWalker;
	LWO_LISTAGEOM *meshWalker;
	float bufferData[4];
	int numMeshes,i;
	Geometry::Model *modelo;

	//conto o numero de meshes
	for(numMeshes=0,meshWalker=ListaMeshes; meshWalker!=nullptr; meshWalker=meshWalker->prox,numMeshes++);

	//se nada houver
	if (numMeshes==0)
		return nullptr;

	//crio espaço para albergar tudo
	modelo = new Geometry::Model();

	//copio tudo em relação às meshes
	for(numMeshes=0,meshWalker=ListaMeshes; meshWalker!=nullptr; meshWalker=meshWalker->prox,numMeshes++)
	{
		//crio uma nova mesh
		modelo->arrayMesh.Add();

		//de momento nada de materiais e posso já copiar o nome
		strcpy(modelo->arrayMesh[numMeshes].meshName,meshWalker->nome);
			
		//crio espaço para os vertices e respectivas coordenadas
		modelo->arrayMesh[numMeshes].mesh.NewAttrib(Geometry::Mesh::Pos, meshWalker->numPosition);
		modelo->arrayMesh[numMeshes].mesh.NewAttrib(Geometry::Mesh::TexCoords,meshWalker->numPosition);

		//os indices que é simples porque uso os que já criei (troco simplesmente de ponteiro)
		modelo->arrayMesh[numMeshes].mesh.SetNewIndices(Geometry::Mesh::Int32, meshWalker->numIndices, meshWalker->Indices);
		meshWalker->Indices=nullptr;
		meshWalker->numIndices=0;

		//passo os dados para lá
		for(i=0; i<meshWalker->numPosition; i++)
		{
			//tirar a posicao e depois escrever
			bufferData[0]=meshWalker->listPosition[i].x;
			bufferData[1]=meshWalker->listPosition[i].y;
			bufferData[2]=meshWalker->listPosition[i].z;
			modelo->arrayMesh[numMeshes].mesh.SetData(Geometry::Mesh::Pos, i, bufferData);

			//tirar a coordenada de textura e depois escrever
			bufferData[0]=meshWalker->listPosition[i].u;
			bufferData[1]=1.0f-meshWalker->listPosition[i].v;
			modelo->arrayMesh[numMeshes].mesh.SetData(Geometry::Mesh::TexCoords, i, bufferData);
		}

		//normais e já tá
		modelo->arrayMesh[numMeshes].mesh.Ortho(MESH_ORTHO_CREATE_NORMALS);	
	}

	//BUGGGGYYYYY
	for(numMeshes=0,meshWalker=ListaMeshes; meshWalker!=nullptr; meshWalker=meshWalker->prox,numMeshes++)
	{
		if (meshWalker->tagIndex==nullptr)
			continue;

		LWO_TAGSNAMES *walkerTags;
		for(i=0, walkerTags=ListaTagsNames; walkerTags!=nullptr && i<meshWalker->tagIndex[0]; walkerTags=walkerTags->prox,i++);

		if (walkerTags==nullptr || walkerTags->material==nullptr)
			continue;

		//basta copiar o nome
		strcpy(modelo->arrayMesh[numMeshes].materialName, walkerTags->material->nome);
	}

	//devolvo o resultado
	return modelo;
}

/*§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§
§§§§§§   -= Função para exportar =-   §§§§§
§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§*/
Geometry::Model* SModelReadMemLWO(const void *file, const unsigned int fileSize)
{
	Geometry::Model *modelo;
	LWO_TAGSNAMES *walkerTags;
	LWO_LISTAMATERIAL *walkerSurfs;

	//verificar parametros
	if (file==nullptr || fileSize==0)
		return nullptr;

	//iniciar isto a limpo
	ListaMeshes=nullptr;
	ListaMateriais=nullptr;
	ListaTagsNames=nullptr;

	//começo a ler os chunck recursivamente
	getLWOChunk((const char*)file);

	//por os tags com as respectivas superficies
	for(walkerTags=ListaTagsNames; walkerTags!=nullptr; walkerTags=walkerTags->prox)
	{
		walkerTags->material=nullptr;
		for(walkerSurfs=ListaMateriais; walkerSurfs!=nullptr; walkerSurfs=walkerSurfs->prox)
		{
			if (strcmp(walkerTags->nome,walkerSurfs->nome))
				continue;
			walkerTags->material=walkerSurfs;
			break;
		}
	}

	//crio espaço para o modelo
	modelo = criaModelo();

	//apago tudo
	destroiMesh();
	ListaMeshes=nullptr;
	ListaMateriais=nullptr;

	//já tá porra
	return modelo;
}