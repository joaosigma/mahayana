#include "common\Platform.hpp"
#include "common\Types.hpp"
#include "common\Mesh.hpp"
#include "common\MeshFactory.hpp"
#include "common\Stream.hpp"

using namespace HorseRadish;

struct OBJ{
	char nome[88];
	float *vertex, *tCoord;
	int numVertex, numtCoord, numNormals;
	int numAVertex, numAtCoord;

	struct FACE{
		int index[3], indexT[3];
		int group;
	}*faces;

	int numFaces;
	int numAFaces;
};

struct INDEX_SORT{
	unsigned int index, indexT, pos;
};

static bool readFaces(HorseRadish::Streams::StreamReader &streamReader, OBJ * const object, const int * const accumNum);
static bool doMesh(OBJ * const obj, Geometry::Mesh *mesh);

static
bool leLinha(HorseRadish::Streams::StreamReader &streamReader, char * const buffer, const int bufferSize)
{
	int lerMesmo;
	HorseRadish::hInt8 curChar, lastChar;

	if (buffer == nullptr || bufferSize <= 1)
		return false;

	if (streamReader.CanRead() == false)
		return false;

	buffer[0] = lastChar = '\0';
	lerMesmo = 0;
	while ((lerMesmo < bufferSize) && (streamReader.CanRead() == true))
	{
		if (streamReader.ReadInt8(curChar) == false)
			break;

		if (curChar == 10 || curChar == 13)
			break;

		if (curChar == '#')
		{
			HorseRadish::hInt8 commentChar;

			while (streamReader.CanRead() == true)
			{
				if (streamReader.ReadInt8(commentChar) == false)
					break;
				if (commentChar == 10 || commentChar == 13)
					break;
			}
			break;
		}

		if (curChar == 9)
			continue;

		if (curChar == ' ' && (lastChar == ' ' || lerMesmo == 0))
			continue;

		buffer[lerMesmo++] = lastChar = curChar;
	}

	buffer[lerMesmo] = '\0';

	if (lerMesmo == 0)
		return leLinha(streamReader, buffer, bufferSize);

	return true;
}

static
bool backupLinha(HorseRadish::Streams::StreamReader &streamReader)
{
	streamReader.Seek(-2, HorseRadish::Streams::Stream::Current);

	while(streamReader.GetPosition() > 0)
	{
		HorseRadish::hInt8 charRead;

		if (streamReader.ReadInt8(charRead) == false)
			break;
		if ((charRead == 10) || (charRead == 13))
			break;
		streamReader.Seek(-2, HorseRadish::Streams::Stream::Current);
	}
		
	return true;
}

static
void qsortFacesPerGroup(OBJ::FACE * const arrayBase, const int left, const int right)
{
	int i, j;

	//localidade para x e y
	{
		OBJ::FACE x, y;

		memcpy(&x, &arrayBase[(left + right) / 2], sizeof(OBJ::FACE));
		i = left;
		j = right;
		do{
			while ((arrayBase[i].group > x.group) && (i<right))
				i++;
			while ((x.group> arrayBase[j].group) && (j > left))
				j--;
			if (i <= j)
			{
				memcpy(&y, &arrayBase[i], sizeof(OBJ::FACE));
				memcpy(&arrayBase[i], &arrayBase[j], sizeof(OBJ::FACE));
				memcpy(&arrayBase[j], &y, sizeof(OBJ::FACE));
				i++;
				j--;
			}
		} while (i <= j);

	}

	if (left < j)
		qsortFacesPerGroup(arrayBase, left, j);
	if (i < right)
		qsortFacesPerGroup(arrayBase, i, right);
}

static
bool readFacesAndDoMesh(HorseRadish::Streams::StreamReader &streamReader, Geometry::Model * const modelo, OBJ * const obj, const int * const accumNum, const char * const objName)
{
	if (modelo == nullptr || obj == nullptr || obj->vertex == nullptr || obj->numVertex <= 0)
		return false;

	if (readFaces(streamReader, obj, accumNum) == false)
		return false;

	auto meshIndex = modelo->arrayMesh.size();
	modelo->arrayMesh.push_back(Geometry::Model::MeshData());
	modelo->arrayMesh[meshIndex].meshName[0] = '\0';
	modelo->arrayMesh[meshIndex].materialName[0] = '\0';

	auto mesh = &modelo->arrayMesh[meshIndex].mesh;

	if (objName != nullptr && objName[0] != '\0')
		strcpy(modelo->arrayMesh[meshIndex].meshName, objName);

	memset(mesh, 0, sizeof(Geometry::Mesh));

	doMesh(obj, mesh);

	if (mesh->FindAttribData(Geometry::Mesh::Normal) == nullptr)
		mesh->NewAttrib(Geometry::Mesh::Normal, mesh->GetNumElements());

	mesh->Ortho(HorseRadish::Geometry::Mesh::MeshOrthoCreateNormals);

	if (mesh->FindAttribData(Geometry::Mesh::TexCoords) == nullptr)
		mesh->NewAttrib(Geometry::Mesh::TexCoords, mesh->GetNumElements());

	return true;
}

static
bool doMesh(OBJ * const obj, Geometry::Mesh *mesh)
{
	int stride, curVertex, numVertex, numNewIndices, numAVertex, numANewIndices, curIndex, curIndexT;
	float *meshData;
	unsigned int *newIndices;

	if (obj == nullptr || mesh == nullptr || obj->numFaces <= 0)
		return nullptr;

	auto cacheIndex = reinterpret_cast<INDEX_SORT*>(malloc(sizeof(INDEX_SORT)*obj->numFaces * 3));
	if (cacheIndex == nullptr)
		return false;

	qsortFacesPerGroup(obj->faces, 0, obj->numFaces - 1);

	stride = 3;
	if (obj->numtCoord > 0)
		stride += 2;

	curVertex = numVertex = numAVertex = numNewIndices = numANewIndices = 0;
	meshData = nullptr;
	newIndices = nullptr;

	for (OBJ::FACE *curFace = obj->faces, *lastFace = obj->faces; (lastFace - obj->faces) < obj->numFaces; lastFace = curFace)
	{
		for (; (curFace - obj->faces) < obj->numFaces; curFace++)
		{
			if (curFace->group != lastFace->group)
				break;

			if (curFace->index[0] == curFace->index[1] || curFace->index[0] == curFace->index[2] || curFace->index[1] == curFace->index[2])
				continue;

			if (numNewIndices >= numANewIndices)
			{
				newIndices = (unsigned int*)realloc(newIndices, sizeof(unsigned int)*(numNewIndices + 24));
				numANewIndices += 24;
			}

			for (int i = 0; i < 3; i++)
			{
				if (curFace->index[i] < 0)
					curFace->index[i] = obj->numVertex + curFace->index[i] + 1;
				if (curFace->indexT[i] < 0)
					curFace->indexT[i] = obj->numtCoord + curFace->indexT[i] + 1;

				if (curFace->index[i] < 0 || curFace->index[i] >= obj->numVertex)
					continue;

				auto achou = false;

				curIndex = curFace->index[i] * 3;
				if (obj->numtCoord > 0)
					curIndexT = curFace->indexT[i] * 2;

				auto findIndex = cacheIndex;

				if (obj->numtCoord <= 0)
				{
					for (int j = 0; j < numVertex - curVertex; j++, findIndex++)
					{
						if (findIndex->index != curIndex)
							continue;

						achou = true;
						break;
					}
				}
				else
				{
					for (int j = 0; j < numVertex - curVertex; j++, findIndex++)
					{
						if (findIndex->index != curIndex || findIndex->indexT != curIndexT)
							continue;

						achou = true;
						break;
					}
				}

				if (achou == true)
				{
					newIndices[numNewIndices] = findIndex->pos;
					numNewIndices++;
					continue;
				}

				cacheIndex[numVertex - curVertex].index = curIndex;
				if (obj->numtCoord > 0)
					cacheIndex[numVertex - curVertex].indexT = curIndexT;
				cacheIndex[numVertex - curVertex].pos = numVertex;

				if (numVertex >= numAVertex)
				{
					meshData = (float*)realloc(meshData, sizeof(float)*((numVertex + 10)*stride + stride));
					numAVertex += 10;
				}

				meshData[numVertex*stride + 0] = obj->vertex[curIndex + 0];
				meshData[numVertex*stride + 1] = obj->vertex[curIndex + 1];
				meshData[numVertex*stride + 2] = obj->vertex[curIndex + 2];
				if (obj->numtCoord > 0)
				{
					meshData[numVertex*stride + 3] = obj->tCoord[curIndexT + 0];
					meshData[numVertex*stride + 4] = obj->tCoord[curIndexT + 1];
				}

				newIndices[numNewIndices++] = numVertex;
				numVertex++;
			}
		}

		curVertex = numVertex;
	}

	free(cacheIndex);

	mesh->SetNewIndices(Geometry::Mesh::Int32, numNewIndices, newIndices);

	if (stride == 3)
	{
		auto novosAttrib = new Geometry::Mesh::Attribute();
		novosAttrib[0].attribData = meshData;
		novosAttrib[0].attribType = Geometry::Mesh::Pos;

		mesh->SetVerticesData(1, numVertex, 0, novosAttrib);
		return true;
	}

	auto novosAttrib = new Geometry::Mesh::Attribute[2];
	novosAttrib[0].attribData = meshData;
	novosAttrib[0].attribType = Geometry::Mesh::Pos;
	novosAttrib[1].attribData = meshData + 3;
	novosAttrib[1].attribType = Geometry::Mesh::TexCoords;

	mesh->SetVerticesData(2, numVertex, sizeof(float) * 5, novosAttrib);
	return true;
}

static
void parseFacesTri(OBJ::FACE * const face, OBJ * const object, const char * const linha, const int * const accumNum)
{
	int trash[3];

	if (object->numVertex > 0 && object->numtCoord > 0 && object->numNormals > 0)
		sscanf(linha, "f %d/%d/%d %d/%d/%d %d/%d/%d", &face->index[0], &face->indexT[0], &trash[0], &face->index[1], &face->indexT[1], &trash[1], &face->index[2], &face->indexT[2], &trash[2]);
	else if (object->numVertex > 0 && object->numtCoord > 0)
		sscanf(linha, "f %d/%d %d/%d %d/%d", &face->index[0], &face->indexT[0], &face->index[1], &face->indexT[1], &face->index[2], &face->indexT[2]);
	else if (object->numVertex > 0 && object->numNormals > 0)
		sscanf(linha, "f %d/%d %d/%d %d/%d", &face->index[0], &trash[0], &face->index[1], &trash[1], &face->index[2], &trash[2]);
	else
		sscanf(linha, "f %d %d %d", &face->index[0], &face->index[1], &face->index[2]);

	object->numFaces++;

	face->index[0] -= accumNum[0];
	face->index[1] -= accumNum[0];
	face->index[2] -= accumNum[0];
	face->indexT[0] -= accumNum[1];
	face->indexT[1] -= accumNum[1];
	face->indexT[2] -= accumNum[1];
}

static
void parseFacesQuad(OBJ::FACE * const face, OBJ * const object, const char * const linha, const int * const accumNum)
{
	int trash[4], index[4], indexT[4];

	auto faceN = face + 1;
	memset(index, 0, sizeof(int) * 4);
	memset(indexT, 0, sizeof(int) * 4);

	if (object->numVertex > 0 && object->numtCoord > 0 && object->numNormals > 0)
		sscanf(linha, "f %d/%d/%d %d/%d/%d %d/%d/%d %d/%d/%d", &index[0], &indexT[0], &trash[0], &index[1], &indexT[1], &trash[1], &index[2], &indexT[2], &trash[2], &index[3], &indexT[3], &trash[3]);
	else if (object->numVertex > 0 && object->numtCoord > 0)
		sscanf(linha, "f %d/%d %d/%d %d/%d %d/%d", &index[0], &indexT[0], &index[1], &indexT[1], &index[2], &indexT[2], &index[3], &indexT[3]);
	else if (object->numVertex > 0 && object->numNormals > 0)
		sscanf(linha, "f %d/%d %d/%d %d/%d %d/%d", &index[0], &trash[0], &index[1], &trash[1], &index[2], &trash[2], &index[3], &trash[3]);
	else
		sscanf(linha, "f %d %d %d %d", &index[0], &index[1], &index[2], &index[3]);

	face->index[0] = index[2] - accumNum[0];
	face->index[1] = index[1] - accumNum[0];
	face->index[2] = index[0] - accumNum[0];
	faceN->index[0] = face->index[0];
	faceN->index[1] = face->index[2];
	faceN->index[2] = index[3] - accumNum[0];

	if (object->numtCoord > 0)
	{
		face->indexT[0] = indexT[2] - accumNum[1];
		face->indexT[1] = indexT[1] - accumNum[1];
		face->indexT[2] = indexT[0] - accumNum[1];
		faceN->indexT[0] = face->indexT[0];
		faceN->indexT[1] = face->indexT[2];
		faceN->indexT[2] = indexT[3] - accumNum[1];
	}

	object->numFaces += 2;
}

static
int contaEspacos(const char * const linha)
{
	auto count = 0;
	auto walker = linha;

	for (; *walker != '\0' && *walker == ' '; walker++);

	char ultimo = 0;
	for (; *walker != '\0'; walker++)
	{
		if (*walker == ultimo)
			continue;

		if (*walker == ' ')
			count++;
		ultimo = *walker;
	}

	if (ultimo == ' ')
		count--;
	return count;
}

static
bool readFaces(HorseRadish::Streams::StreamReader &streamReader, OBJ * const object, const int * const accumNum)
{
	int curGroup, espacos;
	char linha[512];
	bool sucesso;

	if (object == nullptr)
		return false;

	curGroup = 0;
	sucesso = true;

	linha[0] = '\0';
	while (sucesso && (linha[0] == ' ' || linha[0] == '\0' || (linha[0] != 'f' && linha[0] != 's'))){
		sucesso = leLinha(streamReader, linha, 512);
	}

	while (linha[0] == 'f' || linha[0] == 's')
	{
		if (linha[0] == 's')
		{
			sscanf(linha, "s %d", &curGroup);
			sucesso = leLinha(streamReader, linha, 512);
			continue;
		}

		espacos = contaEspacos(linha);

		if (espacos != 3 && espacos != 4)
		{
			sucesso = leLinha(streamReader, linha, 512);
			continue;
		}

		if (espacos == 3)
		{
			if (object->numFaces >= object->numAFaces)
			{
				object->faces = (OBJ::FACE*)realloc(object->faces, sizeof(OBJ::FACE)*(object->numAFaces + 10));
				object->numAFaces += 10;
			}

			object->faces[object->numFaces].group = curGroup;

			parseFacesTri(object->faces + object->numFaces, object, linha, accumNum);
		}
		else
		{
			if ((object->numFaces + 1) >= object->numAFaces)
			{
				object->faces = (OBJ::FACE*)realloc(object->faces, sizeof(OBJ::FACE)*(object->numAFaces + 10));
				object->numAFaces += 10;
			}

			object->faces[object->numFaces].group = curGroup;
			object->faces[object->numFaces + 1].group = curGroup;

			parseFacesQuad(object->faces + object->numFaces, object, linha, accumNum);
		}

		sucesso = leLinha(streamReader, linha, 512);
	}

	return true;
}

static
bool readVertices(HorseRadish::Streams::StreamReader &streamReader, OBJ * const object, char *linha)
{
	if (object==nullptr || linha==nullptr || linha[0]!='v')
		return false;

	if (object->vertex)
		free(object->vertex);
	object->vertex=nullptr;
	object->numVertex=0;
	object->numAVertex=0;

	while(linha[0]=='v' && linha[1]==' ')
	{
		if (object->numVertex>=object->numAVertex)
		{
			object->vertex=(float*)realloc(object->vertex,sizeof(float)*3*(object->numAVertex+10));
			object->numAVertex+=10;
		}

		sscanf(linha,"v %f %f %f",&object->vertex[object->numVertex*3+2],&object->vertex[object->numVertex*3+1],&object->vertex[object->numVertex*3+0]);
		object->vertex[object->numVertex*3+2]=-object->vertex[object->numVertex*3+2];
		object->numVertex++;

		leLinha(streamReader,linha,512);
	}

	return true;
}

static
bool readTexCoords(HorseRadish::Streams::StreamReader &streamReader, OBJ * const object, char *linha)
{
	if (object == nullptr || linha == nullptr || linha[0] != 'v' || linha[1] != 't')
		return false;

	if (object->tCoord)
		free(object->tCoord);
	object->tCoord = nullptr;
	object->numtCoord = 0;
	object->numAtCoord = 0;

	while (linha[0] == 'v' && linha[1] == 't')
	{
		if (object->numtCoord >= object->numAtCoord)
		{
			object->tCoord = (float*)realloc(object->tCoord, sizeof(float) * 2 * (object->numAtCoord + 10));
			object->numAtCoord += 10;
		}

		sscanf(linha, "vt %f %f", &object->tCoord[object->numtCoord * 2 + 0], &object->tCoord[object->numtCoord * 2 + 1]);

		object->numtCoord++;

		leLinha(streamReader, linha, 512);
	}

	return true;
}

static
bool readNormals(HorseRadish::Streams::StreamReader &streamReader, OBJ * const object, char *linha)
{
	if (object == nullptr || linha == nullptr || linha[0] != 'v' || linha[1] != 'n')
		return false;

	while (linha[0] == 'v' && linha[1] == 'n')
	{
		object->numNormals++;
		leLinha(streamReader, linha, 512);
	}

	return true;
}

Geometry::Model* mfReadOBJ(HorseRadish::Streams::StreamReader &streamReader)
{
	auto modeloReturn = new Geometry::Model();
	if (modeloReturn == nullptr)
		return nullptr;

	int accumNum[3];
	bool sucesso;
	OBJ object;
	char linha[512], nomeObj[66];

	sucesso = true;
	accumNum[0] = accumNum[1] = accumNum[2] = 1;
	while (sucesso)
	{
		memset(&object, 0, sizeof(OBJ));

		sucesso = leLinha(streamReader, linha, 512);

		nomeObj[0] = '\0';
		while (sucesso)
		{
			if (linha[0] == ' ' || linha[0] == '\0' || linha[0] == 'f' || linha[0] == 's')
				break;

			while (sucesso && (linha[0] == ' ' || linha[0] == '\0'))
				sucesso = leLinha(streamReader, linha, 512);

			if (linha[0] == 'g')
			{
				strcpy(nomeObj, linha + 2);
				sucesso = leLinha(streamReader, linha, 512);
				continue;
			}

			if (linha[0] == 'v' && linha[1] == ' ')				sucesso = readVertices(streamReader, &object, linha);
			else if (linha[0] == 'v' && linha[1] == 't')		sucesso = readTexCoords(streamReader, &object, linha);
			else if (linha[0] == 'v' && linha[1] == 'n')		sucesso = readNormals(streamReader, &object, linha);
			else sucesso = leLinha(streamReader, linha, 512);
		}

		if (sucesso == false)
			break;

		backupLinha(streamReader);
		sucesso = readFacesAndDoMesh(streamReader, modeloReturn, &object, accumNum, nomeObj);

		accumNum[0] += object.numVertex;
		accumNum[1] += object.numtCoord;
		accumNum[2] += object.numNormals;

		if (object.vertex)		free(object.vertex);
		if (object.tCoord)		free(object.tCoord);
	}

	return modeloReturn;
}