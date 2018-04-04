#include "world.hpp"

#include "common/encoders.hpp"
#include "common/imageFactory.hpp"
#include "common/fileSystem.hpp"

#include "libs/lz4/lz4.h"
#include "libs/lz4/lz4hc.h"
#include "libs/cppformat/format.h"
#include "libs/rapidjson/document.h"
#include "libs/rapidjson/rapidjson.h"
#include "libs/rapidjson/prettywriter.h"
#include "libs/rapidjson/stringbuffer.h"
#include "libs/tinyobjloader/tiny_obj_loader.h"

#include <array>
#include <cstdint>
#include <algorithm>
#include <unordered_set>
#include <experimental/filesystem>

namespace hr { namespace render
{
	namespace
	{
#pragma pack(push, 1)
		std::array<unsigned char, 6> GeomFileSig = { 'h', 'r', 'g', 'e', 'o', 'm' };

		struct GeomHeader
		{
			unsigned char fileSig[GeomFileSig.size()];
			unsigned char version;
			std::uint32_t numGeoms;
		};

		struct GeomChunkInfo
		{
			std::uint32_t id;
			std::uint32_t geomsOffset;
			std::uint32_t size;
			std::uint32_t numVertices;
			std::uint32_t numIndices;
			float bboxMin[3], bboxMax[3];
		};
#pragma pack(pop)
	}

	class RendererObjectProxy
		: public hr::render::IRenderObject
	{
	public:
		size_t objectId = 0;
		struct {
			hr::BBox bbox;
			size_t numVertices = 0, numIndices = 0;
			size_t fstreamVertexOffset = 0, fstreamIndexOffset = 0;
		} geom;
		std::string matDiffusePath, matNormalPath;
		hr::streams::FileStream& fileStream;

	public:
		RendererObjectProxy(hr::streams::FileStream& fileStream)
			: fileStream(fileStream)
		{ }

	private:
		size_t id() const override
		{
			return objectId;
		}

		BBox bbox() const override
		{
			return geom.bbox;
		}

		size_t numVertices() const override
		{
			return geom.numVertices;
		}

		size_t numIndices() const override
		{
			return geom.numIndices;
		}

		std::string_view texDiffusePath() const override
		{
			return matDiffusePath;
		}

		std::string_view texNormalPath() const override
		{
			return matNormalPath;
		}

		size_t readVertices(void* const destBuffer, size_t requestedDataSize) override
		{
			fileStream.seek(hr::streams::Stream::SeekOrigin::Begin, geom.fstreamVertexOffset);
			
			auto bytesRead = fileStream.read(destBuffer, requestedDataSize);
			assert(bytesRead == requestedDataSize);
			
			return bytesRead;
		}

		size_t readIndices(void* const destBuffer, size_t requestedDataSize) override
		{
			fileStream.seek(hr::streams::Stream::SeekOrigin::Begin, geom.fstreamIndexOffset);
			
			auto bytesRead = fileStream.read(destBuffer, requestedDataSize);
			assert(bytesRead == requestedDataSize);

			return bytesRead;
		}
	};

	World::World(bool editorMode)
		: m_editorMode(editorMode)
	{ }

	World::~World()
	{
		std::vector<AreaId> ids;

		ids.reserve(mAreas.size());
		for (auto&& [areaId, area] : mAreas)
			ids.push_back(areaId);

		for (auto&& areaId : ids)
			unloadArea(areaId);
	}

	World::AreaId World::loadArea(IRenderer& renderer, const std::string& scenePath, const std::string& geomPath)
	{
		auto id = mGenAreaIds++;

		auto& area = mAreas[id];
		area.id = id;

		auto success = load(renderer, area, scenePath, geomPath);
		if (!success)
		{
			unloadArea(id);
			return 0;
		}

		return id;
	}

	void World::unloadArea(AreaId areaId)
	{

	}

	void World::prepareNextFrame(IRenderer& renderer, const tools::Camera& hrCamera, const hr::gl::tools::Viewport& hrViewport)
	{
		auto camPos = hrCamera.getPos();

		hr::gl::tools::Frustum camFrustum;
		camFrustum.setCamPosition(camPos);
		camFrustum.setZNear(hrViewport.znear());
		camFrustum.setZFar(hrViewport.zfar());
		camFrustum.calculateFrustum(hrViewport.getProjection(hr::gl::tools::Viewport::ProjectionType::Proj3D), hrCamera.modelView());

		std::vector<IRenderObject::ObjectId> targetObjects;

		for (const auto& [areaId, area] : mAreas)
		{
			targetObjects.clear();

			for (auto& instance : area.mInstances)
			{
				if (!camFrustum.testBox(instance.bbox))
					continue;

				targetObjects.push_back(instance.objectId);
			}

			renderer.prepareNextFrame(area.sceneId, targetObjects);
		}
	}

	World::AreaId World::addEmptyArea()
	{
		auto id = mGenAreaIds++;

		auto& area = mAreas[id];
		area.id = id;

		return id;
	}

	bool World::load(IRenderer& renderer, Area& area, const std::string& scenePath, const std::string& geomPath)
	{
		hr::streams::FileStream sceneFileStream(scenePath, true, false);
		hr::streams::StreamReader streamScene(sceneFileStream);

		area.mGeomFileStream = hr::streams::FileStream(geomPath, true, m_editorMode ? true : false);
		hr::streams::StreamReader streamGeom(area.mGeomFileStream);

		std::map<size_t, RendererObjectProxy> mRendererObjects;

		{
			std::unordered_map<size_t, GeomChunkInfo> geomInfo;

			//read geom info
			size_t geomsOffset = 0;
			{
				GeomHeader geomHeader;
				if (streamGeom.read(&geomHeader, sizeof(GeomHeader)) != sizeof(GeomHeader))
					return false;
				if (std::memcmp(geomHeader.fileSig, GeomFileSig.data(), sizeof(geomHeader.fileSig)) != 0)
					return false;

				geomsOffset = streamGeom.position();
				streamGeom.seek(hr::streams::Stream::SeekOrigin::End, -static_cast<int>(sizeof(GeomChunkInfo) * geomHeader.numGeoms));

				geomInfo.rehash(geomHeader.numGeoms);
				for (size_t curGeom = 0; curGeom < geomHeader.numGeoms; ++curGeom)
				{
					GeomChunkInfo chunkInfo;
					streamGeom.read(&chunkInfo, sizeof(GeomChunkInfo));

					geomInfo[chunkInfo.id] = chunkInfo;
				}
			}

			//read scene
			{
				std::shared_ptr<unsigned char> buffer;
				size_t bufferSize;

				streamScene.stream().cloneAllContent(buffer, bufferSize, [](size_t requiredSize)
				{
					std::shared_ptr<unsigned char> buffer(new unsigned char[requiredSize + 1], std::default_delete<unsigned char[]>());
					buffer.get()[requiredSize] = '\0';
					return buffer;
				});

				rapidjson::Document d;
				d.ParseInsitu(reinterpret_cast<char*>(buffer.get()));

				{
					auto& jsonConcepts = d["concepts"];
					assert(jsonConcepts.IsArray());

					for (rapidjson::Value::ConstValueIterator itr = jsonConcepts.Begin(); itr != jsonConcepts.End(); ++itr)
					{
						size_t objectId = (*itr)["id"].GetUint();

						auto& object = area.mObjects[objectId];
						auto& objectRenderer = mRendererObjects.try_emplace(objectId, area.mGeomFileStream).first->second;

						object.objectId = objectId;
						objectRenderer.objectId = objectId;

						//read geom info and bbox
						{
							auto geomIt = geomInfo.find(object.objectId);
							if (geomIt == geomInfo.end())
								continue;

							objectRenderer.geom.numVertices = geomIt->second.numVertices;
							objectRenderer.geom.numIndices = geomIt->second.numIndices;
							objectRenderer.geom.fstreamVertexOffset = geomsOffset + geomIt->second.geomsOffset;
							objectRenderer.geom.fstreamIndexOffset = objectRenderer.geom.fstreamVertexOffset + (sizeof(hr::geom::Mesh::VertexData) * objectRenderer.geom.numVertices);

							object.bbox.setMinMax(geomIt->second.bboxMin, geomIt->second.bboxMax);
							objectRenderer.geom.bbox = object.bbox;
						}

						//read material info
						if ((*itr).HasMember("material"))
						{
							auto& jsonMaterial = ((*itr).FindMember("material"))->value;

							objectRenderer.matDiffusePath = jsonMaterial["diffusePath"].GetString();
							objectRenderer.matNormalPath = jsonMaterial["normalPath"].GetString();
						}
					}
				}

				{
					auto& jsonObjects = d["objects"];
					assert(jsonObjects.IsArray());

					area.mInstances.reserve(jsonObjects.Size());
					for (rapidjson::Value::ConstValueIterator itr = jsonObjects.Begin(); itr != jsonObjects.End(); ++itr)
					{
						Instance newInstance;
						newInstance.type = static_cast<Instance::Type>((*itr)["type"].GetUint());
						newInstance.objectId = (*itr)["conceptId"].GetUint();
						if (area.mObjects.find(newInstance.objectId) == area.mObjects.end())
							continue;

						newInstance.bbox = area.mObjects[newInstance.objectId].bbox;
						area.mInstances.push_back(newInstance);
					}
				}
			}
		}

		//send data to the render
		{
			struct RendererProxy
				: public IRenderObjectManager
			{
				std::map<size_t, RendererObjectProxy>& renderObjects;

				RendererProxy(std::map<size_t, RendererObjectProxy>& renderObjects)
					: renderObjects(renderObjects)
				{ }

				size_t numObjects() const override
				{
					return renderObjects.size();
				}

				void iterateObjects(std::function<void(IRenderObject&)> cb) const override
				{
					if (!cb)
						return;

					for (auto&[objectId, object] : renderObjects)
						cb(object);
				}
			};

			area.sceneId = renderer.loadScene(RendererProxy(mRendererObjects));
		}

		return true;
	}

	void WorldEditor::createEmptyArea(const std::string& scenePath, const std::string& geomPath)
	{
		if (std::experimental::filesystem::exists(scenePath))
			std::experimental::filesystem::remove(scenePath);
		if (std::experimental::filesystem::exists(geomPath))
			std::experimental::filesystem::remove(geomPath);

		//empty scene
		{
			hr::streams::FileStream streamScene(scenePath, false, true);

			rapidjson::StringBuffer s;
			rapidjson::PrettyWriter<rapidjson::StringBuffer> writer(s);

			writer.StartObject();

			writer.String("version");
			writer.StartArray();
			writer.Int(1);
			writer.Int(0);
			writer.Int(0);
			writer.EndArray();

			writer.String("concepts");
			writer.StartArray();
			writer.EndArray();

			writer.String("objects");
			writer.StartArray();
			writer.EndArray();

			writer.EndObject();

			streamScene.write(s.GetString(), s.GetSize());
		}

		//empty geom
		{
			hr::streams::FileStream streamGeom(geomPath, false, true);

			GeomHeader geomHeader;
			geomHeader.numGeoms = 0;
			geomHeader.version = 1;
			std::memcpy(geomHeader.fileSig, GeomFileSig.data(), sizeof(geomHeader.fileSig));

			streamGeom.write(&geomHeader, sizeof(geomHeader));
		}
	}

	WorldEditor::WorldEditor()
		: World(true)
	{ }

	WorldEditor::AreaId WorldEditor::newArea(std::string_view scenePath, std::string_view geomPath)
	{
		auto newId = addEmptyArea();

		assert(mAreasData.find(newId) == mAreasData.end());
		auto& areaData = mAreasData[newId];

		areaData.pathScene = scenePath;
		areaData.pathGeom = geomPath;

		return newId;
	}

	bool WorldEditor::importMesh(AreaId areaId, std::string_view name, const hr::geom::Mesh& mesh)
	{
		if (!mesh.check() || (mAreas.find(areaId) == mAreas.end()))
			return false;

		auto& area = mAreas[areaId];
		auto& areaData = mAreasData[areaId];

		//create object
		auto objectId = genObjectId(area);
		{
			auto& object = area.mObjects[objectId];
			auto& objectData = areaData.objects[objectId];
			
			object.objectId = objectId;
			objectData.objectId = objectId;
			objectData.name = name;
		}

		//we store new geometry immediately
		addMesh(area, objectId, mesh);

		//create an instance associated with the object
		{
			Instance newInstance;
			newInstance.type = Instance::Type::Static;
			newInstance.objectId = objectId;
			newInstance.bbox = mesh.getBoundingBox();
			area.mInstances.push_back(newInstance);
		}

		//need to save everything to file (new geometry was already saved)
		saveArea(area);

		return true;
	}

	bool WorldEditor::importObj(AreaId areaId, std::string_view basePath, std::string_view fileName)
	{
		if (mAreas.find(areaId) == mAreas.end())
			return false;

		tinyobj::attrib_t objVertexAttribs;
		std::vector<tinyobj::shape_t> objShapes;
		std::vector<tinyobj::material_t> objMaterials;

		{
			std::string fullPath;
			fullPath.reserve(basePath.size() + fileName.size() + 1);
			fullPath.append(basePath).append(fileName);

			std::string errorDesc;
			auto success = tinyobj::LoadObj(&objVertexAttribs, &objShapes, &objMaterials, &errorDesc, fullPath.c_str(), std::string(basePath).c_str());
			if (!success)
				return false;
		}

		if (objShapes.empty())
			return true;

		auto& area = mAreas[areaId];
		auto& areaData = mAreasData[areaId];

		area.mInstances.reserve(area.mInstances.size() + objShapes.size());

		for (const auto& shape : objShapes)
		{
			//ignore empty shapes
			if (shape.mesh.indices.empty())
				continue;

			//all tris must belong to the same material
			if (!shape.mesh.material_ids.empty())
			{
				if (std::count(shape.mesh.material_ids.begin(), shape.mesh.material_ids.end(), shape.mesh.material_ids[0]) != shape.mesh.material_ids.size())
					continue;
			}

			//all indices must have position and tex coords
			if (std::any_of(shape.mesh.indices.begin(), shape.mesh.indices.end(), [](const tinyobj::index_t& index)
			{
				return ((index.vertex_index == -1) || (index.texcoord_index == -1));
			}))
				continue;

			//ignore shapes if a object with the same name already exists
			if (std::find_if(areaData.objects.begin(), areaData.objects.end(), [&name = shape.name](const auto& keyValue) { return (keyValue.second.name == name); }) != areaData.objects.end())
				continue;

			//create object
			auto objectId = genObjectId(area);
			auto& object = area.mObjects[objectId];
			auto& objectData = areaData.objects[objectId];

			object.objectId = objectId;
			objectData.objectId = objectId;
			objectData.name = shape.name;

			//process mesh
			{
				bool ignoreNormals = std::any_of(shape.mesh.indices.begin(), shape.mesh.indices.end(), [](const tinyobj::index_t& index)
				{
					return (index.normal_index == -1);
				});

				hr::geom::Mesh newMesh;
				{
					struct hashableIndex
						: public tinyobj::index_t
					{
						struct hash
						{
							size_t operator()(const hashableIndex& index) const
							{
								size_t seed = index.vertex_index;
								seed ^= index.texcoord_index + 0x9e3779b9 + (seed << 6) + (seed >> 2);
								seed ^= index.normal_index + 0x9e3779b9 + (seed << 6) + (seed >> 2);
								return seed;
							}
						};

						hashableIndex(tinyobj::index_t index)
							: tinyobj::index_t(index)
						{ }

						bool operator ==(const hashableIndex& other) const
						{
							return (vertex_index == other.vertex_index) && (texcoord_index == other.texcoord_index) && (normal_index == other.normal_index);
						}
					};

					std::unordered_map<hashableIndex, size_t, hashableIndex::hash> mapping;

					for (const auto& index : shape.mesh.indices)
					{
						if (mapping.find(index) != mapping.end())
							continue;

						mapping[index] = mapping.size();
					}

					if (mapping.size() >= hr::geom::Mesh::maxVertexCount())
						continue;

					newMesh = hr::geom::Mesh(mapping.size(), shape.mesh.indices.size());

					for (const auto& keyValue : mapping)
					{
						auto vertexIndex = keyValue.second;

						newMesh.vertices()[vertexIndex].pos[0] = objVertexAttribs.vertices[keyValue.first.vertex_index * 3 + 0];
						newMesh.vertices()[vertexIndex].pos[1] = objVertexAttribs.vertices[keyValue.first.vertex_index * 3 + 1];
						newMesh.vertices()[vertexIndex].pos[2] = objVertexAttribs.vertices[keyValue.first.vertex_index * 3 + 2];

						newMesh.vertices()[vertexIndex].uv[0] = objVertexAttribs.texcoords[keyValue.first.texcoord_index * 2 + 0];
						newMesh.vertices()[vertexIndex].uv[1] = objVertexAttribs.texcoords[keyValue.first.texcoord_index * 2 + 1];

						if (!ignoreNormals)
						{
							newMesh.vertices()[vertexIndex].normal[0] = hr::geom::Mesh::pack(objVertexAttribs.normals[keyValue.first.normal_index * 3 + 0]);
							newMesh.vertices()[vertexIndex].normal[1] = hr::geom::Mesh::pack(objVertexAttribs.normals[keyValue.first.normal_index * 3 + 1]);
							newMesh.vertices()[vertexIndex].normal[2] = hr::geom::Mesh::pack(objVertexAttribs.normals[keyValue.first.normal_index * 3 + 2]);
						}
					}

					size_t curIndex = 0;
					for (const auto& index : shape.mesh.indices)
					{
						auto mappingIt = mapping.find(index);
						assert(mappingIt != mapping.end());

						newMesh.indices()[curIndex++] = static_cast<unsigned short>(mappingIt->second);
					}
				}

				assert(newMesh.check());
				if (ignoreNormals)
					newMesh.genNormals();
				newMesh.genTangents4();

				newMesh.optimizeIndices();

				//we store new geometry immediately
				addMesh(area, objectId, newMesh);
			}

			//process material
			if (!shape.mesh.material_ids.empty())
			{
				auto materialId = shape.mesh.material_ids[0];
				if ((materialId >= 0) && (materialId < objMaterials.size()))
				{
					const auto& mat = objMaterials[materialId];

					if (!mat.diffuse_texname.empty())
						objectData.matDiffusePath = mat.diffuse_texname;
					else if (!mat.ambient_texname.empty())
						objectData.matDiffusePath = mat.ambient_texname;
					else
						materialId = materialId;

					if (!mat.normal_texname.empty())
						objectData.matNormalPath = mat.normal_texname;
					else if (!mat.bump_texname.empty())
						objectData.matNormalPath = mat.bump_texname;
					else if (mat.unknown_parameter.find("bump") != mat.unknown_parameter.end())
						objectData.matNormalPath = mat.unknown_parameter.find("bump")->second;
					else if (mat.unknown_parameter.find("map_bump") != mat.unknown_parameter.end())
						objectData.matNormalPath = mat.unknown_parameter.find("map_bump")->second;
					else
						materialId = materialId;
				}
			}

			//create an object associated with the concept
			Instance newInstance;
			newInstance.type = Instance::Type::Static;
			newInstance.objectId = object.objectId;
			newInstance.bbox = object.bbox;
			area.mInstances.push_back(newInstance);
		}

		//need to save everything to file (new geometry was already saved)
		saveArea(area);

		return true;
	}

	void WorldEditor::processMesh(AreaId areaId, const std::vector<size_t>& objectIds, std::function<void(hr::geom::Mesh&)> cb)
	{
		if (!cb || (mAreas.find(areaId) == mAreas.end()))
			return;

		auto& area = mAreas[areaId];
		auto& areaData = mAreasData[areaId];

		auto func = [cb](Area& area, AreaData::ObjectData& object)
		{
			hr::geom::Mesh mesh(object.geom.numVertices, object.geom.numIndices);

			area.mGeomFileStream.seek(hr::streams::Stream::SeekOrigin::Begin, object.geom.fstreamVertexOffset);
			area.mGeomFileStream.read(mesh.vertices(), mesh.sizeVertices());

			area.mGeomFileStream.seek(hr::streams::Stream::SeekOrigin::Begin, object.geom.fstreamIndexOffset);
			area.mGeomFileStream.read(mesh.indices(), mesh.sizeIndices());

			{
				auto numVertices = mesh.numVertices();
				auto numIndices = mesh.numIndices();
				cb(mesh);

				if ((mesh.numVertices() != numVertices) || (mesh.numIndices() != numIndices))
					return;
			}

			area.mGeomFileStream.seek(hr::streams::Stream::SeekOrigin::Begin, object.geom.fstreamVertexOffset);
			area.mGeomFileStream.write(mesh.vertices(), mesh.sizeVertices());

			area.mGeomFileStream.seek(hr::streams::Stream::SeekOrigin::Begin, object.geom.fstreamIndexOffset);
			area.mGeomFileStream.write(mesh.indices(), mesh.sizeIndices());
		};

		if (objectIds.empty())
		{
			for (auto& [objectId, object] : area.mObjects)
			{
				assert(areaData.objects.find(objectId) != areaData.objects.end());
				func(area, areaData.objects[objectId]);
			}
		}
		else
		{
			for (const auto& objectId : objectIds)
			{
				auto it = area.mObjects.find(objectId);
				if (it == area.mObjects.end())
					continue;

				assert(areaData.objects.find(objectId) != areaData.objects.end());
				func(area, areaData.objects[objectId]);
			}
		}
	}

	std::vector<size_t> WorldEditor::unusedObjects(AreaId areaId) const
	{
		auto areaIt = mAreas.find(areaId);
		if (areaIt == mAreas.end())
			return { };

		const auto& area = areaIt->second;

		std::vector<size_t> objectIds;
		for (const auto& [objectId, object] : area.mObjects)
		{
			if (std::find_if(area.mInstances.begin(), area.mInstances.end(), [objectId](const auto& instance) { return (instance.objectId == objectId); }) != area.mInstances.end())
				continue;

			objectIds.push_back(objectId);
		}

		return objectIds;
	}

	void WorldEditor::removeObjects(AreaId areaId, const std::vector<size_t>& objectIds)
	{
		if (mAreas.find(areaId) == mAreas.end())
			return;

		auto& area = mAreas[areaId];

		for (const auto& objectId : objectIds)
		{
			area.mObjects.erase(objectId);
			std::remove_if(area.mInstances.begin(), area.mInstances.end(), [objectId](const auto& instance) { return (instance.objectId == objectId); });
		}
	}

	size_t WorldEditor::genObjectId(Area& area) const
	{
		size_t curId = 1;
		while (area.mObjects.find(curId) != area.mObjects.end())
			curId++;

		return curId;
	}

	void WorldEditor::addMesh(Area& area, size_t geomId, const hr::geom::Mesh& mesh)
	{
		size_t numGeoms = 0;

		//update file header
		{
			GeomHeader geomHeader;
			{
				hr::streams::StreamReader stream(area.mGeomFileStream);

				stream.seek(hr::streams::Stream::SeekOrigin::Begin, 0);
				if (stream.read(&geomHeader, sizeof(GeomHeader)) != sizeof(GeomHeader))
					return;
			}

			geomHeader.numGeoms++;
			numGeoms = geomHeader.numGeoms;

			{
				hr::streams::StreamWriter stream(area.mGeomFileStream);

				stream.seek(hr::streams::Stream::SeekOrigin::Begin, 0);
				if (stream.write(&geomHeader, sizeof(GeomHeader)) != sizeof(GeomHeader))
					return;
			}
		}

		GeomChunkInfo newGeomChunk;
		newGeomChunk.id = geomId;
		newGeomChunk.numVertices = mesh.numVertices();
		newGeomChunk.numIndices = mesh.numIndices();
		newGeomChunk.size = mesh.sizeVertices() + mesh.sizeVertices();
		newGeomChunk.geomsOffset = 0;
		{
			auto bbox = mesh.getBoundingBox();
			bbox.min(newGeomChunk.bboxMin);
			bbox.max(newGeomChunk.bboxMax);
		}

		//if this is the first geom, simply write it and then the chunk
		if (numGeoms == 1)
		{
			hr::streams::StreamWriter stream(area.mGeomFileStream);

			stream.seek(hr::streams::Stream::SeekOrigin::Begin, sizeof(GeomHeader));

			if (stream.write(mesh.vertices(), mesh.sizeVertices()) != mesh.sizeVertices())
				return;
			if (stream.write(mesh.indices(), mesh.sizeIndices()) != mesh.sizeIndices())
				return;

			if (stream.write(&newGeomChunk, sizeof(GeomChunkInfo)) != sizeof(GeomChunkInfo))
				return;

			area.mGeomFileStream.flush();
			return;
		}

		int chunksSize = sizeof(GeomChunkInfo) * (numGeoms - 1);

		//read all geom chunks
		auto chunksTmp = std::unique_ptr<GeomChunkInfo[]>(new GeomChunkInfo[numGeoms - 1]);
		{
			hr::streams::StreamReader stream(area.mGeomFileStream);

			stream.seek(hr::streams::Stream::SeekOrigin::End, -chunksSize);
			if (stream.read(chunksTmp.get(), chunksSize) != chunksSize)
				return;
		}
		
		//write the new geom, old chunks and the new chunk
		{
			hr::streams::StreamWriter stream(area.mGeomFileStream);

			stream.seek(hr::streams::Stream::SeekOrigin::End, -chunksSize);
			newGeomChunk.geomsOffset = stream.position() - sizeof(GeomHeader);

			if (stream.write(mesh.vertices(), mesh.sizeVertices()) != mesh.sizeVertices())
				return;
			if (stream.write(mesh.indices(), mesh.sizeIndices()) != mesh.sizeIndices())
				return;

			if (stream.write(chunksTmp.get(), chunksSize) != chunksSize)
				return;
			
			if (stream.write(&newGeomChunk, sizeof(GeomChunkInfo)) != sizeof(GeomChunkInfo))
				return;
		}

		area.mGeomFileStream.flush();
	}

	void WorldEditor::saveArea(Area& area)
	{
		assert(mAreasData.find(area.id) == mAreasData.end());
		auto& areaData = mAreasData[area.id];

		//the area is always exported whole, which means that we can destroy the old version completly
		//also: we only need to save the scene (geom was already taken care of)

		if (std::experimental::filesystem::exists(areaData.pathScene))
			std::experimental::filesystem::remove(areaData.pathScene);

		hr::streams::FileStream streamScene(areaData.pathScene, false, true);

		rapidjson::StringBuffer s;
		rapidjson::PrettyWriter<rapidjson::StringBuffer> writer(s);

		writer.StartObject();

		writer.String("version");
		writer.StartArray();
		writer.Int(1);
		writer.Int(0);
		writer.Int(0);
		writer.EndArray();

		writer.String("concepts");
		writer.StartArray();
		for (auto& object : areaData.objects)
		{
			writer.StartObject();

			writer.String("id");
			writer.Uint(object.second.objectId);

			writer.String("name");
			writer.String(object.second.name.c_str());

			writer.String("material");
			writer.StartObject();
			writer.String("diffusePath");
			writer.String(object.second.matDiffusePath.c_str());

			writer.String("normalPath");
			writer.String(object.second.matNormalPath.c_str());
			writer.EndObject();

			writer.EndObject();
		}
		writer.EndArray();

		writer.String("objects");
		writer.StartArray();
		for (auto& curObject : area.mInstances)
		{
			writer.StartObject();

			writer.String("type");
			writer.Uint(static_cast<unsigned int>(curObject.type));

			writer.String("conceptId");
			writer.Uint(curObject.objectId);

			writer.EndObject();
		}
		writer.EndArray();

		writer.EndObject();

		streamScene.write(s.GetString(), s.GetSize());
	}
} }