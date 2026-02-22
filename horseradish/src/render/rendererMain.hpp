#pragma once

#include "renderer.hpp"

#include "../common/fileSystem.hpp"
#include "../common/openGL/tools/immediateMode.hpp"
#include "../common/openGL/tools/viewport.hpp"
#include "../common/random.hpp"
#include "tools/camera.hpp"
#include "world.hpp"

#include <atomic>
#include <string_view>

namespace hr::render
{
    class RendererDebug;

    class RendererMain: public IRenderer
    {
        friend class RendererDebug;

    public:
        enum RenderTargetType
        {
            Albedo,
            Normals,
            MiscA,
            MiscB
        };

    private:
        struct Scene
        {
            struct Material
            {
                size_t id{0};
            };
            std::unordered_map<size_t, Material> mMaterials;

            struct TextureSet
            {
                size_t id{0};

                hr::gl::objects::Texture texDiffuse, texNormal;
            };
            std::unordered_map<size_t, TextureSet> mTextureSets;

            struct Object
            {
                size_t id{0};
                size_t materialId{0}, textureSetId{0};

                int meshVBOStartPos{0};
                int meshVBOVertexOffset{0};
                unsigned int meshDrawIndirectOffset{0};
                void* meshTriListOffset{nullptr};

                hr::BBox<> bbox;
            };
            std::unordered_map<size_t, Object> mObjects;

            struct ObjectAnim: public Object
            {};
            std::unordered_map<size_t, ObjectAnim> mObjectsAnim;

            struct RenderData
            {
                std::vector<Object*> objects;

                hr::gl::objects::Buffer vboMeshData, vboMeshIndexData, vboIndirectDraw;
                hr::gl::objects::VertexArray vaoMesh;
            };
            RenderData mRenderData;
        };

        struct FBOs
        {
            hr::gl::objects::Sampler samplerTex;
            hr::gl::objects::Sampler samplerLuminance;
            hr::gl::objects::FrameBuffer fboZPass, fboForward;
            hr::gl::objects::Texture texZ, texLighting, texNormals, texSpecular, texAvgLuminance;
        };

        struct Shaders
        {
            struct
            {
                hr::gl::objects::ShaderProgram vertex;
                hr::gl::objects::ShaderProgram fragment;
                hr::gl::objects::ProgramPipeline pipeline;
            } forwardPassZ, forwardPassLighting, forwardPassSky;

            struct UniformLayout
            {
                float matProj[4 * 4];
                float matMView[4 * 4];
                uint32_t numLights;
            };
            static const constexpr size_t LightLayoutMaxElements = 256;
            struct LightLayout
            {
                float dir[4];
                float diffuse[4];
            };
            struct
            {
                hr::gl::objects::FenceSync fence;
                hr::gl::objects::Buffer uniform;
                hr::gl::objects::Buffer storage;
            } forwardPassBuffers;

            struct
            {
                hr::gl::objects::ShaderProgram vertex;
                hr::gl::objects::ShaderProgram fragment;
                hr::gl::objects::ProgramPipeline pipeline;

                hr::gl::objects::ShaderProgram computeSaturateShader;
                hr::gl::objects::ProgramPipeline computeSaturatePipeline;
            } postprocess;
        };

        struct Samplers
        {
            hr::gl::objects::Sampler samplerSky;
            hr::gl::objects::Sampler samplerAlbedo;
            hr::gl::objects::Sampler samplerNormals;
        };

        FBOs mFBOs;
        Shaders mShaders;
        Samplers mSamplers;
        hr::Random mRand;
        hr::io::FileSystem& mFileSystem;
        hr::gl::tools::ImmediateMode mGlImmediateMode;
        int mShadersWatchFolderID;
        hr::gl::objects::Texture mTexSky;
        hr::gl::objects::Texture mTexDefaultAlbedo, mTexDefaultNormals;
        std::unordered_map<SceneId, Scene> mScenes;
        std::atomic<SceneId> mGenSceneIds{1};
        const hr::gl::objects::Context& mGlContext;

        void passDepth(Scene& scene, const tools::Camera& hrCamera, const hr::gl::tools::Viewport& hrViewport);
        void passLighting(Scene& scene, const tools::Camera& hrCamera, const hr::gl::tools::Viewport& hrViewport);
        void passSky(Scene& scene, const tools::Camera& hrCamera, const hr::gl::tools::Viewport& hrViewport);

        void compositePostProcessing(const hr::gl::tools::Viewport& hrViewport);

        void loadGeometry(Scene& scene, const IRenderManager& manager);
        void loadDiffuse(std::string_view texFilePath, hr::gl::objects::Texture& targetTexture, bool compress);
        void loadNormal(std::string_view texFilePath, hr::gl::objects::Texture& targetTexture, bool compress);
        void loadMaterialsTextures(Scene& scene, const IRenderManager& manager);

    public:
        RendererMain(const hr::gl::objects::Context& glContext, hr::io::FileSystem& fileSystem, size_t renderWidth, size_t renderHeight);
        ~RendererMain();

        void render(const tools::Camera& hrCamera, const hr::gl::tools::Viewport& hrViewport);
        void renderDebug(RendererDebug& rendererDebug, const hr::render::World& world, const tools::Camera& hrCamera, const hr::gl::tools::Viewport& hrViewport);
        void renderComposite(const hr::gl::tools::Viewport& hrViewport);

        // IRenderer implementation

        SceneId loadScene(const IRenderManager& manager) override;
        void unloadScene(SceneId sceneId) override;

        void updateVertexData(SceneId sceneId, const IRenderManager& manager) override;
        void prepareNextFrame(SceneId sceneId, const std::vector<IRenderObject::ObjectId>& objects) override;
    };
}
