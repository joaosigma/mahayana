#include "camera.hpp"

#include "common\Quaternion.hpp"

static
void evalPointListCatmullRom(const HorseRadish::Vector3f * const pList, const int pNum, float nrmTime, HorseRadish::Vector3f& pWrite)
{
	if (pList == nullptr || pNum < 4)
		return;

	float tPos = HorseRadish::Math::fClamp(nrmTime, 0.0f, 1.0f);
	float step = (float)(pNum - 3);

	int start = HorseRadish::Math::ftoi((tPos*step) - (fmod(tPos, 1.0f / step)*step));
	start = std::min(std::max(start, 0), pNum - 4);

	tPos = tPos*step - ((float)start);

	pWrite = HorseRadish::Vector3f::evalSplineCatmullRom(pList[start + 0], pList[start + 1], pList[start + 2], pList[start + 3], tPos);
}

static
void evalPointListHermite(const HorseRadish::Vector3f * const pList, const int pNum, float nrmTime, HorseRadish::Vector3f& pWrite)
{
	if (pList == nullptr || pNum < 4)
		return;

	float tPos = HorseRadish::Math::fClamp(nrmTime, 0.0f, 1.0f);
	float step = (float)(pNum - 3);

	int start = HorseRadish::Math::ftoi((tPos*step) - (fmod(tPos, 1.0f / step)*step));
	start = std::min(std::max(start, 0), pNum - 4);

	tPos = tPos*step - ((float)start);

	pWrite = HorseRadish::Vector3f::evalSplineHermite(pList[start + 0], pList[start + 1], pList[start + 2], pList[start + 3], tPos);
}

namespace HorseRadish { namespace Render { namespace Tools {

void CameraFPS::commitInput(CameraAction actionBitfield, float mouseDeltaX, float mouseDeltaY, bool updatePosition, float timeDeltaS)
{
	HorseRadish::Quaternion quat;
	HorseRadish::Vector3f viewDir;

	float angX = mouseDeltaX * mScale.mouse;
	float angY = mouseDeltaY * mScale.mouse * (-1.0f);

	auto axis = mAxis.dir.crossProduct(mAxis.up);
	axis.normalize();

	quat.setAxisAngle(axis, angY);
	quat.rotateVector3(mAxis.dir, viewDir);
	viewDir.normalize();
	quat.setAxisAngle(0.0f, 1.0f, 0.0f, -angX);
	quat.rotateVector3(viewDir);

	mAxis.dir = viewDir;
	mAxis.dir.normalize();

	if (updatePosition)
	{
		auto strideDir = getStrideDir();
		auto movementScale = mScale.keys * timeDeltaS * ((actionBitfield & Run) ? 2.0f : 1.0f);

		if ((actionBitfield & Forward) == Forward)
			mAxis.pos += mAxis.dir * movementScale;
		if ((actionBitfield & Backward) == Backward)
			mAxis.pos -= mAxis.dir * movementScale;
		if ((actionBitfield & StrifeLeft) == StrifeLeft)
			mAxis.pos -= strideDir * movementScale;
		if ((actionBitfield & StrifeRight) == StrifeRight)
			mAxis.pos += strideDir * movementScale;
		if ((actionBitfield & Up) == Up)
			mAxis.pos[1] += movementScale;
		if ((actionBitfield & Down) == Down)
			mAxis.pos[1] -= movementScale;
	}

	mModelView.setGLModelView(mAxis.pos, getTarget(), mAxis.up);
}

void CameraFPS::setMovementScale(CameraInput input, float scale)
{
	switch (input)
	{
	case CameraInput::Keyboard:
		mScale.keys = scale;
		break;
	case CameraInput::Mouse:
		mScale.mouse = scale;
		break;
	default:
		return;
	}
}

float CameraFPS::getMovementScale(CameraInput input) const
{
	switch (input)
	{
	case CameraInput::Keyboard:
		return mScale.keys;
	case CameraInput::Mouse:
		return mScale.mouse;
	default:
		break;
	}

	return 0.0f;
}

void CameraTarget::commitInput(CameraAction actionBitfield, float mouseDeltaX, float mouseDeltaY, float timeDeltaS)
{
	HorseRadish::Quaternion quat;
	HorseRadish::Vector3f newDir;

	float angX = mouseDeltaX * mScale.mouse;
	float angY = mouseDeltaY * mScale.mouse*(-1.0f);

	newDir.set(0.0f, 0.0f, -1.0f);
	quat.setFromEuler(angX, -angY, 0.0f);
	quat.rotateVector3(newDir);
	newDir.normalize();

	auto curTarget = getTarget();
	mAxis.dir = newDir;

	float moveAmount = ((actionBitfield & Run) == Run) ? 2.0f : 1.0f;

	if ((actionBitfield & Up) && (mAbsFocus > mOnSphereMinDist))
		mAbsFocus -= (mScale.keys * timeDeltaS * moveAmount);
	if ((actionBitfield & Down) && (mAbsFocus < mOnSphereMaxDist))
		mAbsFocus += (mScale.keys * timeDeltaS * moveAmount);

	mAbsFocus = HorseRadish::Math::fClamp(mAbsFocus, mOnSphereMinDist, mOnSphereMaxDist);
	mAxis.pos = curTarget - (newDir * mAbsFocus);

	mModelView.setGLModelView(mAxis.pos, getTarget(), mAxis.up);
}

void CameraTarget::setMovementScale(CameraInput input, float scale)
{
	switch (input)
	{
	case CameraInput::Keyboard:
		mScale.keys = scale;
		break;
	case CameraInput::Mouse:
		mScale.mouse = scale;
		break;
	default:
		return;
	}
}

void CameraTarget::setOnSphereDists(float minDist, float maxDist)
{
	if (minDist < 0.0f)
		minDist *= -1.0f;
	if (maxDist < 0.0f)
		maxDist *= -1.0f;

	if (minDist < maxDist)
	{
		mOnSphereMinDist = minDist;
		mOnSphereMaxDist = maxDist;
	}
	else
	{
		mOnSphereMinDist = maxDist;
		mOnSphereMaxDist = minDist;
	}
}

float CameraTarget::getMovementScale(CameraInput input) const
{
	switch (input)
	{
	case CameraInput::Keyboard:
		return mScale.keys;
	case CameraInput::Mouse:
		return mScale.mouse;
	default:
		break;
	}

	return 0.0f;
}

void CameraPath::commitCatmullRom(CameraComponent component, float normalizedTime)
{
	if ((component & Position) && (mNumPos >= 4))
	{
		HorseRadish::Vector3f pos;
		evalPointListCatmullRom(mPointsPos.get(), mNumPos, normalizedTime, pos);

		setPos(pos);
	}

	if ((component & Target) && (mNumTarget >= 4))
	{
		HorseRadish::Vector3f target;
		evalPointListCatmullRom(mPointsTarget.get(), mNumTarget, normalizedTime, target);

		setTarget(target);
	}
}

void CameraPath::commitHermite(CameraComponent component, float normalizedTime)
{
	if ((component & Position) && (mNumPos >= 4))
	{
		HorseRadish::Vector3f pos;
		evalPointListHermite(mPointsPos.get(), mNumPos, normalizedTime, pos);

		setPos(pos);
	}

	if ((component & Target) && (mNumTarget >= 4))
	{
		HorseRadish::Vector3f target;
		evalPointListHermite(mPointsTarget.get(), mNumTarget, normalizedTime, target);

		setTarget(target);
	}
}

void CameraPath::pathClear(CameraComponent componentsBitField)
{
	if (componentsBitField & Position)
		mNumPos = 0;
	if (componentsBitField & Target)
		mNumTarget = 0;
}

void CameraPath::pathAdd(CameraComponent component, float x, float y, float z)
{
	if ((component == Position) && (mPointsPos != nullptr) && (mNumPos < CameraPath::MaxNumPoints))
	{
		mPointsPos[mNumPos].set(x, y, z);
		mNumPos++;
	}
	else if ((component == Target) && (mPointsTarget != nullptr) && (mNumTarget < CameraPath::MaxNumPoints))
	{
		mPointsTarget[mNumTarget].set(x, y, z);
		mNumTarget++;
	}
}

void CameraPath::pathAdd(CameraComponent component, const HorseRadish::Vector3f &vec)
{
	if ((component == Position) && (mPointsPos != nullptr) && (mNumPos < CameraPath::MaxNumPoints))
	{
		mPointsPos[mNumPos].set(vec);
		mNumPos++;
	}
	else if ((component == Target) && (mPointsTarget != nullptr) && (mNumTarget < CameraPath::MaxNumPoints))
	{
		mPointsTarget[mNumTarget].set(vec);
		mNumTarget++;
	}
}

} } }