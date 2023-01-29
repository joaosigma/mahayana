#pragma once

#include <string>
#include <optional>

namespace hr::gl::objects
{
	class Context
	{
	public:
		enum Extensions { ExtCompressionS3 = (1 << 1), ExtCompressionVTC = (1 << 2) };
		enum class InformationType { Version, Vendor, Renderer, GLSLVersion, MaxAnisotropicLevel, MaxDrawBuffers, MaxColorAttachments, MaxTextureSize, MaxTexture3DSize, MaxTextureCubemapSize, MaxTextureRectSize };

	private:
		struct Info {
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
		bool isExtPresent(const char* const extensionName) const;
		void dispatchDebugMessages() const;

		std::optional<int> infoInt(const InformationType& informationType) const;
		std::optional<float> infoFloat(const InformationType& informationType) const;
		std::optional<std::string> infoStr(const InformationType& informationType) const;
	};
}
