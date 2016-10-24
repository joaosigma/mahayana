#pragma once

#include "renderer.hpp"
#include "world.hpp"

#include "tools/camera.hpp"
#include "common/fileSystem.hpp"
#include "common/openGL/tools/viewport.hpp"
#include "common/openGL/tools/immediateMode.hpp"

namespace HorseRadish
{
	namespace Render
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

			HorseRadish::Render::World& renderWorld;
			VBOs vbos;
			FBOs fbos;
			Shaders shaders;
			Samplers samplers;
			HorseRadish::IO::FileSystem *fileSystem;
			HorseRadish::OpenGL::Tools::ImmediateMode mGlImmediateMode;
			int renderWidth, renderHeight, shadersWatchFolderID;
			HorseRadish::OpenGL::Objects::Texture texDefaultAlbedo, texDefaultNormals;

			void renderGBuffer(const Tools::Camera& hrCamera, const HorseRadish::OpenGL::Tools::Viewport& hrViewport);
			void renderFinal(const Tools::Camera& hrCamera, const HorseRadish::OpenGL::Tools::Viewport& hrViewport);
			void loadGeometry();
			void loadDiffuse(HorseRadish::IO::FileSystem &fileSystem, const std::string& texFilePath, HorseRadish::OpenGL::Objects::Texture& targetTexture);
			void loadNormal(HorseRadish::IO::FileSystem &fileSystem, const std::string& texFilePath, HorseRadish::OpenGL::Objects::Texture& targetTexture);
			void loadTextures(HorseRadish::IO::FileSystem &fileSystem);

		public:
			RendererDeferred(const HorseRadish::OpenGL::Objects::Context& glContext, HorseRadish::Render::World& renderWorld);
			~RendererDeferred();

			void Initialize(const int &renderWidth, const int &renderHeight, HorseRadish::IO::FileSystem * const fileSystem);

			//const HorseRadish::OpenGL::Objects::Texture* GetRTTexture(const RenderTargetType &renderTargetType) const;

			void LoadWorld(HorseRadish::IO::FileSystem &fileSystem);

			void Render(const Tools::Camera& hrCamera, const HorseRadish::OpenGL::Tools::Viewport& hrViewport);
		};

	} //Render
} //HorseRadish
