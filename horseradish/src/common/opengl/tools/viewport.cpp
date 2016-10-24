#include "viewport.hpp"

namespace HorseRadish { namespace OpenGL { namespace Tools {

HorseRadish::Matrix Viewport::genMatrix2DProj(size_t width, size_t height)
{
	Viewport viewport(90.0f, width, height);
	return viewport.mMatrices.mp2D;
}

void Viewport::calcMatrices()
{
	double n = static_cast<double>(mZNear);
	double f = static_cast<double>(mZFar);
	double ymax = n * tan(static_cast<double>(mFov)* 0.00872664625997164788461845384);
	double ymin = -ymax;
	double aspect = static_cast<double>(mWidth) / static_cast<double>(mHeight);
	double xmin = ymin * aspect;
	double xmax = ymax * aspect;

	//3D
	mMatrices.mp3D.setZero();
	mMatrices.mp3D[0] = static_cast<float>((2.0 * n) / (xmax - xmin));
	mMatrices.mp3D[5] = static_cast<float>((2.0 * n) / (ymax - ymin));
	mMatrices.mp3D[8] = static_cast<float>((xmax + xmin) / (xmax - xmin));
	mMatrices.mp3D[9] = static_cast<float>((ymax + ymin) / (ymax - ymin));
	mMatrices.mp3D[10] = -static_cast<float>((f + n) / (f - n));
	mMatrices.mp3D[11] = -1.0f;
	mMatrices.mp3D[14] = -static_cast<float>((2.0 * f *n) / (f - n));

	//3D infinite
	mMatrices.mp3DInfinite.setZero();
	mMatrices.mp3DInfinite[0] = static_cast<float>((2.0 * n) / (xmax - xmin));
	mMatrices.mp3DInfinite[5] = static_cast<float>((2.0 * n) / (ymax - ymin));
	mMatrices.mp3DInfinite[8] = static_cast<float>((xmax + xmin) / (xmax - xmin));
	mMatrices.mp3DInfinite[9] = static_cast<float>((ymax + ymin) / (ymax - ymin));
	mMatrices.mp3DInfinite[10] = -1.0f;
	mMatrices.mp3DInfinite[11] = -1.0f;
	mMatrices.mp3DInfinite[14] = -static_cast<float>(2.0 * n);

	//2D
	mMatrices.mp2D.setIdentity();
	mMatrices.mp2D[0] = 2.0f / static_cast<float>(mWidth);
	mMatrices.mp2D[5] = 2.0f / static_cast<float>(mHeight);
	mMatrices.mp2D[10] = -1.0f;
	mMatrices.mp2D[12] = -1.0f;
	mMatrices.mp2D[13] = -1.0f;
}

const HorseRadish::Matrix& Viewport::getProjection(ProjectionType projectionType) const
{
	switch (projectionType)
	{
	case ProjectionType::Proj2D:
		return mMatrices.mp2D;
	case ProjectionType::Proj3DInf:
		return mMatrices.mp3DInfinite;
	};

	return mMatrices.mp3D;
}

void Viewport::pointOnZNear(HorseRadish::Vector3f& center) const
{
	center[0] = mZNear * tan(mFov * 0.5f);
	center[1] = (center[1]) * static_cast<float>(mWidth) / static_cast<float>(mHeight);
	center[2] = mZNear;
}

void Viewport::projectPoint(ProjectionType projType, const HorseRadish::Matrix& modelView, HorseRadish::Vector3f * const listPoints, size_t numPoints) const
{
	if (listPoints == nullptr || numPoints <= 0)
		return;

	HorseRadish::Matrix transMat;
	switch (projType)
	{
		case ProjectionType::Proj2D:
			transMat = mMatrices.mp2D;
			break;
		case ProjectionType::Proj3D:
			transMat = mMatrices.mp3D;
			break;
		case ProjectionType::Proj3DInf:
			transMat = mMatrices.mp3DInfinite;
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
		HorseRadish::Vector4f result(listPoints[i], 1.0f);
		transMat.transform(result);

		float rhw = 1.0f / result[3];

		float projX = (1.0f + result[0] * rhw) * winX * 0.5f;
		float projY = winY - ((1.0f - result[1] * rhw) * winY * 0.5f);
		float projZ = (result[2] * rhw) * (depthRange[1] - depthRange[0]) + depthRange[0];

		listPoints[i].set(projX, projY, projZ);
	}
}

} } }