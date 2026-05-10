module Renderer2D;

namespace hr::render
{
    void Renderer2D::initialize(size_t renderWidth, size_t renderHeight, const hr::io::FileSystem& fileSystem, std::string_view textFont)
    {
        mRenderWidth = renderWidth;
        mRenderHeight = renderHeight;

        mShaders.drawNoTex.progVertex.init(hr::gl::objects::ShaderProgram::Type::Vertex, fileSystem.readFileAsString("shaders/2dDraw.vshader"));
        mShaders.drawNoTex.progFragment.init(hr::gl::objects::ShaderProgram::Type::Fragment, fileSystem.readFileAsString("shaders/2dDraw.fshader"));

        mShaders.drawNoTex.progPipeline.init();
        mShaders.drawNoTex.progPipeline.setStage(mShaders.drawNoTex.progVertex);
        mShaders.drawNoTex.progPipeline.setStage(mShaders.drawNoTex.progFragment);

        mShaders.text.progVertex.init(hr::gl::objects::ShaderProgram::Type::Vertex, fileSystem.readFileAsString("shaders/2dText.vshader"));
        mShaders.text.progFragment.init(hr::gl::objects::ShaderProgram::Type::Fragment, fileSystem.readFileAsString("shaders/2dText.fshader"));

        mShaders.text.progPipeline.init();
        mShaders.text.progPipeline.setStage(mShaders.text.progVertex);
        mShaders.text.progPipeline.setStage(mShaders.text.progFragment);

        mGui.font = std::make_unique<tools::Font>(textFont, mShaders.text.progVertex.id(), mShaders.text.progFragment.id(), mShaders.text.progPipeline.id());
    }
}
