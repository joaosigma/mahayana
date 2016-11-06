#pragma once

#include "renderer.hpp"
#include "world.hpp"

#include "tools/camera.hpp"
#include "../common/fileSystem.hpp"
#include "../common/openGL/tools/viewport.hpp"
#include "../common/openGL/tools/immediateMode.hpp"

namespace HorseRadish { namespace Render
{
	class RendererDeferred : public Renderer
	{
	public:
		enum RenderTargetType { Albedo, Normals, MiscA, MiscB };

	private:
		struct VBOs{
			unsigned int vboMeshSize, vboMeshIndexSize;
			HorseRadish::OpenGL::Objects::Buffer vboMeshData, vboMeshIndexData, vboIndirectDraw;
			HorseRadish::OpenGL::Objects::VertexArray vaoMesh;

			VBOs() : vboMeshSize(0), vboMeshIndexSize(0) { }
		};

		struct FBOs{
			HorseRadish::OpenGL::Objects::Sampler samplerTexs;
			HorseRadish::OpenGL::Objects::FrameBuffer fboDeferredGBuffer;
			HorseRadish::OpenGL::Objects::Texture texDeferredZ, texDeferredAlbedo, texDeferredNormals, texDeferredMiscA, texDeferredMiscB;
		};

		struct Shaders{
			struct {
				HorseRadish::OpenGL::Objects::ShaderProgram vertex;
				HorseRadish::OpenGL::Objects::ShaderProgram fragment;
				HorseRadish::OpenGL::Objects::ProgramPipeline pipeline;
			} deferred;

			struct {
				HorseRadish::OpenGL::Objects::ShaderProgram vertex;
				HorseRadish::OpenGL::Objects::ShaderProgram fragment;
				HorseRadish::OpenGL::Objects::ProgramPipeline pipeline;
			} postprocess;
		};

		struct Samplers{
			HorseRadish::OpenGL::Objects::Sampler samplerAlbedo;
			HorseRadish::OpenGL::Objects::Sampler samplerNormals;
		};

		HorseRadish::Render::World& mWorld;
		VBOs mVBOs;
		FBOs mFBOs;
		Shaders mShaders;
		Samplers mSamplers;
		HorseRadish::IO::FileSystem& mFileSystem;
		HorseRadish::OpenGL::Tools::ImmediateMode mGlImmediateMode;
		int mShadersWatchFolderID;
		HorseRadish::OpenGL::Objects::Texture mTexDefaultAlbedo, mTexDefaultNormals;

		void renderGBuffer(const Tools::Camera& hrCamera, const HorseRadish::OpenGL::Tools::Viewport& hrViewport);
		void renderFinal(const HorseRadish::OpenGL::Tools::Viewport& hrViewport);
		void loadGeometry();
		void loadDiffuse(const std::string& texFilePath, HorseRadish::OpenGL::Objects::Texture& targetTexture);
		void loadNormal(const std::string& texFilePath, HorseRadish::OpenGL::Objects::Texture& targetTexture);
		void loadTextures();

	public:
		RendererDeferred(const HorseRadish::OpenGL::Objects::Context& glContext, HorseRadish::IO::FileSystem& fileSystem, HorseRadish::Render::World& renderWorld, size_t renderWidth, size_t renderHeight);
		~RendererDeferred();

		void loadWorld();

		void render(const Tools::Camera& hrCamera, const HorseRadish::OpenGL::Tools::Viewport& hrViewport);
		void renderComposite(const HorseRadish::OpenGL::Tools::Viewport& hrViewport);
	};
} }
