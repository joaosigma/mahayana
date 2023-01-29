#include "rendererMain.hpp"

#include "rendererDebug.hpp"
#include "common/timer.hpp"
#include "common/mesh.hpp"
#include "common/stringUtils.hpp"
#include "common/imageFactory.hpp"
#include "tools/texture.hpp"

#include <cstddef>
#include <algorithm>

namespace hr::render
{
	void RendererMain::passDepth(Scene& scene, const tools::Camera& hrCamera, const hr::gl::tools::Viewport& hrViewport)
	{
		mFBOs.fboZPass.bind();

		hr::gl::glEnable(GL_DEPTH_TEST);
		hr::gl::glDepthMask(GL_TRUE);
		hr::gl::glDepthFunc(GL_GREATER);

		hr::gl::glClear(GL_DEPTH_BUFFER_BIT);

		hr::gl::glBindProgramPipeline(mShaders.forwardPassZ.pipeline.id());

		if (!scene.mRenderData.objects.empty())
		{
			scene.mRenderData.vaoMesh.bind();

			scene.mRenderData.vboIndirectDraw.bind();
			
			for (auto& curObject : scene.mRenderData.objects)
				hr::gl::glMultiDrawElementsIndirect(GL_TRIANGLES, GL_UNSIGNED_SHORT, reinterpret_cast<void*>(curObject->meshDrawIndirectOffset), 1, 0);

			scene.mRenderData.vboIndirectDraw.unbind();
		}
	}

	void RendererMain::passLighting(Scene& scene, const tools::Camera& hrCamera, const hr::gl::tools::Viewport& hrViewport)
	{
		hr::gl::glEnable(GL_DEPTH_TEST);
		hr::gl::glDepthMask(GL_FALSE);
		hr::gl::glDepthFunc(GL_EQUAL); //match against already written Z (on the depth pass)

		hr::gl::glBindProgramPipeline(mShaders.forwardPassLighting.pipeline.id());

		mSamplers.samplerNormals.bind(1);
		mSamplers.samplerAlbedo.bind(0);

		//static
		if (scene.mRenderData.objects.empty())
			return;

		scene.mRenderData.vaoMesh.bind();
		scene.mRenderData.vboIndirectDraw.bind();
		for (auto& curObject : scene.mRenderData.objects)
		{
			if (curObject->textureSetId > 0)
			{
				assert(scene.mTextureSets.contains(curObject->textureSetId));
				const auto& texSet = scene.mTextureSets[curObject->textureSetId];

				if (texSet.texNormal.isValid())
					texSet.texNormal.bind(1);
				else
					mTexDefaultNormals.bind(1);

				if (texSet.texDiffuse.isValid())
					texSet.texDiffuse.bind(0);
				else
					mTexDefaultAlbedo.bind(0);
			}
			else
			{
				mTexDefaultNormals.bind(1);
				mTexDefaultAlbedo.bind(0);
			}

			hr::gl::glMultiDrawElementsIndirect(GL_TRIANGLES, GL_UNSIGNED_SHORT, reinterpret_cast<void*>(curObject->meshDrawIndirectOffset), 1, 0);			
		}
		scene.mRenderData.vboIndirectDraw.unbind();
	}

	void RendererMain::passSky(Scene& scene, const tools::Camera& hrCamera, const hr::gl::tools::Viewport& hrViewport)
	{
		mFBOs.fboForward.bind();
		mFBOs.fboForward.drawBuffers<3>({ GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2 });

		hr::gl::glEnable(GL_DEPTH_TEST);
		hr::gl::glDepthMask(GL_FALSE);
		hr::gl::glDepthFunc(GL_GREATER);

		auto matrixModelView = hrCamera.modelView();
		auto matrixProjection = hrViewport.getProjection(hr::gl::tools::Viewport::ProjectionType::Proj3D);

		hr::gl::glProgramUniformMatrix4fv(mShaders.forwardPassSky.vertex.id(), mShaders.forwardPassSky.vertex.getUniformLocation("modelviewMatrix"), 1, false, matrixModelView.data().data());
		hr::gl::glProgramUniformMatrix4fv(mShaders.forwardPassSky.vertex.id(), mShaders.forwardPassSky.vertex.getUniformLocation("projectionMatrix"), 1, false, matrixProjection.data().data());
		hr::gl::glBindProgramPipeline(mShaders.forwardPassSky.pipeline.id());

		mSamplers.samplerSky.bind(0);
		mTexSky.bind(0);

		mGlImmediateMode.beginDraw(hr::gl::tools::ImmediateMode::GeometryType::Quads);
			mGlImmediateMode.setColorF(1.0f);

			mGlImmediateMode.setTexCoord(0.0f, 0.0f);
			mGlImmediateMode.addPosition(-1.0f, -1.0f, 1.0f);
			
			mGlImmediateMode.setTexCoord(0.0f, 1.0f);
		    mGlImmediateMode.addPosition(-1.0f, 1.0f, 1.0f);

			mGlImmediateMode.setTexCoord(1.0f, 1.0f);
			mGlImmediateMode.addPosition(1.0f, 1.0f, 1.0f);

			mGlImmediateMode.setTexCoord(1.0f, 0.0f);
		    mGlImmediateMode.addPosition(1.0f, -1.0f, 1.0f);

		mGlImmediateMode.endDraw();
	}

	void RendererMain::compositePostProcessing(const hr::gl::tools::Viewport& hrViewport)
	{
		auto winX = hrViewport.width();
		auto winY = hrViewport.height();

		hr::gl::glBindFramebuffer(GL_FRAMEBUFFER, 0);

		hr::gl::glDisable(GL_DEPTH_TEST);
		hr::gl::glDepthMask(GL_FALSE);

		{
			hr::gl::glMemoryBarrier(GL_FRAMEBUFFER_BARRIER_BIT);

			hr::gl::glBindProgramPipeline(mShaders.postprocess.computeSaturatePipeline.id());
			hr::gl::glBindImageTexture(0, mFBOs.texLighting.id(), 0, GL_FALSE, 0, GL_READ_WRITE, GL_RGBA16F);
			hr::gl::glBindImageTexture(1, mFBOs.texAvgLuminance.id(), 0, GL_FALSE, 0, GL_READ_WRITE, GL_R16F);
			hr::gl::glDispatchCompute(mFBOs.texLighting.width() /4, mFBOs.texLighting.height()/4, 1);

			hr::gl::glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);

			mFBOs.texAvgLuminance.genMipmaps();
		}

		hr::gl::glProgramUniformMatrix4fv(mShaders.postprocess.vertex.id(), mShaders.postprocess.vertex.getUniformLocation("projectionMatrix"), 1, false, hrViewport.getProjection(hr::gl::tools::Viewport::ProjectionType::Proj2D).data().data());
		hr::gl::glBindProgramPipeline(mShaders.postprocess.pipeline.id());

		mFBOs.samplerLuminance.bind(3);
		mFBOs.samplerTex.bind(3);
		mFBOs.samplerTex.bind(2);
		mFBOs.samplerTex.bind(1);
		mFBOs.samplerTex.bind(0);

		mFBOs.texAvgLuminance.bind(4);
		mFBOs.texZ.bind(3);
		mFBOs.texSpecular.bind(2);
		mFBOs.texNormals.bind(1);
		mFBOs.texLighting.bind(0);

		mGlImmediateMode.beginDraw(hr::gl::tools::ImmediateMode::GeometryType::Quads);
			mGlImmediateMode.setColorF(1.0f);
			mGlImmediateMode.addQuadTexCoords(0.0f, 0.0f, static_cast<float>(winX), static_cast<float>(winY), true);
		mGlImmediateMode.endDraw();
	}

	void RendererMain::loadGeometry(Scene& scene, const IRenderManager& manager)
	{
		scene.mRenderData.vboMeshData.reset();
		scene.mRenderData.vboMeshIndexData.reset();

		if (manager.numObjects() <= 0)
			return;

		scene.mObjects.reserve(manager.numObjects());

		size_t vboMeshSize = 0, vboMeshIndexSize = 0;
		manager.iterateObjects([&vboMeshSize, &vboMeshIndexSize](IRenderObject& obj)
		{
			vboMeshSize += hr::geom::Mesh<hr::geom::VertexShading, uint16_t>::sizeVertices(obj.numVertices());
			vboMeshIndexSize += hr::geom::Mesh<hr::geom::VertexShading, uint16_t>::sizeIndices(obj.numIndices());
		});

		assert((vboMeshSize > 0) && (vboMeshIndexSize > 0));
		scene.mRenderData.vboMeshData.init(gl::objects::Buffer::Type::ArrayBuffer, vboMeshSize, gl::objects::Buffer::UsageType::PersistentOnlyWrite);
		scene.mRenderData.vboMeshIndexData.init(gl::objects::Buffer::Type::ElementArrayBuffer, vboMeshIndexSize, gl::objects::Buffer::UsageType::PersistentOnlyWrite);

		int baseVertexOffset = 0;
		int poolVertex = 0, poolIndex = 0;
		int numGeoms = 0;
		manager.iterateObjects([&](IRenderObject& obj)
		{
			auto& mesh = scene.mObjects[obj.id()];

			numGeoms++;
			mesh.id = obj.id();
			mesh.bbox = obj.bbox();
			if (obj.materialId() > 0)
				mesh.materialId = obj.materialId();
			if (obj.textureSetId() > 0)
				mesh.textureSetId = obj.textureSetId();

			mesh.meshVBOStartPos = poolVertex;
			mesh.meshVBOVertexOffset = baseVertexOffset;
			mesh.meshTriListOffset = (void*)poolIndex;

			auto sizeVertices = hr::geom::Mesh<hr::geom::VertexShading, uint16_t>::sizeVertices(obj.numVertices());
			auto sizeIndices = hr::geom::Mesh<hr::geom::VertexShading, uint16_t>::sizeIndices(obj.numIndices());
			
			//vertices
			{
				scene.mRenderData.vboMeshData.writeData([&obj](void* const destBuffer, size_t requestedDataSize)
				{
					auto bytesRead = obj.readVertices(destBuffer, requestedDataSize);
					assert(bytesRead == requestedDataSize);

				}, sizeVertices, poolVertex);
				poolVertex += sizeVertices;

				baseVertexOffset += obj.numVertices();
			}

			//indices
			{
				scene.mRenderData.vboMeshIndexData.writeData([&obj](void* const destBuffer, size_t requestedDataSize)
				{
					auto bytesRead = obj.readIndices(destBuffer, requestedDataSize);
					assert(bytesRead == requestedDataSize);

				}, sizeIndices, poolIndex);
				poolIndex += sizeIndices;
			}
		});

		{
			auto drawCommands = std::unique_ptr<gl::objects::Buffer::DrawElementsIndirectCommand[]>(new gl::objects::Buffer::DrawElementsIndirectCommand[numGeoms]);

			numGeoms = 0;
			manager.iterateObjects([&](IRenderObject& obj)
			{
				auto& mesh = scene.mObjects[obj.id()];

				gl::objects::Buffer::DrawElementsIndirectCommand drawIndirect;
				drawIndirect.baseInstance = 0;
				drawIndirect.baseVertex = mesh.meshVBOVertexOffset;
				drawIndirect.count = obj.numIndices();
				drawIndirect.firstIndex = ((GLuint)mesh.meshTriListOffset) / sizeof(uint16_t);
				drawIndirect.instanceCount = 1;

				mesh.meshDrawIndirectOffset = sizeof(gl::objects::Buffer::DrawElementsIndirectCommand) * numGeoms;

				drawCommands[numGeoms] = drawIndirect;
				numGeoms++;
			});

			scene.mRenderData.vboIndirectDraw.reset();
			scene.mRenderData.vboIndirectDraw.init(gl::objects::Buffer::Type::DrawIndirect, drawCommands.get(), sizeof(gl::objects::Buffer::DrawElementsIndirectCommand) * numGeoms, gl::objects::Buffer::UsageType::ServerStatic);
		}

		scene.mRenderData.vaoMesh.reset();
		scene.mRenderData.vaoMesh.init();

		hr::gl::glEnableVertexArrayAttrib(scene.mRenderData.vaoMesh.id(), 0);
		hr::gl::glEnableVertexArrayAttrib(scene.mRenderData.vaoMesh.id(), 1);
		hr::gl::glEnableVertexArrayAttrib(scene.mRenderData.vaoMesh.id(), 2);
		hr::gl::glEnableVertexArrayAttrib(scene.mRenderData.vaoMesh.id(), 3);

		hr::gl::glVertexArrayAttribBinding(scene.mRenderData.vaoMesh.id(), 0, 0);
		hr::gl::glVertexArrayAttribBinding(scene.mRenderData.vaoMesh.id(), 1, 0);
		hr::gl::glVertexArrayAttribBinding(scene.mRenderData.vaoMesh.id(), 2, 0);
		hr::gl::glVertexArrayAttribBinding(scene.mRenderData.vaoMesh.id(), 3, 0);

		hr::gl::glVertexArrayAttribFormat(scene.mRenderData.vaoMesh.id(), 0, 3, GL_FLOAT, false, offsetof(hr::geom::VertexShading, pos));
		hr::gl::glVertexArrayAttribFormat(scene.mRenderData.vaoMesh.id(), 1, 2, GL_UNSIGNED_SHORT, true, offsetof(hr::geom::VertexShading, uv));
		hr::gl::glVertexArrayAttribFormat(scene.mRenderData.vaoMesh.id(), 2, 4, GL_INT_2_10_10_10_REV, true, offsetof(hr::geom::VertexShading, normal));
		hr::gl::glVertexArrayAttribFormat(scene.mRenderData.vaoMesh.id(), 3, 4, GL_INT_2_10_10_10_REV, true, offsetof(hr::geom::VertexShading, tangent));

		hr::gl::glVertexArrayElementBuffer(scene.mRenderData.vaoMesh.id(), scene.mRenderData.vboMeshIndexData.id());
		hr::gl::glVertexArrayVertexBuffer(scene.mRenderData.vaoMesh.id(), 0, scene.mRenderData.vboMeshData.id(), 0, sizeof(hr::geom::VertexShading));
	}

	void RendererMain::loadDiffuse(std::string_view texFilePath, hr::gl::objects::Texture& targetTexture, bool compress)
	{
		if (texFilePath.empty())
			return;

		auto compressedPath = "../" + std::string(texFilePath) + ".hrctex";

		//texture is compressed
		if (mFileSystem.fileExists(compressedPath.c_str()))
		{
			auto fileStream = mFileSystem.fileRead(compressedPath.c_str());
			if (!fileStream)
				return;

			hr::streams::StreamReader reader(*fileStream);
			tools::TextureTools::uploadCompressedDiffuse(reader, targetTexture);
			return;
		}

		//read source image		
		auto fileStream = mFileSystem.fileRead(texFilePath);
		if (!fileStream)
			return;

		if (hr::StringUtils::endsWith(texFilePath, ".hdr"))
		{
			hr::streams::StreamReader reader(*fileStream);
			auto targetImg = hr::imaging::Factory::readHDRI(reader);
			if (targetImg.empty())
				return;

			if (!compress)
			{
				tools::TextureTools::uploadDiffuse(targetImg, targetTexture);
				return;
			}
		}
		else if (hr::StringUtils::endsWith(texFilePath, ".tga"))
		{
			hr::streams::StreamReader reader(*fileStream);
			auto targetImg = hr::imaging::Factory::readTGA(reader);
			if (targetImg.empty())
				return;

			if (!compress)
			{
				tools::TextureTools::uploadDiffuse(targetImg, targetTexture);
				return;
			}

			//compress and store
			{
				hr::streams::FileStream ctexFileStream(compressedPath, false, true);
				hr::streams::StreamWriter stream(ctexFileStream);

				tools::TextureTools::storeCompressedDiffuse(stream, targetImg);
			}

			//upload it
			{
				hr::streams::FileStream ctexFileStream(compressedPath, true, false);
				hr::streams::StreamReader stream(ctexFileStream);

				tools::TextureTools::uploadCompressedDiffuse(stream, targetTexture);
			}
		}
		else
		{
			hr::imaging::Image<unsigned char, hr::imaging::ImageFormatRGB> targetImg;

			hr::streams::StreamReader reader(*fileStream);

			if (hr::StringUtils::endsWith(texFilePath, ".jpg") || hr::StringUtils::endsWith(texFilePath, ".jpeg"))
				targetImg = hr::imaging::Factory::readJPG(reader);
			else if (hr::StringUtils::endsWith(texFilePath, ".png"))
				targetImg = hr::imaging::Factory::readPNG(reader);

			if (targetImg.empty())
				return;

			if (!compress)
			{
				tools::TextureTools::uploadDiffuse(targetImg, targetTexture);
				return;
			}

			//compress and store
			{
				hr::streams::FileStream ctexFileStream(compressedPath, false, true);
				hr::streams::StreamWriter stream(ctexFileStream);

				tools::TextureTools::storeCompressedDiffuse(stream, targetImg);
			}

			//upload it
			{
				hr::streams::FileStream ctexFileStream(compressedPath, true, false);
				hr::streams::StreamReader stream(ctexFileStream);

				tools::TextureTools::uploadCompressedDiffuse(stream, targetTexture);
			}
		}
	}

	void RendererMain::loadNormal(std::string_view texFilePath, hr::gl::objects::Texture& targetTexture, bool compress)
	{
		if (texFilePath.empty())
			return;

		auto compressedPath = "../" + std::string(texFilePath) + ".hrctex";

		//texture is compressed
		if (mFileSystem.fileExists(compressedPath.c_str()))
		{
			auto fileStream = mFileSystem.fileRead(compressedPath.c_str());
			if (!fileStream)
				return;

			hr::streams::StreamReader reader(*fileStream);
			tools::TextureTools::uploadCompressedNormal(reader, targetTexture);
			return;
		}

		//read source image		
		auto fileStream = mFileSystem.fileRead(texFilePath);
		if (!fileStream)
			return;		

		hr::imaging::Image<unsigned char, hr::imaging::ImageFormatRGB> targetImg;

		hr::streams::StreamReader reader(*fileStream);

		if (hr::StringUtils::endsWith(texFilePath, ".jpg") || hr::StringUtils::endsWith(texFilePath, ".jpeg"))
			targetImg = hr::imaging::Factory::readJPG(reader);
		else if (hr::StringUtils::endsWith(texFilePath, ".png"))
			targetImg = hr::imaging::Factory::readPNG(reader);

		if (targetImg.empty())
			return;

		if (!compress)
		{
			tools::TextureTools::uploadNormal(targetImg, targetTexture);
			return;
		}

		//compress and store
		{
			hr::streams::FileStream ctexFileStream(compressedPath, false, true);
			hr::streams::StreamWriter stream(ctexFileStream);

			tools::TextureTools::storeCompressedNormal(stream, targetImg);
		}

		//upload it
		{
			hr::streams::FileStream ctexFileStream(compressedPath, true, false);
			hr::streams::StreamReader stream(ctexFileStream);

			tools::TextureTools::uploadCompressedNormal(stream, targetTexture);
		}
	}

	void RendererMain::loadMaterialsTextures(Scene& scene, const IRenderManager& manager)
	{
		manager.iterateMaterials([this, &scene](IRenderMaterial& material)
		{
		});

		manager.iterateTextureSets([this, &scene](IRenderTextureSet& textureSet)
		{
			auto& texSet = scene.mTextureSets[textureSet.id()];
			
			texSet.texDiffuse.reset();
			loadDiffuse(textureSet.diffusePath(), texSet.texDiffuse, true);

			texSet.texNormal.reset();
			loadNormal(textureSet.normalPath(), texSet.texNormal, true);
		});
	}

	RendererMain::RendererMain(const hr::gl::objects::Context& glContext, hr::io::FileSystem& fileSystem, size_t renderWidth, size_t renderHeight)
		: mFileSystem(fileSystem)
		, mGlContext(glContext)
	{
		loadDiffuse(R"(media\skies\archesPineTree.hdr)", mTexSky, false);

		//loadDiffuse(R"(media\default_albedo.png)", mTexDefaultAlbedo, true);
		//loadNormal(R"(media\default_normal.png)", mTexDefaultNormals, true);
		loadDiffuse(R"(texs\color.white.png)", mTexDefaultAlbedo, true);
		loadNormal(R"(texs\flat.normal.png)", mTexDefaultNormals, true);
	
		{
			auto pathShaders = std::filesystem::current_path();
			pathShaders /= "shaders";

			mShadersWatchFolderID = mFileSystem.watchChangeCreate(pathShaders, false, hr::io::FileSystem::FileLastWrite);
		}

		//FBOs
		mFBOs.texLighting.init(hr::gl::objects::Texture::Type::TexRectangle, hr::gl::objects::Texture::StorageType::RGBA_16F, renderWidth, renderHeight);
		mFBOs.texNormals.init(hr::gl::objects::Texture::Type::TexRectangle, hr::gl::objects::Texture::StorageType::RGBA_16F, renderWidth, renderHeight);
		mFBOs.texSpecular.init(hr::gl::objects::Texture::Type::TexRectangle, hr::gl::objects::Texture::StorageType::RGBA_8, renderWidth, renderHeight);
		mFBOs.texZ.init(hr::gl::objects::Texture::Type::TexRectangle, hr::gl::objects::Texture::StorageType::DEPTH_32F, renderWidth, renderHeight);
		mFBOs.texAvgLuminance.init(hr::gl::objects::Texture::Type::Tex2D, hr::gl::objects::Texture::StorageType::R_16F, renderWidth, renderHeight);

		mFBOs.fboZPass.reset();
		mFBOs.fboZPass.init();
		mFBOs.fboZPass
			.attachTDepth(mFBOs.texZ);
		mFBOs.fboZPass.isStatusComplete();

		mFBOs.fboForward.reset();
		mFBOs.fboForward.init();
		mFBOs.fboForward
			.attachTColor(mFBOs.texLighting, 0)
			.attachTColor(mFBOs.texNormals, 1)
			.attachTColor(mFBOs.texSpecular, 2)
			.attachTDepth(mFBOs.texZ);
		mFBOs.fboForward.isStatusComplete();

		//samplers for the FBOs
		mFBOs.samplerTex.init();
		mFBOs.samplerTex.setMagFilter(hr::gl::objects::Sampler::FilterType::Point);
		mFBOs.samplerTex.setMinFilter(hr::gl::objects::Sampler::FilterType::Point);
		mFBOs.samplerTex.setWrap(hr::gl::objects::Sampler::WrapType::ClampEdge);

		mFBOs.samplerLuminance.init();
		mFBOs.samplerLuminance.setMagFilter(hr::gl::objects::Sampler::FilterType::Linear);
		mFBOs.samplerLuminance.setMinFilter(hr::gl::objects::Sampler::FilterType::LinearMipLinear);
		mFBOs.samplerLuminance.setWrap(hr::gl::objects::Sampler::WrapType::ClampEdge);

		//shaders
		mShaders.forwardPassZ.vertex.init(hr::gl::objects::ShaderProgram::Type::Vertex, mFileSystem.readFileAsString("shaders/rPassZ.vshader"));
		mShaders.forwardPassZ.fragment.init(hr::gl::objects::ShaderProgram::Type::Fragment, mFileSystem.readFileAsString("shaders/rPassZ.fshader"));
		mShaders.forwardPassZ.pipeline.init();
		mShaders.forwardPassZ.pipeline.setStage(mShaders.forwardPassZ.vertex);
		mShaders.forwardPassZ.pipeline.setStage(mShaders.forwardPassZ.fragment);

		mShaders.forwardPassLighting.vertex.init(hr::gl::objects::ShaderProgram::Type::Vertex, mFileSystem.readFileAsString("shaders/rPassLighting.vshader"));
		mShaders.forwardPassLighting.fragment.init(hr::gl::objects::ShaderProgram::Type::Fragment, mFileSystem.readFileAsString("shaders/rPassLighting.fshader"));
		mShaders.forwardPassLighting.pipeline.init();
		mShaders.forwardPassLighting.pipeline.setStage(mShaders.forwardPassLighting.vertex);
		mShaders.forwardPassLighting.pipeline.setStage(mShaders.forwardPassLighting.fragment);

		mShaders.forwardPassSky.vertex.init(hr::gl::objects::ShaderProgram::Type::Vertex, mFileSystem.readFileAsString("shaders/rPassSky.vshader"));
		mShaders.forwardPassSky.fragment.init(hr::gl::objects::ShaderProgram::Type::Fragment, mFileSystem.readFileAsString("shaders/rPassSky.fshader"));
		mShaders.forwardPassSky.pipeline.init();
		mShaders.forwardPassSky.pipeline.setStage(mShaders.forwardPassSky.vertex);
		mShaders.forwardPassSky.pipeline.setStage(mShaders.forwardPassSky.fragment);

		mShaders.postprocess.vertex.init(hr::gl::objects::ShaderProgram::Type::Vertex, mFileSystem.readFileAsString("shaders/rCompositePP.vshader"));
		mShaders.postprocess.fragment.init(hr::gl::objects::ShaderProgram::Type::Fragment, mFileSystem.readFileAsString("shaders/rCompositePP.fshader"));
		mShaders.postprocess.pipeline.init();
		mShaders.postprocess.pipeline.setStage(mShaders.postprocess.vertex);
		mShaders.postprocess.pipeline.setStage(mShaders.postprocess.fragment);

		mShaders.postprocess.computeSaturateShader.init(hr::gl::objects::ShaderProgram::Type::Compute, mFileSystem.readFileAsString("shaders/rSaturatePP.cshader"));
		mShaders.postprocess.computeSaturatePipeline.init();
		mShaders.postprocess.computeSaturatePipeline.setStage(mShaders.postprocess.computeSaturateShader);

		//buffers
		mShaders.forwardPassBuffers.uniform.init(hr::gl::objects::Buffer::Type::UniformBuffer, sizeof(Shaders::UniformLayout), hr::gl::objects::Buffer::UsageType::PersistentOnlyWrite);
		mShaders.forwardPassBuffers.storage.init(hr::gl::objects::Buffer::Type::ShaderStorage, sizeof(Shaders::LightLayout) * Shaders::LightLayoutMaxElements, hr::gl::objects::Buffer::UsageType::PersistentOnlyWrite);

		//samplers
		mSamplers.samplerSky.init(hr::gl::objects::Sampler::FilterType::Linear, hr::gl::objects::Sampler::FilterType::Linear);
		mSamplers.samplerSky.setWrap(hr::gl::objects::Sampler::WrapType::Repeat);
		mSamplers.samplerSky.setAnisotropy(mGlContext, 8.0f);
		
		mSamplers.samplerAlbedo.init(hr::gl::objects::Sampler::FilterType::Linear, hr::gl::objects::Sampler::FilterType::LinearMipPoint);
		mSamplers.samplerAlbedo.setAnisotropy(mGlContext, 8.0f);

		mSamplers.samplerNormals.init(hr::gl::objects::Sampler::FilterType::Linear, hr::gl::objects::Sampler::FilterType::LinearMipPoint);
		mSamplers.samplerNormals.setAnisotropy(mGlContext, 8.0f);
	}

	RendererMain::~RendererMain()
	{
		mTexDefaultAlbedo.reset();
		mTexDefaultNormals.reset();
	}

	RendererMain::SceneId RendererMain::loadScene(const IRenderManager& manager)
	{
		if (manager.numObjects() <= 0)
			return 0;

		auto id = mGenSceneIds++;
		auto& scene = mScenes[id];

		loadGeometry(scene, manager);
		loadMaterialsTextures(scene, manager);

		return id;
	}

	void RendererMain::unloadScene(SceneId sceneId)
	{

	}
	
	void RendererMain::render(const tools::Camera& hrCamera, const hr::gl::tools::Viewport& hrViewport)
	{
		if (mScenes.empty())
			return;

		if (mFileSystem.watchChanged(mShadersWatchFolderID))
		{
			//reload shaders
		}
				
		{
			mShaders.forwardPassBuffers.fence.wait();

			//uniform buffer common to every pass is prepared/set here
			static uint32_t numLights = 1;
			{
				auto matrixMView = hrCamera.modelView();
				auto matrixProj = hrViewport.getProjection(hr::gl::tools::Viewport::ProjectionType::Proj3D);

				Shaders::UniformLayout uniformData;
				matrixProj.write(uniformData.matProj);
				matrixMView.write(uniformData.matMView);
				uniformData.numLights = numLights;

				mShaders.forwardPassBuffers.uniform.writeData(&uniformData, sizeof(Shaders::UniformLayout), 0);
				hr::gl::glBindBufferBase(GL_UNIFORM_BUFFER, 1, mShaders.forwardPassBuffers.uniform.id());
			}

			//shader storage buffer common to every pass is prepared/set here
			{
				static bool done = false;
				static auto animLightIndex = numLights / 2;
				static hr::Timer animTimer;
				static Shaders::LightLayout animLight;

				if (!done)
				{
					done = true;

					std::array<Shaders::LightLayout, Shaders::LightLayoutMaxElements> storageData;
					for (uint32_t i = 0; i < numLights; i++)
					{
						Vector3f dir(mRand.nextDouble(-20.0, 20.0), mRand.nextDouble(10.0, 50.0), mRand.nextDouble(10.0, 50.0));
						dir.normalize();
						dir.write(storageData[i].dir);
						storageData[i].dir[3] = 0.0f;
						
						Vector3f diffuse(mRand.nextDouble(0.5, 1.0), mRand.nextDouble(0.5, 1.0), mRand.nextDouble(0.5, 1.0));
						diffuse.write(storageData[i].diffuse);
						storageData[i].diffuse[3] = 1.0f;
					}

					animLight = storageData[animLightIndex];
					animTimer.reStart();

					mShaders.forwardPassBuffers.storage.writeData(storageData.data(), sizeof(Shaders::LightLayout) * numLights, 0);
					hr::gl::glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, mShaders.forwardPassBuffers.storage.id());
				}
				else
				{
					auto t = animTimer.getTimeS() / 20.0;
					if (t > 1.0)
					{
						t = 1.0;
						animTimer.reStart();
					}
					else if (t > 0.5)
						t = 2.0 - (t * 2.0);
					else
						t *= 2.0;

					auto vec = Vector3f::calcLerp(
						Vector3f{ animLight.dir[0] - 10.0f, animLight.dir[1], animLight.dir[2] },
						Vector3f{ animLight.dir[0] + 10.0f, animLight.dir[1], animLight.dir[2] },
						t);

					Shaders::LightLayout newLight{ animLight };
					vec.write(newLight.dir);
					newLight.dir[3] = 1.0f;

					mShaders.forwardPassBuffers.storage.writeData(&newLight, sizeof(Shaders::LightLayout), sizeof(Shaders::LightLayout) * animLightIndex);
				}
			}

			auto& scene = mScenes.begin()->second;

			passDepth(scene, hrCamera, hrViewport);
			passSky(scene, hrCamera, hrViewport);
			passLighting(scene, hrCamera, hrViewport);

			mShaders.forwardPassBuffers.fence.place();
		}
	}

	void RendererMain::renderDebug(RendererDebug& rendererDebug, const hr::render::World& world, const tools::Camera& hrCamera, const hr::gl::tools::Viewport& hrViewport)
	{
		rendererDebug.render(*this, world, hrCamera, hrViewport);
	}

	void RendererMain::renderComposite(const hr::gl::tools::Viewport& hrViewport)
	{
		compositePostProcessing(hrViewport);
	}

	void RendererMain::updateVertexData(SceneId sceneId, const IRenderManager& manager)
	{
		auto sceneIt = mScenes.find(sceneId);
		if (sceneIt == mScenes.end())
			return;

		auto& scene = sceneIt->second;

		manager.iterateObjects([&scene](IRenderObject& obj)
		{
			auto& mesh = scene.mObjects[obj.id()];
			mesh.bbox = obj.bbox();
			
			auto sizeVertices = hr::geom::Mesh<hr::geom::VertexShading, uint16_t>::sizeVertices(obj.numVertices());

			scene.mRenderData.vboMeshData.writeData([&obj](void* const destBuffer, size_t requestedDataSize)
			{
				auto bytesRead = obj.readVertices(destBuffer, requestedDataSize);
				assert(bytesRead == requestedDataSize);

			}, sizeVertices, mesh.meshVBOStartPos);
		});
	}

	void RendererMain::prepareNextFrame(SceneId sceneId, const std::vector<IRenderObject::ObjectId>& objects)
	{
		auto sceneIt = mScenes.find(sceneId);
		if (sceneIt == mScenes.end())
			return;

		auto& scene = sceneIt->second;

		scene.mRenderData.objects.clear();

		for (auto&& objectId : objects)
		{
			auto objectIt = scene.mObjects.find(objectId);
			if (objectIt != scene.mObjects.end())
				scene.mRenderData.objects.push_back(&objectIt->second);
		}
	}
}
