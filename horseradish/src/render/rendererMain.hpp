#pragma once

#include "renderer.hpp"
#include "world.hpp"

#include "tools/camera.hpp"
#include "../common/random.hpp"
#include "../common/fileSystem.hpp"
#include "../common/openGL/tools/viewport.hpp"
#include "../common/openGL/tools/immediateMode.hpp"

namespace hr { namespace render
{
	class RendererDebug;

	class RendererMain
		: public Renderer
	{
	public:
		enum RenderTargetType { Albedo, Normals, MiscA, MiscB };

	private:
		struct FBOs {
			hr::gl::objects::Sampler samplerTex;
			hr::gl::objects::FrameBuffer fboZPass, fboForward;
			hr::gl::objects::Texture texZ, texLighting, texNormals, texSpecular;
		};

		struct Shaders{
			struct {
				hr::gl::objects::ShaderProgram vertex;
				hr::gl::objects::ShaderProgram fragment;
				hr::gl::objects::ProgramPipeline pipeline;
			} forwardPassZ, forwardPassLighting, forwardPassSky;

			struct UniformLayout {
				float matTrans[4 * 4];
				float matView[4 * 4];
				uint32_t numLights;
			};
			static const constexpr size_t LightLayoutMaxElements = 256;
			struct LightLayout {
				float dir[4];
				float diffuse[4];
			};
			struct {
				hr::gl::objects::FenceSync fence;
				hr::gl::objects::Buffer uniform;
				hr::gl::objects::Buffer storage;
			} forwardPassBuffers;

			struct {
				hr::gl::objects::ShaderProgram vertex;
				hr::gl::objects::ShaderProgram fragment;
				hr::gl::objects::ProgramPipeline pipeline;
			} postprocess;
		};

		struct Samplers{
			hr::gl::objects::Sampler samplerSky;
			hr::gl::objects::Sampler samplerAlbedo;
			hr::gl::objects::Sampler samplerNormals;
		};

		hr::render::World& mWorld;
		FBOs mFBOs;
		Shaders mShaders;
		Samplers mSamplers;
		hr::Random mRand;
		hr::io::FileSystem& mFileSystem;
		hr::gl::tools::ImmediateMode mGlImmediateMode;
		int mShadersWatchFolderID;
		hr::gl::objects::Texture mTexSky;
		hr::gl::objects::Texture mTexDefaultAlbedo, mTexDefaultNormals;

		void passDepth(const tools::Camera& hrCamera, const hr::gl::tools::Viewport& hrViewport);
		void passLighting(const tools::Camera& hrCamera, const hr::gl::tools::Viewport& hrViewport);
		void passSky(const tools::Camera& hrCamera, const hr::gl::tools::Viewport& hrViewport);

		void compositePostProcessing(const hr::gl::tools::Viewport& hrViewport);

		void loadGeometry();
		void loadDiffuse(const std::string& texFilePath, hr::gl::objects::Texture& targetTexture, bool compress);
		void loadNormal(const std::string& texFilePath, hr::gl::objects::Texture& targetTexture, bool compress);
		void loadTextures();

	public:
		RendererMain(const hr::gl::objects::Context& glContext, hr::io::FileSystem& fileSystem, hr::render::World& renderWorld, size_t renderWidth, size_t renderHeight);
		~RendererMain();

		void loadWorld();

		void render(const tools::Camera& hrCamera, const hr::gl::tools::Viewport& hrViewport);
		void renderDebug(RendererDebug& rendererDebug, const tools::Camera& hrCamera, const hr::gl::tools::Viewport& hrViewport);
		void renderComposite(const hr::gl::tools::Viewport& hrViewport);
	};
} }
