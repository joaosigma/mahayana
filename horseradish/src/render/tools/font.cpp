#include "font.hpp"

#include "common/image.hpp"
#include "common/opengl/openGLext.hpp"

#include "libs/sdf/sdf.h"

#include <algorithm>
#include <cstddef>

#include <ft2build.h>
#include FT_FREETYPE_H
#include FT_BITMAP_H
#include FT_GLYPH_H
#include FT_RENDER_H

const unsigned short validFontCharacters[] = {32,126, 192,255, 880,1008, 7936,8176, 1536,1791, 1040,1299};
const wchar_t validAditionalFontCharacters[] = {L"¥§©®±µ€"};

namespace hr { namespace render { namespace tools
{
	void Font::commitGL()
	{
		if (mState.numCharWritten == 0)
			return;

		assert(mState.paintStarted);

		mGl.fence.wait();
			mGl.arrayBuffer.writeData(mState.charData.data(), sizeof(VertexDataLayout) * mState.numCharWritten * 4, 0);
			hr::gl::glDrawRangeElements(GL_TRIANGLES, 0, mState.numCharWritten * 4, mState.numCharWritten * 6, GL_UNSIGNED_SHORT, (void*)0);
		mGl.fence.place();

		mState.numCharWritten = 0;
	}

	bool Font::createCharData()
	{
		auto numCharPairs = sizeof(validFontCharacters) / sizeof(unsigned short);
		if ((numCharPairs % 2) != 0)
			return false;

		numCharPairs /= 2;
		for (size_t curPair = 0; curPair < numCharPairs; curPair++)
		{
			if (validFontCharacters[curPair * 2 + 0] > validFontCharacters[curPair * 2 + 1])
				return false;
		}

		size_t numNormalChars = 0;
		for (size_t curPair = 0; curPair < numCharPairs; curPair++)
			numNormalChars += validFontCharacters[curPair * 2 + 1] - validFontCharacters[curPair * 2 + 0] + 1;

		auto strExtraChars = hr::StringUtils::conv2UTF8(validAditionalFontCharacters);

		mCharMap.reserve(numNormalChars + (sizeof(validAditionalFontCharacters) / sizeof(wchar_t)));

		for (size_t curPair = 0; curPair < numCharPairs; curPair++)
		{
			for (unsigned short curCharIndex = validFontCharacters[curPair * 2 + 0]; curCharIndex <= validFontCharacters[curPair * 2 + 1]; curCharIndex++)
				mCharMap[curCharIndex];
		}

		for (const auto& curChar : hr::StringUtils::utf8Wrapper(strExtraChars))
			mCharMap[curChar];

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
		auto downScale = static_cast<float>(mFontInfo.size) / 96.0f;

		mFontInfo.maxHeight = static_cast<float>(ftFace->size->metrics.height >> 6) * downScale;
		mFontInfo.baseHeight = -1.0f * static_cast<float>(ftFace->size->metrics.descender >> 6) * downScale;

		if (!createCharData())
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

			float glyphOffset[] = { static_cast<float>(ftFace->glyph->bitmap_left), static_cast<float>(ftFace->glyph->bitmap_top) - static_cast<float>(ftFace->glyph->bitmap.rows) };

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

		texWidth = hr::Math::iProxPowerOfTwo(hr::Math::ftoi(hr::Math::sqrt(totalArea)));
		texHeight = 0;

		texLastWidth = 0;
		for (size_t charIndex = 0; charIndex < validGlyphs.size(); charIndex++)
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

		texHeight = hr::Math::iProxPowerOfTwo(texMaxLineHeight + (texMaxLineHeight % 2));

		{
			auto invTexWidth = 1.0f / static_cast<float>(texWidth);
			auto invTexHeight = 1.0f / static_cast<float>(texHeight);

			for (size_t charIndex = 0; charIndex < validGlyphs.size(); charIndex++)
			{
				GlyphData& glyphData = validGlyphs[charIndex];
				CharacterData& charData = mCharMap[glyphData.unicodeId];
				if (charData.skipDraw)
					continue;

				charData.rect.minUV[0] = static_cast<float>(glyphData.texX) * invTexWidth;
				charData.rect.minUV[1] = static_cast<float>(glyphData.texY) * invTexHeight;

				charData.rect.maxUV[0] = charData.rect.minUV[0] + static_cast<float>(glyphData.width + Font::sBufferPadding) * invTexWidth;
				charData.rect.maxUV[1] = charData.rect.minUV[1] + static_cast<float>(glyphData.height + Font::sBufferPadding) * invTexHeight;
			}
		}	

		hr::imaging::Image<unsigned char, hr::imaging::ImageFormatR> imgFinal(texWidth, texHeight);
		imgFinal.clear(0, 0, 0, 0);
	
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


			hr::imaging::Image<unsigned char, hr::imaging::ImageFormatR> imgAux(bitmapGlyph->bitmap.width + (Font::sBufferPadding * 2), bitmapGlyph->bitmap.rows + (Font::sBufferPadding * 2));
			imgAux.clear(0, 0, 0, 0);

			hr::imaging::ImageView<unsigned char, hr::imaging::ImageFormatR> imgGlyph(bitmapGlyph->bitmap.buffer, bitmapGlyph->bitmap.width, bitmapGlyph->bitmap.rows);

			imgAux.setPixelRegion(imgGlyph, Font::sBufferPadding, Font::sBufferPadding, true);

			imgFinal.setPixelRegion(imgAux, glyphData.texX, glyphData.texY);
		}

		//build distance map
		sdfBuild(imgFinal.data(), imgFinal.width(), static_cast<float>(Font::sBufferPadding - 1), imgFinal.data(), imgFinal.width(), imgFinal.height(), imgFinal.width());
		imgFinal = imgFinal.resize(imgFinal.width() / 2, imgFinal.height() / 2);

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

		mGl.texture.init(hr::gl::objects::Texture::Type::Tex2D, hr::gl::objects::Texture::StorageType::R_8, imgFinal.width(), imgFinal.height());
		mGl.texture.uploadData(0, 0, 0, imgFinal.width(), imgFinal.height(), hr::gl::objects::Texture::DataFormat::R, hr::gl::objects::Texture::DataType::UBYTE, imgFinal.data());

		return true;
	}

	void Font::internalWrite(const float &px, const float &py, const std::string& str, UnicodeRange strRange)
	{
		if (!mValid || str.empty())
			return;

		float posX = px;
		float posY = py + (mFontInfo.baseHeight * mState.scale);

		unsigned char colorTemp[4];
		mState.stateColor.write(colorTemp);

		auto writeData = mState.charData.data() + (mState.numCharWritten * 4);

		unsigned int lastCharUnicode = 0;
		for (const auto& curCharUnicode : StringUtils::utf8Wrapper(str))
		{
			if (curCharUnicode == 0)
				break;

			if (strRange.numCharsSkip > 0)
			{
				strRange.numCharsSkip--;
				continue;
			}

			std::unordered_map<unsigned short, CharacterData>::const_iterator it = mCharMap.find(curCharUnicode);
			if (it == mCharMap.end())
				continue;

			if (strRange.numCharsMax >= 0)
			{
				strRange.numCharsMax--;
				if (strRange.numCharsMax < 0)
					break;
			}

			const CharacterData& curCharData = it->second;
			if (curCharData.skipDraw)
			{
				posX += curCharData.advanceX * mState.scale;
				continue;
			}

			if (mState.numCharWritten >= Font::sMumMaxChar)
			{
				commitGL();
				writeData = mState.charData.data() + (mState.numCharWritten * 4);
			}
		
			if (lastCharUnicode && !mKerningData.empty())
				posX += getCharKerning(curCharData, lastCharUnicode, curCharUnicode) * mState.scale;
			lastCharUnicode = curCharUnicode;

			writeData[0].pos[0] = writeData[3].pos[0] = posX + curCharData.rect.offsetX * mState.scale;
			writeData[1].pos[0] = writeData[2].pos[0] = writeData[0].pos[0] + curCharData.rect.width * mState.scale;
			writeData[0].pos[1] = writeData[1].pos[1] = posY + curCharData.rect.offsetY * mState.scale;
			writeData[2].pos[1] = writeData[3].pos[1] = writeData[0].pos[1] + curCharData.rect.height * mState.scale;

			writeData[0].uv[0] = writeData[3].uv[0] = curCharData.rect.minUV[0];
			writeData[1].uv[0] = writeData[2].uv[0] = curCharData.rect.maxUV[0];
			writeData[0].uv[1] = writeData[1].uv[1] = curCharData.rect.minUV[1];
			writeData[2].uv[1] = writeData[3].uv[1] = curCharData.rect.maxUV[1];

			memcpy(writeData[0].color, colorTemp, sizeof(unsigned char) * 4);
			memcpy(writeData[1].color, colorTemp, sizeof(unsigned char) * 4);
			memcpy(writeData[2].color, colorTemp, sizeof(unsigned char) * 4);
			memcpy(writeData[3].color, colorTemp, sizeof(unsigned char) * 4);

			writeData += 4;
			mState.numCharWritten++;
			posX += curCharData.advanceX * mState.scale;
		}
	}

	float Font::getCharKerning(const CharacterData& leftCharData, unsigned short leftCharUnicodeID, unsigned short rightCharUnicodeID) const
	{
		if (!leftCharData.kernData || (rightCharUnicodeID <= 0))
			return 0.0f;

		for (const KerningData *curKerning = leftCharData.kernData; true; curKerning++)
		{
			if (curKerning->char1 != leftCharUnicodeID)
				break;

			if (curKerning->char2 == rightCharUnicodeID)
				return curKerning->offset;
		}

		return 0.0f;
	}

	Font::Font(const size_t fontSize, const char * const fontFilePath, unsigned int glVertexProgramID, unsigned int glFragmentProgramID, unsigned int glProgramPipelineID)
		: mGlVertexProgramID(glVertexProgramID), mGlFragmentProgramID(glFragmentProgramID), mGlProgramPipelineID(glProgramPipelineID)
	{
		mFontInfo.size = fontSize;
		mState.stateColor.set(1.0f, 1.0f, 1.0f, 1.0f);
	
		if (!fontFilePath || (fontSize <= 2))
			return;

		mGlUniformSampler = hr::gl::glGetUniformLocation(glFragmentProgramID, "texTextSampler");
		mGlUniformMatrix = hr::gl::glGetUniformLocation(glVertexProgramID, "transformationMatrix");

		{
			auto fontIndexArray = std::make_unique<unsigned short[]>(Font::sMumMaxChar * 6);
			for (int i = 0, curIndex = 0; i < Font::sMumMaxChar; i++, curIndex += 4)
			{
				fontIndexArray[i * 6 + 0] = curIndex + 0;
				fontIndexArray[i * 6 + 1] = curIndex + 1;
				fontIndexArray[i * 6 + 2] = curIndex + 2;
				fontIndexArray[i * 6 + 3] = curIndex + 0;
				fontIndexArray[i * 6 + 4] = curIndex + 2;
				fontIndexArray[i * 6 + 5] = curIndex + 3;
			}

			mGl.arrayBuffer.init(hr::gl::objects::Buffer::Type::ArrayBuffer, sizeof(Font::VertexDataLayout) * Font::sMumMaxChar * 4, hr::gl::objects::Buffer::UsageType::PersistentOnlyWrite);
			mGl.elementArrayBuffer.init(hr::gl::objects::Buffer::Type::ElementArrayBuffer, fontIndexArray.get(), sizeof(unsigned short) * Font::sMumMaxChar * 6, hr::gl::objects::Buffer::UsageType::ServerStatic);
		}

		mGl.vertexArray.init();

		hr::gl::glEnableVertexArrayAttrib(mGl.vertexArray.getId(), 0);
		hr::gl::glEnableVertexArrayAttrib(mGl.vertexArray.getId(), 1);
		hr::gl::glEnableVertexArrayAttrib(mGl.vertexArray.getId(), 4);

		hr::gl::glVertexArrayAttribBinding(mGl.vertexArray.getId(), 0, 0);
		hr::gl::glVertexArrayAttribFormat(mGl.vertexArray.getId(), 0, 2, GL_FLOAT, false, offsetof(Font::VertexDataLayout, pos));

		hr::gl::glVertexArrayAttribBinding(mGl.vertexArray.getId(), 1, 0);
		hr::gl::glVertexArrayAttribFormat(mGl.vertexArray.getId(), 1, 2, GL_FLOAT, false, offsetof(Font::VertexDataLayout, uv));

		hr::gl::glVertexArrayAttribBinding(mGl.vertexArray.getId(), 4, 0);
		hr::gl::glVertexArrayAttribFormat(mGl.vertexArray.getId(), 4, 4, GL_UNSIGNED_BYTE, true, offsetof(Font::VertexDataLayout, color));

		hr::gl::glVertexArrayElementBuffer(mGl.vertexArray.getId(), mGl.elementArrayBuffer.getId());
		hr::gl::glVertexArrayVertexBuffer(mGl.vertexArray.getId(), 0, mGl.arrayBuffer.getId(), 0, sizeof(Font::VertexDataLayout));

		mGl.sampler.init();
		mGl.sampler.setMinFilter(hr::gl::objects::Sampler::FilterType::Linear);
		mGl.sampler.setMagFilter(hr::gl::objects::Sampler::FilterType::Linear);
		mGl.sampler.setWrap(hr::gl::objects::Sampler::WrapType::ClampBorder);
		mGl.sampler.setBorderColor(0.0f, 0.0f, 0.0f, 0.0f);
	
		if (!initFont(fontFilePath))
			return;

		mValid = true;
	}

	Font::~Font()
	{
		mKerningData.clear();
		mCharMap.clear();
	}

	void Font::layout(const std::string& text, const float maxWidth, std::function<void(size_t, size_t, size_t)> writeCb) const
	{
		if ((maxWidth <= 0.0f) || !writeCb)
			return;

		auto numLines = static_cast<unsigned int>(std::ceil(getTextWidth(text) / maxWidth));
		if (numLines == 0)
			numLines = 1;

		unsigned int numCharsWritten = 0;
		for (; numLines > 0; numLines--)
		{
			auto maxChars = countUnicodeChars(text, numCharsWritten, maxWidth);

			writeCb(numLines - 1, numCharsWritten, maxChars);

			numCharsWritten += maxChars;
		}
	}

	void Font::write(const std::string& text)
	{
		write(0.0f, 0.0f, text);
	}

	void Font::write(const float &px, const float &py, const std::string& text)
	{
		internalWrite(px, py, text, UnicodeRange());
	}

	void Font::write(const float &px, const float &py, const std::string& text, const size_t numUnicodeCharsSkip)
	{
		internalWrite(px, py, text, UnicodeRange(numUnicodeCharsSkip));
	}

	void Font::write(const float &px, const float &py, const std::string& text, const size_t numUnicodeCharsSkip, const size_t maxUnicodeCharsWrite)
	{
		internalWrite(px, py, text, UnicodeRange(numUnicodeCharsSkip, static_cast<int>(maxUnicodeCharsWrite)));
	}

	float Font::writeChar(const unsigned int &unicodeChar)
	{
		return writeChar(0.0f, 0.0f, unicodeChar);
	}

	float Font::writeChar(const float &px, const float &py, const unsigned int &unicodeChar)
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

		writeData[0].pos[0] = writeData[3].pos[0] = px + charData.rect.offsetX * mState.scale;
		writeData[1].pos[0] = writeData[2].pos[0] = writeData[0].pos[0] + charData.rect.width * mState.scale;
		writeData[0].pos[1] = writeData[1].pos[1] = posY + charData.rect.offsetY * mState.scale;
		writeData[2].pos[1] = writeData[3].pos[1] = writeData[0].pos[1] + charData.rect.height * mState.scale;

		writeData[0].uv[0] = writeData[3].uv[0] = charData.rect.minUV[0];
		writeData[1].uv[0] = writeData[2].uv[0] = charData.rect.maxUV[0];
		writeData[0].uv[1] = writeData[1].uv[1] = charData.rect.minUV[1];
		writeData[2].uv[1] = writeData[3].uv[1] = charData.rect.maxUV[1];

		mState.stateColor.write(colorTemp);
		memcpy(writeData[0].color, colorTemp, sizeof(unsigned char) * 4);
		memcpy(writeData[1].color, colorTemp, sizeof(unsigned char) * 4);
		memcpy(writeData[2].color, colorTemp, sizeof(unsigned char) * 4);
		memcpy(writeData[3].color, colorTemp, sizeof(unsigned char) * 4);

		mState.numCharWritten++;
		return charData.advanceX * mState.scale;
	}

	void Font::setColor(const float &r, const float &g, const float &b, const float &a)
	{
		mState.stateColor.set(r, g, b, a);
	}

	void Font::setColor(const float &r, const float &g, const float &b)
	{
		mState.stateColor.set(r, g, b, 1.0f);
	}

	void Font::setColor(const float * const color)
	{
		mState.stateColor.set(color);
	}

	void Font::setColor(const Color &color)
	{
		mState.stateColor.set(color);
	}

	bool Font::isValid() const
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

	size_t Font::countUnicodeChars(const std::string& text, const float maxWidth) const
	{
		return countUnicodeChars(text, 0, maxWidth);
	}

	size_t Font::countUnicodeChars(const std::string& text, const size_t numUnicodeCharsSkip, const float maxWidth) const
	{
		if (!mValid || text.empty() || (maxWidth <= 0.0f))
			return 0;

		float totalWidth = 0.0f;
		size_t numChars = 0;
		size_t numCharsSkip = numUnicodeCharsSkip;

		for (const auto& curCharUnicode : StringUtils::utf8Wrapper(text))
		{
			std::unordered_map<unsigned short, CharacterData>::const_iterator itChar = mCharMap.find(curCharUnicode);
			if (itChar == mCharMap.end())
				continue;

			if (numCharsSkip > 0)
			{
				numCharsSkip--;
				continue;
			}

			totalWidth += itChar->second.advanceX;
			if (totalWidth > maxWidth)
				return numChars;

			numChars++;
		}

		return numChars;
	}

	float Font::getTextWidth(const std::string& text) const
	{
		if (!mValid || text.empty())
			return 0.0f;

		float totalWidth = 0.0f;
		for (const auto& curCharUnicode : StringUtils::utf8Wrapper(text))
		{
			std::unordered_map<unsigned short, CharacterData>::const_iterator itChar = mCharMap.find(curCharUnicode);
			if (itChar == mCharMap.end())
				continue;

			totalWidth += itChar->second.advanceX;
		}

		return totalWidth * mState.scale;
	}

	float Font::getTextWidth(const std::string& text, const size_t numUnicodeCharsSkip, const size_t maxUnicodeCharsRead) const
	{
		if (!mValid || text.empty() || (maxUnicodeCharsRead == 0))
			return 0.0f;

		float totalWidth = 0.0f;
		unsigned int numChars = 0;
		unsigned int numCharsSkip = numUnicodeCharsSkip;

		for (const auto& curCharUnicode : StringUtils::utf8Wrapper(text))
		{
			if (numCharsSkip > 0)
			{
				numCharsSkip--;
				continue;
			}

			if ((numChars++) >= maxUnicodeCharsRead)
				break;

			std::unordered_map<unsigned short, CharacterData>::const_iterator itChar = mCharMap.find(curCharUnicode);
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

		hr::gl::glProgramUniform1i(mGlFragmentProgramID, mGlUniformSampler, 0);
		if (tranformationMatrix)
			hr::gl::glProgramUniformMatrix4fv(mGlVertexProgramID, mGlUniformMatrix, 1, false, tranformationMatrix);

		hr::gl::glBindProgramPipeline(mGlProgramPipelineID);

		mState.scale = scale;
		mState.paintStarted = true;
	}

	void Font::paintEnd()
	{
		if (!mState.paintStarted)
			return;

		commitGL();

		mState.paintStarted = false;
	}

	void Font::paintFlush()
	{
		if (!mState.paintStarted)
			return;

		commitGL();
	}

} } }