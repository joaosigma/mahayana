#pragma once

#include "common/opengl/openGL.hpp"
#include "common/opengl/objects.hpp"
#include "common/color.hpp"
#include "common/stringUtils.hpp"
#include "libs/sparsepp/spp.h"

#include <array>
#include <vector>
#include <unordered_map>

namespace hr { namespace render { namespace tools
{
	class Font
	{
		struct VertexDataLayout
		{
			float pos[2], uv[2];
			unsigned char color[4];
		};

		struct KerningData
		{
			char32_t codepoint1 = 0, codepoint2 = 0;
			float offset = 0;
		};

		struct CharacterData
		{
			bool skipDraw = false;
			float advanceX = 0.0f;
			struct {
				float offsetX, offsetY, width, height;
				float minUV[2], maxUV[2];
			} rect;
			std::pair<size_t, size_t> kernDataIndices; //{start index, size}
		};

		struct UnicodeRange
		{
			int numCharsMax = -1;
			size_t numCharsSkip = 0;

			UnicodeRange() = default;

			UnicodeRange(int numCharsSkip)
				: numCharsSkip(numCharsSkip)
			{ }

			UnicodeRange(unsigned int numCharsSkip, int numCharsMax)
				: numCharsSkip(numCharsSkip), numCharsMax(numCharsMax)
			{ }
		};

		static constexpr size_t sMumMaxChar = 256;

	private:
		bool mValid = false;
		std::vector<KerningData> mKerningData;
		spp::sparse_hash_map<char32_t, CharacterData> mCharMap;
	
		unsigned int mGlVertexProgramID, mGlFragmentProgramID, mGlProgramPipelineID, mGlUniformSampler, mGlUniformMatrix;
	
		struct
		{
			hr::gl::objects::Texture texture;
			hr::gl::objects::Sampler sampler;
			hr::gl::objects::VertexArray vertexArray;
			hr::gl::objects::Buffer arrayBuffer, elementArrayBuffer;
			hr::gl::objects::FenceSync fence;
		} mGl;

		struct
		{
			float scale = 1.0f;
			bool paintStarted = false;
			hr::Color stateColor;

			size_t numCharWritten = 0;
			std::array<VertexDataLayout, Font::sMumMaxChar * 4> charData;
		} mState;

		struct
		{
			float maxHeight = 0.0f, baseHeight = 0.0f;
		} mFontInfo;

		void commitGL();
		bool createCharData(std::unordered_map<uint32_t, Font::CharacterData>& charMap) const;
		float getCharKerning(const CharacterData& leftCharData, char32_t rightCodepoint) const;
		bool initFont(const char * const fontFilePath);
		void internalWrite(const float &px, const float &py, const std::string& str, UnicodeRange strRange);

		size_t countUnicodeChars(float fontScale, const std::string& text, const size_t numCodepointsSkip, float maxWidth) const;

		float getTextWidth(const std::string& text) const;
		float getTextWidth(const std::string& text, const size_t numCodepointsSkip, const size_t maxCodepointsRead) const;

	public:
		Font(const char * const fontFilePath, unsigned int glVertexProgramID, unsigned int glFragmentProgramID, unsigned int glProgramPipelineID);
		~Font();

		bool isValid() const;

		float getMaxHeight(size_t targetSize) const;

		float getTextWidth(size_t targetSize, const std::string& text) const;
		float getTextWidth(size_t targetSize, const std::string& text, const size_t numCodepointsSkip, const size_t maxCodepointsRead) const;

		//these next methods interact with the paint context (e.g.: font size)

		void paintBegin(size_t targetSize, const float * const tranformationMatrix);
		void paintEnd();
		void paintFlush();

		void layout(const std::string& text, const float maxWidth, std::function<void(size_t curLine, size_t unicodeCharOffset, size_t unicodeCharCount)> writeCb) const;

		void write(const std::string& text);
		void write(const float &px, const float &py, const std::string& text);
		void write(const float &px, const float &py, const std::string& text, const size_t numCodepointsSkip);
		void write(const float &px, const float &py, const std::string& text, const size_t numCodepointsSkip, const size_t maxCodepointsRead);

		float writeChar(const char32_t codepoint);
		float writeChar(const float px, const float py, const char32_t codepoint);

		void setColor(const float r, const float g, const float b, const float a);
		void setColor(const float r, const float g, const float b);
		void setColor(const float * const color);
		void setColor(const Color &color);
	};

} } }

