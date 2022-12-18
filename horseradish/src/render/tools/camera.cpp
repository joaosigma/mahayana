#include "camera.hpp"

#include "common/quaternion.hpp"

namespace
{

	void evalPointListCatmullRom(const hr::Vector3f* const pList, const int pNum, float nrmTime, hr::Vector3f& pWrite)
	{
		if (!pList || pNum < 4)
			return;

		float tPos = hr::Math::fClamp(nrmTime, 0.0f, 1.0f);
		float step = (float)(pNum - 3);

		int start = hr::Math::ftoi((tPos * step) - (fmod(tPos, 1.0f / step) * step));
		start = std::min(std::max(start, 0), pNum - 4);

		tPos = tPos * step - ((float)start);

		pWrite = hr::Vector3f::evalSplineCatmullRom(pList[start + 0], pList[start + 1], pList[start + 2], pList[start + 3], tPos);
	}

	void evalPointListHermite(const hr::Vector3f* const pList, const int pNum, float nrmTime, hr::Vector3f& pWrite)
	{
		if (!pList || pNum < 4)
			return;

		float tPos = hr::Math::fClamp(nrmTime, 0.0f, 1.0f);
		float step = (float)(pNum - 3);

		int start = hr::Math::ftoi((tPos * step) - (fmod(tPos, 1.0f / step) * step));
		start = std::min(std::max(start, 0), pNum - 4);

		tPos = tPos * step - ((float)start);

		pWrite = hr::Vector3f::evalSplineHermite(pList[start + 0], pList[start + 1], pList[start + 2], pList[start + 3], tPos);
	}
}

namespace hr::render::tools
{
	void CameraFPS::commitInput(CameraAction actionBitfield, float mouseDeltaX, float mouseDeltaY, bool updatePosition, float timeDeltaS)
	{
		hr::Vector3f viewDir;

		float angX = mouseDeltaX * mScale.mouse;
		float angY = mouseDeltaY * mScale.mouse;

		auto strideDir = getStrideDir();

		auto quat = Quaternionf::fromAxisAngle(strideDir, angY);
		viewDir = quat.unitRotate(mAxis.dir);
		viewDir.normalize();

		quat = Quaternionf::fromAxisAngle(0.0f, 1.0f, 0.0f, angX);
		viewDir = quat.unitRotate(viewDir);

		mAxis.dir = viewDir;
		mAxis.dir.normalize();

		if (updatePosition)
		{
			strideDir = getStrideDir();
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

		mModelView = Matrix4f::glModelView(mAxis.pos, getTarget(), mAxis.up);
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
		hr::Vector3f newDir;

		float angX = mouseDeltaX * mScale.mouse;
		float angY = mouseDeltaY * mScale.mouse * (-1.0f);

		newDir = Vector3f{0.0f, 0.0f, -1.0f};
		auto quat = Quaternionf::fromEuler(angX, -angY, 0.0f, Quaternionf::AxisOrder::XYZ);
		newDir = quat.unitRotate(newDir);
		newDir.normalize();

		auto curTarget = getTarget();
		mAxis.dir = newDir;

		float moveAmount = ((actionBitfield & Run) == Run) ? 2.0f : 1.0f;

		if ((actionBitfield & Up) && (mAbsFocus > mOnSphereMinDist))
			mAbsFocus -= (mScale.keys * timeDeltaS * moveAmount);
		if ((actionBitfield & Down) && (mAbsFocus < mOnSphereMaxDist))
			mAbsFocus += (mScale.keys * timeDeltaS * moveAmount);

		mAbsFocus = hr::Math::fClamp(mAbsFocus, mOnSphereMinDist, mOnSphereMaxDist);
		mAxis.pos = curTarget - (newDir * mAbsFocus);

		mModelView = Matrix4f::glModelView(mAxis.pos, getTarget(), mAxis.up);
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
			hr::Vector3f pos;
			evalPointListCatmullRom(mPointsPos.get(), mNumPos, normalizedTime, pos);

			setPos(pos);
		}

		if ((component & Target) && (mNumTarget >= 4))
		{
			hr::Vector3f target;
			evalPointListCatmullRom(mPointsTarget.get(), mNumTarget, normalizedTime, target);

			setTarget(target);
		}
	}

	void CameraPath::commitHermite(CameraComponent component, float normalizedTime)
	{
		if ((component & Position) && (mNumPos >= 4))
		{
			hr::Vector3f pos;
			evalPointListHermite(mPointsPos.get(), mNumPos, normalizedTime, pos);

			setPos(pos);
		}

		if ((component & Target) && (mNumTarget >= 4))
		{
			hr::Vector3f target;
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
		if ((component == Position) && !mPointsPos && (mNumPos < CameraPath::MaxNumPoints))
		{
			mPointsPos[mNumPos] = Vector3f{x, y, z};
			mNumPos++;
		}
		else if ((component == Target) && !mPointsTarget && (mNumTarget < CameraPath::MaxNumPoints))
		{
			mPointsTarget[mNumTarget] = Vector3f{x, y, z};
			mNumTarget++;
		}
	}

	void CameraPath::pathAdd(CameraComponent component, const hr::Vector3f& vec)
	{
		if ((component == Position) && !mPointsPos && (mNumPos < CameraPath::MaxNumPoints))
		{
			mPointsPos[mNumPos] = vec;
			mNumPos++;
		}
		else if ((component == Target) && !mPointsTarget && (mNumTarget < CameraPath::MaxNumPoints))
		{
			mPointsTarget[mNumTarget] = vec;
			mNumTarget++;
		}
	}
}