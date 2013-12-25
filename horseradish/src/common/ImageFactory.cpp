#include "ImageFactory.hpp"
#include "imageFactory\ifTGA.hpp"
#include "imageFactory\ifHDRI.hpp"

#include "libs\libJPEG\jpeglib.h"
#include "libs\lodepng\lodepng.h"

///§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§
//§§§§§§   -= Coisas auxiliares =-   §§§§§
//§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§
struct jInputManager{
	static const int BufferSize = 4096;

	jpeg_source_mgr intenalMgr;

	HorseRadish::Streams::StreamReader *streamReader;
	JOCTET *bufferAux;
};
struct jOutputManager{
	static const int BufferSize = 4096;

	jpeg_destination_mgr intenalMgr;

	HorseRadish::Streams::StreamWriter *streamWriter;
	JOCTET *bufferAux;
};

static
void j_init_source(j_decompress_ptr cinfo)
{
	//não é preciso fazer nada
}

static
unsigned char j_fill_input_buffer(j_decompress_ptr cinfo)
{
	size_t bytesLidos;
	jInputManager *srcMgr;

	//preciso da minha estrutura
	srcMgr = (jInputManager*)cinfo->src;

	//tento ler
	bytesLidos = srcMgr->streamReader->Read(srcMgr->bufferAux, jInputManager::BufferSize);

	//se nao li nada
	if (bytesLidos <= 0)
	{
		//insert a fake EOI marker
		srcMgr->bufferAux[0] = (JOCTET)0xFF;
		srcMgr->bufferAux[1] = (JOCTET)JPEG_EOI;
		bytesLidos = 2;
	}

	//ajusto o que li e posso sair
	srcMgr->intenalMgr.next_input_byte = srcMgr->bufferAux;
	srcMgr->intenalMgr.bytes_in_buffer = bytesLidos;
	return TRUE;
}

static
void j_skip_input_data(j_decompress_ptr cinfo, long num_bytes)
{
	jInputManager *srcMgr;

	//preciso da minha estrutura
	srcMgr = (jInputManager*)cinfo->src;

	//se for para realmente saltar alguma coisa
	if (num_bytes > 0)
	{
		//enquanto tiver bytes
		while (num_bytes > (long) srcMgr->intenalMgr.bytes_in_buffer)
		{
			//mando ler
			num_bytes -= (long) srcMgr->intenalMgr.bytes_in_buffer;
			srcMgr->intenalMgr.fill_input_buffer(cinfo);
		}

		//ajusto os valores
		srcMgr->intenalMgr.next_input_byte += (size_t)num_bytes;
		srcMgr->intenalMgr.bytes_in_buffer -= (size_t)num_bytes;
	}
}

static
void j_term_source(j_decompress_ptr cinfo)
{
	//não é preciso fazer nada
}

static
void j_init_destination(j_compress_ptr cinfo)
{
	jOutputManager *destMgr;

	//preciso da minha estrutura
	destMgr = (jOutputManager*)cinfo->dest;

	//preciso deste buffer temporário (é gerido pelo gestor, logo não tenho de o libertar)
	destMgr->bufferAux = (JOCTET *) (*cinfo->mem->alloc_small) ((j_common_ptr) cinfo, JPOOL_IMAGE, jOutputManager::BufferSize * sizeof(JOCTET));

	//indico os valores que podem ser usados
	destMgr->intenalMgr.next_output_byte = destMgr->bufferAux;
	destMgr->intenalMgr.free_in_buffer = jOutputManager::BufferSize;
}

static
unsigned char j_empty_output_buffer(j_compress_ptr cinfo)
{
	jOutputManager *destMgr;

	//preciso da minha estrutura
	destMgr = (jOutputManager*)cinfo->dest;

	//escrevo todo o buffer
	destMgr->streamWriter->Write(destMgr->bufferAux, jOutputManager::BufferSize);

	//ajusto os valores para a próxima escrita
	destMgr->intenalMgr.next_output_byte = destMgr->bufferAux;
	destMgr->intenalMgr.free_in_buffer = jOutputManager::BufferSize;

	//correu tudo bem
	return TRUE;
}

static
void j_term_destination(j_compress_ptr cinfo)
{
	size_t datacount;
	jOutputManager *destMgr;

	//preciso da minha estrutura
	destMgr = (jOutputManager*)cinfo->dest;

	//verifico se é preciso escrever alguma coisa
	datacount = jOutputManager::BufferSize - destMgr->intenalMgr.free_in_buffer;

	//se faltar escrever alguma coisa
	if (datacount > 0)
		destMgr->streamWriter->Write(destMgr->bufferAux, datacount);
}

static 
void j_error_exit(j_common_ptr cinfo)
{
}

static
void j_output_message(j_common_ptr cinfo)
{
}


///§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§
//§§§§§§   -= Implementação da Factory =-   §§§§§
//§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§

namespace HorseRadish
{
namespace Imaging
{

HorseRadish::Imaging::Image* Factory::CreateImageUByte(const int width, const int height, const HorseRadish::Imaging::Image::ImageFormat format, hUInt8 ** const imageBuffer)
{
	HorseRadish::Imaging::Image *newImage;

	//por omissão
	*imageBuffer = nullptr;

	//se isto não faz sentido
	if ((width <= 0) || (height <= 0))
		return nullptr;

	//crio então uma nova imagem
	newImage = new HorseRadish::Imaging::Image(width, height, HorseRadish::Imaging::Image::UByte, format);
	if (newImage == nullptr)
		return nullptr;

	//se a imagem não está válida
	if (newImage->IsValid() == false)
	{
		delete newImage;
		return nullptr;
	}

	//guardo o ponteiro do buffer da imagem e já tá
	*imageBuffer = (hUInt8 *)newImage->data;
	return newImage;
}

HorseRadish::Imaging::Image* Factory::CreateImageFloat(const int width, const int height, const HorseRadish::Imaging::Image::ImageFormat format, hFloat ** const imageBuffer)
{
	HorseRadish::Imaging::Image *newImage;

	//por omissão
	*imageBuffer = nullptr;

	//se isto não faz sentido
	if ((width <= 0) || (height <= 0))
		return nullptr;

	//crio então uma nova imagem
	newImage = new HorseRadish::Imaging::Image(width, height, HorseRadish::Imaging::Image::Float, format);
	if (newImage == nullptr)
		return nullptr;

	//se a imagem não está válida
	if (newImage->IsValid() == false)
	{
		delete newImage;
		return nullptr;
	}

	//guardo o ponteiro do buffer da imagem e já tá
	*imageBuffer = (hFloat *)newImage->data;
	return newImage;
}

HorseRadish::Imaging::Image* Factory::Read(HorseRadish::Streams::StreamReader * const streamReader)
{
	unsigned char fileHeader[8];
	
	//tenho de ler pelo menos 8 bytes
	if (streamReader->Read(fileHeader, 8) != 8)
		return nullptr;

	//e ando para trás no stream
	streamReader->Seek(0, HorseRadish::Streams::Stream::Begin);

	//se for JPEG
	if ( (*((unsigned short*)fileHeader) == 0xFFD8) || (*((unsigned short*)fileHeader) == 0xD8FF) )
		return Factory::ReadJPEG(streamReader);

	//se for PNG
	if (fileHeader[0]==0x89 && fileHeader[1]==0x50 && fileHeader[2]==0x4E && fileHeader[3]==0x47 && fileHeader[4]==0x0D && fileHeader[5]==0x0A && fileHeader[6]==0x1A && fileHeader[7]==0x0A)
		return Factory::ReadPNG(streamReader);

	//mando ver se é TGA e se for, leio
	if (isTGA(streamReader) == true)
		return Factory::ReadTGA(streamReader);

	//chegando aqui não é nada
	return nullptr;
}

HorseRadish::Imaging::Image* Factory::ReadTGA(HorseRadish::Streams::StreamReader * const streamReader)
{
	//basta chamar isto
	return readTGA(streamReader);
}

bool Factory::SaveTGA(HorseRadish::Streams::StreamWriter * const streamWriter, const HorseRadish::Imaging::Image * const imageToSave)
{
	//basta chamar isto
	return saveTGA(streamWriter, imageToSave);
}

HorseRadish::Imaging::Image* Factory::ReadPNG(HorseRadish::Streams::StreamReader * const streamReader)
{
	int streamBufferSize;
	bool streamCopied;
	unsigned int imgW, imgH;
	LodePNGState pngState;
	HorseRadish::Imaging::Image *newImage;

	auto streamBuffer = streamReader->ReadContent(streamBufferSize, streamCopied);
	if (streamBuffer == nullptr)
		return nullptr;
	
	lodepng_state_init(&pngState);
	if (lodepng_inspect(&imgW, &imgH, &pngState, static_cast<const unsigned char*>(streamBuffer), streamBufferSize) != 0)
		return nullptr;

	newImage = nullptr;

	//decode to RGBA
	if (lodepng_is_alpha_type(&pngState.info_png.color) || (lodepng_is_palette_type(&pngState.info_png.color) && lodepng_has_palette_alpha(&pngState.info_png.color)))
	{
		unsigned char* outBuffer;
		unsigned int outW, outH;

		if (lodepng_decode32(&outBuffer, &outW, &outH, static_cast<const unsigned char *>(streamBuffer), streamBufferSize) != 0)
			return nullptr;

		if (outBuffer != nullptr)
			newImage = new HorseRadish::Imaging::Image(outW, outH, HorseRadish::Imaging::Image::UByte, HorseRadish::Imaging::Image::RGBA, outBuffer, true);
	}
	//decode to RGB
	else if (pngState.info_png.color.colortype == LCT_RGB)
	{
		unsigned char* outBuffer;
		unsigned int outW, outH;

		if (lodepng_decode24(&outBuffer, &outW, &outH, static_cast<const unsigned char *>(streamBuffer), streamBufferSize) != 0)
			return nullptr;

		if (outBuffer != nullptr)
			newImage = new HorseRadish::Imaging::Image(outW, outH, HorseRadish::Imaging::Image::UByte, HorseRadish::Imaging::Image::RGBA, outBuffer, true);
	}

	if (streamCopied == true)
		free((void*)streamBuffer);

	return newImage;
}

bool Factory::SavePNG(HorseRadish::Streams::StreamWriter * const streamWriter, const HorseRadish::Imaging::Image * const imageToSave)
{
	int imgWidth, imgHeight;
	size_t bufferOutSize;
	unsigned char *bufferOut;

	//verifico algumas coisas
	if ((streamWriter == nullptr) || (imageToSave == nullptr))
		return false;

	//por omissão
	bufferOut = nullptr;
	bufferOutSize = 0;

	//isto dá jeito
	imageToSave->GetDims(imgWidth, imgHeight);

	//se a imagem é ubyte RGBA posso mandá-la directamente
	if (imageToSave->Check(HorseRadish::Imaging::Image::UByte, HorseRadish::Imaging::Image::RGBA) == true)
	{
		lodepng_encode32(&bufferOut, &bufferOutSize, (const unsigned char*)imageToSave->data, imgWidth, imgHeight);
	}
	//se a imagem é ubyte RGB posso mandá-la quase directamente
	else if (imageToSave->Check(HorseRadish::Imaging::Image::UByte, HorseRadish::Imaging::Image::RGB) == true)
	{
		lodepng_encode24(&bufferOut, &bufferOutSize, (const unsigned char*)imageToSave->data, imgWidth, imgHeight);
	}
	//chegando aqui tenho de converter qualquer outro formato para RGBA
	else
	{
		HorseRadish::Imaging::Image *imageTemp;

		imageTemp = imageToSave->Clone(HorseRadish::Imaging::Image::UByte, HorseRadish::Imaging::Image::RGBA);
		if (imageTemp == nullptr)
			return false;

		lodepng_encode32(&bufferOut, &bufferOutSize, (const unsigned char*)imageToSave->data, imgWidth, imgHeight);

		delete imageTemp;
	}

	//se não tenho nada
	if ((bufferOut == nullptr) || (bufferOutSize <= 0))
		return false;

	//gravo a imagem para o stream
	streamWriter->Write(bufferOut, bufferOutSize);

	//e limpo o buffer
	free(bufferOut);

	//chegando aqui correu tudo bem
	return true;
}

HorseRadish::Imaging::Image* Factory::ReadJPEG(HorseRadish::Streams::StreamReader * const streamReader)
{
	unsigned char *imgBuffer, *imgWalker;
	int imgWidth, imgHeight, imgBytes, imgComponentes, imgBufferSize;
	jpeg_decompress_struct cinfo;
	jpeg_error_mgr jerr;
	jInputManager jInputMgr;

	//para gerir as mensagens de erro
	cinfo.err = jpeg_std_error(&jerr);
	jerr.error_exit = j_error_exit;
	jerr.output_message = j_output_message;

	//inicio a lib
	jpeg_create_decompress(&cinfo);

	//preciso de criar espaço para um buffer temporário
	jInputMgr.bufferAux = (JOCTET *) cinfo.mem->alloc_small((j_common_ptr)&cinfo, JPOOL_PERMANENT, jInputManager::BufferSize * sizeof(JOCTET));

	//qual o stream a usar para a leitura
	jInputMgr.streamReader = streamReader;

	//só falta passar as coisas à lib
	cinfo.src = (jpeg_source_mgr*)&jInputMgr;
	jInputMgr.intenalMgr.init_source = j_init_source;
	jInputMgr.intenalMgr.fill_input_buffer = j_fill_input_buffer;
	jInputMgr.intenalMgr.skip_input_data = j_skip_input_data;
	jInputMgr.intenalMgr.resync_to_restart = jpeg_resync_to_restart;
	jInputMgr.intenalMgr.term_source = j_term_source;
	jInputMgr.intenalMgr.bytes_in_buffer = 0;
	jInputMgr.intenalMgr.next_input_byte = nullptr;
	
	//leio o cabeçalho
	jpeg_read_header(&cinfo, TRUE);

	//inicio a descompressão
	jpeg_start_decompress(&cinfo);

	//guardo alguns parametros
	imgWidth = cinfo.output_width;
    imgHeight = cinfo.output_height;
    imgBytes = cinfo.num_components<<3;
    imgComponentes = cinfo.num_components;

	//só aceito estes tipos de imagens
	if ((imgBytes != 24) && (imgBytes != 8))
	{
		//fecho tudo
		jpeg_finish_decompress(&cinfo);
		jpeg_destroy_decompress(&cinfo);
		return nullptr;
	}

	//crio o buffer para ler a imagem
	imgBufferSize = imgWidth * imgHeight * imgComponentes;
    imgBuffer = (unsigned char*)malloc(imgBufferSize);
	if (imgBuffer == nullptr)
	{
		//fecho tudo
		jpeg_finish_decompress(&cinfo);
		jpeg_destroy_decompress(&cinfo);
		return nullptr;
	}

    //read scanlines fazendo o flip automaticamente
    while(cinfo.output_scanline < cinfo.output_height)
	{
		imgWalker = imgBuffer + (imgHeight - cinfo.output_scanline - 1) * imgComponentes * imgWidth;
		jpeg_read_scanlines(&cinfo, &imgWalker, 1);
	}

	//fecho tudo como deve de ser
    jpeg_finish_decompress(&cinfo);
    jpeg_destroy_decompress(&cinfo);

	//se li do tipo RGB
	if (imgBytes == 24)
		return new HorseRadish::Imaging::Image(imgWidth, imgHeight, HorseRadish::Imaging::Image::UByte, HorseRadish::Imaging::Image::RGB, imgBuffer, true);
	
	//uma imagem em greyscale
	if (imgBytes == 8)
		return new HorseRadish::Imaging::Image(imgWidth, imgHeight, HorseRadish::Imaging::Image::UByte, HorseRadish::Imaging::Image::Lum, imgBuffer, true);

	//desconheço o formato
	free(imgBuffer);
	return nullptr;
}

bool Factory::SaveJPEG(HorseRadish::Streams::StreamWriter * const streamWriter, const HorseRadish::Imaging::Image * const imageToSave, int qualityPercent)
{
	int imgWidth, imgHeight, imgNumCanais;
	bool imgGrayscale;
	unsigned char *imgData, *imgWalker;
	jOutputManager jOutputMgr;
	jpeg_compress_struct cinfo;
	jpeg_error_mgr jerr;
	HorseRadish::Imaging::Image *imgTemp;

	//verifico algumas coisas
	if ((streamWriter == nullptr) || (imageToSave == nullptr))
		return false;

	//arranjar a qualidade
	qualityPercent = Math::iClamp(qualityPercent, 1, 100);

	//isto dá jeito
	imageToSave->GetDims(imgWidth, imgHeight);
	imgData = (unsigned char *)imageToSave->GetPixelData();
	imgGrayscale = (imageToSave->Check(HorseRadish::Imaging::Image::Lum) || imageToSave->Check(HorseRadish::Imaging::Image::LumAlpha));
	imgNumCanais = imgGrayscale ? 1: 3;

	//por omissão
	imgTemp = nullptr;

	//se não tenho a imagem nestes formatos, tenho de criar uma temporária
	if ((imageToSave->Check(HorseRadish::Imaging::Image::UByte, HorseRadish::Imaging::Image::Lum) == false) && (imageToSave->Check(HorseRadish::Imaging::Image::UByte, HorseRadish::Imaging::Image::RGB) == false))
	{
		//verifico se é lum para aproveitar o facto de ser grayscale
		if (imageToSave->Check(HorseRadish::Imaging::Image::UByte, HorseRadish::Imaging::Image::LumAlpha) == true)
			imgTemp = imageToSave->Clone(HorseRadish::Imaging::Image::UByte, HorseRadish::Imaging::Image::Lum);
		//transformo sempre RGB
		else
			imgTemp = imageToSave->Clone(HorseRadish::Imaging::Image::UByte, HorseRadish::Imaging::Image::RGB);

		//se deu barraca
		if (imgTemp == nullptr)
			return false;
	}

	//se tenho uma nova imagem
	if (imgTemp != nullptr)
		imgData = (unsigned char *)imgTemp->GetPixelData();
	
	//para gerir as mensagens de erro
	cinfo.err = jpeg_std_error(&jerr);
	jerr.error_exit = j_error_exit;
	jerr.output_message = j_output_message;

	//crio o necessário para compressão
	jpeg_create_compress(&cinfo);

	//inicio as coisas para o gestor de leitura interno
	jOutputMgr.intenalMgr.init_destination = j_init_destination;
	jOutputMgr.intenalMgr.empty_output_buffer = j_empty_output_buffer;
	jOutputMgr.intenalMgr.term_destination = j_term_destination;
	jOutputMgr.streamWriter = streamWriter;
	jOutputMgr.bufferAux = nullptr;
	cinfo.dest = (jpeg_destination_mgr*)&jOutputMgr;

	//os dados para a imagem
	cinfo.image_width = imgWidth;
	cinfo.image_height = imgHeight;
	cinfo.input_components = imgGrayscale ? 1 : 3;
	cinfo.in_color_space = imgGrayscale ? JCS_GRAYSCALE : JCS_RGB;
	cinfo.jpeg_color_space = imgGrayscale ? JCS_GRAYSCALE : JCS_RGB;
	
	//os restantes parametros para a compressão são por omissão
	jpeg_set_defaults(&cinfo);
	
	//a qualidade final do jpeg
	jpeg_set_quality(&cinfo, qualityPercent, TRUE);

	//inicio a compressão
	jpeg_start_compress(&cinfo, TRUE);

	//codifico cada linha
	while(cinfo.next_scanline < cinfo.image_height)
		{
		//tiro o ponteiro para a imagem
		imgWalker = imgData + ((imgHeight - cinfo.next_scanline - 1) * imgNumCanais * imgWidth);

		//mando para o jpeg
		jpeg_write_scanlines(&cinfo, &imgWalker, 1);
		}

	//acabo a compressão
	jpeg_finish_compress(&cinfo);
	
	//liberto tudo
	jpeg_destroy_compress(&cinfo);

	//se tiver uma imagem temporária, tenho de a apagar
	if (imgTemp != nullptr)
		delete imgTemp;
	imgTemp = nullptr;

	//chegando aqui correu tudo bem
	return true;
}

HorseRadish::Imaging::Image* Factory::ReadHDRI(HorseRadish::Streams::StreamReader * const streamReader)
{
	//basta chamar isto
	return readHDRI(streamReader);
}

bool Factory::SaveBMP(HorseRadish::Streams::StreamWriter * const streamWriter, const HorseRadish::Imaging::Image * const imageToSave)
{
	int imgWidth, imgHeight;
	BITMAPINFOHEADER bmp_infoh; 
	BITMAPFILEHEADER bmp_fileh;

	//verifico algumas coisas
	if ((streamWriter == nullptr) || (imageToSave == nullptr))
		return false;

	//isto dá jeito
	imageToSave->GetDims(imgWidth, imgHeight);

	//posso já escrever esta estructura
	bmp_fileh.bfOffBits = sizeof (BITMAPFILEHEADER) + sizeof (BITMAPINFOHEADER); 
	bmp_fileh.bfReserved1 = 0; 
	bmp_fileh.bfReserved2 = 0; 
	bmp_fileh.bfSize = sizeof (BITMAPFILEHEADER); 
	bmp_fileh.bfType = 0x4D42; 
	streamWriter->Write(&bmp_fileh,sizeof(BITMAPFILEHEADER));

	//o header (coisas genéricas)
	bmp_infoh.biClrImportant = 0; 
	bmp_infoh.biClrUsed = 0; 
	bmp_infoh.biCompression = BI_RGB; 
	bmp_infoh.biHeight = imgHeight; 
	bmp_infoh.biPlanes = 1; 
	bmp_infoh.biSize = sizeof (BITMAPINFOHEADER); 
	bmp_infoh.biWidth = imgWidth; 
	bmp_infoh.biXPelsPerMeter = 0; 
	bmp_infoh.biYPelsPerMeter = 0;

	//se for unsigned char e com 3 componentes
	if (imageToSave->Check(HorseRadish::Imaging::Image::UByte, HorseRadish::Imaging::Image::BGR) == true)
	{
		const unsigned char *walker;
		unsigned char auxBuffer[16];
		int numPixels;

		//tenho 24 bit
		bmp_infoh.biSizeImage = imgWidth * imgHeight * 4; 
		bmp_infoh.biBitCount = 32; 

		//posso escrever o header logo seguido dos dados todos,
		streamWriter->Write(&bmp_infoh, sizeof(BITMAPINFOHEADER));

		//limpo isto
		auxBuffer[3]=auxBuffer[7]=auxBuffer[11]=auxBuffer[15]=255;

		//tenho de escrever isto de maneira diferente
		walker=(unsigned char*)imageToSave->GetPixelData();
		for(numPixels = imgWidth * imgHeight; numPixels>=4; numPixels-=4,walker+=12)
		{
			auxBuffer[0]=walker[0];		auxBuffer[1]=walker[1];			auxBuffer[2]=walker[2];
			auxBuffer[4]=walker[3];		auxBuffer[5]=walker[4];			auxBuffer[6]=walker[5];
			auxBuffer[8]=walker[6];		auxBuffer[9]=walker[7];			auxBuffer[10]=walker[8];
			auxBuffer[12]=walker[9];	auxBuffer[13]=walker[10];		auxBuffer[14]=walker[11];
			streamWriter->Write(auxBuffer, 16);
		}
		for(; numPixels>0; numPixels--,walker+=3)
		{
			auxBuffer[0]=walker[0];		auxBuffer[1]=walker[1];			auxBuffer[2]=walker[2];
			streamWriter->Write(auxBuffer, 4);
		}

		//saio todo contente
		return true;
	}

	//se for unsigned char e com 4 componentes
	if (imageToSave->Check(HorseRadish::Imaging::Image::UByte, HorseRadish::Imaging::Image::BGRA) == true)
	{
		//tenho 32 bit
		bmp_infoh.biSizeImage = imgWidth * imgHeight * 4; 
		bmp_infoh.biBitCount = 32; 

		//posso escrever o header, logo seguido dos dados todos
		streamWriter->Write(&bmp_infoh, sizeof (BITMAPINFOHEADER)); 
		streamWriter->Write(imageToSave->GetPixelData(), bmp_infoh.biSizeImage);

		//saio todo contente
		return true;
	}

	//se for unsigned char e com 3 componentes
	if (imageToSave->Check(HorseRadish::Imaging::Image::UByte, HorseRadish::Imaging::Image::RGB) == true)
	{
		const unsigned char *walker;
		unsigned char auxBuffer[16];
		int numPixels;

		//tenho 24 bit
		bmp_infoh.biSizeImage = imgWidth * imgHeight * 4; 
		bmp_infoh.biBitCount = 32; 

		//posso escrever o header logo seguido dos dados todos,
		streamWriter->Write(&bmp_infoh, sizeof (BITMAPINFOHEADER));

		//limpo isto
		auxBuffer[3]=auxBuffer[7]=auxBuffer[11]=auxBuffer[15]=255;

		//tenho de escrever isto de maneira diferente
		walker=(unsigned char*)imageToSave->GetPixelData();
		for(numPixels = imgWidth * imgHeight; numPixels>=4; numPixels-=4,walker+=12)
		{
			auxBuffer[0]=walker[2];		auxBuffer[1]=walker[1];			auxBuffer[2]=walker[0];
			auxBuffer[4]=walker[5];		auxBuffer[5]=walker[4];			auxBuffer[6]=walker[3];
			auxBuffer[8]=walker[8];		auxBuffer[9]=walker[7];			auxBuffer[10]=walker[6];
			auxBuffer[12]=walker[11];	auxBuffer[13]=walker[10];		auxBuffer[14]=walker[9];
			streamWriter->Write(auxBuffer, 16);
		}
		for(; numPixels>0; numPixels--,walker+=3)
		{
			auxBuffer[0]=walker[2];		auxBuffer[1]=walker[1];			auxBuffer[2]=walker[0];
			streamWriter->Write(auxBuffer, 4);
		}

		//saio todo contente
		return true;
	}

	//se for unsigned char e com 4 componentes
	if (imageToSave->Check(HorseRadish::Imaging::Image::UByte, HorseRadish::Imaging::Image::RGBA) == true)
	{
		const unsigned char *walker;
		unsigned char auxBuffer[16];
		int numPixels;

		//tenho 32 bit
		bmp_infoh.biSizeImage = imgWidth * imgHeight * 4; 
		bmp_infoh.biBitCount = 32; 

		//posso escrever o header logo seguido dos dados todos,
		streamWriter->Write(&bmp_infoh, sizeof (BITMAPINFOHEADER)); 

		//tenho de escrever isto de maneira diferente
		walker = (unsigned char*)imageToSave->GetPixelData();
		for(numPixels = imgWidth * imgHeight; numPixels>=4; numPixels-=4,walker+=16)
		{
			auxBuffer[0]=walker[2];		auxBuffer[1]=walker[1];			auxBuffer[2]=walker[0];			auxBuffer[3]=walker[3];
			auxBuffer[4]=walker[6];		auxBuffer[5]=walker[5];			auxBuffer[6]=walker[4];			auxBuffer[7]=walker[7];
			auxBuffer[8]=walker[10];	auxBuffer[9]=walker[9];			auxBuffer[10]=walker[8];		auxBuffer[11]=walker[11];
			auxBuffer[12]=walker[14];	auxBuffer[13]=walker[13];		auxBuffer[14]=walker[12];		auxBuffer[15]=walker[15];
			streamWriter->Write(auxBuffer, 16);
		}
		for(; numPixels>0; numPixels--,walker+=4)
		{
			auxBuffer[0]=walker[2];		auxBuffer[1]=walker[1];			auxBuffer[2]=walker[0];			auxBuffer[3]=walker[3];
			streamWriter->Write(auxBuffer, 4);
		}

		//saio todo contente
		return true;
	}

	//tenho de fazer tudo à pata
	unsigned char pixelB[4];
	HorseRadish::Color pixelValue;

	//tenho sempre 32 bit
	bmp_infoh.biSizeImage = imgWidth * imgHeight * 4; 
	bmp_infoh.biBitCount = 32; 
	streamWriter->Write(&bmp_infoh, sizeof (BITMAPINFOHEADER)); 

	//para cada linha
	for(int i = 0; i < imgHeight; i++)
	{
		//para cada pixel
		for(int j = 0; j < imgWidth; j++)
		{
			//leio o pixel e inverto o R e B
			imageToSave->GetPixel(i, j, pixelValue);
			pixelValue.SwapRB();

			//transformo para unsigned byte e escrevo
			pixelValue.Write(pixelB);
			streamWriter->Write(pixelB, 4);
		}
	}

	//já tá
	return true; 
}

}//namespace Imaging
}//namespace HorseRadish