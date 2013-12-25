#include "tools.hpp"
#include "openGLext.hpp"
#include "common\Platform.hpp"
#include "common\Sorting.hpp"
#include "common\Image.hpp"

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

/*§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§
§§§§§§   -= Classe FontManager =-   §§§§§
§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§*/
FontManager::FontManager()
{
	unsigned short *fontIndexArray, curIndex;

	//preciso de um buffer temporário para os índices: com triangles strips, por caracter dá 4 indices + 1 (para reiniciar)
	fontIndexArray = new unsigned short[Font::numMaxChar * 5];
	if (fontIndexArray == nullptr)
		return;

	//posso e devo já criar os indices a usar
	curIndex = 0;
	for(int i=0; i<Font::numMaxChar; i++)
	{
		fontIndexArray[i*5+0] = curIndex+3;
		fontIndexArray[i*5+1] = curIndex+0;
		fontIndexArray[i*5+2] = curIndex+2;
		fontIndexArray[i*5+3] = curIndex+1;
		fontIndexArray[i*5+4] = Font::primitiveRestartIndex;
		curIndex += 4;
	}

	//crio o VBO para os dados
	HorseRadish::OpenGL::glGenBuffers(1, &this->glArrayBufferID);
	HorseRadish::OpenGL::glBindBuffer(GL_ARRAY_BUFFER, this->glArrayBufferID);
	HorseRadish::OpenGL::glBufferData(GL_ARRAY_BUFFER, sizeof(Font::VertexDataLayout) * Font::numMaxChar * 4, nullptr, GL_DYNAMIC_DRAW);

	//crio o VBO para os indices
	HorseRadish::OpenGL::glGenBuffers(1, &this->glElementArrayBufferID);
	HorseRadish::OpenGL::glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, this->glElementArrayBufferID);
	HorseRadish::OpenGL::glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(unsigned short) * Font::numMaxChar * 5, fontIndexArray, GL_STREAM_DRAW);
	
	//crio o VAO
	HorseRadish::OpenGL::glGenVertexArrays(1, &this->glVertexArrayID);
	HorseRadish::OpenGL::glBindVertexArray(this->glVertexArrayID);
	HorseRadish::OpenGL::glBindBuffer(GL_ARRAY_BUFFER, this->glArrayBufferID);
	HorseRadish::OpenGL::glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, this->glElementArrayBufferID);
	HorseRadish::OpenGL::glEnableVertexAttribArray(0);
	HorseRadish::OpenGL::glEnableVertexAttribArray(1);
	HorseRadish::OpenGL::glEnableVertexAttribArray(4);
	HorseRadish::OpenGL::glVertexAttribPointer(0, 2, GL_FLOAT, false, sizeof(Font::VertexDataLayout), (void*)0);
	HorseRadish::OpenGL::glVertexAttribPointer(1, 2, GL_FLOAT, false, sizeof(Font::VertexDataLayout), (void*)8);
	HorseRadish::OpenGL::glVertexAttribPointer(4, 4, GL_UNSIGNED_BYTE, true, sizeof(Font::VertexDataLayout), (void*)16);
	HorseRadish::OpenGL::glBindVertexArray(0);
}

FontManager::~FontManager()
{
	//apago cada font individual
	for(int i = this->fonts.GetNumElements() - 1; i >= 0; i--)
		delete this->fonts[i];

	//agora que já apaguei tudo, limpo os ponteiros
	this->fonts.Clear();

	//apago tudo do GL
	HorseRadish::OpenGL::glDeleteVertexArrays(1, &this->glVertexArrayID);
	HorseRadish::OpenGL::glDeleteBuffers(1, &this->glElementArrayBufferID);
	HorseRadish::OpenGL::glDeleteBuffers(1, &this->glArrayBufferID);
	this->glVertexArrayID = 0;
	this->glElementArrayBufferID = 0;
	this->glArrayBufferID = 0;
}

Font* FontManager::CreateFont(const int fontSize, const char * const fontFilePath, unsigned int glProgramID)
{
	Font* novaFonte;

	//verifico algumas coisas
	if ( (fontSize <= 2) || (fontFilePath == nullptr))
		return nullptr;

	//crio a nova fonte
	novaFonte = new Font(this, fontSize, fontFilePath, glProgramID);

	//guardo-a internamente e pronto
	this->fonts.Add(novaFonte);
	return novaFonte;
}

/*§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§
§§§§§§   -= Classe Font =-   §§§§§
§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§*/

unsigned short Font::primitiveRestartIndex = 65500;
int Font::numMaxChar = 512;

int Font::sortCharData(const Font::CharacterData &objA, const Font::CharacterData &objB)
{
	if (objA.unicodeID < objB.unicodeID)
		return -1;
	if (objA.unicodeID > objB.unicodeID)
		return 1;
	return 0;
}

int Font::findCharData(const Font::CharacterData &objA, const Font::CharacterData &objB)
{
	if (objA.unicodeID == objB.unicodeID)
		return 0;
	if (objA.unicodeID < objB.unicodeID)
		return -1;
	return 1;
}

Font::Font(FontManager * const fontManager, const int fontSize, const char * const fontFilePath, unsigned int glProgramID)
	: kerningData(16)
{
	//posso já guardar isto
	this->fontManager = fontManager;

	//por defeito não tou activo
	activo=false;
	
	//sem isto não há nada pra ninguem
	if (fontSize<=2 || fontFilePath==nullptr)
		return;

	//já posso guardar isto e aproveito para tirar alguns dados do programa
	glProgID = glProgramID;
	glUniformSampler = HorseRadish::OpenGL::glGetUniformLocation(glProgID, "texTextSampler");
	glUniformMatrix = HorseRadish::OpenGL::glGetUniformLocation(glProgID, "transformationMatrix");

	//não escrevi nada
	this->numCharWritten = 0;

	//posso já guardar isto
	this->fontSize = fontSize;

	//arranjo memória para guardar os dados dos caracteres escritos
	this->fontDataArray = new VertexDataLayout[Font::numMaxChar * 4];
	if (this->fontDataArray == nullptr)
		return;

	//leio o buffer
	if (initFont(fontFilePath) == false)
		return;

	//alguns parametros por omissão
	this->stateColor.Set(1.0f, 1.0f, 1.0f, 1.0f);

	//correu tudo bem
	this->paintStarted = false;
	this->activo = true;
}

Font::~Font()
{
	//limpo as coisas de OpengL
	if (HorseRadish::OpenGL::glIsTexture(this->glTexID))
		HorseRadish::OpenGL::glDeleteTextures(1, &this->glTexID);
	if (HorseRadish::OpenGL::glIsSampler(this->glSamplerID))
		HorseRadish::OpenGL::glDeleteSamplers(1, &this->glSamplerID);
	this->glTexID = 0;
	this->glSamplerID = 0;

	//liberto os dados do kerning e dos caracteres
	this->kerningData.Clear();
	this->charData.Clear();

	//e os buffers para guardar informação
	delete[] this->fontDataArray;
	this->fontDataArray = nullptr;
}

void Font::commitGL() const
{
	//se não tenho nada para escrever
	if (numCharWritten == 0)
		return;

	//sempre de qualquer maneira, tenho de actualizar o VBO com os dados
	HorseRadish::OpenGL::glBindBuffer(GL_ARRAY_BUFFER, this->fontManager->glArrayBufferID);
	HorseRadish::OpenGL::glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(VertexDataLayout) * numCharWritten * 4, fontDataArray);

	//exijo que sempre antes de fazer commit, o paint tenha sido activado
	assert(paintStarted);

	//basta desenhar
	HorseRadish::OpenGL::glDrawRangeElements(GL_TRIANGLE_STRIP, 0, numCharWritten * 4, numCharWritten * 5, GL_UNSIGNED_SHORT, (void*)0);

	//não é preciso desligar propriamente nada do que em cima foi activado (se é que foi, pq o paintStarted pode estar a true)

	//como acabei de mandar tudo para o GL, o número de caracteres escritos passou para zero
	numCharWritten = 0;
}

bool Font::createCharData()
{
	int numCharPairs, numNormalChars, numExtraChars, curCharWrite;
	HorseRadish::String strExtraChars;

	//se o tamanho dos caracteres a usar não for par, barraca
	numCharPairs = sizeof(validFontCharacters) / sizeof(unsigned short);
	if ((numCharPairs % 2) != 0)
		return false;

	//passo por todos as posições e verifico se estão correctas
	numCharPairs /= 2;
	for(int curPair=0; curPair<numCharPairs; curPair++)
	{
		if (validFontCharacters[curPair*2+0] > validFontCharacters[curPair*2+1])
			return false;
	}

	//passo por cada par para acumular o total de caracteres a usar
	numNormalChars = 0;
	for(int curPair=0; curPair<numCharPairs; curPair++)
		numNormalChars += validFontCharacters[curPair*2+1] - validFontCharacters[curPair*2+0] + 1;

	//preciso disto para lidar com os caracteres adicionais
	strExtraChars.Set(HorseRadish::String::Encoding::Windows, validAditionalFontCharacters);
	numExtraChars = strExtraChars.GetSizeChars();

	//crio espaço para guardar os dados todos
	if (this->charData.Reserve(numNormalChars + numExtraChars) == false)
		return false;

	//passo por cada par
	curCharWrite = 0;
	for(int curPair=0; curPair<numCharPairs; curPair++)
	{
		for(int curCharIndex = validFontCharacters[curPair*2+0]; curCharIndex<=validFontCharacters[curPair*2+1]; curCharIndex++,curCharWrite++)
			this->charData[curCharWrite].unicodeID = curCharIndex;
	}

	//verificar isto
	assert(curCharWrite == numNormalChars);

	//tenho agora de colocar os outros caracteres
	for(int curCharIndex=0; curCharIndex<numExtraChars; curCharIndex++,curCharWrite++)
		charData[curCharWrite].unicodeID = strExtraChars.GetUnicodeAt(curCharIndex);

	//verificar isto
	assert(curCharWrite == this->charData.GetNumElements());

	//correu tudo bem
	return true;
}

void Font::cleanCharData()
{
	int numTotalValidos, newIndex;
	HorseRadish::Containers::Array<CharacterData> newCharData;

	//calculo o número total de caracteres válidos
	numTotalValidos = 0;
	for(int charIndex=0; charIndex<this->charData.GetNumElements(); charIndex++)
	{
		if (this->charData[charIndex].unicodeID != 0)
			numTotalValidos++;
	}

	//se não preciso de apagar nenhum
	if (numTotalValidos == this->charData.GetNumElements())
		return;

	//calculo quantos caracteres vou ter e crio espaço para guardar os dados todos
	if (newCharData.Reserve(numTotalValidos) == false)
		return;

	//volto a passar por todos os caracteres
	newIndex = 0;
	for(int charIndex=0; charIndex<this->charData.GetNumElements(); charIndex++)
	{
		if (this->charData[charIndex].unicodeID != 0)
		{
			memcpy(newCharData + newIndex, this->charData + charIndex, sizeof(CharacterData));
			newIndex++;
		}
	}

	//troco os dados
	newCharData.SwapData(this->charData);

	//isto tem de dar igual
	assert(numTotalValidos == newIndex);
	assert(numTotalValidos == this->charData.GetNumElements());
}

bool Font::initFont(const char * const fontFilePath)
{
	float borderColor[4];
	FT_Library ftLibrary;
	FT_Face ftFace;
	FT_Int32 ftLoadFlags;
	int texWidth, texHeight, texMaxLineHeight, texLastWidth, totalSize[2];
	HorseRadish::Imaging::Image *imgFinal;
	bool primeiraEntrada;

	//inicializar o freetype
	if (FT_Init_FreeType( &ftLibrary )) 
		return false;

	//carrego a fonte desejada
	if (FT_New_Face( ftLibrary, fontFilePath, 0, &ftFace ))
	{
		FT_Done_FreeType(ftLibrary);
		return false;
	}

	//indico o tamanho da fonte a usar
	FT_Set_Char_Size( ftFace, 0, fontSize << 6, 96, 96);

	//posso guardar isto
	maxHeight = ftFace->size->metrics.height >> 6;
	baseHeight = -(ftFace->size->metrics.descender >> 6);

	//mando fazer a lista de caracteres iniciais
	if (createCharData() == false)
	{
		FT_Done_Face(ftFace);
		FT_Done_FreeType(ftLibrary);
		return false;
	}

	//as flags a usar para carregar os glyphs
	ftLoadFlags = FT_LOAD_FORCE_AUTOHINT;//FT_LOAD_DEFAULT;

	//para cada caracter a gravar
	totalSize[0] = totalSize[1] = 0;
	for(int charIndex=0; charIndex<this->charData.GetNumElements(); charIndex++)
	{
		FT_UInt freetypeCharIndex;
		FT_Error freetypeError;

		//o indice a usar no freetypex
		freetypeCharIndex = FT_Get_Char_Index( ftFace, this->charData[charIndex].unicodeID );
		if (freetypeCharIndex == 0)
		{
			this->charData[charIndex].unicodeID = 0;
			continue;
		}

		//carrego o glyph desejado
		freetypeError = FT_Load_Glyph( ftFace, freetypeCharIndex, ftLoadFlags );
		if(freetypeError != 0)
		{
			this->charData[charIndex].unicodeID = 0;
			continue;
		}

		//guardo alguns dados do glyph
		this->charData[charIndex].advance = ftFace->glyph->metrics.horiAdvance >> 6;
		this->charData[charIndex].maxUV[0] = ftFace->glyph->metrics.width >> 6;
		this->charData[charIndex].maxUV[1] = ftFace->glyph->metrics.height >> 6;
		this->charData[charIndex].offset[0] = ftFace->glyph->metrics.horiBearingX >> 6;
		this->charData[charIndex].offset[1] = -((ftFace->glyph->metrics.height - ftFace->glyph->metrics.horiBearingY) >> 6);

		//acumulo se este caracter tiver dimensão
		if ((this->charData[charIndex].maxUV[0] != 0) && (this->charData[charIndex].maxUV[1] != 0))
		{
			totalSize[0] += this->charData[charIndex].maxUV[0];
			totalSize[1] = HorseRadish::Math::iMax(totalSize[1], this->charData[charIndex].maxUV[1]);
		}
	}

	//agora tenho de calcular o tamanho da textura que vai ter todos os dados necessários
	texWidth = HorseRadish::Math::iProxPowerOfTwo(HorseRadish::Math::ftoi(HorseRadish::Math::sqrt(totalSize[0]*totalSize[1])));
	texHeight = 0;

	//limpo caracteres inválidos
	cleanCharData();

	//para cada um dos caracteres
	primeiraEntrada = true;
	for(int charIndex=0; charIndex<this->charData.GetNumElements(); charIndex++)
	{
		//ignoro estes casos
		if (this->charData[charIndex].unicodeID == 0)
			continue;

		//neste caso em particular, se este caracter não tem tamanho, não vale a pena renderizar
		if ((this->charData[charIndex].maxUV[0] == 0) || (this->charData[charIndex].maxUV[1] == 0))
			continue;

		//se estou na primeira posição, é fácil descobrir onde é que o caracter fica
		if (primeiraEntrada == true)
		{
			this->charData[charIndex].minUV[0] = 0;
			this->charData[charIndex].minUV[1] = 0;
			texLastWidth = this->charData[charIndex].minUV[0] + this->charData[charIndex].maxUV[0];
			texMaxLineHeight = this->charData[charIndex].minUV[1] + this->charData[charIndex].maxUV[1];
			primeiraEntrada = false;
			continue;
		}

		//se se me acabou o espaço desta linha nesta textura, tenho de passar para cima
		if ( (texLastWidth + 2 + charData[charIndex].maxUV[0] + 1) > texWidth)
		{
			this->charData[charIndex].minUV[0] = 0;
			this->charData[charIndex].minUV[1] = texMaxLineHeight + 2;
			texLastWidth = this->charData[charIndex].minUV[0] + this->charData[charIndex].maxUV[0];
			texMaxLineHeight = this->charData[charIndex].minUV[1] + this->charData[charIndex].maxUV[1];
			continue;
		}

		//ainda tenho espaço, posso colocar este caracter à direita do anterior
		this->charData[charIndex].minUV[0] = texLastWidth + 2;
		this->charData[charIndex].minUV[1] = this->charData[charIndex-1].minUV[1];
		texLastWidth = this->charData[charIndex].minUV[0] + this->charData[charIndex].maxUV[0];
		if ((this->charData[charIndex].minUV[1] + this->charData[charIndex].maxUV[1]) > texMaxLineHeight)
			texMaxLineHeight = this->charData[charIndex].minUV[1] + this->charData[charIndex].maxUV[1];
	}

	//tenho de ficar com esta altura na textura
	texHeight = texMaxLineHeight;

	//crio uma imagem para poder guardar os caracteres
	imgFinal = new HorseRadish::Imaging::Image(texWidth, texHeight, HorseRadish::Imaging::Image::UByte, HorseRadish::Imaging::Image::Alpha); 

	//limpo isto
	imgFinal->Clear(0.0f, 0.0f, 0.0f, 0.0f);

	//para cada caracter a gravar o bitmap
	for(int charIndex=0; charIndex<this->charData.GetNumElements(); charIndex++)
	{
		FT_Glyph ftGlyph;
		FT_Bitmap bitmapGlyph;

		//ignoro estes casos
		if (this->charData[charIndex].unicodeID == 0)
			continue;

		//neste caso em particular, se este caracter não tem tamanho, não vale a pena renderizar
		if ((this->charData[charIndex].maxUV[0] == 0) || (this->charData[charIndex].maxUV[1] == 0))
			continue;

		//carrego o glyph desejado
		if(FT_Load_Glyph( ftFace, FT_Get_Char_Index( ftFace, this->charData[charIndex].unicodeID ), ftLoadFlags ))
			continue;

		//por conveniência
		if(FT_Get_Glyph( ftFace->glyph, &ftGlyph ))
			continue;

		//converto o glyph para um bitmap
		FT_Glyph_To_Bitmap( &ftGlyph, FT_RENDER_MODE_NORMAL, nullptr, 1 );
		bitmapGlyph = ((FT_BitmapGlyph)ftGlyph)->bitmap;

		//isto tem de bater certo
		if ((this->charData[charIndex].maxUV[0] != bitmapGlyph.width) || (this->charData[charIndex].maxUV[1] != bitmapGlyph.rows))
			continue;

		//preparo uma imagem auxiliar para poder chamar a função das imagens
		HorseRadish::Imaging::Image imgAux(bitmapGlyph.width, bitmapGlyph.rows, HorseRadish::Imaging::Image::UByte, HorseRadish::Imaging::Image::Alpha, bitmapGlyph.buffer, false);
		imgAux.Flip();
		imgFinal->CopyRegion(&imgAux, charData[charIndex].minUV[0], charData[charIndex].minUV[1]);
	}

	//agora ordeno a lista dos caracteres (e assim os dados do kerning já ficam ordenados)
	this->charData.QuickSort(Font::sortCharData);

	//se a fonte tiver kerning
	if (FT_HAS_KERNING(ftFace) != 0)
	{
		//para cada caracter disponível
		for(int charIndex1=0; charIndex1<this->charData.GetNumElements(); charIndex1++)
		{
			FT_UInt glyphIndex1;

			//ignoro estes casos
			if (this->charData[charIndex1].unicodeID == 0)
				continue;

			//preciso deste indice
			glyphIndex1 = FT_Get_Char_Index( ftFace, charData[charIndex1].unicodeID );

			//passo pelo caracteres todos outra vez
			for(int charIndex2=0; charIndex2<this->charData.GetNumElements(); charIndex2++)
			{
				FT_UInt glyphIndex2;
				FT_Vector kernVector;
				KerningData *newKerningData;

				//ignoro estes casos
				if (this->charData[charIndex2].unicodeID == 0)
					continue;

				//ignoro ele próprio
				if (charIndex1 == charIndex2)
					continue;

				//preciso deste indice e se forem idênticos, ignoro-os
				glyphIndex2 = FT_Get_Char_Index( ftFace, charData[charIndex2].unicodeID );
				if (glyphIndex1 == glyphIndex2)
					continue;

				//tiro o kerning para estes dois caracteres
				if (FT_Get_Kerning(ftFace, glyphIndex1, glyphIndex2, FT_KERNING_DEFAULT, &kernVector) != 0)
					continue;

				//se não tenho nada
				if (kernVector.x == 0)
					continue;

				//guardo os dados do kerning
				newKerningData = this->kerningData.Add();
				newKerningData->char1 = this->charData[charIndex1].unicodeID;
				newKerningData->char2 = this->charData[charIndex2].unicodeID;
				newKerningData->offset = kernVector.x >> 6;
			}
		}

		//adiciono um último, para ficar a NULL
		this->kerningData.Add();
	}
	
	//tenho de limpar isto
	FT_Done_Face(ftFace);
	FT_Done_FreeType(ftLibrary);

	//agora passo por cada caracter para verificar se tem algum kerning
	for(int i=0; i<this->charData.GetNumElements(); i++)
	{
		//ignoro estes casos
		if (this->charData[i].unicodeID == 0)
			continue;

		//passo agora por cada kerning e procuro pelo primeiro que encontrar
		for(int j=0; j<this->kerningData.GetNumElements(); j++)
		{
			//ignoro estes casos
			if (this->charData[j].unicodeID == 0)
				continue;

			//se for diferente, segue...
			if (this->kerningData[j].char1 != this->charData[i].unicodeID)
				continue;

			//achei um valor de kerning
			this->charData[i].kernData = this->kerningData + j;
			break;
		}
	}

	//tudo a preto
	borderColor[0] = borderColor[1] = borderColor[2] = borderColor[3] = 0.0f;

	//preciso fazer umas mudanças na imagem
	imgFinal->ChangeFormat(HorseRadish::Imaging::Image::LumAlpha);
	imgFinal->ChannelCopy(HorseRadish::Imaging::Image::Alpha, HorseRadish::Imaging::Image::Red);
	imgFinal->ChangeFormat(HorseRadish::Imaging::Image::RGBA);

	//crio a textura
	HorseRadish::OpenGL::glGenTextures(1, &this->glTexID);
	HorseRadish::OpenGL::Extensions::glTextureStorage2DEXT(this->glTexID, GL_TEXTURE_RECTANGLE, 1, GL_RGBA8, imgFinal->GetWidth(), imgFinal->GetHeight());
	HorseRadish::OpenGL::Extensions::glTextureSubImage2DEXT(this->glTexID, GL_TEXTURE_RECTANGLE, 0, 0, 0, imgFinal->GetWidth(), imgFinal->GetHeight(), GL_RGBA, GL_UNSIGNED_BYTE, imgFinal->GetPixelData());

	//crio o sampler a usar na textura
	HorseRadish::OpenGL::glGenSamplers(1, &this->glSamplerID);
	HorseRadish::OpenGL::glSamplerParameteri(this->glSamplerID, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	HorseRadish::OpenGL::glSamplerParameteri(this->glSamplerID, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	HorseRadish::OpenGL::glSamplerParameteri(this->glSamplerID, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
	HorseRadish::OpenGL::glSamplerParameteri(this->glSamplerID, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
	HorseRadish::OpenGL::glSamplerParameteri(this->glSamplerID, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_BORDER);
	HorseRadish::OpenGL::glSamplerParameterfv(this->glSamplerID, GL_TEXTURE_BORDER_COLOR, borderColor);

	//posso destruir a imagem
	delete imgFinal;
	imgFinal = nullptr;

	//e pronto, correu tudo bem
	return true;
}

void Font::internalWrite(const float &px, const float &py, HorseRadish::String::Iterator &strIt) const
{
	float posX, posY;
	VertexDataLayout *writeData;
	unsigned int curCharUnicode;
	const CharacterData *curCharData;

	//verifico algumas coisas
	if (activo == false)
		return;

	//posso pre-calcular isto
	posX=px;
	posY=py + baseHeight;

	//enquanto não chegar ao fim da string
	while(strIt.IsLast() == false)
	{
		//para onde vou escrever
		writeData = fontDataArray + (numCharWritten * 4);

		//até chegar ao fim do texto ou do limite
		while(numCharWritten < Font::numMaxChar)
		{
			unsigned char colorTemp[4];

			//o caracter a usar
			curCharUnicode = strIt.Read();
			if (curCharUnicode == 0)
				break;

			//os dados do caracter a usar
			curCharData = getCharData(curCharUnicode);
			if (curCharData == nullptr)
				continue;

			//se não tenho altura, posso passar à frente
			if (curCharData->maxUV[1]==0)
			{
				posX += (float)curCharData->advance;
				continue;
			}

			//crio os pontos
			writeData[0].px = writeData[3].px = posX + float(curCharData->offset[0]);
			writeData[1].px = writeData[2].px = writeData[0].px + float(curCharData->maxUV[0]);
			writeData[0].py = writeData[1].py = posY + ((float)curCharData->offset[1]);
			writeData[2].py = writeData[3].py = writeData[0].py + ((float)curCharData->maxUV[1]);

			//crio as coordenadas
			writeData[0].tu = writeData[3].tu = (float)(curCharData->minUV[0]);
			writeData[1].tu = writeData[2].tu = (float)(curCharData->minUV[0] + curCharData->maxUV[0]);
			writeData[0].tv = writeData[1].tv = (float)(curCharData->minUV[1]);
			writeData[2].tv = writeData[3].tv = (float)(curCharData->minUV[1] + curCharData->maxUV[1]);			

			//agora a cor
			stateColor.Write(colorTemp);
			memcpy(writeData[0].rgba, colorTemp, sizeof(unsigned char)*4);
			memcpy(writeData[1].rgba, colorTemp, sizeof(unsigned char)*4);
			memcpy(writeData[2].rgba, colorTemp, sizeof(unsigned char)*4);
			memcpy(writeData[3].rgba, colorTemp, sizeof(unsigned char)*4);

			//avanço com os ponteiros
			writeData += 4;
			numCharWritten++;
			posX += (float)curCharData->advance;

			//se tiver kerning, adiciono-o
			if (this->kerningData.IsEmpty() == false)
				posX += (float)getCharKerning(curCharData, *strIt);
		}

		//se passei o numero máximo, tenho de desenhar
		if (numCharWritten >= Font::numMaxChar)
			commitGL();
	}
}

const Font::CharacterData* Font::getCharData(int unicodeID) const
{
	CharacterData testeComparacao;
	int charIndex;

	//é este que tenho de procurar
	testeComparacao.unicodeID = unicodeID;

	//faço a pesquisa binária
	charIndex = this->charData.BinarySearch<CharacterData>(Font::findCharData, testeComparacao);
	return ((charIndex < 0) ? nullptr : (this->charData + charIndex));
}

const int Font::getCharKerning(const CharacterData * const leftCharData, unsigned short rightCharUnicodeID) const
{
	//se nada está como deve de ser
	if ((leftCharData == nullptr) || (leftCharData->kernData == nullptr) || (rightCharUnicodeID <= 0))
		return 0;

	//passo por todos os kernings
	for(const KerningData *curKerning = leftCharData->kernData; true; curKerning++)
	{
		//se já não vale a pena continuar
		if (curKerning->char1 != leftCharData->unicodeID)
			break;

		//se é este o kerning, já tá
		if (curKerning->char2 == rightCharUnicodeID)
			return curKerning->offset;
	}

	//chegando aqui, paciência
	return 0;
}

void Font::write(const float &px, const float &py, HorseRadish::String::Iterator &iterator) const
{	internalWrite(px, py, iterator); }

void Font::write(HorseRadish::String::Iterator &iterator) const
{	write(0.0f, 0.0f, iterator); }

void Font::write(const float &px, const float &py, const HorseRadish::String &texto) const
{	internalWrite(px, py, HorseRadish::String::Iterator(texto)); }

void Font::write(const HorseRadish::String &texto) const
{	write(0.0f, 0.0f, texto); }

int Font::writeChar(const unsigned int &unicodeChar) const
{	return writeChar(0.0f, 0.0f, unicodeChar); }

int Font::writeChar(const float &px, const float &py, const unsigned int &unicodeChar) const
{
	unsigned char colorTemp[4];
	VertexDataLayout *writeData;
	const CharacterData *charData;

	//verificar parametros
	if (this->activo == false)
		return 0;

	//tiro o caracter que vou usar
	charData = getCharData(unicodeChar);
	if (charData == nullptr)
		return 0;

	//se não tenho nada a desenhar
	if (charData->maxUV[1] == 0)
		return charData->advance;

	//se tiver de desenhar antes de continuar
	if (numCharWritten >= Font::numMaxChar)
		commitGL();

	//para onde devo começar a escrever
	writeData = fontDataArray + (numCharWritten * 4);

	//crio os pontos
	writeData[0].px = writeData[3].px = px + float(charData->offset[0]);
	writeData[1].px = writeData[2].px = writeData[0].px + float(charData->maxUV[0]);
	writeData[0].py = writeData[1].py = py + ((float)baseHeight) + ((float)charData->offset[1]);
	writeData[2].py = writeData[3].py = writeData[0].py + ((float)charData->maxUV[1]);

	//crio as coordenadas
	writeData[0].tu = writeData[3].tu = (float)(charData->minUV[0]);
	writeData[1].tu = writeData[2].tu = (float)(charData->minUV[0] + charData->maxUV[0]);
	writeData[0].tv = writeData[1].tv = (float)(charData->minUV[1]);
	writeData[2].tv = writeData[3].tv = (float)(charData->minUV[1] + charData->maxUV[1]);

	//agora a cor
	stateColor.Write(colorTemp);
	memcpy(writeData[0].rgba, colorTemp, sizeof(unsigned char)*4);
	memcpy(writeData[1].rgba, colorTemp, sizeof(unsigned char)*4);
	memcpy(writeData[2].rgba, colorTemp, sizeof(unsigned char)*4);
	memcpy(writeData[3].rgba, colorTemp, sizeof(unsigned char)*4);

	// mais um caracter escrito e devolvo quanto devo avançar
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
	const CharacterData *curCharData;

	//tiro os dados do caracter
	curCharData = this->getCharData(unicodeChar);
	if (curCharData == nullptr)
		return 0;
	return curCharData->advance;
}

int Font::getStringWidth(const HorseRadish::String &texto) const
{
	int totalWidth;
	const CharacterData *curCharData;

	//verifico algumas coisas
	if ( (!activo) || (texto.GetSizeChars()==0) )
		return 0;

	//posso começar a dar as voltas
	totalWidth = 0;
	for(HorseRadish::String::Iterator it(texto); it.IsLast() == false; it++)
	{
		//o caracter a usar
		curCharData = getCharData(*it);
		if (curCharData == nullptr)
			continue;

		//basta acumular este valor
		totalWidth += curCharData->advance;
	}

	//já tá
	return totalWidth;
}

int Font::getStringWidth(const HorseRadish::String &texto, const unsigned int numCharMeta) const
{
	int totalWidth;
	const CharacterData *curCharData;

	//verifico algumas coisas
	if ( (!activo) || (texto.GetSizeChars()==0) || (numCharMeta == 0) )
		return 0;

	//posso começar a dar as voltas
	totalWidth = 0;
	for(HorseRadish::String::Iterator it(texto); it.IsLast() == false; it++)
	{
		//se for para sair
		if (it.GetCaracterPosition() >= numCharMeta)
			break;

		//o caracter a usar
		curCharData = getCharData(*it);
		if (curCharData == nullptr)
			continue;

		//basta acumular este valor
		totalWidth += curCharData->advance;
	}

	//já tá
	return totalWidth;
}

void Font::paintBegin(const float * const tranformationMatrix)
{
	//se já começei nao vale a pena
	if (paintStarted)
		return;

	//coloco a textura, sampler e o VAO (que inclui VBO e ponteiros, etc.)
	HorseRadish::OpenGL::Extensions::glBindMultiTextureEXT(GL_TEXTURE0, GL_TEXTURE_RECTANGLE, this->glTexID);
	HorseRadish::OpenGL::glBindSampler(0, this->glSamplerID);
	HorseRadish::OpenGL::glBindVertexArray(this->fontManager->glVertexArrayID);

	//como vou usar primitive restart
	HorseRadish::OpenGL::glEnable(GL_PRIMITIVE_RESTART);
	HorseRadish::OpenGL::glPrimitiveRestartIndex(Font::primitiveRestartIndex);
	
	//coloco o programa e a localização da unidade de textura
	HorseRadish::OpenGL::glUseProgram(glProgID);
	HorseRadish::OpenGL::glUniform1i(glUniformSampler, 0);
	if (tranformationMatrix != nullptr)
		HorseRadish::OpenGL::glUniformMatrix4fv(glUniformMatrix, 1, GL_FALSE, tranformationMatrix);
	
	//marco-me como pronto a desenhar e já está
	this->paintStarted = true;
}

void Font::paintEnd()
{
	//a única coisa que devo desligar é o primite restart
	HorseRadish::OpenGL::glDisable(GL_PRIMITIVE_RESTART);

	//coloco-me a falso
	this->paintStarted = false;
}

}//namespace Tools
}//namespace OpenGL
}//namespace HorseRadish