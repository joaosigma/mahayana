#pragma once
#ifndef __HOPENGL_CONTEXT__
#define __HOPENGL_CONTEXT__

#include <string>

namespace HorseRadish { namespace OpenGL { namespace Objects {

class Context
{
public:
	enum Extensions { ExtFilterAnisotropic = (1 << 0), ExtCompressionS3 = (1 << 1), ExtCompressionVTC = (1 << 2) };
	enum class InformationType { Version, Vendor, Renderer, GLSLVersion, MaxAnisotropicLevel, MaxDrawBuffers, MaxColorAttachments, MaxTextureSize, MaxTexture3DSize, MaxTextureCubemapSize, MaxTextureRectSize };

private:
	struct Info{
		std::string version, vendor, renderer, glslVersion;
		int versionMajor = 0, versionMinor = 0;
		int maxDrawBuffers = 0, maxColorAttachments = 0, maxTextureSize = 0, maxTexture3DSize = 0, maxTextureCubemapSize = 0, maxTextureRectSize = 0;
		float maxAnisotropy = 0.0f;
	} mInfo;

	int mExtsAvailable = 0;

protected:
	Context()
	{ }

	bool initContext();

public:
	virtual ~Context()
	{ }

	bool isExtPresent(Extensions extension) const;
	bool isExtPresent(const char * const extensionName) const;
	void dispatchDebugMessages() const;

	bool info(const InformationType &informationType, int &infoValue) const;
	bool info(const InformationType &informationType, float &infoValue) const;
	bool info(const InformationType &informationType, std::string &infoValue) const;
};

} } }

#endif