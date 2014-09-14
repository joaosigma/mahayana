#pragma once

#include "renderer.hpp"

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
				const HorseRadish::OpenGL::Objects::VertexBuffer *vboMeshData, *vboMeshIndexData;
				const HorseRadish::OpenGL::Objects::VertexArray *vaoMesh;

				VBOs() : vboMeshSize(0), vboMeshIndexSize(0), vboMeshData(nullptr), vboMeshIndexData(nullptr), vaoMesh(nullptr) { }
			};

			struct FBOs{
				const HorseRadish::OpenGL::Objects::FrameBuffer *fboDeferredGBuffer;
				const HorseRadish::OpenGL::Objects::Sampler *samplerTexs;
				const HorseRadish::OpenGL::Objects::Texture *texDeferredZ, *texDeferredAlbedo, *texDeferredNormals, *texDeferredMiscA, *texDeferredMiscB;

				FBOs() : fboDeferredGBuffer(nullptr), texDeferredZ(0), texDeferredAlbedo(nullptr), texDeferredNormals(nullptr), texDeferredMiscA(nullptr), texDeferredMiscB(nullptr) { }
			};

			struct Shaders{
				const HorseRadish::OpenGL::Objects::Program *progDeferredGBuffer;
				const HorseRadish::OpenGL::Objects::Program *progMainDebug;
				const HorseRadish::OpenGL::Objects::Program *progPerVertexLightDir;

				const HorseRadish::OpenGL::Objects::Program *progPPSimpleColor;

				const HorseRadish::OpenGL::Objects::Program *renderZPass;

				Shaders() : progDeferredGBuffer(nullptr), progMainDebug(nullptr), progPerVertexLightDir(nullptr), progPPSimpleColor(nullptr), renderZPass(nullptr) { }
			};

			struct Samplers{
				const HorseRadish::OpenGL::Objects::Sampler *samplerAlbedo;
				const HorseRadish::OpenGL::Objects::Sampler *samplerNormals;
			};

			HorseRadish::Render::World *renderWorld;
			VBOs vbos;
			FBOs fbos;
			Shaders shaders;
			Samplers samplers;
			HorseRadish::IO::FileSystem *fileSystem;
			HorseRadish::OpenGL::Objects::ObjectsManager *glObjectManager;
			HorseRadish::OpenGL::Objects::ObjectsManager *glTextureManager;
			HorseRadish::OpenGL::Tools::UniformCache *glUniformCache;
			HorseRadish::OpenGL::Tools::ImmediateMode *glImmediateMode;
			int renderWidth, renderHeight, shadersWatchFolderID;
			const HorseRadish::OpenGL::Objects::Texture *texDefaultAlbedo, *texDefaultNormals;

			void renderGBuffer(const HorseRadish::OpenGL::Tools::Camera * const hrCamera, const HorseRadish::OpenGL::Tools::Viewport * const hrViewport);
			void renderFinal(const HorseRadish::OpenGL::Tools::Camera * const hrCamera, const HorseRadish::OpenGL::Tools::Viewport * const hrViewport);
			void loadGeometry();
			TextureSet::Texture* findTexType(TextureSet * const texSet, const int texType);
			const HorseRadish::OpenGL::Objects::Texture* loadDiffuse(HorseRadish::IO::FileSystem &fileSystem, TextureSet::Texture* texture);
			const HorseRadish::OpenGL::Objects::Texture* loadNormal(HorseRadish::IO::FileSystem &fileSystem, TextureSet::Texture* texture);
			void loadTextures(HorseRadish::IO::FileSystem &fileSystem);

		public:
			RendererDeferred(HorseRadish::OpenGL::Objects::Context * const glContext, HorseRadish::Render::World* const renderWorld);
			~RendererDeferred();

			void Initialize(const int &renderWidth, const int &renderHeight, HorseRadish::IO::FileSystem * const fileSystem);

			const HorseRadish::OpenGL::Objects::Texture* GetRTTexture(const RenderTargetType &renderTargetType) const;

			void LoadWorld(HorseRadish::IO::FileSystem &fileSystem);

			void Render(const HorseRadish::OpenGL::Tools::Camera * const hrCamera, const HorseRadish::OpenGL::Tools::Viewport * const hrViewport);
		};

	} //Render
} //HorseRadish
