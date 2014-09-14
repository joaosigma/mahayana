#pragma once
#ifndef __HOPENGL_OBJECTS__
#define __HOPENGL_OBJECTS__

#include "openGL.hpp"
#include "openGLext.hpp"
#include "common\Image.hpp"
#include "common\Mesh.hpp"
#include "common\Stream.hpp"
#include "common\FileSystem.hpp"

namespace HorseRadish
{

namespace OpenGL
{

namespace Objects
{

//algumas definições
class ObjectsManager;
class Context;

//§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§
//§§§§§§   Classe ObjectGL	§§§§§
//§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§
class ObjectGL
{
private:
	virtual void objectDestroy() = 0;

	friend class ObjectsManager;

public:
	unsigned int glID;

public:
	ObjectGL()
		: glID(0)
	{ }
};

//§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§
//§§§§§§   Classe Sampler	§§§§§
//§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§
class Sampler : public ObjectGL
{
public:
	enum FilterType { Point, Linear, PointMipPoint, PointMipLinear, LinearMipPoint, LinearMipLinear };
	enum CompareMode { CompareRefToTexture, None };
	enum CompareFunc { LesserEqual, GreaterEqual, Lesser, Greater, Equal, NotEqual, Always, Never };
	enum WrapType { Repeat, ClampEdge };

private:
	void objectDestroy()
	{
		HorseRadish::OpenGL::glDeleteSamplers(1, &this->glID);
	}

public:
	Sampler()
		: ObjectGL()
	{
		HorseRadish::OpenGL::glCreateSamplers(1, &this->glID);
	}

	static void UnBind(const int textureUnit)
	{
		if ((textureUnit < 0) || (textureUnit > 31))
			return;

		HorseRadish::OpenGL::glBindSampler(textureUnit, 0);
	}

	void Bind(const int textureUnit) const
	{
		if ((textureUnit < 0) || (textureUnit > 31))
			return;

		HorseRadish::OpenGL::glBindSampler(textureUnit, this->glID);
	}

	void SetMinFilter(const FilterType &filterType) const
	{
		if (filterType == Point)
			HorseRadish::OpenGL::glSamplerParameteri(this->glID, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		else if (filterType == Linear)
			HorseRadish::OpenGL::glSamplerParameteri(this->glID, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		else if (filterType == LinearMipPoint)
			HorseRadish::OpenGL::glSamplerParameteri(this->glID, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_NEAREST);
		else if (filterType == LinearMipLinear)
			HorseRadish::OpenGL::glSamplerParameteri(this->glID, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
		else if (filterType == PointMipPoint)
			HorseRadish::OpenGL::glSamplerParameteri(this->glID, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_NEAREST);
		else if (filterType == PointMipLinear)
			HorseRadish::OpenGL::glSamplerParameteri(this->glID, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_LINEAR);
	}
	void SetMagFilter(const FilterType &filterType) const
	{
		if (filterType == Point)
			HorseRadish::OpenGL::glSamplerParameteri(this->glID, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		else if (filterType == Linear)
			HorseRadish::OpenGL::glSamplerParameteri(this->glID, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	}

	void SetWrap(const WrapType &wrapType) const
	{
		if (wrapType == Repeat)
		{
			HorseRadish::OpenGL::glSamplerParameteri(this->glID, GL_TEXTURE_WRAP_S, GL_REPEAT);
			HorseRadish::OpenGL::glSamplerParameteri(this->glID, GL_TEXTURE_WRAP_T, GL_REPEAT);
			HorseRadish::OpenGL::glSamplerParameteri(this->glID, GL_TEXTURE_WRAP_R, GL_REPEAT);
		}
		else if (wrapType == ClampEdge)
		{
			HorseRadish::OpenGL::glSamplerParameteri(this->glID, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
			HorseRadish::OpenGL::glSamplerParameteri(this->glID, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
			HorseRadish::OpenGL::glSamplerParameteri(this->glID, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
		}
	}

	void SetBorderColor(const float &r, const float &g, const float &b, const float &a) const
	{
		float borderColor[] = { r, g, b, a };
		HorseRadish::OpenGL::glSamplerParameterfv(this->glID, GL_TEXTURE_BORDER_COLOR, borderColor);
	}

	void SetBorderColor(const float * const borderColor) const
	{
		if (borderColor == nullptr)
			return;

		HorseRadish::OpenGL::glSamplerParameterfv(this->glID, GL_TEXTURE_BORDER_COLOR, borderColor);
	}

	void SetLOD(const float &minLOD, const float &maxLOD) const
	{
		HorseRadish::OpenGL::glSamplerParameterf(this->glID, GL_TEXTURE_MIN_LOD, minLOD);
		HorseRadish::OpenGL::glSamplerParameterf(this->glID, GL_TEXTURE_MAX_LOD, maxLOD);
	}

	void SetLODBias(const float &lodBias) const
	{
		HorseRadish::OpenGL::glSamplerParameterf(this->glID, GL_TEXTURE_LOD_BIAS, lodBias);
	}

	void SetCompare(const CompareMode &compareMode, const CompareFunc &compareFunc) const
	{
		if (compareMode == CompareRefToTexture)
			HorseRadish::OpenGL::glSamplerParameteri(this->glID, GL_TEXTURE_COMPARE_MODE, GL_COMPARE_REF_TO_TEXTURE);
		else
			HorseRadish::OpenGL::glSamplerParameteri(this->glID, GL_TEXTURE_COMPARE_MODE, GL_NONE);

		if (compareFunc == LesserEqual)
			HorseRadish::OpenGL::glSamplerParameteri(this->glID, GL_TEXTURE_COMPARE_FUNC, GL_LEQUAL);
		else if (compareFunc == GreaterEqual)
			HorseRadish::OpenGL::glSamplerParameteri(this->glID, GL_TEXTURE_COMPARE_FUNC, GL_GEQUAL);
		else if (compareFunc == Lesser)
			HorseRadish::OpenGL::glSamplerParameteri(this->glID, GL_TEXTURE_COMPARE_FUNC, GL_LESS);
		else if (compareFunc == Greater)
			HorseRadish::OpenGL::glSamplerParameteri(this->glID, GL_TEXTURE_COMPARE_FUNC, GL_GREATER);
		else if (compareFunc == Equal)
			HorseRadish::OpenGL::glSamplerParameteri(this->glID, GL_TEXTURE_COMPARE_FUNC, GL_EQUAL);
		else if (compareFunc == NotEqual)
			HorseRadish::OpenGL::glSamplerParameteri(this->glID, GL_TEXTURE_COMPARE_FUNC, GL_NOTEQUAL);
		else if (compareFunc == Always)
			HorseRadish::OpenGL::glSamplerParameteri(this->glID, GL_TEXTURE_COMPARE_FUNC, GL_ALWAYS);
		else  if (compareFunc == Never)
			HorseRadish::OpenGL::glSamplerParameteri(this->glID, GL_TEXTURE_COMPARE_FUNC, GL_NEVER);
		else
			HorseRadish::OpenGL::glSamplerParameteri(this->glID, GL_TEXTURE_COMPARE_FUNC, GL_LEQUAL);
	}
};

//§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§
//§§§§§§   Classe Texture	§§§§§
//§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§
class Texture : public ObjectGL
{
public:
	enum FilterType { Point, Linear, PointMipPoint, PointMipLinear, LinearMipPoint, LinearMipLinear };
	enum WrapType { Repeat, ClampEdge };

	unsigned int glTarget;
	unsigned short width, height, depth;

private:
	void objectDestroy()
	{
		HorseRadish::OpenGL::glDeleteTextures(1, &this->glID);
	}

public:
	static int CalculateNumMipMaps(const int width);
	static int CalculateNumMipMaps(const int width, const int height);
	static int CalculateNumMipMaps(const int width, const int height, const int depth);

public:
	Texture()
		: ObjectGL()
		, glTarget(0), width(0), height(0), depth(0)
	{
	}

	Texture(unsigned int glTarget)
		: ObjectGL()
		, glTarget(glTarget), width(0), height(0), depth(0)
	{
		HorseRadish::OpenGL::glCreateTextures(glTarget, 1, &this->glID);
	}

	void Bind(const int textureUnit) const
	{
		if ((textureUnit < 0) || (textureUnit > 31))
			return;
		HorseRadish::OpenGL::glBindTextureUnit(textureUnit, this->glID);
	}
};

//§§§§§§§§§§§§§§§§§§§§§§§§§§§
//§§§§§§   Classe Query	§§§§§
//§§§§§§§§§§§§§§§§§§§§§§§§§§§
class Query : public ObjectGL
{
private:
	void objectDestroy()
	{
		HorseRadish::OpenGL::glDeleteQueries(1, &this->glID);
	}

public:
	Query()
		: ObjectGL()
	{
		HorseRadish::OpenGL::glGenQueries(1, &this->glID);
	}

	void BeginQuery() const
	{
		HorseRadish::OpenGL::glBeginQuery(GL_SAMPLES_PASSED, this->glID);
	}

	void EndQuery() const
	{
		HorseRadish::OpenGL::glEndQuery(GL_SAMPLES_PASSED);
	}

	unsigned int GetResult() const
	{
		unsigned int queryResult;

		HorseRadish::OpenGL::glGetQueryObjectuiv(this->glID, GL_QUERY_RESULT, &queryResult);
		return queryResult;
	}

	bool ResultAvailable() const
	{
		int queryResult;

		HorseRadish::OpenGL::glGetQueryObjectiv(this->glID, GL_QUERY_RESULT_AVAILABLE, &queryResult);
		return (queryResult != 0);
	}
};

//§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§
//§§§§§§   Classe VertexBuffer	§§§§§
//§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§
class VertexBuffer : public ObjectGL
{
public:
	unsigned int glTarget;

public:
	enum UsageType{
		Stream,
		Static,
		Dynamic
	};

private:
	void objectDestroy()
	{
		HorseRadish::OpenGL::glDeleteBuffers(1, &this->glID);
	}

public:
	VertexBuffer(unsigned int glTarget)
		: ObjectGL()
		, glTarget(glTarget)
	{
		HorseRadish::OpenGL::glCreateBuffers(1, &this->glID);
	}

	bool LoadBuffer(const void * const data, const int &dataSize, const UsageType &usage) const;
	bool LoadMeshData(HorseRadish::Geometry::Mesh * const mesh, const UsageType &usage) const;
	bool LoadMeshIndex(HorseRadish::Geometry::Mesh * const mesh, const UsageType &usage) const;
	bool UpdateBuffer(const void * const data, const int &dataSize, const int &startOffset) const;
	void Unbind() const;

	void Bind() const
	{
		HorseRadish::OpenGL::glBindBuffer(this->glTarget, this->glID);
	}
};

//§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§
//§§§§§§   Classe PixelBuffer	§§§§§
//§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§
class PixelBuffer : public ObjectGL
{
public:
	unsigned int glTarget;

public:
	enum UsageType{
		Stream,
		Static,
		Dynamic
	};

private:
	void objectDestroy()
	{
		HorseRadish::OpenGL::glDeleteBuffers(1, &this->glID);
	}

public:
	PixelBuffer(unsigned int glTarget)
		: ObjectGL()
		, glTarget(glTarget)
	{
		HorseRadish::OpenGL::glCreateBuffers(1, &this->glID);
	}

	bool LoadBuffer(const void * const data, const int &dataSize, const UsageType &usage) const;
	bool UpdateBuffer(const void * const data, const int &dataSize, const int &startOffset) const;
	void Unbind() const;

	void Bind() const
	{
		HorseRadish::OpenGL::glBindBuffer(this->glTarget, this->glID);
	}
};

//§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§
//§§§§§§   Classe VertexArray	§§§§§
//§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§
class VertexArray : public ObjectGL
{
private:
	void objectDestroy()
	{
		HorseRadish::OpenGL::glDeleteVertexArrays(1, &this->glID);
	}

public:
	VertexArray()
		: ObjectGL()
	{
		HorseRadish::OpenGL::glCreateVertexArrays(1, &this->glID);
	}

	void Bind() const
	{
		HorseRadish::OpenGL::glBindVertexArray(this->glID);
	}
};

//§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§
//§§§§§§   Classe RenderBuffer	§§§§§
//§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§
class RenderBuffer : public ObjectGL
{
private:
	void objectDestroy()
	{
		HorseRadish::OpenGL::glDeleteRenderbuffers(1, &this->glID);
	}

public:
	RenderBuffer()
		: ObjectGL()
	{
		HorseRadish::OpenGL::glCreateRenderbuffers(1, &this->glID);
	}

	void Init(const int &format, const int &width, const int &height) const;
};

//§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§
//§§§§§§   Classe FrameBuffer	§§§§§
//§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§
class FrameBuffer : public ObjectGL
{
private:
	void objectDestroy()
	{
		HorseRadish::OpenGL::glDeleteFramebuffers(1, &this->glID);
	}

public:
	FrameBuffer()
		: ObjectGL()
	{
		HorseRadish::OpenGL::glCreateFramebuffers(1, &this->glID);
	}

	bool GetStatusComplete() const;
	void AttachTColor(const Texture * const textureToAttach, const int attachUnit) const;
	void AttachTColor(const Texture * const textureToAttach, const int attachUnit, const int cubemapFaceIndex) const;
	void AttachTDepth(const Texture * const textureToAttach) const;
	void AttachRColor(const RenderBuffer * const renderbufferToAttach, const int attachUnit) const;
	void AttachRDepth(const RenderBuffer * const renderbufferToAttach) const;

	void Bind() const
	{
		HorseRadish::OpenGL::glBindFramebuffer(GL_FRAMEBUFFER, this->glID);
	}
};

//§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§
//§§§§§§   Classe Shader	§§§§§
//§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§
class Shader : public ObjectGL
{
public:
	unsigned int glTarget;

public:
	enum ShaderType{
		Vertex, Fragment
		};

private:
	ShaderType shaderType;

	void objectDestroy()
	{
		HorseRadish::OpenGL::glDeleteShader(this->glID);
	}

public:
	Shader(const ShaderType &shaderType)
		: ObjectGL()
		, shaderType(shaderType), glTarget(0)
	{
		if (this->shaderType == Vertex)
			this->glTarget = GL_VERTEX_SHADER;
		else if (this->shaderType == Fragment)
			this->glTarget = GL_FRAGMENT_SHADER;
		else
			return;

		this->glID = HorseRadish::OpenGL::glCreateShader(this->glTarget);
	}

	bool SupplyCode(const char * const sourceCode) const;
	bool SupplyCode(const char * const sourceCode, const char * const textDefines) const;
	bool SupplyCodeFile(const HorseRadish::Streams::Stream *fileStream) const;
	bool SupplyCodeFile(const HorseRadish::Streams::Stream *fileStream, const char * const textDefines) const;
	bool Compile(char ** const writeOutput) const;
};

//§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§
//§§§§§§   Classe Program	§§§§§
//§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§
class Program : public ObjectGL
{
private:
	void objectDestroy()
	{
		HorseRadish::OpenGL::glDeleteProgram(this->glID);
	}

public:
	Program()
		: ObjectGL()
	{
		this->glID = HorseRadish::OpenGL::glCreateProgram();
	}

	bool Link(char ** const writeOutput) const;
	void AttachShaders(const Shader * const shader) const;

	void Bind() const
	{
		HorseRadish::OpenGL::glUseProgram(this->glID);
	}
};

//§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§
//§§§§§§   Classe ObjectsManager	§§§§§
//§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§

#define STEXTURE_COMPRESS			(1<<0)
#define STEXTURE_NO_MIPMAPS			(1<<1)
#define STEXTURE_NORMAL_MAP_MIPS	(1<<2)

struct TexData;
struct ImgData;

class ObjectsManager
{
public:
	enum ObjectType{
		ArrayBuffer, ElementBuffer, PixelUnpackBuffer, PixelPackBuffer, VertexArray, Sampler, Texture1D, Texture2D, Texture3D, TextureCubeMap, TextureRect, QuerySamples, Program, ShaderVertex, ShaderFragment, FrameBuffer, RenderBuffer
		};

private:
	struct OBJECT_STRUCT
	{
		ObjectGL *data;
		OBJECT_STRUCT *prox,*prev;
	};

	OBJECT_STRUCT *listaObjectos, *listaUltimo;
	const Context *context;
	int logID;

	OBJECT_STRUCT* criaObject();
	void destroiObject(OBJECT_STRUCT * const object);
	OBJECT_STRUCT* procuraObject(const ObjectGL * const object);
	void writeConsola(const char * const logData);
	void writeLog(const char * const logData);

public:
	ObjectsManager(const Context * const context);
	~ObjectsManager();
	
	//criação e remoção
	const Objects::ObjectGL* ObjectCreate(const int &type);
	void ObjectDelete(const Objects::ObjectGL * const object);
	void Clear(void);

	//coisas para GLSLang
	bool ShadersRead(HorseRadish::IO::FileSystem * const fileSystem, const Objects::Program *program, const char * const fileVertexShader, const char * const fileFragmentShader, const char * const textDefines);
	const Objects::Shader* ShaderCreate(const int type, const HorseRadish::Streams::Stream *fileStream);
	const Objects::Shader* ShaderCreate(const int type, const HorseRadish::Streams::Stream *fileStream, const char * const textDefines);
	const Objects::Program* ProgramCreate(const Objects::Shader *vertexShader, const Objects::Shader *fragmentShader);

	//coisas para os samplers
	void SamplerSetAnisotropy(const Objects::Sampler * const sampler, const float &anisotropyLevel) const;
	
public:
	enum TargetType {Alpha8, RGBA16, RGBA32, RGBA64, RGBA128, Depth16, Depth24, DepthStencil, Red32};

	const Objects::Texture* create1D(const ImgData * const img, const TargetType targetType, const int flag);
	const Objects::Texture* Create1D(const HorseRadish::IO::Path &path, const TargetType targetType, const int flag);
	const Objects::Texture* Create1D(const unsigned char * const data, const int comp, const TargetType targetType, const int flag);
	const Objects::Texture* Create1D(const HorseRadish::Imaging::Image * const img, const TargetType targetType, const int flag);

	const Objects::Texture* create2D(const ImgData * const img, const TargetType targetType, const int flag);
	const Objects::Texture* Create2D(const HorseRadish::IO::Path &path, const TargetType targetType, const int flag);
	const Objects::Texture* Create2D(const unsigned char * const data, const int comp, const int larg, const TargetType targetType, const int flag);
	const Objects::Texture* Create2D(const HorseRadish::Imaging::Image * const img, const TargetType targetType, const int flag);

	const Objects::Texture* Create3D(const unsigned char * const data, const int comp, const int larg, const int depth, const TargetType targetType, const int flag);

	const Objects::Texture* CreateCube(const unsigned int faceSize, const TargetType targetType, const int flag);
	const Objects::Texture* CreateCube(const HorseRadish::IO::Path &path, const TargetType targetType, const int flag);

	const Objects::Texture* createRect(const bool immutable, const ImgData * const img, const TargetType targetType);
	const Objects::Texture* CreateRect(const bool immutable, const HorseRadish::IO::Path &path, const TargetType targetType);
	const Objects::Texture* CreateRect(const bool immutable, const unsigned char * const data, const int comp, const int larg, const TargetType targetType);
	const Objects::Texture* CreateRect(const bool immutable, const HorseRadish::Imaging::Image * const img, const TargetType targetType);

	const Objects::Texture* LoadDDS(const HorseRadish::IO::Path &path);
};

//§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§
//§§§§§§   Classe Context	§§§§§
//§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§
class Context
{
public:
	enum Extensions {FilterAnisotropic = (1<<0), CompressionS3 = (1<<1), CompressionVTC = (1<<2)};
	enum InformationType {Version, Vendor, Renderer, GLSLVersion, MaxDrawBuffers, MaxColorAttachments, MaxTextureSize, MaxTexture3DSize, MaxTextureCubemapSize, MaxTextureRectSize};
	enum CounterType {Frames, Triangles, Vertices};

private:
	struct Info{
		String version, vendor, renderer, glslVersion;
		int versionMajor, versionMinor;
		int maxDrawBuffers, maxColorAttachments, maxTextureSize, maxTexture3DSize, maxTextureCubemapSize, maxTextureRectSize;
		float maxAnisotropy;
	} info;
	struct Stats{
		unsigned int frameCount, triangleCount, vertexCount;
	} stats;

	int extensionsAvailable;

	friend class ObjectsManager;

protected:
	bool contextCreated;

public:
	Context();
	virtual ~Context();

	void InitializeContext();
	bool IsContextCreated() const;
	bool IsExtensionPresent(const Extensions &extension) const;
	bool IsExtensionPresent(const char * const extensionName) const;
	void DispatchDebugMessages() const;

	void CounterReset(const CounterType &counterType);
	void CounterIncrease(const CounterType &counterType, unsigned int amount = 1);
	unsigned int CounterGetValue(const CounterType &counterType) const;

	bool GetInformation(const InformationType &informationType, String &infoValue) const;
	bool GetInformation(const InformationType &informationType, int &infoValue) const;
};

}//namespace Objects
}//namespace OpenGL
}//namespace HorseRadish

#endif