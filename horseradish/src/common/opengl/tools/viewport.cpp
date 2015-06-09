#include "Viewport.hpp"

namespace HorseRadish { namespace OpenGL { namespace Tools {

HorseRadish::Matrix Viewport::genMatrix2DProj(const int width, const int height)
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
	mMatrices.mp3D.SetZero();
	mMatrices.mp3D[0] = static_cast<float>((2.0 * n) / (xmax - xmin));
	mMatrices.mp3D[5] = static_cast<float>((2.0 * n) / (ymax - ymin));
	mMatrices.mp3D[8] = static_cast<float>((xmax + xmin) / (xmax - xmin));
	mMatrices.mp3D[9] = static_cast<float>((ymax + ymin) / (ymax - ymin));
	mMatrices.mp3D[10] = -static_cast<float>((f + n) / (f - n));
	mMatrices.mp3D[11] = -1.0f;
	mMatrices.mp3D[14] = -static_cast<float>((2.0 * f *n) / (f - n));

	//3D infinite
	mMatrices.mp3DInfinite.SetZero();
	mMatrices.mp3DInfinite[0] = static_cast<float>((2.0 * n) / (xmax - xmin));
	mMatrices.mp3DInfinite[5] = static_cast<float>((2.0 * n) / (ymax - ymin));
	mMatrices.mp3DInfinite[8] = static_cast<float>((xmax + xmin) / (xmax - xmin));
	mMatrices.mp3DInfinite[9] = static_cast<float>((ymax + ymin) / (ymax - ymin));
	mMatrices.mp3DInfinite[10] = -1.0f;
	mMatrices.mp3DInfinite[11] = -1.0f;
	mMatrices.mp3DInfinite[14] = -static_cast<float>(2.0 * n);

	//2D
	mMatrices.mp2D.SetIdentidade();
	mMatrices.mp2D[0] = 2.0f / static_cast<float>(mWidth);
	mMatrices.mp2D[5] = 2.0f / static_cast<float>(mHeight);
	mMatrices.mp2D[10] = -1.0f;
	mMatrices.mp2D[12] = -1.0f;
	mMatrices.mp2D[13] = -1.0f;
}

Viewport::Viewport(const unsigned int width, const unsigned int height)
	: Viewport(90.0f, width, height)
{
}

Viewport::Viewport(const float fov, const unsigned int width, const unsigned int height)
	: Viewport(fov, width, height, 1.0f, 1000.0f)
{
}

Viewport::Viewport(const float fov, const unsigned int width, const unsigned int height, const float zNear, const float zFar)
	: mFov(fov), mZNear(zNear), mZFar(zFar), mWidth(width ? width : 1), mHeight(height ? height : 1)
{
	calcMatrices();
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

void Viewport::getPointOnZNear(float * const center) const
{
	center[1] = mZNear * tan(mFov * 0.5f);
	center[0] = (center[1]) * static_cast<float>(mWidth) / static_cast<float>(mHeight);
	center[2] = mZNear;
}

void Viewport::getPointOnZNear(HorseRadish::Vector& center) const
{
	center.y = mZNear * tan(mFov * 0.5f);
	center.x = (center.y) * static_cast<float>(mWidth) / static_cast<float>(mHeight);
	center.z = mZNear;
}

void Viewport::projectPoint(ProjectionType projType, const HorseRadish::Matrix& modelView, HorseRadish::Vector * const listPoints, const int numPoints) const
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

	float winX = static_cast<float>(mWidth);
	float winY = static_cast<float>(mHeight);

	float depthRange[2] = {0.0f, 1.0f}; //unless changeed with glDepthRange

	for (int i = 0; i < numPoints; i++)
	{
		HorseRadish::Vector4 result(listPoints[i], 1.0f);
		transMat.TransformVector(result);

		float rhw = 1.0f / result.w;

		float projX = (1.0f + result.x * rhw) * winX * 0.5f;
		float projY = winY - ((1.0f - result.y * rhw) * winY * 0.5f);
		float projZ = (result.z * rhw) * (depthRange[1] - depthRange[0]) + depthRange[0];

		listPoints[i].Set(projX, projY, projZ);
	}
}

} } }