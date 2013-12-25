#include "Plane.hpp"
#include "Math.hpp"

#include <memory.h>

namespace HorseRadish
{

Plane::Plane(const Plane &plane)
{
	a=plane.a;
	b=plane.b;
	c=plane.c;
	d=plane.d;
}

Plane::Plane(const float nx, const float ny, const float nz, const float nd)
{
	a=nx;b=ny;c=nz;
	d=nd;
}

Plane::Plane(const Vector &nN, const float nd)
{
	a=nN.x;b=nN.y;c=nN.z;
	d=nd;
}

void Plane::SetFromPoints(const Vector &p0, const Vector &p1, const Vector &p2)
{
	Vector normal;

	normal.CalcNormal(p0,p1,p2);
	a=normal.x;
	b=normal.y;
	c=normal.z;
	d=-normal.Dot(p0);
}

void Plane::SetFromPoints(const float *p0, const float *p1, const float *p2)
{
	Vector normal;

	normal.CalcNormal(p0,p1,p2);
	a=normal.x;
	b=normal.y;
	c=normal.z;
	d=-normal.Dot(p0);
}

void Plane::Normalize()
{
	float sizeInv;

	sizeInv=1.0f/sqrtf(a*a + b*b + c*c);
	a*=sizeInv;
	b*=sizeInv;
	c*=sizeInv;
	d*=sizeInv;
}

void Plane::NormalizeNormal()
{	
	float sizeInv;

	sizeInv=1.0f/sqrtf(a*a + b*b + c*c);
	a*=sizeInv;
	b*=sizeInv;
	c*=sizeInv;
}

bool Plane::IntersectRay(const Vector &origin, const Vector &dir, Vector *resultado) const
{
	float dot,t;

	//verificar se o raio não está paralelo
	dot=dir.Dot(a,b,c);
	if (Math::isZero(dot))
		return false;
	
	//verificar onde começa a intersecção
	t=-(origin.Dot(a,b,c)+d)/dot;
	if (t<0.0f)
		return false;
		
	//o raio começa antes da intersecta, é válido
	if (resultado)
		{
		resultado->x=origin.x+(dir.x*t);
		resultado->y=origin.y+(dir.y*t);
		resultado->z=origin.z+(dir.z*t);
		}
	return true;
}

bool Plane::IntersectLine(const Vector &p1, const Vector &p2, Vector *resultado) const
{
	Vector dir;
	float dot,t;

	//a direcção dos dois pontos
	dir=p2-p1;

	//não pode ser paralelo ao plano
	dot=dir.Dot(a,b,c);
	if (Math::isZero(dot))
		return false;

	//onde está a intersecção
	t=-(p1.Dot(a,b,c)+d)/dot;

	//como é linha, é infinita, logo a intersecção é sempre válida (desde que passe o teste em cima)
	if (resultado)
		{
		resultado->x=p1.x+(dir.x*t);
		resultado->y=p1.y+(dir.y*t);
		resultado->z=p1.z+(dir.z*t);
		}
	return true;
}

bool Plane::IntersectLineSegment(const Vector &p1, const Vector &p2, Vector *resultado) const
{
	Vector dir;
	float dot,t;

	//a direcção dos dois pontos
	dir=p2-p1;

	//não pode ser paralelo ao plano
	dot=dir.Dot(a,b,c);
	if (Math::isZero(dot))
		return false;

	//onde está a intersecção
	t=-(p1.Dot(a,b,c)+d)/dot;
	if (t<0.0f || t>1.0f)
		return false;

	//existe intersecção
	if (resultado)
		{
		resultado->x=p1.x+(dir.x*t);
		resultado->y=p1.y+(dir.y*t);
		resultado->z=p1.z+(dir.z*t);
		}
	return true;
}

bool Plane::IntersectPlanes(const Plane &p2, const Plane &p3, Vector *resultado) const
{
	float denominator;
	Vector pNormal,p2Normal,p3Normal,tmp1,tmp2,tmp3;
		
	//calcular o denominador
	pNormal.Set(a,b,c);
	p2Normal.Set(p2.a,p2.b,p2.c);
	p3Normal.Set(p3.a,p3.b,p3.c);
	tmp1.CalcCrossProduct(p2Normal,p3Normal);
	denominator=pNormal.Dot(tmp1);

	//sendo zero nao ha intersecçao
	if(Math::isZero(denominator))
		return false;

	//calcular a interseccao
	tmp1.CalcCrossProduct(p2Normal,p3Normal);
	tmp2.CalcCrossProduct(p3Normal,pNormal);
	tmp3.CalcCrossProduct(pNormal,p2Normal);
	tmp1*=d;
	tmp2*=p2.d;
	tmp3*=p3.d;

	//escrevo o resultado
	if (resultado)
		{
		(*resultado)=tmp1+tmp2+tmp3;
		(*resultado)/=(-denominator);
		}

	return true;
}

bool Plane::IntersectSweptSphere(const float &sphereRadius, const Vector &spherePrevPos, const Vector &sphereCurPos, Vector * const hitPoint, float * const hitTime) const
{
	float d0,d1,htime;
		
	d0 = DotCoord(spherePrevPos);
	d1 = DotCoord(sphereCurPos);

	//verifico se estava a tocar na frame anterior
	if (Math::fAbs(d0)<=sphereRadius)
		{
		if (hitPoint)
			hitPoint->Set(spherePrevPos);
		if (hitTime)
			*hitTime=0.0f;
		return true;
		}

	//verifico se esta a tocar nesta
	if (d0>sphereRadius && d1<sphereRadius)
		{
		htime=(d0-sphereRadius)/(d0-d1);
		if (hitTime)
			*hitTime=htime;
		if (hitPoint)
			hitPoint->CalcInterpolate(spherePrevPos,sphereCurPos,htime);
		return true;
		}

	//falhou
	return false;
}

void Plane::Lerp(const Plane &p2, const float factor, Plane &result)
{
	Vector vecAux;

	result.a=a*factor;
	result.b=b*factor;
	result.c=c*factor;
	
	vecAux.Set(p2.a,p2.b,p2.c);
	vecAux*=(1.0f-factor);

	result.a+=vecAux.x;
	result.b+=vecAux.y;
	result.c+=vecAux.z;
	result.NormalizeNormal();

	result.d=d*factor + p2.d*(1.0f-factor);
}

Plane::POSITION Plane::ClassifyPoint(const Vector &point) const
{
	float calculo;

	calculo=point.Dot(a,b,c)+d;

	if(Math::isZero(calculo))
		return Plane::COPLANAR;
	if(calculo>0.0f)
		return Plane::FRONT;
	return Plane::BEHIND;
}

Plane::POSITION Plane::ClassifyTri(const Vector &p1, const Vector &p2, const Vector &p3) const
{
	POSITION c1,c2,c3;

	c1=ClassifyPoint(p1);
	c2=ClassifyPoint(p2);
	c3=ClassifyPoint(p3);

	//se forem todos iguais, o triangulo classifica-se como qualquer um deles
	if (c1==c2 && c2==c3)
		return c1;

	//senão tem de intersectar
	return INTERSECT;
}

}//namespace HorseRadish