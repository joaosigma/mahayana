#pragma once

#include "context.hpp"
#include "openGL.hpp"
#include "openGLext.hpp"

#include "common/math.hpp"

#include <array>
#include <vector>
#include <cassert>
#include <functional>
#include <initializer_list>

namespace hr { namespace gl { namespace objects
{
	class ObjectGL
	{
	protected:
		GLuint mId = 0;

		ObjectGL()
		{ }

		ObjectGL(const ObjectGL&) = delete;
		ObjectGL& operator=(const ObjectGL&) = delete;
		ObjectGL(ObjectGL&&) = default;
		ObjectGL& operator=(ObjectGL&&) = default;

	public:
		GLuint getId() const
		{
			return mId;
		}

		bool isValid() const
		{
			return (mId != 0);
		}
	};

	class Texture final : public ObjectGL
	{
	public:
		enum class Type { Tex1D, Tex2D, Tex3D, TexRectangle, TexCubemap, Tex1DArray, Tex2DArray };
		enum class StorageType
		{
			R_8, R_16F, R_32F, R_8I, R_8UI, R_16I, R_16UI, R_32I, R_32UI,
			RG_8, RG_16F, RG_32F, RG_8I, RG_8UI, RG_16I, RG_16UI, RG_32I, RG_32UI,
			RGB_8, RGB_16F, RGB_32F, RGB_8I, RGB_8UI, RGB_16I, RGB_16UI, RGB_32I, RGB_32UI,
			RGBA_8, RGBA_16F, RGBA_32F, RGBA_8I, RGBA_8UI, RGBA_16I, RGBA_16UI, RGBA_32I, RGBA_32UI,
			DEPTH_16, DEPTH_24, DEPTH_24_STENCIL_8,

			COMPRESSED_BC1, COMPRESSED_SRGB_BC1, //RGB
			COMPRESSED_BC3, COMPRESSED_SRGB_BC3, //RGBA
			COMPRESSED_BC4, //Mono (grayscale)
			COMPRESSED_BC5, //Dual (2xgrayscale)
		};
		enum class DataType { BYTE, UBYTE, SHORT, USHORT, INT, UINT, FLOAT };
		enum class DataFormat { R, G, B, RG, RGB, RGBA, BGR, BGRA };
		enum class CubemapFace { PosX, NegX, PosY, NegY, PosZ, NegZ };

	private:
		GLenum mType = 0;
		GLenum mStorageType = 0;
		GLuint mWidth = 0, mHeight = 0, mDepth = 0;

		static GLenum translate(Type type)
		{
			switch (type)
			{
			case Type::Tex1D:
				return GL_TEXTURE_1D;
			case Type::Tex2D:
				return GL_TEXTURE_2D;
			case Type::Tex3D:
				return GL_TEXTURE_3D;
			case Type::TexRectangle:
				return GL_TEXTURE_RECTANGLE;
			case Type::TexCubemap:
				return GL_TEXTURE_CUBE_MAP;
			case Type::Tex1DArray:
				return GL_TEXTURE_1D_ARRAY;
			case Type::Tex2DArray:
				return GL_TEXTURE_2D_ARRAY;
			};

			assert(false);
			return GL_TEXTURE_2D;
		}

		static GLenum translate(StorageType storageType)
		{
			switch (storageType)
			{
			case StorageType::R_8:
				return GL_R8;
			case StorageType::R_16F:
				return GL_R16F;
			case StorageType::R_32F:
				return GL_R32F;
			case StorageType::R_8I:
				return GL_R8I;
			case StorageType::R_8UI:
				return GL_R8UI;
			case StorageType::R_16I:
				return GL_R16I;
			case StorageType::R_16UI:
				return GL_R16UI;
			case StorageType::R_32I:
				return GL_R32I;
			case StorageType::R_32UI:
				return GL_R32UI;
			
			case StorageType::RG_8:
				return GL_RG8;
			case StorageType::RG_16F:
				return GL_RG16F;
			case StorageType::RG_32F:
				return GL_RG32F;
			case StorageType::RG_8I:
				return GL_RG8I;
			case StorageType::RG_8UI:
				return GL_RG8UI;
			case StorageType::RG_16I:
				return GL_RG16I;
			case StorageType::RG_16UI:
				return GL_RG16UI;
			case StorageType::RG_32I:
				return GL_RG32I;
			case StorageType::RG_32UI:
				return GL_RG32UI;
			
			case StorageType::RGB_8:
				return GL_RGB8;
			case StorageType::RGB_16F:
				return GL_RGB16F;
			case StorageType::RGB_32F:
				return GL_RGB32F;
			case StorageType::RGB_8I:
				return GL_RGB8I;
			case StorageType::RGB_8UI:
				return GL_RGB8UI;
			case StorageType::RGB_16I:
				return GL_RGB16I;
			case StorageType::RGB_16UI:
				return GL_RGB16UI;
			case StorageType::RGB_32I:
				return GL_RGB32I;
			case StorageType::RGB_32UI:
				return GL_RGB32UI;
			
			case StorageType::RGBA_8:
				return GL_RGBA8;
			case StorageType::RGBA_16F:
				return GL_RGBA16F;
			case StorageType::RGBA_32F:
				return GL_RGBA32F;
			case StorageType::RGBA_8I:
				return GL_RGBA8I;
			case StorageType::RGBA_8UI:
				return GL_RGBA8UI;
			case StorageType::RGBA_16I:
				return GL_RGBA16I;
			case StorageType::RGBA_16UI:
				return GL_RGBA16UI;
			case StorageType::RGBA_32I:
				return GL_RGBA32I;
			case StorageType::RGBA_32UI:
				return GL_RGBA32UI;

			case StorageType::DEPTH_16:
				return GL_DEPTH_COMPONENT16;
			case StorageType::DEPTH_24:
				return GL_DEPTH_COMPONENT24;
			case StorageType::DEPTH_24_STENCIL_8:
				return GL_DEPTH24_STENCIL8;

			case StorageType::COMPRESSED_BC1:
				return GL_COMPRESSED_RGB_S3TC_DXT1_EXT;
			case StorageType::COMPRESSED_SRGB_BC1:
				return GL_COMPRESSED_SRGB_S3TC_DXT1_EXT;
			case StorageType::COMPRESSED_BC3:
				return GL_COMPRESSED_RGBA_S3TC_DXT5_EXT;
			case StorageType::COMPRESSED_SRGB_BC3:
				return GL_COMPRESSED_SRGB_ALPHA_S3TC_DXT5_EXT;
			case StorageType::COMPRESSED_BC4:
				return GL_COMPRESSED_RED_RGTC1;
			case StorageType::COMPRESSED_BC5:
				return GL_COMPRESSED_RG_RGTC2;
			};

			assert(false);
			return GL_RGBA8;
		}

		static GLenum translate(DataType dataType)
		{
			switch (dataType)
			{
			case DataType::BYTE:
				return GL_BYTE;
			case DataType::UBYTE:
				return GL_UNSIGNED_BYTE;
			case DataType::SHORT:
				return GL_SHORT;
			case DataType::USHORT:
				return GL_UNSIGNED_SHORT;
			case DataType::INT:
				return GL_INT;
			case DataType::UINT:
				return GL_UNSIGNED_INT;
			case DataType::FLOAT:
				return GL_FLOAT;
			};

			assert(false);
			return GL_UNSIGNED_BYTE;
		}

		static GLenum translate(DataFormat dataFormat)
		{
			switch (dataFormat)
			{
			case DataFormat::R:
				return GL_RED;
			case DataFormat::G:
				return GL_GREEN;
			case DataFormat::B:
				return GL_BLUE;
			case DataFormat::RG:
				return GL_RG;
			case DataFormat::RGB:
				return GL_RGB;
			case DataFormat::RGBA:
				return GL_RGBA;
			case DataFormat::BGR:
				return GL_BGR;
			case DataFormat::BGRA:
				return GL_BGRA;
			};

			assert(false);
			return GL_RGBA;
		}

	public:
		static size_t calculateNumMipMaps(GLuint width)
		{
			return static_cast<size_t>(std::floor(std::log2(width))) + 1;
		}

		static size_t calculateNumMipMaps(GLuint width, GLuint height)
		{
			return calculateNumMipMaps(std::max(width, height));
		}

		static size_t calculateNumMipMaps(GLuint width, GLuint height, GLuint depth)
		{
			return calculateNumMipMaps(std::max(std::max(width, height), depth));
		}

	public:
		Texture()
		{ }

		~Texture()
		{
			reset();
		}

		void init(Type type, StorageType storageType, GLuint width)
		{
			init(type, storageType, width, 0);
		}

		void init(Type type, StorageType storageType, GLuint width, GLuint height)
		{
			init(type, storageType, width, height, 0);
		}

		void init(Type type, StorageType storageType, GLuint width, GLuint height, GLuint depth)
		{
			if (isValid())
				return;

			switch (type)
			{
			case Type::Tex1D:
				assert((width > 0) && (height = 0) && (depth == 0));
				assert(Texture::translate(type) == GL_TEXTURE_1D);

				mType = Texture::translate(type);
				mStorageType = Texture::translate(storageType);
				mWidth = width;
				mHeight = height;
				mDepth = depth;

				glCreateTextures(mType, 1, &mId);
				glTextureStorage1D(mId, Texture::calculateNumMipMaps(width), mStorageType, width);
				break;

			case Type::Tex2D:
				assert((width > 0) && (height > 0) && (depth == 0));
				assert(Texture::translate(type) == GL_TEXTURE_2D);

				mType = Texture::translate(type);
				mStorageType = Texture::translate(storageType);
				mWidth = width;
				mHeight = height;
				mDepth = depth;
				
				glCreateTextures(mType, 1, &mId);
				glTextureStorage2D(mId, Texture::calculateNumMipMaps(width, height), mStorageType, width, height);
				break;

			case Type::Tex3D:
				assert((width > 0) && (height > 0) && (depth > 0));
				assert(Texture::translate(type) == GL_TEXTURE_3D);

				mType = Texture::translate(type);
				mStorageType = Texture::translate(storageType);
				mWidth = width;
				mHeight = height;
				mDepth = depth;

				glCreateTextures(mType, 1, &mId);
				glTextureStorage3D(mId, Texture::calculateNumMipMaps(width, height, depth), mStorageType, width, height, depth);
				break;

			case Type::TexRectangle:
				assert((width > 0) && (height > 0) && (depth == 0));
				assert(Texture::translate(type) == GL_TEXTURE_RECTANGLE);

				mType = Texture::translate(type);
				mStorageType = Texture::translate(storageType);
				mWidth = width;
				mHeight = height;
				mDepth = depth;

				glCreateTextures(mType, 1, &mId);
				glTextureStorage2D(mId, 1, mStorageType, width, height);
				break;

			case Type::TexCubemap:
				assert((width > 0) && (height > 0) && (depth == 0));
				assert(Texture::translate(type) == GL_TEXTURE_CUBE_MAP);

				mType = Texture::translate(type);
				mStorageType = Texture::translate(storageType);
				mWidth = width;
				mHeight = height;
				mDepth = depth;

				glCreateTextures(mType, 1, &mId);
				glTextureStorage2D(mId, Texture::calculateNumMipMaps(width, height), mStorageType, width, height);
				break;

			case Type::Tex1DArray:
				assert((width > 0) && (height > 0) && (depth == 0));
				assert(Texture::translate(type) == GL_TEXTURE_1D_ARRAY);

				mType = Texture::translate(type);
				mStorageType = Texture::translate(storageType);
				mWidth = width;
				mHeight = height;
				mDepth = depth;

				glCreateTextures(mType, 1, &mId);
				glTextureStorage2D(mId, Texture::calculateNumMipMaps(width), mStorageType, width, height);
				break;

			case Type::Tex2DArray:
				assert((width > 0) && (height > 0) && (depth > 0));
				assert(Texture::translate(type) == GL_TEXTURE_2D_ARRAY);

				mType = Texture::translate(type);
				mStorageType = Texture::translate(storageType);
				mWidth = width;
				mHeight = height;
				mDepth = depth;

				glCreateTextures(mType, 1, &mId);
				glTextureStorage3D(mId, Texture::calculateNumMipMaps(width, height), mStorageType, width, height, depth);
				break;

			default:
				return;
			}
		}

		void reset()
		{
			if (!isValid())
				return;

			glDeleteTextures(1, &mId);
			mId = 0;

			mType = 0;
			mStorageType = 0;
			mWidth = mHeight = mDepth = 0;
		}

		size_t width() const
		{
			return mWidth;
		}

		size_t height() const
		{
			return mHeight;
		}

		size_t depth() const
		{
			return mDepth;
		}

		void bind(const GLuint textureUnit) const
		{
			if (!isValid())
				return;

			glBindTextureUnit(textureUnit, mId);
		}

		void clear(float r, float g, float b, float a) const
		{
			clear(0, r, g, b, a);
		}

		void clear(GLint mipLevel, float r, float g, float b, float a) const
		{
			if (!isValid())
				return;

			float colorVec[] = { r, g, b, a };
			glClearTexImage(mId, mipLevel, GL_RGBA, GL_FLOAT, colorVec);
		}

		bool isRenderable() const
		{
			if (!isValid())
				return false;

			switch (mStorageType)
			{
			case GL_R8:
			case GL_R16F:
			case GL_R32F:
			case GL_R8I:
			case GL_R8UI:
			case GL_R16I:
			case GL_R16UI:
			case GL_R32I:
			case GL_R32UI:
			
			case GL_RG8:
			case GL_RG16F:
			case GL_RG32F:
			case GL_RG8I:
			case GL_RG8UI:
			case GL_RG16I:
			case GL_RG16UI:
			case GL_RG32I:
			case GL_RG32UI:
			
			case GL_RGBA8:
			case GL_RGBA16F:
			case GL_RGBA32F:
			case GL_RGBA8I:
			case GL_RGBA8UI:
			case GL_RGBA16I:
			case GL_RGBA16UI:
			case GL_RGBA32I:
			case GL_RGBA32UI:

			case GL_DEPTH_COMPONENT16:
			case GL_DEPTH_COMPONENT24:
			case GL_DEPTH24_STENCIL8:
				return true;
			};

			return false;
		}

		bool isCompressed() const
		{
			if (!isValid())
				return false;

			switch (mStorageType)
			{
			case GL_COMPRESSED_RGB_S3TC_DXT1_EXT:
			case GL_COMPRESSED_SRGB_S3TC_DXT1_EXT:
			case GL_COMPRESSED_RGBA_S3TC_DXT5_EXT:
			case GL_COMPRESSED_SRGB_ALPHA_S3TC_DXT5_EXT:
			case GL_COMPRESSED_RED_RGTC1:
			case GL_COMPRESSED_RG_RGTC2:
				return true;
			};

			return false;
		}

		bool isType(Type type) const
		{
			if (!isValid())
				return false;

			return (mType == (Texture::translate(type)));
		}

		void genMipmaps() const
		{
			if (!isValid() || (mType == GL_TEXTURE_RECTANGLE) || isCompressed())
				return;

			glGenerateTextureMipmap(mId);
		}

		bool uploadData(GLint level, GLint xOffset, GLint width, DataFormat dataFormat, DataType dataType, const void* const data)
		{
			if (!isValid() || (mType != GL_TEXTURE_1D) || isCompressed())
				return false;

			glTextureSubImage1D(mId, level, xOffset, width, Texture::translate(dataFormat), Texture::translate(dataType), data);
			return true;
		}

		bool uploadData(GLint level, GLint xOffset, GLint yOffset, GLint width, GLint height, DataFormat dataFormat, DataType dataType, const void* const data)
		{
			if (!isValid() || ((mType != GL_TEXTURE_2D) && (mType != GL_TEXTURE_1D_ARRAY) && (mType != GL_TEXTURE_RECTANGLE)) || isCompressed())
				return false;

			glTextureSubImage2D(mId, level, xOffset, yOffset, width, height, Texture::translate(dataFormat), Texture::translate(dataType), data);
			return true;
		}

		bool uploadData(GLint level, GLint xOffset, GLint yOffset, GLint zOffset, GLint width, GLint height, GLint depth, DataFormat dataFormat, DataType dataType, const void* const data)
		{
			if (!isValid() || ((mType != GL_TEXTURE_3D) && (mType != GL_TEXTURE_2D_ARRAY)) || isCompressed())
				return false;

			glTextureSubImage3D(mId, level, xOffset, yOffset, zOffset, width, height, depth, Texture::translate(dataFormat), Texture::translate(dataType), data);
			return true;
		}

		bool uploadCubemapData(CubemapFace face, GLint level, GLint xOffset, GLint yOffset, GLint width, GLint height, DataFormat dataFormat, DataType dataType, const void* const data)
		{
			if (!isValid() || (mType != GL_TEXTURE_CUBE_MAP) || isCompressed())
				return false;

			GLint faceIndex;
			switch (face)
			{
			case CubemapFace::PosX:
				faceIndex = 0;
			case CubemapFace::NegX:
				faceIndex = 1;
			case CubemapFace::PosY:
				faceIndex = 2;
			case CubemapFace::NegY:
				faceIndex = 3;
			case CubemapFace::PosZ:
				faceIndex = 4;
			case CubemapFace::NegZ:
				faceIndex = 5;
			default:
				return false;
			}

			glTextureSubImage3D(mId, level, xOffset, yOffset, faceIndex, width, height, 0, Texture::translate(dataFormat), Texture::translate(dataType), data);
			return true;
		}

		bool uploadCompressedData(GLint level, GLint xOffset, GLint yOffset, GLint width, GLint height, StorageType storageType, GLsizei dataSize, const void* const data)
		{
			if (!isValid() || ((mType != GL_TEXTURE_2D) && (mType != GL_TEXTURE_1D_ARRAY)) || !isCompressed())
				return false;

			auto internalStorageType = Texture::translate(storageType);

			//compress types must match with the exception of SRGB types (only the internal type is SRGB)
			auto isValid = (internalStorageType == mStorageType);
			if (!isValid)
			{
				isValid = (mStorageType == GL_COMPRESSED_SRGB_S3TC_DXT1_EXT) && (internalStorageType == GL_COMPRESSED_RGB_S3TC_DXT1_EXT);
				isValid |= (mStorageType == GL_COMPRESSED_SRGB_ALPHA_S3TC_DXT5_EXT) && (internalStorageType == GL_COMPRESSED_RGBA_S3TC_DXT5_EXT);
			}

			if (!isValid)
				return false;

			glCompressedTextureSubImage2D(mId, level, xOffset, yOffset, width, height, mStorageType, dataSize, data);
			return true;
		}
	};

	class Sampler final : public ObjectGL
	{
	public:
		enum class FilterType { Point, Linear, PointMipPoint, PointMipLinear, LinearMipPoint, LinearMipLinear };
		enum class CompareMode { CompareRefToTexture, None };
		enum class CompareFunc { LesserEqual, GreaterEqual, Lesser, Greater, Equal, NotEqual, Always, Never };
		enum class WrapType { Repeat, ClampBorder, ClampEdge };

	public:
		static void unbind(GLuint textureUnit)
		{
			glBindSampler(textureUnit, 0);
		}

	public:
		Sampler()
		{ }

		~Sampler()
		{
			reset();
		}

		void init()
		{
			if (isValid())
				return;

			glCreateSamplers(1, &mId);
		}

		void init(const FilterType magFilter, const FilterType minFilter)
		{
			if (isValid())
				return;

			init();
			setMagFilter(magFilter);
			setMinFilter(minFilter);
		}

		void init(const FilterType magFilter, const FilterType minFilter, const WrapType wrapType)
		{
			if (isValid())
				return;

			init();
			setMagFilter(magFilter);
			setMinFilter(minFilter);
			setWrap(wrapType);
		}

		void reset()
		{
			if (!isValid())
				return;

			glDeleteSamplers(1, &mId);
			mId = 0;
		}

		void bind(const GLuint textureUnit) const
		{
			if (!isValid())
				return;

			glBindSampler(textureUnit, mId);
		}

		bool setAnisotropy(const Context &mCtx, const float &anisotropyLevel) const
		{
			if (!isValid())
				return false;

			float maxAnisoLevel;
			if (!mCtx.isExtPresent(Context::ExtFilterAnisotropic) || !mCtx.info(Context::InformationType::MaxAnisotropicLevel, maxAnisoLevel))
				return false;

			hr::gl::glSamplerParameterf(mId, GL_TEXTURE_MAX_ANISOTROPY_EXT, hr::Math::fClamp(anisotropyLevel, 1.0f, maxAnisoLevel));
			return true;
		}

		void setMinFilter(const FilterType filterType) const
		{
			if (!isValid())
				return;

			switch (filterType)
			{
			case FilterType::Point:
				glSamplerParameteri(mId, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
				break;
			case FilterType::Linear:
				glSamplerParameteri(mId, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
				break;
			case FilterType::LinearMipPoint:
				glSamplerParameteri(mId, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_NEAREST);
				break;
			case FilterType::LinearMipLinear:
				glSamplerParameteri(mId, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
				break;
			case FilterType::PointMipPoint:
				glSamplerParameteri(mId, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_NEAREST);
				break;
			case FilterType::PointMipLinear:
				glSamplerParameteri(mId, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_LINEAR);
				break;
			}
		}
		void setMagFilter(const FilterType filterType) const
		{
			if (!isValid())
				return;

			switch (filterType)
			{
			case FilterType::Point:
				glSamplerParameteri(mId, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
				break;
			case FilterType::Linear:
				glSamplerParameteri(mId, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
				break;
			}
		}

		void setWrap(const WrapType wrapType) const
		{
			if (!isValid())
				return;

			switch (wrapType)
			{
			case WrapType::Repeat:
				glSamplerParameteri(mId, GL_TEXTURE_WRAP_S, GL_REPEAT);
				glSamplerParameteri(mId, GL_TEXTURE_WRAP_T, GL_REPEAT);
				glSamplerParameteri(mId, GL_TEXTURE_WRAP_R, GL_REPEAT);
				break;
			case WrapType::ClampBorder:
				glSamplerParameteri(mId, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
				glSamplerParameteri(mId, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
				glSamplerParameteri(mId, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_BORDER);
				break;
			case WrapType::ClampEdge:
				glSamplerParameteri(mId, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
				glSamplerParameteri(mId, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
				glSamplerParameteri(mId, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
				break;
			}
		}

		void setBorderColor(const float &r, const float &g, const float &b, const float &a) const
		{
			if (!isValid())
				return;

			float borderColor[] = { r, g, b, a };
			glSamplerParameterfv(mId, GL_TEXTURE_BORDER_COLOR, borderColor);
		}

		void setBorderColor(const float * const borderColor) const
		{
			if (!borderColor || !isValid())
				return;

			glSamplerParameterfv(mId, GL_TEXTURE_BORDER_COLOR, borderColor);
		}

		void setLOD(const float &minLOD, const float &maxLOD) const
		{
			if (!isValid())
				return;

			glSamplerParameterf(mId, GL_TEXTURE_MIN_LOD, minLOD);
			glSamplerParameterf(mId, GL_TEXTURE_MAX_LOD, maxLOD);
		}

		void setLODMax(const float &maxLOD) const
		{
			if (!isValid())
				return;

			glSamplerParameterf(mId, GL_TEXTURE_MAX_LOD, maxLOD);
		}

		void setLODMin(const float &minLOD) const
		{
			if (!isValid())
				return;

			glSamplerParameterf(mId, GL_TEXTURE_MIN_LOD, minLOD);
		}

		void setLODBias(const float &lodBias) const
		{
			if (!isValid())
				return;

			glSamplerParameterf(mId, GL_TEXTURE_LOD_BIAS, lodBias);
		}

		void setCompare(const CompareMode &compareMode, const CompareFunc &compareFunc) const
		{
			if (!isValid())
				return;

			switch (compareMode)
			{
			case CompareMode::CompareRefToTexture:
				glSamplerParameteri(mId, GL_TEXTURE_COMPARE_MODE, GL_COMPARE_REF_TO_TEXTURE);
				break;
			case CompareMode::None:
				glSamplerParameteri(mId, GL_TEXTURE_COMPARE_MODE, GL_NONE);
				break;
			}

			switch (compareFunc)
			{
			case CompareFunc::LesserEqual:
				glSamplerParameteri(mId, GL_TEXTURE_COMPARE_FUNC, GL_LEQUAL);
				break;
			case CompareFunc::GreaterEqual:
				glSamplerParameteri(mId, GL_TEXTURE_COMPARE_FUNC, GL_GEQUAL);
				break;
			case CompareFunc::Lesser:
				glSamplerParameteri(mId, GL_TEXTURE_COMPARE_FUNC, GL_LESS);
				break;
			case CompareFunc::Greater:
				glSamplerParameteri(mId, GL_TEXTURE_COMPARE_FUNC, GL_GREATER);
				break;
			case CompareFunc::Equal:
				glSamplerParameteri(mId, GL_TEXTURE_COMPARE_FUNC, GL_EQUAL);
				break;
			case CompareFunc::NotEqual:
				glSamplerParameteri(mId, GL_TEXTURE_COMPARE_FUNC, GL_NOTEQUAL);
				break;
			case CompareFunc::Always:
				glSamplerParameteri(mId, GL_TEXTURE_COMPARE_FUNC, GL_ALWAYS);
				break;
			case CompareFunc::Never:
				glSamplerParameteri(mId, GL_TEXTURE_COMPARE_FUNC, GL_NEVER);
				break;
			}
		}
	};

	class Buffer final : public ObjectGL
	{
	public:
		enum class Type { ArrayBuffer, ElementArrayBuffer, PixelPackBuffer, PixelUnpackBuffer, TextureBuffer, UniformBuffer, DrawIndirect };
		enum class UsageType { ServerStatic, OnlyRead, OnlyWrite, PersistentOnlyRead, PersistentOnlyWrite};

		struct DrawElementsIndirectCommand {
			GLuint count;
			GLuint instanceCount;
			GLuint firstIndex;
			GLuint baseVertex;
			GLuint baseInstance;
		};
		static_assert(sizeof(DrawElementsIndirectCommand) == 20, "DrawElementsIndirectCommand must be tightly packed: sizeof() == 20");

	private:
		GLenum mType = 0;
		void* mMappedPtr = nullptr;
		UsageType mUsageType = UsageType::ServerStatic;
		
	public:
		Buffer()
		{ }

		~Buffer()
		{
			reset();
		}

		bool init(Type type, size_t datasize, UsageType usageType)
		{
			return init(type, nullptr, datasize, usageType);
		}

		bool init(Type type, const void* const dataPtr, size_t dataSize, UsageType usageType)
		{
			if (isValid() || (dataSize <= 0))
				return false;

			switch (usageType)
			{
			case UsageType::ServerStatic:
				glCreateBuffers(1, &mId);
				glNamedBufferStorage(mId, dataSize, dataPtr, 0);
				break;
			case UsageType::OnlyRead:
				glCreateBuffers(1, &mId);
				glNamedBufferStorage(mId, dataSize, dataPtr, GL_MAP_READ_BIT);
				break;
			case UsageType::OnlyWrite:
				glCreateBuffers(1, &mId);
				glNamedBufferStorage(mId, dataSize, dataPtr, GL_MAP_WRITE_BIT);
				break;
			case UsageType::PersistentOnlyRead:
				glCreateBuffers(1, &mId);
				glNamedBufferStorage(mId, dataSize, dataPtr, GL_MAP_READ_BIT | GL_MAP_PERSISTENT_BIT | GL_MAP_COHERENT_BIT);
				mMappedPtr = glMapNamedBufferRange(mId, 0, dataSize, GL_MAP_READ_BIT | GL_MAP_PERSISTENT_BIT | GL_MAP_COHERENT_BIT);
				break;
			case UsageType::PersistentOnlyWrite:
				glCreateBuffers(1, &mId);
				glNamedBufferStorage(mId, dataSize, dataPtr, GL_MAP_WRITE_BIT | GL_MAP_PERSISTENT_BIT | GL_MAP_COHERENT_BIT);
				mMappedPtr = glMapNamedBufferRange(mId, 0, dataSize, GL_MAP_WRITE_BIT | GL_MAP_PERSISTENT_BIT | GL_MAP_COHERENT_BIT);
				break;
			default:
				return false;
			}

			switch (type)
			{
			case Type::ArrayBuffer:
				mType = GL_ARRAY_BUFFER;
				break;
			case Type::ElementArrayBuffer:
				mType = GL_ELEMENT_ARRAY_BUFFER;
				break;
			case Type::PixelPackBuffer:
				mType = GL_PIXEL_PACK_BUFFER;
				break;
			case Type::PixelUnpackBuffer:
				mType = GL_PIXEL_UNPACK_BUFFER;
				break;
			case Type::TextureBuffer:
				mType = GL_TEXTURE_BUFFER;
				break;
			case Type::UniformBuffer:
				mType = GL_UNIFORM_BUFFER;
				break;
			case Type::DrawIndirect:
				mType = GL_DRAW_INDIRECT_BUFFER;
				break;
			default:
				return false;
			}

			mUsageType = usageType;
			return true;
		}

		void reset()
		{
			if (!isValid())
				return;

			if (mMappedPtr)
			{
				glUnmapNamedBuffer(mId);
				mMappedPtr = nullptr;
			}

			glDeleteBuffers(1, &mId);
			mId = 0;

			mType = 0;
			mUsageType = UsageType::ServerStatic;
		}

		void bind() const
		{
			if (!isValid())
				return;

			glBindBuffer(mType, mId);
		}

		void unbind()
		{
			if (!isValid())
				return;

			glBindBuffer(mType, 0);
		}

		bool copyTo(const GLuint targetBufferId, const size_t bufferReadOffset, const size_t bufferWriteOffset, const size_t copyDataSize)
		{
			if (!isValid() || (targetBufferId == 0) || (copyDataSize <= 0))
				return false;

			glCopyNamedBufferSubData(mId, targetBufferId, bufferReadOffset, bufferWriteOffset, copyDataSize);
			return true;
		}

		bool clearData(const size_t bufferOffset, const size_t bufferSize, const float* dataPtr, const size_t dataNumComponents) const
		{
			if (!isValid() || (bufferSize <= 0) || !dataPtr)
				return false;

			switch (dataNumComponents)
			{
			case 0:
				glClearNamedBufferSubData(mId, GL_R32F, bufferOffset, bufferSize, GL_RED, GL_FLOAT, dataPtr);
				return true;
			case 1:
				glClearNamedBufferSubData(mId, GL_RG32F, bufferOffset, bufferSize, GL_RG, GL_FLOAT, dataPtr);
				return true;
			case 2:
				glClearNamedBufferSubData(mId, GL_RGB32F, bufferOffset, bufferSize, GL_RGB, GL_FLOAT, dataPtr);
				return true;
			}

			return false;
		}

		bool writeData(const void* const dataPtr, const size_t dataSize, const size_t bufferOffset) const
		{
			if (!dataPtr || (dataSize <= 0) || !isValid())
				return false;

			if ((mUsageType != UsageType::OnlyWrite) && (mUsageType != UsageType::PersistentOnlyWrite))
				return false;

			if (mMappedPtr)
			{
				memcpy(reinterpret_cast<unsigned char*>(mMappedPtr) + bufferOffset, dataPtr, dataSize);
			}
			else
			{
				auto mappedPtr = glMapNamedBufferRange(mId, bufferOffset, dataSize, GL_MAP_WRITE_BIT);
				memcpy(mappedPtr, dataPtr, dataSize);
				glUnmapNamedBuffer(mId);
			}

			return true;
		}

		bool writeData(std::function<void(void*, size_t)> writeOp, const size_t dataSize, const size_t bufferOffset) const
		{
			if (!writeOp || (dataSize <= 0) || !isValid())
				return false;

			if ((mUsageType != UsageType::OnlyWrite) && (mUsageType != UsageType::PersistentOnlyWrite))
				return false;

			if (mMappedPtr)
			{
				writeOp(reinterpret_cast<unsigned char*>(mMappedPtr) + bufferOffset, dataSize);
			}
			else
			{
				auto mappedPtr = glMapNamedBufferRange(mId, bufferOffset, dataSize, GL_MAP_WRITE_BIT);
				writeOp(mappedPtr, dataSize);
				glUnmapNamedBuffer(mId);
			}

			return true;
		}

		bool readData(const size_t bufferOffset, const size_t dataSize, void* const dataPtr) const
		{
			if (!dataPtr || (dataSize <= 0) || !isValid())
				return false;

			if ((mUsageType != UsageType::OnlyRead) && (mUsageType != UsageType::PersistentOnlyRead))
				return false;

			if (mMappedPtr)
			{
				memcpy(dataPtr, reinterpret_cast<unsigned char*>(mMappedPtr) + bufferOffset, dataSize);
			}
			else
			{
				auto mappedPtr = glMapNamedBufferRange(mId, bufferOffset, dataSize, GL_MAP_READ_BIT);
				memcpy(dataPtr, mappedPtr, dataSize);
				glUnmapNamedBuffer(mId);
			}

			return true;
		}
	};

	class Query final : public ObjectGL
	{
	public:
		enum class Type {
			SamplesPassed, AnySamplePassed, AnySamplePassedConservative, TimeElapsed,
			VerticesSubmitted, PrimitivesSubmitted, VertexShaderInvocations, FragmentShaderInvocations, ClippingInputPrimitives, ClippingOutputPrimitives
		};

		template<int N>
		class Group
		{
			static_assert(N > 0, "A query group must have at least one query");

			Query mQueries[N];

		public:
			Group(std::initializer_list<Type> types)
			{
				assert(types.size() == N);

				int index = 0;
				for (const auto& queryType : types)
					mQueries[index++].init(queryType);
			}

			Group(const Group&) = delete;
			Group& operator=(const Group&) = delete;

			void queriesBegin() const
			{
				for (int i = 0; i < N; i++)
					mQueries[i].queryBegin();
			}

			void queriesEnd() const
			{
				for (int i = 0; i < N; i++)
					mQueries[i].queryEnd();
			}

			template<int Index>
			GLuint getResult(bool wait = true) const
			{
				static_assert((Index >= 0) && (Index < N), "Query index must be positive and less than N");

				return mQueries[Index].getResult(wait);
			}

			template<int Index>
			GLuint64 getResultI64(bool wait = true) const
			{
				static_assert((Index >= 0) && (Index < N), "Query index must be positive and less than N");

				return mQueries[Index].getResultI64(wait);
			}
		};

	private:
		Type mType = Type::SamplesPassed;

	public:
		Query()
		{ }

		~Query()
		{
			reset();
		}

		void init(Type queryType = Type::SamplesPassed)
		{
			if (isValid())
				return;

			mType = queryType;
			glGenQueries(1, &mId);
		}

		void reset()
		{
			if (!isValid())
				return;

			glDeleteQueries(1, &mId);
			mId = 0;

			mType = Type::SamplesPassed;
		}

		void queryBegin() const
		{
			if (!isValid())
				return;

			switch (mType)
			{
			case Type::SamplesPassed:
				glBeginQuery(GL_SAMPLES_PASSED, mId);
				break;
			case Type::AnySamplePassed:
				glBeginQuery(GL_ANY_SAMPLES_PASSED, mId);
				break;
			case Type::AnySamplePassedConservative:
				glBeginQuery(GL_ANY_SAMPLES_PASSED_CONSERVATIVE, mId);
				break;
			case Type::TimeElapsed:
				glBeginQuery(GL_TIME_ELAPSED, mId);
				break;

			case Type::VerticesSubmitted:
				glBeginQuery(GL_VERTICES_SUBMITTED_ARB, mId);
				break;
			case Type::PrimitivesSubmitted:
				glBeginQuery(GL_PRIMITIVES_SUBMITTED_ARB, mId);
				break;
			case Type::VertexShaderInvocations:
				glBeginQuery(GL_VERTEX_SHADER_INVOCATIONS_ARB, mId);
				break;
			case Type::FragmentShaderInvocations:
				glBeginQuery(GL_FRAGMENT_SHADER_INVOCATIONS_ARB, mId);
				break;
			case Type::ClippingInputPrimitives:
				glBeginQuery(GL_CLIPPING_INPUT_PRIMITIVES_ARB, mId);
				break;
			case Type::ClippingOutputPrimitives:
				glBeginQuery(GL_CLIPPING_OUTPUT_PRIMITIVES_ARB, mId);
				break;
			}
		}

		void queryEnd() const
		{
			if (!isValid())
				return;

			switch (mType)
			{
			case Type::SamplesPassed:
				glEndQuery(GL_SAMPLES_PASSED);
				break;
			case Type::AnySamplePassed:
				glEndQuery(GL_ANY_SAMPLES_PASSED);
				break;
			case Type::AnySamplePassedConservative:
				glEndQuery(GL_ANY_SAMPLES_PASSED_CONSERVATIVE);
				break;
			case Type::TimeElapsed:
				glEndQuery(GL_TIME_ELAPSED);
				break;

			case Type::VerticesSubmitted:
				glEndQuery(GL_VERTICES_SUBMITTED_ARB);
				break;
			case Type::PrimitivesSubmitted:
				glEndQuery(GL_PRIMITIVES_SUBMITTED_ARB);
				break;
			case Type::VertexShaderInvocations:
				glEndQuery(GL_VERTEX_SHADER_INVOCATIONS_ARB);
				break;
			case Type::FragmentShaderInvocations:
				glEndQuery(GL_FRAGMENT_SHADER_INVOCATIONS_ARB);
				break;
			case Type::ClippingInputPrimitives:
				glEndQuery(GL_CLIPPING_INPUT_PRIMITIVES_ARB);
				break;
			case Type::ClippingOutputPrimitives:
				glEndQuery(GL_CLIPPING_OUTPUT_PRIMITIVES_ARB);
				break;
			}
		}

		GLuint getResult(bool wait = true) const
		{
			if (!isValid())
				return 0;

			GLuint queryResult = 0;

			glGetQueryObjectuiv(mId, wait ? GL_QUERY_RESULT : GL_QUERY_RESULT_NO_WAIT, &queryResult);
			return queryResult;
		}

		GLuint64 getResultI64(bool wait = true) const
		{
			if (!isValid())
				return 0;

			GLuint64 queryResult = 0;

			glGetQueryObjectui64v(mId, wait ? GL_QUERY_RESULT : GL_QUERY_RESULT_NO_WAIT, &queryResult);
			return queryResult;
		}

		bool isResultAvailable() const
		{
			if (!isValid())
				return 0;

			GLint queryResult;

			glGetQueryObjectiv(mId, GL_QUERY_RESULT_AVAILABLE, &queryResult);
			return (queryResult != 0);
		}
	};

	class QueryCounter final : public ObjectGL
	{
	public:
		enum class Type { Timestamp };

	private:
		Type mType = Type::Timestamp;

	public:
		QueryCounter()
		{ }

		~QueryCounter()
		{
			reset();
		}

		void init(Type queryType = Type::Timestamp)
		{
			if (isValid())
				return;

			mType = queryType;
			glGenQueries(1, &mId);
		}

		void reset()
		{
			if (!isValid())
				return;

			glDeleteQueries(1, &mId);
			mId = 0;

			mType = Type::Timestamp;
		}

		void query() const
		{
			if (!isValid())
				return;

			switch (mType)
			{
			case Type::Timestamp:
				glQueryCounter(mId, GL_TIMESTAMP);
				break;
			}
		}

		GLuint64 getResultI64(bool wait = true) const
		{
			if (!isValid())
				return 0;

			GLuint64 queryResult = 0;

			glGetQueryObjectui64v(mId, wait ? GL_QUERY_RESULT : GL_QUERY_RESULT_NO_WAIT, &queryResult);
			return queryResult;
		}

		bool isResultAvailable() const
		{
			if (!isValid())
				return 0;

			GLint queryResult;

			glGetQueryObjectiv(mId, GL_QUERY_RESULT_AVAILABLE, &queryResult);
			return (queryResult != 0);
		}
	};

	class VertexArray final : public ObjectGL
	{
	public:
		VertexArray()
		{ }

		~VertexArray()
		{
			reset();
		}

		void init()
		{
			if (isValid())
				return;

			glCreateVertexArrays(1, &mId);
		}

		void reset()
		{
			if (!isValid())
				return;

			glDeleteVertexArrays(1, &mId);
			mId = 0;
		}

		void bind() const
		{
			if (!isValid())
				return;

			glBindVertexArray(mId);
		}
	};

	class FrameBuffer final : public ObjectGL
	{
	public:
		enum class Status { Complete, Unsupported, Incomplete, Unknown };

	public:
		static void unbind()
		{
			glBindFramebuffer(GL_FRAMEBUFFER, 0);
		}

	public:
		FrameBuffer()
		{ }

		~FrameBuffer()
		{
			reset();
		}

		void init()
		{
			if (isValid())
				return;

			glCreateFramebuffers(1, &mId);
		}

		void reset()
		{
			if (!isValid())
				return;

			glDeleteFramebuffers(1, &mId);
			mId = 0;
		}

		void bind() const
		{
			glBindFramebuffer(GL_FRAMEBUFFER, mId);
		}

		bool isStatusComplete() const
		{
			return (getStatus() == Status::Complete);
		}

		Status getStatus() const
		{
			switch (glCheckNamedFramebufferStatus(mId, GL_FRAMEBUFFER))
			{
			case GL_FRAMEBUFFER_COMPLETE:
				return Status::Complete;

			case GL_FRAMEBUFFER_UNSUPPORTED:
				return Status::Unsupported;

			case GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT:
			case GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT:
			case GL_FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER:
			case GL_FRAMEBUFFER_INCOMPLETE_READ_BUFFER:
			case GL_FRAMEBUFFER_INCOMPLETE_LAYER_TARGETS:
				return Status::Incomplete;
			}

			return Status::Unknown;
		}

		template<int NBuffers>
		void drawBuffers(std::array<GLenum, NBuffers> buffers)
		{
			hr::gl::glNamedFramebufferDrawBuffers(mId, NBuffers, buffers.data());
		}

		const FrameBuffer& attachTColor(const Texture& textureToAttach, GLuint attachUnit) const
		{
			if (!isValid() || !textureToAttach.isRenderable() || (!textureToAttach.isType(Texture::Type::Tex2D) && !textureToAttach.isType(Texture::Type::TexRectangle)))
				return *this;

			hr::gl::glNamedFramebufferTexture(mId, GL_COLOR_ATTACHMENT0 + attachUnit, textureToAttach.getId(), 0);

			return *this;
		}

		const FrameBuffer& attachTColor(const Texture& textureToAttach, GLuint attachUnit, Texture::CubemapFace cubemapFace) const
		{
			if (!isValid() || !textureToAttach.isRenderable() || !textureToAttach.isType(Texture::Type::TexCubemap))
				return *this;

			switch (cubemapFace)
			{
			case Texture::CubemapFace::PosX:
				hr::gl::glNamedFramebufferTextureLayer(mId, GL_COLOR_ATTACHMENT0 + attachUnit, textureToAttach.getId(), 0, GL_TEXTURE_CUBE_MAP_POSITIVE_X);
			case Texture::CubemapFace::NegX:
				hr::gl::glNamedFramebufferTextureLayer(mId, GL_COLOR_ATTACHMENT0 + attachUnit, textureToAttach.getId(), 0, GL_TEXTURE_CUBE_MAP_NEGATIVE_X);
			case Texture::CubemapFace::PosY:
				hr::gl::glNamedFramebufferTextureLayer(mId, GL_COLOR_ATTACHMENT0 + attachUnit, textureToAttach.getId(), 0, GL_TEXTURE_CUBE_MAP_POSITIVE_Y);
			case Texture::CubemapFace::NegY:
				hr::gl::glNamedFramebufferTextureLayer(mId, GL_COLOR_ATTACHMENT0 + attachUnit, textureToAttach.getId(), 0, GL_TEXTURE_CUBE_MAP_NEGATIVE_Y);
			case Texture::CubemapFace::PosZ:
				hr::gl::glNamedFramebufferTextureLayer(mId, GL_COLOR_ATTACHMENT0 + attachUnit, textureToAttach.getId(), 0, GL_TEXTURE_CUBE_MAP_POSITIVE_Z);
			case Texture::CubemapFace::NegZ:
				hr::gl::glNamedFramebufferTextureLayer(mId, GL_COLOR_ATTACHMENT0 + attachUnit, textureToAttach.getId(), 0, GL_TEXTURE_CUBE_MAP_NEGATIVE_Z);
			}
			
			return *this;
		}

		const FrameBuffer& attachTDepth(const Texture& textureToAttach) const
		{
			if (!isValid() || !textureToAttach.isRenderable() || (!textureToAttach.isType(Texture::Type::Tex2D) && !textureToAttach.isType(Texture::Type::TexRectangle)))
				return *this;

			hr::gl::glNamedFramebufferTexture(mId, GL_DEPTH_ATTACHMENT, textureToAttach.getId(), 0);

			return *this;
		}
	};

	class ShaderProgram final : public ObjectGL
	{
		friend class ProgramPipeline;

		GLenum mType = 0;

	public:
		enum class Type{ Vertex, Fragment };

		ShaderProgram()
		{ }

		~ShaderProgram()
		{
			reset();
		}

		bool init(Type type, const std::string& sourceCode)
		{
			if (sourceCode.empty())
				return false;

			return init(type, sourceCode.c_str());
		}

		bool init(Type type, const char* const sourceCode)
		{
			if (!sourceCode || (sourceCode[0] == '\0') || isValid())
				return false;

			switch (type)
			{
			case Type::Vertex:
			{
				mType = GL_VERTEX_SHADER;
				mId = glCreateShaderProgramv(GL_VERTEX_SHADER, 1, &sourceCode);

				break;
			}
			case Type::Fragment:
			{
				mType = GL_FRAGMENT_SHADER;
				mId = glCreateShaderProgramv(GL_FRAGMENT_SHADER, 1, &sourceCode);

				break;
			}
			default:
				return false;
			}

			GLint isLinked = 0;
			glGetProgramiv(mId, GL_LINK_STATUS, &isLinked);

			return (isLinked == GL_FALSE);
		}

		void reset()
		{
			if (!isValid())
				return;

			glDeleteProgram(mId);
			mId = 0;
		}

		std::string getInfoLog() const
		{
			if (!isValid())
				return std::string();

			GLint maxLength = 0;
			glGetProgramiv(mId, GL_INFO_LOG_LENGTH, &maxLength);
			if (maxLength <= 0)
				return std::string();

			std::vector<GLchar> infoLog(maxLength);
			glGetProgramInfoLog(mId, maxLength, &maxLength, infoLog.data());

			return std::string(&infoLog[0]);
		}

		GLint getUniformLocation(const char* const uniformName)
		{
			if (!isValid())
				return -1;

			return glGetUniformLocation(mId, uniformName);
		}
	};

	class ProgramPipeline final : public ObjectGL
	{
	public:
		ProgramPipeline()
		{
		}

		~ProgramPipeline()
		{
			reset();
		}

		void init()
		{
			if (isValid())
				return;

			glGenProgramPipelines(1, &mId);
		}

		void reset()
		{
			if (!isValid())
				return;

			glDeleteProgramPipelines(1, &mId);
			mId = 0;
		}

		bool setStage(const ShaderProgram &program)
		{
			if (!isValid() || !program.isValid())
				return false;

			GLbitfield stageMask = 0;
			switch (program.mType)
			{
			case GL_VERTEX_SHADER:
				stageMask |= GL_VERTEX_SHADER_BIT;
				break;
			case GL_FRAGMENT_SHADER:
				stageMask |= GL_FRAGMENT_SHADER_BIT;
			}
			
			if (stageMask != 0)
				glUseProgramStages(mId, stageMask, program.mId);

			return true;
		}

		std::string getInfoLog() const
		{
			if (!isValid())
				return std::string();

			GLint maxLength = 0;
			glGetProgramPipelineiv(mId, GL_INFO_LOG_LENGTH, &maxLength);
			if (maxLength <= 0)
				return std::string();

			std::vector<GLchar> infoLog(maxLength);
			glGetProgramPipelineInfoLog(mId, maxLength, &maxLength, infoLog.data());

			return std::string(&infoLog[0]);
		}
	};

	//a glFenceSync is not considered a GL object
	class FenceSync
	{
		GLsync mSync = 0;

	public:
		FenceSync()
		{ }

		~FenceSync()
		{
			glDeleteSync(mSync);
			mSync = 0;
		}

		void place()
		{
			glDeleteSync(mSync);
			mSync = glFenceSync(GL_SYNC_GPU_COMMANDS_COMPLETE, 0);
		}

		void wait() const
		{
			if (!mSync)
				return;

			while (true)
			{
				switch (glClientWaitSync(mSync, GL_SYNC_FLUSH_COMMANDS_BIT, 1))
				{
				case GL_ALREADY_SIGNALED:
				case GL_CONDITION_SATISFIED:
					return;
				}
			}
		}
	};
} } }
