#include "rendererDeferred.hpp"

#include "common\ImageFactory.hpp"

#include <algorithm>

namespace HorseRadish
{
	namespace Render
	{
		void RendererDeferred::renderGBuffer(const HorseRadish::OpenGL::Tools::Camera * const hrCamera, const HorseRadish::OpenGL::Tools::Viewport * const hrViewport)
		{
			HorseRadish::Matrix matrixTransformacao, matrixModelView, matrixIdentidade;

			matrixTransformacao.SetIdentidade();
			matrixModelView.SetIdentidade();
			matrixIdentidade.SetIdentidade();

			this->vbos.vaoMesh->Bind();

			matrixTransformacao.Set(hrViewport->getProjCurrent());
			matrixModelView.Set(hrCamera->GetModelView());
			matrixTransformacao *= matrixModelView;

			this->fbos.fboDeferredGBuffer->Bind();

			hrViewport->updateGL();

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

			this->shaders.progDeferredGBuffer->Bind();
			HorseRadish::OpenGL::glUniformMatrix4fv(this->glUniformCache->GetUniformPos(this->shaders.progDeferredGBuffer->glID, "matView"), 1, GL_FALSE, matrixModelView);
			HorseRadish::OpenGL::glUniformMatrix4fv(this->glUniformCache->GetUniformPos(this->shaders.progDeferredGBuffer->glID, "matTrans"), 1, GL_FALSE, matrixTransformacao);
			HorseRadish::OpenGL::glUniform1f(this->glUniformCache->GetUniformPos(this->shaders.progDeferredGBuffer->glID, "farClipPlane"), hrViewport->getZFar());

			this->samplers.samplerNormals->Bind(1);
			this->samplers.samplerAlbedo->Bind(0);

			for (auto& curSurf : this->renderWorld->renderContent.surfaces)
			{
				if (curSurf == nullptr)
					break;

				auto curGeom = curSurf->geometry;

				if (curSurf->texData.lighting.normal != nullptr)
					curSurf->texData.lighting.normal->Bind(1);
				else
					this->texDefaultNormals->Bind(1);

				if (curSurf->texData.lighting.diffuse != nullptr)
					curSurf->texData.lighting.diffuse->Bind(0);
				else
					this->texDefaultAlbedo->Bind(0);

				HorseRadish::OpenGL::glDrawRangeElementsBaseVertex(GL_TRIANGLES, 0, curGeom->mesh.GetNumElements(), curGeom->mesh.GetNumIndices(), curGeom->mesh.GetIndexType(), curGeom->renderTriListOffset, curGeom->renderVBOVertexOffset);

				this->stats.numGlDrawElements++;
				this->stats.numTris += curGeom->mesh.GetNumIndices() / 3;
			}

			HorseRadish::OpenGL::glBindVertexArray(0);
			HorseRadish::OpenGL::glUseProgram(0);
		}

		void RendererDeferred::renderFinal(const HorseRadish::OpenGL::Tools::Camera * const hrCamera, const HorseRadish::OpenGL::Tools::Viewport * const hrViewport)
		{
			int winX = hrViewport->getWidth();
			int winY = hrViewport->getHeight();

			HorseRadish::OpenGL::glBindFramebuffer(GL_FRAMEBUFFER, 0);

			HorseRadish::OpenGL::glDisable(GL_BLEND);
			HorseRadish::OpenGL::glDisable(GL_DEPTH_TEST);
			HorseRadish::OpenGL::glDepthMask(GL_FALSE);

			this->shaders.progPPSimpleColor->Bind();
			HorseRadish::OpenGL::glUniformMatrix4fv(this->glUniformCache->GetUniformPos(this->shaders.progPPSimpleColor->glID, "projectionMatrix"), 1, GL_FALSE, hrViewport->getProj2D());

			this->fbos.samplerTexs->Bind(4);
			this->fbos.samplerTexs->Bind(3);
			this->fbos.samplerTexs->Bind(2);
			this->fbos.samplerTexs->Bind(1);
			this->fbos.samplerTexs->Bind(0);

			this->fbos.texDeferredZ->Bind(4);
			this->fbos.texDeferredMiscB->Bind(3);
			this->fbos.texDeferredMiscA->Bind(2);
			this->fbos.texDeferredNormals->Bind(1);
			this->fbos.texDeferredAlbedo->Bind(0);
			this->glImmediateMode->BeginDraw(HorseRadish::OpenGL::Tools::ImmediateMode::GeometryType::Quads);
			this->glImmediateMode->AddColorF(1.0f);

			this->glImmediateMode->AddTexCoord(0.0f, 0.0f);
			this->glImmediateMode->AddPosition(0.0f, 0.0f);

			this->glImmediateMode->AddTexCoord(winX, 0.0f);
			this->glImmediateMode->AddPosition(winX, 0.0f);

			this->glImmediateMode->AddTexCoord(winX, winY);
			this->glImmediateMode->AddPosition(winX, winY);

			this->glImmediateMode->AddTexCoord(0.0f, winY);
			this->glImmediateMode->AddPosition(0.0f, winY);
			this->glImmediateMode->EndDraw();
		}

		void RendererDeferred::loadGeometry()
		{
			int poolVertex, poolIndex, baseVertexOffset;

			if (this->vbos.vboMeshData != nullptr)
				this->glObjectManager->ObjectDelete(this->vbos.vboMeshData);
			if (this->vbos.vboMeshIndexData != nullptr)
				this->glObjectManager->ObjectDelete(this->vbos.vboMeshIndexData);
			this->vbos.vboMeshData = this->vbos.vboMeshIndexData = nullptr;
			this->vbos.vboMeshSize = this->vbos.vboMeshIndexSize = 0;

			for (const auto& pGeom : this->renderWorld->geometries)
			{
				this->vbos.vboMeshSize += (unsigned int)pGeom.mesh.GetSize();
				if (pGeom.mesh.GetIndexType() == HorseRadish::Geometry::Mesh::Int16)
					this->vbos.vboMeshIndexSize += pGeom.mesh.GetNumIndices()*sizeof(unsigned short);
				else
					this->vbos.vboMeshIndexSize += pGeom.mesh.GetNumIndices()*sizeof(unsigned int);
			}

			this->vbos.vboMeshData = (HorseRadish::OpenGL::Objects::VertexBuffer*)this->glObjectManager->ObjectCreate(HorseRadish::OpenGL::Objects::ObjectsManager::ArrayBuffer);
			this->vbos.vboMeshIndexData = (HorseRadish::OpenGL::Objects::VertexBuffer*)this->glObjectManager->ObjectCreate(HorseRadish::OpenGL::Objects::ObjectsManager::ElementBuffer);
			this->vbos.vboMeshData->LoadBuffer(nullptr, this->vbos.vboMeshSize, HorseRadish::OpenGL::Objects::VertexBuffer::Static);
			this->vbos.vboMeshIndexData->LoadBuffer(nullptr, this->vbos.vboMeshIndexSize, HorseRadish::OpenGL::Objects::VertexBuffer::Static);

			baseVertexOffset = 0;
			poolVertex = poolIndex = 0;
			for (auto& pGeom : this->renderWorld->geometries)
			{
				int dataSize, indexSize;

				pGeom.renderVBOVertexOffset = baseVertexOffset;
				pGeom.renderTriListOffset = (void*)poolIndex;

				dataSize = pGeom.mesh.GetSize();
				this->vbos.vboMeshData->UpdateBuffer(pGeom.mesh.SingleBufferPointer(), dataSize, poolVertex);
				poolVertex += dataSize;

				baseVertexOffset += pGeom.mesh.GetNumElements();

				if (pGeom.mesh.GetIndexType() == HorseRadish::Geometry::Mesh::Int16)
					indexSize = pGeom.mesh.GetNumIndices()*sizeof(unsigned short);
				else
					indexSize = pGeom.mesh.GetNumIndices()*sizeof(unsigned int);

				this->vbos.vboMeshIndexData->UpdateBuffer(pGeom.mesh.GetIndices(), indexSize, poolIndex);
				poolIndex += indexSize;
			}

			if (this->vbos.vaoMesh != nullptr)
				this->glObjectManager->ObjectDelete(this->vbos.vaoMesh);
			this->vbos.vaoMesh = nullptr;

			this->vbos.vaoMesh = (HorseRadish::OpenGL::Objects::VertexArray*)this->glObjectManager->ObjectCreate(HorseRadish::OpenGL::Objects::ObjectsManager::VertexArray);

			HorseRadish::OpenGL::glEnableVertexArrayAttrib(this->vbos.vaoMesh->glID, 0);
			HorseRadish::OpenGL::glEnableVertexArrayAttrib(this->vbos.vaoMesh->glID, 1);
			HorseRadish::OpenGL::glEnableVertexArrayAttrib(this->vbos.vaoMesh->glID, 2);
			HorseRadish::OpenGL::glEnableVertexArrayAttrib(this->vbos.vaoMesh->glID, 3);

			HorseRadish::OpenGL::glVertexArrayAttribBinding(this->vbos.vaoMesh->glID, 0, 0);
			HorseRadish::OpenGL::glVertexArrayAttribBinding(this->vbos.vaoMesh->glID, 1, 0);
			HorseRadish::OpenGL::glVertexArrayAttribBinding(this->vbos.vaoMesh->glID, 2, 0);
			HorseRadish::OpenGL::glVertexArrayAttribBinding(this->vbos.vaoMesh->glID, 3, 0);

			HorseRadish::OpenGL::glVertexArrayAttribFormat(this->vbos.vaoMesh->glID, 0, 3, GL_FLOAT, false, 0);
			HorseRadish::OpenGL::glVertexArrayAttribFormat(this->vbos.vaoMesh->glID, 1, 2, GL_FLOAT, false, 12);
			HorseRadish::OpenGL::glVertexArrayAttribFormat(this->vbos.vaoMesh->glID, 2, 3, GL_FLOAT, false, 20);
			HorseRadish::OpenGL::glVertexArrayAttribFormat(this->vbos.vaoMesh->glID, 4, 4, GL_FLOAT, false, 32);

			HorseRadish::OpenGL::glVertexArrayElementBuffer(this->vbos.vaoMesh->glID, this->vbos.vboMeshIndexData->glID);
			HorseRadish::OpenGL::glVertexArrayVertexBuffer(this->vbos.vaoMesh->glID, 0, this->vbos.vboMeshData->glID, 0, 64);
		}

		TextureSet::Texture* RendererDeferred::findTexType(TextureSet * const texSet, const int texType)
		{
			if ((texSet == nullptr) || texSet->texs.empty())
				return nullptr;

			for (auto& curTex : texSet->texs)
			{
				if (curTex.type == texType)
					return &curTex;
			}

			return nullptr;
		}

		const HorseRadish::OpenGL::Objects::Texture* RendererDeferred::loadDiffuse(HorseRadish::IO::FileSystem &fileSystem, TextureSet::Texture* texture)
		{
			if ((texture == nullptr) || (texture->type != 1))
				return nullptr;

			auto fileStream = fileSystem.FileRead(texture->filePath.GetData());
			if (fileStream == nullptr)
				return nullptr;

			auto curImage = HorseRadish::Imaging::Factory::Read(HorseRadish::Streams::StreamReader(*fileStream));

			delete fileStream;

			if (curImage == nullptr)
				return nullptr;

			auto glTex = this->glTextureManager->Create2D(curImage, HorseRadish::OpenGL::Objects::ObjectsManager::RGBA32, 0);

			delete curImage;
			return glTex;
		}

		const HorseRadish::OpenGL::Objects::Texture* RendererDeferred::loadNormal(HorseRadish::IO::FileSystem &fileSystem, TextureSet::Texture* texture)
		{
			if ((texture == nullptr) || (texture->type != 3))
				return nullptr;

			auto fileStream = fileSystem.FileRead(texture->filePath.GetData());
			if (fileStream == nullptr)
				return nullptr;

			auto curImage = HorseRadish::Imaging::Factory::Read(HorseRadish::Streams::StreamReader(*fileStream));

			delete fileStream;

			if (curImage == nullptr)
				return nullptr;

			auto glTex = this->glTextureManager->Create2D(curImage, HorseRadish::OpenGL::Objects::ObjectsManager::RGBA32, STEXTURE_NORMAL_MAP_MIPS);

			delete curImage;
			return glTex;
		}

		void RendererDeferred::loadTextures(HorseRadish::IO::FileSystem &fileSystem)
		{
			for (auto& curSurf : this->renderWorld->surfacesTotal)
			{
				if (curSurf.texSet == nullptr)
					continue;

				curSurf.texData.lighting.diffuse = loadDiffuse(fileSystem, findTexType(curSurf.texSet, 1));
				curSurf.texData.lighting.normal = loadNormal(fileSystem, findTexType(curSurf.texSet, 3));
			}
		}

		RendererDeferred::RendererDeferred(HorseRadish::OpenGL::Objects::Context * const glContext, HorseRadish::Render::World* const renderWorld)
			: Renderer(glContext), renderWorld(renderWorld)
		{
			this->glObjectManager = new HorseRadish::OpenGL::Objects::ObjectsManager(glContext);
			this->glTextureManager = new HorseRadish::OpenGL::Objects::ObjectsManager(glContext);
			this->glUniformCache = new HorseRadish::OpenGL::Tools::UniformCache();
			this->glImmediateMode = new HorseRadish::OpenGL::Tools::ImmediateMode(102);

			this->texDefaultAlbedo = this->glObjectManager->Create2D(HorseRadish::IO::Path("media\\defaultAlbedo.png"), HorseRadish::OpenGL::Objects::ObjectsManager::RGBA32, 0);
			this->texDefaultNormals = this->glObjectManager->Create2D(HorseRadish::IO::Path("media\\defaultNormals.png"), HorseRadish::OpenGL::Objects::ObjectsManager::RGBA32, STEXTURE_NORMAL_MAP_MIPS);
		}

		RendererDeferred::~RendererDeferred()
		{
			this->glObjectManager->ObjectDelete(this->fbos.fboDeferredGBuffer);
			this->glObjectManager->ObjectDelete(this->fbos.samplerTexs);
			this->glObjectManager->ObjectDelete(this->fbos.texDeferredAlbedo);
			this->glObjectManager->ObjectDelete(this->fbos.texDeferredNormals);
			this->glObjectManager->ObjectDelete(this->fbos.texDeferredMiscA);
			this->glObjectManager->ObjectDelete(this->fbos.texDeferredMiscB);
			this->glObjectManager->ObjectDelete(this->fbos.texDeferredZ);
			this->glObjectManager->ObjectDelete(this->shaders.progDeferredGBuffer);
			this->glObjectManager->ObjectDelete(this->shaders.progMainDebug);
			this->glObjectManager->ObjectDelete(this->shaders.progPerVertexLightDir);
			this->glObjectManager->ObjectDelete(this->shaders.progPPSimpleColor);
			this->glObjectManager->ObjectDelete(this->shaders.renderZPass);
			this->fbos.fboDeferredGBuffer = nullptr;
			this->fbos.samplerTexs = nullptr;
			this->fbos.texDeferredAlbedo = nullptr;
			this->fbos.texDeferredNormals = nullptr;
			this->fbos.texDeferredMiscA = nullptr;
			this->fbos.texDeferredMiscB = nullptr;
			this->fbos.texDeferredZ = nullptr;
			this->shaders.progDeferredGBuffer = nullptr;
			this->shaders.progMainDebug = nullptr;
			this->shaders.progPerVertexLightDir = nullptr;
			this->shaders.progPPSimpleColor = nullptr;
			this->shaders.renderZPass = nullptr;

			this->glObjectManager->ObjectDelete(this->texDefaultAlbedo);
			this->glObjectManager->ObjectDelete(this->texDefaultNormals);

			delete this->glObjectManager;
			delete this->glTextureManager;
			delete this->glUniformCache;
			delete this->glImmediateMode;

			this->glObjectManager = nullptr;
			this->glTextureManager = nullptr;
			this->glUniformCache = nullptr;
			this->glImmediateMode = nullptr;
			this->texDefaultAlbedo = this->texDefaultNormals = nullptr;
		}

		void RendererDeferred::Initialize(const int &renderWidth, const int &renderHeight, HorseRadish::IO::FileSystem * const fileSystem)
		{
			HorseRadish::IO::Path pathShaders;

			this->renderWidth = renderWidth;
			this->renderHeight = renderHeight;
			this->fileSystem = fileSystem;

			pathShaders.Set(HorseRadish::IO::Path::KnownPath::CurrentFolder);
			pathShaders.Combine((HorseRadish::hChar*)"shaders");

			this->shadersWatchFolderID = this->fileSystem->WatchChangeCreate(pathShaders, false, HorseRadish::IO::FileSystem::FileLastWrite);

			//FBOs
			this->fbos.fboDeferredGBuffer = (HorseRadish::OpenGL::Objects::FrameBuffer*)this->glObjectManager->ObjectCreate(HorseRadish::OpenGL::Objects::ObjectsManager::FrameBuffer);
			this->fbos.texDeferredAlbedo = this->glObjectManager->CreateRect(true, nullptr, renderWidth, renderHeight, HorseRadish::OpenGL::Objects::ObjectsManager::RGBA64);
			this->fbos.texDeferredNormals = this->glObjectManager->CreateRect(true, nullptr, renderWidth, renderHeight, HorseRadish::OpenGL::Objects::ObjectsManager::RGBA64);
			this->fbos.texDeferredMiscA = this->glObjectManager->CreateRect(true, nullptr, renderWidth, renderHeight, HorseRadish::OpenGL::Objects::ObjectsManager::RGBA64);
			this->fbos.texDeferredMiscB = this->glObjectManager->CreateRect(true, nullptr, renderWidth, renderHeight, HorseRadish::OpenGL::Objects::ObjectsManager::RGBA64);
			this->fbos.texDeferredZ = this->glObjectManager->CreateRect(true, nullptr, renderWidth, renderHeight, HorseRadish::OpenGL::Objects::ObjectsManager::Depth24);

			this->fbos.fboDeferredGBuffer->AttachTColor(this->fbos.texDeferredAlbedo, 0);
			this->fbos.fboDeferredGBuffer->AttachTColor(this->fbos.texDeferredNormals, 1);
			this->fbos.fboDeferredGBuffer->AttachTColor(this->fbos.texDeferredMiscA, 2);
			this->fbos.fboDeferredGBuffer->AttachTColor(this->fbos.texDeferredMiscB, 3);
			this->fbos.fboDeferredGBuffer->AttachTDepth(this->fbos.texDeferredZ);
			this->fbos.fboDeferredGBuffer->GetStatusComplete();

			//samplers for the FBOs
			this->fbos.samplerTexs = (HorseRadish::OpenGL::Objects::Sampler*)this->glObjectManager->ObjectCreate(HorseRadish::OpenGL::Objects::ObjectsManager::Sampler);
			this->fbos.samplerTexs->SetMinFilter(HorseRadish::OpenGL::Objects::Sampler::Point);
			this->fbos.samplerTexs->SetMagFilter(HorseRadish::OpenGL::Objects::Sampler::Point);
			this->fbos.samplerTexs->SetWrap(HorseRadish::OpenGL::Objects::Sampler::ClampEdge);

			HorseRadish::OpenGL::glBindFramebuffer(GL_FRAMEBUFFER, 0);

			//shaders
			this->shaders.progDeferredGBuffer = (const HorseRadish::OpenGL::Objects::Program*)this->glObjectManager->ObjectCreate(HorseRadish::OpenGL::Objects::ObjectsManager::Program);
			this->glObjectManager->ShadersRead(fileSystem, this->shaders.progDeferredGBuffer, "shaders/deferred_gbuffer.vshader", "shaders/deferred_gbuffer.fshader", nullptr);
			this->shaders.progMainDebug = (const HorseRadish::OpenGL::Objects::Program*)this->glObjectManager->ObjectCreate(HorseRadish::OpenGL::Objects::ObjectsManager::Program);
			this->glObjectManager->ShadersRead(fileSystem, this->shaders.progMainDebug, "shaders/mainDebug.vshader", "shaders/mainDebug.fshader", nullptr);
			this->shaders.progPerVertexLightDir = (const HorseRadish::OpenGL::Objects::Program*)this->glObjectManager->ObjectCreate(HorseRadish::OpenGL::Objects::ObjectsManager::Program);
			this->glObjectManager->ShadersRead(fileSystem, this->shaders.progPerVertexLightDir, "shaders/pvLightDirectional.vshader", "shaders/pvLightDirectional.fshader", nullptr);

			this->shaders.progPPSimpleColor = (const HorseRadish::OpenGL::Objects::Program*)this->glObjectManager->ObjectCreate(HorseRadish::OpenGL::Objects::ObjectsManager::Program);
			this->glObjectManager->ShadersRead(fileSystem, this->shaders.progPPSimpleColor, "shaders/ppSimpleColor.vshader", "shaders/ppSimpleColor.fshader", nullptr);

			this->shaders.renderZPass = (const HorseRadish::OpenGL::Objects::Program*)this->glObjectManager->ObjectCreate(HorseRadish::OpenGL::Objects::ObjectsManager::Program);
			this->glObjectManager->ShadersRead(fileSystem, this->shaders.renderZPass, "shaders/rZPass.vshader", "shaders/rZPass.fshader", nullptr);

			//samplers
			this->samplers.samplerAlbedo = (HorseRadish::OpenGL::Objects::Sampler*)this->glObjectManager->ObjectCreate(HorseRadish::OpenGL::Objects::ObjectsManager::Sampler);
			this->samplers.samplerAlbedo->SetMinFilter(HorseRadish::OpenGL::Objects::Sampler::LinearMipPoint);
			this->samplers.samplerAlbedo->SetMagFilter(HorseRadish::OpenGL::Objects::Sampler::Linear);
			this->glObjectManager->SamplerSetAnisotropy(this->samplers.samplerAlbedo, 8.0f);

			this->samplers.samplerNormals = (HorseRadish::OpenGL::Objects::Sampler*)this->glObjectManager->ObjectCreate(HorseRadish::OpenGL::Objects::ObjectsManager::Sampler);
			this->samplers.samplerNormals->SetMinFilter(HorseRadish::OpenGL::Objects::Sampler::LinearMipPoint);
			this->samplers.samplerNormals->SetMagFilter(HorseRadish::OpenGL::Objects::Sampler::Linear);
			this->glObjectManager->SamplerSetAnisotropy(this->samplers.samplerAlbedo, 8.0f);
		}

		const HorseRadish::OpenGL::Objects::Texture* RendererDeferred::GetRTTexture(const RendererDeferred::RenderTargetType &renderTargetType) const
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
		}

		void RendererDeferred::LoadWorld(HorseRadish::IO::FileSystem &fileSystem)
		{
			loadGeometry();

			loadTextures(fileSystem);
		}

		void RendererDeferred::Render(const HorseRadish::OpenGL::Tools::Camera * const hrCamera, const HorseRadish::OpenGL::Tools::Viewport * const hrViewport)
		{
			if (this->fileSystem->WatchChanged(this->shadersWatchFolderID) == true)
				this->fileSystem = this->fileSystem;

			renderGBuffer(hrCamera, hrViewport);
			renderFinal(hrCamera, hrViewport);
		}

	} //Render
} //HorseRadish