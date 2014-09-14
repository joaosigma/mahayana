#include "ImageFactory.hpp"

#include "imageFactory\ifTGA.hpp"
#include "imageFactory\ifHDRI.hpp"

#include <jpeglib.h>
#include "libs\lodepng\lodepng.h"

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
}

static
unsigned char j_fill_input_buffer(j_decompress_ptr cinfo)
{
	auto srcMgr = reinterpret_cast<jInputManager*>(cinfo->src);

	auto datacount = srcMgr->streamReader->Read(srcMgr->bufferAux, jInputManager::BufferSize);

	if (datacount <= 0)
	{
		srcMgr->bufferAux[0] = (JOCTET)0xFF;
		srcMgr->bufferAux[1] = (JOCTET)JPEG_EOI;
		datacount = 2;
	}

	srcMgr->intenalMgr.next_input_byte = srcMgr->bufferAux;
	srcMgr->intenalMgr.bytes_in_buffer = datacount;
	return TRUE;
}

static
void j_skip_input_data(j_decompress_ptr cinfo, long num_bytes)
{
	auto srcMgr = reinterpret_cast<jInputManager*>(cinfo->src);

	if (num_bytes > 0)
	{
		while (num_bytes > (long) srcMgr->intenalMgr.bytes_in_buffer)
		{
			num_bytes -= (long) srcMgr->intenalMgr.bytes_in_buffer;
			srcMgr->intenalMgr.fill_input_buffer(cinfo);
		}

		srcMgr->intenalMgr.next_input_byte += (size_t)num_bytes;
		srcMgr->intenalMgr.bytes_in_buffer -= (size_t)num_bytes;
	}
}

static
void j_term_source(j_decompress_ptr cinfo)
{
}

static
void j_init_destination(j_compress_ptr cinfo)
{
	auto destMgr = reinterpret_cast<jOutputManager*>(cinfo->dest);

	destMgr->bufferAux = (JOCTET *) (*cinfo->mem->alloc_small) ((j_common_ptr) cinfo, JPOOL_IMAGE, jOutputManager::BufferSize * sizeof(JOCTET));

	destMgr->intenalMgr.next_output_byte = destMgr->bufferAux;
	destMgr->intenalMgr.free_in_buffer = jOutputManager::BufferSize;
}

static
unsigned char j_empty_output_buffer(j_compress_ptr cinfo)
{
	auto destMgr = reinterpret_cast<jOutputManager*>(cinfo->dest);

	destMgr->streamWriter->Write(destMgr->bufferAux, jOutputManager::BufferSize);

	destMgr->intenalMgr.next_output_byte = destMgr->bufferAux;
	destMgr->intenalMgr.free_in_buffer = jOutputManager::BufferSize;

	return TRUE;
}

static
void j_term_destination(j_compress_ptr cinfo)
{
	auto destMgr = reinterpret_cast<jOutputManager*>(cinfo->dest);

	auto datacount = jOutputManager::BufferSize - destMgr->intenalMgr.free_in_buffer;

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

namespace HorseRadish
{
	namespace Imaging
	{
		HorseRadish::Imaging::Image* Factory::CreateImageUByte(const int width, const int height, const HorseRadish::Imaging::Image::ImageFormat format, hUInt8 ** const imageBuffer)
		{
			*imageBuffer = nullptr;

			if ((width <= 0) || (height <= 0))
				return nullptr;

			auto newImage = new HorseRadish::Imaging::Image(width, height, HorseRadish::Imaging::Image::UByte, format);
			if (newImage == nullptr)
				return nullptr;

			if (newImage->IsValid() == false)
			{
				delete newImage;
				return nullptr;
			}

			*imageBuffer = (hUInt8 *)newImage->data;
			return newImage;
		}

		HorseRadish::Imaging::Image* Factory::CreateImageFloat(const int width, const int height, const HorseRadish::Imaging::Image::ImageFormat format, hFloat ** const imageBuffer)
		{
			*imageBuffer = nullptr;

			if ((width <= 0) || (height <= 0))
				return nullptr;

			auto newImage = new HorseRadish::Imaging::Image(width, height, HorseRadish::Imaging::Image::Float, format);
			if (newImage == nullptr)
				return nullptr;

			if (newImage->IsValid() == false)
			{
				delete newImage;
				return nullptr;
			}

			*imageBuffer = (hFloat *)newImage->data;
			return newImage;
		}

		HorseRadish::Imaging::Image* Factory::Read(HorseRadish::Streams::StreamReader &streamReader)
		{
			unsigned char fileHeader[8];

			if (streamReader.Read(fileHeader, 8) != 8)
				return nullptr;

			streamReader.Seek(0, HorseRadish::Streams::Stream::Begin);

			if ((*((unsigned short*)fileHeader) == 0xFFD8) || (*((unsigned short*)fileHeader) == 0xD8FF))
				return Factory::ReadJPEG(streamReader);

			if (fileHeader[0] == 0x89 && fileHeader[1] == 0x50 && fileHeader[2] == 0x4E && fileHeader[3] == 0x47 && fileHeader[4] == 0x0D && fileHeader[5] == 0x0A && fileHeader[6] == 0x1A && fileHeader[7] == 0x0A)
				return Factory::ReadPNG(streamReader);

			if (isTGA(streamReader) == true)
				return Factory::ReadTGA(streamReader);

			return nullptr;
		}

		HorseRadish::Imaging::Image* Factory::ReadTGA(HorseRadish::Streams::StreamReader &streamReader)
		{
			return readTGA(streamReader);
		}

		bool Factory::SaveTGA(HorseRadish::Streams::StreamWriter &streamWriter, const HorseRadish::Imaging::Image * const imageToSave)
		{
			return saveTGA(streamWriter, imageToSave);
		}

		HorseRadish::Imaging::Image* Factory::ReadPNG(HorseRadish::Streams::StreamReader &streamReader)
		{
			int streamBufferSize;
			bool streamCopied;
			unsigned int imgW, imgH;
			LodePNGState pngState;
			HorseRadish::Imaging::Image *newImage;

			auto streamBuffer = streamReader.ReadContent(streamBufferSize, streamCopied);
			if (streamBuffer == nullptr)
				return nullptr;

			lodepng_state_init(&pngState);
			if (lodepng_inspect(&imgW, &imgH, &pngState, static_cast<const unsigned char*>(streamBuffer), streamBufferSize) != 0)
				return nullptr;

			newImage = nullptr;

			if (lodepng_is_alpha_type(&pngState.info_png.color) || (lodepng_is_palette_type(&pngState.info_png.color) && lodepng_has_palette_alpha(&pngState.info_png.color)))
			{
				unsigned int outW, outH;
				unsigned char* outBuffer;

				outW = outH = 0;
				outBuffer = nullptr;
				if (lodepng_decode32(&outBuffer, &outW, &outH, static_cast<const unsigned char *>(streamBuffer), streamBufferSize) != 0)
					return nullptr;

				if (outBuffer != nullptr)
					newImage = new HorseRadish::Imaging::Image(outW, outH, HorseRadish::Imaging::Image::UByte, HorseRadish::Imaging::Image::RGBA, outBuffer, true);
			}
			else if (pngState.info_png.color.colortype == LCT_RGB)
			{
				unsigned int outW, outH;
				unsigned char* outBuffer;

				outW = outH = 0;
				outBuffer = nullptr;
				if (lodepng_decode24(&outBuffer, &outW, &outH, static_cast<const unsigned char *>(streamBuffer), streamBufferSize) != 0)
					return nullptr;

				if (outBuffer != nullptr)
					newImage = new HorseRadish::Imaging::Image(outW, outH, HorseRadish::Imaging::Image::UByte, HorseRadish::Imaging::Image::RGBA, outBuffer, true);
			}

			if (streamCopied == true)
				free((void*)streamBuffer);

			return newImage;
		}

		bool Factory::SavePNG(HorseRadish::Streams::StreamWriter &streamWriter, const HorseRadish::Imaging::Image * const imageToSave)
		{
			int imgWidth, imgHeight;
			size_t bufferOutSize;
			unsigned char *bufferOut;

			if (imageToSave == nullptr)
				return false;

			bufferOut = nullptr;
			bufferOutSize = 0;

			imageToSave->GetDims(imgWidth, imgHeight);

			if (imageToSave->Check(HorseRadish::Imaging::Image::UByte, HorseRadish::Imaging::Image::RGBA) == true)
			{
				lodepng_encode32(&bufferOut, &bufferOutSize, (const unsigned char*)imageToSave->data, imgWidth, imgHeight);
			}
			else if (imageToSave->Check(HorseRadish::Imaging::Image::UByte, HorseRadish::Imaging::Image::RGB) == true)
			{
				lodepng_encode24(&bufferOut, &bufferOutSize, (const unsigned char*)imageToSave->data, imgWidth, imgHeight);
			}
			else
			{
				HorseRadish::Imaging::Image *imageTemp;

				imageTemp = imageToSave->Clone(HorseRadish::Imaging::Image::UByte, HorseRadish::Imaging::Image::RGBA);
				if (imageTemp == nullptr)
					return false;

				lodepng_encode32(&bufferOut, &bufferOutSize, (const unsigned char*)imageToSave->data, imgWidth, imgHeight);

				delete imageTemp;
			}

			if ((bufferOut == nullptr) || (bufferOutSize <= 0))
				return false;

			streamWriter.Write(bufferOut, bufferOutSize);

			free(bufferOut);

			return true;
		}

		HorseRadish::Imaging::Image* Factory::ReadJPEG(HorseRadish::Streams::StreamReader &streamReader)
		{
			unsigned char *imgBuffer, *imgWalker;
			int imgWidth, imgHeight, imgBytes, imgComponentes, imgBufferSize;
			jpeg_decompress_struct cinfo;
			jpeg_error_mgr jerr;
			jInputManager jInputMgr;

			cinfo.err = jpeg_std_error(&jerr);
			jerr.error_exit = j_error_exit;
			jerr.output_message = j_output_message;

			jpeg_create_decompress(&cinfo);

			jInputMgr.bufferAux = (JOCTET *)cinfo.mem->alloc_small((j_common_ptr)&cinfo, JPOOL_PERMANENT, jInputManager::BufferSize * sizeof(JOCTET));

			jInputMgr.streamReader = &streamReader;

			cinfo.src = (jpeg_source_mgr*)&jInputMgr;
			jInputMgr.intenalMgr.init_source = j_init_source;
			jInputMgr.intenalMgr.fill_input_buffer = j_fill_input_buffer;
			jInputMgr.intenalMgr.skip_input_data = j_skip_input_data;
			jInputMgr.intenalMgr.resync_to_restart = jpeg_resync_to_restart;
			jInputMgr.intenalMgr.term_source = j_term_source;
			jInputMgr.intenalMgr.bytes_in_buffer = 0;
			jInputMgr.intenalMgr.next_input_byte = nullptr;

			jpeg_read_header(&cinfo, TRUE);

			jpeg_start_decompress(&cinfo);

			imgWidth = cinfo.output_width;
			imgHeight = cinfo.output_height;
			imgBytes = cinfo.num_components << 3;
			imgComponentes = cinfo.num_components;

			if ((imgBytes != 24) && (imgBytes != 8))
			{
				jpeg_finish_decompress(&cinfo);
				jpeg_destroy_decompress(&cinfo);
				return nullptr;
			}

			imgBufferSize = imgWidth * imgHeight * imgComponentes;
			imgBuffer = (unsigned char*)malloc(imgBufferSize);
			if (imgBuffer == nullptr)
			{
				jpeg_finish_decompress(&cinfo);
				jpeg_destroy_decompress(&cinfo);
				return nullptr;
			}

			while (cinfo.output_scanline < cinfo.output_height)
			{
				imgWalker = imgBuffer + (imgHeight - cinfo.output_scanline - 1) * imgComponentes * imgWidth;
				jpeg_read_scanlines(&cinfo, &imgWalker, 1);
			}

			jpeg_finish_decompress(&cinfo);
			jpeg_destroy_decompress(&cinfo);

			if (imgBytes == 24)
				return new HorseRadish::Imaging::Image(imgWidth, imgHeight, HorseRadish::Imaging::Image::UByte, HorseRadish::Imaging::Image::RGB, imgBuffer, true);

			if (imgBytes == 8)
				return new HorseRadish::Imaging::Image(imgWidth, imgHeight, HorseRadish::Imaging::Image::UByte, HorseRadish::Imaging::Image::Lum, imgBuffer, true);

			free(imgBuffer);
			return nullptr;
		}

		bool Factory::SaveJPEG(HorseRadish::Streams::StreamWriter &streamWriter, const HorseRadish::Imaging::Image * const imageToSave, int qualityPercent)
		{
			int imgWidth, imgHeight, imgNumCanais;
			bool imgGrayscale;
			unsigned char *imgData, *imgWalker;
			jOutputManager jOutputMgr;
			jpeg_compress_struct cinfo;
			jpeg_error_mgr jerr;
			HorseRadish::Imaging::Image *imgTemp;

			if (imageToSave == nullptr)
				return false;

			qualityPercent = Math::iClamp(qualityPercent, 1, 100);

			imageToSave->GetDims(imgWidth, imgHeight);
			imgData = (unsigned char *)imageToSave->GetPixelData();
			imgGrayscale = (imageToSave->Check(HorseRadish::Imaging::Image::Lum) || imageToSave->Check(HorseRadish::Imaging::Image::LumAlpha));
			imgNumCanais = imgGrayscale ? 1 : 3;

			imgTemp = nullptr;

			if ((imageToSave->Check(HorseRadish::Imaging::Image::UByte, HorseRadish::Imaging::Image::Lum) == false) && (imageToSave->Check(HorseRadish::Imaging::Image::UByte, HorseRadish::Imaging::Image::RGB) == false))
			{
				if (imageToSave->Check(HorseRadish::Imaging::Image::UByte, HorseRadish::Imaging::Image::LumAlpha) == true)
					imgTemp = imageToSave->Clone(HorseRadish::Imaging::Image::UByte, HorseRadish::Imaging::Image::Lum);
				else
					imgTemp = imageToSave->Clone(HorseRadish::Imaging::Image::UByte, HorseRadish::Imaging::Image::RGB);

				if (imgTemp == nullptr)
					return false;
			}

			if (imgTemp != nullptr)
				imgData = (unsigned char *)imgTemp->GetPixelData();

			cinfo.err = jpeg_std_error(&jerr);
			jerr.error_exit = j_error_exit;
			jerr.output_message = j_output_message;

			jpeg_create_compress(&cinfo);

			jOutputMgr.intenalMgr.init_destination = j_init_destination;
			jOutputMgr.intenalMgr.empty_output_buffer = j_empty_output_buffer;
			jOutputMgr.intenalMgr.term_destination = j_term_destination;
			jOutputMgr.streamWriter = &streamWriter;
			jOutputMgr.bufferAux = nullptr;
			cinfo.dest = (jpeg_destination_mgr*)&jOutputMgr;

			cinfo.image_width = imgWidth;
			cinfo.image_height = imgHeight;
			cinfo.input_components = imgGrayscale ? 1 : 3;
			cinfo.in_color_space = imgGrayscale ? JCS_GRAYSCALE : JCS_RGB;
			cinfo.jpeg_color_space = imgGrayscale ? JCS_GRAYSCALE : JCS_RGB;

			jpeg_set_defaults(&cinfo);

			jpeg_set_quality(&cinfo, qualityPercent, TRUE);

			jpeg_start_compress(&cinfo, TRUE);

			while (cinfo.next_scanline < cinfo.image_height)
			{
				imgWalker = imgData + ((imgHeight - cinfo.next_scanline - 1) * imgNumCanais * imgWidth);
				jpeg_write_scanlines(&cinfo, &imgWalker, 1);
			}

			jpeg_finish_compress(&cinfo);

			jpeg_destroy_compress(&cinfo);

			if (imgTemp != nullptr)
				delete imgTemp;
			imgTemp = nullptr;

			return true;
		}

		HorseRadish::Imaging::Image* Factory::ReadHDRI(HorseRadish::Streams::StreamReader &streamReader)
		{
			return readHDRI(streamReader);
		}

		bool Factory::SaveBMP(HorseRadish::Streams::StreamWriter &streamWriter, const HorseRadish::Imaging::Image * const imageToSave)
		{
			int imgWidth, imgHeight;
			BITMAPINFOHEADER bmp_infoh;
			BITMAPFILEHEADER bmp_fileh;

			if (imageToSave == nullptr)
				return false;

			imageToSave->GetDims(imgWidth, imgHeight);

			bmp_fileh.bfOffBits = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER);
			bmp_fileh.bfReserved1 = 0;
			bmp_fileh.bfReserved2 = 0;
			bmp_fileh.bfSize = sizeof(BITMAPFILEHEADER);
			bmp_fileh.bfType = 0x4D42;
			streamWriter.Write(&bmp_fileh, sizeof(BITMAPFILEHEADER));

			bmp_infoh.biClrImportant = 0;
			bmp_infoh.biClrUsed = 0;
			bmp_infoh.biCompression = BI_RGB;
			bmp_infoh.biHeight = imgHeight;
			bmp_infoh.biPlanes = 1;
			bmp_infoh.biSize = sizeof(BITMAPINFOHEADER);
			bmp_infoh.biWidth = imgWidth;
			bmp_infoh.biXPelsPerMeter = 0;
			bmp_infoh.biYPelsPerMeter = 0;

			if (imageToSave->Check(HorseRadish::Imaging::Image::UByte, HorseRadish::Imaging::Image::BGR) == true)
			{
				unsigned char auxBuffer[16];
				int numPixels;

				bmp_infoh.biSizeImage = imgWidth * imgHeight * 4;
				bmp_infoh.biBitCount = 32;

				streamWriter.Write(&bmp_infoh, sizeof(BITMAPINFOHEADER));

				auxBuffer[3] = auxBuffer[7] = auxBuffer[11] = auxBuffer[15] = 255;

				auto walker = reinterpret_cast<const unsigned char*>(imageToSave->GetPixelData());
				for (numPixels = imgWidth * imgHeight; numPixels >= 4; numPixels -= 4, walker += 12)
				{
					auxBuffer[0] = walker[0];		auxBuffer[1] = walker[1];			auxBuffer[2] = walker[2];
					auxBuffer[4] = walker[3];		auxBuffer[5] = walker[4];			auxBuffer[6] = walker[5];
					auxBuffer[8] = walker[6];		auxBuffer[9] = walker[7];			auxBuffer[10] = walker[8];
					auxBuffer[12] = walker[9];	auxBuffer[13] = walker[10];		auxBuffer[14] = walker[11];
					streamWriter.Write(auxBuffer, 16);
				}
				for (; numPixels > 0; numPixels--, walker += 3)
				{
					auxBuffer[0] = walker[0];		auxBuffer[1] = walker[1];			auxBuffer[2] = walker[2];
					streamWriter.Write(auxBuffer, 4);
				}

				return true;
			}

			if (imageToSave->Check(HorseRadish::Imaging::Image::UByte, HorseRadish::Imaging::Image::BGRA) == true)
			{
				bmp_infoh.biSizeImage = imgWidth * imgHeight * 4;
				bmp_infoh.biBitCount = 32;

				streamWriter.Write(&bmp_infoh, sizeof(BITMAPINFOHEADER));
				streamWriter.Write(imageToSave->GetPixelData(), bmp_infoh.biSizeImage);

				return true;
			}

			if (imageToSave->Check(HorseRadish::Imaging::Image::UByte, HorseRadish::Imaging::Image::RGB) == true)
			{
				unsigned char auxBuffer[16];
				int numPixels;

				bmp_infoh.biSizeImage = imgWidth * imgHeight * 4;
				bmp_infoh.biBitCount = 32;

				streamWriter.Write(&bmp_infoh, sizeof(BITMAPINFOHEADER));

				auxBuffer[3] = auxBuffer[7] = auxBuffer[11] = auxBuffer[15] = 255;

				auto walker = reinterpret_cast<const unsigned char*>(imageToSave->GetPixelData());
				for (numPixels = imgWidth * imgHeight; numPixels >= 4; numPixels -= 4, walker += 12)
				{
					auxBuffer[0] = walker[2];		auxBuffer[1] = walker[1];			auxBuffer[2] = walker[0];
					auxBuffer[4] = walker[5];		auxBuffer[5] = walker[4];			auxBuffer[6] = walker[3];
					auxBuffer[8] = walker[8];		auxBuffer[9] = walker[7];			auxBuffer[10] = walker[6];
					auxBuffer[12] = walker[11];	auxBuffer[13] = walker[10];		auxBuffer[14] = walker[9];
					streamWriter.Write(auxBuffer, 16);
				}
				for (; numPixels > 0; numPixels--, walker += 3)
				{
					auxBuffer[0] = walker[2];		auxBuffer[1] = walker[1];			auxBuffer[2] = walker[0];
					streamWriter.Write(auxBuffer, 4);
				}

				return true;
			}

			if (imageToSave->Check(HorseRadish::Imaging::Image::UByte, HorseRadish::Imaging::Image::RGBA) == true)
			{
				unsigned char auxBuffer[16];
				int numPixels;

				bmp_infoh.biSizeImage = imgWidth * imgHeight * 4;
				bmp_infoh.biBitCount = 32;

				streamWriter.Write(&bmp_infoh, sizeof(BITMAPINFOHEADER));

				auto walker = reinterpret_cast<const unsigned char*>(imageToSave->GetPixelData());
				for (numPixels = imgWidth * imgHeight; numPixels >= 4; numPixels -= 4, walker += 16)
				{
					auxBuffer[0] = walker[2];		auxBuffer[1] = walker[1];			auxBuffer[2] = walker[0];			auxBuffer[3] = walker[3];
					auxBuffer[4] = walker[6];		auxBuffer[5] = walker[5];			auxBuffer[6] = walker[4];			auxBuffer[7] = walker[7];
					auxBuffer[8] = walker[10];	auxBuffer[9] = walker[9];			auxBuffer[10] = walker[8];		auxBuffer[11] = walker[11];
					auxBuffer[12] = walker[14];	auxBuffer[13] = walker[13];		auxBuffer[14] = walker[12];		auxBuffer[15] = walker[15];
					streamWriter.Write(auxBuffer, 16);
				}
				for (; numPixels > 0; numPixels--, walker += 4)
				{
					auxBuffer[0] = walker[2];		auxBuffer[1] = walker[1];			auxBuffer[2] = walker[0];			auxBuffer[3] = walker[3];
					streamWriter.Write(auxBuffer, 4);
				}

				return true;
			}

			unsigned char pixelB[4];
			HorseRadish::Color pixelValue;

			bmp_infoh.biSizeImage = imgWidth * imgHeight * 4;
			bmp_infoh.biBitCount = 32;
			streamWriter.Write(&bmp_infoh, sizeof(BITMAPINFOHEADER));

			for (int i = 0; i < imgHeight; i++)
			{
				for (int j = 0; j < imgWidth; j++)
				{
					imageToSave->GetPixel(i, j, pixelValue);
					pixelValue.SwapRB();

					pixelValue.Write(pixelB);
					streamWriter.Write(pixelB, 4);
				}
			}

			return true;
		}

	} //Imaging
} //HorseRadish