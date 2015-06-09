#pragma once

#include "common\Matrix.hpp"
#include "common\Ray.hpp"

namespace HorseRadish { namespace Render { namespace Tools {
			
class Camera
{

public:
	enum CameraType { FirstPerson, OnSphere };
	enum CameraInput { Keyboard, Mouse };
	enum CameraComponent { Position = (1 << 0), Target = (1 << 1) };
	enum CameraAction { None = 0, Forward = (1 << 0), Backward = (1 << 1), StrifeLeft = (1 << 2), StrifeRight = (1 << 3), Up = (1 << 4), Down = (1 << 5), Run = (1 << 6) };

private:
	HorseRadish::Vector camPos, camDir, camUp;
	float rato[3][2], sumRato[2], ratoS, keyS, absFocus, onSphereMaxDist, onSphereMinDist;
	HorseRadish::Matrix modelView;
	HorseRadish::Vector *pointsPos, *pointsTarget;
	int numPos, numTarget;
	CameraType targetMode;

	void commitFirstPerson(const CameraAction &actionBitfield, const float &mouseDeltaX, const float &mouseDeltaY, const bool updatePosition, const float timeDeltaS);
	void commitOnSphere(const CameraAction &actionBitfield, const float &mouseDeltaX, const float &mouseDeltaY, const float timeDeltaS);

public:
	Camera();
	~Camera();

	void CommitInput(const CameraAction &actionBitfield, const float &mouseDeltaX, const float &mouseDeltaY, const bool updatePosition, const float timeDeltaS);
	void CommitCatmullRom(const CameraComponent &component, const float &normalizedTime);
	void CommitHermite(const CameraComponent &component, const float &normalizedTime);

	void PathClear(const CameraComponent &componentsBitField);
	void PathAdd(const CameraComponent &component, const float x, const float y, const float z);
	void PathAdd(const CameraComponent &component, const HorseRadish::Vector &vec);

	bool SetCamType(const CameraType &type);
	void SetSensitivity(const CameraInput &input, const float s);
	void SetPos(const HorseRadish::Vector &pos);
	void SetPos(const float x, const float y, const float z);
	void SetTarget(const HorseRadish::Vector &target);
	void SetTarget(const float x, const float y, const float z);
	void SetDir(const HorseRadish::Vector &direction);
	void SetDir(const float x, const float y, const float z);
	void SetAbsoluteFocus(const float focus);
	void SetOnSphereDists(const float minDist, const float maxDist);

	void GetPos(HorseRadish::Vector &pos) const;
	HorseRadish::Vector GetPos() const;
	void GetTarget(HorseRadish::Vector &target) const;
	HorseRadish::Vector GetTarget() const;
	void GetViewDir(HorseRadish::Vector &dir) const;
	HorseRadish::Vector GetViewDir() const;
	void GetStrideDir(HorseRadish::Vector &dir) const;
	HorseRadish::Vector GetStrideDir() const;

	float GetFocalDist() const;
	float GetSensitivity(const CameraInput &input) const;
	void GetRay(HorseRadish::Ray &ray) const;
	Camera::CameraType GetTargetMode() const;

	const float* GetModelView() const;
	void GetModelView(float * const mat) const;
	void GetFrustumCorners() const;
};

} } }

