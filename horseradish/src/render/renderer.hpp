#pragma once

#include "../common/opengl/tools/viewport.hpp"
#include "tools/camera.hpp"

#include <functional>
#include <string_view>

namespace hr::render
{
    class IRenderMaterial
    {
    public:
        typedef size_t MaterialId;

        static constexpr MaterialId InvalidMaterialId = 0;

    public:
        virtual ~IRenderMaterial() = default;

        virtual MaterialId id() const = 0;
    };

    class IRenderTextureSet
    {
    public:
        typedef size_t TextureSetId;

        static constexpr TextureSetId InvalidTextureSetId = 0;

    public:
        virtual ~IRenderTextureSet() = default;

        virtual TextureSetId id() const = 0;

        virtual std::string_view diffusePath() const = 0;
        virtual std::string_view normalPath() const = 0;
    };

    class IRenderObject
    {
    public:
        typedef size_t ObjectId;

        static constexpr ObjectId InvalidObjectId = 0;

    public:
        virtual ~IRenderObject() = default;

        virtual ObjectId id() const = 0;

        virtual IRenderMaterial::MaterialId materialId() const = 0;
        virtual IRenderTextureSet::TextureSetId textureSetId() const = 0;

        virtual BBox<> bbox() const = 0;

        virtual size_t numVertices() const = 0;
        virtual size_t readVertices(void* const destBuffer, size_t requestedDataSize) const = 0;

        virtual size_t numIndices() const = 0;
        virtual size_t readIndices(void* const destBuffer, size_t requestedDataSize) const = 0;
    };

    class IRenderManager
    {
    public:
        virtual ~IRenderManager() = default;

        virtual size_t numMaterials() const = 0;
        virtual void iterateMaterials(const std::function<void(IRenderMaterial&)>& cb) const = 0;

        virtual size_t numTextureSets() const = 0;
        virtual void iterateTextureSets(const std::function<void(IRenderTextureSet&)>& cb) const = 0;

        virtual size_t numObjects() const = 0;
        virtual void iterateObjects(const std::function<void(IRenderObject&)>& cb) const = 0;
    };

    class IRenderer
    {
    public:
        typedef size_t SceneId;

        static constexpr SceneId InvalidSceneId = 0;

    public:
        virtual ~IRenderer() = default;

        virtual SceneId loadScene(const IRenderManager& manager) = 0;
        virtual void unloadScene(SceneId sceneId) = 0;

        virtual void updateVertexData(SceneId sceneId, const IRenderManager& manager) = 0;
        virtual void prepareNextFrame(SceneId sceneId, const std::vector<IRenderObject::ObjectId>& objects) = 0;
    };
}
