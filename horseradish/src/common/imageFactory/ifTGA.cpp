#include "ifTGA.hpp"

#include "common\imageFactory.hpp"

#pragma pack (push)
#pragma pack (1)

struct Tgaheader {
	__int8	descriptionlen;
	__int8	cmaptype;
	__int8	imagetype;
	__int16	cmapstart;
	__int16	cmapentries;
	__int8	cmapbits;
	__int16	xoffset;
	__int16	yoffset;
	__int16	width;
	__int16	height;
	__int8	bpp;
	__int8	attrib;
};

#pragma pack (pop)

HorseRadish::Imaging::Image* readTGA(HorseRadish::Streams::StreamReader &streamReader)
{
	Tgaheader header;
	bool fliped;

	streamReader.Read(&header, sizeof(header));

	fliped = (header.attrib & (1 << 5)) ? true : false;

	if (!(header.imagetype & 0x08) && (header.bpp == 24 || header.bpp == 32))
	{
		int imgWidth, imgHeight;
		unsigned char *imgWalker;

		auto newImage = HorseRadish::Imaging::Factory::CreateImageUByte(header.width, header.height, HorseRadish::Imaging::Image::BGRA, &imgWalker);
		if (newImage == nullptr)
			return nullptr;

		newImage->GetDims(imgWidth, imgHeight);

		streamReader.Seek(header.descriptionlen + header.cmapentries * header.cmapbits / 8);

		if (header.bpp == 24)
		{
			for (int y = 0; y < imgHeight; y++)
			{
				auto pixelPos = imgWidth * (imgHeight - y - 1) * 3;

				for (int x = 0; x < imgWidth; x++, pixelPos += 3, imgWalker += 4)
				{
					streamReader.Read(imgWalker, 3);
					imgWalker[3] = 255;
				}
			}

			if (fliped == true)
				newImage->Flip();

			return newImage;
		}

		for (int y = 0; y < imgHeight; y++)
		{
			streamReader.Read(imgWalker, imgWidth * 4);
			imgWalker += (imgWidth * 4);
		}

		if (fliped == true)
			newImage->Flip();

		return newImage;
	}

	{
		int imgWidth, imgHeight;
		unsigned int rawSize, dataChannels, channels, tempPixel, pixelPos, palSize;
		unsigned char *rawData, *rawWalker;
		unsigned char *palette, *imgWalker;
		unsigned char v[16];
		unsigned int c, count;
		__int16 temp;

		switch (header.bpp){
		case 8:
			channels = 3;
			dataChannels = 1;
			break;
		case 16:
			channels = 4;
			dataChannels = 2;
			break;
		case 24:
			channels = 3;
			dataChannels = 3;
			break;
		case 32:
			channels = 4;
			dataChannels = 4;
			break;
		default:
			return nullptr;
		}

		auto newImage = HorseRadish::Imaging::Factory::CreateImageUByte(header.width, header.height, HorseRadish::Imaging::Image::BGRA, &imgWalker);
		if (newImage == nullptr)
			return nullptr;

		newImage->GetDims(imgWidth, imgHeight);

		palette = nullptr;
		palSize = header.descriptionlen + header.cmapentries * header.cmapbits / 8;
		if (palSize > 0)
		{
			palette = (unsigned char*)malloc(sizeof(unsigned char)*palSize);
			if (palette == nullptr)
			{
				delete newImage;
				return nullptr;
			}

			streamReader.Read(palette, palSize);
		}

		rawSize = imgWidth * imgHeight * dataChannels;
		rawData = (unsigned char*)malloc(sizeof(unsigned char)*rawSize);
		if (rawData == nullptr)
		{
			if (palette != nullptr)
				free(palette);
			delete newImage;
			return nullptr;
		}

		if (header.imagetype & 0x08)
		{
			rawWalker = rawData;
			while (rawSize > 0)
			{
				c = 0;
				streamReader.Read(&c, 0);

				count = (c & 0x7f) + 1;
				rawSize -= count*dataChannels;
				if (c & 0x80)
				{
					streamReader.Read(v, dataChannels);

					while (count > 0)
					{
						memcpy(rawWalker, v, dataChannels);
						rawWalker += dataChannels;
						count--;
					}
				}
				else
				{
					count *= dataChannels;

					streamReader.Read(rawWalker, count);
					rawWalker += count;
				}
			}
		}
		else
		{
			streamReader.Read(rawData, rawSize);
		}

		switch (header.bpp)
		{
		case 8:

			if (palette)
			{
				for (int y = 0; y < imgHeight; y++)
				{
					rawWalker = rawData + imgWidth * (imgHeight - y - 1);
					for (int x = 0; x < imgWidth; x++)
					{
						tempPixel = (*rawWalker) * 3;
						rawWalker++;

						imgWalker[0] = palette[tempPixel + 0];
						imgWalker[1] = palette[tempPixel + 1];
						imgWalker[2] = palette[tempPixel + 2];
						imgWalker[3] = 255;
						imgWalker += 4;
					}
				}
				break;
			}

			for (int y = 0; y < imgHeight; y++)
			{
				rawWalker = rawData + imgWidth*(imgHeight - y - 1);
				for (int x = 0; x < imgWidth; x++)
				{
					imgWalker[0] = imgWalker[1] = imgWalker[2] = *rawWalker;
					imgWalker[3] = 255;
					rawWalker++;
					imgWalker += 4;
				}
			}
			break;

		case 16:
			for (int y = 0; y < imgHeight; y++)
			{
				for (int x = 0; x < imgWidth; x++)
				{
					temp = ((__int16 *)rawData)[(imgWidth * (imgHeight - y - 1) + x)];
					pixelPos = 4 * (y * imgWidth + x);
					imgWalker[0] = (temp & 0x1F) << 3;
					imgWalker[1] = ((temp >> 5) & 0x1F) << 3;
					imgWalker[2] = ((temp >> 10) & 0x1F) << 3;
					imgWalker[3] = (temp >> 15) ? 255 : 0;
					imgWalker += 4;
				}
			}
			break;

		case 24:
			for (int y = 0; y < imgHeight; y++)
			{
				rawWalker = rawData + (imgWidth*(imgHeight - y - 1) * 3);
				for (int x = 0; x < imgWidth; x++)
				{
					imgWalker[0] = rawWalker[0];
					imgWalker[1] = rawWalker[1];
					imgWalker[2] = rawWalker[2];
					imgWalker[3] = 255;
					rawWalker += 3;
					imgWalker += 4;
				}
			}
			break;

		case 32:
			for (int y = 0; y < imgHeight; y++)
			{
				memcpy(imgWalker, rawData + (imgWidth*(imgHeight - y - 1) * 4), imgWidth * 4);
				imgWalker += imgWidth * 4;
			}
			break;
		}

		if (palette != nullptr)
			free(palette);
		free(rawData);

		if (fliped == true)
			newImage->Flip();

		return newImage;
	}
}

bool isTGA(HorseRadish::Streams::StreamReader &streamReader)
{
	Tgaheader header;

	auto curStreamPos = streamReader.GetPosition();
	streamReader.Read(&header, sizeof(header));
	streamReader.Seek(curStreamPos, HorseRadish::Streams::Stream::Begin);

	if ((header.width <= 0) || (header.height <= 0))
		return false;
	if ((header.bpp != 8) && (header.bpp != 16) && (header.bpp != 24) && (header.bpp != 32))
		return false;

	return true;
}

bool saveTGA(HorseRadish::Streams::StreamWriter &streamWriter, const HorseRadish::Imaging::Image * const imageToSave)
{
	const void *imgData;
	unsigned char header[18];
	int imgWidth, imgHeight;

	if (imageToSave->IsValid() == false)
		return false;

	imageToSave->GetDims(imgWidth, imgHeight);
	imgData = imageToSave->GetPixelData();

	memset(header, 0, 18);
	header[2] = 2;	//ncompressed, RGB images
	header[12] = (unsigned char)(imgWidth % 256);
	header[13] = (unsigned char)(imgWidth / 256);
	header[14] = (unsigned char)(imgHeight % 256);
	header[15] = (unsigned char)(imgHeight / 256);

	if (imageToSave->Check(HorseRadish::Imaging::Image::UByte, HorseRadish::Imaging::Image::BGR) == true)
	{
		header[16] = 24;

		streamWriter.Write(header, 18);
		streamWriter.Write(imgData, imgWidth*imgHeight * 3);
		return true;
	}

	if (imageToSave->Check(HorseRadish::Imaging::Image::UByte, HorseRadish::Imaging::Image::BGRA) == true)
	{
		header[16] = 32;

		streamWriter.Write(header, 18);
		streamWriter.Write(imgData, imgWidth*imgHeight * 4);
		return true;
	}

	if (imageToSave->Check(HorseRadish::Imaging::Image::UByte, HorseRadish::Imaging::Image::RGB) == true)
	{
		int i, auxBuffer[16];

		header[16] = 24;

		streamWriter.Write(header, 18);

		auto walker = reinterpret_cast<const unsigned char*>(imgData);
		auto auxB = reinterpret_cast<unsigned char*>(auxBuffer);
		auto goal = (imgWidth * imgHeight) - 4;

		for (i = 0; i < goal; i += 4, walker += 12)
		{
			auxB[0] = walker[2];		auxB[1] = walker[1];			auxB[2] = walker[0];
			auxB[3] = walker[5];		auxB[4] = walker[4];			auxB[5] = walker[3];
			auxB[6] = walker[8];		auxB[7] = walker[7];			auxB[8] = walker[6];
			auxB[9] = walker[11];		auxB[10] = walker[10];		auxB[11] = walker[9];
			streamWriter.Write(auxBuffer, 12);
		}
		for (goal += 4; i < goal; i++, walker += 3)
		{
			auxB[0] = walker[2];		auxB[1] = walker[1];			auxB[2] = walker[0];
			streamWriter.Write(auxBuffer, 3);
		}

		return true;
	}

	if (imageToSave->Check(HorseRadish::Imaging::Image::UByte, HorseRadish::Imaging::Image::RGBA) == true)
	{
		int i, auxBuffer[16];

		header[16] = 32;

		streamWriter.Write(header, 18);

		auto walker = reinterpret_cast<const unsigned char*>(imgData);
		auto auxB = reinterpret_cast<unsigned char*>(auxBuffer);
		auto goal = (imgWidth*imgHeight) - 4;

		for (i = 0; i < goal; i += 4, walker += 16)
		{
			auxB[0] = walker[2];		auxB[1] = walker[1];			auxB[2] = walker[0];			auxB[3] = walker[3];
			auxB[4] = walker[6];		auxB[5] = walker[5];			auxB[6] = walker[4];			auxB[7] = walker[7];
			auxB[8] = walker[10];		auxB[9] = walker[9];			auxB[10] = walker[8];			auxB[11] = walker[11];
			auxB[12] = walker[14];	auxB[13] = walker[13];		auxB[14] = walker[12];		auxB[15] = walker[15];
			streamWriter.Write(auxBuffer, 16);
		}
		for (goal += 4; i < goal; i++, walker += 4)
		{
			auxB[0] = walker[2];		auxB[1] = walker[1];			auxB[2] = walker[0];			auxB[3] = walker[3];
			streamWriter.Write(auxBuffer, 4);
		}

		return true;
	}

	header[16] = 32;
	streamWriter.Write(header, 18);

	for (int y = 0; y < imgHeight; y++)
	{
		HorseRadish::Color pixelColor;
		unsigned char auxBuffer[4];

		for (int x = 0; x < imgWidth; x++)
		{
			imageToSave->GetPixel(x, y, pixelColor);
			pixelColor.SwapRB();

			pixelColor.Write(auxBuffer);
			streamWriter.Write(auxBuffer, 4);
		}
	}

	return true;
}