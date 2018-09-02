#include "font.hpp"

#include "common/image.hpp"
#include "common/opengl/openGLext.hpp"

#include "libs/stb/stb_truetype.h"

#include <algorithm>
#include <cstddef>

namespace hr { namespace render { namespace tools
{
	namespace
	{
		constexpr unsigned short baseFontSize = 30;
		constexpr unsigned short sdfBufferPadding = 12;
		constexpr unsigned short sdfBufferMargin = 2;

		constexpr std::array<std::pair<uint32_t, uint32_t>, 6> validFontCharacters{ { {32,126}, {192,255}, {880,1008}, {7936,8176}, {1536,1791}, {1040,1299} } };
		constexpr const char* validAditionalFontCharacters{ u8"¥§©®±µ€" };
	}

	void Font::commitGL()
	{
		if (mState.numCharWritten == 0)
			return;

		assert(mState.paintStarted);
		if (mState.paintStarted)
		{
			mGl.fence.wait();
				mGl.arrayBuffer.writeData(mState.charData.data(), sizeof(VertexDataLayout) * mState.numCharWritten * 4, 0);
				hr::gl::glDrawRangeElements(GL_TRIANGLES, 0, mState.numCharWritten * 4, mState.numCharWritten * 6, GL_UNSIGNED_SHORT, (void*)0);
			mGl.fence.place();
		}

		mState.numCharWritten = 0;
	}

	bool Font::createCharData()
	{
		for (const auto& pair : validFontCharacters)
		{
			if (pair.first >= pair.second)
				return false;
		}
		
		size_t numNormalChars = 0;
		for (const auto& pair : validFontCharacters)
			numNormalChars += pair.second - pair.first + 1;

		mCharMap.reserve(numNormalChars + 7);

		for (const auto& pair : validFontCharacters)
		{
			for (auto curCharIndex = pair.first; curCharIndex <= pair.second; curCharIndex++)
				mCharMap[curCharIndex];
		}

		for (const auto& curChar : hr::StringUtils::utf8Wrapper(validAditionalFontCharacters))
			mCharMap[curChar];

		return true;
	}

	bool Font::initFont(const char * const fontFilePath)
	{
		auto fileContent = hr::streams::FileStream::readEntireFile(fontFilePath);
		
		stbtt_fontinfo fontInfo;
		if (!stbtt_InitFont(&fontInfo, reinterpret_cast<const unsigned char*>(fileContent->data()), 0))
			return false;

		float targetScale = stbtt_ScaleForPixelHeight(&fontInfo, baseFontSize);
		{
			int descent;
			stbtt_GetFontVMetrics(&fontInfo, nullptr, &descent, nullptr);

			mFontInfo.baseHeight = -(descent*targetScale); //height that sets us at the baseline
		}

		{
			int x0, x1, y0, y1;
			stbtt_GetFontBoundingBox(&fontInfo, &x0, &y0, &x1, &y1);

			mFontInfo.maxHeight = (y1*targetScale) - (y0*targetScale);
		}

		struct GlyphData
		{
			uint32_t codepoint;
			unsigned short width, height;
			unsigned short texX, texY;

			GlyphData(uint32_t codepoint, unsigned short width, unsigned short height)
				: codepoint(codepoint), width(width), height(height), texX(0), texY(0)
			{ }
		};
		std::vector<GlyphData> validGlyphs;
		validGlyphs.reserve(mCharMap.size());

		int totalArea = 0;
		
		for (auto it = mCharMap.begin(); it != mCharMap.end();)
		{
			auto glyphIndex = stbtt_FindGlyphIndex(&fontInfo, it->first);
			if (glyphIndex == 0)
			{
				it = mCharMap.erase(it);
				continue;
			}

			float advanceX, offsetX, offsetY;
			unsigned short glyphWidth, glyphHeight;
			{
				int advance, leftSideBearing;
				stbtt_GetGlyphHMetrics(&fontInfo, glyphIndex, &advance, &leftSideBearing);
				advanceX = advance * targetScale;
				offsetX = leftSideBearing * targetScale;

				int x0, x1, y0, y1;
				stbtt_GetGlyphBitmapBox(&fontInfo, glyphIndex, targetScale, targetScale, &x0, &y0, &x1, &y1);

				glyphWidth = x1 - x0;
				glyphHeight = y1 - y0;
				offsetY = -y1;
			}

			validGlyphs.push_back(GlyphData(it->first, glyphWidth, glyphHeight));

			auto& curChar = it->second;
			++it;

			curChar.advanceX = advanceX;
			curChar.rect.offsetX = (offsetX - static_cast<float>(sdfBufferPadding));
			curChar.rect.offsetY = (offsetY - static_cast<float>(sdfBufferPadding));
			curChar.rect.width = (static_cast<float>(glyphWidth) + (static_cast<float>(sdfBufferPadding) * 2.0f));
			curChar.rect.height = (static_cast<float>(glyphHeight) + (static_cast<float>(sdfBufferPadding) * 2.0f));

			curChar.skipDraw = stbtt_IsGlyphEmpty(&fontInfo, glyphIndex) != 0;
			if (curChar.skipDraw)
				continue;

			auto charArea = glyphWidth * glyphHeight;
			assert(charArea > 0);
			if (charArea > 0)
				totalArea += (glyphWidth + (sdfBufferPadding * 2)) * (glyphHeight + (sdfBufferPadding * 2));
		}

		int texWidth = hr::Math::iProxPowerOfTwo(hr::Math::ftoi(hr::Math::sqrt(totalArea)));
		int texHeight = 0;

		int texLastWidth = 0, texMaxLineHeight = 0;
		for (size_t charIndex = 0; charIndex < validGlyphs.size(); charIndex++)
		{
			GlyphData& glyphData = validGlyphs[charIndex];
			CharacterData& charData = mCharMap[glyphData.codepoint];

			if (charData.skipDraw)
				continue;

			if (texLastWidth == 0)
			{
				glyphData.texX = 0;
				glyphData.texY = 0;
				texLastWidth = glyphData.texX + glyphData.width + (sdfBufferPadding * 2);
				texMaxLineHeight = glyphData.texY + glyphData.height + (sdfBufferPadding * 2);
				continue;
			}

			if ((texLastWidth + glyphData.width + (sdfBufferPadding * 2)) > texWidth)
			{
				glyphData.texX = 0;
				glyphData.texY = texMaxLineHeight;
				texLastWidth = glyphData.texX + glyphData.width + (sdfBufferPadding * 2);
				texMaxLineHeight = glyphData.texY + glyphData.height + (sdfBufferPadding * 2);
				continue;
			}

			glyphData.texX = texLastWidth;
			glyphData.texY = validGlyphs[charIndex - 1].texY;
			texLastWidth = glyphData.texX + glyphData.width + (sdfBufferPadding * 2);

			auto nextY = glyphData.texY + glyphData.height + (sdfBufferPadding * 2);
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
				CharacterData& charData = mCharMap[glyphData.codepoint];
				if (charData.skipDraw)
					continue;

				charData.rect.minUV[0] = static_cast<float>(glyphData.texX) * invTexWidth;
				charData.rect.minUV[1] = static_cast<float>(glyphData.texY) * invTexHeight;

				charData.rect.maxUV[0] = charData.rect.minUV[0] + static_cast<float>(glyphData.width + sdfBufferPadding + sdfBufferPadding) * invTexWidth;
				charData.rect.maxUV[1] = charData.rect.minUV[1] + static_cast<float>(glyphData.height + sdfBufferPadding + sdfBufferPadding) * invTexHeight;
			}
		}

		hr::imaging::Image<unsigned char, hr::imaging::ImageFormatR> imgFinal(texWidth, texHeight);
		imgFinal.clear(0, 0, 0, 0);

		//read each glyph bitmap
		for (auto& glyphData : validGlyphs)
		{
			CharacterData& charData = mCharMap[glyphData.codepoint];
			if (charData.skipDraw)
				continue;

			int bitmapWidth, bitmapHeight, bitmapXoff, bitmapYoff;
			auto bitmapData = stbtt_GetCodepointSDF(&fontInfo, targetScale, glyphData.codepoint,
				sdfBufferPadding - sdfBufferMargin, 127, 127.5f / static_cast<float>(sdfBufferPadding - sdfBufferMargin),
				&bitmapWidth, &bitmapHeight, &bitmapXoff, &bitmapYoff);

			hr::imaging::ImageView<unsigned char, hr::imaging::ImageFormatR> imgGlyph(bitmapData, bitmapWidth, bitmapHeight);

			imgFinal.setPixelRegion(imgGlyph, glyphData.texX + sdfBufferMargin, glyphData.texY + sdfBufferMargin, true);

			stbtt_FreeSDF(bitmapData, nullptr);
		}

		//get kerning info
		{
			for (auto& curChar1 : mCharMap)
			{
				auto glyphIndex1 = stbtt_FindGlyphIndex(&fontInfo, curChar1.first);

				curChar1.second.kernDataIndices.first = mKerningData.size();
				curChar1.second.kernDataIndices.second = 0;
				
				for (const auto& curChar2 : mCharMap)
				{
					auto kernAmount = stbtt_GetGlyphKernAdvance(&fontInfo, glyphIndex1, stbtt_FindGlyphIndex(&fontInfo, curChar2.first));
					if (kernAmount == 0)
						continue;

					KerningData newKerningData;
					newKerningData.codepoint1 = curChar1.first;
					newKerningData.codepoint2 = curChar2.first;
					newKerningData.offset = static_cast<float>(kernAmount) * targetScale;

					mKerningData.push_back(newKerningData);
				}

				curChar1.second.kernDataIndices.second = mKerningData.size() - curChar1.second.kernDataIndices.first;
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

			auto it = mCharMap.find(curCharUnicode);
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

	size_t Font::countUnicodeChars(float fontScale, const std::string& text, const size_t numUnicodeCharsSkip, float maxWidth) const
	{
		float totalWidth = 0.0f;
		size_t numChars = 0;
		size_t numCharsSkip = numUnicodeCharsSkip;

		for (const auto& curCharUnicode : StringUtils::utf8Wrapper(text))
		{
			if (numCharsSkip > 0)
			{
				numCharsSkip--;
				continue;
			}

			auto itChar = mCharMap.find(curCharUnicode);
			if (itChar == mCharMap.end())
				continue;

			totalWidth += (itChar->second.advanceX * fontScale);
			if (totalWidth > maxWidth)
				return numChars;

			numChars++;
		}

		return numChars;
	}

	float Font::getTextWidth(const std::string& text) const
	{
		float totalWidth = 0.0f;
		for (const auto& curCharUnicode : StringUtils::utf8Wrapper(text))
		{
			auto itChar = mCharMap.find(curCharUnicode);
			if (itChar == mCharMap.end())
				continue;

			totalWidth += itChar->second.advanceX;
		}

		return totalWidth;
	}

	float Font::getTextWidth(const std::string& text, const size_t numUnicodeCharsSkip, const size_t maxUnicodeCharsRead) const
	{
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

			auto itChar = mCharMap.find(curCharUnicode);
			if (itChar == mCharMap.end())
				continue;

			totalWidth += itChar->second.advanceX;
		}

		return totalWidth;
	}

	float Font::getCharKerning(const CharacterData& leftCharData, unsigned short leftCharUnicodeID, unsigned short rightCharUnicodeID) const
	{
		if ((leftCharData.kernDataIndices.second <= 0) || (rightCharUnicodeID <= 0))
			return 0.0f;

		auto kernWalker = mKerningData.data() + leftCharData.kernDataIndices.first;
		for (auto count = leftCharData.kernDataIndices.second; count > 0; count--, kernWalker++)
		{
			if (kernWalker->codepoint2 == rightCharUnicodeID)
				return kernWalker->offset;
		}

		return 0.0f;
	}

	Font::Font(const char * const fontFilePath, unsigned int glVertexProgramID, unsigned int glFragmentProgramID, unsigned int glProgramPipelineID)
		: mGlVertexProgramID(glVertexProgramID), mGlFragmentProgramID(glFragmentProgramID), mGlProgramPipelineID(glProgramPipelineID)
	{
		mState.stateColor.set(1.0f, 1.0f, 1.0f, 1.0f);
	
		if (!fontFilePath)
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

		hr::gl::glEnableVertexArrayAttrib(mGl.vertexArray.id(), 0);
		hr::gl::glEnableVertexArrayAttrib(mGl.vertexArray.id(), 1);
		hr::gl::glEnableVertexArrayAttrib(mGl.vertexArray.id(), 4);

		hr::gl::glVertexArrayAttribBinding(mGl.vertexArray.id(), 0, 0);
		hr::gl::glVertexArrayAttribFormat(mGl.vertexArray.id(), 0, 2, GL_FLOAT, false, offsetof(Font::VertexDataLayout, pos));

		hr::gl::glVertexArrayAttribBinding(mGl.vertexArray.id(), 1, 0);
		hr::gl::glVertexArrayAttribFormat(mGl.vertexArray.id(), 1, 2, GL_FLOAT, false, offsetof(Font::VertexDataLayout, uv));

		hr::gl::glVertexArrayAttribBinding(mGl.vertexArray.id(), 4, 0);
		hr::gl::glVertexArrayAttribFormat(mGl.vertexArray.id(), 4, 4, GL_UNSIGNED_BYTE, true, offsetof(Font::VertexDataLayout, color));

		hr::gl::glVertexArrayElementBuffer(mGl.vertexArray.id(), mGl.elementArrayBuffer.id());
		hr::gl::glVertexArrayVertexBuffer(mGl.vertexArray.id(), 0, mGl.arrayBuffer.id(), 0, sizeof(Font::VertexDataLayout));

		mGl.sampler.init();
		mGl.sampler.setMinFilter(hr::gl::objects::Sampler::FilterType::Linear);
		mGl.sampler.setMagFilter(hr::gl::objects::Sampler::FilterType::Linear);
		mGl.sampler.setWrap(hr::gl::objects::Sampler::WrapType::ClampBorder);
		mGl.sampler.setBorderColor(0.0f, 0.0f, 0.0f, 1.0f);

		if (!createCharData())
			return;
		if (!initFont(fontFilePath))
			return;

		mValid = true;
	}

	Font::~Font()
	{
		mKerningData.clear();
		mCharMap.clear();
	}

	bool Font::isValid() const
	{
		return mValid;
	}

	float Font::getMaxHeight(size_t targetSize) const
	{
		if (!mValid || (targetSize <= 0))
			return 0.0f;

		return mFontInfo.maxHeight * (static_cast<float>(targetSize) / baseFontSize);
	}

	float Font::getTextWidth(size_t targetSize, const std::string& text) const
	{
		if (!mValid || text.empty() || (targetSize <= 0))
			return 0.0f;

		return getTextWidth(text) * (static_cast<float>(targetSize) / baseFontSize);
	}

	float Font::getTextWidth(size_t targetSize, const std::string& text, const size_t numUnicodeCharsSkip, const size_t maxUnicodeCharsRead) const
	{
		return getTextWidth(text, numUnicodeCharsSkip, maxUnicodeCharsRead) * (static_cast<float>(targetSize) / baseFontSize);
	}

	void Font::paintBegin(size_t targetSize, const float * const tranformationMatrix)
	{
		if (mState.paintStarted || (targetSize <= 0))
			return;

		mGl.texture.bind(0);
		mGl.sampler.bind(0);
		mGl.vertexArray.bind();

		hr::gl::glProgramUniform1i(mGlFragmentProgramID, mGlUniformSampler, 0);
		if (tranformationMatrix)
			hr::gl::glProgramUniformMatrix4fv(mGlVertexProgramID, mGlUniformMatrix, 1, false, tranformationMatrix);

		hr::gl::glBindProgramPipeline(mGlProgramPipelineID);

		mState.scale = static_cast<float>(targetSize) / baseFontSize;
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

	void Font::layout(const std::string& text, const float maxWidth, std::function<void(size_t, size_t, size_t)> writeCb) const
	{
		if (text.empty() || (maxWidth <= 0.0f) || !writeCb)
			return;

		auto textWidth = getTextWidth(text) * mState.scale;
		auto numLines = static_cast<unsigned int>(std::ceil(textWidth / maxWidth));
		if (numLines == 0)
			numLines = 1;

		unsigned int numCharsWritten = 0;
		for (; numLines > 0; numLines--)
		{
			auto maxChars = countUnicodeChars(mState.scale, text, numCharsWritten, maxWidth);

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

		auto it = mCharMap.find(unicodeChar);
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

} } }