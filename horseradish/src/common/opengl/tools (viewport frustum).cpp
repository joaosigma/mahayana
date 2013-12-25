#include "tools.hpp"

#include <math.h>

namespace HorseRadish
{

namespace OpenGL
{

namespace Tools
{

//§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§
//§§§§§§   Classe Viewport	§§§§§
//§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§
void Viewport::clearAll()
{
	fov=90.0f;
	zNear=1.0f;
	zFar=1000.0f;

	origin[0]=origin[1]=0; 
	dim[0]=800;
	dim[1]=600;
	projType=Proj3D;

	setMatrices();
}

void Viewport::setMatrices()
{
	double ymin,ymax,xmin,xmax,aspect,n,f;

	//dados auxiliares : 0.00872664625997164788461845384 = pi/360 = (pi/180)/2
	n = (double)zNear;
	f = (double)zFar;
	ymax = n * tan( ((double)fov) * 0.00872664625997164788461845384 );
	ymin = -ymax;
	aspect = ((double)dim[0])/((double)dim[1]);
	xmin = ymin * aspect;
	xmax = ymax * aspect;

	//3D
	mp3D.SetZero();
	mp3D[0]=(float)((2.0*n)/(xmax-xmin));
	mp3D[5]=(float)((2.0*n)/(ymax-ymin));
	mp3D[8]=(float)((xmax+xmin)/(xmax-xmin));
	mp3D[9]=(float)((ymax+ymin)/(ymax-ymin));
	mp3D[10]=-(float)((f+n)/(f-n));
	mp3D[11]=-1.0f;
	mp3D[14]=-(float)((2.0*f*n)/(f-n));

	//3D Infinite
	mpInfinite.SetZero();
	mpInfinite[0]=(float)((2.0*n)/(xmax-xmin));
	mpInfinite[5]=(float)((2.0*n)/(ymax-ymin));
	mpInfinite[8]=(float)((xmax+xmin)/(xmax-xmin));
	mpInfinite[9]=(float)((ymax+ymin)/(ymax-ymin));
	mpInfinite[10]=-1.0f;
	mpInfinite[11]=-1.0f;
	mpInfinite[14]=-(float)(2.0*n);

	//2D
	mp2D.SetIdentidade();
	mp2D[0]=2.0f/((float)dim[0]);
	mp2D[5]=2.0f/((float)dim[1]);
	mp2D[10]=-1.0f;
	mp2D[12]=-1.0f;
	mp2D[13]=-1.0f;

	//matrix corrente
	switch(projType){
		case Proj3D:		mCurrent.Set(mp3D);			break;
		case Proj3DInf:		mCurrent.Set(mpInfinite);	break;
		case Proj2D:		mCurrent.Set(mp2D);			break;
		default:
				mCurrent.Set(mp3D);
				break;
		}
}

Viewport::Viewport()
{
	//basta mandar limpar tudo
	clearAll();
}

Viewport::Viewport(const Viewport &viewport)
{
	origin[0]=viewport.origin[0];
	origin[1]=viewport.origin[1];
	dim[0]=viewport.dim[0];
	dim[1]=viewport.dim[1];

	fov=viewport.fov;
	zNear=viewport.zNear;
	zFar=viewport.zFar;
	projType=viewport.projType;

	setMatrices();
}

Viewport::Viewport(const int startX, const int startY, const int width, const int height)
{
	origin[0]=startX;
	origin[1]=startY;
	dim[0]=width;
	dim[1]=height;

	if (dim[0]<=0)	dim[0]=1;
	if (dim[1]<=0)	dim[1]=1;

	fov=90.0f;
	zNear=1.0f;
	zFar=1000.0f;
	projType=Proj3D;

	setMatrices();
}

Viewport::Viewport(const int width, const int height)
{
	origin[0]=0;
	origin[1]=0;
	dim[0]=width;
	dim[1]=height;

	if (dim[0]<=0)	dim[0]=1;
	if (dim[1]<=0)	dim[1]=1;

	fov=90.0f;
	zNear=1.0f;
	zFar=1000.0f;
	projType=Proj3D;

	setMatrices();
}

Viewport::~Viewport()
{
	//basta mandar limpar tudo
	clearAll();
}

void Viewport::setProjection(const Viewport::ProjectionType projType)
{
	//guardo o tipo e coloco a matriz correcta
	this->projType = projType;

	switch(this->projType)
	{
		case Proj3D:		this->mCurrent.Set(mp3D);			break;
		case Proj3DInf:		this->mCurrent.Set(mpInfinite);		break;
		case Proj2D:		this->mCurrent.Set(mp2D);			break;
		default:
				this->mCurrent.Set(mp3D);
				break;
	}
}

void Viewport::setViewport(const int width, const int height)
{
	dim[0]=width;
	dim[1]=height;

	if (dim[0]<=0)	dim[0]=1;
	if (dim[1]<=0)	dim[1]=1;

	setMatrices();
}

void Viewport::setFOV(const float fov)
{
	this->fov=fov;

	setMatrices();
}

void Viewport::setZRange(const float zNear, const float zFar)
{
	this->zNear=zNear;
	this->zFar=zFar;

	setMatrices();
}

const float * Viewport::getProjCurrent() const
{	return mCurrent;}

const float * Viewport::getProj2D() const
{	return mp2D;}

const float * Viewport::getProj3D() const
{	return mp3D;}

const float * Viewport::getProj3DInf() const
{	return mpInfinite;}

float Viewport::getFOV() const
{	return fov;}

float Viewport::getZNear() const
{	return zNear;}

float Viewport::getZFar() const
{	return zFar;}

void Viewport::getPointOnZNear(float * const vec) const
{
	vec[1] = zNear * tan(fov * 0.5f);
	vec[0] = (vec[1]) * ((float)dim[0]) / ((float)dim[1]);
    vec[2] = zNear;
}

void Viewport::getPointOnZNear(HorseRadish::Vector * const center) const
{
	center->y = zNear * tan(fov * 0.5f);
	center->x = (center->y) * ((float)dim[0]) / ((float)dim[1]);
    center->z = zNear;
}

void Viewport::getPointOnZNear(float * const x, float * const y, float * const z) const
{
	*y = zNear * tan(fov * 0.5f);
	*x = (*y) * ((float)dim[0]) / ((float)dim[1]);
    *z = zNear;
}

void Viewport::projectPoint(const float * const modelView, HorseRadish::Vector * const listPoints, const int numPoints) const
{
	HorseRadish::Matrix trans;
	HorseRadish::Vector4 result;
	float rhw,winX,winY,projX,projY,projZ,depthRange[2];

	//verificar parametros
	if (modelView==nullptr || listPoints==nullptr || numPoints<=0)
		return;

	//tenho de criar uma só matrix
	trans.Set(mCurrent);
	trans*=modelView;

 	//as coordenadas da janela
	winX=(float)dim[0];
	winY=(float)dim[1];

	//vou buscar o depth range
	HorseRadish::OpenGL::glGetFloatv(GL_DEPTH_RANGE, depthRange);

	//para todos os pontos
	for(int i=0; i<numPoints; i++)
	{
		//transforma o vértice
		result.Set(listPoints[i],1.0f);
		trans.TransformVector(result);

		//"omogeneous division"
		rhw=1.0f/result.w;

		//e finalmente projecção
        projX=(1.0f + result.x * rhw) * winX * 0.5f;
        projY=winY - ((1.0f - result.y * rhw) * winY * 0.5f);
        projZ=(result.z * rhw) * (depthRange[1] - depthRange[0]) + depthRange[0];

		//e copio para onde li
		listPoints[i].Set(projX,projY,projZ);
	}
}

void Viewport::projectPoint(const float * const modelView, const float * const auxMat, HorseRadish::Vector * const listPoints, const int numPoints) const
{
	HorseRadish::Matrix trans;
	HorseRadish::Vector4 result;
	float rhw,winX,winY,projX,projY,projZ,depthRange[2];

	//verificar parametros
	if (modelView==nullptr || auxMat==nullptr || listPoints==nullptr || numPoints<=0)
		return;

	//tenho de criar uma só matrix
	trans.Set(mCurrent);
	trans*=modelView;
	trans*=auxMat;

 	//as coordenadas da janela
	winX=(float)dim[0];
	winY=(float)dim[1];

	//vou buscar o depth range
	HorseRadish::OpenGL::glGetFloatv(GL_DEPTH_RANGE, depthRange);

	//para todos os pontos
	for(int i=0; i<numPoints; i++)
	{
		//transforma o vértice
		result.Set(listPoints[i],1.0f);
		trans.TransformVector(result);

		//"omogeneous division"
		rhw=1.0f/result.w;

		//e finalmente projecção
        projX=(1.0f + result.x * rhw) * winX * 0.5f;
        projY=winY - ((1.0f - result.y * rhw) * winY * 0.5f);
        projZ=(result.z * rhw) * (depthRange[1] - depthRange[0]) + depthRange[0];

		//e copio para onde li
		listPoints[i].Set(projX,projY,projZ);
	}
}

void Viewport::updateGL() const
{
	HorseRadish::OpenGL::glViewport(origin[0], origin[1], dim[0], dim[1]);	
	HorseRadish::OpenGL::glScissor(0, 0, dim[0], dim[1]);
}

//§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§
//§§§§§§   Classe Frustum	§§§§§
//§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§
void Frustum::extractPlanes(const HorseRadish::Vector4 &col1, const HorseRadish::Vector4 &col2, const HorseRadish::Vector4 &col3, const HorseRadish::Vector4 &col4)
{
	//calcula-se cada plano
	planes[PlaneLeft].Set(col4.x+col1.x, col4.y+col1.y, col4.z+col1.z, col4.w+col1.w);
	planes[PlaneRight].Set(col4.x-col1.x, col4.y-col1.y, col4.z-col1.z, col4.w-col1.w);
	
	planes[PlaneTop].Set(col4.x-col2.x, col4.y-col2.y, col4.z-col2.z, col4.w-col2.w);
	planes[PlaneBottom].Set(col4.x+col2.x, col4.y+col2.y, col4.z+col2.z, col4.w+col2.w);

	//normaliza-se
	planes[PlaneLeft].Normalize();
	planes[PlaneRight].Normalize();
	planes[PlaneBottom].Normalize();
	planes[PlaneTop].Normalize();

	//o near e far têm um tratamento especial
	planes[PlaneNear].Set(col4.x+col3.x, col4.y+col3.y, col4.z+col3.z, 0.0f);
	planes[PlaneFar].Set(col4.x-col3.x, col4.y-col3.y, col4.z-col3.z, 0.0f);
	planes[PlaneNear].NormalizeNormal();
	planes[PlaneFar].NormalizeNormal();
	planes[PlaneNear].SetD(-(planes[PlaneNear].DotNormal(position) + zNear));
	planes[PlaneFar].SetD(-(planes[PlaneNear].DotNormal(position) - zFar));
}

bool Frustum::sweptSpherePlaneIntersect(float &t0, float &t1, const HorseRadish::Plane &plane, const HorseRadish::Vector &sphereCenter, const float &sphereRadius, const HorseRadish::Vector &sweepDir) const
{
	float b_dot_n,d_dot_n,tmp0,tmp1;

	b_dot_n = plane.GetDistance(sphereCenter);
	d_dot_n = plane.DotNormal(sweepDir);

	if (HorseRadish::Math::isZero(d_dot_n))
	{
		if (b_dot_n<=sphereRadius)
		{
			t0 = 0.0f;
			t1 = HorseRadish::Math::INFINITY;
			return true;
		}
		return false;
	}

	d_dot_n = 1.0f / d_dot_n;
	tmp0 = ( sphereRadius - b_dot_n) * d_dot_n;
	tmp1 = (-sphereRadius - b_dot_n) * d_dot_n;
	t0 = HorseRadish::Math::fMin(tmp0, tmp1);
	t1 = HorseRadish::Math::fMax(tmp0, tmp1);
	return true;
}

Frustum& Frustum::operator=(const Frustum& frustum)
{
	planes[0]=frustum.planes[0];
	planes[1]=frustum.planes[1];
	planes[2]=frustum.planes[2];
	planes[3]=frustum.planes[3];
	planes[4]=frustum.planes[4];
	planes[5]=frustum.planes[5];

	position=frustum.position;
	zNear=frustum.zNear;
	zFar=frustum.zFar;
	return *this;
}

void Frustum::getCorners(HorseRadish::Vector * const points) const
{
	/*
	Os cantos estão ordenados da seguinte forma:

	7---------------6
	|\				|\
	| \				| \
	|  \  zfar		|  \
	|	\			|   \
	|	 \			|    \
	4-----\---------5	  \
	 \	   \		 \	   \
	  \		\		  \	    \
	   \	 3---------------2
		\	 |			\    |
		 \	 |			 \   |
		  \	 |	  znear	  \  |
		   \ |			   \ |
		     0---------------1
	*/

	//porcaria de ponteiros
	if (points == nullptr)
		return;

	//crio cada um dos pontos de intersecção (os do znear)
	planes[PlaneNear].IntersectPlanes(planes[PlaneLeft],planes[PlaneBottom],points+0);
	planes[PlaneNear].IntersectPlanes(planes[PlaneRight],planes[PlaneBottom],points+1);
	planes[PlaneNear].IntersectPlanes(planes[PlaneRight],planes[PlaneTop],points+2);
	planes[PlaneNear].IntersectPlanes(planes[PlaneLeft],planes[PlaneTop],points+3);

	//crio cada um dos pontos de intersecção (os do zfar)
	planes[PlaneFar].IntersectPlanes(planes[PlaneLeft],planes[PlaneBottom],points+4);
	planes[PlaneFar].IntersectPlanes(planes[PlaneRight],planes[PlaneBottom],points+5);
	planes[PlaneFar].IntersectPlanes(planes[PlaneRight],planes[PlaneTop],points+6);
	planes[PlaneFar].IntersectPlanes(planes[PlaneLeft],planes[PlaneTop],points+7);
}

Frustum::IntersectionType Frustum::ClassifyFrustum(const Frustum &frustum) const
{
	HorseRadish::Vector corners[8];
	bool inside,resultado,outside;

	//retiro os cantos do frustum
	frustum.getCorners(corners);

	//pra simplificar, vou ver se todos os pontos estão dentro, se estiverem, está dentro de certeza
	inside=false;
	outside=false;
	for(int i = 0; i < 8; i++)
	{
		resultado=testPoint(corners[i]);
		inside|=resultado;
		outside|=!resultado;
	}

	//se está tudo dentro
	if (inside && !outside)
		return IntersectionType::FullInside;

	//se existem alguns dentro e outros fora
	if (inside && outside)
		return IntersectionType::FrustumIntersect;

	//para cada plano, se todos os pontos estão de lado, só pode estar fora
	for(int i = 0; i < 6; i++)
	{
		//basta um ponto estar à frente ou ser coplaner, para passar para o próximo plano
		if (planes[i].ClassifyPoint(corners[0])!=HorseRadish::Plane::BEHIND)	continue;
		if (planes[i].ClassifyPoint(corners[1])!=HorseRadish::Plane::BEHIND)	continue;
		if (planes[i].ClassifyPoint(corners[2])!=HorseRadish::Plane::BEHIND)	continue;
		if (planes[i].ClassifyPoint(corners[3])!=HorseRadish::Plane::BEHIND)	continue;
		if (planes[i].ClassifyPoint(corners[4])!=HorseRadish::Plane::BEHIND)	continue;
		if (planes[i].ClassifyPoint(corners[5])!=HorseRadish::Plane::BEHIND)	continue;
		if (planes[i].ClassifyPoint(corners[6])!=HorseRadish::Plane::BEHIND)	continue;
		if (planes[i].ClassifyPoint(corners[7])!=HorseRadish::Plane::BEHIND)	continue;

		//chegando aqui, os pontos estavam todos atrás do plano, logo têm de estar fora
		return IntersectionType::FullOutside;
	}

	/*
		Chegando aqui, ainda não tenho bem a certeza se intersecto ou não, pode acontecer isto:

		  X----X
	 	 /    /
	    /    /
	   X----X

		   X-------X
		   |	   |
		   |	   |
		   X-------X

		Ou seja, digo que intersecta (nunca acontece todos os pontos detrás de um plano) quando na verdade
		ele não intersecta... A melhor forma de compensar isto: repetir o teste em relação ao outro frustum!
		Se o outro frustum disser
	*/

	//tiro agora os cantos de mim
	getCorners(corners);

	//para cada plano (daquele que quero verificar, se todos os pontos estão de lado, só pode estar fora
	for(int i = 0; i < 6; i++)
	{
		//basta um ponto estar à frente ou ser coplaner, para passar para o próximo plano
		if (frustum.planes[i].ClassifyPoint(corners[0]) != HorseRadish::Plane::BEHIND)	continue;
		if (frustum.planes[i].ClassifyPoint(corners[1]) != HorseRadish::Plane::BEHIND)	continue;
		if (frustum.planes[i].ClassifyPoint(corners[2]) != HorseRadish::Plane::BEHIND)	continue;
		if (frustum.planes[i].ClassifyPoint(corners[3]) != HorseRadish::Plane::BEHIND)	continue;
		if (frustum.planes[i].ClassifyPoint(corners[4]) != HorseRadish::Plane::BEHIND)	continue;
		if (frustum.planes[i].ClassifyPoint(corners[5]) != HorseRadish::Plane::BEHIND)	continue;
		if (frustum.planes[i].ClassifyPoint(corners[6]) != HorseRadish::Plane::BEHIND)	continue;
		if (frustum.planes[i].ClassifyPoint(corners[7]) != HorseRadish::Plane::BEHIND)	continue;

		//chegando aqui, os pontos estavam todos atrás do plano, logo têm de estar fora
		return IntersectionType::FullOutside;
	}

	//logo intersecta de certeza
	return IntersectionType::FrustumIntersect;
}

float Frustum::DotNormals(const PlaneIndex planeA, const PlaneIndex planeB) const
{
	if (planeA<0 || planeA>5 || planeB<0 || planeB>5)
		return 0.0f;

	HorseRadish::Vector normalB;
	planes[planeB].GetNormal(normalB);
	return planes[planeA].DotNormal(normalB);
}

void Frustum::CalculateFrustum(const float * const transformation)
{
	HorseRadish::Vector4 col1,col2,col3,col4;
	HorseRadish::Matrix matTrans;

	//já tenho a matrix de transformação
	matTrans.Set(transformation);

	//retiro as colunas
	matTrans.GetCol1(col1);
	matTrans.GetCol2(col2);
	matTrans.GetCol3(col3);
	matTrans.GetCol4(col4);

	//extraio os planos
	extractPlanes(col1,col2,col3,col4);
}

void Frustum::CalculateFrustum(const float * const matProjection, const float * const matModelview)
{
	HorseRadish::Vector4 col1,col2,col3,col4;
	HorseRadish::Matrix matTrans;

	//tenho de calcular a matrix de transformação
	matTrans.Set(matProjection);
	matTrans*=matModelview;

	//retiro as colunas
	matTrans.GetCol1(col1);
	matTrans.GetCol2(col2);
	matTrans.GetCol3(col3);
	matTrans.GetCol4(col4);

	//extraio os planos
	extractPlanes(col1,col2,col3,col4);
}

void Frustum::CalculateFrustum(const float * const modelViewMatrix, const float * const projectionMatrix, const HorseRadish::Vector &pos, const float zNear, const float zFar)
{
	HorseRadish::Matrix fClip,auxMat;
	HorseRadish::Vector4 col1,col2,col3,col4;

	//preciso da matrix final
	auxMat.Set(modelViewMatrix);
	fClip.Set(projectionMatrix);
	fClip*=auxMat;

	//guardo estes dados
	this->position=pos;
	this->zNear=zNear;
	this->zFar=zFar;

	//retiro as colunas
	fClip.GetCol1(col1);
	fClip.GetCol2(col2);
	fClip.GetCol3(col3);
	fClip.GetCol4(col4);

	//extraio os planos
	extractPlanes(col1,col2,col3,col4);
}

void Frustum::SetIndividualPlane(const PlaneIndex planeIndex, const HorseRadish::Plane &plane)
{
	if (planeIndex<0 || planeIndex>5)
		return;
	planes[planeIndex]=plane;
}

void Frustum::SetFrustum(const Frustum *const frustum)
{
	planes[0]=frustum->planes[0];
	planes[1]=frustum->planes[1];
	planes[2]=frustum->planes[2];
	planes[3]=frustum->planes[3];
	planes[4]=frustum->planes[4];
	planes[5]=frustum->planes[5];
}

void Frustum::SetFrustum(const HorseRadish::Vector &bboxMin, const HorseRadish::Vector &bboxMax)
{
	planes[PlaneLeft].Set( 1.0f, 0.0f, 0.0f, -bboxMin.x);
	planes[PlaneRight].Set(-1.0f, 0.0f, 0.0f, bboxMax.x);
	planes[PlaneTop].Set( 0.0f, -1.0f, 0.0f, bboxMax.y);
	planes[PlaneBottom].Set( 0.0f, 1.0f, 0.0f, -bboxMin.y);
	planes[PlaneNear].Set( 0.0f, 0.0f, -1.0f, bboxMax.z);
	planes[PlaneFar].Set( 0.0f, 0.0f, 1.0f, -bboxMin.z);
}

void Frustum::SetFrustum(const HorseRadish::Vector &center, const float radius)
{
	planes[PlaneLeft].Set( 1.0f, 0.0f, 0.0f,-(center.x-radius));
	planes[PlaneRight].Set(-1.0f, 0.0f, 0.0f,center.x+radius);
	planes[PlaneTop].Set( 0.0f,-1.0f, 0.0f,center.y+radius);
	planes[PlaneBottom].Set( 0.0f, 1.0f, 0.0f,-(center.y-radius));
	planes[PlaneNear].Set( 0.0f, 0.0f,-1.0f,center.z+radius);
	planes[PlaneFar].Set( 0.0f, 0.0f, 1.0f,-(center.z-radius));
}

void Frustum::SetFrustum(const HorseRadish::BBox &bbox)
{
	HorseRadish::Vector minP, maxP;

	bbox.GetMax(maxP);
	bbox.GetMin(minP);
	SetFrustum(minP, maxP);
}

bool Frustum::testCube(const HorseRadish::Vector &ponto, const float &size) const
{
	HorseRadish::Vector pCubo[8];

	//calculo os oito pontos distintos do cubo
	pCubo[0].Set(ponto);
	pCubo[1].Set(pCubo[0]);	pCubo[1].z+=size;
	pCubo[2].Set(pCubo[1]);	pCubo[2].y+=size;
	pCubo[3].Set(pCubo[0]);	pCubo[3].y+=size;
	pCubo[4].Set(pCubo[0]);	pCubo[4].x+=size;
	pCubo[5].Set(pCubo[4]);	pCubo[5].z+=size;
	pCubo[6].Set(pCubo[5]);	pCubo[6].y+=size;
	pCubo[7].Set(pCubo[4]);	pCubo[7].y+=size;

	//percorro os 6 planos do cubo do frustum
	for (int iCurPlane = 0; iCurPlane < 6; iCurPlane++)
	{
		if (planes[iCurPlane].GetDistance(pCubo[0]) > 0.0f)
			continue;
		if (planes[iCurPlane].GetDistance(pCubo[1]) > 0.0f)
			continue;
		if (planes[iCurPlane].GetDistance(pCubo[2]) > 0.0f)
			continue;
		if (planes[iCurPlane].GetDistance(pCubo[3]) > 0.0f)
			continue;
		if (planes[iCurPlane].GetDistance(pCubo[4]) > 0.0f)
			continue;
		if (planes[iCurPlane].GetDistance(pCubo[5]) > 0.0f)
			continue;
		if (planes[iCurPlane].GetDistance(pCubo[6]) > 0.0f)
			continue;
		if (planes[iCurPlane].GetDistance(pCubo[7]) > 0.0f)
			continue;

		return false;
	}

	return true;
}

bool Frustum::testBox(const HorseRadish::Vector &min, const HorseRadish::Vector &max) const
{	
	HorseRadish::Vector pBox[8],res;

	//calculo os oito pontos distintos da box
	pBox[0].Set(min);
	pBox[1].Set(min);	pBox[1].z=max.z;
	pBox[2].Set(min);	pBox[2].y=max.y;
	pBox[3].Set(max);	pBox[3].x=min.x;

	pBox[4].Set(min);	pBox[4].x=max.x;
	pBox[5].Set(max);	pBox[5].y=min.y;
	pBox[6].Set(max);	pBox[6].z=min.z;
	pBox[7].Set(max);

	//percorro os 6 planos do cubo do frustum
	for (int iCurPlane = 0; iCurPlane < 6; iCurPlane++)
	{
		if(planes[iCurPlane].GetDistance(pBox[0])>0.0f)
			continue;
		if(planes[iCurPlane].GetDistance(pBox[1])>0.0f)
			continue;
		if(planes[iCurPlane].GetDistance(pBox[2])>0.0f)
			continue;
		if(planes[iCurPlane].GetDistance(pBox[3])>0.0f)
			continue;
		if(planes[iCurPlane].GetDistance(pBox[4])>0.0f)
			continue;
		if(planes[iCurPlane].GetDistance(pBox[5])>0.0f)
			continue;
		if(planes[iCurPlane].GetDistance(pBox[6])>0.0f)
			continue;
		if(planes[iCurPlane].GetDistance(pBox[7])>0.0f)
			continue;

		return false;
	}

	return true;
}

bool Frustum::testBox(const HorseRadish::BBox &bbox) const
{
	HorseRadish::Vector minP,maxP;

	bbox.GetMax(maxP);
	bbox.GetMin(minP);
	return (testBox(minP,maxP));
}

bool Frustum::testSphere(const HorseRadish::Vector &ponto, const float &raio) const
{
	if (planes[0].GetDistance(ponto) <= -raio)
		return false;
	if (planes[1].GetDistance(ponto) <= -raio)
		return false;
	if (planes[2].GetDistance(ponto) <= -raio)
		return false;
	if (planes[3].GetDistance(ponto) <= -raio)
		return false;
	if (planes[4].GetDistance(ponto) <= -raio)
		return false;
	if (planes[5].GetDistance(ponto) <= -raio)
		return false;
   return true;
}

bool Frustum::testSphere(const HorseRadish::BSphere &bsphere) const
{
	HorseRadish::Vector ponto;
	float raio;

	bsphere.GetCenter(ponto);
	raio=bsphere.GetRadius();

	if (planes[0].GetDistance(ponto) <= -raio)
		return false;
	if (planes[1].GetDistance(ponto) <= -raio)
		return false;
	if (planes[2].GetDistance(ponto) <= -raio)
		return false;
	if (planes[3].GetDistance(ponto) <= -raio)
		return false;
	if (planes[4].GetDistance(ponto) <= -raio)
		return false;
	if (planes[5].GetDistance(ponto) <= -raio)
		return false;
   return true;
}

bool Frustum::testSphereBox(const HorseRadish::BSphere &bsphere, const HorseRadish::BBox &bbox) const
{
	if (testSphere(bsphere) == false)
		return false;
	return testBox(bbox);
}

bool Frustum::testPoint(const HorseRadish::Vector &ponto) const
{
	if (planes[0].GetDistance(ponto)<=0.0f)
		return false;
	if (planes[1].GetDistance(ponto)<=0.0f)
		return false;
	if (planes[2].GetDistance(ponto)<=0.0f)
		return false;
	if (planes[3].GetDistance(ponto)<=0.0f)
		return false;
	if (planes[4].GetDistance(ponto)<=0.0f)
		return false;
	if (planes[5].GetDistance(ponto)<=0.0f)
		return false;
   return true;
}

bool Frustum::testPolygon(const HorseRadish::Vector * const pontos, const int numPoints) const
{
	int j;

	//percorro os 6 planos do cubo do frustum
	for (int iCurPlane = 0; iCurPlane < 6; iCurPlane++)
	{
		//percorro todos os pontos do poligono
		for(j = 0; j < numPoints; j++)
		{
			if(planes[iCurPlane].GetDistance(pontos[j]) > 0.0f)
				break;
		}

		if (j != numPoints)
			continue;
		return false;
	}

	return true;
}

bool Frustum::testSquare(const HorseRadish::Vector * const pontos) const
{
	//percorro os 6 planos do cubo do frustum
	for (int iCurPlane = 0; iCurPlane < 6; iCurPlane++)
	{
		//para cada ponto (4) do quadrado
		if(planes[iCurPlane].GetDistance(pontos[0])>0.0f)
			continue;
		if(planes[iCurPlane].GetDistance(pontos[1])>0.0f)
			continue;
		if(planes[iCurPlane].GetDistance(pontos[2])>0.0f)
			continue;
		if(planes[iCurPlane].GetDistance(pontos[3])>0.0f)
			continue;

		//chegando aqui não estou no frustum de certeza
		return false;
	}

	//está dentro do frustum
	return true;
}

bool Frustum::testTri(const HorseRadish::Vector * const pontos) const
{
	//percorro os 6 planos do cubo do frustum
	for (int iCurPlane = 0; iCurPlane < 6; iCurPlane++)
	{
		//para cada ponto (3) do tringulo
		if(planes[iCurPlane].GetDistance(pontos[0])>0.0f)
			continue;
		if(planes[iCurPlane].GetDistance(pontos[1])>0.0f)
			continue;
		if(planes[iCurPlane].GetDistance(pontos[2])>0.0f)
			continue;

		//chegando aqui não estou no frustum de certeza
		return false;
	}

	//está dentro do frustum
	return true;
}

bool Frustum::testSweptSphere(const HorseRadish::Vector &sphereCenter, const float &sphereRadius, const HorseRadish::Vector &sweepDir) const
{
	float displacements[12],a,b;
	HorseRadish::Vector auxCenter;
	bool inFrustum;
	int i,cnt;

	cnt=0;
	inFrustum=false;
	for(i=0; i<6; i++)
	{
		if (sweptSpherePlaneIntersect(a, b, planes[i], sphereCenter, sphereRadius, sweepDir))
		{
			if (a>=0.f)
				displacements[cnt++] = a;
			if (b>=0.f)
				displacements[cnt++] = b;
		}
	}

	for(i=0; i<cnt; i++)
	{
		auxCenter.x = sweepDir.x * displacements[i] + sphereCenter.x;
		auxCenter.y = sweepDir.y * displacements[i] + sphereCenter.y;
		auxCenter.z = sweepDir.z * displacements[i] + sphereCenter.z;

		inFrustum|=testSphere(auxCenter, sphereRadius * 1.1f);
	}

	return inFrustum;
}

}//namespace Tools
}//namespace OpenGL
}//namespace HorseRadish