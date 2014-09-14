#include "tools.hpp"

#include "openGLext.hpp"
#include "common\Image.hpp"
#include "common\Platform.hpp"

#include <algorithm>

#include <ft2build.h>
#include FT_FREETYPE_H
#include FT_BITMAP_H
#include FT_GLYPH_H
#include FT_RENDER_H

const unsigned short validFontCharacters[] = {32,126, 192,255, 880,1008, 7936,8176, 1536,1791, 1040,1299};
const wchar_t validAditionalFontCharacters[] = {L"¥§©®±µ€"};

namespace HorseRadish
{
	namespace OpenGL
	{
		namespace Tools
		{
			FontManager::FontManager()
			{
				auto fontIndexArray = new unsigned short[Font::numMaxChar * 5];
				if (fontIndexArray == nullptr)
					return;

				auto curIndex = 0;
				for (int i = 0; i < Font::numMaxChar; i++)
				{
					fontIndexArray[i * 5 + 0] = curIndex + 3;
					fontIndexArray[i * 5 + 1] = curIndex + 0;
					fontIndexArray[i * 5 + 2] = curIndex + 2;
					fontIndexArray[i * 5 + 3] = curIndex + 1;
					fontIndexArray[i * 5 + 4] = Font::primitiveRestartIndex;
					curIndex += 4;
				}

				HorseRadish::OpenGL::glCreateBuffers(1, &this->glArrayBufferID);
				HorseRadish::OpenGL::glNamedBufferStorage(this->glArrayBufferID, sizeof(Font::VertexDataLayout) * Font::numMaxChar * 4, nullptr, GL_DYNAMIC_STORAGE_BIT);

				HorseRadish::OpenGL::glCreateBuffers(1, &this->glElementArrayBufferID);
				HorseRadish::OpenGL::glNamedBufferStorage(this->glElementArrayBufferID, sizeof(unsigned short) * Font::numMaxChar * 5, fontIndexArray, GL_DYNAMIC_STORAGE_BIT);

				HorseRadish::OpenGL::glCreateVertexArrays(1, &this->glVertexArrayID);

				HorseRadish::OpenGL::glEnableVertexArrayAttrib(this->glVertexArrayID, 0);
				HorseRadish::OpenGL::glEnableVertexArrayAttrib(this->glVertexArrayID, 1);
				HorseRadish::OpenGL::glEnableVertexArrayAttrib(this->glVertexArrayID, 4);

				HorseRadish::OpenGL::glVertexArrayAttribBinding(this->glVertexArrayID, 0, 0);
				HorseRadish::OpenGL::glVertexArrayAttribFormat(this->glVertexArrayID, 0, 2, GL_FLOAT, false, 0);

				HorseRadish::OpenGL::glVertexArrayAttribBinding(this->glVertexArrayID, 1, 0);
				HorseRadish::OpenGL::glVertexArrayAttribFormat(this->glVertexArrayID, 1, 2, GL_FLOAT, false, 8);

				HorseRadish::OpenGL::glVertexArrayAttribBinding(this->glVertexArrayID, 4, 0);
				HorseRadish::OpenGL::glVertexArrayAttribFormat(this->glVertexArrayID, 4, 4, GL_UNSIGNED_BYTE, true, 16);

				HorseRadish::OpenGL::glVertexArrayElementBuffer(this->glVertexArrayID, this->glElementArrayBufferID);
				HorseRadish::OpenGL::glVertexArrayVertexBuffer(this->glVertexArrayID, 0, this->glArrayBufferID, 0, sizeof(Font::VertexDataLayout));
			}

			FontManager::~FontManager()
			{
				for (auto &font : this->fonts)
					delete font;
				this->fonts.clear();

				HorseRadish::OpenGL::glDeleteVertexArrays(1, &this->glVertexArrayID);
				HorseRadish::OpenGL::glDeleteBuffers(1, &this->glElementArrayBufferID);
				HorseRadish::OpenGL::glDeleteBuffers(1, &this->glArrayBufferID);
				this->glVertexArrayID = 0;
				this->glElementArrayBufferID = 0;
				this->glArrayBufferID = 0;
			}

			Font* FontManager::CreateFont(const int fontSize, const char * const fontFilePath, unsigned int glProgramID)
			{
				if ((fontSize <= 2) || (fontFilePath == nullptr))
					return nullptr;

				auto newFonte = new Font(this, fontSize, fontFilePath, glProgramID);

				this->fonts.push_back(newFonte);
				return newFonte;
			}

			unsigned short Font::primitiveRestartIndex = 65500;
			int Font::numMaxChar = 512;

			Font::Font(FontManager * const fontManager, const int fontSize, const char * const fontFilePath, unsigned int glProgramID)
				: fontSize(fontSize), glProgID(glProgramID), fontManager(fontManager), numCharWritten(0)
			{
				activo = false;

				if (fontSize <= 2 || fontFilePath == nullptr)
					return;

				glUniformSampler = HorseRadish::OpenGL::glGetUniformLocation(glProgID, "texTextSampler");
				glUniformMatrix = HorseRadish::OpenGL::glGetUniformLocation(glProgID, "transformationMatrix");

				this->fontDataArray = new VertexDataLayout[Font::numMaxChar * 4];
				if (this->fontDataArray == nullptr)
					return;

				if (initFont(fontFilePath) == false)
					return;

				this->stateColor.Set(1.0f, 1.0f, 1.0f, 1.0f);

				this->paintStarted = false;
				this->activo = true;
			}

			Font::~Font()
			{
				if (HorseRadish::OpenGL::glIsTexture(this->glTexID))
					HorseRadish::OpenGL::glDeleteTextures(1, &this->glTexID);
				if (HorseRadish::OpenGL::glIsSampler(this->glSamplerID))
					HorseRadish::OpenGL::glDeleteSamplers(1, &this->glSamplerID);
				this->glTexID = 0;
				this->glSamplerID = 0;

				this->kerningData.clear();
				this->charData.clear();

				delete[] this->fontDataArray;
				this->fontDataArray = nullptr;
			}

			void Font::commitGL() const
			{
				if (numCharWritten == 0)
					return;

				HorseRadish::OpenGL::glNamedBufferSubData(this->fontManager->glArrayBufferID, 0, sizeof(VertexDataLayout) * numCharWritten * 4, fontDataArray);

				assert(paintStarted);

				HorseRadish::OpenGL::glBindBuffer(GL_ARRAY_BUFFER, this->fontManager->glArrayBufferID);
				HorseRadish::OpenGL::glDrawRangeElements(GL_TRIANGLE_STRIP, 0, numCharWritten * 4, numCharWritten * 5, GL_UNSIGNED_SHORT, (void*)0);

				numCharWritten = 0;
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

				this->charData.resize(numNormalChars + numExtraChars);

				curCharWrite = 0;
				for (int curPair = 0; curPair < numCharPairs; curPair++)
				{
					for (int curCharIndex = validFontCharacters[curPair * 2 + 0]; curCharIndex <= validFontCharacters[curPair * 2 + 1]; curCharIndex++, curCharWrite++)
						this->charData[curCharWrite].unicodeID = curCharIndex;
				}

				assert(curCharWrite == numNormalChars);

				for (int curCharIndex = 0; curCharIndex < numExtraChars; curCharIndex++, curCharWrite++)
					charData[curCharWrite].unicodeID = strExtraChars.GetUnicodeAt(curCharIndex);

				assert(curCharWrite == this->charData.size());

				return true;
			}

			void Font::cleanCharData()
			{
				int numTotalValidos = 0;
				for (const auto& curChar : this->charData)
				{
					if (curChar.unicodeID != 0)
						numTotalValidos++;
				}

				if (numTotalValidos == this->charData.size())
					return;

				std::vector<CharacterData> newCharData;
				newCharData.resize(numTotalValidos);

				int newIndex = 0;
				for (const auto& curChar : this->charData)
				{
					if (curChar.unicodeID != 0)
					{
						memcpy(&newCharData[newIndex], &curChar, sizeof(CharacterData));
						newIndex++;
					}
				}

				std::swap(newCharData, this->charData);

				assert(numTotalValidos == newIndex);
				assert(numTotalValidos == this->charData.size());
			}

			bool Font::initFont(const char * const fontFilePath)
			{
				float borderColor[4];
				FT_Library ftLibrary;
				FT_Face ftFace;
				FT_Int32 ftLoadFlags;
				int texWidth, texHeight, texMaxLineHeight, texLastWidth, totalSize[2];
				bool primeiraEntrada;

				if (FT_Init_FreeType(&ftLibrary))
					return false;

				if (FT_New_Face(ftLibrary, fontFilePath, 0, &ftFace))
				{
					FT_Done_FreeType(ftLibrary);
					return false;
				}

				FT_Set_Char_Size(ftFace, 0, fontSize << 6, 96, 96);

				maxHeight = ftFace->size->metrics.height >> 6;
				baseHeight = -(ftFace->size->metrics.descender >> 6);

				if (createCharData() == false)
				{
					FT_Done_Face(ftFace);
					FT_Done_FreeType(ftLibrary);
					return false;
				}

				ftLoadFlags = FT_LOAD_FORCE_AUTOHINT;//FT_LOAD_DEFAULT;

				totalSize[0] = totalSize[1] = 0;
				for (auto& curChar : this->charData)
				{
					auto freetypeCharIndex = FT_Get_Char_Index(ftFace, curChar.unicodeID);
					if (freetypeCharIndex == 0)
					{
						curChar.unicodeID = 0;
						continue;
					}

					auto freetypeError = FT_Load_Glyph(ftFace, freetypeCharIndex, ftLoadFlags);
					if (freetypeError != 0)
					{
						curChar.unicodeID = 0;
						continue;
					}

					curChar.advance = ftFace->glyph->metrics.horiAdvance >> 6;
					curChar.maxUV[0] = ftFace->glyph->metrics.width >> 6;
					curChar.maxUV[1] = ftFace->glyph->metrics.height >> 6;
					curChar.offset[0] = ftFace->glyph->metrics.horiBearingX >> 6;
					curChar.offset[1] = -((ftFace->glyph->metrics.height - ftFace->glyph->metrics.horiBearingY) >> 6);

					if ((curChar.maxUV[0] != 0) && (curChar.maxUV[1] != 0))
					{
						totalSize[0] += curChar.maxUV[0];
						totalSize[1] = HorseRadish::Math::iMax(totalSize[1], curChar.maxUV[1]);
					}
				}

				texWidth = HorseRadish::Math::iProxPowerOfTwo(HorseRadish::Math::ftoi(HorseRadish::Math::sqrt(totalSize[0] * totalSize[1])));
				texHeight = 0;

				cleanCharData();

				primeiraEntrada = true;
				for (int charIndex = 0; charIndex < this->charData.size(); charIndex++)
				{
					if (this->charData[charIndex].unicodeID == 0)
						continue;

					if ((this->charData[charIndex].maxUV[0] == 0) || (this->charData[charIndex].maxUV[1] == 0))
						continue;

					if (primeiraEntrada == true)
					{
						this->charData[charIndex].minUV[0] = 0;
						this->charData[charIndex].minUV[1] = 0;
						texLastWidth = this->charData[charIndex].minUV[0] + this->charData[charIndex].maxUV[0];
						texMaxLineHeight = this->charData[charIndex].minUV[1] + this->charData[charIndex].maxUV[1];
						primeiraEntrada = false;
						continue;
					}

					if ((texLastWidth + 2 + charData[charIndex].maxUV[0] + 1) > texWidth)
					{
						this->charData[charIndex].minUV[0] = 0;
						this->charData[charIndex].minUV[1] = texMaxLineHeight + 2;
						texLastWidth = this->charData[charIndex].minUV[0] + this->charData[charIndex].maxUV[0];
						texMaxLineHeight = this->charData[charIndex].minUV[1] + this->charData[charIndex].maxUV[1];
						continue;
					}

					this->charData[charIndex].minUV[0] = texLastWidth + 2;
					this->charData[charIndex].minUV[1] = this->charData[charIndex - 1].minUV[1];
					texLastWidth = this->charData[charIndex].minUV[0] + this->charData[charIndex].maxUV[0];
					if ((this->charData[charIndex].minUV[1] + this->charData[charIndex].maxUV[1]) > texMaxLineHeight)
						texMaxLineHeight = this->charData[charIndex].minUV[1] + this->charData[charIndex].maxUV[1];
				}

				texHeight = texMaxLineHeight;

				auto imgFinal = new HorseRadish::Imaging::Image(texWidth, texHeight, HorseRadish::Imaging::Image::UByte, HorseRadish::Imaging::Image::Alpha);
				imgFinal->Clear(0.0f, 0.0f, 0.0f, 0.0f);

				for (const auto& curChar : this->charData)
				{
					FT_Glyph ftGlyph;
					FT_Bitmap bitmapGlyph;

					if (curChar.unicodeID == 0)
						continue;

					if ((curChar.maxUV[0] == 0) || (curChar.maxUV[1] == 0))
						continue;

					if (FT_Load_Glyph(ftFace, FT_Get_Char_Index(ftFace, curChar.unicodeID), ftLoadFlags))
						continue;

					if (FT_Get_Glyph(ftFace->glyph, &ftGlyph))
						continue;

					FT_Glyph_To_Bitmap(&ftGlyph, FT_RENDER_MODE_NORMAL, nullptr, 1);
					bitmapGlyph = ((FT_BitmapGlyph)ftGlyph)->bitmap;

					if ((curChar.maxUV[0] != bitmapGlyph.width) || (curChar.maxUV[1] != bitmapGlyph.rows))
						continue;

					HorseRadish::Imaging::Image imgAux(bitmapGlyph.width, bitmapGlyph.rows, HorseRadish::Imaging::Image::UByte, HorseRadish::Imaging::Image::Alpha, bitmapGlyph.buffer, false);
					imgAux.Flip();
					imgFinal->CopyRegion(&imgAux, curChar.minUV[0], curChar.minUV[1]);
				}

				std::sort(this->charData.begin(), this->charData.end(), [](const CharacterData &a, const CharacterData &b) { return (a.unicodeID < b.unicodeID); });

				if (FT_HAS_KERNING(ftFace) != 0)
				{
					for (const auto& curChar1 : this->charData)
					{
						if (curChar1.unicodeID == 0)
							continue;

						auto glyphIndex1 = FT_Get_Char_Index(ftFace, curChar1.unicodeID);

						for (const auto& curChar2 : this->charData)
						{
							FT_Vector kernVector;

							if ((curChar2.unicodeID == 0) || (curChar1.unicodeID == curChar2.unicodeID))
								continue;

							auto glyphIndex2 = FT_Get_Char_Index(ftFace, curChar2.unicodeID);
							if (glyphIndex1 == glyphIndex2)
								continue;

							if (FT_Get_Kerning(ftFace, glyphIndex1, glyphIndex2, FT_KERNING_DEFAULT, &kernVector) != 0)
								continue;

							if (kernVector.x == 0)
								continue;

							KerningData newKerningData;
							newKerningData.char1 = curChar1.unicodeID;
							newKerningData.char2 = curChar2.unicodeID;
							newKerningData.offset = kernVector.x >> 6;

							this->kerningData.push_back(newKerningData);
						}
					}

					this->kerningData.push_back(KerningData());
				}

				FT_Done_Face(ftFace);
				FT_Done_FreeType(ftLibrary);

				for (auto& curChar : this->charData)
				{
					if (curChar.unicodeID == 0)
						continue;

					for (const auto& curKerning : this->kerningData)
					{
						if ((curKerning.char1 == 0) || (curKerning.char1 != curChar.unicodeID))
							continue;

						curChar.kernData = &curKerning;
						break;
					}
				}

				borderColor[0] = borderColor[1] = borderColor[2] = borderColor[3] = 0.0f;

				imgFinal->ChangeFormat(HorseRadish::Imaging::Image::LumAlpha);
				imgFinal->ChannelCopy(HorseRadish::Imaging::Image::Alpha, HorseRadish::Imaging::Image::Red);
				imgFinal->ChangeFormat(HorseRadish::Imaging::Image::RGBA);

				HorseRadish::OpenGL::glCreateTextures(GL_TEXTURE_RECTANGLE, 1, &this->glTexID);
				HorseRadish::OpenGL::glTextureStorage2D(this->glTexID, 1, GL_RGBA8, imgFinal->GetWidth(), imgFinal->GetHeight());
				HorseRadish::OpenGL::glTextureSubImage2D(this->glTexID, 0, 0, 0, imgFinal->GetWidth(), imgFinal->GetHeight(), GL_RGBA, GL_UNSIGNED_BYTE, imgFinal->GetPixelData());

				HorseRadish::OpenGL::glCreateSamplers(1, &this->glSamplerID);
				HorseRadish::OpenGL::glSamplerParameteri(this->glSamplerID, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
				HorseRadish::OpenGL::glSamplerParameteri(this->glSamplerID, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
				HorseRadish::OpenGL::glSamplerParameteri(this->glSamplerID, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
				HorseRadish::OpenGL::glSamplerParameteri(this->glSamplerID, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
				HorseRadish::OpenGL::glSamplerParameteri(this->glSamplerID, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_BORDER);
				HorseRadish::OpenGL::glSamplerParameterfv(this->glSamplerID, GL_TEXTURE_BORDER_COLOR, borderColor);

				delete imgFinal;
				imgFinal = nullptr;

				return true;
			}

			void Font::internalWrite(const float &px, const float &py, HorseRadish::String::Iterator &strIt) const
			{
				float posX, posY;
				VertexDataLayout *writeData;
				unsigned int curCharUnicode;
				const CharacterData *curCharData;

				if (activo == false)
					return;

				posX = px;
				posY = py + baseHeight;

				while (strIt.IsLast() == false)
				{
					writeData = fontDataArray + (numCharWritten * 4);

					while (numCharWritten < Font::numMaxChar)
					{
						unsigned char colorTemp[4];

						curCharUnicode = strIt.Read();
						if (curCharUnicode == 0)
							break;

						curCharData = getCharData(curCharUnicode);
						if (curCharData == nullptr)
							continue;

						if (curCharData->maxUV[1] == 0)
						{
							posX += (float)curCharData->advance;
							continue;
						}

						writeData[0].px = writeData[3].px = posX + float(curCharData->offset[0]);
						writeData[1].px = writeData[2].px = writeData[0].px + float(curCharData->maxUV[0]);
						writeData[0].py = writeData[1].py = posY + ((float)curCharData->offset[1]);
						writeData[2].py = writeData[3].py = writeData[0].py + ((float)curCharData->maxUV[1]);

						writeData[0].tu = writeData[3].tu = (float)(curCharData->minUV[0]);
						writeData[1].tu = writeData[2].tu = (float)(curCharData->minUV[0] + curCharData->maxUV[0]);
						writeData[0].tv = writeData[1].tv = (float)(curCharData->minUV[1]);
						writeData[2].tv = writeData[3].tv = (float)(curCharData->minUV[1] + curCharData->maxUV[1]);

						stateColor.Write(colorTemp);
						memcpy(writeData[0].rgba, colorTemp, sizeof(unsigned char) * 4);
						memcpy(writeData[1].rgba, colorTemp, sizeof(unsigned char) * 4);
						memcpy(writeData[2].rgba, colorTemp, sizeof(unsigned char) * 4);
						memcpy(writeData[3].rgba, colorTemp, sizeof(unsigned char) * 4);

						writeData += 4;
						numCharWritten++;
						posX += (float)curCharData->advance;

						if (!this->kerningData.empty())
							posX += (float)getCharKerning(curCharData, *strIt);
					}

					if (numCharWritten >= Font::numMaxChar)
						commitGL();
				}
			}

			const Font::CharacterData* Font::getCharData(int unicodeID) const
			{
				CharacterData cmpProxy;
				cmpProxy.unicodeID = unicodeID;

				auto charIndex = std::lower_bound(this->charData.begin(), this->charData.end(), cmpProxy, [](const CharacterData &a, const CharacterData &b) { return (a.unicodeID < b.unicodeID); });
				return (charIndex == this->charData.end()) ? nullptr : &(*charIndex);
			}

			const int Font::getCharKerning(const CharacterData * const leftCharData, unsigned short rightCharUnicodeID) const
			{
				if ((leftCharData == nullptr) || (leftCharData->kernData == nullptr) || (rightCharUnicodeID <= 0))
					return 0;

				for (const KerningData *curKerning = leftCharData->kernData; true; curKerning++)
				{
					if (curKerning->char1 != leftCharData->unicodeID)
						break;

					if (curKerning->char2 == rightCharUnicodeID)
						return curKerning->offset;
				}

				return 0;
			}

			void Font::write(const float &px, const float &py, HorseRadish::String::Iterator &iterator) const
			{
				internalWrite(px, py, iterator);
			}

			void Font::write(HorseRadish::String::Iterator &iterator) const
			{
				write(0.0f, 0.0f, iterator);
			}

			void Font::write(const float &px, const float &py, const HorseRadish::String &texto) const
			{
				internalWrite(px, py, HorseRadish::String::Iterator(texto));
			}

			void Font::write(const HorseRadish::String &texto) const
			{
				write(0.0f, 0.0f, texto);
			}

			int Font::writeChar(const unsigned int &unicodeChar) const
			{
				return writeChar(0.0f, 0.0f, unicodeChar);
			}

			int Font::writeChar(const float &px, const float &py, const unsigned int &unicodeChar) const
			{
				unsigned char colorTemp[4];

				if (this->activo == false)
					return 0;

				auto charData = getCharData(unicodeChar);
				if (charData == nullptr)
					return 0;

				if (charData->maxUV[1] == 0)
					return charData->advance;

				if (numCharWritten >= Font::numMaxChar)
					commitGL();

				auto writeData = fontDataArray + (numCharWritten * 4);

				writeData[0].px = writeData[3].px = px + float(charData->offset[0]);
				writeData[1].px = writeData[2].px = writeData[0].px + float(charData->maxUV[0]);
				writeData[0].py = writeData[1].py = py + ((float)baseHeight) + ((float)charData->offset[1]);
				writeData[2].py = writeData[3].py = writeData[0].py + ((float)charData->maxUV[1]);

				writeData[0].tu = writeData[3].tu = (float)(charData->minUV[0]);
				writeData[1].tu = writeData[2].tu = (float)(charData->minUV[0] + charData->maxUV[0]);
				writeData[0].tv = writeData[1].tv = (float)(charData->minUV[1]);
				writeData[2].tv = writeData[3].tv = (float)(charData->minUV[1] + charData->maxUV[1]);

				stateColor.Write(colorTemp);
				memcpy(writeData[0].rgba, colorTemp, sizeof(unsigned char) * 4);
				memcpy(writeData[1].rgba, colorTemp, sizeof(unsigned char) * 4);
				memcpy(writeData[2].rgba, colorTemp, sizeof(unsigned char) * 4);
				memcpy(writeData[3].rgba, colorTemp, sizeof(unsigned char) * 4);

				numCharWritten++;
				return charData->advance;
			}

			void Font::draw() const
			{
				commitGL();
			}

			void Font::setColor(const float &r, const float &g, const float &b, const float &a)
			{
				stateColor.Set(r, g, b, a);
			}

			void Font::setColor(const float &r, const float &g, const float &b)
			{
				stateColor.Set(r, g, b, 1.0f);
			}

			void Font::setColor(const float * const novaCor)
			{
				stateColor.Set(novaCor);
			}

			int Font::getCharWidth(const unsigned int &unicodeChar) const
			{
				auto curCharData = this->getCharData(unicodeChar);
				if (curCharData == nullptr)
					return 0;
				return curCharData->advance;
			}

			int Font::getStringWidth(const HorseRadish::String &texto) const
			{
				if ((!activo) || (texto.GetSizeChars() == 0))
					return 0;

				int totalWidth = 0;
				for (HorseRadish::String::Iterator it(texto); it.IsLast() == false; it++)
				{
					auto curCharData = getCharData(*it);
					if (curCharData == nullptr)
						continue;

					totalWidth += curCharData->advance;
				}

				return totalWidth;
			}

			int Font::getStringWidth(const HorseRadish::String &texto, const unsigned int numCharMeta) const
			{
				if ((!activo) || (texto.GetSizeChars() == 0) || (numCharMeta == 0))
					return 0;

				int totalWidth = 0;
				for (HorseRadish::String::Iterator it(texto); it.IsLast() == false; it++)
				{
					if (it.GetCaracterPosition() >= numCharMeta)
						break;

					auto curCharData = getCharData(*it);
					if (curCharData == nullptr)
						continue;

					totalWidth += curCharData->advance;
				}

				return totalWidth;
			}

			void Font::paintBegin(const float * const tranformationMatrix)
			{
				if (paintStarted)
					return;

				HorseRadish::OpenGL::glBindTextureUnit(0, this->glTexID);
				HorseRadish::OpenGL::glBindSampler(0, this->glSamplerID);
				HorseRadish::OpenGL::glBindVertexArray(this->fontManager->glVertexArrayID);

				HorseRadish::OpenGL::glEnable(GL_PRIMITIVE_RESTART);
				HorseRadish::OpenGL::glPrimitiveRestartIndex(Font::primitiveRestartIndex);

				HorseRadish::OpenGL::glUseProgram(glProgID);
				HorseRadish::OpenGL::glUniform1i(glUniformSampler, 0);
				if (tranformationMatrix != nullptr)
					HorseRadish::OpenGL::glUniformMatrix4fv(glUniformMatrix, 1, GL_FALSE, tranformationMatrix);

				this->paintStarted = true;
			}

			void Font::paintEnd()
			{
				HorseRadish::OpenGL::glDisable(GL_PRIMITIVE_RESTART);

				this->paintStarted = false;
			}

		} //Tools
	} //OpenGL
} //HorseRadish