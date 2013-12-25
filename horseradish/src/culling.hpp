#pragma once
#ifndef __SCULLING__
#define __SCULLING__

#include "common\Common.hpp"
#include "common\Containers.hpp"
#include "common\opengl\tools.hpp"

#define SCULLING_MAX_AREA_OBJECTS	3

typedef void (APIENTRY * SCULLING_DEBUG_CALLBACK_AREA)	(const bool areaOfCamera, const HorseRadish::BBox &bbox);
typedef void (APIENTRY * SCULLING_DEBUG_CALLBACK_PORTAL)(const HorseRadish::Vector &p1, const HorseRadish::Vector &p2, const HorseRadish::Vector &p3, const HorseRadish::Vector &p4);

//§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§
//§§§§   -= Classe SCULLING =-	§§§§
//§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§
class SCULLING
{
	//estruturas internas
	struct AREA;
	struct PORTAL;
	struct BNODE;

	struct AREA{
		HorseRadish::Containers::Array<void*> data[SCULLING_MAX_AREA_OBJECTS];
		HorseRadish::Containers::Array<PORTAL*> portals;
		HorseRadish::BBox bbox;
	};
	struct PORTAL{
		HorseRadish::Vector pontos[4];
		AREA *neg,*pos;
	};
	struct BNODE{
		HorseRadish::Plane plano;
		unsigned char flag;
		void *pos,*neg;
	};

	//variáveis internas
	HorseRadish::Containers::Array<BNODE> bsp;
	HorseRadish::Containers::Array<AREA> areas;
	HorseRadish::Containers::Array<PORTAL> portals;
	int numBSPNodes,numAreas,numPortals;
	mutable int currentDepth;

	//métodos internos
	bool addPortal2Area(AREA * const area, PORTAL * const portal);
	int recurseGetArea(const BNODE * const tree, const HorseRadish::Vector &ponto) const;
	void getDataFromArea(const AREA * const area, void *** const output, unsigned int * const outputCount, const int outputTypeCount) const;
	
	void recurseAreaPortal(const AREA * const area, const PORTAL * const portalEnter, const HorseRadish::OpenGL::Tools::Frustum * const frustum, void *** const output, unsigned int * const outputCount, const int &outputTypeCount) const;
	void recurseAreaPortalDebug(const AREA * const area, const PORTAL * const portalEnter, const HorseRadish::OpenGL::Tools::Frustum * const frustum, SCULLING_DEBUG_CALLBACK_AREA cbArea, SCULLING_DEBUG_CALLBACK_PORTAL cbPortal) const;

public:
	//métodos públicos
	SCULLING();
	~SCULLING();

	//para adicionar todos os elementos necessários
	bool createBSPNodes(const int numNodes);
	bool createAreas(const int numAreas);
	bool createPortal(const unsigned int positiveArea, const unsigned int negativeArea, const HorseRadish::Vector &p1, const HorseRadish::Vector &p2, const HorseRadish::Vector &p3, const HorseRadish::Vector &p4);
	void setPortals2Areas();
	void setBSPNodeData(const unsigned int nodeIndex, const HorseRadish::Plane &plane, const int positiveChildData, const int negativeChildData);

	//para realizar operações sobre dados das áreas
	void areaSetBBox(const unsigned int areaIndex, const HorseRadish::BBox &bbox);
	void areaMergeBBox(const unsigned int areaIndex, const HorseRadish::BBox &bbox);
	bool areaAddData(const unsigned int areaIndex, const unsigned int typeObject, void* const dataObject);
	void areaClearData(const unsigned int areaIndex, const unsigned int typeObject);
	int areaGetTypeCount(const unsigned int areaIndex, const unsigned int typeObject) const;
	void* areaGetTypeData(const unsigned int areaIndex, const unsigned int typeObject, const unsigned int dataIndex) const;
	void areaGetBBox(const unsigned int areaIndex, HorseRadish::BBox * const bbox) const;

	//atravessa a árvore e portais, gravando os dados a usar
	void transverse(const HorseRadish::OpenGL::Tools::Frustum * const frustum, void*** const output, unsigned int * const outputCount, const int outputTypeCount) const;
	void transverseDebug(const HorseRadish::OpenGL::Tools::Frustum * const frustum, SCULLING_DEBUG_CALLBACK_AREA cbArea, SCULLING_DEBUG_CALLBACK_PORTAL cbPortal) const;
	
	//operações para retornar dados
	int getNumPortals() const {return numPortals;}
	int getNumAreas() const {return numAreas;}
	int getNumBSPNodes() const	{return numBSPNodes;}
	int getArea(const HorseRadish::Vector &point, const bool BSPTree) const;
	int getAreaClosest(const HorseRadish::Vector &point) const;
};

#endif