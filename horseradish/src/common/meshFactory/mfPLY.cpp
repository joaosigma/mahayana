#include "common\Platform.hpp"
#include "common\String.hpp"
#include "common\Mesh.hpp"
#include "common\MeshFactory.hpp"
#include "common\Stream.hpp"

using namespace HorseRadish;

#define PLY_FORMAT_ASCII	0x123a
#define PLY_FORMAT_BENDIAN	0x123b
#define PLY_FORMAT_LENDIAN	0x123c

#define PLY_ELEMENT_VERTEX	0xabc1
#define PLY_ELEMENT_FACE	0xabc2

typedef __int8				PLY_CHAR;
typedef unsigned __int8		PLY_UCHAR;
typedef __int16				PLY_SHORT;
typedef unsigned __int16	PLY_USHORT;
typedef __int32				PLY_INT;
typedef unsigned __int32	PLY_UINT;
typedef float				PLY_FLOAT;
typedef double				PLY_DOUBLE;

#define PLY_TYPE_CHAR		0xdea1
#define PLY_TYPE_UCHAR		0xdea2
#define PLY_TYPE_SHORT		0xdea3
#define PLY_TYPE_USHORT		0xdea4
#define PLY_TYPE_INT		0xdea5
#define PLY_TYPE_UINT		0xdea6
#define PLY_TYPE_FLOAT		0xdea7
#define PLY_TYPE_DOUBLE		0xdea8

static
void __fastcall swapEndian32(void * const what)
{
	char temp;
	
	temp=*((char*)what+3);
	*((char*)what+3)=*((char*)what+0);
	*((char*)what+0)=temp;

	temp=*((char*)what+2);
	*((char*)what+2)=*((char*)what+1);
	*((char*)what+1)=temp;
}

static
void __fastcall swapEndian32(void * what, int num)
{
	char temp;

	while(num)
		{
		temp=*((char*)what+3);
		*((char*)what+3)=*((char*)what+0);
		*((char*)what+0)=temp;

		temp=*((char*)what+2);
		*((char*)what+2)=*((char*)what+1);
		*((char*)what+1)=temp;

		num--;
		what=((char*)what)+4;
		}
}

static
bool leLinha(HorseRadish::Streams::StreamReader * const streamReader, HorseRadish::String &readTo)
{
	char buffer[256];
	int i;

	for(i=0; i<256 && streamReader->Read(buffer+i,1)==1; i++)
	{
		if (buffer[i]==10)
			break;
	}
	
	if (buffer[i]!=10)
	{
		readTo.SetEmpty();
		return false;
	}

	buffer[i+1]='\0';

	readTo.SetEmpty();
	readTo.Set(HorseRadish::String::Encoding::UTF8, buffer);
	readTo.RemoveAllChars(13);
	readTo.RemoveAllChars(10);

	return true;
}

static
bool checkIsPLY(HorseRadish::Streams::StreamReader * const streamReader)
{
	HorseRadish::String linha;

	//leio linha
	leLinha(streamReader, linha);

	//preciso de um tokenizer
	HorseRadish::String::Tokenizer tok(linha, ' ');

	//se nao tiver lá "ply"
	if (tok.Read() != "ply")
		return false;

	//é PLY
	return true;
}

static
bool readHeader(HorseRadish::Streams::StreamReader * const streamReader, int *plyFormat, int *plyNumVertex, int *plyNumFaces, int *plyVSize, int *plyFSize)
{
	HorseRadish::String linha,param;
	int whatElement;

	//percorro toda a linha do  ficheiro
	whatElement=0;
	while(streamReader != nullptr)
	{
		leLinha(streamReader,linha);
		if (linha.GetSizeBytes()<=0)
			continue;

		//se cheguei ao fim, fixe
		if (linha=="end_header")
			break;

		//preciso de um tokenizer
		HorseRadish::String::Tokenizer tok(linha, ' ');

		//o primeiro parametro
		param = tok.Read();
		
		//se for o formato
		if (param=="format")
		{
			param = tok.Read();

			if (param=="ascii")
				*plyFormat=PLY_FORMAT_ASCII;
			else if (param=="binary_big_endian")
				*plyFormat=PLY_FORMAT_BENDIAN;
			else if (param=="binary_little_endian")
				*plyFormat=PLY_FORMAT_LENDIAN;
			continue;
		}
		
		//se for um elemento
		if (param=="element")
		{
			param = tok.Read();

			if (param=="vertex")
			{
				*plyNumVertex = tok.Read().ToInt();
				whatElement=PLY_ELEMENT_VERTEX;
			}
			else if (param=="face")
			{
				*plyNumFaces = tok.Read().ToInt();
				whatElement=PLY_ELEMENT_FACE;
			}
			continue;
		}

		//se for uma propriedade
		if (param=="property")
		{
			//o segundo parametro
			param = tok.Read();

			//qual o tipo
			if (param=="char")
			{
				switch(whatElement)
				{
					case PLY_ELEMENT_VERTEX: *plyVSize+=sizeof(PLY_CHAR);	break;
					case PLY_ELEMENT_FACE: *plyFSize+=sizeof(PLY_CHAR);	break;
				}
				continue;
			}
			if (param=="uchar")
			{
				switch(whatElement)
				{
					case PLY_ELEMENT_VERTEX: *plyVSize+=sizeof(PLY_UCHAR);	break;
					case PLY_ELEMENT_FACE: *plyFSize+=sizeof(PLY_UCHAR);	break;
				}
				continue;
			}
			if (param=="short")
			{
				switch(whatElement)
				{
					case PLY_ELEMENT_VERTEX: *plyVSize+=sizeof(PLY_SHORT);	break;
					case PLY_ELEMENT_FACE: *plyFSize+=sizeof(PLY_SHORT);	break;
				}
				continue;
			}
			if (param=="ushort")
			{
				switch(whatElement)
				{
					case PLY_ELEMENT_VERTEX: *plyVSize+=sizeof(PLY_USHORT);	break;
					case PLY_ELEMENT_FACE: *plyFSize+=sizeof(PLY_USHORT);	break;
				}
				continue;
			}
			if (param=="int")
			{
				switch(whatElement)
				{
					case PLY_ELEMENT_VERTEX: *plyVSize+=sizeof(PLY_INT);	break;
					case PLY_ELEMENT_FACE: *plyFSize+=sizeof(PLY_INT);	break;
				}
				continue;
			}
			if (param=="uint")
			{
				switch(whatElement){
					case PLY_ELEMENT_VERTEX: *plyVSize+=sizeof(PLY_UINT);	break;
					case PLY_ELEMENT_FACE: *plyFSize+=sizeof(PLY_UINT);	break;
					}
				continue;
			}
			if (param=="float" || param=="float32")
			{
				switch(whatElement){
					case PLY_ELEMENT_VERTEX: *plyVSize+=sizeof(PLY_FLOAT);	break;
					case PLY_ELEMENT_FACE: *plyFSize+=sizeof(PLY_FLOAT);	break;
					}
				continue;
			}
			if (param=="double")
			{
				switch(whatElement){
					case PLY_ELEMENT_VERTEX: *plyVSize+=sizeof(PLY_DOUBLE);	break;
					case PLY_ELEMENT_FACE: *plyFSize+=sizeof(PLY_DOUBLE);	break;
					}
				continue;
			}
		}
	}

	return true;
}

static
Geometry::Model * iniciaModelo(const int numVertex, const int numFaces)
{
	Geometry::Mesh *novaMesh;
	float *bigBuffer;
	Geometry::Model *modeloReturn;
	Geometry::Mesh::Attribute *novosAttrib;
	unsigned int *newIndices;

	modeloReturn=new Geometry::Model(1);
	if (modeloReturn==nullptr)
		return nullptr;

	//mando criar já uma mesh
	modeloReturn->arrayMesh.push_back(Geometry::Model::MeshData());

	//e prontos, tenho a minha mesh pronta a ser utilizada
	modeloReturn->arrayMesh[0].meshName[0]='\0';
	modeloReturn->arrayMesh[0].materialName[0]='\0';
	memset(&modeloReturn->arrayMesh[0].mesh,0,sizeof(Geometry::Mesh));
	novaMesh=&modeloReturn->arrayMesh[0].mesh;

	//crio os indices
	newIndices = (unsigned int*)novaMesh->CreateNewIndices(Geometry::Mesh::Int32, numFaces * 2 * 3);

	//crio as restantes coisas
	novosAttrib=new Geometry::Mesh::Attribute[1];
	bigBuffer=new float[numVertex*3];
	if (newIndices==nullptr || novosAttrib==nullptr || bigBuffer==nullptr)
	{
		if (novosAttrib)
			delete [] novosAttrib;
		if (bigBuffer)
			delete [] bigBuffer;
		delete modeloReturn;
		return nullptr;
	}

	//posso já arranjar este attrib
	novosAttrib[0].attribData=bigBuffer;
	novosAttrib[0].attribType=Geometry::Mesh::Pos;

	//ajusto os parametros na mesh
	novaMesh->SetVerticesData(1, numVertex, 0, novosAttrib);

	//e já tá
	return modeloReturn;
}

static
bool readFaces(Geometry::Mesh *mesh, HorseRadish::Streams::StreamReader * const streamReader, const int plyFormat, const int plyNumFaces, const int sizeofNumIdex, const int extraBytes)
{
	unsigned int *walker,faceBuffer[5];
	int realNumIndices,numIndex,i;
	HorseRadish::String linha;

	//começo a preparar as coisas
	walker=(unsigned int*)mesh->GetIndices();
	realNumIndices=0;

	//se for ascii
	if (plyFormat==PLY_FORMAT_ASCII)
	{
		//leio as faces
		for(i=0; i<plyNumFaces; i++)
		{
			//leio uma linha e quantos indices tenho
			leLinha(streamReader, linha);
			sscanf(linha.GetData(), "%d", &faceBuffer[0]);

			//leio de acordo
			if (faceBuffer[0]==3)
			{
				//leio os três
				sscanf(linha.GetData(), "%d %d %d %d", &faceBuffer[0],&faceBuffer[1],&faceBuffer[2],&faceBuffer[3]);
				walker[0]=faceBuffer[1];
				walker[1]=faceBuffer[2];
				walker[2]=faceBuffer[3];
				walker+=3;
				realNumIndices+=3;
				}
			else if (faceBuffer[0]==4)
			{
				//só pode ser quatro
				sscanf(linha.GetData(), "%d %d %d %d %d", &faceBuffer[0],&faceBuffer[1],&faceBuffer[2],&faceBuffer[3],&faceBuffer[4]);
				walker[0]=faceBuffer[1];
				walker[1]=faceBuffer[2];
				walker[2]=faceBuffer[3];
				walker[3]=faceBuffer[1];
				walker[4]=faceBuffer[3];
				walker[5]=faceBuffer[4];
				walker+=6;
				realNumIndices+=6;
			}
			else
			{
				return false;
			}
		}

		//tenho de escrever o numero real de indices
		mesh->SetNewIndices(mesh->GetIndexType(), realNumIndices, mesh->GetIndices());
		return true;
	}

	//se for formato little_endian
	if (plyFormat==PLY_FORMAT_BENDIAN)
	{
		for(i=0; i<plyNumFaces; i++)
		{		
			//quantos tenho de ler
			memset(&numIndex,0,sizeof(int));
			streamReader->Read(&numIndex,sizeofNumIdex);
			if (numIndex==3)
			{
				streamReader->Read(faceBuffer,sizeof(int)*3);
				walker[0]=faceBuffer[0];
				walker[1]=faceBuffer[1];
				walker[2]=faceBuffer[2];
				swapEndian32(walker+0);
				swapEndian32(walker+1);
				swapEndian32(walker+2);
				walker+=3;
				realNumIndices+=3;
			}
			else
			{
				streamReader->Read(faceBuffer,sizeof(int)*4);
				walker[0]=faceBuffer[0];
				walker[1]=faceBuffer[1];
				walker[2]=faceBuffer[2];
				walker[3]=faceBuffer[0];
				walker[4]=faceBuffer[2];
				walker[5]=faceBuffer[3];
				walker+=6;
				realNumIndices+=6;
			}

			//avanço estes numeros extras de bytes
			streamReader->Seek(extraBytes, HorseRadish::Streams::Stream::Current);
		}

		//tenho de escrever o numero real de indices
		mesh->SetNewIndices(mesh->GetIndexType(), realNumIndices, mesh->GetIndices());
		return true;
	}

	//se for formato little_endian
	if (plyFormat==PLY_FORMAT_LENDIAN)
	{
		for(i=0; i<plyNumFaces; i++)
		{		
			//quantos tenho de ler
			memset(&numIndex,0,sizeof(int));
			streamReader->Read(&numIndex,sizeofNumIdex);
			if (numIndex==3)
			{
				streamReader->Read(faceBuffer,sizeof(int)*3);
				walker[0]=faceBuffer[0];
				walker[1]=faceBuffer[1];
				walker[2]=faceBuffer[2];
				walker+=3;
				realNumIndices+=3;
			}
			else
			{
				streamReader->Read(faceBuffer,sizeof(int)*4);
				walker[0]=faceBuffer[0];
				walker[1]=faceBuffer[1];
				walker[2]=faceBuffer[2];
				walker[3]=faceBuffer[0];
				walker[4]=faceBuffer[2];
				walker[5]=faceBuffer[3];
				walker+=6;
				realNumIndices+=6;
			}

			//avanço estes numeros extras de bytes
			streamReader->Seek(extraBytes, HorseRadish::Streams::Stream::Current);
		}

		//tenho de escrever o numero real de indices
		mesh->SetNewIndices(mesh->GetIndexType(), realNumIndices, mesh->GetIndices());
		return true;
	}

	//se chegar aqui, deu barraca, logo, fecho o ficheiro e devolvo NULL
	return false;
}

static
float readDataTypeF(const char * const line, const int dataOffset, const int dataType)
{
	const char *readData;

	readData=line+dataOffset;
	switch(dataType){
		case PLY_TYPE_CHAR:		return (float)( *((PLY_CHAR*)readData)); break;
		case PLY_TYPE_UCHAR:	return (float)( *((PLY_UCHAR*)readData)); break;
		case PLY_TYPE_SHORT:	return (float)( *((PLY_SHORT*)readData)); break;
		case PLY_TYPE_USHORT:	return (float)( *((PLY_USHORT*)readData)); break;
		case PLY_TYPE_INT:		return (float)( *((PLY_INT*)readData)); break;
		case PLY_TYPE_UINT:		return (float)( *((PLY_UINT*)readData)); break;
		case PLY_TYPE_FLOAT:	return (float)( *((PLY_FLOAT*)readData)); break;
		case PLY_TYPE_DOUBLE:	return (float)( *((PLY_DOUBLE*)readData)); break;
		}
	return 0.0f;
}

static
bool readVertex(Geometry::Mesh *mesh, HorseRadish::Streams::StreamReader * const streamReader, const int plyFormat, const int plyNumVertex, const int lineSize, const int xOffset, const int xType, const int yOffset, const int yType, const int zOffset, const int zType)
{
	int i,stride;
	HorseRadish::String linha;
	float *walker;
	char *lineData;

	//check stuff out
	if (streamReader==nullptr)
		return false;
	if (mesh==nullptr || plyNumVertex<=0)
		return false;

	//vou buscar isto
	walker=mesh->FindAttribData(Geometry::Mesh::Pos);
	stride=3;
	if (mesh->GetStride() != 0)
		stride=mesh->GetStride()/sizeof(float);

	//se for ascii
	if (plyFormat==PLY_FORMAT_ASCII)
	{
		for(i=0; i<plyNumVertex; i++,walker+=stride)
		{		
			leLinha(streamReader, linha);
			sscanf(linha.GetData(), "%f %f %f", walker, walker + 1, walker + 2);
		}
		return true;
	}

	//se for big_endian
	if (plyFormat==PLY_FORMAT_BENDIAN)
		{
		//vou criar memoria para ler a linha inteira
		lineData=new char[lineSize];
		if (lineData==nullptr)
			return false;

		//para cada vertice
		for(i=0; i<plyNumVertex; i++,walker+=stride)
		{
			//leio a linha
			streamReader->Read(lineData,lineSize);

			//e agora leio os componentes
			walker[0]=readDataTypeF(lineData,xOffset,xType);
			walker[1]=readDataTypeF(lineData,yOffset,yType);
			walker[2]=readDataTypeF(lineData,zOffset,zType);

			//como é big_endian tenho de mudar
			swapEndian32(walker+0);
			swapEndian32(walker+1);
			swapEndian32(walker+2);
		}

		//apago lixo e sigo na minha vida
		delete [] lineData;
		return true;
		}

	//se for little_endian
	if (plyFormat==PLY_FORMAT_LENDIAN)
		{
		//acso porreiro, tenho de ler os pontos e só tenho pontos para ler! :)
		if (stride==3 && lineSize==12)
			{
			streamReader->Read(walker,sizeof(float)*3*plyNumVertex);
			return true;
			}

		//vou criar memoria para ler a linha inteira
		lineData=new char[lineSize];
		if (lineData==nullptr)
			return false;

		//para cada vertice
		for(i=0; i<plyNumVertex; i++,walker+=stride)
			{
			//leio a linha
			streamReader->Read(lineData,lineSize);

			//e agora leio os componentes
			walker[0]=readDataTypeF(lineData,xOffset,xType);
			walker[1]=readDataTypeF(lineData,yOffset,yType);
			walker[2]=readDataTypeF(lineData,zOffset,zType);

			//SFileRead(fp,walker,sizeof(float)*3);
			}

		//apago lixo e sigo na minha vida
		delete [] lineData;
		return true;
		}

	//chegando aqui deu barraca
	return false;
}

//extern "C" __declspec(dllexport)
//Geometry::Model * SModelReadPLY(const char *file)
//{
//	void *fileData;
//	int fileSize;
//	Geometry::Model *modeloReturn;
//	int plyFormat,plyNumVertex,plyNumFaces,vertexLSize,faceLSize;
//
//	//maratos
//	if (file==nullptr || file[0]=='\0')
//		return nullptr;
//
//	//abro o ficheiro
//	fileData=SFileReturn(file, &fileSize);
//	if (fileData == nullptr)
//		return nullptr;
//
//	{
//		HorseRadish::Streams::MemoryStream memoryStream(fileData, fileSize);
//
//		{
//			HorseRadish::Streams::StreamReader streamReader(&memoryStream);
//			bool sucesso;
//
//			//dá jeito ser um formato ply
//			if (checkIsPLY(&streamReader) == false)
//				return nullptr;
//			
//			//toca a limpar esta cena toda
//			plyFormat=plyNumVertex=plyNumFaces=vertexLSize=faceLSize=0;
//			
//			//leio o header
//			sucesso = readHeader(&streamReader, &plyFormat,&plyNumVertex,&plyNumFaces,&vertexLSize,&faceLSize);
//
//			//agora toca a ver se isto tá tudo como deve de ser
//			if (sucesso==false || plyFormat==0 || plyNumVertex<=0 || plyNumFaces<=0 || vertexLSize<=0 /*|| faceLSize<=0*/)
//				{
//				free(fileData);
//				return nullptr;
//				}
//
//			//construo o modelo
//			modeloReturn=iniciaModelo(plyNumVertex,plyNumFaces);
//			if (modeloReturn==nullptr)
//				{
//				free(fileData);
//				return nullptr;
//				}
//
//			//leio os vertices
//			sucesso = readVertex(&modeloReturn->Meshes[0].mesh, &streamReader, plyFormat, plyNumVertex, vertexLSize, sizeof(PLY_FLOAT)*0, PLY_TYPE_FLOAT, sizeof(PLY_FLOAT)*1, PLY_TYPE_FLOAT, sizeof(PLY_FLOAT)*2, PLY_TYPE_FLOAT);
//			if (sucesso == false)
//				{
//				SModelDelete(modeloReturn);
//				free(fileData);
//				return nullptr;
//				}
//
//			//e agora, finalmente, leio os indices
//			sucesso = readFaces(&modeloReturn->Meshes[0].mesh, &streamReader, plyFormat, plyNumFaces, sizeof(PLY_CHAR), faceLSize);
//			if (sucesso == false)
//				{
//				SModelDelete(modeloReturn);
//				free(fileData);
//				return nullptr;
//				}
//		}
//	}
//
//	//tenho de finalmente fechar o ficheiro e wwwweeeeeeeee :)
//	free(fileData);
//	return modeloReturn;
//}

Geometry::Model* SModelReadMemPLY(const void *file, const unsigned int fileSize)
{
	return nullptr;
}