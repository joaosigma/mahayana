#include "common\platform.hpp"

#include <windows.h>
#include <stdio.h>

#include "culling.hpp"

/*§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§
§§§§§§   -= Algumas coisa locais =-	§§§§§§
§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§*/
#define BSP_POS_OPAQUE		(1<<0)
#define BSP_POS_AREA		(1<<1)
#define BSP_POS_NODE		(1<<2)
#define BSP_NEG_OPAQUE		(1<<3)
#define BSP_NEG_AREA		(1<<4)
#define BSP_NEG_NODE		(1<<5)

#define TRANSVERSE_MAX_DEPTH 5

/*§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§
§§§§§§   -= Classe SCULLING =-	§§§§§§
§§§§§§      métodos internos	§§§§§§
§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§*/
bool SCULLING::addPortal2Area(AREA * const area, PORTAL * const portal)
{
	return area->portals.Add(portal);
}

int SCULLING::recurseGetArea(const BNODE * const tree, const HorseRadish::Vector &ponto) const
{
	//se estou do lado negativo
	if (tree->plano.ClassifyPoint(ponto) == HorseRadish::Plane::BEHIND)
	{
		if (tree->flag & BSP_NEG_OPAQUE)
			return -1;
		if (tree->flag & BSP_NEG_AREA)
			return (((AREA*)tree->neg) - this->areas.GetMainPointer());

		return recurseGetArea((BNODE*)tree->neg, ponto);
	}

	//estou do lado positivo
	if (tree->flag & BSP_POS_OPAQUE)
		return -1;
	if (tree->flag & BSP_POS_AREA)
		return (((AREA*)tree->pos) - this->areas.GetMainPointer());

	return recurseGetArea((BNODE*)tree->pos, ponto);
}

void SCULLING::getDataFromArea(const AREA * const area, void *** const output, unsigned int * const outputCount, const int outputTypeCount) const
{
	//para cada tipo de dados
	for(int i=0; i<outputTypeCount; i++)
	{
		//se nada tenho
		if (area->data[i].GetNumElements() <= 0)
			continue;

		//gravo as coisas
		memcpy(output[i]+outputCount[i], area->data[i].GetMainPointer(), area->data[i].GetSize());
		outputCount[i]+=area->data[i].GetNumElements();
	}
}

void SCULLING::recurseAreaPortal(const AREA * const area, const PORTAL * const portalEnter, const HorseRadish::OpenGL::Tools::Frustum * const frustum, void *** const output, unsigned int * const outputCount, const int &outputTypeCount) const
{
	//devo verificar se já não desci demais
	if (currentDepth >= TRANSVERSE_MAX_DEPTH)
		return;

	//entrei num nível
	currentDepth++;

	//pra cada portal
	for(int curPortal=0; curPortal<area->portals.GetNumElements(); curPortal++)
		{
		//se entrei por este portal, não vale a pena sequer pensar nele
		if (area->portals[curPortal]==portalEnter)
			continue;

		//se este portal não está no frustum, posso bazar
		if (frustum->testSquare(area->portals[curPortal]->pontos)==false)
			continue;

		//vejo o portal, logo tenho de ter em consideração a outra área dele
		const AREA *moveArea;
		HorseRadish::OpenGL::Tools::Frustum newFrustum;
		HorseRadish::Plane novoNear;

		//tenho de escolher a area correcta (se não voltava a entrar onde já estou)
		moveArea = area->portals[curPortal]->neg;
		if (area->portals[curPortal]->neg == area)
			moveArea = area->portals[curPortal]->pos;

		//coloco os dados desta área
		getDataFromArea(moveArea,output,outputCount,outputTypeCount);

		//o novo frustum vai ser igual ao que estou a usar, mas o NEAR para a ser o plano do quadrado
		//novoNear.SetFromPoints(area->portals[curPortal]->pontos[0],area->portals[curPortal]->pontos[1],area->portals[curPortal]->pontos[2]);
		newFrustum.SetFrustum(frustum);
		/*newFrustum.SetIndividualPlane(SFRUSTUM::PLANE_NEAR,novoNear);
		if (newFrustum.DotNormals(SFRUSTUM::PLANE_NEAR,SFRUSTUM::PLANE_FAR)>0.0f)
			{
			novoNear.NegateNormal();
			newFrustum.SetIndividualPlane(SFRUSTUM::PLANE_NEAR,novoNear);
			}*/
		
		//posso avançar recursivamente para dentro dela
		recurseAreaPortal(moveArea,area->portals[curPortal],&newFrustum,output,outputCount,outputTypeCount);
		}

	//vou sair, logo baixo um nível
	currentDepth--;
}

void SCULLING::recurseAreaPortalDebug(const AREA * const area, const PORTAL * const portalEnter, const HorseRadish::OpenGL::Tools::Frustum * const frustum, SCULLING_DEBUG_CALLBACK_AREA cbArea, SCULLING_DEBUG_CALLBACK_PORTAL cbPortal) const
{
	//devo verificar se já não desci demais
	if (currentDepth >= TRANSVERSE_MAX_DEPTH)
		return;

	//entrei num nível
	currentDepth++;

	//pra cada portal
	for(int curPortal=0; curPortal<area->portals.GetNumElements(); curPortal++)
		{
		//se entrei por este portal, não vale a pena sequer pensar nele
		if (area->portals[curPortal]==portalEnter)
			continue;

		//se este portal não está no frustum, posso bazar
		if (frustum->testSquare(area->portals[curPortal]->pontos)==false)
			continue;

		//vou usar este portal
		if (cbPortal)
			cbPortal(area->portals[curPortal]->pontos[0],area->portals[curPortal]->pontos[1],area->portals[curPortal]->pontos[2],area->portals[curPortal]->pontos[3]);

		//vejo o portal, logo tenho de ter em consideração a outra área dele
		const AREA *moveArea;
		HorseRadish::OpenGL::Tools::Frustum newFrustum;
		HorseRadish::Plane novoNear;

		//tenho de escolher a area correcta (se não voltava a entrar onde já estou)
		moveArea = area->portals[curPortal]->neg;
		if (area->portals[curPortal]->neg == area)
			moveArea = area->portals[curPortal]->pos;

		//o novo frustum vai ser igual ao que estou a usar, mas o NEAR para a ser o plano do quadrado
		//novoNear.SetFromPoints(area->portals[curPortal]->pontos[0],area->portals[curPortal]->pontos[1],area->portals[curPortal]->pontos[2]);
		newFrustum.SetFrustum(frustum);
		/*newFrustum.SetIndividualPlane(SFRUSTUM::PLANE_NEAR,novoNear);
		if (newFrustum.DotNormals(SFRUSTUM::PLANE_NEAR,SFRUSTUM::PLANE_FAR)>0.0f)
			{
			novoNear.NegateNormal();
			newFrustum.SetIndividualPlane(SFRUSTUM::PLANE_NEAR,novoNear);
			}*/
		
		//vou usar esta area
		if (cbArea)
			cbArea(false,moveArea->bbox);

		//posso avançar recursivamente para dentro dela
		recurseAreaPortalDebug(moveArea,area->portals[curPortal],&newFrustum,cbArea,cbPortal);
		}

	//vou sair, logo baixo um nível
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
	this->bsp.Clear();
	this->areas.Clear();
	this->portals.Clear();
}

bool SCULLING::createBSPNodes(const int numNodes)
{
	//verificar argumento
	if (numNodes <= 0)
		return false;

	//apago coisas antigas se houver
	this->bsp.Clear();

	//crio os novos nós
	return this->bsp.Reserve(numNodes);
}

bool SCULLING::createAreas(const int numberAreas)
{
	//verificar argumento
	if (numberAreas <= 0)
		return false;

	//apago coisas antigas se houver
	this->areas.Clear();

	//crio as novas áreas
	return this->areas.Reserve(numberAreas);
}

bool SCULLING::createPortal(const unsigned int positiveArea, const unsigned int negativeArea, const HorseRadish::Vector &p1, const HorseRadish::Vector &p2, const HorseRadish::Vector &p3, const HorseRadish::Vector &p4)
{
	PORTAL *newPortal;

	//verificar argumentos
	if (positiveArea < 0 || negativeArea < 0 || positiveArea >= this->areas.GetNumElements() || negativeArea >= this->areas.GetNumElements())
		return false;

	//crio mais um portal
	newPortal = this->portals.Add();
	if (newPortal == nullptr)
		return false;

	//arranjo os valores
	newPortal->pos = this->areas + positiveArea;
	newPortal->neg = this->areas + negativeArea;
	newPortal->pontos[0]=p1;
	newPortal->pontos[1]=p2;
	newPortal->pontos[2]=p3;
	newPortal->pontos[3]=p4;

	//e posso sair
	return true;
}

void SCULLING::setPortals2Areas()
{
	PORTAL *portal;

	//para cada portal, associo as duas àreas ao respectivo portal
	portal = this->portals.GetMainPointer();
	for(int i=0; i<this->portals.GetNumElements(); i++,portal++)
	{
		addPortal2Area(portal->neg, portal);
		addPortal2Area(portal->pos, portal);
	}
}

void SCULLING::setBSPNodeData(const unsigned int nodeIndex, const HorseRadish::Plane &plane, const int positiveChildData, const int negativeChildData)
{
	BNODE *target;

	//dá jeito verificar isto
	if (nodeIndex >= this->bsp.GetNumElements())
		return;

	//pra onde devo escrever e escrevo logo o plano
	target=bsp+nodeIndex;
	target->plano=plane;

	//se o filho positivo for maior que 0, é filho, se for negativo é area, senão é área fechada
	if ( (positiveChildData>0) && (positiveChildData<this->bsp.GetNumElements()))
	{
		target->flag |= BSP_POS_NODE;
		target->pos = this->bsp + positiveChildData;
	}
	else if ( (positiveChildData<0) && ((-positiveChildData-1)<this->areas.GetNumElements()))
	{
		target->flag |= BSP_POS_AREA;
		target->pos = this->areas + (- positiveChildData - 1);
	}
	else
	{
		target->flag |= BSP_POS_OPAQUE;
		target->pos = nullptr;;
	}

	//o mesmo acontece para o filho negativo
	if ( (negativeChildData>0) && (negativeChildData<this->bsp.GetNumElements()))
	{
		target->flag |= BSP_NEG_NODE;
		target->neg = this->bsp + negativeChildData;
	}
	else if ( (negativeChildData<0) && ((-negativeChildData-1)<this->areas.GetNumElements()))
	{
		target->flag |= BSP_NEG_AREA;
		target->neg = this->areas + (- negativeChildData - 1);
	}
	else
	{
		target->flag |= BSP_NEG_OPAQUE;
		target->neg = nullptr;;
	}
}

void SCULLING::areaSetBBox(const unsigned int areaIndex, const HorseRadish::BBox &bbox)
{
	if (areaIndex>=this->areas.GetNumElements())
		return;
	areas[areaIndex].bbox=bbox;
}

void SCULLING::areaMergeBBox(const unsigned int areaIndex, const HorseRadish::BBox &bbox)
{
	if (areaIndex>=this->areas.GetNumElements())
		return;
	areas[areaIndex].bbox+=bbox;
}

bool SCULLING::areaAddData(const unsigned int areaIndex, const unsigned int typeObject, void* const dataObject)
{
	//verificar algumas coisas
	if ( (areaIndex>=this->areas.GetNumElements()) || (typeObject>SCULLING_MAX_AREA_OBJECTS) )
		return false;

	//posso escrever
	return (this->areas[areaIndex].data[typeObject].Add(dataObject) == dataObject);
}

void SCULLING::areaClearData(const unsigned int areaIndex, const unsigned int typeObject)
{
	if ( (areaIndex>=this->areas.GetNumElements()) || (typeObject>SCULLING_MAX_AREA_OBJECTS) )
		return;

	//basta mandar limpar
	this->areas[areaIndex].data[typeObject].Clear();
}

int SCULLING::areaGetTypeCount(const unsigned int areaIndex, const unsigned int typeObject) const
{
	if ( (areaIndex>=this->areas.GetNumElements()) || (typeObject>SCULLING_MAX_AREA_OBJECTS) )
		return 0;

	return this->areas[areaIndex].data[typeObject].GetNumElements();
}

void* SCULLING::areaGetTypeData(const unsigned int areaIndex, const unsigned int typeObject, const unsigned int dataIndex) const
{
	if ( (areaIndex>=this->areas.GetNumElements()) || (typeObject>SCULLING_MAX_AREA_OBJECTS) )
		return 0;
	if (dataIndex>=areas[areaIndex].data[typeObject].GetNumElements())
		return 0;

	return this->areas[areaIndex].data[typeObject][dataIndex];
}

void SCULLING::areaGetBBox(const unsigned int areaIndex, HorseRadish::BBox * const bbox) const
{
	if (areaIndex>=this->areas.GetNumElements() || bbox==nullptr)
		return;
	bbox->Set(&areas[areaIndex].bbox);
}

void SCULLING::transverse(const HorseRadish::OpenGL::Tools::Frustum * const frustum, void*** const output, unsigned int * const outputCount, const int outputTypeCount) const
{
	int startArea;

	//verificar os parametros
	if (this->areas.GetNumElements()<=0 || output==nullptr || outputCount==nullptr || outputTypeCount<=0 || outputTypeCount>SCULLING_MAX_AREA_OBJECTS)
		return;

	//limpo isto
	for(int i=0; i<outputTypeCount; i++)
		outputCount[i]=0;

	//acho a primeira área onde estou
	startArea=getArea(frustum->getCamPosition(),true);
	if (startArea==-1)
		return;

	//posso já arranjar os dados desta área
	getDataFromArea(areas+startArea,output,outputCount,outputTypeCount);

	//começo em zero, claro está
	currentDepth = 1;

	//atravesso a árvore
	recurseAreaPortal(areas+startArea,nullptr,frustum,output,outputCount,outputTypeCount);
}

void SCULLING::transverseDebug(const HorseRadish::OpenGL::Tools::Frustum * const frustum, SCULLING_DEBUG_CALLBACK_AREA cbArea, SCULLING_DEBUG_CALLBACK_PORTAL cbPortal) const
{
	int startArea;

	//verificar os parametros
	if (this->areas.GetNumElements()<=0 || cbArea==nullptr || cbPortal==nullptr)
		return;

	//acho a primeira área onde estou
	startArea=getArea(frustum->getCamPosition(),true);
	if (startArea==-1)
		return;

	//entrei nesta area
	if (cbArea)
		cbArea(true,areas[startArea].bbox);

	//começo em zero, claro está
	currentDepth = 1;

	//atravesso a árvore
	recurseAreaPortalDebug(areas+startArea,nullptr,frustum,cbArea,cbPortal);
}

int SCULLING::getArea(const HorseRadish::Vector &point, const bool useBSPTree) const
{	
	//se for para usar a bsp
	if (useBSPTree == true)
		return recurseGetArea(this->bsp.GetMainPointer(), point);
		
	//chegando aqui é para verificar pelas áreas
	AREA *areaWalker;

	//tenho de passar por todas as áreas e verificar em qual estou
	areaWalker = this->areas.GetMainPointer();
	for(int i=0; i<this->areas.GetNumElements(); i++,areaWalker++)
	{
		if (areaWalker->bbox.ContainsPoint(point))
			return i;
	}

	//não estou em nenhuma
	return -1;
}

int SCULLING::getAreaClosest(const HorseRadish::Vector &point) const
{
	HorseRadish::Vector pontoBBox;
	AREA *areaWalker;
	float closestDist;
	int closestArea;

	//tenho de passar por todas as áreas e verificar em qual estou
	closestDist=HorseRadish::Math::INFINITY;
	areaWalker = this->areas.GetMainPointer();
	for(int i=0; i<this->areas.GetNumElements(); i++, areaWalker++)
	{
		areaWalker->bbox.GetCenter(pontoBBox);
		if (pontoBBox.GetDist(point)<closestDist)
		{
			closestDist=pontoBBox.GetDist(point);
			closestArea=i;
		}
	}

	//é esta área
	return closestArea;
}