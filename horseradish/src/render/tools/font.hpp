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
		unsigned short char1, char2;
		float offset;

		KerningData()
			: char1(0), char2(0), offset(0)
		{ }
	};

	struct CharacterData
	{
		bool skipDraw;
		float advanceX;
		struct {
			float offsetX, offsetY, width, height;
			float minUV[2], maxUV[2];
		} rect;
		const KerningData *kernData;

		CharacterData()
			: skipDraw(true), advanceX(0.0f), kernData(nullptr)
		{ }
	};

	struct UnicodeRange
	{
		int numCharsMax;
		unsigned int numCharsSkip;

		UnicodeRange()
			: numCharsSkip(0), numCharsMax(-1)
		{ }

		UnicodeRange(int numCharsSkip)
			: numCharsSkip(numCharsSkip), numCharsMax(-1)
		{ }

		UnicodeRange(unsigned int numCharsSkip, int numCharsMax)
			: numCharsSkip(numCharsSkip), numCharsMax(numCharsMax)
		{ }
	};

	static constexpr unsigned int sMumMaxChar = 256;
	static constexpr unsigned short sBufferPadding = 5;

private:
	bool mValid;
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
		float scale;
		bool paintStarted;
		HorseRadish::Color stateColor;

		unsigned int numCharWritten;
		std::array<VertexDataLayout, Font::sMumMaxChar * 4> charData;
	} mState;

	struct
	{
		unsigned short size;
		float maxHeight, baseHeight;
	} mFontInfo;

	void commitGL();
	bool createCharData();
	float getCharKerning(const CharacterData& leftCharData, unsigned short leftCharUnicodeID, unsigned short rightCharUnicodeID) const;
	bool initFont(const char * const fontFilePath);
	void internalWrite(const float &px, const float &py, const std::string& str, UnicodeRange strRange);

public:
	Font(const int fontSize, const char * const fontFilePath, unsigned int glVertexProgramID, unsigned int glFragmentProgramID, unsigned int glProgramPipelineID);
	~Font();

	void layout(const std::string& text, const float maxWidth, std::function<void(unsigned int curLine, unsigned int unicodeCharOffset, unsigned int unicodeCharCount)> writeCb) const;

	void write(const std::string& text);
	void write(const float &px, const float &py, const std::string& text);
	void write(const float &px, const float &py, const std::string& text, const unsigned int numUnicodeCharsSkip);
	void write(const float &px, const float &py, const std::string& text, const unsigned int numUnicodeCharsSkip, const unsigned int maxUnicodeCharsWrite);

	float writeChar(const unsigned int &unicodeChar);
	float writeChar(const float &px, const float &py, const unsigned int &unicodeChar);

	void setColor(const float &r, const float &g, const float &b, const float &a);
	void setColor(const float &r, const float &g, const float &b);
	void setColor(const float * const color);
	void setColor(const Color &color);

	bool isValid() const;

	float getMaxHeight() const;
	float getCharWidth(const unsigned int &unicodeChar) const;

	unsigned int countUnicodeChars(const std::string& text, const float maxWidth) const;
	unsigned int countUnicodeChars(const std::string& text, const unsigned int numUnicodeCharsSkip, const float maxWidth) const;

	float getTextWidth(const std::string& text) const;
	float getTextWidth(const std::string&, const unsigned int numUnicodeCharsSkip, const unsigned int maxUnicodeCharsRead) const;

	void paintBegin(const float * const tranformationMatrix, float scale = 1.0f);
	void paintEnd();
	void paintFlush();
};

} } }

