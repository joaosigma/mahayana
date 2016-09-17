#pragma once

#include "common\opengl\openGL.hpp"
#include "common\opengl\objects.hpp"
#include "common\Color.hpp"
#include "common\stringUtils.hpp"

#include <array>
#include <vector>
#include <unordered_map>

namespace HorseRadish { namespace Render { namespace Tools {

class Font
{
	struct VertexDataLayout
	{
		float pos[2], uv[2];
		unsigned char color[4];
	};

	struct KerningData
	{
		unsigned short char1 = 0, char2 = 0;
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
		const KerningData *kernData = nullptr;
	};

	struct UnicodeRange
	{
		int numCharsMax = -1;
		size_t numCharsSkip = 0;

		UnicodeRange()
		{ }

		UnicodeRange(int numCharsSkip)
			: numCharsSkip(numCharsSkip)
		{ }

		UnicodeRange(unsigned int numCharsSkip, int numCharsMax)
			: numCharsSkip(numCharsSkip), numCharsMax(numCharsMax)
		{ }
	};

	static constexpr size_t sMumMaxChar = 256;
	static constexpr unsigned short sBufferPadding = 5;

private:
	bool mValid = false;
	std::vector<KerningData> mKerningData;
	std::unordered_map<unsigned short, CharacterData> mCharMap;
	
	unsigned int mGlVertexProgramID, mGlFragmentProgramID, mGlProgramPipelineID, mGlUniformSampler, mGlUniformMatrix;
	
	struct
	{
		HorseRadish::OpenGL::Objects::Texture texture;
		HorseRadish::OpenGL::Objects::Sampler sampler;
		HorseRadish::OpenGL::Objects::VertexArray vertexArray;
		HorseRadish::OpenGL::Objects::Buffer arrayBuffer, elementArrayBuffer;
		HorseRadish::OpenGL::Objects::FenceSync fence;
	} mGl;

	struct
	{
		float scale = 1.0f;
		bool paintStarted = false;
		HorseRadish::Color stateColor;

		size_t numCharWritten = 0;
		std::array<VertexDataLayout, Font::sMumMaxChar * 4> charData;
	} mState;

	struct
	{
		size_t size = 0;
		float maxHeight = 0.0f, baseHeight = 0.0f;
	} mFontInfo;

	void commitGL();
	bool createCharData();
	float getCharKerning(const CharacterData& leftCharData, unsigned short leftCharUnicodeID, unsigned short rightCharUnicodeID) const;
	bool initFont(const char * const fontFilePath);
	void internalWrite(const float &px, const float &py, const std::string& str, UnicodeRange strRange);

public:
	Font(const size_t fontSize, const char * const fontFilePath, unsigned int glVertexProgramID, unsigned int glFragmentProgramID, unsigned int glProgramPipelineID);
	~Font();

	void layout(const std::string& text, const float maxWidth, std::function<void(size_t curLine, size_t unicodeCharOffset, size_t unicodeCharCount)> writeCb) const;

	void write(const std::string& text);
	void write(const float &px, const float &py, const std::string& text);
	void write(const float &px, const float &py, const std::string& text, const size_t numUnicodeCharsSkip);
	void write(const float &px, const float &py, const std::string& text, const size_t numUnicodeCharsSkip, const size_t maxUnicodeCharsWrite);

	float writeChar(const unsigned int &unicodeChar);
	float writeChar(const float &px, const float &py, const unsigned int &unicodeChar);

	void setColor(const float &r, const float &g, const float &b, const float &a);
	void setColor(const float &r, const float &g, const float &b);
	void setColor(const float * const color);
	void setColor(const Color &color);

	bool isValid() const;

	float getMaxHeight() const;
	float getCharWidth(const unsigned int &unicodeChar) const;

	size_t countUnicodeChars(const std::string& text, const float maxWidth) const;
	size_t countUnicodeChars(const std::string& text, const size_t numUnicodeCharsSkip, const float maxWidth) const;

	float getTextWidth(const std::string& text) const;
	float getTextWidth(const std::string&, const size_t numUnicodeCharsSkip, const size_t maxUnicodeCharsRead) const;

	void paintBegin(const float * const tranformationMatrix, float scale = 1.0f);
	void paintEnd();
	void paintFlush();
};

} } }

