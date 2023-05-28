#include "utils.hpp"

#include "../imageFactory.hpp"
#include "../fileSystem.hpp"

#include <cassert>

namespace hr::vulkan
{
	namespace
	{
		bool hasStencil(VkFormat format)
		{
			return ((format == VK_FORMAT_D32_SFLOAT_S8_UINT) || (format == VK_FORMAT_D24_UNORM_S8_UINT));
		}
	}

	Texture Texture::gen2D(App& app, CommandPool& cmdPool, VkFormat format, const hr::io::FileSystem& fileSystem, std::string_view filePath)
	{
		//read image and create a staging memory and buffer to transfer the pixels
		vulkan::Memory stagingMemory;
		vulkan::Buffer stagingBuffer;
		hr::imaging::Image<uint8_t, hr::imaging::ImageFormatRGBA> imgData;
		{
			{
				auto fstream = fileSystem.fileRead(filePath);
				hr::streams::StreamReader reader(*fstream);
				auto img = hr::imaging::Factory::readJPG(reader);
				imgData = img.convert<uint8_t, hr::imaging::ImageFormatRGBA>(0, 255);
			}

			stagingBuffer = vulkan::Buffer::gen(app.device(), imgData.size(), VK_BUFFER_USAGE_TRANSFER_SRC_BIT);
			stagingMemory = app.allocateMemory(stagingBuffer.hostRequiredSize(), stagingBuffer.hostRequiredMemoryType(),
			  VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

			stagingBuffer.allocate(stagingMemory);
			stagingMemory.write(std::as_bytes(imgData.asSpan()), 0);
		}

		auto mipLevels = Image::calculateNumMipMaps(imgData.width(), imgData.height());
		
		//the new texture
		Texture newTexture;
		newTexture.mInfo.width = imgData.width();
		newTexture.mInfo.height = imgData.height();
		newTexture.mInfo.depth = 1;
		newTexture.mImage = vulkan::Image::gen2D(app.device(), newTexture.mInfo.width, newTexture.mInfo.height, true, format);
		newTexture.mMemory = app.allocateMemory(newTexture.mImage.hostRequiredSize(), newTexture.mImage.hostRequiredMemoryType(), VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
		newTexture.mImage.allocate(newTexture.mMemory);

		app.executeOneTimeCommand(cmdPool,
		  [&newTexture, &stagingBuffer, mipLevels](vulkan::CommandBuffer::Recorder& recorder)
		  {
			  //transition to VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL

			  VkImageMemoryBarrier barrier{};
			  barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
			  barrier.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
			  barrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
			  barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
			  barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
			  barrier.image = newTexture.mImage.native();
			  barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
			  barrier.subresourceRange.baseMipLevel = 0;
			  barrier.subresourceRange.levelCount = mipLevels;
			  barrier.subresourceRange.baseArrayLayer = 0;
			  barrier.subresourceRange.layerCount = 1;
			  barrier.srcAccessMask = 0;
			  barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

			  recorder.pipelineBarrier(VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, barrier);
		  });

		//actual transfers - level 0
		app.executeOneTimeCommand(cmdPool,
		  [&newTexture, &stagingBuffer, mipLevels](vulkan::CommandBuffer::Recorder& recorder)
		  {
			  recorder.copyBufferToImage(newTexture.mImage.native(), stagingBuffer.native(), newTexture.mInfo.width, newTexture.mInfo.height, 0);
		  });

		//remaining levels
		imgData.iterateMipmaps(false,
		  [&stagingMemory, &app, &cmdPool, &newTexture, &stagingBuffer, mipLevels](size_t mipLevel, const hr::imaging::Image<uint8_t, hr::imaging::ImageFormatRGBA>& mipmap)
		  {
			  stagingMemory.write(std::as_bytes(mipmap.asSpan()), 0);

			  app.executeOneTimeCommand(cmdPool,
			    [&mipmap, image = newTexture.mImage.native(), & stagingBuffer, mipLevel](vulkan::CommandBuffer::Recorder& recorder)
			    {
				    recorder.copyBufferToImage(image, stagingBuffer.native(), mipmap.width(), mipmap.height(), mipLevel);
			    });
		  });

		app.executeOneTimeCommand(cmdPool,
		  [&newTexture, &stagingBuffer, mipLevels](vulkan::CommandBuffer::Recorder& recorder)
		  {
			  //transition to VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL

			  VkImageMemoryBarrier barrier{};
			  barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
			  barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
			  barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
			  barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
			  barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
			  barrier.image = newTexture.mImage.native();
			  barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
			  barrier.subresourceRange.baseMipLevel = 0;
			  barrier.subresourceRange.levelCount = mipLevels;
			  barrier.subresourceRange.baseArrayLayer = 0;
			  barrier.subresourceRange.layerCount = 1;
			  barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
			  barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

			  recorder.pipelineBarrier(VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, barrier);
		  });

		//and finally, the image view for the image
		newTexture.mImageView = vulkan::ImageView::gen2D(app.device(), newTexture.mImage.native(), mipLevels, format);

		return newTexture;
	}

	Texture Texture::genDepth(App& app, CommandPool& cmdPool, VkFormat format, size_t width, size_t height)
	{
		Texture newTexture;

		newTexture.mInfo.width = width;
		newTexture.mInfo.height = height;
		newTexture.mInfo.depth = 1;

		newTexture.mImage = vulkan::Image::genDepth(app.device(), newTexture.mInfo.width, newTexture.mInfo.height, format);
		newTexture.mMemory = app.allocateMemory(newTexture.mImage.hostRequiredSize(), newTexture.mImage.hostRequiredMemoryType(), VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
		newTexture.mImage.allocate(newTexture.mMemory);

		app.executeOneTimeCommand(cmdPool, [&newTexture, format](vulkan::CommandBuffer::Recorder& recorder)
		{
			//transition to VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL

			VkImageMemoryBarrier barrier{};
			barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
			barrier.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
			barrier.newLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
			barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
			barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
			barrier.image = newTexture.mImage.native();
			barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT | (hasStencil(format) ? VK_IMAGE_ASPECT_STENCIL_BIT : 0);
			barrier.subresourceRange.baseMipLevel = 0;
			barrier.subresourceRange.levelCount = 1;
			barrier.subresourceRange.baseArrayLayer = 0;
			barrier.subresourceRange.layerCount = 1;
			barrier.srcAccessMask = 0;
			barrier.dstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT | (hasStencil(format) ? VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT : 0);

			recorder.pipelineBarrier(VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT, barrier);
		});

		newTexture.mImageView = vulkan::ImageView::genDepth(app.device(), newTexture.mImage.native(), format);

		return newTexture;
	}
}