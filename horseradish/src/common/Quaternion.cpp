#include "Quaternion.hpp"
#include "Math.hpp"

namespace HorseRadish
{

void Quaternion::operator+=(const Quaternion &quat)
{
	x+=quat.x;
	y+=quat.y;
	z+=quat.z;
	w+=quat.w;
}

void Quaternion::operator-=(const Quaternion &quat)
{
	x-=quat.x;
	y-=quat.y;
	z-=quat.z;
	w-=quat.w;
}

void Quaternion::operator*=(const Quaternion &quat)
{
	float qX,qY,qZ;

	qX=x;
	qY=y;
	qZ=z;

	x=w*quat.x + qX*quat.w + qY*quat.z - qZ*quat.y;
	y=w*quat.y - qX*quat.z + qY*quat.w + qZ*quat.x;
	z=w*quat.z + qX*quat.y - qY*quat.x + qZ*quat.w;
	w=w*quat.w - qX*quat.x - qY*quat.y - qZ*quat.z;
}

void Quaternion::operator*=(const float &scalar)
{
	x*=scalar;
	y*=scalar;
	z*=scalar;
	w*=scalar;
}

void Quaternion::operator/=(const Quaternion &quat)
{
	Quaternion q, r, s;

	//copia e inverto logo
	q.x=-quat.x;
	q.y=-quat.y;
	q.z=-quat.z;
	q.w=quat.w;
	
	r.x=x;
	r.y=y;
	r.z=z;
	r.w=w;
	r*=q;

	s.x=q.x;
	s.y=q.y;
	s.z=q.z;
	s.w=q.w;
	s*=q;

	x=r.x/s.w;
	y=r.y/s.w;
	z=r.z/s.w;
	w=r.w/s.w;
}

void Quaternion::SetAxisAngle(const float &vx, const float &vy, const float &vz, const float &angulo)
{
	float anguloRad,resSin;

	//o novo angulo e divido logo por 2
	anguloRad=(angulo*0.017453292519943295f)*0.5f;

	//calculo logo o sin e o W é o coseno do angulo
	Math::sinCosR(anguloRad,resSin,w);

	//o resto das coordenadas
	x=vx*resSin;
	y=vy*resSin;
	z=vz*resSin;
}

void Quaternion::SetAxisAngle(const Vector &vec, const float &angulo)
{
	float anguloRad,resSin;

	//o novo angulo e divido logo por 2
	anguloRad=(angulo*0.017453292519943295f)*0.5f;

	//calculo logo o sin e o W é o coseno do angulo
	Math::sinCosR(anguloRad,resSin,this->w);

	//o resto das coordenadas
	this->x=vec.x*resSin;
	this->y=vec.y*resSin;
	this->z=vec.z*resSin;
}

void Quaternion::SetFromMatrix3x3(const float * const matrix)
{
	float s;
	
	s = matrix[0] + matrix[4] + matrix[8];
	if (s>0.0f) 
		{
		s=Math::sqrt(s+1.0f);

		w=s*0.5f;
		s=0.5/s;
		x=(matrix[5]-matrix[7])*s;
		y=(matrix[6]-matrix[2])*s;
		z=(matrix[1]-matrix[3])*s;
		return;
		}

	if ( (matrix[4]<=matrix[0]) && (matrix[8]<=matrix[0]) )
		{
		s=Math::sqrt((matrix[0]-(matrix[4]+matrix[8]))+1.0f);

		x=s*0.5f;
		s=0.5/s;
		y=(matrix[1]+matrix[3])*s;
		z=(matrix[2]+matrix[6])*s;
		w=(matrix[5]-matrix[7])*s;
		return;
		}

	if ( (matrix[4]>matrix[0]) && (matrix[8]<=matrix[4]) )
		{
		s=Math::sqrt((matrix[4]-(matrix[8]+matrix[0]))+1.0f);

		y=s*0.5f;
		s=0.5f/s;
		w=(matrix[6]-matrix[2])*s;
		z=(matrix[5]+matrix[7])*s;
		x=(matrix[3]+matrix[1])*s;
		return;
		}

	s=Math::sqrt((matrix[8]-(matrix[0]+matrix[4]))+1.0f);

	z=s*0.5f;
	s=0.5f/s;
	w=(matrix[1]-matrix[3])*s;
	x=(matrix[6]+matrix[2])*s;
	y=(matrix[7]+matrix[5])*s;
}

void Quaternion::SetFromMatrix4x4(const float * const matrix)
{
	float s;

	//calculo o quaternion a partir da matriz
	s = matrix[0] + matrix[5] + matrix[10];
	if (s>0.0f) 
		{
		s=Math::sqrt(s+1.0f);

		w=s*0.5f;
		s=0.5/s;
		x=(matrix[6]-matrix[9])*s;
		y=(matrix[8]-matrix[2])*s;
		z=(matrix[1]-matrix[4])*s;
		return;
		}

	if ( (matrix[5]<=matrix[0]) && (matrix[10]<=matrix[0]) )
		{
		s=Math::sqrt((matrix[0]-(matrix[5]+matrix[10]))+1.0f);

		x=s*0.5f;
		s=0.5/s;
		y=(matrix[1]+matrix[4])*s;
		z=(matrix[2]+matrix[8])*s;
		w=(matrix[6]-matrix[9])*s;
		return;
		}

	if ( (matrix[5]>matrix[0]) && (matrix[10]<=matrix[5]) )
		{
		s=Math::sqrt((matrix[5]-(matrix[10]+matrix[0]))+1.0f);

		y=s*0.5f;
		s=0.5f/s;
		w=(matrix[8]-matrix[2])*s;
		z=(matrix[6]+matrix[9])*s;
		x=(matrix[4]+matrix[1])*s;
		return;
		}

	s=Math::sqrt((matrix[10]-(matrix[0]+matrix[5]))+1.0f);

	z=s*0.5f;
	s=0.5f/s;
	w=(matrix[1]-matrix[4])*s;
	x=(matrix[8]+matrix[2])*s;
	y=(matrix[9]+matrix[6])*s;
}

void Quaternion::SetFromEuler(const float &angX, const float &angY, const float &angZ)
{
	float cosR, cosP, cosY, sinR, sinP, sinY, cpcy, spsy;

	Math::sinCosR(angX*0.0087266462599716478846184f, sinR, cosR);
	Math::sinCosR(angY*0.0087266462599716478846184f, sinP, cosP);
	Math::sinCosR(angZ*0.0087266462599716478846184f, sinY, cosY);
	spsy = sinP * sinY;
	cpcy = cosP * cosY;

	w = cosR * cpcy + sinR * spsy;
	x = sinR * cpcy - cosR * spsy;
	y = cosR * sinP * cosY + sinR * cosP * sinY;
	z = cosR * cosP * sinY - sinR * sinP * cosY;

	Normaliza();
}

void Quaternion::SetSLerp(const Quaternion &from, const Quaternion &to, const float &t)
{
	float dot,c,s;
	Quaternion qAux;

	dot=from.Dot(to);
	if (dot>0.99999f)
	{
		x =  from.x + (to.x + from.x) * t;
		y =  from.y + (to.y + from.y) * t;
		z =  from.z + (to.z + from.z) * t;
		w =  from.w + (to.w + from.w) * t;
		Normaliza();
		return;
	}

	dot=Math::fClamp(dot,-1.0f,1.0f);
	Math::sinCosR(acosf(dot)*t,s,c);
    
	qAux.x = to.x - from.x * dot;
	qAux.y = to.y - from.y * dot;
	qAux.z = to.z - from.z * dot;
	qAux.w = to.w - from.w * dot;
	qAux.Normaliza();

	x = from.x * c + qAux.x * s;
	y = from.y * c + qAux.y * s;
	z = from.z * c + qAux.z * s;
	w = from.w * c + qAux.w * s;
}

void Quaternion::SetLerp(const Quaternion &from, const Quaternion &to, const float &t)
{
	float to1[4],cosom,scale0,scale1;

	// calc cosine
	cosom=from.x*to.x + from.y*to.y + from.z*to.z + from.w*to.w;

	// adjust signs (if necessary)
	if (cosom<0.0f)
	{
		to1[0]=-to.x;
		to1[1]=-to.y;
		to1[2]=-to.z;
		to1[3]=-to.w;
	}
	else 
	{
		to1[0]=to.x;
		to1[1]=to.y;
		to1[2]=to.z;
		to1[3]=to.w;
	}

	// interpolate linearly
	scale0=1.0f-t;
	scale1=t;

	// calculate final values
	this->x=scale0*from.x + scale1*to1[0];
	this->y=scale0*from.y + scale1*to1[1];
	this->z=scale0*from.z + scale1*to1[2];
	this->w=scale0*from.w + scale1*to1[3];
}

void Quaternion::Set(const float &nx, const float &ny, const float &nz, const float &nw)
{
	x=nx;
	y=ny;
	z=nz;
	w=nw;
}

void Quaternion::Set(const Vector &vec, const float &nw)
{
	x=vec.x;
	y=vec.y;
	z=vec.z;
	w=nw;
}

void Quaternion::SetAngle(const float &nx, const float &ny, const float &nz, const float &angulo)
{
	x=nx;
	y=ny;
	z=nz;
	w=cos(angulo*0.00872664625997164788461845384f);	//tambem didive por 2
}

void Quaternion::SetAngle(const Vector &vec, const float &angulo)
{
	x=vec.x;
	y=vec.y;
	z=vec.z;
	w=cos(angulo*0.00872664625997164788461845384f);	//tambem didive por 2
}

void Quaternion::Set(const Quaternion &quat)
{
	x=quat.x;
	y=quat.y;
	z=quat.z;
	w=quat.w;
}

void Quaternion::ScaleAngle(const float &scale)
{	this->w *= scale; }

void Quaternion::Invert()
{
	this->x = -x;
	this->y = -y;
	this->z = -z;
}

void Quaternion::Normaliza()
{
	float tamanho = 1.0f / sqrt(x*x+y*y+z*z+w*w);
	this->x *= tamanho;
	this->y *= tamanho;
	this->z *= tamanho;
	this->w *= tamanho;
}

void Quaternion::RotateVector(Vector &vec) const
{
	float auxX,auxY,auxZ,auxW,W;

	auxX=-(w*vec.x + y*vec.z - z*vec.y);
	auxY=-(w*vec.y - x*vec.z + z*vec.x);
	auxZ=-(w*vec.z + x*vec.y - y*vec.x);
	auxW=-(x*vec.x - y*vec.y - z*vec.z);
	auxW=x*vec.x + y*vec.y + z*vec.z;

	W=-w;
	vec.x=auxW*x + auxX*W + auxY*z - auxZ*y;
	vec.y=auxW*y - auxX*z + auxY*W + auxZ*x;
	vec.z=auxW*z + auxX*y - auxY*x + auxZ*W;
}

void Quaternion::RotateVector(const Vector &vec, Vector &dest) const
{
	float auxX,auxY,auxZ,auxW,W;

	auxX=-(w*vec.x + y*vec.z - z*vec.y);
	auxY=-(w*vec.y - x*vec.z + z*vec.x);
	auxZ=-(w*vec.z + x*vec.y - y*vec.x);
	auxW=-(x*vec.x - y*vec.y - z*vec.z);
	auxW=x*vec.x + y*vec.y + z*vec.z;

	W=-w;
	dest.x=auxW*x + auxX*W + auxY*z - auxZ*y;
	dest.y=auxW*y - auxX*z + auxY*W	+ auxZ*x;
	dest.z=auxW*z + auxX*y - auxY*x + auxZ*W;
}

void Quaternion::RotateVector(const float * const vec, float * const dest) const
{
	float auxX,auxY,auxZ,auxW,W;

	auxX=-(w*vec[0] + y*vec[2] - z*vec[1]);
	auxY=-(w*vec[1] - x*vec[2] + z*vec[0]);
	auxZ=-(w*vec[2] + x*vec[1] - y*vec[0]);
	auxW=-(x*vec[0] - y*vec[1] - z*vec[2]);
	auxW=x*vec[0] + y*vec[1] + z*vec[2];

	W=-w;
	dest[0]=auxW*x + auxX*W + auxY*z - auxZ*y;
	dest[1]=auxW*y - auxX*z + auxY*W + auxZ*x;
	dest[2]=auxW*z + auxX*y - auxY*x + auxZ*W;
}

void Quaternion::RotateVector(const float &vx, const float &vy, const float &vz, Vector &dest) const
{
	float auxX,auxY,auxZ,auxW,W;

	auxX=-(w*vx + y*vz - z*vy);
	auxY=-(w*vy - x*vz + z*vx);
	auxZ=-(w*vz + x*vy - y*vx);
	auxW=-(x*vx - y*vy - z*vz);
	auxW=x*vx + y*vy + z*vz;

	W=-w;
	dest.x=auxW*x + auxX*W + auxY*z - auxZ*y;
	dest.y=auxW*y - auxX*z + auxY*W	+ auxZ*x;
	dest.z=auxW*z + auxX*y - auxY*x + auxZ*W;
}

void Quaternion::RotateVectorAdd(Vector &vec, const Vector &vecAdd) const
{
	float auxX,auxY,auxZ,auxW,W;

	auxX=-(w*vec.x + y*vec.z - z*vec.y);
	auxY=-(w*vec.y - x*vec.z + z*vec.x);
	auxZ=-(w*vec.z + x*vec.y - y*vec.x);
	auxW=-(x*vec.x - y*vec.y - z*vec.z);
	auxW=x*vec.x + y*vec.y + z*vec.z;

	W=-w;
	vec.x=auxW*x + auxX*W + auxY*z - auxZ*y + vecAdd.x;
	vec.y=auxW*y - auxX*z + auxY*W + auxZ*x + vecAdd.y;
	vec.z=auxW*z + auxX*y - auxY*x + auxZ*W + vecAdd.z;
}

void Quaternion::RotateVectorAdd(Vector &dest, const Vector &vec, const Vector &vecAdd) const
{
	float auxX,auxY,auxZ,auxW,W;

	auxX=-(w*vec.x + y*vec.z - z*vec.y);
	auxY=-(w*vec.y - x*vec.z + z*vec.x);
	auxZ=-(w*vec.z + x*vec.y - y*vec.x);
	auxW=-(x*vec.x - y*vec.y - z*vec.z);
	auxW=x*vec.x + y*vec.y + z*vec.z;

	W=-w;
	dest.x=auxW*x + auxX*W + auxY*z - auxZ*y + vecAdd.x;
	dest.y=auxW*y - auxX*z + auxY*W + auxZ*x + vecAdd.y;
	dest.z=auxW*z + auxX*y - auxY*x + auxZ*W + vecAdd.z;
}

float Quaternion::Dot(const Quaternion &quat) const
{
	return (w*quat.w + x*quat.x + y*quat.y + z*quat.z);
}

void Quaternion::Identidade()
{
	x=y=z=0.0f;
	w=1.0f;
}

void Quaternion::MulEulerAngles(const float &angX, const float &angY, const float &angZ)
{
	float auxX,auxY,auxZ,auxW;
	float degX, degY, degZ;
	float cosR, cosP, cosY, sinR, sinP, sinY, cpcy, spsy;

	Math::sinCosR(angX*0.0087266462599716478846184f, sinR, cosR);
	Math::sinCosR(angY*0.0087266462599716478846184f, sinP, cosP);
	Math::sinCosR(angZ*0.0087266462599716478846184f, sinY, cosY);
	spsy = sinP * sinY;
	cpcy = cosP * cosY;
	
	auxW = cosR * cpcy + sinR * spsy;
	auxX = sinR * cpcy - cosR * spsy;
	auxY = cosR * sinP * cosY + sinR * cosP * sinY;
	auxZ = cosR * cosP * sinY - sinR * sinP * cosY;

	spsy=1.0f/sqrt(auxX*auxX + auxY*auxY + auxZ*auxZ + auxW*auxW);
	auxX*=spsy;
	auxY*=spsy;
	auxZ*=spsy;
	auxW*=spsy;

	degX=x;
	degY=y;
	degZ=z;
	cosP=w;

	x=cosP*auxX + degX*auxW + degY*auxZ - degZ*auxY;
	y=cosP*auxY - degX*auxZ + degY*auxW + degZ*auxX;
	z=cosP*auxZ + degX*auxY - degY*auxX + degZ*auxW;
	w=cosP*auxW - degX*auxX - degY*auxY - degZ*auxZ;
}

void Quaternion::ExpandW()
{
	float term;

	term=1.0f-(x*x)-(y*y)-(z*z);
	w=0.0f;
	if (term>=0.0f)
		w=-(float)sqrt(term);
}

void Quaternion::ExpandWNormaliza()
{
	float auxF;

	auxF=1.0f-(x*x)-(y*y)-(z*z);
	w=0.0f;
	if (auxF>=0.0f)
		w=-(float)sqrt(auxF);

	auxF=1.0f/sqrt((auxF-1.0f)*(-1.0f)+w*w);
	x*=auxF;
	y*=auxF;
	z*=auxF;
	w*=auxF;
}

void Quaternion::GetVector(float * const vec) const
{
	vec[0]=x;
	vec[1]=y;
	vec[2]=z;
}

void Quaternion::GetVector(Vector &vec) const
{	vec.Set(x,y,z);}

void Quaternion::GetMatrix3x3(float * const matrix) const
{
	float xx,yy,zz,wx,wy,wz,xy,xz,yz;

	xx=x*x;
	yy=y*y;
	zz=z*z;
	wx=w*x;
	wy=w*y;
	wz=w*z;
	xy=x*y;
	xz=x*z;
	yz=y*z;

	matrix[0]=1.0f - 2.0f * ( yy + zz );
	matrix[1]=2.0f * ( xy - wz );
	matrix[2]=2.0f * ( xz + wy );

	matrix[3]=2.0f * ( xy + wz );
	matrix[4]=1.0f - 2.0f * ( xx + zz );
	matrix[5]=2.0f * ( yz - wx );

	matrix[6]=2.0f * ( xz - wy );
	matrix[7]=2.0f * ( yz + wx );
	matrix[8]=1.0f - 2.0f * ( xx + yy );
}

void Quaternion::GetMatrix4x4(float * const matrix) const
{
	matrix[ 0]=1.0f - 2.0f * ( y * y + z * z );  
	matrix[ 1]=2.0f * ( x * y - w * z );  
	matrix[ 2]=2.0f * ( x * z + w * y );  
	matrix[ 3]=0.0f;  

	matrix[ 4]=2.0f * ( x * y + w * z );  
	matrix[ 5]=1.0f - 2.0f * ( x * x + z * z );  
	matrix[ 6]=2.0f * ( y * z - w * x );  
	matrix[ 7]=0.0f;  

	matrix[ 8]=2.0f * ( x * z - w * y );  
	matrix[ 9]=2.0f * ( y * z + w * x );  
	matrix[10]=1.0f - 2.0f * ( x * x + y * y );  
	matrix[11]=0.0f;  

	matrix[12]=0.0f;  
	matrix[13]=0.0f;  
	matrix[14]=0.0f;  
	matrix[15]=1.0f;
}

void Quaternion::GetAxisAngle(float * const vecX, float * const vecY, float * const vecZ, float * const ang) const
{
	float len,auxX,auxY,auxZ;

	len=x*x+y*y+z*z;
	if (len==0.0f) 
		{
		*vecX=0.0f;
		*vecY=0.0f;
		*vecZ=1.0f;
		*ang=0.0f;
		return;
		}
    
	len=1.0f/len;
	auxX=x*len;
	auxY=y*len;
	auxZ=z*len;

	len=1.0f/sqrt(auxX*auxX+auxY*auxY+auxZ*auxZ);
	*vecX=auxX*len;
	*vecY=auxY*len;
	*vecZ=auxZ*len;
	
	*ang=((float)acos(w))*114.5915590261646417f; // 180/pi=57.295779513082320876f * 2.0f (o 2 é de formula)
}

void Quaternion::GetAxisAngle(Vector &vec, float * const ang) const
{
	float len,auxX,auxY,auxZ;

	len=x*x+y*y+z*z;
	if (len==0.0f) 
		{
		vec.x=0.0f;
		vec.y=0.0f;
		vec.z=1.0f;
		*ang=0.0f;
		return;
		}
    
	len=1.0f/len;
	auxX=x*len;
	auxY=y*len;
	auxZ=z*len;

	len=1.0f/sqrt(auxX*auxX+auxY*auxY+auxZ*auxZ);
	vec.x=auxX*len;
	vec.y=auxY*len;
	vec.z=auxZ*len;
	
	*ang=((float)acos(w))*114.5915590261646417f; // 180/pi=57.295779513082320876f * 2.0f (o 2 é de formula)
}

void Quaternion::GetEulerAngles(float * const angX, float * const angY, float * const angZ) const
{
	//faz nada
}

void Quaternion::SetFromVectors(const Vector &v1, const Vector &v2)
{
    float tx, ty, tz, temp, dist;
    float cost, len, ss;

	// get dot product of two vectors
    cost = v1.x * v2.x + v1.y * v2.y + v1.z * v2.z;

    // check if parallel
    if (cost > 0.99999f)
		{
		this->x=this->y=this->z=0.0f;
		this->w=1.0f;
		return;
		}
    else if (cost < -0.99999f)
		{		// check if opposite

		// check if we can use cross product of from vector with [1, 0, 0]
		tx = 0.0;
		ty = v1.x;
		tz = -v1.y;

		len = sqrt(ty*ty + tz*tz);
		if (len < 1e-6)
			{
			// nope! we need cross product of from vector with [0, 1, 0]
			tx = -v1.z;
			ty = 0.0;
			tz = v1.x;
			}

		// normalize
		temp = tx*tx + ty*ty + tz*tz;

		dist = (float)(1.0 / sqrt(temp));

		tx *= dist;
		ty *= dist;
		tz *= dist;
		
		this->x = tx;
		this->y = ty;
		this->z = tz;
		this->w = 0.0;

		return;
		}

	// ... else we can just cross two vectors

	tx = v1.y * v2.z - v1.z * v2.y;
	ty = v1.z * v2.x - v1.x * v2.z;
	tz = v1.x * v2.y - v1.y * v2.x;

	temp = tx*tx + ty*ty + tz*tz;

    dist = (float)(1.0 / sqrt(temp));

    tx *= dist;
    ty *= dist;
    tz *= dist;


    // we have to use half-angle formulae (sin^2 t = ( 1 - cos (2t) ) /2)
	
	ss = (float)sqrt(0.5f * (1.0f - cost));

	tx *= ss;
	ty *= ss;
    tz *= ss;

    // scale the axis to get the normalized quaternion
    this->x = tx;
    this->y = ty;
    this->z = tz;

    // cos^2 t = ( 1 + cos (2t) ) / 2
    // w part is cosine of half the rotation angle
    this->w = (float)sqrt(0.5f * (1.0f + cost));
}

}//namespace HorseRadish