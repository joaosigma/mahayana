#pragma once

#include "../common/fileSystem.hpp"
#include "../common/openGL/tools/immediateMode.hpp"
#include "renderer.hpp"
#include "tools/font.hpp"

namespace hr::render
{
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

        void initialize(size_t renderWidth, size_t renderHeight, hr::io::FileSystem* const fileSystem, const char* const textFont);
    };
}
