#include "viewport.hpp"

namespace hr::gl::tools
{
	namespace
	{
		hr::Matrix4f funcProjection(double fovy, double aspectRatio, double znear) noexcept
		{
			/*double ymax = near * std::tan(fov * 0.00872664625997164788461845384); //0.008726646259971 = pi / 180.0 / 2.0
			double ymin = -ymax;
			double xmin = ymin * aspectRatio;
			double xmax = ymax * aspectRatio;

			mat.set(0.0f);
			mat[0] = static_cast<float>((2.0 * near) / (xmax - xmin));
			mat[5] = static_cast<float>((2.0 * near) / (ymax - ymin));
			mat[8] = static_cast<float>((xmax + xmin) / (xmax - xmin));
			mat[9] = static_cast<float>((ymax + ymin) / (ymax - ymin));
			mat[10] = -static_cast<float>((far + near) / (far - near));
			mat[11] = -1.0f;
			mat[14] = -static_cast<float>((2.0 * far * near) / (far - near));*/

			auto mat = hr::Matrix4f::zero();

			double f = 1.0 / std::tan(fovy / 2.0);
			mat[0] = static_cast<float>(f / aspectRatio);
			mat[5] = static_cast<float>(f);
			mat[11] = -1.0f;
			mat[14] = static_cast<float>(znear);

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
			hr::Vector4f result(listPoints[i], 1.0f);
			transMat.transform(result);

			float rhw = 1.0f / result[3];

			float projX = (1.0f + result[0] * rhw) * winX * 0.5f;
			float projY = winY - ((1.0f - result[1] * rhw) * winY * 0.5f);
			float projZ = (result[2] * rhw) * (depthRange[1] - depthRange[0]) + depthRange[0];

			listPoints[i].set(projX, projY, projZ);
		}
	}
}
