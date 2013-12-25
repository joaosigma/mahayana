#include "common\imageFactory.hpp"
#include "ifTGA.hpp"

//§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§
//§§§§§§ Alguns dados para usar o TGA		§§
//§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§

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

HorseRadish::Imaging::Image* readTGA(HorseRadish::Streams::StreamReader * const streamReader)
{
	Tgaheader header;
	bool fliped;

	//leio o header
	streamReader->Read(&header, sizeof(header));

	//verificar se está invertida
	//POR OMISSÃO: leio tudo como se tivesse flip pq é o caso mais comum
	fliped = (header.attrib & (1<<5)) ? true : false;

	//se a imagem não for comprimida e for de 24bit ou 32bit, posso logo despachar
	if (!(header.imagetype & 0x08) && (header.bpp==24 || header.bpp==32))
	{
		int imgWidth, imgHeight;
		unsigned char *imgWalker;
		HorseRadish::Imaging::Image *newImage;

		//a imagem final pra onde irão as coisas
		newImage = HorseRadish::Imaging::Factory::CreateImageUByte(header.width, header.height, HorseRadish::Imaging::Image::BGRA, &imgWalker);
		if (newImage == nullptr)
			return nullptr;

		//as dimensões
		newImage->GetDims(imgWidth, imgHeight);

		//passo à frente da palette
		streamReader->Seek(header.descriptionlen + header.cmapentries * header.cmapbits / 8);

		//se for de 24bits
		if (header.bpp==24)
		{
			//para cada linha
			for (int y=0; y<imgHeight; y++)
			{
				unsigned int pixelPos;

				//ajusto o ponteiro para esta linha
				pixelPos = imgWidth * (imgHeight - y - 1) * 3;

				//para cada coluna
				for (int x = 0; x < imgWidth; x++, pixelPos+=3, imgWalker+=4)
				{
					streamReader->Read(imgWalker, 3);
					imgWalker[3] = 255;
				}
			}

			//se for preciso fazer flip
			if (fliped == true)
				newImage->Flip();
				
			//e devolvo
			return newImage;
		}

		//só pode ser de 32bit, faço tudo por linha
		for (int y=0; y<imgHeight; y++)
		{
			streamReader->Read(imgWalker, imgWidth * 4);
			imgWalker += (imgWidth * 4);
		}

		//se for preciso fazer flip
		if (fliped == true)
			newImage->Flip();

		//e devolvo
		return newImage;
	}

	//chegando aqui tenho de me preparar para pallete e possível compressão
	{
		HorseRadish::Imaging::Image *newImage;
		int imgWidth, imgHeight;
		unsigned int rawSize, dataChannels,channels,tempPixel,pixelPos,palSize;
		unsigned char *rawData,*rawWalker;
		unsigned char *palette,*imgWalker;
		unsigned char v[16];
		unsigned int c,count;
		__int16 temp;

		//preparação
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

		//a imagem final pra onde irão as coisas
		newImage = HorseRadish::Imaging::Factory::CreateImageUByte(header.width, header.height, HorseRadish::Imaging::Image::BGRA, &imgWalker);
		if (newImage == nullptr)
			return nullptr;

		//as dimensões
		newImage->GetDims(imgWidth, imgHeight);

		//se tem pallete
		palette = nullptr;
		palSize = header.descriptionlen + header.cmapentries * header.cmapbits / 8;
		if (palSize > 0)
		{
			//crio espaço pra ela
			palette = (unsigned char*)malloc(sizeof(unsigned char)*palSize);
			if (palette == nullptr)
			{
				delete newImage;
				return nullptr;
			}

			//copio
			streamReader->Read(palette, palSize);
		}

		//ler dados por tratar da imagem
		rawSize = imgWidth * imgHeight * dataChannels;
		rawData = (unsigned char*)malloc(sizeof(unsigned char)*rawSize);
		if (rawData == nullptr)
		{
			if (palette != nullptr)
				free(palette);
			delete newImage;
			return nullptr;
		}

		// Image can be raw or rle, bit 3 of .imagetype tells.
		if (header.imagetype & 0x08)
		{
			rawWalker = rawData;
			while (rawSize > 0)
			{
				c = 0;
				streamReader->Read(&c, 0);

				count=(c&0x7f)+1;
				rawSize-=count*dataChannels;
				if(c&0x80)
				{
					//rle packet
					streamReader->Read(v, dataChannels);

					while(count>0)
					{
						memcpy(rawWalker,v,dataChannels);
						rawWalker += dataChannels;
						count--;
					}
				} 
				else
				{
					count *= dataChannels;

					streamReader->Read(rawWalker, count);
					rawWalker += count;
				}
			}
		}
		else
		{
			//uso este pq a imagem é todo e é grande
			streamReader->Read(rawData, rawSize);
		}
		
		//copio agora tudo o que é necessário para a imagem final
		switch(header.bpp)
			{
			case 8:

				//se tiver palette é uma imagem de palette
				if (palette)
				{
					for (int y = 0; y < imgHeight; y++)
					{
						rawWalker = rawData + imgWidth * (imgHeight - y - 1);
						for (int x = 0; x < imgWidth; x++)
						{
							tempPixel=(*rawWalker)*3;
							rawWalker++;
	          						
							imgWalker[0]=palette[tempPixel+0];
							imgWalker[1]=palette[tempPixel+1];
							imgWalker[2]=palette[tempPixel+2];
							imgWalker[3]=255;
							imgWalker+=4;
						}
					}
					break;
				}
				
				//simples imagem a preto e branco
				for (int y = 0; y < imgHeight; y++)
				{
					rawWalker=rawData+imgWidth*(imgHeight-y-1);
					for (int x = 0; x < imgWidth; x++)
					{
						imgWalker[0]=imgWalker[1]=imgWalker[2]=*rawWalker;
						imgWalker[3]=255;
						rawWalker++;
						imgWalker+=4;
					}
				}
				break;

			case 16:
				for (int y = 0; y < imgHeight; y++)
				{
					for (int x = 0; x < imgWidth; x++)
					{
						temp = ((__int16 *) rawData)[(imgWidth * (imgHeight - y - 1) + x)];
						pixelPos = 4 * (y * imgWidth + x);
						imgWalker[0]=(temp & 0x1F) << 3;
						imgWalker[1]=((temp >> 5) & 0x1F) << 3;
						imgWalker[2]=((temp >> 10) & 0x1F) << 3;
						imgWalker[3]=(temp >> 15)? 255:0;
						imgWalker+=4;
					}
				}
				break;

			case 24:
				for (int y = 0; y < imgHeight; y++)
				{
					rawWalker=rawData+(imgWidth*(imgHeight-y-1)*3);
					for (int x = 0; x < imgWidth; x++)
					{
						imgWalker[0] = rawWalker[0];
						imgWalker[1] = rawWalker[1];
						imgWalker[2] = rawWalker[2];
						imgWalker[3] = 255;
						rawWalker+=3;
						imgWalker+=4;
					}
				}
				break;

			case 32:
				for (int y=0; y<imgHeight; y++)
				{
					memcpy(imgWalker, rawData+(imgWidth*(imgHeight-y-1)*4), imgWidth*4);
					imgWalker += imgWidth*4;
				}
				break;
			}

		//apagar o que não interessa
		if (palette != nullptr)
			free(palette);
		free(rawData);

		//se for preciso fazer flip
		if (fliped == true)
			newImage->Flip();

		//e já está
		return newImage;
	}
}

bool isTGA(HorseRadish::Streams::StreamReader * const streamReader)
{
	Tgaheader header;
	int curStreamPos;

	//leio a posição actual do stream
	curStreamPos = streamReader->GetPosition();

	//leio o header
	streamReader->Read(&header, sizeof(header));

	//reponho a posição do stream
	streamReader->Seek(curStreamPos, HorseRadish::Streams::Stream::Begin);

	//se o tamnho da imagem não for correcto
	if ((header.width <= 0) || (header.height <= 0))
		return false;

	//os tipos de imagem que são suportados
	if ((header.bpp != 8) && (header.bpp != 16) && (header.bpp != 24) && (header.bpp != 32))
		return false;

	//chegando aqui tenho um TGA que posso ler
	return true;
}

bool saveTGA(HorseRadish::Streams::StreamWriter * const streamWriter, const HorseRadish::Imaging::Image * const imageToSave)
{
	const void *imgData;
	unsigned char header[18];
	int imgWidth, imgHeight;

	//verificar isto
	if ((streamWriter == nullptr) || (imageToSave->IsValid() == false))
		return false;

	//isto dá jeito
	imageToSave->GetDims(imgWidth, imgHeight);
	imgData = imageToSave->GetPixelData();

	//o header (coisas genéricas)
	memset(header, 0, 18);
	header[2] = 2;	//ncompressed, RGB images
	header[12] = (unsigned char)(imgWidth%256);
	header[13] = (unsigned char)(imgWidth/256);
	header[14] = (unsigned char)(imgHeight%256);
	header[15] = (unsigned char)(imgHeight/256);

	//se for unsigned char e com 3 componentes (nao esquecer a ordem dos TGAs)
	if (imageToSave->Check(HorseRadish::Imaging::Image::UByte, HorseRadish::Imaging::Image::BGR) == true)
	{
		//tenho 24 bit
		header[16]=24;

		//posso escrever o header, logo seguido dos dados todos, fecho e saio todo contente
		streamWriter->Write(header, 18);
		streamWriter->Write(imgData, imgWidth*imgHeight*3);
		return true;
	}

	//se for unsigned char e com 4 componentes (nao esquecer a ordem dos TGAs)
	if (imageToSave->Check(HorseRadish::Imaging::Image::UByte, HorseRadish::Imaging::Image::BGRA) == true)
	{
		//tenho 32 bit
		header[16]=32;

		//posso escrever o header, logo seguido dos dados todos, fecho e saio todo contente
		streamWriter->Write(header, 18);
		streamWriter->Write(imgData, imgWidth*imgHeight*4);
		return true;
	}

	//se for unsigned char e com 3 componentes (nao esquecer a ordem dos TGAs)
	if (imageToSave->Check(HorseRadish::Imaging::Image::UByte, HorseRadish::Imaging::Image::RGB) == true)
	{
		unsigned char *auxB;
		int i, meta, auxBuffer[16];
		const unsigned char *walker;

		//tenho 24 bit
		header[16]=24;

		//posso escrever o header logo seguido dos dados todos,
		streamWriter->Write(header, 18);

		//tenho de escrever isto de maneira diferente
		walker = (unsigned char*)imgData;
		meta = imgWidth*imgHeight - 4;
		auxB = (unsigned char*)auxBuffer;
		for(i=0; i<meta; i+=4,walker+=12)
		{
			auxB[0]=walker[2];		auxB[1]=walker[1];			auxB[2]=walker[0];
			auxB[3]=walker[5];		auxB[4]=walker[4];			auxB[5]=walker[3];
			auxB[6]=walker[8];		auxB[7]=walker[7];			auxB[8]=walker[6];
			auxB[9]=walker[11];		auxB[10]=walker[10];		auxB[11]=walker[9];
			streamWriter->Write(auxBuffer, 12);
		}
		for(meta+=4; i<meta; i++,walker+=3)
		{
			auxB[0]=walker[2];		auxB[1]=walker[1];			auxB[2]=walker[0];
			streamWriter->Write(auxBuffer, 3);
		}

		//saio todo contente
		return true;
	}

	//se for unsigned char e com 3 componentes (nao esquecer a ordem dos TGAs)
	if (imageToSave->Check(HorseRadish::Imaging::Image::UByte, HorseRadish::Imaging::Image::RGBA) == true)
	{
		unsigned char *auxB;
		int i, meta, auxBuffer[16];
		const unsigned char *walker;

		//tenho 32 bit
		header[16]=32;

		//posso escrever o header logo seguido dos dados todos,
		streamWriter->Write(header,18);

		//tenho de escrever isto de maneira diferente
		walker=(unsigned char*)imgData;
		meta= imgWidth*imgHeight - 4;
		auxB=(unsigned char*)auxBuffer;
		for(i=0; i<meta; i+=4,walker+=16)
		{
			auxB[0]=walker[2];		auxB[1]=walker[1];			auxB[2]=walker[0];			auxB[3]=walker[3];
			auxB[4]=walker[6];		auxB[5]=walker[5];			auxB[6]=walker[4];			auxB[7]=walker[7];
			auxB[8]=walker[10];		auxB[9]=walker[9];			auxB[10]=walker[8];			auxB[11]=walker[11];
			auxB[12]=walker[14];	auxB[13]=walker[13];		auxB[14]=walker[12];		auxB[15]=walker[15];
			streamWriter->Write(auxBuffer, 16);
		}
		for(meta+=4; i<meta; i++,walker+=4)
		{
			auxB[0]=walker[2];		auxB[1]=walker[1];			auxB[2]=walker[0];			auxB[3]=walker[3];
			streamWriter->Write(auxBuffer, 4);
		}

		//saio todo contente
		return true;
	}

	//tenho de fazer tudo à pata
	//tenho sempre 32 bit
	header[16] = 32;
	streamWriter->Write(header,18);

	//para cada linha da imagem
	for(int y = 0; y < imgHeight; y++)
	{
		HorseRadish::Color pixelColor;
		unsigned char auxBuffer[4];

		//para cada pixel
		for(int x = 0; x < imgWidth; x++)
		{
			//leio o pixel e troco o vermelho com o azul
			imageToSave->GetPixel(x, y, pixelColor);
			pixelColor.SwapRB();
			
			//converto para bytes e escrevo
			pixelColor.Write(auxBuffer);
			streamWriter->Write(auxBuffer, 4);
		}
	}

	//maravilha!
	return true;
}