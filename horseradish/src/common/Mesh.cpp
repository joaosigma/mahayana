#include "Mesh.hpp"
#include "Math.hpp"
#include "Vector.hpp"

#include <windows.h>
#include <stdio.h>
#include <malloc.h>
#include <vector>
#include <set>
#include <assert.h>

namespace HorseRadish
{
namespace Geometry
{

static
bool rayTriIntersect(const float * const origin, const float * const dir, const float * const vert0, const float * const vert1, const float * const vert2, HorseRadish::Vector * const hitPoint)
{
	HorseRadish::Vector edge1,edge2,tvec,pvec,qvec;
	float det,u,v;

	//find vectors for two edges sharing vert0
	edge1.x=vert1[0]-vert0[0];
	edge1.y=vert1[1]-vert0[1];
	edge1.z=vert1[2]-vert0[2];
	edge2.x=vert2[0]-vert0[0];
	edge2.y=vert2[1]-vert0[1];
	edge2.z=vert2[2]-vert0[2];

	//begin calculating determinant - also used to calculate U parameter
	pvec.CalcCrossProduct(dir,(const float*)edge2);

	//se o determinante for zero, o ray está no mesmo plano do triangulo
	det=edge1.Dot(pvec);
	if (HorseRadish::Math::isZero(det))
		return false;

	//calculate distance from vert0 to ray origin
	tvec.x=origin[0]-vert0[0];
	tvec.y=origin[1]-vert0[1];
	tvec.z=origin[2]-vert0[2];
	qvec.CalcCrossProduct(tvec,edge1);

	//calcula-se o parametro U
	u=tvec.Dot(pvec);

	//para optimizar, verifica-se de acordo com o sinal do determinante
	if (det > 0.0f)
	{
		//test bounds
		if (u<0.0 || u>det)
			return false;

		//calculate V parameter and test bounds
		v = qvec.Dot(dir);
		if ((v<0.0) || ((u+v)>det))
			return false;
	}
	else
	{
		//calculate U test bounds
		if (u>0.0 || u<det)
			return false;

		//calculate V parameter and test bounds
		v=qvec.Dot(dir);
		if ((v>0.0) || ((u+v)<det))
			return false;
	}

	//calculo o hitPoint
	if (hitPoint)
		{
		det=1.0f/det;
		u*=det;
		v*=det;
		hitPoint->x=(1.0f-u-v)*vert0[0] + u*vert1[0] + v*vert2[0];
		hitPoint->y=(1.0f-u-v)*vert0[1] + u*vert1[1] + v*vert2[1];
		hitPoint->z=(1.0f-u-v)*vert0[2] + u*vert1[2] + v*vert2[2];
		}

	//e acertou
	return true;
}

static
void __fastcall somaVect(float * const dest, const HorseRadish::Vector &vec)
{
	dest[0]+=vec.x;
	dest[1]+=vec.y;
	dest[2]+=vec.z;
}

static
void __fastcall somaVect(float * const dest, const float * const vec)
{
	dest[0]+=vec[0];
	dest[1]+=vec[1];
	dest[2]+=vec[2];
}

static
void __fastcall somaVecCheckDir(float * const dest, const float * const vec)
{
	HorseRadish::Vector v1,v2;

	v1=dest;
	v1.Normaliza();
	v2=dest;
	v2.Normaliza();
	if (v1.Dot(v2)<0)
		{
		dest[0]-=vec[0];
		dest[1]-=vec[1];
		dest[2]-=vec[2];
		return;
		}
	dest[0]+=vec[0];
	dest[1]+=vec[1];
	dest[2]+=vec[2];
}

static
bool optimize_vertex_cache_order( std::vector<unsigned int> &tri_indices, unsigned int cache_size )
{
	struct tri_data {
		bool added;
		float score;
		unsigned int verts[3];
	};
	struct vert_data {
		float score;
		std::set<unsigned int> remaining_tris;
	};

	if( (tri_indices.size() < 3) || (tri_indices.size() % 3 != 0) || (cache_size < 4) )
		return false;

	unsigned int num_triangles = tri_indices.size() / 3;
	unsigned int num_vertices = 0;
	for( unsigned int i = 0; i < num_triangles*3; ++i )
		if( tri_indices[i] > num_vertices )
			num_vertices = tri_indices[i];
	++num_vertices;
	//	size of the optimization cache
	std::vector<float> cache_score(cache_size + 3, 0.75);
	std::vector<int> cache_idx(cache_size + 3, -1);
	std::vector<int> grow_cache_idx(cache_size + 3, -1);
	for( unsigned int i = 3; i < cache_size; ++i )
		cache_score[i] = powf( (cache_size - i) / (cache_size - 3.0), 1.5 );
	for( unsigned int i = 0; i < 3; ++i )
		cache_score[cache_size+i] = 0.0;
	//	how many tris do we need to add?
	int tris_left = num_triangles;
	//	add all verts and tris to the lists
	std::vector<tri_data> t(num_triangles);
	std::vector<vert_data> v(num_vertices);
	for( unsigned int i = 0; i < num_vertices; ++i )
	{
		//	initialize this vert
		v[i].score = 0.0;
		v[i].remaining_tris.clear();
	}
	for( unsigned int i = 0; i < num_triangles; ++i )
	{
		//	set up this tri
		t[i].added = false;
		t[i].score = 0.0;
		t[i].verts[0] = tri_indices[i*3+0];
		t[i].verts[1] = tri_indices[i*3+1];
		t[i].verts[2] = tri_indices[i*3+2];
		//	and add this tri index to each of it's verts
		v[tri_indices[i*3+0]].remaining_tris.insert( i );
		v[tri_indices[i*3+1]].remaining_tris.insert( i );
		v[tri_indices[i*3+2]].remaining_tris.insert( i );
	}
	//	now initialize all the scores for the vertices
	for( unsigned int i = 0; i < num_vertices; ++i )
	{
		//	none of them are in the index yet, just use thier valence score
		v[i].score = powf( v[i].remaining_tris.size(), -0.5 ) * 2.0;
	}
	//	and the triangles' scores
	float best_score = 0.0;
	int best_idx = -1;
	for( unsigned int i = 0; i < num_triangles; ++i )
	{
		t[i].score = v[t[i].verts[0]].score + v[t[i].verts[1]].score + v[t[i].verts[2]].score;
		if( t[i].score > best_score )
		{
			best_score = t[i].score;
			best_idx = i;
		}
	}
	//	now keep adding triangles
	while( tris_left > 0 )
	{
		//	scan all tris if the best score is suspicious
		//if( best_score < 1.0 )
		if( best_score < 0.01 )
		{
			//std::cerr << "low score looks suspicious...re-checking!" << std::endl;
			best_score = 0.0;
			best_idx = -1;
			for( unsigned int i = 0; i < num_triangles; ++i )
			if( !t[i].added )
			{
				if( t[i].score > best_score )
				{
					best_score = t[i].score;
					best_idx = i;
				}
			}
		}
		if( best_idx < 0 )
		{
			tris_left = 0;
			//	need a warning...for now, an assert
			assert( !"Your stupid triangle re-organizing code isn't working, Jonathan!!" );
		}
		else
		{
			//	add in this tri
			int a = t[best_idx].verts[0];
			int b = t[best_idx].verts[1];
			int c = t[best_idx].verts[2];
			//	put this tri back into circulation
			tri_indices[(num_triangles - tris_left)*3+0] = a;
			tri_indices[(num_triangles - tris_left)*3+1] = b;
			tri_indices[(num_triangles - tris_left)*3+2] = c;
			//	remove this tri from the association of each of the verts
			for( int i = 0; i < 3; ++i )
			{
				v[t[best_idx].verts[i]].remaining_tris.erase( best_idx );
			}
			t[best_idx].added = true;
			--tris_left;
			//std::cerr << best_idx << "," << best_score << std::endl;
			//	put these 3 verts at the top of the LRU list
			grow_cache_idx[0] = a;
			grow_cache_idx[1] = b;
			grow_cache_idx[2] = c;
			int idx = 3;
			for( unsigned int i = 0; i < cache_size; ++i )
			{
				//	clear out my growing cache
				grow_cache_idx[i+3] = -1;
				if( (cache_idx[i] != a) &&
					(cache_idx[i] != b) &&
					(cache_idx[i] != c) )
				{
					grow_cache_idx[idx++] = cache_idx[i];
				}
			}
			cache_idx = grow_cache_idx;

			//	update the weights
			//	(cache_size+3 because I want to update the triangles whose
			//	vertices fell out of the cache as well.)
			for( unsigned int i = 0; i < cache_size+3; ++i )
			if( cache_idx[i] >= 0 )
			{
				idx = cache_idx[i];
				//	store the old score
				float old_score = v[idx].score;
				//	get the new score
				float new_score = cache_score[i] + 2.0 * powf( v[idx].remaining_tris.size(), -0.5 );
				 v[idx].score = new_score;
				//	now update all remaining linked triangles!
				for( std::set<unsigned int>::iterator
						it = v[idx].remaining_tris.begin();
						it != v[idx].remaining_tris.end();
						++it )
				{
					t[*it].score += new_score - old_score;
				}
			}

			//	search for the next best tri
			best_score = 0.0;
			best_idx = -1;
			for( unsigned int i = 0; i < cache_size; ++i )
			if( cache_idx[i] >= 0 )
			{
				idx = cache_idx[i];
				//	is one of these triangles the best?
				for( std::set<unsigned int>::iterator
						it = v[idx].remaining_tris.begin();
						it != v[idx].remaining_tris.end();
						++it )
				{
					if( t[*it].score > best_score )
					{
						best_score = t[*it].score;
						best_idx = *it;
					}
				}
			}
		}
	}
	return true;
}

static
void calcSpec(const int shaderModel, HorseRadish::Vector &specular, const float *camPos, const float *lightPos, const float *lightColor, const float *vPos, const float *vNormal)
{
	HorseRadish::Vector viewVec,lightVec,auxVec;

	//iniciar assim
	specular.Set(0.0f,0.0f,0.0f);

	//usar Phong
	if (shaderModel == MESH_SHADE_PHONG)
	{
		//só verificar isto
		if (lightPos==nullptr || camPos==nullptr || lightColor==nullptr)
			return;

		//o lightVec
		lightVec.Set(lightPos);
		lightVec-=vPos;
		lightVec.Normaliza();

		//o viewVec
		viewVec.Set(camPos);
		viewVec-=vPos;
		viewVec.Normaliza();

		//refleccção
		auxVec.Set(vNormal);
		auxVec*=(-2.0f * lightVec.Dot(vNormal));
		auxVec+=lightVec;
		auxVec.Normaliza();
		
		specular.Set(lightColor);
		specular*=pow(HorseRadish::Math::fClamp(auxVec.Dot(viewVec),0.0f,1.0f),16.0f);
		return;
	}

	//usar Blinn
	if (shaderModel == MESH_SHADE_BLINN)
	{
		//só verificar isto
		if (lightPos==nullptr || camPos==nullptr || lightColor==nullptr)
			return;

		//o half vector
		auxVec.Set(vPos);
		auxVec*=-2.0f;
		auxVec+=camPos;
		auxVec+=lightPos;
		auxVec.Normaliza();
		
		specular.Set(lightColor);
		specular*=pow(HorseRadish::Math::fMax(auxVec.Dot(vNormal),0.0f),16.0f);
		return;
	}
}

int Mesh::calcComponents(const MeshAtributeType attribType)
{
	switch(attribType){
		case Mesh::Pos:
		case Mesh::Normal:
		case Mesh::Tangent:
		case Mesh::Binormal:
		case Mesh::Generic3:
								return 3;
		case Mesh::TexCoords:
		case Mesh::Generic2:
								return 2;
		case Mesh::Generic1:
								return 1;
		case Mesh::Tangent4:
		case Mesh::Generic4:
								return 4;
		}
	return 0;
}

void Mesh::cleanMesh(Mesh * const mesh)
{
	//dá jeito ter alguma coisa
	if (mesh == nullptr)
		return;

	//apago os dados, tendo em atenção que o stride diferente de 0 significa que só existe um unico buffer
	for(int i=0; i<mesh->numAttrib; i++)
	{
		//apago
		delete mesh->attrib[i].attribData;

		//se o stride não é 0, posso sair porque acabei de apagar tudo
		if (mesh->strideAttrib != 0)
			break;
	}

	//apago atributos
	if (mesh->attrib != nullptr)
		delete mesh->attrib;
	mesh->attrib = nullptr;

	//apago indices
	if (mesh->pIndex != nullptr)
		delete mesh->pIndex;
	mesh->pIndex = nullptr;

	//limpo os números
	mesh->numAttrib = mesh->numElements = mesh->numIndex = 0;
	mesh->strideAttrib = 0;
	mesh->typeIndex = (Mesh::MeshIndexType)0;
}

bool Mesh::checkMesh(const Mesh * const mesh)
{
	//verifico alguns problemas
	if (mesh==nullptr || mesh->pIndex==nullptr || mesh->attrib==nullptr || mesh->numIndex<=0 || mesh->numIndex%3!=0 || mesh->numAttrib<=0 || mesh->numElements<=0 || mesh->strideAttrib<0)
		return false;
	if (mesh->typeIndex!=Mesh::Int16 && mesh->typeIndex!=Mesh::Int32)
		return false;

	//está tudo bem
	return true;
}

bool Mesh::checkMesh(const Mesh * const mesh, const MeshAtributeType attribFlag)
{
	int juntar;

	//verifico o atributo mais simples
	if (checkMesh(mesh) == false)
		return false;

	//procuro pelo atributo
	juntar=0;
	for(int i=0; i<mesh->numAttrib; i++)
	{
		if (mesh->attrib[i].attribType & attribFlag)
		{
			juntar|=mesh->attrib[i].attribType;
			if (juntar==attribFlag)
				return true;
		}
	}

	//nao encontrei
	return false;
}

int Mesh::createFlagSetData(const MeshAtributeType flagSet, Mesh * const mesh, float ** const attribData, int * const attribStride, int * const attribComponents, MeshAtributeType * const attribType)
{
	int numAttrib,stride1,stride2,stride3,stride4;

	//comecar a zero
	numAttrib=0;

	//calculo os strides
	if (mesh->strideAttrib==0)
	{
		stride1=1;
		stride2=2;
		stride3=3;
		stride4=4;
	}
	else
	{
		stride1=mesh->strideAttrib/sizeof(float);
		stride2=mesh->strideAttrib/sizeof(float);
		stride3=mesh->strideAttrib/sizeof(float);
		stride4=mesh->strideAttrib/sizeof(float);
	}

	//a posicao
	if (flagSet & Mesh::Pos)
	{
		if (attribData)
			attribData[numAttrib]=mesh->FindAttribData(Mesh::Pos);
		if (attribStride)
			attribStride[numAttrib]=stride3;
		if (attribComponents)
			attribComponents[numAttrib]=calcComponents(Mesh::Pos);
		if (attribType)
			attribType[numAttrib]=Mesh::Pos;
		numAttrib++;
	}

	//a normal
	if (flagSet & Mesh::Normal)
	{
		if (attribData)
			attribData[numAttrib]=mesh->FindAttribData(Mesh::Normal);
		if (attribStride)
			attribStride[numAttrib]=stride3;
		if (attribComponents)
			attribComponents[numAttrib]=calcComponents(Mesh::Normal);
		if (attribType)
			attribType[numAttrib]=Mesh::Normal;
		numAttrib++;
	}

	//a tangente
	if (flagSet & Mesh::Tangent)
	{
		if (attribData)
			attribData[numAttrib]=mesh->FindAttribData(Mesh::Tangent);
		if (attribStride)
			attribStride[numAttrib]=stride3;
		if (attribComponents)
			attribComponents[numAttrib]=calcComponents(Mesh::Tangent);
		if (attribType)
			attribType[numAttrib]=Mesh::Tangent;
		numAttrib++;
	}

	//a binormal
	if (flagSet & Mesh::Binormal)
	{
		if (attribData)
			attribData[numAttrib]=mesh->FindAttribData(Mesh::Binormal);
		if (attribStride)
			attribStride[numAttrib]=stride3;
		if (attribComponents)
			attribComponents[numAttrib]=calcComponents(Mesh::Binormal);
		if (attribType)
			attribType[numAttrib]=Mesh::Binormal;
		numAttrib++;
	}

	//a tangent4
	if (flagSet & Mesh::Tangent4)
	{
		if (attribData)
			attribData[numAttrib]=mesh->FindAttribData(Mesh::Tangent4);
		if (attribStride)
			attribStride[numAttrib]=stride4;
		if (attribComponents)
			attribComponents[numAttrib]=calcComponents(Mesh::Tangent4);
		if (attribType)
			attribType[numAttrib]=Mesh::Tangent4;
		numAttrib++;
	}

	//a tex
	if (flagSet & Mesh::TexCoords)
	{
		if (attribData)
			attribData[numAttrib]=mesh->FindAttribData(Mesh::TexCoords);
		if (attribStride)
			attribStride[numAttrib]=stride2;
		if (attribComponents)
			attribComponents[numAttrib]=calcComponents(Mesh::TexCoords);
		if (attribType)
			attribType[numAttrib]=Mesh::TexCoords;
		numAttrib++;
	}

	//verificar se tudo o que foi pedido existe
	for(int i=0; i<numAttrib; i++)
	{
		if (attribData[i]==nullptr)
			return 0;
	}

	//devolver este numero e mais nada
	return numAttrib;
}

void Mesh::copyVertexData(const Mesh * const meshSource, const int vertSource, Mesh * const meshDest, const int vertDest)
{
	int i,componentes,strideS,strideD;
	Attribute *attribS,*attribD;
	float *attribDataTo,*attribDataFrom;

	if (meshSource==nullptr || meshDest==nullptr || vertSource<0 || vertDest<0 || vertSource>=meshSource->numElements || vertDest>=meshDest->numElements)
		return;
	if (meshSource->numAttrib!=meshDest->numAttrib)
		return;

	//se for para escrever no mesmo sitio
	if (meshSource==meshDest)
	{
		//se for para o mesmo sitio, nao faço nada
		if (vertDest==vertSource)
			return;

		//tenho de verificar que tipo de stride tenho
		if(meshSource->strideAttrib==0)
		{
			for(attribS=meshSource->attrib,i=0; i<meshSource->numAttrib; i++,attribS++)
			{
				componentes=calcComponents(attribS->attribType);
				attribDataTo=attribS->attribData+vertDest*componentes;
				attribDataFrom=attribS->attribData+vertSource*componentes;
				memcpy(attribDataTo,attribDataFrom,sizeof(float)*componentes);
			}
			return;
		}

		//tenho strie
		strideS=meshSource->strideAttrib/sizeof(float);
		attribDataTo=meshSource->attrib[0].attribData+strideS*vertDest;
		attribDataFrom=meshSource->attrib[0].attribData+strideS*vertSource;
		memcpy(attribDataTo,attribDataFrom,meshSource->strideAttrib);
		return;
	}

	//tenho de verificar que tipo de stride tenho
	attribS=meshSource->attrib;
	attribD=meshDest->attrib;
	strideS=meshSource->strideAttrib/sizeof(float);
	strideD=meshDest->strideAttrib/sizeof(float);
	for(i=0; i<meshSource->numAttrib; i++,attribS++,attribD++)
	{
		componentes=calcComponents(attribS->attribType);
		
		if (strideD)
			attribDataTo=attribD->attribData+vertDest*strideD;
		else
			attribDataTo=attribD->attribData+vertDest*componentes;
		
		if (strideS)
			attribDataFrom=attribS->attribData+vertSource*strideS;
		else
			attribDataFrom=attribS->attribData+vertSource*componentes;

		memcpy(attribDataTo,attribDataFrom,sizeof(float)*componentes);
	}
}

bool Mesh::criaMaisVertices(Mesh * const mesh, const int newCountVert)
{
	float *newBuffer,*offset;
	int i,componentes;

	//tenho de verificar isto
	if (mesh==nullptr || mesh->numAttrib<=0 || mesh->numElements>=newCountVert)
		return false;

	//tenho de fazer a distinção entre ubber buffer e buffers individuais
	if (mesh->strideAttrib==0)
	{
		//para cada atributo
		for(i=0; i<mesh->numAttrib; i++)
		{
			//quantos componentes
			componentes=calcComponents(mesh->attrib[i].attribType);

			//aumento o buffer
			newBuffer=(float*)realloc(mesh->attrib[i].attribData,componentes*newCountVert*sizeof(float));
			if (newBuffer==nullptr)
				return false;
			mesh->attrib[i].attribData=newBuffer;
		}

		//só falta mudar o novo numero de elementos
		mesh->numElements=newCountVert;

		//já tá, correu tudo bem
		return true;
	}

	//tenho um ubber buffer
	//conto quantos componentes tenho no total
	for(i=componentes=0; i<mesh->numAttrib; i++)
		componentes+=calcComponents(mesh->attrib[i].attribType);

	//aumento o buffer
	newBuffer=(float*)realloc(mesh->attrib[0].attribData,componentes*newCountVert*sizeof(float));
	if (newBuffer==nullptr)
		return false;

	//para cada atributo, apago o antigo e arranjo o novo ponteiro
	offset=newBuffer;
	for(i=0; i<mesh->numAttrib; i++)
	{
		mesh->attrib[i].attribData=offset;
		offset+=calcComponents(mesh->attrib[i].attribType);
	}

	//só falta mudar o novo numero de elementos
	mesh->numElements=newCountVert;

	//e já está
	return true;
}

bool Mesh::criaMaisIndices(Mesh * const mesh, const int newCountIndex)
{
	unsigned int *bufferI,*walkerI;
	unsigned short *bufferS,*walkerS;
	int i;

	//tenho de verificar isto
	if (checkMesh(mesh)==false || mesh->numIndex>=newCountIndex)
		return false;

	//se os antigos forem short e o novo tiver de ser int
	if (mesh->typeIndex==Mesh::Int16 && newCountIndex>=65500)
	{
		//crio o espaco para mais indices
		bufferI=new unsigned int[newCountIndex];
		if (bufferI==nullptr)
			return false;

		//copio para la as coisas
		walkerI=bufferI;
		walkerS=(unsigned short*)mesh->pIndex;
		for(i=0; i<mesh->numIndex; i++)
			*walkerI=*walkerS;

		//mudo as coisas e já tá
		delete [] mesh->pIndex;
		mesh->pIndex=bufferI;
		mesh->numIndex=newCountIndex;
		mesh->typeIndex=Mesh::Int32;

		//pronto, fixe, correu tudo bem
		return true;
	}
	
	//chegando aqui nao é preciso mudar o tipo de indices, basta fazer o realloc
	if (mesh->typeIndex==Mesh::Int16)
	{
		//crio o espaco para mais indices
		bufferS=(unsigned short*)realloc(mesh->pIndex,sizeof(unsigned short)*newCountIndex);
		if (bufferS==nullptr)
			return false;

		//mudar isto e mais nada
		mesh->pIndex=bufferS;
		mesh->numIndex=newCountIndex;

		//vamos embora
		return true;
	}

	//crio o espaco para mais indices
	bufferI=(unsigned int*)realloc(mesh->pIndex,sizeof(unsigned int)*newCountIndex);
	if (bufferI==nullptr)
		return false;

	//mudar isto e mais nada
	mesh->pIndex=bufferI;
	mesh->numIndex=newCountIndex;

	//about time
	return true;
}

unsigned int Mesh::getIndex(const Mesh * const mesh, const int index)
{
	if (mesh==nullptr || index<0 || index>=mesh->numIndex)
		return 0;

	if (mesh->typeIndex==Mesh::Int16)
		return ((unsigned short*)mesh->pIndex)[index];
	if (mesh->typeIndex==Mesh::Int32)
		return ((unsigned int*)mesh->pIndex)[index];
	return 0;
}

void Mesh::writeIndex(Mesh * const mesh, const int index, const unsigned int val)
{
	if (mesh==nullptr || index<0 || index>=mesh->numIndex)
		return;

	if (mesh->typeIndex==Mesh::Int16)
	{
		((unsigned short*)mesh->pIndex)[index]=(unsigned short)val;
		return;
	}

	if (mesh->typeIndex==Mesh::Int32)
	{
		((unsigned int*)mesh->pIndex)[index]=val;
		return;
	}
}

void Mesh::copyIndexTri(const Mesh * const mesh, const int indexTo, const int indexFrom)
{
	unsigned short *pointerS;
	unsigned int *pointerI;

	if (mesh==nullptr || indexTo<0 || indexTo>=mesh->numIndex || indexFrom<0 || indexFrom>=mesh->numIndex)
		return;

	if (mesh->typeIndex==Mesh::Int16)
	{
		pointerS=(unsigned short*)mesh->pIndex;
		pointerS[indexTo+0]=pointerS[indexFrom+0];
		pointerS[indexTo+1]=pointerS[indexFrom+1];
		pointerS[indexTo+2]=pointerS[indexFrom+2];
		return;
	}

	if (mesh->typeIndex==Mesh::Int32)
	{
		pointerI=(unsigned int*)mesh->pIndex;
		pointerI[indexTo+0]=pointerI[indexFrom+0];
		pointerI[indexTo+1]=pointerI[indexFrom+1];
		pointerI[indexTo+2]=pointerI[indexFrom+2];
		return;
	}
}

bool Mesh::zeroAreaTri(Mesh * const mesh, const unsigned int &v1, const unsigned int &v2, const unsigned int &v3)
{
	int stride;
	float *pPos;
	HorseRadish::Vector p1,p2,p3,d01,d02,prod;

	//vou buscar o ponteiro para a posicao
	pPos=mesh->FindAttribData(Mesh::Pos);
	if (pPos==nullptr)
		return false;

	//vou buscar os pontos
	if (mesh->strideAttrib==0)
	{
		p1.Set(pPos+v1*3);
		p2.Set(pPos+v2*3);
		p3.Set(pPos+v3*3);
	}
	else
	{
		stride=mesh->strideAttrib/sizeof(float);
		p1.Set(pPos+v1*stride);
		p2.Set(pPos+v2*stride);
		p3.Set(pPos+v3*stride);
	}

	d01.Set(p1);
	d02.Set(p1);
	d01-=p2;
	d02-=p3;

	prod.CalcCrossProduct(d01,d02);
	return HorseRadish::Math::isZero(prod.x*prod.x + prod.y*prod.y + prod.z*prod.z);
}

void Mesh::createFull(Mesh *mesh, const int flags)
{
	HorseRadish::Vector dir1,dir2,normal,vecAux,tangent,binormal,sdir,tdir;
	float *pPos,*pTex,*pNor,*pTan,*pBin,*walkN,*walkT,*walkB,s1,s2,t1,t2,r;
	bool hasBro,*vertTouched;
	int i,j,stride2,stride3;
	unsigned int index1,index2,index3;

	//os atributos
	pPos=mesh->FindAttribData(Mesh::Pos);
	pNor=mesh->FindAttribData(Mesh::Normal);
	pTan=mesh->FindAttribData(Mesh::Tangent);
	pBin=mesh->FindAttribData(Mesh::Binormal);
	pTex=mesh->FindAttribData(Mesh::TexCoords);

	//preciso de todos
	if (pPos==nullptr || pTex==nullptr || pNor==nullptr || pTan==nullptr || pBin==nullptr)
		return;

	//calculo os strides
	stride2=2;
	stride3=3;
	if (mesh->strideAttrib!=0)
	{
		stride2=mesh->strideAttrib/sizeof(float);
		stride3=stride2;
	}

	//vou buscar esta cena
	walkN=pNor;
	walkT=pTan;
	walkB=pBin;

	//para todos os vertices, inicio a zero
	if (stride3==3)
	{
		if (!(flags & MESH_ORTHO_KEEP_NORMALS))
			memset(walkN, 0, sizeof(float)*3*mesh->numElements);
		memset(walkT, 0, sizeof(float)*3*mesh->numElements);
		memset(walkB, 0, sizeof(float)*3*mesh->numElements);
	}
	else
	{
		for(i=0; i<mesh->numElements; i++,walkN+=stride3,walkT+=stride3,walkB+=stride3)
		{
			if (!(flags & MESH_ORTHO_KEEP_NORMALS))
				walkN[0]=walkN[1]=walkN[2]=0.0f;
			walkT[0]=walkT[1]=walkT[2]=0.0f;
			walkB[0]=walkB[1]=walkB[2]=0.0f;
		}
	}

	//para todos os triangulos
	for(i=0; i<mesh->numIndex; i+=3)
	{
		//os indices
		index1=getIndex(mesh,i+0);
		index2=getIndex(mesh,i+1);
		index3=getIndex(mesh,i+2);

		//as direcções
		dir1.Set(pPos+index2*stride3);
		dir2.Set(pPos+index3*stride3);
		dir1-=pPos+index1*stride3;
		dir2-=pPos+index1*stride3;

		//posso calcular a normal
		normal.CalcCrossProduct(dir1,dir2);
		normal.Normaliza();

		//as direcções das texturas
		s1=pTex[index2*stride2+0]-pTex[index1*stride2+0];
		s2=pTex[index3*stride2+0]-pTex[index1*stride2+0];
		t1=pTex[index2*stride2+1]-pTex[index1*stride2+1];
		t2=pTex[index3*stride2+1]-pTex[index1*stride2+1];
		r=1.0f/(s1 * t2 - s2 * t1);

		//primeiras tangentes e binormais
		sdir.x=(t2 * dir1.x - t1 * dir2.x) * r;
		sdir.y=(t2 * dir1.y - t1 * dir2.y) * r;
		sdir.z=(t2 * dir1.z - t1 * dir2.z) * r;

		tdir.x=(s1 * dir2.x - s2 * dir1.x) * r;
		tdir.y=(s1 * dir2.y - s2 * dir1.y) * r;
		tdir.z=(s1 * dir2.z - s2 * dir1.z) * r;

		//posso fazer isto
		index1*=stride3;
		index2*=stride3;
		index3*=stride3;

		//soma as contribiuções dos vertices
		if (!(flags & MESH_ORTHO_KEEP_NORMALS))
		{
			somaVect(pNor+index1,normal);
			somaVect(pNor+index2,normal);
			somaVect(pNor+index3,normal);
		}
		somaVect(pTan+index1,sdir);
		somaVect(pTan+index2,sdir);
		somaVect(pTan+index3,sdir);
		somaVect(pBin+index1,tdir);
		somaVect(pBin+index2,tdir);
		somaVect(pBin+index3,tdir);
	}

	//vou buscar esta cena
	walkN=pNor;
	walkT=pTan;
	walkB=pBin;

	//para todos os vertices, normalizo a normal, tangent e binormal
	for(i=0; i<mesh->numElements; i++,walkN+=stride3,walkT+=stride3,walkB+=stride3)
	{
		//vou buscar isto
		normal.Set(walkN);
		sdir.Set(walkT);
		tdir.Set(walkB);

		//tenho de normalizar a normal
		normal.Normaliza();

		//calculo a tangente
		vecAux.Set(normal);
		vecAux*=normal.Dot(sdir);
		tangent.x=sdir.x-vecAux.x;
		tangent.y=sdir.y-vecAux.y;
		tangent.z=sdir.z-vecAux.z;
		tangent.Normaliza();

		//calculo a binormal
		vecAux.CalcCrossProduct(normal,sdir);
		if (vecAux.Dot(tdir)<0.0f)
			binormal.CalcCrossProduct(tangent,normal);
		else
			binormal.CalcCrossProduct(normal,tangent);
		binormal.Normaliza();

		//escrevo as cenas
		if (!(flags & MESH_ORTHO_KEEP_NORMALS))
			normal.Write(walkN);
		tangent.Write(walkT);
		binormal.Write(walkB);
	}

	//se tiver de fazer esta cena por vértice
	if (flags & MESH_ORTHO_PER_POSITION_NRM)
	{
		//criar esta coisinha
		vertTouched=new bool[mesh->numElements];
		if (vertTouched)
		{
			//colocar tudo a zero
			for(i=0; i<mesh->numElements; i++)
				vertTouched[i]=false;

			//para cada vertice
			for(i=0; i<mesh->numElements; i++)
			{
				//se por acaso já foi tocado, segue
				if (vertTouched[i])
					continue;

				//este vertice
				dir1.Set(pPos+i*stride3);
				hasBro=false;

				//se tiver vertices iguais
				for(j=i+1; j<mesh->numElements; j++)
				{
					//este vértice e se for diferente, segue caminho
					dir2.Set(pPos+j*stride3);
					if (dir1.x!=dir2.x || dir1.y!=dir2.y || dir1.z!=dir2.z)
						continue;
					
					//este vértice tá tocado e existe irmão
					vertTouched[j]=true;
					hasBro=true;

					//a normal do segundo e soma-se para o primeiro
					somaVect(pNor+i*stride3,pNor+j*stride3);
					somaVecCheckDir(pTan+i*stride3,pTan+j*stride3);
					somaVecCheckDir(pBin+i*stride3,pBin+j*stride3);
				}

				//se não teve irmão, fixe, nao se fez nada, segue para o próximo
				if (hasBro==false)
					continue;

				//primeiro normalizo tudo e volto a escrever
				normal.Set(pNor+i*stride3);		normal.Normaliza();			normal.Write(pNor+i*stride3);
				tangent.Set(pTan+i*stride3);	tangent.Normaliza();		tangent.Write(pTan+i*stride3);
				binormal.Set(pBin+i*stride3);	binormal.Normaliza();		binormal.Write(pBin+i*stride3);

				//tenho de escrever a nova normal nos outros pontos todos
				for(j=i+1; j<mesh->numElements; j++)
				{
					//este vértice e se for diferente, segue caminho
					dir2.Set(pPos+j*stride3);
					if (dir1.x!=dir2.x || dir1.y!=dir2.y || dir1.z!=dir2.z)
						continue;

					//escreve a normal que é para escrever
					normal.Write(pNor+j*stride3);
					tangent.Write(pTan+j*stride3);
					binormal.Write(pBin+j*stride3);
				}
			}

			//apagar isto
			delete [] vertTouched;
		}
	}
}

void Mesh::createTangent4(Mesh *mesh, const int flags)
{
	HorseRadish::Vector dir1,dir2,normal,vecAux,tangent,sdir,tdir;
	float *pPos,*pTex,*pNor,*pGen,*walkN,*walkG,s1,s2,t1,t2,r,*novoArray,weight;
	int i,j,stride2,stride3,stride4;
	unsigned int index1,index2,index3;
	bool hasBro,*vertTouched;

	//os atributos
	pPos=mesh->FindAttribData(Mesh::Pos);
	pNor=mesh->FindAttribData(Mesh::Normal);
	pGen=mesh->FindAttribData(Mesh::Tangent4);
	pTex=mesh->FindAttribData(Mesh::TexCoords);

	//preciso de todos
	if (pPos==nullptr || pTex==nullptr || pNor==nullptr || pGen==nullptr)
		return;

	//tenho de criar este espaco auxiliar para guardar algumas cenas
	novoArray=(float*)malloc(sizeof(float)*mesh->numElements*3);
	if (novoArray==nullptr)
		return;

	//calculo os strides
	stride2=2;
	stride3=3;
	stride4=4;
	if (mesh->strideAttrib!=0)
	{
		stride2=mesh->strideAttrib/sizeof(float);
		stride4=stride3=stride2;
	}

	//vou buscar esta cena
	walkN=pNor;
	walkG=pGen;

	//para todos os vertices, inicios a zero
	memset(novoArray, 0, sizeof(float)*mesh->numElements*3);
	if (stride3==3 && stride4==4)
	{
		memset(walkN, 0, sizeof(float)*mesh->numElements*3);
		memset(walkG, 0, sizeof(float)*mesh->numElements*4);
	}
	else
	{
		for(i=0; i<mesh->numElements; i++,walkN+=stride3,walkG+=stride4)
		{
			walkN[0]=walkN[1]=walkN[2]=0.0f;
			walkG[0]=walkG[1]=walkG[2]=walkG[3]=0.0f;
		}
	}

	//para todos os triangulos
	for(i=0; i<mesh->numIndex; i+=3)
	{
		//os indices
		index1=getIndex(mesh,i+0);
		index2=getIndex(mesh,i+1);
		index3=getIndex(mesh,i+2);

		//as direcções
		dir1.Set(pPos+index2*stride3);
		dir2.Set(pPos+index3*stride3);
		dir1-=pPos+index1*stride3;
		dir2-=pPos+index1*stride3;

		//posso calcular a normal
		normal.CalcCrossProduct(dir1,dir2);
		normal.Normaliza();

		//as direcções das texturas
		s1=pTex[index2*stride2+0]-pTex[index1*stride2+0];
		s2=pTex[index3*stride2+0]-pTex[index1*stride2+0];
		t1=pTex[index2*stride2+1]-pTex[index1*stride2+1];
		t2=pTex[index3*stride2+1]-pTex[index1*stride2+1];
		r=1.0f/(s1 * t2 - s2 * t1);

		//primeiras tangentes e binormais
		sdir.x=(t2 * dir1.x - t1 * dir2.x) * r;
		sdir.y=(t2 * dir1.y - t1 * dir2.y) * r;
		sdir.z=(t2 * dir1.z - t1 * dir2.z) * r;

		tdir.x=(s1 * dir2.x - s2 * dir1.x) * r;
		tdir.y=(s1 * dir2.y - s2 * dir1.y) * r;
		tdir.z=(s1 * dir2.z - s2 * dir1.z) * r;

		//soma as contribiuções dos vertices
		somaVect(pNor+index1*stride3,normal);
		somaVect(pNor+index2*stride3,normal);
		somaVect(pNor+index3*stride3,normal);
		somaVect(pGen+index1*stride4,sdir);
		somaVect(pGen+index2*stride4,sdir);
		somaVect(pGen+index3*stride4,sdir);
		somaVect(novoArray+index1*3,tdir);
		somaVect(novoArray+index2*3,tdir);
		somaVect(novoArray+index3*3,tdir);
		continue;

		//calculo o "peso" para o primeiro vértice
		dir1=pPos+index2*stride3;		dir1-=pPos+index1*stride3;	dir1.Normaliza();
		dir2=pPos+index3*stride3;		dir2-=pPos+index1*stride3;	dir2.Normaliza();	
		weight=acosf(dir1.Dot(dir2));
		somaVect(pNor+index1*stride3,normal*weight);
		/*somaVect(pGen+index1*stride4,sdir*weight);
		somaVect(novoArray+index1*3,tdir*weight);*/

		//calculo o "peso" para o segundo vértice
		dir1=pPos+index3*stride3;		dir1-=pPos+index2*stride3;	dir1.Normaliza();
		dir2=pPos+index1*stride3;		dir2-=pPos+index2*stride3;	dir2.Normaliza();	
		weight=acosf(dir1.Dot(dir2));
		somaVect(pNor+index2*stride3,normal*weight);
		/*somaVect(pGen+index2*stride4,sdir*weight);
		somaVect(novoArray+index2*3,tdir*weight);*/

		//calculo o "peso" para o terceiro vértice
		dir1=pPos+index1*stride3;		dir1-=pPos+index3*stride3;	dir1.Normaliza();
		dir2=pPos+index2*stride3;		dir2-=pPos+index3*stride3;	dir2.Normaliza();	
		weight=acosf(dir1.Dot(dir2));
		somaVect(pNor+index3*stride3,normal*weight);
		/*somaVect(pGen+index3*stride4,sdir*weight);
		somaVect(novoArray+index3*3,tdir*weight);*/
	}

	//vou buscar esta cena
	walkN=pNor;
	walkG=pGen;

	//para todos os vertices, normalizo a normal, tangent e binormal
	for(i=0; i<mesh->numElements; i++,walkN+=stride3,walkG+=stride4)
	{
		//vou buscar isto
		normal.Set(walkN);
		sdir.Set(walkG);
		tdir.Set(novoArray+i*3);

		//tenho de normalizar a normal
		normal.Normaliza();

		//calculo a tangente
		vecAux.Set(normal);
		vecAux*=normal.Dot(sdir);
		tangent.x=sdir.x-vecAux.x;
		tangent.y=sdir.y-vecAux.y;
		tangent.z=sdir.z-vecAux.z;
		tangent.Normaliza();

		//calculo a binormal
		r=1.0f;
		vecAux.CalcCrossProduct(normal,sdir);
		if (vecAux.Dot(tdir)<0.0f)
			r=-1.0f;

		//escrevo as cenas
		normal.Write(walkN);
		tangent.Write(walkG);
		walkG[3]=r;
	}

	//apago sito porque já não preciso
	free(novoArray);

	//se tiver de fazer esta cena por vértice
	if (flags & MESH_ORTHO_PER_POSITION_NRM)
	{
		//criar esta coisinha
		vertTouched=(bool*)malloc(sizeof(bool)*mesh->numElements);
		if (vertTouched != nullptr)
		{
			//colocar tudo a zero
			for(i=0; i<mesh->numElements; i++)
				vertTouched[i]=false;

			//para cada vertice
			for(i=0; i<mesh->numElements; i++)
			{
				//se por acaso já foi tocado, segue
				if (vertTouched[i])
					continue;

				//este vertice
				dir1.Set(pPos+i*stride3);
				hasBro=false;

				//se tiver vertices iguais
				for(j=i+1; j<mesh->numElements; j++)
				{
					//este vértice e se for diferente, segue caminho
					dir2.Set(pPos+j*stride3);
					if (dir1.x!=dir2.x || dir1.y!=dir2.y || dir1.z!=dir2.z)
						continue;
					
					//este vértice tá tocado e existe irmão
					vertTouched[j]=true;
					hasBro=true;

					//a normal do segundo e soma-se para o primeiro
					somaVect(pNor+i*stride3,pNor+j*stride3);
					somaVecCheckDir(pGen+i*stride4,pGen+j*stride4);
				}

				//se não teve irmão, fixe, nao se fez nada, segue para o próximo
				if (hasBro==false)
					continue;

				//primeiro normalizo tudo e volto a escrever
				normal.Set(pNor+i*stride3);		normal.Normaliza();			normal.Write(pNor+i*stride3);
				tangent.Set(pGen+i*stride4);	tangent.Normaliza();		tangent.Write(pGen+i*stride4);

				//tenho de escrever a nova normal nos outros pontos todos
				for(j=i+1; j<mesh->numElements; j++)
				{
					//este vértice e se for diferente, segue caminho
					dir2.Set(pPos+j*stride3);
					if (dir1.x!=dir2.x || dir1.y!=dir2.y || dir1.z!=dir2.z)
						continue;

					//escreve a normal que é para escrever
					normal.Write(pNor+j*stride3);
					tangent.Write(pGen+j*stride4);
				}
			}

			//apagar isto
			free(vertTouched);
		}
	}
}

void Mesh::createNormals(Mesh *mesh, const int flags)
{
	float *normalWalker,*posWalker,*escreveNorm,weight;
	bool hasBro,*vertTouched;
	unsigned int i1,i2,i3;
	HorseRadish::Vector norm,v1,v2;
	int i,j,stride;

	if (checkMesh(mesh,Mesh::Pos)==false)
		return;

	//se não tenho normais
	if (mesh->FindAttribData(Mesh::Normal) == nullptr)
		return;

	//calculo o stride
	stride=3;
	if (mesh->strideAttrib!=0)
		stride=mesh->strideAttrib/sizeof(float);

	//coloco as normais a zero
	normalWalker=mesh->FindAttribData(Mesh::Normal);
	if (stride==3){
		memset(normalWalker,0,sizeof(float)*mesh->numElements*3);
		}
	else
		{
		for(i=0; i<mesh->numElements; i++,normalWalker+=stride){
			normalWalker[0]=normalWalker[1]=normalWalker[2]=0.0f;
			}
		}

	//os ponteiros dão imenso jeito
	posWalker=mesh->FindAttribData(Mesh::Pos);
	normalWalker=mesh->FindAttribData(Mesh::Normal);

	//ando por todos os triangulo e calculo a normal desse triangulo
	for(i=0; i<mesh->numIndex; i+=3)
	{
		//os indices
		if (mesh->typeIndex==Mesh::Int16)
		{
			i1=((unsigned short*)mesh->pIndex)[i+0];
			i2=((unsigned short*)mesh->pIndex)[i+1];
			i3=((unsigned short*)mesh->pIndex)[i+2];
		}
		else
		{
			i1=((unsigned int*)mesh->pIndex)[i+0];
			i2=((unsigned int*)mesh->pIndex)[i+1];
			i3=((unsigned int*)mesh->pIndex)[i+2];
		}

		//vou sempre precisar deles multiplicados pelo stride, logo, faço já isso
		i1*=stride;
		i2*=stride;
		i3*=stride;

		//calculo a normal desta face
		norm.CalcNormal(posWalker+i1,posWalker+i2,posWalker+i3);

		//calculo o "peso" para o primeiro vértice
		v1=posWalker+i2;		v1-=posWalker+i1;	v1.Normaliza();
		v2=posWalker+i3;		v2-=posWalker+i1;	v2.Normaliza();	
		weight=acosf(v1.Dot(v2));
		somaVect(normalWalker+i1,norm*weight);

		//calculo o "peso" para o segundo vértice
		v1=posWalker+i3;		v1-=posWalker+i2;	v1.Normaliza();
		v2=posWalker+i1;		v2-=posWalker+i2;	v2.Normaliza();	
		weight=acosf(v1.Dot(v2));
		somaVect(normalWalker+i2,norm*weight);

		//calculo o "peso" para o terceiro vértice
		v1=posWalker+i1;		v1-=posWalker+i3;	v1.Normaliza();
		v2=posWalker+i2;		v2-=posWalker+i3;	v2.Normaliza();	
		weight=acosf(v1.Dot(v2));
		somaVect(normalWalker+i3,norm*weight);
	}

	//se tiver de fazer o smooth por vértice de posição idêntica
	if (flags & MESH_ORTHO_PER_POSITION_NRM)
	{
		//criar esta coisinha
		vertTouched=new bool[mesh->numElements];
		if (vertTouched)
		{
			//colocar tudo a zero
			for(i=0; i<mesh->numElements; i++)
				vertTouched[i]=false;

			//para cada vertice
			for(i=0; i<mesh->numElements; i++)
			{
				//se por acaso já foi tocado, segue
				if (vertTouched[i])
					continue;

				//este vertice e o sitio para a normal
				v1.Set(posWalker+i*stride);
				escreveNorm=normalWalker+i*stride;
				hasBro=false;

				//se tiver vertices iguais
				for(j=i+1; j<mesh->numElements; j++)
				{
					//este vértice e se for diferente, segue caminho
					v2.Set(posWalker+j*stride);
					if (v1.x!=v2.x || v1.y!=v2.y || v1.z!=v2.z)
						continue;
					
					//este vértice tá tocado e existe irmão
					vertTouched[j]=true;
					hasBro=true;

					//a normal do segundo e soma-se para o primeiro
					norm.Set(normalWalker+j*stride);
					somaVect(escreveNorm,norm);
				}

				//se não teve irmão, fixe, nao se fez nada, segue para o próximo
				if (hasBro==false)
					continue;

				//quero escrever esta normal nos outros todos
				norm.Set(escreveNorm);

				//tenho de escrever a nova normal nos outros pontos todos
				for(j=i+1; j<mesh->numElements; j++)
				{
					//este vértice e se for diferente, segue caminho
					v2.Set(posWalker+j*stride);
					if (v1.x!=v2.x || v1.y!=v2.y || v1.z!=v2.z)
						continue;

					//escreve a normal que é para escrever
					escreveNorm=normalWalker+j*stride;
					norm.Write(escreveNorm);
				}
			}

			//apagar isto
			delete [] vertTouched;
		}
	}

	//normalizo as normais
	if (!(flags & MESH_ORTHO_NO_FINAL_NRM))
	{
		normalWalker=mesh->FindAttribData(Mesh::Normal);
		for(i=0; i<mesh->numElements; i++)
		{
			//normalizo
			norm.Set(normalWalker);
			norm.Normaliza();

			//copio e avanço com o ponteiro
			norm.Write(normalWalker);
			normalWalker=normalWalker+stride;
		}
	}
}

Mesh::Mesh()
{
	//por omissão limpo tudo
	this->numAttrib = this->numElements = this->numIndex = 0;
	this->strideAttrib = 0;
	this->typeIndex = (Mesh::MeshIndexType)0;
	this->attrib = nullptr;
	this->pIndex = nullptr;
}

Mesh::~Mesh()
{
	//basta chamar esta função que trata de tudo
	cleanMesh(this);
}

Mesh* Mesh::Clone() const
{
	int i,componentes;
	float *newBuffer,*offset;
	Mesh *meshDest;

	//verificar toujours le parametros
	if (checkMesh(this)==false)
		return nullptr;

	//preciso disto
	meshDest = new Mesh();
	if (meshDest == nullptr)
		return nullptr;
	
	//criar espaço para os indices
	if (this->typeIndex==Mesh::Int32)
	{
		meshDest->typeIndex=Mesh::Int32;
		meshDest->pIndex=new unsigned int[this->numIndex];
		if (meshDest->pIndex==nullptr)
		{
			delete meshDest;
			return nullptr;
		}
		memcpy(meshDest->pIndex,this->pIndex,sizeof(unsigned int)*this->numIndex);
	}
	else
	{
		meshDest->typeIndex=Mesh::Int16;
		meshDest->pIndex=new unsigned short[this->numIndex];
		if (meshDest->pIndex==nullptr)
		{
			delete meshDest;
			return nullptr;
		}
		memcpy(meshDest->pIndex,this->pIndex,sizeof(unsigned short)*this->numIndex);
	}

	//copio todas as coisas que posso e devo copiar
	meshDest->numIndex=this->numIndex;
	meshDest->numAttrib=this->numAttrib;
	meshDest->strideAttrib=this->strideAttrib;
	meshDest->numElements=this->numElements;

	//crio espaço para os atributos
	meshDest->attrib=new Attribute[meshDest->numAttrib];
	if (meshDest->attrib==nullptr)
	{
		delete meshDest;
		return nullptr;
	}

	//para cada atributo, copio de que tipo é
	for(i=0; i<meshDest->numAttrib; i++)
		meshDest->attrib[i].attribType=this->attrib[i].attribType;

	//agora, de acordo com o stride
	if (meshDest->strideAttrib==0)
	{
		//para cada atributo
		for(i=0; i<meshDest->numAttrib; i++)
		{
			//quantos componentes tem
			componentes=calcComponents(meshDest->attrib[i].attribType);

			//crio novo buffer
			newBuffer=new float[componentes*meshDest->numElements];
			if (newBuffer==nullptr)
			{
				delete meshDest;
				return nullptr;
			}

			//copio para lá tudo
			memcpy(newBuffer,this->attrib[i].attribData,sizeof(float)*componentes*meshDest->numElements);

			//arranjo o ponteiro antigo
			meshDest->attrib[i].attribData=newBuffer;
		}
	}
	else
	{
		//quantos componentes tenho
		for(i=componentes=0; i<meshDest->numAttrib; i++)
			componentes+=calcComponents(meshDest->attrib[i].attribType);

		//crio um novo ubber buffer
		newBuffer=new float[componentes*meshDest->numElements];
		if (newBuffer==nullptr)
		{
			delete meshDest;
			return nullptr;
		}

		//copio para lá tudo
		memcpy(newBuffer,this->attrib[0].attribData,sizeof(float)*componentes*this->numElements);

		//agora, para cada atributo tenho de arranjar o ponteiro
		offset=newBuffer;
		for(i=0; i<meshDest->numAttrib; i++)
		{	
			meshDest->attrib[i].attribData=offset;
			offset+=calcComponents(meshDest->attrib[i].attribType);
		}
	}

	//já tá, correu tudo bem
	return meshDest;
}

void Mesh::Clear()
{
	//basta chamar esta função que trata de tudo
	cleanMesh(this);
}

Mesh::Attribute* Mesh::FindAttrib(const MeshAtributeType type)
{
	int i;

	//verificar parametro e se a mesh está em condições
	if (this->attrib==nullptr || this->numAttrib<=0)
		return nullptr;

	//esta função detecta todos os atributos exceptos os genericos
	if (type<Mesh::Pos || type>Mesh::TexCoords)
		return nullptr;

	//procuro pelo atributo
	for(i=0; i<this->numAttrib; i++)
		{
		//se for encontrado
		if (this->attrib[i].attribType==type)
			return &this->attrib[i];
		}

	//não encontrado
	return nullptr;
}

Mesh::Attribute* Mesh::FindAttribGeneric(const MeshAtributeType type, const int attribCount)
{
	int i,count;

	//verificar parametro e se a mesh está em condições
	if (attribCount<=0 || this->attrib==nullptr || this->numAttrib<=0)
		return nullptr;

	//esta função detecta só os genericos
	if (type<Mesh::Generic1 || type>Mesh::Generic4)
		return nullptr;

	//procuro pelo atributo
	for(i=count=0; i<this->numAttrib; i++)
		{
		//se for encontrado
		if (this->attrib[i].attribType==type)
			{
			count++;
			if (count==attribCount)
				return &this->attrib[i];
			}
		}

	//não encontrado
	return nullptr;
}

Mesh::Attribute* Mesh::NewAttrib(const MeshAtributeType type, const int numElements)
{
	int i,j,componentes,totalComponentes,stride;
	Attribute *atributo;
	float *newUbberBuffer,*ubberWalker;

	//se isto nao tiver de acordo, pois nao posso mudar o numero de elementos que já
	//existe ou meter menos ou mais elementos que o que já lá está
	if ( (this->numElements!=0) && (this->numElements!=numElements) )
		return nullptr;

	//descubro quantas caracteristicas vou ter de ter
	componentes=calcComponents(type);
	if (componentes==0)
		return nullptr;

	//se não for genérico, não pode ser repetido
	if ( !(type>=Mesh::Generic1 && type<=Mesh::Generic4) )
	{
		if (this->FindAttrib(type)!=nullptr)
			return nullptr;
	}

	//guardo isto
	this->numElements=numElements;
	
	//tenho mais um elemento, crio mais espaço para ele
	atributo=(Attribute*)realloc(this->attrib,sizeof(Attribute)*(this->numAttrib+1));
	if(atributo==nullptr)
		return nullptr;

	//arranjo os ponteiros e contadores
	this->attrib=atributo;
	this->numAttrib++;

	//vou escrever neste atributo
	atributo=atributo+this->numAttrib-1;

	//o tipo de atributo
	atributo->attribType=type;

	//se o stride é zero
	if (this->strideAttrib==0)
	{
		//crio espaço para os dados
		atributo->attribData = (float*)malloc(sizeof(float)*(this->numElements*componentes));
		if (atributo->attribData==nullptr)
		{
			this->numAttrib--;
			return nullptr;
		}
		return atributo;
	}
	
	//se nao for zero, tenho de arranjar o ubberbuffer
	for(i=totalComponentes=0; i<this->numAttrib; i++)
		totalComponentes+=calcComponents(this->attrib[i].attribType);

	//crio o meu novo ubber buffer
	newUbberBuffer = (float*)malloc(sizeof(float)*(this->numElements*totalComponentes));
	if (newUbberBuffer==nullptr)
	{
		this->numAttrib--;
		return nullptr;
	}

	//um calculo auxiliar
	stride=this->strideAttrib/sizeof(float);

	//já nada pode correr mal. Agora copio os dados
	ubberWalker=newUbberBuffer;
	for(i=0; i<this->numElements; i++)
	{
		for(j=0; j<this->numAttrib; j++)
		{
			//só copio os dados se for todos menos o novo atributo
			if ((this->attrib+j)!=atributo)
				memcpy(ubberWalker,this->attrib[j].attribData+stride*i,sizeof(float)*calcComponents(this->attrib[j].attribType));

			//avanço com o walker
			ubberWalker+=calcComponents(this->attrib[j].attribType);
		}
	}

	//óptimo, está tudo copiado, só falta agora apagar o ubber buffer antigo
	if (this->attrib!=nullptr && this->attrib->attribData!=nullptr && this->attrib!=atributo)
		free(this->attrib->attribData);

	//arranjo os novos ponteiros
	ubberWalker=newUbberBuffer;
	for(i=0; i<this->numAttrib; i++)
	{
		this->attrib[i].attribData=ubberWalker;
		ubberWalker+=calcComponents(this->attrib[i].attribType);
	}

	//e agora tenho um novo stride
	this->strideAttrib=(totalComponentes)*sizeof(float);

	//devolvo o ponteiro para os dados finalmente!
	return atributo;
}

float* Mesh::FindAttribData(const MeshAtributeType type)
{
	//verificar parametro e se a mesh está em condições
	if (this->attrib==nullptr || this->numAttrib<=0)
		return nullptr;

	//esta função detecta todos os atributos exceptos os genericos
	if (type<MeshAtributeType::Pos || type>Mesh::TexCoords)
		return nullptr;

	//procuro pelo atributo
	for(int i=0; i<this->numAttrib; i++)
	{
		//se for encontrado
		if (this->attrib[i].attribType==type)
			return this->attrib[i].attribData;
	}

	//não encontrado
	return nullptr;
}

float* Mesh::FindAttribGenericData(const MeshAtributeType type, const int attribCount)
{
	int i,count;

	//verificar parametro e se a mesh está em condições
	if (attribCount<=0 || this->attrib==nullptr || this->numAttrib<=0)
		return nullptr;

	//esta função detecta só os genericos
	if (type<MeshAtributeType::Generic1 || type>MeshAtributeType::Generic4)
		return nullptr;

	//procuro pelo atributo
	for(i=count=0; i<this->numAttrib; i++)
	{
		//se for encontrado
		if (this->attrib[i].attribType==type)
		{
			count++;
			if (count==attribCount)
				return this->attrib[i].attribData;
		}
	}

	//não encontrado
	return nullptr;
}

float* Mesh::FindAttribIndex(const int attribIndex)
{
	//verificar parametro e se a mesh está em condições
	if (this->attrib==nullptr || this->numAttrib<=0)
		return nullptr;

	//esta função detecta todos os atributos exceptos os genericos
	if (attribIndex < 0 || attribIndex >= this->numAttrib)
		return nullptr;

	//basta devolver isto
	return this->attrib[attribIndex].attribData;
}

bool Mesh::CopyAttrib(Mesh * const meshDest, const MeshAtributeType flagSet)
{
	int i,j,stride,totalComp,numAttrib,attribStride[8],attribComponents[8];
	MeshAtributeType attribType[8];
	float *newBuffer,*offset,*attribData[8];

	//verificar toujours le parametros
	if (meshDest==nullptr || (checkMesh(this)==false))
		return false;

	//limpar o destino (é só naquela)
	cleanMesh(meshDest);
	
	//criar espaço para os indices
	if (this->typeIndex==Mesh::Int32)
	{
		meshDest->typeIndex=Mesh::Int32;
		meshDest->pIndex=new unsigned int[this->numIndex];
		if (meshDest->pIndex==nullptr)
			return false;
		memcpy(meshDest->pIndex,this->pIndex,sizeof(unsigned int)*this->numIndex);
	}
	else
	{
		meshDest->typeIndex=Mesh::Int16;
		meshDest->pIndex=new unsigned short[this->numIndex];
		if (meshDest->pIndex==nullptr)
			return false;
		memcpy(meshDest->pIndex,this->pIndex,sizeof(unsigned short)*this->numIndex);
	}

	//o numero de indices
	meshDest->numIndex=this->numIndex;

	//agora crio as coisas para a flag
	numAttrib=createFlagSetData(flagSet,this,attribData,attribStride,attribComponents,attribType);
	if (numAttrib==0)
		return true;

	//posso arranjar esta cena
	meshDest->numAttrib=numAttrib;
	meshDest->numElements=this->numElements;

	//crio espaço
	meshDest->attrib=new Attribute[meshDest->numAttrib];
	if (meshDest->attrib==nullptr)
	{
		cleanMesh(meshDest);
		return false;
	}

	//agora, depende tudo de como tenho a mesh
	if (this->strideAttrib==0)
	{
		//este posso copiar logo
		meshDest->strideAttrib=0;

		//agora para cada atributo
		for(i=0; i<meshDest->numAttrib; i++)
		{
			//arranjo e crio espaco
			meshDest->attrib[i].attribType=attribType[i];
			meshDest->attrib[i].attribData=new float[attribComponents[i]*meshDest->numElements];
			if (meshDest->attrib[i].attribData==nullptr)
			{
				cleanMesh(meshDest);
				return false;
			}

			//copio as cenas
			memcpy(meshDest->attrib[i].attribData,attribData[i],sizeof(float)*attribComponents[i]*meshDest->numElements);
		}

		//já está
		return true;
	}

	//tenho ubber buffer
	for(i=totalComp=0; i<numAttrib; i++)
		totalComp+=attribComponents[i];

	//o stride attrib deste é
	meshDest->strideAttrib=totalComp*sizeof(float);

	//crio ubber buffer
	newBuffer=new float[totalComp*meshDest->numElements];
	if (newBuffer==nullptr)
	{
		cleanMesh(meshDest);
		return false;
	}

	//ponteiro e o stride a usar
	offset=newBuffer;
	stride=this->strideAttrib/sizeof(float);

	//para cada um dos elementos da mesh, copio os valores
	for(i=0; i<meshDest->numElements; i++)
	{
		//para cada atributo
		for(j=0; j<numAttrib; j++)
		{
			memcpy(offset,attribData[j]+i*stride,sizeof(float)*attribComponents[j]);
			offset+=attribComponents[j];
		}
	}

	//só tenho de arranjar os ponteiros e mais nada
	offset=newBuffer;
	for(i=0; i<meshDest->numAttrib; i++)
	{
		meshDest->attrib[i].attribType=attribType[i];
		meshDest->attrib[i].attribData=offset;
		offset+=attribComponents[i];
	}

	//já tá
	return true;
}

int Mesh::GetSize() const
{
	int size;

	//para cada atributo
	size = 0;
	for(int i=0; i<this->numAttrib; i++)
		size += calcComponents(this->attrib[i].attribType);

	//basta calcular o final
	return (size*this->numElements*sizeof(float));
}

void* Mesh::CreateNewIndices(const MeshIndexType indexType, const int numIndices)
{
	void *newBuffer;

	//ignoro isto
	if (numIndices < 0)
		return nullptr;

	//se é para ter 0
	if (numIndices == 0)
	{
		//limpo qualquer outra coisa que tenho antes
		if (this->pIndex != nullptr)
			delete[] this->pIndex;
		this->pIndex = nullptr;
		this->numIndex = 0;

		//mas guardo o tipo
		this->typeIndex = indexType;
		return nullptr;
	}

	//conforme o tipo, crio um novo buffer
	newBuffer = nullptr;
	if (indexType == Mesh::Int16)
		newBuffer = new HorseRadish::hUInt16[numIndices];
	else if (indexType == Mesh::Int32)
		newBuffer = new HorseRadish::hUInt32[numIndices];

	//se correu mal
	if (newBuffer == nullptr)
		return nullptr;

	//limpo qualquer outra coisa que tenho antes
	if (this->pIndex != nullptr)
		delete[] this->pIndex;
	
	//guardo os novos dados
	this->pIndex = newBuffer;
	this->numIndex = numIndices;
	this->typeIndex = indexType;

	//e posso devolver o novo buffer
	return newBuffer;
}

void Mesh::SetVerticesData(const int numAttrib, const int numElements, const int strideAttrib, Geometry::Mesh::Attribute * const attribData)
{
	//guardo os novos dados
	this->numAttrib = numAttrib;
	this->strideAttrib = strideAttrib;
	this->numElements = numElements;
	this->attrib = attribData;
}

void Mesh::SetNewIndices(const MeshIndexType indexType, const int numIndices, void* indicesBuffer)
{
	//guardo os novos dados
	this->pIndex = indicesBuffer;
	this->numIndex = numIndices;
	this->typeIndex = indexType;
}

void Mesh::OpFunc(const MeshAtributeType type, const MeshOperation operation)
{
	float data[4],temp,invNumComp;

	if (checkMesh(this)==false || calcComponents(type)<=0)
		return;

	//posso guardar ja isto
	invNumComp=1.0f/(float)calcComponents(type);

	//para cada elemento
	for(int i=0; i<this->numElements; i++)
	{
		//obtenho os dados
		data[0]=data[1]=data[2]=data[3]=0.0f;
		this->GetData(type,i,data);

		//o que hei-de fazer
		switch(operation)
		{
			case Mesh::OpNormalize:		temp=data[0]*data[0] + data[1]*data[1] + data[2]*data[2] + data[3]*data[3];
										temp=1.0f/sqrt(temp);
										data[0]*=temp;
										data[1]*=temp;
										data[2]*=temp;
										data[3]*=temp;
										break;
			case Mesh::OpCClamp:		data[0]=HorseRadish::Math::fClamp(data[0],0.0f,1.0f);
										data[1]=HorseRadish::Math::fClamp(data[1],0.0f,1.0f);
										data[2]=HorseRadish::Math::fClamp(data[2],0.0f,1.0f);
										data[3]=HorseRadish::Math::fClamp(data[3],0.0f,1.0f);
										break;
			case Mesh::OpAverage:		temp=(data[0] + data[1] + data[2] + data[3])*invNumComp;
										data[0]=temp;
										data[1]=temp;
										data[2]=temp;
										data[3]=temp;
										break;
			case Mesh::OpNegate:		data[0]=-data[0];
										data[1]=-data[1];
										data[2]=-data[2];
										data[3]=-data[3];
										break;
			case Mesh::OpInvert:		data[0]=1.0f/data[0];
										data[1]=1.0f/data[1];
										data[2]=1.0f/data[2];
										data[3]=1.0f/data[3];
										break;
			case Mesh::OpOneMinus:		data[0]=1.0f-data[0];
										data[1]=1.0f-data[1];
										data[2]=1.0f-data[2];
										data[3]=1.0f-data[3];
										break;
									
			}

		//escrevo os dados
		this->SetData(type, i, data);
	}
}

void Mesh::OpMAD(const MeshAtributeType type, const int component, const float op1, const float op2)
{
	Attribute *attrib;
	int i,stride,numComp,compIndex;

	//verificar este parametro
	if (checkMesh(this)==false || component<=0)
		return;

	//tentar encontrar o atributo
	attrib=this->FindAttrib(type);
	if (attrib==nullptr)
		return;

	//o numero de componentes deste atributo
	numComp=calcComponents(attrib->attribType);
	if (component>numComp)
		return;

	//arranjar o stride
	stride=this->strideAttrib/sizeof(float);
	if (stride==0)
		stride=numComp;

	//para todos os pontos
	compIndex=component-1;
	for(i=0; i<this->numElements; i++)
	{
		attrib->attribData[i*stride+compIndex]*=op1;
		attrib->attribData[i*stride+compIndex]+=op2;
	}
}

void Mesh::OpDAM(const MeshAtributeType type, const int component, const float op1, const float op2)
{
	Attribute *attrib;
	int i,stride,numComp,compIndex;

	//verificar este parametro
	if (checkMesh(this)==false || component<=0)
		return;

	//tentar encontrar o atributo
	attrib=this->FindAttrib(type);
	if (attrib==nullptr)
		return;

	//o numero de componentes deste atributo
	numComp=calcComponents(attrib->attribType);
	if (component>numComp)
		return;

	//arranjar o stride
	stride=this->strideAttrib/sizeof(float);
	if (stride==0)
		stride=numComp;

	//para todos os pontos
	compIndex=component-1;
	for(i=0; i<this->numElements; i++)
	{
		attrib->attribData[i*stride+compIndex]+=op1;
		attrib->attribData[i*stride+compIndex]*=op2;
	}
}

void Mesh::OpMatrix(const MeshAtributeType type, const float * const mat4x4)
{
	Attribute *attrib;
	int stride,numComp;
	float x,y,z,w;

	//verificar este parametro
	if (checkMesh(this)==false || mat4x4==nullptr)
		return;

	//tentar encontrar o atributo
	attrib=this->FindAttrib(type);
	if (attrib==nullptr)
		return;

	//o numero de componentes deste atributo
	numComp=calcComponents(attrib->attribType);
	if (numComp<3)
		return;

	//arranjar o stride
	stride=this->strideAttrib/sizeof(float);
	if (stride==0)
		stride=numComp;

	//se tiver 3 componentes
	if (numComp==3)
	{
		//para todos os pontos
		for(int i=0; i<this->numElements; i++)
		{
			x=attrib->attribData[i*stride+0];
			y=attrib->attribData[i*stride+1];
			z=attrib->attribData[i*stride+2];

			attrib->attribData[i*stride+0] = x*mat4x4[0] + y*mat4x4[4] + z*mat4x4[8] + mat4x4[12];
			attrib->attribData[i*stride+1] = x*mat4x4[1] + y*mat4x4[5] + z*mat4x4[9] + mat4x4[13];
			attrib->attribData[i*stride+2] = x*mat4x4[2] + y*mat4x4[6] + z*mat4x4[10]+ mat4x4[14];
		}

		//já tá
		return;
	}

	//tenho 4 compoentes. Para todos os pontos
	for(int i=0; i<this->numElements; i++)
	{
		x=attrib->attribData[i*stride+0];
		y=attrib->attribData[i*stride+1];
		z=attrib->attribData[i*stride+2];
		w=attrib->attribData[i*stride+3];

		attrib->attribData[i*stride+0] = x*mat4x4[0] + y*mat4x4[4] + z*mat4x4[8] + w*mat4x4[12];
		attrib->attribData[i*stride+1] = x*mat4x4[1] + y*mat4x4[5] + z*mat4x4[9] + w*mat4x4[13];
		attrib->attribData[i*stride+2] = x*mat4x4[2] + y*mat4x4[6] + z*mat4x4[10]+ w*mat4x4[14];
		attrib->attribData[i*stride+3] = x*mat4x4[3] + y*mat4x4[7] + z*mat4x4[11]+ w*mat4x4[15];
	}
}

void Mesh::GetData(const MeshAtributeType type, const int element, float *buffer)
{
	Attribute *attrib;
	int stride,numComp;

	//verificar este parametro
	if (checkMesh(this)==false || buffer==nullptr || element<0 || element>=this->numElements)
		return;

	//tentar encontrar o atributo
	attrib=this->FindAttrib(type);
	if (attrib==nullptr)
		return;

	//o numero de componentes deste atributo
	numComp=calcComponents(attrib->attribType);

	//arranjar o stride
	stride=this->strideAttrib/sizeof(float);
	if (stride==0)
		stride=numComp;

	//copio
	for(int i=0; i<numComp; i++)
		buffer[i]=attrib->attribData[element*stride+i];
}

void Mesh::SetData(const MeshAtributeType type, const int element, const float *buffer)
{
	Attribute *attrib;
	int stride,numComp;

	//verificar este parametro
	if (checkMesh(this)==false || buffer==nullptr || element<0 || element>=this->numElements)
		return;

	//tentar encontrar o atributo
	attrib=this->FindAttrib(type);
	if (attrib==nullptr)
		return;

	//o numero de componentes deste atributo
	numComp=calcComponents(attrib->attribType);

	//arranjar o stride
	stride=this->strideAttrib/sizeof(float);
	if (stride==0)
		stride=numComp;

	//copio
	for(int i=0; i<numComp; i++)
		attrib->attribData[element*stride+i]=buffer[i];
}

void Mesh::GetDataGeneric(const MeshAtributeType type, const int attribCount, const int element, float *buffer)
{
	Attribute *attrib;
	int stride,numComp;

	//verificar este parametro
	if (checkMesh(this)==false || buffer==nullptr || element<0 || element>=this->numElements)
		return;

	//tentar encontrar o atributo
	attrib=this->FindAttribGeneric(type,attribCount);
	if (attrib==nullptr)
		return;

	//arranjar o stride
	stride=this->strideAttrib/sizeof(float);
	if (stride==0)
		stride=3;

	//o numero de componentes deste atributo
	numComp=calcComponents(attrib->attribType);

	//copio
	for(int i=0; i<numComp; i++)
		buffer[i]=attrib->attribData[element*stride+i];
}

void Mesh::SetDataGeneric(const MeshAtributeType type, const int attribCount, const int element, float *buffer)
{
	Attribute *attrib;
	int stride,numComp;

	//verificar este parametro
	if (checkMesh(this)==false || buffer==nullptr || element<0 || element>=this->numElements)
		return;

	//tentar encontrar o atributo
	attrib=this->FindAttribGeneric(type,attribCount);
	if (attrib==nullptr)
		return;

	//arranjar o stride
	stride=this->strideAttrib/sizeof(float);
	if (stride==0)
		stride=calcComponents(attrib->attribType);

	//o numero de componentes deste atributo
	numComp=calcComponents(attrib->attribType);

	//copio
	for(int i=0; i<numComp; i++)
		attrib->attribData[element*stride+i]=buffer[i];
}

bool Mesh::AddElements(const int numberNew)
{
	//basta chamar esta função
	return criaMaisVertices(this,this->numElements + numberNew);
}

void Mesh::CopyElementsData(const int vertSource, Mesh * const meshDest, const int vertDest)
{
	//basta chamar esta função
	copyVertexData(this,vertSource,meshDest,vertDest);
}

float* Mesh::SingleBufferPointer()
{
	if (this->numAttrib<=0 || this->strideAttrib==0)
		return nullptr;
	return this->attrib[0].attribData;
}

bool Mesh::ReorderAttrib(const MeshAtributeType *typeAttrib, const int numAttrib, bool deleteExtras)
{
	int curAttrib;

	//verificar parametros
	if (typeAttrib==nullptr || numAttrib<=0 || numAttrib>this->numAttrib)
		return false;

	//se a mesh estiver separada, torna-se mais fácil
	if (this->strideAttrib==0)
	{
		Attribute troca,*attribData,*oldAttrib;

		//para cada atributo que é preciso trocar
		oldAttrib=this->attrib;
		for(curAttrib=0; curAttrib<numAttrib; curAttrib++)
		{
			//encontro o attributo em questão e se não tiver, teste os genericos e ai se não existir, basta passar à frente
			attribData=this->FindAttrib(typeAttrib[curAttrib]);
			if (attribData==nullptr)
			{
				attribData=this->FindAttribGeneric(typeAttrib[curAttrib],1);
				if (attribData==nullptr)
					continue;
			}

			//se forem o mesmo, fixe, não é preciso fazer nada
			if (attribData==oldAttrib)
			{
				oldAttrib++;
				continue;
			}

			//só tenho de trocar entre um e o outro e mais nada
			memcpy(&troca,oldAttrib,sizeof(Attribute));
			memcpy(oldAttrib,attribData,sizeof(Attribute));
			memcpy(attribData,&troca,sizeof(Attribute));
			oldAttrib++;
		}

		//se for para apagar extras, como a mesh está separada, é muito simples
		if ((deleteExtras == true) && (this->numAttrib > numAttrib))
			this->numAttrib = numAttrib;

		//e prontos, já terminei
		return true;
	}

	//chegando aqui os dados estão todos agrupados num único buffer, ou seja,
	//tenho de ir a cada elemento e mudar a sua posição de acordo
	int curElement,auxBSize,curStride;
	float *auxBuffer,*bigBuffer,*walkData;
	struct ATTRIB_OFFSET{
		int numComp,oldOffset;
	}*attribOffsets;

	//o ponteiro para o grande buffer
	bigBuffer=this->attrib[0].attribData;

	//qual o tamanho que cada elemento ocupa e crio um buffer para colocar todos os dados
	auxBSize=this->GetSize()/(this->numElements*sizeof(float));
	auxBuffer=(float*)malloc(sizeof(float)*auxBSize);
	if (auxBuffer==nullptr)
		return false;

	//crio espaço
	attribOffsets=(ATTRIB_OFFSET*)malloc(sizeof(ATTRIB_OFFSET)*numAttrib);
	if (attribOffsets==nullptr)
	{
		free(auxBuffer);
		return false;
	}

	//calculo os ofsetts que vou usar
	for(curAttrib=0; curAttrib<numAttrib; curAttrib++)
	{
		attribOffsets[curAttrib].numComp=calcComponents(typeAttrib[curAttrib]);
		attribOffsets[curAttrib].oldOffset=this->FindAttribData(typeAttrib[curAttrib])-bigBuffer;
	}

	//para cada elemento
	walkData=bigBuffer;
	for(curElement=0; curElement<this->numElements; curElement++,walkData+=auxBSize)
	{
		//copio os dados todos deste elemento
		memcpy(auxBuffer,walkData,sizeof(float)*auxBSize);

		//para cada atributo que é preciso trocar, gravo para o sitio certo
		curStride=0;
		for(curAttrib=0; curAttrib<numAttrib; curAttrib++)
		{
			//se tiver o mesmo offset, nao é preciso copiar
			if (curStride==attribOffsets[curAttrib].oldOffset)
			{
				curStride+=attribOffsets[curAttrib].numComp;
				continue;
			}

			//copio
			memcpy(walkData+curStride,auxBuffer+attribOffsets[curAttrib].oldOffset,sizeof(float)*attribOffsets[curAttrib].numComp);
			curStride+=attribOffsets[curAttrib].numComp;
		}
	}

	//por fim, só tenho de reorganizar os atributos, mas é simples, basta
	curStride=0;
	for(curAttrib=0; curAttrib<numAttrib; curAttrib++)
	{
		//o tipo de atributo e o ponteiro para os dados
		this->attrib[curAttrib].attribType=typeAttrib[curAttrib];
		this->attrib[curAttrib].attribData=bigBuffer+curStride;

		//tenho de incrementar o número de atributos
		curStride+=calcComponents(typeAttrib[curAttrib]);
	}

	//limpar coisas e não é preciso fazer mais nada
	free(attribOffsets);
	free(auxBuffer);
	return true;
}

void Mesh::SingleBuffer()
{
	int i,j,k,tamanhoTotal,components;
	float *ubberBuffer,*walker;

	//se o stride já for diferente de zero, já está tudo num ubber
	if (this->strideAttrib!=0)
		return;

	//calcular o tamanho total
	for(i=tamanhoTotal=0; i<this->numAttrib; i++)
		tamanhoTotal+=calcComponents(this->attrib[i].attribType);

	//se nada houver;
	if (tamanhoTotal==0)
		return;

	//crio novo ubber
	ubberBuffer=new float[tamanhoTotal*this->numElements];
	if (ubberBuffer==nullptr)
		return;

	//a partir daqui já nada corre mal
	this->strideAttrib=tamanhoTotal*sizeof(float);

	//começo a copiar para lá as coisas
	walker=ubberBuffer;
	for(i=0; i<this->numElements; i++)
	{
		//para cada atributo
		for(j=0; j<this->numAttrib; j++)
		{
			//quantos componentes
			components=calcComponents(this->attrib[j].attribType);

			//para cada um
			for(k=0; k<components; k++)
				walker[k]=this->attrib[j].attribData[i*components+k];

			//avanço com o ponteiro
			walker+=components;
		}
	}

	//já ta tudo copiado, posso apagar os antigos e depois rearranjar o ponteiro
	walker=ubberBuffer;
	for(i=0; i<this->numAttrib; i++)
	{
		//apago o antigo
		free(this->attrib[i].attribData);

		//o novo ponteiro
		this->attrib[i].attribData=walker;

		//quantos componentes devo avançar
		walker+=calcComponents(this->attrib[i].attribType);
	}
}

void Mesh::SingleBufferInv()
{
	float *ubberBuffer;

	//verificar parametros
	if (this->numAttrib<=0)
		return;

	//se o stride for diferente de zero, já está tudo dividido
	if (this->strideAttrib==0)
		return;

	//guardo o ubberBufer
	ubberBuffer = this->attrib[0].attribData;

	//passo por todos os atributos
	for(int curAttrib=0; curAttrib < this->numAttrib; curAttrib++)
	{
		float *startBuffer;
		int attribNumComponents;

		//tiro o inicio dos dados deste attrib e quantos componentes fazem parte dele
		startBuffer = this->attrib[curAttrib].attribData;
		attribNumComponents = calcComponents(this->attrib[curAttrib].attribType);

		//crio espaço para guardar os dados para este atributo
		this->attrib[curAttrib].attribData = (float*)malloc(sizeof(float)*this->numElements*attribNumComponents);
		if (this->attrib[curAttrib].attribData == nullptr)
			continue;

		//agora para cada elemento, copio os dados do ubberBuffer
		for(int curElem=0; curElem < this->numElements; curElem++)
			memcpy(this->attrib[curAttrib].attribData+curElem*attribNumComponents, startBuffer+curElem*(this->strideAttrib/sizeof(float)), sizeof(float)*attribNumComponents);
	}

	//posso apagar o ubberBuffer (que já não está a ser usado)
	free(ubberBuffer);

	//para acabar o stride passa a zero
	this->strideAttrib = 0;
}

void Mesh::BoundingBox(float * const minPoint, float * const maxPoint)
{
	__m128 finalMin,finalMax,curVal;
	float bufferWrite[4];
	float *posWalker;
	int stride;

	//parametros
	if ( (checkMesh(this, Mesh::Pos)==false) || this->numAttrib<=0 || minPoint==nullptr || maxPoint==nullptr)
		return;

	//calculo o stride
	stride=this->strideAttrib/sizeof(float);
	if (stride==0)
		stride=3;

	//busco o ponteiro
	posWalker=this->FindAttribData(Mesh::Pos);

	//para o primeiro triangulo
	finalMin = finalMax = _mm_loadu_ps(posWalker);

	//passo por todos os pontos disponiveis (evito o ultimo pq o SSE lê à frente)
	posWalker+=stride;
	for(int i=1; i<this->numElements-1; i++,posWalker+=stride)
	{		
		curVal = _mm_loadu_ps(posWalker);
		finalMin = _mm_min_ps(finalMin, curVal);
		finalMax = _mm_max_ps(finalMax, curVal);
	}

	//basta só tratar do último ponto
	_mm_storeu_ps(bufferWrite,finalMin);
	minPoint[0]=HorseRadish::Math::fMin(bufferWrite[0],posWalker[0]);
	minPoint[1]=HorseRadish::Math::fMin(bufferWrite[1],posWalker[1]);
	minPoint[2]=HorseRadish::Math::fMin(bufferWrite[2],posWalker[2]);
	_mm_storeu_ps(bufferWrite,finalMax);
	maxPoint[0]=HorseRadish::Math::fMax(bufferWrite[0],posWalker[0]);
	maxPoint[1]=HorseRadish::Math::fMax(bufferWrite[1],posWalker[1]);
	maxPoint[2]=HorseRadish::Math::fMax(bufferWrite[2],posWalker[2]);
}

bool Mesh::RayIntersect(const float *rayOrigin, const float *rayDir, float *distHit)
{
	bool wasHit;
	float minDist;
	int stride,curTri;
	HorseRadish::Vector hit,normal;
	const float *walker,*p1,*p2,*p3;

	//parametros
	if ((checkMesh(this,Mesh::Pos)==false) || this->numAttrib<=0 || rayOrigin==nullptr || rayDir==nullptr)
		return false;

	//arranjar ponteiros
	walker=this->FindAttribData(Mesh::Pos);
	if (walker==nullptr)
		return false;

	//tiro o stride
	stride=this->strideAttrib/sizeof(float);
	if (stride==0)
		stride=3;

	//para cada tri
	minDist=HorseRadish::Math::INFINITY;
	wasHit=false;
	for(curTri=0; curTri<this->numIndex; curTri+=3)
	{
		//tiro os três indices
		if (this->typeIndex==Mesh::Int32)
		{
			p1 = walker + ((unsigned int*)this->pIndex)[curTri+0]*stride;
			p2 = walker + ((unsigned int*)this->pIndex)[curTri+1]*stride;
			p3 = walker + ((unsigned int*)this->pIndex)[curTri+2]*stride;
		}
		else
		{
			p1 = walker + ((unsigned short*)this->pIndex)[curTri+0]*stride;
			p2 = walker + ((unsigned short*)this->pIndex)[curTri+1]*stride;
			p3 = walker + ((unsigned short*)this->pIndex)[curTri+2]*stride;
		}

		//calculo a normal
		normal.CalcNormal(p1,p2,p3);
		normal.Normaliza();

		//se a normal não está de frente para o raio, cago no assunto
		if (normal.Dot(rayDir)>0.0f)
			continue;

		//verifico então se intersecta o triangulo
		if (rayTriIntersect(rayOrigin,rayDir,p1,p2,p3,&hit)==false)
			continue;

		//tenho a certeza que acertou e se for menor, aproveito-o
		wasHit=true;
		minDist=HorseRadish::Math::fMin(minDist,hit.GetDist(rayOrigin));
	}

	//se não houve nenhum hit, posso bazar
	if (wasHit==false)
		return false;

	//se querem que eu escreva a distancia
	if (*distHit)
		*distHit=minDist;

	//correu tudo bem
	return true;
}

void Mesh::IndexOptimize()
{
	unsigned short *newIndex;

	//tenho de verificar isto
	if (this->pIndex==nullptr || this->numIndex<=0)
		return;
	if (this->typeIndex!=Mesh::Int32 && this->typeIndex!=Mesh::Int16)
		return;

	//se o numero de indices for muito grande, então nao faço nada (65532 dá conta certa para 21844 triangulos)
	if (this->numIndex>=65532 || this->typeIndex==Mesh::Int16)
		return;

	//posso optimizar
	newIndex=(unsigned short*)malloc(sizeof(unsigned short)*this->numIndex);
	if (newIndex==nullptr)
		return;

	//copio os valores
	for(int i=0; i<this->numIndex; i++)
		newIndex[i]=(unsigned short)(((unsigned int*)this->pIndex)[i]);

	//ultimas coisas e já está
	this->typeIndex=Mesh::Int16;
	free(this->pIndex);
	this->pIndex=newIndex;
}

int Mesh::IndexSize() const
{
	if (this->pIndex==nullptr || this->numIndex<=0)
		return 0;

	if (this->typeIndex==Mesh::Int32)
		return sizeof(unsigned int)*this->numIndex;
	if (this->typeIndex==Mesh::Int16)
		return sizeof(unsigned short)*this->numIndex;
	return 0;
}

void Mesh::InvertFaces()
{
	unsigned short *walkerS,tempS;
	unsigned int *walkerI,tempI;

	if (checkMesh(this)==false)
		return;

	if (this->typeIndex==Mesh::Int16)
	{
		walkerS=(unsigned short*)this->pIndex;
		for(int i=0; i<this->numIndex; i+=3,walkerS+=3)
		{
			tempS=walkerS[0];
			walkerS[0]=walkerS[2];
			walkerS[2]=tempS;
		}
		return;
	}

	walkerI=(unsigned int*)this->pIndex;
	for(int i=0; i<this->numIndex; i+=3,walkerI+=3)
	{
		tempI=walkerI[0];
		walkerI[0]=walkerI[2];
		walkerI[2]=tempI;
	}
}

float Mesh::GetIndexCacheRatio(const unsigned int numCacheEntries)
{
	int *cache,hits,index;
	bool cacheHit;

	//verificar este parametro
	if (checkMesh(this)==false)
		return -1.0f;

	//tenho de verificar isto
	if (numCacheEntries == 0)
		return 0.0f;
	if (numCacheEntries >= this->numIndex)
		return 1.0f;

	//preciso da memória
	cache = (int*)malloc(sizeof(int)*numCacheEntries);
	if (cache == nullptr)
		return 0.0f;
	
	//limpo a cache
	for(int i=0; i<numCacheEntries; i++)
		cache[i]=-1;

	//passo por todos os indices
	hits = 0;
	for(int i=0; i<this->numIndex; i++)
	{
		//o indice em questão
		index=getIndex(this,i);

		//procuro se tenho este indice na cache
		cacheHit = false;
		for(int j=0; j<numCacheEntries; j++)
		{
			if (cache[j] == index)
			{
				cacheHit = true;
				break;
			}
		}

		//se acertei
		if (cacheHit == true)
		{
			hits++;
			continue;
		}

		//como não tenho o vertice na cache, tiro o mais antigo, movo todos os outros e insiro este novo indice
		for(int j=(numCacheEntries-1); j>0; j--)
			cache[j] = cache[j-1];
		cache[0] = index;
	}

	//já não preciso da cache
	free(cache);
	cache = nullptr;

	//basta devolver isto
	return (((float)hits) / ((float)this->numIndex));
}

void Mesh::CenterMass(const float newX, const float newY, const float newZ)
{
	float minP[3],maxP[3],distancia[3];
	Attribute *attrib;
	int i,stride;

	//verificar este parametro
	if (checkMesh(this, Mesh::Pos)==false)
		return;

	//qual a bounding box
	this->BoundingBox(minP,maxP);

	//calcular para o paralelipipedo os lados
	distancia[0]=maxP[0]-minP[0];
	distancia[1]=maxP[1]-minP[1];
	distancia[2]=maxP[2]-minP[2];
	distancia[0]=HorseRadish::Math::fAbs(distancia[0])*0.5f;
	distancia[1]=HorseRadish::Math::fAbs(distancia[1])*0.5f;
	distancia[2]=HorseRadish::Math::fAbs(distancia[2])*0.5f;

	//calcular a distancia para fazer a translacao para o ponto indicado. Entre () é o centro de massa
	distancia[0]=newX-(minP[0]+distancia[0]);
	distancia[1]=newY-(minP[1]+distancia[1]);
	distancia[2]=newZ-(minP[2]+distancia[2]);

	//fazer a translacao para centrar na posicao nova indicada
	attrib=this->FindAttrib(Mesh::Pos);
	stride=this->strideAttrib/sizeof(float);
	if (stride==0)
		stride=3;
	for(i=0; i<this->numElements; i++)
	{
		attrib->attribData[i*stride+0]+=distancia[0];
		attrib->attribData[i*stride+1]+=distancia[1];
		attrib->attribData[i*stride+2]+=distancia[2];
	}
}

void Mesh::MaxBBox(const float maxDist)
{
	float minP[3],maxP[3],distancia[3],distanciaMax,scaleVal;
	Attribute *attrib;
	int i,stride;

	//boa mesh e maxDist tem de ser sempre maior do que 0
	if (checkMesh(this,Mesh::Pos)==false || maxDist<=0.0f)
		return;

	//qual a bounding box
	this->BoundingBox(minP,maxP);

	//calcular para o paralelipipedo os lados
	distancia[0]=maxP[0]-minP[0];
	distancia[1]=maxP[1]-minP[1];
	distancia[2]=maxP[2]-minP[2];
	distancia[0]=HorseRadish::Math::fAbs(distancia[0]);
	distancia[1]=HorseRadish::Math::fAbs(distancia[1]);
	distancia[2]=HorseRadish::Math::fAbs(distancia[2]);

	//a maior das distancias
	distanciaMax=HorseRadish::Math::fMax(distancia[0],distancia[1]);
	distanciaMax=HorseRadish::Math::fMax(distanciaMax,distancia[2]);

	//achar o valor de scale com a distancia maxima
	scaleVal=maxDist/distanciaMax;

	//fazer o scale correcto para ficar com o valor desejado mas só no atributo posição
	attrib=this->FindAttrib(Mesh::Pos);
	stride=this->strideAttrib/sizeof(float);
	if (stride==0)
		stride=3;
	for(i=0; i<this->numElements; i++)
	{
		attrib->attribData[i*stride+0]*=scaleVal;
		attrib->attribData[i*stride+1]*=scaleVal;
		attrib->attribData[i*stride+2]*=scaleVal;
	}
}

void Mesh::Confine(const float newX, const float newY, const float newZ, const float maxDist)
{
	float *walker,minP[3],maxP[3],distancia[3],distanciaMax,scaleVal;
	int i,stride;

	//verificar este parametro
	if (checkMesh(this, Mesh::Pos)==false || maxDist<=0.0f)
		return;

	//qual a bounding box
	this->BoundingBox(minP,maxP);

	//calcular para o paralelipipedo os lados
	distancia[0]=maxP[0]-minP[0];
	distancia[1]=maxP[1]-minP[1];
	distancia[2]=maxP[2]-minP[2];
	distancia[0]=HorseRadish::Math::fAbs(distancia[0]);
	distancia[1]=HorseRadish::Math::fAbs(distancia[1]);
	distancia[2]=HorseRadish::Math::fAbs(distancia[2]);

	//a maior das distancias
	distanciaMax=HorseRadish::Math::fMax(distancia[0],distancia[1]);
	distanciaMax=HorseRadish::Math::fMax(distanciaMax,distancia[2]);

	//achar o valor de scale com a distancia maxima
	scaleVal=maxDist/distanciaMax;

	//quero metade dos lados
	distancia[0]*=0.5f;
	distancia[1]*=0.5f;
	distancia[2]*=0.5f;

	//calcular a distancia para fazer a translacao para o ponto indicado. Entre () é o centro de massa
	distancia[0]=newX-(minP[0]+distancia[0]);
	distancia[1]=newY-(minP[1]+distancia[1]);
	distancia[2]=newZ-(minP[2]+distancia[2]);

	//fazer a translacao para centrar na posicao nova indicada e o scale para ficar dentro dos limites
	walker=this->FindAttribData(Mesh::Pos);
	stride=this->strideAttrib/sizeof(float);
	if (stride==0)
		stride=3;
	for(i=0; i<this->numElements; i++,walker+=stride)
	{
		walker[0]=(walker[0]+distancia[0])*scaleVal;
		walker[1]=(walker[1]+distancia[1])*scaleVal;
		walker[2]=(walker[2]+distancia[2])*scaleVal;
	}
}

bool Mesh::Merge(const Mesh *merge)
{
	int i,current,oldVert,oldIndex;

	//pursopuesto
	if (merge==nullptr || checkMesh(this)==false || checkMesh(merge)==false)
		return 0;

	//têm de ter o mesmo numero
	if (merge->numAttrib!=this->numAttrib)
		return false;

	//e os mesmos tipos de atributos (comparo sempre com o primeiro)
	for(i=0; i<merge->numAttrib; i++)
	{
		//se forem diferentes, à chatice! toca a sair cheio de raiva
		if (merge->attrib[i].attribType!=this->attrib[i].attribType)
			return false;
	}

	//guardo isto
	oldVert=this->numElements;
	oldIndex=this->numIndex;

	//ok, chegando aqui posso copiar à vontade tudo de um lugar para o outro
	if (criaMaisVertices(this,this->numElements+merge->numElements)==false || criaMaisIndices(this,this->numIndex+merge->numIndex)==false)
	{
		this->numElements=oldVert;
		this->numIndex=oldIndex;
		return false;
	}

	//já tá tudo, basta copiar agora os vertices
	for(i=0,current=oldVert; i<merge->numElements; i++,current++)
		copyVertexData(merge,i,this,current);

	//e agora os indices
	for(i=0,current=oldIndex; i<merge->numIndex; i+=3,current+=3)
	{
		writeIndex(this,current+0,getIndex(merge,i+0)+oldVert);
		writeIndex(this,current+1,getIndex(merge,i+1)+oldVert);
		writeIndex(this,current+2,getIndex(merge,i+2)+oldVert);
	}

	//e já está
	return true;
}

int Mesh::RemoveDuplicate(const MeshAtributeType flagSet)
{
	float *attribData[8];
	int i,j,k,l,numAttrib,attribStride[8],attribComponents[8],vertRemoved;

	if (checkMesh(this)==false)
		return 0;

	//vejo todos os atributos que tenho
	numAttrib=createFlagSetData(flagSet,this,attribData,attribStride,attribComponents,nullptr);
	if (numAttrib<=0)
		return 0;

	//nao retirei nenhum
	vertRemoved=0;

	//primeiro nivel
	for(i=0; i<this->numElements; i++)
	{
		//segundo nivel
		for(j=i+1; j<this->numElements; j++)
		{
			//passo por todos atributos
			for(k=0; k<numAttrib; k++)
			{
				//e agora por todos os componentes dos atributos
				for(l=0; l<attribComponents[k]; l++)
				{
					if (attribData[k][i*attribStride[k]+l] != attribData[k][j*attribStride[k]+l])
						break;
				}

				if (l<attribComponents[k])
					break;
			}

			//se nao cheguei ao fim, este fica
			if (k<numAttrib)
				continue;

			//vou retirar um
			vertRemoved++;

			//agora todos os triangulos que tenham j têm de ir para i
			for (k=0; k<this->numIndex; k++)
			{
				if (getIndex(this, k) == j)
					writeIndex(this, k, i);
			}
		}
	}

	//digo quantos tirei
	return vertRemoved;
}

void Mesh::EliminateDegenerateTri()
{
	int i;
	unsigned int v1,v2,v3;

	//verificar parametros
	if (checkMesh(this)==false)
		return;

	//vamos lá
	i=0;
	while(i<this->numIndex)
	{
		//vou buscar os indices
		if (this->typeIndex == Mesh::Int32)
		{
			v1=((unsigned int*)this->pIndex)[i+0];
			v2=((unsigned int*)this->pIndex)[i+1];
			v3=((unsigned int*)this->pIndex)[i+2];
		}
		else
		{
			v1=((unsigned short*)this->pIndex)[i+0];
			v2=((unsigned short*)this->pIndex)[i+1];
			v3=((unsigned short*)this->pIndex)[i+2];
		}

		//se tiverem dois os mais indices em comum, tem area 0 de certeza
		if ( (v1==v2) || (v1==v3) || (v2==v3) || (zeroAreaTri(this,v1,v2,v3)==true) )
			{
			//se este é o ultimo, elimina e sai logo
			if (this->numIndex==3)
			{
				this->numIndex=0;
				return;
			}

			//copia o que está em ultimo para a posicao deste e nao avança
			if (this->typeIndex==Mesh::Int32)
			{
				memcpy(((unsigned int*)this->pIndex+i),((unsigned int*)this->pIndex)+this->numIndex-3,sizeof(unsigned int)*3);
				this->numIndex-=3;
			}
			else
			{
				memcpy(((unsigned short*)this->pIndex+i),((unsigned short*)this->pIndex)+this->numIndex-3,sizeof(unsigned short)*3);
				this->numIndex-=3;
			}

			continue;
		}

		//avança
		i+=3;
	}
}

void Mesh::Ortho(const int flags)
{
	//só normais
	if (flags & MESH_ORTHO_CREATE_NORMALS)
	{
		//isto é estupido
		if (flags & MESH_ORTHO_KEEP_NORMALS)
			return;

		//mandar seguir
		createNormals(this, flags);

		//mais nada
		return;
	}

	//espaço orthonormal completo com binormais e tangents
	if (flags & MESH_ORTHO_CREATE_FULL)
	{
		//posso mandar ir normalmente
		createFull(this, flags);
		return;
	}

	//espaço completo com tangent4 (usar w)
	if (flags & MESH_ORTHO_CREATE_TANGENT4)
	{
		//posso mandar ir normalmente
		createTangent4(this, flags);
		return;
	}
}

bool Mesh::Shade(const int lightModel, const float *lightPos, const float * lightDiffuse, const float *lightSpecular, const float *camPos)
{
	HorseRadish::Vector finalColor,lightVec,viewVec,diffuse,specular,auxVec;
	float *posWalker,*normalWalker,*colorWalker;
	int i,stride3,stride1;

	//boa mesh e maxDist tem de ser sempre maior do que 0
	if (checkMesh(this, (Mesh::MeshAtributeType)(Mesh::Pos | Mesh::Normal))==false || lightPos==nullptr)
		return 0;

	//por agora tenho de ter ou um ou o outro
	if ( (lightModel & MESH_SHADE_PHONG)==false && (lightModel & MESH_SHADE_BLINN)==false)
		return 0;

	//preciso de pelos menos uma coisa pra fazer a iluminação
	if (lightDiffuse==nullptr && (lightSpecular==nullptr || camPos==nullptr))
		return 0;

	//obter isto dá muito jeito
	posWalker=this->FindAttribData(Mesh::Pos);
	normalWalker=this->FindAttribData(Mesh::Normal);
	colorWalker=nullptr;
	if (lightModel & MESH_SHADE_COLOR_FLOAT)
		colorWalker=this->FindAttribGenericData(Mesh::Generic3, 1);
	else if (lightModel & MESH_SHADE_COLOR_UBYTE)
		colorWalker=this->FindAttribGenericData(Mesh::Generic1, 1);
	
	//tenho de ter isto tudo, logo dá jeito verificar
	if (posWalker==nullptr || normalWalker==nullptr || colorWalker==nullptr)
		return 0;

	//os strides
	stride1=1;
	stride3=3;
	if (this->strideAttrib!=0)
	{
		stride1=stride3=this->strideAttrib/sizeof(float);
	}

	//para todos os elementos
	for(i=0; i<this->numElements; i++,posWalker+=stride3,normalWalker+=stride3)
	{
		//qual modelo devo usar pra calcular a cor
		finalColor.Set(0.0f,0.0f,0.0f);
		
		//o lightVec
		lightVec.Set(lightPos);
		lightVec-=posWalker;
		lightVec.Normaliza();

		//a parte difusa
		diffuse.Set(0.0f,0.0f,0.0f);
		if (lightDiffuse)
		{
			diffuse.Set(lightDiffuse);
			diffuse*=HorseRadish::Math::fMax(lightVec.Dot(normalWalker),0.0f);
		}

		//a parte especular
		specular.Set(0.0f,0.0f,0.0f);
		if (lightModel & MESH_SHADE_BLINN)
			calcSpec(MESH_SHADE_BLINN,specular,camPos,lightPos,lightSpecular,posWalker,normalWalker);
		else if (lightModel & MESH_SHADE_PHONG)
			calcSpec(MESH_SHADE_PHONG,specular,camPos,lightPos,lightSpecular,posWalker,normalWalker);

		//se for para somar, vou antes ler a cor antiga, senão fica a zero!
		finalColor.Set(0.0f,0.0f,0.0f);
		if (lightModel & MESH_SHADE_ADD_LIGHT)
		{
			if (lightModel & MESH_SHADE_COLOR_FLOAT)
			{
				finalColor.Set(colorWalker);
			}
			else
			{
				finalColor.x=((float)((unsigned char*)colorWalker)[0])*0.003921568627f;
				finalColor.y=((float)((unsigned char*)colorWalker)[1])*0.003921568627f;
				finalColor.z=((float)((unsigned char*)colorWalker)[2])*0.003921568627f;
			}
		}

		//a cor final fica assim
		finalColor+=diffuse;
		finalColor+=specular;

		//clampar sempre que vou pra ubyte
		if (lightModel & MESH_SHADE_COLOR_UBYTE)
			finalColor.Clamp(0.0f,1.0);
		
		//escrever e avancar com a cor
		if (lightModel & MESH_SHADE_COLOR_FLOAT)
		{
			finalColor.Write(colorWalker);
			colorWalker+=stride3;
		}
		else
		{
			finalColor*=255.0f;
			((unsigned char*)colorWalker)[0]=(unsigned char)finalColor.x;
			((unsigned char*)colorWalker)[1]=(unsigned char)finalColor.y;
			((unsigned char*)colorWalker)[2]=(unsigned char)finalColor.z;
			colorWalker+=stride1;
		}
	}

	//já está
	return true;
}

void Mesh::ReorderTriIndex()
{
	Mesh *meshNova;
	int *newVertIndex;
	unsigned int oldIndex, newIndex;
	std::vector<unsigned int> indices;

	//verificar parametros
	if (checkMesh(this)==false)
		return;

	//tento criar uma mesh auxiliar
	meshNova = this->Clone();
	if (meshNova == nullptr)
		return;

	//crio a lista com os indices
	for(int i=0; i<this->numIndex; i++)
		indices.push_back(getIndex(this, i));

	//mando optimizar os indices
	optimize_vertex_cache_order(indices, 16);

	//guardo os novos indices
	for(int i=0; i<this->numIndex; i++)
		writeIndex(this, i, indices[i]);

	//preciso disto para guardar as novas posicoes dos vertices
	newVertIndex = (int*)malloc(sizeof(int)*this->numElements);
	if (newVertIndex == nullptr)
	{
		delete meshNova;
		return;
	}

	//limpo a lista
	for(int i=0; i<this->numElements; i++)
		newVertIndex[i] = -1;

	//agora reordeno os valores dos vertices para ficar melhor na cache dos vertices
	//volto a passar por todos os indices
	newIndex = 0;
	for(int i=0; i<this->numIndex; i++)
	{
		//se isto já tiver uma entrada, uso-a
		if (newVertIndex[indices[i]] != -1)
		{
			writeIndex(this, i, newVertIndex[indices[i]]);
			continue;
		}

		//verifico qual o indice que tenho de usar
		oldIndex = indices[i];

		//agora simplesmente copio os valores da meshAntiga para a nova mesh
		copyVertexData(meshNova, oldIndex, this, newIndex);

		//guardo o novo indice na lista e no proprio indice
		newVertIndex[indices[i]] = newIndex;
		writeIndex(this, i, newIndex);

		//o próximo indice
		newIndex++;
	}

	//posso apagar os indices e a mesh auxiliar
	free(newVertIndex);
	delete meshNova;
}

bool Mesh::TexGen(const int texGenType, const float *values)
{
	float *posWalker,*normalWalker,*texWalker;
	int i,stride3,stride2;

	//boa mesh
	if (checkMesh(this)==false)
		return false;

	//obter isto dá muito jeito
	posWalker=this->FindAttribData(Mesh::Pos);
	texWalker=this->FindAttribData(Mesh::TexCoords);
	
	//tenho de ter isto tudo, logo dá jeito verificar
	if (posWalker==nullptr || texWalker==nullptr)
		return false;

	//os strides
	stride2=2;
	stride3=3;
	if (this->strideAttrib!=0)
	{
		stride2=stride3=this->strideAttrib/sizeof(float);
	}

	//agora de acordo com o que quiser fazer
	if (texGenType == MESH_TEXGEN_OBJECT_LINEAR)
	{
		//para todos os elementos
		for(i=0; i<this->numElements; i++,posWalker+=stride3,texWalker+=stride2)
		{
			//posso copiar isto
			texWalker[0]=posWalker[0];
			texWalker[1]=posWalker[1];

			//se tiver valores para usar
			if (values)
			{
				texWalker[0]=(texWalker[0]*values[0])/values[2];
				texWalker[1]=(texWalker[1]*values[1])/values[2];
			}
		}

		//já tá
		return true;
	}

	//agora de acordo com o que quiser fazer
	if (texGenType == MESH_TEXGEN_SPHERE_MAP)
	{
		//vars auxiliares
		HorseRadish::Vector centro,fVec,uVec;
		float m,minP[3],maxP[3];

		//qual a bounding box
		this->BoundingBox(minP,maxP);

		//calcular o centro de massa do objecto
		centro.x=minP[0]+HorseRadish::Math::fAbs(maxP[0]-minP[0])*0.5f;
		centro.y=minP[0]+HorseRadish::Math::fAbs(maxP[1]-minP[1])*0.5f;
		centro.z=minP[0]+HorseRadish::Math::fAbs(maxP[2]-minP[2])*0.5f;

		//para todos os elementos
		for(i=0; i<this->numElements; i++,posWalker+=stride3,texWalker+=stride2)
		{
			uVec.x=centro.x-posWalker[0];
			uVec.y=centro.y-posWalker[1];
			uVec.z=centro.z-posWalker[2];

			fVec=uVec;
			uVec*=2.0f*uVec.Dot(uVec);
			fVec-=uVec;

			m=2.0f*sqrt(fVec.x*fVec.x + fVec.y*fVec.y + (fVec.z+1.0)*(fVec.z+1.0));
			m=1.0f/m;

			texWalker[0]=fVec.x*m+0.5f;
			texWalker[1]=fVec.y*m+0.5f;
		}

		//já tá
		return true;
	}

	//agora de acordo com o que quiser fazer
	if (texGenType == MESH_TEXGEN_NORMAL_MAP)
	{
		//vars auxiliares
		HorseRadish::Vector centro,fVec,uVec;
		float *normalWalker, invM, minP[3],maxP[3];

		//preciso disto
		normalWalker=this->FindAttribData(Mesh::Normal);
		if (normalWalker == nullptr)
			return false;

		//qual a bounding box
		this->BoundingBox(minP,maxP);

		//calcular o centro de massa do objecto
		centro.x=minP[0]+HorseRadish::Math::fAbs(maxP[0]-minP[0])*0.5f;
		centro.y=minP[0]+HorseRadish::Math::fAbs(maxP[1]-minP[1])*0.5f;
		centro.z=minP[0]+HorseRadish::Math::fAbs(maxP[2]-minP[2])*0.5f;

		//para todos os elementos
		for(i=0; i<this->numElements; i++,normalWalker+=stride3,texWalker+=stride2)
		{
			uVec.x=normalWalker[0];
			uVec.y=normalWalker[1];
			uVec.z=normalWalker[2];

			fVec=uVec;
			uVec*=2.0f*uVec.Dot(uVec);
			fVec-=uVec;

			invM=2.0f*sqrt(fVec.x*fVec.x + fVec.y*fVec.y + (fVec.z+1.0)*(fVec.z+1.0));
			invM=1.0f/invM;

			texWalker[0]=fVec.x*invM+0.5f;
			texWalker[1]=fVec.y*invM+0.5f;
		}

		//já tá
		return true;
	}

	//deu barraca, pq nao reconheci o que estava em texGenType
	return false;
}

bool Mesh::Deserialize(Streams::StreamReader * const streamReader)
{
	return false;
}

bool Mesh::DeserializeHRF(Streams::StreamReader * const streamReader)
{
	hInt32 typeIndexTemp;

	//a primeira coisa é limpar tudo
	cleanMesh(this);

	//escrevo quantos vertices e indices tenho, assim como outros tipos de dados
	streamReader->ReadInt32(this->numElements);
	streamReader->ReadInt32(this->numIndex);
	streamReader->ReadInt32(this->numAttrib);
	streamReader->ReadInt32(this->strideAttrib);
	streamReader->ReadInt32(typeIndexTemp);
	this->typeIndex = (Mesh::MeshIndexType)typeIndexTemp;

	//verificar isto
	if ((this->numElements <= 0) || (this->numIndex <= 0) || (this->numAttrib <= 0))
		return false;
	if ((this->typeIndex != Mesh::Int32) && (this->typeIndex != Mesh::Int16))
		return false;

	//tenho de criar espaço para os atributos
	this->attrib = new Mesh::Attribute[this->numAttrib];
	if (this->attrib == nullptr)
		return false;

	//conforme o tipo indicado
	switch(0xfaa5)
	{
		case 0xfaa1:
			return false;
			/*if (this->numAttrib != 5)
				return false;
			this->attrib[0].attribType = Mesh::Pos;
			this->attrib[1].attribType = Mesh::TexCoords;
			this->attrib[2].attribType = Mesh::Generic1;
			this->attrib[3].attribType = Mesh::Generic1;
			this->attrib[4].attribType = Mesh::Generic1;*/
			break;
		case 0xfaa2:
			if (this->numAttrib != 8)
				return false;
			this->attrib[0].attribType = Mesh::Pos;
			this->attrib[1].attribType = Mesh::TexCoords;
			this->attrib[2].attribType = Mesh::Generic1;
			this->attrib[3].attribType = Mesh::Generic1;
			this->attrib[4].attribType = Mesh::Generic1;
			this->attrib[5].attribType = Mesh::Normal;
			this->attrib[6].attribType = Mesh::Tangent4;
			this->attrib[7].attribType = Mesh::Generic1;
			break;
		case 0xfaa3:
			return false;
			/*if (this->numAttrib != 4)
				return false;
			this->attrib[0].attribType = Mesh::POS;
			this->attrib[1].attribType = Mesh::TEXCOORDS;
			this->attrib[2].attribType = Mesh::GENERIC2;
			this->attrib[3].attribType = Mesh::GENERIC1;*/
			break;
		case 0xfaa4:
			if (this->numAttrib != 7)
				return false;
			this->attrib[0].attribType = Mesh::Pos;
			this->attrib[1].attribType = Mesh::TexCoords;
			this->attrib[2].attribType = Mesh::Generic2;
			this->attrib[3].attribType = Mesh::Generic1;
			this->attrib[4].attribType = Mesh::Normal;
			this->attrib[5].attribType = Mesh::Tangent4;
			this->attrib[6].attribType = Mesh::Generic1;
			break;
		case 0xfaa5:
			if (this->numAttrib != 5)
				return false;
			this->attrib[0].attribType = Mesh::Pos;
			this->attrib[1].attribType = Mesh::TexCoords;
			this->attrib[2].attribType = Mesh::Normal;
			this->attrib[3].attribType = Mesh::Tangent4;
			this->attrib[4].attribType = Mesh::Generic4;
			break;
		default:
			return false;
	}

	//crio espaço no primeiro atributo para levar com tudo
	this->attrib[0].attribData = (float*)malloc(this->GetSize());
	if (this->attrib[0].attribData == nullptr)
		return false;

	//leio o que tenho a ler e arranjo os ponteiros dos dados dos atributos
	streamReader->Read(this->attrib[0].attribData, this->GetSize());
	for(int i=1; i<this->numAttrib; i++)
		{
		//conforme o tipo de atributo
		switch(this->attrib[i-1].attribType)
		{
			case Mesh::Pos:
			case Mesh::Normal:
			case Mesh::Tangent:
			case Mesh::Binormal:
			case Mesh::Generic3:
									this->attrib[i].attribData = this->attrib[i-1].attribData + 3;
									break;
			case Mesh::TexCoords:
			case Mesh::Generic2:
									this->attrib[i].attribData = this->attrib[i-1].attribData + 2;
									break;
			case Mesh::Generic1:
									this->attrib[i].attribData = this->attrib[i-1].attribData + 1;
									break;
			case Mesh::Tangent4:
			case Mesh::Generic4:
									this->attrib[i].attribData = this->attrib[i-1].attribData + 4;
									break;
			default:
				return false;
			}
		}

	//crio espaço para ler os indices e leio-os
	if (this->typeIndex == Mesh::Int32)
		this->pIndex = malloc(sizeof(unsigned int) * this->numIndex);
	else
		this->pIndex = malloc(sizeof(unsigned short) * this->numIndex);

	//se fiquei sem espaço, grande barraca
	if (this->pIndex == nullptr)
		return false;

	//posso ler os indices do ficheiro
	streamReader->Read(this->pIndex, this->IndexSize());
}

bool Mesh::Serialize(Streams::StreamWriter * const streamWriter)
{
	int dataSize, indexSize;

	//escrevo quantos vertices e indices tenho, assim como outros tipos de dados
	streamWriter->WriteInt32(this->numElements);
	streamWriter->WriteInt32(this->numIndex);
	streamWriter->WriteInt32(this->numAttrib);
	streamWriter->WriteInt32(this->strideAttrib);
	streamWriter->WriteInt32(this->typeIndex);

	//para cada atributo, escrevo o tipo dele
	for(int i=0; i<this->numAttrib; i++)
		streamWriter->WriteInt32(this->attrib[i].attribType);

	//dá jeito ter estes valores
	dataSize = this->GetSize();
	indexSize = this->IndexSize();

	//posso escrever todos os dados: dos vértices e dos indices
	streamWriter->WriteInt32(dataSize);
	streamWriter->Write(this->SingleBufferPointer(), dataSize);
	streamWriter->WriteInt32(indexSize);
	streamWriter->Write(this->pIndex, indexSize);

	//correu tudo bem
	return true;
}

bool Mesh::SerializeHRF(Streams::StreamWriter * const streamWriter)
{
	int dataSize, indexSize;

	//tenho de ter esta config
	if ((this->numAttrib != 5) || (this->attrib[0].attribType != Mesh::Pos) || (this->attrib[1].attribType != Mesh::TexCoords) || (this->attrib[2].attribType != Mesh::Normal) || (this->attrib[3].attribType != Mesh::Tangent4) || (this->attrib[4].attribType != Mesh::Generic4))
		return false;
	if (this->SingleBufferPointer() == nullptr)
		return false;

	//escrevo quantos vertices e indices tenho, assim como outros tipos de dados
	streamWriter->WriteInt32(this->numElements);
	streamWriter->WriteInt32(this->numIndex);
	streamWriter->WriteInt32(this->numAttrib);
	streamWriter->WriteInt32(this->strideAttrib);
	streamWriter->WriteInt32(this->typeIndex);

	//certifico-me que tenho tudo o que é preciso
	/*if (this->FindAttribData(Mesh::Pos) == nullptr)
		this->NewAttrib(Mesh::Pos, this->numElements);
	if (this->FindAttribData(Mesh::TexCoords) == nullptr)
		this->NewAttrib(Mesh::TexCoords, this->numElements);
	if (this->FindAttribData(Mesh::Normal) == nullptr)
		this->NewAttrib(Mesh::Normal, this->numElements);
	if (this->FindAttribData(Mesh::Tangent4) == nullptr)
		this->NewAttrib(Mesh::Tangent4, this->numElements);

	//mando calcular algumas coisas
	this->Ortho(MESH_ORTHO_CREATE_TANGENT4);

	//ordeno tudo como deve de ser
	MeshAtributeType treta[] = {Mesh::Pos, Mesh::TexCoords, Mesh::Normal, Mesh::Tangent4};
	this->ReorderAttrib(treta, 4, true);

	//falta um genérico
	this->NewAttrib(Mesh::Generic4, this->numElements);

	//tudo para um único buffer
	this->SingleBuffer();*/

	//basta escrever os dados
	streamWriter->Write(this->SingleBufferPointer(), this->GetSize());
	streamWriter->Write(this->pIndex, this->IndexSize());

	//correu tudo bem
	return true;
}

}//namespace Geometry
}//namespace HorseRadish