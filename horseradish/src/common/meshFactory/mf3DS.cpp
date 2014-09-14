#include "common\Mesh.hpp"
#include "common\MeshFactory.hpp"
#include "common\Stream.hpp"

using namespace HorseRadish;

#define M3DMAGIC		0x4d4d
#define M3D_VERSION		0x0002
#define MDATA			0x3d3d
#define NAMED_OBJECT	0x4000
#define TRI_OBJECT		0x4100
#define POINT_ARRAY		0x4110
#define FACE_ARRAY		0x4120
#define TEX_ARRAY		0x4140
#define LOCAL_COORD		0x4160

struct chunk3DS{
	unsigned short id;
	long len;
};

struct LISTAMESH{
	Geometry::Mesh mesh;
	char nome[256];
	struct LISTAMESH *prox;
};

static LISTAMESH *ListaMeshes;

static bool readString(HorseRadish::Streams::StreamReader &streamReader, char * const data, const int &dataLength);
static bool readUShort(HorseRadish::Streams::StreamReader &streamReader, unsigned short *data);
static bool readULong(HorseRadish::Streams::StreamReader &streamReader, unsigned long *data);
static bool readFloat(HorseRadish::Streams::StreamReader &streamReader, float *data);
static bool readLong(HorseRadish::Streams::StreamReader &streamReader, long *data);
static bool readChunk(HorseRadish::Streams::StreamReader &streamReader, chunk3DS * const chunk);
static bool is3DS(HorseRadish::Streams::StreamReader &streamReader);

static bool readM3DMAGIC(HorseRadish::Streams::StreamReader &streamReader, const long &chunkStart, const long &chunkLength);
static bool readMDATA(HorseRadish::Streams::StreamReader &streamReader, const long &chunkStart, const long &chunkLength);
static bool readNAMED_OBJECT(HorseRadish::Streams::StreamReader &streamReader, const long &chunkStart, const long &chunkLength);
static bool readTRI_OBJECT(HorseRadish::Streams::StreamReader &streamReader, const long &chunkStart, const long &chunkLength, Geometry::Mesh * const mesh);

static bool readPOINT_ARRAY(HorseRadish::Streams::StreamReader &streamReader, Geometry::Mesh * const mesh);
static bool readFACE_ARRAY(HorseRadish::Streams::StreamReader &streamReader, Geometry::Mesh * const mesh);
static bool readTEX_ARRAY(HorseRadish::Streams::StreamReader &streamReader, Geometry::Mesh * const mesh);
static bool readLOCAL_COORD(HorseRadish::Streams::StreamReader &streamReader, Geometry::Mesh * const mesh);

static
struct Geometry::Mesh* criaMesh(const char * const nome)
{
	LISTAMESH *walker;

	if (ListaMeshes == nullptr)
	{
		ListaMeshes = new LISTAMESH;
		if (ListaMeshes == nullptr)
			return nullptr;
		ListaMeshes->nome[0] = '\0';
		if (nome)
			strcpy(ListaMeshes->nome, nome);
		ListaMeshes->prox = nullptr;
		return &ListaMeshes->mesh;
	}

	for (walker = ListaMeshes; walker->prox != nullptr; walker = walker->prox);

	walker->prox = new LISTAMESH;
	if (walker->prox == nullptr)
		return nullptr;
	walker->prox->nome[0] = '\0';
	if (nome)
		strcpy(walker->prox->nome, nome);
	walker->prox->prox = nullptr;
	return &walker->prox->mesh;
}

static
void destroiMesh(void)
{
	LISTAMESH *walker, *lixo;

	walker = ListaMeshes;
	while (walker != nullptr)
	{
		lixo = walker;
		walker = walker->prox;
		delete lixo;
	}
}

static
bool readString(HorseRadish::Streams::StreamReader &streamReader, char * const data, const int &dataLength)
{
	int i, result;

	if (data == nullptr || dataLength <= 0)
		return false;

	for (i = 0; (i != (dataLength - 1)); i++)
	{
		result = streamReader.Read(&data[i], sizeof(char));

		if (result == 0)
		{
			data[i] = '\0';
			return false;
		}

		if (data[i] == '\0')
			return true;
	}

	data[i] = '\0';
	return true;
}

static
bool readUShort(HorseRadish::Streams::StreamReader &streamReader, unsigned short *data)
{
	if (data == nullptr)
		return false;

	if (streamReader.Read(data, sizeof(unsigned short)) != sizeof(unsigned short))
		return false;

	return true;
}

static
bool readULong(HorseRadish::Streams::StreamReader &streamReader, unsigned long *data)
{
	if (data == nullptr)
		return false;

	if (streamReader.Read(data, sizeof(unsigned long)) != sizeof(unsigned long))
		return false;

	return true;
}

static
bool readFloat(HorseRadish::Streams::StreamReader &streamReader, float *data)
{
	if (data == nullptr)
		return false;

	if (streamReader.Read(data, sizeof(float)) != sizeof(float))
		return false;

	return true;
}

static
bool readLong(HorseRadish::Streams::StreamReader &streamReader, long *data)
{
	if (data == nullptr)
		return false;

	if (streamReader.Read(data, sizeof(long)) != sizeof(long))
		return false;

	return true;
}

static
bool readChunk(HorseRadish::Streams::StreamReader &streamReader, chunk3DS * const chunk)
{
	if (chunk == nullptr)
		return false;

	if (readUShort(streamReader, &chunk->id) == false)
		return false;
	if (readLong(streamReader, &chunk->len) == false)
		return false;

	return true;
}

static
bool readM3DMAGIC(HorseRadish::Streams::StreamReader &streamReader, const long &chunkStart, const long &chunkLength)
{
	chunk3DS chunk;
	unsigned long versao;

	if (chunkStart < 0 || chunkLength < 0)
		return false;

	auto pos = streamReader.GetPosition();
	while ((pos < (chunkStart + chunkLength)) && (readChunk(streamReader, &chunk)))
	{
		if (chunk.id == M3D_VERSION)
		{
			if (readULong(streamReader, &versao) == false)
				return false;
		}
		else if (chunk.id == MDATA)
		{
			if (readMDATA(streamReader, pos, chunk.len) == false)
				return false;
		}

		streamReader.Seek(pos + chunk.len, HorseRadish::Streams::Stream::Begin);
		pos = streamReader.GetPosition();
	}

	return true;
}

static
bool readMDATA(HorseRadish::Streams::StreamReader &streamReader, const long &chunkStart, const long &chunkLength)
{
	chunk3DS chunk;

	if (chunkStart < 0 || chunkLength < 0)
		return false;

	auto pos = streamReader.GetPosition();
	while ((pos < (chunkStart + chunkLength)) && (readChunk(streamReader, &chunk)))
	{
		if (chunk.id == NAMED_OBJECT)
		{
			if (readNAMED_OBJECT(streamReader, pos, chunk.len) == false)
				return false;
		}
		streamReader.Seek(pos + chunk.len, HorseRadish::Streams::Stream::Begin);
		pos = streamReader.GetPosition();
	}

	return true;
}

static
bool readNAMED_OBJECT(HorseRadish::Streams::StreamReader &streamReader, const long &chunkStart, const long &chunkLength)
{
	chunk3DS chunk;
	char nome[256];

	if (chunkStart < 0 || chunkLength < 0)
		return false;

	if (readString(streamReader, nome, 256) == false)
		return false;

	auto pos = streamReader.GetPosition();
	while ((pos < (chunkStart + chunkLength)) && (readChunk(streamReader, &chunk)))
	{
		if (chunk.id == TRI_OBJECT)
		{
			auto mesh = criaMesh(nome);

			if (readTRI_OBJECT(streamReader, pos, chunk.len, mesh) == false)
				return false;
		}

		streamReader.Seek(pos + chunk.len, HorseRadish::Streams::Stream::Begin);
		pos = streamReader.GetPosition();
	}

	return true;
}

static
bool readTRI_OBJECT(HorseRadish::Streams::StreamReader &streamReader, const long &chunkStart, const long &chunkLength, Geometry::Mesh * const mesh)
{
	chunk3DS chunk;

	if (chunkStart < 0 || chunkLength < 0 || mesh == nullptr)
		return false;

	auto pos = streamReader.GetPosition();
	while ((pos < (chunkStart + chunkLength)) && (readChunk(streamReader, &chunk)))
	{
		if (chunk.id == POINT_ARRAY)
		{
			if (readPOINT_ARRAY(streamReader, mesh) == false)
				return false;
		}
		else if (chunk.id == FACE_ARRAY)
		{
			if (readFACE_ARRAY(streamReader, mesh) == false)
				return false;
		}
		else if (chunk.id == TEX_ARRAY)
		{
			if (readTEX_ARRAY(streamReader, mesh) == false)
				return false;
		}
		else if (chunk.id == LOCAL_COORD)
		{
			if (readLOCAL_COORD(streamReader, mesh) == false)
				return false;
		}

		streamReader.Seek(pos + chunk.len, HorseRadish::Streams::Stream::Begin);
		pos = streamReader.GetPosition();
	}

	return true;
}

static
bool readPOINT_ARRAY(HorseRadish::Streams::StreamReader &streamReader, Geometry::Mesh * const mesh)
{
	unsigned short numElements;

	if (mesh == nullptr)
		return false;

	if (readUShort(streamReader, &numElements) == false)
		return false;

	if (numElements <= 0)
		return true;

	auto newAttrib = mesh->NewAttrib(Geometry::Mesh::Pos, numElements);
	if (newAttrib == nullptr)
		return false;

	auto points = newAttrib->attribData;

	for (; numElements > 0; numElements--)
	{
		if (readFloat(streamReader, points + 0) == false)
			return false;
		if (readFloat(streamReader, points + 2) == false)
			return false;
		if (readFloat(streamReader, points + 1) == false)
			return false;

		points[2] *= -1.0f;

		points += 3;
	}

	return true;
}

static
bool readFACE_ARRAY(HorseRadish::Streams::StreamReader &streamReader, Geometry::Mesh * const mesh)
{
	unsigned short numElements, dataRead[4];

	if (mesh == nullptr)
		return false;

	if (readUShort(streamReader, &numElements) == false)
		return false;

	if (numElements <= 0)
		return true;

	auto newIndices = reinterpret_cast<unsigned int*>(mesh->CreateNewIndices(Geometry::Mesh::Int32, numElements * 3));

	for (int curIndex = 0; numElements > 0; numElements--, curIndex += 3)
	{
		if (readUShort(streamReader, dataRead + 0) == false)
			return false;
		if (readUShort(streamReader, dataRead + 1) == false)
			return false;
		if (readUShort(streamReader, dataRead + 2) == false)
			return false;
		if (readUShort(streamReader, dataRead + 3) == false)
			return false;

		newIndices[curIndex + 0] = dataRead[0];
		newIndices[curIndex + 1] = dataRead[1];
		newIndices[curIndex + 2] = dataRead[2];
	}

	return true;
}

static
bool readTEX_ARRAY(HorseRadish::Streams::StreamReader &streamReader, Geometry::Mesh * const mesh)
{
	unsigned short numElements;

	if (mesh == nullptr)
		return false;

	if (readUShort(streamReader, &numElements) == false)
		return false;

	if (numElements <= 0)
		return true;

	auto newAttrib = mesh->NewAttrib(Geometry::Mesh::TexCoords, numElements);
	if (newAttrib == nullptr)
		return false;

	auto points = newAttrib->attribData;

	for (; numElements > 0; numElements--)
	{
		if (readFloat(streamReader, points + 0) == false)
			return false;
		if (readFloat(streamReader, points + 1) == false)
			return false;
		points += 2;
	}

	return true;
}

static
bool readLOCAL_COORD(HorseRadish::Streams::StreamReader &streamReader, Geometry::Mesh * const mesh)
{
	float vec1[3], vec2[3], vec3[3], vecO[3];

	if (mesh == nullptr)
		return false;

	if (readFloat(streamReader, vec1 + 0) == false)
		return false;
	if (readFloat(streamReader, vec1 + 2) == false)
		return false;
	if (readFloat(streamReader, vec1 + 1) == false)
		return false;

	if (readFloat(streamReader, vec2 + 0) == false)
		return false;
	if (readFloat(streamReader, vec2 + 2) == false)
		return false;
	if (readFloat(streamReader, vec2 + 1) == false)
		return false;

	if (readFloat(streamReader, vec3 + 0) == false)
		return false;
	if (readFloat(streamReader, vec3 + 2) == false)
		return false;
	if (readFloat(streamReader, vec3 + 1) == false)
		return false;

	if (readFloat(streamReader, vecO + 0) == false)
		return false;
	if (readFloat(streamReader, vecO + 2) == false)
		return false;
	if (readFloat(streamReader, vecO + 1) == false)
		return false;

	vec1[2] *= -1.0f;
	vec2[2] *= -1.0f;
	vec3[2] *= -1.0f;
	vecO[2] *= -1.0f;

	mesh->OpMAD(Geometry::Mesh::Pos, 0, 1.0f, vecO[0]);
	mesh->OpMAD(Geometry::Mesh::Pos, 1, 1.0f, vecO[1]);
	mesh->OpMAD(Geometry::Mesh::Pos, 2, 1.0f, vecO[2]);

	return true;
}

static
Geometry::Model *criaModelo()
{
	LISTAMESH *meshWalker;
	int numMeshes;

	for (numMeshes = 0, meshWalker = ListaMeshes; meshWalker != nullptr; meshWalker = meshWalker->prox, numMeshes++);

	if (numMeshes == 0)
		return nullptr;

	auto modelo = new Geometry::Model(numMeshes);
	if (modelo == nullptr)
		return nullptr;

	for (numMeshes = 0, meshWalker = ListaMeshes; meshWalker != nullptr; meshWalker = meshWalker->prox, numMeshes++)
	{
		modelo->arrayMesh.push_back(Geometry::Model::MeshData());

		memcpy(&modelo->arrayMesh[numMeshes].mesh, &meshWalker->mesh, sizeof(Geometry::Mesh));

		strcpy(modelo->arrayMesh[numMeshes].meshName, meshWalker->nome);

		modelo->arrayMesh[numMeshes].mesh.EliminateDegenerateTri();

		modelo->arrayMesh[numMeshes].mesh.Ortho((HorseRadish::Geometry::Mesh::MeshOrthoTypes)(HorseRadish::Geometry::Mesh::MeshOrthoCreateNormals | HorseRadish::Geometry::Mesh::MeshOrthoPerPositionNormal));
	}

	return modelo;
}

Geometry::Model* mfRead3DS(HorseRadish::Streams::StreamReader &streamReader)
{
	chunk3DS chunk;

	ListaMeshes = nullptr;

	auto pos = streamReader.GetPosition();
	while( (streamReader.CanRead() == true) && (readChunk(streamReader, &chunk)) )
	{
		if (chunk.id==M3DMAGIC)
		{
			if (readM3DMAGIC(streamReader, pos, chunk.len)==false)
				return nullptr;
			break;
		}

		streamReader.Seek(pos+chunk.len, HorseRadish::Streams::Stream::Begin);
		pos = streamReader.GetPosition();
	}

	auto modelo=criaModelo();

	destroiMesh();
	ListaMeshes=nullptr;

	return modelo;
}