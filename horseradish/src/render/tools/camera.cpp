#include "camera.hpp"

#include "common\Quaternion.hpp"

#define CAMERA_MAX_POINTS 20

typedef void(*EVAL_FUNC)(const HorseRadish::Vector3f * const points, const float t, HorseRadish::Vector3f * const output);

static
void evalCatmullRom(const HorseRadish::Vector3f * const points, const float t, HorseRadish::Vector3f * const output)
{
	HorseRadish::Vector3f a, b, c, d;

	float newT = HorseRadish::Math::fClamp(t, 0.0f, 1.0f);

	float tSqr = newT*newT*0.5f;
	float tSqrSqr = newT*tSqr;
	newT *= 0.5f;

	output->set(0.0f);

	a.set(points[0]);
	b.set(points[1]);
	c.set(points[2]);
	d.set(points[3]);
	a *= -tSqrSqr;
	b *= tSqrSqr*3.0f;
	c *= tSqrSqr*(-3.0f);
	d *= tSqrSqr;

	(*output) += a;
	(*output) += b;
	(*output) += c;
	(*output) += d;

	a.set(points[0]);
	b.set(points[1]);
	c.set(points[2]);
	d.set(points[3]);
	a *= tSqr * 2;
	b *= tSqr*(-5.0f);
	c *= tSqr*4.0f;
	d *= -tSqr;

	(*output) += a;
	(*output) += b;
	(*output) += c;
	(*output) += d;

	a.set(points[0]);
	b.set(points[2]);
	a *= -newT;
	b *= newT;

	(*output) += a;
	(*output) += b;

	(*output) += *(points + 1);
}

static
void evalHermite(const HorseRadish::Vector3f * const points, const float t, HorseRadish::Vector3f * const output)
{
	HorseRadish::Vector3f aux, d1, d2;

	float newT = HorseRadish::Math::fClamp(t, 0.0f, 1.0f);

	float tSqr = newT*newT;
	float tCube = newT*tSqr;

	d1.set(points[1]);
	d1 -= points[0];
	d2.set(points[3]);
	d2 -= points[2];

	(*output).set(points[1]);
	(*output) *= 2.0f*tCube - 3.0f*tSqr + 1.0f;

	aux.set(points[2]);
	aux *= -2.0f*tCube + 3.0f*tSqr;
	(*output) += aux;

	d1 *= tCube - 2.0f*tSqr + t;
	d2 *= tCube - tSqr;
	(*output) += d1;
	(*output) += d2;
}

static
void evalPointList(const HorseRadish::Vector3f * const pList, const int pNum, const float nrmTime, EVAL_FUNC funcEval, HorseRadish::Vector3f * const pWrite)
{
	if (pList == nullptr || pNum < 4 || funcEval == nullptr || pWrite == nullptr)
		return;

	float tPos = HorseRadish::Math::fClamp(nrmTime, 0.0f, 1.0f);

	float step = (float)(pNum - 3);

	int start = HorseRadish::Math::ftoi((tPos*step) - (fmod(tPos, 1.0f / step)*step));
	start = std::min(std::max(start, 0), pNum - 4);

	tPos = tPos*step - ((float)start);

	funcEval(pList + start, tPos, pWrite);
}

namespace HorseRadish { namespace Render { namespace Tools {

void Camera::commitFirstPerson(const CameraAction &actionBitfield, const float &mouseDeltaX, const float &mouseDeltaY, const bool updatePosition, const float timeDeltaS)
{
	HorseRadish::Quaternion quat;
	HorseRadish::Vector3f viewDir, eixo;

	rato[0][0] = rato[1][0];
	rato[0][1] = rato[1][1];
	rato[1][0] = rato[2][0];
	rato[1][1] = rato[2][1];
	rato[2][0] = mouseDeltaX*ratoS;
	rato[2][1] = mouseDeltaY*ratoS*(-1.0f);

	float angX = (rato[0][0] + rato[1][0] + rato[2][0] + rato[2][0])*0.25f;
	float angY = (rato[0][1] + rato[1][1] + rato[2][1] + rato[2][1])*0.25f;

	eixo.storeCrossProduct(camDir, camUp);
	eixo.normalize();

	quat.setAxisAngle(eixo, angY);
	quat.rotateVector3(camDir, viewDir);
	viewDir.normalize();
	quat.setAxisAngle(0.0f, 1.0f, 0.0f, -angX);
	quat.rotateVector3(viewDir);

	camDir = viewDir;
	camDir.normalize();

	if (updatePosition)
	{
		float moveAmount = ((actionBitfield & Run) == Run) ? 2.0f : 1.0f;

		auto strideDir = getStrideDir();

		if ((actionBitfield & Forward) == Forward)
			camPos += camDir*(keyS*timeDeltaS*moveAmount);
		if ((actionBitfield & Backward) == Backward)
			camPos -= camDir*(keyS*timeDeltaS*moveAmount);
		if ((actionBitfield & StrifeLeft) == StrifeLeft)
			camPos -= strideDir*(keyS*timeDeltaS*moveAmount);
		if ((actionBitfield & StrifeRight) == StrifeRight)
			camPos += strideDir*(keyS*timeDeltaS*moveAmount);
		if ((actionBitfield & Up) == Up)
			camPos[1] += keyS*timeDeltaS*moveAmount;
		if ((actionBitfield & Down) == Down)
			camPos[1] -= keyS*timeDeltaS*moveAmount;
	}

	modelView.setGLModelView(camPos, getTarget(), camUp);
}

void Camera::commitOnSphere(const CameraAction &actionBitfield, const float &mouseDeltaX, const float &mouseDeltaY, const float timeDeltaS)
{
	HorseRadish::Quaternion quat;
	HorseRadish::Vector3f newDir;

	sumRato[0] += mouseDeltaX * ratoS;
	sumRato[1] += mouseDeltaY * ratoS * (-1.0f);

	newDir.set(0.0f, 0.0f, -1.0f);
	quat.setFromEuler(sumRato[1], -sumRato[0], 0.0f);
	quat.rotateVector3(newDir);
	newDir.normalize();

	auto curTarget = getTarget();
	camDir = newDir;

	float moveAmount = ((actionBitfield & Run) == Run) ? 2.0f : 1.0f;

	if (((actionBitfield & Up) == Up) && (absFocus > onSphereMinDist))
		absFocus -= keyS * timeDeltaS * moveAmount;
	if (((actionBitfield & Down) == Down) && (absFocus < onSphereMaxDist))
		absFocus += keyS * timeDeltaS * moveAmount;

	absFocus = HorseRadish::Math::fClamp(absFocus, onSphereMinDist, onSphereMaxDist);
	camPos = curTarget - (newDir * absFocus);

	modelView.setGLModelView(camPos, getTarget(), camUp);
}

Camera::Camera()
{
	camPos.set(0.0f, 0.0f, 1.0f);
	camDir.set(0.0f, 0.0f, 0.0f);
	camUp.set(0.0f, 1.0f, 0.0f);

	modelView.setIdentity();

	pointsPos = std::unique_ptr<HorseRadish::Vector3f[]>(new HorseRadish::Vector3f[CAMERA_MAX_POINTS]);
	pointsTarget = std::unique_ptr<HorseRadish::Vector3f[]>(new HorseRadish::Vector3f[CAMERA_MAX_POINTS]);
}

void Camera::commitInput(const CameraAction &actionBitfield, const float &mouseDeltaX, const float &mouseDeltaY, const bool updatePosition, const float timeDeltaS)
{
	switch (targetMode)
	{
	case CameraType::FirstPerson:
		commitFirstPerson(actionBitfield, mouseDeltaX, mouseDeltaY, updatePosition, timeDeltaS);
		return;
	case CameraType::OnSphere:
		commitOnSphere(actionBitfield, mouseDeltaX, mouseDeltaY, timeDeltaS);
		return;
	}
}

void Camera::commitCatmullRom(const CameraComponent &component, const float &normalizedTime)
{
	if ((component == Position) && (numPos >= 4))
	{
		evalPointList(pointsPos.get(), numPos, normalizedTime, evalCatmullRom, &camPos);

		modelView.setGLModelView(camPos, getTarget(), camUp);
	}

	if ((component == Target) && (numTarget >= 4))
	{
		HorseRadish::Vector3f camTarget;

		evalPointList(pointsTarget.get(), numTarget, normalizedTime, evalCatmullRom, &camTarget);

		setTarget(camTarget);
		modelView.setGLModelView(camPos, getTarget(), camUp);
	}
}

void Camera::commitHermite(const CameraComponent &component, const float &normalizedTime)
{
	if ((component == Position) && (numPos >= 4))
	{
		evalPointList(pointsPos.get(), numPos, normalizedTime, evalHermite, &camPos);

		modelView.setGLModelView(camPos, getTarget(), camUp);
	}

	if ((component == Target) && (numTarget >= 4))
	{
		HorseRadish::Vector3f camTarget;

		evalPointList(pointsTarget.get(), numTarget, normalizedTime, evalHermite, &camTarget);

		setTarget(camTarget);
		modelView.setGLModelView(camPos, getTarget(), camUp);
	}
}

void Camera::pathClear(const CameraComponent &componentsBitField)
{
	if ((componentsBitField & Position) == Position)
		this->numPos = 0;
	if ((componentsBitField & Target) == Target)
		this->numTarget = 0;
}

void Camera::pathAdd(const CameraComponent &component, const float x, const float y, const float z)
{
	if ((component == Position) && (pointsPos != nullptr) && (numPos < CAMERA_MAX_POINTS))
	{
		pointsPos[numPos].set(x, y, z);
		numPos++;
	}

	if ((component == Target) && (pointsTarget != nullptr) && (numTarget < CAMERA_MAX_POINTS))
	{
		pointsTarget[numTarget].set(x, y, z);
		numTarget++;
	}
}

void Camera::pathAdd(const CameraComponent &component, const HorseRadish::Vector3f &vec)
{
	if ((component == Position) && (pointsPos != nullptr) && (numPos < CAMERA_MAX_POINTS))
	{
		pointsPos[numPos].set(vec);
		numPos++;
	}

	if ((component == Target) && (pointsTarget != nullptr) && (numTarget < CAMERA_MAX_POINTS))
	{
		pointsTarget[numTarget].set(vec);
		numTarget++;
	}
}

bool Camera::setCamType(const Camera::CameraType &type)
{
	if (type == CameraType::FirstPerson)
	{
		targetMode = CameraType::FirstPerson;
		return true;
	}

	if (type == CameraType::OnSphere)
	{
		targetMode = CameraType::OnSphere;
		sumRato[0] = sumRato[1] = 0.0f;
		return true;
	}

	return false;
}

void Camera::setSensitivity(const CameraInput &input, const float s)
{
	switch (input)
	{
	case CameraInput::Keyboard:
		keyS = s;
		break;
	case CameraInput::Mouse:
		ratoS = s;
		break;
	default:
		return;
	}
}

void Camera::setPos(const HorseRadish::Vector3f &pos)
{
	camPos.set(pos);
	modelView.setGLModelView(camPos, getTarget(), camUp);
}

void Camera::setPos(const float x, const float y, const float z)
{
	camPos.set(x, y, z);
	modelView.setGLModelView(camPos, getTarget(), camUp);
}

void Camera::setTarget(const HorseRadish::Vector3f &target)
{
	camDir = target - camPos;
	camDir.normalize();
	absFocus = target.getDistance(camPos);
	modelView.setGLModelView(camPos, getTarget(), camUp);
}

void Camera::setTarget(const float x, const float y, const float z)
{
	camDir = HorseRadish::Vector3f(x, y, z) - camPos;
	camDir.normalize();
	absFocus = camPos.getDistance(x, y, z);
	modelView.setGLModelView(camPos, getTarget(), camUp);
}

void Camera::setDir(const HorseRadish::Vector3f &direction)
{
	camDir.set(direction);
	camDir.normalize();
	modelView.setGLModelView(camPos, getTarget(), camUp);
}

void Camera::setDir(const float x, const float y, const float z)
{
	camDir.set(x, y, z);
	camDir.normalize();
	modelView.setGLModelView(camPos, getTarget(), camUp);
}

void Camera::setAbsoluteFocus(const float focus)
{
	absFocus = focus;
}

void Camera::setOnSphereDists(const float minDist, const float maxDist)
{
	float realMin, realMax;

	realMin = minDist;
	realMax = maxDist;

	if (realMin < 0.0f)
		realMin *= -1.0f;
	if (realMax < 0.0f)
		realMax *= -1.0f;

	if (realMin < realMax)
	{
		onSphereMinDist = realMin;
		onSphereMaxDist = realMax;
	}
	else
	{
		onSphereMinDist = realMax;
		onSphereMaxDist = realMin;
	}
}

HorseRadish::Vector3f Camera::getPos() const
{
	return HorseRadish::Vector3f(camPos);
}

HorseRadish::Ray Camera::getRay() const
{
	HorseRadish::Ray ray;

	ray.setOrigin(camPos);
	ray.setDirection(camDir);
	return ray;
}

HorseRadish::Vector3f Camera::getTarget() const
{
	HorseRadish::Vector3f target;

	target = camDir;
	target *= absFocus;
	target += camPos;
	return target;
}

HorseRadish::Vector3f Camera::getViewDir() const
{
	return HorseRadish::Vector3f(camDir);
}

HorseRadish::Vector3f Camera::getStrideDir() const
{
	HorseRadish::Vector3f strideDir, auxVec;

	auxVec = camDir;
	auxVec[1] += 1.0f;

	strideDir.storeCrossProduct(camDir, auxVec);
	strideDir.normalize();

	return strideDir;
}

float Camera::getFocalDist() const
{
	return absFocus;
}

float Camera::getSensitivity(const CameraInput &input) const
{
	switch (input)
	{
	case CameraInput::Keyboard:
		return keyS;
	case CameraInput::Mouse:
		return ratoS;
	default:
		break;
	}
	
	return 0.0f;
}

const float* Camera::getModelView() const
{
	return modelView.data();
}

void Camera::getModelView(float * const mat) const
{
	modelView.write(mat);
}

Camera::CameraType Camera::getTargetMode() const
{
	return targetMode;
}

} } }