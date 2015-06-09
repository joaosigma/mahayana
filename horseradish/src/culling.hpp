#pragma once

#include "common\Vector.hpp"
#include "common\opengl\tools\frustum.hpp"

#include <array>
#include <vector>
#include <functional>

class SCULLING
{
public:
	static const int MaxAreaObjects = 3;
	typedef std::function<void(const bool areaOfCamera, const HorseRadish::BBox &bbox)> SCULLING_DEBUG_CALLBACK_AREA;
	typedef std::function<void(const HorseRadish::Vector &p1, const HorseRadish::Vector &p2, const HorseRadish::Vector &p3, const HorseRadish::Vector &p4)> SCULLING_DEBUG_CALLBACK_PORTAL;

private:
	struct AREA;
	struct PORTAL;
	struct BNODE;

	struct AREA{
		std::array<std::vector<void*>, SCULLING::MaxAreaObjects> data;
		std::vector<PORTAL*> portals;
		HorseRadish::BBox bbox;
	};
	struct PORTAL{
		HorseRadish::Vector pontos[4];
		AREA *neg, *pos;
	};
	struct BNODE{
		HorseRadish::Plane plano;
		unsigned char flag;
		void *pos, *neg;
	};

	std::vector<BNODE> bsp;
	std::vector<AREA> areas;
	std::vector<PORTAL> portals;
	int numBSPNodes, numAreas, numPortals;
	mutable int currentDepth;

	bool addPortal2Area(AREA * const area, PORTAL * const portal);
	int recurseGetArea(const BNODE * const tree, const HorseRadish::Vector &ponto) const;
	void getDataFromArea(const AREA * const area, void *** const output, unsigned int * const outputCount, const int outputTypeCount) const;

	void recurseAreaPortal(const AREA * const area, const PORTAL * const portalEnter, const HorseRadish::OpenGL::Tools::Frustum * const frustum, void *** const output, unsigned int * const outputCount, const int &outputTypeCount) const;
	void recurseAreaPortalDebug(const AREA * const area, const PORTAL * const portalEnter, const HorseRadish::OpenGL::Tools::Frustum * const frustum, SCULLING_DEBUG_CALLBACK_AREA cbArea, SCULLING_DEBUG_CALLBACK_PORTAL cbPortal) const;

public:
	SCULLING();
	~SCULLING();

	bool createBSPNodes(const int numNodes);
	bool createAreas(const int numAreas);
	bool createPortal(const unsigned int positiveArea, const unsigned int negativeArea, const HorseRadish::Vector &p1, const HorseRadish::Vector &p2, const HorseRadish::Vector &p3, const HorseRadish::Vector &p4);
	void setPortals2Areas();
	void setBSPNodeData(const unsigned int nodeIndex, const HorseRadish::Plane &plane, const int positiveChildData, const int negativeChildData);

	void areaSetBBox(const unsigned int areaIndex, const HorseRadish::BBox &bbox);
	void areaMergeBBox(const unsigned int areaIndex, const HorseRadish::BBox &bbox);
	bool areaAddData(const unsigned int areaIndex, const unsigned int typeObject, void* const dataObject);
	void areaClearData(const unsigned int areaIndex, const unsigned int typeObject);
	int areaGetTypeCount(const unsigned int areaIndex, const unsigned int typeObject) const;
	void* areaGetTypeData(const unsigned int areaIndex, const unsigned int typeObject, const unsigned int dataIndex) const;
	void areaGetBBox(const unsigned int areaIndex, HorseRadish::BBox * const bbox) const;

	void transverse(const HorseRadish::OpenGL::Tools::Frustum * const frustum, void*** const output, unsigned int * const outputCount, const int outputTypeCount) const;
	void transverseDebug(const HorseRadish::OpenGL::Tools::Frustum * const frustum, SCULLING_DEBUG_CALLBACK_AREA cbArea, SCULLING_DEBUG_CALLBACK_PORTAL cbPortal) const;

	int getNumPortals() const { return numPortals; }
	int getNumAreas() const { return numAreas; }
	int getNumBSPNodes() const	{ return numBSPNodes; }
	int getArea(const HorseRadish::Vector &point, const bool BSPTree) const;
	int getAreaClosest(const HorseRadish::Vector &point) const;
};
