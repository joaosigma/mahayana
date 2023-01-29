#include "context.hpp"

#include "openGL.hpp"
#include "openGLext.hpp"

#include <memory>

namespace hr::gl::objects
{
	bool Context::initContext()
	{
		if (!hr::gl::OpenGLGetProcs())
			return false;

		hr::gl::extensions::extensionsLoad("OpenGL32.dll");

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
		hr::gl::glGetFloatv(GL_MAX_TEXTURE_MAX_ANISOTROPY, &mInfo.maxAnisotropy);

		return true;
	}

	bool Context::isExtPresent(Context::Extensions extension) const
	{
		return ((mExtsAvailable & extension) == extension);
	}

	bool Context::isExtPresent(const char* const extensionName) const
	{
		return hr::gl::extensions::extensionExists(extensionName);
	}

	void Context::dispatchDebugMessages() const
	{
		GLint numMsgsLogged;

		hr::gl::glGetIntegerv(GL_DEBUG_LOGGED_MESSAGES, &numMsgsLogged);
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

	std::optional<int> Context::infoInt(const InformationType& informationType) const
	{
		switch (informationType)
		{
		case InformationType::MaxDrawBuffers:
			return mInfo.maxDrawBuffers;
		case InformationType::MaxColorAttachments:
			return mInfo.maxColorAttachments;
		case InformationType::MaxTextureSize:
			return mInfo.maxTextureSize;
		case InformationType::MaxTexture3DSize:
			return mInfo.maxTexture3DSize;
		case InformationType::MaxTextureCubemapSize:
			return mInfo.maxTextureCubemapSize;
		case InformationType::MaxTextureRectSize:
			return mInfo.maxTextureRectSize;
		}

		return {};
	}

	std::optional<float> Context::infoFloat(const InformationType& informationType) const
	{
		switch (informationType)
		{
		case InformationType::MaxAnisotropicLevel:
			return mInfo.maxAnisotropy;
		}

		return {};
	}

	std::optional<std::string> Context::infoStr(const InformationType& informationType) const
	{
		switch (informationType)
		{
		case InformationType::Version:
			return mInfo.version;
		case InformationType::Vendor:
			return mInfo.vendor;
		case InformationType::Renderer:
			return mInfo.renderer;
		case InformationType::GLSLVersion:
			return mInfo.glslVersion;
		}

		return {};
	}
}
