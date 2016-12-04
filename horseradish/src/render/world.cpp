#include "world.hpp"

#include "common/encoders.hpp"
#include "common/imageFactory.hpp"

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
	};
	#pragma pack(pop)

	size_t World::genId() const
	{
		size_t curId = 1;
		while (mConcepts.find(curId) != mConcepts.end())
			curId++;

		return curId;
	}

	World::World()
	{ }

	World::~World()
	{
		cleanup();
	}

	void World::cleanup()
	{
		mConcepts.clear();
		mObjects.clear();

		mRenderData.objects.clear();
	}

	bool World::importAll(hr::streams::StreamReader &streamScene, hr::streams::StreamReader &streamGeom)
	{
		std::unordered_map<size_t, GeomChunkInfo> geomInfo;

		//read geom info
		size_t geomsOffset = 0;
		{
			GeomHeader geomHeader;
			streamGeom.read(&geomHeader, sizeof(GeomHeader));
			if (std::memcmp(geomHeader.fileSig, GeomFileSig.data(), sizeof(geomHeader.fileSig)) != 0)
				return false;

			geomInfo.rehash(geomHeader.numGeoms);
			for (size_t curGeom = 0; curGeom < geomHeader.numGeoms; ++curGeom)
			{
				GeomChunkInfo chunkInfo;
				streamGeom.read(&chunkInfo, sizeof(GeomChunkInfo));

				geomInfo[chunkInfo.id] = chunkInfo;
			}

			geomsOffset = streamGeom.position();
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

					if ((*itr).HasMember("mesh"))
					{
						auto& jsonMesh = ((*itr).FindMember("mesh"))->value;

						size_t numVertices = jsonMesh["numVertices"].GetUint();
						size_t numIndices = jsonMesh["numIndices"].GetUint();

						hr::geom::Mesh newMesh(numVertices, numIndices);

						auto geomIt = geomInfo.find(concept.id);
						if (geomIt != geomInfo.end())
						{
							float bbox[6];
							streamGeom.seek(hr::streams::Stream::SeekOrigin::Begin, geomsOffset + geomIt->second.geomsOffset);
							streamGeom.read(newMesh.vertices(), newMesh.sizeVertices());
							streamGeom.read(newMesh.indices(), newMesh.sizeIndices());
							streamGeom.read(bbox, sizeof(float) * 6);

							concept.meshBBox.setMinMax(bbox + 0, bbox + 3);
						}

						assert(newMesh.check());
						concept.mesh = std::move(newMesh);
					}

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

					newObject.bbox = mConcepts[newObject.conceptId].meshBBox;
					mObjects.push_back(newObject);
				}
			}
		}

		return true;
	}

	bool World::exportAll(hr::streams::StreamWriter &streamScene, hr::streams::StreamWriter &streamGeom)
	{
		//export scene file
		{
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

				writer.String("mesh");
				writer.StartObject();
					writer.String("numVertices");
					writer.Uint(concept.second.mesh.numVertices());

					writer.String("numIndices");
					writer.Uint(concept.second.mesh.numIndices());
				writer.EndObject();

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

		//export geom file
		{
			auto numGeoms = mConcepts.size();
			auto geomInfos = std::unique_ptr<GeomChunkInfo[]>(new GeomChunkInfo[numGeoms]);

			{
				size_t curGeom = 0, fileOffset = 0;
				for (auto& concept : mConcepts)
				{
					auto& geomInfo = geomInfos[curGeom++];

					geomInfo.id = concept.second.id;
					geomInfo.geomsOffset = fileOffset;
					geomInfo.size = concept.second.mesh.sizeVertices() + concept.second.mesh.sizeIndices() + (sizeof(float) * 6); // vertices + indices + bbox

					fileOffset += geomInfo.size;
				}
			}

			{
				GeomHeader geomHeader;
				memcpy(geomHeader.fileSig, GeomFileSig.data(), sizeof(geomHeader.fileSig));
				geomHeader.version = 1;
				geomHeader.numGeoms = numGeoms;

				streamGeom.write(&geomHeader, sizeof(GeomHeader));
			}

			streamGeom.write(geomInfos.get(), sizeof(GeomChunkInfo) * numGeoms);

			for (auto& concept : mConcepts)
			{
				//TODO: seems that lz4 is corrupting the buffer

				//auto maxCompressedSize = LZ4_compressBound(curGeom.mesh.sizeVertices());
				//auto compressedBuffer = std::unique_ptr<char[]>(new char[maxCompressedSize]);
				//auto compressedSize = LZ4_compress_HC(reinterpret_cast<const char*>(curGeom.mesh.dataVertices()), compressedBuffer.get(), curGeom.mesh.sizeVertices(), maxCompressedSize, 16);
				//
				//if ((compressedSize > 0) && (compressedSize < curGeom.mesh.sizeVertices()))
				//	writer.String(hr::Encoders::EncodeBase64(compressedBuffer.get(), compressedSize).c_str());

				streamGeom.write(concept.second.mesh.vertices(), concept.second.mesh.sizeVertices());
				streamGeom.write(concept.second.mesh.indices(), concept.second.mesh.sizeIndices());

				streamGeom.write(concept.second.meshBBox.min().data(), sizeof(float) * 3);
				streamGeom.write(concept.second.meshBBox.max().data(), sizeof(float) * 3);
			}
		}

		return true;
	}

	bool World::importObj(const std::string& basePath, const std::string& fileName)
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

				std::swap(concept.mesh, newMesh);
				concept.meshBBox = concept.mesh.getBoundingBox();
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
			newObject.bbox = concept.meshBBox;
			mObjects.push_back(newObject);
		}

		return true;
	}

	void World::optimizeConcept(size_t conceptId)
	{
		if (mConcepts.find(conceptId) == mConcepts.end())
			return;

		auto& concept = mConcepts[conceptId];
		concept.mesh.optimizeIndices();
	}

	void World::optimizeConcepts()
	{
		std::vector<size_t> conceptIds;
		std::transform(mConcepts.begin(), mConcepts.end(), std::back_inserter(conceptIds), [](const std::map<size_t, Concept>::value_type &keyValue) {return keyValue.first; });

		for (const auto& conceptId : conceptIds)
			optimizeConcept(conceptId);
	}

	std::vector<size_t> World::unusedConcepts() const
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

	void World::removeConcepts(const std::vector<size_t>& conceptIds)
	{
		for (const auto& conceptId : conceptIds)
		{
			mConcepts.erase(conceptId);
			std::remove_if(mObjects.begin(), mObjects.end(), [conceptId](const Object& object) { return (object.conceptId == conceptId); });
		}
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
} }