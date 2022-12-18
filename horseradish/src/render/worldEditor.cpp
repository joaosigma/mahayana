#include "worldEditor.hpp"

#include "common/encoders.hpp"
#include "common/imageFactory.hpp"
#include "common/meshAnim.hpp"
#include "common/fileSystem.hpp"
#include "common/quaternion.hpp"
#include "common/stringUtils.hpp"

#include "libs/rapidjson/document.h"
#include "libs/rapidjson/rapidjson.h"
#include "libs/rapidjson/prettywriter.h"
#include "libs/rapidjson/stringbuffer.h"

#include "libs/tinygltf/tiny_gltf.h"
#include "libs/tinyobjloader/tiny_obj_loader.h"

#include "glcorearb.h"

#include <array>
#include <cstdint>
#include <algorithm>
#include <filesystem>
#include <unordered_set>

namespace hr::render
{
	namespace
	{
		struct gltfPrimitiveState
		{
			bool hasPos = false;
			bool hasNormal = false;
			bool hasTangent = false;
			bool hasTexCoords = false;
			bool hasJoints = false;
			bool hasJointsWheights = false;
		};

		template<typename T>
		Vector3f gltfReadVec3f(const T* const data)
		{
			static_assert(std::is_same_v<T, double> || std::is_same_v<T, float>, "Unsupported data type");

			if constexpr (std::is_same_v<T, float>)
				return Vector3f{data[0], data[1], data[2]};
			else
				return Vector3d{data[0], data[1], data[2]}.convert<float, 3>();
		};

		template<typename T>
		Vector4f gltfReadVec4f(const T* const data)
		{
			static_assert(std::is_same_v<T, double> || std::is_same_v<T, float>, "Unsupported data type");

			if constexpr (std::is_same_v<T, float>)
				return Vector4f{data[0], data[1], data[2], data[3]};
			else
				return Vector4d{data[0], data[1], data[2], data[3]}.convert<float, 4>();
		};

		template<typename T>
		Quaternionf gltfReadQuat(const T* const data)
		{
			static_assert(std::is_same_v<T, double> || std::is_same_v<T, float>, "Unsupported data type");

			if constexpr (std::is_same_v<T, float>)
				return Quaternionf::from(data[0], data[1], data[2], data[3]);
			else
				return Quaterniond::from(data[0], data[1], data[2], data[3]).convert<float>();
		};

		gltfPrimitiveState gltfReadPrimitive(const tinygltf::Model& gltfModel, const tinygltf::Primitive& gltfPrimitive, geom::Mesh<geom::VertexFull, uint32_t>& mesh, geom::MeshAnim &meshAnim)
		{
			gltfPrimitiveState state;

			for (const auto& gltfAttrib : gltfPrimitive.attributes)
			{
				if (gltfAttrib.second < 0)
					continue;

				const auto& gltfAccessor = gltfModel.accessors[gltfAttrib.second];
				if (gltfAccessor.bufferView < 0)
					continue;

				const auto& gltfBufferView = gltfModel.bufferViews[gltfAccessor.bufferView];
				const auto& gltfBuffer = gltfModel.buffers[gltfBufferView.buffer];
				if (gltfBuffer.data.empty())
					continue;

				auto gltfDataStride = gltfAccessor.ByteStride(gltfBufferView);
				if (gltfDataStride < 0)
					continue;

				assert(gltfAccessor.count == mesh.numVertices());

				auto vertexData = mesh.vertices();
				auto vertexJoints = meshAnim.verticesJoints();

				auto gltfData = gltfBuffer.data.data() + gltfBufferView.byteOffset + gltfAccessor.byteOffset;

				if ((gltfAttrib.first == "POSITION") && (gltfAccessor.type == TINYGLTF_TYPE_VEC3) && (gltfAccessor.componentType == TINYGLTF_COMPONENT_TYPE_FLOAT))
				{
					state.hasPos = true;
					for (size_t curIndex = 0; curIndex < mesh.numVertices(); curIndex++)
					{
						auto pos = gltfReadVec3f(reinterpret_cast<const float*>(gltfData));
						pos.write(vertexData[curIndex].pos);

						gltfData += gltfDataStride;
					}
				}
				else if ((gltfAttrib.first == "TEXCOORD_0") && (gltfAccessor.type == TINYGLTF_TYPE_VEC2) && (gltfAccessor.componentType == TINYGLTF_COMPONENT_TYPE_FLOAT))
				{
					state.hasTexCoords = true;
					for (size_t curIndex = 0; curIndex < mesh.numVertices(); curIndex++)
					{
						std::memcpy(vertexData[curIndex].uv, gltfData, sizeof(float) * 2);
						gltfData += gltfDataStride;
					}
				}
				else if ((gltfAttrib.first == "NORMAL") && (gltfAccessor.type == TINYGLTF_TYPE_VEC3) && (gltfAccessor.componentType == TINYGLTF_COMPONENT_TYPE_FLOAT))
				{
					state.hasNormal = true;
					for (size_t curIndex = 0; curIndex < mesh.numVertices(); curIndex++)
					{
						auto normal = gltfReadVec3f(reinterpret_cast<const float*>(gltfData));
						normal.write(vertexData[curIndex].normal);

						gltfData += gltfDataStride;
					}
				}
				else if ((gltfAttrib.first == "TANGENT") && (gltfAccessor.type == TINYGLTF_TYPE_VEC4) && (gltfAccessor.componentType == TINYGLTF_COMPONENT_TYPE_FLOAT))
				{
					state.hasTangent = true;
					for (size_t curIndex = 0; curIndex < mesh.numVertices(); curIndex++)
					{
						auto tangent = gltfReadVec4f(reinterpret_cast<const float*>(gltfData));
						tangent.write(vertexData[curIndex].tangent);

						gltfData += gltfDataStride;
					}
				}
				else if ((gltfAttrib.first == "JOINTS_0") && (gltfAccessor.type == TINYGLTF_TYPE_VEC4))
				{
					if (gltfAccessor.componentType == TINYGLTF_COMPONENT_TYPE_UNSIGNED_BYTE)
					{
						state.hasJoints = true;

						size_t curIndex = 0;
						for (; curIndex < mesh.numVertices(); curIndex++)
						{
							vertexJoints[(curIndex * 4) + 0].jointIndex = gltfData[0];
							vertexJoints[(curIndex * 4) + 1].jointIndex = gltfData[1];
							vertexJoints[(curIndex * 4) + 2].jointIndex = gltfData[2];
							vertexJoints[(curIndex * 4) + 3].jointIndex = gltfData[3];
							gltfData += gltfDataStride;
						}

						assert(meshAnim.numJoints() == (curIndex * 4));
					}
					else if (gltfAccessor.componentType == TINYGLTF_COMPONENT_TYPE_UNSIGNED_SHORT)
					{
						state.hasJoints = true;

						size_t curIndex = 0;
						for (; curIndex < mesh.numVertices(); curIndex++)
						{
							auto dataPtr = reinterpret_cast<const uint16_t*>(gltfData);
							vertexJoints[(curIndex * 4) + 0].jointIndex = dataPtr[0];
							vertexJoints[(curIndex * 4) + 1].jointIndex = dataPtr[1];
							vertexJoints[(curIndex * 4) + 2].jointIndex = dataPtr[2];
							vertexJoints[(curIndex * 4) + 3].jointIndex = dataPtr[3];
							gltfData += gltfDataStride;
						}

						assert(meshAnim.numJoints() == (curIndex * 4));
					}
				}
				else if ((gltfAttrib.first == "WEIGHTS_0") && (gltfAccessor.type == TINYGLTF_TYPE_VEC4))
				{
					if (gltfAccessor.componentType == TINYGLTF_COMPONENT_TYPE_UNSIGNED_SHORT)
					{
						state.hasJointsWheights = true;

						size_t curIndex = 0;
						for (; curIndex < mesh.numVertices(); curIndex++)
						{
							auto dataPtr = reinterpret_cast<const uint16_t*>(gltfData);
							vertexJoints[(curIndex * 4) + 0].jointWeight = dataPtr[0];
							vertexJoints[(curIndex * 4) + 1].jointWeight = dataPtr[1];
							vertexJoints[(curIndex * 4) + 2].jointWeight = dataPtr[2];
							vertexJoints[(curIndex * 4) + 3].jointWeight = dataPtr[3];
							gltfData += gltfDataStride;
						}

						assert(meshAnim.numJoints() == (curIndex * 4));
					}
					else if (gltfAccessor.componentType == TINYGLTF_COMPONENT_TYPE_FLOAT)
					{
						state.hasJointsWheights = true;

						size_t curIndex = 0;
						for (; curIndex < mesh.numVertices(); curIndex++)
						{
							auto dataPtr = reinterpret_cast<const float*>(gltfData);
							vertexJoints[(curIndex * 4) + 0].jointWeight = types::packFloat<uint16_t>(dataPtr[0]);
							vertexJoints[(curIndex * 4) + 1].jointWeight = types::packFloat<uint16_t>(dataPtr[1]);
							vertexJoints[(curIndex * 4) + 2].jointWeight = types::packFloat<uint16_t>(dataPtr[2]);
							vertexJoints[(curIndex * 4) + 3].jointWeight = types::packFloat<uint16_t>(dataPtr[3]);
							gltfData += gltfDataStride;
						}

						assert(meshAnim.numJoints() == (curIndex * 4));
					}
				}
			}

			return state;
		}

		bool gltfReadIndices(const tinygltf::Model& gltfModel, int gltfAcessorIndex, std::unique_ptr<uint32_t[]>& newIndices, size_t& newIndicesCount)
		{
			const auto& gltfAccessor = gltfModel.accessors[gltfAcessorIndex];
			if ((gltfAccessor.bufferView < 0) || (gltfAccessor.count <= 0) || ((gltfAccessor.count % 3) != 0) || gltfAccessor.normalized || (gltfAccessor.type != TINYGLTF_TYPE_SCALAR))
				return false;

			const auto& gltfBufferView = gltfModel.bufferViews[gltfAccessor.bufferView];
			const auto& gltfBuffer = gltfModel.buffers[gltfBufferView.buffer];
			if (gltfBuffer.data.empty())
				return false;

			if ((gltfAccessor.maxValues.size() == 1) && (gltfAccessor.maxValues[0] >= geom::Mesh<geom::VertexShading, uint32_t>::maxVertexCount()))
				return false;

			auto gltfData = gltfBuffer.data.data() + gltfBufferView.byteOffset + gltfAccessor.byteOffset;

			switch (gltfAccessor.componentType)
			{
			case TINYGLTF_COMPONENT_TYPE_UNSIGNED_BYTE:
				if (gltfAccessor.ByteStride(gltfBufferView) != 1)
					return false;

				newIndicesCount = gltfAccessor.count;
				newIndices = std::unique_ptr<uint32_t[]>(new uint32_t[newIndicesCount]);

				for (size_t i = 0; i < newIndicesCount; i++)
					newIndices[i] = static_cast<uint32_t>(gltfData[i]);

				return true;

			case TINYGLTF_COMPONENT_TYPE_UNSIGNED_SHORT:
				if (gltfAccessor.ByteStride(gltfBufferView) != 2)
					return false;

				newIndicesCount = gltfAccessor.count;
				newIndices = std::unique_ptr<uint32_t[]>(new uint32_t[newIndicesCount]);

				{
					auto dataPtr = reinterpret_cast<const uint16_t*>(gltfData);
					for (size_t i = 0; i < newIndicesCount; i++)
						newIndices[i] = static_cast<uint32_t>(dataPtr[i]);
				}

				return true;

			case TINYGLTF_COMPONENT_TYPE_UNSIGNED_INT:
				if (gltfAccessor.ByteStride(gltfBufferView) != 4)
					return false;

				newIndicesCount = gltfAccessor.count;
				newIndices = std::unique_ptr<uint32_t[]>(new uint32_t[newIndicesCount]);

				{
					auto dataPtr = reinterpret_cast<const uint32_t*>(gltfData);
					for (size_t i = 0; i < newIndicesCount; i++)
						newIndices[i] = dataPtr[i];
				}

				return true;

			default:
				break;
			}

			return false;
		}

		size_t gltfReadInverseBindMatrices(const tinygltf::Model& gltfModel, int gltfAcessorIndex, const std::function<bool(size_t, Matrix4f)> &cb)
		{
			const auto& gltfAccessor = gltfModel.accessors[gltfAcessorIndex];
			if ((gltfAccessor.bufferView < 0) || (gltfAccessor.type != TINYGLTF_TYPE_MAT4) || (gltfAccessor.componentType != TINYGLTF_COMPONENT_TYPE_FLOAT))
				return 0;

			const auto& gltfBufferView = gltfModel.bufferViews[gltfAccessor.bufferView];

			const auto& gltfBuffer = gltfModel.buffers[gltfBufferView.buffer];
			if (gltfBuffer.data.empty())
				return 0;

			auto gltfDataStride = gltfAccessor.ByteStride(gltfBufferView);
			if (gltfDataStride < 0)
				return 0;

			auto gltfData = gltfBuffer.data.data() + gltfBufferView.byteOffset + gltfAccessor.byteOffset;

			for (size_t curIndex = 0; curIndex < gltfAccessor.count; curIndex++)
			{
				auto mat = Matrix4f::from<float>({reinterpret_cast<const float*>(gltfData), 16});
				gltfData += gltfDataStride;

				if (!cb(curIndex, std::move(mat)))
					return (curIndex + 1);
			}

			return gltfAccessor.count;
		}

		bool gltfReadAnimTimestamps(const tinygltf::Model& gltfModel, int gltfSamplerInput, std::vector<float>& newAnimTimestamps)
		{
			const auto& gltfAccessor = gltfModel.accessors[gltfSamplerInput];
			if (gltfAccessor.bufferView < 0)
				return false;

			const auto& gltfBufferView = gltfModel.bufferViews[gltfAccessor.bufferView];
			const auto& gltfBuffer = gltfModel.buffers[gltfBufferView.buffer];
			if (gltfBuffer.data.empty())
				return false;

			auto gltfDataStride = gltfAccessor.ByteStride(gltfBufferView);
			if (gltfDataStride < 0)
				return false;

			if ((gltfAccessor.type == TINYGLTF_TYPE_SCALAR) && (gltfAccessor.componentType == TINYGLTF_COMPONENT_TYPE_FLOAT))
			{
				auto gltfData = gltfBuffer.data.data() + gltfBufferView.byteOffset + gltfAccessor.byteOffset;

				newAnimTimestamps.reserve(newAnimTimestamps.size() + gltfAccessor.count);
				for (size_t curIndex = 0; curIndex < gltfAccessor.count; curIndex++)
				{
					newAnimTimestamps.push_back(*reinterpret_cast<const float*>(gltfData));
					gltfData += gltfDataStride;
				}

				return true;
			}

			return false;
		}

		bool gltfReadAnimSamplerData(const tinygltf::Model& gltfModel, int gltfSamplerOutput, bool has4Components, std::vector<Vector4f>& animData)
		{
			const auto& gltfAccessor = gltfModel.accessors[gltfSamplerOutput];
			if (gltfAccessor.bufferView < 0)
				return false;

			const auto& gltfBufferView = gltfModel.bufferViews[gltfAccessor.bufferView];
			const auto& gltfBuffer = gltfModel.buffers[gltfBufferView.buffer];
			if (gltfBuffer.data.empty())
				return false;

			auto gltfDataStride = gltfAccessor.ByteStride(gltfBufferView);
			if (gltfDataStride < 0)
				return false;

			if (has4Components)
			{
				if ((gltfAccessor.type == TINYGLTF_TYPE_VEC4) && (gltfAccessor.componentType == TINYGLTF_COMPONENT_TYPE_FLOAT))
				{
					auto gltfData = gltfBuffer.data.data() + gltfBufferView.byteOffset + gltfAccessor.byteOffset;

					animData.reserve(animData.size() + gltfAccessor.count);
					for (size_t curIndex = 0; curIndex < gltfAccessor.count; curIndex++)
					{
						auto data = reinterpret_cast<const float*>(gltfData);

						animData.push_back(Vector4f{data[0], data[1], data[2], data[3]});
						gltfData += gltfDataStride;
					}

					return true;
				}

				return false;
			}
			else
			{
				if ((gltfAccessor.type == TINYGLTF_TYPE_VEC3) && (gltfAccessor.componentType == TINYGLTF_COMPONENT_TYPE_FLOAT))
				{
					auto gltfData = gltfBuffer.data.data() + gltfBufferView.byteOffset + gltfAccessor.byteOffset;

					animData.reserve(animData.size() + gltfAccessor.count);
					for (size_t curIndex = 0; curIndex < gltfAccessor.count; curIndex++)
					{
						auto data = reinterpret_cast<const float*>(gltfData);

						animData.push_back(Vector4f{data[0], data[1], data[2], 0.0f});
						gltfData += gltfDataStride;
					}

					return true;
				}

				return false;
			}
		}
	}

	WorldEditor::WorldEditor()
	{ }

	WorldEditor::AreaId WorldEditor::newArea(std::string_view scenePath, std::string_view binPath)
	{
		AreaId newId;
		{
			newId = mGenAreaIds++;

			auto& area = mAreas[newId];
			area.id = newId;
		}

		assert(mAreasData.find(newId) == mAreasData.end());
		auto& areaData = mAreasData[newId];

		areaData.pathScene = scenePath;
		areaData.pathBin = binPath;

		//create "empty" files
		{
			if (std::filesystem::exists(std::string(scenePath)))
				std::filesystem::remove(std::string(scenePath));
			if (std::filesystem::exists(std::string(binPath)))
				std::filesystem::remove(std::string(binPath));

			{
				hr::streams::FileStream streamScene(std::string(scenePath), false, true);

				rapidjson::StringBuffer s;
				rapidjson::PrettyWriter<rapidjson::StringBuffer> writer(s);

				writer.StartObject();

				writer.String("version");
				writer.StartArray();
				writer.Int(1);
				writer.Int(0);
				writer.Int(0);
				writer.EndArray();

				writer.String("objects");
				writer.StartArray();
				writer.EndArray();

				writer.String("instances");
				writer.StartArray();
				writer.EndArray();

				writer.EndObject();

				streamScene.write(s.GetString(), s.GetSize());
			}

			{
				hr::streams::FileStream streamGeom(std::string(binPath), false, true);

				World::geomFileCreate(streamGeom);
			}
		}

		return newId;
	}

	bool WorldEditor::importMesh(AreaId areaId, std::string_view name, const geom::Mesh<geom::VertexFull, uint32_t>& mesh)
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
			
			object.id = objectId;
			object.type = Object::Type::Static;
			objectData.objectId = objectId;
			objectData.name = name;
		}

		//we store new geometry immediately
		{
			hr::streams::FileStream geomFileStream(areaData.pathBin, true, true);
			World::geomFileAddMesh(geomFileStream, objectId, mesh);
		}

		//create an instance associated with the object
		{
			Instance newInstance;
			newInstance.type = Instance::Type::Static;
			newInstance.objectId = objectId;
			newInstance.bbox = mesh.bbox();
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

		tinyobj::ObjReader objReader;
		{
			std::string fullPath;
			fullPath.reserve(basePath.size() + fileName.size() + 1);
			fullPath.append(basePath).append(fileName);

			tinyobj::ObjReaderConfig objConfig;
			objConfig.triangulate = true;
			objConfig.vertex_color = false;
			objConfig.mtl_search_path = ""; //use the same folder as the obj file

			objReader.ParseFromFile(fullPath.c_str(), objConfig);
		}

		if (!objReader.Valid())
			return false;

		if (objReader.GetShapes().empty())
			return true;

		auto& area = mAreas[areaId];
		auto& areaData = mAreasData[areaId];

		area.mInstances.reserve(area.mInstances.size() + objReader.GetShapes().size());

		for (const auto& shape : objReader.GetShapes())
		{
			//ignore empty shapes
			if (shape.mesh.indices.empty())
				continue;

			//all tris must belong to the same material
			if (!shape.mesh.material_ids.empty())
			{
				if (!std::all_of(shape.mesh.material_ids.begin(), shape.mesh.material_ids.end(), [target = shape.mesh.material_ids[0]](auto id) { return (id == target); }))
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

			object.id = objectId;
			object.type = Object::Type::Static;
			objectData.objectId = objectId;
			objectData.name = shape.name;

			//process mesh
			{
				bool ignoreNormals = std::any_of(shape.mesh.indices.begin(), shape.mesh.indices.end(), [](const tinyobj::index_t& index)
				{
					return (index.normal_index == -1);
				});

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

				if (mapping.size() <= geom::Mesh<geom::VertexFull, uint32_t>::maxVertexCount())
				{
					auto newMesh = geom::Mesh<geom::VertexFull, uint32_t>{ mapping.size(), shape.mesh.indices.size() };

					for (const auto& keyValue : mapping)
					{
						auto& objVertexAttribs = objReader.GetAttrib();

						auto vertexIndex = keyValue.second;

						std::memcpy(newMesh.vertices()[vertexIndex].pos, objVertexAttribs.vertices.data() + (keyValue.first.vertex_index * 3), sizeof(float) * 3);
						std::memcpy(newMesh.vertices()[vertexIndex].uv, objVertexAttribs.texcoords.data() + (keyValue.first.texcoord_index * 2), sizeof(float) * 2);

						if (!ignoreNormals)
							std::memcpy(newMesh.vertices()[vertexIndex].normal, objVertexAttribs.normals.data() + (keyValue.first.normal_index * 3), sizeof(float) * 3);
					}

					size_t curIndex = 0;
					for (const auto& index : shape.mesh.indices)
					{
						auto mappingIt = mapping.find(index);
						assert(mappingIt != mapping.end());

						newMesh.indices()[curIndex++] = static_cast<uint16_t>(mappingIt->second);
					}

					assert(newMesh.check());
					if (ignoreNormals)
						newMesh.genNormals();
					newMesh.genTangents4();

					newMesh.optimizeIndices();

					{
						hr::streams::FileStream geomFileStream(areaData.pathBin, true, true);
						World::geomFileAddMesh(geomFileStream, objectId, newMesh);
					}
				}
				else
				{
					continue; //couldn't create mesh
				}
			}

			//process material
			if (!shape.mesh.material_ids.empty())
			{
				auto& objMaterials = objReader.GetMaterials();

				auto materialId = shape.mesh.material_ids[0];
				if ((materialId >= 0) && (materialId < objMaterials.size()))
				{
					const auto& mat = objMaterials[materialId];

					if (!mat.diffuse_texname.empty())
						objectData.matDiffusePath = mat.diffuse_texname;
					else if (!mat.ambient_texname.empty())
						objectData.matDiffusePath = mat.ambient_texname;

					if (!mat.normal_texname.empty())
						objectData.matNormalPath = mat.normal_texname;
					else if (!mat.bump_texname.empty())
						objectData.matNormalPath = mat.bump_texname;
					else if (mat.unknown_parameter.find("bump") != mat.unknown_parameter.end())
						objectData.matNormalPath = mat.unknown_parameter.find("bump")->second;
					else if (mat.unknown_parameter.find("map_bump") != mat.unknown_parameter.end())
						objectData.matNormalPath = mat.unknown_parameter.find("map_bump")->second;
				}
			}

			//create an object associated with the concept
			Instance newInstance;
			newInstance.type = Instance::Type::Static;
			newInstance.objectId = object.id;
			newInstance.bbox = object.bbox;
			area.mInstances.push_back(newInstance);
		}

		//need to save everything to file (new geometry was already saved)
		saveArea(area);

		return true;
	}

	bool WorldEditor::importGLTF(AreaId areaId, std::string_view gltfPath)
	{
		if (mAreas.find(areaId) == mAreas.end())
			return false;

		tinygltf::Model gltfModel;
		{
			std::string err, warn;
			tinygltf::TinyGLTF gltf_ctx;

			bool ret = false;
			if (std::filesystem::path(gltfPath.begin(), gltfPath.end()).extension().string() == ".glb")
				ret = gltf_ctx.LoadBinaryFromFile(&gltfModel, &err, &warn, gltfPath.data());
			else
				ret = gltf_ctx.LoadASCIIFromFile(&gltfModel, &err, &warn, gltfPath.data());

			if (!ret)
				return false;
		}

		if (gltfModel.scenes.empty() || gltfModel.nodes.empty() || gltfModel.meshes.empty())
			return true;

		auto& area = mAreas[areaId];
		auto& areaData = mAreasData[areaId];

		/*
		* NOTES:
		*  - gltf uses column-major but also post-multiplication, since ours is row-major and pre-multiplication, there's
		*      no need to change anything
		*  - gltf axis are -X points right, +Y up and +Z forward, so there's only a difference in the X axis
		*/

		//read all the nodes local transformations
		std::vector<geom::MeshAnimSet::Node> nodes;
		{
			nodes.reserve(gltfModel.nodes.size());
			for (const auto& gltfNode : gltfModel.nodes)
			{
				auto localTrans = Matrix4f::identity();
				
				if (gltfNode.matrix.size() == 16)
				{
					assert(gltfNode.scale.empty() && gltfNode.rotation.empty() && gltfNode.translation.empty());
					localTrans = Matrix4f::from<double>({gltfNode.matrix.data(), 16});
				}
				else
				{
					//first scale, then rotation and finally translation
					if (gltfNode.scale.size() == 3) localTrans *= Matrix4f::scale(gltfReadVec3f(gltfNode.scale.data()));
					if (gltfNode.rotation.size() == 4) localTrans *= gltfReadQuat(gltfNode.rotation.data());
					if (gltfNode.translation.size() == 3) localTrans *= Matrix4f::translation(gltfReadVec3f(gltfNode.translation.data()));
				}

				geom::MeshAnimSet::Node node;
				node.name = gltfNode.name;
				node.localTransform = localTrans;

				nodes.push_back(std::move(node));
			}

			//calculate the parents
			size_t curNodeIndex{ 0 };
			for (const auto& gltfNode : gltfModel.nodes)
			{
				for (const auto childIndex : gltfNode.children)
				{
					assert(nodes[childIndex].parentIndex == -1);
					nodes[childIndex].parentIndex = curNodeIndex;
				}
				
				curNodeIndex++;
			}
		}

		//read the skins
		struct Skin
		{
			std::string name;
			size_t animSetId{0};
			std::vector<geom::MeshAnimSet::Joint> joints;
		};
		std::vector<Skin> skins;
		skins.reserve(gltfModel.skins.size());

		for (const auto& gltfSkin : gltfModel.skins)
		{
			Skin targetSkin;
			targetSkin.name = gltfSkin.name;
			targetSkin.joints.reserve(gltfSkin.joints.size());

			for (const auto nodeIndex : gltfSkin.joints)
			{
				assert((nodeIndex >= 0) && (nodeIndex < gltfModel.nodes.size()));
				auto& gltfNode = gltfModel.nodes[nodeIndex];

				geom::MeshAnimSet::Joint joint;
				joint.nodeIndex = static_cast<size_t>(nodeIndex);
				joint.parentIndex = -1;
				if (gltfNode.matrix.size() == 16)
				{
					joint.localTransform.scale = Vector3f{1.0f};
					joint.localTransform.rotation = Quaternionf::identity();
					joint.localTransform.translation = Vector3f::zero();
					joint.localTransform.matrix = Matrix4f::from<double>({gltfNode.matrix.data(), 16});
				}
				else
				{
					joint.localTransform.scale = (gltfNode.scale.size() == 3) ? gltfReadVec3f(gltfNode.scale.data()) : Vector3f{1.0f};
					joint.localTransform.rotation = (gltfNode.rotation.size() == 4) ? gltfReadQuat(gltfNode.rotation.data()) : Quaternionf::identity();
					joint.localTransform.translation = (gltfNode.translation.size() == 3) ? gltfReadVec3f(gltfNode.translation.data()) : Vector3f::zero();
					joint.localTransform.matrix = Matrix4f::identity();
				}

				targetSkin.joints.push_back(std::move(joint));
			}

			if (gltfSkin.inverseBindMatrices >= 0)
			{
				auto count = gltfReadInverseBindMatrices(gltfModel, gltfSkin.inverseBindMatrices, [&targetSkin](size_t index, Matrix4f inverseBindMatrix)
				{
					assert(index < targetSkin.joints.size());
					targetSkin.joints[index].transformInvert = std::move(inverseBindMatrix);
					return true;
				});

				if (count != targetSkin.joints.size())
					continue; //error
			}

			//calculate the joints parents
			for (auto& joint : targetSkin.joints)
			{
				const auto& node = nodes[joint.nodeIndex];
				if (node.parentIndex < 0) continue;

				auto it = std::find_if(targetSkin.joints.begin(), targetSkin.joints.end(), [&node](const auto& joint) { return (joint.nodeIndex == node.parentIndex); });

				if (it != targetSkin.joints.end()) joint.parentIndex = std::distance(targetSkin.joints.begin(), it);
			}

			skins.push_back(std::move(targetSkin));
		}

		//to recursively read all nodes
		{
			std::function<void(int, const tinygltf::Node&, const Matrix4f&)> recurNodes;
			recurNodes = [this, &recurNodes, &area, &areaData, &gltfModel, &skins, &nodes](int gltfNodeIndex, const tinygltf::Node& gltfNode, const Matrix4f& previousGlobalTrans)
			{
				auto globalTrans = nodes[gltfNodeIndex].localTransform * previousGlobalTrans;
				
				auto hasSkin = (gltfNode.mesh >= 0) && (gltfNode.skin >= 0) && (gltfNode.skin < skins.size());
				if (hasSkin && (skins[gltfNode.skin].animSetId == 0)) //create animation set if we don't have one for this skeleton
				{
					auto& targetSkin = skins[gltfNode.skin];

					targetSkin.animSetId = genAnimSetId(area);

					auto& animSet = area.mAnimationSets[targetSkin.animSetId];
					auto& animSetData = areaData.animationSets[targetSkin.animSetId];

					animSet.id = targetSkin.animSetId;
					animSetData.animSetId = targetSkin.animSetId;
					animSetData.name = targetSkin.name;

					{
						hr::streams::FileStream geomFileStream(areaData.pathBin, true, true);
						World::geomFileAddAnimationSet(geomFileStream, targetSkin.animSetId, geom::MeshAnimSet{gltfModel.skins[gltfNode.skin].name, nodes, static_cast<size_t>(gltfNodeIndex), targetSkin.joints});
					}
				}

				if (gltfNode.mesh >= 0)
				{
					auto& gltfMesh = gltfModel.meshes[gltfNode.mesh];
					for (const auto& gltfPrim : gltfMesh.primitives)
					{
						if ((gltfPrim.mode != TINYGLTF_MODE_TRIANGLES) || (gltfPrim.indices < 0))
							continue;

						geom::MeshAnim newMeshAnim;
						geom::Mesh<geom::VertexFull, uint32_t> newMesh;

						//read indices and prepare mesh
						{
							size_t newIndicesCount;
							std::unique_ptr<uint32_t[]> newIndices;

							if (!gltfReadIndices(gltfModel, gltfPrim.indices, newIndices, newIndicesCount))
								continue;

							size_t numVertices{ 0 };
							for (size_t i = 0; i < newIndicesCount; i++)
								numVertices = std::max<size_t>(numVertices, newIndices[i]);
							numVertices++;

							newMesh = geom::Mesh<geom::VertexFull, uint32_t>{ std::unique_ptr<geom::VertexFull[]>(new geom::VertexFull[numVertices]), numVertices, std::move(newIndices), newIndicesCount };
							newMeshAnim = hr::geom::MeshAnim{ geom::Mesh<geom::VertexShading, uint16_t>::convertMesh(newMesh), hr::geom::MeshAnim::SkinningType::Vertex4Joints };
						}

						//read vertex data
						auto primState = gltfReadPrimitive(gltfModel, gltfPrim, newMesh, newMeshAnim);
						
						if (!newMesh.check() || !primState.hasPos || (hasSkin && (!primState.hasJoints || !primState.hasJointsWheights)))
							continue;

						//very rare and special case, but allowed
						if (!primState.hasTexCoords)
						{
							newMesh.iterateVertices([](size_t, geom::VertexFull& v)
							{
								v.uv[0] = v.uv[1] = 0.0f;
								return true;
							});
						}

						//in GLTF, the determinant of the node’s global transform defines the winding order of that primitive
						//if (globalTrans.determinant() > 0.0f)
						//	newMesh.invertTriWinding();

						if (!primState.hasNormal)
							newMesh.genNormals();
						if (!primState.hasTangent)
							newMesh.genTangents4();
						
						if (!hasSkin)
						{
							newMesh.optimizeIndices();
							newMesh.transform(globalTrans, globalTrans.clone(Matrix4f::CloneTransform::InverseTranspose).convert<Matrix3, float>());
						}
						else
						{
							newMeshAnim.correctWeights();
						}

						//auto reflect = Matrix4f::reflection(Plane<float>{1.0f, 0.0f, 0.0f, 0.0f});
						//newMesh.transform(reflect, reflect.convert<Matrix3, float>());

						//we can now create a new object

						{
							//create object
							auto objectId = genObjectId(area);
							auto& object = area.mObjects[objectId];
							auto& objectData = areaData.objects[objectId];

							//object can already be prepared
							object.id = objectId;
							object.type = Object::Type::Static;
							object.bbox = newMesh.bbox();
							objectData.objectId = objectId;
							objectData.name = gltfMesh.name;
							objectData.geom.numVertices = newMesh.numVertices();
							objectData.geom.numIndices = newMesh.numIndices();

							//store new geometry
							if (hasSkin)
							{
								newMeshAnim.mesh() = geom::Mesh<geom::VertexShading, uint16_t>::convertMesh(newMesh);

								hr::streams::FileStream geomFileStream(areaData.pathBin, true, true);
								World::geomFileAddMeshAnim(geomFileStream, objectId, newMeshAnim, skins[gltfNode.skin].animSetId);
							}
							else
							{
								hr::streams::FileStream geomFileStream(areaData.pathBin, true, true);
								World::geomFileAddMesh(geomFileStream, objectId, newMesh);
							}

							//update the offsets
							{
								hr::streams::FileStream geomFileStream(areaData.pathBin, true, true);
								World::geomFileRetrieveOffsets(geomFileStream, objectId, objectData.geom.fstreamVertexOffset, objectData.geom.fstreamIndexOffset);
							}

							//read material info
							if (gltfPrim.material >= 0)
							{
								const auto& gltfMat = gltfModel.materials[gltfPrim.material];
								if (!gltfMat.name.empty())
									objectData.name = objectData.name + "_" + gltfMat.name;

								auto extractImageUri = [](const  tinygltf::Model& model, const tinygltf::Material& material, std::string_view componentName) -> std::string
								{
									auto itComponent = material.values.find(std::string(componentName));
									if (itComponent == material.values.end())
									{
										itComponent = material.additionalValues.find(std::string(componentName));
										if (itComponent == material.additionalValues.end())
											return {};
									}

									auto itIndex = itComponent->second.json_double_value.find("index");
									auto itTexCoord = itComponent->second.json_double_value.find("texCoord");
									if ((itIndex == itComponent->second.json_double_value.end()) && (itTexCoord == itComponent->second.json_double_value.end()))
										return {};

									auto texIndex = static_cast<int>(itIndex->second);
									if ((texIndex < 0) && (static_cast<int>(itTexCoord->second) != 0)) // we support only one set of UVs
										return {};

									if (model.textures[texIndex].source < 0)
										return {};

									return model.images[model.textures[texIndex].source].uri;
								};

								objectData.matDiffusePath = extractImageUri(gltfModel, gltfMat, "baseColorTexture");
								objectData.matNormalPath = extractImageUri(gltfModel, gltfMat, "normalTexture");
							}

							//create an object associated with the concept
							Instance newInstance;
							newInstance.type = Instance::Type::Static;
							newInstance.objectId = object.id;
							newInstance.bbox = object.bbox;
							area.mInstances.push_back(newInstance);
						}
					}
				}

				for (const auto& nodeIndex : gltfNode.children)
					recurNodes(nodeIndex, gltfModel.nodes[nodeIndex], globalTrans);
			};

			//recursively parse all nodes *only* in the default scene
			for (const auto& nodeIndex : gltfModel.scenes[gltfModel.defaultScene].nodes)
				recurNodes(nodeIndex, gltfModel.nodes[nodeIndex], Matrix4f::identity());
		}

		//now all that is left is to read animations of used skins
		for (const auto& gltfAnim : gltfModel.animations)
		{
			if (gltfAnim.samplers.empty() || gltfAnim.channels.empty())
				continue;

			std::optional<size_t> animationSetId;

			struct SamplerData
			{
				struct IndexOutput
				{
					int output;
					size_t sampleIndex;
				};
				int input;
				std::vector<IndexOutput> indexOutputs;

				geom::MeshAnimSet::Animation::Sampler finalSampler;
			};
			std::vector<SamplerData> samplers;

			//make sure all samplers are linear and read all their data
			{
				bool ignoreAnim{false};
				size_t sampleIndex{ 0 };
				for (const auto& gltfSampler : gltfAnim.samplers)
				{
					ignoreAnim = (gltfSampler.interpolation != "LINEAR");
					if (ignoreAnim)
						break;

					auto it = std::find_if(samplers.begin(), samplers.end(), [&gltfSampler](const auto& sampler) { return (gltfSampler.input == sampler.input); });
					if (it == samplers.end())
					{
						SamplerData newSampler;
						newSampler.input = gltfSampler.input;
						newSampler.indexOutputs.push_back({ .output = gltfSampler.output, .sampleIndex = sampleIndex });

						//can already read the input (timeline)
						if (!gltfReadAnimTimestamps(gltfModel, gltfSampler.input, newSampler.finalSampler.timePoints) || newSampler.finalSampler.timePoints.empty())
						{
							ignoreAnim = true;
							break;
						}

						samplers.push_back(std::move(newSampler));
					}
					else
					{
						it->indexOutputs.push_back({.output = gltfSampler.output, .sampleIndex = sampleIndex});
					}

					sampleIndex++;
				}

				if (ignoreAnim)
					continue;
			}
			
			//for each channel
			for (const auto& gltfChannel : gltfAnim.channels)
			{
				if (gltfChannel.target_node < 0)
					continue;
				if ((gltfChannel.sampler < 0) || (gltfChannel.sampler > gltfAnim.samplers.size()))
					continue;

				auto itSampler = std::find_if(samplers.begin(), samplers.end(), [&gltfChannel](const auto& sampler)
				{
					auto itOutput = std::find_if(sampler.indexOutputs.begin(), sampler.indexOutputs.end(), [&gltfChannel](const auto& indexOutput)
					{
						return (indexOutput.sampleIndex == gltfChannel.sampler);
					});

					return (itOutput != sampler.indexOutputs.end());
				});
				assert(itSampler != samplers.end());

				//find the skin where the target node is
				{
					size_t skinIndex{ 0 };
			
					bool hasAnimSet{ false };
					for (auto&& skin : skins)
					{
						skinIndex++;
			
						auto it = std::find_if(skin.joints.begin(), skin.joints.end(), [&gltfChannel](const auto& joint) { return (joint.nodeIndex == gltfChannel.target_node); });
						if (it == skin.joints.end())
							continue;
			
						hasAnimSet = true;
			
						if (!animationSetId)
							animationSetId = skin.animSetId;
			
						assert(animationSetId.value() == skin.animSetId);
						break;
					}
			
					if (!hasAnimSet)
						continue;
				}

				auto itSamplerOutput = std::find_if(itSampler->indexOutputs.begin(), itSampler->indexOutputs.end(), [&gltfChannel](const auto& indexOutput)
				{
					return (indexOutput.sampleIndex == gltfChannel.sampler);
				});
				assert(itSamplerOutput != itSampler->indexOutputs.end());

				geom::MeshAnimSet::Animation::Channel channel;
				channel.nodeIndex = gltfChannel.target_node;
												
				if (gltfChannel.target_path == "scale")
				{
					channel.target = geom::MeshAnimSet::Animation::Channel::Target::Scale;
					if (!gltfReadAnimSamplerData(gltfModel, itSamplerOutput->output, false, channel.frameData) || (channel.frameData.size() != itSampler->finalSampler.timePoints.size()))
						continue;
				}
				else if (gltfChannel.target_path == "rotation")
				{
					channel.target = geom::MeshAnimSet::Animation::Channel::Target::Rotation;
					if (!gltfReadAnimSamplerData(gltfModel, itSamplerOutput->output, true, channel.frameData) || (channel.frameData.size() != itSampler->finalSampler.timePoints.size()))
						continue;
				}
				else if (gltfChannel.target_path == "translation")
				{
					channel.target = geom::MeshAnimSet::Animation::Channel::Target::Translation;
					if (!gltfReadAnimSamplerData(gltfModel, itSamplerOutput->output, false, channel.frameData) || (channel.frameData.size() != itSampler->finalSampler.timePoints.size()))
						continue;
				}

				itSampler->finalSampler.channels.push_back(std::move(channel));
			}
			
			//animation is ready, include in file
			if (animationSetId.has_value()) //glTF has non skinned animations
			{
				geom::MeshAnimSet::Animation animation;
				animation.name = gltfAnim.name;
				animation.minTimePoint = std::numeric_limits<float>::max();
				animation.maxTimePoint = std::numeric_limits<float>::min();
				animation.samplers.reserve(samplers.size());
				for (auto& sampler : samplers)
				{
					if (sampler.finalSampler.timePoints.empty())
						continue;

					sampler.finalSampler.minTimePoint = sampler.finalSampler.timePoints.front();
					sampler.finalSampler.maxTimePoint = sampler.finalSampler.timePoints.back();

					animation.minTimePoint = std::min(animation.minTimePoint, sampler.finalSampler.minTimePoint);
					animation.maxTimePoint = std::max(animation.maxTimePoint, sampler.finalSampler.maxTimePoint);

					animation.samplers.push_back(std::move(sampler.finalSampler));
				}

				size_t newAnimId = 1; //generate anim id
				{
					auto& animSet = areaData.animationSets[animationSetId.value()];
					while (std::find_if(animSet.anims.begin(), animSet.anims.end(), [&newAnimId](const auto& anim) { return (anim.animId == newAnimId); }) != animSet.anims.end())
						newAnimId++;
			
					animSet.anims.push_back(AreaData::AnimSetData::Animation{ newAnimId, gltfAnim.name });
				}
			
				hr::streams::FileStream geomFileStream(areaData.pathBin, true, true);
				World::geomFileAddAnimation(geomFileStream, newAnimId, animationSetId.value(), std::move(animation));
			}
		}

		//need to save everything to file (new geometry was already saved)
		saveArea(area);

		return true;
	}

	void WorldEditor::processMesh(AreaId areaId, const std::vector<size_t>& objectIds, const std::function<void(geom::Mesh<geom::VertexFull, uint32_t>&)>& cb)
	{
		if (!cb || (mAreas.find(areaId) == mAreas.end()))
			return;

		auto& area = mAreas[areaId];
		auto& areaData = mAreasData[areaId];

		hr::streams::FileStream geomFileStream(areaData.pathBin, true, true);

		World::geomFileTransformMeshes(geomFileStream, objectIds, cb);
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

	void WorldEditor::removeObjects(AreaId areaId, const std::vector<std::string_view>& objectsNames)
	{
		if (objectsNames.empty() || (mAreas.find(areaId) == mAreas.end()))
			return;

		auto& area = mAreas[areaId];
		auto& areaData = mAreasData[areaId];

		std::unordered_set<size_t> objectIds;
		for (auto&& targetName : objectsNames)
		{
			for (const auto&[objectId, object] : areaData.objects)
			{
				if (object.name == targetName)
					objectIds.insert(objectId);
			}
		}

		if (objectIds.empty())
			return;

		for (const auto& objectId : objectIds)
		{
			area.mObjects.erase(objectId);
			area.mInstances.erase(
				std::remove_if(area.mInstances.begin(), area.mInstances.end(), [objectId](const auto& instance) { return (instance.objectId == objectId); }),
				area.mInstances.end());

			areaData.objects.erase(objectId);
		}

		saveArea(area); //this only takes care of the scene

		//now we take care of the binary data
		{
			std::filesystem::rename(areaData.pathBin, areaData.pathBin + ".tmp");

			{
				hr::streams::FileStream geomFileStreamOld(areaData.pathBin + ".tmp", true, false);
				hr::streams::FileStream geomFileStreamNew(areaData.pathBin, false, true);

				std::vector<size_t> finalObjectIds;
				finalObjectIds.insert(finalObjectIds.end(), objectIds.begin(), objectIds.end());

				geomFileRemoveGeom(geomFileStreamOld, geomFileStreamNew, std::move(finalObjectIds));
			}

			std::filesystem::remove(areaData.pathBin + ".tmp");
		}
	}

	size_t WorldEditor::genObjectId(Area& area) const
	{
		size_t curId = 1;
		while (area.mObjects.find(curId) != area.mObjects.end())
			curId++;

		return curId;
	}

	size_t WorldEditor::genAnimSetId(Area& area) const
	{
		size_t curId = 1;
		while (area.mAnimationSets.find(curId) != area.mAnimationSets.end())
			curId++;

		return curId;
	}

	size_t WorldEditor::findAnimSetId(AreaData& area, std::string_view name)
	{
		for (auto&&[animSetId, animSet] : area.animationSets)
		{
			if (animSet.name == name)
				return animSetId;
		}

		return 0;
	}

	void WorldEditor::saveArea(Area& area)
	{
		assert(mAreasData.find(area.id) != mAreasData.end());
		auto& areaData = mAreasData[area.id];

		//the area is always exported whole, which means that we can destroy the old version completly
		//also: we only need to save the scene (geom was already taken care of)

		if (std::filesystem::exists(areaData.pathScene))
			std::filesystem::remove(areaData.pathScene);

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

		writer.String("animationSets");
		writer.StartArray();
		for (auto& animSet : areaData.animationSets)
		{
			writer.StartObject();

			writer.String("id");
			writer.Uint(animSet.second.animSetId);

			writer.String("name");
			writer.String(animSet.second.name.c_str());

			writer.String("joints");
			writer.StartArray();
			for (auto& joint : animSet.second.joints)
			{
				writer.StartObject();

				writer.String("index");
				writer.Uint(joint.index);

				writer.String("parent");
				writer.Int(joint.parentIndex);

				writer.String("name");
				writer.String(joint.name.data());

				writer.EndObject();
			}
			writer.EndArray();

			writer.String("animation");
			writer.StartArray();
			for (auto& anim : animSet.second.anims)
			{
				writer.StartObject();

				writer.String("id");
				writer.Uint(anim.animId);

				writer.String("name");
				writer.String(anim.name.c_str());

				writer.EndObject();
			}
			writer.EndArray();

			writer.EndObject();
		}
		writer.EndArray();

		writer.String("objects");
		writer.StartArray();
		for (auto& object : areaData.objects)
		{
			writer.StartObject();

			writer.String("id");
			writer.Uint(object.second.objectId);

			writer.String("type");
			writer.Uint(static_cast<unsigned int>(Object::Type::Static));

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

		writer.String("instances");
		writer.StartArray();
		for (auto& curObject : area.mInstances)
		{
			writer.StartObject();

			writer.String("type");
			writer.Uint(static_cast<unsigned int>(curObject.type));

			writer.String("objectId");
			writer.Uint(curObject.objectId);

			writer.EndObject();
		}
		writer.EndArray();

		writer.EndObject();

		streamScene.write(s.GetString(), s.GetSize());
	}
}
