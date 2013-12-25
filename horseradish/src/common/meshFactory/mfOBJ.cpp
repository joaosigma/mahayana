#include "common\Platform.hpp"
#include "common\Types.hpp"
#include "common\Mesh.hpp"
#include "common\MeshFactory.hpp"
#include "common\Stream.hpp"

//os includes habituais
#include <windows.h>
#include <stdlib.h>
#include <stdio.h>
#include <math.h>

using namespace HorseRadish;

//***************************************
//*****	 PARA PASSAR DADOS DO OBJ	*****
//***************************************
struct OBJ{
	char nome[88];
	float *vertex,*tCoord;
	int numVertex,numtCoord,numNormals;
	int numAVertex,numAtCoord;

	struct FACE{
		int index[3],indexT[3];
		int group;
		}*faces;
	int numFaces;
	int numAFaces;
};

struct INDEX_SORT{
	unsigned int index,indexT,pos;
};

//***********************
//*****	DECLARAÇÕES	*****
//***********************
static bool readFaces(HorseRadish::Streams::StreamReader * const streamReader, OBJ * const object, const int * const accumNum);
static bool doMesh(OBJ * const obj, Geometry::Mesh *mesh);

//***********************
//*****	  LOCAIS	*****
//***********************
static
bool leLinha(HorseRadish::Streams::StreamReader * const streamReader, char * const buffer, const int bufferSize)
{
	int lerMesmo;
	HorseRadish::hInt8 curChar, lastChar;

	//verificar isto
	if (streamReader==nullptr || buffer==nullptr || bufferSize<=1)
		return false;

	//se o ficheiro estiver no fim
	if (streamReader->CanRead() == false)
		return false;
	
	//toca a ler a string
	buffer[0]=lastChar='\0';
	lerMesmo=0;
	while((lerMesmo<bufferSize) && (streamReader->CanRead() == true))
	{
		if (streamReader->ReadInt8(curChar) == false)
			break;

		//se for para sair
		if (curChar==10 || curChar==13)
			break;

		//a partir daqui não escrevo mais nada! Mas tenho de avançar até ao final da linha
		if (curChar=='#')
		{
			HorseRadish::hInt8 commentChar;

			while(streamReader->CanRead() == true)
			{
				if (streamReader->ReadInt8(commentChar) == false)
					break;
				if (commentChar==10 || commentChar==13)
					break;
			}
			break;
		}

		//se nao quiser copiar este caracteres
		if (curChar==9)
			continue;

		//para nao haver duplicação de espaços ou haver espaços no inicio
		if (curChar==' ' && (lastChar==' ' || lerMesmo==0) )
			continue;

		//leio este
		buffer[lerMesmo++]=lastChar=curChar;
	}

	//tranco a string
	buffer[lerMesmo]='\0';

	//se tiver vazio, volta a ler
	if (lerMesmo==0)
		return leLinha(streamReader,buffer,bufferSize);

	//correu tudo bem
	return true;
}

static
bool backupLinha(HorseRadish::Streams::StreamReader * const streamReader)
{
	char *walker;

	if (streamReader==nullptr)
		return false;

	streamReader->Seek(-2, HorseRadish::Streams::Stream::Current);

	while(streamReader->GetPosition() > 0)
	{
		HorseRadish::hInt8 charRead;

		if (streamReader->ReadInt8(charRead) == false)
			break;
		if ((charRead == 10) || (charRead == 13))
			break;
		streamReader->Seek(-2, HorseRadish::Streams::Stream::Current);
	}
		
	//já tá
	return true;
}

static
void qsortFacesPerGroup(OBJ::FACE * const arrayBase, const int left, const int right)
{
	int i,j;

	//localidade para x e y
	{
	OBJ::FACE x,y;

	memcpy(&x,&arrayBase[(left + right) / 2],sizeof(OBJ::FACE));
	i=left;
	j=right;
	do{
		while( (arrayBase[i].group > x.group)  && (i<right) )
			i++;
		while( (x.group> arrayBase[j].group)  && (j>left) )
			j--;
		if(i <= j)
			{
			memcpy(&y,&arrayBase[i],sizeof(OBJ::FACE));
			memcpy(&arrayBase[i],&arrayBase[j],sizeof(OBJ::FACE));
			memcpy(&arrayBase[j],&y,sizeof(OBJ::FACE));
			i++;
			j--;
			}
	} while(i <= j);

	}

	if(left < j)
		qsortFacesPerGroup(arrayBase,left,j);
	if(i < right)
		qsortFacesPerGroup(arrayBase,i,right);
}

static
bool readFacesAndDoMesh(HorseRadish::Streams::StreamReader * const streamReader, Geometry::Model * const modelo, OBJ * const obj, const int * const accumNum, const char * const objName)
{
	Geometry::Mesh *mesh;
	int meshIndex;
	
	//isto tem de estar bom
	if (modelo==nullptr || obj==nullptr || obj->vertex==nullptr || obj->numVertex<=0)
		return false;

	//não esquecer de ler os indices
	if (readFaces(streamReader,obj,accumNum)==false)
		return false;

	//aumentar o número de meshes do modelo
	meshIndex = modelo->arrayMesh.GetNumElements();
	modelo->arrayMesh.Add();
	modelo->arrayMesh[meshIndex].meshName[0]='\0';
	modelo->arrayMesh[meshIndex].materialName[0]='\0';
	mesh=&modelo->arrayMesh[meshIndex].mesh;

	//copiar o nome
	if (objName!=nullptr && objName[0]!='\0')
		strcpy(modelo->arrayMesh[meshIndex].meshName,objName);

	//limpar a mesh
	memset(mesh,0,sizeof(Geometry::Mesh));
	
	//faço a mesh e já tá
	doMesh(obj,mesh);

	//se não tenho normais
	if (mesh->FindAttribData(Geometry::Mesh::Normal) == nullptr)
		mesh->NewAttrib(Geometry::Mesh::Normal, mesh->GetNumElements());

	//só falta calcular as normais
	mesh->Ortho(MESH_ORTHO_CREATE_NORMALS);

	//crio coordenadas de textura caso nao as haja
	if (mesh->FindAttribData(Geometry::Mesh::TexCoords) == nullptr)
		mesh->NewAttrib(Geometry::Mesh::TexCoords, mesh->GetNumElements());

	//já está
	return true;
}

static
bool doMesh(OBJ * const obj, Geometry::Mesh *mesh)
{
	int i,j,stride,curVertex,numVertex,numNewIndices,numAVertex,numANewIndices,curIndex,curIndexT;
	float *meshData;
	OBJ::FACE *curFace,*lastFace;
	unsigned int *newIndices;
	INDEX_SORT *cacheIndex;
	const INDEX_SORT *findIndex;
	bool achou;
	Geometry::Mesh::Attribute *novosAttrib;

	//checkups
	if (obj==nullptr || mesh==nullptr || obj->numFaces<=0)
		return nullptr;

	//crio espaço para guardar indices que já escrevi (cache)
	cacheIndex=(INDEX_SORT*)malloc(sizeof(INDEX_SORT)*obj->numFaces*3);
	if (cacheIndex==nullptr)
		return false;

	//pra começar, toca a ordenar o raio das faces por smoothing group
	qsortFacesPerGroup(obj->faces,0,obj->numFaces-1);

	//calcular o stride
	stride=3;
	if (obj->numtCoord>0)
		stride+=2;

	//limpar isto
	curVertex=numVertex=numAVertex=numNewIndices=numANewIndices=0;
	meshData=nullptr;
	newIndices=nullptr;

	//agora para cada smoothing group, crio os triangulos
	for(curFace=lastFace=obj->faces; (lastFace-obj->faces)<obj->numFaces; lastFace=curFace)
	{
		//vou a cada face do smoothing group
		for(; (curFace-obj->faces)<obj->numFaces; curFace++)
		{
			//se já não estou no mesmo grupo, saio
			if (curFace->group!=lastFace->group)
				break;

			//posso verificar isto (degenerados)
			if (curFace->index[0]==curFace->index[1] || curFace->index[0]==curFace->index[2] || curFace->index[1]==curFace->index[2])
				continue;
			
			//tenho de fazer esta face
			if (numNewIndices>=numANewIndices)
			{
				newIndices=(unsigned int*)realloc(newIndices,sizeof(unsigned int)*(numNewIndices+24));
				numANewIndices+=24;
			}

			//para cada um dos três
			for(i=0; i<3; i++)
			{
				//se o indice é negativo, arranjo
				if (curFace->index[i] < 0)
					curFace->index[i] = obj->numVertex + curFace->index[i] + 1;
				if (curFace->indexT[i] < 0)
					curFace->indexT[i] = obj->numtCoord + curFace->indexT[i] + 1;

				//verificar se saem fora dos limites
				if (curFace->index[i]<0 || curFace->index[i]>=obj->numVertex)
					continue;

				//por defeito não achei nada
				achou=false;

				//os indices a usar
				curIndex=curFace->index[i]*3;
				if (obj->numtCoord > 0)
					curIndexT=curFace->indexT[i]*2;

				//verifico se o ponto já não existe (com ou sem as coordenadas de textura)
				if (obj->numtCoord<=0)
				{
					//vou procurar
					for(j=0,findIndex=cacheIndex; j<numVertex-curVertex; j++,findIndex++)
					{	
						//se for diferente, continua à procura
						if (findIndex->index!=curIndex)
							continue;

						//chegando aqui é igual, logo sai daqui
						achou=true;
						break;
					}
				}
				else
				{
					//vou procurar
					for(j=0,findIndex=cacheIndex; j<numVertex-curVertex; j++,findIndex++)
					{	
						//se for diferente, continua à procura
						if (findIndex->index!=curIndex || findIndex->indexT!=curIndexT)
							continue;

						//chegando aqui é igual, logo sai daqui
						achou=true;
						break;
					}
				}

				//se achou, porreiro, só preciso de usar o indice e mais nada
				if (achou==true)
				{
					newIndices[numNewIndices] = findIndex->pos;
					numNewIndices++;
					continue;
				}

				//chegando aqui, vai-se criar um novo vértice
				cacheIndex[numVertex-curVertex].index=curIndex;
				if (obj->numtCoord>0)
					cacheIndex[numVertex-curVertex].indexT=curIndexT;
				cacheIndex[numVertex-curVertex].pos=numVertex;

				//chegando aqui, tenho de criar mais um novo vertice
				//se não achou, tenho de criar o raio do indice
				if (numVertex>=numAVertex)
				{
					meshData=(float*)realloc(meshData,sizeof(float)*((numVertex+10)*stride+stride));
					numAVertex+=10;
				}

				//preencho o novo vertice com os dados
				meshData[numVertex*stride+0]=obj->vertex[curIndex+0];
				meshData[numVertex*stride+1]=obj->vertex[curIndex+1];
				meshData[numVertex*stride+2]=obj->vertex[curIndex+2];
				if (obj->numtCoord>0)
				{
					meshData[numVertex*stride+3]=obj->tCoord[curIndexT+0];
					meshData[numVertex*stride+4]=obj->tCoord[curIndexT+1];
				}

				//escrevo o novo indice e andor
				newIndices[numNewIndices++]=numVertex;
				numVertex++;
			}
		}

		//acabei um smoothing group! weeeeee
		curVertex=numVertex;
	}

	//arre, finalmente acabou!

	//apago a cache
	free(cacheIndex);

	//arranjar a mesh
	mesh->SetNewIndices(Geometry::Mesh::Int32, numNewIndices, newIndices);

	//separar isto por tipo de stride
	if (stride==3)
	{
		//preparo tudo
		novosAttrib = new Geometry::Mesh::Attribute();
		novosAttrib[0].attribData = meshData;
		novosAttrib[0].attribType = Geometry::Mesh::Pos;

		//ajusto os parametros na mesh
		mesh->SetVerticesData(1, numVertex, 0, novosAttrib);
		return true;
	}

	//preparo tudo
	novosAttrib = new Geometry::Mesh::Attribute[2];
	novosAttrib[0].attribData=meshData;
	novosAttrib[0].attribType=Geometry::Mesh::Pos;
	novosAttrib[1].attribData=meshData+3;
	novosAttrib[1].attribType=Geometry::Mesh::TexCoords;

	//ajusto os parametros na mesh
	mesh->SetVerticesData(2, numVertex, sizeof(float)*5, novosAttrib);
	return true;
}

static
void parseFacesTri(OBJ::FACE * const face, OBJ * const object, const char * const linha, const int * const accumNum)
{
	int trash[3];

	//tenho de ter em atenção o que tenho de ler (os tris em OBJ estão invertidos, logo troca a face na leitura)
	if (object->numVertex>0 && object->numtCoord>0 && object->numNormals>0)
	{
		sscanf(linha,"f %d/%d/%d %d/%d/%d %d/%d/%d",&face->index[0],&face->indexT[0],&trash[0],&face->index[1],&face->indexT[1],&trash[1],&face->index[2],&face->indexT[2],&trash[2]);
	}
	else if (object->numVertex>0 && object->numtCoord>0)
	{
		sscanf(linha,"f %d/%d %d/%d %d/%d",&face->index[0],&face->indexT[0],&face->index[1],&face->indexT[1],&face->index[2],&face->indexT[2]);
	}
	else if (object->numVertex>0 && object->numNormals>0)
	{
		sscanf(linha,"f %d/%d %d/%d %d/%d",&face->index[0],&trash[0],&face->index[1],&trash[1],&face->index[2],&trash[2]);
	}
	else
	{
		sscanf(linha,"f %d %d %d",&face->index[0],&face->index[1],&face->index[2]);
	}

	//próximo
	object->numFaces++;

	//e que também os indices devem ser decrementados
	face->index[0]-=accumNum[0];
	face->index[1]-=accumNum[0];
	face->index[2]-=accumNum[0];
	face->indexT[0]-=accumNum[1];
	face->indexT[1]-=accumNum[1];
	face->indexT[2]-=accumNum[1];
}

static
void parseFacesQuad(OBJ::FACE * const face, OBJ * const object, const char * const linha, const int * const accumNum)
{
	int trash[4],index[4],indexT[4];
	OBJ::FACE *faceN;

	//arranjar e limpar isto
	faceN=face+1;
	memset(index,0,sizeof(int)*4);
	memset(indexT,0,sizeof(int)*4);

	//tenho de ter em atenção o que tenho de ler (os tris em OBJ estão invertidos, logo troca a face na leitura)
	if (object->numVertex>0 && object->numtCoord>0 && object->numNormals>0)
	{
		sscanf(linha,"f %d/%d/%d %d/%d/%d %d/%d/%d %d/%d/%d",&index[0],&indexT[0],&trash[0],&index[1],&indexT[1],&trash[1],&index[2],&indexT[2],&trash[2],&index[3],&indexT[3],&trash[3]);
	}
	else if (object->numVertex>0 && object->numtCoord>0)
	{
		sscanf(linha,"f %d/%d %d/%d %d/%d %d/%d",&index[0],&indexT[0],&index[1],&indexT[1],&index[2],&indexT[2],&index[3],&indexT[3]);
	}
	else if (object->numVertex>0 && object->numNormals>0)
	{
		sscanf(linha,"f %d/%d %d/%d %d/%d %d/%d",&index[0],&trash[0],&index[1],&trash[1],&index[2],&trash[2],&index[3],&trash[3]);
	}
	else
	{
		sscanf(linha,"f %d %d %d %d",&index[0],&index[1],&index[2],&index[3]);
	}

	//escrevo a posição (e aproveito e decremento os indices)
	face->index[0]=index[2]-accumNum[0];
	face->index[1]=index[1]-accumNum[0];
	face->index[2]=index[0]-accumNum[0];
	faceN->index[0]=face->index[0];
	faceN->index[1]=face->index[2];
	faceN->index[2]=index[3]-accumNum[0];

	//escrevo as coordenadas se for caso disso (e aproveito e decremento os indices)
	if (object->numtCoord>0)
	{
		face->indexT[0]=indexT[2]-accumNum[1];
		face->indexT[1]=indexT[1]-accumNum[1];
		face->indexT[2]=indexT[0]-accumNum[1];
		faceN->indexT[0]=face->indexT[0];
		faceN->indexT[1]=face->indexT[2];
		faceN->indexT[2]=indexT[3]-accumNum[1];
	}

	//próximo
	object->numFaces+=2;
}

static
int contaEspacos(const char * const linha)
{
	int count;
	char ultimo;
	const char *walker;

	//avança até ao primeiro caracter
	count=0;
	for(walker=linha; *walker!='\0' && *walker==' '; walker++);
		
	//a partir daqui posso comecar a contar
	for(ultimo=0; *walker!='\0'; walker++)
	{
		//se for igual ao ultimos, dispenso
		if (*walker==ultimo)
			continue;
		
		//se for um espaco, counto
		if (*walker==' ')
			count++;
		ultimo=*walker;
	}

	//se cheguei ao fim e o ultimo era um espaço, tiro então esse espaço do fim
	if (ultimo==' ')
		count--;
	return count;
}

static
bool readFaces(HorseRadish::Streams::StreamReader * const streamReader, OBJ * const object, const int * const accumNum)
{
	int curGroup,espacos;
	char linha[512];
	bool sucesso;

	//verificar inicio
	if (streamReader==nullptr || object==nullptr)
		return false;

	//o grupo corrente
	curGroup=0;
	sucesso=true;

	//como espacos em branco
	linha[0]='\0';
	while(sucesso && (linha[0]==' ' || linha[0]=='\0' || (linha[0]!='f' && linha[0]!='s'))){
		sucesso=leLinha(streamReader,linha,512);
		}

	//enquanto ler tudo
	while(linha[0]=='f' || linha[0]=='s')
	{
		//se for um grupo, é simples, só tenho de mudar o grupo corrente
		if (linha[0]=='s')
		{
			//leio o grupo, avanço e continuo
			sscanf(linha,"s %d",&curGroup);
			sucesso=leLinha(streamReader,linha,512);
			continue;
		}
		
		//conto o numero de faces na linha
		espacos=contaEspacos(linha);

		//se não for nem triangulos, nem quadrados, segue para a próxima linha
		if (espacos!=3 && espacos!=4)
		{
			sucesso=leLinha(streamReader,linha,512);
			continue;
		}

		//se forem triangulos
		if (espacos==3)
		{
			//se tenho de criar espaco
			if (object->numFaces>=object->numAFaces)
			{
				object->faces=(OBJ::FACE*)realloc(object->faces,sizeof(OBJ::FACE)*(object->numAFaces+10));
				object->numAFaces+=10;
			}

			//o grupo ao qual pertence
			object->faces[object->numFaces].group=curGroup;

			//e leio o que tenho a ler
			parseFacesTri(object->faces+object->numFaces,object,linha,accumNum);
		}
		//então sao quadrados
		else
		{
			//se tenho de criar espaco
			if ((object->numFaces+1)>=object->numAFaces)
			{
				object->faces=(OBJ::FACE*)realloc(object->faces,sizeof(OBJ::FACE)*(object->numAFaces+10));
				object->numAFaces+=10;
			}

			//o grupo ao qual pertence
			object->faces[object->numFaces].group=curGroup;
			object->faces[object->numFaces+1].group=curGroup;

			//e leio o que tenho a ler
			parseFacesQuad(object->faces+object->numFaces,object,linha,accumNum);
		}

		//próxima linha
		sucesso=leLinha(streamReader,linha,512);
	}

	//já tá
	return true;
}

static
bool readVertices(HorseRadish::Streams::StreamReader * const streamReader, OBJ * const object, char *linha)
{
	//verificar inicio
	if (streamReader==nullptr || object==nullptr || linha==nullptr || linha[0]!='v')
		return false;

	//se tiver la alguma coisa, simplesmente apago
	if (object->vertex)
		free(object->vertex);
	object->vertex=nullptr;
	object->numVertex=0;
	object->numAVertex=0;

	//enquanto ler tudo
	while(linha[0]=='v' && linha[1]==' ')
	{
		//se tenho de criar espaco
		if (object->numVertex>=object->numAVertex)
		{
			object->vertex=(float*)realloc(object->vertex,sizeof(float)*3*(object->numAVertex+10));
			object->numAVertex+=10;
		}

		//e leio para
		sscanf(linha,"v %f %f %f",&object->vertex[object->numVertex*3+2],&object->vertex[object->numVertex*3+1],&object->vertex[object->numVertex*3+0]);
		object->vertex[object->numVertex*3+2]=-object->vertex[object->numVertex*3+2];
		object->numVertex++;

		//próxima linha
		leLinha(streamReader,linha,512);
	}

	//já tá
	return true;
}

static
bool readTexCoords(HorseRadish::Streams::StreamReader * const streamReader, OBJ * const object, char *linha)
{
	//verificar inicio
	if (streamReader==nullptr || object==nullptr || linha==nullptr || linha[0]!='v' || linha[1]!='t')
		return false;

	//se tiver la alguma coisa, simplesmente apago
	if (object->tCoord)
		free(object->tCoord);
	object->tCoord=nullptr;
	object->numtCoord=0;
	object->numAtCoord=0;

	//enquanto ler tudo
	while(linha[0]=='v' && linha[1]=='t')
	{
		//se tenho de criar espaco
		if (object->numtCoord>=object->numAtCoord)
		{
			object->tCoord=(float*)realloc(object->tCoord,sizeof(float)*2*(object->numAtCoord+10));
			object->numAtCoord+=10;
		}

		//e leio para
		sscanf(linha,"vt %f %f",&object->tCoord[object->numtCoord*2+0],&object->tCoord[object->numtCoord*2+1]);
				
		//próximo
		object->numtCoord++;

		//próxima linha
		leLinha(streamReader,linha,512);
	}

	//já tá
	return true;
}

static
bool readNormals(HorseRadish::Streams::StreamReader * const streamReader, OBJ * const object, char *linha)
{
	//verificar inicio
	if (streamReader==nullptr || object==nullptr || linha==nullptr || linha[0]!='v' || linha[1]!='n')
		return false;

	//enquanto ler tudo, marco só o número de normais que já li
	while(linha[0]=='v' && linha[1]=='n')
	{
		object->numNormals++;
		leLinha(streamReader,linha,512);
	}

	//já tá
	return true;
}

Geometry::Model* mfReadOBJ(HorseRadish::Streams::StreamReader * const streamReader)
{
	Geometry::Model *modeloReturn;

	//marotos
	if (streamReader==nullptr)
		return nullptr;

	//inicio modelo
	modeloReturn = new Geometry::Model();
	if (modeloReturn == nullptr)
		return nullptr;

	int accumNum[3];
	bool sucesso;
	OBJ object;
	char linha[512],nomeObj[66];

	//enquanto nao sair daqui
	sucesso=true;
	accumNum[0]=accumNum[1]=accumNum[2]=1;
	while(sucesso)
	{
		//limpo este objecto para comecar a ler as coisas.
		memset(&object,0,sizeof(OBJ));

		//leio a linha e vejo o que existe
		sucesso=leLinha(streamReader,linha,512);

		//enquanto nao arrebentar
		nomeObj[0]='\0';
		while(sucesso)
		{
			//para sair
			if (linha[0]==' ' || linha[0]=='\0' || linha[0]=='f' || linha[0]=='s')
				break;

			//como espacos em branco
			while(sucesso && (linha[0]==' ' || linha[0]=='\0'))
				sucesso=leLinha(streamReader,linha,512);

			//se for um 'g', posso ler o nome do modelo
			if (linha[0]=='g')
			{
				strcpy(nomeObj,linha+2);
				sucesso=leLinha(streamReader,linha,512);
				continue;
			}

			//vejo o que devo ler (se não sei o que é, passo à frente)
			if (linha[0]=='v' && linha[1]==' ')				sucesso=readVertices(streamReader,&object,linha);
			else if (linha[0]=='v' && linha[1]=='t')		sucesso=readTexCoords(streamReader,&object,linha);
			else if (linha[0]=='v' && linha[1]=='n')		sucesso=readNormals(streamReader,&object,linha);
			else sucesso=leLinha(streamReader,linha,512);
		}

		//se deu barraca
		if (sucesso==false)
			break;

		//chego atrás e agora leio o raio das faces criando o modelo
		backupLinha(streamReader);
		sucesso=readFacesAndDoMesh(streamReader,modeloReturn,&object,accumNum,nomeObj);

		//acumular estes valores
		accumNum[0]+=object.numVertex;
		accumNum[1]+=object.numtCoord;
		accumNum[2]+=object.numNormals;

		//apago e limpo tudo
		if (object.vertex)		free(object.vertex);
		if (object.tCoord)		free(object.tCoord);
	}

	//wwwweeeeeeeee :)
	return modeloReturn;
}