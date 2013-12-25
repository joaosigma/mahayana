#include <string.h>
#include <xmmintrin.h>

#include "Matrix.hpp"
#include "Math.hpp"

namespace HorseRadish
{

/*§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§
§§§§§§   -= Classe Matrix =-	§§§§§§
§§§§§§			  4x4			§§§§§§
§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§*/
void Matrix::asmMat4x4Vec3(float *vecWrite, const float *vecRead, const float wCompMul, const unsigned int stride, const float *mat, const unsigned int numVec)
{
	unsigned int leftOver;
	__m128 mat1,mat2,mat3,mat4,final;

	//leio as coisas da matriz e multiplico logo pelo w que é fornecido
	mat1 = _mm_loadu_ps(mat+0);
	mat2 = _mm_loadu_ps(mat+4);
	mat3 = _mm_loadu_ps(mat+8);
	mat4 = _mm_mul_ps(_mm_loadu_ps(mat+12),_mm_load_ps1(&wCompMul));

	//vou tentar fazer 4 de cada vez
	leftOver=numVec;
	for(; leftOver>=4; leftOver-=4)
	{
		//faço as minhas continhas e guardo
		final=_mm_mul_ps(_mm_load_ps1(vecRead+0),mat1);
		final=_mm_add_ps(final,_mm_mul_ps(_mm_load_ps1(vecRead+1),mat2));
		final=_mm_add_ps(final,_mm_mul_ps(_mm_load_ps1(vecRead+2),mat3));
		final=_mm_add_ps(final,mat4);
		_mm_storel_pi((__m64 *)vecWrite, final);
		_mm_store_ss(vecWrite+2, _mm_movehl_ps(final,final));

		//avanço com os ponteiros
		vecRead=(float*)(((unsigned char*)vecRead)+stride);
		vecWrite=(float*)(((unsigned char*)vecWrite)+stride);

		//faço as minhas continhas e guardo
		final=_mm_mul_ps(_mm_load_ps1(vecRead+0),mat1);
		final=_mm_add_ps(final,_mm_mul_ps(_mm_load_ps1(vecRead+1),mat2));
		final=_mm_add_ps(final,_mm_mul_ps(_mm_load_ps1(vecRead+2),mat3));
		final=_mm_add_ps(final,mat4);
		_mm_storel_pi((__m64 *)vecWrite, final);
		_mm_store_ss(vecWrite+2, _mm_movehl_ps(final,final));

		//avanço com os ponteiros
		vecRead=(float*)(((unsigned char*)vecRead)+stride);
		vecWrite=(float*)(((unsigned char*)vecWrite)+stride);

		//faço as minhas continhas e guardo
		final=_mm_mul_ps(_mm_load_ps1(vecRead+0),mat1);
		final=_mm_add_ps(final,_mm_mul_ps(_mm_load_ps1(vecRead+1),mat2));
		final=_mm_add_ps(final,_mm_mul_ps(_mm_load_ps1(vecRead+2),mat3));
		final=_mm_add_ps(final,mat4);
		_mm_storel_pi((__m64 *)vecWrite, final);
		_mm_store_ss(vecWrite+2, _mm_movehl_ps(final,final));

		//avanço com os ponteiros
		vecRead=(float*)(((unsigned char*)vecRead)+stride);
		vecWrite=(float*)(((unsigned char*)vecWrite)+stride);

		//faço as minhas continhas e guardo
		final=_mm_mul_ps(_mm_load_ps1(vecRead+0),mat1);
		final=_mm_add_ps(final,_mm_mul_ps(_mm_load_ps1(vecRead+1),mat2));
		final=_mm_add_ps(final,_mm_mul_ps(_mm_load_ps1(vecRead+2),mat3));
		final=_mm_add_ps(final,mat4);
		_mm_storel_pi((__m64 *)vecWrite, final);
		_mm_store_ss(vecWrite+2, _mm_movehl_ps(final,final));

		//avanço com os ponteiros
		vecRead=(float*)(((unsigned char*)vecRead)+stride);
		vecWrite=(float*)(((unsigned char*)vecWrite)+stride);
	}

	//basta só fazer os últimos
	for(; leftOver>0; leftOver--)
	{
		//faço as minhas continhas e guardo
		final=_mm_mul_ps(_mm_load_ps1(vecRead+0),mat1);
		final=_mm_add_ps(final,_mm_mul_ps(_mm_load_ps1(vecRead+1),mat2));
		final=_mm_add_ps(final,_mm_mul_ps(_mm_load_ps1(vecRead+2),mat3));
		final=_mm_add_ps(final,mat4);
		_mm_storel_pi((__m64 *)vecWrite, final);
		_mm_store_ss(vecWrite+2, _mm_movehl_ps(final,final));

		//avanço com os ponteiros
		vecRead=(float*)(((unsigned char*)vecRead)+stride);
		vecWrite=(float*)(((unsigned char*)vecWrite)+stride);
	}
}

void Matrix::asmMat4x4Vec4(float *vecWrite, const float *vecRead, const unsigned int stride, const float *mat, const unsigned int numVec)
{
	unsigned int leftOver;
	__m128 mat1,mat2,mat3,mat4,final,curVec;

	//leio as coisas da matriz e multiplico logo pelo w que é fornecido
	mat1 = _mm_loadu_ps(mat+0);
	mat2 = _mm_loadu_ps(mat+4);
	mat3 = _mm_loadu_ps(mat+8);
	mat4 = _mm_loadu_ps(mat+12);

	//tento fazer 4 de cada vez
	leftOver=numVec;

	//se tiver tudo alinhadinho, facilita a vida
	if ( (((unsigned int)vecWrite)%16==0) && (((unsigned int)vecRead)%16==0) && (stride%16==0) )
	{
		//lá vamos nós
		for(; leftOver>=4; leftOver-=4)
		{
			//faço as minhas continhas e guardo
			curVec = _mm_load_ps(vecRead);
			final=_mm_mul_ps(_mm_shuffle_ps(curVec,curVec,0x00),mat1);
			final=_mm_add_ps(final,_mm_mul_ps(_mm_shuffle_ps(curVec,curVec,0x55),mat2));
			final=_mm_add_ps(final,_mm_mul_ps(_mm_shuffle_ps(curVec,curVec,0xAA),mat3));
			final=_mm_add_ps(final,_mm_mul_ps(_mm_shuffle_ps(curVec,curVec,0xFF),mat4));
			_mm_store_ps(vecWrite, final);

			//avanço com os ponteiros
			vecRead=(float*)(((unsigned char*)vecRead)+stride);
			vecWrite=(float*)(((unsigned char*)vecWrite)+stride);

			//faço as minhas continhas e guardo
			curVec = _mm_load_ps(vecRead);
			final=_mm_mul_ps(_mm_shuffle_ps(curVec,curVec,0x00),mat1);
			final=_mm_add_ps(final,_mm_mul_ps(_mm_shuffle_ps(curVec,curVec,0x55),mat2));
			final=_mm_add_ps(final,_mm_mul_ps(_mm_shuffle_ps(curVec,curVec,0xAA),mat3));
			final=_mm_add_ps(final,_mm_mul_ps(_mm_shuffle_ps(curVec,curVec,0xFF),mat4));
			_mm_store_ps(vecWrite, final);

			//avanço com os ponteiros
			vecRead=(float*)(((unsigned char*)vecRead)+stride);
			vecWrite=(float*)(((unsigned char*)vecWrite)+stride);

			//faço as minhas continhas e guardo
			curVec = _mm_load_ps(vecRead);
			final=_mm_mul_ps(_mm_shuffle_ps(curVec,curVec,0x00),mat1);
			final=_mm_add_ps(final,_mm_mul_ps(_mm_shuffle_ps(curVec,curVec,0x55),mat2));
			final=_mm_add_ps(final,_mm_mul_ps(_mm_shuffle_ps(curVec,curVec,0xAA),mat3));
			final=_mm_add_ps(final,_mm_mul_ps(_mm_shuffle_ps(curVec,curVec,0xFF),mat4));
			_mm_store_ps(vecWrite, final);

			//avanço com os ponteiros
			vecRead=(float*)(((unsigned char*)vecRead)+stride);
			vecWrite=(float*)(((unsigned char*)vecWrite)+stride);

			//faço as minhas continhas e guardo
			curVec = _mm_load_ps(vecRead);
			final=_mm_mul_ps(_mm_shuffle_ps(curVec,curVec,0x00),mat1);
			final=_mm_add_ps(final,_mm_mul_ps(_mm_shuffle_ps(curVec,curVec,0x55),mat2));
			final=_mm_add_ps(final,_mm_mul_ps(_mm_shuffle_ps(curVec,curVec,0xAA),mat3));
			final=_mm_add_ps(final,_mm_mul_ps(_mm_shuffle_ps(curVec,curVec,0xFF),mat4));
			_mm_store_ps(vecWrite, final);

			//avanço com os ponteiros
			vecRead=(float*)(((unsigned char*)vecRead)+stride);
			vecWrite=(float*)(((unsigned char*)vecWrite)+stride);
		}
	}
	else
	{
		//lá vamos nós
		for(; leftOver>=4; leftOver-=4)
		{
			//faço as minhas continhas e guardo
			curVec = _mm_loadu_ps(vecRead);
			final=_mm_mul_ps(_mm_shuffle_ps(curVec,curVec,0x00),mat1);
			final=_mm_add_ps(final,_mm_mul_ps(_mm_shuffle_ps(curVec,curVec,0x55),mat2));
			final=_mm_add_ps(final,_mm_mul_ps(_mm_shuffle_ps(curVec,curVec,0xAA),mat3));
			final=_mm_add_ps(final,_mm_mul_ps(_mm_shuffle_ps(curVec,curVec,0xFF),mat4));
			_mm_storeu_ps(vecWrite, final);

			//avanço com os ponteiros
			vecRead=(float*)(((unsigned char*)vecRead)+stride);
			vecWrite=(float*)(((unsigned char*)vecWrite)+stride);

			//faço as minhas continhas e guardo
			curVec = _mm_loadu_ps(vecRead);
			final=_mm_mul_ps(_mm_shuffle_ps(curVec,curVec,0x00),mat1);
			final=_mm_add_ps(final,_mm_mul_ps(_mm_shuffle_ps(curVec,curVec,0x55),mat2));
			final=_mm_add_ps(final,_mm_mul_ps(_mm_shuffle_ps(curVec,curVec,0xAA),mat3));
			final=_mm_add_ps(final,_mm_mul_ps(_mm_shuffle_ps(curVec,curVec,0xFF),mat4));
			_mm_storeu_ps(vecWrite, final);

			//avanço com os ponteiros
			vecRead=(float*)(((unsigned char*)vecRead)+stride);
			vecWrite=(float*)(((unsigned char*)vecWrite)+stride);

			//faço as minhas continhas e guardo
			curVec = _mm_loadu_ps(vecRead);
			final=_mm_mul_ps(_mm_shuffle_ps(curVec,curVec,0x00),mat1);
			final=_mm_add_ps(final,_mm_mul_ps(_mm_shuffle_ps(curVec,curVec,0x55),mat2));
			final=_mm_add_ps(final,_mm_mul_ps(_mm_shuffle_ps(curVec,curVec,0xAA),mat3));
			final=_mm_add_ps(final,_mm_mul_ps(_mm_shuffle_ps(curVec,curVec,0xFF),mat4));
			_mm_storeu_ps(vecWrite, final);

			//avanço com os ponteiros
			vecRead=(float*)(((unsigned char*)vecRead)+stride);
			vecWrite=(float*)(((unsigned char*)vecWrite)+stride);

			//faço as minhas continhas e guardo
			curVec = _mm_loadu_ps(vecRead);
			final=_mm_mul_ps(_mm_shuffle_ps(curVec,curVec,0x00),mat1);
			final=_mm_add_ps(final,_mm_mul_ps(_mm_shuffle_ps(curVec,curVec,0x55),mat2));
			final=_mm_add_ps(final,_mm_mul_ps(_mm_shuffle_ps(curVec,curVec,0xAA),mat3));
			final=_mm_add_ps(final,_mm_mul_ps(_mm_shuffle_ps(curVec,curVec,0xFF),mat4));
			_mm_storeu_ps(vecWrite, final);

			//avanço com os ponteiros
			vecRead=(float*)(((unsigned char*)vecRead)+stride);
			vecWrite=(float*)(((unsigned char*)vecWrite)+stride);
		}
	}

	//basta só fazer os últimos
	for(; leftOver>0; leftOver--)
	{
		//faço as minhas continhas e guardo
		curVec = _mm_loadu_ps(vecRead);
		final=_mm_mul_ps(_mm_shuffle_ps(curVec,curVec,0x00),mat1);
		final=_mm_add_ps(final,_mm_mul_ps(_mm_shuffle_ps(curVec,curVec,0x55),mat2));
		final=_mm_add_ps(final,_mm_mul_ps(_mm_shuffle_ps(curVec,curVec,0xAA),mat3));
		final=_mm_add_ps(final,_mm_mul_ps(_mm_shuffle_ps(curVec,curVec,0xFF),mat4));
		_mm_storeu_ps(vecWrite, final);

		//avanço com os ponteiros
		vecRead=(float*)(((unsigned char*)vecRead)+stride);
		vecWrite=(float*)(((unsigned char*)vecWrite)+stride);
	}
}

void Matrix::asmMult(float * const result, const float * const mat1, const float * const mat2)
{
	__m128 row1, row2, row3, row4, curRow, curRowFinal;

	//leio toda a mat1
	row1 = _mm_loadu_ps(mat1 + 0);
	row2 = _mm_loadu_ps(mat1 + 4);
	row3 = _mm_loadu_ps(mat1 + 8);
	row4 = _mm_loadu_ps(mat1 + 12);

	//calculo a primeira linha
	curRow = _mm_loadu_ps(mat2 + 0);
	curRowFinal = _mm_mul_ps(_mm_shuffle_ps(curRow, curRow, 0x00), row1);
	curRowFinal = _mm_add_ps(curRowFinal, _mm_mul_ps(_mm_shuffle_ps(curRow, curRow, 0x55), row2));
	curRowFinal = _mm_add_ps(curRowFinal, _mm_mul_ps(_mm_shuffle_ps(curRow, curRow, 0xAA), row3));
	curRowFinal = _mm_add_ps(curRowFinal, _mm_mul_ps(_mm_shuffle_ps(curRow, curRow, 0xFF), row4));
	_mm_storeu_ps(result + 0, curRowFinal);

	//calculo a segunda linha
	curRow = _mm_loadu_ps(mat2 + 4);
	curRowFinal = _mm_mul_ps(_mm_shuffle_ps(curRow, curRow, 0x00), row1);
	curRowFinal = _mm_add_ps(curRowFinal, _mm_mul_ps(_mm_shuffle_ps(curRow, curRow, 0x55), row2));
	curRowFinal = _mm_add_ps(curRowFinal, _mm_mul_ps(_mm_shuffle_ps(curRow, curRow, 0xAA), row3));
	curRowFinal = _mm_add_ps(curRowFinal, _mm_mul_ps(_mm_shuffle_ps(curRow, curRow, 0xFF), row4));
	_mm_storeu_ps(result + 4, curRowFinal);

	//calculo a terceira linha
	curRow = _mm_loadu_ps(mat2 + 8);
	curRowFinal = _mm_mul_ps(_mm_shuffle_ps(curRow, curRow, 0x00), row1);
	curRowFinal = _mm_add_ps(curRowFinal, _mm_mul_ps(_mm_shuffle_ps(curRow, curRow, 0x55), row2));
	curRowFinal = _mm_add_ps(curRowFinal, _mm_mul_ps(_mm_shuffle_ps(curRow, curRow, 0xAA), row3));
	curRowFinal = _mm_add_ps(curRowFinal, _mm_mul_ps(_mm_shuffle_ps(curRow, curRow, 0xFF), row4));
	_mm_storeu_ps(result + 8, curRowFinal);

	//calculo a quarta linha
	curRow = _mm_loadu_ps(mat2 + 12);
	curRowFinal = _mm_mul_ps(_mm_shuffle_ps(curRow, curRow, 0x00), row1);
	curRowFinal = _mm_add_ps(curRowFinal, _mm_mul_ps(_mm_shuffle_ps(curRow, curRow, 0x55), row2));
	curRowFinal = _mm_add_ps(curRowFinal, _mm_mul_ps(_mm_shuffle_ps(curRow, curRow, 0xAA), row3));
	curRowFinal = _mm_add_ps(curRowFinal, _mm_mul_ps(_mm_shuffle_ps(curRow, curRow, 0xFF), row4));
	_mm_storeu_ps(result + 12, curRowFinal);
}

Matrix::Matrix(const float *s)
{	memcpy(m,s,sizeof(float)*16);}

Matrix::Matrix(const Matrix &mat)
{	memcpy(m,mat.m,sizeof(float)*16);}

Matrix::Matrix(const Matrix3 &mat)
{
	m[0]=mat.m[0];	m[1]=mat.m[1];	m[2]=mat.m[2];
	m[4]=mat.m[3];	m[5]=mat.m[4];	m[6]=mat.m[5];
	m[8]=mat.m[6];	m[9]=mat.m[7];	m[10]=mat.m[8];
	m[3]=m[7]=m[11]=m[12]=m[13]=m[14]=0.0f;
	m[15]=1.0f;
}

void Matrix::operator*=(const Matrix &s)
{	Matrix::asmMult(this->m, this->m, s.m);	}

void Matrix::operator*=(const Matrix3 &s)
{
	__m128 mat1,mat2,mat3,row1,row2,row3;

	mat1 = _mm_loadu_ps(m+0);
	mat2 = _mm_loadu_ps(m+4);
	mat3 = _mm_loadu_ps(m+8);

	row1 = _mm_mul_ps(_mm_load_ps1(s.m+0), mat1);
	row2 = _mm_mul_ps(_mm_load_ps1(s.m+1), mat2);
	row3 = _mm_mul_ps(_mm_load_ps1(s.m+2), mat3);
	_mm_storeu_ps(m+0, _mm_add_ps(_mm_add_ps(row1,row2), row3));

	row1 = _mm_mul_ps(_mm_load_ps1(s.m+3), mat1);
	row2 = _mm_mul_ps(_mm_load_ps1(s.m+4), mat2);
	row3 = _mm_mul_ps(_mm_load_ps1(s.m+5), mat3);
	_mm_storeu_ps(m+4, _mm_add_ps(_mm_add_ps(row1,row2), row3));

	row1 = _mm_mul_ps(_mm_load_ps1(s.m+6), mat1);
	row2 = _mm_mul_ps(_mm_load_ps1(s.m+7), mat2);
	row3 = _mm_mul_ps(_mm_load_ps1(s.m+8), mat3);
	_mm_storeu_ps(m+8, _mm_add_ps(_mm_add_ps(row1,row2), row3));
}

void Matrix::operator*=(const float *s)
{	Matrix::asmMult(this->m, this->m, s);	}

void Matrix::operator+=(const Matrix &s)
{
	_mm_storeu_ps(m, _mm_add_ps(_mm_loadu_ps(m),_mm_loadu_ps(s.m)));
	_mm_storeu_ps(m+4, _mm_add_ps(_mm_loadu_ps(m+4),_mm_loadu_ps(s.m+4)));
	_mm_storeu_ps(m+8, _mm_add_ps(_mm_loadu_ps(m+8),_mm_loadu_ps(s.m+8)));
	_mm_storeu_ps(m+12, _mm_add_ps(_mm_loadu_ps(m+12),_mm_loadu_ps(s.m+12)));
}

void Matrix::operator+=(const float *s)
{
	_mm_storeu_ps(m, _mm_add_ps(_mm_loadu_ps(m),_mm_loadu_ps(s)));
	_mm_storeu_ps(m+4, _mm_add_ps(_mm_loadu_ps(m+4),_mm_loadu_ps(s+4)));
	_mm_storeu_ps(m+8, _mm_add_ps(_mm_loadu_ps(m+8),_mm_loadu_ps(s+8)));
	_mm_storeu_ps(m+12, _mm_add_ps(_mm_loadu_ps(m+12),_mm_loadu_ps(s+12)));
}

void Matrix::operator-=(const Matrix &s)
{
	_mm_storeu_ps(m, _mm_sub_ps(_mm_loadu_ps(m),_mm_loadu_ps(s.m)));
	_mm_storeu_ps(m+4, _mm_sub_ps(_mm_loadu_ps(m+4),_mm_loadu_ps(s.m+4)));
	_mm_storeu_ps(m+8, _mm_sub_ps(_mm_loadu_ps(m+8),_mm_loadu_ps(s.m+8)));
	_mm_storeu_ps(m+12, _mm_sub_ps(_mm_loadu_ps(m+12),_mm_loadu_ps(s.m+12)));
}

void Matrix::operator-=(const float *s)
{
	_mm_storeu_ps(m, _mm_sub_ps(_mm_loadu_ps(m),_mm_loadu_ps(s)));
	_mm_storeu_ps(m+4, _mm_sub_ps(_mm_loadu_ps(m+4),_mm_loadu_ps(s+4)));
	_mm_storeu_ps(m+8, _mm_sub_ps(_mm_loadu_ps(m+8),_mm_loadu_ps(s+8)));
	_mm_storeu_ps(m+12, _mm_sub_ps(_mm_loadu_ps(m+12),_mm_loadu_ps(s+12)));
}

void Matrix::TransformVector(float *vec) const
{
	float vecX,vecY,vecZ;

	vecX=vec[0];
	vecY=vec[1];
	vecZ=vec[2];

	vec[0] = vecX*m[0] + vecY*m[4] + vecZ*m[8] + m[12];
	vec[1] = vecX*m[1] + vecY*m[5] + vecZ*m[9] + m[13];
	vec[2] = vecX*m[2] + vecY*m[6] + vecZ*m[10]+ m[14];
}

void Matrix::TransformVector(Vector& vec) const
{
	__m128 row1,row2,row3;

	row1 = _mm_mul_ps(_mm_load_ps1(&vec.x), _mm_loadu_ps(m));
	row2 = _mm_mul_ps(_mm_load_ps1(&vec.y), _mm_loadu_ps(m+4));
	row3 = _mm_mul_ps(_mm_load_ps1(&vec.z), _mm_loadu_ps(m+8));
	_mm_storeu_ps(&vec.x, _mm_add_ps(_mm_add_ps(row1,row2), _mm_add_ps(row3,_mm_loadu_ps(m+12))) );
}

void Matrix::TransformVector(const Vector& vec, Vector &result) const
{
	__m128 row1,row2,row3;

	row1 = _mm_mul_ps(_mm_load_ps1(&vec.x), _mm_loadu_ps(m));
	row2 = _mm_mul_ps(_mm_load_ps1(&vec.y), _mm_loadu_ps(m+4));
	row3 = _mm_mul_ps(_mm_load_ps1(&vec.z), _mm_loadu_ps(m+8));
	_mm_storeu_ps(&result.x, _mm_add_ps(_mm_add_ps(row1,row2), _mm_add_ps(row3,_mm_loadu_ps(m+12))) );
}

void Matrix::TransformVector(Vector * const vec, const int numVec) const
{	Matrix::asmMat4x4Vec3((float*)vec,(float*)vec,1.0f,sizeof(Vector),m,numVec);}

void Matrix::TransformVector(Vector4& vec) const
{
	__m128 row1,row2,row3,row4;

	row1 = _mm_mul_ps(_mm_load_ps1(&vec.x), _mm_loadu_ps(m));
	row2 = _mm_mul_ps(_mm_load_ps1(&vec.y), _mm_loadu_ps(m+4));
	row3 = _mm_mul_ps(_mm_load_ps1(&vec.z), _mm_loadu_ps(m+8));
	row4 = _mm_mul_ps(_mm_load_ps1(&vec.w), _mm_loadu_ps(m+12));
	_mm_storeu_ps(&vec.x, _mm_add_ps(_mm_add_ps(row1,row2), _mm_add_ps(row3,row4)) );
}

void Matrix::TransformVector(const Vector4& vec, Vector4 &result) const
{
	__m128 row1,row2,row3,row4;

	row1 = _mm_mul_ps(_mm_load_ps1(&vec.x), _mm_loadu_ps(m));
	row2 = _mm_mul_ps(_mm_load_ps1(&vec.y), _mm_loadu_ps(m+4));
	row3 = _mm_mul_ps(_mm_load_ps1(&vec.z), _mm_loadu_ps(m+8));
	row4 = _mm_mul_ps(_mm_load_ps1(&vec.w), _mm_loadu_ps(m+12));
	_mm_storeu_ps(&result.x, _mm_add_ps(_mm_add_ps(row1,row2), _mm_add_ps(row3,row4)) );
}

void Matrix::TransformVector(Vector4 * const vec, const int numVec) const
{	Matrix::asmMat4x4Vec4((float*)vec,(float*)vec,sizeof(Vector4),m,numVec);}

void Matrix::TransformVector3D(float *vec) const
{
	float vecX,vecY,vecZ;

	vecX=vec[0];
	vecY=vec[1];
	vecZ=vec[2];

	vec[0] = vecX*m[0] + vecY*m[4] + vecZ*m[8];
	vec[1] = vecX*m[1] + vecY*m[5] + vecZ*m[9];
	vec[2] = vecX*m[2] + vecY*m[6] + vecZ*m[10];
}

void Matrix::TransformVector3D(Vector &vec) const
{
	__m128 row1,row2,row3;

	row1 = _mm_mul_ps(_mm_load_ps1(&vec.x), _mm_loadu_ps(m));
	row2 = _mm_mul_ps(_mm_load_ps1(&vec.y), _mm_loadu_ps(m+4));
	row3 = _mm_mul_ps(_mm_load_ps1(&vec.z), _mm_loadu_ps(m+8));
	_mm_storeu_ps(&vec.x, _mm_add_ps(_mm_add_ps(row1,row2), row3) );
}

void Matrix::TransformVector3D(const Vector &vec, Vector &result) const
{
	__m128 row1,row2,row3;

	row1 = _mm_mul_ps(_mm_load_ps1(&vec.x), _mm_loadu_ps(m));
	row2 = _mm_mul_ps(_mm_load_ps1(&vec.y), _mm_loadu_ps(m+4));
	row3 = _mm_mul_ps(_mm_load_ps1(&vec.z), _mm_loadu_ps(m+8));
	_mm_storeu_ps(&result.x, _mm_add_ps(_mm_add_ps(row1,row2), row3) );
}

void Matrix::TransformVector3D(Vector * const vec, const int numVec) const
{	Matrix::asmMat4x4Vec3((float*)vec,(float*)vec,0.0f,sizeof(Vector),m,numVec);}

void Matrix::TransformBBox(BBox &bbox) const
{
	Vector pts[8];

	//obtenho os cantos da bbox e transformo-os
	bbox.GetCorners(pts);
	this->TransformVector(pts,8);

	//basta fazer reset e repasar os pontos
	bbox.Reset();
	bbox.Merge(pts,8);
}

void Matrix::TransformBBox(const BBox &bbox, BBox &bboxDest) const
{
	Vector pts[8];

	//obtenho os cantos da bbox e transformo-os
	bbox.GetCorners(pts);
	this->TransformVector(pts,8);

	//basta fazer reset e repasar os pontos
	bboxDest.Reset();
	bboxDest.Merge(pts,8);
}

void Matrix::GetCol1(Vector4 &result) const
{
	result.x=m[0];
	result.y=m[4];
	result.z=m[8];
	result.w=m[12];
}

void Matrix::GetCol2(Vector4 &result) const
{
	result.x=m[1];
	result.y=m[5];
	result.z=m[9];
	result.w=m[13];
}

void Matrix::GetCol3(Vector4 &result) const
{
	result.x=m[2];
	result.y=m[6];
	result.z=m[10];
	result.w=m[14];
}

void Matrix::GetCol4(Vector4 &result) const
{
	result.x=m[3];
	result.y=m[7];
	result.z=m[11];
	result.w=m[15];
}

void Matrix::GetRow1(Vector4 &result) const
{
	result.x=m[0];
	result.y=m[1];
	result.z=m[2];
	result.w=m[3];
}

void Matrix::GetRow2(Vector4 &result) const
{
	result.x=m[4];
	result.y=m[5];
	result.z=m[6];
	result.w=m[7];
}

void Matrix::GetRow3(Vector4 &result) const
{
	result.x=m[8];
	result.y=m[9];
	result.z=m[10];
	result.w=m[11];
}

void Matrix::GetRow4(Vector4 &result) const
{
	result.x=m[12];
	result.y=m[13];
	result.z=m[14];
	result.w=m[15];
}

void Matrix::GetCol1(Vector &result) const
{
	result.x=m[0];
	result.y=m[4];
	result.z=m[8];
}

void Matrix::GetCol2(Vector &result) const
{
	result.x=m[1];
	result.y=m[5];
	result.z=m[9];
}

void Matrix::GetCol3(Vector &result) const
{
	result.x=m[2];
	result.y=m[6];
	result.z=m[10];
}

void Matrix::GetCol4(Vector &result) const
{
	result.x=m[3];
	result.y=m[7];
	result.z=m[11];
}

void Matrix::GetRow1(Vector &result) const
{
	result.x=m[0];
	result.y=m[1];
	result.z=m[2];
}

void Matrix::GetRow2(Vector &result) const
{
	result.x=m[4];
	result.y=m[5];
	result.z=m[6];
}

void Matrix::GetRow3(Vector &result) const
{
	result.x=m[8];
	result.y=m[9];
	result.z=m[10];
}

void Matrix::GetRow4(Vector &result) const
{
	result.x=m[12];
	result.y=m[13];
	result.z=m[14];
}

void Matrix::GetRotate(Vector &vec, float &angulo) const
{
	float s,tr,quat[4];
	int op;

	//calculo o quaternion a partir da matriz
	tr = m[0] + m[5] + m[10];

	if (tr>0.0f) 
		{
		s=sqrt(tr+1.0f);

		quat[3]=s*0.5f;
	    
		s=0.5/s;

		quat[0]=(m[6]-m[9])*s;
		quat[1]=(m[8]-m[2])*s;
		quat[2]=(m[1]-m[4])*s;
		}
	else
		{
		op=0;
		if (m[5]>m[0])
			op=1;
		if (m[10]>m[op*5])
			op=2;

		switch(op){
			case 0:
					s=sqrt((m[0]-(m[5]+m[10]))+1.0f);

					quat[0]=s*0.5f;
					if (s!=0.0)
						s=0.5/s;

					quat[1]=(m[1]+m[4])*s;
					quat[2]=(m[2]+m[8])*s;
					quat[3]=(m[6]-m[9])*s;
			
					break;

			case 1:
					s=sqrt((m[5]-(m[10]+m[0]))+1.0f);

					quat[1]=s*0.5f;
					if (s!=0.0f)
						s=0.5f/s;

					quat[3]=(m[8]-m[2])*s;
					quat[2]=(m[6]+m[9])*s;
					quat[0]=(m[4]+m[1])*s;

					break;

			case 2:
					s=sqrt((m[10]-(m[0]+m[5]))+1.0f);

					quat[2]=s*0.5f;
					if (s!=0.0f)
						s=0.5f/s;

					quat[3]=(m[1]-m[4])*s;
					quat[0]=(m[8]+m[2])*s;
					quat[1]=(m[9]+m[6])*s;
					break;
			}
		}

	//a partir do quaterniao calculo o angulo e vector
	tr=quat[0]*quat[0]+quat[1]*quat[1]+quat[2]*quat[2];
	if (tr==0.0f) 
		{
		vec.x=0.0f;
		vec.y=0.0f;
		vec.z=1.0f;
		angulo=0.0f;
		return;
		}
    
	tr=1.0f/tr;
	vec.x=quat[0]*tr;
	vec.y=quat[1]*tr;
	vec.z=quat[2]*tr;
	vec.Normaliza();
	angulo=((float)acos(quat[3]))*114.5915590261646417f; // 180/pi=57.295779513082320876f * 2.0f (o 2 é de formula)
}

void Matrix::GetEulerAngles(float &rfYAngle, float &rfPAngle, float &rfRAngle) const
{
	if ( -1.0f < -m[8] )
		{
		if ( -m[8] < 1.0 )
			rfPAngle = asin(-m[8]);
		else
			rfPAngle = -(2.0f*atanf(1.0f));
		}
	else
		{
		rfPAngle = 2.0f*atanf(1.0f);
		}

	if ( rfPAngle < (2.0f*atanf(1.0f)) )
		{
		if ( rfPAngle > -(2.0f*atanf(1.0f)) )
			{
			rfYAngle = atan2(m[4],m[0]);
			rfRAngle = atan2(m[9],m[10]);
			}
		else
			{
			float fRmY = atan2(-m[1],m[2]);
			rfRAngle = 0.0f;
			rfYAngle = rfRAngle - fRmY;
			}
		}

	else
		{
		float fRpY = atan2(-m[0],m[2]);
		rfRAngle = 0.0f;
		rfYAngle = fRpY - rfRAngle;
		}
}

void Matrix::GetFrom3x3(Matrix3 &mat3) const
{
	mat3.m[0]=m[0];
	mat3.m[1]=m[1];
	mat3.m[2]=m[2];
	mat3.m[3]=m[4];
	mat3.m[4]=m[5];
	mat3.m[5]=m[6];
	mat3.m[6]=m[8];
	mat3.m[7]=m[9];
	mat3.m[8]=m[10];
}

void Matrix::GetFrom3x3(float * const src) const
{
	src[0]=m[0];
	src[1]=m[1];
	src[2]=m[2];
	src[3]=m[4];
	src[4]=m[5];
	src[5]=m[6];
	src[6]=m[8];
	src[7]=m[9];
	src[8]=m[10];
}

void Matrix::GetFrom2x2(float * const src) const
{
	src[0]=m[0];
	src[1]=m[1];
	src[2]=m[4];
	src[3]=m[5];
}

void Matrix::Write(float * const s) const
{	memcpy(s,m,sizeof(float)*16);}

void Matrix::Transpose(Matrix &dest) const
{
	__m128 row1,row2,row3,row4;

	row1 = _mm_loadu_ps(m+0);
	row2 = _mm_loadu_ps(m+4);
	row3 = _mm_loadu_ps(m+8);
	row4 = _mm_loadu_ps(m+12);
	_MM_TRANSPOSE4_PS(row1, row2, row3, row4);
	_mm_storeu_ps(dest.m+0, row1);
	_mm_storeu_ps(dest.m+4, row2);
	_mm_storeu_ps(dest.m+8, row3);
	_mm_storeu_ps(dest.m+12, row4);
}

void Matrix::Transpose()
{
	__m128 row1,row2,row3,row4;

	row1 = _mm_loadu_ps(m+0);
	row2 = _mm_loadu_ps(m+4);
	row3 = _mm_loadu_ps(m+8);
	row4 = _mm_loadu_ps(m+12);
	_MM_TRANSPOSE4_PS(row1, row2, row3, row4);
	_mm_storeu_ps(m+0, row1);
	_mm_storeu_ps(m+4, row2);
	_mm_storeu_ps(m+8, row3);
	_mm_storeu_ps(m+12, row4);
}

void Matrix::Inverse(Matrix &dest) const
{
	float tmp[12],det;

	//calculate pairs for first 8 elements (cofactors)
	tmp[0] = m[10]* m[15];	tmp[1] = m[11]* m[14];
	tmp[2] = m[9] * m[15];	tmp[3] = m[11]* m[13];
	tmp[4] = m[9] * m[14];	tmp[5] = m[10]* m[13];
	tmp[6] = m[8] * m[15];	tmp[7] = m[11]* m[12];
	tmp[8] = m[8] * m[14];	tmp[9] = m[10]* m[12];
	tmp[10]= m[8] * m[13];	tmp[11]= m[9] * m[12];

	//calculate first 8 elements (cofactors)
	dest.m[0] =tmp[0]*m[5] + tmp[3]*m[6] + tmp[4]*m[7] - tmp[1]*m[5] - tmp[2]*m[6] - tmp[5]*m[7];
	dest.m[4] =tmp[1]*m[4] + tmp[6]*m[6] + tmp[9]*m[7] - tmp[0]*m[4] - tmp[7]*m[6] - tmp[8]*m[7];
	dest.m[8] =tmp[2]*m[4] + tmp[7]*m[5] + tmp[10]*m[7]- tmp[3]*m[4] - tmp[6]*m[5] - tmp[11]*m[7];
	dest.m[12]=tmp[5]*m[4] + tmp[8]*m[5] + tmp[11]*m[6]- tmp[4]*m[4] - tmp[9]*m[5] - tmp[10]*m[6];
	dest.m[1] =tmp[1]*m[1] + tmp[2]*m[2] + tmp[5]*m[3] - tmp[0]*m[1] - tmp[3]*m[2] - tmp[4]*m[3];
	dest.m[5] =tmp[0]*m[0] + tmp[7]*m[2] + tmp[8]*m[3] - tmp[1]*m[0] - tmp[6]*m[2] - tmp[9]*m[3];
	dest.m[9] =tmp[3]*m[0] + tmp[6]*m[1] + tmp[11]*m[3]- tmp[2]*m[0] - tmp[7]*m[1] - tmp[10]*m[3];
	dest.m[13]=tmp[4]*m[0] + tmp[9]*m[1] + tmp[10]*m[2]- tmp[5]*m[0] - tmp[8]*m[1] - tmp[11]*m[2];

	//calculate pairs for second 8 elements (cofactors)
	tmp[0] = m[2]*m[7];		tmp[1] = m[3]*m[6];
	tmp[2] = m[1]*m[7];		tmp[3] = m[3]*m[5];
	tmp[4] = m[1]*m[6];		tmp[5] = m[2]*m[5];
	tmp[6] = m[0]*m[7];		tmp[7] = m[3]*m[4];
	tmp[8] = m[0]*m[6];		tmp[9] = m[2]*m[4];
	tmp[10]= m[0]*m[5];		tmp[11]= m[1]*m[4];

	//calculate second 8 elements (cofactors)
	dest.m[2] = tmp[0]*m[13] + tmp[3]*m[14] + tmp[4]*m[15]  - tmp[1]*m[13] - tmp[2]*m[14] - tmp[5]*m[15];
	dest.m[6] = tmp[1]*m[12] + tmp[6]*m[14] + tmp[9]*m[15]  - tmp[0]*m[12] - tmp[7]*m[14] - tmp[8]*m[15];
	dest.m[10]=tmp[2]*m[12] + tmp[7]*m[13] + tmp[10]*m[15] - tmp[3]*m[12] - tmp[6]*m[13] - tmp[11]*m[15];
	dest.m[14]=tmp[5]*m[12] + tmp[8]*m[13] + tmp[11]*m[14] - tmp[4]*m[12] - tmp[9]*m[13] - tmp[10]*m[14];
	dest.m[3] =tmp[2]*m[10] + tmp[5]*m[11] + tmp[1]*m[9]   - tmp[4]*m[11] - tmp[0]*m[9]  - tmp[3]*m[10];
	dest.m[7] =tmp[8]*m[11] + tmp[0]*m[8]  + tmp[7]*m[10]  - tmp[6]*m[10] - tmp[9]*m[11] - tmp[1]*m[8];
	dest.m[11]=tmp[6]*m[9]  + tmp[11]*m[11]+ tmp[3]*m[8]   - tmp[10]*m[11]- tmp[2]*m[8]  - tmp[7]*m[9];
	dest.m[15]=tmp[10]*m[10]+ tmp[4]*m[8]  + tmp[9]*m[9]   - tmp[8]*m[9]  - tmp[11]*m[10]- tmp[5]*m[8];

	// calculate determinant
	det=m[0]*dest.m[0] + m[1]*dest.m[4] + m[2]*dest.m[8] + m[3]*dest.m[12];
	if(Math::isZero(det))
		return;
	
	//multiplicar tudo pelo determinante
	det=1.0f/det;
	dest.m[0]*=det;		dest.m[1]*=det;		dest.m[2]*=det;		dest.m[3]*=det;
	dest.m[4]*=det;		dest.m[5]*=det;		dest.m[6]*=det;		dest.m[7]*=det;
	dest.m[8]*=det;		dest.m[9]*=det;		dest.m[10]*=det;	dest.m[11]*=det;
	dest.m[12]*=det;	dest.m[13]*=det;	dest.m[14]*=det;	dest.m[15]*=det;
}

void Matrix::Inverse()
{
	float tmp[12],result[16],det;

	//calculate pairs for first 8 elements (cofactors)
	tmp[0] = m[10]* m[15];	tmp[1] = m[11]* m[14];
	tmp[2] = m[9] * m[15];	tmp[3] = m[11]* m[13];
	tmp[4] = m[9] * m[14];	tmp[5] = m[10]* m[13];
	tmp[6] = m[8] * m[15];	tmp[7] = m[11]* m[12];
	tmp[8] = m[8] * m[14];	tmp[9] = m[10]* m[12];
	tmp[10]= m[8] * m[13];	tmp[11]= m[9] * m[12];

	//calculate first 8 elements (cofactors)
	result[0] =tmp[0]*m[5] + tmp[3]*m[6] + tmp[4]*m[7] - tmp[1]*m[5] - tmp[2]*m[6] - tmp[5]*m[7];
	result[4] =tmp[1]*m[4] + tmp[6]*m[6] + tmp[9]*m[7] - tmp[0]*m[4] - tmp[7]*m[6] - tmp[8]*m[7];
	result[8] =tmp[2]*m[4] + tmp[7]*m[5] + tmp[10]*m[7]- tmp[3]*m[4] - tmp[6]*m[5] - tmp[11]*m[7];
	result[12]=tmp[5]*m[4] + tmp[8]*m[5] + tmp[11]*m[6]- tmp[4]*m[4] - tmp[9]*m[5] - tmp[10]*m[6];
	result[1] =tmp[1]*m[1] + tmp[2]*m[2] + tmp[5]*m[3] - tmp[0]*m[1] - tmp[3]*m[2] - tmp[4]*m[3];
	result[5] =tmp[0]*m[0] + tmp[7]*m[2] + tmp[8]*m[3] - tmp[1]*m[0] - tmp[6]*m[2] - tmp[9]*m[3];
	result[9] =tmp[3]*m[0] + tmp[6]*m[1] + tmp[11]*m[3]- tmp[2]*m[0] - tmp[7]*m[1] - tmp[10]*m[3];
	result[13]=tmp[4]*m[0] + tmp[9]*m[1] + tmp[10]*m[2]- tmp[5]*m[0] - tmp[8]*m[1] - tmp[11]*m[2];

	//calculate pairs for second 8 elements (cofactors)
	tmp[0] = m[2]*m[7];		tmp[1] = m[3]*m[6];
	tmp[2] = m[1]*m[7];		tmp[3] = m[3]*m[5];
	tmp[4] = m[1]*m[6];		tmp[5] = m[2]*m[5];
	tmp[6] = m[0]*m[7];		tmp[7] = m[3]*m[4];
	tmp[8] = m[0]*m[6];		tmp[9] = m[2]*m[4];
	tmp[10]= m[0]*m[5];		tmp[11]= m[1]*m[4];

	//calculate second 8 elements (cofactors)
	result[2] = tmp[0]*m[13] + tmp[3]*m[14] + tmp[4]*m[15]  - tmp[1]*m[13] - tmp[2]*m[14] - tmp[5]*m[15];
	result[6] = tmp[1]*m[12] + tmp[6]*m[14] + tmp[9]*m[15]  - tmp[0]*m[12] - tmp[7]*m[14] - tmp[8]*m[15];
	result[10]=tmp[2]*m[12] + tmp[7]*m[13] + tmp[10]*m[15] - tmp[3]*m[12] - tmp[6]*m[13] - tmp[11]*m[15];
	result[14]=tmp[5]*m[12] + tmp[8]*m[13] + tmp[11]*m[14] - tmp[4]*m[12] - tmp[9]*m[13] - tmp[10]*m[14];
	result[3] =tmp[2]*m[10] + tmp[5]*m[11] + tmp[1]*m[9]   - tmp[4]*m[11] - tmp[0]*m[9]  - tmp[3]*m[10];
	result[7] =tmp[8]*m[11] + tmp[0]*m[8]  + tmp[7]*m[10]  - tmp[6]*m[10] - tmp[9]*m[11] - tmp[1]*m[8];
	result[11]=tmp[6]*m[9]  + tmp[11]*m[11]+ tmp[3]*m[8]   - tmp[10]*m[11]- tmp[2]*m[8]  - tmp[7]*m[9];
	result[15]=tmp[10]*m[10]+ tmp[4]*m[8]  + tmp[9]*m[9]   - tmp[8]*m[9]  - tmp[11]*m[10]- tmp[5]*m[8];

	// calculate determinant
	det=m[0]*result[0] + m[1]*result[4] + m[2]*result[8] + m[3]*result[12];
	if(Math::isZero(det))
		return;
	
	//multiplicar tudo pelo determinante
	det=1.0f/det;
	result[0]*=det;		result[1]*=det;		result[2]*=det;		result[3]*=det;
	result[4]*=det;		result[5]*=det;		result[6]*=det;		result[7]*=det;
	result[8]*=det;		result[9]*=det;		result[10]*=det;	result[11]*=det;
	result[12]*=det;	result[13]*=det;	result[14]*=det;	result[15]*=det;

	//basta copiar para mim próprio e pronto
	memcpy(m,result,sizeof(float)*16);
}

void Matrix::InverseTranspose(Matrix &dest) const
{
	float tmp[12],det;

	//calculate pairs for first 8 elements (cofactors)
	tmp[0] = m[10]* m[15];	tmp[1] = m[11]* m[14];
	tmp[2] = m[9] * m[15];	tmp[3] = m[11]* m[13];
	tmp[4] = m[9] * m[14];	tmp[5] = m[10]* m[13];
	tmp[6] = m[8] * m[15];	tmp[7] = m[11]* m[12];
	tmp[8] = m[8] * m[14];	tmp[9] = m[10]* m[12];
	tmp[10]= m[8] * m[13];	tmp[11]= m[9] * m[12];

	//calculate first 8 elements (cofactors)
	dest.m[0]=tmp[0]*m[5] + tmp[3]*m[6] + tmp[4]*m[7] - tmp[1]*m[5] - tmp[2]*m[6] - tmp[5]*m[7];
	dest.m[1]=tmp[1]*m[4] + tmp[6]*m[6] + tmp[9]*m[7] - tmp[0]*m[4] - tmp[7]*m[6] - tmp[8]*m[7];
	dest.m[2]=tmp[2]*m[4] + tmp[7]*m[5] + tmp[10]*m[7]- tmp[3]*m[4] - tmp[6]*m[5] - tmp[11]*m[7];
	dest.m[3]=tmp[5]*m[4] + tmp[8]*m[5] + tmp[11]*m[6]- tmp[4]*m[4] - tmp[9]*m[5] - tmp[10]*m[6];
	dest.m[4]=tmp[1]*m[1] + tmp[2]*m[2] + tmp[5]*m[3] - tmp[0]*m[1] - tmp[3]*m[2] - tmp[4]*m[3];
	dest.m[5]=tmp[0]*m[0] + tmp[7]*m[2] + tmp[8]*m[3] - tmp[1]*m[0] - tmp[6]*m[2] - tmp[9]*m[3];
	dest.m[6]=tmp[3]*m[0] + tmp[6]*m[1] + tmp[11]*m[3]- tmp[2]*m[0] - tmp[7]*m[1] - tmp[10]*m[3];
	dest.m[7]=tmp[4]*m[0] + tmp[9]*m[1] + tmp[10]*m[2]- tmp[5]*m[0] - tmp[8]*m[1] - tmp[11]*m[2];

	//calculate pairs for second 8 elements (cofactors)
	tmp[0] = m[2]*m[7];		tmp[1] = m[3]*m[6];
	tmp[2] = m[1]*m[7];		tmp[3] = m[3]*m[5];
	tmp[4] = m[1]*m[6];		tmp[5] = m[2]*m[5];
	tmp[6] = m[0]*m[7];		tmp[7] = m[3]*m[4];
	tmp[8] = m[0]*m[6];		tmp[9] = m[2]*m[4];
	tmp[10]= m[0]*m[5];		tmp[11]= m[1]*m[4];

	//calculate second 8 elements (cofactors)
	dest.m[8]= tmp[0]*m[13] + tmp[3]*m[14] + tmp[4]*m[15]  - tmp[1]*m[13] - tmp[2]*m[14] - tmp[5]*m[15];
	dest.m[9]= tmp[1]*m[12] + tmp[6]*m[14] + tmp[9]*m[15]  - tmp[0]*m[12] - tmp[7]*m[14] - tmp[8]*m[15];
	dest.m[10]=tmp[2]*m[12] + tmp[7]*m[13] + tmp[10]*m[15] - tmp[3]*m[12] - tmp[6]*m[13] - tmp[11]*m[15];
	dest.m[11]=tmp[5]*m[12] + tmp[8]*m[13] + tmp[11]*m[14] - tmp[4]*m[12] - tmp[9]*m[13] - tmp[10]*m[14];
	dest.m[12]=tmp[2]*m[10] + tmp[5]*m[11] + tmp[1]*m[9]   - tmp[4]*m[11] - tmp[0]*m[9]  - tmp[3]*m[10];
	dest.m[13]=tmp[8]*m[11] + tmp[0]*m[8]  + tmp[7]*m[10]  - tmp[6]*m[10] - tmp[9]*m[11] - tmp[1]*m[8];
	dest.m[14]=tmp[6]*m[9]  + tmp[11]*m[11]+ tmp[3]*m[8]   - tmp[10]*m[11]- tmp[2]*m[8]  - tmp[7]*m[9];
	dest.m[15]=tmp[10]*m[10]+ tmp[4]*m[8]  + tmp[9]*m[9]   - tmp[8]*m[9]  - tmp[11]*m[10]- tmp[5]*m[8];

	// calculate determinant
	det=m[0]*dest.m[0] + m[1]*dest.m[1] + m[2]*dest.m[2] + m[3]*dest.m[3];
	if(Math::isZero(det))
		return;
	
	//multiplicar tudo pelo determinante
	det=1.0f/det;
	dest.m[0]*=det;		dest.m[1]*=det;		dest.m[2]*=det;		dest.m[3]*=det;
	dest.m[4]*=det;		dest.m[5]*=det;		dest.m[6]*=det;		dest.m[7]*=det;
	dest.m[8]*=det;		dest.m[9]*=det;		dest.m[10]*=det;	dest.m[11]*=det;
	dest.m[12]*=det;	dest.m[13]*=det;	dest.m[14]*=det;	dest.m[15]*=det;
}

void Matrix::InverseTranspose()
{
	float tmp[12],result[16],det;

	//calculate pairs for first 8 elements (cofactors)
	tmp[0] = m[10]* m[15];	tmp[1] = m[11]* m[14];
	tmp[2] = m[9] * m[15];	tmp[3] = m[11]* m[13];
	tmp[4] = m[9] * m[14];	tmp[5] = m[10]* m[13];
	tmp[6] = m[8] * m[15];	tmp[7] = m[11]* m[12];
	tmp[8] = m[8] * m[14];	tmp[9] = m[10]* m[12];
	tmp[10]= m[8] * m[13];	tmp[11]= m[9] * m[12];

	//calculate first 8 elements (cofactors)
	result[0]=tmp[0]*m[5] + tmp[3]*m[6] + tmp[4]*m[7] - tmp[1]*m[5] - tmp[2]*m[6] - tmp[5]*m[7];
	result[1]=tmp[1]*m[4] + tmp[6]*m[6] + tmp[9]*m[7] - tmp[0]*m[4] - tmp[7]*m[6] - tmp[8]*m[7];
	result[2]=tmp[2]*m[4] + tmp[7]*m[5] + tmp[10]*m[7]- tmp[3]*m[4] - tmp[6]*m[5] - tmp[11]*m[7];
	result[3]=tmp[5]*m[4] + tmp[8]*m[5] + tmp[11]*m[6]- tmp[4]*m[4] - tmp[9]*m[5] - tmp[10]*m[6];
	result[4]=tmp[1]*m[1] + tmp[2]*m[2] + tmp[5]*m[3] - tmp[0]*m[1] - tmp[3]*m[2] - tmp[4]*m[3];
	result[5]=tmp[0]*m[0] + tmp[7]*m[2] + tmp[8]*m[3] - tmp[1]*m[0] - tmp[6]*m[2] - tmp[9]*m[3];
	result[6]=tmp[3]*m[0] + tmp[6]*m[1] + tmp[11]*m[3]- tmp[2]*m[0] - tmp[7]*m[1] - tmp[10]*m[3];
	result[7]=tmp[4]*m[0] + tmp[9]*m[1] + tmp[10]*m[2]- tmp[5]*m[0] - tmp[8]*m[1] - tmp[11]*m[2];

	//calculate pairs for second 8 elements (cofactors)
	tmp[0] = m[2]*m[7];		tmp[1] = m[3]*m[6];
	tmp[2] = m[1]*m[7];		tmp[3] = m[3]*m[5];
	tmp[4] = m[1]*m[6];		tmp[5] = m[2]*m[5];
	tmp[6] = m[0]*m[7];		tmp[7] = m[3]*m[4];
	tmp[8] = m[0]*m[6];		tmp[9] = m[2]*m[4];
	tmp[10]= m[0]*m[5];		tmp[11]= m[1]*m[4];

	//calculate second 8 elements (cofactors)
	result[8]= tmp[0]*m[13] + tmp[3]*m[14] + tmp[4]*m[15]  - tmp[1]*m[13] - tmp[2]*m[14] - tmp[5]*m[15];
	result[9]= tmp[1]*m[12] + tmp[6]*m[14] + tmp[9]*m[15]  - tmp[0]*m[12] - tmp[7]*m[14] - tmp[8]*m[15];
	result[10]=tmp[2]*m[12] + tmp[7]*m[13] + tmp[10]*m[15] - tmp[3]*m[12] - tmp[6]*m[13] - tmp[11]*m[15];
	result[11]=tmp[5]*m[12] + tmp[8]*m[13] + tmp[11]*m[14] - tmp[4]*m[12] - tmp[9]*m[13] - tmp[10]*m[14];
	result[12]=tmp[2]*m[10] + tmp[5]*m[11] + tmp[1]*m[9]   - tmp[4]*m[11] - tmp[0]*m[9]  - tmp[3]*m[10];
	result[13]=tmp[8]*m[11] + tmp[0]*m[8]  + tmp[7]*m[10]  - tmp[6]*m[10] - tmp[9]*m[11] - tmp[1]*m[8];
	result[14]=tmp[6]*m[9]  + tmp[11]*m[11]+ tmp[3]*m[8]   - tmp[10]*m[11]- tmp[2]*m[8]  - tmp[7]*m[9];
	result[15]=tmp[10]*m[10]+ tmp[4]*m[8]  + tmp[9]*m[9]   - tmp[8]*m[9]  - tmp[11]*m[10]- tmp[5]*m[8];

	// calculate determinant
	det=m[0]*result[0] + m[1]*result[1] + m[2]*result[2] + m[3]*result[3];
	if(Math::isZero(det))
		return;
	
	//multiplicar tudo pelo determinante
	det=1.0f/det;
	result[0]*=det;		result[1]*=det;		result[2]*=det;		result[3]*=det;
	result[4]*=det;		result[5]*=det;		result[6]*=det;		result[7]*=det;
	result[8]*=det;		result[9]*=det;		result[10]*=det;	result[11]*=det;
	result[12]*=det;	result[13]*=det;	result[14]*=det;	result[15]*=det;

	//basta copiar para mim próprio e pronto
	memcpy(m,result,sizeof(float)*16);
}

void Matrix::InverseHomogenous(Matrix &dest) const
{
	float aux1,aux2;

	memcpy(dest.m,m,sizeof(float)*16);

	aux1=dest.m[1];	dest.m[1]=dest.m[4];	dest.m[4]=aux1;
	aux1=dest.m[2];	dest.m[2]=dest.m[8];	dest.m[8]=aux1;
	aux1=dest.m[6];	dest.m[6]=dest.m[9];	dest.m[9]=aux1;

	aux1 = -(dest.m[0] * dest.m[12] + dest.m[4] * dest.m[13] + dest.m[8] * dest.m[14]);
	aux2 = -(dest.m[1] * dest.m[12] + dest.m[5] * dest.m[13] + dest.m[9] * dest.m[14]);
	dest.m[14] = -(dest.m[2] * dest.m[12] + dest.m[6] * dest.m[13] + dest.m[10] * dest.m[14]);
	dest.m[13] = aux2;
	dest.m[12] = aux1;
}

void Matrix::InverseHomogenous()
{
	float aux1,aux2;

	aux1=m[1];	m[1]=m[4];	m[4]=aux1;
	aux1=m[2];	m[2]=m[8];	m[8]=aux1;
	aux1=m[6];	m[6]=m[9];	m[9]=aux1;

	aux1 = -(m[0] * m[12] + m[4] * m[13] + m[8] * m[14]);
	aux2 = -(m[1] * m[12] + m[5] * m[13] + m[9] * m[14]);
	m[14] = -(m[2] * m[12] + m[6] * m[13] + m[10] * m[14]);
	m[13] = aux2;
	m[12] = aux1;
}

void Matrix::MultTranslate(const float &x, const float &y, const float &z)
{
	__m128 row1,row2,row3;

	row1 = _mm_mul_ps(_mm_load_ps1(&x), _mm_loadu_ps(m));
	row2 = _mm_mul_ps(_mm_load_ps1(&y), _mm_loadu_ps(m+4));
	row3 = _mm_mul_ps(_mm_load_ps1(&z), _mm_loadu_ps(m+8));
	_mm_storeu_ps(m+12, _mm_add_ps(_mm_add_ps(row1,row2), _mm_add_ps(row3,_mm_loadu_ps(m+12))) );
}

void Matrix::MultTranslate(const float * const vec)
{
	__m128 row1,row2,row3;

	row1 = _mm_mul_ps(_mm_load_ps1(vec+0), _mm_loadu_ps(m));
	row2 = _mm_mul_ps(_mm_load_ps1(vec+1), _mm_loadu_ps(m+4));
	row3 = _mm_mul_ps(_mm_load_ps1(vec+2), _mm_loadu_ps(m+8));
	_mm_storeu_ps(m+12, _mm_add_ps(_mm_add_ps(row1,row2), _mm_add_ps(row3,_mm_loadu_ps(m+12))) );
}

void Matrix::MultScale(const float &x, const float &y, const float &z)
{
	_mm_storeu_ps(m+0, _mm_mul_ps(_mm_load_ps1(&x), _mm_loadu_ps(m+0)));
	_mm_storeu_ps(m+4, _mm_mul_ps(_mm_load_ps1(&y), _mm_loadu_ps(m+4)));
	_mm_storeu_ps(m+8, _mm_mul_ps(_mm_load_ps1(&z), _mm_loadu_ps(m+8)));
}

void Matrix::MultScale(const float * const vec)
{
	_mm_storeu_ps(m+0, _mm_mul_ps(_mm_load_ps1(vec+0), _mm_loadu_ps(m+0)));
	_mm_storeu_ps(m+4, _mm_mul_ps(_mm_load_ps1(vec+1), _mm_loadu_ps(m+4)));
	_mm_storeu_ps(m+8, _mm_mul_ps(_mm_load_ps1(vec+2), _mm_loadu_ps(m+8)));
}

void Matrix::MultRotateX(const float &angulo)
{
	float c,s,p1,p2,p3,p4;

	Math::sinCosG(angulo,s,c);

	p1=m[4];
	p2=m[5];
	p3=m[6];
	p4=m[7];

	m[4] = c * m[4] + s * m[8];
	m[5] = c * m[5] + s * m[9];
	m[6] = c * m[6] + s * m[10];
	m[7] = c * m[7] + s * m[11];

	m[8] = -s * p1 + c * m[8];
	m[9] = -s * p2 + c * m[9];
	m[10]= -s * p3 + c * m[10];
	m[11]= -s * p4 + c * m[11];
}

void Matrix::MultRotateY(const float &angulo)
{
	float c,s,p1,p2,p3,p4;

	Math::sinCosG(angulo,s,c);

	p1=m[0];
	p2=m[1];
	p3=m[2];
	p4=m[3];

	m[0] = c * m[0] + -s * m[8];
	m[1] = c * m[1] + -s * m[9];
	m[2] = c * m[2] + -s * m[10];
	m[3] = c * m[3] + -s * m[11];

	m[8] = s * p1 + c * m[8];
	m[9] = s * p2 + c * m[9];
	m[10]= s * p3 + c * m[10];
	m[11]= s * p4 + c * m[11];
}

void Matrix::MultRotateZ(const float &angulo)
{
	float c,s,p1,p2,p3,p4;

	Math::sinCosG(angulo,s,c);

	p1=m[0];
	p2=m[1];
	p3=m[2];
	p4=m[3];

	m[0] = c * m[0] + s * m[4];
	m[1] = c * m[1] + s * m[5];
	m[2] = c * m[2] + s * m[6];
	m[3] = c * m[3] + s * m[7];

	m[4] = -s * p1 + c * m[4];
	m[5] = -s * p2 + c * m[5];
	m[6] = -s * p3 + c * m[6];
	m[7] = -s * p4 + c * m[7];
}

void Matrix::Mult(const Matrix &s)
{	Matrix::asmMult(this->m, this->m, s.m);	}

void Matrix::Mult(const float *s)
{	Matrix::asmMult(this->m, this->m, s);	}

void Matrix::MultInverseOrder(const Matrix &s)
{	Matrix::asmMult(this->m, s.m, this->m);	}

void Matrix::MultInverseOrder(const float *s)
{	Matrix::asmMult(this->m, s, this->m);	}

void Matrix::Set(const float x)
{
	__m128 scalar;

	scalar = _mm_load_ps1(&x);
	_mm_storeu_ps(m+0, scalar);
	_mm_storeu_ps(m+4, scalar);
	_mm_storeu_ps(m+8, scalar);
	_mm_storeu_ps(m+12, scalar);
}

void Matrix::Set(const float *src)
{	memcpy(m,src,sizeof(float)*16);}

void Matrix::Set(const Matrix &mat)
{	memcpy(m,mat.m,sizeof(float)*16);}

void Matrix::SetZero(void)
{	memset(m,0,sizeof(float)*16);}

void Matrix::SetIdentidade(void)
{
	memset(m,0,sizeof(float)*16);
	m[0]=m[5]=m[10]=m[15]=1.0f;
}

void Matrix::SetFrom3x3(const Matrix3 &mat3)
{
	m[0]=mat3.m[0];
	m[1]=mat3.m[1];
	m[2]=mat3.m[2];
	m[4]=mat3.m[3];
	m[5]=mat3.m[4];
	m[6]=mat3.m[5];
	m[8]=mat3.m[6];
	m[9]=mat3.m[7];
	m[10]=mat3.m[8];

	m[3]=m[7]=m[11]=m[12]=m[13]=m[14]=0.0f;
	m[15]=1.0f;
}

void Matrix::SetFrom3x3(const float *src)
{
	m[0]=src[0];
	m[1]=src[1];
	m[2]=src[2];
	m[4]=src[3];
	m[5]=src[4];
	m[6]=src[5];
	m[8]=src[6];
	m[9]=src[7];
	m[10]=src[8];

	m[3]=m[7]=m[11]=m[12]=m[13]=m[14]=0.0f;
	m[15]=1.0f;
}

void Matrix::SetFrom2x2(const float *src)
{
	m[0]=src[0];
	m[1]=src[1];
	m[4]=src[2];
	m[5]=src[3];

	m[2]=m[3]=m[6]=m[7]=m[8]=m[9]=m[11]=m[12]=m[13]=m[14]=0.0f;
	m[10]=m[15]=1.0f;
}

void Matrix::SetTranspose(const float *src)
{
	__m128 row1,row2,row3,row4;

	row1 = _mm_loadu_ps(src+0);
	row2 = _mm_loadu_ps(src+4);
	row3 = _mm_loadu_ps(src+8);
	row4 = _mm_loadu_ps(src+12);
	_MM_TRANSPOSE4_PS(row1, row2, row3, row4);
	_mm_storeu_ps(m+0, row1);
	_mm_storeu_ps(m+4, row2);
	_mm_storeu_ps(m+8, row3);
	_mm_storeu_ps(m+12, row4);
}

void Matrix::SetTranspose(const Matrix &mat)
{
	__m128 row1,row2,row3,row4;

	row1 = _mm_loadu_ps(mat.m+0);
	row2 = _mm_loadu_ps(mat.m+4);
	row3 = _mm_loadu_ps(mat.m+8);
	row4 = _mm_loadu_ps(mat.m+12);
	_MM_TRANSPOSE4_PS(row1, row2, row3, row4);
	_mm_storeu_ps(m+0, row1);
	_mm_storeu_ps(m+4, row2);
	_mm_storeu_ps(m+8, row3);
	_mm_storeu_ps(m+12, row4);
}

void Matrix::SetTranslate(const float &x, const float &y, const float &z)
{
	m[0]=m[5]=m[10]=m[15]=1.0f;
	m[1]=m[2]=m[3]=m[4]=m[6]=m[7]=m[8]=m[9]=m[11]=0.0f;
	m[12]=x;
	m[13]=y;
	m[14]=z;
}


void Matrix::SetTranslate(const float * const vec)
{
	m[0]=m[5]=m[10]=m[15]=1.0f;
	m[1]=m[2]=m[3]=m[4]=m[6]=m[7]=m[8]=m[9]=m[11]=0.0f;
	m[12]=vec[0];
	m[13]=vec[1];
	m[14]=vec[2];
}

void Matrix::SetTranslate(const Vector &vec)
{
	m[0]=m[5]=m[10]=m[15]=1.0f;
	m[1]=m[2]=m[3]=m[4]=m[6]=m[7]=m[8]=m[9]=m[11]=0.0f;
	m[12]=vec.x;
	m[13]=vec.y;
	m[14]=vec.z;
}

void Matrix::SetScale(const float &x, const float &y, const float &z)
{
	memset(m,0,sizeof(float)*16);
	m[0]=x;
	m[5]=y;
	m[10]=z;
	m[15]=1.0f;
}

void Matrix::SetScale(const Vector &vec)
{
	memset(m,0,sizeof(float)*16);
	m[0]=vec.x;
	m[5]=vec.y;
	m[10]=vec.z;
	m[15]=1.0f;
}

void Matrix::SetReflect(const Plane &plane)
{
	Vector pNormal;
	float d;

	plane.GetNormal(pNormal);
	d=plane.GetD();
	pNormal.Normaliza();

	m[0]=-2.0f * pNormal.x * pNormal.x + 1.0f;
	m[1]=-2.0f * pNormal.y * pNormal.x;
	m[2]=-2.0f * pNormal.z * pNormal.x;
	m[3]=0.0f;

	m[4]=-2.0f * pNormal.x * pNormal.y;
	m[5]=-2.0f * pNormal.y * pNormal.y + 1.0f;
	m[6]=-2.0f * pNormal.z * pNormal.y;
	m[7]=0.0f;

	m[8]=-2.0f * pNormal.x * pNormal.z;
	m[9]=-2.0f * pNormal.y * pNormal.z;
	m[10]=-2.0f * pNormal.z * pNormal.z + 1.0f;
	m[11]=0.0f;

	m[12]=-2.0f * pNormal.x * d;
	m[13]=-2.0f * pNormal.y * d;
	m[14]=-2.0f * pNormal.z * d;
	m[15]=1.0f;
}

void Matrix::SetReflect(const float &a, const float &b, const float &c, const float &d)
{
	m[0]=-2.0f * a * a + 1.0f;
	m[1]=-2.0f * b * a;
	m[2]=-2.0f * c * a;
	m[3]=0.0f;

	m[4]=-2.0f * a * b;
	m[5]=-2.0f * b * b + 1.0f;
	m[6]=-2.0f * c * b;
	m[7]=0.0f;

	m[8]=-2.0f * a * c;
	m[9]=-2.0f * b * c;
	m[10]=-2.0f * c * c + 1.0f;
	m[11]=0.0f;

	m[12]=-2.0f * a * d;
	m[13]=-2.0f * b * d;
	m[14]=-2.0f * c * d;
	m[15]=1.0f;
}

void Matrix::SetRotateX(const float &angulo)
{
	float c,s;

	Math::sinCosG(angulo,s,c);

	m[9] =-s;
	m[10]= c;
	m[5] = c;
	m[6] = s;

	m[1]=m[2]=m[3]=m[4]=m[7]=m[8]=m[11]=m[12]=m[13]=m[14]=0.0f;
	m[0]=m[15]=1.0f;
}

void Matrix::SetRotateY(const float &angulo)
{
	float c,s;

	Math::sinCosG(angulo,s,c);

	m[0] = c;
	m[2] =-s;
	m[8] = s;
	m[10]= c;

	m[1]=m[3]=m[4]=m[6]=m[7]=m[9]=m[11]=m[12]=m[13]=m[14]=0.0f;
	m[5]=m[15]=1.0f;
}

void Matrix::SetRotateZ(const float &angulo)
{
	float c,s;

	Math::sinCosG(angulo,s,c);

	m[0]= c;
	m[1]= s;
	m[4]=-s;
	m[5]= c;

	m[2]=m[3]=m[6]=m[7]=m[8]=m[9]=m[11]=m[12]=m[13]=m[14]=0.0f;
	m[10]=m[15]=1.0f;
}

void Matrix::SetRotate(const float &angulo, const Vector &vec)
{
	Vector aux;
	float c,s,t,txx,tyy,tzz,txy,txz,tyz,sx,sy,sz;

	//normalizo vector
	aux=vec;
	aux.Normaliza();

	//calculos extra
	Math::sinCosG(angulo,s,c);

	t = 1.0f-c;
	txx = aux.x * aux.x * t;
	tyy = aux.y * aux.y * t;
	tzz = aux.z * aux.z * t;
	txy = aux.x * aux.y * t;
	txz = aux.x * aux.z * t;
	tyz = aux.y * aux.z * t;
	sx  = s * aux.x;
	sy  = s * aux.y;
	sz  = s * aux.z;

	//calculo os parametros necessários
	m[0] = txx + c;
	m[1] = txy + sz;
	m[2] = txz - sy;

	m[4] = txy - sz;
	m[5] = tyy + c;
	m[6] = tyz + sx;

	m[8] = txz + sy;
	m[9] = tyz - sx;
	m[10]= tzz + c;

	m[3]=m[7]=m[11]=m[12]=m[13]=m[14]=0.0f;
	m[15]=1.0f;
}

void Matrix::SetRotate(const float &anguloX, const float &anguloY, const float &anguloZ)
{
	float cx,cy,cz,sx,sy,sz;

	Math::sinCosG(anguloX,sx,cx);
	Math::sinCosG(anguloY,sy,cy);
	Math::sinCosG(anguloZ,sz,cz);

	m[0]=cy * cz;
	m[1]=-cy * sz;
	m[2]=sy;

	m[4]=cz * sx * sy + cx * sz;
	m[5]=cx * cz - sx * sy * sz;
	m[6]=-cy * sx;

	m[8]=-cx * cz * sy + sx * sz;
	m[9]=cz * sx + cx * sy + sz;
	m[10]=cx * cy;

	m[15]=1.0f;
	m[3]=m[7]=m[11]=m[12]=m[13]=m[14]=0.0f;
}

void Matrix::SetRotate(const float &angulo, const float &x, const float &y, const float &z)
{
	float X,Y,Z,tamanho;
	float c,s,t,txx,tyy,tzz,txy,txz,tyz,sx,sy,sz;

	//normalizo vector
	X=x;
	Y=y;
	Z=z;
	tamanho=Math::sqrtInv(X*X+Y*Y+Z*Z);
	X*=tamanho;
	Y*=tamanho;
	Z*=tamanho;

	//calculos extra
	Math::sinCosG(angulo,s,c);

	t = 1.0f-c;
	txx = X * X * t;
	tyy = Y * Y * t;
	tzz = Z * Z * t;
	txy = X * Y * t;
	txz = X * Z * t;
	tyz = Y * Z * t;
	sx  = s * X;
	sy  = s * Y;
	sz  = s * Z;

	//calculo os parametros necessários
	m[0] = txx + c;
	m[1] = txy + sz;
	m[2] = txz - sy;

	m[4] = txy - sz;
	m[5] = tyy + c;
	m[6] = tyz + sx;

	m[8] = txz + sy;
	m[9] = tyz - sx;
	m[10]= tzz + c;

	m[3]=m[7]=m[11]=m[12]=m[13]=m[14]=0.0f;
	m[15]=1.0f;
}

void Matrix::SetSaturation(const float sat)
{
	float minusS,posS;

	//tenho de fazer isto
	posS=Math::fClamp(sat,-1.0f,1.0f);
	minusS=1.0f-posS;

	//preencho a matriz
	m[0]=minusS*0.3086 + posS;
	m[1]=m[2]=minusS*0.3086;
	m[4]=m[6]=minusS*0.6094;
	m[5]=minusS*0.6094 + posS;
	m[8]=m[9]=minusS*0.0820;
	m[10]=minusS*0.0820 + posS;

	//o resto dos valores
	m[3]=m[7]=m[11]=m[12]=m[13]=m[14]=0.0f;
	m[15]=1.0f;
}

void Matrix::SetRotationFromTo(const float * const from, const float * const to)
{
	Vector v;
	float e, h, f;

	//comecar por ver orthogonalidade
	v.CalcCrossProduct(from,to);
	f=e=(from[0]*to[0]+from[1]*to[1]+from[2]*to[2]);
	if (e<0.0f)
		f=-e;
    
	//quase paralelos
	if (f>(1.0f-0.000000001))
		{
		float u[3],v[3],x[3],c1,c2,c3;
		int i, j;

		x[0] = (from[0] > 0.0)? from[0] : -from[0];
		x[1] = (from[1] > 0.0)? from[1] : -from[1];
		x[2] = (from[2] > 0.0)? from[2] : -from[2];

		if (x[0]<x[1])
			{
			if (x[0] < x[2])
				{
				x[0]=1.0f;
				x[1]=x[2]=0.0f;
				}
			else
				{
				x[2]=1.0f;
				x[0]=x[1]=0.0f;
				}
			}
		else
			{
			if (x[1] < x[2])
				{
				x[1]=1.0f;
				x[0]=x[2]=0.0f;
				}
			else
				{
				x[2]=1.0f;
				x[0]=x[1]=0.0f;
				}
			}

		u[0]=x[0]-from[0];
		u[1]=x[1]-from[1];
		u[2]=x[2]-from[2];
		v[0]=x[0]-to[0];
		v[1]=x[1]-to[1];
		v[2]=x[2]-to[2];


		c1 = 2.0f / (u[0]*u[0]+u[1]*u[1]+u[2]*u[2]);
		c2 = 2.0f / (v[0]*v[0]+v[1]*v[1]+v[2]*v[2]);
		c3 = c1 * c2  * (u[0]*v[0]+u[1]*v[1]+u[2]*v[2]);

		for(i=0; i<3; i++)
			{
			for(j=0; j<3; j++)
				m[j*4+i]= - c1 * u[i] * u[j] - c2 * v[i] * v[j] + c3 * v[i] * u[j];
			m[i*4+i]+=1.0f;
			}
		m[3]=m[7]=m[11]=m[12]=m[13]=m[14]=0.0f;
		m[15]=1.0f;
		return;
		}

	//o mais usual que é ambos nao serem paralelos
	float hvx, hvz, hvxy, hvxz, hvyz;

	h = 1.0f/(1.0f + e);
	hvx = h * v[0];
	hvz = h * v[2];
	hvxy = hvx * v[1];
	hvxz = hvx * v[2];
	hvyz = hvz * v[1];

	m[0] = e + hvx * v[0];
	m[4] = hvxy - v[2];
	m[8] = hvxz + v[1];

	m[1] = hvxy + v[2];
	m[5] = e + h * v[1] * v[1];
	m[9] = hvyz - v[0];

	m[2] = hvxz - v[1];
	m[6] = hvyz + v[0];
	m[10]= e + hvz * v[2];

	m[3]=m[7]=m[11]=m[12]=m[13]=m[14]=0.0f;
	m[15]=1.0f;
}

void Matrix::SetRotationFromTo(const Vector &from, const Vector &to)
{
	Vector v;
	float e, h, f;

	//comecar por ver orthogonalidade
	v.CalcCrossProduct(from,to);
	f=e=from.Dot(to);
	if (e<0.0f)
		f=-e;
    
	//quase paralelos
	if (f>(1.0f-0.000000001))
		{
		float u[3],v[3],x[3],c1,c2,c3;
		int i, j;

		x[0] = (from.x > 0.0)? from.x : -from.x;
		x[1] = (from.y > 0.0)? from.y : -from.y;
		x[2] = (from.z > 0.0)? from.z : -from.z;

		if (x[0]<x[1])
			{
			if (x[0] < x[2])
				{
				x[0]=1.0f;
				x[1]=x[2]=0.0f;
				}
			else
				{
				x[2]=1.0f;
				x[0]=x[1]=0.0f;
				}
			}
		else
			{
			if (x[1] < x[2])
				{
				x[1]=1.0f;
				x[0]=x[2]=0.0f;
				}
			else
				{
				x[2]=1.0f;
				x[0]=x[1]=0.0f;
				}
			}

		u[0] = x[0] - from.x;
		u[1] = x[1] - from.y;
		u[2] = x[2] - from.z;
		v[0] = x[0] - to.x;
		v[1] = x[1] - to.y;
		v[2] = x[2] - to.z;


		c1 = 2.0f / (u[0]*u[0]+u[1]*u[1]+u[2]*u[2]);
		c2 = 2.0f / (v[0]*v[0]+v[1]*v[1]+v[2]*v[2]);
		c3 = c1 * c2  * (u[0]*v[0]+u[1]*v[1]+u[2]*v[2]);

		for(i=0; i<3; i++)
			{
			for(j=0; j<3; j++)
				m[j*4+i]= - c1 * u[i] * u[j] - c2 * v[i] * v[j] + c3 * v[i] * u[j];
			m[i*4+i]+=1.0f;
			}
		m[3]=m[7]=m[11]=m[12]=m[13]=m[14]=0.0f;
		m[15]=1.0f;
		return;
		}

	//o mais usual que é ambos nao serem paralelos
	float hvx, hvz, hvxy, hvxz, hvyz;

	h = 1.0f/(1.0f + e);
	hvx = h * v[0];
	hvz = h * v[2];
	hvxy = hvx * v[1];
	hvxz = hvx * v[2];
	hvyz = hvz * v[1];

	m[0] = e + hvx * v[0];
	m[4] = hvxy - v[2];
	m[8] = hvxz + v[1];

	m[1] = hvxy + v[2];
	m[5] = e + h * v[1] * v[1];
	m[9] = hvyz - v[0];

	m[2] = hvxz - v[1];
	m[6] = hvyz + v[0];
	m[10]= e + hvz * v[2];

	m[3]=m[7]=m[11]=m[12]=m[13]=m[14]=0.0f;
	m[15]=1.0f;
}

void Matrix::SetGLModelView(const Vector &pos, const Vector &target, const Vector &up)
{
	Vector x,y,z;
	
	z.Set(pos);
	z-=target;
	z.Normaliza();

	x.CalcCrossProduct(up,z);
	y.CalcCrossProduct(z,x);

	x.Normaliza();
	y.Normaliza();

	m[0]=x.x;
	m[1]=y.x;
	m[2]=z.x;
	m[4]=x.y;
	m[5]=y.y;
	m[6]=z.y;
	m[8]=x.z;
	m[9]=y.z;
	m[10]=z.z;

	x*=-1.0f;
	y*=-1.0f;
	z*=-1.0f;
	
	m[12]=x.x*pos.x + x.y*pos.y + x.z*pos.z;
	m[13]=y.x*pos.x + y.y*pos.y + y.z*pos.z;
	m[14]=z.x*pos.x + z.y*pos.y + z.z*pos.z;

	m[3]=m[7]=m[11]=0.0f;
	m[15]=1.0f;
}

void Matrix::SetGLModelView(const Vector &pos, const Vector &target)
{	
	Vector up(0.0f,1.0f,0.0f);
	SetGLModelView(pos,target,up);
}

void Matrix::SetGLModelView(const float *pos, const float *target, const float *up)
{
	Vector x,y,z,aux;
	
	z.Set(pos);
	z-=target;
	z.Normaliza();

	x.CalcCrossProduct(up,(const float*)z);
	y.CalcCrossProduct(z,x);

	x.Normaliza();
	y.Normaliza();

	m[0]=x.x;
	m[1]=y.x;
	m[2]=z.x;
	m[4]=x.y;
	m[5]=y.y;
	m[6]=z.y;
	m[8]=x.z;
	m[9]=y.z;
	m[10]=z.z;

	x*=-1.0f;
	y*=-1.0f;
	z*=-1.0f;
	
	m[12]=x.x*pos[0] + x.y*pos[1] + x.z*pos[2];
	m[13]=y.x*pos[0] + y.y*pos[1] + y.z*pos[2];
	m[14]=z.x*pos[0] + z.y*pos[1] + z.z*pos[2];

	m[3]=m[7]=m[11]=0.0f;
	m[15]=1.0f;
}

void Matrix::SetGLModelView(const float *pos, const float *target)
{
	float up[]={0.0f,1.0f,0.0f};
	SetGLModelView(pos,target,up);
}

void Matrix::SetGLModelView(const Vector &pos, const float angX, const float angY, const Vector &up)
{
	Vector target,x,y,z;
	float sx,sy,cx,cy;

	Math::sinCosG(angX,sx,cx);
	Math::sinCosG(angY,sy,cy);
	
	target.x=pos.x+sx*cy;
	target.y=pos.y+sy;
	target.z=pos.z-(cx*cy);
	
	z.Set(pos);
	z-=target;
	z.Normaliza();

	x.CalcCrossProduct(up,z);
	y.CalcCrossProduct(z,x);

	x.Normaliza();
	y.Normaliza();

	m[0]=x.x;
	m[1]=y.x;
	m[2]=z.x;
	m[4]=x.y;
	m[5]=y.y;
	m[6]=z.y;
	m[8]=x.z;
	m[9]=y.z;
	m[10]=z.z;

	x*=-1.0f;
	y*=-1.0f;
	z*=-1.0f;
	
	m[12]=x.x*pos.x + x.y*pos.y + x.z*pos.z;
	m[13]=y.x*pos.x + y.y*pos.y + y.z*pos.z;
	m[14]=z.x*pos.x + z.y*pos.y + z.z*pos.z;

	m[3]=m[7]=m[11]=0.0f;
	m[15]=1.0f;
}

void Matrix::SetGLModelView(const float *pos, const float angX, const float angY, const float *up)
{
	Vector target,x,y,z;
	float sx,sy,cx,cy;

	Math::sinCosG(angX,sx,cx);
	Math::sinCosG(angY,sy,cy);

	target.x=pos[0]+sx*cy;
	target.y=pos[1]+sy;
	target.z=pos[2]-(cx*cy);
	
	z.Set(pos);
	z-=target;
	z.Normaliza();

	x.CalcCrossProduct(up,(const float*)z);
	y.CalcCrossProduct(z,x);

	x.Normaliza();
	y.Normaliza();

	m[0]=x.x;
	m[1]=y.x;
	m[2]=z.x;
	m[4]=x.y;
	m[5]=y.y;
	m[6]=z.y;
	m[8]=x.z;
	m[9]=y.z;
	m[10]=z.z;

	x*=-1.0f;
	y*=-1.0f;
	z*=-1.0f;
	
	m[12]=x.x*pos[0] + x.y*pos[1] + x.z*pos[2];
	m[13]=y.x*pos[0] + y.y*pos[1] + y.z*pos[2];
	m[14]=z.x*pos[0] + z.y*pos[1] + z.z*pos[2];

	m[3]=m[7]=m[11]=0.0f;
	m[15]=1.0f;
}

void Matrix::SetGLModelView(const int cubemapFace, const Vector &centerCube)
{
	//posso limpar isto
	memset(m,0,sizeof(float)*16);

	//positive X
	if (cubemapFace==0)
	{
		m[2]=m[5]=m[8]=-1.0f;
		m[12]=centerCube.z;
		m[13]=centerCube.y;
		m[14]=centerCube.x;
		m[15]=1.0f;
		return;
	}

	//negative X
	if (cubemapFace==1)
	{
		m[2]=m[8]=1.0f;
		m[5]=-1.0f;
		m[12]=-centerCube.z;
		m[13]=centerCube.y;
		m[14]=-centerCube.x;
		m[15]=1.0f;
		return;
	}

	//positive Y
	if (cubemapFace==2)
	{
		m[0]=m[9]=1.0f;
		m[6]=-1.0f;
		m[12]=-centerCube.x;
		m[13]=-centerCube.z;
		m[14]=centerCube.y;
		m[15]=1.0f;
		return;
	}

	//negative Y
	if (cubemapFace==3)
	{
		m[0]=m[6]=1.0f;
		m[9]=-1.0f;
		m[12]=-centerCube.x;
		m[13]=centerCube.z;
		m[14]=-centerCube.y;
		m[15]=1.0f;
		return;
	}

	//positive Z
	if (cubemapFace==4)
	{
		m[5]=m[10]=-1.0f;
		m[0]=1.0f;
		m[12]=-centerCube.x;
		m[13]=centerCube.y;
		m[14]=centerCube.z;
		m[15]=1.0f;
		return;
	}

	//negative Z
	if (cubemapFace==5)
	{
		m[0]=m[5]=-1.0f;
		m[10]=1.0f;
		m[12]=centerCube.x;
		m[13]=centerCube.y;
		m[14]=-centerCube.z;
		m[15]=1.0f;
		return;
	}

	//identidade e prontos
	m[0]=m[5]=m[10]=m[15]=1.0f;
}

void Matrix::SetGLProjection3D(const float fovy, const float aspect, const float zNear, const float zFar)
{
	double ymin,ymax,xmin,xmax,n,f;

	n = (double)zNear;
	f = (double)zFar;
	ymax = n * tan( ((double)fovy) * 0.00872664625997164788461845384 );
	ymin = -ymax;
	xmin = ymin * ((double)aspect);
	xmax = ymax * ((double)aspect);

	memset(m,0,sizeof(float)*16);
	m[0]=(float)((2.0*n)/(xmax-xmin));
	m[5]=(float)((2.0*n)/(ymax-ymin));
	m[8]=(float)((xmax+xmin)/(xmax-xmin));
	m[9]=(float)((ymax+ymin)/(ymax-ymin));
	m[10]=-(float)((f+n)/(f-n));
	m[11]=-1.0f;
	m[14]=-(float)((2.0*f*n)/(f-n));
}

void Matrix::SetGLProjection3D(const float fovy, const float aspect, const float zNear)
{
	double ymin,ymax,xmin,xmax,n;

	n = (double)zNear;
	ymax = n * tan( ((double)fovy) * 0.00872664625997164788461845384 );
	ymin = -ymax;
	xmin = ymin * ((double)aspect);
	xmax = ymax * ((double)aspect);

	memset(m,0,sizeof(float)*16);
	m[0]=(float)((2.0*n)/(xmax-xmin));
	m[5]=(float)((2.0*n)/(ymax-ymin));
	m[8]=(float)((xmax+xmin)/(xmax-xmin));
	m[9]=(float)((ymax+ymin)/(ymax-ymin));
	m[10]=m[11]=-1.0f;
	m[14]=-(float)(2.0*n);
}

void Matrix::SetGLProjection2D(const float width, const float height)
{
	memset(m,0,sizeof(float)*16);
	m[0]=2.0f/width;
	m[5]=2.0f/height;
	m[10]=m[12]=m[13]=-1.0f;
	m[15]=1.0f;
}

void Matrix::SetGLProjectionOrtho(const float left, const float right, const float bottom, const float top, const float zNear, const float zFar)
{
	memset(m,0,sizeof(float)*16);
	m[0]=2.0f/(right-left);
	m[5]=2.0f/(top-bottom);
	m[10]=-2.0f/(zFar-zNear);
	m[15]=1.0f;

	m[12]=-(right+left)/(right-left);
	m[13]=-(top+bottom)/(top-bottom);
	m[14]=-(zFar+zNear)/(zFar-zNear);
}

void Matrix::SetGLProjectionOrtho(const BBox &bbox)
{
	Vector bmin,bmax;
	float xDist,yDist,zDist;

	bbox.GetMinMax(bmin,bmax);
	xDist = (bmax.x - bmin.x)*0.5f;
	yDist = (bmax.y - bmin.y)*0.5f;
	zDist = bmax.z - bmin.z;

	memset(m,0,sizeof(float)*16);
	m[0]=2.0f/(xDist+xDist);
	m[5]=2.0f/(yDist+yDist);
	m[10]=-2.0f/(zDist);
	m[15]=1.0f;

	m[12]=-(xDist-xDist)/(xDist+xDist);
	m[13]=-(yDist-yDist)/(yDist+yDist);
	m[14]=-1.0f;
}

/*§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§
§§§§§§   -= Classe Matrix3 =-	§§§§§§
§§§§§§			  3x3			§§§§§§
§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§*/
Matrix3::Matrix3(const float *s)
{	memcpy(m,s,sizeof(float)*9);}

Matrix3::Matrix3(const Matrix &mat)
{	
	m[0]=mat.m[0];		m[1]=mat.m[1];		m[2]=mat.m[2];
	m[3]=mat.m[4];		m[4]=mat.m[5];		m[5]=mat.m[6];
	m[6]=mat.m[8];		m[7]=mat.m[9];		m[8]=mat.m[10];
}

Matrix3::Matrix3(const Matrix3 &mat)
{	memcpy(m,mat.m,sizeof(float)*9);}

void Matrix3::operator*=(const Matrix &s)
{	
	float matAux[9];

	memcpy(matAux,m,sizeof(float)*9);

	m[0] = s.m[0] * matAux[0] + s.m[1] * matAux[3] + s.m[2] * matAux[6];
	m[1] = s.m[0] * matAux[1] + s.m[1] * matAux[4] + s.m[2] * matAux[7];
	m[2] = s.m[0] * matAux[2] + s.m[1] * matAux[5] + s.m[2] * matAux[8];

	m[3] = s.m[4] * matAux[0] + s.m[5] * matAux[3] + s.m[6] * matAux[6];
	m[4] = s.m[4] * matAux[1] + s.m[5] * matAux[4] + s.m[6] * matAux[7];
	m[5] = s.m[4] * matAux[2] + s.m[5] * matAux[5] + s.m[6] * matAux[8];

	m[6] = s.m[8] * matAux[0] + s.m[9] * matAux[3] + s.m[10]* matAux[6];
	m[7] = s.m[8] * matAux[1] + s.m[9] * matAux[4] + s.m[10]* matAux[7];
	m[8] = s.m[8] * matAux[2] + s.m[9] * matAux[5] + s.m[10]* matAux[8];
}

void Matrix3::operator*=(const Matrix3 &s)
{	
	float matAux[9];

	memcpy(matAux,m,sizeof(float)*9);

	m[0] = s.m[0] * matAux[0] + s.m[1] * matAux[3] + s.m[2] * matAux[6];
	m[1] = s.m[0] * matAux[1] + s.m[1] * matAux[4] + s.m[2] * matAux[7];
	m[2] = s.m[0] * matAux[2] + s.m[1] * matAux[5] + s.m[2] * matAux[8];

	m[3] = s.m[3] * matAux[0] + s.m[4] * matAux[3] + s.m[5] * matAux[6];
	m[4] = s.m[3] * matAux[1] + s.m[4] * matAux[4] + s.m[5] * matAux[7];
	m[5] = s.m[3] * matAux[2] + s.m[4] * matAux[5] + s.m[5] * matAux[8];

	m[6] = s.m[6] * matAux[0] + s.m[7] * matAux[3] + s.m[8] * matAux[6];
	m[7] = s.m[6] * matAux[1] + s.m[7] * matAux[4] + s.m[8] * matAux[7];
	m[8] = s.m[6] * matAux[2] + s.m[7] * matAux[5] + s.m[8] * matAux[8];
}


void Matrix3::operator*=(const float *s)
{	
	float matAux[9];

	memcpy(matAux,m,sizeof(float)*9);

	m[0] = s[0] * matAux[0] + s[1] * matAux[3] + s[2] * matAux[6];
	m[1] = s[0] * matAux[1] + s[1] * matAux[4] + s[2] * matAux[7];
	m[2] = s[0] * matAux[2] + s[1] * matAux[5] + s[2] * matAux[8];

	m[3] = s[3] * matAux[0] + s[4] * matAux[3] + s[5] * matAux[6];
	m[4] = s[3] * matAux[1] + s[4] * matAux[4] + s[5] * matAux[7];
	m[5] = s[3] * matAux[2] + s[4] * matAux[5] + s[5] * matAux[8];

	m[6] = s[6] * matAux[0] + s[7] * matAux[3] + s[8] * matAux[6];
	m[7] = s[6] * matAux[1] + s[7] * matAux[4] + s[8] * matAux[7];
	m[8] = s[6] * matAux[2] + s[7] * matAux[5] + s[8] * matAux[8];
}

void Matrix3::operator+=(const Matrix3 &s)
{
	m[0]+=s.m[0];
	m[1]+=s.m[1];
	m[2]+=s.m[2];

	m[3]+=s.m[3];
	m[4]+=s.m[4];
	m[5]+=s.m[5];

	m[6]+=s.m[6];
	m[7]+=s.m[7];
	m[8]+=s.m[8];
}

void Matrix3::operator+=(const float *s)
{
	m[0]+=s[0];
	m[1]+=s[1];
	m[2]+=s[2];

	m[3]+=s[3];
	m[4]+=s[4];
	m[5]+=s[5];

	m[6]+=s[6];
	m[7]+=s[7];
	m[8]+=s[8];
}

void Matrix3::operator-=(const Matrix3 &s)
{
	m[0]-=s.m[0];
	m[1]-=s.m[1];
	m[2]-=s.m[2];

	m[3]-=s.m[3];
	m[4]-=s.m[4];
	m[5]-=s.m[5];

	m[6]-=s.m[6];
	m[7]-=s.m[7];
	m[8]-=s.m[8];
}

void Matrix3::operator-=(const float *s)
{
	m[0]-=s[0];
	m[1]-=s[1];
	m[2]-=s[2];

	m[3]-=s[3];
	m[4]-=s[4];
	m[5]-=s[5];

	m[6]-=s[6];
	m[7]-=s[7];
	m[8]-=s[8];
}

void Matrix3::MulVector(float *vec) const
{
	float vecX,vecY,vecZ;

	vecX=vec[0];
	vecY=vec[1];
	vecZ=vec[2];

	vec[0] = vecX*m[0] + vecY*m[3] + vecZ*m[6];
	vec[1] = vecX*m[1] + vecY*m[4] + vecZ*m[7];
	vec[2] = vecX*m[2] + vecY*m[5] + vecZ*m[8];
}

void Matrix3::MulVector(Vector& vec) const
{
	float result[3];

	result[0] = vec.x*m[0] + vec.y*m[3] + vec.z*m[6];
	result[1] = vec.x*m[1] + vec.y*m[4] + vec.z*m[7];
	result[2] = vec.x*m[2] + vec.y*m[5] + vec.z*m[8];

	vec.x=result[0];
	vec.y=result[1];
	vec.z=result[2];
}

void Matrix3::MulVector(const Vector& vec, Vector &result) const
{
	result.x = vec.x*m[0] + vec.y*m[3] + vec.z*m[6];
	result.y = vec.x*m[1] + vec.y*m[4] + vec.z*m[7];
	result.z = vec.x*m[2] + vec.y*m[5] + vec.z*m[8];
}

void Matrix3::MulVector(Vector * const vec, const int numVec) const
{	
	float result[3];

	for(int i=0; i<numVec; i++)
		{
		result[0] = vec[i].x*m[0] + vec[i].y*m[3] + vec[i].z*m[6];
		result[1] = vec[i].x*m[1] + vec[i].y*m[4] + vec[i].z*m[7];
		result[2] = vec[i].x*m[2] + vec[i].y*m[5] + vec[i].z*m[8];

		vec[i].x=result[0];
		vec[i].y=result[1];
		vec[i].z=result[2];
		}
}

void Matrix3::GetCol1(Vector &result) const
{
	result.x=m[0];
	result.y=m[3];
	result.z=m[6];
}

void Matrix3::GetCol2(Vector &result) const
{
	result.x=m[1];
	result.y=m[4];
	result.z=m[7];
}

void Matrix3::GetCol3(Vector &result) const
{
	result.x=m[2];
	result.y=m[5];
	result.z=m[8];
}

void Matrix3::GetRow1(Vector &result) const
{
	result.x=m[0];
	result.y=m[1];
	result.z=m[2];
}

void Matrix3::GetRow2(Vector &result) const
{
	result.x=m[3];
	result.y=m[4];
	result.z=m[5];
}

void Matrix3::GetRow3(Vector &result) const
{
	result.x=m[6];
	result.y=m[7];
	result.z=m[8];
}

void Matrix3::GetRotate(Vector &vec, float &angulo) const
{
	float s,tr,quat[4];
	int op;

	//calculo o quaternion a partir da matriz
	tr = m[0] + m[4] + m[8];

	if (tr>0.0f) 
		{
		s=sqrt(tr+1.0f);

		quat[3]=s*0.5f;
	    
		s=0.5f/s;

		quat[0]=(m[5]-m[7])*s;
		quat[1]=(m[6]-m[2])*s;
		quat[2]=(m[1]-m[3])*s;
		}
	else
		{
		op=0;
		if (m[4]>m[0])
			op=1;
		if (m[8]>m[op*4])
			op=2;

		switch(op){
			case 0:
					s=sqrt((m[0]-(m[4]+m[8]))+1.0f);

					quat[0]=s*0.5f;
					if (s!=0.0f)
						s=0.5f/s;

					quat[1]=(m[1]+m[3])*s;
					quat[2]=(m[2]+m[6])*s;
					quat[3]=(m[5]-m[7])*s;
			
					break;

			case 1:
					s=sqrt((m[4]-(m[8]+m[0]))+1.0f);

					quat[1]=s*0.5f;
					if (s!=0.0f)
						s=0.5f/s;

					quat[3]=(m[6]-m[2])*s;
					quat[2]=(m[5]+m[7])*s;
					quat[0]=(m[3]+m[1])*s;

					break;

			case 2:
					s=sqrt((m[8]-(m[0]+m[4]))+1.0f);

					quat[2]=s*0.5f;
					if (s!=0.0f)
						s=0.5f/s;

					quat[3]=(m[1]-m[3])*s;
					quat[0]=(m[6]+m[2])*s;
					quat[1]=(m[7]+m[5])*s;
					break;
			}
		}

	//a partir do quaterniao calculo o angulo e vector
	tr=quat[0]*quat[0]+quat[1]*quat[1]+quat[2]*quat[2];
	if (tr==0.0f) 
		{
		vec.x=0.0f;
		vec.y=0.0f;
		vec.z=1.0f;
		angulo=0.0f;
		return;
		}
    
	tr=1.0f/tr;
	vec.x=quat[0]*tr;
	vec.y=quat[1]*tr;
	vec.z=quat[2]*tr;
	vec.Normaliza();
	angulo=((float)acos(quat[3]))*114.5915590261646417f; // 180/pi=57.295779513082320876f * 2.0f (o 2 é de formula)
}

void Matrix3::GetEulerAngles(float &rfYAngle, float &rfPAngle, float &rfRAngle) const
{
	if ( -1.0f < -m[6] )
		{
		if ( -m[6] < 1.0 )
			rfPAngle = asin(-m[6]);
		else
			rfPAngle = -(2.0f*atanf(1.0f));
		}
	else
		{
		rfPAngle = 2.0f*atanf(1.0f);
		}

	if ( rfPAngle < (2.0f*atanf(1.0f)) )
		{
		if ( rfPAngle > -(2.0f*atanf(1.0f)) )
			{
			rfYAngle = atan2(m[3],m[0]);
			rfRAngle = atan2(m[7],m[8]);
			}
		else
			{
			float fRmY = atan2(-m[1],m[2]);
			rfRAngle = 0.0f;
			rfYAngle = rfRAngle - fRmY;
			}
		}

	else
		{
		float fRpY = atan2(-m[0],m[2]);
		rfRAngle = 0.0f;
		rfYAngle = fRpY - rfRAngle;
		}
}

void Matrix3::Write(float * const s) const
{	memcpy(s,m,sizeof(float)*9);}

void Matrix3::Transpose(Matrix3 &dest) const
{
	dest.m[1] = m[3];
	dest.m[2] = m[6];
	dest.m[3] = m[1];
	dest.m[5] = m[7];
	dest.m[6] = m[2];
	dest.m[7] = m[5];

	dest.m[0] = m[0];
	dest.m[4] = m[4];
	dest.m[8] = m[8];
}

void Matrix3::Transpose()
{
	float tmp;

	tmp=m[1];
	m[1]=m[3];
	m[3]=tmp;

	tmp=m[2];
	m[2]=m[6];
	m[6]=tmp;

	tmp=m[5];
	m[5]=m[7];
	m[7]=tmp;
}

void Matrix3::MulRotateX(const float &angulo)
{
	float c,s,p1,p2,p3;

	Math::sinCosG(angulo,s,c);

	p1 = m[3];
	p2 = m[4];
	p3 = m[5];

	m[3] = (c * p1) + (s * m[6]);
	m[4] = (c * p2) + (s * m[7]);
	m[5] = (c * p3) + (s * m[8]);

	m[6] = (-s * p1) + (c * m[6]);
	m[7] = (-s * p2) + (c * m[7]);
	m[8] = (-s * p3) + (c * m[8]);
}

void Matrix3::MulRotateY(const float &angulo)
{
	float c,s,p1,p2,p3;

	Math::sinCosG(angulo,s,c);

	p1 = m[0];
	p2 = m[1];
	p3 = m[2];

	m[0] = (c * p1) + (-s * m[6]);
	m[1] = (c * p2) + (-s * m[7]);
	m[2] = (c * p3) + (-s * m[8]);

	m[6] = (s * p1) + (c * m[6]);
	m[7] = (s * p2) + (c * m[7]);
	m[8] = (s * p3) + (c * m[8]);
}

void Matrix3::MulRotateZ(const float &angulo)
{
	float c,s,p1,p2,p3;

	Math::sinCosG(angulo,s,c);

	p1 = m[0];
	p2 = m[1];
	p3 = m[2];

	m[0] = (c * p1) + (s * m[3]);
	m[1] = (c * p2) + (s * m[4]);
	m[2] = (c * p3) + (s * m[5]);

	m[3] = (-s * p1) + (c * m[3]);
	m[4] = (-s * p2) + (c * m[4]);
	m[5] = (-s * p3) + (c * m[5]);
}

void Matrix3::Set(const float x)
{
	m[0]=m[1]=m[2]=x;
	m[3]=m[4]=m[5]=x;
	m[6]=m[7]=m[8]=x;
}

void Matrix3::Set(const float *src)
{	memcpy(m,src,sizeof(float)*9);}

void Matrix3::Set(const Matrix &mat)
{
	m[0]=mat.m[0];		m[1]=mat.m[1];		m[2]=mat.m[2];
	m[3]=mat.m[4];		m[4]=mat.m[5];		m[5]=mat.m[6];
	m[6]=mat.m[8];		m[7]=mat.m[9];		m[8]=mat.m[10];
}

void Matrix3::Set(const Matrix3 &mat)
{	memcpy(m,mat.m,sizeof(float)*9);}

void Matrix3::SetZero(void)
{	memset(m,0,sizeof(float)*9);}

void Matrix3::SetIdentidade(void)
{
	memset(m,0,sizeof(float)*9);
	m[0]=m[4]=m[8]=1.0f;
}

void Matrix3::SetRotateX(const float &angulo)
{
	float c,s;

	Math::sinCosG(angulo,s,c);

	m[7] =-s;
	m[8] = c;
	m[4] = c;
	m[5] = s;

	m[1]=m[2]=m[3]=m[6]=0.0f;
	m[0]=1.0f;
}

void Matrix3::SetRotateY(const float &angulo)
{
	float c,s;

	Math::sinCosG(angulo,s,c);

	m[0] = c;
	m[2] =-s;
	m[6] = s;
	m[8]= c;

	m[1]=m[3]=m[5]=m[7]=0.0f;
	m[4]=1.0f;
}

void Matrix3::SetRotateZ(const float &angulo)
{
	float c,s;

	Math::sinCosG(angulo,s,c);

	m[0]= c;
	m[1]= s;
	m[3]=-s;
	m[4]= c;

	m[2]=m[5]=m[6]=m[7]=0.0f;
	m[8]=1.0f;
}

void Matrix3::SetRotate(const float &angulo, const Vector &vec)
{
	Vector aux;
	float c,s,t,txx,tyy,tzz,txy,txz,tyz,sx,sy,sz;

	//normalizo vector
	aux=vec;
	aux.Normaliza();

	//calculos extra
	Math::sinCosG(angulo,s,c);

	t = 1.0f-c;
	txx = aux.x * aux.x * t;
	tyy = aux.y * aux.y * t;
	tzz = aux.z * aux.z * t;
	txy = aux.x * aux.y * t;
	txz = aux.x * aux.z * t;
	tyz = aux.y * aux.z * t;
	sx  = s * aux.x;
	sy  = s * aux.y;
	sz  = s * aux.z;

	//calculo os parametros necessários
	m[0] = txx + c;
	m[1] = txy + sz;
	m[2] = txz - sy;

	m[3] = txy - sz;
	m[4] = tyy + c;
	m[5] = tyz + sx;

	m[6] = txz + sy;
	m[7] = tyz - sx;
	m[8]= tzz + c;
}

void Matrix3::SetRotate(const float &anguloX, const float &anguloY, const float &anguloZ)
{
	float cx,cy,cz,sx,sy,sz;

	Math::sinCosG(anguloX,sx,cx);
	Math::sinCosG(anguloY,sy,cy);
	Math::sinCosG(anguloZ,sz,cz);

	m[0]=cy * cz;
	m[1]=-cy * sz;
	m[2]=sy;

	m[3]=cz * sx * sy + cx * sz;
	m[4]=cx * cz - sx * sy * sz;
	m[5]=-cy * sx;

	m[6]=-cx * cz * sy + sx * sz;
	m[7]=cz * sx + cx * sy + sz;
	m[8]=cx * cy;
}

void Matrix3::SetRotate(const float &angulo, const float &x, const float &y, const float &z)
{
	float X,Y,Z,tamanho;
	float c,s,t,txx,tyy,tzz,txy,txz,tyz,sx,sy,sz;

	//normalizo vector
	X=x;
	Y=y;
	Z=z;
	tamanho=Math::sqrtInv(X*X+Y*Y+Z*Z);
	X*=tamanho;
	Y*=tamanho;
	Z*=tamanho;

	//calculos extra
	Math::sinCosG(angulo,s,c);

	t = 1.0f-c;
	txx = X * X * t;
	tyy = Y * Y * t;
	tzz = Z * Z * t;
	txy = X * Y * t;
	txz = X * Z * t;
	tyz = Y * Z * t;
	sx  = s * X;
	sy  = s * Y;
	sz  = s * Z;

	//calculo os parametros necessários
	m[0] = txx + c;
	m[1] = txy + sz;
	m[2] = txz - sy;

	m[3] = txy - sz;
	m[4] = tyy + c;
	m[5] = tyz + sx;

	m[6] = txz + sy;
	m[7] = tyz - sx;
	m[8]= tzz + c;
}

void Matrix3::SetRotationFromTo(const float * const from, const float * const to)
{
	Vector v;
	float e, h, f;

	//comecar por ver orthogonalidade
	v.CalcCrossProduct(from,to);
	f=e=(from[0]*to[0]+from[1]*to[1]+from[2]*to[2]);
	if (e<0.0f)
		f=-e;
    
	//quase paralelos
	if (f>(1.0f-0.000000001))
		{
		float u[3],v[3],x[3],c1,c2,c3;
		int i, j;

		x[0] = (from[0] > 0.0)? from[0] : -from[0];
		x[1] = (from[1] > 0.0)? from[1] : -from[1];
		x[2] = (from[2] > 0.0)? from[2] : -from[2];

		if (x[0]<x[1])
			{
			if (x[0] < x[2])
				{
				x[0]=1.0f;
				x[1]=x[2]=0.0f;
				}
			else
				{
				x[2]=1.0f;
				x[0]=x[1]=0.0f;
				}
			}
		else
			{
			if (x[1] < x[2])
				{
				x[1]=1.0f;
				x[0]=x[2]=0.0f;
				}
			else
				{
				x[2]=1.0f;
				x[0]=x[1]=0.0f;
				}
			}

		u[0]=x[0]-from[0];
		u[1]=x[1]-from[1];
		u[2]=x[2]-from[2];
		v[0]=x[0]-to[0];
		v[1]=x[1]-to[1];
		v[2]=x[2]-to[2];


		c1 = 2.0f / (u[0]*u[0]+u[1]*u[1]+u[2]*u[2]);
		c2 = 2.0f / (v[0]*v[0]+v[1]*v[1]+v[2]*v[2]);
		c3 = c1 * c2  * (u[0]*v[0]+u[1]*v[1]+u[2]*v[2]);

		for(i=0; i<3; i++)
			{
			for(j=0; j<3; j++)
				m[j*3+i]= - c1 * u[i] * u[j] - c2 * v[i] * v[j] + c3 * v[i] * u[j];
			m[i*3+i]+=1.0f;
			}
		return;
		}

	//o mais usual que é ambos nao serem paralelos
	float hvx, hvz, hvxy, hvxz, hvyz;

	h = 1.0f/(1.0f + e);
	hvx = h * v[0];
	hvz = h * v[2];
	hvxy = hvx * v[1];
	hvxz = hvx * v[2];
	hvyz = hvz * v[1];

	m[0] = e + hvx * v[0];
	m[3] = hvxy - v[2];
	m[6] = hvxz + v[1];

	m[1] = hvxy + v[2];
	m[4] = e + h * v[1] * v[1];
	m[7] = hvyz - v[0];

	m[2] = hvxz - v[1];
	m[5] = hvyz + v[0];
	m[8]= e + hvz * v[2];
}

void Matrix3::SetRotationFromTo(const Vector &from, const Vector &to)
{
	Vector v;
	float e, h, f;

	//comecar por ver orthogonalidade
	v.CalcCrossProduct(from,to);
	f=e=from.Dot(to);
	if (e<0.0f)
		f=-e;
    
	//quase paralelos
	if (f>(1.0f-0.000000001))
		{
		float u[3],v[3],x[3],c1,c2,c3;
		int i, j;

		x[0] = (from.x > 0.0)? from.x : -from.x;
		x[1] = (from.y > 0.0)? from.y : -from.y;
		x[2] = (from.z > 0.0)? from.z : -from.z;

		if (x[0]<x[1])
			{
			if (x[0] < x[2])
				{
				x[0]=1.0f;
				x[1]=x[2]=0.0f;
				}
			else
				{
				x[2]=1.0f;
				x[0]=x[1]=0.0f;
				}
			}
		else
			{
			if (x[1] < x[2])
				{
				x[1]=1.0f;
				x[0]=x[2]=0.0f;
				}
			else
				{
				x[2]=1.0f;
				x[0]=x[1]=0.0f;
				}
			}

		u[0] = x[0] - from.x;
		u[1] = x[1] - from.y;
		u[2] = x[2] - from.z;
		v[0] = x[0] - to.x;
		v[1] = x[1] - to.y;
		v[2] = x[2] - to.z;


		c1 = 2.0f / (u[0]*u[0]+u[1]*u[1]+u[2]*u[2]);
		c2 = 2.0f / (v[0]*v[0]+v[1]*v[1]+v[2]*v[2]);
		c3 = c1 * c2  * (u[0]*v[0]+u[1]*v[1]+u[2]*v[2]);

		for(i=0; i<3; i++)
			{
			for(j=0; j<3; j++)
				m[j*3+i]= - c1 * u[i] * u[j] - c2 * v[i] * v[j] + c3 * v[i] * u[j];
			m[i*3+i]+=1.0f;
			}
		return;
		}

	//o mais usual que é ambos nao serem paralelos
	float hvx, hvz, hvxy, hvxz, hvyz;

	h = 1.0f/(1.0f + e);
	hvx = h * v[0];
	hvz = h * v[2];
	hvxy = hvx * v[1];
	hvxz = hvx * v[2];
	hvyz = hvz * v[1];

	m[0] = e + hvx * v[0];
	m[3] = hvxy - v[2];
	m[6] = hvxz + v[1];

	m[1] = hvxy + v[2];
	m[4] = e + h * v[1] * v[1];
	m[7] = hvyz - v[0];

	m[2] = hvxz - v[1];
	m[5] = hvyz + v[0];
	m[8]= e + hvz * v[2];
}

}//namespace HorseRadish