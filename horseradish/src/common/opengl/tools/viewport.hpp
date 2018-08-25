#pragma once

#include "common/vector.hpp"
#include "common/matrix.hpp"

namespace hr { namespace gl { namespace tools
{		
	class Viewport
	{
	public:
		enum class ProjectionType { Proj3D, Proj2D };

		static hr::Matrix genMatrix2DProj(size_t width, size_t height);

	private:
		float mFovY, mZNear;
		size_t mWidth, mHeight;
		struct {
			hr::Matrix mp2D, mp3D;
		} mMatrices;

		void calcMatrices();

	public:
		Viewport() = delete;

		explicit Viewport(const size_t width, const size_t height)
			: Viewport(90.0f, width, height)
		{ }

		explicit Viewport(const float fovY, const size_t width, const size_t height)
			: Viewport(fovY, width, height, 0.1f)
		{ }

		explicit Viewport(const float fovY, const size_t width, const size_t height, const float zNear)
			: mFovY(fovY), mZNear(zNear), mWidth(width ? width : 1), mHeight(height ? height : 1)
		{
			calcMatrices();
		}

		const hr::Matrix& getProjection(ProjectionType projectionType) const;
		
		float fovY() const { return mFovY; }
		float znear() const { return mZNear; }

		size_t width() const { return mWidth; }
		size_t height() const { return mHeight; }

		void pointOnZNear(hr::Vector3f& center) const;

		void projectPoint(ProjectionType projType, const hr::Matrix& modelView, hr::Vector3f * const listPoints, size_t numPoints) const;
	};
} } }
