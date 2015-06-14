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
		float px, py, tu, tv;
		unsigned char rgba[4];
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

	static int sMumMaxChar;
	static unsigned short sBufferPadding;

private:
	bool mValid;
	std::vector<KerningData> mKerningData;
	std::unordered_map<unsigned short, CharacterData> mCharMap;
	
	unsigned int glVertexProgramID, glFragmentProgramID, glProgramPipelineID, glUniformSampler, glUniformMatrix;
	
	struct
	{
		HorseRadish::OpenGL::Objects::Texture texture;
		HorseRadish::OpenGL::Objects::Sampler sampler;
		HorseRadish::OpenGL::Objects::VertexArray vertexArray;
		HorseRadish::OpenGL::Objects::Buffer arrayBuffer, elementArrayBuffer;
	} mGl;

	struct
	{
		float scale;
		bool paintStarted;
		HorseRadish::Color stateColor;

		mutable unsigned int numCharWritten;
		mutable std::array<VertexDataLayout, 512 * 4> charData;
	} mState;

	struct
	{
		unsigned short size;
		float maxHeight, baseHeight;
	} mFontInfo;

	void commitGL() const;
	bool createCharData();
	float getCharKerning(const CharacterData& leftCharData, unsigned short leftCharUnicodeID, unsigned short rightCharUnicodeID) const;
	bool initFont(const char * const fontFilePath);
	void internalWrite(const float &px, const float &py, const std::string& text) const;

public:
	Font(const int fontSize, const char * const fontFilePath, unsigned int glVertexProgramID, unsigned int glFragmentProgramID, unsigned int glProgramPipelineID);
	~Font();

	void write(const float &px, const float &py, const std::string& text) const;
	void write(const std::string& text) const;
	float writeChar(const unsigned int &unicodeChar) const;
	float writeChar(const float &px, const float &py, const unsigned int &unicodeChar) const;

	void setColor(const float &r, const float &g, const float &b, const float &a);
	void setColor(const float &r, const float &g, const float &b);
	void setColor(const float * const color);
	void setColor(const Color &color);

	bool getOperacional() const;
	float getMaxHeight() const;
	float getCharWidth(const unsigned int &unicodeChar) const;
	float getStringWidth(const std::string& text) const;
	float getStringWidth(const std::string&, const unsigned int numMaxChar) const;

	void paintBegin(const float * const tranformationMatrix, float scale = 1.0f);
	void paintEnd();
	void paintFlush();
};

} } }

