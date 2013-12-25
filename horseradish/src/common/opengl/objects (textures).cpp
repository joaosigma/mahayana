#include "objects.hpp"
#include "openGLext.hpp"
#include "common\Platform.hpp"
#include "common\Timer.hpp"
#include "common\Path.hpp"
#include "common\ImageFactory.hpp"

namespace HorseRadish
{

namespace OpenGL
{

namespace Objects
{

/*§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§
§§§§§§   -= Declarações e estructuras =-   §§§§§
§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§*/
#define FILE_SIZE	100

#define EXT_ANISO	(1<<0)
#define EXT_S3		(1<<1)
#define EXT_VTC		(1<<2)

#define DDS_FOURCC	0x00000004
#define DDS_RGB		0x00000040
#define DDS_RGBA	0x00000041
#define DDS_DEPTH	0x00800000

#define DDS_COMPLEX		0x00000008
#define DDS_CUBEMAP		0x00000200
#define DDS_VOLUME		0x00200000

#define FOURCC_DXT1		0x31545844
#define FOURCC_DXT3		0x33545844
#define FOURCC_DXT5		0x35545844

#define DDS_FORMAT_RGB		0xffe1
#define DDS_FORMAT_RGBA		0xffe2
#define DDS_FORMAT_DXT1		0xffe3
#define DDS_FORMAT_DXT3		0xffe4
#define DDS_FORMAT_DXT5		0xffe5
#define DDS_FORMAT_3DC		0xffe6

#pragma pack (push)
#pragma pack (1)

//cenas para conseguir ler DDS
struct DDS_PIXELFORMAT
{
    unsigned long dwSize;
    unsigned long dwFlags;
    unsigned long dwFourCC;
    unsigned long dwRGBBitCount;
    unsigned long dwRBitMask;
    unsigned long dwGBitMask;
    unsigned long dwBBitMask;
    unsigned long dwABitMask;
};

struct DDS_HEADER
{
    unsigned long dwSize;
    unsigned long dwFlags;
    unsigned long dwHeight;
    unsigned long dwWidth;
    unsigned long dwPitchOrLinearSize;
    unsigned long dwDepth;
    unsigned long dwMipMapCount;
    unsigned long dwReserved1[11];
    DDS_PIXELFORMAT ddspf;
    unsigned long dwCaps1;
    unsigned long dwCaps2;
    unsigned long dwReserved2[3];
};

#pragma pack (pop)

struct ImgData{
	int width, height;
	const void *data;
	HorseRadish::Imaging::Image::ImageFormat format;
	HorseRadish::Imaging::Image::ImageType type;
};

/*§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§
§§§§§§   -= Funções locais =-   §§§§§
§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§*/
static
unsigned int toGLInternalFormat(const ObjectsManager::TargetType targetType)
{
	switch(targetType)
	{
		//case ObjectsManager::Alpha8:		return GL_ALPHA8;
		case ObjectsManager::RGBA16:		return GL_RGBA4;
		case ObjectsManager::RGBA32:		return GL_RGBA8;
		case ObjectsManager::Depth16:		return GL_DEPTH_COMPONENT16;
		case ObjectsManager::Depth24:		return GL_DEPTH_COMPONENT24;
		case ObjectsManager::DepthStencil:	return GL_DEPTH24_STENCIL8;
		case ObjectsManager::RGBA64:		return GL_RGBA16F;
		case ObjectsManager::RGBA128:		return GL_RGBA32F;
		case ObjectsManager::Red32:			return 0x888B;
	}

	//SConsole::SConsolePLog(logID,HorseRadish::String("Unknow texture internal format 0x%x (could not convert to GL).",targetType));
	return 0;
}

static
bool checkDDSandSuport(const DDS_HEADER * const header, int &format, int &components)
{
	//o formato e os componentes
	format=components=0;

	//figure out what the image format is
    if (header->ddspf.dwFlags & DDS_FOURCC) 
		{
		//se for algum que eu conheço
        switch(header->ddspf.dwFourCC){
            case FOURCC_DXT1:
                format = DDS_FORMAT_DXT1;
                components = 3;
                return true;
            case FOURCC_DXT3:
                format = DDS_FORMAT_DXT3;
                components = 4;
                return true;
            case FOURCC_DXT5:
                format = DDS_FORMAT_DXT5;
                components = 4;
                return true;
			}

		//desconhecido
		return false;
		}

	//ambos são idênticos RGBA 32bit
    if (header->ddspf.dwFlags == DDS_RGBA && header->ddspf.dwRGBBitCount == 32)
		{
        format = DDS_FORMAT_RGBA;
        components = 4;
		return true;
		}
    if (header->ddspf.dwFlags == DDS_RGB  && header->ddspf.dwRGBBitCount == 32)
		{
        format = DDS_FORMAT_RGBA; 
        components = 4;
		return true;
		}

	//imagem RGB 24bit
    else if (header->ddspf.dwFlags == DDS_RGB  && header->ddspf.dwRGBBitCount == 24)
		{
        format = DDS_FORMAT_RGB; 
        components = 3;
		return true;
		}
    
	//desconheco
	return false;
}

static
int size_dxtc(const int width, const int height, const int format)
{    return ((width+3)/4)*((height+3)/4)*(format == DDS_FORMAT_DXT1 ? 8 : 16);}

static
int size_rgb(const int width, const int height, const int components)
{    return width*height*components;}

static
unsigned int toGLFormat(const HorseRadish::Imaging::Image::ImageFormat &imageFormat)
{
	switch(imageFormat)
	{
		case HorseRadish::Imaging::Image::RGB:		return GL_RGB;
		case HorseRadish::Imaging::Image::RGBA:		return GL_RGBA;
		case HorseRadish::Imaging::Image::BGR:		return GL_BGR;
		case HorseRadish::Imaging::Image::BGRA:		return GL_BGRA;
		case HorseRadish::Imaging::Image::Red:		return GL_RED;
		case HorseRadish::Imaging::Image::Green:	return GL_GREEN;
		case HorseRadish::Imaging::Image::Blue:		return GL_BLUE;
		case HorseRadish::Imaging::Image::Alpha:	return GL_ALPHA;
		/*case SIMAGE_LUM:		return GL_LUMINANCE;
		case SIMAGE_LUMALPHA:	return GL_LUMINANCE_ALPHA;*/
	}

	//SConsole::SConsolePLog(logID, HorseRadish::String("Unknow format 0x%x (could not convert to GL).", imgFormat));
	return 0;
}

static
unsigned int toGLType(const HorseRadish::Imaging::Image::ImageType &imageType)
{
	switch(imageType)
	{
		case HorseRadish::Imaging::Image::Float:		return GL_FLOAT;
		case HorseRadish::Imaging::Image::UByte:		return GL_UNSIGNED_BYTE;
	}

	//SConsole::SConsolePLog(logID,HorseRadish::String("Unknow type 0x%x (could not convert to GL).", imgType));
	return 0;
}

static
void prepareImgData(ImgData * const imgData, const HorseRadish::Imaging::Image * const image)
{
	//basta passar os dados de um lado para o outro
	imgData->data = image->GetPixelData();
	image->GetDims(imgData->width, imgData->height);

	imgData->format = image->GetFormat();
	imgData->type = image->GetType();
}

static
bool uploadDDS(const void *fileData, const unsigned int &fileSize, Objects::Texture* const textura)
{
	unsigned char *fwalker;
	DDS_HEADER ddsh;
	int dataSize,target,format,components,curImage,curMipMap,curWidth,curHeight,curDepth;

	//verificar alguns parametros
	if (fileData==nullptr || fileSize==0 || textura==nullptr)
		return false;

	 //a assinatura
	fwalker=(unsigned char*)fileData;
    if (fwalker[0]!='D' || fwalker[1]!='D' || fwalker[2]!='S' || fwalker[3]!=' ')
		return false;

	//o header
	fwalker+=4;
	memcpy(&ddsh,fwalker,sizeof(ddsh));
	fwalker+=sizeof(ddsh);

	//descobro qual os componentes da textura e se tenho suporte no GL pra ela
	if (checkDDSandSuport(&ddsh,format,components)==false)
		return false;

	//crio a textura propriamente dita
	if (ddsh.dwCaps2 & DDS_CUBEMAP)
		target=GL_TEXTURE_CUBE_MAP;
	else if (ddsh.dwCaps2 & DDS_VOLUME)
		target=GL_TEXTURE_3D;
	else if (ddsh.dwHeight==1)
		target=GL_TEXTURE_1D;
	else
		target=GL_TEXTURE_2D;

	//crio a textura e arranjo o filtro por defeito
	HorseRadish::OpenGL::glBindTexture(target,textura->glID);

	//as dimensões também são uteis
	curWidth=ddsh.dwWidth;
	curHeight=ddsh.dwHeight;
	curDepth=ddsh.dwDepth;

	//não esquecer isto
	textura->width=curWidth;
	textura->height=curHeight;
	textura->glTarget=target;

	//agora para uma textura 2D
	if (target==GL_TEXTURE_2D)
		{
		//se não tem mipmap, tem pelo menos 1
		if (ddsh.dwMipMapCount==0)
			ddsh.dwMipMapCount=1;

		//agora para cada mipmap
		for(curMipMap=0; curMipMap<ddsh.dwMipMapCount; curMipMap++)
			{
			//tenho de ter em atenção qual o tipo de dados
			switch(format){
				case DDS_FORMAT_DXT1:
						dataSize=size_dxtc(curWidth,curHeight,format);
						HorseRadish::OpenGL::glCompressedTexImage2D(GL_TEXTURE_2D,curMipMap,GL_COMPRESSED_RGB_S3TC_DXT1_EXT,curWidth,curHeight,0,dataSize,fwalker);
						fwalker+=dataSize;
						break;
				case DDS_FORMAT_DXT3:
						dataSize=size_dxtc(curWidth,curHeight,format);
						HorseRadish::OpenGL::glCompressedTexImage2D(GL_TEXTURE_2D,curMipMap,GL_COMPRESSED_RGBA_S3TC_DXT3_EXT,curWidth,curHeight,0,dataSize,fwalker);
						fwalker+=dataSize;
						break;
				case DDS_FORMAT_DXT5:
						dataSize=size_dxtc(curWidth,curHeight,format);
						HorseRadish::OpenGL::glCompressedTexImage2D(GL_TEXTURE_2D,curMipMap,GL_COMPRESSED_RGBA_S3TC_DXT5_EXT,curWidth,curHeight,0,dataSize,fwalker);
						fwalker+=dataSize;
						break;
				case DDS_FORMAT_RGB:
						dataSize=size_rgb(curWidth,curHeight,components);
						HorseRadish::OpenGL::glTexImage2D(GL_TEXTURE_2D, curMipMap, GL_RGB8, curWidth, curHeight, 0, GL_BGR, GL_UNSIGNED_BYTE, fwalker);
						fwalker+=dataSize;
						break;
				case DDS_FORMAT_RGBA:
						dataSize=size_rgb(curWidth,curHeight,components);
						HorseRadish::OpenGL::glTexImage2D(GL_TEXTURE_2D, curMipMap, GL_RGBA8, curWidth, curHeight, 0, GL_BGRA, GL_UNSIGNED_BYTE, fwalker);
						fwalker+=dataSize;
						break;
				}

			//o tamanho pro proximo mipmap
			curWidth = HorseRadish::Math::iMax(curWidth >> 1, 1);
			curHeight = HorseRadish::Math::iMax(curHeight >> 1, 1);
			}

		//no fim de carregar a textura 2D, só tenho de fechar o ficheiro, mandar a textura e prontos
		return true;
		}

	//agora para uma textura 1D
	if (target==GL_TEXTURE_1D)
		{
		//se não tem mipmap, tem pelo menos 1
		if (ddsh.dwMipMapCount==0)
			ddsh.dwMipMapCount=1;

		//agora para cada mipmap
		for(curMipMap=0; curMipMap<ddsh.dwMipMapCount; curMipMap++)
			{
			//tenho de ter em atenção qual o tipo de dados
			switch(format){
				case DDS_FORMAT_DXT1:
						dataSize=size_dxtc(curWidth,curHeight,format);
						HorseRadish::OpenGL::glCompressedTexImage1D(GL_TEXTURE_1D,curMipMap,GL_COMPRESSED_RGB_S3TC_DXT1_EXT,curWidth,0,dataSize,fwalker);
						fwalker+=dataSize;
						break;
				case DDS_FORMAT_DXT3:
						dataSize=size_dxtc(curWidth,curHeight,format);
						HorseRadish::OpenGL::glCompressedTexImage1D(GL_TEXTURE_1D,curMipMap,GL_COMPRESSED_RGBA_S3TC_DXT3_EXT,curWidth,0,dataSize,fwalker);
						fwalker+=dataSize;
						break;
				case DDS_FORMAT_DXT5:
						dataSize=size_dxtc(curWidth,curHeight,format);
						HorseRadish::OpenGL::glCompressedTexImage1D(GL_TEXTURE_1D,curMipMap,GL_COMPRESSED_RGBA_S3TC_DXT5_EXT,curWidth,0,dataSize,fwalker);
						fwalker+=dataSize;
						break;
				case DDS_FORMAT_RGB:
						dataSize=size_rgb(curWidth,curHeight,components);
						HorseRadish::OpenGL::glTexImage1D(GL_TEXTURE_1D, curMipMap, GL_RGBA8, curWidth, 0, GL_RGB, GL_UNSIGNED_BYTE, fwalker);
						fwalker+=dataSize;
						break;
				case DDS_FORMAT_RGBA:
						dataSize=size_rgb(curWidth,curHeight,components);
						HorseRadish::OpenGL::glTexImage1D(GL_TEXTURE_1D, curMipMap, GL_RGBA8, curWidth, 0, GL_RGBA, GL_UNSIGNED_BYTE, fwalker);
						fwalker+=dataSize;
						break;
				}

			//o tamanho pro proximo mipmap
			curWidth = HorseRadish::Math::iMax(curWidth >> 1, 1);
			}

		//não é preciso fazer mais nada
		return true;
		}

	//agora para uma textura CUBEMAP
	if (target==GL_TEXTURE_CUBE_MAP)
		{
		//para cada uma das faces do cubemap
		for(curImage=0; curImage<6; curImage++)
			{
			//escolho a face
			switch(curImage){
				case 0: target=GL_TEXTURE_CUBE_MAP_POSITIVE_X;	break;
				case 1: target=GL_TEXTURE_CUBE_MAP_POSITIVE_Y;	break;
				case 2: target=GL_TEXTURE_CUBE_MAP_POSITIVE_Z;	break;
				case 3: target=GL_TEXTURE_CUBE_MAP_NEGATIVE_X;	break;
				case 4: target=GL_TEXTURE_CUBE_MAP_NEGATIVE_Y;	break;
				case 5: target=GL_TEXTURE_CUBE_MAP_NEGATIVE_Z;	break;
				}

			//tenho sempre de reiniciar isto
			curWidth=ddsh.dwWidth;
			curHeight=ddsh.dwHeight;

			//se não tem mipmap, tem pelo menos 1
			if (ddsh.dwMipMapCount==0)
				ddsh.dwMipMapCount=1;

			//agora para cada mipmap
			for(curMipMap=0; curMipMap<ddsh.dwMipMapCount; curMipMap++)
				{
				//tenho de ter em atenção qual o tipo de dados
				switch(format){
					case DDS_FORMAT_DXT1:
							dataSize=size_dxtc(curWidth,curHeight,format);
							HorseRadish::OpenGL::glCompressedTexImage2D(target,curMipMap,GL_COMPRESSED_RGB_S3TC_DXT1_EXT,curWidth,curHeight,0,dataSize,fwalker);
							fwalker+=dataSize;
							break;
					case DDS_FORMAT_DXT3:
							dataSize=size_dxtc(curWidth,curHeight,format);
							HorseRadish::OpenGL::glCompressedTexImage2D(target,curMipMap,GL_COMPRESSED_RGBA_S3TC_DXT3_EXT,curWidth,curHeight,0,dataSize,fwalker);
							fwalker+=dataSize;
							break;
					case DDS_FORMAT_DXT5:
							dataSize=size_dxtc(curWidth,curHeight,format);
							HorseRadish::OpenGL::glCompressedTexImage2D(target,curMipMap,GL_COMPRESSED_RGBA_S3TC_DXT5_EXT,curWidth,curHeight,0,dataSize,fwalker);
							fwalker+=dataSize;
							break;
					case DDS_FORMAT_RGB:
							dataSize=size_rgb(curWidth,curHeight,components);
							HorseRadish::OpenGL::glTexImage2D(target, curMipMap, GL_RGBA8, curWidth, curHeight, 0, GL_RGB, GL_UNSIGNED_BYTE, fwalker);
							fwalker+=dataSize;
							break;
					case DDS_FORMAT_RGBA:
							dataSize=size_rgb(curWidth,curHeight,components);
							HorseRadish::OpenGL::glTexImage2D(target, curMipMap, GL_RGBA8, curWidth, curHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, fwalker);
							fwalker+=dataSize;
							break;
					}

				//o tamanho pro proximo mipmap
				curWidth = HorseRadish::Math::iMax(curWidth >> 1, 1);
				curHeight = HorseRadish::Math::iMax(curHeight >> 1, 1);
				}
			}

		//só basta sair
		return true;
		}
	
	//chegando aqui alguma coisa se passou de muito errado
	return false;
}

static
unsigned char* compressTex(const ImgData * const img, const Objects::Texture* const tex, const int internalFormat, const int currentMipMap, int * const bufferSize, int * const bufferFormat, const bool extS3, const bool extVTC)
{
	int isTC,compressFormat;
	unsigned char *buffer;

	//pois, né?!
	if (img==nullptr || tex==nullptr || bufferSize==nullptr || bufferFormat==nullptr || currentMipMap<0)
		return nullptr;
 
	//não se pode comprimir texturas rect ou cujo formato seja RGBA >= 64
	if (internalFormat==GL_RGBA32F || internalFormat==GL_RGBA16F)
		{
		//SConsole::SConsolePLog(logID,"Cannot compress textures of float internal format.");
		return nullptr;
		}

	//verifico qual o formato mais fixe
	switch(internalFormat){
		//case GL_LUMINANCE8:			compressFormat=GL_COMPRESSED_LUMINANCE;			break;
		//case GL_ALPHA8:				compressFormat=GL_COMPRESSED_ALPHA;				break;
		//case GL_LUMINANCE8_ALPHA8:	compressFormat=GL_COMPRESSED_LUMINANCE_ALPHA;	break;
		default:					compressFormat=GL_COMPRESSED_RGBA;				break;
		}

	//se tiver a compressao GL_COMPRESSED_RGBA então é RGBA normal, logo o S3TC é melhor pra isto, MAS só pra 2D
	if ( (compressFormat==GL_COMPRESSED_RGBA) && (extS3) && (tex->glTarget==GL_TEXTURE_2D) )
		compressFormat=GL_COMPRESSED_RGBA_S3TC_DXT5_EXT;

	//se tiver o mesmo mas pra texturas 3D, uso extVTC
	if ( (compressFormat==GL_COMPRESSED_RGBA) && (extVTC) && (tex->glTarget==GL_TEXTURE_3D) )
		compressFormat=GL_COMPRESSED_RGBA_S3TC_DXT5_EXT;
		
	//mando então comprimir
	switch(tex->glTarget){
		case GL_TEXTURE_1D:		HorseRadish::OpenGL::glTexImage1D(tex->glTarget, currentMipMap, compressFormat, img->width, 0, toGLFormat(img->format), toGLType(img->type), img->data);
								break;
		case GL_TEXTURE_2D:		HorseRadish::OpenGL::glTexImage2D(tex->glTarget, currentMipMap, compressFormat, img->width, img->height, 0, toGLFormat(img->format), toGLType(img->type), img->data);
								break;
		case GL_TEXTURE_3D:		HorseRadish::OpenGL::glTexImage3D(tex->glTarget, currentMipMap, compressFormat, img->width, img->height, tex->depth, 0, toGLFormat(img->format), toGLType(img->type), img->data);
								break;
		}

	//vou ver se está tudo como deve de ser
	HorseRadish::OpenGL::glGetTexLevelParameteriv(tex->glTarget, currentMipMap, GL_TEXTURE_COMPRESSED, &isTC);

	//comprimiu
	if (isTC!=0)
		{
		//vou buscar os parametros
		HorseRadish::OpenGL::glGetTexLevelParameteriv(tex->glTarget, currentMipMap, GL_TEXTURE_INTERNAL_FORMAT, bufferFormat);
		HorseRadish::OpenGL::glGetTexLevelParameteriv(tex->glTarget, currentMipMap, GL_TEXTURE_COMPRESSED_IMAGE_SIZE, bufferSize);

		//crio o novo buffer
		buffer = new unsigned char[*bufferSize];
		
		//vou buscar a tex comprimida e envio
		HorseRadish::OpenGL::glGetCompressedTexImage(tex->glTarget, currentMipMap, buffer);
		return buffer;
		}
	
	//nao comprimiu, não faço nada
	//SConsole::SConsolePLog(logID,"Unable to compress texture.");
	return nullptr;
}

static
unsigned char* compressTexCube(const ImgData * const img, const int curFaceTarget, const int internalFormat, const int currentMipMap, int * const bufferSize, int * const bufferFormat, const bool extS3)
{
	int isTC,compressFormat;
	unsigned char *buffer;

	//pois, né?!
	if (img==nullptr || bufferSize==nullptr || bufferFormat==nullptr || currentMipMap<0)
		return nullptr;
 
	//não se pode comprimir texturas rect ou cujo formato seja RGBA >= 64
	if (internalFormat==GL_RGBA32F || internalFormat==GL_RGBA16F)
		{
		//SConsole::SConsolePLog(logID,"Cannot compress textures of float internal format.");
		return nullptr;
		}

	//verifico qual o formato mais fixe
	switch(internalFormat){
		//case GL_LUMINANCE8:			compressFormat=GL_COMPRESSED_LUMINANCE;			break;
		//case GL_ALPHA8:				compressFormat=GL_COMPRESSED_ALPHA;				break;
		//case GL_LUMINANCE8_ALPHA8:	compressFormat=GL_COMPRESSED_LUMINANCE_ALPHA;	break;
		default:					compressFormat=GL_COMPRESSED_RGBA;				break;
		}

	//se tiver a compressao GL_COMPRESSED_RGBA então é RGBA normal, logo o S3TC é melhor pra isto
	if ( (compressFormat==GL_COMPRESSED_RGBA) && (extS3) )
		compressFormat=GL_COMPRESSED_RGBA_S3TC_DXT5_EXT;
		
	//mando então comprimir
	HorseRadish::OpenGL::glTexImage2D(curFaceTarget, currentMipMap, compressFormat, img->width, img->height, 0, toGLFormat(img->format), toGLType(img->type), img->data);
	
	//vou ver se está tudo como deve de ser
	HorseRadish::OpenGL::glGetTexLevelParameteriv(curFaceTarget, currentMipMap, GL_TEXTURE_COMPRESSED, &isTC);

	//comprimiu
	if (isTC!=0)
		{
		//vou buscar os parametros
		HorseRadish::OpenGL::glGetTexLevelParameteriv(curFaceTarget, currentMipMap, GL_TEXTURE_INTERNAL_FORMAT, bufferFormat);
		HorseRadish::OpenGL::glGetTexLevelParameteriv(curFaceTarget, currentMipMap, GL_TEXTURE_COMPRESSED_IMAGE_SIZE, bufferSize);

		//crio o novo buffer
		buffer = new unsigned char[*bufferSize];
		
		//vou buscar a tex comprimida e envio
		HorseRadish::OpenGL::glGetCompressedTexImage(curFaceTarget, currentMipMap, buffer);
		return buffer;
		}
	
	//nao comprimiu, não faço nada
	//SConsole::SConsolePLog(logID,"Unable to compress texture.");
	return nullptr;
}

static
void fazSemDados(const ImgData * const imagem, const Objects::Texture* const texture, const ObjectsManager::TargetType targetType, const int flags)
{
	int internalFormat,iGLFormat,iGLType;
	int mipComp,mipLarg,mipDepth,mipmap;

	//escolher o formato interno
	internalFormat=toGLInternalFormat(targetType);
	if (internalFormat==0)
		return;

	//quais os formatos das texturas
	iGLFormat=GL_RGBA;
	iGLType=GL_UNSIGNED_BYTE;
	if (targetType==ObjectsManager::Depth16 || targetType==ObjectsManager::Depth24)
	{
		iGLFormat = GL_DEPTH_COMPONENT;
		iGLType = GL_UNSIGNED_INT;
	}
	else if (targetType==ObjectsManager::DepthStencil)
	{
		iGLFormat = GL_DEPTH_STENCIL;
		iGLType = GL_UNSIGNED_INT_24_8;
	}

	//se for 1D
	if (texture->glTarget==GL_TEXTURE_1D)
	{
		//o de primeiro nivel
		HorseRadish::OpenGL::Extensions::glTextureStorage1DEXT(texture->glID, GL_TEXTURE_1D, Texture::CalculateNumMipMaps(imagem->width), internalFormat, imagem->width);

		//já tá
		return;
	}

	//se for 2D
	if (texture->glTarget==GL_TEXTURE_2D)
	{
		//o de primeiro nivel
		HorseRadish::OpenGL::Extensions::glTextureStorage2DEXT(texture->glID, GL_TEXTURE_2D, Texture::CalculateNumMipMaps(imagem->width, imagem->height), internalFormat, imagem->width, imagem->height);

		//já tá
		return;
	}

	//se for 3D
	if (texture->glTarget==GL_TEXTURE_3D)
	{
		//o de primeiro nivel
		HorseRadish::OpenGL::Extensions::glTextureStorage3DEXT(texture->glID, GL_TEXTURE_3D, Texture::CalculateNumMipMaps(imagem->width, imagem->height, texture->depth), internalFormat, imagem->width, imagem->height, texture->depth);

		//já tá
		return;
	}
}

static
void fazTexturas(const ImgData * const imagem, const Objects::Texture* const texture, const ObjectsManager::TargetType targetType, const int flags, const bool extS3, const bool extVTC)
{
	int internalFormat;
	const ImgData *novaImagem;

	//nada faço
	if (imagem==nullptr || texture==nullptr)
		return;

	//se nao tiver dados, simplifica muito as coisas, porque escuso de fazer muitas contas
	//também se o alvo for DEPTH, posso logo fazer isto (não tem lógica ler depth a partir de uma textura)
	if (imagem->data==nullptr || targetType==ObjectsManager::Depth16 || targetType==ObjectsManager::Depth24 || targetType == ObjectsManager::DepthStencil)
	{
		/*if ( (targetType==ObjectsManager::Depth16 || targetType==ObjectsManager::Depth24) && (imagem->data!=nullptr) )
			SConsole::SConsolePLog(logID,"Forcing depth textures to init with nullptr data.");*/
		fazSemDados(imagem,texture,targetType,flags);
		return;
	}

	//detalhe a usar
	novaImagem=imagem;

	//escolher o formato interno
	internalFormat=toGLInternalFormat(targetType);
	if (internalFormat==0)
		return;
	
	//*****************************
	//não estou num normal map
	if (!(flags & STEXTURE_NORMAL_MAP_MIPS))
	{
		//se é preciso ter compressao, vou tentar comprimir
		if (flags & STEXTURE_COMPRESS)
		{
			int buflen,TCformat;
			unsigned char *bufferZip;

			//tenho de fazer o bind
			HorseRadish::OpenGL::glBindTexture(texture->glTarget, texture->glID);

			//tento e se der então segue na compressao e depois devolve
			bufferZip = compressTex(novaImagem,texture,internalFormat,0,&buflen,&TCformat, extS3, extVTC);
			if (bufferZip == nullptr)
				return;

			//mando os dados comprimidos
			switch(texture->glTarget){
				case GL_TEXTURE_1D:		HorseRadish::OpenGL::glCompressedTexImage1D(GL_TEXTURE_1D, 0, TCformat, novaImagem->width, 0, buflen, bufferZip);
										break;
				case GL_TEXTURE_2D:		HorseRadish::OpenGL::glCompressedTexImage2D(GL_TEXTURE_2D, 0, TCformat, novaImagem->width, novaImagem->height, 0, buflen, bufferZip);
										break;
				case GL_TEXTURE_3D:		HorseRadish::OpenGL::glCompressedTexImage3D(GL_TEXTURE_3D, 0, TCformat, novaImagem->width, novaImagem->height, texture->depth, 0, buflen, bufferZip);
										break;
				}

			//já não preciso disto
			delete[] bufferZip;
		}
		else
		{
			//nao preciso de compressão (ou ela falhou)
			switch(texture->glTarget){
				case GL_TEXTURE_1D:		HorseRadish::OpenGL::Extensions::glTextureStorage1DEXT(texture->glID, GL_TEXTURE_1D, Texture::CalculateNumMipMaps(novaImagem->width), internalFormat, novaImagem->width);
										HorseRadish::OpenGL::Extensions::glTextureSubImage1DEXT(texture->glID, GL_TEXTURE_1D, 0, 0, novaImagem->width, toGLFormat(novaImagem->format), toGLType(novaImagem->type), novaImagem->data);
										break;
				case GL_TEXTURE_2D:		
										HorseRadish::OpenGL::Extensions::glTextureStorage2DEXT(texture->glID, GL_TEXTURE_2D, Texture::CalculateNumMipMaps(novaImagem->width, novaImagem->height), internalFormat, novaImagem->width, novaImagem->height);
										HorseRadish::OpenGL::Extensions::glTextureSubImage2DEXT(texture->glID, GL_TEXTURE_2D, 0, 0, 0, novaImagem->width, novaImagem->height, toGLFormat(novaImagem->format), toGLType(novaImagem->type), novaImagem->data);
										break;
				case GL_TEXTURE_3D:		
										HorseRadish::OpenGL::Extensions::glTextureStorage3DEXT(texture->glID, GL_TEXTURE_3D, Texture::CalculateNumMipMaps(novaImagem->width, novaImagem->height, texture->depth), internalFormat, novaImagem->width, novaImagem->height, texture->depth);
										HorseRadish::OpenGL::Extensions::glTextureSubImage3DEXT(texture->glID, GL_TEXTURE_3D, 0, 0, 0, 0, novaImagem->width, novaImagem->height, texture->depth, toGLFormat(novaImagem->format), toGLType(novaImagem->type), novaImagem->data);
										break;
				}
		}

		//mando gerar os mipmaps e prontos
		HorseRadish::OpenGL::Extensions::glGenerateTextureMipmapEXT(texture->glID, texture->glTarget);
		return;
	}

	//*****************************
	//é para gerar mip maps de um normal map
	//se a textura nao for 2D, vou logo embora
	if (texture->glTarget != GL_TEXTURE_2D)
		return;

	//se é preciso ter compressao, vou tentar comprimir
	if (flags & STEXTURE_COMPRESS)
	{
		int buflen,TCformat;
		unsigned char *bufferZip;

		//tenho de fazer o bind
		HorseRadish::OpenGL::glBindTexture(texture->glTarget, GL_TEXTURE_2D);

		//tento e se der então segue na compressao e depois devolve
		bufferZip = compressTex(novaImagem,texture,internalFormat,0,&buflen,&TCformat,extS3,extVTC);
		if (bufferZip != nullptr)
		{
			HorseRadish::OpenGL::glCompressedTexImage2D(GL_TEXTURE_2D, 0, TCformat, novaImagem->width, novaImagem->height, 0, buflen, bufferZip);
			delete[] bufferZip;
		}

		//e agora se querem mipmaps, tenho de os fazer
		if (!(flags & STEXTURE_NO_MIPMAPS))
			HorseRadish::OpenGL::Extensions::glGenerateTextureMipmapEXT(texture->glID, GL_TEXTURE_2D);

		//e ja tá
		return;
	}

	//não é pra comprimir, mando tudo normalmente
	HorseRadish::OpenGL::Extensions::glTextureStorage2DEXT(texture->glID, GL_TEXTURE_2D, Texture::CalculateNumMipMaps(novaImagem->width, novaImagem->height), internalFormat, novaImagem->width, novaImagem->height);
	HorseRadish::OpenGL::Extensions::glTextureSubImage2DEXT(texture->glID, GL_TEXTURE_2D, 0, 0, 0, novaImagem->width, novaImagem->height, toGLFormat(novaImagem->format), toGLType(novaImagem->type), novaImagem->data);

	//se for preciso mipmaps
	if (!(flags & STEXTURE_NO_MIPMAPS))
		HorseRadish::OpenGL::Extensions::glGenerateTextureMipmapEXT(texture->glID, GL_TEXTURE_2D);
}

//§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§
//§§§§§§   -= Implementação da classe =-   §§§§§
//§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§
const Objects::Texture* ObjectsManager::Create1D(const HorseRadish::IO::Path &path, const ObjectsManager::TargetType targetType, const int flag)
{
	HorseRadish::Imaging::Image *img;
	const Objects::Texture *final;

	//crio o stream para o ficheiro
	HorseRadish::Streams::FileStream fileStream(path, true, false);

	//leio a imagem
	img = HorseRadish::Imaging::Factory::Read(&HorseRadish::Streams::StreamReader(&fileStream));
	if (img == nullptr)
	{
		//SConsole::SConsolePLog(logID,HorseRadish::String("Unable to open file \"%s\"",path));
		return nullptr;
	}

	//crio a textura (não é preciso log)
	final = this->Create1D(img, targetType, flag);

	//limpo o lixo e devolvo
	delete img;
	return final;
}

const Objects::Texture* ObjectsManager::Create1D(const unsigned char * const data, const int comp, const ObjectsManager::TargetType targetType, const int flag)
{
	ImgData img;

	//verificar parametros de entrada
	if (comp < 1)
		return nullptr;

	//preparo isto pra mandar pro fazTexturas
	img.width=comp;
	img.height=1;
	img.data=(void *)data;
	img.type = HorseRadish::Imaging::Image::UByte;
	img.format = HorseRadish::Imaging::Image::RGBA;
	
	//carrego (não é preciso log)
	return this->create1D(&img, targetType, flag);
}

const Objects::Texture* ObjectsManager::Create1D(const HorseRadish::Imaging::Image * const img, const ObjectsManager::TargetType targetType, const int flag)
{
	ImgData imgData;

	prepareImgData(&imgData, img);
	return this->create1D(&imgData, targetType, flag);
}

const Objects::Texture* ObjectsManager::create1D(const ImgData * const img, const ObjectsManager::TargetType targetType, const int flag)
{
	Objects::Texture *textura;

	//verificar parametros de entrada
	if (img==nullptr || toGLInternalFormat(targetType)==0)
		return nullptr;

	//crio nova entrada
	textura = (Objects::Texture*)this->ObjectCreate(ObjectsManager::Texture1D);
	if (textura==nullptr)
		return nullptr;

	//preencho a entrada
	textura->width = img->width;
	textura->glTarget = GL_TEXTURE_1D;

	//a imagem a ir tem de ter largura 1
	if (img->height == 1)
	{
		//faço a textura
		fazTexturas(img, textura,targetType,flag, this->context->extensionsAvailable & Context::CompressionS3, this->context->extensionsAvailable & Context::CompressionVTC);
	}
	else
	{
		ImgData imgLarg1;

		//brinco com ponteiros de forma a que aponte para o meio
		imgLarg1.width = img->width;
		imgLarg1.type = img->type;
		imgLarg1.format = img->format;
		//imgLarg1.data=((unsigned char*)img->data)+(SImgGetRowSize(img)*(img->height/2));
		imgLarg1.data = img->data;

		//e é isto que vai
		fazTexturas(&imgLarg1,textura,targetType,flag, this->context->extensionsAvailable & Context::CompressionS3, this->context->extensionsAvailable & Context::CompressionVTC);
	}
	
	//posso bazar
	//SConsole::SConsolePLog(logID,HorseRadish::String("Loaded 1D texture (texture count: %d).",numTextureList));
	return textura;
}

const Objects::Texture* ObjectsManager::Create2D(const HorseRadish::IO::Path &path, const ObjectsManager::TargetType targetType, const int flag)
{
	HorseRadish::Imaging::Image *img;
	const Objects::Texture *final;

	//crio o stream para o ficheiro
	HorseRadish::Streams::FileStream fileStream(path, true, false);

	//leio a imagem
	img = HorseRadish::Imaging::Factory::Read(&HorseRadish::Streams::StreamReader(&fileStream));
	if (img == nullptr)
	{
		//SConsole::SConsolePLog(logID,HorseRadish::String("Unable to open file \"%s\"",path));
		return nullptr;
	}

	//crio a textura (não é preciso log)
	final=this->Create2D(img,targetType,flag);

	//limpo o lixo e devolvo
	delete img;
	return final;
}

const Objects::Texture* ObjectsManager::Create2D(const unsigned char * const data, const int comp, const int larg, const ObjectsManager::TargetType targetType, const int flag)
{
	ImgData img;

	//verificar parametros de entrada
	if (comp<1 || larg<1)
		return nullptr;

	//preparo isto pra mandar pro fazTexturas
	img.width=comp;
	img.height=larg;
	img.data=(void *)data;
	img.type = HorseRadish::Imaging::Image::UByte;
	img.format = HorseRadish::Imaging::Image::RGBA;

	//crio a textura (não é preciso log)
	return this->create2D(&img,targetType,flag);
}

const Objects::Texture* ObjectsManager::Create2D(const HorseRadish::Imaging::Image * const img, const ObjectsManager::TargetType targetType, const int flag)
{
	ImgData imgData;

	prepareImgData(&imgData, img);
	return this->create2D(&imgData, targetType, flag);
}

const Objects::Texture* ObjectsManager::create2D(const ImgData * const img, const ObjectsManager::TargetType targetType, const int flag)
{
	Objects::Texture *textura;

	//verificar parametros de entrada
	if (img==nullptr || toGLInternalFormat(targetType)==0)
		return nullptr;

	//crio nova entrada
	textura = (Objects::Texture*)this->ObjectCreate(ObjectsManager::Texture2D);
	if (textura==nullptr)
		return nullptr;

	//preencho a entrada
	textura->width = img->width;
	textura->height = img->height;
	textura->glTarget = GL_TEXTURE_2D;

	//faço a textura
	fazTexturas(img, textura, targetType, flag, this->context->extensionsAvailable & Context::CompressionS3, this->context->extensionsAvailable & Context::CompressionVTC);
	
	//posso bazar
	//SConsole::SConsolePLog(logID,HorseRadish::String("Loaded 2D texture (texture count: %d).", numTextureList));
	return textura;
}

const Objects::Texture* ObjectsManager::Create3D(const unsigned char * const data, const int comp, const int larg, const int depth, const ObjectsManager::TargetType targetType, const int flag)
{
	Objects::Texture *textura;
	ImgData img;

	//verificar parametros de entrada
	if (comp<1 || larg<1 || depth<1 || toGLInternalFormat(targetType)==0)
		return nullptr;

	//o formato não pode ser de depth
	if (targetType==ObjectsManager::Depth16 || targetType==ObjectsManager::Depth24 || targetType == ObjectsManager::DepthStencil)
		return nullptr;

	//crio nova entrada
	textura = (Objects::Texture*)this->ObjectCreate(ObjectsManager::Texture3D);
	if (textura==nullptr)
		return nullptr;

	//preencho a entrada
	textura->width = comp;
	textura->height = larg;
	textura->depth = depth;
	textura->glTarget=GL_TEXTURE_3D;

	//preparo isto pra mandar pro fazTexturas
	img.width=comp;
	img.height=larg;
	img.data=(void *)data;

	//faço a textura
	fazTexturas(&img, textura, targetType,flag, this->context->extensionsAvailable & Context::CompressionS3, this->context->extensionsAvailable & Context::CompressionVTC);
	
	//posso bazar
	return textura;
}

const Objects::Texture* ObjectsManager::CreateCube(const unsigned int faceSize, const ObjectsManager::TargetType targetType, const int flag)
{
	Objects::Texture *textura;
	unsigned int mipSize,mipmap,internalFormat;

	//verifica parametros (tamanho e formato)
	if ( (faceSize==0) || (toGLInternalFormat(targetType)==0) )
		return nullptr;

	//posso criar uma nova entrada
	textura = (Objects::Texture*)this->ObjectCreate(ObjectsManager::TextureCubeMap);
	if (textura==nullptr)
		return nullptr;

	//preencho a entrada
	textura->width = faceSize;
	textura->height = faceSize;
	textura->glTarget=GL_TEXTURE_CUBE_MAP;

	//alguns parametros como é óbvio
	HorseRadish::OpenGL::glBindTexture(GL_TEXTURE_CUBE_MAP,textura->glID);

	//qual o formato GL dela
	internalFormat=toGLInternalFormat(targetType);
	if (internalFormat==0)
		return nullptr;

	//o de primeiro nivel vai sempre
	HorseRadish::OpenGL::glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X, 0, internalFormat, faceSize, faceSize, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
	HorseRadish::OpenGL::glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_Y, 0, internalFormat, faceSize, faceSize, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
	HorseRadish::OpenGL::glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_Z, 0, internalFormat, faceSize, faceSize, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
	HorseRadish::OpenGL::glTexImage2D(GL_TEXTURE_CUBE_MAP_NEGATIVE_X, 0, internalFormat, faceSize, faceSize, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
	HorseRadish::OpenGL::glTexImage2D(GL_TEXTURE_CUBE_MAP_NEGATIVE_Y, 0, internalFormat, faceSize, faceSize, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
	HorseRadish::OpenGL::glTexImage2D(GL_TEXTURE_CUBE_MAP_NEGATIVE_Z, 0, internalFormat, faceSize, faceSize, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);

	//se quiserem mais niveis
	if (!(flag & STEXTURE_NO_MIPMAPS) && (faceSize!=1))
		{
		mipSize=faceSize/2;
		mipmap=1;
		while(1)
			{
			HorseRadish::OpenGL::glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X, mipmap, internalFormat, mipSize, mipSize, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
			HorseRadish::OpenGL::glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_Y, mipmap, internalFormat, mipSize, mipSize, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
			HorseRadish::OpenGL::glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_Z, mipmap, internalFormat, mipSize, mipSize, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
			HorseRadish::OpenGL::glTexImage2D(GL_TEXTURE_CUBE_MAP_NEGATIVE_X, mipmap, internalFormat, mipSize, mipSize, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
			HorseRadish::OpenGL::glTexImage2D(GL_TEXTURE_CUBE_MAP_NEGATIVE_Y, mipmap, internalFormat, mipSize, mipSize, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
			HorseRadish::OpenGL::glTexImage2D(GL_TEXTURE_CUBE_MAP_NEGATIVE_Z, mipmap, internalFormat, mipSize, mipSize, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
			if (mipSize==1)
				break;
			mipmap++;
			mipSize/=2;
			if (mipSize<=0)
				mipSize=1;
			}
			
		}

	//já tá
	return textura;
}

const Objects::Texture* ObjectsManager::CreateCube(const HorseRadish::IO::Path &path, const ObjectsManager::TargetType targetType, const int flag)
{
	HorseRadish::Imaging::Image *img, *imgCrop;
	bool compress, domipmap;
	Objects::Texture *textura;
	int curFace, curFaceTarget, buflen, TCformat;
	unsigned int internalFormat, imgComp, imgLarg;
	unsigned char *bufferZip;

	//verifica parametros
	if (path.IsEmpty() || toGLInternalFormat(targetType) == 0)
		return nullptr;

	//crio o stream para o ficheiro
	HorseRadish::Streams::FileStream fileStream(path, true, false);

	//leio a imagem
	img = HorseRadish::Imaging::Factory::Read(&HorseRadish::Streams::StreamReader(&fileStream));
	if (img == nullptr)
	{
		//SConsole::SConsolePLog(logID,HorseRadish::String("Unable to open file \"%s\"",path));
		return nullptr;
	}

	//verifico se a textura corresponde aquilo que quero
	if (img->GetWidth() % 3 != 0 || img->GetHeight() % 4 != 0 || img->GetWidth() % 3 != img->GetHeight() % 4)
	{
		delete img;
		return nullptr;
	}

	//qual o formato GL dela
	internalFormat = toGLInternalFormat(targetType);
	if (internalFormat == 0)
	{
		delete img;
		return nullptr;
	}

	//crio nova entrada
	textura = (Objects::Texture*)this->ObjectCreate(ObjectsManager::TextureCubeMap);
	if (textura == nullptr)
	{
		delete img;
		return nullptr;
	}

	//preencho a entrada
	imgComp = img->GetWidth() / 3;
	imgLarg = img->GetHeight() / 4;
	textura->width = imgComp;
	textura->height = imgLarg;
	textura->glTarget = GL_TEXTURE_CUBE_MAP;

	//o bind tem sempre de ser e coloco um filtro por defeito
	HorseRadish::OpenGL::glBindTexture(GL_TEXTURE_CUBE_MAP, textura->glID);

	//o que quero fazer
	domipmap = !(flag & STEXTURE_NO_MIPMAPS);
	compress = (flag & STEXTURE_COMPRESS);

	//se é para fazer mipmaps e nao quero comprimir e tenho geração de mipmaps, já tá
	/*if ( domipmap && (extAvailable & EXT_GEN_MIP))
		GlTexParameteri(GL_TEXTURE_CUBE_MAP,GL_GENERATE_MIPMAP_SGIS,GL_TRUE);*/

	//************
	//carrego todas as 6 imagens
	imgCrop = nullptr;
	for (curFace = 0; curFace < 6; curFace++)
	{
		//se não tenho de apagar nada antes
		if (imgCrop)
			delete imgCrop;

		//faço uma cópia e depois corto a textura
		imgCrop = img->Clone();
		if (imgCrop == nullptr)
			continue;

		//contruo o caminho
		switch (curFace){
		case 0:	imgCrop->Crop(imgComp * 2, imgLarg * 1, imgComp, imgLarg);
			curFaceTarget = GL_TEXTURE_CUBE_MAP_POSITIVE_X;
			break;
		case 1:	imgCrop->Crop(imgComp * 1, imgLarg * 0, imgComp, imgLarg);
			curFaceTarget = GL_TEXTURE_CUBE_MAP_POSITIVE_Y;
			break;
		case 2:	imgCrop->Crop(imgComp * 1, imgLarg * 3, imgComp, imgLarg);
			curFaceTarget = GL_TEXTURE_CUBE_MAP_POSITIVE_Z;
			break;
		case 3:	imgCrop->Crop(imgComp * 0, imgLarg * 1, imgComp, imgLarg);
			curFaceTarget = GL_TEXTURE_CUBE_MAP_NEGATIVE_X;
			break;
		case 4:	imgCrop->Crop(imgComp * 1, imgLarg * 2, imgComp, imgLarg);
			curFaceTarget = GL_TEXTURE_CUBE_MAP_NEGATIVE_Y;
			break;
		case 5:	imgCrop->Crop(imgComp * 1, imgLarg * 1, imgComp, imgLarg);
			curFaceTarget = GL_TEXTURE_CUBE_MAP_NEGATIVE_Z;
			break;
		}

		//se for para não comprimir e fazer os mipmaps
		if ((domipmap && !compress) || (!domipmap && !compress))
		{
			HorseRadish::OpenGL::glTexImage2D(curFaceTarget, 0, internalFormat, imgComp, imgLarg, 0, toGLFormat(imgCrop->GetFormat()), toGLType(imgCrop->GetType()), imgCrop->GetPixelData());
			if (domipmap)
				HorseRadish::OpenGL::glGenerateMipmap(curFaceTarget);
			continue;
		}

		//se tenho compressão, mando o comprimido
		if (compress)
		{
			ImgData imgCropData;

			prepareImgData(&imgCropData, imgCrop);

			//tento comprimir
			bufferZip = compressTexCube(&imgCropData, curFaceTarget, internalFormat, 0, &buflen, &TCformat, this->context->extensionsAvailable & Context::CompressionS3);
			if (bufferZip != nullptr)
			{
				HorseRadish::OpenGL::glCompressedTexImage2D(curFaceTarget, 0, TCformat, imgComp, imgLarg, 0, buflen, bufferZip);
				delete[] bufferZip;
				bufferZip = nullptr;
			}
			else
			{
				HorseRadish::OpenGL::glTexImage2D(curFaceTarget, 0, internalFormat, imgComp, imgLarg, 0, toGLFormat(imgCrop->GetFormat()), toGLType(imgCrop->GetType()), imgCrop->GetPixelData());
			}

			//não é preciso fazer mais nada
			continue;
		}
	}

	//posso apagar isto
	delete img;
	if (imgCrop != nullptr)
		delete imgCrop;

	//posso sair todo contente
	return textura;
}

const Objects::Texture* ObjectsManager::CreateRect(const bool immutable, const HorseRadish::IO::Path &path, const ObjectsManager::TargetType targetType)
{
	HorseRadish::Imaging::Image *img;
	const Objects::Texture *final;

	//crio o stream para o ficheiro
	HorseRadish::Streams::FileStream fileStream(path, true, false);

	//leio a imagem
	img = HorseRadish::Imaging::Factory::Read(&HorseRadish::Streams::StreamReader(&fileStream));
	if (img == nullptr)
	{
		//SConsole::SConsolePLog(logID,HorseRadish::String("Unable to open file \"%s\"",path));
		return nullptr;
	}

	//crio a textura (não é preciso log)
	final = this->CreateRect(immutable, img, targetType);

	//limpo o lixo e devolvo
	delete img;
	return final;
}

const Objects::Texture* ObjectsManager::CreateRect(const bool immutable, const unsigned char * const data, const int comp, const int larg, const ObjectsManager::TargetType targetType)
{
	ImgData img;

	//verificar parametros de entrada
	if (comp < 1 || larg < 1)
		return nullptr;

	//preparo isto pra mandar pro fazTexturas
	img.width = comp;
	img.height = larg;
	img.data = (void *)data;
	img.type = HorseRadish::Imaging::Image::UByte;
	img.format = HorseRadish::Imaging::Image::RGBA;

	//crio a textura (não é preciso log)
	return this->createRect(immutable, &img, targetType);
}

const Objects::Texture* ObjectsManager::CreateRect(const bool immutable, const HorseRadish::Imaging::Image * const img, const ObjectsManager::TargetType targetType)
{
	ImgData imgData;

	prepareImgData(&imgData, img);
	return this->createRect(immutable, &imgData, targetType);
}

const Objects::Texture* ObjectsManager::createRect(const bool immutable, const ImgData * const img, const ObjectsManager::TargetType targetType)
{
	Objects::Texture *textura;
	unsigned char *bufferZip;
	unsigned int iGLFormat, iGLType;
	int internalFormat, buflenZip, TCformat;

	//verificar parametros de entrada
	if (img == nullptr)
		return nullptr;

	//qual o formato GL dela
	internalFormat = toGLInternalFormat(targetType);
	if (internalFormat == 0)
		return nullptr;

	//crio nova entrada
	textura = (Objects::Texture*)this->ObjectCreate(ObjectsManager::TextureRect);
	if (textura == nullptr)
		return nullptr;

	//preencho a entrada
	textura->width = img->width;
	textura->height = img->height;
	textura->glTarget = GL_TEXTURE_RECTANGLE;

	//calculo o formato e tipo de dados que tenho de ter (se o formato for DEPTH tenho de ter cuidado)
	iGLFormat = toGLFormat(img->format);
	iGLType = toGLType(img->type);
	if (targetType == ObjectsManager::Depth16 || targetType == ObjectsManager::Depth24)
	{
		iGLFormat = GL_DEPTH_COMPONENT;
		iGLType = GL_UNSIGNED_INT;
	}
	else if (targetType == ObjectsManager::DepthStencil)
	{
		iGLFormat = GL_DEPTH_STENCIL;
		iGLType = GL_UNSIGNED_INT_24_8;
	}

	//se for para ficar immutable
	if (immutable == true)
	{
		//tenho de a criar com esta função
		HorseRadish::OpenGL::Extensions::glTextureStorage2DEXT(textura->glID, GL_TEXTURE_RECTANGLE, 1, internalFormat, img->width, img->height);

		//se não tenho dados já posso sair
		if (img->data == nullptr)
			return textura;

		//mando os dados para a textura
		HorseRadish::OpenGL::Extensions::glTextureSubImage2DEXT(textura->glID, GL_TEXTURE_RECTANGLE, 0, 0, 0, img->width, img->height, iGLFormat, iGLType, img->data);
	}
	else
	{
		//crio a textura normalmente
		HorseRadish::OpenGL::Extensions::glTextureImage2DEXT(textura->glID, GL_TEXTURE_RECTANGLE, 0, internalFormat, img->width, img->height, 0, iGLFormat, iGLType, img->data);
	}

	//posso bazar
	//SConsole::SConsolePLog(logID,HorseRadish::String("Loaded RECT texture (texture count: %d).",numTextureList));
	return textura;
}

const Objects::Texture* ObjectsManager::LoadDDS(const HorseRadish::IO::Path &path)
{
	Objects::Texture *textura;
	const void *fileData;
	int fileSize;
	bool fileDataCopied;

	//verificar parametros de entrada
	if (path.IsEmpty() == true)
		return nullptr;

	//crio nova entrada
	textura = nullptr;
	/*textura = novaEntrada();
	if (textura == nullptr)
		return nullptr;

	//crio o stream para o ficheiro
	HorseRadish::Streams::FileStream fileStream(path, true, false);

	//leio tudo
	fileData = fileStream.ReadContent(fileSize, fileDataCopied);

	//tenta fazer o upload
	if (uploadDDS(fileData, fileSize, textura)==false)
	{
		//apago o buffer do ficheiro se for preciso
		if (fileDataCopied == true)
			free((void*)fileData);

		//retiro a textura e saio
		retiraEntrada(textura);
		return nullptr;
	}
	 
	//apago o buffer do ficheiro se for preciso
	if (fileDataCopied == true)
		free((void*)fileData);*/

	//correu tudo bem, posso devolver a textura
	return textura;
}


}//namespace Objects
}//namespace OpenGL
}//namespace HorseRadish