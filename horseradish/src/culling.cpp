#include "culling.hpp"

#include "common\platform.hpp"

#include <stdio.h>

#define BSP_POS_OPAQUE		(1<<0)
#define BSP_POS_AREA		(1<<1)
#define BSP_POS_NODE		(1<<2)
#define BSP_NEG_OPAQUE		(1<<3)
#define BSP_NEG_AREA		(1<<4)
#define BSP_NEG_NODE		(1<<5)

#define TRANSVERSE_MAX_DEPTH 5

bool SCULLING::addPortal2Area(AREA * const area, PORTAL * const portal)
{
	area->portals.push_back(portal);
	return true;
}

int SCULLING::recurseGetArea(const BNODE * const tree, const HorseRadish::Vector &ponto) const
{
	if (tree->plano.ClassifyPoint(ponto) == HorseRadish::Plane::Position::BEHIND)
	{
		if (tree->flag & BSP_NEG_OPAQUE)
			return -1;
		if (tree->flag & BSP_NEG_AREA)
			return (((AREA*)tree->neg) - this->areas.data());

		return recurseGetArea((BNODE*)tree->neg, ponto);
	}

	if (tree->flag & BSP_POS_OPAQUE)
		return -1;
	if (tree->flag & BSP_POS_AREA)
		return (((AREA*)tree->pos) - this->areas.data());

	return recurseGetArea((BNODE*)tree->pos, ponto);
}

void SCULLING::getDataFromArea(const AREA * const area, void *** const output, unsigned int * const outputCount, const int outputTypeCount) const
{
	for (int i = 0; i < outputTypeCount; i++)
	{
		if (area->data[i].empty())
			continue;

		memcpy(output[i] + outputCount[i], area->data[i].data(), area->data[i].size() * sizeof(void*));
		outputCount[i] += area->data[i].size();
	}
}

void SCULLING::recurseAreaPortal(const AREA * const area, const PORTAL * const portalEnter, const HorseRadish::OpenGL::Tools::Frustum * const frustum, void *** const output, unsigned int * const outputCount, const int &outputTypeCount) const
{
	if (currentDepth >= TRANSVERSE_MAX_DEPTH)
		return;

	currentDepth++;

	for (auto& curPortal : area->portals)
	{
		if (curPortal == portalEnter)
			continue;

		if (frustum->testSquare(curPortal->pontos) == false)
			continue;

		const AREA *moveArea;
		HorseRadish::OpenGL::Tools::Frustum newFrustum;
		HorseRadish::Plane novoNear;

		moveArea = curPortal->neg;
		if (curPortal->neg == area)
			moveArea = curPortal->pos;

		getDataFromArea(moveArea, output, outputCount, outputTypeCount);

		//novoNear.SetFromPoints(area->portals[curPortal]->pontos[0],area->portals[curPortal]->pontos[1],area->portals[curPortal]->pontos[2]);
		newFrustum.SetFrustum(frustum);
		/*newFrustum.SetIndividualPlane(SFRUSTUM::PLANE_NEAR,novoNear);
		if (newFrustum.DotNormals(SFRUSTUM::PLANE_NEAR,SFRUSTUM::PLANE_FAR)>0.0f)
		{
		novoNear.NegateNormal();
		newFrustum.SetIndividualPlane(SFRUSTUM::PLANE_NEAR,novoNear);
		}*/

		recurseAreaPortal(moveArea, curPortal, &newFrustum, output, outputCount, outputTypeCount);
	}

	currentDepth--;
}

void SCULLING::recurseAreaPortalDebug(const AREA * const area, const PORTAL * const portalEnter, const HorseRadish::OpenGL::Tools::Frustum * const frustum, SCULLING_DEBUG_CALLBACK_AREA cbArea, SCULLING_DEBUG_CALLBACK_PORTAL cbPortal) const
{
	if (currentDepth >= TRANSVERSE_MAX_DEPTH)
		return;

	currentDepth++;

	for (auto& curPortal : area->portals)
	{
		if (curPortal == portalEnter)
			continue;

		if (frustum->testSquare(curPortal->pontos) == false)
			continue;

		if (cbPortal)
			cbPortal(curPortal->pontos[0], curPortal->pontos[1], curPortal->pontos[2], curPortal->pontos[3]);

		const AREA *moveArea;
		HorseRadish::OpenGL::Tools::Frustum newFrustum;
		HorseRadish::Plane novoNear;

		moveArea = curPortal->neg;
		if (curPortal->neg == area)
			moveArea = curPortal->pos;

		//novoNear.SetFromPoints(area->portals[curPortal]->pontos[0],area->portals[curPortal]->pontos[1],area->portals[curPortal]->pontos[2]);
		newFrustum.SetFrustum(frustum);
		/*newFrustum.SetIndividualPlane(SFRUSTUM::PLANE_NEAR,novoNear);
		if (newFrustum.DotNormals(SFRUSTUM::PLANE_NEAR,SFRUSTUM::PLANE_FAR)>0.0f)
		{
		novoNear.NegateNormal();
		newFrustum.SetIndividualPlane(SFRUSTUM::PLANE_NEAR,novoNear);
		}*/

		if (cbArea)
			cbArea(false, moveArea->bbox);

		recurseAreaPortalDebug(moveArea, curPortal, &newFrustum, cbArea, cbPortal);
	}

	currentDepth--;
}

/*§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§
§§§§§§   -= Classe SCULLING =-	§§§§§§
§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§*/
SCULLING::SCULLING()
{
}

SCULLING::~SCULLING()
{
	//basta limpar isto
	this->bsp.clear();
	this->areas.clear();
	this->portals.clear();
}

bool SCULLING::createBSPNodes(const int numNodes)
{
	//verificar argumento
	if (numNodes <= 0)
		return false;

	//apago coisas antigas se houver
	this->bsp.clear();

	//crio os novos nós
	this->bsp.reserve(numNodes);
	return true;
}

bool SCULLING::createAreas(const int numberAreas)
{
	if (numberAreas <= 0)
		return false;

	this->areas.clear();
	this->areas.reserve(numberAreas);
	return true;
}

bool SCULLING::createPortal(const unsigned int positiveArea, const unsigned int negativeArea, const HorseRadish::Vector &p1, const HorseRadish::Vector &p2, const HorseRadish::Vector &p3, const HorseRadish::Vector &p4)
{
	if (positiveArea < 0 || negativeArea < 0 || positiveArea >= this->areas.size() || negativeArea >= this->areas.size())
		return false;

	this->portals.push_back(PORTAL());
	auto& newPortal = this->portals[this->portals.size() - 1];

	newPortal.pos = &this->areas[positiveArea];
	newPortal.neg = &this->areas[negativeArea];
	newPortal.pontos[0] = p1;
	newPortal.pontos[1] = p2;
	newPortal.pontos[2] = p3;
	newPortal.pontos[3] = p4;

	return true;
}

void SCULLING::setPortals2Areas()
{
	for(auto& curPortal : this->portals)
	{
		addPortal2Area(curPortal.neg, &curPortal);
		addPortal2Area(curPortal.pos, &curPortal);
	}
}

void SCULLING::setBSPNodeData(const unsigned int nodeIndex, const HorseRadish::Plane &plane, const int positiveChildData, const int negativeChildData)
{
	if (nodeIndex >= this->bsp.size())
		return;

	auto target = &bsp[nodeIndex];
	target->plano = plane;

	if ((positiveChildData>0) && (positiveChildData < this->bsp.size()))
	{
		target->flag |= BSP_POS_NODE;
		target->pos = &this->bsp[positiveChildData];
	}
	else if ((positiveChildData < 0) && ((-positiveChildData - 1) < this->areas.size()))
	{
		target->flag |= BSP_POS_AREA;
		target->pos = &this->areas[-positiveChildData - 1];
	}
	else
	{
		target->flag |= BSP_POS_OPAQUE;
		target->pos = nullptr;;
	}

	if ((negativeChildData > 0) && (negativeChildData < this->bsp.size()))
	{
		target->flag |= BSP_NEG_NODE;
		target->neg = &this->bsp[negativeChildData];
	}
	else if ((negativeChildData < 0) && ((-negativeChildData - 1) < this->areas.size()))
	{
		target->flag |= BSP_NEG_AREA;
		target->neg = &this->areas[-negativeChildData - 1];
	}
	else
	{
		target->flag |= BSP_NEG_OPAQUE;
		target->neg = nullptr;;
	}
}

void SCULLING::areaSetBBox(const unsigned int areaIndex, const HorseRadish::BBox &bbox)
{
	if (areaIndex >= this->areas.size())
		return;
	areas[areaIndex].bbox=bbox;
}

void SCULLING::areaMergeBBox(const unsigned int areaIndex, const HorseRadish::BBox &bbox)
{
	if (areaIndex >= this->areas.size())
		return;

	areas[areaIndex].bbox+=bbox;
}

bool SCULLING::areaAddData(const unsigned int areaIndex, const unsigned int typeObject, void* const dataObject)
{
	if ((areaIndex >= this->areas.size()) || (typeObject>SCULLING_MAX_AREA_OBJECTS))
		return false;

	this->areas[areaIndex].data[typeObject].push_back(dataObject);
	return true;
}

void SCULLING::areaClearData(const unsigned int areaIndex, const unsigned int typeObject)
{
	if ((areaIndex >= this->areas.size()) || (typeObject>SCULLING_MAX_AREA_OBJECTS))
		return;

	this->areas[areaIndex].data[typeObject].clear();
}

int SCULLING::areaGetTypeCount(const unsigned int areaIndex, const unsigned int typeObject) const
{
	if ((areaIndex >= this->areas.size()) || (typeObject>SCULLING_MAX_AREA_OBJECTS))
		return 0;

	return this->areas[areaIndex].data[typeObject].size();
}

void* SCULLING::areaGetTypeData(const unsigned int areaIndex, const unsigned int typeObject, const unsigned int dataIndex) const
{
	if ((areaIndex >= this->areas.size()) || (typeObject>SCULLING_MAX_AREA_OBJECTS))
		return 0;
	if (dataIndex >= areas[areaIndex].data[typeObject].size())
		return 0;

	return this->areas[areaIndex].data[typeObject][dataIndex];
}

void SCULLING::areaGetBBox(const unsigned int areaIndex, HorseRadish::BBox * const bbox) const
{
	if (areaIndex >= this->areas.size() || bbox == nullptr)
		return;
	bbox->Set(&areas[areaIndex].bbox);
}

void SCULLING::transverse(const HorseRadish::OpenGL::Tools::Frustum * const frustum, void*** const output, unsigned int * const outputCount, const int outputTypeCount) const
{
	if (this->areas.empty() || output == nullptr || outputCount == nullptr || outputTypeCount <= 0 || outputTypeCount > SCULLING_MAX_AREA_OBJECTS)
		return;

	for (int i = 0; i < outputTypeCount; i++)
		outputCount[i] = 0;

	auto startArea = getArea(frustum->getCamPosition(), true);
	if (startArea == -1)
		return;

	getDataFromArea(&areas[startArea], output, outputCount, outputTypeCount);

	currentDepth = 1;

	recurseAreaPortal(&areas[startArea], nullptr, frustum, output, outputCount, outputTypeCount);
}

void SCULLING::transverseDebug(const HorseRadish::OpenGL::Tools::Frustum * const frustum, SCULLING_DEBUG_CALLBACK_AREA cbArea, SCULLING_DEBUG_CALLBACK_PORTAL cbPortal) const
{
	if (this->areas.empty() || cbArea == nullptr || cbPortal == nullptr)
		return;

	auto startArea = getArea(frustum->getCamPosition(), true);
	if (startArea == -1)
		return;

	if (cbArea)
		cbArea(true, areas[startArea].bbox);

	currentDepth = 1;

	recurseAreaPortalDebug(&areas[startArea], nullptr, frustum, cbArea, cbPortal);
}

int SCULLING::getArea(const HorseRadish::Vector &point, const bool useBSPTree) const
{
	if (useBSPTree == true)
		return recurseGetArea(this->bsp.data(), point);

	const AREA *areaWalker = this->areas.data();
	for (int i = 0; i < this->areas.size(); i++, areaWalker++)
	{
		if (areaWalker->bbox.ContainsPoint(point))
			return i;
	}

	return -1;
}

int SCULLING::getAreaClosest(const HorseRadish::Vector &point) const
{
	HorseRadish::Vector pontoBBox;
	float closestDist;
	int closestArea;

	closestDist = HorseRadish::Math::INFINITY;

	const AREA *areaWalker = this->areas.data();
	for (int i = 0; i < this->areas.size(); i++, areaWalker++)
	{
		areaWalker->bbox.GetCenter(pontoBBox);
		if (pontoBBox.GetDistance(point) < closestDist)
		{
			closestDist = pontoBBox.GetDistance(point);
			closestArea = i;
		}
	}

	return closestArea;
}