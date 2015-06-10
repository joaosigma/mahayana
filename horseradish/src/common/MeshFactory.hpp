#pragma once

#include "Mesh.hpp"
#include "Vector.hpp"
#include "Stream.hpp"

#include <vector>

namespace HorseRadish
{
	namespace Geometry
	{
		class Factory;

		class Model {

			friend class Factory;

		public:
			struct MeshData{
				Mesh mesh;
				char meshName[128], materialName[128];
			};

		private:
			static void getData(const Mesh * const mesh, const int attrib, const int element, float *const buffer);

		public://private:
			std::vector<MeshData> arrayMesh;

		public:
			Model();
			Model(int numMesh);
			~Model();

			Mesh* Join() const;
		};

		class Factory {

		private:
			static Mesh* criaNovaMesh(const int numVert, const int numIndex);
			static int drawAroundPoint(Mesh * const mesh, int startPoint, const float xTexCoord, const float radius, const int precision, const float * const matrix);
			static void dividePatch(Mesh * const mesh, const HorseRadish::Vector p[4][4], const int n, int * const curVert);

		public:
			static Mesh* CreateTorus(const float innerRadius, const float outerRadius, const int precision);
			static Mesh* CreateDisk(const float radius, const int precision);
			static Mesh* CreateCone(const float radius, const float height, const int precision);
			static Mesh* CreateCylinder(const float topRadius, const float bottomRadius, const float height, const int precision);
			static Mesh* CreateBox(const float width, const float height, const float depth, const int precision);
			static Mesh* CreatePlane(const float width, const float height, const int precision);
			static Mesh* CreateSphere(const float radius, const int slices, const int stacks);
			static Mesh* CreateParabloid(const float xRadius, const float yRadius, const float zRadius, const int slices, const int stacks);
			static Mesh* CreateTorusKnot(const int turns, const float innerRadius, const float outRadius, const int innerPrecision, const int outPrecision);
			static Mesh* CreatePQTorus(const int P, const int Q, const float innerRadius, const float outRadius, const int innerPrecision, const int outPrecision);
			static Mesh* CreateTeapot(const float radius, const int precision);

			static Model* Read3DS(HorseRadish::Streams::StreamReader &streamReader);
			static Model* ReadOBJ(HorseRadish::Streams::StreamReader &streamReader);
		};

	} //Geometry
} //HorseRadish
