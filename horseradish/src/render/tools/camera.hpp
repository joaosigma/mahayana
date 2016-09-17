#pragma once

#include "common\Matrix.hpp"
#include "common\Ray.hpp"

#include <memory>

namespace HorseRadish { namespace Render { namespace Tools {
			
class Camera
{
public:
	enum class CameraType { FirstPerson, OnSphere };
	enum class CameraInput { Keyboard, Mouse };
	enum CameraComponent { Position = (1 << 0), Target = (1 << 1) };
	enum CameraAction { None = 0, Forward = (1 << 0), Backward = (1 << 1), StrifeLeft = (1 << 2), StrifeRight = (1 << 3), Up = (1 << 4), Down = (1 << 5), Run = (1 << 6) };

private:
	HorseRadish::Vector3f camPos, camDir, camUp;
	float rato[3][2] = { {0.0f, 0.0f}, { 0.0f, 0.0f }, { 0.0f, 0.0f } }, sumRato[2] = { 0.0f, 0.0f }, ratoS = 1.0f, keyS = 1.0f;
	float absFocus = 1.0f, onSphereMaxDist = 1000.0f, onSphereMinDist = 1.0f;
	HorseRadish::Matrix modelView;
	std::unique_ptr<Vector3f[]> pointsPos, pointsTarget;
	int numPos = 0, numTarget = 0;
	CameraType targetMode = CameraType::FirstPerson;

	void commitFirstPerson(const CameraAction &actionBitfield, const float &mouseDeltaX, const float &mouseDeltaY, const bool updatePosition, const float timeDeltaS);
	void commitOnSphere(const CameraAction &actionBitfield, const float &mouseDeltaX, const float &mouseDeltaY, const float timeDeltaS);

public:
	Camera();

	void commitInput(const CameraAction &actionBitfield, const float &mouseDeltaX, const float &mouseDeltaY, const bool updatePosition, const float timeDeltaS);
	void commitCatmullRom(const CameraComponent &component, const float &normalizedTime);
	void commitHermite(const CameraComponent &component, const float &normalizedTime);

	void pathClear(const CameraComponent &componentsBitField);
	void pathAdd(const CameraComponent &component, const float x, const float y, const float z);
	void pathAdd(const CameraComponent &component, const HorseRadish::Vector3f &vec);

	bool setCamType(const CameraType &type);
	void setSensitivity(const CameraInput &input, const float s);
	void setPos(const HorseRadish::Vector3f &pos);
	void setPos(const float x, const float y, const float z);
	void setTarget(const HorseRadish::Vector3f &target);
	void setTarget(const float x, const float y, const float z);
	void setDir(const HorseRadish::Vector3f &direction);
	void setDir(const float x, const float y, const float z);
	void setAbsoluteFocus(const float focus);
	void setOnSphereDists(const float minDist, const float maxDist);

	HorseRadish::Vector3f getPos() const;
	HorseRadish::Ray getRay() const;
	HorseRadish::Vector3f getTarget() const;
	HorseRadish::Vector3f getViewDir() const;
	HorseRadish::Vector3f getStrideDir() const;

	float getFocalDist() const;
	float getSensitivity(const CameraInput &input) const;
	
	Camera::CameraType getTargetMode() const;

	const float* getModelView() const;
	void getModelView(float * const mat) const;
};

} } }

