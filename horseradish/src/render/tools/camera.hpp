#pragma once

#include "common/matrix.hpp"
#include "common/ray.hpp"

#include <memory>

namespace HorseRadish { namespace Render { namespace Tools {
			
class Camera
{
protected:
	HorseRadish::Matrix mModelView;
	struct {
		HorseRadish::Vector3f pos, dir, up;
	} mAxis;
	float mAbsFocus;
	
protected:
	Camera()
	{
		mAxis.pos.set(0.0f, 0.0f, 1.0f);
		mAxis.dir.set(0.0f, 0.0f, 0.0f);
		mAxis.up.set(0.0f, 1.0f, 0.0f);
		mModelView.setIdentity();
	}

public:
	void setPos(const HorseRadish::Vector3f &pos)
	{
		mAxis.pos.set(pos);
		mModelView.setGLModelView(mAxis.pos, getTarget(), mAxis.up);
	}

	void setPos(float x, float y, float z)
	{
		mAxis.pos.set(x, y, z);
		mModelView.setGLModelView(mAxis.pos, getTarget(), mAxis.up);
	}

	void setTarget(const HorseRadish::Vector3f &target)
	{
		mAxis.dir = target - mAxis.pos;
		mAxis.dir.normalize();
		mAbsFocus = target.getDistance(mAxis.pos);
		mModelView.setGLModelView(mAxis.pos, getTarget(), mAxis.up);
	}

	void setTarget(float x, float y, float z)
	{
		mAxis.dir = HorseRadish::Vector3f(x, y, z) - mAxis.pos;
		mAxis.dir.normalize();
		mAbsFocus = mAxis.pos.getDistance(x, y, z);
		mModelView.setGLModelView(mAxis.pos, getTarget(), mAxis.up);
	}

	void setDir(const HorseRadish::Vector3f &direction)
	{
		mAxis.dir.set(direction);
		mAxis.dir.normalize();
		mModelView.setGLModelView(mAxis.pos, getTarget(), mAxis.up);
	}

	void setDir(float x, float y, float z)
	{
		mAxis.dir.set(x, y, z);
		mAxis.dir.normalize();
		mModelView.setGLModelView(mAxis.pos, getTarget(), mAxis.up);
	}

	void setAbsoluteFocus(float focus)
	{
		mAbsFocus = focus;
	}

	HorseRadish::Vector3f getPos() const
	{
		return mAxis.pos;
	}

	HorseRadish::Ray getRay() const
	{
		return HorseRadish::Ray(mAxis.pos, mAxis.dir);
	}

	HorseRadish::Vector3f getTarget() const
	{
		return ((mAxis.dir * mAbsFocus) + mAxis.pos);
	}

	HorseRadish::Vector3f getViewDir() const
	{
		return mAxis.dir;
	}

	HorseRadish::Vector3f getStrideDir() const
	{
		auto auxVec = mAxis.dir;
		auxVec[1] += 1.0f;

		auto strideDir = mAxis.dir.crossProduct(auxVec);
		strideDir.normalize();

		return strideDir;
	}

	float getFocalDist() const
	{
		return mAbsFocus;
	}

	const HorseRadish::Matrix& modelView() const
	{
		return mModelView;
	}
};

class CameraFPS final : public Camera
{
public:
	enum class CameraInput { Keyboard, Mouse };
	enum CameraAction { None = 0, Forward = (1 << 0), Backward = (1 << 1), StrifeLeft = (1 << 2), StrifeRight = (1 << 3), Up = (1 << 4), Down = (1 << 5), Run = (1 << 6) };

private:
	struct {
		float keys = 1.0f, mouse = 1.0f;
	} mScale;

public:
	CameraFPS()
		: Camera()
	{ }

	void commitInput(CameraAction actionBitfield, float mouseDeltaX, float mouseDeltaY, bool updatePosition, float timeDeltaS);

	void setMovementScale(CameraInput input, float scale);
	float getMovementScale(CameraInput input) const;
};

class CameraTarget final : public Camera
{
public:
	enum class CameraInput { Keyboard, Mouse };
	enum CameraAction { None = 0, Forward = (1 << 0), Backward = (1 << 1), StrifeLeft = (1 << 2), StrifeRight = (1 << 3), Up = (1 << 4), Down = (1 << 5), Run = (1 << 6) };

private:
	struct {
		float keys = 1.0f, mouse = 1.0f;
	} mScale;
	float mOnSphereMaxDist = 1000.0f, mOnSphereMinDist = 1.0f;

public:
	CameraTarget()
		: Camera()
	{ }

	void commitInput(CameraAction actionBitfield, float mouseDeltaX, float mouseDeltaY, float timeDeltaS);

	void setMovementScale(CameraInput input, float scale);
	void setOnSphereDists(float minDist, float maxDist);

	float getMovementScale(CameraInput input) const;
};

class CameraPath final : public Camera
{
public:
	enum CameraComponent { Position = (1 << 0), Target = (1 << 1) };

private:
	static const constexpr size_t MaxNumPoints = 20;

private:
	std::unique_ptr<Vector3f[]> mPointsPos, mPointsTarget;
	size_t mNumPos = 0, mNumTarget = 0;

public:
	CameraPath()
		: Camera()
	{
		mPointsPos = std::unique_ptr<HorseRadish::Vector3f[]>(new HorseRadish::Vector3f[MaxNumPoints]);
		mPointsTarget = std::unique_ptr<HorseRadish::Vector3f[]>(new HorseRadish::Vector3f[MaxNumPoints]);
	}

	void commitCatmullRom(CameraComponent component, float normalizedTime);
	void commitHermite(CameraComponent component, float normalizedTime);

	void pathClear(CameraComponent componentsBitField);
	void pathAdd(CameraComponent component, float x, float y, float z);
	void pathAdd(CameraComponent component, const HorseRadish::Vector3f &vec);
};

} } }

