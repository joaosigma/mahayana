#pragma once

#include "common\Vector.hpp"
#include "common\Matrix.hpp"

namespace HorseRadish { namespace OpenGL { namespace Tools {
			
class Viewport
{
public:
	enum class ProjectionType { Proj3D, Proj3DInf, Proj2D };

	static HorseRadish::Matrix genMatrix2DProj(size_t width, size_t height);

private:
	float mFov, mZNear, mZFar;
	size_t mWidth, mHeight;
	struct {
		HorseRadish::Matrix mp2D, mp3D, mp3DInfinite;
	} mMatrices;

	void calcMatrices();

public:
	Viewport() = delete;

	explicit Viewport(const size_t width, const size_t height)
		: Viewport(90.0f, width, height)
	{ }

	explicit Viewport(const float fov, const size_t width, const size_t height)
		: Viewport(fov, width, height, 1.0f, 1000.0f)
	{ }

	explicit Viewport(const float fov, const size_t width, const size_t height, const float zNear, const float zFar)
		: mFov(fov), mZNear(zNear), mZFar(zFar), mWidth(width ? width : 1), mHeight(height ? height : 1)
	{
		calcMatrices();
	}

	const HorseRadish::Matrix& getProjection(ProjectionType projectionType) const;
	
	float fov() const { return mFov; }
	float znear() const { return mZNear; }
	float zfar() const { return mZFar; }

	size_t width() const { return mWidth; }
	size_t height() const { return mHeight; }

	void pointOnZNear(HorseRadish::Vector3f& center) const;

	void projectPoint(ProjectionType projType, const HorseRadish::Matrix& modelView, HorseRadish::Vector3f * const listPoints, size_t numPoints) const;
};

} } }

