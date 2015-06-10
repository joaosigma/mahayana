#include "world.hpp"

#include "common\ImageFactory.hpp"

#include <algorithm>

static
bool searchBlock(HorseRadish::Streams::StreamReader * const streamReader, const int chunkTargetID, bool fromFileStart)
{
	if (fromFileStart)
		streamReader->Seek(sizeof(int)+sizeof(char)*3, HorseRadish::Streams::Stream::Begin);

	while(streamReader->CanRead() == true)
	{
		int curChunkID, curChunkSize;

		streamReader->ReadInt32(curChunkID);
		streamReader->ReadInt32(curChunkSize);

		if (curChunkID == chunkTargetID)
			return true;

		streamReader->Seek(curChunkSize, HorseRadish::Streams::Stream::Current);
	}

	return false;
}

namespace HorseRadish
{
	namespace Render
	{
		Light::Light()
		{
			this->type = Omni;
			this->active = true;
			this->insideCamera = false;
			this->noShadows = true;
			this->scissor[0] = this->scissor[1] = this->scissor[2] = this->scissor[3] = 0;
		}

		Light::~Light()
		{
		}

		Geometry::Geometry()
		{
			this->id = -1;
			this->type = Static3;
			this->renderVBOVertexOffset = 0;
			this->renderTriListOffset = nullptr;
		}

		Geometry::~Geometry()
		{
		}

		TextureSet::TextureSet()
		{
			this->id = -1;
		}

		TextureSet::~TextureSet()
		{
		}

		Surface::Surface()
			: texData(nullptr)
		{
			this->id = -1;
			this->type = Static;
			this->material = nullptr;
			this->geometry = nullptr;
			this->texSet = nullptr;
			this->renderValues.distToCam = 0.0f;
		}

		Surface::~Surface()
		{
			if (texData)
				delete texData;
			texData = nullptr;
		}

		Material::Material()
		{
			this->materialLib = nullptr;
			memset(&this->dataShadering, 0, sizeof(Shadering));
			memset(&this->dataLighting, 0, sizeof(Lighting));
			this->matProperties = (MaterialProperties)0;
			this->alphaTestValue = 1.0f;
		}

		Material::~Material()
		{
		}

		MaterialLibrary::MaterialLibrary()
		{
		}

		MaterialLibrary::~MaterialLibrary()
		{
		}

		Brush::Brush()
		{
		}

		Brush::~Brush()
		{
		}

		const int World::HRFChunckTextureSetsID = 2;
		const int World::HRFChunckGeometriesID = 1;
		const int World::HRFChunckSurfacesID = 3;

		bool World::prepareMeshForType(HorseRadish::Geometry::Mesh * const modelMesh, const Geometry::GeometryType geomType)
		{
			if (geomType == Geometry::Static1)
			{
				HorseRadish::Geometry::Mesh::MeshAtributeType attribOrder[4];

				if (modelMesh->FindAttrib(HorseRadish::Geometry::Mesh::Pos) == nullptr)
					modelMesh->NewAttrib(HorseRadish::Geometry::Mesh::Pos, modelMesh->GetNumElements());
				if (modelMesh->FindAttrib(HorseRadish::Geometry::Mesh::TexCoords) == nullptr)
					modelMesh->NewAttrib(HorseRadish::Geometry::Mesh::TexCoords, modelMesh->GetNumElements());
				if (modelMesh->FindAttribGeneric(HorseRadish::Geometry::Mesh::Generic1, 1) == nullptr)
					modelMesh->NewAttrib(HorseRadish::Geometry::Mesh::Generic1, modelMesh->GetNumElements());
				if (modelMesh->FindAttribGeneric(HorseRadish::Geometry::Mesh::Generic2, 1) == nullptr)
					modelMesh->NewAttrib(HorseRadish::Geometry::Mesh::Generic2, modelMesh->GetNumElements());

				attribOrder[0] = HorseRadish::Geometry::Mesh::Pos;
				attribOrder[1] = HorseRadish::Geometry::Mesh::TexCoords;
				attribOrder[2] = HorseRadish::Geometry::Mesh::Generic2;
				attribOrder[3] = HorseRadish::Geometry::Mesh::Generic1;
				modelMesh->ReorderAttrib(attribOrder, 4, true);

				return true;
			}

			if (geomType == Geometry::Static2)
			{
				HorseRadish::Geometry::Mesh::MeshAtributeType attribOrder[6];

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

				attribOrder[0] = HorseRadish::Geometry::Mesh::Pos;
				attribOrder[1] = HorseRadish::Geometry::Mesh::TexCoords;
				attribOrder[4] = HorseRadish::Geometry::Mesh::Generic2;
				attribOrder[5] = HorseRadish::Geometry::Mesh::Generic1;
				attribOrder[2] = HorseRadish::Geometry::Mesh::Normal;
				attribOrder[3] = HorseRadish::Geometry::Mesh::Tangent4;
				modelMesh->ReorderAttrib(attribOrder, 6, true);

				modelMesh->NewAttrib(HorseRadish::Geometry::Mesh::Generic1, modelMesh->GetNumElements());
				return true;
			}

			if (geomType == Geometry::Static3)
			{
				HorseRadish::Geometry::Mesh::MeshAtributeType attribOrder[4];

				bool createDummyUV = (modelMesh->FindAttrib(HorseRadish::Geometry::Mesh::TexCoords) == nullptr);

				if (modelMesh->FindAttrib(HorseRadish::Geometry::Mesh::Pos) == nullptr)
					modelMesh->NewAttrib(HorseRadish::Geometry::Mesh::Pos, modelMesh->GetNumElements());
				if (modelMesh->FindAttrib(HorseRadish::Geometry::Mesh::TexCoords) == nullptr)
					modelMesh->NewAttrib(HorseRadish::Geometry::Mesh::TexCoords, modelMesh->GetNumElements());
				if (modelMesh->FindAttrib(HorseRadish::Geometry::Mesh::Normal) == nullptr)
					modelMesh->NewAttrib(HorseRadish::Geometry::Mesh::Normal, modelMesh->GetNumElements());
				if (modelMesh->FindAttrib(HorseRadish::Geometry::Mesh::Tangent4) == nullptr)
					modelMesh->NewAttrib(HorseRadish::Geometry::Mesh::Tangent4, modelMesh->GetNumElements());

				if (createDummyUV)
					modelMesh->TexGen(HorseRadish::Geometry::Mesh::TexGenModel::ObjectLinear, nullptr);

				attribOrder[0] = HorseRadish::Geometry::Mesh::Pos;
				attribOrder[1] = HorseRadish::Geometry::Mesh::TexCoords;
				attribOrder[2] = HorseRadish::Geometry::Mesh::Normal;
				attribOrder[3] = HorseRadish::Geometry::Mesh::Tangent4;
				modelMesh->ReorderAttrib(attribOrder, 4, true);

				modelMesh->Ortho(HorseRadish::Geometry::Mesh::MeshOrthoCreateTangent4);

				modelMesh->NewAttrib(HorseRadish::Geometry::Mesh::Generic4, modelMesh->GetNumElements());
				return true;
			}

			return false;
		}

		void World::arranjaPonteirosGeom(const Geometry * const originalList)
		{
			auto currentList = this->geometries.data();

			if (currentList == originalList)
				return;

			for (auto& curSurf : this->surfacesTotal)
				curSurf.geometry = currentList + (curSurf.geometry - originalList);
		}

		void World::arranjaPonteirosSurf(const Surface * const originalList)
		{
			if (this->surfacesTotal.data() == originalList)
				return;

			for (unsigned int i = 0; i < this->surfacesRoot.size(); i++)
				this->surfacesRoot[i] = this->surfacesTotal.data() + (this->surfacesRoot[i] - originalList);

			for (auto& curSurf : this->surfacesTotal)
			{
				for (unsigned int j = 0; j < curSurf.childs.size(); j++)
					curSurf.childs[j] = this->surfacesTotal.data() + (curSurf.childs[j] - originalList);
			}
		}

		Geometry* World::createGeometry(HorseRadish::Geometry::Mesh * const modelMesh, const Geometry::GeometryType geomType)
		{
			float bbMin[3], bbMax[3];

			if (modelMesh->SingleBufferPointer() != nullptr)
				modelMesh->SingleBufferInv();

			if (World::prepareMeshForType(modelMesh, geomType) == false)
				return nullptr;

			modelMesh->IndexOptimize();
			modelMesh->ReorderTriIndex();

			auto listaOriginal = this->geometries.data();

			this->geometries.push_back(Geometry());
			auto& novaGeom = this->geometries[this->geometries.size() - 1];

			novaGeom.type = geomType;
			memcpy(&novaGeom.mesh, modelMesh, sizeof(HorseRadish::Geometry::Mesh));

			novaGeom.id = ++this->genGeometryID;

			memset(modelMesh, 0, sizeof(HorseRadish::Geometry::Mesh));

			novaGeom.mesh.BoundingBox(bbMin, bbMax);
			novaGeom.bbox.SetMinMax(bbMin, bbMax);
			novaGeom.bsphere.Set(novaGeom.bbox);

			novaGeom.mesh.SingleBuffer();

			arranjaPonteirosGeom(listaOriginal);
			return &novaGeom;
		}

		int World::processModel(HorseRadish::Geometry::Model * const modelo, const Geometry::GeometryType geomType, const bool joinModels)
		{
			auto oldNumGeom = this->geometries.size();

			if (joinModels == true)
			{
				auto meshAux = modelo->Join();
				createGeometry(meshAux, geomType);

				delete meshAux;
			}
			else
			{
				for (auto& curMesh : modelo->arrayMesh)
					createGeometry(&curMesh.mesh, geomType);
			}

			return (this->geometries.size() - oldNumGeom);
		}

		bool World::writeGeometries(HorseRadish::Streams::StreamWriter * const streamWriter)
		{
			int chunkSizePos, chunkEndPos;

			streamWriter->WriteInt32(World::HRFChunckGeometriesID);
			streamWriter->WriteInt32(0);
			chunkSizePos = streamWriter->GetPosition();

			streamWriter->WriteInt32(this->geometries.size());

			for (auto& curGeom : this->geometries)
			{
				float auxBuffer[10];

				streamWriter->WriteInt32(curGeom.id);
				streamWriter->WriteInt32(curGeom.type);

				curGeom.mesh.SerializeHRF(streamWriter);

				curGeom.bbox.GetMin(auxBuffer + 0);
				curGeom.bbox.GetMax(auxBuffer + 3);
				curGeom.bsphere.GetCenter(auxBuffer + 6);
				auxBuffer[9] = curGeom.bsphere.GetRadius();
				streamWriter->Write(auxBuffer, sizeof(auxBuffer));
			}

			chunkEndPos = streamWriter->GetPosition();
			streamWriter->Seek(chunkSizePos - 4, HorseRadish::Streams::Stream::Begin);
			streamWriter->WriteInt32(chunkEndPos - chunkSizePos);
			streamWriter->Seek(chunkEndPos, HorseRadish::Streams::Stream::Begin);

			return true;
		}

		bool World::readGeometries(HorseRadish::Streams::StreamReader * const streamReader)
		{
			int numGeometrias;

			if (searchBlock(streamReader, World::HRFChunckGeometriesID, true) == false)
				return false;

			streamReader->ReadInt32(numGeometrias);
			this->geometries.reserve(numGeometrias);

			for (int i = 0; i < numGeometrias; i++)
			{
				float auxBuffer[10];
				int geomType;

				this->geometries.push_back(Geometry());
				auto& curGeom = this->geometries[this->geometries.size() - 1];

				streamReader->ReadInt32(curGeom.id);
				streamReader->ReadInt32(geomType);

				curGeom.type = (Geometry::GeometryType)geomType;

				if (curGeom.type != Geometry::Static3)
					exit(0);

				curGeom.mesh.DeserializeHRF(streamReader);

				streamReader->Read(auxBuffer, sizeof(auxBuffer));
				curGeom.bbox.SetMinMax(auxBuffer + 0, auxBuffer + 3);
				curGeom.bsphere.SetCenter(auxBuffer[6], auxBuffer[7], auxBuffer[8]);
				curGeom.bsphere.SetRadius(auxBuffer[9]);
			}

			return true;
		}

		bool World::writeTextureSets(HorseRadish::Streams::StreamWriter * const streamWriter)
		{
			int chunkSizePos, chunkEndPos;

			streamWriter->WriteInt32(World::HRFChunckTextureSetsID);
			streamWriter->WriteInt32(0);
			chunkSizePos = streamWriter->GetPosition();

			streamWriter->WriteInt32(this->textureSets.size());

			for (auto& curSet : this->textureSets)
			{
				streamWriter->WriteInt32(curSet.id);
				streamWriter->WriteInt32(curSet.texs.size());

				if (curSet.texs.empty())
					continue;

				for (auto& curTex : curSet.texs)
				{
					streamWriter->WriteInt16(curTex.type);
					streamWriter->WriteInt16(curTex.format);
					streamWriter->WriteInt16(curTex.filter);
					streamWriter->WriteInt32(curTex.flags);

					streamWriter->WriteString(curTex.filePath.c_str(), true);
				}
			}

			chunkEndPos = streamWriter->GetPosition();
			streamWriter->Seek(chunkSizePos - 4, HorseRadish::Streams::Stream::Begin);
			streamWriter->WriteInt32(chunkEndPos - chunkSizePos);
			streamWriter->Seek(chunkEndPos, HorseRadish::Streams::Stream::Begin);

			return true;
		}

		bool World::readTextureSets(HorseRadish::Streams::StreamReader * const streamReader)
		{
			int numTextureSets;

			if (searchBlock(streamReader, World::HRFChunckTextureSetsID, true) == false)
				return false;

			streamReader->ReadInt32(numTextureSets);
			this->textureSets.reserve(numTextureSets);

			for (int i = 0; i < numTextureSets; i++)
			{
				int numTexs;

				this->textureSets.push_back(TextureSet());
				auto& curSet = this->textureSets[this->textureSets.size() - 1];

				streamReader->ReadInt32(curSet.id);
				streamReader->ReadInt32(numTexs);
				if (numTexs <= 0)
					continue;

				curSet.texs.reserve(numTexs);

				for (int j = 0; j < numTexs; j++)
				{
					HorseRadish::hInt16 valorAux;
					char texPath[512];

					curSet.texs.push_back(TextureSet::Texture());
					auto& curTex = curSet.texs[curSet.texs.size() - 1];

					streamReader->ReadInt16(valorAux);
					curTex.type = valorAux;
					streamReader->ReadInt16(valorAux);
					curTex.format = valorAux;
					streamReader->ReadInt16(valorAux);
					curTex.filter = valorAux;
					streamReader->ReadInt32(curTex.flags);

					streamReader->ReadUntil(texPath, sizeof(texPath), '\0');
					curTex.filePath = texPath;
				}
			}

			return true;
		}

		bool World::writeSurfaces(HorseRadish::Streams::StreamWriter * const streamWriter)
		{
			int chunkSizePos, chunkEndPos;

			streamWriter->WriteInt32(World::HRFChunckSurfacesID);
			streamWriter->WriteInt32(0);
			chunkSizePos = streamWriter->GetPosition();

			streamWriter->WriteInt32(this->surfacesTotal.size());

			for (auto& curSurf : this->surfacesTotal)
			{
				streamWriter->WriteInt32(curSurf.id);
				streamWriter->WriteInt32(curSurf.type);
				streamWriter->WriteInt32(curSurf.childs.size());

				if (curSurf.material == nullptr)
					streamWriter->WriteInt8(0);
				else
					streamWriter->WriteString(curSurf.material->materialLib->nomeMaterial.c_str(), true);

				streamWriter->WriteInt32(curSurf.geometry->id);
				streamWriter->WriteInt32(curSurf.texSet->id);
			}

			streamWriter->WriteInt32(this->surfacesRoot.size());

			for (auto& curSurf : this->surfacesRoot)
				streamWriter->WriteInt32(curSurf->id);

			chunkEndPos = streamWriter->GetPosition();
			streamWriter->Seek(chunkSizePos - 4, HorseRadish::Streams::Stream::Begin);
			streamWriter->WriteInt32(chunkEndPos - chunkSizePos);
			streamWriter->Seek(chunkEndPos, HorseRadish::Streams::Stream::Begin);

			return true;
		}

		bool World::readSurfaces(HorseRadish::Streams::StreamReader * const streamReader)
		{
			int numSurfacesTotal, numSurfacesRoot;

			if (searchBlock(streamReader, World::HRFChunckSurfacesID, true) == false)
				return false;

			streamReader->ReadInt32(numSurfacesTotal);
			this->surfacesTotal.clear();
			this->surfacesTotal.reserve(numSurfacesTotal);

			for (int i = 0; i < numSurfacesTotal; i++)
			{
				char materialName[512];
				int geomID, texSetID, numChilds, surfType;

				this->surfacesTotal.push_back(Surface());
				auto& curSurf = this->surfacesTotal[this->surfacesTotal.size() - 1];

				streamReader->ReadInt32(curSurf.id);
				streamReader->ReadInt32(surfType);
				streamReader->ReadInt32(numChilds);

				curSurf.type = (Surface::SurfaceType)surfType;

				streamReader->ReadUntil(materialName, sizeof(materialName), '\0');
				streamReader->ReadInt32(geomID);
				streamReader->ReadInt32(texSetID);

				for (auto& curGeom : this->geometries)
				{
					if (curGeom.id == geomID)
					{
						curSurf.geometry = &curGeom;
						break;
					}
				}

				for (auto& curTex : this->textureSets)
				{
					if (curTex.id == texSetID)
					{
						curSurf.texSet = &curTex;
						break;
					}
				}

				if ((curSurf.geometry == nullptr) || (curSurf.texSet == nullptr))
					return false;
			}

			streamReader->ReadInt32(numSurfacesRoot);
			this->surfacesRoot.clear();
			this->surfacesRoot.reserve(numSurfacesRoot);

			for (int i = 0; i < numSurfacesRoot; i++)
			{
				int surfRootID;

				streamReader->ReadInt32(surfRootID);

				for (unsigned int j = 0; j < this->surfacesTotal.size(); j++)
				{
					if (this->surfacesTotal[j].id == surfRootID)
					{
						this->surfacesRoot[i] = this->surfacesTotal.data() + j;
						break;
					}
				}

				if (this->surfacesRoot[i] == nullptr)
					return false;
			}

			return true;
		}

		World::World()
			: genGeometryID(0), genSurfaceID(0)
		{ }

		World::~World()
		{
			this->genGeometryID = 0;
			this->genSurfaceID = 0;
		}

		void World::Cleanup()
		{
			this->lights.clear();
			this->geometries.clear();
			this->materials.clear();
			this->textureSets.clear();

			this->genGeometryID = 0;
			this->genSurfaceID = 0;

			this->surfacesRoot.clear();
			this->surfacesTotal.clear();
		}

		int World::ImportHRF(HorseRadish::Streams::Stream &fileStream)
		{
			int fileMagic;
			HorseRadish::hInt8 fileVersionMajor, fileVersionMinor, fileVersionBuild;

			HorseRadish::Streams::StreamReader streamReader(fileStream);

			streamReader.ReadInt32(fileMagic);
			streamReader.ReadInt8(fileVersionMajor);
			streamReader.ReadInt8(fileVersionMinor);
			streamReader.ReadInt8(fileVersionBuild);

			if (fileMagic != 0xabcdeff0)
				return -4;
			if (fileVersionMajor != 1)
				return -4;

			if (readGeometries(&streamReader) == false)
				return -5;

			if (readTextureSets(&streamReader) == false)
				return -6;

			if (readSurfaces(&streamReader) == false)
				return -7;

			return 0;
		}

		int World::ExportHRF(HorseRadish::Streams::Stream &fileStream)
		{
			HorseRadish::Streams::StreamWriter streamWriter(fileStream);

			streamWriter.WriteInt32(0xabcdeff0);
			streamWriter.WriteInt8(1);
			streamWriter.WriteInt8(0);
			streamWriter.WriteInt8(0);

			writeGeometries(&streamWriter);

			writeTextureSets(&streamWriter);

			writeSurfaces(&streamWriter);

			return 0;
		}

		int World::CreateGeometry3DS(HorseRadish::Streams::StreamReader &streamReader, const Geometry::GeometryType geomType, const bool joinModels)
		{
			auto modelo = HorseRadish::Geometry::Factory::Read3DS(streamReader);
			if (modelo == nullptr)
				return -2;

			if (modelo->arrayMesh.empty())
				return 0;

			auto numNewGeom = processModel(modelo, geomType, joinModels);
			delete modelo;

			return numNewGeom;
		}

		int World::CreateGeometryOBJ(HorseRadish::Streams::StreamReader &streamReader, const Geometry::GeometryType geomType, const bool joinModels)
		{
			auto modelo = HorseRadish::Geometry::Factory::ReadOBJ(streamReader);
			if (modelo == nullptr)
				return -2;

			if (modelo->arrayMesh.empty())
				return 0;

			auto numNewGeom = processModel(modelo, geomType, joinModels);
			delete modelo;

			return numNewGeom;
		}

		int World::CreateSurface(const Surface::SurfaceType surfType, int surfaceParentID, Material * const material, Geometry * const geometry)
		{
			auto listaOriginal = this->surfacesTotal.data();

			this->surfacesTotal.push_back(Surface());
			auto& newSurfTotal = this->surfacesTotal[this->surfacesTotal.size() - 1];

			newSurfTotal.id = ++this->genSurfaceID;

			newSurfTotal.type = surfType;
			newSurfTotal.material = material;
			newSurfTotal.geometry = geometry;

			arranjaPonteirosSurf(listaOriginal);

			if ((surfaceParentID > 0) && (surfaceParentID != newSurfTotal.id))
			{
				for (auto &curSurf : this->surfacesTotal)
				{
					if (curSurf.id == surfaceParentID)
					{
						curSurf.childs.push_back(&newSurfTotal);
						break;
					}
				}
			}

			return newSurfTotal.id;
		}

		void World::LoadData(HorseRadish::IO::FileSystem * const fileSystem)
		{
			this->renderContent.surfaces.resize(this->surfacesTotal.size() + 1);
			this->renderContent.surfaces[0] = nullptr;
		}

		void World::PrepareNextFrame(const Tools::Camera& hrCamera, const HorseRadish::OpenGL::Tools::Viewport& hrViewport)
		{
			HorseRadish::OpenGL::Tools::Frustum camFrustum;
			HorseRadish::Vector camPos;

			camPos = hrCamera.GetPos();

			camFrustum.setCamPosition(camPos);
			camFrustum.setZNear(hrViewport.getZNear());
			camFrustum.setZFar(hrViewport.getZFar());
			camFrustum.calculateFrustum(hrViewport.getProjection(HorseRadish::OpenGL::Tools::Viewport::ProjectionType::Proj3D), hrCamera.GetModelView());

			int numValidSurfaces = 0;
			for (auto& curSurf : this->surfacesTotal)
			{
				HorseRadish::Vector surfCenter;

				auto curGeom = curSurf.geometry;

				/*if (camFrustum.testSphere(curGeom->bsphere) == false)
					continue;*/
				if (camFrustum.testBox(curGeom->bbox) == false)
					continue;

				curGeom->bbox.GetCenter(surfCenter);
				curSurf.renderValues.distToCam = camPos.GetDistance(surfCenter);

				this->renderContent.surfaces[numValidSurfaces] = &curSurf;
				numValidSurfaces++;
			}

			this->renderContent.surfaces[numValidSurfaces] = nullptr;
			if (numValidSurfaces <= 0)
				return;

			std::sort(this->renderContent.surfaces.begin(), this->renderContent.surfaces.begin() + numValidSurfaces, [](const Surface* surfA, const Surface* surfB)
			{
				if (surfA->material == surfB->material)
					return (surfA->renderValues.distToCam < surfB->renderValues.distToCam);

				return (surfA->material < surfB->material);
			});
		}

	} //Render
} //HorseRadish