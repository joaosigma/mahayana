#pragma once

#include "common/opengl/objects.hpp"
#include "common/opengl/tools/viewport.hpp"
#include "engine/logger.hpp"
#include "engine/runtime.hpp"
#include "render/scene.hpp"

#include "platform/window.hpp"

namespace hr::render
{
    class Stage
    {
        class SceneRuntimeProxy;

    private:
        bool mScenesDrawned = false;
        engine::Runtime& mRuntime;
        engine::Logger::Context& mLogger;
        hr::io::FileSystem& mFileSystem;
        std::vector<SceneRuntimeProxy*> mTempScenes;
        hr::gl::objects::Context& mGlCtx;
        std::unordered_map<std::string, SceneRuntimeProxy*> mRuntimeScenes;

        struct RenderData
        {
            hr::gl::objects::FrameBuffer fbo;
            hr::gl::objects::Sampler sampler;
            hr::gl::objects::ShaderProgram progVertex;
            hr::gl::objects::ShaderProgram progFragment;
            hr::gl::objects::ProgramPipeline progPipeline;
            hr::gl::objects::Texture texColor;

            hr::gl::tools::ImmediateMode imode;
        } mRenderData;

        void runtimeFuncSceneGet(const std::string& funcName, engine::Runtime::FunctionReturnContext& ctx);
        void runtimeFuncSceneCreate(const std::string& funcName, engine::Runtime::FunctionReturnContext& ctx);
        void runtimeFuncSceneDestroy(const std::string& funcName, engine::Runtime::FunctionReturnContext& ctx);
        void runtimeFuncSceneAddRemove(const std::string& funcName, engine::Runtime::FunctionReturnContext& ctx);

    public:
        Stage(engine::Runtime& runtime, engine::Logger::Context& logger, hr::io::FileSystem& fileSystem, hr::gl::objects::Context& glCtx, size_t renderWidth, size_t renderHeight);
        ~Stage();

        Stage(const Stage&) = delete;
        Stage& operator=(const Stage&) = delete;

        void drawScenes();
        void drawComposite(const hr::gl::tools::Viewport& viewport);
        void processStep();
        void processMessage(const platform::Window::Message& msg);
    };
}
