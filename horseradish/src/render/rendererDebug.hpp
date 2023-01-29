#pragma once

#include "renderer.hpp"

#include "world.hpp"
#include "tools/camera.hpp"
#include "../common/fileSystem.hpp"
#include "../common/openGL/tools/viewport.hpp"
#include "../common/openGL/tools/immediateMode.hpp"

namespace hr::render
{
	class Renderer2D;
	class RendererMain;

	class RendererDebug
	{
		Renderer2D& mRenderer2D;
		hr::io::FileSystem& mFileSystem;
		const hr::gl::objects::Context& mGlContext;
		hr::gl::tools::ImmediateMode mGlImmediateMode;

		struct {
			bool drawBBoxes{ false };
			bool drawNormals{ false };
			bool drawTris{ false };
		} mOptions;

		struct {
			struct {
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
