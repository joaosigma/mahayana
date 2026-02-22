#include "rendererDebug.hpp"

#include "renderer2D.hpp"
#include "rendererMain.hpp"

#include <algorithm>
#include <cstddef>

namespace hr::render
{
    RendererDebug::RendererDebug(const hr::gl::objects::Context& glContext, hr::io::FileSystem& fileSystem, Renderer2D& renderer2D)
      : mRenderer2D(renderer2D), mFileSystem(fileSystem), mGlContext(glContext)
    {
        // shaders
        mShaders.geomDebug.vertex.init(hr::gl::objects::ShaderProgram::Type::Vertex, mFileSystem.readFileAsString("shaders/debug_geom.vshader"));
        mShaders.geomDebug.fragment.init(hr::gl::objects::ShaderProgram::Type::Fragment, mFileSystem.readFileAsString("shaders/debug_geom.fshader"));
        mShaders.geomDebug.pipeline.init();
        mShaders.geomDebug.pipeline.setStage(mShaders.geomDebug.vertex);
        mShaders.geomDebug.pipeline.setStage(mShaders.geomDebug.fragment);
    }

    RendererDebug::~RendererDebug()
    {}

    void RendererDebug::render(RendererMain& rendererMain, const hr::render::World& world, const tools::Camera& hrCamera, const hr::gl::tools::Viewport& hrViewport)
    {
        hr::gl::glEnable(GL_DEPTH_TEST);
        hr::gl::glDepthMask(GL_FALSE);
        hr::gl::glDepthFunc(GL_GREATER);

        auto matrixTransform = hrCamera.modelView() * hrViewport.getProjection(hr::gl::tools::Viewport::ProjectionType::Proj3D);

        if (mOptions.drawBBoxes || mOptions.drawNormals)
        {
            hr::gl::glBindProgramPipeline(mRenderer2D.mShaders.drawNoTex.progPipeline.id());
            hr::gl::glProgramUniformMatrix4fv(mRenderer2D.mShaders.drawNoTex.progVertex.id(), mRenderer2D.mShaders.drawNoTex.progVertex.getUniformLocation("matTrans"), 1, false,
                                              matrixTransform.data().data());

            for (auto&& [sceneId, scene] : rendererMain.mScenes)
            {
                for (auto& curObject : scene.mRenderData.objects)
                {
                    if (mOptions.drawBBoxes)
                    {
                        Vector3f points[8];
                        curObject->bbox.corners(points);

                        // bottom and top (with an extra edge)
                        mGlImmediateMode.beginDraw(hr::gl::tools::ImmediateMode::GeometryType::LineStrip);
                        mGlImmediateMode.setColor(255, 0, 0);

                        mGlImmediateMode.addPosition(points[0][0], points[0][1], points[0][2]);
                        mGlImmediateMode.addPosition(points[1][0], points[1][1], points[1][2]);
                        mGlImmediateMode.addPosition(points[5][0], points[5][1], points[5][2]);
                        mGlImmediateMode.addPosition(points[4][0], points[4][1], points[4][2]);

                        mGlImmediateMode.addPosition(points[0][0], points[0][1], points[0][2]);
                        mGlImmediateMode.addPosition(points[2][0], points[2][1], points[2][2]);
                        mGlImmediateMode.addPosition(points[3][0], points[3][1], points[3][2]);
                        mGlImmediateMode.addPosition(points[7][0], points[7][1], points[7][2]);
                        mGlImmediateMode.addPosition(points[6][0], points[6][1], points[6][2]);
                        mGlImmediateMode.addPosition(points[2][0], points[2][1], points[2][2]);

                        mGlImmediateMode.endDraw();

                        // the remaing three edges
                        mGlImmediateMode.beginDraw(hr::gl::tools::ImmediateMode::GeometryType::Lines);
                        mGlImmediateMode.setColor(255, 0, 0);

                        mGlImmediateMode.addPosition(points[1][0], points[1][1], points[1][2]);
                        mGlImmediateMode.addPosition(points[3][0], points[3][1], points[3][2]);

                        mGlImmediateMode.addPosition(points[5][0], points[5][1], points[5][2]);
                        mGlImmediateMode.addPosition(points[7][0], points[7][1], points[7][2]);

                        mGlImmediateMode.addPosition(points[4][0], points[4][1], points[4][2]);
                        mGlImmediateMode.addPosition(points[6][0], points[6][1], points[6][2]);
                        mGlImmediateMode.endDraw();
                    }

                    // if (mOptions.drawNormals)
                    //{
                    //	mGlImmediateMode.beginDraw(hr::gl::tools::ImmediateMode::GeometryType::Lines);
                    //	mGlImmediateMode.setColor(255, 0, 0);
                    //
                    //	curObject
                    //	scene.mObjects
                    //
                    //	auto& mesh = mWorld.mConcepts[curObject->conceptId].mesh;
                    //	for (size_t i = 0; i < mesh.numVertices(); i++)
                    //	{
                    //		Vector3f pos(mesh.vertices()[i].pos);
                    //
                    //		Vector3f normal;
                    //		hr::geom::Mesh::unpack(mesh.vertices()[i].normal, normal.data(), 3);
                    //
                    //		mGlImmediateMode.addPosition(pos[0], pos[1], pos[2]);
                    //		pos += normal;
                    //		mGlImmediateMode.addPosition(pos[0], pos[1], pos[2]);
                    //	}
                    //
                    //	mGlImmediateMode.endDraw();
                    // }
                }
            }
        }

        if (mOptions.drawTris)
        {
            hr::gl::glProgramUniformMatrix4fv(mShaders.geomDebug.vertex.id(), mShaders.geomDebug.vertex.getUniformLocation("matTrans"), 1, false, matrixTransform.data().data());
            hr::gl::glBindProgramPipeline(mShaders.geomDebug.pipeline.id());

            hr::gl::glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

            hr::gl::glEnable(GL_POLYGON_OFFSET_LINE);
            hr::gl::glPolygonOffset(1.0f, 1.0f);

            for (auto&& [sceneId, scene] : rendererMain.mScenes)
            {
                scene.mRenderData.vaoMesh.bind();
                scene.mRenderData.vboIndirectDraw.bind();

                for (auto& curObject : scene.mRenderData.objects)
                    hr::gl::glMultiDrawElementsIndirect(GL_TRIANGLES, GL_UNSIGNED_SHORT, reinterpret_cast<void*>(curObject->meshDrawIndirectOffset), 1, 0);

                scene.mRenderData.vboIndirectDraw.unbind();
            }

            hr::gl::glPolygonOffset(0.0f, 0.0f);
            hr::gl::glDisable(GL_POLYGON_OFFSET_LINE);

            hr::gl::glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
        }
    }
}
