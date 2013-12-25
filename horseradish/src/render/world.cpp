#include "world.hpp"
#include "common\ImageFactory.hpp"
#include "common\Sorting.hpp"

static
bool searchBlock(HorseRadish::Streams::StreamReader * const streamReader, const int chunkTargetID, bool fromFileStart)
{
	//se for preciso iniciar a procura desde o inicio do ficheiro
	if (fromFileStart)
		streamReader->Seek(sizeof(int)+sizeof(char)*3, HorseRadish::Streams::Stream::Begin);

	//enquanto não chegar ao fim do ficheiro
	while(streamReader->CanRead() == true)
	{
		int curChunkID, curChunkSize;

		//a informação de que preciso para os chunks
		streamReader->ReadInt32(curChunkID);
		streamReader->ReadInt32(curChunkSize);

		//se o chunk for este posso sair
		if (curChunkID == chunkTargetID)
			return true;

		//como não é este o bloco pretendido, basta passar para o próximo
		streamReader->Seek(curChunkSize, HorseRadish::Streams::Stream::Current);
	}

	//chegando aqui não achei nada
	return false;
}

static
int qsortSurfaces(const HorseRadish::Render::Surface *surfaceA, const HorseRadish::Render::Surface *surfaceB)
{
	//se o material for igual, é por distância À camera
	if (surfaceA->material == surfaceB->material)
		return ((surfaceA->renderValues.distToCam < surfaceB->renderValues.distToCam) ? -1 : 1);

	//senão é por material
	return ((surfaceA->material < surfaceB->material) ? -1 : 1);
}

namespace HorseRadish
{

namespace Render
{

//§§§§§§§§§§§§§§§§§§§§§§§§§§§
//§§§§§§   Classe Light	§§§§§
//§§§§§§§§§§§§§§§§§§§§§§§§§§§
Light::Light()
{
	//limpo tudo
	this->type = Omni;
	this->active = true;
	this->insideCamera = false;
	this->noShadows = true;
	this->scissor[0] = this->scissor[1] = this->scissor[2] = this->scissor[3] = 0;
	this->textures.spotTex = nullptr;
	this->textures.attenTex = nullptr;
	this->textures.cubeEnvTex = nullptr;
}

Light::~Light()
{
}

//§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§
//§§§§§§   Classe Geometry	§§§§§
//§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§
Geometry::Geometry()
{
	//limpo tudo
	this->id = -1;
	this->type = Static3;
	this->renderVBOVertexOffset = 0;
	this->renderTriListOffset = nullptr;
}

Geometry::~Geometry()
{
}

//§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§
//§§§§§§   Classe TextureSet	§§§§§
//§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§
TextureSet::TextureSet()
{
	//limpo tudo
	this->id = -1;
}

TextureSet::~TextureSet()
{
}

//§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§
//§§§§§§   Classe Surface	§§§§§
//§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§
Surface::Surface()
{
	//limpo tudo
	this->id = -1;
	this->type = Static;
	this->material = nullptr;
	this->geometry = nullptr;
	this->texSet = nullptr;
	this->texData.general.tex0 = this->texData.general.tex1 = this->texData.general.tex2 = this->texData.general.tex3 = nullptr;
	this->renderValues.distToCam = 0.0f;
}

Surface::~Surface()
{
}

//§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§
//§§§§§§   Classe Material	§§§§§
//§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§
Material::Material()
{
	//limpo tudo
	this->materialLib = nullptr;
	memset(&this->dataShadering, 0, sizeof(Shadering));
	memset(&this->dataLighting, 0, sizeof(Lighting));
	this->matProperties = (MaterialProperties)0;
	this->alphaTestValue = 1.0f;
}

Material::~Material()
{
}

//§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§
//§§§§§§   Classe MaterialLibrary	§§§§§
//§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§
MaterialLibrary::MaterialLibrary()
{
}

MaterialLibrary::~MaterialLibrary()
{
}

//§§§§§§§§§§§§§§§§§§§§§§§§§§§
//§§§§§§   Classe Brush	§§§§§
//§§§§§§§§§§§§§§§§§§§§§§§§§§§
Brush::Brush()
{
}

Brush::~Brush()
{
}

//§§§§§§§§§§§§§§§§§§§§§§§§§§§
//§§§§§§   Classe World	§§§§§
//§§§§§§§§§§§§§§§§§§§§§§§§§§§

//algumas constantes
const int World::HRFChunckTextureSetsID = 2;
const int World::HRFChunckGeometriesID = 1;
const int World::HRFChunckSurfacesID = 3;

bool World::prepareMeshForType(HorseRadish::Geometry::Mesh * const modelMesh, const Geometry::GeometryType geomType)
{
	//se for do tipo estático 1
	if (geomType == Geometry::Static1)
	{
		HorseRadish::Geometry::Mesh::MeshAtributeType attribOrder[4];

		//agora basta criar atributos se eles ainda nao existirem
		if (modelMesh->FindAttrib(HorseRadish::Geometry::Mesh::Pos) == nullptr)
			modelMesh->NewAttrib(HorseRadish::Geometry::Mesh::Pos, modelMesh->GetNumElements());
		if (modelMesh->FindAttrib(HorseRadish::Geometry::Mesh::TexCoords) == nullptr)
			modelMesh->NewAttrib(HorseRadish::Geometry::Mesh::TexCoords, modelMesh->GetNumElements());
		if (modelMesh->FindAttribGeneric(HorseRadish::Geometry::Mesh::Generic1, 1) == nullptr)
			modelMesh->NewAttrib(HorseRadish::Geometry::Mesh::Generic1, modelMesh->GetNumElements());
		if (modelMesh->FindAttribGeneric(HorseRadish::Geometry::Mesh::Generic2, 1) == nullptr)
			modelMesh->NewAttrib(HorseRadish::Geometry::Mesh::Generic2, modelMesh->GetNumElements());

		//ordeno os atributos
		attribOrder[0] = HorseRadish::Geometry::Mesh::Pos;
		attribOrder[1] = HorseRadish::Geometry::Mesh::TexCoords;
		attribOrder[2] = HorseRadish::Geometry::Mesh::Generic2;
		attribOrder[3] = HorseRadish::Geometry::Mesh::Generic1;
		modelMesh->ReorderAttrib(attribOrder, 4, true);

		//já tá
		return true;
	}

	//se for do tipo estático 2
	if (geomType == Geometry::Static2)
	{
		HorseRadish::Geometry::Mesh::MeshAtributeType attribOrder[6];

		//agora basta criar atributos se eles ainda nao existirem
		if (modelMesh->FindAttrib(HorseRadish::Geometry::Mesh::Pos) == nullptr)
			modelMesh->NewAttrib(HorseRadish::Geometry::Mesh::Pos, modelMesh->GetNumElements());
		if (modelMesh->FindAttrib(HorseRadish::Geometry::Mesh::TexCoords) == nullptr)
			modelMesh->NewAttrib(HorseRadish::Geometry::Mesh::TexCoords, modelMesh->GetNumElements());
		if (modelMesh->FindAttrib(HorseRadish::Geometry::Mesh::Normal) == nullptr)
			modelMesh->NewAttrib(HorseRadish::Geometry::Mesh::Normal, modelMesh->GetNumElements());
		if (modelMesh->FindAttrib(HorseRadish::Geometry::Mesh::Tangent4) == nullptr)
			modelMesh->NewAttrib(HorseRadish::Geometry::Mesh::Tangent4, modelMesh->GetNumElements());
		if (modelMesh->FindAttribGeneric(HorseRadish::Geometry::Mesh::Generic1, 1) == nullptr)
			modelMesh->NewAttrib(HorseRadish::Geometry::Mesh::Generic1, modelMesh->GetNumElements());
		if (modelMesh->FindAttribGeneric(HorseRadish::Geometry::Mesh::Generic2, 1) == nullptr)
			modelMesh->NewAttrib(HorseRadish::Geometry::Mesh::Generic2, modelMesh->GetNumElements());

		//ordeno os atributos
		attribOrder[0] = HorseRadish::Geometry::Mesh::Pos;
		attribOrder[1] = HorseRadish::Geometry::Mesh::TexCoords;
		attribOrder[4] = HorseRadish::Geometry::Mesh::Generic2;
		attribOrder[5] = HorseRadish::Geometry::Mesh::Generic1;
		attribOrder[2] = HorseRadish::Geometry::Mesh::Normal;
		attribOrder[3] = HorseRadish::Geometry::Mesh::Tangent4;
		modelMesh->ReorderAttrib(attribOrder, 6, true);

		//crio um atributo extra e já tá
		modelMesh->NewAttrib(HorseRadish::Geometry::Mesh::Generic1, modelMesh->GetNumElements());
		return true;
	}

	//se for do tipo estático 3
	if (geomType == Geometry::Static3)
	{
		HorseRadish::Geometry::Mesh::MeshAtributeType attribOrder[4];
		bool createDummyUV;

		//se não tenho UVs, tenho de as criar para os calculos das tangentes não ficarem marados
		createDummyUV = (modelMesh->FindAttrib(HorseRadish::Geometry::Mesh::TexCoords) == nullptr);

		//agora basta criar atributos se eles ainda nao existirem
		if (modelMesh->FindAttrib(HorseRadish::Geometry::Mesh::Pos) == nullptr)
			modelMesh->NewAttrib(HorseRadish::Geometry::Mesh::Pos, modelMesh->GetNumElements());
		if (modelMesh->FindAttrib(HorseRadish::Geometry::Mesh::TexCoords) == nullptr)
			modelMesh->NewAttrib(HorseRadish::Geometry::Mesh::TexCoords, modelMesh->GetNumElements());
		if (modelMesh->FindAttrib(HorseRadish::Geometry::Mesh::Normal) == nullptr)
			modelMesh->NewAttrib(HorseRadish::Geometry::Mesh::Normal, modelMesh->GetNumElements());
		if (modelMesh->FindAttrib(HorseRadish::Geometry::Mesh::Tangent4) == nullptr)
			modelMesh->NewAttrib(HorseRadish::Geometry::Mesh::Tangent4, modelMesh->GetNumElements());

		//se for para criar UVs
		if (createDummyUV)
			modelMesh->TexGen(MESH_TEXGEN_OBJECT_LINEAR, nullptr);

		//ordeno os atributos
		attribOrder[0] = HorseRadish::Geometry::Mesh::Pos;
		attribOrder[1] = HorseRadish::Geometry::Mesh::TexCoords;
		attribOrder[2] = HorseRadish::Geometry::Mesh::Normal;
		attribOrder[3] = HorseRadish::Geometry::Mesh::Tangent4;
		modelMesh->ReorderAttrib(attribOrder, 4, true);

		//agora crio as normais e tangents
		modelMesh->Ortho(MESH_ORTHO_CREATE_TANGENT4);

		//crio um atributo extra e já tá
		modelMesh->NewAttrib(HorseRadish::Geometry::Mesh::Generic4, modelMesh->GetNumElements());
		return true;
	}

	//não conheço este tipo
	return false;
}

void World::arranjaPonteirosGeom(const Geometry * const originalList)
{
	Geometry *currentList;

	//a lista actual
	currentList = this->geometries.GetMainPointer();

	//se não existir nenhuma diferença entre as listas, posso sair
	if (currentList == originalList)
		return;

	//basta passar por todas as superficies e arranjar o novo ponteiro
	for(int i=0; i<this->surfacesTotal.GetNumElements(); i++)
		this->surfacesTotal[i].geometry = currentList + (this->surfacesTotal[i].geometry - originalList);
}

void World::arranjaPonteirosSurf(const Surface * const originalList)
{
	Surface *walkerSurf, **walkerSurfSurf;

	//se não existir nenhuma diferença entre as listas, posso sair
	if (this->surfacesTotal.GetMainPointer() == originalList)
		return;

	//passo por todas as superficies que sejam só raiz
	for(int i=0; i<this->surfacesRoot.GetNumElements(); i++)
		this->surfacesRoot.Replace(i, this->surfacesTotal + (this->surfacesRoot[i] - originalList));

	//basta passar por todas as superficies
	for(int i=0; i<this->surfacesTotal.GetNumElements(); i++)
	{
		//pra faciliatar
		walkerSurf = this->surfacesTotal + i;

		//para todos os filhos
		for(int j=0; j<walkerSurf->childs.GetNumElements(); j++)
			walkerSurf->childs.Replace(j, this->surfacesTotal + (walkerSurf->childs[j] - originalList));
	}
}

Geometry* World::createGeometry(HorseRadish::Geometry::Mesh * const modelMesh, const Geometry::GeometryType geomType)
{
	Geometry *novaGeom, *listaOriginal;
	float bbMin[3], bbMax[3];

	//a mesh não pode estar toda junta, não dá nada jeito
	if (modelMesh->SingleBufferPointer() != nullptr)
		modelMesh->SingleBufferInv();

	//agora de acordo com o tipo de geometria indicada, tenho de fazer alguns ajustes
	if (World::prepareMeshForType(modelMesh, geomType) == false)
		return nullptr;

	//optimizo sempre os indices e reordeno-os
	modelMesh->IndexOptimize();
	modelMesh->ReorderTriIndex();

	//guardo a lista original (ver a ultima função que é chamada antes desta terminar)
	listaOriginal = this->geometries.GetMainPointer();

	//a geometria em questão
	novaGeom = this->geometries.Add();

	//guardo o tipo, a mesh propriamente dita e crio um ID
	novaGeom->type = geomType;
	memcpy(&novaGeom->mesh, modelMesh, sizeof(HorseRadish::Geometry::Mesh));

	//gero o novo ID
	novaGeom->id = ++this->genGeometryID;

	//como copiei a mesh antiga, para evitar que alguém apague esta depois, limpo tudo
	memset(modelMesh, 0, sizeof(HorseRadish::Geometry::Mesh));

	//agora que tenho uma mesh posso calcular o volume que ela ocupa
	novaGeom->mesh.BoundingBox(bbMin, bbMax);
	novaGeom->bbox.SetMinMax(bbMin, bbMax);
	novaGeom->bsphere.Set(novaGeom->bbox);

	//depois disto posso juntar tudo num único buffer
	novaGeom->mesh.SingleBuffer();

	//finalmente só tenho de arranjar os ponteiros caso tenha modificado a lista original devido à allocação
	arranjaPonteirosGeom(listaOriginal);
	return novaGeom;
}

int World::processModel(const HorseRadish::Geometry::Model * const modelo, const Geometry::GeometryType geomType, const bool joinModels)
{
	int oldNumGeom;

	//guardo o número de geometrias antigo
	oldNumGeom = this->geometries.GetNumElements();

	//se me pedem para juntar tudo
	if (joinModels == true)
	{
		HorseRadish::Geometry::Mesh *meshAux;

		//junto o modelo num só e crio-o no motor
		meshAux = modelo->Join();
		createGeometry(meshAux, geomType);

		//limpar coisas
		delete meshAux;
	}
	else
	{
		//para cada modelo, crio a sua geometria
		for(int curModelo=0; curModelo < modelo->arrayMesh.GetNumElements(); curModelo++)
			createGeometry(&modelo->arrayMesh[curModelo].mesh, geomType);
	}

	//está tudo (basta devolver quantas geometrias criei)
	return (this->geometries.GetNumElements() - oldNumGeom);
}

bool World::writeGeometries(HorseRadish::Streams::StreamWriter * const streamWriter)
{
	int chunkSizePos, chunkEndPos;

	//escrevo o chunck a dizer que vou escrever as geometrias, escrevo o espaço que ocupa este chunck (ainda não sei, portanto é 0) e guardo a nova posição
	streamWriter->WriteInt32(World::HRFChunckGeometriesID);
	streamWriter->WriteInt32(0);
	chunkSizePos = streamWriter->GetPosition();

	//a primeira coisa a fazer é escrever quantas geometrias tenho
	streamWriter->WriteInt32(this->geometries.GetNumElements());

	//agora para cada geometria
	for(int i=0; i<this->geometries.GetNumElements(); i++)
	{
		Geometry *curGeom;
		float auxBuffer[10];

		//a geometria a escrever para o disco
		curGeom = this->geometries + i;

		//escrevo o ID desta geometria assim como o tipo dela
		streamWriter->WriteInt32(curGeom->id);
		streamWriter->WriteInt32(curGeom->type);

		//mando serializar a mesh para ficheiro
		curGeom->mesh.SerializeHRF(streamWriter);

		//já agora escrevo 6 floats (xyz para o min e max da bbox) + 4 floats (xyz centro e raio da bsphere)
		curGeom->bbox.GetMin(auxBuffer+0);
		curGeom->bbox.GetMax(auxBuffer+3);
		curGeom->bsphere.GetCenter(auxBuffer+6);
		auxBuffer[9] = curGeom->bsphere.GetRadius();
		streamWriter->Write(auxBuffer, sizeof(auxBuffer));
	}

	//retiro a posição onde estou, recuo para escrever o tamanho do chunk, escrevo e volto à posição onde estava
	chunkEndPos = streamWriter->GetPosition();
	streamWriter->Seek(chunkSizePos-4, HorseRadish::Streams::Stream::Begin);
	streamWriter->WriteInt32(chunkEndPos-chunkSizePos);
	streamWriter->Seek(chunkEndPos, HorseRadish::Streams::Stream::Begin);
	
	//e já tá
	return true;
}

bool World::readGeometries(HorseRadish::Streams::StreamReader * const streamReader)
{
	int numGeometrias;

	//procuro pelo chunck das geometrias
	if (searchBlock(streamReader, World::HRFChunckGeometriesID, true) == false)
		return false;
	
	//a primeira coisa a fazer é ler quantas geometrias tenho e crio espaço para as ler
	streamReader->ReadInt32(numGeometrias);
	if (this->geometries.SetCapacity(numGeometrias) == false)
		return false;

	//agora para cada geometria
	for(int i=0; i<numGeometrias; i++)
	{
		Geometry *curGeom;
		float auxBuffer[10];
		int geomType;

		//a geometria a ler
		curGeom = this->geometries.Add();

		//leio o ID desta geometria assim como o tipo dela
		streamReader->ReadInt32(curGeom->id);
		streamReader->ReadInt32(geomType);

		//converto
		curGeom->type = (Geometry::GeometryType)geomType;

		//por agora tem de ser sempre assim
		if (curGeom->type != Geometry::Static3)
			exit(0);

		//esta função lê tudo o que está no ficheiro
		curGeom->mesh.DeserializeHRF(streamReader);
		
		//leio também os dados da bbox e bsphere
		streamReader->Read(auxBuffer, sizeof(auxBuffer));
		curGeom->bbox.SetMinMax(auxBuffer+0, auxBuffer+3);
		curGeom->bsphere.SetCenter(auxBuffer[6], auxBuffer[7], auxBuffer[8]);
		curGeom->bsphere.SetRadius(auxBuffer[9]);
	}
	
	//e já tá
	return true;
}

bool World::writeTextureSets(HorseRadish::Streams::StreamWriter * const streamWriter)
{
	int chunkSizePos, chunkEndPos;

	//escrevo o chunck a dizer que vou escrever os texture sets, salvo onde estou e escrevo o espaço que ocupa este chunck
	streamWriter->WriteInt32(World::HRFChunckTextureSetsID);
	streamWriter->WriteInt32(0);
	chunkSizePos = streamWriter->GetPosition();

	//a primeira coisa a fazer é escrever quantos texture sets tenho
	streamWriter->WriteInt32(this->textureSets.GetNumElements());

	//agora para cada texture set	
	for(int i = 0; i < this->textureSets.GetNumElements(); i++)
	{
		TextureSet *curSet;

		//isto dá jeito
		curSet = this->textureSets + i;

		//escrevo o ID do set e quantas texturas tem
		streamWriter->WriteInt32(curSet->id);
		streamWriter->WriteInt32(curSet->texs.GetNumElements());

		//se não tem texturas
		if (curSet->texs.GetNumElements() <= 0)
			continue;

		//para cada textura individualmente
		for(int j = 0; j < curSet->texs.GetNumElements(); j++)
		{
			TextureSet::Texture *curTex;

			//crio uma nova textura
			curTex = curSet->texs + j;

			//escrevo isto tudo
			streamWriter->WriteInt16(curTex->type);
			streamWriter->WriteInt16(curTex->format);
			streamWriter->WriteInt16(curTex->filter);
			streamWriter->WriteInt32(curTex->flags);

			//e finalmente o caminho da mesma
			streamWriter->WriteString(curTex->filePath.GetData(), true);
		}
	}

	//retiro a posição onde estou, recuo para escrever o tamanho do chunk, escrevo e volto à posição onde estava
	chunkEndPos = streamWriter->GetPosition();
	streamWriter->Seek(chunkSizePos-4, HorseRadish::Streams::Stream::Begin);
	streamWriter->WriteInt32(chunkEndPos-chunkSizePos);
	streamWriter->Seek(chunkEndPos, HorseRadish::Streams::Stream::Begin);
	
	//e já tá
	return true;
}

bool World::readTextureSets(HorseRadish::Streams::StreamReader * const streamReader)
{
	int numTextureSets;

	//procuro pelo chunck das texturas
	if (searchBlock(streamReader, World::HRFChunckTextureSetsID, true) == false)
		return false;
	
	//a primeira coisa a fazer é ler quantos texture sets tenho e crio espaço para os ler
	streamReader->ReadInt32(numTextureSets);
	if (this->textureSets.SetCapacity(numTextureSets) == false)
		return false;

	//agora para cada texture set
	for(int i=0; i<numTextureSets; i++)
	{
		TextureSet *curSet;
		int numTexs;

		//crio um set
		curSet = this->textureSets.Add();

		//leio o ID desta geometria assim como quantas texturas compôem este set
		streamReader->ReadInt32(curSet->id);
		streamReader->ReadInt32(numTexs);
		if (numTexs <= 0)
			continue;

		//preciso de espaço para as texturas
		if (curSet->texs.SetCapacity(numTexs) == false)
			return false;

		//agora leio cada textura individualmente
		for(int j=0; j<numTexs; j++)
		{
			HorseRadish::hInt16 valorAux;
			TextureSet::Texture *curTex;
			char texPath[512];

			//crio uma nova textura
			curTex = curSet->texs.Add();

			//tiro os dados mais importantes (tipo, formato, filtro e flags)
			streamReader->ReadInt16(valorAux);
			curTex->type = valorAux;
			streamReader->ReadInt16(valorAux);
			curTex->format = valorAux;
			streamReader->ReadInt16(valorAux);
			curTex->filter = valorAux;
			streamReader->ReadInt32(curTex->flags);

			//finalmente o caminho
			streamReader->ReadUntil(texPath, sizeof(texPath), '\0');
			curTex->filePath.Set(HorseRadish::String::UTF8, texPath);
		}
	}

	//e já tá
	return true;
}

bool World::writeSurfaces(HorseRadish::Streams::StreamWriter * const streamWriter)
{
	int chunkSizePos, chunkEndPos;

	//escrevo o chunck a dizer que vou escrever as superfícies, salvo onde estou e escrevo o espaço que ocupa este chunck
	streamWriter->WriteInt32(World::HRFChunckSurfacesID);
	streamWriter->WriteInt32(0);
	chunkSizePos = streamWriter->GetPosition();

	//a primeira coisa a fazer é escrever quantas superfícies tenho
	streamWriter->WriteInt32(this->surfacesTotal.GetNumElements());

	//agora para cada superficie
	for(int i = 0; i < this->surfacesTotal.GetNumElements(); i++)
	{
		Surface *curSurf;

		//isto dá jeito
		curSurf = this->surfacesTotal + i;

		//escrevo algumas coisas da superficie
		streamWriter->WriteInt32(curSurf->id);
		streamWriter->WriteInt32(curSurf->type);
		streamWriter->WriteInt32(curSurf->childs.GetNumElements());

		//e o seu material, geom e texturas
		if (curSurf->material == nullptr)
			streamWriter->WriteInt8(0);
		else
			streamWriter->WriteString(curSurf->material->materialLib->nomeMaterial.GetData(), true);
		streamWriter->WriteInt32(curSurf->geometry->id);
		streamWriter->WriteInt32(curSurf->texSet->id);
	}

	//quantas superfícies na raíz tenho
	streamWriter->WriteInt32(this->surfacesRoot.GetNumElements());

	//agora para cada superficie na raíz, escrevo o seu ID
	for(int i = 0; i < this->surfacesRoot.GetNumElements(); i++)
		streamWriter->WriteInt32(this->surfacesRoot[i]->id);

	//retiro a posição onde estou, recuo para escrever o tamanho do chunk, escrevo e volto à posição onde estava
	chunkEndPos = streamWriter->GetPosition();
	streamWriter->Seek(chunkSizePos-4, HorseRadish::Streams::Stream::Begin);
	streamWriter->WriteInt32(chunkEndPos-chunkSizePos);
	streamWriter->Seek(chunkEndPos, HorseRadish::Streams::Stream::Begin);
	
	//e já tá
	return true;
}

bool World::readSurfaces(HorseRadish::Streams::StreamReader * const streamReader)
{
	int numSurfacesTotal, numSurfacesRoot;

	//procuro pelo chunck das superficies
	if (searchBlock(streamReader, World::HRFChunckSurfacesID, true) == false)
		return false;
	
	//a primeira coisa a fazer é ler quantas superficies tenho e crio espaço para as ler
	streamReader->ReadInt32(numSurfacesTotal);
	this->surfacesTotal.Clear();
	if (this->surfacesTotal.SetCapacity(numSurfacesTotal) == false)
		return false;

	//agora para cada superficie
	for(int i=0; i<numSurfacesTotal; i++)
	{
		Surface *curSurf;
		char materialName[512];
		int geomID, texSetID, numTextures, numChilds, surfType;

		//a geometria a ler
		curSurf = this->surfacesTotal.Add();

		//leio o ID desta geometria assim como o tipo dela e quantos filhos tem
		streamReader->ReadInt32(curSurf->id);
		streamReader->ReadInt32(surfType);
		streamReader->ReadInt32(numChilds);

		//converto
		curSurf->type = (Surface::SurfaceType)surfType;

		//leio o nome do material assim como o ID da geometria a usar e o ID do texture set
		streamReader->ReadUntil(materialName, sizeof(materialName), '\0');
		streamReader->ReadInt32(geomID);
		streamReader->ReadInt32(texSetID);

		//procuro pela geometria e coloco o ponteiro para a mesma
		for(int j=0; j<this->geometries.GetNumElements(); j++)
		{
			//se achei guardo a geometria e posso sair
			if (this->geometries[j].id == geomID)
			{
				curSurf->geometry = this->geometries + j;
				break;
			}
		}

		//procuro pelo texture set e coloco o ponteiro para o mesmo
		for(int j=0; j<this->textureSets.GetNumElements(); j++)
		{
			//se achei guardo a geometria e posso sair
			if (this->textureSets[j].id == texSetID)
			{
				curSurf->texSet = this->textureSets + j;
				break;
			}
		}

		//tenho obrigatoriamente de ter uma geometria e um texture set, portanto se algo der para o torto
		if ((curSurf->geometry == nullptr) || (curSurf->texSet == nullptr))
			return false;
	}

	//agora tenho de ler quantas superficies de topo tenho e crio espaço para as ler
	streamReader->ReadInt32(numSurfacesRoot);
	this->surfacesRoot.Clear();
	if (this->surfacesRoot.Reserve(numSurfacesRoot) == false)
		return false;

	//agora leio que superficies fazem parte da lista de raíz
	for(int i=0; i<numSurfacesRoot; i++)
	{
		int surfRootID; 

		//leio o ID desta superficie
		streamReader->ReadInt32(surfRootID);

		//agora basta procurar por ela
		for(int j=0; j<this->surfacesTotal.GetNumElements(); j++)
		{
			//se achei
			if (this->surfacesTotal[j].id == surfRootID)
			{
				this->surfacesRoot.Replace(i, this->surfacesTotal + j);
				break;
			}
		}

		//tenho obrigatoriamente de ter uma superficie nesta região
		if (this->surfacesRoot[i] == nullptr)
			return false;
	}

	//e já tá
	return true;
}

World::World()
{
	//por omissão
	this->genGeometryID = 0;
	this->genSurfaceID = 0;
}

World::~World()
{
	//inicio os IDs
	this->genGeometryID = 0;
	this->genSurfaceID = 0;
}

void World::Cleanup()
{
	//limpo as luzes, geometrias, materiais e texture sets
	this->lights.Clear();
	this->geometries.Clear();
	this->materials.Clear();
	this->textureSets.Clear();

	//inicio os IDs
	this->genGeometryID = 0;
	this->genSurfaceID = 0;

	//apago as superficies
	this->surfacesRoot.Clear();
	this->surfacesTotal.Clear();
}

int World::ImportHRF(HorseRadish::Streams::Stream * const fileStream)
{
	//preciso disto
	if (fileStream == nullptr)
		return -1;

	{
		int fileMagic;
		HorseRadish::hInt8 fileVersionMajor, fileVersionMinor, fileVersionBuild;

		//preciso de criar um stream reader
		HorseRadish::Streams::StreamReader streamReader(fileStream);

		//leio o int
		streamReader.ReadInt32(fileMagic);
		streamReader.ReadInt8(fileVersionMajor);
		streamReader.ReadInt8(fileVersionMinor);
		streamReader.ReadInt8(fileVersionBuild);

		//o magic tem de ser este seguido da versão
		if (fileMagic != 0xabcdeff0)
			return -4;
		if (fileVersionMajor != 1)
			return -4;

		//leio as geometrias
		if (readGeometries(&streamReader) == false)
			return -5;

		//leio as texturas que vou usar
		if (readTextureSets(&streamReader) == false)
			return -6;

		//leio as superficies
		if (readSurfaces(&streamReader) == false)
			return -7;
	}

	//correu tudo bem
	return 0;
}

int World::ExportHRF(HorseRadish::Streams::Stream * const fileStream)
{
	//preciso disto
	if (fileStream == nullptr)
		return -1;

	{
		//crio um stream writer para simplifar a vida
		HorseRadish::Streams::StreamWriter streamWriter(fileStream);

		//começo por escrever o header
		streamWriter.WriteInt32(0xabcdeff0);		
		streamWriter.WriteInt8(1);
		streamWriter.WriteInt8(0);
		streamWriter.WriteInt8(0);

		//escrevo as geometrias (se elas existirem)
		writeGeometries(&streamWriter);

		//escrevo os texture sets
		writeTextureSets(&streamWriter);

		//escrevo as superfícies
		writeSurfaces(&streamWriter);
	}

	//posso sair
	return 0;
}

int World::CreateGeometry3DS(HorseRadish::Streams::StreamReader * const streamReader, const Geometry::GeometryType geomType, const bool joinModels)
{
	HorseRadish::Geometry::Model *modelo;
	int numNewGeom;

	//verificar alguns dados
	if (streamReader == nullptr)
		return -1;

	//tento ler o modelo
	modelo = HorseRadish::Geometry::Factory::Read3DS(streamReader);
	if (modelo == nullptr)
		return -2;

	//se não tenho modelos nenhums
	if (modelo->arrayMesh.GetNumElements() <= 0)
		return 0;

	//mando processar o modelo
	numNewGeom = processModel(modelo, geomType, joinModels);

	//posso limpar o modelo que criei
	delete modelo;

	//correu bem (posso devolver quantas geometrias criei)
	return numNewGeom;
}

int World::CreateGeometryOBJ(HorseRadish::Streams::StreamReader * const streamReader, const Geometry::GeometryType geomType, const bool joinModels)
{
	HorseRadish::Geometry::Model *modelo;
	int numNewGeom;

	//verificar alguns dados
	if (streamReader == nullptr)
		return -1;

	//tento ler o modelo
	modelo = HorseRadish::Geometry::Factory::ReadOBJ(streamReader);
	if (modelo == nullptr)
		return -2;

	//se não tenho modelos nenhums
	if (modelo->arrayMesh.GetNumElements() <= 0)
		return 0;

	//mando processar o modelo
	numNewGeom = processModel(modelo, geomType, joinModels);

	//posso limpar o modelo que criei
	delete modelo;

	//correu bem (posso devolver quantas geometrias criei)
	return numNewGeom;
}

int World::CreateGeometryCOLLADA(HorseRadish::Streams::StreamReader * const streamReader, const Geometry::GeometryType geomType, const bool joinModels)
{
	HorseRadish::Geometry::Model *modelo;
	int numNewGeom;

	//verificar alguns dados
	if (streamReader == nullptr)
		return -1;

	//tento ler o modelo
	modelo = HorseRadish::Geometry::Factory::ReadCollada(streamReader);
	if (modelo == nullptr)
		return -2;

	//se não tenho modelos nenhums
	if (modelo->arrayMesh.GetNumElements() <= 0)
		return 0;

	//mando processar o modelo
	numNewGeom = processModel(modelo, geomType, joinModels);
	
	//posso limpar o modelo que criei
	delete modelo;

	//correu bem (posso devolver quantas geometrias criei)
	return numNewGeom;
}

int World::CreateSurface(const Surface::SurfaceType surfType, int surfaceParentID, Material * const material, Geometry * const geometry)
{
	Surface *newSurfTotal, *listaOriginal;

	//guardo a lista original (ver quando a função arranjaPonteirosSurf é chamada umas linhas de código mais à frente)
	listaOriginal = this->surfacesTotal.GetMainPointer();

	//crio mais espaço para esta nova superficie
	newSurfTotal = this->surfacesTotal.Add();
	if (newSurfTotal == nullptr)
		return -1;

	//gero o novo ID
	newSurfTotal->id = ++this->genSurfaceID;

	//copio algumas coisas
	newSurfTotal->type = surfType;
	newSurfTotal->material = material;
	newSurfTotal->geometry = geometry;

	//mando arranjar os ponteiros das superficies
	arranjaPonteirosSurf(listaOriginal);

	//se o parentID for alguma coisa de jeito, vou à procura do pai e se achar, coloco lá dentro este filho
	//(isto tem de ser feito depois de chamar a função arranjaPonteirosSurf)
	if ((surfaceParentID > 0) && (surfaceParentID != newSurfTotal->id))
	{
		//vou à procura do pai
		for(int i=0; i<this->surfacesTotal.GetNumElements(); i++)
		{
			//se achei o pai
			if (this->surfacesTotal[i].id == surfaceParentID)
			{
				//basta acrescentar um filho ao conjunto
				this->surfacesTotal[i].childs.Add(newSurfTotal);
				break;
			}
		}
	}

	//prontinho, posso sair indicando o ID da nova superficie criada
	return newSurfTotal->id;
}

void World::LoadData(HorseRadish::OpenGL::Objects::ObjectsManager *glObjectManager, HorseRadish::IO::FileSystem * const fileSystem, HorseRadish::OpenGL::Objects::ObjectsManager* const textureManager)
{
	//crio espaço para poder ter visivel todas as superficies que tenho
	this->renderContent.surfaces = new Surface*[this->surfacesTotal.GetNumElements()+1];
	if (this->renderContent.surfaces == nullptr)
		return;

	//limpo tudo
	memset(this->renderContent.surfaces, 0, sizeof(Surface*) * (this->surfacesTotal.GetNumElements()+1));
}

void World::PrepareNextFrame(const HorseRadish::OpenGL::Tools::Camera * const hrCamera, const HorseRadish::OpenGL::Tools::Viewport * const hrViewport)
{
	Surface **surfacesWalker;
	HorseRadish::OpenGL::Tools::Frustum camFrustum;
	HorseRadish::Vector camPos;

	//para onde começo a escrever as superficies que estou a visualizar
	surfacesWalker = this->renderContent.surfaces;

	//dá jeito saber a posição da camera
	camPos = hrCamera->GetPos();

	//crio o frustum para esta camera
	camFrustum.SetCamPosition(camPos);
	camFrustum.SetZNear(hrViewport->getZNear());
	camFrustum.SetZFar(hrViewport->getZFar());
	camFrustum.CalculateFrustum(hrViewport->getProj3D(), hrCamera->GetModelView());

	//passo por todas as superficies
	for(int i=0; i<this->surfacesTotal.GetNumElements(); i++)
	{
		Geometry *curGeom;
		Surface *curSurf;
		HorseRadish::Vector surfCenter;

		//tiro a superficie e geometria actual
		curSurf = this->surfacesTotal + i;
		curGeom = curSurf->geometry;

		//se esta superficie não está no frustum, posso seguir para a próxima
		/*if (camFrustum.testSphere(curGeom->bsphere) == false)
			continue;*/
		if (camFrustum.testBox(curGeom->bbox) == false)
			continue;

		//calculo a distancia desta superficie à camera
		curGeom->bbox.GetCenter(surfCenter);
		curSurf->renderValues.distToCam = camPos.GetDist(surfCenter);

		//está na camera portanto posso desenhar
		*surfacesWalker = curSurf;
		surfacesWalker++;
	}

	//fecho por fim a lista e mando ordenar
	*surfacesWalker = nullptr;
	HorseRadish::Sorting::QuickSort<Surface*>(this->renderContent.surfaces, surfacesWalker -  this->renderContent.surfaces, qsortSurfaces);
}

}//namespace Render
}//namespace HorseRadish