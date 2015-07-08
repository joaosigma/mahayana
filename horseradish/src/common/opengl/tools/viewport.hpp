#pragma once

#include "common\Vector.hpp"
#include "common\Matrix.hpp"

namespace HorseRadish { namespace OpenGL { namespace Tools {
			
class Viewport
{
public:
	enum class ProjectionType { Proj3D, Proj3DInf, Proj2D };

	static HorseRadish::Matrix genMatrix2DProj(const int width, const int height);

private:
	float mFov, mZNear, mZFar;
	unsigned int mWidth, mHeight;
	struct {
		HorseRadish::Matrix mp2D, mp3D, mp3DInfinite;
	} mMatrices;

	void calcMatrices();

public:
	Viewport() = delete;
	explicit Viewport(const unsigned int width, const unsigned int height);
	explicit Viewport(const float fov, const unsigned int width, const unsigned int height);
	explicit Viewport(const float fov, const unsigned int width, const unsigned int height, const float zNear, const float zFar);

	const HorseRadish::Matrix& getProjection(ProjectionType projectionType) const;
	
	float getFOV() const { return mFov; }
	float getZNear() const { return mZNear; }
	float getZFar() const { return mZFar; }

	unsigned int getWidth() const { return mWidth; }
	unsigned int getHeight() const { return mHeight; }

	void getPointOnZNear(float * const center) const;
	void getPointOnZNear(HorseRadish::Vector3f& center) const;

	void projectPoint(ProjectionType projType, const HorseRadish::Matrix& modelView, HorseRadish::Vector3f * const listPoints, const int numPoints) const;
};

} } }

