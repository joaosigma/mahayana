#include "font.hpp"

#include "common\opengl\openGLext.hpp"
#include "common\Image.hpp"
#include "common\Platform.hpp"
#include "common\ImageFactory.hpp"

#include "libs\sdf\sdf.h"

#include <algorithm>

#include <ft2build.h>
#include FT_FREETYPE_H
#include FT_BITMAP_H
#include FT_GLYPH_H
#include FT_RENDER_H

const unsigned short validFontCharacters[] = {32,126, 192,255, 880,1008, 7936,8176, 1536,1791, 1040,1299};
const wchar_t validAditionalFontCharacters[] = {L"¥§©®±µ€"};

namespace HorseRadish { namespace Render { namespace Tools {

int Font::sMumMaxChar = 512; //check out declaration of mState.charData
unsigned short Font::sBufferPadding = 5;
unsigned short Font::sPrimitiveRestartIndex = 65500;

void Font::commitGL() const
{
	if (mState.numCharWritten == 0)
		return;

	mGl.arrayBuffer.writeData(mState.charData.data(), sizeof(VertexDataLayout) * mState.numCharWritten * 4, 0);

	assert(mState.paintStarted);

	mGl.vertexArray.bind();
	HorseRadish::OpenGL::glDrawRangeElements(GL_TRIANGLE_STRIP, 0, mState.numCharWritten * 4, mState.numCharWritten * 5, GL_UNSIGNED_SHORT, (void*)0);

	mState.numCharWritten = 0;
}

bool Font::createCharData()
{
	int numCharPairs, numNormalChars, numExtraChars, curCharWrite;
	HorseRadish::String strExtraChars;

	numCharPairs = sizeof(validFontCharacters) / sizeof(unsigned short);
	if ((numCharPairs % 2) != 0)
		return false;

	numCharPairs /= 2;
	for (int curPair = 0; curPair < numCharPairs; curPair++)
	{
		if (validFontCharacters[curPair * 2 + 0] > validFontCharacters[curPair * 2 + 1])
			return false;
	}

	numNormalChars = 0;
	for (int curPair = 0; curPair < numCharPairs; curPair++)
		numNormalChars += validFontCharacters[curPair * 2 + 1] - validFontCharacters[curPair * 2 + 0] + 1;

	strExtraChars.Set(HorseRadish::String::Encoding::Windows, validAditionalFontCharacters);
	numExtraChars = strExtraChars.GetSizeChars();

	mCharMap.reserve(numNormalChars + numExtraChars);

	curCharWrite = 0;
	for (int curPair = 0; curPair < numCharPairs; curPair++)
	{
		for (int curCharIndex = validFontCharacters[curPair * 2 + 0]; curCharIndex <= validFontCharacters[curPair * 2 + 1]; curCharIndex++, curCharWrite++)
			mCharMap[curCharIndex];
	}

	assert(curCharWrite == numNormalChars);

	for (int curCharIndex = 0; curCharIndex < numExtraChars; curCharIndex++, curCharWrite++)
		mCharMap[curCharIndex];

	assert(curCharWrite == mCharMap.size());

	return true;
}

bool Font::initFont(const char * const fontFilePath)
{
	FT_Library ftLibrary;
	FT_Face ftFace;
	FT_Int32 ftLoadFlags;
	int texWidth, texHeight, texMaxLineHeight, texLastWidth, totalArea;

	if (FT_Init_FreeType(&ftLibrary))
		return false;

	if (FT_New_Face(ftLibrary, fontFilePath, 0, &ftFace))
	{
		FT_Done_FreeType(ftLibrary);
		return false;
	}

	if (FT_Select_Charmap(ftFace, FT_ENCODING_UNICODE))
	{
		FT_Done_Face(ftFace);
		FT_Done_FreeType(ftLibrary);
		return false;
	}

	FT_Set_Pixel_Sizes(ftFace, 0, 96); //always choose 64 pixels
	float downScale = static_cast<float>(mFontInfo.size) / 96.0f;

	mFontInfo.maxHeight = static_cast<float>(ftFace->size->metrics.height >> 6) * downScale;
	mFontInfo.baseHeight = -1.0f * static_cast<float>(ftFace->size->metrics.descender >> 6) * downScale;

	if (createCharData() == false)
	{
		FT_Done_Face(ftFace);
		FT_Done_FreeType(ftLibrary);
		return false;
	}

	ftLoadFlags = FT_LOAD_DEFAULT | FT_LOAD_NO_HINTING | FT_LOAD_RENDER;
	totalArea = 0;

	struct GlyphData
	{
		unsigned short unicodeId;
		unsigned short width, height;
		unsigned short texX, texY;

		GlyphData(unsigned short unicodeId, unsigned short width, unsigned short height)
			: unicodeId(unicodeId), width(width), height(height), texX(0), texY(0)
		{ }
	};
	std::vector<GlyphData> validGlyphs;
	validGlyphs.reserve(mCharMap.size());

	for (std::unordered_map<unsigned short, CharacterData>::iterator it = mCharMap.begin(); it != mCharMap.end();)
	{
		auto freetypeCharIndex = FT_Get_Char_Index(ftFace, it->first);
		if (freetypeCharIndex == 0)
		{
			it = mCharMap.erase(it);
			continue;
		}

		auto freetypeError = FT_Load_Glyph(ftFace, freetypeCharIndex, ftLoadFlags);
		if (freetypeError != 0)
		{
			it = mCharMap.erase(it);
			continue;
		}

		auto glyphWidth = ftFace->glyph->bitmap.width;
		auto glyphHeight = ftFace->glyph->bitmap.rows;

		validGlyphs.push_back(GlyphData(it->first, glyphWidth, glyphHeight));

		CharacterData& curChar = it->second;
		++it;

		float glyphOffset[] = { static_cast<float>(ftFace->glyph->bitmap_left), static_cast<float>(ftFace->glyph->bitmap_top - ftFace->glyph->bitmap.rows) };

		curChar.advanceX = static_cast<float>(ftFace->glyph->advance.x >> 6) * downScale;
		curChar.rect.offsetX = (glyphOffset[0] - static_cast<float>(Font::sBufferPadding)) * downScale;
		curChar.rect.offsetY = (glyphOffset[1] - static_cast<float>(Font::sBufferPadding)) * downScale;
		curChar.rect.width = (static_cast<float>(ftFace->glyph->bitmap.width) + static_cast<float>(Font::sBufferPadding)) * downScale;
		curChar.rect.height = (static_cast<float>(ftFace->glyph->bitmap.rows) + static_cast<float>(Font::sBufferPadding)) * downScale;

		auto charArea = glyphWidth * glyphHeight;
		curChar.skipDraw = (charArea < 4);
		if (curChar.skipDraw)
			continue;

		if (charArea > 0)
			totalArea += (glyphWidth + (Font::sBufferPadding * 2)) * (glyphHeight + (Font::sBufferPadding * 2));
	}

	texWidth = HorseRadish::Math::iProxPowerOfTwo(HorseRadish::Math::ftoi(HorseRadish::Math::sqrt(totalArea)));
	texHeight = 0;

	texLastWidth = 0;
	for (int charIndex = 0; charIndex < validGlyphs.size(); charIndex++)
	{
		GlyphData& glyphData = validGlyphs[charIndex];
		CharacterData& charData = mCharMap[glyphData.unicodeId];
		
		if (charData.skipDraw)
			continue;

		if (texLastWidth == 0)
		{
			glyphData.texX = 0;
			glyphData.texY = 0;
			texLastWidth = glyphData.texX + glyphData.width + (Font::sBufferPadding * 2);
			texMaxLineHeight = glyphData.texY + glyphData.height + (Font::sBufferPadding * 2);
			continue;
		}

		if ((texLastWidth + glyphData.width + (Font::sBufferPadding * 2)) > texWidth)
		{
			glyphData.texX = 0;
			glyphData.texY = texMaxLineHeight;
			texLastWidth = glyphData.texX + glyphData.width + (Font::sBufferPadding * 2);
			texMaxLineHeight = glyphData.texY + glyphData.height + (Font::sBufferPadding * 2);
			continue;
		}

		glyphData.texX = texLastWidth;
		glyphData.texY = validGlyphs[charIndex - 1].texY;
		texLastWidth = glyphData.texX + glyphData.width + (Font::sBufferPadding * 2);

		auto nextY = glyphData.texY + glyphData.height + (Font::sBufferPadding * 2);
		if (nextY > texMaxLineHeight)
			texMaxLineHeight = nextY;
	}

	texHeight = HorseRadish::Math::iProxPowerOfTwo(texMaxLineHeight + (texMaxLineHeight % 2));

	{
		float invTexWidth = 1.0f / static_cast<float>(texWidth);
		float invTexHeight = 1.0f / static_cast<float>(texHeight);

		for (int charIndex = 0; charIndex < validGlyphs.size(); charIndex++)
		{
			GlyphData& glyphData = validGlyphs[charIndex];
			CharacterData& charData = mCharMap[glyphData.unicodeId];
			if (charData.skipDraw)
				continue;

			charData.rect.minUV[0] = static_cast<float>(glyphData.texX)* invTexWidth;
			charData.rect.minUV[1] = static_cast<float>(glyphData.texY) * invTexHeight;

			charData.rect.maxUV[0] = charData.rect.minUV[0] + static_cast<float>(glyphData.width + Font::sBufferPadding) * invTexWidth;
			charData.rect.maxUV[1] = charData.rect.minUV[1] + static_cast<float>(glyphData.height + Font::sBufferPadding) * invTexHeight;
		}
	}	

	auto imgFinal = new HorseRadish::Imaging::Image(texWidth, texHeight, HorseRadish::Imaging::Image::UByte, HorseRadish::Imaging::Image::Alpha);
	imgFinal->Clear(0.0f, 0.0f, 0.0f, 0.0f);

	for (auto& glyphData : validGlyphs)
	{
		CharacterData& charData = mCharMap[glyphData.unicodeId];

		if (charData.skipDraw)
			continue;

		if (FT_Load_Glyph(ftFace, FT_Get_Char_Index(ftFace, glyphData.unicodeId), ftLoadFlags))
			continue;

		FT_Glyph ftGlyph;
		if (FT_Get_Glyph(ftFace->glyph, &ftGlyph))
			continue;

		FT_Glyph_To_Bitmap(&ftGlyph, FT_RENDER_MODE_NORMAL, nullptr, 1);
		FT_BitmapGlyph bitmapGlyph = (FT_BitmapGlyph)ftGlyph;

		HorseRadish::Imaging::Image imgAux(bitmapGlyph->bitmap.width + (Font::sBufferPadding * 2), bitmapGlyph->bitmap.rows + (Font::sBufferPadding * 2), HorseRadish::Imaging::Image::UByte, HorseRadish::Imaging::Image::Alpha);
		imgAux.Clear(0.0f, 0.0f, 0.0f, 0.0f);
		for (int y = 0; y < bitmapGlyph->bitmap.rows; y++)
		{
			auto ptr = (unsigned char*)imgAux.GetPixelData() + (y * imgAux.GetRowSize()) + (Font::sBufferPadding * imgAux.GetRowSize());

			memcpy(ptr + Font::sBufferPadding, bitmapGlyph->bitmap.buffer + (y * bitmapGlyph->bitmap.width), bitmapGlyph->bitmap.width);
		}
		imgAux.Flip();

		imgFinal->CopyRegion(&imgAux, glyphData.texX, glyphData.texY);
	}

	//build distance map
	sdfBuild((unsigned char*)imgFinal->GetPixelData(), imgFinal->GetWidth(), static_cast<float>(Font::sBufferPadding - 1), (const unsigned char*)imgFinal->GetPixelData(), imgFinal->GetWidth(), imgFinal->GetHeight(), imgFinal->GetWidth());

	imgFinal->Scale(imgFinal->GetWidth() / 2, imgFinal->GetHeight() / 2, HorseRadish::Imaging::Image::SampleType::Bicubic2);

	if (FT_HAS_KERNING(ftFace) != 0)
	{
		for (const auto& curChar1 : mCharMap)
		{
			auto glyphIndex1 = FT_Get_Char_Index(ftFace, curChar1.first);

			for (const auto& curChar2 : mCharMap)
			{
				FT_Vector kernVector;

				if (curChar1.first == curChar2.first)
					continue;

				auto glyphIndex2 = FT_Get_Char_Index(ftFace, curChar2.first);
				if (glyphIndex1 == glyphIndex2)
					continue;

				if (FT_Get_Kerning(ftFace, glyphIndex1, glyphIndex2, FT_KERNING_DEFAULT, &kernVector) != 0)
					continue;

				if (kernVector.x == 0)
					continue;

				KerningData newKerningData;
				newKerningData.char1 = curChar1.first;
				newKerningData.char2 = curChar2.first;
				newKerningData.offset = (kernVector.x >> 6) * downScale;

				mKerningData.push_back(newKerningData);
			}
		}

		mKerningData.push_back(KerningData());
	}

	FT_Done_Face(ftFace);
	FT_Done_FreeType(ftLibrary);

	for (auto& curChar : mCharMap)
	{
		for (const auto& curKerning : mKerningData)
		{
			if ((curKerning.char1 == 0) || (curKerning.char1 != curChar.first))
				continue;

			curChar.second.kernData = &curKerning;
			break;
		}
	}

	mGl.texture.init(HorseRadish::OpenGL::Objects::Texture::Type::Tex2D, HorseRadish::OpenGL::Objects::Texture::StorageType::R_8, imgFinal->GetWidth(), imgFinal->GetHeight());
	mGl.texture.uploadData(0, 0, 0, imgFinal->GetWidth(), imgFinal->GetHeight(), HorseRadish::OpenGL::Objects::Texture::DataFormat::R, HorseRadish::OpenGL::Objects::Texture::DataType::UBYTE, imgFinal->GetPixelData());

	delete imgFinal;
	imgFinal = nullptr;

	return true;
}

void Font::internalWrite(const float &px, const float &py, HorseRadish::String::Iterator &strIt) const
{
	if (!mValid)
		return;

	float posX = px;
	float posY = py + (mFontInfo.baseHeight * mState.scale);

	while (!strIt.IsLast())
	{
		auto writeData = mState.charData.data() + (mState.numCharWritten * 4);

		while (mState.numCharWritten < Font::sMumMaxChar)
		{
			unsigned char colorTemp[4];

			auto curCharUnicode = strIt.Read();
			if (curCharUnicode == 0)
				break;

			std::unordered_map<unsigned short, CharacterData>::const_iterator it = mCharMap.find(curCharUnicode);
			if (it == mCharMap.end())
				continue;

			const CharacterData& curCharData = it->second;

			if (curCharData.skipDraw)
			{
				posX += curCharData.advanceX * mState.scale;
				continue;
			}

			writeData[0].px = writeData[3].px = posX + curCharData.rect.offsetX * mState.scale;
			writeData[1].px = writeData[2].px = writeData[0].px + curCharData.rect.width * mState.scale;
			writeData[0].py = writeData[1].py = posY + curCharData.rect.offsetY * mState.scale;
			writeData[2].py = writeData[3].py = writeData[0].py + curCharData.rect.height * mState.scale;

			writeData[0].tu = writeData[3].tu = curCharData.rect.minUV[0];
			writeData[1].tu = writeData[2].tu = curCharData.rect.maxUV[0];
			writeData[0].tv = writeData[1].tv = curCharData.rect.minUV[1];
			writeData[2].tv = writeData[3].tv = curCharData.rect.maxUV[1];

			mState.stateColor.Write(colorTemp);
			memcpy(writeData[0].rgba, colorTemp, sizeof(unsigned char) * 4);
			memcpy(writeData[1].rgba, colorTemp, sizeof(unsigned char) * 4);
			memcpy(writeData[2].rgba, colorTemp, sizeof(unsigned char) * 4);
			memcpy(writeData[3].rgba, colorTemp, sizeof(unsigned char) * 4);

			writeData += 4;
			mState.numCharWritten++;
			posX += curCharData.advanceX * mState.scale;

			if (!mKerningData.empty())
				posX += static_cast<float>(getCharKerning(curCharData, curCharUnicode, *strIt)) * mState.scale;
		}

		if (mState.numCharWritten >= Font::sMumMaxChar)
			commitGL();
	}
}

int Font::getCharKerning(const CharacterData& leftCharData, unsigned short leftCharUnicodeID, unsigned short rightCharUnicodeID) const
{
	if ((leftCharData.kernData == nullptr) || (rightCharUnicodeID <= 0))
		return 0;

	for (const KerningData *curKerning = leftCharData.kernData; true; curKerning++)
	{
		if (curKerning->char1 != leftCharUnicodeID)
			break;

		if (curKerning->char2 == rightCharUnicodeID)
			return curKerning->offset;
	}

	return 0;
}

Font::Font(const int fontSize, const char * const fontFilePath, unsigned int glVertexProgramID, unsigned int glFragmentProgramID, unsigned int glProgramPipelineID)
	: glVertexProgramID(glVertexProgramID), glFragmentProgramID(glFragmentProgramID), glProgramPipelineID(glProgramPipelineID)
{
	mValid = false;

	mFontInfo.size = fontSize;
	mFontInfo.baseHeight = mFontInfo.maxHeight = 0.0f;

	mState.scale = 1.0f;
	mState.numCharWritten = 0;
	mState.paintStarted = false;
	mState.stateColor.Set(1.0f, 1.0f, 1.0f, 1.0f);
	
	if (fontSize <= 2 || fontFilePath == nullptr)
		return;

	glUniformSampler = HorseRadish::OpenGL::glGetUniformLocation(glFragmentProgramID, "texTextSampler");
	glUniformMatrix = HorseRadish::OpenGL::glGetUniformLocation(glVertexProgramID, "transformationMatrix");

	{
		std::unique_ptr<unsigned short[]> fontIndexArray = std::unique_ptr<unsigned short[]>(new unsigned short[Font::sMumMaxChar * 5]);
		for (int i = 0, curIndex = 0; i < Font::sMumMaxChar; i++, curIndex += 4)
		{
			fontIndexArray[i * 5 + 0] = curIndex + 3;
			fontIndexArray[i * 5 + 1] = curIndex + 0;
			fontIndexArray[i * 5 + 2] = curIndex + 2;
			fontIndexArray[i * 5 + 3] = curIndex + 1;
			fontIndexArray[i * 5 + 4] = Font::sPrimitiveRestartIndex;
		}

		mGl.arrayBuffer.init(HorseRadish::OpenGL::Objects::Buffer::Type::ArrayBuffer, sizeof(Font::VertexDataLayout) * Font::sMumMaxChar * 4, HorseRadish::OpenGL::Objects::Buffer::UsageType::FrequentOnlyWrite);
		mGl.elementArrayBuffer.init(HorseRadish::OpenGL::Objects::Buffer::Type::ElementArrayBuffer, fontIndexArray.get(), sizeof(unsigned short) * Font::sMumMaxChar * 5, HorseRadish::OpenGL::Objects::Buffer::UsageType::ServerStatic);
	}

	mGl.vertexArray.init();

	HorseRadish::OpenGL::glEnableVertexArrayAttrib(mGl.vertexArray.getId(), 0);
	HorseRadish::OpenGL::glEnableVertexArrayAttrib(mGl.vertexArray.getId(), 1);
	HorseRadish::OpenGL::glEnableVertexArrayAttrib(mGl.vertexArray.getId(), 4);

	HorseRadish::OpenGL::glVertexArrayAttribBinding(mGl.vertexArray.getId(), 0, 0);
	HorseRadish::OpenGL::glVertexArrayAttribFormat(mGl.vertexArray.getId(), 0, 2, GL_FLOAT, false, 0);

	HorseRadish::OpenGL::glVertexArrayAttribBinding(mGl.vertexArray.getId(), 1, 0);
	HorseRadish::OpenGL::glVertexArrayAttribFormat(mGl.vertexArray.getId(), 1, 2, GL_FLOAT, false, 8);

	HorseRadish::OpenGL::glVertexArrayAttribBinding(mGl.vertexArray.getId(), 4, 0);
	HorseRadish::OpenGL::glVertexArrayAttribFormat(mGl.vertexArray.getId(), 4, 4, GL_UNSIGNED_BYTE, true, 16);

	HorseRadish::OpenGL::glVertexArrayElementBuffer(mGl.vertexArray.getId(), mGl.elementArrayBuffer.getId());
	HorseRadish::OpenGL::glVertexArrayVertexBuffer(mGl.vertexArray.getId(), 0, mGl.arrayBuffer.getId(), 0, sizeof(Font::VertexDataLayout));

	mGl.sampler.init();
	mGl.sampler.setMinFilter(HorseRadish::OpenGL::Objects::Sampler::FilterType::Linear);
	mGl.sampler.setMagFilter(HorseRadish::OpenGL::Objects::Sampler::FilterType::Linear);
	mGl.sampler.setWrap(HorseRadish::OpenGL::Objects::Sampler::WrapType::ClampBorder);
	mGl.sampler.setBorderColor(0.0f, 0.0f, 0.0f, 0.0f);
	
	if (initFont(fontFilePath) == false)
		return;

	mValid = true;
}

Font::~Font()
{
	mKerningData.clear();
	mCharMap.clear();
}

void Font::write(const float &px, const float &py, HorseRadish::String::Iterator &iterator) const
{
	internalWrite(px, py, iterator);
}

void Font::write(HorseRadish::String::Iterator &iterator) const
{
	write(0.0f, 0.0f, iterator);
}

void Font::write(const float &px, const float &py, const HorseRadish::String &text) const
{
	internalWrite(px, py, HorseRadish::String::Iterator(text));
}

void Font::write(const HorseRadish::String &text) const
{
	write(0.0f, 0.0f, text);
}

float Font::writeChar(const unsigned int &unicodeChar) const
{
	return writeChar(0.0f, 0.0f, unicodeChar);
}

float Font::writeChar(const float &px, const float &py, const unsigned int &unicodeChar) const
{
	unsigned char colorTemp[4];

	if (!mValid)
		return 0.0f;

	std::unordered_map<unsigned short, CharacterData>::const_iterator it = mCharMap.find(unicodeChar);
	if (it == mCharMap.end())
		return 0.0f;

	const CharacterData& charData = it->second;

	if (charData.skipDraw)
		return charData.advanceX * mState.scale;

	if (mState.numCharWritten >= Font::sMumMaxChar)
		commitGL();

	auto writeData = mState.charData.data() + (mState.numCharWritten * 4);

	float posY = py + (mFontInfo.baseHeight * mState.scale);

	writeData[0].px = writeData[3].px = px + charData.rect.offsetX * mState.scale;
	writeData[1].px = writeData[2].px = writeData[0].px + charData.rect.width * mState.scale;
	writeData[0].py = writeData[1].py = posY + charData.rect.offsetY * mState.scale;
	writeData[2].py = writeData[3].py = writeData[0].py + charData.rect.height * mState.scale;

	writeData[0].tu = writeData[3].tu = charData.rect.minUV[0];
	writeData[1].tu = writeData[2].tu = charData.rect.maxUV[0];
	writeData[0].tv = writeData[1].tv = charData.rect.minUV[1];
	writeData[2].tv = writeData[3].tv = charData.rect.maxUV[1];

	mState.stateColor.Write(colorTemp);
	memcpy(writeData[0].rgba, colorTemp, sizeof(unsigned char) * 4);
	memcpy(writeData[1].rgba, colorTemp, sizeof(unsigned char) * 4);
	memcpy(writeData[2].rgba, colorTemp, sizeof(unsigned char) * 4);
	memcpy(writeData[3].rgba, colorTemp, sizeof(unsigned char) * 4);

	mState.numCharWritten++;
	return charData.advanceX * mState.scale;
}

void Font::setColor(const float &r, const float &g, const float &b, const float &a)
{
	mState.stateColor.Set(r, g, b, a);
}

void Font::setColor(const float &r, const float &g, const float &b)
{
	mState.stateColor.Set(r, g, b, 1.0f);
}

void Font::setColor(const float * const color)
{
	mState.stateColor.Set(color);
}

void Font::setColor(const Color &color)
{
	mState.stateColor.Set(color);
}

bool Font::getOperacional() const
{
	return mValid;
}

float Font::getMaxHeight() const
{
	return mFontInfo.maxHeight * mState.scale;
}

float Font::getCharWidth(const unsigned int &unicodeChar) const
{
	std::unordered_map<unsigned short, CharacterData>::const_iterator it = mCharMap.find(unicodeChar);
	if (it == mCharMap.end())
		return 0.0f;

	return it->second.advanceX * mState.scale;
}

float Font::getStringWidth(const HorseRadish::String &text) const
{
	if (!mValid || (text.GetSizeChars() == 0))
		return 0.0f;

	float totalWidth = 0.0f;
	for (HorseRadish::String::Iterator it(text); it.IsLast() == false; it++)
	{
		std::unordered_map<unsigned short, CharacterData>::const_iterator itChar = mCharMap.find(*it);
		if (itChar == mCharMap.end())
			continue;

		totalWidth += itChar->second.advanceX;
	}

	return totalWidth * mState.scale;
}

float Font::getStringWidth(const HorseRadish::String &text, const unsigned int numMaxChar) const
{
	if (!mValid || (text.GetSizeChars() == 0) || (numMaxChar == 0))
		return 0.0f;

	float totalWidth = 0.0f;
	for (HorseRadish::String::Iterator it(text); it.IsLast() == false; it++)
	{
		if (it.GetCaracterPosition() >= numMaxChar)
			break;

		std::unordered_map<unsigned short, CharacterData>::const_iterator itChar = mCharMap.find(*it);
		if (itChar == mCharMap.end())
			continue;

		totalWidth += itChar->second.advanceX;
	}

	return totalWidth * mState.scale;
}

void Font::paintBegin(const float * const tranformationMatrix, float scale)
{
	if (mState.paintStarted || (scale == 0.0f))
		return;

	mGl.texture.bind(0);
	mGl.sampler.bind(0);
	mGl.vertexArray.bind();

	HorseRadish::OpenGL::glEnable(GL_PRIMITIVE_RESTART);
	HorseRadish::OpenGL::glPrimitiveRestartIndex(Font::sPrimitiveRestartIndex);

	HorseRadish::OpenGL::glProgramUniform1i(glFragmentProgramID, glUniformSampler, 0);
	if (tranformationMatrix != nullptr)
		HorseRadish::OpenGL::glProgramUniformMatrix4fv(glVertexProgramID, glUniformMatrix, 1, false, tranformationMatrix);

	HorseRadish::OpenGL::glUseProgram(0);
	HorseRadish::OpenGL::glBindProgramPipeline(glProgramPipelineID);

	mState.scale = scale;
	mState.paintStarted = true;
}

void Font::paintEnd()
{
	if (!mState.paintStarted)
		return;

	commitGL();

	HorseRadish::OpenGL::glDisable(GL_PRIMITIVE_RESTART);

	mState.paintStarted = false;
}

void Font::paintFlush()
{
	if (!mState.paintStarted)
		return;

	commitGL();
}

} } }