#include "common\Mesh.hpp"
#include "common\MeshFactory.hpp"
#include "common\Stream.hpp"

#include <windows.h>
#include <stdlib.h>

using namespace HorseRadish;

struct ASE_TVERTEX{
	float u,v;
};
struct ASE_PVERTEX{
	float x,y,z,u,v;
	bool texSet;
};
struct ASE_FACE{
	int a,b,c,tA,tB,tC;
};

static
Geometry::Model::MeshData* criaMesh(Geometry::Model * const modelo)
{
	Geometry::Model::MeshData *novo;

	if (modelo==nullptr)
		return nullptr;

	//crio uma nova mesh
	novo = modelo->arrayMesh.Add();

	//preparo as coisas e já tá
	novo->meshName[0]='\0';
	novo->materialName[0]='\0';
	return novo;
}

static
int searchString(const char * const buffer, char caracter)
{
	int i,count;

	count=0;
    for(i=0; buffer[i]!='\0'; i++)
		{
		if (buffer[i]==caracter)
			count++;
		}

	return count;
}

static
const char * leString(const char * const bufferStart, const char * const bufferEnd, char * const toString, const int &stringSize)
{
	int i;
	const char *bufferWalker;

	//começo já por fecha-la
	if (toString)
		toString[0]='\0';

	//razoes obvias
	if (bufferStart==nullptr || bufferEnd==nullptr || toString==nullptr || bufferStart>=bufferEnd)
		return nullptr;

	//leio string
	i=0;
	bufferWalker=bufferStart;
	while( (bufferWalker<bufferEnd) && ((*bufferWalker)!=13) && (i<(stringSize-1)) )
		{
		toString[i]=*bufferWalker;
		bufferWalker++;
		i++;
		}

	//fecho string
	toString[i]='\0';

	//avança dois (13 e 10)
	bufferWalker+=2;

	//retiro simplesmente os TABS da string
	for(i=0; toString[i]!='\0'; i++)
		{
		if (toString[i]==9)
			toString[i]=' ';
		}

	//e já está
	return bufferWalker;
}

static
int parseCountCommands(const char * const command)
{
	int countCommands;
	const char *walkerCommand;

	//verifico parametros
	if (command==nullptr || command[0]=='\0')
		return 0;
	
	//preparo as coisas
	walkerCommand=command;
	countCommands=0;

	//espaços brancos no principio nao contam
	while((*walkerCommand)==' ')
		walkerCommand++;

	//verificar isto
	if ((*walkerCommand)=='\0')
		return 0;

	//começo a ler
	while( ((*walkerCommand)!=13) && ((*walkerCommand)!=10) && ((*walkerCommand)!='\0') )
		{
		//se tiver aspas, tu o que estiver lá dentro é um só argumento
		if ((*walkerCommand)=='"')
			{
			//tenho de copiar esta vez
			walkerCommand++;

			//copio até chegar ao fim
			while(!((*walkerCommand)=='"' || (*walkerCommand)=='\0'))
				walkerCommand++;

			//se sai por causa das aspas
			if ((*walkerCommand)=='"')
				walkerCommand++;
		
			continue;
			}

		//é um caracter normal, mas se for um espaço nao leio
		if ((*walkerCommand)==' ')
			{
			countCommands++;
			walkerCommand++;
			while((*walkerCommand)==' ')
				walkerCommand++;
			continue;
			}

		//proximo
		walkerCommand++;
		}

	//por fim
	return countCommands+1;
}

static
void parseReadCommands(const char * const command, const int numArg, char * const buffer)
{
	char *walkerBuffer;
	int countCommands;
	const char *walkerCommand;

	//primeiro check
	if (buffer==nullptr || numArg<=0)
		return;

	//limpo isto
	buffer[0]='\0';

	//verifico parametros
	if (command==nullptr || command[0]=='\0')
		return;
	
	//preparo as coisas
	walkerCommand=command;
	walkerBuffer=buffer;
	countCommands=0;

	//espaços brancos no principio nao contam
	while((*walkerCommand)==' ')
		walkerCommand++;

	//verificar isto
	if ((*walkerCommand)=='\0')
		return;

	//começo a ler
	while( ((*walkerCommand)!=13) && ((*walkerCommand)!=10) && ((*walkerCommand)!='\0') )
		{
		//é um caracter normal, mas se for um espaço nao leio
		if ((*walkerCommand)==' ')
			{
			countCommands++;
			if (countCommands==numArg)
				{
				*walkerBuffer='\0';
				return;
				}
			walkerBuffer=buffer;
			walkerCommand++;
			while((*walkerCommand)==' ')
				walkerCommand++;
			continue;
			}

		//se tiver aspas, tudo o que estiver lá dentro é um só argumento
		if ((*walkerCommand)=='"')
			{
			//tenho de copiar esta vez
			walkerCommand++;

			//copio até chegar ao fim
			while(!((*walkerCommand)=='"' || (*walkerCommand)=='\0'))
				(*walkerBuffer++)=(*walkerCommand++);

			//se foi pelas aspas
			if ((*walkerCommand)=='"')
				walkerCommand++;

			continue;
			}

		//copio e avanço com os dois
		(*walkerBuffer++)=(*walkerCommand++);
		}

	//fecho ultimo tendo o cuidado de verificar se alguma coisa foi escrita e já está
	*walkerBuffer='\0';
	countCommands++;
	if (countCommands!=numArg)
		buffer[0]='\0';
}

static
void preencheVertex(ASE_PVERTEX *listaPVertex, const int numPVertex, const char * const bufferStart, const char * const bufferEnd)
{
	const char *walker;
	char linha[256],command[64];
	int vertexNumber;

	//tenho de ter isto
	if (listaPVertex==nullptr || numPVertex<=0 || bufferStart==nullptr || bufferEnd==nullptr)
		return;

	//o ponteiro
	walker=bufferStart;

	//leio os parametros do material
	while(walker!=nullptr)
		{
		//leio uma string
		walker=leString(walker,bufferEnd,linha,sizeof(linha));
		if (parseCountCommands(linha)!=5)
			break;

		//o vertice a ler
		parseReadCommands(linha,2,command);
		vertexNumber=atoi(command);
		if (vertexNumber<0 || vertexNumber>=numPVertex)
			continue;

		//leio os valores
		parseReadCommands(linha,3,command);
		listaPVertex[vertexNumber].x=(float)atof(command);
		parseReadCommands(linha,5,command);
		listaPVertex[vertexNumber].y=(float)atof(command);
		parseReadCommands(linha,4,command);
		listaPVertex[vertexNumber].z=-(float)atof(command);
		}
}

static
void preencheFaces(ASE_FACE *listaFaces, const int numFaces, const char * const bufferStart, const char * const bufferEnd)
{
	const char *walker;
	char linha[256],command[64];
	int faceNumber;

	//tenho de ter isto
	if (listaFaces==nullptr || numFaces<=0 || bufferStart==nullptr || bufferEnd==nullptr)
		return;

	//os ponteiros
	walker=bufferStart;

	//leio os parametros do material
	while(walker!=nullptr)
		{
		//leio uma string
		walker=leString(walker,bufferEnd,linha,sizeof(linha));
		if (parseCountCommands(linha)<7)
			break;

		//o vertice a ler
		parseReadCommands(linha,2,command);
		faceNumber=atoi(command);
		if (faceNumber<0 || faceNumber>=numFaces)
			continue;

		//leio os valores
		parseReadCommands(linha,4,command);
		listaFaces[faceNumber].a=atoi(command);
		parseReadCommands(linha,6,command);
		listaFaces[faceNumber].b=atoi(command);
		parseReadCommands(linha,8,command);
		listaFaces[faceNumber].c=atoi(command);
		}
}

static
void preencheTVertex(ASE_TVERTEX * const listaTVertex, const int numTVertex, const char * const bufferStart, const char * const bufferEnd)
{
	const char *walker;
	char linha[256],command[64];
	int indice;

	//tenho de ter isto
	if (listaTVertex==nullptr || numTVertex<=0 || bufferStart==nullptr || bufferEnd==nullptr)
		return;

	//leio os parametros
	walker=bufferStart;
	while(walker!=nullptr)
		{
		//leio uma string
		walker=leString(walker,bufferEnd,linha,sizeof(linha));
		if (parseCountCommands(linha)<5)
			break;

		//leio o indice do tvertex
		parseReadCommands(linha,2,command);
		indice=atoi(command);
		if (indice<0 || indice>=numTVertex)
			continue;

		//leio os valores da textura
		parseReadCommands(linha,3,command);
		listaTVertex[indice].u=(float)atof(command);
		parseReadCommands(linha,4,command);
		listaTVertex[indice].v=1.0f-(float)atof(command);
		}
}

static
void preencheTFace(ASE_FACE * const listaFaces, const int numFaces, const char * const bufferStart, const char * const bufferEnd)
{
	const char *walker;
	char linha[256],command[64];
	int indice;

	//tenho de ter isto
	if (listaFaces==nullptr || numFaces<=0 || bufferStart==nullptr || bufferEnd==nullptr)
		return;

	//leio os parametros
	walker=bufferStart;
	while(walker!=nullptr)
		{
		//leio uma string e preciso de 5 argumentos
		walker=leString(walker,bufferEnd,linha,sizeof(linha));
		if (parseCountCommands(linha)<5)
			break;

		//leio o indice da tface
		parseReadCommands(linha,2,command);
		indice=atoi(command);
		if (indice<0 || indice>=numFaces)
			continue;

		//leio os indices da face
		parseReadCommands(linha,3,command);
		listaFaces[indice].tA=atoi(command);
		parseReadCommands(linha,4,command);
		listaFaces[indice].tB=atoi(command);
		parseReadCommands(linha,5,command);
		listaFaces[indice].tC=atoi(command);
		}
}

static
const char * leGeomASE(Geometry::Model::MeshData * const mesh, const char * const bufferStart, const char * const bufferEnd)
{
	float *posWalker,*texWalker;
	const char *walker,*walkerLast;
	char linha[256],command[64],param1[128];
	int i,j,nParam,chavetas,numPVertex,numTVertex,numFVertex,numFaces,curVertex,curTVertex;
	ASE_PVERTEX *listaPVertex,*newListaFinal;
	ASE_TVERTEX *listaTVertex;
	ASE_FACE *listaFaces;
	unsigned int *newIndices;

	//tenho de ter isto
	if (bufferStart==nullptr || bufferEnd==nullptr)
		return nullptr;

	//o começo
	walker=walkerLast=bufferStart;
	listaTVertex=nullptr;
	listaPVertex=nullptr;
	listaFaces=nullptr;
	numTVertex=0;
	numPVertex=0;
	numFaces=0;

	//tenho uma chaveta, porque tou dentro de uma material
	chavetas=1;

	//leio os parametros do material
	while(walker!=nullptr)
		{
		//leio uma string
		walkerLast=walker;
		walker=leString(walker,bufferEnd,linha,sizeof(linha));
		nParam=parseCountCommands(linha);
		if (nParam==0)
			continue;

		//adiciono chavetas { e retiro }
		chavetas+=searchString(linha,'{');
		chavetas-=searchString(linha,'}');

		//acabei de chegar ao fim
		if (chavetas==0)
			break;

		//se nao tiver material para o qual posso escrever
		if (mesh==nullptr)
			continue;

		//leio o primeiro argumento
		parseReadCommands(linha,1,command);

		//leio o numero de vertices
		if ( (!stricmp(command,"*MESH_NUMVERTEX")) && (nParam>=2) )
			{
			parseReadCommands(linha,2,param1);

			numPVertex=atoi(param1);
			listaPVertex=new ASE_PVERTEX[numPVertex];
			if (listaPVertex==nullptr)
				numPVertex=0;
			continue;
			}

		//leio o numero de faces
		if ( (!stricmp(command,"*MESH_NUMFACES")) && (nParam>=2) )
			{
			parseReadCommands(linha,2,param1);
			if (numFaces!=0 && numFaces!=atoi(param1))
				break;
			if (numFaces==0)
				{
				numFaces=atoi(param1);
				listaFaces=new ASE_FACE[numFaces];
				if (listaFaces==nullptr)
					numFaces=0;
				}
			continue;
			}

		//leio o numero de tvertex
		if ( (!stricmp(command,"*MESH_NUMTVERTEX")) && (nParam>=2) )
			{
			parseReadCommands(linha,2,param1);

			numTVertex=atoi(param1);
			listaTVertex=new ASE_TVERTEX[numTVertex];
			if (listaTVertex==nullptr)
				numTVertex=0;
			continue;
			}

		//leio o numero de tfaces
		if ( (!stricmp(command,"*MESH_NUMTVFACES")) && (nParam>=2) )
			{
			parseReadCommands(linha,2,param1);
			if (numFaces!=0 && numFaces!=atoi(param1))
				break;
			if (numFaces==0)
				{
				numFaces=atoi(param1);
				listaFaces=new ASE_FACE[numFaces];
				if (listaFaces==nullptr)
					numFaces=0;
				}
			continue;
			}

		//leio os vertices
		if (!stricmp(command,"*MESH_VERTEX_LIST"))
			{
			preencheVertex(listaPVertex,numPVertex,walker,bufferEnd);
			continue;
			}

		//leio as faces
		if ( (!stricmp(command,"*MESH_FACE_LIST")) )
			{
			preencheFaces(listaFaces,numFaces,walker,bufferEnd);
			continue;
			}

		//leio os tvertex
		if (!stricmp(command,"*MESH_TVERTLIST"))
			{
			preencheTVertex(listaTVertex,numTVertex,walker,bufferEnd);
			continue;
			}

		//leio as tface
		if (!stricmp(command,"*MESH_TFACELIST"))
			{
			preencheTFace(listaFaces,numFaces,walker,bufferEnd);
			continue;
			}
		}

	//por causa da chaveta que tem de ir para fora
	walker=walkerLast;

	//******************
	//= Tenho de criar a lista final de vertices
	//******************

	//posso arranjar as coisas
	numFVertex=numPVertex;
	for(i=0; i<numPVertex; i++)
		{
		listaPVertex[i].texSet=false;
		}

	//vou percorrer todas as faces e vou copiando o u,v e criando novos quando estes forem repetidos
	for(i=0; i<numFaces; i++)
		{
		//para os tres vertices do triangulo
		for(j=0; j<3; j++)
			{
			//qual o vertice actual
			switch(j){
				case 0:
						curVertex=listaFaces[i].a;
						curTVertex=listaFaces[i].tA;
						break;
				case 1:
						curVertex=listaFaces[i].b;
						curTVertex=listaFaces[i].tB;
						break;
				case 2:
						curVertex=listaFaces[i].c;
						curTVertex=listaFaces[i].tC;
						break;
				default:
						curVertex=curTVertex=-1;
						break;
				}

			//só para nao crashar mesmo
			if (curVertex==-1 || curTVertex==-1)
				continue;

			//se ainda nao foi posto com coordenadas, posso por
			if (listaPVertex[curVertex].texSet==false)
				{
				listaPVertex[curVertex].u=listaTVertex[curTVertex].u;
				listaPVertex[curVertex].v=listaTVertex[curTVertex].v;
				listaPVertex[curVertex].texSet=true;
				continue;
				}

			//agora se já existem, posso ver simplesmente se forem iguais para evitar copiar
			if ( (listaPVertex[curVertex].u==listaTVertex[curTVertex].u) && (listaPVertex[curVertex].v==listaTVertex[curTVertex].v) )
				continue;

			//tenho de criar espaço para este vertice para poder copiar os dados
			newListaFinal=(ASE_PVERTEX*)realloc(listaPVertex,sizeof(ASE_PVERTEX)*(numFVertex+1));
			if (newListaFinal==nullptr)
				continue;

			//arranjo os ponteiros e coisas assim
			listaPVertex=newListaFinal;
			numFVertex++;

			//para a ultima posicao copio as coisas
			listaPVertex[numFVertex-1].x=listaPVertex[curVertex].x;
			listaPVertex[numFVertex-1].y=listaPVertex[curVertex].y;
			listaPVertex[numFVertex-1].z=listaPVertex[curVertex].z;
			listaPVertex[numFVertex-1].u=listaTVertex[curTVertex].u;
			listaPVertex[numFVertex-1].v=listaTVertex[curTVertex].v;
			listaPVertex[numFVertex-1].texSet=true;

			//tenho de indicar a nova posição para a face que estou a verificar
			switch(j){
				case 0:
						listaFaces[i].a=numFVertex-1;
						break;
				case 1:
						listaFaces[i].b=numFVertex-1;
						break;
				case 2:
						listaFaces[i].c=numFVertex-1;
						break;
				}
			}
		}

	//******************
	//= Toca a criar a lista para a minha mesh
	//******************

	//crio espaço para esta cambada toda
	mesh->mesh.NewAttrib(Geometry::Mesh::Pos,numFVertex);
	mesh->mesh.NewAttrib(Geometry::Mesh::TexCoords,numFVertex);

	//mando criar os indices
	newIndices = (unsigned int*)mesh->mesh.CreateNewIndices(Geometry::Mesh::Int32, numFaces*3);
	
	//copio já os indices
	if (newIndices != nullptr)
	{
		for(i=0; i<numFaces; i++)
		{
			newIndices[i*3+0] = listaFaces[i].a;
			newIndices[i*3+1] = listaFaces[i].b;
			newIndices[i*3+2] = listaFaces[i].c;
		}
	}

	//copio as posições dos vertices e as coordenadas de texturas
	posWalker=mesh->mesh.FindAttribData(Geometry::Mesh::Pos);
	texWalker=mesh->mesh.FindAttribData(Geometry::Mesh::TexCoords);
	for(i=0; i<numFVertex; i++)
	{
		posWalker[i*3+0]=listaPVertex[i].x;
		posWalker[i*3+1]=listaPVertex[i].y;
		posWalker[i*3+2]=listaPVertex[i].z;

		texWalker[i*2+0]=listaPVertex[i].u;
		texWalker[i*2+1]=listaPVertex[i].v;
	}

	//limpo isto caso existam
	if (listaPVertex!=nullptr)
		delete [] listaPVertex;
	if (listaTVertex!=nullptr)
		delete [] listaTVertex;
	if (listaFaces!=nullptr)
		delete [] listaFaces;

	//tenho de devolver isto
	return walker;
}

Geometry::Model* readMemASE(const void *file, const unsigned int fileSize)
{
	Geometry::Model *modelo;
	char linha[256];
	const char *walker,*bufferFim;
	char command[64],param1[64];
	int nParam,chavetas;
	Geometry::Model::MeshData *mesh;

	//verificar parametros
	if (file==nullptr || fileSize==0)
		return nullptr;

	//crio o modelo
	modelo=new Geometry::Model();
	if (modelo==nullptr)
		return nullptr;

	//começo a ler
	walker=(const char*)file;
	bufferFim=walker+fileSize;

	//começo a ler strings
	while(walker!=nullptr)
	{
		//leio uma string e vejo que parametros a compoem
		walker=leString(walker,bufferFim,linha,sizeof(linha));
		nParam=parseCountCommands(linha);
		if (nParam==0)
			continue;

		//leio o primeiro argumento
		parseReadCommands(linha,1,command);

		//se for uma mesh
		if (!stricmp(command,"*GEOMOBJECT"))
		{
			//crio a mesh
			mesh = criaMesh(modelo);
			if (mesh == nullptr)
				continue;

			//se ja tinha uma chaveta na linha de geom
			chavetas=0;
			chavetas+=searchString(linha,'{');
			chavetas-=searchString(linha,'}');

			//enquanto nao chegar ao fim
			while(walker!=nullptr)
			{
				//leio uma string e vejo que parametros a compoem
				walker=leString(walker,bufferFim,linha,sizeof(linha));
				nParam=parseCountCommands(linha);
				if (nParam==0)
					continue;

				//leio o primeiro argumento
				parseReadCommands(linha,1,command);

				//adiciono chavetas { e retiro } e saio se cheguei ao fim
				chavetas+=searchString(linha,'{');
				chavetas-=searchString(linha,'}');
				if (chavetas==0)
					break;

				//copio o nome
				if ( (!stricmp(command,"*NODE_NAME")) && (nParam>=2) && (mesh!=nullptr) )
				{
					parseReadCommands(linha,2,param1);
					strcpy(mesh->meshName,param1);
					continue;
				}

				//se encontrei a mesh
				if (!stricmp(command,"*MESH"))
				{
					walker=leGeomASE(mesh,walker,bufferFim);
					continue;
				}
			}
			continue;
		}
	}

	//se nao tiver nada, nao devolvo nada
	if (modelo->arrayMesh.GetNumElements() <= 0)
	{
		delete modelo;
		return nullptr;
	}

	//já tá porra
	return modelo;
}