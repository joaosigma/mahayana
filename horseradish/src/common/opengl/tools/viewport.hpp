#pragma once

#include "common/vector.hpp"
#include "common/matrix.hpp"

namespace hr::gl::tools
{
	class Viewport
	{
	public:
		enum class ProjectionType { Proj3D, Proj2D };

		static hr::Matrix genMatrix2DProj(size_t width, size_t height);

	private:
		double mYFov, mZNear;
		struct {
			double aspectRatio;
			size_t width, height;
		} mDims;
		
		struct {
			hr::Matrix mp2D = hr::Matrix::identity();
			hr::Matrix mp3D = hr::Matrix::identity();
		} mMatrices;

		void calcMatrices()  noexcept;

	public:
		Viewport() = delete;

		explicit Viewport(const size_t width, const size_t height) noexcept
			: Viewport(Math::Deg2Rad<double> * 90.0, width, height)
		{ }

		explicit Viewport(const double yfov, const size_t width, const size_t height) noexcept
			: Viewport(yfov, width, height, 0.1)
		{ }

		explicit Viewport(const double yfov, const size_t width, const size_t height, const double zNear) noexcept
			: mYFov(yfov), mZNear(zNear)
		{
			mDims.width = width ? width : 1;
			mDims.height = height ? height : 1;
			mDims.aspectRatio = (static_cast<double>(mDims.width) / static_cast<double>(mDims.height));
			calcMatrices();
		}

		const hr::Matrix& getProjection(ProjectionType projectionType) const;

		double yfov() const { return mYFov; }
		double znear() const { return mZNear; }

		size_t width() const { return mDims.width; }
		size_t height() const { return mDims.height; }
		double aspectRatio() const { return mDims.aspectRatio; }

		void projectPoint(ProjectionType projType, const hr::Matrix& modelView, hr::Vector3f* const listPoints, size_t numPoints) const;
	};
}
