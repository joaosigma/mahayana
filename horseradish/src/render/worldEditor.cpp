#include "worldEditor.hpp"

#include "common/encoders.hpp"
#include "common/fileSystem.hpp"
#include "common/imageFactory.hpp"
#include "common/meshAnim.hpp"
#include "common/quaternion.hpp"
#include "common/stringUtils.hpp"

#include <nlohmann/json.hpp>

#include <tiny_gltf.h>
#include <tiny_obj_loader.h>

#include "glcorearb.h"

#include <algorithm>
#include <array>
#include <cstdint>
#include <filesystem>
#include <map>
#include <set>
#include <unordered_set>

namespace hr::render
{
    namespace
    {
        struct gltfNode
        {
            std::string name;
            int32_t parentIndex{-1};
            Matrix4f localTransform{Matrix4f::identity()};
        };

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

        std::vector<gltfNode> gltfReadNodes(const tinygltf::Model& gltfModel)
        {
            std::vector<gltfNode> nodes;
            nodes.reserve(gltfModel.nodes.size());

            for (const auto& node : gltfModel.nodes)
            {
                auto localTrans = Matrix4f::identity();
                if (node.matrix.size() == 16)
                {
                    assert(node.scale.empty() && node.rotation.empty() && node.translation.empty());
                    localTrans = Matrix4f::from<double>({node.matrix.data(), 16});
                }
                else
                {
                    // first scale, then rotation and finally translation
                    if (node.scale.size() == 3)
                        localTrans *= Matrix4f::scale(gltfReadVec3f(node.scale.data()));
                    if (node.rotation.size() == 4)
                        localTrans *= gltfReadQuat(node.rotation.data());
                    if (node.translation.size() == 3)
                        localTrans *= Matrix4f::translation(gltfReadVec3f(node.translation.data()));
                }

                gltfNode newNode;
                newNode.name = node.name;
                newNode.localTransform = localTrans;

                nodes.push_back(std::move(newNode));
            }

            // calculate the parents
            size_t curNodeIndex{0};
            for (const auto& node : gltfModel.nodes)
            {
                for (const auto childIndex : node.children)
                {
                    assert(nodes[childIndex].parentIndex == -1);
                    nodes[childIndex].parentIndex = curNodeIndex;
                }

                curNodeIndex++;
            }

            return nodes;
        }

        Matrix4f gltfCalcNodeGlobalTransform(std::span<const gltfNode> nodes, size_t nodeIndex)
        {
            assert((nodeIndex >= 0) && (nodeIndex < nodes.size()));
            auto& node = nodes[nodeIndex];

            if (node.parentIndex < 0)
                return node.localTransform;

            return node.localTransform * gltfCalcNodeGlobalTransform(nodes, static_cast<size_t>(node.parentIndex));
        }

        gltfPrimitiveState gltfReadPrimitive(const tinygltf::Model& gltfModel,
                                             const tinygltf::Primitive& gltfPrimitive,
                                             geom::Mesh<geom::VertexFull, uint32_t>& mesh,
                                             geom::MeshAnim& meshAnim)
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
                            vertexJoints[(curIndex * 4) + 0].index = gltfData[0];
                            vertexJoints[(curIndex * 4) + 1].index = gltfData[1];
                            vertexJoints[(curIndex * 4) + 2].index = gltfData[2];
                            vertexJoints[(curIndex * 4) + 3].index = gltfData[3];
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
                            vertexJoints[(curIndex * 4) + 0].index = dataPtr[0];
                            vertexJoints[(curIndex * 4) + 1].index = dataPtr[1];
                            vertexJoints[(curIndex * 4) + 2].index = dataPtr[2];
                            vertexJoints[(curIndex * 4) + 3].index = dataPtr[3];
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
                            vertexJoints[(curIndex * 4) + 0].weight = dataPtr[0];
                            vertexJoints[(curIndex * 4) + 1].weight = dataPtr[1];
                            vertexJoints[(curIndex * 4) + 2].weight = dataPtr[2];
                            vertexJoints[(curIndex * 4) + 3].weight = dataPtr[3];
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
                            vertexJoints[(curIndex * 4) + 0].weight = types::packFloat<uint16_t>(dataPtr[0]);
                            vertexJoints[(curIndex * 4) + 1].weight = types::packFloat<uint16_t>(dataPtr[1]);
                            vertexJoints[(curIndex * 4) + 2].weight = types::packFloat<uint16_t>(dataPtr[2]);
                            vertexJoints[(curIndex * 4) + 3].weight = types::packFloat<uint16_t>(dataPtr[3]);
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
            if ((gltfAccessor.bufferView < 0) || (gltfAccessor.count <= 0) || ((gltfAccessor.count % 3) != 0) || gltfAccessor.normalized ||
                (gltfAccessor.type != TINYGLTF_TYPE_SCALAR))
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

        size_t gltfReadInverseBindMatrices(const tinygltf::Model& gltfModel, int gltfAcessorIndex, const std::function<bool(size_t, Matrix4f)>& cb)
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

    size_t WorldEditor::genObjectId(const Area& area, Random& rand)
    {
        while (true)
        {
            auto candidate = static_cast<size_t>(rand.nextInteger(10000000, 19999999));
            if (!area.mObjects.contains(candidate))
                return candidate;
        }
    }

    size_t WorldEditor::genMaterialId(const Area& area, Random& rand)
    {
        while (true)
        {
            auto candidate = static_cast<size_t>(rand.nextInteger(20000000, 29999999));
            if (!area.mMaterials.contains(candidate))
                return candidate;
        }
    }

    size_t WorldEditor::genTextureSetId(const Area& area, Random& rand)
    {
        while (true)
        {
            auto candidate = static_cast<size_t>(rand.nextInteger(30000000, 39999999));
            if (!area.mTextureSets.contains(candidate))
                return candidate;
        }
    }

    size_t WorldEditor::genAnimSetId(const Area& area, Random& rand)
    {
        while (true)
        {
            auto candidate = static_cast<size_t>(rand.nextInteger(40000000, 49999999));
            if (!area.mSkeletonAnims.contains(candidate))
                return candidate;
        }
    }

    size_t WorldEditor::findAnimSetId(const AreaData& area, std::string_view name)
    {
        for (auto&& [animSetId, animSet] : area.animationSets)
        {
            if (animSet.name == name)
                return animSetId;
        }

        return 0;
    }

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

        // create "empty" files
        {
            if (std::filesystem::exists(std::string(scenePath)))
                std::filesystem::remove(std::string(scenePath));
            if (std::filesystem::exists(std::string(binPath)))
                std::filesystem::remove(std::string(binPath));

            {
                std::string jsonContent;
                {
                    using nlohmann::json;

                    json jFile;
                    jFile["version"] = json::array({1, 0, 0});
                    jFile["textureSets"] = json::array();
                    jFile["materials"] = json::array();
                    jFile["objects"] = json::array();
                    jFile["instances"] = json::array();

                    jsonContent = jFile.dump(4);
                }

                hr::streams::FileStream streamScene(std::string(scenePath), false, true);
                streamScene.write(jsonContent.data(), jsonContent.size());
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

        // create object
        auto objectId = genObjectId(area, mRandom);
        {
            auto& object = area.mObjects[objectId];
            auto& objectData = areaData.objects[objectId];

            object.id = objectId;
            object.type = Object::Type::Static;
            objectData.objectId = objectId;
            objectData.name = name;
        }

        // we store new geometry immediately
        {
            hr::streams::FileStream geomFileStream(areaData.pathBin, true, true);
            World::geomFileAddMesh(geomFileStream, objectId, mesh);
        }

        // create an instance associated with the object
        {
            Instance newInstance;
            newInstance.type = Instance::Type::Static;
            newInstance.objectId = objectId;
            newInstance.bbox = mesh.bbox();
            area.mInstances.push_back(newInstance);
        }

        // need to save everything to file (new geometry was already saved)
        saveArea(area);

        return true;
    }

    bool WorldEditor::importObj(AreaId areaId, std::string_view basePath, std::string_view fileName)
    {
        if (mAreas.find(areaId) == mAreas.end())
            return false;

        tinyobj::attrib_t attrib;
        std::vector<tinyobj::shape_t> shapes;
        std::vector<tinyobj::material_t> materials;
        {
            std::string strBasePath{basePath};

            std::string fullPath;
            fullPath.reserve(basePath.size() + fileName.size() + 1);
            fullPath.append(basePath).append(fileName);

            std::string warn, err;
            if (!tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err, fullPath.c_str(), strBasePath.c_str()))
                return false;
        }

        if (shapes.empty())
            return true;

        struct MaterialTextureSet
        {
            size_t materialId;
            size_t textureSetId;
        };
        std::unordered_map<int, MaterialTextureSet> materialMapping;

        auto& area = mAreas[areaId];
        auto& areaData = mAreasData[areaId];

        for (const auto& shape : shapes)
        {
            // ignore empty shapes
            if (shape.mesh.indices.empty())
                continue;

            // all faces must have 3 vertices (be triangles)
            if (!std::all_of(shape.mesh.num_face_vertices.begin(), shape.mesh.num_face_vertices.end(), [](auto numVertices) { return (numVertices == 3); }))
                continue;

            // all tris must belong to the same material
            if (!shape.mesh.material_ids.empty())
            {
                if (!std::all_of(shape.mesh.material_ids.begin(), shape.mesh.material_ids.end(), [target = shape.mesh.material_ids[0]](auto id) { return (id == target); }))
                    continue;
            }

            // all indices must have position
            if (std::any_of(shape.mesh.indices.begin(), shape.mesh.indices.end(), [](const tinyobj::index_t& index) { return (index.vertex_index == -1); }))
                continue;

            // ignore shapes if a object with the same name already exists
            if (std::find_if(areaData.objects.begin(), areaData.objects.end(), [&name = shape.name](const auto& keyValue) { return (keyValue.second.name == name); }) !=
                areaData.objects.end())
                continue;

            // process mesh
            geom::Mesh<geom::VertexFull, uint32_t> newMesh;
            {
                bool ignoreTexCoords =
                  std::any_of(shape.mesh.indices.begin(), shape.mesh.indices.end(), [](const tinyobj::index_t& index) { return (index.texcoord_index == -1); });

                bool ignoreNormals = std::any_of(shape.mesh.indices.begin(), shape.mesh.indices.end(), [](const tinyobj::index_t& index) { return (index.normal_index == -1); });

                struct hashableIndex: public tinyobj::index_t
                {
                    struct hash
                    {
                        size_t operator()(const hashableIndex& index) const noexcept
                        {
                            size_t seed = index.vertex_index;
                            seed ^= index.texcoord_index + 0x9e3779b9 + (seed << 6) + (seed >> 2);
                            seed ^= index.normal_index + 0x9e3779b9 + (seed << 6) + (seed >> 2);
                            return seed;
                        }
                    };

                    hashableIndex(tinyobj::index_t index) noexcept
                      : tinyobj::index_t(index)
                    {}

                    bool operator==(const hashableIndex& other) const noexcept
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

                if (mapping.size() > geom::Mesh<geom::VertexFull, uint32_t>::maxVertexCount())
                    continue; // can't create mesh

                newMesh = geom::Mesh<geom::VertexFull, uint32_t>{mapping.size(), shape.mesh.indices.size()};

                for (const auto& keyValue : mapping)
                {
                    auto vertexIndex = keyValue.second;

                    std::memcpy(newMesh.vertices()[vertexIndex].pos, attrib.vertices.data() + (keyValue.first.vertex_index * 3), sizeof(float) * 3);

                    if (!ignoreTexCoords)
                        std::memcpy(newMesh.vertices()[vertexIndex].uv, attrib.texcoords.data() + (keyValue.first.texcoord_index * 2), sizeof(float) * 2);

                    if (!ignoreNormals)
                        std::memcpy(newMesh.vertices()[vertexIndex].normal, attrib.normals.data() + (keyValue.first.normal_index * 3), sizeof(float) * 3);
                }

                size_t curIndex = 0;
                for (const auto& index : shape.mesh.indices)
                {
                    auto mappingIt = mapping.find(index);
                    assert(mappingIt != mapping.end());

                    newMesh.indices()[curIndex++] = static_cast<uint32_t>(mappingIt->second);
                }

                assert(newMesh.check());

                // generate stuff that wasn't read (mind the order: it is important)
                if (ignoreNormals)
                    newMesh.genNormals();
                if (ignoreTexCoords)
                    newMesh.genUVs(geom::Mesh<geom::VertexFull, uint32_t>::UVGenType::Sphere);
                else
                    newMesh.flipUV();
                newMesh.genTangents4();

                newMesh.optimizeIndices();
            }

            // process material
            std::optional<MaterialTextureSet> materialTexSet;
            if (!shape.mesh.material_ids.empty() && (shape.mesh.material_ids.front() >= 0) && (shape.mesh.material_ids.front() < materials.size()))
            {
                auto matObjId = shape.mesh.material_ids[0];

                if (auto it = materialMapping.find(matObjId); it != materialMapping.end())
                {
                    // reuse existing one
                    materialTexSet = it->second;
                }
                else
                {
                    const auto& matObj = materials[matObjId];

                    materialTexSet = MaterialTextureSet{.materialId = genMaterialId(area, mRandom), .textureSetId = genTextureSetId(area, mRandom)};
                    materialMapping[matObjId] = *materialTexSet;

                    auto& finalMat = area.mMaterials[materialTexSet->materialId];
                    finalMat.id = materialTexSet->materialId;
                    finalMat.name = matObj.name;

                    auto& finalTexSet = area.mTextureSets[materialTexSet->textureSetId];
                    finalTexSet.id = materialTexSet->textureSetId;

                    if (!matObj.diffuse_texname.empty())
                        finalTexSet.diffusePath = matObj.diffuse_texname;
                    else if (!matObj.ambient_texname.empty())
                        finalTexSet.diffusePath = matObj.ambient_texname;

                    if (!matObj.normal_texname.empty())
                        finalTexSet.normalPath = matObj.normal_texname;
                    else if (!matObj.bump_texname.empty())
                        finalTexSet.normalPath = matObj.bump_texname;
                    else if (matObj.unknown_parameter.find("bump") != matObj.unknown_parameter.end())
                        finalTexSet.normalPath = matObj.unknown_parameter.find("bump")->second;
                    else if (matObj.unknown_parameter.find("map_bump") != matObj.unknown_parameter.end())
                        finalTexSet.normalPath = matObj.unknown_parameter.find("map_bump")->second;
                }
            }

            auto processMesh = [this, &area, &areaData, &shape, &materialTexSet](hr::streams::FileStream& geomFileStream, const geom::Mesh<geom::VertexFull, uint32_t>& mesh)
            {
                // create object
                auto objectId = genObjectId(area, mRandom);
                auto& object = area.mObjects[objectId];
                auto& objectData = areaData.objects[objectId];

                object.id = objectId;
                object.type = Object::Type::Static;
                object.materialId = materialTexSet ? materialTexSet->materialId : 0;
                object.textureSetId = materialTexSet ? materialTexSet->textureSetId : 0;
                objectData.objectId = objectId;
                objectData.name = shape.name;

                // add geometry
                World::geomFileAddMesh(geomFileStream, objectId, mesh);

                // create an object associated with the concept
                Instance newInstance;
                newInstance.type = Instance::Type::Static;
                newInstance.objectId = object.id;
                newInstance.bbox = object.bbox;
                area.mInstances.push_back(newInstance);
            };

            // split the mesh if necessary
            auto smallMeshes = newMesh.split(geom::MeshBase<geom::VertexShading, uint16_t>::maxVertexCount());
            if (smallMeshes.empty())
            {
                hr::streams::FileStream geomFileStream(areaData.pathBin, true, true);
                processMesh(geomFileStream, newMesh);
            }
            else
            {
                hr::streams::FileStream geomFileStream(areaData.pathBin, true, true);
                for (const auto& curMesh : smallMeshes)
                    processMesh(geomFileStream, curMesh);
            }
        }

        // need to save everything to file (new geometry was already saved)
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

        // read all the nodes local transformations
        auto nodes = gltfReadNodes(gltfModel);

        // read the skins
        struct Skin
        {
            struct Joint
            {
                size_t nodeIndex;
                geom::SkeletonAnim::Joint finalJoint;
            };

            std::string name;
            size_t animSetId{0};
            std::vector<Joint> joints;
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

                Skin::Joint joint;
                joint.nodeIndex = static_cast<size_t>(nodeIndex);
                joint.finalJoint.name = gltfNode.name;
                joint.finalJoint.parentIndex = -1;
                if (gltfNode.matrix.size() == 16)
                {
                    joint.finalJoint.localTransform.scale = Vector3f{1.0f};
                    joint.finalJoint.localTransform.rotation = Quaternionf::identity();
                    joint.finalJoint.localTransform.translation = Vector3f::zero();
                    joint.finalJoint.localTransform.matrix = Matrix4f::from<double>({gltfNode.matrix.data(), 16});
                }
                else
                {
                    joint.finalJoint.localTransform.scale = (gltfNode.scale.size() == 3) ? gltfReadVec3f(gltfNode.scale.data()) : Vector3f{1.0f};
                    joint.finalJoint.localTransform.rotation = (gltfNode.rotation.size() == 4) ? gltfReadQuat(gltfNode.rotation.data()) : Quaternionf::identity();
                    joint.finalJoint.localTransform.translation = (gltfNode.translation.size() == 3) ? gltfReadVec3f(gltfNode.translation.data()) : Vector3f::zero();
                    joint.finalJoint.localTransform.matrix = Matrix4f::identity();
                }

                targetSkin.joints.push_back(std::move(joint));
            }

            if (gltfSkin.inverseBindMatrices >= 0)
            {
                auto count = gltfReadInverseBindMatrices(gltfModel, gltfSkin.inverseBindMatrices,
                                                         [&targetSkin](size_t index, Matrix4f inverseBindMatrix)
                                                         {
                                                             assert(index < targetSkin.joints.size());
                                                             targetSkin.joints[index].finalJoint.transformInvert = std::move(inverseBindMatrix);
                                                             return true;
                                                         });

                if (count != targetSkin.joints.size())
                    continue; // error
            }

            // calculate the joints parents
            for (auto& joint : targetSkin.joints)
            {
                const auto& node = nodes[joint.nodeIndex];
                if (node.parentIndex < 0)
                    continue;

                auto it = std::find_if(targetSkin.joints.begin(), targetSkin.joints.end(), [&node](const auto& joint) { return (joint.nodeIndex == node.parentIndex); });

                if (it != targetSkin.joints.end())
                    joint.finalJoint.parentIndex = std::distance(targetSkin.joints.begin(), it);
            }

            skins.push_back(std::move(targetSkin));
        }

        // to re-use materials
        struct MaterialTextureSet
        {
            size_t materialId;
            size_t textureSetId;
        };
        std::unordered_map<int, MaterialTextureSet> materialMapping;

        // process all the nodes
        for (size_t nodeIndex = 0; nodeIndex < gltfModel.nodes.size(); ++nodeIndex)
        {
            const auto& node = gltfModel.nodes[nodeIndex];

            if (node.mesh < 0)
                continue;

            auto globalTrans = gltfCalcNodeGlobalTransform(nodes, nodeIndex);

            auto hasSkin = (node.skin >= 0);
            assert(!hasSkin || (node.skin < skins.size()));

            // if it's animated (and the associated animation set hasn't been created yet)
            if (hasSkin && (skins[node.skin].animSetId == 0))
            {
                auto& targetSkin = skins[node.skin];

                targetSkin.animSetId = genAnimSetId(area, mRandom);

                auto& animSet = area.mSkeletonAnims[targetSkin.animSetId];
                auto& animSetData = areaData.animationSets[targetSkin.animSetId];

                animSet.id = targetSkin.animSetId;
                animSetData.id = targetSkin.animSetId;
                animSetData.name = targetSkin.name;

                {
                    std::vector<geom::SkeletonAnim::Joint> finalJoints;
                    finalJoints.reserve(targetSkin.joints.size());
                    for (const auto& joint : targetSkin.joints)
                        finalJoints.push_back(joint.finalJoint);

                    // switch the x-axis
                    auto rootTransform = globalTrans * Matrix4f::reflection(Plane<float>{1.0f, 0.0f, 0.0f, 0.0f});

                    hr::streams::FileStream geomFileStream(areaData.pathBin, true, true);
                    World::geomFileAddAnimationSet(geomFileStream, targetSkin.animSetId,
                                                   geom::SkeletonAnim{gltfModel.skins[node.skin].name, rootTransform, std::move(finalJoints)});
                }
            }

            auto& gltfMesh = gltfModel.meshes[node.mesh];
            for (const auto& gltfPrim : gltfMesh.primitives)
            {
                if ((gltfPrim.mode != TINYGLTF_MODE_TRIANGLES) || (gltfPrim.indices < 0))
                    continue;

                geom::MeshAnim newMeshAnim;
                geom::Mesh<geom::VertexFull, uint32_t> newMesh;

                // read indices and prepare mesh
                {
                    size_t newIndicesCount;
                    std::unique_ptr<uint32_t[]> newIndices;

                    if (!gltfReadIndices(gltfModel, gltfPrim.indices, newIndices, newIndicesCount))
                        continue;

                    size_t numVertices{0};
                    for (size_t i = 0; i < newIndicesCount; i++)
                        numVertices = std::max<size_t>(numVertices, newIndices[i]);
                    numVertices++;

                    newMesh = geom::Mesh<geom::VertexFull, uint32_t>{std::unique_ptr<geom::VertexFull[]>(new geom::VertexFull[numVertices]), numVertices, std::move(newIndices),
                                                                     newIndicesCount};
                    newMeshAnim = hr::geom::MeshAnim{newMesh.convert<geom::VertexShading, uint16_t>(), hr::geom::MeshAnim::SkinningType::Vertex4Joints};

                    // can split an animation mesh
                    if (hasSkin && (numVertices > geom::MeshBase<geom::VertexShading, uint16_t>::maxVertexCount()))
                        continue;
                }

                // read vertex data
                auto primState = gltfReadPrimitive(gltfModel, gltfPrim, newMesh, newMeshAnim);

                if (!newMesh.check() || !primState.hasPos || (hasSkin && (!primState.hasJoints || !primState.hasJointsWheights)))
                    continue;

                // generate stuff that wasn't read (mind the order: it is important)
                if (!primState.hasNormal)
                    newMesh.genNormals();
                if (!primState.hasTexCoords)
                    newMesh.genUVs(geom::Mesh<geom::VertexFull, uint32_t>::UVGenType::Sphere);
                if (!primState.hasTangent)
                    newMesh.genTangents4();

                if (!hasSkin)
                {
                    newMesh.optimizeIndices();
                    newMesh.transform(globalTrans, globalTrans.clone(Matrix4f::CloneTransform::InverseTranspose).convert<Matrix3, float>());

                    // switch the x-axis
                    auto reflect = Matrix4f::reflection(Plane<float>{1.0f, 0.0f, 0.0f, 0.0f});
                    newMesh.transform(reflect, reflect.clone(Matrix4f::CloneTransform::InverseTranspose).convert<Matrix3, float>());
                    newMesh.invertTriWinding();

                    // in GLTF, the determinant of the node’s global transform defines the winding order of that primitive
                    // if (globalTrans.determinant() > 0.0f)
                    //	newMesh.invertTriWinding();
                }
                else
                {
                    // the root transform animation already has the reflection transform (switched the x-axis) so all we have to do is to invert the winding
                    newMesh.invertTriWinding();

                    newMeshAnim.correctWeights();
                }

                // we can now create a new object
                {
                    // create object
                    auto objectId = genObjectId(area, mRandom);
                    auto& object = area.mObjects[objectId];
                    auto& objectData = areaData.objects[objectId];

                    // object can already be prepared
                    object.id = objectId;
                    object.type = Object::Type::Static;
                    object.bbox = newMesh.bbox();
                    objectData.objectId = objectId;
                    objectData.name = gltfMesh.name;

                    // store new geometry
                    if (hasSkin)
                    {
                        auto& targetSkin = skins[node.skin];

                        assert(targetSkin.animSetId > 0);
                        objectData.animSetId = targetSkin.animSetId;

                        newMeshAnim.mesh() = newMesh.convert<geom::VertexShading, uint16_t>();

                        hr::streams::FileStream geomFileStream(areaData.pathBin, true, true);
                        World::geomFileAddMeshAnim(geomFileStream, objectId, newMeshAnim, targetSkin.animSetId);
                    }
                    else
                    {
                        hr::streams::FileStream geomFileStream(areaData.pathBin, true, true);
                        World::geomFileAddMesh(geomFileStream, objectId, newMesh);
                    }

                    // read material info
                    if (gltfPrim.material >= 0)
                    {
                        if (auto it = materialMapping.find(gltfPrim.material); it != materialMapping.end())
                        {
                            object.materialId = it->second.materialId;
                            object.textureSetId = it->second.textureSetId;
                        }
                        else
                        {
                            const auto& gltfMat = gltfModel.materials[gltfPrim.material];

                            auto materialTexSet = MaterialTextureSet{.materialId = genMaterialId(area, mRandom), .textureSetId = genTextureSetId(area, mRandom)};
                            materialMapping[gltfPrim.material] = materialTexSet;

                            auto& finalMat = area.mMaterials[materialTexSet.materialId];
                            finalMat.id = materialTexSet.materialId;
                            finalMat.name = gltfMat.name;

                            auto& finalTexSet = area.mTextureSets[materialTexSet.textureSetId];
                            finalTexSet.id = materialTexSet.textureSetId;

                            object.materialId = finalMat.id;
                            object.textureSetId = finalTexSet.id;
                            if (objectData.name.empty() && !finalMat.name.empty())
                                objectData.name = finalMat.name;

                            auto extractImageUri = [](const tinygltf::Model& model, const tinygltf::Material& material, std::string_view componentName) -> std::string
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

                            finalTexSet.diffusePath = extractImageUri(gltfModel, gltfMat, "baseColorTexture");
                            finalTexSet.normalPath = extractImageUri(gltfModel, gltfMat, "normalTexture");
                        }
                    }

                    // create an object associated with the concept
                    Instance newInstance;
                    newInstance.type = Instance::Type::Static;
                    newInstance.objectId = object.id;
                    newInstance.bbox = object.bbox;
                    area.mInstances.push_back(newInstance);
                }
            }
        };

        // now all that is left is to read animations of used skins
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

                geom::SkeletonAnim::Animation::Sampler finalSampler;
            };
            std::vector<SamplerData> samplers;

            // make sure all samplers are linear and read all their data
            {
                bool ignoreAnim{false};
                size_t sampleIndex{0};
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
                        newSampler.indexOutputs.push_back({.output = gltfSampler.output, .sampleIndex = sampleIndex});

                        // can already read the input (timeline)
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

            // for each channel
            for (const auto& gltfChannel : gltfAnim.channels)
            {
                if (gltfChannel.target_node < 0)
                    continue;
                if ((gltfChannel.sampler < 0) || (gltfChannel.sampler > gltfAnim.samplers.size()))
                    continue;

                auto itSampler = std::find_if(samplers.begin(), samplers.end(),
                                              [&gltfChannel](const auto& sampler)
                                              {
                                                  auto itOutput = std::find_if(sampler.indexOutputs.begin(), sampler.indexOutputs.end(), [&gltfChannel](const auto& indexOutput)
                                                                               { return (indexOutput.sampleIndex == gltfChannel.sampler); });

                                                  return (itOutput != sampler.indexOutputs.end());
                                              });
                assert(itSampler != samplers.end());

                // find the skin and the joint in that skin where the target node is
                std::optional<size_t> channelJointIndex;
                for (auto&& skin : skins)
                {
                    auto it = std::find_if(skin.joints.begin(), skin.joints.end(), [&gltfChannel](const auto& joint) { return (joint.nodeIndex == gltfChannel.target_node); });
                    if (it == skin.joints.end())
                        continue;

                    channelJointIndex = std::distance(skin.joints.begin(), it);

                    if (!animationSetId)
                        animationSetId = skin.animSetId;

                    assert(animationSetId.value() == skin.animSetId); // all the channels in an animation must point to the same skin
                    break;
                }

                if (!channelJointIndex)
                    continue;

                auto itSamplerOutput = std::find_if(itSampler->indexOutputs.begin(), itSampler->indexOutputs.end(),
                                                    [&gltfChannel](const auto& indexOutput) { return (indexOutput.sampleIndex == gltfChannel.sampler); });
                assert(itSamplerOutput != itSampler->indexOutputs.end());

                geom::SkeletonAnim::Animation::Channel channel;
                channel.jointIndex = *channelJointIndex;

                if (gltfChannel.target_path == "scale")
                {
                    channel.target = geom::SkeletonAnim::Animation::Channel::Target::Scale;
                    if (!gltfReadAnimSamplerData(gltfModel, itSamplerOutput->output, false, channel.frameData) ||
                        (channel.frameData.size() != itSampler->finalSampler.timePoints.size()))
                        continue;

                    // optimize away scaling by 1.0
                    if (std::all_of(channel.frameData.begin(), channel.frameData.end(),
                                    [](const auto& scale) { return Math::isZero(scale[0] - 1.0f) && Math::isZero(scale[1] - 1.0f) && Math::isZero(scale[2] - 1.0f); }))
                        continue;
                }
                else if (gltfChannel.target_path == "rotation")
                {
                    channel.target = geom::SkeletonAnim::Animation::Channel::Target::Rotation;
                    if (!gltfReadAnimSamplerData(gltfModel, itSamplerOutput->output, true, channel.frameData) ||
                        (channel.frameData.size() != itSampler->finalSampler.timePoints.size()))
                        continue;
                }
                else if (gltfChannel.target_path == "translation")
                {
                    channel.target = geom::SkeletonAnim::Animation::Channel::Target::Translation;
                    if (!gltfReadAnimSamplerData(gltfModel, itSamplerOutput->output, false, channel.frameData) ||
                        (channel.frameData.size() != itSampler->finalSampler.timePoints.size()))
                        continue;
                }

                itSampler->finalSampler.channels.push_back(std::move(channel));
            }

            // animation is ready, include in file

            if (animationSetId.has_value()) // glTF may have non skinned animations
            {
                geom::SkeletonAnim::Animation animation;
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

                size_t newAnimId = 1; // generate animation id
                {
                    auto& animSet = areaData.animationSets[animationSetId.value()];

                    newAnimId = animSet.animations.size() + 1;

                    AreaData::AnimSetData::Animation areaAnimation;
                    areaAnimation.id = newAnimId;
                    areaAnimation.name = gltfAnim.name;

                    animSet.animations.push_back(std::move(areaAnimation));
                }

                hr::streams::FileStream geomFileStream(areaData.pathBin, true, true);
                World::geomFileAddAnimation(geomFileStream, newAnimId, animationSetId.value(), std::move(animation));
            }
        }

        // need to save everything to file (new geometry was already saved)
        saveArea(area);

        return true;
    }

    void WorldEditor::processMesh(AreaId areaId, std::span<const size_t> objectIds, const std::function<void(geom::Mesh<geom::VertexFull, uint32_t>&)>& cb)
    {
        if (!cb || (mAreas.find(areaId) == mAreas.end()))
            return;

        assert(mAreasData.contains(areaId));

        hr::streams::FileStream geomFileStream(mAreasData[areaId].pathBin, true, true);
        World::geomFileTransformMeshes(geomFileStream, objectIds, cb);
    }

    std::vector<size_t> WorldEditor::unusedObjects(AreaId areaId) const
    {
        auto areaIt = mAreas.find(areaId);
        if (areaIt == mAreas.end())
            return {};

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

    void WorldEditor::removeObjects(AreaId areaId, std::span<std::string_view> objectsNames)
    {
        if (objectsNames.empty() || (mAreas.find(areaId) == mAreas.end()))
            return;

        auto& area = mAreas[areaId];
        auto& areaData = mAreasData[areaId];

        std::unordered_set<size_t> objectIds;
        for (auto&& targetName : objectsNames)
        {
            for (const auto& [objectId, object] : areaData.objects)
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
            area.mInstances.erase(std::remove_if(area.mInstances.begin(), area.mInstances.end(), [objectId](const auto& instance) { return (instance.objectId == objectId); }),
                                  area.mInstances.end());

            areaData.objects.erase(objectId);
        }

        saveArea(area); // this only takes care of the scene

        // now we take care of the binary data
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

    void WorldEditor::optimizeTextureSets(Area& area)
    {
        std::map<size_t, size_t> duplicated;

        // gather data
        for (const auto& [texSetId, texSet] : area.mTextureSets)
        {
            // we already found that this is one is a duplicate
            if (duplicated.contains(texSetId))
                continue;

            for (auto& [texSetCompId, texSetCompSet] : area.mTextureSets)
            {
                // if this one was already processed
                if (texSetId == texSetCompId)
                    continue;
                if (duplicated.contains(texSetCompId))
                    continue;

                // check if data is the same
                if (texSet.diffusePath != texSetCompSet.diffusePath)
                    continue;
                if (texSet.normalPath != texSetCompSet.normalPath)
                    continue;
                if (texSet.metallicRoughnessPath != texSetCompSet.metallicRoughnessPath)
                    continue;

                // this is a duplicate of texSet
                duplicated[texSetCompId] = texSetId;
            }
        }

        // remove duplicates
        for (const auto& ids : duplicated)
            area.mTextureSets.erase(ids.first);

        // for every object, replace the old id (duplicated) with the same one
        for (auto& [objectId, object] : area.mObjects)
        {
            if (object.textureSetId <= 0)
                continue;

            auto it = duplicated.find(object.textureSetId);
            if (it != duplicated.end())
                object.textureSetId = it->second;

            assert(area.mTextureSets.contains(object.textureSetId));
        }
    }

    void WorldEditor::saveArea(Area& area)
    {
        assert(mAreasData.find(area.id) != mAreasData.end());
        auto& areaData = mAreasData[area.id];

        // the area is always exported whole, which means that we can destroy the old version completly
        // also: we only need to save the scene (geom was already taken care of)

        if (std::filesystem::exists(areaData.pathScene))
            std::filesystem::remove(areaData.pathScene);

        // before we save, we can perform some optimizations
        optimizeTextureSets(area);

        // create file contents
        std::string jsonContent;
        {
            using nlohmann::json;

            json jFile;

            jFile["version"] = json::array({1, 0, 0});

            jFile["textureSets"] = json::array();
            for (auto& [texSetId, texSet] : area.mTextureSets)
            {
                auto jTexSet = json::object();

                assert(texSetId == texSet.id);
                jTexSet["id"] = texSetId;
                jTexSet["diffusePath"] = texSet.diffusePath;
                jTexSet["normalPath"] = texSet.normalPath;
                if (!texSet.metallicRoughnessPath.empty())
                    jTexSet["metallicRoughnessPath"] = texSet.metallicRoughnessPath;

                jFile["textureSets"].push_back(std::move(jTexSet));
            }

            jFile["materials"] = json::array();
            for (auto& [matId, mat] : area.mMaterials)
            {
                auto jMat = json::object();

                assert(matId == mat.id);
                jMat["id"] = matId;
                jMat["name"] = mat.name;
                jMat["baseColor"] = json::array({mat.baseColor[0], mat.baseColor[1], mat.baseColor[2], mat.baseColor[3]});
                jMat["metallicFactor"] = mat.metallicFactor;
                jMat["roughnessFactor"] = mat.roughnessFactor;

                jFile["materials"].push_back(std::move(jMat));
            }

            jFile["animationSets"] = json::array();
            for (auto& [animSetId, animSet] : areaData.animationSets)
            {
                auto jAnimSet = json::object();

                assert(animSetId == animSet.id);
                jAnimSet["id"] = animSetId;
                if (!animSet.name.empty())
                    jAnimSet["name"] = animSet.name;

                jAnimSet["animations"] = json::array();
                for (auto& animation : animSet.animations)
                {
                    auto jAnimation = json::object();

                    jAnimation["id"] = animation.id;
                    jAnimation["name"] = animation.name;

                    jAnimSet["animations"].push_back(std::move(jAnimation));
                }

                jFile["animationSets"].push_back(std::move(jAnimSet));
            }

            jFile["objects"] = json::array();
            for (auto& [objectId, object] : areaData.objects)
            {
                auto jObject = json::object();

                assert(objectId == object.objectId);
                assert(area.mObjects.contains(objectId));

                auto& areaObj = area.mObjects[objectId];

                jObject["id"] = objectId;
                jObject["type"] = static_cast<unsigned int>(Object::Type::Static);
                jObject["name"] = object.name;
                if (areaObj.materialId > 0)
                    jObject["materialId"] = areaObj.materialId;
                if (areaObj.textureSetId > 0)
                    jObject["textureSetId"] = areaObj.textureSetId;
                jObject["name"] = object.name;

                if (object.animSetId.has_value())
                    jObject["animationSetId"] = object.animSetId.value();

                jFile["objects"].push_back(std::move(jObject));
            }

            jFile["instances"] = json::array();
            for (auto& curObject : area.mInstances)
            {
                auto jInstance = json::object();

                jInstance["type"] = static_cast<unsigned int>(curObject.type);
                jInstance["objectId"] = curObject.objectId;

                jFile["instances"].push_back(std::move(jInstance));
            }

            jsonContent = jFile.dump(4);
        }

        hr::streams::FileStream streamScene(areaData.pathScene, false, true);
        streamScene.write(jsonContent.data(), jsonContent.size());
    }
}
