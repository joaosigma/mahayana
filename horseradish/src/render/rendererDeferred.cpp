#include "rendererDeferred.hpp"

#include "common\stringUtils.hpp"
#include "common\imageFactory.hpp"

#include <cstddef>
#include <algorithm>

namespace HorseRadish
{
	namespace Render
	{
		void RendererDeferred::renderGBuffer(const Tools::Camera& hrCamera, const HorseRadish::OpenGL::Tools::Viewport& hrViewport)
		{
			HorseRadish::Matrix matrixModelView;

			HorseRadish::Matrix matrixTransformacao = hrViewport.getProjection(HorseRadish::OpenGL::Tools::Viewport::ProjectionType::Proj3D);
			matrixModelView.set(hrCamera.GetModelView());
			matrixTransformacao *= matrixModelView;

			vbos.vaoMesh.bind();
			fbos.fboDeferredGBuffer.bind();

			HorseRadish::OpenGL::glViewport(0, 0, hrViewport.getWidth(), hrViewport.getHeight());

			GLenum mrt[] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2, GL_COLOR_ATTACHMENT3 };
			HorseRadish::OpenGL::glDrawBuffers(4, mrt);

			HorseRadish::OpenGL::glEnable(GL_DEPTH_TEST);
			HorseRadish::OpenGL::glDepthMask(GL_TRUE);
			HorseRadish::OpenGL::glDepthFunc(GL_LEQUAL);

			HorseRadish::OpenGL::glDisable(GL_SCISSOR_TEST);
			HorseRadish::OpenGL::glDisable(GL_BLEND);
			HorseRadish::OpenGL::glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
			HorseRadish::OpenGL::glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);

			HorseRadish::OpenGL::glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

			HorseRadish::OpenGL::glUseProgram(0);
			HorseRadish::OpenGL::glProgramUniformMatrix4fv(this->shaders.deferred.vertex.getId(), this->shaders.deferred.vertex.getUniformLocation("matView"), 1, false, matrixModelView.data());
			HorseRadish::OpenGL::glProgramUniformMatrix4fv(this->shaders.deferred.vertex.getId(), this->shaders.deferred.vertex.getUniformLocation("matTrans"), 1, false, matrixTransformacao.data());
			HorseRadish::OpenGL::glProgramUniform1f(this->shaders.deferred.fragment.getId(), this->shaders.deferred.fragment.getUniformLocation("farClipPlane"), hrViewport.getZFar());
			HorseRadish::OpenGL::glBindProgramPipeline(this->shaders.deferred.pipeline.getId());

			this->samplers.samplerNormals.bind(1);
			this->samplers.samplerAlbedo.bind(0);

			vbos.vboIndirectDraw.bind();
			for (auto& curObject : this->renderWorld.mRenderData.objects)
			{
				auto& concept = this->renderWorld.mConcepts[curObject->conceptName];

				if (concept.renderData.texNormal.isValid())
					concept.renderData.texNormal.bind(1);
				else
					this->texDefaultNormals.bind(1);

				if (concept.renderData.texDiffuse.isValid())
					concept.renderData.texDiffuse.bind(0);
				else
					this->texDefaultAlbedo.bind(0);

				HorseRadish::OpenGL::glMultiDrawElementsIndirect(GL_TRIANGLES, GL_UNSIGNED_SHORT, reinterpret_cast<void*>(concept.renderData.meshDrawIndirectOffset), 1, 0);
			}
			vbos.vboIndirectDraw.unbind();

			HorseRadish::OpenGL::glBindVertexArray(0);
			HorseRadish::OpenGL::glUseProgram(0);
		}

		void RendererDeferred::renderFinal(const Tools::Camera& hrCamera, const HorseRadish::OpenGL::Tools::Viewport& hrViewport)
		{
			auto winX = hrViewport.getWidth();
			auto winY = hrViewport.getHeight();

			HorseRadish::OpenGL::glBindFramebuffer(GL_FRAMEBUFFER, 0);

			HorseRadish::OpenGL::glDisable(GL_BLEND);
			HorseRadish::OpenGL::glDisable(GL_DEPTH_TEST);
			HorseRadish::OpenGL::glDepthMask(GL_FALSE);

			HorseRadish::OpenGL::glUseProgram(0);
			HorseRadish::OpenGL::glProgramUniformMatrix4fv(this->shaders.postprocess.vertex.getId(), this->shaders.postprocess.vertex.getUniformLocation("projectionMatrix"), 1, false, hrViewport.getProjection(HorseRadish::OpenGL::Tools::Viewport::ProjectionType::Proj2D).data());
			HorseRadish::OpenGL::glBindProgramPipeline(this->shaders.postprocess.pipeline.getId());

			fbos.samplerTexs.bind(4);
			fbos.samplerTexs.bind(3);
			fbos.samplerTexs.bind(2);
			fbos.samplerTexs.bind(1);
			fbos.samplerTexs.bind(0);

			fbos.texDeferredZ.bind(4);
			fbos.texDeferredMiscB.bind(3);
			fbos.texDeferredMiscA.bind(2);
			fbos.texDeferredNormals.bind(1);
			fbos.texDeferredAlbedo.bind(0);

			mGlImmediateMode.beginDraw(HorseRadish::OpenGL::Tools::ImmediateMode::GeometryType::Quads);
				mGlImmediateMode.setColorF(1.0f);
				mGlImmediateMode.addQuadTexCoords(0.0f, 0.0f, winX, winY, false);
			mGlImmediateMode.endDraw();
		}

		void RendererDeferred::loadGeometry()
		{
			vbos.vboMeshData.reset();
			vbos.vboMeshIndexData.reset();
			this->vbos.vboMeshSize = this->vbos.vboMeshIndexSize = 0;

			for (const auto& concept : this->renderWorld.mConcepts)
			{
				this->vbos.vboMeshSize += concept.second.mesh.sizeVertices();
				this->vbos.vboMeshIndexSize += concept.second.mesh.sizeIndices();
			}

			vbos.vboMeshData.init(OpenGL::Objects::Buffer::Type::ArrayBuffer, vbos.vboMeshSize, OpenGL::Objects::Buffer::UsageType::PersistentOnlyWrite);
			vbos.vboMeshIndexData.init(OpenGL::Objects::Buffer::Type::ElementArrayBuffer, vbos.vboMeshIndexSize, OpenGL::Objects::Buffer::UsageType::PersistentOnlyWrite);

			int baseVertexOffset = 0;
			int poolVertex = 0, poolIndex = 0;
			int numGeoms = 0;
			for (auto& concept : this->renderWorld.mConcepts)
			{
				numGeoms++;
				concept.second.renderData.meshVBOVertexOffset = baseVertexOffset;
				concept.second.renderData.meshTriListOffset = (void*)poolIndex;

				vbos.vboMeshData.writeData(concept.second.mesh.dataVertices(), concept.second.mesh.sizeVertices(), poolVertex);
				poolVertex += concept.second.mesh.sizeVertices();

				baseVertexOffset += concept.second.mesh.numVertices();

				vbos.vboMeshIndexData.writeData(concept.second.mesh.dataIndices(), concept.second.mesh.sizeIndices(), poolIndex);
				poolIndex += concept.second.mesh.sizeIndices();
			}

			{
				auto drawCommands = std::unique_ptr<OpenGL::Objects::Buffer::DrawElementsIndirectCommand[]>(new OpenGL::Objects::Buffer::DrawElementsIndirectCommand[numGeoms]);

				numGeoms = 0;
				for (auto& concept : this->renderWorld.mConcepts)
				{
					OpenGL::Objects::Buffer::DrawElementsIndirectCommand drawIndirect;
					drawIndirect.baseInstance = 0;
					drawIndirect.baseVertex = concept.second.renderData.meshVBOVertexOffset;
					drawIndirect.count = concept.second.mesh.numIndices();
					drawIndirect.firstIndex = ((unsigned int)concept.second.renderData.meshTriListOffset) / sizeof(unsigned short);
					drawIndirect.instanceCount = 1;

					concept.second.renderData.meshDrawIndirectOffset = sizeof(OpenGL::Objects::Buffer::DrawElementsIndirectCommand) * numGeoms;

					drawCommands[numGeoms] = drawIndirect;
					numGeoms++;
				}

				vbos.vboIndirectDraw.reset();
				vbos.vboIndirectDraw.init(OpenGL::Objects::Buffer::Type::DrawIndirect, drawCommands.get(), sizeof(OpenGL::Objects::Buffer::DrawElementsIndirectCommand) * numGeoms, OpenGL::Objects::Buffer::UsageType::ServerStatic);
			}

			vbos.vaoMesh.reset();
			vbos.vaoMesh.init();

			HorseRadish::OpenGL::glEnableVertexArrayAttrib(vbos.vaoMesh.getId(), 0);
			HorseRadish::OpenGL::glEnableVertexArrayAttrib(vbos.vaoMesh.getId(), 1);
			HorseRadish::OpenGL::glEnableVertexArrayAttrib(vbos.vaoMesh.getId(), 2);
			HorseRadish::OpenGL::glEnableVertexArrayAttrib(vbos.vaoMesh.getId(), 3);

			HorseRadish::OpenGL::glVertexArrayAttribBinding(vbos.vaoMesh.getId(), 0, 0);
			HorseRadish::OpenGL::glVertexArrayAttribBinding(vbos.vaoMesh.getId(), 1, 0);
			HorseRadish::OpenGL::glVertexArrayAttribBinding(vbos.vaoMesh.getId(), 2, 0);
			HorseRadish::OpenGL::glVertexArrayAttribBinding(vbos.vaoMesh.getId(), 3, 0);

			HorseRadish::OpenGL::glVertexArrayAttribFormat(vbos.vaoMesh.getId(), 0, 3, GL_FLOAT, false, offsetof(HorseRadish::Geometry::Mesh::VertexData, pos));
			HorseRadish::OpenGL::glVertexArrayAttribFormat(vbos.vaoMesh.getId(), 1, 2, GL_FLOAT, false, offsetof(HorseRadish::Geometry::Mesh::VertexData, uv));
			HorseRadish::OpenGL::glVertexArrayAttribFormat(vbos.vaoMesh.getId(), 2, 3, GL_UNSIGNED_SHORT, true, offsetof(HorseRadish::Geometry::Mesh::VertexData, normal));
			HorseRadish::OpenGL::glVertexArrayAttribFormat(vbos.vaoMesh.getId(), 3, 4, GL_UNSIGNED_SHORT, true, offsetof(HorseRadish::Geometry::Mesh::VertexData, tangent));

			HorseRadish::OpenGL::glVertexArrayElementBuffer(vbos.vaoMesh.getId(), vbos.vboMeshIndexData.getId());
			HorseRadish::OpenGL::glVertexArrayVertexBuffer(vbos.vaoMesh.getId(), 0, vbos.vboMeshData.getId(), 0, sizeof(HorseRadish::Geometry::Mesh::VertexData));
		}

		void RendererDeferred::loadDiffuse(HorseRadish::IO::FileSystem &fileSystem, const std::string& texFilePath, HorseRadish::OpenGL::Objects::Texture& targetTexture)
		{
			if (texFilePath.empty())
				return;

			auto fileStream = fileSystem.FileRead(texFilePath.c_str());
			if (!fileStream)
				return;
		
			if (HorseRadish::StringUtils::endsWith(texFilePath, ".tga"))
			{
				HorseRadish::Imaging::Image<unsigned char, HorseRadish::Imaging::ImageFormatRGBA> targetImg;

				targetImg = HorseRadish::Imaging::Factory::readTGA(HorseRadish::Streams::StreamReader(*fileStream));

				if (targetImg.empty())
					return;

				targetImg.removeGamma();

				targetTexture.init(HorseRadish::OpenGL::Objects::Texture::Type::Tex2D, HorseRadish::OpenGL::Objects::Texture::StorageType::RGBA_8, targetImg.width(), targetImg.height());
				targetTexture.uploadData(0, 0, 0, targetImg.width(), targetImg.height(), HorseRadish::OpenGL::Objects::Texture::DataFormat::RGBA, HorseRadish::OpenGL::Objects::Texture::DataType::UBYTE, targetImg.data());
				targetTexture.genMipmaps();
			}
			else
			{
				HorseRadish::Imaging::Image<unsigned char, HorseRadish::Imaging::ImageFormatRGB> targetImg;

				if (HorseRadish::StringUtils::endsWith(texFilePath, ".jpg") || HorseRadish::StringUtils::endsWith(texFilePath, ".jpeg"))
					targetImg = HorseRadish::Imaging::Factory::readJPG(HorseRadish::Streams::StreamReader(*fileStream));
				else if (HorseRadish::StringUtils::endsWith(texFilePath, ".png"))
					targetImg = HorseRadish::Imaging::Factory::readPNG(HorseRadish::Streams::StreamReader(*fileStream));

				if (targetImg.empty())
					return;

				targetImg.removeGamma();

				targetTexture.init(HorseRadish::OpenGL::Objects::Texture::Type::Tex2D, HorseRadish::OpenGL::Objects::Texture::StorageType::RGB_8, targetImg.width(), targetImg.height());
				targetTexture.uploadData(0, 0, 0, targetImg.width(), targetImg.height(), HorseRadish::OpenGL::Objects::Texture::DataFormat::RGB, HorseRadish::OpenGL::Objects::Texture::DataType::UBYTE, targetImg.data());
				targetTexture.genMipmaps();
			}
		}

		void RendererDeferred::loadNormal(HorseRadish::IO::FileSystem &fileSystem, const std::string& texFilePath, HorseRadish::OpenGL::Objects::Texture& targetTexture)
		{
			if (texFilePath.empty())
				return;

			auto fileStream = fileSystem.FileRead(texFilePath.c_str());
			if (!fileStream)
				return;

			if (HorseRadish::StringUtils::endsWith(texFilePath, ".tga"))
			{
			}
			else
			{
				HorseRadish::Imaging::Image<unsigned char, HorseRadish::Imaging::ImageFormatRGB> targetImg;

				if (HorseRadish::StringUtils::endsWith(texFilePath, ".png"))
					targetImg = HorseRadish::Imaging::Factory::readPNG(HorseRadish::Streams::StreamReader(*fileStream));
				else if (HorseRadish::StringUtils::endsWith(texFilePath, ".jpg") || HorseRadish::StringUtils::endsWith(texFilePath, ".jpeg"))
					targetImg = HorseRadish::Imaging::Factory::readJPG(HorseRadish::Streams::StreamReader(*fileStream));

				if (targetImg.empty())
					return;

				targetTexture.init(HorseRadish::OpenGL::Objects::Texture::Type::Tex2D, HorseRadish::OpenGL::Objects::Texture::StorageType::RGB_8, targetImg.width(), targetImg.height());
				targetTexture.uploadData(0, 0, 0, targetImg.width(), targetImg.height(), HorseRadish::OpenGL::Objects::Texture::DataFormat::RGB, HorseRadish::OpenGL::Objects::Texture::DataType::UBYTE, targetImg.data());
				targetTexture.genMipmaps();
			}
		}

		void RendererDeferred::loadTextures(HorseRadish::IO::FileSystem &fileSystem)
		{
			for (auto& concept : this->renderWorld.mConcepts)
			{
				concept.second.renderData.texDiffuse.reset();
				concept.second.renderData.texNormal.reset();

				loadDiffuse(fileSystem, concept.second.matDiffusePath, concept.second.renderData.texDiffuse);
				loadNormal(fileSystem, concept.second.matNormalPath, concept.second.renderData.texNormal);
			}
		}

		RendererDeferred::RendererDeferred(const HorseRadish::OpenGL::Objects::Context& glContext, HorseRadish::Render::World& renderWorld)
			: Renderer(glContext), renderWorld(renderWorld)
		{
			//this->texDefaultAlbedo = this->glObjectManager->Create2D(HorseRadish::IO::Path("media\\defaultAlbedo.png"), HorseRadish::OpenGL::Objects::ObjectsManager::TargetType::RGBA32, 0);
			//this->texDefaultNormals = this->glObjectManager->Create2D(HorseRadish::IO::Path("media\\defaultNormals.png"), HorseRadish::OpenGL::Objects::ObjectsManager::TargetType::RGBA32, STEXTURE_NORMAL_MAP_MIPS);
		}

		RendererDeferred::~RendererDeferred()
		{
			this->texDefaultAlbedo.reset();
			this->texDefaultNormals.reset();
		}

		void RendererDeferred::Initialize(const int &renderWidth, const int &renderHeight, HorseRadish::IO::FileSystem * const fileSystem)
		{
			HorseRadish::IO::Path pathShaders;

			this->renderWidth = renderWidth;
			this->renderHeight = renderHeight;
			this->fileSystem = fileSystem;

			pathShaders.Set(HorseRadish::IO::Path::KnownPath::CurrentFolder);
			pathShaders.Combine("shaders");

			this->shadersWatchFolderID = this->fileSystem->WatchChangeCreate(pathShaders.str().c_str(), false, HorseRadish::IO::FileSystem::FileLastWrite);

			//FBOs
			fbos.texDeferredAlbedo.init(HorseRadish::OpenGL::Objects::Texture::Type::TexRectangle, HorseRadish::OpenGL::Objects::Texture::StorageType::RGBA_16F, renderWidth, renderHeight);
			fbos.texDeferredNormals.init(HorseRadish::OpenGL::Objects::Texture::Type::TexRectangle, HorseRadish::OpenGL::Objects::Texture::StorageType::RGBA_16F, renderWidth, renderHeight);
			fbos.texDeferredMiscA.init(HorseRadish::OpenGL::Objects::Texture::Type::TexRectangle, HorseRadish::OpenGL::Objects::Texture::StorageType::RGBA_16F, renderWidth, renderHeight);
			fbos.texDeferredMiscB.init(HorseRadish::OpenGL::Objects::Texture::Type::TexRectangle, HorseRadish::OpenGL::Objects::Texture::StorageType::RGBA_16F, renderWidth, renderHeight);
			fbos.texDeferredZ.init(HorseRadish::OpenGL::Objects::Texture::Type::TexRectangle, HorseRadish::OpenGL::Objects::Texture::StorageType::DEPTH_24, renderWidth, renderHeight);

			fbos.fboDeferredGBuffer.reset();
			fbos.fboDeferredGBuffer.init();
			fbos.fboDeferredGBuffer.attachTColor(fbos.texDeferredAlbedo, 0)
									.attachTColor(fbos.texDeferredNormals, 1)
									.attachTColor(fbos.texDeferredMiscA, 2)
									.attachTColor(fbos.texDeferredMiscB, 3)
									.attachTDepth(fbos.texDeferredZ);
			fbos.fboDeferredGBuffer.isStatusComplete();

			//samplers for the FBOs
			this->fbos.samplerTexs.init();
			this->fbos.samplerTexs.setMinFilter(HorseRadish::OpenGL::Objects::Sampler::FilterType::Point);
			this->fbos.samplerTexs.setMagFilter(HorseRadish::OpenGL::Objects::Sampler::FilterType::Point);
			this->fbos.samplerTexs.setWrap(HorseRadish::OpenGL::Objects::Sampler::WrapType::ClampEdge);

			HorseRadish::OpenGL::glBindFramebuffer(GL_FRAMEBUFFER, 0);

			//shaders
			this->shaders.deferred.vertex.init(HorseRadish::OpenGL::Objects::ShaderProgram::Type::Vertex, fileSystem->readFileAsString("shaders/deferred_gbuffer.vshader"));
			this->shaders.deferred.fragment.init(HorseRadish::OpenGL::Objects::ShaderProgram::Type::Fragment, fileSystem->readFileAsString("shaders/deferred_gbuffer.fshader"));
			this->shaders.deferred.pipeline.init();
			this->shaders.deferred.pipeline.setStage(this->shaders.deferred.vertex);
			this->shaders.deferred.pipeline.setStage(this->shaders.deferred.fragment);

			this->shaders.postprocess.vertex.init(HorseRadish::OpenGL::Objects::ShaderProgram::Type::Vertex, fileSystem->readFileAsString("shaders/ppSimpleColor.vshader"));
			this->shaders.postprocess.fragment.init(HorseRadish::OpenGL::Objects::ShaderProgram::Type::Fragment, fileSystem->readFileAsString("shaders/ppSimpleColor.fshader"));
			this->shaders.postprocess.pipeline.init();
			this->shaders.postprocess.pipeline.setStage(this->shaders.postprocess.vertex);
			this->shaders.postprocess.pipeline.setStage(this->shaders.postprocess.fragment);

			//samplers
			this->samplers.samplerAlbedo.init(HorseRadish::OpenGL::Objects::Sampler::FilterType::Linear, HorseRadish::OpenGL::Objects::Sampler::FilterType::LinearMipPoint);
			this->samplers.samplerAlbedo.setAnisotropy(mGlContext, 8.0f);

			this->samplers.samplerNormals.init(HorseRadish::OpenGL::Objects::Sampler::FilterType::Linear, HorseRadish::OpenGL::Objects::Sampler::FilterType::LinearMipPoint);
			this->samplers.samplerNormals.setAnisotropy(mGlContext, 8.0f);
		}

		/*const HorseRadish::OpenGL::Objects::Texture* RendererDeferred::GetRTTexture(const RendererDeferred::RenderTargetType &renderTargetType) const
		{
			if (renderTargetType == RendererDeferred::RenderTargetType::Albedo)
				return this->fbos.texDeferredAlbedo;
			if (renderTargetType == RendererDeferred::RenderTargetType::Normals)
				return this->fbos.texDeferredNormals;
			if (renderTargetType == RendererDeferred::RenderTargetType::MiscA)
				return this->fbos.texDeferredMiscA;
			if (renderTargetType == RendererDeferred::RenderTargetType::MiscB)
				return this->fbos.texDeferredMiscB;
			return nullptr;
		}*/

		void RendererDeferred::LoadWorld(HorseRadish::IO::FileSystem &fileSystem)
		{
			loadGeometry();

			loadTextures(fileSystem);
		}

		void RendererDeferred::Render(const Tools::Camera& hrCamera, const HorseRadish::OpenGL::Tools::Viewport& hrViewport)
		{
			if (this->fileSystem->WatchChanged(this->shadersWatchFolderID) == true)
				this->fileSystem = this->fileSystem;

			renderGBuffer(hrCamera, hrViewport);
			renderFinal(hrCamera, hrViewport);
		}

	} //Render
} //HorseRadish