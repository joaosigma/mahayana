#include "common\Platform.hpp"
#include "common\Mesh.hpp"
#include "common\MeshFactory.hpp"

#include <windows.h>
#include <stdlib.h>

using namespace HorseRadish;

#define SMD_FLAG_TEX	(1<<0)
#define SMD_FLAG_NRM	(1<<1)
#define SMD_FLAG_TG4	(1<<2)

static
bool carregaSMD(const void * const fileData, const unsigned int fileSize, Geometry::Model * const modelo)
{
	Geometry::Model::MeshData *curModel;
	unsigned char *fwalker;
	Geometry::Mesh *curMesh;
	float *walker;
	int numMeshes, curIndex,numElements,flags,trash, numIndices;

	//verificar os parametros
	if (fileData==nullptr || fileSize<=8 || modelo==nullptr)
		return false;

	//leio a sig do ficheiro
	if (*((unsigned int*)fileData)!=0x543fde99)
		return false;

	//gravo o ponteiro e passo à frente da sig e do checksum
	fwalker=((unsigned char*)fileData)+sizeof(unsigned int)+sizeof(unsigned int);

	//****
	//leio o numero de objectos dentro do ficheiro
	numMeshes = *((unsigned short*)fwalker);
	if (numMeshes <= 0)
		return false;

	//avanço o número de objectos
	fwalker+=sizeof(short);

	//para cada mesh que o ficheiro tem
	for(curIndex=0; curIndex<numMeshes; curIndex++)
	{
		//se estou no fim do ficheiro, algo correu mal
		if ((fwalker-fileData)>=fileSize)
			return false;

		//crio uma nova mesh
		modelo->arrayMesh.Add();

		//qual o modelo e mesh que estou a usar
		curModel = &modelo->arrayMesh[curIndex];
		curMesh = &curModel->mesh;

		//leio o nome do objecto
		strcpy(curModel->meshName,(const char*)fwalker);
		fwalker+=(strlen((const char*)fwalker)+1);

		//leio o nome do material
		strcpy(curModel->materialName,(const char*)fwalker);
		fwalker+=(strlen((const char*)fwalker)+1);

		//leio o numero de elementos desta mesh
		numElements=*((unsigned int*)fwalker);
		fwalker+=sizeof(unsigned int);
		if (numElements<=0)
			continue; 

		//e a flags
		flags=*((unsigned int*)fwalker);
		fwalker+=sizeof(unsigned int);

		//crio a mesh com tudo o que é preciso
		curMesh->NewAttrib(Geometry::Mesh::Pos,numElements);
		if (flags & SMD_FLAG_NRM)
			curMesh->NewAttrib(Geometry::Mesh::Normal,numElements);
		if (flags & SMD_FLAG_TG4)
			curMesh->NewAttrib(Geometry::Mesh::Tangent4,numElements);
		if (flags & SMD_FLAG_TEX)
			curMesh->NewAttrib(Geometry::Mesh::TexCoords,numElements);

		//****
		//leio os vertices
		walker=curMesh->FindAttribData(Geometry::Mesh::Pos);
		memcpy(walker,fwalker,sizeof(float)*3*numElements);
		fwalker+=sizeof(float)*3*numElements;

		//****
		//leio as coordenadas de textura
		if (flags & SMD_FLAG_TEX)
		{
			walker=curMesh->FindAttribData(Geometry::Mesh::TexCoords);
			memcpy(walker,fwalker,sizeof(float)*2*numElements);
			fwalker+=sizeof(float)*2*numElements;
		}

		//****
		//leio as normais
		if (flags & SMD_FLAG_NRM)
		{
			walker=curMesh->FindAttribData(Geometry::Mesh::Normal);
			memcpy(walker,fwalker,sizeof(float)*3*numElements);
			fwalker+=sizeof(float)*3*numElements;
		}
		
		//****
		//leio as tangents
		if (flags & SMD_FLAG_TG4)
		{
			walker=curMesh->FindAttribData(Geometry::Mesh::Tangent4);
			memcpy(walker,fwalker,sizeof(float)*4*numElements);
			fwalker+=sizeof(float)*4*numElements;
		}

		//agora tenho de ler os indices
		numIndices = *((unsigned int*)fwalker);
		fwalker+=sizeof(unsigned int);

		//leio de maneira diferente de acordo com o número que têm
		if (numIndices >= 65500)
		{
			unsigned int *newIndices;

			//mando criar os indices
			newIndices = (unsigned int*)curMesh->CreateNewIndices(Geometry::Mesh::Int32, numIndices);

			//leio
			memcpy(newIndices,fwalker,sizeof(unsigned int)*numIndices);
			fwalker+=sizeof(unsigned int)*numIndices;
		}
		else
		{
			unsigned short *newIndices;

			//mando criar os indices
			newIndices = (unsigned short*)curMesh->CreateNewIndices(Geometry::Mesh::Int16, numIndices);

			//leio
			memcpy(newIndices,fwalker,sizeof(unsigned short)*numIndices);
			fwalker+=sizeof(unsigned short)*numIndices;
		}
		
		//agora leio quantos strips tenho
		trash=*((unsigned int*)fwalker);
		fwalker+=sizeof(unsigned int);

		//avanço algo para a frente, caso tenho
		if (trash!=0)
		{
			//só tenho de decidir quanto acanço
			if (trash>=65500)
				fwalker+=sizeof(unsigned int)*numIndices;
			else
				fwalker+=sizeof(unsigned short)*numIndices;
		}

		//agora leio quantos triangulos (lista) tenho
		trash=*((unsigned int*)fwalker);
		fwalker+=sizeof(unsigned int);

		//avanço algo para a frente, caso tenho
		if (trash!=0)
		{
			//só tenho de decidir quanto acanço
			if (trash>=65500)
				fwalker+=sizeof(unsigned int)*numIndices;
			else
				fwalker+=sizeof(unsigned short)*numIndices;
		}
	}

	//e pronto, chegando aqui tá tudo bem
	return true;
}

Geometry::Model* SModelReadMemSMD(const void *file, const unsigned int fileSize)
{
	Geometry::Model *modelo;

	//verificar parametros
	if (file == nullptr || fileSize==0)
		return nullptr;

	//crio o modelo
	modelo = new Geometry::Model();
	if (modelo == nullptr)
		return nullptr;

	//simplesmente leio as coisas
	if (carregaSMD(file,fileSize,modelo)==false)
	{
		//apago e saio
		delete modelo;
		return nullptr;
	}

	
	//posso sair todo contente
	return modelo;
}