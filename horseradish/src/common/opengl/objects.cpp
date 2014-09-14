#include "objects.hpp"
#include "openGLext.hpp"
#include "common\math.hpp"

namespace HorseRadish
{

namespace OpenGL
{

namespace Objects
{

//§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§
//§§§§§§   Classe Sampler	§§§§§
//§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§

//§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§
//§§§§§§   Classe Texture	§§§§§
//§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§
int Texture::CalculateNumMipMaps(const int width)
{
	//basta fazer esta conta	
	return HorseRadish::Math::ftoi(HorseRadish::Math::floor(HorseRadish::Math::iLog2(width))) + 1;
}

int Texture::CalculateNumMipMaps(const int width, const int height)
{
	//basta fazer esta conta	
	return HorseRadish::Math::ftoi(HorseRadish::Math::floor(HorseRadish::Math::iLog2(HorseRadish::Math::iMax(width, height)))) + 1;
}

int Texture::CalculateNumMipMaps(const int width, const int height, const int depth)
{
	//basta fazer esta conta	
	return HorseRadish::Math::ftoi(HorseRadish::Math::floor(HorseRadish::Math::iLog2(HorseRadish::Math::iMax(HorseRadish::Math::iMax(width, height), depth)))) + 1;
}

//§§§§§§§§§§§§§§§§§§§§§§§§§§§
//§§§§§§   Classe Query	§§§§§
//§§§§§§§§§§§§§§§§§§§§§§§§§§§

//§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§
//§§§§§§   Classe VertexBuffer	§§§§§
//§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§
bool VertexBuffer::LoadBuffer(const void * const data, const int &dataSize, const UsageType &usage) const
{
	if (dataSize <= 0)
		return false;

	GLenum glUsage = 0;
	switch (usage)
	{
	case VertexBuffer::Stream:
		glUsage = GL_STREAM_DRAW;
		break;
	case VertexBuffer::Static:
		glUsage = GL_STATIC_DRAW;
		break;
	case VertexBuffer::Dynamic:
		glUsage = GL_DYNAMIC_DRAW;
		break;
	default:
		return false;
	}

	HorseRadish::OpenGL::glNamedBufferData(this->glID, dataSize, data, glUsage);
	return true;
}

bool VertexBuffer::LoadMeshData(HorseRadish::Geometry::Mesh * const mesh, const UsageType &usage) const
{
	const void *data;
	int dataSize;

	if (mesh == nullptr)
		return false;

	if (this->glTarget != GL_ARRAY_BUFFER)
		return false;

	data = (const void*)mesh->SingleBufferPointer();
	dataSize = mesh->GetSize();
	if ((data == nullptr) || (dataSize <= 0))
		return false;

	GLenum glUsage = 0;
	switch (usage)
	{
	case VertexBuffer::Stream:
		glUsage = GL_STREAM_DRAW;
		break;
	case VertexBuffer::Static:
		glUsage = GL_STATIC_DRAW;
		break;
	case VertexBuffer::Dynamic:
		glUsage = GL_DYNAMIC_DRAW;
		break;
	default:
		return false;
	}

	HorseRadish::OpenGL::glNamedBufferData(this->glID, dataSize, data, glUsage);
	return true;
}

bool VertexBuffer::LoadMeshIndex(HorseRadish::Geometry::Mesh * const mesh, const UsageType &usage) const
{
	const void *data;
	int dataSize;

	if (mesh==nullptr)
		return false;

	if (this->glTarget != GL_ELEMENT_ARRAY_BUFFER)
		return false;

	data = (const void*)mesh->GetIndices();
	dataSize = 0;
	if (mesh->GetIndexType() == HorseRadish::Geometry::Mesh::Int16)
		dataSize = sizeof(unsigned short) * mesh->GetNumIndices();
	if (mesh->GetIndexType() == HorseRadish::Geometry::Mesh::Int32)
		dataSize = sizeof(unsigned int) * mesh->GetNumIndices();
	if ((data == nullptr) || (dataSize <= 0))
		return false;

	GLenum glUsage = 0;
	switch (usage)
	{
	case VertexBuffer::Stream:
		glUsage = GL_STREAM_DRAW;
		break;
	case VertexBuffer::Static:
		glUsage = GL_STATIC_DRAW;
		break;
	case VertexBuffer::Dynamic:
		glUsage = GL_DYNAMIC_DRAW;
		break;
	default:
		return false;
	}

	HorseRadish::OpenGL::glNamedBufferData(this->glID, dataSize, data, glUsage);
	return true;
}

bool VertexBuffer::UpdateBuffer(const void * const data, const int &dataSize, const int &startOffset) const
{
	if ((data == nullptr) || (dataSize <= 0) || (startOffset < 0))
		return false;

	HorseRadish::OpenGL::glNamedBufferSubData(this->glID, startOffset, dataSize, data);
	return true;
}

void VertexBuffer::Unbind() const
{
	//array buffer
	if (this->glTarget == GL_ARRAY_BUFFER)
	{
		HorseRadish::OpenGL::glBindBuffer(GL_ARRAY_BUFFER, 0);
		return;
	}

	//element buffer
	if (this->glTarget == GL_ELEMENT_ARRAY_BUFFER)
	{
		HorseRadish::OpenGL::glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
		return;
	}
}

//§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§
//§§§§§§   Classe PixelBuffer	§§§§§
//§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§
bool PixelBuffer::LoadBuffer(const void * const data, const int &dataSize, const UsageType &usage) const
{
	if (dataSize <= 0)
		return false;

	GLenum glUsage = 0;
	switch (usage)
	{
		case VertexBuffer::Stream:
			glUsage = GL_STREAM_DRAW;
			break;
		case VertexBuffer::Static:
			glUsage = GL_STATIC_DRAW;
			break;
		case VertexBuffer::Dynamic:
			glUsage = GL_DYNAMIC_DRAW;
			break;
		default:
			return false;
	}

	HorseRadish::OpenGL::glNamedBufferData(this->glID, dataSize, data, glUsage);
	return true;
}

bool PixelBuffer::UpdateBuffer(const void * const data, const int &dataSize, const int &startOffset) const
{
	if ((data == nullptr) || (dataSize <= 0) || (startOffset < 0))
		return false;

	HorseRadish::OpenGL::glNamedBufferSubData(this->glID, startOffset, dataSize, data);
	return true;
}

void PixelBuffer::Unbind() const
{
	//array buffer
	if (this->glTarget == GL_PIXEL_UNPACK_BUFFER)
	{
		HorseRadish::OpenGL::glBindBuffer(GL_PIXEL_UNPACK_BUFFER, 0);
		return;
	}

	//element buffer
	if (this->glTarget == GL_PIXEL_PACK_BUFFER)
	{
		HorseRadish::OpenGL::glBindBuffer(GL_PIXEL_PACK_BUFFER, 0);
		return;
	}
}

//§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§
//§§§§§§   Classe VertexArray	§§§§§
//§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§

//§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§
//§§§§§§   Classe RenderBuffer	§§§§§
//§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§
void RenderBuffer::Init(const int &format, const int &width, const int &height) const
{
	if ((format == 0) || (width <= 0) || (height <= 0))
		return;

	HorseRadish::OpenGL::glNamedRenderbufferStorage(this->glID, format, width, height);
}

//§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§
//§§§§§§   Classe FrameBuffer	§§§§§
//§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§
bool FrameBuffer::GetStatusComplete() const
{
	switch (HorseRadish::OpenGL::glCheckNamedFramebufferStatus(this->glID, GL_FRAMEBUFFER))
	{
		case GL_FRAMEBUFFER_COMPLETE:
				return true;

		case GL_FRAMEBUFFER_UNSUPPORTED:
				return false;

		case GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT:
				return false;
		case GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT:
				return false;
		case GL_FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER:
				return false;
		case GL_FRAMEBUFFER_INCOMPLETE_READ_BUFFER:
				return false;
		case GL_FRAMEBUFFER_INCOMPLETE_LAYER_TARGETS:
			return false;
		}

	return false;
}

void FrameBuffer::AttachTColor(const Texture * const textureToAttach, const int attachUnit) const
{
	if ((textureToAttach == nullptr) || (attachUnit < 0))
		return;

	if ((textureToAttach->glTarget == GL_TEXTURE_2D) || (textureToAttach->glTarget == GL_TEXTURE_RECTANGLE))
		HorseRadish::OpenGL::glNamedFramebufferTexture(this->glID, GL_COLOR_ATTACHMENT0 + attachUnit, textureToAttach->glID, 0);
}

void FrameBuffer::AttachTColor(const Texture * const textureToAttach, const int attachUnit, const int cubemapFaceIndex) const
{
	//verificar algumas coisas
	if ((textureToAttach == nullptr) || (textureToAttach->glTarget != GL_TEXTURE_CUBE_MAP) || (attachUnit < 0))
		return;

	//faço bind ao framebuffer a ligo-o à textura
	HorseRadish::OpenGL::glNamedFramebufferTextureLayer(this->glID, GL_COLOR_ATTACHMENT0 + attachUnit, textureToAttach->glID, 0, GL_TEXTURE_CUBE_MAP_POSITIVE_X + cubemapFaceIndex);
}

void FrameBuffer::AttachTDepth(const Texture * const textureToAttach) const
{
	if (textureToAttach == nullptr)
		return;

	if ((textureToAttach->glTarget == GL_TEXTURE_2D) || (textureToAttach->glTarget == GL_TEXTURE_RECTANGLE))
		HorseRadish::OpenGL::glNamedFramebufferTexture(this->glID, GL_DEPTH_ATTACHMENT, textureToAttach->glID, 0);
}

void FrameBuffer::AttachRColor(const RenderBuffer * const renderbufferToAttach, const int attachUnit) const
{
	if (renderbufferToAttach == nullptr)
		return;

	HorseRadish::OpenGL::glNamedFramebufferRenderbuffer(this->glID, GL_COLOR_ATTACHMENT0 + attachUnit, GL_RENDERBUFFER, renderbufferToAttach->glID);
}

void FrameBuffer::AttachRDepth(const RenderBuffer * const renderbufferToAttach) const
{
	if (renderbufferToAttach == nullptr)
		return;

	HorseRadish::OpenGL::glNamedFramebufferRenderbuffer(this->glID, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, renderbufferToAttach->glID);
}

//§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§
//§§§§§§   Classe Shader	§§§§§
//§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§
bool Shader::SupplyCode(const char * const sourceCode) const
{
	//verificar certas cenas
	if ((sourceCode == nullptr) || (sourceCode[0] == '\0'))
		return false;

	//insiro o código
	HorseRadish::OpenGL::glShaderSource(this->glID, 1, (const GLchar**)(&sourceCode), nullptr);
	return true;
}

bool Shader::SupplyCode(const char * const sourceCode, const char * const textDefines) const
{
	const char *sourceStrings[2];

	//verificar certas cenas
	if ((sourceCode == nullptr) || (sourceCode[0] == '\0'))
		return false;

	//se não tenho nada como defines, mando para o normal
	if ((textDefines == nullptr) || (textDefines[0] == '\0'))
		return this->SupplyCode(sourceCode);

	//insiro o código
	sourceStrings[0] = textDefines;
	sourceStrings[1] = sourceCode;
	HorseRadish::OpenGL::glShaderSource(this->glID, 2, (const GLchar**)sourceStrings, nullptr);
	return true;
}

bool Shader::SupplyCodeFile(const HorseRadish::Streams::Stream *fileStream) const
{
	bool dataCopiada;
	int tamanho;
	const void *ficheiro;

	//verificar certas cenas
	if (fileStream == nullptr)
		return false;

	//mando abrir o ficheiro
	ficheiro = fileStream->ReadContent(tamanho, dataCopiada);
	if (ficheiro == nullptr)
		return false;

	//agora é simples, basta mandar para lá o código
	HorseRadish::OpenGL::glShaderSource(this->glID, 1, (const GLchar**)(&ficheiro), &tamanho);

	//se os dados foram copiados
	if (dataCopiada == true)
		free((void*)ficheiro);
	return true;
}

bool Shader::SupplyCodeFile(const HorseRadish::Streams::Stream *fileStream, const char * const textDefines) const
{
	const void *fileData;
	bool dataCopiada;
	int fileSize,sourceLengths[2];
	const char *sourceStrings[2];

	//verificar certas cenas
	if (fileStream == nullptr)
		return false;

	//se não tenho nada como defines, mando para o normal
	if ((textDefines == nullptr) || (textDefines[0] == '\0'))
		return this->SupplyCodeFile(fileStream);

	//mando abrir o ficheiro
	fileData = fileStream->ReadContent(fileSize, dataCopiada);
	if (fileData == nullptr)
		return false;

	//agora é simples, basta mandar para lá o código
	sourceLengths[0] = strlen(textDefines);
	sourceStrings[0] = textDefines;
	sourceLengths[1] = fileSize;
	sourceStrings[1] = (const char*)fileData;
	HorseRadish::OpenGL::glShaderSource(this->glID, 2, (const GLchar**)sourceStrings, sourceLengths);

	//se os dados foram copiados
	if (dataCopiada == true)
		free((void*)fileData);
	return true;
}

bool Shader::Compile(char ** const writeOutput) const
{
	int compiled, maxLength;

	//mando compilar e obtenho resultados
	HorseRadish::OpenGL::glCompileShader(this->glID);
	HorseRadish::OpenGL::glGetShaderiv(this->glID, GL_COMPILE_STATUS, &compiled);
	HorseRadish::OpenGL::glGetShaderiv(this->glID, GL_INFO_LOG_LENGTH, &maxLength);

	//se ninguem quiser justificações não houver justificações a dar
	if ((writeOutput == nullptr) || (maxLength <= 1))
		return true;

	//crio memoria para escrever as coisas e se deu, escrovo o log no buffer
	*writeOutput = (char*)malloc(maxLength);
	if (*writeOutput != nullptr)
		HorseRadish::OpenGL::glGetShaderInfoLog(this->glID, maxLength, nullptr, *writeOutput);

	//finalmente, devolvo o resultado encontrado
	return compiled;
}

//§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§
//§§§§§§   Classe Program	§§§§§
//§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§
bool Program::Link(char ** const writeOutput) const
{
	int maxLength, linked;

	//linko o programa e obtenho estado da linkagem
	HorseRadish::OpenGL::glLinkProgram(this->glID);
	HorseRadish::OpenGL::glGetProgramiv(this->glID, GL_LINK_STATUS, &linked);
	HorseRadish::OpenGL::glGetProgramiv(this->glID, GL_INFO_LOG_LENGTH, &maxLength);

	//se ninguem quiser justificações ou se não houver justificações a dar
	if (writeOutput==nullptr || maxLength<=1)
		{
		if (linked)
			return true;
		return false;
		}

	//crio memoria para escrever as coisas e se deu, escrovo o log no buffer
	*writeOutput=(char*)malloc(maxLength);
	if (*writeOutput!=nullptr)
		HorseRadish::OpenGL::glGetProgramInfoLog(this->glID, maxLength, nullptr, *writeOutput);

	//finalmente, devolvo o resultado encontrado
	return linked;
}

void Program::AttachShaders(const Shader * const shader) const
{
	if (shader==nullptr || (shader->glTarget!=GL_VERTEX_SHADER && shader->glTarget!=GL_FRAGMENT_SHADER) )
		return;

	//faço o attach 
	HorseRadish::OpenGL::glAttachShader(this->glID, shader->glID);
}

//§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§
//§§§§§§   Classe ObjectsManager	§§§§§
//§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§
ObjectsManager::OBJECT_STRUCT* ObjectsManager::criaObject()
{
	if (listaObjectos==nullptr)
	{
		listaObjectos=new OBJECT_STRUCT;
		listaObjectos->data = nullptr;
		listaObjectos->prev=listaObjectos->prox=nullptr;
		listaUltimo=listaObjectos;
		return listaObjectos;
	}

	listaUltimo->prox=new OBJECT_STRUCT;
	listaUltimo->prox->data = nullptr;
	listaUltimo->prox->prox=nullptr;
	listaUltimo->prox->prev=listaUltimo;
	listaUltimo=listaUltimo->prox;
	return listaUltimo;
}

void ObjectsManager::destroiObject(ObjectsManager::OBJECT_STRUCT * const object)
{
	if (object==nullptr)
		return;

	delete object->data;

	if (object==listaObjectos)
	{
		listaObjectos=listaObjectos->prox;
		if (listaObjectos)
			listaObjectos->prev=nullptr;
		else
			listaUltimo=nullptr;
		delete object;
		return;
	}

	if (object->prox)
		object->prox->prev=object->prev;
	else
		listaUltimo=listaUltimo->prev;
	object->prev->prox=object->prox;
	delete object;
}

ObjectsManager::OBJECT_STRUCT* ObjectsManager::procuraObject(const Objects::ObjectGL * const object)
{
	OBJECT_STRUCT *walker;

	if (object==nullptr)
		return nullptr;

	for(walker=listaObjectos; walker!=nullptr; walker=walker->prox)
	{
		if (walker->data==object)
			return walker;
	}

	return nullptr;
}

void ObjectsManager::writeConsola(const char * const logData)
{
	const char *walker,*fim;
	char stringEscrever[512];

	if (logData==nullptr || logData[0]=='\0')
		return;
    
	//mostro o output
	walker=fim=logData;
	while(*fim!='\0')
	{
		//até a próxima linha
		while(*fim!=10 && *fim!=13 && *fim!=0)
			fim++;

		//escrevo
		memcpy(stringEscrever,walker,fim-walker);
		stringEscrever[fim-walker]='\0';
		////SConsole::SConsoleLogTab(stringEscrever,2);

		//avanco
		if (*fim==10 || *fim==13)	fim++;
		if (*fim==10 || *fim==13)	fim++;
		walker=fim;
	}
}

void ObjectsManager::writeLog(const char * const logData)
{
	const char *walker,*fim;
	char stringEscrever[512];

	if (logData==nullptr || logData[0]=='\0')
		return;
    
	//mostro o output
	walker=fim=logData;
	while(*fim!='\0')
	{
		//até a próxima linha
		while(*fim!=10 && *fim!=13 && *fim!=0)
			fim++;

		//escrevo
		memcpy(stringEscrever,walker,fim-walker);
		stringEscrever[fim-walker]='\0';
		//SConsole::SConsolePLogTab(logID,stringEscrever,2);

		//avanco
		if (*fim==10 || *fim==13)	fim++;
		if (*fim==10 || *fim==13)	fim++;
		walker=fim;
	}
}

ObjectsManager::ObjectsManager(const Context * const context)
{
	//inicio isto a zero
	listaObjectos=listaUltimo=nullptr;

	//guardo o contexto
	this->context = context;

	//faço log de tudo pra ficar bonitinho
	//logID=SConsole::SConsolePOpen("logs\\glObjectsGes.txt",SCONSOLE_LOGTYPE_TEXT);
	//SConsole::SConsolePLogInfo(logID,"> OpenGL objects manager inicialized. <");
	//SConsole::SConsolePLog(logID," ");
}

ObjectsManager::~ObjectsManager()
{
	//limpar tudo
	Clear();

	//fechar o log
	//SConsole::SConsolePLog(logID," ");
	//SConsole::SConsolePLogInfo(logID,".:OpenGL objects manager has been shutdown:.");
	//SConsole::SConsolePClose(logID);
	logID=0;
}

void ObjectsManager::Clear()
{
	OBJECT_STRUCT *walker,*lixo;

	//destruo os objectos (do GL)
	for(walker=listaObjectos; walker!=nullptr; walker=walker->prox)
		walker->data->objectDestroy();

	//destruo os meus objectos
	walker=listaObjectos; 
	while(walker!=nullptr)
	{
		lixo=walker;
		walker=walker->prox;
		delete lixo;
	}

	//só falta isto e mais nada
	listaObjectos=listaUltimo=nullptr;
}

const Objects::ObjectGL* ObjectsManager::ObjectCreate(const int &type)
{
	OBJECT_STRUCT *novo;

	//framebuffer object
	if (type==ObjectsManager::FrameBuffer)
	{
		novo=criaObject();
		if (novo==nullptr)
			return nullptr;
		novo->data = new Objects::FrameBuffer();
		return novo->data;
	}

	//renderbuffer object
	if (type==ObjectsManager::RenderBuffer)
	{
		novo=criaObject();
		if (novo==nullptr)
			return nullptr;
		novo->data = new Objects::RenderBuffer();
		return novo->data;
	}

	//array buffers
	if (type==ObjectsManager::ArrayBuffer)
	{
		novo=criaObject();
		if (novo==nullptr)
			return nullptr;
		novo->data = new Objects::VertexBuffer(GL_ARRAY_BUFFER);
		return novo->data;
	}

	//element buffers
	if (type==ObjectsManager::ElementBuffer)
	{
		novo=criaObject();
		if (novo==nullptr)
			return nullptr;
		novo->data = new Objects::VertexBuffer(GL_ELEMENT_ARRAY_BUFFER);
		return novo->data;
	}

	//pixel unpack buffers
	if (type==ObjectsManager::PixelUnpackBuffer)
	{
		novo=criaObject();
		if (novo==nullptr)
			return nullptr;
		novo->data = new Objects::PixelBuffer(GL_PIXEL_UNPACK_BUFFER);
		return novo->data;
	}

	//pixel pack buffers
	if (type==ObjectsManager::PixelPackBuffer)
	{
		novo=criaObject();
		if (novo==nullptr)
			return nullptr;
		novo->data = new Objects::PixelBuffer(GL_PIXEL_PACK_BUFFER);
		return novo->data;
	}

	//vertex arrays
	if (type==ObjectsManager::VertexArray)
	{
		novo=criaObject();
		if (novo==nullptr)
			return nullptr;
		novo->data = new Objects::VertexArray();
		return novo->data;
	}

	//sampler
	if (type==ObjectsManager::Sampler)
	{
		novo=criaObject();
		if (novo==nullptr)
			return nullptr;
		novo->data = new Objects::Sampler();
		return novo->data;
	}

	//texturas 1D
	if (type==ObjectsManager::Texture1D)
	{
		novo=criaObject();
		if (novo==nullptr)
			return nullptr;
		novo->data = new Objects::Texture(GL_TEXTURE_1D);
		return novo->data;
	}

	//texturas 2D
	if (type==ObjectsManager::Texture2D)
	{
		novo=criaObject();
		if (novo==nullptr)
			return nullptr;
		novo->data = new Objects::Texture(GL_TEXTURE_2D);
		return novo->data;
	}

	//texturas 3D
	if (type==ObjectsManager::Texture3D)
	{
		novo=criaObject();
		if (novo==nullptr)
			return nullptr;
		novo->data = new Objects::Texture(GL_TEXTURE_3D);
		return novo->data;
	}

	//texturas cubemap
	if (type==ObjectsManager::TextureCubeMap)
	{
		novo=criaObject();
		if (novo==nullptr)
			return nullptr;
		novo->data = new Objects::Texture(GL_TEXTURE_CUBE_MAP);
		return novo->data;
	}

	//texturas rectangle
	if (type == ObjectsManager::TextureRect)
	{
		novo = criaObject();
		if (novo == nullptr)
			return nullptr;
		novo->data = new Objects::Texture(GL_TEXTURE_RECTANGLE);
		return novo->data;
	}

	//queries
	if (type==ObjectsManager::QuerySamples)
	{
		novo=criaObject();
		if (novo==nullptr)
			return nullptr;
		novo->data = new Objects::Query();
		return novo->data;
	}

	//glslang programs
	if (type==ObjectsManager::Program)
	{
		novo=criaObject();
		if (novo==nullptr)
			return nullptr;
		novo->data = new Objects::Program();
		return novo->data;
	}

	//vertex shader
	if (type==ObjectsManager::ShaderVertex)
	{
		novo=criaObject();
		if (novo==nullptr)
			return nullptr;
		novo->data = new Objects::Shader(Shader::Vertex);
		return novo->data;
	}

	//fragment shader
	if (type==ObjectsManager::ShaderFragment)
	{
		novo=criaObject();
		if (novo==nullptr)
			return nullptr;
		novo->data = new Objects::Shader(Shader::Fragment);
		return novo->data;
	}

	//não reconheço o tipo de ficheiro
	return nullptr;
}

void ObjectsManager::ObjectDelete(const Objects::ObjectGL * const object)
{
	OBJECT_STRUCT *objStruct;
	
	//procuro objecto a apagar
	objStruct=procuraObject(object);

	//não existe
	if (objStruct==nullptr)
		return;

	//apago os dois
	objStruct->data->objectDestroy();
	destroiObject(objStruct);
}

bool ObjectsManager::ShadersRead(HorseRadish::IO::FileSystem * const fileSystem, const Objects::Program *program, const char * const fileVertexShader, const char * const fileFragmentShader, const char * const textDefines)
{
	bool linkedCorrect;
	char *errorOut;
	HorseRadish::Streams::Stream *vertexShaderStream, *fragmentShaderStream;

	//verificar certas cenas
	if (program == nullptr)
		return false;

	//aviso o que vou fazer
	//SConsole::SConsolePLog(logID, "Creating program:");

	//carrego os ficheiros
	vertexShaderStream = fileSystem->FileRead(fileVertexShader);
	fragmentShaderStream = fileSystem->FileRead(fileFragmentShader);
	
	//inicio algumas coisas
	errorOut=nullptr;

	//caso tenha vertex shader
	if (vertexShaderStream != nullptr)
	{
		Objects::Shader *shader;
		//aviso o que vou fazer
		////SConsole::SConsolePLog(logID, HorseRadish::String(" - vertex shader: %s", vertexShaderFile));

		//crio o shader
		shader = (Shader*)ObjectCreate(ObjectsManager::ShaderVertex);
		if (shader == nullptr)
			return false;

		//coloco o código no shader, mando compilar e adiciono o shader ao programa
		shader->SupplyCodeFile(vertexShaderStream, textDefines);
		shader->Compile(&errorOut);
		program->AttachShaders(shader);
		ObjectDelete(shader);

		//se tiver log
		if (errorOut != nullptr)
		{
			writeLog(errorOut);
			free(errorOut);
			errorOut=nullptr;
		}
	}

	//caso tenha fragment shader
	if (fragmentShaderStream != nullptr)
	{
		Objects::Shader *shader;
		//aviso o que vou fazer
		////SConsole::SConsolePLog(logID, HorseRadish::String(" - fragment shader: %s", fragmentShaderFile));

		//crio o shader
		shader = (Shader*)ObjectCreate(ObjectsManager::ShaderFragment);
		if (shader == nullptr)
			return false;

		//coloco o código no shader, mando compilar e adiciono o shader ao programa
		shader->SupplyCodeFile(fragmentShaderStream,textDefines);
		shader->Compile(&errorOut);
		program->AttachShaders(shader);
		ObjectDelete(shader);

		//se tiver log
		if (errorOut != nullptr)
		{
			writeLog(errorOut);
			free(errorOut);
			errorOut=nullptr;
		}
	}

	//apago os ficheiros
	delete vertexShaderStream;
	delete fragmentShaderStream;

	//aviso o que vou fazer...
	//SConsole::SConsolePLog(logID, HorseRadish::String(" - linking program: %d", program->glID));

	//agora que já tratei dos shaders, posso mandar linkar esta cena toda
	linkedCorrect = program->Link(&errorOut);

	//se tiver log
	if (errorOut != nullptr)
	{
		writeLog(errorOut);
		free(errorOut);
		errorOut=nullptr;
	}

	//para não ficar tudo num monte
	//SConsole::SConsolePLog(logID, " ");

	//e saio conforme linkei ou não
	return linkedCorrect;
}

const Objects::Shader* ObjectsManager::ShaderCreate(const int type, const HorseRadish::Streams::Stream *fileStream)
{
	char *errorOut;
	const Objects::Shader *newShader;

	//tem de ser ou de um tipo ou de outro
	if (type!=ObjectsManager::ShaderVertex && type!=ObjectsManager::ShaderFragment)
		return nullptr;

	//crio o shader
	newShader = (Shader*)ObjectCreate(type);
	if (newShader == nullptr)
		return nullptr;

	//mando o código
	if (newShader->SupplyCodeFile(fileStream) == false)
	{
		//faço log do erro (na consola principal e no meu log), apago o shader e desapareço
		////SConsole::SConsoleLogError(HorseRadish::String("Erro ao fornecer código do ficheiro: %s",shaderFile));
		//SConsole::SConsolePLogError(logID, "Error supplying source code from file");
		ObjectDelete(newShader);
		return nullptr;
	}

	//tento compilar
	errorOut=nullptr;
	if (newShader->Compile(&errorOut) == false)
	{
		//digo qual o mal e escrevo o problema na consola
		////SConsole::SConsoleLogError(HorseRadish::String("Erro ao compilar código do ficheiro: %s",shaderFile));
		writeConsola(errorOut);

		//faço log interno
		//SConsole::SConsolePLog(logID, "Output compiling shader from file");
		writeLog(errorOut);
        
		//posso apagar o log, o shader e sair
		if (errorOut)
			free(errorOut);
		ObjectDelete(newShader);
		return nullptr;
	}

	//se tiver log
	if (errorOut)
	{
		//SConsole::SConsolePLog(logID, "Output compiling shader from file");
		writeLog(errorOut);
		free(errorOut);
		errorOut=nullptr;
	}

	//e já tá
	return newShader;
}

const Objects::Shader* ObjectsManager::ShaderCreate(const int type, const HorseRadish::Streams::Stream *fileStream, const char * const textDefines)
{
	char *errorOut;
	const Objects::Shader *newShader;

	//tem de ser ou de um tipo ou de outro
	if (type!=ObjectsManager::ShaderVertex && type!=ObjectsManager::ShaderFragment)
		return nullptr;

	//crio o shader
	newShader = (Shader*)ObjectCreate(type);
	if (newShader == nullptr)
		return nullptr;

	//mando o código
	if (newShader->SupplyCodeFile(fileStream, textDefines) == false)
	{
		//faço log do erro (na consola principal e no meu log), apago o shader e desapareço
		////SConsole::SConsoleLogError(HorseRadish::String("Erro ao fornecer código do ficheiro: %s",shaderFile));
		//SConsole::SConsolePLogError(logID, "Error supplying source code from file");
		ObjectDelete(newShader);
		return nullptr;
	}

	//tento compilar
	errorOut = nullptr;
	if (newShader->Compile(&errorOut) == false)
	{
		//digo qual o mal e escrevo o problema na consola
		////SConsole::SConsoleLogError(HorseRadish::String("Erro ao compilar código do ficheiro: %s",shaderFile));
		writeConsola(errorOut);

		//faço log interno
		//SConsole::SConsolePLog(logID, "Output compiling shader from file");
		writeLog(errorOut);
        
		//posso apagar o log, o shader e sair
		if (errorOut)
			free(errorOut);
		ObjectDelete(newShader);
		return nullptr;
	}

	//se tiver log
	if (errorOut)
	{
		//SConsole::SConsolePLog(logID, "Output compiling shader from file");
		writeLog(errorOut);
		free(errorOut);
		errorOut=nullptr;
	}

	//e já tá
	return newShader;
}

const Objects::Program* ObjectsManager::ProgramCreate(const Objects::Shader *vertexShader, const Objects::Shader *fragmentShader)
{
	char *errorOut;
	const Objects::Program *newProgram;

	//verificar os dados
	if (vertexShader==nullptr && fragmentShader==nullptr)
		return nullptr;

	//crio o programa
	newProgram = (const Objects::Program*)ObjectCreate(ObjectsManager::Program);
	if (newProgram == nullptr)
		return nullptr;

	//agora ligo os shades
	if (vertexShader)
		newProgram->AttachShaders(vertexShader);
	if (fragmentShader)
		newProgram->AttachShaders(fragmentShader);

	//agora que já tratei dos shaders, posso mandar linkar esta cena toda
	errorOut=nullptr;
	if (newProgram->Link(&errorOut) == false)
	{
		//digo qual o mal e escrevo o problema na consola
		////SConsole::SConsoleLogError("Erro ao linkar código:");
		writeConsola(errorOut);

		//faço log interno
		//SConsole::SConsolePLog(logID,"Error linking code:");
		writeLog(errorOut);

		//posso apagar o log, o programa e sair
		if (errorOut)
			free(errorOut);
		ObjectDelete(newProgram);
		return nullptr;
	}

	//se tiver log
	if (errorOut)
	{
		//SConsole::SConsolePLog(logID,"Output linking program:");
		writeLog(errorOut);
		free(errorOut);
		errorOut=nullptr;
	}

	//correu tudo bem, fixe
	return newProgram;
}

void ObjectsManager::SamplerSetAnisotropy(const Objects::Sampler * const sampler, const float &anisotropyLevel) const
{
	//verificar algumas coisas
	if ((this->context == nullptr) || (sampler == nullptr))
		return;

	//tem de haver suporte
	if ((this->context->extensionsAvailable & Context::FilterAnisotropic) == 0)
		return;

	//basta guardar o valor correcto
	HorseRadish::OpenGL::glSamplerParameterf(sampler->glID, GL_TEXTURE_MAX_ANISOTROPY_EXT, HorseRadish::Math::fClamp(anisotropyLevel, 1.0f, this->context->info.maxAnisotropy));
}

//§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§
//§§§§§§   Classe Context	§§§§§
//§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§
Context::Context()
{
	//limpo isto tudo
	this->extensionsAvailable = 0;
	this->contextCreated = false;
	memset(&this->info, 0, sizeof(Context::Info));
	memset(&this->stats, 0, sizeof(Context::Stats));
}

Context::~Context()
{
	//limpo isto tudo
	this->extensionsAvailable = 0;
	this->contextCreated = false;
	memset(&this->info, 0, sizeof(Context::Info));
	memset(&this->stats, 0, sizeof(Context::Stats));
}

void Context::InitializeContext()
{
	//se o contexto não está criado
	if (this->contextCreated == false)
		return;

	//verificar as extensões
	if (HorseRadish::OpenGL::Extensions::ExtensionExists("GL_EXT_texture_filter_anisotropic"))
		this->extensionsAvailable |= Context::FilterAnisotropic;
	if (HorseRadish::OpenGL::Extensions::ExtensionExists("GL_EXT_texture_compression_s3tc"))
		this->extensionsAvailable |= Context::CompressionS3;
	if (HorseRadish::OpenGL::Extensions::ExtensionExists("GL_NV_texture_compression_vtc"))
		this->extensionsAvailable |= Context::CompressionVTC;
	
	//preciso de alguma informação
	this->info.glslVersion.Set(HorseRadish::String::Encoding::ASCII, HorseRadish::OpenGL::glGetString(GL_SHADING_LANGUAGE_VERSION));
	this->info.renderer.Set(HorseRadish::String::Encoding::ASCII, HorseRadish::OpenGL::glGetString(GL_RENDERER));
	this->info.vendor.Set(HorseRadish::String::Encoding::ASCII, HorseRadish::OpenGL::glGetString(GL_VENDOR));
	this->info.version.Set(HorseRadish::String::Encoding::ASCII, HorseRadish::OpenGL::glGetString(GL_VERSION));
	HorseRadish::OpenGL::glGetIntegerv(GL_MAX_DRAW_BUFFERS, &this->info.maxDrawBuffers);
	HorseRadish::OpenGL::glGetIntegerv(GL_MAX_COLOR_ATTACHMENTS, &this->info.maxColorAttachments);
	HorseRadish::OpenGL::glGetIntegerv(GL_MAX_TEXTURE_SIZE, &this->info.maxTextureSize);
	HorseRadish::OpenGL::glGetIntegerv(GL_MAX_3D_TEXTURE_SIZE, &this->info.maxTexture3DSize);
	HorseRadish::OpenGL::glGetIntegerv(GL_MAX_CUBE_MAP_TEXTURE_SIZE, &this->info.maxTextureCubemapSize);
	HorseRadish::OpenGL::glGetIntegerv(GL_MAX_RECTANGLE_TEXTURE_SIZE, &this->info.maxTextureRectSize);
	HorseRadish::OpenGL::glGetIntegerv(GL_MAJOR_VERSION, &this->info.versionMajor);
	HorseRadish::OpenGL::glGetIntegerv(GL_MINOR_VERSION, &this->info.versionMinor);
	this->info.maxAnisotropy = 0.0f;

	//se tiver a extensão
	if ((this->extensionsAvailable & Context::FilterAnisotropic) != 0)
		HorseRadish::OpenGL::glGetFloatv(GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT, &this->info.maxAnisotropy);
}

bool Context::IsContextCreated() const
{
	//basta devolver isto
	return this->contextCreated;
}

bool Context::IsExtensionPresent(const Context::Extensions &extension) const
{
	//basta retornar isto
	return ((this->extensionsAvailable & extension) == extension);
}

bool Context::IsExtensionPresent(const char * const extensionName) const
{
	//basta retornar isto
	return HorseRadish::OpenGL::Extensions::ExtensionExists(extensionName);
}

void Context::DispatchDebugMessages() const
{
	int numMsgsLogged;

	if (this->contextCreated == false)
		return;

	//verifico se tenho alguma coisa guardada
	HorseRadish::OpenGL::glGetIntegerv(GL_DEBUG_LOGGED_MESSAGES_ARB, &numMsgsLogged);
	if (numMsgsLogged <= 0)
		return;

	GLenum listSources[5], listTypes[5], listIDs[5], listSeverities[5];
	GLsizei messageLogSize, listLengths[5];
	GLchar *messageLog, *messageLogWalker;
	GLuint messagesRead;

	//preciso de espaço para o conteúdo das mensagens
	messageLogSize = sizeof(GLchar) * 5 * HorseRadish::Platform::KiloByte;
	messageLog = new GLchar[messageLogSize];

	//enquanto tiver coisas a ler
	while (numMsgsLogged > 0)
	{
		//leio de 5 em 5
		messagesRead = HorseRadish::OpenGL::glGetDebugMessageLog(5, messageLogSize, listSources, listTypes, listIDs, listSeverities, listLengths, messageLog);

		//já li estas
		numMsgsLogged -= messagesRead;

		//para cada mensagem lida, mando-a para a função de callback
		messageLogWalker = messageLog;
		for(int i=0; i<messagesRead; i++)
		{
			//mando a mensagem
			HorseRadish::OpenGL::glDebugMessageInsert(listSources[i], listTypes[i], listIDs[i], listSeverities[i], listLengths[i], messageLogWalker);

			//avanço com o log
			messageLogWalker += listLengths[i];
		}
	}

	//posso apagar isto
	delete[] messageLog;
	messageLog = nullptr;
}

bool Context::GetInformation(const InformationType &informationType, String &infoValue) const
{
	//por omissão
	infoValue.SetEmpty();

	//para cada um dos tipos
	if (informationType == Context::Version)
	{
		infoValue = this->info.version;
		return true;
	}
	if (informationType == Context::Vendor)
	{
		infoValue = this->info.vendor;
		return true;
	}
	if (informationType == Context::Renderer)
	{
		infoValue = this->info.renderer;
		return true;
	}
	if (informationType == Context::GLSLVersion)
	{
		infoValue = this->info.glslVersion;
		return true;
	}

	//não conheço o tipo de informação
	return false;
}

void Context::CounterReset(const CounterType &counterType)
{
	//conforme o tipo de contador
	if (counterType == Context::Frames)
		this->stats.frameCount = 0;
	else if (counterType == Context::Triangles)
		this->stats.triangleCount = 0;
	else if (counterType == Context::Vertices)
		this->stats.vertexCount = 0;
}

void Context::CounterIncrease(const CounterType &counterType, unsigned int amount)
{
	//conforme o tipo de contador
	if (counterType == Context::Frames)
		this->stats.frameCount += amount;
	else if (counterType == Context::Triangles)
		this->stats.triangleCount += amount;
	else if (counterType == Context::Vertices)
		this->stats.vertexCount += amount;
}

unsigned int Context::CounterGetValue(const CounterType &counterType) const
{
	//conforme o tipo de contador
	if (counterType == Context::Frames)
		return this->stats.frameCount;
	else if (counterType == Context::Triangles)
		return this->stats.triangleCount;
	else if (counterType == Context::Vertices)
		return this->stats.vertexCount;

	//não faço nada
	return 0;
}

bool Context::GetInformation(const InformationType &informationType, int &infoValue) const
{
	//por omissão
	infoValue = 0;

	//para cada um dos tipos
	if (informationType == Context::MaxDrawBuffers)
	{
		infoValue = this->info.maxDrawBuffers;
		return true;
	}
	if (informationType == Context::MaxColorAttachments)
	{
		infoValue = this->info.maxColorAttachments;
		return true;
	}
	if (informationType == Context::MaxTextureSize)
	{
		infoValue = this->info.maxTextureSize;
		return true;
	}
	if (informationType == Context::MaxTexture3DSize)
	{
		infoValue = this->info.maxTexture3DSize;
		return true;
	}
	if (informationType == Context::MaxTextureCubemapSize)
	{
		infoValue = this->info.maxTextureCubemapSize;
		return true;
	}
	if (informationType == Context::MaxTextureRectSize)
	{
		infoValue = this->info.maxTextureRectSize;
		return true;
	}

	//não conheço o tipo de informação
	return false;
}

}//namespace Objects
}//namespace OpenGL
}//namespace HorseRadish