#include "rendererDeferred.hpp"

#include "common/stringUtils.hpp"
#include "common/imageFactory.hpp"

#include <cstddef>
#include <algorithm>

namespace HorseRadish { namespace Render
{
	void RendererDeferred::renderGBuffer(const Tools::Camera& hrCamera, const HorseRadish::OpenGL::Tools::Viewport& hrViewport)
	{
		HorseRadish::Matrix matrixModelView;

		HorseRadish::Matrix matrixTransformacao = hrViewport.getProjection(HorseRadish::OpenGL::Tools::Viewport::ProjectionType::Proj3D);
		matrixModelView.set(hrCamera.modelView());
		matrixTransformacao *= matrixModelView;

		mVBOs.vaoMesh.bind();
		mFBOs.fboDeferredGBuffer.bind();

		GLenum mrt[] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2, GL_COLOR_ATTACHMENT3 };
		HorseRadish::OpenGL::glDrawBuffers(4, mrt);

		HorseRadish::OpenGL::glEnable(GL_DEPTH_TEST);
		HorseRadish::OpenGL::glDepthMask(GL_TRUE);
		HorseRadish::OpenGL::glDepthFunc(GL_LEQUAL);

		HorseRadish::OpenGL::glDisable(GL_SCISSOR_TEST);
		HorseRadish::OpenGL::glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
		HorseRadish::OpenGL::glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);

		HorseRadish::OpenGL::glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		HorseRadish::OpenGL::glProgramUniformMatrix4fv(mShaders.deferred.vertex.getId(), mShaders.deferred.vertex.getUniformLocation("matView"), 1, false, matrixModelView.data());
		HorseRadish::OpenGL::glProgramUniformMatrix4fv(mShaders.deferred.vertex.getId(), mShaders.deferred.vertex.getUniformLocation("matTrans"), 1, false, matrixTransformacao.data());
		HorseRadish::OpenGL::glProgramUniform1f(mShaders.deferred.fragment.getId(), mShaders.deferred.fragment.getUniformLocation("farClipPlane"), hrViewport.zfar());
		HorseRadish::OpenGL::glBindProgramPipeline(mShaders.deferred.pipeline.getId());

		mSamplers.samplerNormals.bind(1);
		mSamplers.samplerAlbedo.bind(0);

		mVBOs.vboIndirectDraw.bind();
		for (auto& curObject : mWorld.mRenderData.objects)
		{			
			auto& concept = mWorld.mConcepts[curObject->conceptName];

			if (concept.renderData.texNormal.isValid())
				concept.renderData.texNormal.bind(1);
			else
				mTexDefaultNormals.bind(1);

			if (concept.renderData.texDiffuse.isValid())
				concept.renderData.texDiffuse.bind(0);
			else
				mTexDefaultAlbedo.bind(0);

			HorseRadish::OpenGL::glMultiDrawElementsIndirect(GL_TRIANGLES, GL_UNSIGNED_SHORT, reinterpret_cast<void*>(concept.renderData.meshDrawIndirectOffset), 1, 0);
		}
		mVBOs.vboIndirectDraw.unbind();
	}

	void RendererDeferred::renderFinal(const HorseRadish::OpenGL::Tools::Viewport& hrViewport)
	{
		auto winX = hrViewport.width();
		auto winY = hrViewport.height();

		HorseRadish::OpenGL::glBindFramebuffer(GL_FRAMEBUFFER, 0);

		HorseRadish::OpenGL::glDisable(GL_DEPTH_TEST);
		HorseRadish::OpenGL::glDepthMask(GL_FALSE);

		HorseRadish::OpenGL::glProgramUniformMatrix4fv(mShaders.postprocess.vertex.getId(), mShaders.postprocess.vertex.getUniformLocation("projectionMatrix"), 1, false, hrViewport.getProjection(HorseRadish::OpenGL::Tools::Viewport::ProjectionType::Proj2D).data());
		HorseRadish::OpenGL::glBindProgramPipeline(mShaders.postprocess.pipeline.getId());

		mFBOs.samplerTexs.bind(4);
		mFBOs.samplerTexs.bind(3);
		mFBOs.samplerTexs.bind(2);
		mFBOs.samplerTexs.bind(1);
		mFBOs.samplerTexs.bind(0);

		mFBOs.texDeferredZ.bind(4);
		mFBOs.texDeferredMiscB.bind(3);
		mFBOs.texDeferredMiscA.bind(2);
		mFBOs.texDeferredNormals.bind(1);
		mFBOs.texDeferredAlbedo.bind(0);

		mGlImmediateMode.beginDraw(HorseRadish::OpenGL::Tools::ImmediateMode::GeometryType::Quads);
			mGlImmediateMode.setColorF(1.0f);
			mGlImmediateMode.addQuadTexCoords(0.0f, 0.0f, winX, winY, false);
		mGlImmediateMode.endDraw();
	}

	void RendererDeferred::loadGeometry()
	{
		mVBOs.vboMeshData.reset();
		mVBOs.vboMeshIndexData.reset();
		mVBOs.vboMeshSize = mVBOs.vboMeshIndexSize = 0;

		for (const auto& concept : mWorld.mConcepts)
		{
			mVBOs.vboMeshSize += concept.second.mesh.sizeVertices();
			mVBOs.vboMeshIndexSize += concept.second.mesh.sizeIndices();
		}

		mVBOs.vboMeshData.init(OpenGL::Objects::Buffer::Type::ArrayBuffer, mVBOs.vboMeshSize, OpenGL::Objects::Buffer::UsageType::PersistentOnlyWrite);
		mVBOs.vboMeshIndexData.init(OpenGL::Objects::Buffer::Type::ElementArrayBuffer, mVBOs.vboMeshIndexSize, OpenGL::Objects::Buffer::UsageType::PersistentOnlyWrite);

		int baseVertexOffset = 0;
		int poolVertex = 0, poolIndex = 0;
		int numGeoms = 0;
		for (auto& concept : mWorld.mConcepts)
		{
			numGeoms++;
			concept.second.renderData.meshVBOVertexOffset = baseVertexOffset;
			concept.second.renderData.meshTriListOffset = (void*)poolIndex;

			mVBOs.vboMeshData.writeData(concept.second.mesh.vertices(), concept.second.mesh.sizeVertices(), poolVertex);
			poolVertex += concept.second.mesh.sizeVertices();

			baseVertexOffset += concept.second.mesh.numVertices();

			mVBOs.vboMeshIndexData.writeData(concept.second.mesh.indices(), concept.second.mesh.sizeIndices(), poolIndex);
			poolIndex += concept.second.mesh.sizeIndices();
		}

		{
			auto drawCommands = std::unique_ptr<OpenGL::Objects::Buffer::DrawElementsIndirectCommand[]>(new OpenGL::Objects::Buffer::DrawElementsIndirectCommand[numGeoms]);

			numGeoms = 0;
			for (auto& concept : mWorld.mConcepts)
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

			mVBOs.vboIndirectDraw.reset();
			mVBOs.vboIndirectDraw.init(OpenGL::Objects::Buffer::Type::DrawIndirect, drawCommands.get(), sizeof(OpenGL::Objects::Buffer::DrawElementsIndirectCommand) * numGeoms, OpenGL::Objects::Buffer::UsageType::ServerStatic);
		}

		mVBOs.vaoMesh.reset();
		mVBOs.vaoMesh.init();

		HorseRadish::OpenGL::glEnableVertexArrayAttrib(mVBOs.vaoMesh.getId(), 0);
		HorseRadish::OpenGL::glEnableVertexArrayAttrib(mVBOs.vaoMesh.getId(), 1);
		HorseRadish::OpenGL::glEnableVertexArrayAttrib(mVBOs.vaoMesh.getId(), 2);
		HorseRadish::OpenGL::glEnableVertexArrayAttrib(mVBOs.vaoMesh.getId(), 3);

		HorseRadish::OpenGL::glVertexArrayAttribBinding(mVBOs.vaoMesh.getId(), 0, 0);
		HorseRadish::OpenGL::glVertexArrayAttribBinding(mVBOs.vaoMesh.getId(), 1, 0);
		HorseRadish::OpenGL::glVertexArrayAttribBinding(mVBOs.vaoMesh.getId(), 2, 0);
		HorseRadish::OpenGL::glVertexArrayAttribBinding(mVBOs.vaoMesh.getId(), 3, 0);

		HorseRadish::OpenGL::glVertexArrayAttribFormat(mVBOs.vaoMesh.getId(), 0, 3, GL_FLOAT, false, offsetof(HorseRadish::Geometry::Mesh::VertexData, pos));
		HorseRadish::OpenGL::glVertexArrayAttribFormat(mVBOs.vaoMesh.getId(), 1, 2, GL_FLOAT, false, offsetof(HorseRadish::Geometry::Mesh::VertexData, uv));
		HorseRadish::OpenGL::glVertexArrayAttribFormat(mVBOs.vaoMesh.getId(), 2, 3, GL_UNSIGNED_SHORT, true, offsetof(HorseRadish::Geometry::Mesh::VertexData, normal));
		HorseRadish::OpenGL::glVertexArrayAttribFormat(mVBOs.vaoMesh.getId(), 3, 4, GL_UNSIGNED_SHORT, true, offsetof(HorseRadish::Geometry::Mesh::VertexData, tangent));

		HorseRadish::OpenGL::glVertexArrayElementBuffer(mVBOs.vaoMesh.getId(), mVBOs.vboMeshIndexData.getId());
		HorseRadish::OpenGL::glVertexArrayVertexBuffer(mVBOs.vaoMesh.getId(), 0, mVBOs.vboMeshData.getId(), 0, sizeof(HorseRadish::Geometry::Mesh::VertexData));
	}

	void RendererDeferred::loadDiffuse(const std::string& texFilePath, HorseRadish::OpenGL::Objects::Texture& targetTexture)
	{
		if (texFilePath.empty())
			return;

		auto fileStream = mFileSystem.fileRead(texFilePath.c_str());
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

	void RendererDeferred::loadNormal(const std::string& texFilePath, HorseRadish::OpenGL::Objects::Texture& targetTexture)
	{
		if (texFilePath.empty())
			return;

		auto fileStream = mFileSystem.fileRead(texFilePath.c_str());
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

	void RendererDeferred::loadTextures()
	{
		for (auto& concept : mWorld.mConcepts)
		{
			concept.second.renderData.texDiffuse.reset();
			concept.second.renderData.texNormal.reset();

			loadDiffuse(concept.second.matDiffusePath, concept.second.renderData.texDiffuse);
			loadNormal(concept.second.matNormalPath, concept.second.renderData.texNormal);
		}
	}

	RendererDeferred::RendererDeferred(const HorseRadish::OpenGL::Objects::Context& glContext, HorseRadish::IO::FileSystem& fileSystem, HorseRadish::Render::World& renderWorld, size_t renderWidth, size_t renderHeight)
		: Renderer(glContext), mWorld(renderWorld), mFileSystem(fileSystem)
	{
		//this->texDefaultAlbedo = this->glObjectManager->Create2D(HorseRadish::IO::Path("media\\defaultAlbedo.png"), HorseRadish::OpenGL::Objects::ObjectsManager::TargetType::RGBA32, 0);
		//this->texDefaultNormals = this->glObjectManager->Create2D(HorseRadish::IO::Path("media\\defaultNormals.png"), HorseRadish::OpenGL::Objects::ObjectsManager::TargetType::RGBA32, STEXTURE_NORMAL_MAP_MIPS);

		HorseRadish::IO::Path pathShaders;
		pathShaders.set(HorseRadish::IO::Path::KnownPath::CurrentFolder);
		pathShaders.combine("shaders");

		mShadersWatchFolderID = mFileSystem.watchChangeCreate(pathShaders.str().c_str(), false, HorseRadish::IO::FileSystem::FileLastWrite);

		//FBOs
		mFBOs.texDeferredAlbedo.init(HorseRadish::OpenGL::Objects::Texture::Type::TexRectangle, HorseRadish::OpenGL::Objects::Texture::StorageType::RGBA_16F, renderWidth, renderHeight);
		mFBOs.texDeferredNormals.init(HorseRadish::OpenGL::Objects::Texture::Type::TexRectangle, HorseRadish::OpenGL::Objects::Texture::StorageType::RGBA_16F, renderWidth, renderHeight);
		mFBOs.texDeferredMiscA.init(HorseRadish::OpenGL::Objects::Texture::Type::TexRectangle, HorseRadish::OpenGL::Objects::Texture::StorageType::RGBA_16F, renderWidth, renderHeight);
		mFBOs.texDeferredMiscB.init(HorseRadish::OpenGL::Objects::Texture::Type::TexRectangle, HorseRadish::OpenGL::Objects::Texture::StorageType::RGBA_16F, renderWidth, renderHeight);
		mFBOs.texDeferredZ.init(HorseRadish::OpenGL::Objects::Texture::Type::TexRectangle, HorseRadish::OpenGL::Objects::Texture::StorageType::DEPTH_24, renderWidth, renderHeight);

		mFBOs.fboDeferredGBuffer.reset();
		mFBOs.fboDeferredGBuffer.init();
		mFBOs.fboDeferredGBuffer.attachTColor(mFBOs.texDeferredAlbedo, 0)
			.attachTColor(mFBOs.texDeferredNormals, 1)
			.attachTColor(mFBOs.texDeferredMiscA, 2)
			.attachTColor(mFBOs.texDeferredMiscB, 3)
			.attachTDepth(mFBOs.texDeferredZ);
		mFBOs.fboDeferredGBuffer.isStatusComplete();

		//samplers for the FBOs
		mFBOs.samplerTexs.init();
		mFBOs.samplerTexs.setMinFilter(HorseRadish::OpenGL::Objects::Sampler::FilterType::Point);
		mFBOs.samplerTexs.setMagFilter(HorseRadish::OpenGL::Objects::Sampler::FilterType::Point);
		mFBOs.samplerTexs.setWrap(HorseRadish::OpenGL::Objects::Sampler::WrapType::ClampEdge);

		//shaders
		mShaders.deferred.vertex.init(HorseRadish::OpenGL::Objects::ShaderProgram::Type::Vertex, mFileSystem.readFileAsString("shaders/deferred_gbuffer.vshader"));
		mShaders.deferred.fragment.init(HorseRadish::OpenGL::Objects::ShaderProgram::Type::Fragment, mFileSystem.readFileAsString("shaders/deferred_gbuffer.fshader"));
		mShaders.deferred.pipeline.init();
		mShaders.deferred.pipeline.setStage(mShaders.deferred.vertex);
		mShaders.deferred.pipeline.setStage(mShaders.deferred.fragment);

		mShaders.postprocess.vertex.init(HorseRadish::OpenGL::Objects::ShaderProgram::Type::Vertex, mFileSystem.readFileAsString("shaders/ppSimpleColor.vshader"));
		mShaders.postprocess.fragment.init(HorseRadish::OpenGL::Objects::ShaderProgram::Type::Fragment, mFileSystem.readFileAsString("shaders/ppSimpleColor.fshader"));
		mShaders.postprocess.pipeline.init();
		mShaders.postprocess.pipeline.setStage(mShaders.postprocess.vertex);
		mShaders.postprocess.pipeline.setStage(mShaders.postprocess.fragment);

		//samplers
		mSamplers.samplerAlbedo.init(HorseRadish::OpenGL::Objects::Sampler::FilterType::Linear, HorseRadish::OpenGL::Objects::Sampler::FilterType::LinearMipPoint);
		mSamplers.samplerAlbedo.setAnisotropy(mGlContext, 8.0f);

		mSamplers.samplerNormals.init(HorseRadish::OpenGL::Objects::Sampler::FilterType::Linear, HorseRadish::OpenGL::Objects::Sampler::FilterType::LinearMipPoint);
		mSamplers.samplerNormals.setAnisotropy(mGlContext, 8.0f);
	}

	RendererDeferred::~RendererDeferred()
	{
		mTexDefaultAlbedo.reset();
		mTexDefaultNormals.reset();
	}
	
	void RendererDeferred::loadWorld()
	{
		loadGeometry();
		loadTextures();
	}

	void RendererDeferred::render(const Tools::Camera& hrCamera, const HorseRadish::OpenGL::Tools::Viewport& hrViewport)
	{
		if (mFileSystem.watchChanged(mShadersWatchFolderID))
		{
			//reload shaders
		}

		renderGBuffer(hrCamera, hrViewport);
	}

	void RendererDeferred::renderComposite(const HorseRadish::OpenGL::Tools::Viewport& hrViewport)
	{
		renderFinal(hrViewport);
	}
} }
