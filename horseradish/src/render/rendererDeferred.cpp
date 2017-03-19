#include "rendererDeferred.hpp"

#include "rendererDebug.hpp"
#include "common/stringUtils.hpp"
#include "common/imageFactory.hpp"

#include <cstddef>
#include <algorithm>

namespace hr { namespace render
{
	void RendererDeferred::renderGBuffer(const tools::Camera& hrCamera, const hr::gl::tools::Viewport& hrViewport)
	{
		hr::Matrix matrixModelView, matrixTransform;

		matrixModelView.set(hrCamera.modelView());
		matrixTransform = hrViewport.getProjection(hr::gl::tools::Viewport::ProjectionType::Proj3D);
		matrixTransform *= matrixModelView;

		mWorld.mRenderData.vaoMesh.bind();
		mFBOs.fboDeferredGBuffer.bind();

		GLenum mrt[] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2, GL_COLOR_ATTACHMENT3 };
		hr::gl::glDrawBuffers(4, mrt);

		hr::gl::glEnable(GL_DEPTH_TEST);
		hr::gl::glDepthMask(GL_TRUE);
		hr::gl::glDepthFunc(GL_LEQUAL);

		hr::gl::glDisable(GL_SCISSOR_TEST);
		hr::gl::glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
		hr::gl::glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);

		hr::gl::glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		hr::gl::glProgramUniformMatrix4fv(mShaders.deferred.vertex.getId(), mShaders.deferred.vertex.getUniformLocation("matView"), 1, false, matrixModelView.data());
		hr::gl::glProgramUniformMatrix4fv(mShaders.deferred.vertex.getId(), mShaders.deferred.vertex.getUniformLocation("matTrans"), 1, false, matrixTransform.data());
		hr::gl::glProgramUniform1f(mShaders.deferred.fragment.getId(), mShaders.deferred.fragment.getUniformLocation("farClipPlane"), hrViewport.zfar());
		hr::gl::glBindProgramPipeline(mShaders.deferred.pipeline.getId());

		mSamplers.samplerNormals.bind(1);
		mSamplers.samplerAlbedo.bind(0);

		mWorld.mRenderData.vboIndirectDraw.bind();
		for (auto& curObject : mWorld.mRenderData.objects)
		{			
			auto& concept = mWorld.mConcepts[curObject->conceptId];

			if (concept.renderData.texNormal.isValid())
				concept.renderData.texNormal.bind(1);
			else
				mTexDefaultNormals.bind(1);

			if (concept.renderData.texDiffuse.isValid())
				concept.renderData.texDiffuse.bind(0);
			else
				mTexDefaultAlbedo.bind(0);

			hr::gl::glMultiDrawElementsIndirect(GL_TRIANGLES, GL_UNSIGNED_SHORT, reinterpret_cast<void*>(concept.renderData.meshDrawIndirectOffset), 1, 0);
		}
		mWorld.mRenderData.vboIndirectDraw.unbind();
	}

	void RendererDeferred::renderFinal(const hr::gl::tools::Viewport& hrViewport)
	{
		auto winX = hrViewport.width();
		auto winY = hrViewport.height();

		hr::gl::glBindFramebuffer(GL_FRAMEBUFFER, 0);

		hr::gl::glDisable(GL_DEPTH_TEST);
		hr::gl::glDepthMask(GL_FALSE);

		hr::gl::glProgramUniformMatrix4fv(mShaders.postprocess.vertex.getId(), mShaders.postprocess.vertex.getUniformLocation("projectionMatrix"), 1, false, hrViewport.getProjection(hr::gl::tools::Viewport::ProjectionType::Proj2D).data());
		hr::gl::glBindProgramPipeline(mShaders.postprocess.pipeline.getId());

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

		mGlImmediateMode.beginDraw(hr::gl::tools::ImmediateMode::GeometryType::Quads);
			mGlImmediateMode.setColorF(1.0f);
			mGlImmediateMode.addQuadTexCoords(0.0f, 0.0f, winX, winY, false);
		mGlImmediateMode.endDraw();
	}

	void RendererDeferred::loadGeometry()
	{
		mWorld.mRenderData.vboMeshData.reset();
		mWorld.mRenderData.vboMeshIndexData.reset();

		size_t vboMeshSize = 0, vboMeshIndexSize = 0;
		for (const auto& concept : mWorld.mConcepts)
		{
			vboMeshSize += sizeof(hr::geom::Mesh::VertexData) * concept.second.geom.numVertices;
			vboMeshIndexSize += sizeof(unsigned short) * concept.second.geom.numIndices;
		}

		mWorld.mRenderData.vboMeshData.init(gl::objects::Buffer::Type::ArrayBuffer, vboMeshSize, gl::objects::Buffer::UsageType::PersistentOnlyWrite);
		mWorld.mRenderData.vboMeshIndexData.init(gl::objects::Buffer::Type::ElementArrayBuffer, vboMeshIndexSize, gl::objects::Buffer::UsageType::PersistentOnlyWrite);

		int baseVertexOffset = 0;
		int poolVertex = 0, poolIndex = 0;
		int numGeoms = 0;
		for (auto& concept : mWorld.mConcepts)
		{
			numGeoms++;
			concept.second.renderData.meshVBOVertexOffset = baseVertexOffset;
			concept.second.renderData.meshTriListOffset = (void*)poolIndex;

			auto sizeVertices = sizeof(hr::geom::Mesh::VertexData) * concept.second.geom.numVertices;
			auto sizeIndices = sizeof(unsigned short) * concept.second.geom.numIndices;

			mWorld.mRenderData.vboMeshData.writeData([this, &concept](void* const destBuffer, size_t requestedDataSize)
			{
				mWorld.mGeomFileStream.seek(hr::streams::Stream::SeekOrigin::Begin, concept.second.geom.fstreamVertexOffset);
				auto bytesRead = mWorld.mGeomFileStream.read(destBuffer, requestedDataSize);
				assert(bytesRead == requestedDataSize);

			}, sizeVertices, poolVertex);
			poolVertex += sizeVertices;

			baseVertexOffset += concept.second.geom.numVertices;

			mWorld.mRenderData.vboMeshIndexData.writeData([this, &concept, &sizeVertices](void* const destBuffer, size_t requestedDataSize)
			{
				mWorld.mGeomFileStream.seek(hr::streams::Stream::SeekOrigin::Begin, concept.second.geom.fstreamIndexOffset);
				auto bytesRead = mWorld.mGeomFileStream.read(destBuffer, requestedDataSize);
				assert(bytesRead == requestedDataSize);

			}, sizeIndices, poolIndex);
			poolIndex += sizeIndices;
		}

		{
			auto drawCommands = std::unique_ptr<gl::objects::Buffer::DrawElementsIndirectCommand[]>(new gl::objects::Buffer::DrawElementsIndirectCommand[numGeoms]);

			numGeoms = 0;
			for (auto& concept : mWorld.mConcepts)
			{
				gl::objects::Buffer::DrawElementsIndirectCommand drawIndirect;
				drawIndirect.baseInstance = 0;
				drawIndirect.baseVertex = concept.second.renderData.meshVBOVertexOffset;
				drawIndirect.count = concept.second.geom.numIndices;
				drawIndirect.firstIndex = ((unsigned int)concept.second.renderData.meshTriListOffset) / sizeof(unsigned short);
				drawIndirect.instanceCount = 1;

				concept.second.renderData.meshDrawIndirectOffset = sizeof(gl::objects::Buffer::DrawElementsIndirectCommand) * numGeoms;

				drawCommands[numGeoms] = drawIndirect;
				numGeoms++;
			}

			mWorld.mRenderData.vboIndirectDraw.reset();
			mWorld.mRenderData.vboIndirectDraw.init(gl::objects::Buffer::Type::DrawIndirect, drawCommands.get(), sizeof(gl::objects::Buffer::DrawElementsIndirectCommand) * numGeoms, gl::objects::Buffer::UsageType::ServerStatic);
		}

		mWorld.mRenderData.vaoMesh.reset();
		mWorld.mRenderData.vaoMesh.init();

		hr::gl::glEnableVertexArrayAttrib(mWorld.mRenderData.vaoMesh.getId(), 0);
		hr::gl::glEnableVertexArrayAttrib(mWorld.mRenderData.vaoMesh.getId(), 1);
		hr::gl::glEnableVertexArrayAttrib(mWorld.mRenderData.vaoMesh.getId(), 2);
		hr::gl::glEnableVertexArrayAttrib(mWorld.mRenderData.vaoMesh.getId(), 3);

		hr::gl::glVertexArrayAttribBinding(mWorld.mRenderData.vaoMesh.getId(), 0, 0);
		hr::gl::glVertexArrayAttribBinding(mWorld.mRenderData.vaoMesh.getId(), 1, 0);
		hr::gl::glVertexArrayAttribBinding(mWorld.mRenderData.vaoMesh.getId(), 2, 0);
		hr::gl::glVertexArrayAttribBinding(mWorld.mRenderData.vaoMesh.getId(), 3, 0);

		hr::gl::glVertexArrayAttribFormat(mWorld.mRenderData.vaoMesh.getId(), 0, 3, GL_FLOAT, false, offsetof(hr::geom::Mesh::VertexData, pos));
		hr::gl::glVertexArrayAttribFormat(mWorld.mRenderData.vaoMesh.getId(), 1, 2, GL_FLOAT, false, offsetof(hr::geom::Mesh::VertexData, uv));
		hr::gl::glVertexArrayAttribFormat(mWorld.mRenderData.vaoMesh.getId(), 2, 3, GL_UNSIGNED_SHORT, true, offsetof(hr::geom::Mesh::VertexData, normal));
		hr::gl::glVertexArrayAttribFormat(mWorld.mRenderData.vaoMesh.getId(), 3, 4, GL_UNSIGNED_SHORT, true, offsetof(hr::geom::Mesh::VertexData, tangent));

		hr::gl::glVertexArrayElementBuffer(mWorld.mRenderData.vaoMesh.getId(), mWorld.mRenderData.vboMeshIndexData.getId());
		hr::gl::glVertexArrayVertexBuffer(mWorld.mRenderData.vaoMesh.getId(), 0, mWorld.mRenderData.vboMeshData.getId(), 0, sizeof(hr::geom::Mesh::VertexData));
	}

	void RendererDeferred::loadDiffuse(const std::string& texFilePath, hr::gl::objects::Texture& targetTexture)
	{
		if (texFilePath.empty())
			return;

		auto fileStream = mFileSystem.fileRead(texFilePath.c_str());
		if (!fileStream)
			return;
	
		if (hr::StringUtils::endsWith(texFilePath, ".tga"))
		{
			hr::imaging::Image<unsigned char, hr::imaging::ImageFormatRGBA> targetImg;

			targetImg = hr::imaging::Factory::readTGA(hr::streams::StreamReader(*fileStream));

			if (targetImg.empty())
				return;

			targetImg.removeGamma();

			targetTexture.init(hr::gl::objects::Texture::Type::Tex2D, hr::gl::objects::Texture::StorageType::RGBA_8, targetImg.width(), targetImg.height());
			targetTexture.uploadData(0, 0, 0, targetImg.width(), targetImg.height(), hr::gl::objects::Texture::DataFormat::RGBA, hr::gl::objects::Texture::DataType::UBYTE, targetImg.data());
			targetTexture.genMipmaps();
		}
		else
		{
			hr::imaging::Image<unsigned char, hr::imaging::ImageFormatRGB> targetImg;

			if (hr::StringUtils::endsWith(texFilePath, ".jpg") || hr::StringUtils::endsWith(texFilePath, ".jpeg"))
				targetImg = hr::imaging::Factory::readJPG(hr::streams::StreamReader(*fileStream));
			else if (hr::StringUtils::endsWith(texFilePath, ".png"))
				targetImg = hr::imaging::Factory::readPNG(hr::streams::StreamReader(*fileStream));

			if (targetImg.empty())
				return;

			targetImg.removeGamma();

			targetTexture.init(hr::gl::objects::Texture::Type::Tex2D, hr::gl::objects::Texture::StorageType::RGB_8, targetImg.width(), targetImg.height());
			targetTexture.uploadData(0, 0, 0, targetImg.width(), targetImg.height(), hr::gl::objects::Texture::DataFormat::RGB, hr::gl::objects::Texture::DataType::UBYTE, targetImg.data());
			targetTexture.genMipmaps();
		}
	}

	void RendererDeferred::loadNormal(const std::string& texFilePath, hr::gl::objects::Texture& targetTexture)
	{
		if (texFilePath.empty())
			return;

		auto fileStream = mFileSystem.fileRead(texFilePath.c_str());
		if (!fileStream)
			return;

		if (hr::StringUtils::endsWith(texFilePath, ".tga"))
		{
		}
		else
		{
			hr::imaging::Image<unsigned char, hr::imaging::ImageFormatRGB> targetImg;

			if (hr::StringUtils::endsWith(texFilePath, ".png"))
				targetImg = hr::imaging::Factory::readPNG(hr::streams::StreamReader(*fileStream));
			else if (hr::StringUtils::endsWith(texFilePath, ".jpg") || hr::StringUtils::endsWith(texFilePath, ".jpeg"))
				targetImg = hr::imaging::Factory::readJPG(hr::streams::StreamReader(*fileStream));

			if (targetImg.empty())
				return;

			targetTexture.init(hr::gl::objects::Texture::Type::Tex2D, hr::gl::objects::Texture::StorageType::RGB_8, targetImg.width(), targetImg.height());
			targetTexture.uploadData(0, 0, 0, targetImg.width(), targetImg.height(), hr::gl::objects::Texture::DataFormat::RGB, hr::gl::objects::Texture::DataType::UBYTE, targetImg.data());
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

	RendererDeferred::RendererDeferred(const hr::gl::objects::Context& glContext, hr::io::FileSystem& fileSystem, hr::render::World& renderWorld, size_t renderWidth, size_t renderHeight)
		: Renderer(glContext)
		, mWorld(renderWorld)
		, mFileSystem(fileSystem)
	{
		//this->texDefaultAlbedo = this->glObjectManager->Create2D(hr::io::Path("media\\defaultAlbedo.png"), hr::gl::objects::ObjectsManager::TargetType::RGBA32, 0);
		//this->texDefaultNormals = this->glObjectManager->Create2D(hr::io::Path("media\\defaultNormals.png"), hr::gl::objects::ObjectsManager::TargetType::RGBA32, STEXTURE_NORMAL_MAP_MIPS);

		hr::io::Path pathShaders;
		pathShaders.set(hr::io::Path::KnownPath::CurrentFolder);
		pathShaders.combine("shaders");

		mShadersWatchFolderID = mFileSystem.watchChangeCreate(pathShaders.str().c_str(), false, hr::io::FileSystem::FileLastWrite);

		//FBOs
		mFBOs.texDeferredAlbedo.init(hr::gl::objects::Texture::Type::TexRectangle, hr::gl::objects::Texture::StorageType::RGBA_16F, renderWidth, renderHeight);
		mFBOs.texDeferredNormals.init(hr::gl::objects::Texture::Type::TexRectangle, hr::gl::objects::Texture::StorageType::RGBA_16F, renderWidth, renderHeight);
		mFBOs.texDeferredMiscA.init(hr::gl::objects::Texture::Type::TexRectangle, hr::gl::objects::Texture::StorageType::RGBA_16F, renderWidth, renderHeight);
		mFBOs.texDeferredMiscB.init(hr::gl::objects::Texture::Type::TexRectangle, hr::gl::objects::Texture::StorageType::RGBA_16F, renderWidth, renderHeight);
		mFBOs.texDeferredZ.init(hr::gl::objects::Texture::Type::TexRectangle, hr::gl::objects::Texture::StorageType::DEPTH_24, renderWidth, renderHeight);

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
		mFBOs.samplerTexs.setMinFilter(hr::gl::objects::Sampler::FilterType::Point);
		mFBOs.samplerTexs.setMagFilter(hr::gl::objects::Sampler::FilterType::Point);
		mFBOs.samplerTexs.setWrap(hr::gl::objects::Sampler::WrapType::ClampEdge);

		//shaders
		mShaders.deferred.vertex.init(hr::gl::objects::ShaderProgram::Type::Vertex, mFileSystem.readFileAsString("shaders/deferred_gbuffer.vshader"));
		mShaders.deferred.fragment.init(hr::gl::objects::ShaderProgram::Type::Fragment, mFileSystem.readFileAsString("shaders/deferred_gbuffer.fshader"));
		mShaders.deferred.pipeline.init();
		mShaders.deferred.pipeline.setStage(mShaders.deferred.vertex);
		mShaders.deferred.pipeline.setStage(mShaders.deferred.fragment);

		mShaders.postprocess.vertex.init(hr::gl::objects::ShaderProgram::Type::Vertex, mFileSystem.readFileAsString("shaders/ppSimpleColor.vshader"));
		mShaders.postprocess.fragment.init(hr::gl::objects::ShaderProgram::Type::Fragment, mFileSystem.readFileAsString("shaders/ppSimpleColor.fshader"));
		mShaders.postprocess.pipeline.init();
		mShaders.postprocess.pipeline.setStage(mShaders.postprocess.vertex);
		mShaders.postprocess.pipeline.setStage(mShaders.postprocess.fragment);

		//samplers
		mSamplers.samplerAlbedo.init(hr::gl::objects::Sampler::FilterType::Linear, hr::gl::objects::Sampler::FilterType::LinearMipPoint);
		mSamplers.samplerAlbedo.setAnisotropy(mGlContext, 8.0f);

		mSamplers.samplerNormals.init(hr::gl::objects::Sampler::FilterType::Linear, hr::gl::objects::Sampler::FilterType::LinearMipPoint);
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

	void RendererDeferred::render(const tools::Camera& hrCamera, const hr::gl::tools::Viewport& hrViewport)
	{
		if (mFileSystem.watchChanged(mShadersWatchFolderID))
		{
			//reload shaders
		}

		renderGBuffer(hrCamera, hrViewport);
	}

	void RendererDeferred::renderDebug(RendererDebug& rendererDebug, const tools::Camera& hrCamera, const hr::gl::tools::Viewport& hrViewport)
	{
		rendererDebug.render(hrCamera, hrViewport);
	}

	void RendererDeferred::renderComposite(const hr::gl::tools::Viewport& hrViewport)
	{
		renderFinal(hrViewport);
	}
} }
