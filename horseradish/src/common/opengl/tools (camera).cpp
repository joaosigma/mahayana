#include "tools.hpp"

#include "common\Quaternion.hpp"

#define CAMERA_MAX_POINTS			20

typedef void (*EVAL_FUNC)(const HorseRadish::Vector * const points, const float t, HorseRadish::Vector * const output);

static
void evalCatmullRom(const HorseRadish::Vector * const points, const float t, HorseRadish::Vector * const output)
{
	HorseRadish::Vector a, b, c, d;

	float newT = HorseRadish::Math::fClamp(t, 0.0f, 1.0f);

	float tSqr = newT*newT*0.5f;
	float tSqrSqr = newT*tSqr;
	newT *= 0.5f;

	output->Set(0.0f);

	a.Set(points + 0);
	b.Set(points + 1);
	c.Set(points + 2);
	d.Set(points + 3);
	a *= -tSqrSqr;
	b *= tSqrSqr*3.0f;
	c *= tSqrSqr*(-3.0f);
	d *= tSqrSqr;

	(*output) += a;
	(*output) += b;
	(*output) += c;
	(*output) += d;

	a.Set(points + 0);
	b.Set(points + 1);
	c.Set(points + 2);
	d.Set(points + 3);
	a *= tSqr * 2;
	b *= tSqr*(-5.0f);
	c *= tSqr*4.0f;
	d *= -tSqr;

	(*output) += a;
	(*output) += b;
	(*output) += c;
	(*output) += d;

	a.Set(points + 0);
	b.Set(points + 2);
	a *= -newT;
	b *= newT;

	(*output) += a;
	(*output) += b;

	(*output) += *(points + 1);
}

static
void evalHermite(const HorseRadish::Vector * const points, const float t, HorseRadish::Vector * const output)
{
	HorseRadish::Vector aux, d1, d2;

	float newT = HorseRadish::Math::fClamp(t, 0.0f, 1.0f);

	float tSqr = newT*newT;
	float tCube = newT*tSqr;

	d1.Set(points + 1);
	d1 -= *(points + 0);
	d2.Set(points + 3);
	d2 -= *(points + 2);

	(*output).Set(points + 1);
	(*output) *= 2.0f*tCube - 3.0f*tSqr + 1.0f;

	aux.Set(points + 2);
	aux *= -2.0f*tCube + 3.0f*tSqr;
	(*output) += aux;

	d1 *= tCube - 2.0f*tSqr + t;
	d2 *= tCube - tSqr;
	(*output) += d1;
	(*output) += d2;
}

static
void evalPointList(const HorseRadish::Vector * const pList, const int pNum, const float nrmTime, EVAL_FUNC funcEval, HorseRadish::Vector * const pWrite)
{
	if (pList == nullptr || pNum < 4 || funcEval == nullptr || pWrite == nullptr)
		return;

	float tPos = HorseRadish::Math::fClamp(nrmTime, 0.0f, 1.0f);

	float step = (float)(pNum - 3);

	int start = HorseRadish::Math::ftoi((tPos*step) - (fmod(tPos, 1.0f / step)*step));
	start = HorseRadish::Math::iClampZero(start, pNum - 4);

	tPos = tPos*step - ((float)start);

	funcEval(pList + start, tPos, pWrite);
}

namespace HorseRadish
{
	namespace OpenGL
	{
		namespace Tools
		{
			void Camera::commitFirstPerson(const CameraAction &actionBitfield, const float &mouseDeltaX, const float &mouseDeltaY, const bool updatePosition, const float timeDeltaS)
			{
				HorseRadish::Quaternion quat;
				HorseRadish::Vector viewDir, eixo;

				rato[0][0] = rato[1][0];
				rato[0][1] = rato[1][1];
				rato[1][0] = rato[2][0];
				rato[1][1] = rato[2][1];
				rato[2][0] = mouseDeltaX*ratoS;
				rato[2][1] = mouseDeltaY*ratoS*(-1.0f);

				float angX = (rato[0][0] + rato[1][0] + rato[2][0] + rato[2][0])*0.25f;
				float angY = (rato[0][1] + rato[1][1] + rato[2][1] + rato[2][1])*0.25f;

				eixo.StoreCrossProduct(camDir, camUp);
				eixo.Normalize();

				quat.SetAxisAngle(eixo, angY);
				quat.RotateVector(camDir, viewDir);
				viewDir.Normalize();
				quat.SetAxisAngle(0.0f, 1.0f, 0.0f, -angX);
				quat.RotateVector(viewDir);

				camDir = viewDir;
				camDir.Normalize();

				if (updatePosition)
				{
					HorseRadish::Vector strideDir;

					float moveAmount = ((actionBitfield & Run) == Run) ? 2.0f : 1.0f;

					GetStrideDir(strideDir);

					if ((actionBitfield & Forward) == Forward)
						camPos += camDir*(keyS*timeDeltaS*moveAmount);
					if ((actionBitfield & Backward) == Backward)
						camPos -= camDir*(keyS*timeDeltaS*moveAmount);
					if ((actionBitfield & StrifeLeft) == StrifeLeft)
						camPos -= strideDir*(keyS*timeDeltaS*moveAmount);
					if ((actionBitfield & StrifeRight) == StrifeRight)
						camPos += strideDir*(keyS*timeDeltaS*moveAmount);
					if ((actionBitfield & Up) == Up)
						camPos.y += keyS*timeDeltaS*moveAmount;
					if ((actionBitfield & Down) == Down)
						camPos.y -= keyS*timeDeltaS*moveAmount;
				}

				modelView.SetGLModelView(camPos, GetTarget(), camUp);
			}

			void Camera::commitOnSphere(const CameraAction &actionBitfield, const float &mouseDeltaX, const float &mouseDeltaY, const float timeDeltaS)
			{
				HorseRadish::Quaternion quat;
				HorseRadish::Vector newDir, posChange, curTarget;

				sumRato[0] += mouseDeltaX * ratoS;
				sumRato[1] += mouseDeltaY * ratoS * (-1.0f);

				newDir.Set(0.0f, 0.0f, -1.0f);
				quat.SetFromEuler(sumRato[1], -sumRato[0], 0.0f);
				quat.RotateVector(newDir);
				newDir.Normalize();

				GetTarget(curTarget);
				camDir = newDir;

				float moveAmount = ((actionBitfield & Run) == Run) ? 2.0f : 1.0f;

				if (((actionBitfield & Up) == Up) && (absFocus > onSphereMinDist))
					absFocus -= keyS * timeDeltaS * moveAmount;
				if (((actionBitfield & Down) == Down) && (absFocus < onSphereMaxDist))
					absFocus += keyS * timeDeltaS * moveAmount;

				absFocus = HorseRadish::Math::fClamp(absFocus, onSphereMinDist, onSphereMaxDist);
				camPos = curTarget - (newDir * absFocus);

				modelView.SetGLModelView(camPos, GetTarget(), camUp);
			}

			Camera::Camera()
			{
				camPos.Set(0.0f, 0.0f, 1.0f);
				camDir.Set(0.0f, 0.0f, 0.0f);
				camUp.Set(0.0f, 1.0f, 0.0f);
				absFocus = 1.0f;

				rato[0][0] = rato[0][1] = 0.0f;
				rato[1][0] = rato[1][1] = 0.0f;
				rato[2][0] = rato[2][1] = 0.0f;
				sumRato[0] = sumRato[1] = 0.0f;

				modelView.SetIdentidade();

				ratoS = keyS = 1.0f;

				targetMode = FirstPerson;
				onSphereMaxDist = 1000.0f;
				onSphereMinDist = 1.0f;

				numPos = numTarget = 0;
				pointsPos = new HorseRadish::Vector[CAMERA_MAX_POINTS];
				pointsTarget = new HorseRadish::Vector[CAMERA_MAX_POINTS];
				if (pointsPos == nullptr || pointsTarget == nullptr)
				{
					if (pointsPos)
						delete[] pointsPos;
					if (pointsTarget)
						delete[] pointsTarget;
					pointsPos = pointsTarget = nullptr;
				}
			}

			Camera::~Camera()
			{
				modelView.SetIdentidade();

				camPos.Set(0.0f, 0.0f, 1.0f);
				camDir.Set(0.0f, 0.0f, -1.0f);
				camUp.Set(0.0f, 1.0f, 0.0f);
				absFocus = 1.0f;

				numPos = numTarget = 0;
				if (pointsPos)
					delete[] pointsPos;
				if (pointsTarget)
					delete[] pointsTarget;
				pointsPos = pointsTarget = nullptr;
			}

			void Camera::CommitInput(const CameraAction &actionBitfield, const float &mouseDeltaX, const float &mouseDeltaY, const bool updatePosition, const float timeDeltaS)
			{
				switch (FirstPerson)
				{
				case FirstPerson:
					commitFirstPerson(actionBitfield, mouseDeltaX, mouseDeltaY, updatePosition, timeDeltaS);
					return;
				case OnSphere:
					commitOnSphere(actionBitfield, mouseDeltaX, mouseDeltaY, timeDeltaS);
					return;
				}
			}

			void Camera::CommitCatmullRom(const CameraComponent &component, const float &normalizedTime)
			{
				if ((component == Position) && (numPos >= 4))
				{
					evalPointList(pointsPos, numPos, normalizedTime, evalCatmullRom, &camPos);

					modelView.SetGLModelView(camPos, GetTarget(), camUp);
				}

				if ((component == Target) && (numTarget >= 4))
				{
					HorseRadish::Vector camTarget;

					evalPointList(pointsTarget, numTarget, normalizedTime, evalCatmullRom, &camTarget);

					SetTarget(camTarget);
					modelView.SetGLModelView(camPos, GetTarget(), camUp);
				}
			}

			void Camera::CommitHermite(const CameraComponent &component, const float &normalizedTime)
			{
				if ((component == Position) && (numPos >= 4))
				{
					evalPointList(pointsPos, numPos, normalizedTime, evalHermite, &camPos);

					modelView.SetGLModelView(camPos, GetTarget(), camUp);
				}

				if ((component == Target) && (numTarget >= 4))
				{
					HorseRadish::Vector camTarget;

					evalPointList(pointsTarget, numTarget, normalizedTime, evalHermite, &camTarget);

					SetTarget(camTarget);
					modelView.SetGLModelView(camPos, GetTarget(), camUp);
				}
			}

			void Camera::PathClear(const CameraComponent &componentsBitField)
			{
				if ((componentsBitField & Position) == Position)
					this->numPos = 0;
				if ((componentsBitField & Target) == Target)
					this->numTarget = 0;
			}

			void Camera::PathAdd(const CameraComponent &component, const float x, const float y, const float z)
			{
				if ((component == Position) && (pointsPos != nullptr) && (numPos < CAMERA_MAX_POINTS))
				{
					pointsPos[numPos].Set(x, y, z);
					numPos++;
				}

				if ((component == Target) && (pointsTarget != nullptr) && (numTarget < CAMERA_MAX_POINTS))
				{
					pointsTarget[numTarget].Set(x, y, z);
					numTarget++;
				}
			}

			void Camera::PathAdd(const CameraComponent &component, const HorseRadish::Vector &vec)
			{
				if ((component == Position) && (pointsPos != nullptr) && (numPos < CAMERA_MAX_POINTS))
				{
					pointsPos[numPos].Set(vec);
					numPos++;
				}

				if ((component == Target) && (pointsTarget != nullptr) && (numTarget < CAMERA_MAX_POINTS))
				{
					pointsTarget[numTarget].Set(vec);
					numTarget++;
				}
			}

			bool Camera::SetCamType(const Camera::CameraType &type)
			{
				if (type == FirstPerson)
				{
					targetMode = FirstPerson;
					return true;
				}

				if (type == OnSphere)
				{
					targetMode = OnSphere;
					sumRato[0] = sumRato[1] = 0.0f;
					return true;
				}

				return false;
			}

			void Camera::SetSensitivity(const CameraInput &input, const float s)
			{
				if (input == Keyboard)
					keyS = s;
				if (input == Mouse)
					ratoS = s;
			}

			void Camera::SetPos(const HorseRadish::Vector &pos)
			{
				camPos.Set(pos);
				modelView.SetGLModelView(camPos, GetTarget(), camUp);
			}

			void Camera::SetPos(const float x, const float y, const float z)
			{
				camPos.Set(x, y, z);
				modelView.SetGLModelView(camPos, GetTarget(), camUp);
			}

			void Camera::SetTarget(const HorseRadish::Vector &target)
			{
				camDir.StoreVector(target, camPos);
				camDir.Normalize();
				absFocus = target.GetDistance(camPos);
				modelView.SetGLModelView(camPos, GetTarget(), camUp);
			}

			void Camera::SetTarget(const float x, const float y, const float z)
			{
				camDir.StoreVector(HorseRadish::Vector(x, y, z), camPos);
				camDir.Normalize();
				absFocus = camPos.GetDistance(x, y, z);
				modelView.SetGLModelView(camPos, GetTarget(), camUp);
			}

			void Camera::SetDir(const HorseRadish::Vector &direction)
			{
				camDir.Set(direction);
				camDir.Normalize();
				modelView.SetGLModelView(camPos, GetTarget(), camUp);
			}

			void Camera::SetDir(const float x, const float y, const float z)
			{
				camDir.Set(x, y, z);
				camDir.Normalize();
				modelView.SetGLModelView(camPos, GetTarget(), camUp);
			}

			void Camera::SetAbsoluteFocus(const float focus)
			{
				absFocus = focus;
			}

			void Camera::SetOnSphereDists(const float minDist, const float maxDist)
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

			void Camera::GetPos(HorseRadish::Vector &pos) const
			{
				pos.Set(camPos);
			}

			HorseRadish::Vector Camera::GetPos() const
			{
				return HorseRadish::Vector(camPos);
			}

			void Camera::GetTarget(HorseRadish::Vector &target) const
			{
				target = camDir;
				target *= absFocus;
				target += camPos;
			}

			HorseRadish::Vector Camera::GetTarget() const
			{
				HorseRadish::Vector target;

				target = camDir;
				target *= absFocus;
				target += camPos;
				return target;
			}

			void Camera::GetRay(HorseRadish::Ray &ray) const
			{
				ray.SetOrigin(camPos);
				ray.SetDirection(camDir);
			}

			void Camera::GetViewDir(HorseRadish::Vector &dir) const
			{
				dir.Set(camDir);
			}

			HorseRadish::Vector Camera::GetViewDir() const
			{
				return HorseRadish::Vector(camDir);
			}


			void Camera::GetStrideDir(HorseRadish::Vector &dir) const
			{
				HorseRadish::Vector auxVec;

				auxVec = camDir;
				auxVec.y += 1.0f;

				dir.StoreCrossProduct(camDir, auxVec);
				dir.Normalize();
			}

			HorseRadish::Vector Camera::GetStrideDir() const
			{
				HorseRadish::Vector strideDir;

				GetStrideDir(strideDir);
				return strideDir;
			}

			float Camera::GetFocalDist() const
			{
				return absFocus;
			}

			float Camera::GetSensitivity(const CameraInput &input) const
			{
				if (input & Keyboard)
					return keyS;
				if (input & Mouse)
					return ratoS;
				return 0.0f;
			}

			const float* Camera::GetModelView() const
			{
				return modelView;
			}

			void Camera::GetModelView(float * const mat) const
			{
				modelView.Write(mat);
			}

			Camera::CameraType Camera::GetTargetMode() const
			{
				return targetMode;
			}

		} //Tools
	} //OpenGL
} //HorseRadish