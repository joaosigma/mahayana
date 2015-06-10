#include "MeshFactory.hpp"

#include "Math.hpp"
#include "Vector.hpp"
#include "meshFactory\mf.hpp"

//vertices for the Teapot
static float vertTP[306][3]={{1.4f ,0.0f ,2.4f}, {1.4f ,-0.784f ,2.4f},
{0.784f ,-1.4f ,2.4f}, {0.0f ,-1.4f ,2.4f}, {1.3375f ,0.0f ,2.53125f},
{1.3375f ,-0.749f ,2.53125f}, {0.749f ,-1.3375f ,2.53125f}, {0.0f ,-1.3375f ,2.53125f}, 
{1.4375f ,0.0f ,2.53125f}, {1.4375f ,-0.805f ,2.53125f}, {0.805f ,-1.4375f ,2.53125f},
{0.0f ,-1.4375f ,2.53125f}, {1.5f ,0.0f ,2.4f}, {1.5f ,-0.84f ,2.4f},
{0.84f ,-1.5f ,2.4f}, {0.0f ,-1.5f ,2.4f}, {-0.784f ,-1.4f ,2.4f},
{-1.4f ,-0.784f ,2.4f}, {-1.4f ,0.0f ,2.4f}, {-0.749f ,-1.3375f ,2.53125f},
{-1.3375f ,-0.749f ,2.53125f}, {-1.3375f ,0.0f ,2.53125f}, {-0.805f ,-1.4375f ,2.53125f},
{-1.4375f ,-0.805f ,2.53125f}, {-1.4375f ,0.0f ,2.53125f}, {-0.84f ,-1.5f ,2.4f},
{-1.5f ,-0.84f ,2.4f}, {-1.5f ,0.0f ,2.4f}, {-1.4f ,0.784f ,2.4f},
{-0.784f ,1.4f ,2.4f}, {0.0f ,1.4f ,2.4f}, {-1.3375f ,0.749f ,2.53125f},
{-0.749f ,1.3375f ,2.53125f}, {0.0f ,1.3375f ,2.53125f}, {-1.4375f ,0.805f ,2.53125f},
{-0.805f ,1.4375f ,2.53125f}, {0.0f ,1.4375f ,2.53125f}, {-1.5f ,0.84f ,2.4f},
{-0.84f ,1.5f ,2.4f}, {0.0f ,1.5f ,2.4f}, {0.784f ,1.4f ,2.4f},
{1.4f ,0.784f ,2.4f}, {0.749f ,1.3375f ,2.53125f}, {1.3375f ,0.749f ,2.53125f},
{0.805f ,1.4375f ,2.53125f}, {1.4375f ,0.805f ,2.53125f}, {0.84f ,1.5f ,2.4f},
{1.5f ,0.84f ,2.4f}, {1.75f ,0.0f ,1.875f}, {1.75f ,-0.98f ,1.875f},
{0.98f ,-1.75f ,1.875f}, {0.0f ,-1.75f ,1.875f}, {2.0f ,0.0f ,1.35f},
{2.0f ,-1.12f ,1.35f}, {1.12f ,-2.0f ,1.35f}, {0.0f ,-2.0f ,1.35f},
{2.0f ,0.0f ,0.9f}, {2.0f ,-1.12f ,0.9f}, {1.12f ,-2.0f ,0.9f},
{0.0f ,-2.0f ,0.9f}, {-0.98f ,-1.75f ,1.875f}, {-1.75f ,-0.98f ,1.875f},
{-1.75f ,0.0f ,1.875f}, {-1.12f ,-2.0f ,1.35f}, {-2.0f ,-1.12f ,1.35f},
{-2.0f ,0.0f ,1.35f}, {-1.12f ,-2.0f ,0.9f}, {-2.0f ,-1.12f ,0.9f},
{-2.0f ,0.0f ,0.9f}, {-1.75f ,0.98f ,1.875f}, {-0.98f ,1.75f ,1.875f},
{0.0f ,1.75f ,1.875f}, {-2.0f ,1.12f ,1.35f}, {-1.12f ,2.0f ,1.35f},
{0.0f ,2.0f ,1.35f}, {-2.0f ,1.12f ,0.9f}, {-1.12f ,2.0f ,0.9f},
{0.0f ,2.0f ,0.9f}, {0.98f ,1.75f ,1.875f}, {1.75f ,0.98f ,1.875f},
{1.12f ,2.0f ,1.35f}, {2.0f ,1.12f ,1.35f}, {1.12f ,2.0f ,0.9f},
{2.0f ,1.12f ,0.9f}, {2.0f ,0.0f ,0.45f}, {2.0f ,-1.12f ,0.45f},
{1.12f ,-2.0f ,0.45f}, {0.0f ,-2.0f ,0.45f}, {1.5f ,0.0f ,0.225f},
{1.5f ,-0.84f ,0.225f}, {0.84f ,-1.5f ,0.225f}, {0.0f ,-1.5f ,0.225f},
{1.5f ,0.0f ,0.15f}, {1.5f ,-0.84f ,0.15f}, {0.84f ,-1.5f ,0.15f},
{0.0f ,-1.5f ,0.15f}, {-1.12f ,-2.0f ,0.45f}, {-2.0f ,-1.12f ,0.45f},
{-2.0f ,0.0f ,0.45f}, {-0.84f ,-1.5f ,0.225f}, {-1.5f ,-0.84f ,0.225f},
{-1.5f ,0.0f ,0.225f}, {-0.84f ,-1.5f ,0.15f}, {-1.5f ,-0.84f ,0.15f},
{-1.5f ,0.0f ,0.15f}, {-2.0f ,1.12f ,0.45f}, {-1.12f ,2.0f ,0.45f},
{0.0f ,2.0f ,0.45f}, {-1.5f ,0.84f ,0.225f}, {-0.84f ,1.5f ,0.225f},
{0.0f ,1.5f ,0.225f}, {-1.5f ,0.84f ,0.15f}, {-0.84f ,1.5f ,0.15f},
{0.0f ,1.5f ,0.15f}, {1.12f ,2.0f ,0.45f}, {2.0f ,1.12f ,0.45f},
{0.84f ,1.5f ,0.225f}, {1.5f ,0.84f ,0.225f}, {0.84f ,1.5f ,0.15f},
{1.5f ,0.84f ,0.15f}, {-1.6f ,0.0f ,2.025f}, {-1.6f ,-0.3f ,2.025f},
{-1.5f ,-0.3f ,2.25f}, {-1.5f ,0.0f ,2.25f}, {-2.3f ,0.0f ,2.025f},
{-2.3f ,-0.3f ,2.025f}, {-2.5f ,-0.3f ,2.25f}, {-2.5f ,0.0f ,2.25f},
{-2.7f ,0.0f ,2.025f}, {-2.7f ,-0.3f ,2.025f}, {-3.0f ,-0.3f ,2.25f},
{-3.0f ,0.0f ,2.25f}, {-2.7f ,0.0f ,1.8f}, {-2.7f ,-0.3f ,1.8f},
{-3.0f ,-0.3f ,1.8f}, {-3.0f ,0.0f ,1.8f}, {-1.5f ,0.3f ,2.25f},
{-1.6f ,0.3f ,2.025f}, {-2.5f ,0.3f ,2.25f}, {-2.3f ,0.3f ,2.025f},
{-3.0f ,0.3f ,2.25f}, {-2.7f ,0.3f ,2.025f}, {-3.0f ,0.3f ,1.8f},
{-2.7f ,0.3f ,1.8f}, {-2.7f ,0.0f ,1.575f}, {-2.7f ,-0.3f ,1.575f},
{-3.0f ,-0.3f ,1.35f}, {-3.0f ,0.0f ,1.35f}, {-2.5f ,0.0f ,1.125f},
{-2.5f ,-0.3f ,1.125f}, {-2.65f ,-0.3f ,0.9375f}, {-2.65f ,0.0f ,0.9375f},
{-2.0f ,-0.3f ,0.9f}, {-1.9f ,-0.3f ,0.6f}, {-1.9f ,0.0f ,0.6f},
{-3.0f ,0.3f ,1.35f}, {-2.7f ,0.3f ,1.575f}, {-2.65f ,0.3f ,0.9375f},
{-2.5f ,0.3f ,1.125f}, {-1.9f ,0.3f ,0.6f}, {-2.0f ,0.3f ,0.9f},
{1.7f ,0.0f ,1.425f}, {1.7f ,-0.66f ,1.425f}, {1.7f ,-0.66f ,0.6f},
{1.7f ,0.0f ,0.6f}, {2.6f ,0.0f ,1.425f}, {2.6f ,-0.66f ,1.425f},
{3.1f ,-0.66f ,0.825f}, {3.1f ,0.0f ,0.825f}, {2.3f ,0.0f ,2.1f},
{2.3f ,-0.25f ,2.1f}, {2.4f ,-0.25f ,2.025f}, {2.4f ,0.0f ,2.025f},
{2.7f ,0.0f ,2.4f}, {2.7f ,-0.25f ,2.4f}, {3.3f ,-0.25f ,2.4f},
{3.3f ,0.0f ,2.4f}, {1.7f ,0.66f ,0.6f}, {1.7f ,0.66f ,1.425f},
{3.1f ,0.66f ,0.825f}, {2.6f ,0.66f ,1.425f}, {2.4f ,0.25f ,2.025f},
{2.3f ,0.25f ,2.1f}, {3.3f ,0.25f ,2.4f}, {2.7f ,0.25f ,2.4f},
{2.8f ,0.0f ,2.475f}, {2.8f ,-0.25f ,2.475f}, {3.525f ,-0.25f ,2.49375f},
{3.525f ,0.0f ,2.49375f}, {2.9f ,0.0f ,2.475f}, {2.9f ,-0.15f ,2.475f},
{3.45f ,-0.15f ,2.5125f}, {3.45f ,0.0f ,2.5125f}, {2.8f ,0.0f ,2.4f},
{2.8f ,-0.15f ,2.4f}, {3.2f ,-0.15f ,2.4f}, {3.2f ,0.0f ,2.4f},
{3.525f ,0.25f ,2.49375f}, {2.8f ,0.25f ,2.475f}, {3.45f ,0.15f ,2.5125f},
{2.9f ,0.15f ,2.475f}, {3.2f ,0.15f ,2.4f}, {2.8f ,0.15f ,2.4f},
{0.0f ,0.0f ,3.15f}, {0.0f ,-0.002f ,3.15f}, {0.002f ,0.0f ,3.15f},
{0.8f ,0.0f ,3.15f}, {0.8f ,-0.45f ,3.15f}, {0.45f ,-0.8f ,3.15f},
{0.0f ,-0.8f ,3.15f}, {0.0f ,0.0f ,2.85f}, {0.2f ,0.0f ,2.7f},
{0.2f ,-0.112f ,2.7f}, {0.112f ,-0.2f ,2.7f}, {0.0f ,-0.2f ,2.7f},
{-0.002f ,0.0f ,3.15f}, {-0.45f ,-0.8f ,3.15f}, {-0.8f ,-0.45f ,3.15f},
{-0.8f ,0.0f ,3.15f}, {-0.112f ,-0.2f ,2.7f}, {-0.2f ,-0.112f ,2.7f},
{-0.2f ,0.0f ,2.7f}, {0.0f ,0.002f ,3.15f}, {-0.8f ,0.45f ,3.15f},
{-0.45f ,0.8f ,3.15f}, {0.0f ,0.8f ,3.15f}, {-0.2f ,0.112f ,2.7f},
{-0.112f ,0.2f ,2.7f}, {0.0f ,0.2f ,2.7f}, {0.45f ,0.8f ,3.15f},
{0.8f ,0.45f ,3.15f}, {0.112f ,0.2f ,2.7f}, {0.2f ,0.112f ,2.7f},
{0.4f ,0.0f ,2.55f}, {0.4f ,-0.224f ,2.55f}, {0.224f ,-0.4f ,2.55f},
{0.0f ,-0.4f ,2.55f}, {1.3f ,0.0f ,2.55f}, {1.3f ,-0.728f ,2.55f},
{0.728f ,-1.3f ,2.55f}, {0.0f ,-1.3f ,2.55f}, {1.3f ,0.0f ,2.4f},
{1.3f ,-0.728f ,2.4f}, {0.728f ,-1.3f ,2.4f}, {0.0f ,-1.3f ,2.4f},
{-0.224f ,-0.4f ,2.55f}, {-0.4f ,-0.224f ,2.55f}, {-0.4f ,0.0f ,2.55f},
{-0.728f ,-1.3f ,2.55f}, {-1.3f ,-0.728f ,2.55f}, {-1.3f ,0.0f ,2.55f},
{-0.728f ,-1.3f ,2.4f}, {-1.3f ,-0.728f ,2.4f}, {-1.3f ,0.0f ,2.4f},
{-0.4f ,0.224f ,2.55f}, {-0.224f ,0.4f ,2.55f}, {0.0f ,0.4f ,2.55f},
{-1.3f ,0.728f ,2.55f}, {-0.728f ,1.3f ,2.55f}, {0.0f ,1.3f ,2.55f},
{-1.3f ,0.728f ,2.4f}, {-0.728f ,1.3f ,2.4f}, {0.0f ,1.3f ,2.4f},
{0.224f ,0.4f ,2.55f}, {0.4f ,0.224f ,2.55f}, {0.728f ,1.3f ,2.55f},
{1.3f ,0.728f ,2.55f}, {0.728f ,1.3f ,2.4f}, {1.3f ,0.728f ,2.4f},
{0.0f ,0.0f ,0.0f}, {1.5f ,0.0f ,0.15f}, {1.5f ,0.84f ,0.15f},
{0.84f ,1.5f ,0.15f}, {0.0f ,1.5f ,0.15f}, {1.5f ,0.0f ,0.075f},
{1.5f ,0.84f ,0.075f}, {0.84f ,1.5f ,0.075f}, {0.0f ,1.5f ,0.075f},
{1.425f ,0.0f ,0.0f}, {1.425f ,0.798f ,0.0f}, {0.798f ,1.425f ,0.0f},
{0.0f ,1.425f ,0.0f}, {-0.84f ,1.5f ,0.15f}, {-1.5f ,0.84f ,0.15f},
{-1.5f ,0.0f ,0.15f}, {-0.84f ,1.5f ,0.075f}, {-1.5f ,0.84f ,0.075f},
{-1.5f ,0.0f ,0.075f}, {-0.798f ,1.425f ,0.0f}, {-1.425f ,0.798f ,0.0f},
{-1.425f ,0.0f ,0.0f}, {-1.5f ,-0.84f ,0.15f}, {-0.84f ,-1.5f ,0.15f},
{0.0f ,-1.5f ,0.15f}, {-1.5f ,-0.84f ,0.075f}, {-0.84f ,-1.5f ,0.075f},
{0.0f ,-1.5f ,0.075f}, {-1.425f ,-0.798f ,0.0f}, {-0.798f ,-1.425f ,0.0f},
{0.0f ,-1.425f ,0.0f}, {0.84f ,-1.5f ,0.15f}, {1.5f ,-0.84f ,0.15f},
{0.84f ,-1.5f ,0.075f}, {1.5f ,-0.84f ,0.075f}, {0.798f ,-1.425f ,0.0f},
{1.425f ,-0.798f ,0.0}}; 

/* 32 patches each defined by 16 vertices, arranged in a 4 x 4 array */
/* NOTE: numbering scheme for teapot has vertices labeled from 1 to 306 */
/* remnant of the days of FORTRAN */

static int indexTP[32][4][4]={{1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16},
{4, 17, 18, 19, 8, 20, 21, 22, 12, 23, 24, 25, 16, 26, 27, 28},
{19, 29, 30, 31, 22, 32, 33, 34, 25, 35, 36, 37, 28, 38, 39, 40},
{31, 41, 42, 1, 34, 43, 44, 5, 37, 45, 46, 9, 40, 47, 48, 13},
{13, 14, 15, 16, 49, 50, 51, 52, 53, 54, 55, 56, 57, 58, 59, 60},
{16, 26, 27, 28, 52, 61, 62, 63, 56, 64, 65, 66, 60, 67, 68, 69},
{28, 38, 39, 40, 63, 70, 71, 72, 66, 73, 74, 75, 69, 76, 77, 78},
{40, 47, 48, 13, 72, 79, 80, 49, 75, 81, 82, 53, 78, 83, 84, 57},
{57, 58, 59, 60, 85, 86, 87, 88, 89, 90, 91, 92, 93, 94, 95, 96},
{60, 67, 68, 69, 88, 97, 98, 99, 92, 100, 101, 102, 96, 103, 104, 105},
{69, 76, 77, 78, 99, 106, 107, 108, 102, 109, 110, 111, 105, 112, 113, 114},
{78, 83, 84, 57, 108, 115, 116, 85, 111, 117, 118, 89, 114, 119, 120, 93},
{121, 122, 123, 124, 125, 126, 127, 128, 129, 130, 131, 132, 133, 134, 135, 136},
{124, 137, 138, 121, 128, 139, 140, 125, 132, 141, 142, 129, 136, 143, 144, 133},
{133, 134, 135, 136, 145, 146, 147, 148, 149, 150, 151, 152, 69, 153, 154, 155},
{136, 143, 144, 133, 148, 156, 157, 145, 152, 158, 159, 149, 155, 160, 161, 69},
{162, 163, 164, 165, 166, 167, 168, 169, 170, 171, 172, 173, 174, 175, 176, 177},
{165, 178, 179, 162, 169, 180, 181, 166, 173, 182, 183, 170, 177, 184, 185, 174},
{174, 175, 176, 177, 186, 187, 188, 189, 190, 191, 192, 193, 194, 195, 196, 197},
{177, 184, 185, 174, 189, 198, 199, 186, 193, 200, 201, 190, 197, 202, 203, 194},
{204, 204, 204, 204, 207, 208, 209, 210, 211, 211, 211, 211, 212, 213, 214, 215},
{204, 204, 204, 204, 210, 217, 218, 219, 211, 211, 211, 211, 215, 220, 221, 222},
{204, 204, 204, 204, 219, 224, 225, 226, 211, 211, 211, 211, 222, 227, 228, 229},
{204, 204, 204, 204, 226, 230, 231, 207, 211, 211, 211, 211, 229, 232, 233, 212},
{212, 213, 214, 215, 234, 235, 236, 237, 238, 239, 240, 241, 242, 243, 244, 245},
{215, 220, 221, 222, 237, 246, 247, 248, 241, 249, 250, 251, 245, 252, 253, 254},
{222, 227, 228, 229, 248, 255, 256, 257, 251, 258, 259, 260, 254, 261, 262, 263},
{229, 232, 233, 212, 257, 264, 265, 234, 260, 266, 267, 238, 263, 268, 269, 242},
{270, 270, 270, 270, 279, 280, 281, 282, 275, 276, 277, 278, 271, 272, 273, 274},
{270, 270, 270, 270, 282, 289, 290, 291, 278, 286, 287, 288, 274, 283, 284, 285},
{270, 270, 270, 270, 291, 298, 299, 300, 288, 295, 296, 297, 285, 292, 293, 294},
{270, 270, 270, 270, 300, 305, 306, 279, 297, 303, 304, 275, 294, 301, 302, 271}};

static
void transpose(HorseRadish::Vector a[4][4])
{
	HorseRadish::Vector tt;

	tt=a[0][1];
	a[0][1]=a[1][0];
	a[1][0]=tt;

	tt=a[0][2];
	a[0][2]=a[2][0];
	a[2][0]=tt;

	tt=a[0][3];
	a[0][3]=a[3][0];
	a[3][0]=tt;

	tt=a[1][2];
	a[1][2]=a[2][1];
	a[2][1]=tt;

	tt=a[1][3];
	a[1][3]=a[3][1];
	a[3][1]=tt;

	tt=a[2][3];
	a[2][3]=a[3][2];
	a[3][2]=tt;
}

static
void divideCurve(const HorseRadish::Vector c[4], HorseRadish::Vector r[4], HorseRadish::Vector l[4])
{
	float t;

	l[0].x=c[0].x;
	r[3].x=c[3].x;
	l[1].x=(c[1].x+c[0].x)*0.5f;
	r[2].x=(c[2].x+c[3].x)*0.5f;
	t=(c[1].x+c[2].x)*0.5f;
	l[2].x=(t+l[1].x)*0.5f;
	r[1].x=(t+r[2].x)*0.5f;
	l[3].x=r[0].x=(l[2].x+r[1].x)*0.5f;

	l[0].y=c[0].y;
	r[3].y=c[3].y;
	l[1].y=(c[1].y+c[0].y)*0.5f;
	r[2].y=(c[2].y+c[3].y)*0.5f;
	t=(c[1].y+c[2].y)*0.5f;
	l[2].y=(t+l[1].y)*0.5f;
	r[1].y=(t+r[2].y)*0.5f;
	l[3].y=r[0].y=(l[2].y+r[1].y)*0.5f;

	l[0].z=c[0].z;
	r[3].z=c[3].z;
	l[1].z=(c[1].z+c[0].z)*0.5f;
	r[2].z=(c[2].z+c[3].z)*0.5f;
	t=(c[1].z+c[2].z)*0.5f;
	l[2].z=(t+l[1].z)*0.5f;
	r[1].z=(t+r[2].z)*0.5f;
	l[3].z=r[0].z=(l[2].z+r[1].z)*0.5f;
}

namespace HorseRadish
{
	namespace Geometry
	{
		void Model::getData(const Mesh * const mesh, const int attrib, const int element, float *const buffer)
		{
			if (Mesh::checkMesh(mesh) == false || buffer == nullptr || element < 0 || element >= mesh->numElements || attrib < 0 || attrib >= mesh->numAttrib)
				return;

			auto attribP = mesh->attrib + attrib;
			auto numComp = Mesh::calcComponents(attribP->attribType);

			int stride = mesh->strideAttrib / sizeof(float);
			if (stride == 0)
				stride = numComp;

			for (int i = 0; i < numComp; i++)
				buffer[i] = attribP->attribData[element*stride + i];
		}

		Model::Model()
		{
		}

		Model::Model(int numMesh)
		{
			if (numMesh > 0)
				arrayMesh.reserve(numMesh);
		}

		Model::~Model()
		{
		}

		Mesh* Model::Join() const
		{
			if (this->arrayMesh.size() <= 0)
				return nullptr;

			if (this->arrayMesh.size() == 1)
				return this->arrayMesh[0].mesh.Clone();

			auto numAttrib = this->arrayMesh[0].mesh.numAttrib;
			auto numVert = this->arrayMesh[0].mesh.numElements;
			auto numIndex = this->arrayMesh[0].mesh.numIndex;
			for (int i = 1; i < this->arrayMesh.size(); i++)
			{
				auto curMesh = &this->arrayMesh[i].mesh;

				numVert += curMesh->numElements;
				numIndex += curMesh->numIndex;

				if (curMesh->numAttrib != numAttrib)
					return nullptr;

				for (int j = 0; j < curMesh->numAttrib; j++)
				{
					if (curMesh->attrib[j].attribType != this->arrayMesh[0].mesh.attrib[j].attribType)
						return nullptr;
				}
			}

			auto numComponents = 0;
			auto curMesh = &this->arrayMesh[0].mesh;
			for (int i = 0; i < curMesh->numAttrib; i++)
				numComponents += Mesh::calcComponents(curMesh->attrib[i].attribType);

			auto novaMesh = new Mesh;

			memset(novaMesh, 0, sizeof(Mesh));

			novaMesh->numAttrib = numAttrib;
			novaMesh->numIndex = numIndex;
			novaMesh->strideAttrib = numComponents*sizeof(float);
			novaMesh->typeIndex = Mesh::Int32;
			novaMesh->numElements = numVert;

			novaMesh->pIndex = new unsigned int[novaMesh->numIndex];
			novaMesh->attrib = new Mesh::Attribute[novaMesh->numAttrib];
			auto bigBuffer = new float[novaMesh->numElements*numComponents];
			if (novaMesh->pIndex == nullptr || novaMesh->attrib == nullptr || bigBuffer == nullptr)
			{
				if (novaMesh->pIndex)
					delete[] novaMesh->pIndex;
				if (novaMesh->attrib)
					delete[] novaMesh->attrib;
				if (bigBuffer)
					delete[] bigBuffer;
				delete novaMesh;
				return nullptr;
			}

			auto walker = bigBuffer;
			for (const auto& curMesh : this->arrayMesh)
			{
				for (int j = 0; j < curMesh.mesh.numElements; j++)
				{
					for (int k = 0; k < curMesh.mesh.numAttrib; k++)
					{
						getData(&curMesh.mesh, k, j, walker);
						walker += Mesh::calcComponents(curMesh.mesh.attrib[k].attribType);
					}
				}
			}

			walker = bigBuffer;
			for (int i = 0; i < novaMesh->numAttrib; i++)
			{
				novaMesh->attrib[i].attribData = walker;
				novaMesh->attrib[i].attribType = this->arrayMesh[0].mesh.attrib[i].attribType;
				walker += Mesh::calcComponents(novaMesh->attrib[i].attribType);
			}

			auto walkerI = reinterpret_cast<unsigned int*>(novaMesh->pIndex);
			auto offsetIndex = 0;
			for (const auto& curMesh : this->arrayMesh)
			{
				if (curMesh.mesh.typeIndex == Mesh::Int16)
				{
					for (int j = 0; j < curMesh.mesh.numIndex; j++)
					{
						*walkerI = ((unsigned short *)curMesh.mesh.pIndex)[j];
						*walkerI += offsetIndex;
						walkerI++;
					}
					continue;
				}

				for (int j = 0; j < curMesh.mesh.numIndex; j++)
				{
					*walkerI = ((unsigned int *)curMesh.mesh.pIndex)[j];
					*walkerI += offsetIndex;
					walkerI++;
				}

				offsetIndex += curMesh.mesh.numElements;
			}

			return novaMesh;
		}

		Mesh* Factory::criaNovaMesh(const int numVert, const int numIndex)
		{
			if (numVert <= 0 || numIndex <= 0)
				return nullptr;

			auto mesh = new Mesh();
			if (mesh == nullptr)
				return nullptr;

			mesh->NewAttrib(Mesh::Pos, numVert);
			mesh->NewAttrib(Mesh::Normal, numVert);
			mesh->NewAttrib(Mesh::TexCoords, numVert);
			if (mesh->numAttrib != 3)
			{
				delete mesh;
				return nullptr;
			}

			mesh->numIndex = numIndex;
			mesh->typeIndex = Mesh::Int32;
			mesh->pIndex = new unsigned int[mesh->numIndex];
			if (mesh->pIndex == nullptr)
			{
				delete mesh;
				return nullptr;
			}

			return mesh;
		}

		int Factory::drawAroundPoint(Mesh * const mesh, int startPoint, const float xTexCoord, const float radius, const int precision, const float * const matrix)
		{
			float walk, jump, jTexCoord, texCoord[2], tmpX, tmpY, newPos[3];
			int i;

			texCoord[0] = xTexCoord;
			texCoord[1] = 0.0f;

			jump = Math::TWO_PI / ((float)precision);
			jTexCoord = 1.0f / ((float)precision);

			walk = 0.0f;
			for (i = 0; i < precision; walk += jump, texCoord[1] += jTexCoord, i++)
			{
				Math::sinCosR(walk, tmpY, tmpX, radius);
				newPos[0] = tmpX*matrix[0] + tmpY*matrix[4] + matrix[12];
				newPos[1] = tmpX*matrix[1] + tmpY*matrix[5] + matrix[13];
				newPos[2] = tmpX*matrix[2] + tmpY*matrix[6] + matrix[14];

				mesh->SetData(Mesh::Pos, startPoint, newPos);
				mesh->SetData(Mesh::TexCoords, startPoint, texCoord);
				startPoint++;
			}

			newPos[0] = radius*matrix[0] + matrix[12];
			newPos[1] = radius*matrix[1] + matrix[13];
			newPos[2] = radius*matrix[2] + matrix[14];
			mesh->SetData(Mesh::Pos, startPoint, newPos);
			mesh->SetData(Mesh::TexCoords, startPoint, texCoord);

			return (precision + 1);
		}

		void Factory::dividePatch(Mesh * const mesh, const HorseRadish::Vector p[4][4], const int n, int * const curVert)
		{
			if (n <= 0)
			{
				mesh->SetData(Mesh::Pos, *curVert, p[0][0]);	*curVert = *curVert + 1;
				mesh->SetData(Mesh::Pos, *curVert, p[3][0]);	*curVert = *curVert + 1;
				mesh->SetData(Mesh::Pos, *curVert, p[3][3]);	*curVert = *curVert + 1;
				mesh->SetData(Mesh::Pos, *curVert, p[0][3]);	*curVert = *curVert + 1;
				return;
			}

			HorseRadish::Vector q[4][4], r[4][4], s[4][4], t[4][4];
			HorseRadish::Vector a[4][4], b[4][4];

			divideCurve(p[0], a[0], b[0]);
			divideCurve(p[1], a[1], b[1]);
			divideCurve(p[2], a[2], b[2]);
			divideCurve(p[3], a[3], b[3]);

			transpose(a);
			transpose(b);

			divideCurve(a[0], q[0], r[0]);	divideCurve(b[0], s[0], t[0]);
			divideCurve(a[1], q[1], r[1]);	divideCurve(b[1], s[1], t[1]);
			divideCurve(a[2], q[2], r[2]);	divideCurve(b[2], s[2], t[2]);
			divideCurve(a[3], q[3], r[3]);	divideCurve(b[3], s[3], t[3]);

			dividePatch(mesh, q, n - 1, curVert);
			dividePatch(mesh, r, n - 1, curVert);
			dividePatch(mesh, s, n - 1, curVert);
			dividePatch(mesh, t, n - 1, curVert);
		}

		Mesh* Factory::CreateTorus(const float innerRadius, const float outerRadius, const int precision)
		{
			float ang, angRot, xTexC, jTexC, matrix[16];
			int i, curP;
			HorseRadish::Vector P1, P2, T, B, N;

			if (outerRadius <= 0.0f || innerRadius <= 0 || precision <= 2)
				return nullptr;

			jTexC = 1.0f / ((float)precision);
			angRot = Math::TWO_PI / ((float)precision);

			auto mesh = criaNovaMesh((precision + 1)*(precision + 1), (precision * 2 * 3)*precision);
			if (mesh == nullptr)
				return nullptr;

			curP = 0;
			for (i = 0, ang = xTexC = 0.0f; i <= precision; xTexC += jTexC, i++)
			{
				if (i == precision)
					ang = 0.0f;

				Math::sinCosR(ang, P1.z, P1.x, outerRadius);
				P1.y = 0.0;
				ang += angRot;

				Math::sinCosR(ang, P2.z, P2.x, outerRadius);
				P2.y = 0.0;

				T = P2;
				T -= P1;
				T.Normalize();

				N = P2;
				N += P1;
				N.Normalize();

				B.StoreCrossProduct(T, N);
				N.StoreCrossProduct(B, T);
				B.Normalize();
				N.Normalize();

				matrix[0] = N.x;
				matrix[1] = N.y;
				matrix[2] = N.z;
				matrix[4] = B.x;
				matrix[5] = B.y;
				matrix[6] = B.z;
				matrix[8] = T.x;
				matrix[9] = T.y;
				matrix[10] = T.z;
				matrix[12] = P1.x;
				matrix[13] = P1.y;
				matrix[14] = P1.z;

				curP += drawAroundPoint(mesh, curP, xTexC, innerRadius, precision, matrix);
			}

			auto walkerI = reinterpret_cast<unsigned int*>(mesh->pIndex);
			auto stride = precision + 1;
			for (i = 0; i < precision; i++)
			{
				for (int j = 0; j < precision; j++)
				{
					walkerI[0] = j + (i*stride);
					walkerI[1] = j + (i*stride) + 1;
					walkerI[2] = j + (i*stride) + stride;

					walkerI[3] = j + (i*stride) + 1;
					walkerI[4] = j + (i*stride) + stride + 1;
					walkerI[5] = j + (i*stride) + stride;

					walkerI += 6;
				}
			}

			mesh->Ortho((HorseRadish::Geometry::Mesh::MeshOrthoTypes)(HorseRadish::Geometry::Mesh::MeshOrthoCreateNormals | HorseRadish::Geometry::Mesh::MeshOrthoPerPositionNormal));
			return mesh;
		}

		Mesh* Factory::CreateDisk(const float radius, const int precision)
		{
			HorseRadish::Vector pos, normal, texCoord;
			float ang, offset;

			if (radius <= 0.0f || precision <= 2)
				return nullptr;

			auto mesh = criaNovaMesh(precision + 1, precision * 3);
			if (mesh == nullptr)
				return nullptr;

			for (int i = 0, index = 1; i < mesh->numIndex; i += 3, index++)
			{
				((unsigned int*)mesh->pIndex)[i + 0] = 0;
				((unsigned int*)mesh->pIndex)[i + 1] = index + 1;
				((unsigned int*)mesh->pIndex)[i + 2] = index;
			}

			((unsigned int*)mesh->pIndex)[mesh->numIndex - 2] = 1;

			ang = 0.0f;
			offset = 360.0f / ((float)precision);

			pos.Set(0.0f, 0.0f, 0.0f);
			normal.Set(0.0f, 1.0f, 0.0f);
			texCoord.Set(0.5f, 0.5f, 0.0f);
			mesh->SetData(Mesh::Pos, 0, pos);
			mesh->SetData(Mesh::Normal, 0, normal);
			mesh->SetData(Mesh::TexCoords, 0, texCoord);

			normal.Set(0.0f, 1.0f, 0.0f);

			for (int i = 1; i < mesh->numElements; i++, ang += offset)
			{
				Math::sinCosR(ang * 0.017453292519943295769236907, pos.z, pos.x, radius);

				Math::sinCosR(ang*0.017453292519943295769236907, texCoord.y, texCoord.x);
				texCoord.x = (texCoord.x + 1.0f)*0.5f;
				texCoord.y = (texCoord.y + 1.0f)*0.5f;

				mesh->SetData(Mesh::Pos, i, pos);
				mesh->SetData(Mesh::Normal, i, normal);
				mesh->SetData(Mesh::TexCoords, i, texCoord);
			}

			return mesh;
		}

		Mesh* Factory::CreateCone(const float radius, const float height, const int precision)
		{
			HorseRadish::Vector pos;
			float ang, offset, texCoord[2], texOffset;

			if (radius <= 0.0f || height <= 0.0f || precision <= 2)
				return nullptr;

			auto mesh = criaNovaMesh(precision * 2, precision * 3);
			if (mesh == nullptr)
				return nullptr;

			for (int i = 0, index = 0; i < mesh->numIndex; i += 3, index++)
			{
				((unsigned int*)mesh->pIndex)[i + 0] = precision + index;
				((unsigned int*)mesh->pIndex)[i + 1] = index + 1;
				((unsigned int*)mesh->pIndex)[i + 2] = index;
			}

			((unsigned int*)mesh->pIndex)[mesh->numIndex - 2] = 0;

			ang = 0.0f;
			offset = 360.0f / ((float)precision);
			texOffset = 1.0f / ((float)(precision - 1));

			texCoord[0] = 0.0f;
			texCoord[1] = 0.0f;
			for (int i = 0; i < precision; i++, ang += offset, texCoord[0] += texOffset)
			{
				pos.y = 0.0f;
				Math::sinCosR(ang * 0.017453292519943295769236907, pos.z, pos.x, radius);

				mesh->SetData(Mesh::Pos, i, pos);
				mesh->SetData(Mesh::TexCoords, i, texCoord);
			}

			pos.Set(0.0f, height, 0.0f);
			texCoord[0] = 0.0f;
			texCoord[1] = 1.0f;
			for (int i = precision; i < precision * 2; i++, texCoord[0] += texOffset)
			{
				mesh->SetData(Mesh::Pos, i, pos);
				mesh->SetData(Mesh::TexCoords, i, texCoord);
			}

			mesh->Ortho((HorseRadish::Geometry::Mesh::MeshOrthoTypes)(HorseRadish::Geometry::Mesh::MeshOrthoCreateNormals | HorseRadish::Geometry::Mesh::MeshOrthoPerPositionNormal));

			return mesh;
		}

		Mesh* Factory::CreateCylinder(const float topRadius, const float bottomRadius, const float height, const int precision)
		{
			float ang, offset, realHeight, texCoord[2], texOffset;
			HorseRadish::Vector pos;

			if (topRadius <= 0.0f || bottomRadius <= 0.0f || height <= 0.0f || precision <= 2)
				return nullptr;

			auto mesh = criaNovaMesh(precision * 2, precision * 2 * 3);
			if (mesh == nullptr)
				return nullptr;

			for (int i = 0, index = 0; i < mesh->numIndex; i += 6, index += 2)
			{
				((unsigned int*)mesh->pIndex)[i + 0] = index;
				((unsigned int*)mesh->pIndex)[i + 1] = index + 2;
				((unsigned int*)mesh->pIndex)[i + 2] = index + 1;

				((unsigned int*)mesh->pIndex)[i + 3] = index + 3;
				((unsigned int*)mesh->pIndex)[i + 4] = index + 1;
				((unsigned int*)mesh->pIndex)[i + 5] = index + 2;
			}

			((unsigned int*)mesh->pIndex)[mesh->numIndex - 5] = 0;
			((unsigned int*)mesh->pIndex)[mesh->numIndex - 3] = 1;
			((unsigned int*)mesh->pIndex)[mesh->numIndex - 1] = 0;

			ang = 0.0f;
			offset = 360.0f / ((float)precision);
			texOffset = 1.0f / ((float)(precision - 1));

			realHeight = height*0.5f;
			texCoord[0] = 1.0f;
			for (int i = 0; i < precision * 2; i += 2, ang += offset, texCoord[0] -= texOffset)
			{
				pos.y = realHeight;
				Math::sinCosR(ang * 0.017453292519943295769236907, pos.z, pos.x, topRadius);
				texCoord[1] = 1.0f;
				mesh->SetData(Mesh::Pos, i, pos);
				mesh->SetData(Mesh::TexCoords, i, texCoord);

				pos.y = -realHeight;
				Math::sinCosR(ang * 0.017453292519943295769236907, pos.z, pos.x, bottomRadius);
				texCoord[1] = 0.0f;
				mesh->SetData(Mesh::Pos, i + 1, pos);
				mesh->SetData(Mesh::TexCoords, i + 1, texCoord);
			}

			mesh->Ortho((HorseRadish::Geometry::Mesh::MeshOrthoTypes)(HorseRadish::Geometry::Mesh::MeshOrthoCreateNormals | HorseRadish::Geometry::Mesh::MeshOrthoPerPositionNormal));

			return mesh;
		}

		Mesh* Factory::CreateBox(const float width, const float height, const float depth, const int precision)
		{
			float realWidth, realHeight, realDepth, offsetW, offsetH, offsetD, offsetTex;
			int i, j, index, jump, jump2;

			if (width <= 0.0f || height <= 0.0f || depth <= 0.0f || precision <= 0)
				return nullptr;

			auto mesh = criaNovaMesh((2 + precision - 1)*(precision + 1) * 6, precision*precision * 6 * 6);
			if (mesh == nullptr)
				return nullptr;

			for (i = 0, index = jump = jump2 = 0; i < mesh->numIndex; i += 6)
			{
				((unsigned int*)mesh->pIndex)[i + 0] = index;
				((unsigned int*)mesh->pIndex)[i + 1] = index + 1;
				((unsigned int*)mesh->pIndex)[i + 2] = index + precision + 1;

				((unsigned int*)mesh->pIndex)[i + 3] = index + 1;
				((unsigned int*)mesh->pIndex)[i + 4] = index + precision + 2;
				((unsigned int*)mesh->pIndex)[i + 5] = index + precision + 1;

				index++;
				jump++;
				if (jump == precision)
				{
					index++;
					jump = 0;
					jump2++;
					if (jump2 == precision)
					{
						index += precision + 1;
						jump2 = 0;
					}
				}
			}

			realWidth = width*0.5f;
			realHeight = height*0.5f;
			realDepth = depth*0.5f;
			offsetW = width / ((float)precision);
			offsetH = height / ((float)precision);
			offsetD = depth / ((float)precision);
			offsetTex = 1.0f / ((float)precision);
			auto walker = mesh->FindAttribData(Mesh::Pos);
			auto walkerT = mesh->FindAttribData(Mesh::TexCoords);
			auto walkerN = mesh->FindAttribData(Mesh::Normal);

			for (i = 0; i < (precision + 1); i++)
			{
				for (j = 0; j < (2 + precision - 1); j++, walker += 3, walkerT += 2, walkerN += 3)
				{
					walker[0] = offsetW*((float)j) - realWidth;
					walker[1] = offsetH*((float)i) - realHeight;
					walker[2] = realDepth;

					walkerT[0] = offsetTex*((float)j);
					walkerT[1] = offsetTex*((float)i);

					walkerN[0] = walkerN[1] = 0.0f;
					walkerN[2] = 1.0f;
				}
			}

			for (i = 0; i < (precision + 1); i++)
			{
				for (j = 0; j < (2 + precision - 1); j++, walker += 3, walkerT += 2, walkerN += 3)
				{
					walker[0] = realWidth;
					walker[1] = offsetH*((float)i) - realHeight;
					walker[2] = realDepth - offsetD*((float)j);

					walkerT[0] = offsetTex*((float)j);
					walkerT[1] = offsetTex*((float)i);

					walkerN[1] = walkerN[2] = 0.0f;
					walkerN[0] = 1.0f;
				}
			}

			for (i = 0; i < (precision + 1); i++)
			{
				for (j = 0; j < (2 + precision - 1); j++, walker += 3, walkerT += 2, walkerN += 3)
				{
					walker[0] = realWidth - offsetW*((float)j);
					walker[1] = offsetH*((float)i) - realHeight;
					walker[2] = -realDepth;

					walkerT[0] = offsetTex*((float)j);
					walkerT[1] = offsetTex*((float)i);

					walkerN[0] = walkerN[1] = 0.0f;
					walkerN[2] = -1.0f;
				}
			}

			for (i = 0; i < (precision + 1); i++)
			{
				for (j = 0; j < (2 + precision - 1); j++, walker += 3, walkerT += 2, walkerN += 3)
				{
					walker[0] = -realWidth;
					walker[1] = offsetH*((float)i) - realHeight;
					walker[2] = offsetD*((float)j) - realDepth;

					walkerT[0] = offsetTex*((float)j);
					walkerT[1] = offsetTex*((float)i);

					walkerN[1] = walkerN[2] = 0.0f;
					walkerN[0] = -1.0f;
				}
			}

			for (i = 0; i < (precision + 1); i++)
			{
				for (j = 0; j < (2 + precision - 1); j++, walker += 3, walkerT += 2, walkerN += 3)
				{
					walker[0] = offsetW*((float)j) - realWidth;
					walker[1] = realHeight;
					walker[2] = realDepth - offsetD*((float)i);

					walkerT[0] = offsetTex*((float)j);
					walkerT[1] = offsetTex*((float)i);

					walkerN[0] = walkerN[2] = 0.0f;
					walkerN[1] = 1.0f;
				}
			}

			for (i = 0; i < (precision + 1); i++)
			{
				for (j = 0; j < (2 + precision - 1); j++, walker += 3, walkerT += 2, walkerN += 3)
				{
					walker[0] = offsetW*((float)j) - realWidth;
					walker[1] = -realHeight;
					walker[2] = offsetD*((float)i) - realDepth;

					walkerT[0] = offsetTex*((float)j);
					walkerT[1] = offsetTex*((float)i);

					walkerN[0] = walkerN[2] = 0.0f;
					walkerN[1] = -1.0f;
				}
			}

			return mesh;
		}

		Mesh* Factory::CreatePlane(const float width, const float height, const int precision)
		{
			if (width <= 0.0f || height <= 0.0f || precision <= 0)
				return nullptr;

			auto mesh = criaNovaMesh((precision + 1)*(precision + 1), precision*precision * 6);
			if (mesh == nullptr)
				return nullptr;

			for (int i = 0, index = 0, jump = 0; i < mesh->numIndex; i += 6)
			{
				((unsigned int*)mesh->pIndex)[i + 0] = index;
				((unsigned int*)mesh->pIndex)[i + 1] = index + 1;
				((unsigned int*)mesh->pIndex)[i + 2] = index + precision + 1;

				((unsigned int*)mesh->pIndex)[i + 3] = index + 1;
				((unsigned int*)mesh->pIndex)[i + 4] = index + precision + 2;
				((unsigned int*)mesh->pIndex)[i + 5] = index + precision + 1;

				index++;
				jump++;
				if (jump == precision)
				{
					index++;
					jump = 0;
				}
			}

			float realWidth = width*0.5f;
			float realHeight = height*0.5f;
			float offsetW = width / static_cast<float>(precision);
			float offsetH = height / static_cast<float>(precision);
			float offsetTex = 1.0f / static_cast<float>(precision);

			auto walker = mesh->FindAttribData(Mesh::Pos);
			auto walkerT = mesh->FindAttribData(Mesh::TexCoords);
			auto walkerN = mesh->FindAttribData(Mesh::Normal);

			for (int i = 0; i <= precision; i++)
			{
				for (int j = 0; j <= precision; j++, walker += 3, walkerT += 2, walkerN += 3)
				{
					walker[0] = offsetW * static_cast<float>(j) - realWidth;
					walker[1] = offsetH * static_cast<float>(i) - realHeight;
					walker[2] = 0.0f;

					walkerT[0] = offsetTex * static_cast<float>(j);
					walkerT[1] = offsetTex * static_cast<float>(i);

					walkerN[0] = walkerN[1] = 0.0f;
					walkerN[2] = 1.0f;
				}
			}

			return mesh;
		}

		Mesh* Factory::CreateSphere(const float radius, const int slices, const int stacks)
		{
			float rho, drho, theta, dtheta, s, t, ds, dt, sinRho, cosRho;
			int i, j, index, last;
			HorseRadish::Vector calc, tex;

			if (radius <= 0.0f || slices <= 2 || stacks <= 2)
				return nullptr;

			auto mesh = criaNovaMesh((stacks - 1)*slices + 2, (stacks - 2)*slices * 2 * 3 + slices * 2 * 3);
			if (mesh == nullptr)
				return nullptr;

			ds = 1.0f / ((float)slices);
			dt = 1.0f / ((float)stacks);
			t = 1.0f;
			drho = Math::PI / (float)stacks;
			dtheta = Math::TWO_PI / (float)(slices - 1);

			auto pIndeces = reinterpret_cast<unsigned int*>(mesh->pIndex);

			for (i = index = 0; i <= stacks; i++, t -= dt)
			{
				rho = ((float)i) * drho;
				s = 0.0f;

				if (i == 0)
				{
					calc.x = calc.z = 0.0f;
					calc.y = 1.0f;
					tex.x = 0.5;
					tex.y = 1.0;

					mesh->SetData(Mesh::Normal, index, calc);
					calc *= radius;
					mesh->SetData(Mesh::Pos, index, calc);
					mesh->SetData(Mesh::TexCoords, index, tex);
					index++;
					continue;
				}

				if (i == stacks)
				{
					calc.x = calc.z = 0.0f;
					calc.y = -1.0f;
					tex.x = 0.5;
					tex.y = 0.0;

					mesh->SetData(Mesh::Normal, index, calc);
					calc *= radius;
					mesh->SetData(Mesh::Pos, index, calc);
					mesh->SetData(Mesh::TexCoords, index, tex);
					index++;
					continue;
				}

				Math::sinCosR(rho, sinRho, cosRho);

				for (j = 0; j < slices; j++, s += ds)
				{
					theta = ((float)j) * dtheta;

					Math::sinCosR(theta, calc.x, calc.z);
					calc.x *= -sinRho;
					calc.z *= sinRho;
					calc.y = cosRho;

					tex.x = s;
					tex.y = t;

					mesh->SetData(Mesh::Normal, index, calc);
					calc *= radius;
					mesh->SetData(Mesh::Pos, index, calc);
					mesh->SetData(Mesh::TexCoords, index, tex);
					index++;
				}
			}

			index = 1;
			for (j = 0; j < (slices - 1); j++, index++)
			{
				*(pIndeces++) = 0;
				*(pIndeces++) = index + 1;
				*(pIndeces++) = index;
			}
			*(pIndeces++) = 0;
			*(pIndeces++) = 1;
			*(pIndeces++) = index;

			index = 1;
			for (i = 0; i < (stacks - 2); i++)
			{
				for (j = 0; j < (slices - 1); j++, index++)
				{
					*(pIndeces++) = index;
					*(pIndeces++) = index + slices + 1;
					*(pIndeces++) = index + slices;

					*(pIndeces++) = index;
					*(pIndeces++) = index + 1;
					*(pIndeces++) = index + slices + 1;
				}

				*(pIndeces++) = index;
				*(pIndeces++) = (i*slices + 1) + slices;
				*(pIndeces++) = index + slices;

				*(pIndeces++) = index;
				*(pIndeces++) = (i*slices + 1);
				*(pIndeces++) = (i*slices + 1) + slices;
				index++;
			}

			last = (stacks - 1)*slices + 1;
			for (j = 0; j < (slices - 1); j++, index++)
			{
				*(pIndeces++) = index;
				*(pIndeces++) = index + 1;
				*(pIndeces++) = last;
			}
			*(pIndeces++) = index;
			*(pIndeces++) = (i*slices + 1);
			*(pIndeces++) = last;

			return mesh;
		}

		Mesh* Factory::CreateParabloid(const float xRadius, const float yRadius, const float zRadius, const int slices, const int stacks)
		{
			int stride, i;

			if (xRadius <= 0.0f || yRadius <= 0.0f || zRadius <= 0.0f || slices <= 2 || stacks <= 2)
				return nullptr;

			auto mesh = CreateSphere(1.0, slices, stacks);

			auto walkerP = mesh->FindAttribData(Mesh::Pos);
			stride = 3;
			if (mesh->strideAttrib != 0)
				stride = mesh->strideAttrib / sizeof(float);

			for (i = 0; i < mesh->numElements; i++, walkerP += stride)
			{
				walkerP[0] *= xRadius;
				walkerP[1] *= yRadius;
				walkerP[2] *= zRadius;
			}

			mesh->Ortho((HorseRadish::Geometry::Mesh::MeshOrthoTypes)(HorseRadish::Geometry::Mesh::MeshOrthoCreateNormals | HorseRadish::Geometry::Mesh::MeshOrthoPerPositionNormal));
			return mesh;
		}

		Mesh* Factory::CreateTorusKnot(const int turns, const float innerRadius, const float outRadius, const int innerPrecision, const int outPrecision)
		{
			float jWalk, jOut, xTexC, jTexC, matrix[16], wOutTurns, wOut2x, auxCalc;
			int i, j, curP, stride;
			HorseRadish::Vector P1, P2, T, B, N;

			if (turns <= 0 || outRadius <= 0.0f || innerRadius <= 0 || outPrecision <= 2 || innerPrecision <= 2)
				return nullptr;

			jOut = Math::TWO_PI / ((float)outPrecision);
			jTexC = 1.0f / ((float)outPrecision);

			auto mesh = criaNovaMesh((outPrecision + 1)*(innerPrecision + 1), (innerPrecision * 2 * 3)*outPrecision);
			if (mesh == nullptr)
				return nullptr;

			curP = 0;
			for (i = 0, xTexC = jWalk = 0.0f; i <= outPrecision; i++, xTexC += jTexC, jWalk += jOut)
			{
				if (i == outPrecision)
					jWalk = 0.0f;

				wOutTurns = float(turns)*jWalk;
				wOut2x = 2.0f*jWalk;
				auxCalc = 1.0 + 0.3*cos(wOutTurns);
				P1.x = auxCalc*cos(wOut2x);
				P1.y = auxCalc*sin(wOutTurns)*0.3;
				P1.z = auxCalc*sin(wOut2x);
				P1 *= outRadius;

				wOutTurns = float(turns)*(jWalk + jOut);
				wOut2x = 2.0f*(jWalk + jOut);
				auxCalc = 1.0 + 0.3*cos(wOutTurns);
				P2.x = auxCalc*cos(wOut2x);
				P2.y = auxCalc*sin(wOutTurns)*0.3;
				P2.z = auxCalc*sin(wOut2x);
				P2 *= outRadius;
				if ((i + 1) == outPrecision)
				{
					P2.x = 1.3*outRadius;
					P2.y = P2.z = 0.0f;
				}

				T = P2;
				T -= P1;
				T.Normalize();

				N = P2;
				N += P1;
				N.Normalize();

				B.StoreCrossProduct(T, N);
				N.StoreCrossProduct(B, T);
				B.Normalize();
				N.Normalize();

				matrix[0] = N.x;
				matrix[1] = N.y;
				matrix[2] = N.z;
				matrix[4] = B.x;
				matrix[5] = B.y;
				matrix[6] = B.z;
				matrix[8] = T.x;
				matrix[9] = T.y;
				matrix[10] = T.z;
				matrix[12] = P1.x;
				matrix[13] = P1.y;
				matrix[14] = P1.z;

				curP += drawAroundPoint(mesh, curP, xTexC, innerRadius, innerPrecision, matrix);
			}

			auto walkerI = reinterpret_cast<unsigned int*>(mesh->pIndex);
			stride = innerPrecision + 1;
			for (i = 0; i < outPrecision; i++)
			{
				for (j = 0; j < innerPrecision; j++)
				{
					walkerI[0] = j + (i*stride);
					walkerI[1] = j + (i*stride) + 1;
					walkerI[2] = j + (i*stride) + stride;

					walkerI[3] = j + (i*stride) + 1;
					walkerI[4] = j + (i*stride) + stride + 1;
					walkerI[5] = j + (i*stride) + stride;

					walkerI += 6;
				}
			}

			mesh->Ortho((HorseRadish::Geometry::Mesh::MeshOrthoTypes)(HorseRadish::Geometry::Mesh::MeshOrthoCreateNormals | HorseRadish::Geometry::Mesh::MeshOrthoPerPositionNormal));
			return mesh;
		}

		Mesh* Factory::CreatePQTorus(const int P, const int Q, const float innerRadius, const float outRadius, const int innerPrecision, const int outPrecision)
		{
			float r, wOut, jOut, xTexC, jTexC, matrix[16];
			int i, j, curP, stride;
			HorseRadish::Vector P1, P2, T, B, N;

			if (P <= 0 || Q <= 0 || outRadius <= 0.0f || innerRadius <= 0 || outPrecision <= 2 || innerPrecision <= 2)
				return nullptr;

			jOut = Math::TWO_PI / ((float)outPrecision);
			jTexC = 1.0f / ((float)outPrecision);

			auto mesh = criaNovaMesh((outPrecision + 1)*(innerPrecision + 1), (innerPrecision * 2 * 3)*outPrecision);
			if (mesh == nullptr)
				return nullptr;

			curP = 0;
			for (i = 0, wOut = xTexC = 0.0f; i <= outPrecision; wOut += jOut, xTexC += jTexC, i++)
			{
				if (i == outPrecision)
					wOut = 0.0f;

				r = 0.5f * (2.0f + sin(((float)Q) * wOut));
				P1.x = r * cos(((float)P) * wOut) * outRadius;
				P1.y = r * cos(((float)Q) * wOut) * outRadius;
				P1.z = r * sin(((float)P) * wOut) * outRadius;

				r = 0.5f * (2.0f + sin(((float)Q) * (wOut + jOut)));
				P2.x = r * cos(((float)P) * (wOut + jOut)) * outRadius;
				P2.y = r * cos(((float)Q) * (wOut + jOut)) * outRadius;
				P2.z = r * sin(((float)P) * (wOut + jOut)) * outRadius;
				if ((i + 1) == outPrecision)
				{
					P2.x = P2.y = outRadius;
					P2.z = 0.0f;
				}

				T = P2;
				T -= P1;
				T.Normalize();

				N = P2;
				N += P1;
				N.Normalize();

				B.StoreCrossProduct(T, N);
				N.StoreCrossProduct(B, T);
				B.Normalize();
				N.Normalize();

				matrix[0] = N.x;
				matrix[1] = N.y;
				matrix[2] = N.z;
				matrix[4] = B.x;
				matrix[5] = B.y;
				matrix[6] = B.z;
				matrix[8] = T.x;
				matrix[9] = T.y;
				matrix[10] = T.z;
				matrix[12] = P1.x;
				matrix[13] = P1.y;
				matrix[14] = P1.z;

				curP += drawAroundPoint(mesh, curP, xTexC, innerRadius, innerPrecision, matrix);
			}

			auto walkerI = reinterpret_cast<unsigned int*>(mesh->pIndex);
			stride = innerPrecision + 1;
			for (i = 0; i < outPrecision; i++)
			{
				for (j = 0; j < innerPrecision; j++)
				{
					walkerI[0] = j + (i*stride);
					walkerI[1] = j + (i*stride) + 1;
					walkerI[2] = j + (i*stride) + stride;

					walkerI[3] = j + (i*stride) + 1;
					walkerI[4] = j + (i*stride) + stride + 1;
					walkerI[5] = j + (i*stride) + stride;

					walkerI += 6;
				}
			}

			mesh->Ortho((HorseRadish::Geometry::Mesh::MeshOrthoTypes)(HorseRadish::Geometry::Mesh::MeshOrthoCreateNormals | HorseRadish::Geometry::Mesh::MeshOrthoPerPositionNormal));
			return mesh;
		}

		Mesh* Factory::CreateTeapot(const float radius, const int precision)
		{
			HorseRadish::Vector data[32][4][4];
			int curVert;

			if (radius <= 0.0f || precision < 0)
				return nullptr;

			auto mesh = criaNovaMesh(32 * pow(4.0f, precision) * 4, 32 * pow(4.0f, precision) * 6);
			if (mesh == nullptr)
				return nullptr;

			for (int i = 0; i < 32; i++)
			{
				for (int j = 0; j < 4; j++)
				{
					for (int k = 0; k < 4; k++)
					{
						curVert = indexTP[i][j][k] - 1;
						data[i][j][k].x = vertTP[curVert][0];
						data[i][j][k].y = vertTP[curVert][1];
						data[i][j][k].z = vertTP[curVert][2];
					}
				}
			}

			curVert = 0;
			for (int i = 0; i < 32; i++)
				dividePatch(mesh, data[i], precision, &curVert);

			auto walker = reinterpret_cast<unsigned int*>(mesh->pIndex);
			auto meta = walker + mesh->numIndex;
			curVert = 0;

			if (precision % 2 == 0)
			{
				while (walker < meta)
				{
					*(walker++) = curVert + 2;	*(walker++) = curVert + 1;		*(walker++) = curVert + 0;
					*(walker++) = curVert + 3;	*(walker++) = curVert + 2;		*(walker++) = curVert + 0;
					curVert += 4;
				}
			}
			else
			{
				while (walker < meta)
				{
					*(walker++) = curVert + 0;	*(walker++) = curVert + 1;		*(walker++) = curVert + 2;
					*(walker++) = curVert + 0;	*(walker++) = curVert + 2;		*(walker++) = curVert + 3;
					curVert += 4;
				}
			}

			mesh->RemoveDuplicate(Mesh::Pos);
			mesh->CenterMass(0.0f, 0.0f, 0.0f);
			mesh->MaxBBox(radius);
			mesh->Ortho((HorseRadish::Geometry::Mesh::MeshOrthoTypes)(HorseRadish::Geometry::Mesh::MeshOrthoCreateNormals | HorseRadish::Geometry::Mesh::MeshOrthoPerPositionNormal));
			return mesh;
		}

		Model* Factory::Read3DS(HorseRadish::Streams::StreamReader &streamReader)
		{
			return mfRead3DS(streamReader);
		}

		Model* Factory::ReadOBJ(HorseRadish::Streams::StreamReader &streamReader)
		{
			return mfReadOBJ(streamReader);
		}

	} //Geometry
} //HorseRadish