#include "common\Platform.hpp"
#include "common\Matrix.hpp"
#include "common\Mesh.hpp"
#include "common\MeshFactory.hpp"

#include "libs\tinyxml2\tinyxml2.h"

#include <vector>
#include <algorithm>

using namespace HorseRadish;

struct COLLADA_SOURCE{
	float *values;
	int numValues, numComponents, numElements;
	const char *nome;
};

struct COLLADA_INPUT{
	const char *semantic, *source;
	int offset;
};

struct COLLADA_VERTICES{
	const char *name;
	COLLADA_INPUT *inputs;
	int numInputs;
};

struct COLLADA_TRIANGLES{
	COLLADA_INPUT *inputs;
	const char *material;
	int *indices;
	int numInputs, numIndices, numTri, strideOffset;
};

struct COLLADA_POLYLIST{
	COLLADA_INPUT *inputs;
	const char *material;
	int *indices, *vcount;
	int numInputs, numIndices, numPolygons;
};

struct GEOM_ID_PAIR{
	const char *geomID;
	unsigned int meshIndex;
};

struct VERTEX_INDEX_PAIR{
	int indexVertex, indexUV;
	int indexOld, indexNew;
};

static
const GEOM_ID_PAIR* findGeomID(const char * const geomID, const GEOM_ID_PAIR * const pairList, const int numPair)
{
	if ((geomID == nullptr) || (pairList == nullptr) || (numPair <= 0))
		return nullptr;

	for (int curPair = 0; curPair < numPair; curPair++)
	{
		if (strcmp(pairList[curPair].geomID, geomID) == 0)
			return (pairList + curPair);
	}

	return nullptr;
}

static
int readValuesInt(int * const arrayInt, const int numInt, const char * const attribValues)
{
	if (arrayInt == nullptr || numInt <= 0 || attribValues == nullptr || *attribValues == '\0')
		return 0;

	memset(arrayInt, 0, sizeof(int)*numInt);

	auto walker = attribValues;

	while (*walker != '\0' && *walker == ' ')
		walker++;

	int curInt = 0;
	for (; curInt < numInt; curInt++)
	{
		if (*walker == '\0')
			break;

		arrayInt[curInt] = atoi(walker);

		while (*walker != '\0' && *walker != ' ')
			walker++;

		if (*walker == ' ')
			walker++;
	}

	return curInt;
}

double strtod(const char *str)
{
	double number;
	int exponent;
	int negative;
	char *p = (char *)str;
	double p10;
	int n;
	int num_digits;
	int num_decimals;

	// Skip leading whitespace
	while (isspace(*p)) p++;

	// Handle optional sign
	negative = 0;
	switch (*p)
	{
	case '-': negative = 1; // Fall through to increment position
	case '+': p++;
	}

	number = 0.;
	exponent = 0;
	num_digits = 0;
	num_decimals = 0;

	// Process string of digits
	while (isdigit(*p))
	{
		number = number * 10. + (*p - '0');
		p++;
		num_digits++;
	}

	// Process decimal part
	if ((*p == '.') || (*p == ','))
	{
		p++;

		while (isdigit(*p))
		{
			number = number * 10. + (*p - '0');
			p++;
			num_digits++;
			num_decimals++;
		}

		exponent -= num_decimals;
	}

	if (num_digits == 0)
	{
		errno = ERANGE;
		return 0.0;
	}

	// Correct for sign
	if (negative) number = -number;

	// Process an exponent string
	if (*p == 'e' || *p == 'E')
	{
		// Handle optional sign
		negative = 0;
		switch (*++p)
		{
		case '-': negative = 1;   // Fall through to increment pos
		case '+': p++;
		}

		// Process string of digits
		n = 0;
		while (isdigit(*p))
		{
			n = n * 10 + (*p - '0');
			p++;
		}

		if (negative)
			exponent -= n;
		else
			exponent += n;
	}

	if (exponent < DBL_MIN_EXP || exponent > DBL_MAX_EXP)
	{
		errno = ERANGE;
		return HUGE_VAL;
	}

	// Scale the result
	p10 = 10.;
	n = exponent;
	if (n < 0) n = -n;
	while (n)
	{
		if (n & 1)
		{
			if (exponent < 0)
				number /= p10;
			else
				number *= p10;
		}
		n >>= 1;
		p10 *= p10;
	}

	if (number == HUGE_VAL)
		errno = ERANGE;

	return number;
}

static
void readValuesFloat(float * const arrayFloat, const int numFloat, const char * const attribValues)
{
	if (arrayFloat == nullptr || numFloat <= 0 || attribValues == nullptr || *attribValues == '\0')
		return;

	memset(arrayFloat, 0, sizeof(float)*numFloat);

	auto walker = attribValues;

	while (*walker != '\0' && *walker == ' ')
		walker++;

	for (int curFloat = 0; curFloat < numFloat; curFloat++)
	{
		if (*walker == '\0')
			break;

		//arrayFloat[curFloat]=(float)atof(walker);
		arrayFloat[curFloat] = (float)strtod(walker);

		while (*walker != '\0' && *walker != ' ')
			walker++;
		if (*walker == ' ')
			walker++;
	}
}

static
const COLLADA_VERTICES* findVertex(const char * const vertexName, const COLLADA_VERTICES * const colVertices, const int colNumVertices)
{
	if ((colVertices == nullptr) || (colNumVertices <= 0))
		return nullptr;

	for (int curVertex = 0; curVertex < colNumVertices; curVertex++)
	{
		if (strcmp(colVertices[curVertex].name, vertexName + 1) == 0)
			return (colVertices + curVertex);
	}

	return nullptr;
}

static
const COLLADA_INPUT* findInput(const char * const inputSemantic, const COLLADA_INPUT * const colInputs, const int colNumInputs)
{
	if ((colInputs == nullptr) || (colNumInputs <= 0))
		return nullptr;

	for (int curInput = 0; curInput < colNumInputs; curInput++)
	{
		if (strcmp(colInputs[curInput].semantic, inputSemantic) == 0)
			return (colInputs + curInput);
	}

	return nullptr;
}

static
const COLLADA_SOURCE* findSource(const char * const sourceName, const COLLADA_SOURCE * const colSources, const int colNumSources)
{
	if ((colSources == nullptr) || (colNumSources <= 0))
		return nullptr;

	for (int curSource = 0; curSource < colNumSources; curSource++)
	{
		if (strcmp(colSources[curSource].nome, sourceName + 1) == 0)
			return (colSources + curSource);
	}

	return nullptr;
}

static
void criaModelo(Geometry::Model * const modelo, const char *geomName, const COLLADA_SOURCE * const colSources, const COLLADA_VERTICES * const colVertices, const COLLADA_TRIANGLES * const colTriangles, const int colNumSources, const int colNumVertices, const int colNumTriangles)
{
	if ((modelo == nullptr) || (colSources == nullptr) || (colTriangles == nullptr) || (colNumSources <= 0) || (colNumTriangles <= 0))
		return;

	auto meshArray = new Geometry::Mesh[colNumTriangles];

	for (int curTriIndex = 0; curTriIndex < colNumTriangles; curTriIndex++)
	{
		const COLLADA_SOURCE *sourcePos, *sourceUV;
		int inputOffsetPos, inputOffsetUV;
		int numPairTotal, numPairGood, lastUsedIndex;

		auto curTri = colTriangles + curTriIndex;

		sourcePos = sourceUV = nullptr;
		inputOffsetPos = inputOffsetUV = -1;

		auto curInput = findInput("VERTEX", curTri->inputs, curTri->numInputs);
		if (curInput != nullptr)
		{
			sourcePos = findSource(curInput->source, colSources, colNumSources);
			inputOffsetPos = curInput->offset;
			if (sourcePos == nullptr)
			{
				auto vertexElement = findVertex(curInput->source, colVertices, colNumVertices);
				if (vertexElement != nullptr)
				{
					curInput = findInput("POSITION", vertexElement->inputs, vertexElement->numInputs);
					if (curInput != nullptr)
					{
						sourcePos = findSource(curInput->source, colSources, colNumSources);
						inputOffsetPos = curInput->offset;
					}
				}
			}
		}

		curInput = findInput("TEXCOORD", curTri->inputs, curTri->numInputs);
		if (curInput != nullptr)
		{
			sourceUV = findSource(curInput->source, colSources, colNumSources);
			inputOffsetUV = curInput->offset;
		}

		if (sourcePos == nullptr)
			continue;

		if (sourceUV == nullptr)
		{
			meshArray[curTriIndex].NewAttrib(Geometry::Mesh::Pos, sourcePos->numElements);

			auto newIndices = reinterpret_cast<unsigned int*>(meshArray[curTriIndex].CreateNewIndices(Geometry::Mesh::Int32, curTri->numTri * 3));

			memcpy(meshArray[curTriIndex].FindAttribIndex(0), sourcePos->values, sizeof(float)*sourcePos->numElements * 3);

			for (int curIndex = 0; curIndex < meshArray[curTriIndex].GetNumIndices(); curIndex++)
				newIndices[curIndex] = (unsigned int)(curTri->indices[curTri->strideOffset*curIndex + inputOffsetPos]);

			continue;
		}

		numPairTotal = curTri->numTri * 3;

		std::vector<VERTEX_INDEX_PAIR> pares;
		pares.resize(numPairTotal);

		for (int curPairIndex = 0; curPairIndex < numPairTotal; curPairIndex++)
		{
			pares[curPairIndex].indexOld = curPairIndex;
			pares[curPairIndex].indexNew = -1;
			pares[curPairIndex].indexVertex = curTri->indices[curTri->strideOffset*curPairIndex + inputOffsetPos];
			pares[curPairIndex].indexUV = curTri->indices[curTri->strideOffset*curPairIndex + inputOffsetUV];
		}

		std::sort(pares.begin(), pares.end(), [](const VERTEX_INDEX_PAIR& parA, const VERTEX_INDEX_PAIR& parB) {

			if (parA.indexVertex == parB.indexVertex)
				return (parA.indexUV < parB.indexUV);

			return (parA.indexVertex < parB.indexVertex);
		});

		numPairGood = 0;
		for (int curPairIndex = 0; curPairIndex < numPairTotal;)
		{
			int curPairIndexNext;

			numPairGood++;

			for (curPairIndexNext = curPairIndex + 1; curPairIndexNext < numPairTotal; curPairIndexNext++)
			{
				if ((pares[curPairIndexNext].indexVertex != pares[curPairIndex].indexVertex) || (pares[curPairIndexNext].indexUV != pares[curPairIndex].indexUV))
					break;

				pares[curPairIndexNext].indexVertex = pares[curPairIndexNext].indexUV = -1;
			}

			curPairIndex = curPairIndexNext;
		}

		meshArray[curTriIndex].NewAttrib(Geometry::Mesh::Pos, numPairGood);
		meshArray[curTriIndex].NewAttrib(Geometry::Mesh::TexCoords, numPairGood);

		auto newIndices = reinterpret_cast<unsigned int*>(meshArray[curTriIndex].CreateNewIndices(Geometry::Mesh::Int32, curTri->numTri * 3));

		numPairGood = lastUsedIndex = 0;
		for (int curPairIndex = 0; curPairIndex < numPairTotal; curPairIndex++)
		{
			if ((pares[curPairIndex].indexVertex < 0) || (pares[curPairIndex].indexUV < 0))
			{
				pares[curPairIndex].indexNew = lastUsedIndex;
				continue;
			}

			lastUsedIndex = numPairGood;
			pares[curPairIndex].indexNew = lastUsedIndex;

			meshArray[curTriIndex].SetData(Geometry::Mesh::Pos, numPairGood, sourcePos->values + (pares[curPairIndex].indexVertex * sourcePos->numComponents));
			meshArray[curTriIndex].SetData(Geometry::Mesh::TexCoords, numPairGood, sourceUV->values + (pares[curPairIndex].indexUV * sourceUV->numComponents));
			numPairGood++;
		}

		std::sort(pares.begin(), pares.end(), [](const VERTEX_INDEX_PAIR& parA, const VERTEX_INDEX_PAIR& parB) { return (parA.indexOld < parB.indexOld); });

		for (int curPairIndex = 0; curPairIndex < meshArray[curTriIndex].GetNumIndices(); curPairIndex++)
			newIndices[curPairIndex] = (unsigned int)pares[curPairIndex].indexNew;
	}

	//***********************
	//can create a new mesh in the model

	if (colNumTriangles == 1)
	{
		modelo->arrayMesh.push_back(Geometry::Model::MeshData());
		auto novaMesh = &modelo->arrayMesh[modelo->arrayMesh.size() - 1];

		memset(novaMesh->materialName, 0, sizeof(novaMesh->materialName));
		memset(novaMesh->meshName, 0, sizeof(novaMesh->meshName));

		memcpy(&novaMesh->mesh, meshArray[0].Clone(), sizeof(Geometry::Mesh));
		if ((colTriangles[0].material != nullptr) && (colTriangles[0].material[0] != '\0'))
			strcpy(novaMesh->materialName, colTriangles[0].material);

		if ((geomName != nullptr) || (*geomName != '\0'))
			strcpy(novaMesh->meshName, geomName);
	}
	else
	{
		int meshAcumula = 0;

		for (int curMesh = 1; curMesh < colNumTriangles; curMesh++)
		{
			if (strcmp(colTriangles[meshAcumula].material, colTriangles[curMesh].material) != 0)
			{
				modelo->arrayMesh.push_back(Geometry::Model::MeshData());
				auto novaMesh = &modelo->arrayMesh[modelo->arrayMesh.size() - 1];

				memset(novaMesh->materialName, 0, sizeof(novaMesh->materialName));
				memset(novaMesh->meshName, 0, sizeof(novaMesh->meshName));

				memcpy(&novaMesh->mesh, meshArray[meshAcumula].Clone(), sizeof(Geometry::Mesh));

				if ((colTriangles[meshAcumula].material != nullptr) && (colTriangles[meshAcumula].material[0] != '\0'))
					strcpy(novaMesh->materialName, colTriangles[meshAcumula].material);

				if ((geomName != nullptr) || (*geomName != '\0'))
					strcpy(novaMesh->meshName, geomName);

				meshAcumula = curMesh;
				continue;
			}

			meshArray[meshAcumula].Merge(&meshArray[curMesh]);
			meshArray[curMesh].Clear();
		}

		modelo->arrayMesh.push_back(Geometry::Model::MeshData());
		auto novaMesh = &modelo->arrayMesh[modelo->arrayMesh.size() - 1];
		
		memset(novaMesh->materialName, 0, sizeof(novaMesh->materialName));
		memset(novaMesh->meshName, 0, sizeof(novaMesh->meshName));

		memcpy(&novaMesh->mesh, meshArray[meshAcumula].Clone(), sizeof(Geometry::Mesh));

		if ((colTriangles[meshAcumula].material != nullptr) && (colTriangles[meshAcumula].material[0] != '\0'))
			strcpy(novaMesh->materialName, colTriangles[meshAcumula].material);

		if ((geomName != nullptr) || (*geomName != '\0'))
			strcpy(novaMesh->meshName, geomName);
	}

	delete[] meshArray;
	meshArray = nullptr;
}

static
void leSceneGraph(Geometry::Model* const modelo, const GEOM_ID_PAIR * const geomIDList, int geomIDNum, const tinyxml2::XMLElement* xmlRootNode, const HorseRadish::Matrix &matrixStack)
{
	auto xmlNode = xmlRootNode->FirstChildElement("node");
	for (; xmlNode != nullptr; xmlNode = xmlNode->NextSiblingElement("node"))
	{
		HorseRadish::Matrix finalMat;

		auto nodeType = xmlNode->Attribute("type");
		if ((nodeType != nullptr) && (strcmp(nodeType, "NODE") != 0))
			continue;

		finalMat.SetIdentidade();

		auto xmlChild = xmlNode->FirstChild();
		for (; xmlChild != nullptr; xmlChild = xmlChild->NextSibling())
		{
			const char *elementName;

			elementName = xmlChild->Value();
			if (elementName == nullptr)
				continue;

			if (strcmp(elementName, "translate") == 0)
			{
				float transValues[3];

				auto attribValues = xmlChild->ToElement()->GetText();
				readValuesFloat(transValues, 3, attribValues);

				finalMat.MultTranslate(transValues[0], transValues[1], transValues[2]);
				continue;
			}

			if (strcmp(elementName, "rotate") == 0)
			{
				float rotateValues[4];
				HorseRadish::Matrix matRot;

				auto attribValues = xmlChild->ToElement()->GetText();
				readValuesFloat(rotateValues, 4, attribValues);

				if (HorseRadish::Math::isZero(rotateValues[3]) == true)
					continue;

				matRot.SetRotate(rotateValues[3], rotateValues[0], rotateValues[1], rotateValues[2]);
				finalMat *= matRot;
				continue;
			}

			if (strcmp(elementName, "scale") == 0)
			{
				float scaleValues[3];

				auto attribValues = xmlChild->ToElement()->GetText();
				readValuesFloat(scaleValues, 3, attribValues);

				finalMat.MultScale(scaleValues[0], scaleValues[1], scaleValues[2]);
				continue;
			}

			if (strcmp(elementName, "matrix") == 0)
			{
				HorseRadish::Matrix matAux;
				float matElements[16];

				auto attribValues = xmlChild->ToElement()->GetText();
				readValuesFloat(matElements, 16, attribValues);

				matAux.Set(matElements);
				matAux.Transpose();
				finalMat *= matAux;
				continue;
			}
		}

		finalMat.MultInverseOrder(matrixStack);

		auto xmlInstanceGeom = xmlNode->FirstChildElement("instance_geometry");
		if (xmlInstanceGeom != nullptr)
		{
			auto geomInstanceURL = xmlInstanceGeom->Attribute("url");
			auto geometria = findGeomID(geomInstanceURL + 1, geomIDList, geomIDNum);
			if (geometria != nullptr)
				modelo->arrayMesh[geometria->meshIndex].mesh.OpMatrix(Geometry::Mesh::Pos, finalMat);
		}

		leSceneGraph(modelo, geomIDList, geomIDNum, xmlNode, finalMat);
	}
}

static
void convertPolyList(COLLADA_TRIANGLES **colTriangles, int * const colNumTriangles, COLLADA_POLYLIST * const colPolylist, const int colNumPolylist)
{
	int numQuads, numTris;

	for (int curPoly = 0; curPoly < colNumPolylist; curPoly++)
	{
		int *writerWalker, *sourceWalker;

		numQuads = numTris = 0;
		for (int curVCount = 0; curVCount < colPolylist[curPoly].numPolygons; curVCount++)
		{
			if (colPolylist[curPoly].vcount[curVCount] == 4)
				numQuads++;
			else if (colPolylist[curPoly].vcount[curVCount] == 3)
				numTris++;
		}

		if ((numQuads == 0) && (numTris == 0))
			continue;

		auto newTriangle = reinterpret_cast<COLLADA_TRIANGLES*>(realloc(*colTriangles, sizeof(COLLADA_TRIANGLES)*((*colNumTriangles) + 1)));
		if (newTriangle == nullptr)
			continue;

		(*colTriangles) = newTriangle;
		newTriangle += (*colNumTriangles);
		(*colNumTriangles)++;

		memset(newTriangle, 0, sizeof(COLLADA_TRIANGLES));

		newTriangle->material = colPolylist[curPoly].material;
		newTriangle->inputs = colPolylist[curPoly].inputs;
		newTriangle->numInputs = colPolylist[curPoly].numInputs;

		newTriangle->numTri = numQuads * 2 + numTris;
		newTriangle->numIndices = newTriangle->numTri * 3 * newTriangle->numInputs;
		newTriangle->indices = (int*)malloc(sizeof(int)*newTriangle->numIndices);
		if (newTriangle->indices == nullptr)
		{
			memset(newTriangle, 0, sizeof(COLLADA_TRIANGLES));
			continue;
		}

		writerWalker = newTriangle->indices;
		sourceWalker = colPolylist[curPoly].indices;
		for (int curVCount = 0; curVCount < colPolylist[curPoly].numPolygons; curVCount++)
		{
			if ((colPolylist[curPoly].vcount[curVCount] != 3) && (colPolylist[curPoly].vcount[curVCount] != 4))
			{
				sourceWalker += colPolylist[curPoly].numInputs*colPolylist[curPoly].vcount[curVCount];
				continue;
			}

			memcpy(writerWalker, sourceWalker, sizeof(int)*colPolylist[curPoly].numInputs * 3);
			writerWalker += colPolylist[curPoly].numInputs * 3;
			sourceWalker += colPolylist[curPoly].numInputs * 3;

			if (colPolylist[curPoly].vcount[curVCount] == 3)
			{
				numTris--;
				continue;
			}

			memcpy(writerWalker, sourceWalker - (colPolylist[curPoly].numInputs) * 3, sizeof(int)*colPolylist[curPoly].numInputs);
			writerWalker += colPolylist[curPoly].numInputs;
			memcpy(writerWalker, sourceWalker - (colPolylist[curPoly].numInputs) * 1, sizeof(int)*colPolylist[curPoly].numInputs);
			writerWalker += colPolylist[curPoly].numInputs;
			memcpy(writerWalker, sourceWalker, sizeof(int)*colPolylist[curPoly].numInputs);
			writerWalker += colPolylist[curPoly].numInputs;
			sourceWalker += colPolylist[curPoly].numInputs;

			numQuads--;
		}

		if (colPolylist[curPoly].vcount != nullptr)
			free(colPolylist[curPoly].vcount);
		if (colPolylist[curPoly].indices != nullptr)
			free(colPolylist[curPoly].indices);
		memset(colPolylist + curPoly, 0, sizeof(COLLADA_POLYLIST));

		newTriangle->strideOffset = 0;
		for (int i = 0; i<newTriangle->numInputs; i++)
		{
			if (newTriangle->inputs[i].offset > newTriangle->strideOffset)
				newTriangle->strideOffset = newTriangle->inputs[i].offset;
		}
		newTriangle->strideOffset++;
	}
}

static
void readInputs(const tinyxml2::XMLElement* xmlSourceElement, COLLADA_INPUT **inputList, int * const numInputs)
{
	auto xmlInput = xmlSourceElement->FirstChildElement("input");
	for (; xmlInput != nullptr; xmlInput = xmlInput->NextSiblingElement("input"))
	{
		auto newInput = reinterpret_cast<COLLADA_INPUT*>(realloc((*inputList), sizeof(COLLADA_INPUT)*((*numInputs) + 1)));
		if (newInput == nullptr)
			continue;

		(*inputList) = newInput;
		newInput += (*numInputs);
		(*numInputs)++;

		memset(newInput, 0, sizeof(COLLADA_INPUT));

		newInput->semantic = xmlInput->Attribute("semantic");
		newInput->source = xmlInput->Attribute("source");
		auto attribData = xmlInput->Attribute("offset");
		if (attribData != nullptr)
			newInput->offset = atoi(attribData);
	}
}

static
void parseCollada(Geometry::Model * const modelo, const tinyxml2::XMLDocument * const xmlDoc)
{
	const char *axisValue;
	GEOM_ID_PAIR *geomIDList;
	int geomIDNum;

	geomIDList = nullptr;
	geomIDNum = 0;

	auto xmlMain = xmlDoc->FirstChildElement("COLLADA");
	if (xmlMain == nullptr)
		return;

	axisValue = nullptr;
	auto xmlAsset = xmlMain->FirstChildElement("asset");
	if (xmlAsset != nullptr)
	{
		auto xmlAxis = xmlAsset->FirstChildElement("up_axis");
		if (xmlAxis != nullptr)
			axisValue = xmlAxis->ToElement()->GetText();
	}

	auto xmlLibGeom = xmlMain->FirstChildElement("library_geometries");
	if (xmlLibGeom == nullptr)
		return;

	auto xmlGeometry = xmlLibGeom->FirstChildElement("geometry");
	for (; xmlGeometry != nullptr; xmlGeometry = xmlGeometry->NextSiblingElement("geometry"))
	{
		COLLADA_SOURCE *colSources;
		COLLADA_VERTICES *colVertices;
		COLLADA_TRIANGLES *colTriangles;
		COLLADA_POLYLIST *colPolylist;
		int colNumSources, colNumVertices, colNumTriangles, colNumPolylist;

		auto geomName = xmlGeometry->Attribute("name");

		auto xmlMesh = xmlGeometry->FirstChildElement("mesh");

		colSources = nullptr;
		colVertices = nullptr;
		colTriangles = nullptr;
		colPolylist = nullptr;
		colNumSources = colNumVertices = colNumTriangles = colNumPolylist = 0;

		auto xmlSource = xmlMesh->FirstChildElement("source");
		for (; xmlSource != nullptr; xmlSource = xmlSource->NextSiblingElement("source"))
		{
			auto xmlFloatArray = xmlSource->FirstChildElement("float_array");

			auto attribData = xmlFloatArray->Attribute("count");
			if (attribData == nullptr)
				continue;

			auto numFloat = atoi(attribData);
			if (numFloat <= 0)
				continue;

			auto arrayFloat = reinterpret_cast<float*>(malloc(sizeof(float)*numFloat));
			if (arrayFloat == nullptr)
				continue;

			auto attribValues = xmlFloatArray->ToElement()->GetText();
			readValuesFloat(arrayFloat, numFloat, attribValues);

			auto newSource = reinterpret_cast<COLLADA_SOURCE*>(realloc(colSources, sizeof(COLLADA_SOURCE)*(colNumSources + 1)));
			if (newSource == nullptr)
			{
				free(arrayFloat);
				continue;
			}

			colSources = newSource;
			newSource += colNumSources;
			colNumSources++;

			memset(newSource, 0, sizeof(COLLADA_SOURCE));
			newSource->values = arrayFloat;
			newSource->numValues = numFloat;

			newSource->nome = xmlSource->Attribute("id");

			auto xlmTechnique = xmlSource->FirstChildElement("technique_common");
			auto xmlAccessor = xlmTechnique->FirstChildElement("accessor");

			attribData = xmlAccessor->Attribute("count");
			if (attribData != nullptr)
				newSource->numElements = atoi(attribData);
			attribData = xmlAccessor->Attribute("stride");
			if (attribData != nullptr)
				newSource->numComponents = atoi(attribData);
		}

		auto xmlVertice = xmlMesh->FirstChildElement("vertices");
		for (; xmlVertice != nullptr; xmlVertice = xmlVertice->NextSiblingElement("vertices"))
		{
			auto newVertex = reinterpret_cast<COLLADA_VERTICES*>(realloc(colVertices, sizeof(COLLADA_VERTICES)*(colNumVertices + 1)));
			if (newVertex == nullptr)
				continue;

			colVertices = newVertex;
			newVertex += colNumVertices;
			colNumVertices++;

			memset(newVertex, 0, sizeof(COLLADA_VERTICES));

			newVertex->name = xmlVertice->Attribute("id");

			auto xmlInput = xmlVertice->FirstChildElement("input");
			for (; xmlInput != nullptr; xmlInput = xmlInput->NextSiblingElement("input"))
			{
				auto newInput = reinterpret_cast<COLLADA_INPUT*>(realloc(newVertex->inputs, sizeof(COLLADA_INPUT)*(newVertex->numInputs + 1)));
				if (newInput == nullptr)
					continue;

				newVertex->inputs = newInput;
				newInput += newVertex->numInputs;
				newVertex->numInputs++;

				memset(newInput, 0, sizeof(COLLADA_INPUT));

				newInput->semantic = xmlInput->Attribute("semantic");
				newInput->source = xmlInput->Attribute("source");
				auto attribData = xmlInput->Attribute("offset");
				if (attribData != nullptr)
					newInput->offset = atoi(attribData);
			}
		}

		auto xmlTriangle = xmlMesh->FirstChildElement("triangles");
		for (; xmlTriangle != nullptr; xmlTriangle = xmlTriangle->NextSiblingElement("triangles"))
		{
			auto newTriangle = reinterpret_cast<COLLADA_TRIANGLES*>(realloc(colTriangles, sizeof(COLLADA_TRIANGLES)*(colNumTriangles + 1)));
			if (newTriangle == nullptr)
				continue;

			colTriangles = newTriangle;
			newTriangle += colNumTriangles;
			colNumTriangles++;

			memset(newTriangle, 0, sizeof(COLLADA_TRIANGLES));

			newTriangle->numTri = atoi(xmlTriangle->Attribute("count"));
			newTriangle->material = xmlTriangle->Attribute("material");

			readInputs(xmlTriangle, &newTriangle->inputs, &newTriangle->numInputs);

			newTriangle->strideOffset = 0;
			for (int i = 0; i<newTriangle->numInputs; i++)
			{
				if (newTriangle->inputs[i].offset > newTriangle->strideOffset)
					newTriangle->strideOffset = newTriangle->inputs[i].offset;
			}
			newTriangle->strideOffset++;

			auto xmlP = xmlTriangle->FirstChildElement("p");
			if (xmlP != nullptr)
			{
				newTriangle->numIndices = (newTriangle->numTri * newTriangle->strideOffset * 3);

				newTriangle->indices = (int*)malloc(sizeof(int)*newTriangle->numIndices);
				if (newTriangle->indices == nullptr)
					continue;

				auto attribValues = xmlP->ToElement()->GetText();
				readValuesInt(newTriangle->indices, newTriangle->numIndices, attribValues);
			}
		}

		auto xmlPolylist = xmlMesh->FirstChildElement("polylist");
		for (; xmlPolylist != nullptr; xmlPolylist = xmlPolylist->NextSiblingElement("polylist"))
		{
			auto newPolylist = reinterpret_cast<COLLADA_POLYLIST*>(realloc(colPolylist, sizeof(COLLADA_POLYLIST)*(colNumPolylist + 1)));
			if (newPolylist == nullptr)
				continue;

			colPolylist = newPolylist;
			newPolylist += colNumPolylist;
			colNumPolylist++;

			memset(newPolylist, 0, sizeof(COLLADA_POLYLIST));

			newPolylist->numPolygons = atoi(xmlPolylist->Attribute("count"));
			newPolylist->material = xmlPolylist->Attribute("material");

			readInputs(xmlPolylist, &newPolylist->inputs, &newPolylist->numInputs);

			auto xmlVCount = xmlPolylist->FirstChildElement("vcount");
			if (xmlVCount != nullptr)
			{
				newPolylist->vcount = (int*)malloc(sizeof(int)*newPolylist->numPolygons);
				if (newPolylist->vcount == nullptr)
					return;

				auto attribValues = xmlVCount->ToElement()->GetText();
				readValuesInt(newPolylist->vcount, newPolylist->numPolygons, attribValues);
			}

			auto xmlP = xmlPolylist->FirstChildElement("p");
			if (xmlP != nullptr)
			{
				newPolylist->numIndices = 0;
				for (int i = 0; i < newPolylist->numPolygons; i++)
					newPolylist->numIndices += newPolylist->vcount[i];

				newPolylist->numIndices *= newPolylist->numInputs;

				newPolylist->indices = (int*)malloc(sizeof(int)*newPolylist->numIndices);
				if (newPolylist->indices == nullptr)
					continue;

				auto attribValues = xmlP->ToElement()->GetText();
				readValuesInt(newPolylist->indices, newPolylist->numIndices, attribValues);
			}
		}

		auto xmlPolygons = xmlMesh->FirstChildElement("polygons");
		for (; xmlPolygons != nullptr; xmlPolygons = xmlPolygons->NextSiblingElement("polygons"))
		{
			auto newPolylist = reinterpret_cast<COLLADA_POLYLIST*>(realloc(colPolylist, sizeof(COLLADA_POLYLIST)*(colNumPolylist + 1)));
			if (newPolylist == nullptr)
				continue;

			colPolylist = newPolylist;
			newPolylist += colNumPolylist;
			colNumPolylist++;

			memset(newPolylist, 0, sizeof(COLLADA_POLYLIST));

			newPolylist->numPolygons = atoi(xmlPolygons->Attribute("count"));
			newPolylist->material = xmlPolygons->Attribute("material");

			readInputs(xmlPolygons, &newPolylist->inputs, &newPolylist->numInputs);

			newPolylist->vcount = (int*)malloc(sizeof(int)*newPolylist->numPolygons);
			if (newPolylist->vcount == nullptr)
				return;

			memset(newPolylist->vcount, -1, sizeof(int)*newPolylist->numPolygons);

			newPolylist->numIndices = newPolylist->numPolygons * 4 * newPolylist->numInputs;
			newPolylist->indices = (int*)malloc(sizeof(int)*newPolylist->numIndices);
			if (newPolylist->indices == nullptr)
				return;

			memset(newPolylist->indices, -1, sizeof(int)*newPolylist->numIndices);

			auto auxBufferIndices = reinterpret_cast<int*>(malloc(sizeof(int)*newPolylist->numInputs * 4));
			if (auxBufferIndices == nullptr)
				return;

			auto numPolyLidos = 0;
			auto numIndicesLidos = 0;
			auto xmlP = xmlPolygons->FirstChildElement("p");
			for (; xmlP != nullptr; xmlP = xmlP->NextSiblingElement("p"))
			{
				int numValoresLidos, poligonoVertices;

				memset(auxBufferIndices, -1, sizeof(int)*newPolylist->numInputs * 4);

				auto attribValues = xmlP->ToElement()->GetText();
				numValoresLidos = readValuesInt(auxBufferIndices, newPolylist->numInputs * 4, attribValues);

				poligonoVertices = numValoresLidos / newPolylist->numInputs;
				if ((poligonoVertices != 3) && (poligonoVertices != 4))
					continue;

				newPolylist->vcount[numPolyLidos++] = poligonoVertices;
				memcpy(newPolylist->indices + numIndicesLidos, auxBufferIndices, sizeof(int)*numValoresLidos);
				numIndicesLidos += numValoresLidos;
			}

			if (auxBufferIndices != nullptr)
				free(auxBufferIndices);
			auxBufferIndices = nullptr;

			newPolylist->numPolygons = numPolyLidos;
			newPolylist->numIndices = numIndicesLidos;
		}

		if (colNumPolylist > 0)
			convertPolyList(&colTriangles, &colNumTriangles, colPolylist, colNumPolylist);

		geomIDList = (GEOM_ID_PAIR*)realloc(geomIDList, sizeof(GEOM_ID_PAIR)*(geomIDNum + 1));
		geomIDList[geomIDNum].geomID = xmlGeometry->Attribute("id");
		geomIDList[geomIDNum].meshIndex = modelo->arrayMesh.size();
		geomIDNum++;

		criaModelo(modelo, geomName, colSources, colVertices, colTriangles, colNumSources, colNumVertices, colNumTriangles);

		for (int i = 0; i < colNumSources; i++)
		{
			if (colSources[i].values != nullptr)
				free(colSources[i].values);
		}
		for (int i = 0; i < colNumVertices; i++)
		{
			if (colVertices[i].inputs != nullptr)
				free(colVertices[i].inputs);
		}
		for (int i = 0; i < colNumTriangles; i++)
		{
			if (colTriangles[i].inputs != nullptr)
				free(colTriangles[i].inputs);
			if (colTriangles[i].indices != nullptr)
				free(colTriangles[i].indices);
		}
		for (int i = 0; i < colNumPolylist; i++)
		{
			if (colPolylist[i].inputs != nullptr)
				free(colPolylist[i].inputs);
			if (colPolylist[i].indices != nullptr)
				free(colPolylist[i].indices);
			if (colPolylist[i].vcount != nullptr)
				free(colPolylist[i].vcount);
		}

		if (colSources != nullptr)
			free(colSources);
		if (colVertices != nullptr)
			free(colVertices);
		if (colTriangles != nullptr)
			free(colTriangles);
		if (colPolylist != nullptr)
			free(colPolylist);
		colSources = nullptr;
		colVertices = nullptr;
		colTriangles = nullptr;
		colPolylist = nullptr;
		colNumSources = colNumVertices = colNumTriangles = colNumPolylist = 0;
	}

	auto xmlLibVisualScene = xmlMain->FirstChildElement("library_visual_scenes");
	if (xmlLibVisualScene != nullptr)
	{
		auto xmlVisualScene = xmlLibVisualScene->FirstChildElement("visual_scene");
		if (xmlVisualScene != 0)
		{
			HorseRadish::Matrix matrixStack;

			matrixStack.SetIdentidade();
			leSceneGraph(modelo, geomIDList, geomIDNum, xmlVisualScene, matrixStack);
		}
	}

	if (geomIDList != nullptr)
		free(geomIDList);
	geomIDList = nullptr;
	geomIDNum = 0;

	//COLLADA is right-handed, so...
	for (auto& curMesh : modelo->arrayMesh)
	{
		auto vertData = curMesh.mesh.FindAttribData(HorseRadish::Geometry::Mesh::Pos);
		if (vertData != nullptr)
		{
			float posTemp;

			auto numVertices = curMesh.mesh.GetNumElements();

			for (int curVertex = 0; curVertex < numVertices; curVertex++, vertData += 3)
			{
				posTemp = vertData[2];
				vertData[2] = -vertData[1];
				vertData[1] = posTemp;
			}
		}
	}

	if ((axisValue != nullptr) && (strcmp(axisValue, "Z_UP") == 0))
	{
		for (auto& curMesh : modelo->arrayMesh)
		{
			float auxVal;

			auto numVertices = curMesh.mesh.GetNumElements();
			auto vData = curMesh.mesh.FindAttribIndex(0);

			for (int curVertex = 0; curVertex < numVertices; curVertex++, vData += 3)
			{
				auxVal = vData[2];
				vData[2] = -vData[1];
				vData[1] = auxVal;
			}
		}
	}
}

static
tinyxml2::XMLDocument* documentOpenMem(const void *fileData, const int fileSize, char *errorDesc, const int errorBufferSize, int *errorLine)
{
	tinyxml2::XMLDocument *xmlDoc;

	if (fileData == nullptr || fileSize <= 0)
		return nullptr;

	xmlDoc = new tinyxml2::XMLDocument();
	if (xmlDoc->Parse(static_cast<const char*>(fileData), fileSize) != tinyxml2::XML_NO_ERROR)
	{
		auto errorStr = xmlDoc->GetErrorStr1();
		if (errorStr != nullptr && errorBufferSize > 0)
		{
			memset(errorDesc, 0, errorBufferSize);
			strcpy_s(errorDesc, errorBufferSize, errorStr);

			if (errorLine)
				*errorLine = -1;
		}

		xmlDoc->Clear();
		delete xmlDoc;
		return nullptr;
	}

	return xmlDoc;
}

/*§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§
§§§§§§   -= Funções para exportar =-   §§§§§
§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§*/
Geometry::Model* mfReadCollada(HorseRadish::Streams::StreamReader &streamReader)
{
	auto streamContent = streamReader.getStream().readEntireContent();

	auto xmlDoc = documentOpenMem(streamContent->getData(), streamContent->GetLength(), nullptr, 0, nullptr);
	if (xmlDoc <= 0)
		return nullptr;

	auto modelo = new Geometry::Model();
	if (modelo == nullptr)
	{
		xmlDoc->Clear();
		delete xmlDoc;

		return nullptr;
	}

	parseCollada(modelo, xmlDoc);

	xmlDoc->Clear();
	delete xmlDoc;

	return modelo;
}