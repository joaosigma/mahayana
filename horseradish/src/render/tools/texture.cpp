#include "texture.hpp"

#include "libs/stb/stb_dxt.h"
#include "libs/bc7/bc7enc16.h"
#include "libs/libsquish/squish.h"

namespace hr::render::tools
{
	namespace
	{
#pragma pack(push, 1)
		std::array<uint8_t, 6> CTextureFileSig = { 'h', 'r', 'c', 't', 'e', 'x' };

		struct CTextureHeader
		{
			uint8_t fileSig[CTextureFileSig.size()];
			uint8_t version;
			uint32_t width;
			uint32_t height;
			uint16_t compression; //[1, 3, 4, 5, 6] corresponding to [BC1, BC3, BC4, BC5, BC7]
			uint16_t numLevels; //num mipmaps
		};

		struct CTextureLevelInfo
		{
			uint16_t level;
			uint32_t width;
			uint32_t height;
			uint32_t size;
		};
#pragma pack(pop)

		bool sampleR(const hr::imaging::ImageView<uint8_t, hr::imaging::ImageFormatRGBA>& image, size_t x, size_t y, uint8_t sample[1])
		{
			if ((x >= image.width()) || (y >= image.height()))
				return false;

			uint8_t sampleRGBA[4];
			image.getPixel(x, y, sampleRGBA);

			sample[0] = sampleRGBA[0];
			return true;
		}

		bool sampleRG(const hr::imaging::ImageView<uint8_t, hr::imaging::ImageFormatRGBA>& image, size_t x, size_t y, uint8_t sample[2])
		{
			if ((x >= image.width()) || (y >= image.height()))
				return false;

			uint8_t sampleRGBA[4];
			image.getPixel(x, y, sampleRGBA);

			sample[0] = sampleRGBA[0];
			sample[1] = sampleRGBA[1];
			return true;
		}

		bool sampleRGBA(const hr::imaging::ImageView<uint8_t, hr::imaging::ImageFormatRGBA>& image, size_t x, size_t y, uint8_t sample[4])
		{
			if ((x >= image.width()) || (y >= image.height()))
				return false;

			image.getPixel(x, y, sample);
			return true;
		}

		void sample4x4BlockR(const hr::imaging::ImageView<uint8_t, hr::imaging::ImageFormatRGBA>& image, size_t blockX, size_t blockY, uint8_t sample[16], int& pixelMask)
		{
			auto imageX = blockX * 4;
			auto imageY = blockY * 4;

			pixelMask |= sampleRG(image, imageX + 0, imageY + 0, sample + 0) ? (1 << 0) : 0;
			pixelMask |= sampleRG(image, imageX + 1, imageY + 0, sample + 1) ? (1 << 1) : 0;
			pixelMask |= sampleRG(image, imageX + 2, imageY + 0, sample + 2) ? (1 << 2) : 0;
			pixelMask |= sampleRG(image, imageX + 3, imageY + 0, sample + 3) ? (1 << 3) : 0;

			sample += 4;
			pixelMask |= sampleRG(image, imageX + 0, imageY + 1, sample + 0) ? (1 << 4) : 0;
			pixelMask |= sampleRG(image, imageX + 1, imageY + 1, sample + 1) ? (1 << 5) : 0;
			pixelMask |= sampleRG(image, imageX + 2, imageY + 1, sample + 2) ? (1 << 6) : 0;
			pixelMask |= sampleRG(image, imageX + 3, imageY + 1, sample + 3) ? (1 << 7) : 0;

			sample += 4;
			pixelMask |= sampleRG(image, imageX + 0, imageY + 2, sample + 0) ? (1 << 8) : 0;
			pixelMask |= sampleRG(image, imageX + 1, imageY + 2, sample + 1) ? (1 << 9) : 0;
			pixelMask |= sampleRG(image, imageX + 2, imageY + 2, sample + 2) ? (1 << 10) : 0;
			pixelMask |= sampleRG(image, imageX + 3, imageY + 2, sample + 3) ? (1 << 11) : 0;

			sample += 4;
			pixelMask |= sampleRG(image, imageX + 0, imageY + 3, sample + 0) ? (1 << 12) : 0;
			pixelMask |= sampleRG(image, imageX + 1, imageY + 3, sample + 1) ? (1 << 13) : 0;
			pixelMask |= sampleRG(image, imageX + 2, imageY + 3, sample + 2) ? (1 << 14) : 0;
			pixelMask |= sampleRG(image, imageX + 3, imageY + 3, sample + 3) ? (1 << 15) : 0;
		}

		void sample4x4BlockRG(const hr::imaging::ImageView<uint8_t, hr::imaging::ImageFormatRGBA>& image, size_t blockX, size_t blockY, uint8_t sample[32], int& pixelMask)
		{
			auto imageX = blockX * 4;
			auto imageY = blockY * 4;

			pixelMask |= sampleRG(image, imageX + 0, imageY + 0, sample + 0) ? (1 << 0) : 0;
			pixelMask |= sampleRG(image, imageX + 1, imageY + 0, sample + 2) ? (1 << 1) : 0;
			pixelMask |= sampleRG(image, imageX + 2, imageY + 0, sample + 4) ? (1 << 2) : 0;
			pixelMask |= sampleRG(image, imageX + 3, imageY + 0, sample + 6) ? (1 << 3) : 0;

			sample += 8;
			pixelMask |= sampleRG(image, imageX + 0, imageY + 1, sample + 0) ? (1 << 4) : 0;
			pixelMask |= sampleRG(image, imageX + 1, imageY + 1, sample + 2) ? (1 << 5) : 0;
			pixelMask |= sampleRG(image, imageX + 2, imageY + 1, sample + 4) ? (1 << 6) : 0;
			pixelMask |= sampleRG(image, imageX + 3, imageY + 1, sample + 6) ? (1 << 7) : 0;

			sample += 8;
			pixelMask |= sampleRG(image, imageX + 0, imageY + 2, sample + 0) ? (1 << 8) : 0;
			pixelMask |= sampleRG(image, imageX + 1, imageY + 2, sample + 2) ? (1 << 9) : 0;
			pixelMask |= sampleRG(image, imageX + 2, imageY + 2, sample + 4) ? (1 << 10) : 0;
			pixelMask |= sampleRG(image, imageX + 3, imageY + 2, sample + 6) ? (1 << 11) : 0;

			sample += 8;
			pixelMask |= sampleRG(image, imageX + 0, imageY + 3, sample + 0) ? (1 << 12) : 0;
			pixelMask |= sampleRG(image, imageX + 1, imageY + 3, sample + 2) ? (1 << 13) : 0;
			pixelMask |= sampleRG(image, imageX + 2, imageY + 3, sample + 4) ? (1 << 14) : 0;
			pixelMask |= sampleRG(image, imageX + 3, imageY + 3, sample + 6) ? (1 << 15) : 0;
		}

		void sample4x4BlockRGBA(const hr::imaging::ImageView<uint8_t, hr::imaging::ImageFormatRGBA>& image, size_t blockX, size_t blockY, uint8_t sample[64], int& pixelMask)
		{
			auto imageX = blockX * 4;
			auto imageY = blockY * 4;

			pixelMask |= sampleRGBA(image, imageX + 0, imageY + 0, sample + 0) ? (1 << 0) : 0;
			pixelMask |= sampleRGBA(image, imageX + 1, imageY + 0, sample + 4) ? (1 << 1) : 0;
			pixelMask |= sampleRGBA(image, imageX + 2, imageY + 0, sample + 8) ? (1 << 2) : 0;
			pixelMask |= sampleRGBA(image, imageX + 3, imageY + 0, sample + 12) ? (1 << 3) : 0;

			sample += 16;
			pixelMask |= sampleRGBA(image, imageX + 0, imageY + 1, sample + 0) ? (1 << 4) : 0;
			pixelMask |= sampleRGBA(image, imageX + 1, imageY + 1, sample + 4) ? (1 << 5) : 0;
			pixelMask |= sampleRGBA(image, imageX + 2, imageY + 1, sample + 8) ? (1 << 6) : 0;
			pixelMask |= sampleRGBA(image, imageX + 3, imageY + 1, sample + 12) ? (1 << 7) : 0;

			sample += 16;
			pixelMask |= sampleRGBA(image, imageX + 0, imageY + 2, sample + 0) ? (1 << 8) : 0;
			pixelMask |= sampleRGBA(image, imageX + 1, imageY + 2, sample + 4) ? (1 << 9) : 0;
			pixelMask |= sampleRGBA(image, imageX + 2, imageY + 2, sample + 8) ? (1 << 10) : 0;
			pixelMask |= sampleRGBA(image, imageX + 3, imageY + 2, sample + 12) ? (1 << 11) : 0;

			sample += 16;
			pixelMask |= sampleRGBA(image, imageX + 0, imageY + 3, sample + 0) ? (1 << 12) : 0;
			pixelMask |= sampleRGBA(image, imageX + 1, imageY + 3, sample + 4) ? (1 << 13) : 0;
			pixelMask |= sampleRGBA(image, imageX + 2, imageY + 3, sample + 8) ? (1 << 14) : 0;
			pixelMask |= sampleRGBA(image, imageX + 3, imageY + 3, sample + 12) ? (1 << 15) : 0;
		}

		struct CompressedBlock
		{
			size_t width = 0;
			size_t height = 0;
			size_t perPixelBytes = 0;
		};

		CompressedBlock compressInitBlockBC1(size_t imageWidth, size_t imageHeight)
		{
			CompressedBlock block;

			block.width = imageWidth / 4 + (((imageWidth % 4) > 0) ? 1 : 0);
			block.height = imageHeight / 4 + (((imageHeight % 4) > 0) ? 1 : 0);
			block.perPixelBytes = 8;
			return block;
		}

		CompressedBlock compressInitBlockBC3(size_t imageWidth, size_t imageHeight)
		{
			CompressedBlock block;

			block.width = imageWidth / 4 + (((imageWidth % 4) > 0) ? 1 : 0);
			block.height = imageHeight / 4 + (((imageHeight % 4) > 0) ? 1 : 0);
			block.perPixelBytes = 16;
			return block;
		}

		CompressedBlock compressInitBlockBC4(size_t imageWidth, size_t imageHeight)
		{
			CompressedBlock block;

			block.width = imageWidth / 4 + (((imageWidth % 4) > 0) ? 1 : 0);
			block.height = imageHeight / 4 + (((imageHeight % 4) > 0) ? 1 : 0);
			block.perPixelBytes = 8;
			return block;
		}

		CompressedBlock compressInitBlockBC5(size_t imageWidth, size_t imageHeight)
		{
			CompressedBlock block;

			block.width = imageWidth / 4 + (((imageWidth % 4) > 0) ? 1 : 0);
			block.height = imageHeight / 4 + (((imageHeight % 4) > 0) ? 1 : 0);
			block.perPixelBytes = 16;
			return block;
		}

		CompressedBlock compressInitBlockBC7(size_t imageWidth, size_t imageHeight)
		{
			CompressedBlock block;

			block.width = imageWidth / 4 + (((imageWidth % 4) > 0) ? 1 : 0);
			block.height = imageHeight / 4 + (((imageHeight % 4) > 0) ? 1 : 0);
			block.perPixelBytes = 16;
			return block;
		}

		std::unique_ptr<uint8_t[]> compressImageBC1(const CompressedBlock& compressBlock, const hr::imaging::ImageView<uint8_t, hr::imaging::ImageFormatRGBA>& image)
		{
			assert(compressBlock.perPixelBytes == 8);
			auto compressedImg = std::unique_ptr<uint8_t[]>(new uint8_t[compressBlock.width * compressBlock.height * compressBlock.perPixelBytes]);

			int pixelMask;
			std::array<uint8_t, 8> dstBlock;
			std::array<uint8_t, 4 * 4 * 4> srcBlock; //4x4 RGBA

			for (size_t y = 0; y < compressBlock.height; ++y)
			{
				for (size_t x = 0; x < compressBlock.width; ++x)
				{
					srcBlock.fill(0);
					pixelMask = 0;
					sample4x4BlockRGBA(image, x, y, srcBlock.data(), pixelMask);

					squish::CompressMasked(srcBlock.data(), pixelMask, dstBlock.data(), squish::kDxt1 | squish::kColourClusterFit, nullptr);

					std::memcpy(compressedImg.get() + (y * compressBlock.width * compressBlock.perPixelBytes) + (x * compressBlock.perPixelBytes), dstBlock.data(), dstBlock.size());
				}
			}

			return compressedImg;
		}

		std::unique_ptr<uint8_t[]> compressImageBC3(const CompressedBlock& compressBlock, const hr::imaging::ImageView<uint8_t, hr::imaging::ImageFormatRGBA>& image)
		{
			assert(compressBlock.perPixelBytes == 16);
			auto compressedImg = std::unique_ptr<uint8_t[]>(new uint8_t[compressBlock.width * compressBlock.height * compressBlock.perPixelBytes]);

			int pixelMask;
			std::array<uint8_t, 16> dstBlock;
			std::array<uint8_t, 4 * 4 * 4> srcBlock; //4x4 RGBA

			for (size_t y = 0; y < compressBlock.height; ++y)
			{
				for (size_t x = 0; x < compressBlock.width; ++x)
				{
					srcBlock.fill(0);
					pixelMask = 0;
					sample4x4BlockRGBA(image, x, y, srcBlock.data(), pixelMask);

					squish::CompressMasked(srcBlock.data(), pixelMask, dstBlock.data(), squish::kDxt5 | squish::kColourClusterFit, nullptr);

					std::memcpy(compressedImg.get() + (y * compressBlock.width * compressBlock.perPixelBytes) + (x * compressBlock.perPixelBytes), dstBlock.data(), dstBlock.size());
				}
			}

			return compressedImg;
		}

		std::unique_ptr<uint8_t[]> compressImageBC5(const CompressedBlock& compressBlock, const hr::imaging::ImageView<uint8_t, hr::imaging::ImageFormatRGBA>& image)
		{
			assert(compressBlock.perPixelBytes == 16);
			auto compressedImg = std::unique_ptr<uint8_t[]>(new uint8_t[compressBlock.width * compressBlock.height * compressBlock.perPixelBytes]);

			int pixelMask;
			std::array<uint8_t, 16> dstBlock;
			std::array<uint8_t, 4 * 4 * 2> srcBlock; //4x4 RG

			for (size_t y = 0; y < compressBlock.height; ++y)
			{
				for (size_t x = 0; x < compressBlock.width; ++x)
				{
					srcBlock.fill(0);
					pixelMask = 0;
					sample4x4BlockRG(image, x, y, srcBlock.data(), pixelMask);

					stb_compress_bc5_block(dstBlock.data(), srcBlock.data());

					std::memcpy(compressedImg.get() + (y * compressBlock.width * compressBlock.perPixelBytes) + (x * compressBlock.perPixelBytes), dstBlock.data(), dstBlock.size());
				}
			}

			return compressedImg;
		}

		std::unique_ptr<uint8_t[]> compressImageBC7(const CompressedBlock& compressBlock, const hr::imaging::ImageView<uint8_t, hr::imaging::ImageFormatRGBA>& image)
		{
			assert(compressBlock.perPixelBytes == 16);
			auto compressedImg = std::unique_ptr<uint8_t[]>(new uint8_t[compressBlock.width * compressBlock.height * compressBlock.perPixelBytes]);

			int pixelMask;
			std::array<uint8_t, 16> dstBlock;
			std::array<uint8_t, 4 * 4 * 4> srcBlock; //4x4 RGBA

			bc7enc16_compress_block_init();

			bc7enc16_compress_block_params bc7Params;
			bc7enc16_compress_block_params_init(&bc7Params);

			for (size_t y = 0; y < compressBlock.height; ++y)
			{
				for (size_t x = 0; x < compressBlock.width; ++x)
				{
					srcBlock.fill(0);
					pixelMask = 0;
					sample4x4BlockRGBA(image, x, y, srcBlock.data(), pixelMask);

					auto hasAlpha = bc7enc16_compress_block(dstBlock.data(), srcBlock.data(), &bc7Params);

					std::memcpy(compressedImg.get() + (y * compressBlock.width * compressBlock.perPixelBytes) + (x * compressBlock.perPixelBytes), dstBlock.data(), dstBlock.size());
				}
			}

			return compressedImg;
		}
	}

	bool TextureTools::uploadDiffuse(const hr::imaging::ImageView<uint8_t, hr::imaging::ImageFormatRGBA>& imageSrc, hr::gl::objects::Texture& textureDst)
	{
		if (imageSrc.empty())
			return false;

		textureDst.init(hr::gl::objects::Texture::Type::Tex2D, hr::gl::objects::Texture::StorageType::RGBA_8, imageSrc.width(), imageSrc.height());

		size_t curLevel = 0;
		auto imageScaled = imageSrc.clone();

		while (true)
		{
			textureDst.uploadData(curLevel, 0, 0, imageScaled.width(), imageScaled.height(), hr::gl::objects::Texture::DataFormat::RGBA, hr::gl::objects::Texture::DataType::UBYTE, imageScaled.data());
			if (imageScaled.area() <= 1)
				break;

			curLevel++;
			imageScaled = imageScaled.resize(std::max<size_t>(1, imageScaled.width() >> 1), std::max<size_t>(1, imageScaled.height() >> 1), true);
		};

		return true;
	}

	bool TextureTools::uploadDiffuse(const hr::imaging::ImageView<uint8_t, hr::imaging::ImageFormatRGB>& imageSrc, hr::gl::objects::Texture& textureDst)
	{
		if (imageSrc.empty())
			return false;

		textureDst.init(hr::gl::objects::Texture::Type::Tex2D, hr::gl::objects::Texture::StorageType::RGB_8, imageSrc.width(), imageSrc.height());

		size_t curLevel = 0;
		auto imageScaled = imageSrc.convert<uint8_t, hr::imaging::ImageFormatRGBA>(0, 255);

		while (true)
		{
			textureDst.uploadData(curLevel, 0, 0, imageScaled.width(), imageScaled.height(), hr::gl::objects::Texture::DataFormat::RGBA, hr::gl::objects::Texture::DataType::UBYTE, imageScaled.data());
			if (imageScaled.area() <= 1)
				break;

			curLevel++;
			imageScaled = imageScaled.resize(std::max<size_t>(1, imageScaled.width() >> 1), std::max<size_t>(1, imageScaled.height() >> 1), true);
		};

		return true;
	}

	bool TextureTools::uploadDiffuse(const hr::imaging::ImageView<float, hr::imaging::ImageFormatRGB>& imageSrc, hr::gl::objects::Texture& textureDst)
	{
		if (imageSrc.empty())
			return false;

		textureDst.init(hr::gl::objects::Texture::Type::Tex2D, hr::gl::objects::Texture::StorageType::RGB_16F, imageSrc.width(), imageSrc.height());

		size_t curLevel = 0;
		auto imageScaled = imageSrc.clone();

		while (true)
		{
			textureDst.uploadData(curLevel, 0, 0, imageScaled.width(), imageScaled.height(), hr::gl::objects::Texture::DataFormat::RGB, hr::gl::objects::Texture::DataType::FLOAT, imageScaled.data());
			if (imageScaled.area() <= 1)
				break;

			curLevel++;
			imageScaled = imageScaled.resize(std::max<size_t>(1, imageScaled.width() >> 1), std::max<size_t>(1, imageScaled.height() >> 1), false);
		};

		return true;
	}

	bool TextureTools::uploadCompressedDiffuse(hr::streams::StreamReader& streamIn, hr::gl::objects::Texture& textureDst)
	{
		CTextureHeader ctexHeader;
		if (streamIn.read(&ctexHeader, sizeof(CTextureHeader)) != sizeof(CTextureHeader))
			return false;
		if (std::memcmp(ctexHeader.fileSig, CTextureFileSig.data(), sizeof(ctexHeader.fileSig)) != 0)
			return false;

		hr::gl::objects::Texture::StorageType storageType;
		switch (ctexHeader.compression)
		{
		case 1: //BC1
			storageType = hr::gl::objects::Texture::StorageType::COMPRESSED_SRGB_BC1;
			break;
		case 6: //BC7
			storageType = hr::gl::objects::Texture::StorageType::COMPRESSED_SRGB_BC7;
			break;
		default:
			return false;
		}

		textureDst.init(hr::gl::objects::Texture::Type::Tex2D, storageType, ctexHeader.width, ctexHeader.height);

		std::unique_ptr<uint8_t[]> tmpCompressedData;
		for (size_t curLevel = 0; curLevel < ctexHeader.numLevels; ++curLevel)
		{
			CTextureLevelInfo ctexLevelInfo;
			if (streamIn.read(&ctexLevelInfo, sizeof(CTextureLevelInfo)) != sizeof(CTextureLevelInfo))
				return false;

			if (!tmpCompressedData)
				tmpCompressedData = std::unique_ptr<uint8_t[]>(new uint8_t[ctexLevelInfo.size]);

			if (streamIn.read(tmpCompressedData.get(), ctexLevelInfo.size) != ctexLevelInfo.size)
				return false;

			textureDst.uploadCompressedData(curLevel, 0, 0, ctexLevelInfo.width, ctexLevelInfo.height, storageType, ctexLevelInfo.size, tmpCompressedData.get());
		}

		return true;
	}

	bool TextureTools::storeCompressedDiffuse(hr::streams::StreamWriter& streamOut, const hr::imaging::ImageView<uint8_t, hr::imaging::ImageFormatRGB>& imageSrc)
	{
		if (imageSrc.empty())
			return false;

		auto imageRGBA = imageSrc.convert<uint8_t, hr::imaging::ImageFormatRGBA>(0, 255); //we have to work with RGBA
		return TextureTools::storeCompressedDiffuse(streamOut, imageRGBA);
	}

	bool TextureTools::storeCompressedDiffuse(hr::streams::StreamWriter& streamOut, const hr::imaging::ImageView<uint8_t, hr::imaging::ImageFormatRGBA>& imageSrc)
	{
		if (imageSrc.empty())
			return false;

		CTextureHeader ctexHeader;
		std::memcpy(ctexHeader.fileSig, CTextureFileSig.data(), CTextureFileSig.size());
		ctexHeader.version = 1;
		ctexHeader.width = imageSrc.width();
		ctexHeader.height = imageSrc.height();
		ctexHeader.compression = 6; // BC7
		ctexHeader.numLevels = static_cast<uint16_t>(hr::gl::objects::Texture::calculateNumMipMaps(ctexHeader.width, ctexHeader.height));
		streamOut.write(&ctexHeader, sizeof(CTextureHeader));

		uint16_t curLevel = 0;
		auto imageScaled = imageSrc.clone();

		while (true)
		{
			auto compressBlock = compressInitBlockBC7(imageScaled.width(), imageScaled.height());
			auto compressedImg = compressImageBC7(compressBlock, imageScaled);

			CTextureLevelInfo ctexLevelInfo;
			ctexLevelInfo.level = curLevel;
			ctexLevelInfo.width = imageScaled.width();
			ctexLevelInfo.height = imageScaled.height();
			ctexLevelInfo.size = compressBlock.width * compressBlock.height * compressBlock.perPixelBytes;
			streamOut.write(&ctexLevelInfo, sizeof(CTextureLevelInfo));

			streamOut.write(compressedImg.get(), ctexLevelInfo.size);

			if (imageScaled.area() <= 1)
				break;

			curLevel++;
			imageScaled = imageScaled.resize(std::max<size_t>(1, imageScaled.width() >> 1), std::max<size_t>(1, imageScaled.height() >> 1), true);
		};

		assert((curLevel + 1) == ctexHeader.numLevels);
		if ((curLevel + 1) != ctexHeader.numLevels)
			return false;

		return true;
	}

	bool TextureTools::uploadNormal(const hr::imaging::ImageView<uint8_t, hr::imaging::ImageFormatRGB>& imageSrc, hr::gl::objects::Texture& textureDst)
	{
		if (imageSrc.empty())
			return false;

		textureDst.init(hr::gl::objects::Texture::Type::Tex2D, hr::gl::objects::Texture::StorageType::RG_8, imageSrc.width(), imageSrc.height());

		size_t curLevel = 0;
		auto imageScaled = imageSrc.convert<float, hr::imaging::ImageFormatRGBA>(0, 255);

		while (true)
		{
			auto imageNormals = imageScaled.clone();
			imageNormals.renormalizeNormals(true);
			auto imageByte = imageNormals.convert<uint8_t, hr::imaging::ImageFormatRGBA>(0.0f, 1.0f);

			textureDst.uploadData(curLevel, 0, 0, imageByte.width(), imageByte.height(), hr::gl::objects::Texture::DataFormat::RGBA, hr::gl::objects::Texture::DataType::UBYTE, imageByte.data());
			if (imageByte.area() <= 1)
				break;

			curLevel++;
			imageScaled = imageScaled.resize(std::max<size_t>(1, imageScaled.width() >> 1), std::max<size_t>(1, imageScaled.height() >> 1), false);
		};

		return true;
	}

	bool TextureTools::uploadCompressedNormal(hr::streams::StreamReader& streamIn, hr::gl::objects::Texture& textureDst)
	{
		CTextureHeader ctexHeader;
		if (streamIn.read(&ctexHeader, sizeof(CTextureHeader)) != sizeof(CTextureHeader))
			return false;
		if (std::memcmp(ctexHeader.fileSig, CTextureFileSig.data(), sizeof(ctexHeader.fileSig)) != 0)
			return false;

		if (ctexHeader.compression != 5) //BC5
			return false;

		textureDst.init(hr::gl::objects::Texture::Type::Tex2D, hr::gl::objects::Texture::StorageType::COMPRESSED_BC5, ctexHeader.width, ctexHeader.height);

		std::unique_ptr<uint8_t[]> tmpCompressedData;
		for (size_t curLevel = 0; curLevel < ctexHeader.numLevels; ++curLevel)
		{
			CTextureLevelInfo ctexLevelInfo;
			if (streamIn.read(&ctexLevelInfo, sizeof(CTextureLevelInfo)) != sizeof(CTextureLevelInfo))
				return false;

			if (!tmpCompressedData)
				tmpCompressedData = std::unique_ptr<uint8_t[]>(new uint8_t[ctexLevelInfo.size]);

			if (streamIn.read(tmpCompressedData.get(), ctexLevelInfo.size) != ctexLevelInfo.size)
				return false;

			textureDst.uploadCompressedData(curLevel, 0, 0, ctexLevelInfo.width, ctexLevelInfo.height, hr::gl::objects::Texture::StorageType::COMPRESSED_BC5, ctexLevelInfo.size, tmpCompressedData.get());
		}

		return true;
	}

	bool TextureTools::storeCompressedNormal(hr::streams::StreamWriter& streamOut, const hr::imaging::ImageView<uint8_t, hr::imaging::ImageFormatRGB>& imageSrc)
	{
		if (imageSrc.empty())
			return false;

		auto imageRGBA = imageSrc.convert<uint8_t, hr::imaging::ImageFormatRGBA>(0, 255); //we have to work with RGBA
		return TextureTools::storeCompressedNormal(streamOut, imageRGBA);
	}

	bool TextureTools::storeCompressedNormal(hr::streams::StreamWriter& streamOut, const hr::imaging::ImageView<uint8_t, hr::imaging::ImageFormatRGBA>& imageSrc)
	{
		if (imageSrc.empty())
			return false;

		CTextureHeader ctexHeader;
		std::memcpy(ctexHeader.fileSig, CTextureFileSig.data(), CTextureFileSig.size());
		ctexHeader.version = 1;
		ctexHeader.width = imageSrc.width();
		ctexHeader.height = imageSrc.height();
		ctexHeader.compression = 5; // BC5
		ctexHeader.numLevels = static_cast<uint16_t>(hr::gl::objects::Texture::calculateNumMipMaps(ctexHeader.width, ctexHeader.height));
		streamOut.write(&ctexHeader, sizeof(CTextureHeader));

		uint16_t curLevel = 0;

		if (imageSrc.area() > (4096 * 4096)) //above this we have memory limitations because of converting to float
		{
			hr::imaging::Image<uint8_t, hr::imaging::ImageFormatRGBA> imageScaled;
			while (true)
			{
				auto imageNormals = imageScaled.empty() ? imageSrc.clone() : imageScaled.clone();
				imageNormals.renormalizeNormals(true);

				auto curWidth = imageScaled.empty() ? imageSrc.width() : imageScaled.width();
				auto curHeight = imageScaled.empty() ? imageSrc.height() : imageScaled.height();

				auto compressBlock = compressInitBlockBC5(curWidth, curHeight);
				auto compressedImg = compressImageBC5(compressBlock, imageNormals);

				CTextureLevelInfo ctexLevelInfo;
				ctexLevelInfo.level = curLevel;
				ctexLevelInfo.width = curWidth;
				ctexLevelInfo.height = curHeight;
				ctexLevelInfo.size = compressBlock.width * compressBlock.height * compressBlock.perPixelBytes;
				streamOut.write(&ctexLevelInfo, sizeof(CTextureLevelInfo));

				streamOut.write(compressedImg.get(), ctexLevelInfo.size);

				if ((curWidth * curHeight) <= 1)
					break;

				curLevel++;
				if (imageScaled.empty())
					imageScaled = imageSrc.resize(std::max<size_t>(1, imageSrc.width() >> 1), std::max<size_t>(1, imageSrc.height() >> 1), true);
				else
					imageScaled = imageScaled.resize(std::max<size_t>(1, imageScaled.width() >> 1), std::max<size_t>(1, imageScaled.height() >> 1), true);
			};
		}
		else
		{
			auto imageScaled = imageSrc.convert<float, hr::imaging::ImageFormatRGBA>(0, 255);
			while (true)
			{
				auto imageNormals = imageScaled.clone();
				imageNormals.renormalizeNormals(true);
				auto imageByte = imageNormals.convert<uint8_t, hr::imaging::ImageFormatRGBA>(0.0f, 1.0f);

				auto compressBlock = compressInitBlockBC5(imageScaled.width(), imageScaled.height());
				auto compressedImg = compressImageBC5(compressBlock, imageByte);

				CTextureLevelInfo ctexLevelInfo;
				ctexLevelInfo.level = curLevel;
				ctexLevelInfo.width = imageScaled.width();
				ctexLevelInfo.height = imageScaled.height();
				ctexLevelInfo.size = compressBlock.width * compressBlock.height * compressBlock.perPixelBytes;
				streamOut.write(&ctexLevelInfo, sizeof(CTextureLevelInfo));

				streamOut.write(compressedImg.get(), ctexLevelInfo.size);

				if (imageScaled.area() <= 1)
					break;

				curLevel++;
				imageScaled = imageScaled.resize(std::max<size_t>(1, imageScaled.width() >> 1), std::max<size_t>(1, imageScaled.height() >> 1), false);
			};
		}

		assert((curLevel + 1) == ctexHeader.numLevels);
		if ((curLevel + 1) != ctexHeader.numLevels)
			return false;

		return true;
	}
}