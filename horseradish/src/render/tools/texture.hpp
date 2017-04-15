#pragma once

#include "common/image.hpp"
#include "common/openGL/objects.hpp"

#include "common/stream.hpp"

namespace hr { namespace render { namespace tools
{
	class TextureTools final
	{
	public:
		//diffuse

		static bool uploadDiffuse(const hr::imaging::ImageView<unsigned char, hr::imaging::ImageFormatRGBA>& imageSrc, hr::gl::objects::Texture& textureDst);
		static bool uploadDiffuse(const hr::imaging::ImageView<unsigned char, hr::imaging::ImageFormatRGB>& imageSrc, hr::gl::objects::Texture& textureDst);

		static bool uploadCompressedDiffuse(hr::streams::StreamReader& streamIn, hr::gl::objects::Texture& textureDst);

		static bool storeCompressedDiffuse(hr::streams::StreamWriter& streamOut, const hr::imaging::ImageView<unsigned char, hr::imaging::ImageFormatRGB>& imageSrc);
		static bool storeCompressedDiffuse(hr::streams::StreamWriter& streamOut, const hr::imaging::ImageView<unsigned char, hr::imaging::ImageFormatRGBA>& imageSrc);

		//normals

		static bool uploadNormal(const hr::imaging::ImageView<unsigned char, hr::imaging::ImageFormatRGB>& imageSrc, hr::gl::objects::Texture& textureDst);

		static bool uploadCompressedNormal(hr::streams::StreamReader& streamIn, hr::gl::objects::Texture& textureDst);

		static bool storeCompressedNormal(hr::streams::StreamWriter& streamOut, const hr::imaging::ImageView<unsigned char, hr::imaging::ImageFormatRGB>& imageSrc);
		static bool storeCompressedNormal(hr::streams::StreamWriter& streamOut, const hr::imaging::ImageView<unsigned char, hr::imaging::ImageFormatRGBA>& imageSrc);
	};

} } }
