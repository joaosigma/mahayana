export module Renderer2D;

import std;

import core;
import gal;
import Font;

namespace hr::render
{
    export
    class Renderer2D
    {
    public:
        struct Shaders
        {
            struct
            {
                hr::gl::objects::ShaderProgram progVertex;
                hr::gl::objects::ShaderProgram progFragment;
                hr::gl::objects::ProgramPipeline progPipeline;
            } drawNoTex, text;

        } mShaders;

        struct GUI
        {
            std::unique_ptr<tools::Font> font;
        } mGui;

        size_t mRenderWidth = 0, mRenderHeight = 0;
        hr::gl::tools::ImmediateMode mGlImmediateMode;
        const hr::gl::objects::Context& glContext;

    public:
        Renderer2D(const hr::gl::objects::Context& glContext)
          : glContext(glContext)
        {}

        void initialize(size_t renderWidth, size_t renderHeight, const hr::io::FileSystem& fileSystem, std::string_view textFont);
    };
}
