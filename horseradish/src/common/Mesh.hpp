#pragma once

#include "Platform.hpp"
#include "Vector.hpp"
#include "Types.hpp"
#include "Stream.hpp"

namespace HorseRadish
{
	namespace Geometry
	{
		class Mesh
		{
		public:
			enum class ShadeModel : int {
				Phong = (1 << 0),
				Blinn = (1 << 1),
				ColorFloat = (1 << 2),
				Add_Light = (1 << 3)
			};

			enum class TexGenModel {
				ObjectLinear,
				SphereMap,
				NormalMap
			};

			enum MeshOrthoTypes {
				MeshOrthoCreateNormals = (1 << 0),
				MeshOrthoCreateFull = (1 << 1),
				MeshOrthoCreateTangent4 = (1 << 2),
				MeshOrthoNoFinalNormalization = (1 << 3),
				MeshOrthoPerPositionNormal = (1 << 4),
				MeshOrthoKeepNormals = (1 << 5)
			};

			enum MeshAtributeType{
				Pos = (1 << 0),
				Normal = (1 << 1),
				Tangent = (1 << 2),
				Binormal = (1 << 3),
				Tangent4 = (1 << 4),
				TexCoords = (1 << 5),
				Generic1 = (1 << 6),
				Generic2 = (1 << 7),
				Generic3 = (1 << 8),
				Generic4 = (1 << 9)
			};

			enum MeshIndexType{
				Int16 = 0x1403,
				Int32 = 0x1405
			};

			enum MeshOperation{
				OpNormalize = 0x1701,
				OpCClamp = 0x1702,
				OpAverage = 0x1703,
				OpNegate = 0x1704,
				OpInvert = 0x1705,
				OpOneMinus = 0x1706
			};

			struct Attribute{
				MeshAtributeType attribType;
				float *attribData;
			};

		private:
			int numAttrib, numElements, strideAttrib;
			Attribute *attrib;

			void *pIndex;
			int numIndex;
			MeshIndexType typeIndex;

			friend class Factory;
			friend class Model;

			static int calcComponents(const MeshAtributeType attribType);
			static void cleanMesh(Mesh * const mesh);
			static bool checkMesh(const Mesh * const mesh);
			static bool checkMesh(const Mesh * const mesh, const MeshAtributeType attribFlag);
			static int createFlagSetData(const MeshAtributeType flagSet, Mesh * const mesh, float ** const attribData, int * const attribStride, int * const attribComponents, MeshAtributeType * const attribType);
			static void copyVertexData(const Mesh * const meshSource, const int vertSource, Mesh * const meshDest, const int vertDest);
			static bool criaMaisVertices(Mesh * const mesh, const int newCountVert);
			static bool criaMaisIndices(Mesh * const mesh, const int newCountIndex);
			static unsigned int getIndex(const Mesh * const mesh, const int index);
			static void writeIndex(Mesh * const mesh, const int index, const unsigned int val);
			static void copyIndexTri(const Mesh * const mesh, const int indexTo, const int indexFrom);
			static bool zeroAreaTri(Mesh * const mesh, const unsigned int &v1, const unsigned int &v2, const unsigned int &v3);

			static void createFull(Mesh *mesh, const MeshOrthoTypes flags);
			static void createTangent4(Mesh *mesh, const MeshOrthoTypes flags);
			static void createNormals(Mesh *mesh, const MeshOrthoTypes flags);
		public:

			Mesh();
			~Mesh();

			Mesh* Clone() const;
			void Clear();

			Attribute* FindAttrib(const MeshAtributeType type);
			Attribute* FindAttribGeneric(const MeshAtributeType type, const int attribCount);
			Attribute* NewAttrib(const MeshAtributeType type, const int numElements);
			float* FindAttribData(const MeshAtributeType type);
			float* FindAttribGenericData(const MeshAtributeType type, const int attribCount);
			float* FindAttribIndex(const int attribIndex);

			bool CopyAttrib(Mesh * const meshDest, const MeshAtributeType flagSet);

			int GetSize() const;

			int GetNumIndices() const { return this->numIndex; }
			int GetNumElements() const { return this->numElements; }
			int GetNumAttributes() const { return this->numAttrib; }
			int GetStride() const { return this->strideAttrib; }
			MeshIndexType GetIndexType() const { return this->typeIndex; }
			void* GetIndices() const { return this->pIndex; }

			void* CreateNewIndices(const MeshIndexType indexType, const int numIndices);
			void SetVerticesData(const int numAttrib, const int numElements, const int strideAttrib, Geometry::Mesh::Attribute * const attribData);
			void SetNewIndices(const MeshIndexType indexType, const int numIndices, void* indicesBuffer);

			void OpFunc(const MeshAtributeType type, const MeshOperation operation);
			void OpMAD(const MeshAtributeType type, const int component, const float op1, const float op2);
			void OpDAM(const MeshAtributeType type, const int component, const float op1, const float op2);
			void OpMatrix(const MeshAtributeType type, const float * const mat4x4);
			void GetData(const MeshAtributeType type, const int element, float *buffer);
			void SetData(const MeshAtributeType type, const int element, const float *buffer);
			void GetDataGeneric(const MeshAtributeType type, const int attribCount, const int element, float *buffer);
			void SetDataGeneric(const MeshAtributeType type, const int attribCount, const int element, float *buffer);
			bool AddElements(const int numberNew);
			void CopyElementsData(const int vertSource, Mesh * const meshDest, const int vertDest);
			float * SingleBufferPointer();
			bool ReorderAttrib(const MeshAtributeType *typeAttrib, const int numAttrib, bool deleteExtras);
			void SingleBuffer();
			void SingleBufferInv();
			void BoundingBox(float * const minPoint, float * const maxPoint);
			bool RayIntersect(const float *rayOrigin, const float *rayDir, float *distHit);

			void IndexOptimize();
			int  IndexSize() const;
			void InvertFaces();

			float GetIndexCacheRatio(const unsigned int numCacheEntries);
			void CenterMass(const float newX, const float newY, const float newZ);
			void MaxBBox(const float maxDist);
			void Confine(const float newX, const float newY, const float newZ, const float maxDist);
			bool Merge(const Mesh *merge);
			int  RemoveDuplicate(const MeshAtributeType flagSet);
			void EliminateDegenerateTri();

			void Ortho(const MeshOrthoTypes flags);
			bool Shade(const ShadeModel shadeModel, const Vector& lightPos, const Vector& lightDiffuse, const Vector& lightSpecular, const Vector& camPos);
			void ReorderTriIndex();
			bool TexGen(const TexGenModel texGenType, const float *values);

			bool Deserialize(Streams::StreamReader * const streamReader);
			bool DeserializeHRF(Streams::StreamReader * const streamReader);
			bool Serialize(Streams::StreamWriter * const streamWriter);
			bool SerializeHRF(Streams::StreamWriter * const streamWriter);

			/*STRIANGLE* TriInfo(int *numTri, int *numTriClosed);
			SEDGE*	EdgeInfo(const STRIANGLE *triInfo, const int numTri, int *numEdge);
			SPRIMITIVE* Stripper(int *numPrimitives, const int flags);
			SMESH* SubDivide(int flags, const float param);*/
		};

	} //Geometry
} //HorseRadish
