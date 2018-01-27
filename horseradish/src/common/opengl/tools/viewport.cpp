#include "viewport.hpp"

namespace hr { namespace gl { namespace tools
{
	namespace
	{
		void funcProjection(hr::Matrix& mat, double fov, double aspectRatio, double near, double far)
		{
			double ymax = near * tan(fov * 0.00872664625997164788461845384); //0.008726646259971 = pi / 180.0 / 2.0
			double ymin = -ymax;
			double xmin = ymin * aspectRatio;
			double xmax = ymax * aspectRatio;

			mat.setZero();
			mat[0] = static_cast<float>((2.0 * near) / (xmax - xmin));
			mat[5] = static_cast<float>((2.0 * near) / (ymax - ymin));
			mat[8] = static_cast<float>((xmax + xmin) / (xmax - xmin));
			mat[9] = static_cast<float>((ymax + ymin) / (ymax - ymin));
			mat[10] = -static_cast<float>((far + near) / (far - near));
			mat[11] = -1.0f;
			mat[14] = -static_cast<float>((2.0 * far * near) / (far - near));
		};

		void funcOrtho(hr::Matrix& mat, double left, double right, double bottom, double top, double near, double far)
		{
			mat.setZero();
			mat[0] = static_cast<float>(2.0 / (right - left));
			mat[5] = static_cast<float>(2.0 / (top - bottom));
			mat[10] = static_cast<float>(-2.0 / (far - near));
			mat[12] = static_cast<float>(-(right + left) / (right - left));
			mat[13] = static_cast<float>(-(top + bottom) / (top - bottom));
			mat[14] = static_cast<float>(-(far + near) / (far - near));
			mat[15] = 1.0f;
		};
	}

	hr::Matrix Viewport::genMatrix2DProj(size_t width, size_t height)
	{
		hr::Matrix mat;
		funcOrtho(mat, 0.0, width, 0.0, height, 1.0, -1.0);
		return mat;
	}

	void Viewport::calcMatrices()
	{
		funcProjection(mMatrices.mp3D, mFov, static_cast<double>(mWidth) / static_cast<double>(mHeight), mZNear, mZFar);
		funcOrtho(mMatrices.mp2D, 0.0, mWidth, 0.0, mHeight, 1.0, -1.0);
	}

	const hr::Matrix& Viewport::getProjection(ProjectionType projectionType) const
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

	void Viewport::pointOnZNear(hr::Vector3f& center) const
	{
		center[0] = mZNear * tan(mFov * 0.5f);
		center[1] = (center[1]) * static_cast<float>(mWidth) / static_cast<float>(mHeight);
		center[2] = mZNear;
	}

	void Viewport::projectPoint(ProjectionType projType, const hr::Matrix& modelView, hr::Vector3f * const listPoints, size_t numPoints) const
	{
		if (!listPoints || numPoints <= 0)
			return;

		hr::Matrix transMat;
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

		auto winX = static_cast<float>(mWidth);
		auto winY = static_cast<float>(mHeight);

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
} } }
