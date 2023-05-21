#pragma once

#include "objects.hpp"
#include "app.hpp"

#include <string_view>

namespace hr::io
{
	class FileSystem;
}

namespace hr::vulkan
{
	class Texture
	{
		Image mImage;
		ImageView mImageView;
		Memory mMemory;
		struct
		{
			size_t width{0}, height{0}, depth{0};
		} mInfo;

	public:
		static Texture gen2D(App& app, CommandPool& cmdPool, VkFormat format, const hr::io::FileSystem& fileSystem, std::string_view filePath);
		static Texture genDepth(App& app, CommandPool& cmdPool, VkFormat format, size_t width, size_t height);

	private:
		Texture() noexcept = default;

	public:
		~Texture() noexcept = default;

		Texture(const Texture&) = delete;
		Texture& operator=(const Texture&) = delete;
		Texture(Texture&&) noexcept = default;
		Texture& operator=(Texture&&) noexcept = default;

		explicit operator bool() const noexcept
		{
			return (mInfo.width > 0) && (mInfo.height > 0) && (mInfo.depth > 0);
		}

		bool isValid() const
		{
			return operator bool();
		}

		size_t width() const noexcept
		{
			return mInfo.width;
		}

		size_t height() const noexcept
		{
			return mInfo.height;
		}

		size_t depth() const noexcept
		{
			return mInfo.depth;
		}

		const ImageView& imageView() const noexcept
		{
			return mImageView;
		}

		ImageView& imageView() noexcept
		{
			return mImageView;
		}
	};
}
