#include "rendererDeferred.hpp"
#include "common\ImageFactory.hpp"

namespace HorseRadish
{

namespace Render
{

//§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§
//§§§§§§   Classe Renderer	§§§§§
//§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§
void RendererDeferred::renderGBuffer(const HorseRadish::OpenGL::Tools::Camera * const hrCamera, const HorseRadish::OpenGL::Tools::Viewport * const hrViewport)
{
	HorseRadish::Matrix matrixTransformacao, matrixModelView, matrixIdentidade;

	//isto dá jeito
	matrixTransformacao.SetIdentidade();
	matrixModelView.SetIdentidade();
	matrixIdentidade.SetIdentidade();

	//preparo a geom
	this->vbos.vaoMesh->Bind();

	//preciso destas matrizes
	matrixTransformacao.Set(hrViewport->getProjCurrent());
	matrixModelView.Set(hrCamera->GetModelView());
	matrixTransformacao *= matrixModelView;

	//*****************
	//quero desenhar para uma textura, logo tenho de coloca o framebuffer adequado
	this->fbos.fboDeferredGBuffer->Bind();

	//colocar viewport e scissor
	hrViewport->updateGL();

	//os buffers para onde vou escrever
	HorseRadish::OpenGL::GLenum mrt[] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2, GL_COLOR_ATTACHMENT3 };
	HorseRadish::OpenGL::glDrawBuffers(4, mrt);

	//preparo o Z
	HorseRadish::OpenGL::glEnable(GL_DEPTH_TEST);
	HorseRadish::OpenGL::glDepthMask(GL_TRUE);
	HorseRadish::OpenGL::glDepthFunc(GL_LEQUAL);

	//só quero passar pelo Z, logo preciso disto tudo
	HorseRadish::OpenGL::glDisable(GL_SCISSOR_TEST);
	HorseRadish::OpenGL::glDisable(GL_BLEND);
	HorseRadish::OpenGL::glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
	HorseRadish::OpenGL::glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);

	//mando limpar tudo
	HorseRadish::OpenGL::glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	//coloco o programa a usar e os uniforms que posso usar
	this->shaders.progDeferredGBuffer->Bind();
	HorseRadish::OpenGL::glUniformMatrix4fv(this->glUniformCache->GetUniformPos(this->shaders.progDeferredGBuffer->glID, "matView"), 1, GL_FALSE, matrixModelView);
	HorseRadish::OpenGL::glUniformMatrix4fv(this->glUniformCache->GetUniformPos(this->shaders.progDeferredGBuffer->glID, "matTrans"), 1, GL_FALSE, matrixTransformacao);
	HorseRadish::OpenGL::glUniform1f(this->glUniformCache->GetUniformPos(this->shaders.progDeferredGBuffer->glID, "farClipPlane"), hrViewport->getZFar());

	//os samplers
	this->samplers.samplerNormals->Bind(1);
	this->samplers.samplerAlbedo->Bind(0);

	//passo por todas as superficies
	for(Surface **surfacesWalker = this->renderWorld->renderContent.surfaces; *surfacesWalker!=nullptr; surfacesWalker++)
	{
		Surface *curSurf;
		Geometry *curGeom;

		//tiro a superficie actual e a geometria que quero
		curSurf = *surfacesWalker;
		curGeom = curSurf->geometry;

		//coloco a textura normal
		if (curSurf->texData.lighting.normal != nullptr)
			curSurf->texData.lighting.normal->Bind(1);
		else
			this->texDefaultNormals->Bind(1);

		//coloco a textura diffusa
		if (curSurf->texData.lighting.diffuse != nullptr)
			curSurf->texData.lighting.diffuse->Bind(0);
		else
			this->texDefaultAlbedo->Bind(0);

		//mando desenhar
		HorseRadish::OpenGL::glDrawRangeElementsBaseVertex(GL_TRIANGLES, 0, curGeom->mesh.GetNumElements(), curGeom->mesh.GetNumIndices(), curGeom->mesh.GetIndexType(), curGeom->renderTriListOffset, curGeom->renderVBOVertexOffset);

		//meto as estatisticas
		this->stats.numGlDrawElements++;
		this->stats.numTris += curGeom->mesh.GetNumIndices()/3;
	}

	//*************
	//finalmente a limpeza
	HorseRadish::OpenGL::glBindVertexArray(0);
	HorseRadish::OpenGL::glUseProgram(0);
}

void RendererDeferred::renderFinal(const HorseRadish::OpenGL::Tools::Camera * const hrCamera, const HorseRadish::OpenGL::Tools::Viewport * const hrViewport)
{
	int winX,winY;

	//isto dá jeito
	winX = hrViewport->getWidth();
	winY = hrViewport->getHeight();

	//a partir de agora quero escrever para o framebuffer nativo
	HorseRadish::OpenGL::glBindFramebuffer(GL_FRAMEBUFFER, 0);

	//nada destas cenas
	HorseRadish::OpenGL::glDisable(GL_BLEND);
	HorseRadish::OpenGL::glDisable(GL_DEPTH_TEST);
	HorseRadish::OpenGL::glDepthMask(GL_FALSE);
	
	//o programa que vou usar e arranjo os seus uniforms
	this->shaders.progPPSimpleColor->Bind();
	HorseRadish::OpenGL::glUniformMatrix4fv(this->glUniformCache->GetUniformPos(this->shaders.progPPSimpleColor->glID, "projectionMatrix"), 1, GL_FALSE, hrViewport->getProj2D());

	//o mesmo sampler é usado em todas as texturas do FBO
	this->fbos.samplerTexs->Bind(4);
	this->fbos.samplerTexs->Bind(3);
	this->fbos.samplerTexs->Bind(2);
	this->fbos.samplerTexs->Bind(1);
	this->fbos.samplerTexs->Bind(0);

	//desenho o quadrado
	this->fbos.texDeferredZ->Bind(4);
	this->fbos.texDeferredMiscB->Bind(3);
	this->fbos.texDeferredMiscA->Bind(2);
	this->fbos.texDeferredNormals->Bind(1);
	this->fbos.texDeferredAlbedo->Bind(0);
	this->glImmediateMode->BeginDraw(HorseRadish::OpenGL::Tools::ImmediateMode::Quads);
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
	int poolVertex,poolIndex, baseVertexOffset;
	Geometry *pGeom;

	//limpo as coisas antes (caso haja alguma coisa)
	if (this->vbos.vboMeshData != nullptr)
		this->glObjectManager->ObjectDelete(this->vbos.vboMeshData);
	if (this->vbos.vboMeshIndexData != nullptr)
		this->glObjectManager->ObjectDelete(this->vbos.vboMeshIndexData);
	this->vbos.vboMeshData = this->vbos.vboMeshIndexData = nullptr;
	this->vbos.vboMeshSize = this->vbos.vboMeshIndexSize = 0;

	//começo por calcular o tamanho dos buffers
	for(int i = 0; i < this->renderWorld->geometries.GetNumElements(); i++)
	{
		//pra ajudar
		pGeom = this->renderWorld->geometries + i;

		//tiro os tamanhos
		this->vbos.vboMeshSize += (unsigned int)pGeom->mesh.GetSize();
		if (pGeom->mesh.GetIndexType() == HorseRadish::Geometry::Mesh::Int16)
			this->vbos.vboMeshIndexSize += pGeom->mesh.GetNumIndices()*sizeof(unsigned short);
		else
			this->vbos.vboMeshIndexSize += pGeom->mesh.GetNumIndices()*sizeof(unsigned int);
	}

	//crio os buffers
	this->vbos.vboMeshData = (HorseRadish::OpenGL::Objects::VertexBuffer*)this->glObjectManager->ObjectCreate(HorseRadish::OpenGL::Objects::ObjectsManager::ArrayBuffer);
	this->vbos.vboMeshIndexData = (HorseRadish::OpenGL::Objects::VertexBuffer*)this->glObjectManager->ObjectCreate(HorseRadish::OpenGL::Objects::ObjectsManager::ElementBuffer);
	this->vbos.vboMeshData->LoadBuffer(nullptr, this->vbos.vboMeshSize, HorseRadish::OpenGL::Objects::VertexBuffer::Static);
	this->vbos.vboMeshIndexData->LoadBuffer(nullptr, this->vbos.vboMeshIndexSize, HorseRadish::OpenGL::Objects::VertexBuffer::Static);

	//agora basta copiar os dados todos para os VBOs
	baseVertexOffset = 0;
	poolVertex = poolIndex = 0;
	for(int i = 0; i < this->renderWorld->geometries.GetNumElements(); i++)
	{
		int dataSize, indexSize;

		//pra ajudar
		pGeom = this->renderWorld->geometries + i;

		//guardo estes ponteiros
		pGeom->renderVBOVertexOffset = baseVertexOffset;
		pGeom->renderTriListOffset = (void*)poolIndex;

		//carrego os dados do vertices
		dataSize = pGeom->mesh.GetSize();
		this->vbos.vboMeshData->UpdateBuffer(pGeom->mesh.SingleBufferPointer(), dataSize, poolVertex);
		poolVertex += dataSize;

		baseVertexOffset+= pGeom->mesh.GetNumElements();

		//tiro o tamanho dos indices
		if (pGeom->mesh.GetIndexType() == HorseRadish::Geometry::Mesh::Int16)
			indexSize = pGeom->mesh.GetNumIndices()*sizeof(unsigned short);
		else
			indexSize = pGeom->mesh.GetNumIndices()*sizeof(unsigned int);

		//carrego os dados do indices
		this->vbos.vboMeshIndexData->UpdateBuffer(pGeom->mesh.GetIndices(), indexSize, poolIndex);
		poolIndex += indexSize;
	}

	//limpo o VAO anterior
	if (this->vbos.vaoMesh != nullptr)
		this->glObjectManager->ObjectDelete(this->vbos.vaoMesh);
	this->vbos.vaoMesh = nullptr;
	
#define VBO_POSITION_OFFSET(start)		((void*)(((intptr_t)start)+0))
#define VBO_TEXCOORD_OFFSET(start)		((void*)(((intptr_t)start)+12))
#define VBO_NORMAL_OFFSET(start)		((void*)(((intptr_t)start)+20))
#define VBO_TANGENT4_OFFSET(start)		((void*)(((intptr_t)start)+32))

	//crio o novo VAO
	this->vbos.vaoMesh = (HorseRadish::OpenGL::Objects::VertexArray*)this->glObjectManager->ObjectCreate(HorseRadish::OpenGL::Objects::ObjectsManager::VertexArray);
	this->vbos.vaoMesh->Bind();
	this->vbos.vboMeshData->Bind();
	this->vbos.vboMeshIndexData->Bind();
	HorseRadish::OpenGL::glEnableVertexAttribArray(0);
	HorseRadish::OpenGL::glEnableVertexAttribArray(1);
	HorseRadish::OpenGL::glEnableVertexAttribArray(2);
	HorseRadish::OpenGL::glEnableVertexAttribArray(3);
	HorseRadish::OpenGL::glVertexAttribPointer(0, 3, GL_FLOAT, false, 64, VBO_POSITION_OFFSET(0));
	HorseRadish::OpenGL::glVertexAttribPointer(1, 2, GL_FLOAT, false, 64, VBO_TEXCOORD_OFFSET(0));
	HorseRadish::OpenGL::glVertexAttribPointer(2, 3, GL_FLOAT, false, 64, VBO_NORMAL_OFFSET(0));
	HorseRadish::OpenGL::glVertexAttribPointer(3, 4, GL_FLOAT, false, 64, VBO_TANGENT4_OFFSET(0));
	HorseRadish::OpenGL::glBindVertexArray(0);
}

TextureSet::Texture* RendererDeferred::findTexType(TextureSet * const texSet, const int texType)
{
	//tenho de ter algo onde procurar
	if ((texSet == nullptr) || (texSet->texs.GetNumElements() <= 0))
		return nullptr;

	//passo por todas à procura
	for(int i=0; i<texSet->texs.GetNumElements(); i++)
	{
		//se achei o que estava a procurar
		if (texSet->texs[i].type == texType)
			return (&texSet->texs[i]);
	}

	//chegando aqui dá barraca
	return nullptr;
}

const HorseRadish::OpenGL::Objects::Texture* RendererDeferred::loadDiffuse(HorseRadish::IO::FileSystem * const fileSystem, TextureSet::Texture* texture)
{
	HorseRadish::Imaging::Image *curImage;
	HorseRadish::Streams::Stream *fileStream;
	const HorseRadish::OpenGL::Objects::Texture *glTex;

	//verificar algumas coisas
	if ((texture == nullptr) || (texture->type != 1))
		return nullptr;

	//tento ler o ficheiro
	fileStream = fileSystem->FileRead(texture->filePath.GetData());
	if (fileStream == nullptr)
		return nullptr;
	
	//tento carregar a imagem
	curImage = HorseRadish::Imaging::Factory::Read(&HorseRadish::Streams::StreamReader(fileStream));

	//apago o filestream
	delete fileStream;

	//se não tenho imagem, posso sair
	if (curImage == nullptr)
		return nullptr;
	
	//carrego a textura
	glTex = this->glTextureManager->Create2D(curImage, HorseRadish::OpenGL::Objects::ObjectsManager::RGBA32, 0);

	//posso destruir a imagem e já tá
	delete curImage;
	return glTex;
}

const HorseRadish::OpenGL::Objects::Texture* RendererDeferred::loadNormal(HorseRadish::IO::FileSystem * const fileSystem, TextureSet::Texture* texture)
{
	HorseRadish::Imaging::Image *curImage;
	HorseRadish::Streams::Stream *fileStream;
	const HorseRadish::OpenGL::Objects::Texture *glTex;

	//verificar algumas coisas
	if ((texture == nullptr) || (texture->type != 3))
		return nullptr;

	//tento ler o ficheiro
	fileStream = fileSystem->FileRead(texture->filePath.GetData());
	if (fileStream == nullptr)
		return nullptr;
	
	//tento carregar a imagem
	curImage = HorseRadish::Imaging::Factory::Read(&HorseRadish::Streams::StreamReader(fileStream));

	//apago o filestream
	delete fileStream;

	//se não tenho imagem, posso sair
	if (curImage == nullptr)
		return nullptr;
	
	//carrego a textura
	glTex = this->glTextureManager->Create2D(curImage, HorseRadish::OpenGL::Objects::ObjectsManager::RGBA32, STEXTURE_NORMAL_MAP_MIPS);

	//posso destruir a imagem e já tá
	delete curImage;
	return glTex;
}

void RendererDeferred::loadTextures(HorseRadish::IO::FileSystem * const fileSystem)
{
	//passo por todas as superficies
	for(int i = 0; i < this->renderWorld->surfacesTotal.GetNumElements(); i++)
	{
		TextureSet *pSet;
		
		//tiro o texture set que esta superficie vai usar
		pSet = this->renderWorld->surfacesTotal[i].texSet;
		if (pSet == nullptr)
			continue;

		//carrego as texturas
		this->renderWorld->surfacesTotal[i].texData.lighting.diffuse = loadDiffuse(fileSystem, findTexType(pSet, 1));
		this->renderWorld->surfacesTotal[i].texData.lighting.normal = loadNormal(fileSystem, findTexType(pSet, 3));
	}
}

RendererDeferred::RendererDeferred(HorseRadish::OpenGL::Objects::Context * const glContext, HorseRadish::Render::World* const renderWorld)
	: Renderer(glContext), renderWorld(renderWorld)
{
	//crio isto tudo
	this->glObjectManager = new HorseRadish::OpenGL::Objects::ObjectsManager(glContext);
	this->glTextureManager = new HorseRadish::OpenGL::Objects::ObjectsManager(glContext);
	this->glUniformCache = new HorseRadish::OpenGL::Tools::UniformCache();
	this->glImmediateMode = new HorseRadish::OpenGL::Tools::ImmediateMode(102);

	//as texturas a usar por omissão
	this->texDefaultAlbedo = this->glObjectManager->Create2D(HorseRadish::IO::Path("media\\defaultAlbedo.png"), HorseRadish::OpenGL::Objects::ObjectsManager::RGBA32, 0);
	this->texDefaultNormals = this->glObjectManager->Create2D(HorseRadish::IO::Path("media\\defaultNormals.png"), HorseRadish::OpenGL::Objects::ObjectsManager::RGBA32, STEXTURE_NORMAL_MAP_MIPS);
}

RendererDeferred::~RendererDeferred()
{
	//termino estes objectos todos
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

	//elimino as texturas
	this->glObjectManager->ObjectDelete(this->texDefaultAlbedo);
	this->glObjectManager->ObjectDelete(this->texDefaultNormals);

	//elimino isto tudo
	delete this->glObjectManager;
	delete this->glTextureManager;
	delete this->glUniformCache;
	delete this->glImmediateMode;

	//limpo tudo
	this->glObjectManager = nullptr;
	this->glTextureManager = nullptr;
	this->glUniformCache = nullptr;
	this->glImmediateMode = nullptr;
	this->texDefaultAlbedo = this->texDefaultNormals = nullptr;
}

void RendererDeferred::Initialize(const int &renderWidth, const int &renderHeight, HorseRadish::IO::FileSystem * const fileSystem)
{
	HorseRadish::IO::Path pathShaders;

	//guardo isto
	this->renderWidth = renderWidth;
	this->renderHeight = renderHeight;
	this->fileSystem = fileSystem;

	//crio o caminho completo para os shaders
	pathShaders.Set(HorseRadish::IO::Path::CurrentFolder);
	pathShaders.Combine((HorseRadish::hChar*)"shaders");

	//crio um watch para estar à escuta de alterações nos shaders
	this->shadersWatchFolderID = this->fileSystem->WatchChangeCreate(pathShaders, false, HorseRadish::IO::FileSystem::FileLastWrite);

	//tenho de criar os FBOs que preciso (e respectivas texturas)
	this->fbos.fboDeferredGBuffer = (HorseRadish::OpenGL::Objects::FrameBuffer*)this->glObjectManager->ObjectCreate(HorseRadish::OpenGL::Objects::ObjectsManager::FrameBuffer);
	this->fbos.texDeferredAlbedo = this->glObjectManager->CreateRect(true, nullptr, renderWidth, renderHeight, HorseRadish::OpenGL::Objects::ObjectsManager::RGBA64);
	this->fbos.texDeferredNormals = this->glObjectManager->CreateRect(true, nullptr, renderWidth, renderHeight, HorseRadish::OpenGL::Objects::ObjectsManager::RGBA64);
	this->fbos.texDeferredMiscA = this->glObjectManager->CreateRect(true, nullptr, renderWidth, renderHeight, HorseRadish::OpenGL::Objects::ObjectsManager::RGBA64);
	this->fbos.texDeferredMiscB = this->glObjectManager->CreateRect(true, nullptr, renderWidth, renderHeight, HorseRadish::OpenGL::Objects::ObjectsManager::RGBA64);
	this->fbos.texDeferredZ = this->glObjectManager->CreateRect(true, nullptr, renderWidth, renderHeight, HorseRadish::OpenGL::Objects::ObjectsManager::Depth24);

	//configuro o FBO principal do render (com cor e depth)
	this->fbos.fboDeferredGBuffer->AttachTColor(this->fbos.texDeferredAlbedo, 0);
	this->fbos.fboDeferredGBuffer->AttachTColor(this->fbos.texDeferredNormals, 1);
	this->fbos.fboDeferredGBuffer->AttachTColor(this->fbos.texDeferredMiscA, 2);
	this->fbos.fboDeferredGBuffer->AttachTColor(this->fbos.texDeferredMiscB, 3);
	this->fbos.fboDeferredGBuffer->AttachTDepth(this->fbos.texDeferredZ);
	this->fbos.fboDeferredGBuffer->GetStatusComplete();

	//crio o sampler a usar nas texturas do FBO
	this->fbos.samplerTexs = (HorseRadish::OpenGL::Objects::Sampler*)this->glObjectManager->ObjectCreate(HorseRadish::OpenGL::Objects::ObjectsManager::Sampler);
	this->fbos.samplerTexs->SetMinFilter(HorseRadish::OpenGL::Objects::Sampler::Point);
	this->fbos.samplerTexs->SetMagFilter(HorseRadish::OpenGL::Objects::Sampler::Point);
	this->fbos.samplerTexs->SetWrap(HorseRadish::OpenGL::Objects::Sampler::ClampEdge);

	//não quero nenhum por defeito
	HorseRadish::OpenGL::glBindFramebuffer(GL_FRAMEBUFFER, 0);

	//**********************
	//************ SHADERS
	//**********************
	//os shaders principais
	this->shaders.progDeferredGBuffer = (const HorseRadish::OpenGL::Objects::Program*)this->glObjectManager->ObjectCreate(HorseRadish::OpenGL::Objects::ObjectsManager::Program);
	this->glObjectManager->ShadersRead(fileSystem, this->shaders.progDeferredGBuffer, "shaders/deferred_gbuffer.vshader", "shaders/deferred_gbuffer.fshader", nullptr);
	this->shaders.progMainDebug = (const HorseRadish::OpenGL::Objects::Program*)this->glObjectManager->ObjectCreate(HorseRadish::OpenGL::Objects::ObjectsManager::Program);
	this->glObjectManager->ShadersRead(fileSystem, this->shaders.progMainDebug, "shaders/mainDebug.vshader", "shaders/mainDebug.fshader", nullptr);
	this->shaders.progPerVertexLightDir = (const HorseRadish::OpenGL::Objects::Program*)this->glObjectManager->ObjectCreate(HorseRadish::OpenGL::Objects::ObjectsManager::Program);
	this->glObjectManager->ShadersRead(fileSystem, this->shaders.progPerVertexLightDir, "shaders/pvLightDirectional.vshader", "shaders/pvLightDirectional.fshader", nullptr);

	//os shaders para fazer o postprocessing
	this->shaders.progPPSimpleColor = (const HorseRadish::OpenGL::Objects::Program*)this->glObjectManager->ObjectCreate(HorseRadish::OpenGL::Objects::ObjectsManager::Program);
	this->glObjectManager->ShadersRead(fileSystem, this->shaders.progPPSimpleColor,"shaders/ppSimpleColor.vshader","shaders/ppSimpleColor.fshader", nullptr);
	//o shader para fazer o zpass
	this->shaders.renderZPass = (const HorseRadish::OpenGL::Objects::Program*)this->glObjectManager->ObjectCreate(HorseRadish::OpenGL::Objects::ObjectsManager::Program);
	this->glObjectManager->ShadersRead(fileSystem, this->shaders.renderZPass,"shaders/rZPass.vshader","shaders/rZPass.fshader", nullptr);

	//**********************
	//************ SAMPLER
	//**********************
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

void RendererDeferred::LoadWorld(HorseRadish::IO::FileSystem * const fileSystem)
{
	//carrego todas as geometrias para os VBOs
	loadGeometry();

	//carrego todas as texturas
	loadTextures(fileSystem);
}

void RendererDeferred::Render(const HorseRadish::OpenGL::Tools::Camera * const hrCamera, const HorseRadish::OpenGL::Tools::Viewport * const hrViewport)
{
	//se os shaders mudaram
	if (this->fileSystem->WatchChanged(this->shadersWatchFolderID) == true)
	{
		this->fileSystem = this->fileSystem;
	}

	//basta chamar isto
	renderGBuffer(hrCamera, hrViewport);
	renderFinal(hrCamera, hrViewport);
}

}//namespace Render
}//namespace HorseRadish