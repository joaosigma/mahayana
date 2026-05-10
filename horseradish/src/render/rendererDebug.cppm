export module RendererDebug;

import core;
import gal;
import World;
import Renderer2D;
import RendererMain;
import Camera;

namespace hr::render
{
    export
    class RendererDebug
    {
        Renderer2D& mRenderer2D;
        hr::io::FileSystem& mFileSystem;
        const hr::gl::objects::Context& mGlContext;
        hr::gl::tools::ImmediateMode mGlImmediateMode;

        struct
        {
            bool drawBBoxes{false};
            bool drawNormals{false};
            bool drawTris{false};
        } mOptions;

        struct
        {
            struct
            {
                hr::gl::objects::ShaderProgram vertex;
                hr::gl::objects::ShaderProgram fragment;
                hr::gl::objects::ProgramPipeline pipeline;
            } geomDebug;
        } mShaders;

    public:
        RendererDebug(const hr::gl::objects::Context& glContext, hr::io::FileSystem& fileSystem, Renderer2D& renderer2D);
        ~RendererDebug();

        void render(RendererMain& rendererMain, const hr::render::World& world, const tools::Camera& hrCamera, const hr::gl::tools::Viewport& hrViewport);
    };
}
