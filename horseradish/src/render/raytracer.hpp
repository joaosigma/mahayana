#pragma once

#include "tools/camera.hpp"
#include "../common/tasks.hpp"
#include "../common/image.hpp"
#include "../common/opengl/tools/viewport.hpp"

#include <random>

namespace hr::render
{
	class Raytracer
	{
		std::mt19937 mRandGen;
		Dispatcher& mAsyncDispatcher;
		imaging::Image<float, imaging::ImageFormatRGB> mBuffer;
	
	private:	
		Vector3d randomInHemisphere(const Vector3d& normal);

		Colord rayColor(const Ray<Vector3d>& r, size_t depth);

	public:
		Raytracer(Dispatcher& asyncDispatcher, size_t maxWidth, size_t maxHeight);

		bool trace(const tools::Camera& camera, const gl::tools::Viewport& viewport);

		const imaging::ImageView<float, imaging::ImageFormatRGB>& buffer() const
		{
			return mBuffer;
		}
	};
}
