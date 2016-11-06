#include "context.hpp"

#include "openGL.hpp"
#include "openGLext.hpp"

#include <memory>

namespace hr { namespace gl { namespace objects
{
	bool Context::initContext()
	{
		if (!hr::gl::OpenGLGetProcs())
			return false;

		hr::gl::extensions::extensionsLoad("OpenGL32.dll");

		if (hr::gl::extensions::extensionExists("GL_EXT_texture_filter_anisotropic"))
			mExtsAvailable |= Context::ExtFilterAnisotropic;
		if (hr::gl::extensions::extensionExists("GL_EXT_texture_compression_s3tc"))
			mExtsAvailable |= Context::ExtCompressionS3;
		if (hr::gl::extensions::extensionExists("GL_NV_texture_compression_vtc"))
			mExtsAvailable |= Context::ExtCompressionVTC;

		mInfo.glslVersion = reinterpret_cast<const char*>(hr::gl::glGetString(GL_SHADING_LANGUAGE_VERSION));
		mInfo.renderer = reinterpret_cast<const char*>(hr::gl::glGetString(GL_RENDERER));
		mInfo.vendor = reinterpret_cast<const char*>(hr::gl::glGetString(GL_VENDOR));
		mInfo.version = reinterpret_cast<const char*>(hr::gl::glGetString(GL_VERSION));

		hr::gl::glGetIntegerv(GL_MAX_DRAW_BUFFERS, &mInfo.maxDrawBuffers);
		hr::gl::glGetIntegerv(GL_MAX_COLOR_ATTACHMENTS, &mInfo.maxColorAttachments);
		hr::gl::glGetIntegerv(GL_MAX_TEXTURE_SIZE, &mInfo.maxTextureSize);
		hr::gl::glGetIntegerv(GL_MAX_3D_TEXTURE_SIZE, &mInfo.maxTexture3DSize);
		hr::gl::glGetIntegerv(GL_MAX_CUBE_MAP_TEXTURE_SIZE, &mInfo.maxTextureCubemapSize);
		hr::gl::glGetIntegerv(GL_MAX_RECTANGLE_TEXTURE_SIZE, &mInfo.maxTextureRectSize);
		hr::gl::glGetIntegerv(GL_MAJOR_VERSION, &mInfo.versionMajor);
		hr::gl::glGetIntegerv(GL_MINOR_VERSION, &mInfo.versionMinor);
		mInfo.maxAnisotropy = 0.0f;

		if ((mExtsAvailable & Context::ExtFilterAnisotropic) != 0)
			hr::gl::glGetFloatv(GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT, &mInfo.maxAnisotropy);

		return true;
	}

	bool Context::isExtPresent(Context::Extensions extension) const
	{
		return ((mExtsAvailable & extension) == extension);
	}

	bool Context::isExtPresent(const char * const extensionName) const
	{
		return hr::gl::extensions::extensionExists(extensionName);
	}

	void Context::dispatchDebugMessages() const
	{
		GLint numMsgsLogged;

		hr::gl::glGetIntegerv(GL_DEBUG_LOGGED_MESSAGES_ARB, &numMsgsLogged);
		if (numMsgsLogged <= 0)
			return;

		auto messageLogSize = sizeof(GLchar) * 5 * 1024;
		auto messageLog = std::unique_ptr<GLchar>(new GLchar[messageLogSize]);

		while (numMsgsLogged > 0)
		{
			GLsizei listLengths[5];
			GLenum listSources[5], listTypes[5], listIDs[5], listSeverities[5];

			auto messagesRead = hr::gl::glGetDebugMessageLog(5, messageLogSize, listSources, listTypes, listIDs, listSeverities, listLengths, messageLog.get());

			numMsgsLogged -= messagesRead;

			auto messageLogWalker = messageLog.get();
			for (GLuint i = 0; i < messagesRead; i++)
			{
				hr::gl::glDebugMessageInsert(listSources[i], listTypes[i], listIDs[i], listSeverities[i], listLengths[i], messageLogWalker);

				messageLogWalker += listLengths[i];
			}
		}
	}

	bool Context::info(const InformationType &informationType, int &infoValue) const
	{
		switch (informationType)
		{
		case InformationType::MaxDrawBuffers:
			infoValue = mInfo.maxDrawBuffers;
			return true;
		case InformationType::MaxColorAttachments:
			infoValue = mInfo.maxColorAttachments;
			return true;
		case InformationType::MaxTextureSize:
			infoValue = mInfo.maxTextureSize;
			return true;
		case InformationType::MaxTexture3DSize:
			infoValue = mInfo.maxTexture3DSize;
			return true;
		case InformationType::MaxTextureCubemapSize:
			infoValue = mInfo.maxTextureCubemapSize;
			return true;
		case InformationType::MaxTextureRectSize:
			infoValue = mInfo.maxTextureRectSize;
			return true;
		}

		infoValue = 0;
		return false;
	}

	bool Context::info(const InformationType &informationType, float &infoValue) const
	{
		switch (informationType)
		{
		case InformationType::MaxAnisotropicLevel:
			infoValue = mInfo.maxAnisotropy;
			return true;
		}

		infoValue = 0.0f;
		return false;
	}

	bool Context::info(const InformationType &informationType, std::string &infoValue) const
	{
		switch (informationType)
		{
		case InformationType::Version:
			infoValue = mInfo.version;
			return true;
		case InformationType::Vendor:
			infoValue = mInfo.vendor;
			return true;
		case InformationType::Renderer:
			infoValue = mInfo.renderer;
			return true;
		case InformationType::GLSLVersion:
			infoValue = mInfo.glslVersion;
			return true;
		}

		infoValue.clear();
		return false;
	}
} } }
