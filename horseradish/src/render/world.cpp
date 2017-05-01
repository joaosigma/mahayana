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

	World::World(const std::string& scenePath, const std::string& geomPath, bool editorMode)
		: m_editorMode(editorMode)
	{
		auto success = loadWorld(scenePath, geomPath);
		if (!success)
		{
			cleanup();
			return;
		}
	}

	World::~World()
	{
		cleanup();
	}

	void World::loadData(hr::io::FileSystem& fileSystem)
	{
		mRenderData.objects.reserve(mObjects.size());
	}

	void World::prepareNextFrame(const tools::Camera& hrCamera, const hr::gl::tools::Viewport& hrViewport)
	{
		auto camPos = hrCamera.getPos();

		hr::gl::tools::Frustum camFrustum;
		camFrustum.setCamPosition(camPos);
		camFrustum.setZNear(hrViewport.znear());
		camFrustum.setZFar(hrViewport.zfar());
		camFrustum.calculateFrustum(hrViewport.getProjection(hr::gl::tools::Viewport::ProjectionType::Proj3D), hrCamera.modelView());

		mRenderData.objects.clear();
		for (auto& curObject : mObjects)
		{
			if (!camFrustum.testBox(curObject.bbox))
				continue;

			mRenderData.objects.push_back(&curObject);
		}

		if (mRenderData.objects.empty())
			return;
	}

	void World::cleanup()
	{
		mConcepts.clear();
		mObjects.clear();

		mRenderData.objects.clear();
	}

	bool World::loadWorld(const std::string& scenePath, const std::string& geomPath)
	{
		hr::streams::FileStream sceneFileStream(scenePath, true, false);
		hr::streams::StreamReader streamScene(sceneFileStream);

		mGeomFileStream = hr::streams::FileStream(geomPath, true, m_editorMode ? true : false);
		hr::streams::StreamReader streamGeom(mGeomFileStream);

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
			streamGeom.seek(hr::streams::Stream::SeekOrigin::End, -(sizeof(GeomChunkInfo) * geomHeader.numGeoms));

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
					size_t conceptId = (*itr)["id"].GetUint();

					auto& concept = mConcepts[conceptId];
					concept.id = conceptId;
					concept.name = (*itr)["name"].GetString();

					auto geomIt = geomInfo.find(concept.id);
					if (geomIt == geomInfo.end())
						continue;

					concept.geom.numVertices = geomIt->second.numVertices;
					concept.geom.numIndices = geomIt->second.numIndices;
					concept.geom.fstreamVertexOffset = geomsOffset + geomIt->second.geomsOffset;
					concept.geom.fstreamIndexOffset = concept.geom.fstreamVertexOffset + (sizeof(hr::geom::Mesh::VertexData) * concept.geom.numVertices);
					concept.geom.bbox.setMinMax(geomIt->second.bboxMin, geomIt->second.bboxMax);

					if ((*itr).HasMember("material"))
					{
						auto& jsonMaterial = ((*itr).FindMember("material"))->value;

						concept.matDiffusePath = jsonMaterial["diffusePath"].GetString();
						concept.matNormalPath = jsonMaterial["normalPath"].GetString();
					}
				}
			}

			{
				auto& jsonObjects = d["objects"];
				assert(jsonObjects.IsArray());

				mObjects.reserve(jsonObjects.Size());
				for (rapidjson::Value::ConstValueIterator itr = jsonObjects.Begin(); itr != jsonObjects.End(); ++itr)
				{
					Object newObject;
					newObject.type = static_cast<Object::Type>((*itr)["type"].GetUint());
					newObject.conceptId = (*itr)["conceptId"].GetUint();
					if (mConcepts.find(newObject.conceptId) == mConcepts.end())
						continue;

					newObject.bbox = mConcepts[newObject.conceptId].geom.bbox;
					mObjects.push_back(newObject);
				}
			}
		}

		return true;
	}

	void WorldEditor::createEmptyScene(const std::string& scenePath, const std::string& geomPath)
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

	WorldEditor::WorldEditor(const std::string& scenePath, const std::string& geomPath)
		: World(scenePath, geomPath, true)
	{
		m_paths.scene = scenePath;
		m_paths.geom = geomPath;
	}

	bool WorldEditor::importMesh(const std::string& name, const hr::geom::Mesh& mesh)
	{
		if (!mesh.check())
			return false;

		//create concept
		auto conceptId = genId();
		{
			auto& concept = mConcepts[conceptId];
			concept.id = conceptId;
			concept.name = name;
		}

		//we store new geometry immediately
		addMesh(conceptId, mesh);

		//create an object associated with the concept
		{
			Object newObject;
			newObject.type = Object::Type::Static;
			newObject.conceptId = conceptId;
			newObject.bbox = mesh.getBoundingBox();
			mObjects.push_back(newObject);
		}

		//need to save everything to file (new geometry was already saved)
		saveScene();

		return true;
	}

	bool WorldEditor::importObj(const std::string& basePath, const std::string& fileName)
	{
		tinyobj::attrib_t objVertexAttribs;
		std::vector<tinyobj::shape_t> objShapes;
		std::vector<tinyobj::material_t> objMaterials;

		{
			std::string errorDesc;
			auto success = tinyobj::LoadObj(&objVertexAttribs, &objShapes, &objMaterials, &errorDesc, (basePath + fileName).c_str(), basePath.c_str());
			if (!success)
				return false;
		}

		if (objShapes.empty())
			return true;

		mObjects.reserve(mObjects.size() + objShapes.size());

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

			//ignore shapes if a concept with the same name already exists
			if (std::find_if(mConcepts.begin(), mConcepts.end(), [&name = shape.name](const std::map<size_t, Concept>::value_type& keyValue) { return (keyValue.second.name == name); }) != mConcepts.end())
				continue;

			//create concept
			auto conceptId = genId();
			auto& concept = mConcepts[conceptId];
			concept.id = conceptId;
			concept.name = shape.name;

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
				addMesh(conceptId, newMesh);
			}

			//process material
			if (!shape.mesh.material_ids.empty())
			{
				auto materialId = shape.mesh.material_ids[0];
				if ((materialId >= 0) && (materialId < objMaterials.size()))
				{
					const auto& mat = objMaterials[materialId];

					if (!mat.diffuse_texname.empty())
						concept.matDiffusePath = mat.diffuse_texname;
					else if (!mat.ambient_texname.empty())
						concept.matDiffusePath = mat.ambient_texname;
					else
						materialId = materialId;

					if (!mat.normal_texname.empty())
						concept.matNormalPath = mat.normal_texname;
					else if (!mat.bump_texname.empty())
						concept.matNormalPath = mat.bump_texname;
					else if (mat.unknown_parameter.find("bump") != mat.unknown_parameter.end())
						concept.matNormalPath = mat.unknown_parameter.find("bump")->second;
					else if (mat.unknown_parameter.find("map_bump") != mat.unknown_parameter.end())
						concept.matNormalPath = mat.unknown_parameter.find("map_bump")->second;
					else
						materialId = materialId;
				}
			}

			//create an object associated with the concept
			Object newObject;
			newObject.type = Object::Type::Static;
			newObject.conceptId = concept.id;
			newObject.bbox = concept.geom.bbox;
			mObjects.push_back(newObject);
		}

		//need to save everything to file (new geometry was already saved)
		saveScene();

		return true;
	}

	void WorldEditor::recalcTangentSpace(const std::vector<size_t>& conceptIds)
	{
		auto func = [this](Concept& concept)
		{
			hr::geom::Mesh mesh(concept.geom.numVertices, concept.geom.numIndices);

			mGeomFileStream.seek(hr::streams::Stream::SeekOrigin::Begin, concept.geom.fstreamVertexOffset);
			mGeomFileStream.read(mesh.vertices(), mesh.sizeVertices());

			mGeomFileStream.seek(hr::streams::Stream::SeekOrigin::Begin, concept.geom.fstreamIndexOffset);
			mGeomFileStream.read(mesh.indices(), mesh.sizeIndices());

			mesh.genNormals();
			mesh.genTangents4();

			mGeomFileStream.seek(hr::streams::Stream::SeekOrigin::Begin, concept.geom.fstreamVertexOffset);
			mGeomFileStream.write(mesh.vertices(), mesh.sizeVertices());

			mGeomFileStream.seek(hr::streams::Stream::SeekOrigin::Begin, concept.geom.fstreamIndexOffset);
			mGeomFileStream.write(mesh.indices(), mesh.sizeIndices());
		};

		if (conceptIds.empty())
		{
			for (auto& concept : mConcepts)
				func(concept.second);
		}
		else
		{
			for (const auto& conceptId : conceptIds)
			{
				auto it = mConcepts.find(conceptId);
				if (it != mConcepts.end())
					func(it->second);
			}
		}
	}

	std::vector<size_t> WorldEditor::unusedConcepts() const
	{
		std::vector<size_t> conceptIds;

		for (const auto& concept : mConcepts)
		{
			if (std::find_if(mObjects.begin(), mObjects.end(), [conceptId = concept.first](const Object& object) { return (object.conceptId == conceptId); }) != mObjects.end())
				continue;

			conceptIds.push_back(concept.first);
		}

		return conceptIds;
	}

	void WorldEditor::removeConcepts(const std::vector<size_t>& conceptIds)
	{
		for (const auto& conceptId : conceptIds)
		{
			mConcepts.erase(conceptId);
			std::remove_if(mObjects.begin(), mObjects.end(), [conceptId](const Object& object) { return (object.conceptId == conceptId); });
		}
	}

	size_t WorldEditor::genId() const
	{
		size_t curId = 1;
		while (mConcepts.find(curId) != mConcepts.end())
			curId++;

		return curId;
	}

	void WorldEditor::addMesh(size_t geomId, const hr::geom::Mesh& mesh)
	{
		size_t numGeoms = 0;

		//update file header
		{
			GeomHeader geomHeader;
			{
				hr::streams::StreamReader stream(mGeomFileStream);

				stream.seek(hr::streams::Stream::SeekOrigin::Begin, 0);
				if (stream.read(&geomHeader, sizeof(GeomHeader)) != sizeof(GeomHeader))
					return;
			}

			geomHeader.numGeoms++;
			numGeoms = geomHeader.numGeoms;

			{
				hr::streams::StreamWriter stream(mGeomFileStream);

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
			hr::streams::StreamWriter stream(mGeomFileStream);

			stream.seek(hr::streams::Stream::SeekOrigin::Begin, sizeof(GeomHeader));

			if (stream.write(mesh.vertices(), mesh.sizeVertices()) != mesh.sizeVertices())
				return;
			if (stream.write(mesh.indices(), mesh.sizeIndices()) != mesh.sizeIndices())
				return;

			if (stream.write(&newGeomChunk, sizeof(GeomChunkInfo)) != sizeof(GeomChunkInfo))
				return;

			mGeomFileStream.flush();
			return;
		}

		int chunksSize = sizeof(GeomChunkInfo) * (numGeoms - 1);

		//read all geom chunks
		auto chunksTmp = std::unique_ptr<GeomChunkInfo[]>(new GeomChunkInfo[numGeoms - 1]);
		{
			hr::streams::StreamReader stream(mGeomFileStream);

			stream.seek(hr::streams::Stream::SeekOrigin::End, -chunksSize);
			if (stream.read(chunksTmp.get(), chunksSize) != chunksSize)
				return;
		}
		
		//write the new geom, old chunks and the new chunk
		{
			hr::streams::StreamWriter stream(mGeomFileStream);

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

		mGeomFileStream.flush();
	}

	void WorldEditor::saveScene()
	{
		//the scene is always exported whole, which means that we can destroy the old version completly

		if (std::experimental::filesystem::exists(m_paths.scene))
			std::experimental::filesystem::remove(m_paths.scene);

		hr::streams::FileStream streamScene(m_paths.scene, false, true);

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
		for (auto& concept : mConcepts)
		{
			writer.StartObject();

			writer.String("id");
			writer.Uint(concept.second.id);

			writer.String("name");
			writer.String(concept.second.name.c_str());

			writer.String("material");
			writer.StartObject();
			writer.String("diffusePath");
			writer.String(concept.second.matDiffusePath.c_str());

			writer.String("normalPath");
			writer.String(concept.second.matNormalPath.c_str());
			writer.EndObject();

			writer.EndObject();
		}
		writer.EndArray();

		writer.String("objects");
		writer.StartArray();
		for (auto& curObject : mObjects)
		{
			writer.StartObject();

			writer.String("type");
			writer.Uint(static_cast<unsigned int>(curObject.type));

			writer.String("conceptId");
			writer.Uint(curObject.conceptId);

			writer.EndObject();
		}
		writer.EndArray();

		writer.EndObject();

		streamScene.write(s.GetString(), s.GetSize());
	}
} }