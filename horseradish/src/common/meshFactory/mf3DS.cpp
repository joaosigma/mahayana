#include "common\Mesh.hpp"
#include "common\MeshFactory.hpp"
#include "common\Stream.hpp"

#include <windows.h>

using namespace HorseRadish;

/*§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§
§§§§§§   -= Estructuras e declarações necessárias =-   §§§§§
§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§*/

#define M3DMAGIC		0x4d4d
#define M3D_VERSION		0x0002
#define MDATA			0x3d3d
#define NAMED_OBJECT	0x4000
#define TRI_OBJECT		0x4100
#define POINT_ARRAY		0x4110
#define FACE_ARRAY		0x4120
#define TEX_ARRAY		0x4140
#define LOCAL_COORD		0x4160

//o chunk 3DS
struct chunk3DS{
	unsigned short id;
	long len;
};

//estructura auxiliar para guardar os meus dados
struct LISTAMESH{
	Geometry::Mesh mesh;
	char nome[256];
	struct LISTAMESH *prox;
};

//a lista de meshes
static LISTAMESH *ListaMeshes;

//declarações
static bool readString(HorseRadish::Streams::StreamReader * const streamReader, char * const data, const int &dataLength);
static bool readUShort(HorseRadish::Streams::StreamReader * const streamReader, unsigned short *data);
static bool readULong(HorseRadish::Streams::StreamReader * const streamReader, unsigned long *data);
static bool readFloat(HorseRadish::Streams::StreamReader * const streamReader, float *data);
static bool readLong(HorseRadish::Streams::StreamReader * const streamReader, long *data);
static bool readChunk(HorseRadish::Streams::StreamReader * const streamReader, chunk3DS * const chunk);
static bool is3DS(HorseRadish::Streams::StreamReader * const streamReader);

static bool readM3DMAGIC(HorseRadish::Streams::StreamReader * const streamReader, const long &chunkStart, const long &chunkLength);
static bool readMDATA(HorseRadish::Streams::StreamReader * const streamReader, const long &chunkStart, const long &chunkLength);
static bool readNAMED_OBJECT(HorseRadish::Streams::StreamReader * const streamReader, const long &chunkStart, const long &chunkLength);
static bool readTRI_OBJECT(HorseRadish::Streams::StreamReader * const streamReader, const long &chunkStart, const long &chunkLength, Geometry::Mesh * const mesh);

static bool readPOINT_ARRAY(HorseRadish::Streams::StreamReader * const streamReader, Geometry::Mesh * const mesh);
static bool readFACE_ARRAY(HorseRadish::Streams::StreamReader * const streamReader, Geometry::Mesh * const mesh);
static bool readTEX_ARRAY(HorseRadish::Streams::StreamReader * const streamReader, Geometry::Mesh * const mesh);
static bool readLOCAL_COORD(HorseRadish::Streams::StreamReader * const streamReader, Geometry::Mesh * const mesh);

/*§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§
§§§§§§   -= Funções para gerir tudo =-   §§§§§
§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§*/
static
struct Geometry::Mesh* criaMesh(const char * const nome)
{
	LISTAMESH *walker;

	if (ListaMeshes==nullptr)
		{
		ListaMeshes=new LISTAMESH;
		if (ListaMeshes==nullptr)
			return nullptr;
		ListaMeshes->nome[0]='\0';
		if (nome)
			strcpy(ListaMeshes->nome,nome);
		ListaMeshes->prox=nullptr;
		return &ListaMeshes->mesh;
		}

	for(walker=ListaMeshes; walker->prox!=nullptr; walker=walker->prox);

	walker->prox=new LISTAMESH;
	if (walker->prox==nullptr)
		return nullptr;
	walker->prox->nome[0]='\0';
	if (nome)
		strcpy(walker->prox->nome,nome);
	walker->prox->prox=nullptr;
	return &walker->prox->mesh;
}

static
void destroiMesh(void)
{
	LISTAMESH *walker,*lixo;

	walker=ListaMeshes;
	while(walker!=nullptr)
		{
		lixo=walker;
		walker=walker->prox;
		delete lixo;
		}
}

static
bool readString(HorseRadish::Streams::StreamReader * const streamReader, char * const data, const int &dataLength)
{
	int i,result;

	//verificar parametros
	if (streamReader==nullptr || data==nullptr || dataLength<=0)
		return false;

	//começo a ler
	for(i=0; (i!=(dataLength-1)); i++)
		{
		//leio um caracter
		result = streamReader->Read(&data[i],sizeof(char));

		//se nao leu
		if (result==0)
			{
			data[i]='\0';
			return false;
			}

		//se terminou a string
		if (data[i]=='\0')
			return true;
		}

	//fecho a string
	data[i]='\0';
	return true;
}

static
bool readUShort(HorseRadish::Streams::StreamReader * const streamReader, unsigned short *data)
{
	if (streamReader==nullptr || data==nullptr)
		return false;

	if (streamReader->Read(data,sizeof(unsigned short))!=sizeof(unsigned short))
		return false;
	
	return true;
}

static
bool readULong(HorseRadish::Streams::StreamReader * const streamReader, unsigned long *data)
{
	if (streamReader==nullptr || data==nullptr)
		return false;

	if (streamReader->Read(data,sizeof(unsigned long))!=sizeof(unsigned long))
		return false;
	
	return true;
}

static
bool readFloat(HorseRadish::Streams::StreamReader * const streamReader, float *data)
{
	if (streamReader==nullptr || data==nullptr)
		return false;

	if (streamReader->Read(data,sizeof(float))!=sizeof(float))
		return false;
	
	return true;
}

static
bool readLong(HorseRadish::Streams::StreamReader * const streamReader, long *data)
{
	if (streamReader==nullptr || data==nullptr)
		return false;

	if (streamReader->Read(data,sizeof(long))!=sizeof(long))
		return false;
	
	return true;
}

static
bool readChunk(HorseRadish::Streams::StreamReader * const streamReader, chunk3DS * const chunk)
{
	if (streamReader==nullptr || chunk==nullptr)
		return false;

	if (readUShort(streamReader,&chunk->id)==false)
		return false;
	if (readLong(streamReader,&chunk->len)==false)
		return false;

	return true;
}

//****************************************************

static
bool readM3DMAGIC(HorseRadish::Streams::StreamReader * const streamReader, const long &chunkStart, const long &chunkLength)
{
	int pos;
	chunk3DS chunk;
	unsigned long versao;

	if (streamReader==nullptr || chunkStart<0 || chunkLength<0)
		return false;

	pos=streamReader->GetPosition();
	while( (pos<(chunkStart+chunkLength)) && (readChunk(streamReader,&chunk))  )
		{
		if (chunk.id==M3D_VERSION)
			{
			if (readULong(streamReader,&versao)==false)
				return false;
			}
		else if (chunk.id==MDATA)
			{
			if (readMDATA(streamReader,pos,chunk.len)==false)
				return false;
			}
		streamReader->Seek(pos+chunk.len, HorseRadish::Streams::Stream::Begin);
		pos=streamReader->GetPosition();
		}

	return true;
}

static
bool readMDATA(HorseRadish::Streams::StreamReader * const streamReader, const long &chunkStart, const long &chunkLength)
{
	chunk3DS chunk;
	int pos;

	if (streamReader==nullptr || chunkStart<0 || chunkLength<0)
		return false;

	pos=streamReader->GetPosition();
	while( (pos<(chunkStart+chunkLength)) && (readChunk(streamReader,&chunk)))
		{
		if (chunk.id==NAMED_OBJECT)
			{
			if (readNAMED_OBJECT(streamReader,pos,chunk.len)==false)
				return false;
			}
		streamReader->Seek(pos+chunk.len, HorseRadish::Streams::Stream::Begin);
		pos=streamReader->GetPosition();
		}

	return true;
}

static
bool readNAMED_OBJECT(HorseRadish::Streams::StreamReader * const streamReader, const long &chunkStart, const long &chunkLength)
{
	int pos;
	Geometry::Mesh *mesh;
	chunk3DS chunk;
	char nome[256];

	if (streamReader==nullptr || chunkStart<0 || chunkLength<0)
		return false;

	//descubro o nome do objecto
	if (readString(streamReader,nome,256)==false)
		return false;

	//vou ler os chunks
	pos=streamReader->GetPosition();
	while( (pos<(chunkStart+chunkLength)) && (readChunk(streamReader,&chunk)) )	
		{
		if (chunk.id==TRI_OBJECT)
			{
			//crio uma mesh para guardar as coisas
			mesh=criaMesh(nome);

			//continuo
			if (readTRI_OBJECT(streamReader,pos,chunk.len,mesh)==false)
				return false;
			}

		streamReader->Seek(pos+chunk.len, HorseRadish::Streams::Stream::Begin);
		pos=streamReader->GetPosition();
		}

	return true;
}

static
bool readTRI_OBJECT(HorseRadish::Streams::StreamReader * const streamReader, const long &chunkStart, const long &chunkLength, Geometry::Mesh * const mesh)
{
	chunk3DS chunk;
	int pos;

	if (streamReader==nullptr || chunkStart<0 || chunkLength<0 || mesh==nullptr)
		return false;

	pos=streamReader->GetPosition();
	while( (pos<(chunkStart+chunkLength)) && (readChunk(streamReader,&chunk)) )
		{
		if (chunk.id==POINT_ARRAY)
			{
			if (readPOINT_ARRAY(streamReader,mesh)==false)
				return false;
			}
		else if (chunk.id==FACE_ARRAY)
			{
			if (readFACE_ARRAY(streamReader,mesh)==false)
				return false;
			}
		else if (chunk.id==TEX_ARRAY)
			{
			if (readTEX_ARRAY(streamReader,mesh)==false)
				return false;
			}
		else if (chunk.id==LOCAL_COORD)
			{
			if (readLOCAL_COORD(streamReader,mesh)==false)
				return false;
			}

		streamReader->Seek(pos+chunk.len, HorseRadish::Streams::Stream::Begin);
		pos=streamReader->GetPosition();
		}

	return true;
}

static
bool readPOINT_ARRAY(HorseRadish::Streams::StreamReader * const streamReader, Geometry::Mesh * const mesh)
{
	Geometry::Mesh::Attribute *newAttrib;
	unsigned short numElements;
	float *points;

	//verifica parametros
	if (streamReader==nullptr || mesh==nullptr)
		return false;

	//tento ler os dados
	if (readUShort(streamReader,&numElements)==false)
		return false;

	//0 nao interessa
	if (numElements<=0)
		return true;

	//crio espaço para mais um atributo
	newAttrib = mesh->NewAttrib(Geometry::Mesh::Pos, numElements);
	if (newAttrib == nullptr)
		return false;

	//o ponteiro para os dados
	points=newAttrib->attribData;

	//leio os pontos (o sistema o 3DStudio é diferente: xyz->x(-z)y
	for(;numElements>0; numElements--)
	{
		if (readFloat(streamReader,points+0)==false)
			return false;
		if (readFloat(streamReader,points+2)==false)
			return false;
		if (readFloat(streamReader,points+1)==false)
			return false;

		//inverto o y (z em 3DStudio)
		points[2]*=-1.0f;

		//avanço com ponteiro
		points+=3;
	}

	return true;
}

static
bool readFACE_ARRAY(HorseRadish::Streams::StreamReader * const streamReader, Geometry::Mesh * const mesh)
{
	int curIndex;
	unsigned int *newIndices;
	unsigned short numElements, lidos[4];

	//verificar parametros
	if (streamReader==nullptr || mesh==nullptr)
		return false;

	//tento ler os dados
	if (readUShort(streamReader,&numElements)==false)
		return false;

	//este valor nao interessa
	if (numElements<=0)
		return true;

	//mando criar os indices
	newIndices = (unsigned int*)mesh->CreateNewIndices(Geometry::Mesh::Int32, numElements * 3);

	//toca a ler
	for(curIndex=0; numElements>0; numElements--,curIndex+=3)
	{
		if (readUShort(streamReader, lidos+0)==false)
			return false;
		if (readUShort(streamReader, lidos+1)==false)
			return false;
		if (readUShort(streamReader, lidos+2)==false)
			return false;
		if (readUShort(streamReader, lidos+3)==false)
			return false;

		newIndices[curIndex+0] = lidos[0];
		newIndices[curIndex+1] = lidos[1];
		newIndices[curIndex+2] = lidos[2];
	}

	return true;
}

static
bool readTEX_ARRAY(HorseRadish::Streams::StreamReader * const streamReader, Geometry::Mesh * const mesh)
{
	Geometry::Mesh::Attribute *newAttrib;
	unsigned short numElements;
	float *points;

	//verificar parametros
	if (streamReader==nullptr || mesh==nullptr)
		return false;

	//tento ler os dados
	if (readUShort(streamReader,&numElements)==false)
		return false;

	//este valor nao interessa
	if (numElements<=0)
		return true;

	//crio um novo atributo
	newAttrib=mesh->NewAttrib(Geometry::Mesh::TexCoords,numElements);
	if (newAttrib==nullptr)
		return false;

	//coloco o ponteiro
	points=newAttrib->attribData;

	//leio os pontos
	for(;numElements>0; numElements--)
	{
		if (readFloat(streamReader,points+0)==false)
			return false;
		if (readFloat(streamReader,points+1)==false)
			return false;
		points+=2;
	}

	return true;
}

static
bool readLOCAL_COORD(HorseRadish::Streams::StreamReader * const streamReader, Geometry::Mesh * const mesh)
{
	float vec1[3],vec2[3],vec3[3],vecO[3];

	//verificar parametros
	if (streamReader==nullptr || mesh==nullptr)
		return false;

	//leio os dados
	if (readFloat(streamReader,vec1+0)==false)
		return false;
	if (readFloat(streamReader,vec1+2)==false)
		return false;
	if (readFloat(streamReader,vec1+1)==false)
		return false;

	if (readFloat(streamReader,vec2+0)==false)
		return false;
	if (readFloat(streamReader,vec2+2)==false)
		return false;
	if (readFloat(streamReader,vec2+1)==false)
		return false;

	if (readFloat(streamReader,vec3+0)==false)
		return false;
	if (readFloat(streamReader,vec3+2)==false)
		return false;
	if (readFloat(streamReader,vec3+1)==false)
		return false;

	if (readFloat(streamReader,vecO+0)==false)
		return false;
	if (readFloat(streamReader,vecO+2)==false)
		return false;
	if (readFloat(streamReader,vecO+1)==false)
		return false;

	//não esquecer de negar o z (porque a troca de y pra z já foi feita)
	vec1[2]*=-1.0f;
	vec2[2]*=-1.0f;
	vec3[2]*=-1.0f;
	vecO[2]*=-1.0f;

	mesh->OpMAD(Geometry::Mesh::Pos,0,1.0f,vecO[0]);
	mesh->OpMAD(Geometry::Mesh::Pos,1,1.0f,vecO[1]);
	mesh->OpMAD(Geometry::Mesh::Pos,2,1.0f,vecO[2]);

	return true;
}

static
Geometry::Model *criaModelo()
{
	LISTAMESH *meshWalker;
	Geometry::Model *modelo;
	int numMeshes;

	//conto o numero de meshes
	for(numMeshes=0,meshWalker=ListaMeshes; meshWalker!=nullptr; meshWalker=meshWalker->prox,numMeshes++);

	//se nada houver
	if (numMeshes==0)
		return nullptr;

	//crio espaço para albergar tudo
	modelo=new Geometry::Model(numMeshes);
	if (modelo==nullptr)
		return nullptr;

	//copio tudo
	for(numMeshes=0,meshWalker=ListaMeshes; meshWalker!=nullptr; meshWalker=meshWalker->prox,numMeshes++)
	{
		//crio uma mesh
		modelo->arrayMesh.Add();

		//copio os dados
		memcpy(&modelo->arrayMesh[numMeshes].mesh,&meshWalker->mesh,sizeof(Geometry::Mesh));

		//copio o nome
		strcpy(modelo->arrayMesh[numMeshes].meshName,meshWalker->nome);

		//operações de limpeza
		modelo->arrayMesh[numMeshes].mesh.EliminateDegenerateTri();

		//mando criar as normais
		modelo->arrayMesh[numMeshes].mesh.Ortho(MESH_ORTHO_CREATE_NORMALS | MESH_ORTHO_PER_POSITION_NRM);
	}

	//devolvo o resultado
	return modelo;
}

/*§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§
§§§§§§   -= Funções para exportar =-   §§§§§
§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§*/
Geometry::Model* mfRead3DS(HorseRadish::Streams::StreamReader * const streamReader)
{
	chunk3DS chunk;
	Geometry::Model *modelo;
	int pos;

	//verificar parametros
	if (streamReader == nullptr)
		return nullptr;

	//coloco isto com NULL que é para começar de novo
	ListaMeshes=nullptr;

	//começo a ler
	pos = streamReader->GetPosition();
	while( (streamReader->CanRead() == true) && (readChunk(streamReader, &chunk)) )
	{
		//procutro pelo chunck principal
		if (chunk.id==M3DMAGIC)
		{
			if (readM3DMAGIC(streamReader, pos, chunk.len)==false)
				return nullptr;
			break;
		}

		//sigo
		streamReader->Seek(pos+chunk.len, HorseRadish::Streams::Stream::Begin);
		pos = streamReader->GetPosition();
	}

	//crio espaço para o modelo
	modelo=criaModelo();

	//apago tudo
	destroiMesh();
	ListaMeshes=nullptr;

	//já tá porra
	return modelo;
}