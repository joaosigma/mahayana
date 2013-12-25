#include "common\Platform.hpp"
#include "common\Sorting.hpp"
#include "common\Matrix.hpp"
#include "common\Mesh.hpp"
#include "common\MeshFactory.hpp"

#include "libs\tinyXML\tinyxml.h"

#include <windows.h>
#include <stdio.h>
#include <float.h>
#include <xmmintrin.h>

using namespace HorseRadish;

/*§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§
§§§§§§   -= Estructuras e declarações necessárias =-   §§§§§
§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§*/
struct COLLADA_SOURCE{
	float *values;
	int numValues,numComponents, numElements;
	const char *nome;
};
struct COLLADA_INPUT{
	const char *semantic,*source;
	int offset;
};
struct COLLADA_VERTICES{
	const char *nome;
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
	int *indices,*vcount;
	int numInputs, numIndices, numPolygons;
};

struct GEOM_ID_PAIR{
	const char *geomID;
	unsigned int meshIndex;
};
struct VERTEX_INDEX_PAIR{
	int indexVertex,indexUV;
	int indexOld, indexNew;
};

/*§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§
§§§§§§   -= Funções locais para gerir tudo =-   §§§§§
§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§*/
static
const GEOM_ID_PAIR* findGeomID(const char * const geomID, const GEOM_ID_PAIR * const pairList, const int numPair)
{
	if ( (geomID == nullptr) || (pairList == nullptr) || (numPair <= 0))
		return nullptr;

	for(int curPair = 0; curPair < numPair; curPair++)
		{
		if (strcmp(pairList[curPair].geomID, geomID) == 0)
			return (pairList + curPair);
		}
	return nullptr;
}

static
int readValuesInt(int * const arrayInt, const int numInt, const char * const attribValues)
{
	const char *walker;
	int curInt;

	//verificar alguns parametros
	if (arrayInt==nullptr || numInt<=0 || attribValues==nullptr || *attribValues=='\0')
		return 0;

	//ao principio tudo aponta para o mesmo
	walker = attribValues;

	//limpo tudo
	memset(arrayInt,0,sizeof(int)*numInt);

	//tenho de meter o walker a apontar para o próximo valor
	while(*walker!='\0' && *walker==' ')
		walker++;

	//para cada valor que tenho de ler
	for(curInt=0; curInt<numInt; curInt++)
	{
		//algo correu mal
		if (*walker=='\0')
			break;

		//converter a string
		arrayInt[curInt]=atoi(walker);

		//avanço para o próximo valor
		while(*walker!='\0' && *walker!=' ')
			walker++;
		if (*walker==' ')
			walker++;
	}

	//li este numero de valores
	return curInt;
}

double strtod(const char *str)
{
  double number;
  int exponent;
  int negative;
  char *p = (char *) str;
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
  if ((*p == '.') || (*p == ',') )
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
    switch(*++p) 
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

  if (exponent < DBL_MIN_EXP  || exponent > DBL_MAX_EXP)
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
	const char *walker;

	//verificar alguns parametros
	if (arrayFloat==nullptr || numFloat<=0 || attribValues==nullptr || *attribValues=='\0')
		return;

	//ao principio tudo aponta para o mesmo
	walker = attribValues;

	//limpo tudo
	memset(arrayFloat,0,sizeof(float)*numFloat);

	//tenho de meter o walker a apontar para o próximo valor
	while(*walker!='\0' && *walker==' ')
		walker++;

	//para cada valor que tenho de ler
	for(int curFloat=0; curFloat<numFloat; curFloat++)
	{
		//algo correu mal
		if (*walker=='\0')
			break;

		//converter a string
		//arrayFloat[curFloat]=(float)atof(walker);
		arrayFloat[curFloat]=(float)strtod(walker);

		//avanço para o próximo valor
		while(*walker!='\0' && *walker!=' ')
			walker++;
		if (*walker==' ')
			walker++;
	}
}

static
const COLLADA_VERTICES* findVertex(const char * const vertexName, const COLLADA_VERTICES * const colVertices, const int colNumVertices)
{
	if ((colVertices == nullptr) || (colNumVertices <= 0))
		return nullptr;

	for(int curVertex = 0; curVertex < colNumVertices; curVertex++)
	{
		if (strcmp(colVertices[curVertex].nome, vertexName+1) == 0)
			return (colVertices + curVertex);
	}
	return nullptr;
}

static
const COLLADA_INPUT* findInput(const char * const inputSemantic, const COLLADA_INPUT * const colInputs, const int colNumInputs)
{
	if ((colInputs == nullptr) || (colNumInputs <= 0))
		return nullptr;

	for(int curInput = 0; curInput < colNumInputs; curInput++)
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

	for(int curSource = 0; curSource < colNumSources; curSource++)
	{
		if (strcmp(colSources[curSource].nome, sourceName+1) == 0)
			return (colSources + curSource);
	}
	return nullptr;
}

static
int qSortParIndex(const VERTEX_INDEX_PAIR& parA, const VERTEX_INDEX_PAIR& parB)
{
	if (parA.indexVertex < parB.indexVertex)
		return -1;
	if (parA.indexVertex > parB.indexVertex)
		return 1;

	if (parA.indexUV < parB.indexUV)
		return -1;
	if (parA.indexUV > parB.indexUV)
		return 1;

	return 0;
}
static
int qSortParIndexMain(const VERTEX_INDEX_PAIR& parA, const VERTEX_INDEX_PAIR& parB)
{
	if (parA.indexOld < parB.indexOld)
		return -1;
	if (parA.indexOld > parB.indexOld)
		return 1;
	return 0;
}

static
void criaModelo(Geometry::Model * const modelo, const char *geomName, const COLLADA_SOURCE * const colSources, const COLLADA_VERTICES * const colVertices, const COLLADA_TRIANGLES * const colTriangles, const int colNumSources, const int colNumVertices, const int colNumTriangles)
{
	Geometry::Mesh *meshArray;

	//verificar dados
	if ((modelo == nullptr) || (colSources == nullptr) || (colTriangles == nullptr) || (colNumSources <= 0) || (colNumTriangles <= 0))
		return;

	//crio espaço para as meshes que vou criar (1 por cada grupo de triangulos deste modelo)
	meshArray = new Geometry::Mesh[colNumTriangles];

	//para cada triangulo
	for(int curTriIndex = 0; curTriIndex < colNumTriangles; curTriIndex++)
	{
		VERTEX_INDEX_PAIR *pares;
		const COLLADA_TRIANGLES *curTri;
		const COLLADA_SOURCE *sourcePos,*sourceUV;
		const COLLADA_INPUT *curInput;
		int inputOffsetPos, inputOffsetUV;
		int numPairTotal, numPairGood, lastUsedIndex;
		unsigned int *newIndices;

		//o triangulo que estou a usar
		curTri = colTriangles + curTriIndex;

		//por defeito não tenho nada donde tirar os dados para este conjunto de triangulos
		sourcePos = sourceUV = nullptr;
		inputOffsetPos = inputOffsetUV = -1;

		//procuro pelos vertices
		curInput = findInput("VERTEX", curTri->inputs, curTri->numInputs);
		if (curInput != nullptr)
		{
			//tiro a source e se estiver a NULL, tento ir à dos vertices
			sourcePos = findSource(curInput->source, colSources, colNumSources);
			inputOffsetPos = curInput->offset;
			if (sourcePos == nullptr)
			{
				const COLLADA_VERTICES *vertexElement;

				//procuro pelo input nos vertices
				vertexElement = findVertex(curInput->source, colVertices, colNumVertices);
				if (vertexElement != nullptr)
				{
					//agora retiro o input da posicao dos inputs deste vertice e se existe, é essa source a usar para os vertices
					curInput = findInput("POSITION", vertexElement->inputs, vertexElement->numInputs);
					if (curInput != nullptr)
					{
						sourcePos = findSource(curInput->source, colSources, colNumSources);
						inputOffsetPos = curInput->offset;
					}
				}
			}
		}

		//procuro pelos UVs
		curInput = findInput("TEXCOORD", curTri->inputs, curTri->numInputs);
		if (curInput != nullptr)
		{
			//tiro a source e se estiver a NULL, tento ir à dos vertices
			sourceUV = findSource(curInput->source, colSources, colNumSources);
			inputOffsetUV = curInput->offset;
		}
		
		//***********************
		//e pronto. chegando aqui já tenho o que vou precisar, no entanto tenho de verificar se tenho posições (é obrigatório)
		if (sourcePos == nullptr)
			continue;

		//se só tenho posições, a coisa fica muito mais simpificada
		if (sourceUV == nullptr)
		{
			unsigned int *newIndices;

			//chegando aqui, numPairGood tem o número de vertices únicos para este pool de triangulos
			meshArray[curTriIndex].NewAttrib(Geometry::Mesh::Pos, sourcePos->numElements);

			//mando criar os indices
			newIndices = (unsigned int*)meshArray[curTriIndex].CreateNewIndices(Geometry::Mesh::Int32, curTri->numTri * 3);

			//copio todos os pontos de uma lista para a outra
			memcpy(meshArray[curTriIndex].FindAttribIndex(0), sourcePos->values, sizeof(float)*sourcePos->numElements*3);

			//agora basta copiar os indices
			for(int curIndex = 0; curIndex < meshArray[curTriIndex].GetNumIndices(); curIndex++)
				newIndices[curIndex] = (unsigned int)(curTri->indices[curTri->strideOffset*curIndex + inputOffsetPos]);

			//não faço mais nada
			continue;
		}

		//como tenho de fazer o modelo todos aos pares, tenho de alocar o máximo de memória para optimizar o número de vértices
		numPairTotal = curTri->numTri * 3;
		pares = (VERTEX_INDEX_PAIR*)malloc(sizeof(VERTEX_INDEX_PAIR)*numPairTotal);
		if (pares == nullptr)
			continue;

		//limpo tudo
		memset(pares, 0, sizeof(VERTEX_INDEX_PAIR)*numPairTotal);

		//basta comecar a construir os pares, ou seja, passo por todos os indices dos triangulos e crio o vertice
		for(int curPairIndex = 0; curPairIndex < numPairTotal; curPairIndex++)
		{
			pares[curPairIndex].indexOld = curPairIndex;
			pares[curPairIndex].indexNew = -1;
			pares[curPairIndex].indexVertex = curTri->indices[curTri->strideOffset*curPairIndex + inputOffsetPos];
			pares[curPairIndex].indexUV = curTri->indices[curTri->strideOffset*curPairIndex + inputOffsetUV];
		}

		//agora ordeno a lista pelos respectivos pares
		HorseRadish::Sorting::QuickSort<VERTEX_INDEX_PAIR>(pares, numPairTotal, qSortParIndex);	

		//passo por cada vertice e verifico se está duplicado
		numPairGood = 0;
		for(int curPairIndex = 0; curPairIndex < numPairTotal; )
		{
			int curPairIndexNext;

			//este par conta sempre
			numPairGood++;

			//agora só saio quando o pai for diferente
			for(curPairIndexNext = curPairIndex + 1; curPairIndexNext < numPairTotal; curPairIndexNext++)
			{		
				//se este (que está à frente) for diferente do actual, devo sair
				if ((pares[curPairIndexNext].indexVertex != pares[curPairIndex].indexVertex) || (pares[curPairIndexNext].indexUV != pares[curPairIndex].indexUV))
					break;

				//como é igual, limpo os valores e o indice fica identico ao anterior
				pares[curPairIndexNext].indexVertex = pares[curPairIndexNext].indexUV = -1;
			}

			//começo sempre onde acabei
			curPairIndex = curPairIndexNext;
		}

		//chegando aqui, numPairGood tem o número de vertices únicos para este pool de triangulos
		meshArray[curTriIndex].NewAttrib(Geometry::Mesh::Pos, numPairGood);
		meshArray[curTriIndex].NewAttrib(Geometry::Mesh::TexCoords, numPairGood);

		//mando criar os indices
		newIndices = (unsigned int*)meshArray[curTriIndex].CreateNewIndices(Geometry::Mesh::Int32, curTri->numTri * 3);

		//agora passo por cada vertice (agora par) para preencher os arrays
		numPairGood = lastUsedIndex = 0;
		for(int curPairIndex = 0; curPairIndex < numPairTotal; curPairIndex++)
		{
			//se este estava copiado
			if ((pares[curPairIndex].indexVertex < 0) || (pares[curPairIndex].indexUV < 0))
			{
				pares[curPairIndex].indexNew = lastUsedIndex;
				continue;
			}

			//este par vai ficar nesta nova posicao
			lastUsedIndex = numPairGood;
			pares[curPairIndex].indexNew = lastUsedIndex;

			//basta gravar a posição e os UVs de uma lado para o outro
			meshArray[curTriIndex].SetData(Geometry::Mesh::Pos, numPairGood, sourcePos->values + (pares[curPairIndex].indexVertex * sourcePos->numComponents));
			meshArray[curTriIndex].SetData(Geometry::Mesh::TexCoords, numPairGood, sourceUV->values + (pares[curPairIndex].indexUV * sourceUV->numComponents));
			numPairGood++;
		}

		//volto a reordenar os indices mas agora de volto à ordem antiga
		HorseRadish::Sorting::QuickSort<VERTEX_INDEX_PAIR>(pares, numPairTotal, qSortParIndexMain);

		//agora basta passar por todos os indices e copiar o indice novo para o array da mesh
		for(int curPairIndex = 0; curPairIndex < meshArray[curTriIndex].GetNumIndices(); curPairIndex++)
			newIndices[curPairIndex] = (unsigned int)pares[curPairIndex].indexNew;

		//posso apagar os pares pq já não preciso deles
		free(pares);
		pares = nullptr;
	}

	//***********************
	//posso criar uma nova mesh no modelo

	//se só tenho um conjunto de triangulos, facilita
	if (colNumTriangles == 1)
	{
		Geometry::Model::MeshData *novaMesh;

		//crio uma nova mesh e limpo os nomes
		novaMesh = modelo->arrayMesh.Add();
		memset(novaMesh->materialName, 0, sizeof(novaMesh->materialName));
		memset(novaMesh->meshName, 0, sizeof(novaMesh->meshName));

		//agora clono a primeira mesh e o nome do material pra cima da do modelo e pronto
		memcpy(&novaMesh->mesh, meshArray[0].Clone(), sizeof(Geometry::Mesh));
		if ((colTriangles[0].material != nullptr) && (colTriangles[0].material[0] != '\0'))
			strcpy(novaMesh->materialName, colTriangles[0].material);

		//se tiver um nome para esta geometria, copio-o
		if ((geomName != nullptr) || (*geomName != '\0'))
			strcpy(novaMesh->meshName, geomName);
	}
	else
	{
		int meshAcumula;
		Geometry::Model::MeshData *novaMesh;

		//por omissão, acumulo para esta mesh
		meshAcumula = 0;

		//junto todas as meshes para cima da primeira e liberto o espaço
		for(int curMesh = 1; curMesh < colNumTriangles; curMesh++)
		{
			//se o material mudou, tenho de criar uma nova mesh
			if (strcmp(colTriangles[meshAcumula].material, colTriangles[curMesh].material) != 0)
			{
				//crio uma nova mesh e limpo os nomes
				novaMesh = modelo->arrayMesh.Add();
				memset(novaMesh->materialName, 0, sizeof(novaMesh->materialName));
				memset(novaMesh->meshName, 0, sizeof(novaMesh->meshName));

				//agora clono a mesh que acumulou as outras
				memcpy(&novaMesh->mesh, meshArray[meshAcumula].Clone(), sizeof(Geometry::Mesh));

				//guardo o nome do material desta mesh
				if ((colTriangles[meshAcumula].material != nullptr) && (colTriangles[meshAcumula].material[0] != '\0'))
					strcpy(novaMesh->materialName, colTriangles[meshAcumula].material);

				//se tiver um nome para esta geometria, copio-o
				if ((geomName != nullptr) || (*geomName != '\0'))
					strcpy(novaMesh->meshName, geomName);

				//a partir de agora acumulo para esta mesh e passo para a próxima
				meshAcumula = curMesh;
				continue;
			}

			//chegando aqui, as meshs são do mesmo material, portanto acumulo-as
			meshArray[meshAcumula].Merge(&meshArray[curMesh]);
			meshArray[curMesh].Clear();
		}

		//não podendo esquecer a última mesh
		
		//crio uma nova mesh e limpo os nomes
		novaMesh = modelo->arrayMesh.Add();
		memset(novaMesh->materialName, 0, sizeof(novaMesh->materialName));
		memset(novaMesh->meshName, 0, sizeof(novaMesh->meshName));

		//agora clono a mesh que acumulou as outras
		memcpy(&novaMesh->mesh, meshArray[meshAcumula].Clone(), sizeof(Geometry::Mesh));

		//guardo o nome do material desta mesh
		if ((colTriangles[meshAcumula].material != nullptr) && (colTriangles[meshAcumula].material[0] != '\0'))
			strcpy(novaMesh->materialName, colTriangles[meshAcumula].material);

		//se tiver um nome para esta geometria, copio-o
		if ((geomName != nullptr) || (*geomName != '\0'))
			strcpy(novaMesh->meshName, geomName);
	}

	//posso apagar o array auxiliar para as meshes
	delete[] meshArray;
	meshArray = nullptr;
}

static
void leSceneGraph(Geometry::Model* const modelo, const GEOM_ID_PAIR * const geomIDList, int geomIDNum, const TiXmlElement* xmlRootNode, const HorseRadish::Matrix &matrixStack)
{
	const TiXmlElement* xmlNode;

	//para cada nó desta cena
	xmlNode = xmlRootNode->FirstChildElement("node");
	for(; xmlNode != nullptr; xmlNode = xmlNode->NextSiblingElement("node"))
	{
		const TiXmlNode* xmlChild;
		const TiXmlElement* xmlInstanceGeom;
		const char *nodeType,*attribValues;
		HorseRadish::Matrix finalMat;

		//só aceito este tipo de nós
		nodeType = xmlNode->Attribute("type");
		if ((nodeType != nullptr) && (strcmp(nodeType, "NODE") != 0))
			continue;

		//inicializo a matrix com a identidade
		finalMat.SetIdentidade();
			
		//passo por todos os elementos
		xmlChild = xmlNode->FirstChild();
		for(; xmlChild != nullptr; xmlChild = xmlChild->NextSibling())
		{
			const char *elementName;

			//tiro o nome do elemento
			elementName = xmlChild->Value();
			if (elementName == nullptr)
				continue;

			//se for uma translação
			if (strcmp(elementName, "translate") == 0)
			{
				float transValues[3];

				//leio os valores
				attribValues = xmlChild->ToElement()->GetText();
				readValuesFloat(transValues, 3, attribValues);

				//actualizo a matriz e pronto
				finalMat.MultTranslate(transValues[0], transValues[1], transValues[2]);
				continue;
			}

			//se for uma rotação
			if (strcmp(elementName, "rotate") == 0)
			{
				float rotateValues[4];
				HorseRadish::Matrix matRot;

				//leio os valores
				attribValues = xmlChild->ToElement()->GetText();
				readValuesFloat(rotateValues, 4, attribValues);

				//se por acaso isto for zero
				if (HorseRadish::Math::isZero(rotateValues[3]) == true)
					continue;
				
				//actualizo a matriz e pronto
				matRot.SetRotate(rotateValues[3], rotateValues[0], rotateValues[1], rotateValues[2]);
				finalMat *= matRot;
				continue;
			}

			//se for um scale
			if (strcmp(elementName, "scale") == 0)
			{
				float scaleValues[3];

				//leio os valores
				attribValues = xmlChild->ToElement()->GetText();
				readValuesFloat(scaleValues, 3, attribValues);
				
				//actualizo a matriz e pronto
				finalMat.MultScale(scaleValues[0], scaleValues[1], scaleValues[2]);
				continue;
			}

			//se for uma matrix
			if (strcmp(elementName, "matrix") == 0)
			{
				HorseRadish::Matrix matAux;
				float matElements[16];

				//leio os valores
				attribValues = xmlChild->ToElement()->GetText();
				readValuesFloat(matElements, 16, attribValues);
				
				//actualizo a matriz e pronto
				matAux.Set(matElements);
				matAux.Transpose();
				finalMat *= matAux;
				continue;
			}
		}

		//tenho de juntar as transformações do ultimo node
		finalMat.MultInverseOrder(matrixStack);

		//em que geometria aplico isto
		xmlInstanceGeom = xmlNode->FirstChildElement("instance_geometry");
		if (xmlInstanceGeom != nullptr)
		{
			const char *geomInstanceURL;
			const GEOM_ID_PAIR *geometria;

			//tiro a geometria em que aplico os valores
			geomInstanceURL = xmlInstanceGeom->Attribute("url");
			geometria = findGeomID(geomInstanceURL+1, geomIDList, geomIDNum);
			if (geometria != nullptr)
				modelo->arrayMesh[geometria->meshIndex].mesh.OpMatrix(Geometry::Mesh::Pos, finalMat);
		}
		
		//faço o mesmo para os nós meus filhos
		leSceneGraph(modelo, geomIDList, geomIDNum, xmlNode, finalMat);
	}
}

static
void convertPolyList(COLLADA_TRIANGLES **colTriangles, int * const colNumTriangles, COLLADA_POLYLIST * const colPolylist, const int colNumPolylist)
{
	int numQuads, numTris;

	//para cada polygono na lista
	for(int curPoly = 0; curPoly < colNumPolylist; curPoly++)
		{
		COLLADA_TRIANGLES *newTriangle;
		int *writerWalker,*sourceWalker;

		//tenho de contar quantos quads ou triangulos tenho na lista
		numQuads = numTris = 0;
		for(int curVCount=0; curVCount<colPolylist[curPoly].numPolygons; curVCount++)
			{
			if (colPolylist[curPoly].vcount[curVCount] == 4)
				numQuads++;
			else if (colPolylist[curPoly].vcount[curVCount] == 3)
				numTris++;
			}

		//agora tenho de verificar não tenho nada que possa usar
		if ((numQuads == 0) && (numTris == 0))
			continue;
		
		//crio este novo triangulo
		newTriangle = (COLLADA_TRIANGLES*)realloc(*colTriangles, sizeof(COLLADA_TRIANGLES)*((*colNumTriangles)+1));
		if (newTriangle == nullptr)
			continue;

		//arranjo o ponteiro
		(*colTriangles) = newTriangle;
		newTriangle += (*colNumTriangles);
		(*colNumTriangles)++;

		//limpo o novo trianglo
		memset(newTriangle, 0, sizeof(COLLADA_TRIANGLES));

		//algumas coisas posso já copiar (e retiro do poligono pq não quero que seja apagado)
		newTriangle->material = colPolylist[curPoly].material;
		newTriangle->inputs = colPolylist[curPoly].inputs;
		newTriangle->numInputs = colPolylist[curPoly].numInputs;

		//também já posso saber quantos triangulos tenho na realidade, e também o número de indices, pelo que posso logo criar espaço para os mesmos
		newTriangle->numTri = numQuads*2 + numTris;
		newTriangle->numIndices = newTriangle->numTri*3*newTriangle->numInputs;
		newTriangle->indices = (int*)malloc(sizeof(int)*newTriangle->numIndices);
		if (newTriangle->indices == nullptr)
			{
			memset(newTriangle, 0, sizeof(COLLADA_TRIANGLES));
			continue;
			}

		//prontos, chegado aqui basta simplesmente andar pelos indices do poligono e copiar/criar de acordo
		writerWalker = newTriangle->indices;
		sourceWalker = colPolylist[curPoly].indices;
		for(int curVCount=0; curVCount<colPolylist[curPoly].numPolygons; curVCount++)
			{
			//se não for nem quad nem tri, passo à frente desses indices
			if ((colPolylist[curPoly].vcount[curVCount] != 3) && (colPolylist[curPoly].vcount[curVCount] != 4))
				{
				sourceWalker += colPolylist[curPoly].numInputs*colPolylist[curPoly].vcount[curVCount];
				continue;
				}

			//independente do numero de indices (se quad ou tri), copio sempre os 3 primeiros indices
			memcpy(writerWalker, sourceWalker, sizeof(int)*colPolylist[curPoly].numInputs*3);
			writerWalker += colPolylist[curPoly].numInputs*3;
			sourceWalker += colPolylist[curPoly].numInputs*3;

			//se for um triangulo, escuso de fazer mais nada...
			if (colPolylist[curPoly].vcount[curVCount] == 3)
				{
				//um triangulo já tá
				numTris--;
				continue;
				}

			//chegando aqui, tenho de criar o triangulo que falta
			memcpy(writerWalker, sourceWalker-(colPolylist[curPoly].numInputs)*3, sizeof(int)*colPolylist[curPoly].numInputs);
			writerWalker += colPolylist[curPoly].numInputs;
			memcpy(writerWalker, sourceWalker-(colPolylist[curPoly].numInputs)*1, sizeof(int)*colPolylist[curPoly].numInputs);
			writerWalker += colPolylist[curPoly].numInputs;
			memcpy(writerWalker, sourceWalker, sizeof(int)*colPolylist[curPoly].numInputs);
			writerWalker += colPolylist[curPoly].numInputs;
			sourceWalker += colPolylist[curPoly].numInputs;

			//um quadrado já tá
			numQuads--;
			}

		//posso limpar todos este poligono porque foi usado com sucesso (não quero que seja apagado sem querer mais tarde: os inputs não podem ser apagados)
		if (colPolylist[curPoly].vcount != nullptr)
			free(colPolylist[curPoly].vcount);
		if (colPolylist[curPoly].indices != nullptr)
			free(colPolylist[curPoly].indices);
		memset(colPolylist+curPoly, 0, sizeof(COLLADA_POLYLIST));

		//tenho de saber de quantos em quantos inputs salto para o próximo indice do triangulo
		newTriangle->strideOffset = 0;
		for(int i=0; i<newTriangle->numInputs; i++)
			{
			if (newTriangle->inputs[i].offset > newTriangle->strideOffset)
				newTriangle->strideOffset = newTriangle->inputs[i].offset;
			}
		newTriangle->strideOffset++;
		}
}

static
void readInputs(const TiXmlElement* xmlSourceElement, COLLADA_INPUT **inputList, int * const numInputs)
{
	const TiXmlElement *xmlInput;
	const char *attribData;

	//para cada input deste trianglo
	xmlInput = xmlSourceElement->FirstChildElement("input");
	for(; xmlInput != nullptr; xmlInput = xmlInput->NextSiblingElement("input"))
		{
		COLLADA_INPUT *newInput;

		//crio este novo vertice
		newInput = (COLLADA_INPUT*)realloc((*inputList), sizeof(COLLADA_INPUT)*((*numInputs)+1));
		if (newInput == nullptr)
			continue;

		//arranjo o ponteiro
		(*inputList) = newInput;
		newInput += (*numInputs);
		(*numInputs)++;

		//limpo o novo input
		memset(newInput, 0, sizeof(COLLADA_INPUT));

		//tiro os dados do input
		newInput->semantic = xmlInput->Attribute("semantic");
		newInput->source = xmlInput->Attribute("source");
		attribData = xmlInput->Attribute("offset");
		if (attribData != nullptr)
			newInput->offset = atoi(attribData);
		}
}

static
void parseCollada(Geometry::Model * const modelo, const TiXmlDocument * const xmlDoc)
{
	const TiXmlElement *xmlMain, *xmlLibGeom, *xmlGeometry, *xmlAsset, *xmlLibVisualScene;
	const char *attribData, *attribValues, *axisValue;
	GEOM_ID_PAIR *geomIDList;
	int geomIDNum;

	//limpo isto
	geomIDList = nullptr;
	geomIDNum = 0;

	//vou até ao primeiro elemento COLLADA
	xmlMain = xmlDoc->FirstChildElement("COLLADA");
	if (xmlMain == nullptr)
		return;

	//ando até aos assets
	axisValue = nullptr;
	xmlAsset = xmlMain->FirstChildElement("asset");
	if (xmlAsset != nullptr)
	{
		const TiXmlElement *xmlAxis;

		//procuro pelo elemento que me identifica o eixo e tiro-o
		xmlAxis = xmlAsset->FirstChildElement("up_axis");
		if (xmlAxis != nullptr)
			axisValue = xmlAxis->ToElement()->GetText();
	}

	//ando até à biblioteca de geometrias
	xmlLibGeom = xmlMain->FirstChildElement("library_geometries");
	if (xmlLibGeom == nullptr)
		return;

	//andando em todos os library
	xmlGeometry = xmlLibGeom->FirstChildElement("geometry");
	for(; xmlGeometry != nullptr; xmlGeometry = xmlGeometry->NextSiblingElement("geometry"))
	{
		const TiXmlElement *xmlMesh, *xmlSource, *xmlVertice, *xmlTriangle, *xmlPolylist, *xmlPolygons;
		COLLADA_SOURCE *colSources;
		COLLADA_VERTICES *colVertices;
		COLLADA_TRIANGLES *colTriangles;
		COLLADA_POLYLIST *colPolylist;
		int colNumSources,colNumVertices,colNumTriangles,colNumPolylist;
		const char *geomName;

		//tiro o nome da geometria
		geomName = xmlGeometry->Attribute("name");

		//vou buscar a mesh
		xmlMesh = xmlGeometry->FirstChildElement("mesh");

		//ao principio não tenho nada
		colSources = nullptr;
		colVertices = nullptr;
		colTriangles = nullptr;
		colPolylist = nullptr;
		colNumSources = colNumVertices = colNumTriangles = colNumPolylist = 0;

		//enquanto houver sources para esta mesh / ou geometria
		xmlSource = xmlMesh->FirstChildElement("source");
		for(; xmlSource != nullptr; xmlSource = xmlSource->NextSiblingElement("source"))
		{
			const TiXmlElement *xmlFloatArray, *xlmTechnique, *xmlAccessor;
			COLLADA_SOURCE *newSource;
			int numFloat;
			float *arrayFloat;

			//tiro o float_array desta source
			xmlFloatArray = xmlSource->FirstChildElement("float_array");

			//e quero saber quantos floats estão lá
			attribData = xmlFloatArray->Attribute("count");
			if (attribData==nullptr)
				continue;

			//leio o valor e se for válido crio um array para onde devo ler cada ponto
			numFloat=atoi(attribData);
			if (numFloat<=0)
				continue;

			//crio espaço para ler tudo
			arrayFloat = (float*)malloc(sizeof(float)*numFloat);
			if (arrayFloat == nullptr)
				continue;

			//os valores dos atributos e leio toda a informação
			attribValues = xmlFloatArray->ToElement()->GetText();
			readValuesFloat(arrayFloat, numFloat, attribValues);

			//crio esta source
			newSource = (COLLADA_SOURCE*)realloc(colSources, sizeof(COLLADA_SOURCE)*(colNumSources+1));
			if (newSource == nullptr)
			{
				free(arrayFloat);
				continue;
			}

			//arranjo o ponteiro
			colSources = newSource;
			newSource += colNumSources;
			colNumSources++;

			//limpo a source e gravo para lá alguns valores
			memset(newSource, 0, sizeof(COLLADA_SOURCE));
			newSource->values = arrayFloat;
			newSource->numValues = numFloat;

			//tiro o ID (nome) da source
			newSource->nome = xmlSource->Attribute("id");

			//tiro o ID do technique e depois do accessor
			xlmTechnique = xmlSource->FirstChildElement("technique_common");
			xmlAccessor = xlmTechnique->FirstChildElement("accessor");

			//agora tiro coisas simples como
			attribData = xmlAccessor->Attribute("count");
			if (attribData != nullptr)
				newSource->numElements = atoi(attribData);
			attribData = xmlAccessor->Attribute("stride");
			if (attribData != nullptr)
				newSource->numComponents = atoi(attribData);
		}

		//enquanto houver vertices para esta mesh / ou geometria
		xmlVertice = xmlMesh->FirstChildElement("vertices");
		for(; xmlVertice != nullptr; xmlVertice = xmlVertice->NextSiblingElement("vertices"))
		{
			COLLADA_VERTICES *newVertex;
			const TiXmlElement *xmlInput;

			//crio este novo vertice
			newVertex = (COLLADA_VERTICES*)realloc(colVertices, sizeof(COLLADA_VERTICES)*(colNumVertices+1));
			if (newVertex == nullptr)
				continue;

			//arranjo o ponteiro
			colVertices = newVertex;
			newVertex += colNumVertices;
			colNumVertices++;

			//limpo o novo vertice
			memset(newVertex, 0, sizeof(COLLADA_VERTICES));

			//tiro o ID (nome) do vertice
			newVertex->nome = xmlVertice->Attribute("id");

			//para cada input deste vertice
			xmlInput = xmlVertice->FirstChildElement("input");
			for(; xmlInput != nullptr; xmlInput = xmlInput->NextSiblingElement("input"))
			{
				COLLADA_INPUT *newInput;

				//crio este novo vertice
				newInput = (COLLADA_INPUT*)realloc(newVertex->inputs, sizeof(COLLADA_INPUT)*(newVertex->numInputs+1));
				if (newInput == nullptr)
					continue;

				//arranjo o ponteiro
				newVertex->inputs = newInput;
				newInput += newVertex->numInputs;
				newVertex->numInputs++;

				//limpo o novo input
				memset(newInput, 0, sizeof(COLLADA_INPUT));

				//tiro os dados do input
				newInput->semantic = xmlInput->Attribute("semantic");
				newInput->source = xmlInput->Attribute("source");
				attribData = xmlInput->Attribute("offset");
				if (attribData != nullptr)
					newInput->offset = atoi(attribData);
			}
		}

		//enquanto houver triangulos para esta mesh / ou geometria
		xmlTriangle = xmlMesh->FirstChildElement("triangles");
		for(; xmlTriangle != nullptr; xmlTriangle = xmlTriangle->NextSiblingElement("triangles"))
		{
			COLLADA_TRIANGLES *newTriangle;
			const TiXmlElement *xmlP;

			//crio este novo triangulo
			newTriangle = (COLLADA_TRIANGLES*)realloc(colTriangles, sizeof(COLLADA_TRIANGLES)*(colNumTriangles+1));
			if (newTriangle == nullptr)
				continue;

			//arranjo o ponteiro
			colTriangles = newTriangle;
			newTriangle += colNumTriangles;
			colNumTriangles++;

			//limpo o novo trianglo
			memset(newTriangle, 0, sizeof(COLLADA_TRIANGLES));

			//tiro quantos triangulos fazem parte
			newTriangle->numTri = atoi(xmlTriangle->Attribute("count"));
			newTriangle->material = xmlTriangle->Attribute("material");

			//leio os inputs deste trianglo
			readInputs(xmlTriangle, &newTriangle->inputs, &newTriangle->numInputs);

			//tenho de saber de quantos em quantos inputs salto para o próximo indice do triangulo
			newTriangle->strideOffset = 0;
			for(int i=0; i<newTriangle->numInputs; i++)
			{
				if (newTriangle->inputs[i].offset > newTriangle->strideOffset)
					newTriangle->strideOffset = newTriangle->inputs[i].offset;
			}
			newTriangle->strideOffset++;
			
			//leio o <p> deste trianglo
			xmlP = xmlTriangle->FirstChildElement("p");
			if (xmlP != nullptr)
			{
				//no entanto isto é o numero de triangulos, o numero total de indices será: numTris * strideOffset * 3
				newTriangle->numIndices = (newTriangle->numTri * newTriangle->strideOffset * 3);

				//crio espaço para ler tudo
				newTriangle->indices = (int*)malloc(sizeof(int)*newTriangle->numIndices);
				if (newTriangle->indices == nullptr)
					continue;

				//os valores dos atributos e leio toda a informação
				attribValues = xmlP->ToElement()->GetText();
				readValuesInt(newTriangle->indices, newTriangle->numIndices, attribValues);
			}
		}

		//enquanto houver polylist para esta mesh / ou geometria
		xmlPolylist = xmlMesh->FirstChildElement("polylist");
		for(; xmlPolylist != nullptr; xmlPolylist = xmlPolylist->NextSiblingElement("polylist"))
		{
			COLLADA_POLYLIST *newPolylist;
			const TiXmlElement *xmlP, *xmlVCount;

			//crio este novo polylist
			newPolylist = (COLLADA_POLYLIST*)realloc(colPolylist, sizeof(COLLADA_POLYLIST)*(colNumPolylist+1));
			if (newPolylist == nullptr)
				continue;

			//arranjo o ponteiro
			colPolylist = newPolylist;
			newPolylist += colNumPolylist;
			colNumPolylist++;

			//limpo a nova lista de polys
			memset(newPolylist, 0, sizeof(COLLADA_POLYLIST));

			//alguns dados desta lista de polys
			newPolylist->numPolygons = atoi(xmlPolylist->Attribute("count"));
			newPolylist->material = xmlPolylist->Attribute("material");

			//leio os inputs deste trianglo
			readInputs(xmlPolylist, &newPolylist->inputs, &newPolylist->numInputs);
			
			//leio agora o vcount desta polylist
			xmlVCount = xmlPolylist->FirstChildElement("vcount");
			if (xmlVCount != nullptr)
			{
				//crio espaço para ler tudo
				newPolylist->vcount = (int*)malloc(sizeof(int)*newPolylist->numPolygons);
				if (newPolylist->vcount == nullptr)
					return;

				//os valores dos atributos e leio toda a informação
				attribValues = xmlVCount->ToElement()->GetText();
				readValuesInt(newPolylist->vcount, newPolylist->numPolygons, attribValues);
			}

			//leio o <p> desta polylist
			xmlP = xmlPolylist->FirstChildElement("p");
			if (xmlP != nullptr)
			{
				//tenho de passar por todos os polygonos e contar quandos indices tenho
				newPolylist->numIndices = 0;
				for(int i=0; i<newPolylist->numPolygons; i++)
					newPolylist->numIndices += newPolylist->vcount[i];

				//finalmente só tenho de multiplicar por quantos inputs tenho
				newPolylist->numIndices *= newPolylist->numInputs;

				//crio espaço para ler tudo
				newPolylist->indices = (int*)malloc(sizeof(int)*newPolylist->numIndices);
				if (newPolylist->indices == nullptr)
					continue;

				//os valores dos atributos e leio toda a informação
				attribValues = xmlP->ToElement()->GetText();
				readValuesInt(newPolylist->indices, newPolylist->numIndices, attribValues);
			}
		}

		//enquanto houver polygons para esta mesh / ou geometria (um polygono vai ficar guardado como polylist)
		xmlPolygons = xmlMesh->FirstChildElement("polygons");
		for(; xmlPolygons != nullptr; xmlPolygons = xmlPolygons->NextSiblingElement("polygons"))
		{
			COLLADA_POLYLIST *newPolylist;
			const TiXmlElement *xmlP, *xmlVCount;
			int *auxBufferIndices, numPolyLidos, numIndicesLidos;

			//crio este novo polylist
			newPolylist = (COLLADA_POLYLIST*)realloc(colPolylist, sizeof(COLLADA_POLYLIST)*(colNumPolylist+1));
			if (newPolylist == nullptr)
				continue;

			//arranjo o ponteiro
			colPolylist = newPolylist;
			newPolylist += colNumPolylist;
			colNumPolylist++;

			//limpo a nova lista de polys
			memset(newPolylist, 0, sizeof(COLLADA_POLYLIST));

			//alguns dados desta lista de polys
			newPolylist->numPolygons = atoi(xmlPolygons->Attribute("count"));
			newPolylist->material = xmlPolygons->Attribute("material");

			//leio os inputs deste trianglo
			readInputs(xmlPolygons, &newPolylist->inputs, &newPolylist->numInputs);

			//sei que no máximo posso vir a ter até a um máximo de numPolygons de vcount, portanto crio espaço para ler tudo
			newPolylist->vcount = (int*)malloc(sizeof(int)*newPolylist->numPolygons);
			if (newPolylist->vcount == nullptr)
				return;

			//limpo os dados todos
			memset(newPolylist->vcount, -1, sizeof(int)*newPolylist->numPolygons);

			//como só suporto no máximo quadrados, sei que no máximo posso vir a ter até a um máximo de (numPolygons * 4 * numInputs) indices, portanto crio espaço para ler tudo
			newPolylist->numIndices = newPolylist->numPolygons * 4 * newPolylist->numInputs;
			newPolylist->indices = (int*)malloc(sizeof(int)*newPolylist->numIndices);
			if (newPolylist->indices == nullptr)
				return;

			//limpo os dados todos
			memset(newPolylist->indices, -1, sizeof(int)*newPolylist->numIndices);

			//como vou ter de ler os polygonos às pingas, tenho de ter um buffer auxiliar
			auxBufferIndices = (int*)malloc(sizeof(int)*newPolylist->numInputs*4);
			if (auxBufferIndices == nullptr)
				return;
			
			//leio todos os ps deste poligono
			numPolyLidos = numIndicesLidos = 0;
			xmlP = xmlPolygons->FirstChildElement("p");
			for(; xmlP != nullptr; xmlP = xmlP->NextSiblingElement("p"))
			{
				int numValoresLidos, poligonoVertices;

				//limpo os dados todos
				memset(auxBufferIndices, -1, sizeof(int)*newPolylist->numInputs*4);

				//os valores dos atributos e leio toda a informação
				attribValues = xmlP->ToElement()->GetText();
				numValoresLidos = readValuesInt(auxBufferIndices, newPolylist->numInputs*4, attribValues);

				//ok, à partida tenho todos os valores de que preciso, só preciso de verificar se tenho um triangulo ou tenho um quadrado
				poligonoVertices = numValoresLidos / newPolylist->numInputs;
				if ((poligonoVertices != 3) && (poligonoVertices != 4))
					continue;

				//basta gravar quantos vertices tenho e os dados propriamente ditos
				newPolylist->vcount[numPolyLidos++] = poligonoVertices;
				memcpy(newPolylist->indices+numIndicesLidos, auxBufferIndices, sizeof(int)*numValoresLidos);
				numIndicesLidos += numValoresLidos;
			}

			//já posso apagar isto
			if (auxBufferIndices != nullptr)
				free(auxBufferIndices);
			auxBufferIndices = nullptr;

			//agora, dependendo de quantos poligonos realmente li, tenho de fazer alguns ajustes e já está tudo
			newPolylist->numPolygons = numPolyLidos;
			newPolylist->numIndices = numIndicesLidos;
		}

		//se tenho alguma lista de poligonos, então transformo-a em triangulos
		if (colNumPolylist > 0)
			convertPolyList(&colTriangles, &colNumTriangles, colPolylist, colNumPolylist);

		//tenho mais uma geometria, portanto tenho de criar espaço para a mesma e depois posso guardar o ID da mesma e a mesh a usar
		geomIDList = (GEOM_ID_PAIR*)realloc(geomIDList, sizeof(GEOM_ID_PAIR)*(geomIDNum+1));
		geomIDList[geomIDNum].geomID = xmlGeometry->Attribute("id");
		geomIDList[geomIDNum].meshIndex = modelo->arrayMesh.GetNumElements();
		geomIDNum++;

		//prontinho, agora só me falta criar um modelo propriamente dito
		criaModelo(modelo, geomName, colSources, colVertices, colTriangles, colNumSources, colNumVertices, colNumTriangles);

		//e como já acabei, basta limpar os dados antigos e continuar para a próxima mesh
		for(int i=0; i<colNumSources; i++)
		{
			if (colSources[i].values != nullptr)
				free(colSources[i].values);
		}
		for(int i=0; i<colNumVertices; i++)
		{
			if (colVertices[i].inputs != nullptr)
				free(colVertices[i].inputs);
		}
		for(int i=0; i<colNumTriangles; i++)
		{
			if (colTriangles[i].inputs != nullptr)
				free(colTriangles[i].inputs);
			if (colTriangles[i].indices != nullptr)
				free(colTriangles[i].indices);
		}
		for(int i=0; i<colNumPolylist; i++)
		{
			if (colPolylist[i].inputs != nullptr)
				free(colPolylist[i].inputs);
			if (colPolylist[i].indices != nullptr)
				free(colPolylist[i].indices);
			if (colPolylist[i].vcount != nullptr)
				free(colPolylist[i].vcount);
		}

		//apagar e limpar tudo
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

	//tenho de ler o scenegraph
	xmlLibVisualScene = xmlMain->FirstChildElement("library_visual_scenes");
	if (xmlLibVisualScene != nullptr)
	{
		const TiXmlElement *xmlVisualScene;

		//só leio uma cena...
		xmlVisualScene = xmlLibVisualScene->FirstChildElement("visual_scene");
		if (xmlVisualScene != 0)
		{
			HorseRadish::Matrix matrixStack;

			//o primeiro nó não tem nenhuma transformação
			matrixStack.SetIdentidade();

			//basta mandar ler os nós (isto está numa função porque é recursivo)
			leSceneGraph(modelo, geomIDList, geomIDNum, xmlVisualScene, matrixStack);
		}
	}

	//já não preciso disto
	if (geomIDList != nullptr)
		free(geomIDList);
	geomIDList = nullptr;
	geomIDNum = 0;

	//como o COLLADA é right-handed, tenho de ir a cada mesh
	for(int curMesh = 0; curMesh < modelo->arrayMesh.GetNumElements(); curMesh++)
	{
		float *vertData;

		//peço a posição
		vertData = modelo->arrayMesh[curMesh].mesh.FindAttribData(HorseRadish::Geometry::Mesh::Pos);
		if (vertData != nullptr)
		{
			int numVertices;
			float posTemp;

			//quantos vertives tenho
			numVertices = modelo->arrayMesh[curMesh].mesh.GetNumElements();

			//para cada vertice faço as trocas necessárias
			for(int curVertex = 0; curVertex < numVertices; curVertex++, vertData+=3)
			{
				posTemp = vertData[2];
				vertData[2] = -vertData[1];
				vertData[1] = posTemp;
			}
		}
	}

	//agora que já sei que está tudo criado, caso tenha um eixo meio estranho, ponho as coisas no sitio correcto
	if ((axisValue != nullptr) && (strcmp(axisValue, "Z_UP")==0))
	{
		//para cada mesh dentro deste modelo
		for(int curMesh = 0; curMesh < modelo->arrayMesh.GetNumElements(); curMesh++)
		{
			float *vData, auxVal;
			int numVertices;

			//tiro estes dados para fazer a minha passagem pelos dados
			numVertices = modelo->arrayMesh[curMesh].mesh.GetNumElements();
			vData = modelo->arrayMesh[curMesh].mesh.FindAttribIndex(0);

			//para cada vertice faço as trocas necessárias
			for(int curVertex = 0; curVertex < numVertices; curVertex++, vData+=3)
			{
				auxVal = vData[2];
				vData[2] = -vData[1];
				vData[1] = auxVal;
			}
		}
	}
}

static
TiXmlDocument* documentOpenMem(const void *fileData, const int fileSize, char *errorDesc, const int errorBufferSize, int *errorLine)
{
	TiXmlDocument *xmlDoc;
	char *newData;
	const char *mainBuffer;
	int mainBufferSize;

	//verificar isto
	if (fileData==nullptr || fileSize<=0)
		return 0;

	//preciso deste ponteiro
	mainBuffer = (const char*)fileData;
	if (*mainBuffer == '\0')
		return nullptr;

	//por defeito o tamanho do ficheiro está igual e não allocei nenhum buffer auxiliar
	mainBufferSize = fileSize;
	newData = nullptr;

	//o ficheiro tem de estar terminado em '\0', logo tenho de me certificar disso
	if (mainBuffer[mainBufferSize-1]!='\0')
		{
		//crio mais espaço
		newData=(char *)malloc(mainBufferSize+1);
		if (newData==nullptr)
			return nullptr;

		//copio tudo de um lado para o outro
		memcpy(newData,mainBuffer,mainBufferSize);

		//basta arranjar o ponteiro e fechar o ficheiro no sitio correcto
		newData[mainBufferSize]='\0';
		mainBuffer=(const char*)newData;
		mainBufferSize++;
		}

	//basta criar as coisas internas do ponteiro
	xmlDoc = new TiXmlDocument();
	if (xmlDoc == nullptr)
		{
		//deu barraca, só tenho de sair
		if (newData != nullptr)
			free(newData);
		return 0;
		}

	//iniciar algumas coisas internas (o Parser devolve o ponteiro avançado até ao fim)
	if (xmlDoc->Parse(mainBuffer, nullptr, TIXML_ENCODING_UTF8) != (mainBuffer+mainBufferSize-1))
		{
		//se houver algum erro digno de registo
		if (xmlDoc->Error())
			{
			//se for para gravar uma descrição do erro
			if (errorDesc!=nullptr && errorBufferSize>0)
				{
				//limpo o buffer, tiro a descrição do erro e gravo
				memset(errorDesc, 0, errorBufferSize);
				strcpy_s(errorDesc, errorBufferSize, xmlDoc->ErrorDesc());
				}		

			//tiro a linha do erro se for caso disso
			if (errorLine)
				*errorLine = xmlDoc->ErrorRow();
			}

		//limpo as coisas e pronto, bazo sem nada
		if (newData != nullptr)
			free(newData);
		xmlDoc->Clear();
		delete xmlDoc;
		return nullptr;
		}

	//posso apagar o ficheiro
	if (newData != nullptr)
		free(newData);

	//correu tudo bem...
	return xmlDoc;
}

/*§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§
§§§§§§   -= Funções para exportar =-   §§§§§
§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§*/
Geometry::Model* mfReadCollada(HorseRadish::Streams::StreamReader * const streamReader)
{
	Geometry::Model *modelo;
	TiXmlDocument *xmlDoc;
	const void *streamContent;
	int streamContentSize;
	bool streamContentCopied;

	//verificar parametros
	if (streamReader == nullptr)
		return nullptr;

	//preciso do conteúdo do ficheiro completo
	streamContent = streamReader->ReadContent(streamContentSize, streamContentCopied);

	//tento ler e fazer parse do XML
	xmlDoc = documentOpenMem(streamContent, streamContentSize, nullptr, 0, nullptr);
	if (xmlDoc<=0)
	{
		//posso apagar o buffer intermédio se for caso disso
		if (streamContentCopied == true)
			delete streamContent;
		return nullptr;
	}

	//crio espaço para albergar tudo
	modelo = new Geometry::Model();
	if (modelo == nullptr)
	{
		//limpo o XML
		xmlDoc->Clear();
		delete xmlDoc;

		//posso apagar o buffer intermédio se for caso disso
		if (streamContentCopied == true)
			delete streamContent;
		return nullptr;
	}

	//passo então pelos elementos do XML e vou fazendo o modelo
	parseCollada(modelo, xmlDoc);

	//posso fechar o xml
	xmlDoc->Clear();
	delete xmlDoc;

	//posso apagar o buffer intermédio se for caso disso
	if (streamContentCopied == true)
		delete streamContent;

	//já tá
	return modelo;
}