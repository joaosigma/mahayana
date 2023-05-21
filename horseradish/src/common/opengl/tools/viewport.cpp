#include "viewport.hpp"

namespace hr::gl::tools
{
	namespace
	{
		/******
		* NOTE:
		*	The perspective projection matrices produce a left-hand coordinate system, which means:
		*		- plus X points right, plus Y points up and plus Z points forward (to the horizon)
		*
		*	The orthographic projection matrices uses the same orientation as Vulkan: (0, 0) is the top-left corner.
		*/
		hr::Matrix4f funcProjection(double fovy, double aspectRatio, double znear, double zfar, hr::Matrix4f* inverse) noexcept
		{
			//this calculates a reverse-Z projection for Vulkan
			
			double f = 1.0 / std::tan(fovy * 0.5);
			double x = f / aspectRatio;
			double y = -f;
			double A = znear / (zfar - znear);
			double B = f * A;

			auto mat = hr::Matrix4f::zero();
			mat[0] = static_cast<float>(x);
			mat[5] = static_cast<float>(y);
			mat[10] = static_cast<float>(A);
			mat[11] = static_cast<float>(B);
			mat[14] = -1.0;

			if (inverse)
			{
				auto matInverse = hr::Matrix4f::zero();
				matInverse[0] = static_cast<float>(1.0 / x);
				matInverse[5] = static_cast<float>(1.0 / y);
				matInverse[11] = static_cast<float>(-1.0);
				matInverse[14] = static_cast<float>(1.0 / B);
				matInverse[15] = A / B;

				*inverse = matInverse;
			}

			mat.transpose();
			return mat;
		};

		hr::Matrix4f funcProjection(double fovy, double aspectRatio, double znear) noexcept
		{
			//this calculates a reverse-Z projection, with an infinite Z far, for Vulkan

			double f = 1.0 / std::tan(fovy * 0.5);

			auto mat = hr::Matrix4f::zero();
			mat[0] = static_cast<float>(f / aspectRatio);
			mat[5] = static_cast<float>(-f);
			mat[11] = static_cast<float>(znear);
			mat[14] = 1.0f;

			mat.transpose();
			return mat;
		};

		hr::Matrix4f funcOrtho(double left, double right, double bottom, double top, double near, double far) noexcept
		{
			auto mat = hr::Matrix4f::zero();

			mat[0] = static_cast<float>(2.0 / (right - left));
			mat[5] = static_cast<float>(2.0 / (top - bottom));
			mat[10] = static_cast<float>(-2.0 / (far - near));
			mat[12] = static_cast<float>(-(right + left) / (right - left));
			mat[13] = static_cast<float>(-(top + bottom) / (top - bottom));
			mat[14] = static_cast<float>(-(far + near) / (far - near));
			mat[15] = 1.0f;

			return mat;
		};
	}

	hr::Matrix4f Viewport::genMatrix2DProj(size_t width, size_t height)
	{
		return funcOrtho(0.0, width, 0.0, height, 1.0, -1.0);
	}

	void Viewport::calcMatrices() noexcept
	{
		mMatrices.mp3D = funcProjection(mYFov, mDims.aspectRatio, mZNear);
		mMatrices.mp2D = funcOrtho(0.0, mDims.width, 0.0, mDims.height, 1.0, -1.0);
	}

	const hr::Matrix4f& Viewport::getProjection(ProjectionType projectionType) const
	{
		switch (projectionType)
		{
		case ProjectionType::Proj2D:
			return mMatrices.mp2D;
		default:
			break;
		};

		return mMatrices.mp3D;
	}

	void Viewport::projectPoint(ProjectionType projType, const hr::Matrix4f& modelView, hr::Vector3f* const listPoints, size_t numPoints) const
	{
		if (!listPoints || numPoints <= 0)
			return;

		auto transMat = hr::Matrix4f::identity();
		switch (projType)
		{
		case ProjectionType::Proj2D:
			transMat = mMatrices.mp2D;
			break;
		case ProjectionType::Proj3D:
			transMat = mMatrices.mp3D;
			break;
		default:
			return;
		}
		transMat *= modelView;

		auto winX = static_cast<float>(mDims.width);
		auto winY = static_cast<float>(mDims.height);

		float depthRange[2] = { 0.0f, 1.0f }; //unless changed with glDepthRange

		for (size_t i = 0; i < numPoints; i++)
		{
			auto result = listPoints->convert<float, 4>(1.0f);
			transMat.transform(result);

			float rhw = 1.0f / result[3];

			float projX = (1.0f + result[0] * rhw) * winX * 0.5f;
			float projY = winY - ((1.0f - result[1] * rhw) * winY * 0.5f);
			float projZ = (result[2] * rhw) * (depthRange[1] - depthRange[0]) + depthRange[0];

			listPoints[i] = Vector3f{projX, projY, projZ};
		}
	}
}
