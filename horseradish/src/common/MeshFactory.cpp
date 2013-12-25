#include "Math.hpp"
#include "Vector.hpp"
#include "MeshFactory.hpp"
#include "meshFactory\mf.hpp"

#include <string.h>

///§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§
//§§§§§§   -= Dados auxiliares para construir o TeaPot =-   §§§§§
//§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§

//vertices para o Teapot
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
void divide_curve(const HorseRadish::Vector c[4], HorseRadish::Vector r[4], HorseRadish::Vector l[4])
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

///§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§
//§§§§§§   -= Implementação da Factory =-   §§§§§
//§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§

namespace HorseRadish
{
namespace Geometry
{

void Model::getData(const Mesh * const mesh, const int attrib, const int element, float *const buffer)
{
	Mesh::Attribute *attribP;
	int i,stride,numComp;

	//verificar este parametro
	if (Mesh::checkMesh(mesh)==false || buffer==nullptr || element<0 || element>=mesh->numElements || attrib<0 || attrib>=mesh->numAttrib)
		return;

	//o numero de componentes deste atributo
	attribP=mesh->attrib+attrib;
	numComp=Mesh::calcComponents(attribP->attribType);

	//arranjar o stride
	stride=mesh->strideAttrib/sizeof(float);
	if (stride==0)
		stride=numComp;

	//copio
	for(i=0; i<numComp; i++)
		buffer[i]=attribP->attribData[element*stride+i];
}

Model::Model()
{
	//não há nada a fazer
}

Model::Model(int numMesh) : arrayMesh(numMesh)
{
	//não há nada a fazer
}

Model::~Model()
{
	//não há nada a fazer
}

Mesh* Model::Join() const
{
	unsigned int *walkerI;
	Mesh *novaMesh,*curMesh;
	float *bigBuffer,*walker;
	int numAttrib,numVert,numIndex,numComponents,offsetIndex;

	//pursopuesto
	if (this->arrayMesh.GetNumElements() <= 0)
		return nullptr;

	//se tiver só uma superficies, simplifico as coisas
	if (this->arrayMesh.GetNumElements() == 1)
		return this->arrayMesh[0].mesh.Clone();

	//todas as meshes têm de ter os mesmo atributos e começo a contar numero total de vertices
	//e tambem de indices
	numAttrib=this->arrayMesh[0].mesh.numAttrib;
	numVert=this->arrayMesh[0].mesh.numElements;
	numIndex=this->arrayMesh[0].mesh.numIndex;
	for(int i=1; i<this->arrayMesh.GetNumElements(); i++)
	{
		//esta mesh
		curMesh=&this->arrayMesh[i].mesh;

		//conto vertices e indices
		numVert+=curMesh->numElements;
		numIndex+=curMesh->numIndex;

		//têm de ter o mesmo numero
		if (curMesh->numAttrib!=numAttrib)
			return nullptr;

		//e os mesmos tipos de atributos (comparo sempre com o primeiro)
		for(int j=0; j<curMesh->numAttrib; j++)
		{
			//se forem diferentes, à chatice! toca a sair cheio de raiva
			if (curMesh->attrib[j].attribType!=this->arrayMesh[0].mesh.attrib[j].attribType)
				return nullptr;
		}
	}

	//quero contar quantos compoentes existem então
	numComponents=0;
	curMesh=&this->arrayMesh[0].mesh;
	for(int i=0; i<curMesh->numAttrib; i++)
		numComponents+=Mesh::calcComponents(curMesh->attrib[i].attribType); 

	//pronto, chegando aqui, todas as superficies são iguais.... :D
	//e fica tudo num grande buffer
	novaMesh = new Mesh;

	//crio espaco para as cenas
	memset(novaMesh, 0, sizeof(Mesh));

	novaMesh->numAttrib=numAttrib;
	novaMesh->numIndex=numIndex;
	novaMesh->strideAttrib=numComponents*sizeof(float);
	novaMesh->typeIndex=Mesh::Int32;
	novaMesh->numElements=numVert;

	novaMesh->pIndex=new unsigned int[novaMesh->numIndex];
	novaMesh->attrib=new Mesh::Attribute[novaMesh->numAttrib];
	bigBuffer=new float[novaMesh->numElements*numComponents];
	if (novaMesh->pIndex==nullptr || novaMesh->attrib==nullptr || bigBuffer==nullptr)
	{
		if (novaMesh->pIndex)
			delete [] novaMesh->pIndex;
		if (novaMesh->attrib)
			delete [] novaMesh->attrib;
		if (bigBuffer)
			delete [] bigBuffer;
		delete novaMesh;
		return nullptr;
	}

	//que maravilha! :D Em principio já nada dá erro. Vou arranjar os dados
	walker=bigBuffer;
	for(int i=0; i<this->arrayMesh.GetNumElements(); i++)
	{
		//para esta mesh
		curMesh=&this->arrayMesh[i].mesh;

		//para cada atributo vertice
		for(int j=0; j<curMesh->numElements; j++)
		{
			//e finalmente para cada atributo
			for(int k=0; k<curMesh->numAttrib; k++)
			{
				getData(curMesh,k,j,walker);
				walker += Mesh::calcComponents(curMesh->attrib[k].attribType);
			}
		}
	}

	//e pronto, tá tudo copiado. Vou arranjar os ponteiros dos atributos
	walker=bigBuffer;
	for(int i=0; i<novaMesh->numAttrib; i++)
	{
		novaMesh->attrib[i].attribData=walker;
		novaMesh->attrib[i].attribType=this->arrayMesh[0].mesh.attrib[i].attribType;
		walker += Mesh::calcComponents(novaMesh->attrib[i].attribType);
	}

	//e falta finalmente os indices
	walkerI=(unsigned int*)novaMesh->pIndex;
	offsetIndex=0;
	for(int i=0; i<this->arrayMesh.GetNumElements(); i++)
	{
		//para esta mesh
		curMesh=&this->arrayMesh[i].mesh;

		//para cada atributo vertice, tipo short
		if (curMesh->typeIndex==Mesh::Int16)
		{
			for(int j=0; j<curMesh->numIndex; j++)
			{
				*walkerI=((unsigned short *)curMesh->pIndex)[j];
				*walkerI+=offsetIndex;
				walkerI++;
			}
			continue;
		}

		//para cada atributo vertice, tipo int
		for(int j=0; j<curMesh->numIndex; j++)
		{
			*walkerI=((unsigned int *)curMesh->pIndex)[j];
			*walkerI+=offsetIndex;
			walkerI++;
		}

		//a partir daqui, os pontos sao outros
		offsetIndex+=curMesh->numElements;
	}

	//e já está
	return novaMesh;
}

}//namespace Geometry
}//namespace HorseRadish

///§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§
//§§§§§§   -= Implementação da Factory =-   §§§§§
//§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§

namespace HorseRadish
{
namespace Geometry
{

Mesh* Factory::criaNovaMesh(const int numVert, const int numIndex)
{
	Mesh *mesh;

	//verifica
	if (numVert<=0 || numIndex<=0)
		return nullptr;

	//crio a mesh
	mesh = new Mesh();
	if (mesh == nullptr)
		return nullptr;

	//crio os atributos
	mesh->NewAttrib(Mesh::Pos, numVert);
	mesh->NewAttrib(Mesh::Normal, numVert);
	mesh->NewAttrib(Mesh::TexCoords, numVert);
	if (mesh->numAttrib != 3)
	{
		delete mesh;
		return nullptr;
	}

	//os indices
	mesh->numIndex = numIndex;
	mesh->typeIndex = Mesh::Int32;
	mesh->pIndex = new unsigned int[mesh->numIndex];
	if (mesh->pIndex == nullptr)
	{
		delete mesh;
		return nullptr;
	}

	//já tá
	return mesh;
}

int Factory::drawAroundPoint(Mesh * const mesh, int startPoint, const float xTexCoord, const float radius, const int precision, const float * const matrix)
{
	float walk,jump,jTexCoord,texCoord[2],tmpX,tmpY,newPos[3];
	int i;

	texCoord[0]=xTexCoord;
	texCoord[1]=0.0f;

	jump = Math::TWO_PI /((float)precision);
	jTexCoord = 1.0f/((float)precision);

	walk=0.0f;
	for(i=0; i<precision; walk+=jump,texCoord[1]+=jTexCoord,i++)
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

	return (precision+1);
}

void Factory::divide_patch(Mesh * const mesh, const HorseRadish::Vector p[4][4], const int n, int * const curVert)
{
	//nao é preciso subdividir mais
	if(n<=0)
	{
		//desenho e ponho-me na alheta
		mesh->SetData(Mesh::Pos, *curVert,p[0][0]);	*curVert=*curVert+1;
		mesh->SetData(Mesh::Pos, *curVert,p[3][0]);	*curVert=*curVert+1;
		mesh->SetData(Mesh::Pos, *curVert,p[3][3]);	*curVert=*curVert+1;
		mesh->SetData(Mesh::Pos, *curVert,p[0][3]);	*curVert=*curVert+1;
		return;
	}

	//here we go
	HorseRadish::Vector q[4][4], r[4][4], s[4][4], t[4][4];
	HorseRadish::Vector a[4][4], b[4][4];

	/* subdivide curves in u direction, transpose results, divide
	in u direction again (equivalent to subdivision in v) */
	divide_curve(p[0], a[0], b[0]);
	divide_curve(p[1], a[1], b[1]);
	divide_curve(p[2], a[2], b[2]);
	divide_curve(p[3], a[3], b[3]);

	transpose(a);
	transpose(b);

	divide_curve(a[0], q[0], r[0]);		divide_curve(b[0], s[0], t[0]);
	divide_curve(a[1], q[1], r[1]);		divide_curve(b[1], s[1], t[1]);
	divide_curve(a[2], q[2], r[2]);		divide_curve(b[2], s[2], t[2]);
	divide_curve(a[3], q[3], r[3]);		divide_curve(b[3], s[3], t[3]);

	//toca a recursivamente fazer o mesmo
	divide_patch(mesh,q, n-1,curVert);
	divide_patch(mesh,r, n-1,curVert);
	divide_patch(mesh,s, n-1,curVert);
	divide_patch(mesh,t, n-1,curVert);
}

Mesh* Factory::CreateTorus(const float innerRadius, const float outerRadius, const int precision)
{
	float ang,angRot,xTexC,jTexC,matrix[16];
	int i,j,curP,stride;
	unsigned int *walkerI;
	HorseRadish::Vector P1,P2,T,B,N;
	Mesh* mesh;

	//opsy daisies
	if (outerRadius<=0.0f || innerRadius<=0 || precision<=2)
		return nullptr;

	//como avanço
	jTexC=1.0f/((float)precision);
	angRot=Math::TWO_PI/((float)precision);

	//crio a mesh
	mesh=criaNovaMesh((precision+1)*(precision+1),(precision*2*3)*precision);
	if (mesh==nullptr)
		return nullptr;
	
	//crio os pontos
	curP=0;
	for(i=0,ang=xTexC=0.0f; i<=precision; xTexC+=jTexC,i++)
	{
		if (i==precision)
			ang=0.0f;

		Math::sinCosR(ang, P1.z, P1.x, outerRadius);
		P1.y=0.0;
		ang+=angRot;

		Math::sinCosR(ang, P2.z, P2.x, outerRadius);
		P2.y=0.0;

		T = P2;
		T-= P1;
		T.Normaliza();

		N = P2;
		N+= P1;
		N.Normaliza();

		B.CalcCrossProduct(T,N);
		N.CalcCrossProduct(B,T);
		B.Normaliza();
		N.Normaliza();

		matrix[0]=N.x;
		matrix[1]=N.y;
		matrix[2]=N.z;
		matrix[4]=B.x;
		matrix[5]=B.y;
		matrix[6]=B.z;
		matrix[8]=T.x;
		matrix[9]=T.y;
		matrix[10]=T.z;
		matrix[12]=P1.x;
		matrix[13]=P1.y;
		matrix[14]=P1.z;

		curP+=drawAroundPoint(mesh,curP,xTexC,innerRadius,precision,matrix);
	}

	//agora crio os indices excluindo o ultimo arco
	walkerI=(unsigned int*)mesh->pIndex;
	stride=precision+1;
	for(i=0; i<precision; i++)
	{
		//para cada ponto do arco
		for(j=0; j<precision; j++)
		{
			walkerI[0]=j+(i*stride);
			walkerI[1]=j+(i*stride)+1;
			walkerI[2]=j+(i*stride)+stride;

			walkerI[3]=j+(i*stride)+1;
			walkerI[4]=j+(i*stride)+stride+1;
			walkerI[5]=j+(i*stride)+stride;

			walkerI+=6;
		}
	}

	//criar normais e já tá
	mesh->Ortho(MESH_ORTHO_CREATE_NORMALS | MESH_ORTHO_PER_POSITION_NRM);
	return mesh;
}

Mesh* Factory::CreateDisk(const float radius, const int precision)
{
	HorseRadish::Vector pos,normal,texCoord;
	float ang,offset;
	Mesh* mesh;
	int i,index;

	//opsy daisies
	if (radius<=0.0f || precision<=2)
		return nullptr;

	//crio a mesh
	mesh=criaNovaMesh(precision+1,precision*3);
	if (mesh==nullptr)
		return nullptr;

	//parte mais simples, os indices :D
	for(i=0,index=1; i<mesh->numIndex; i+=3,index++)
	{
		((unsigned int*)mesh->pIndex)[i+0]=0;
		((unsigned int*)mesh->pIndex)[i+1]=index+1;
		((unsigned int*)mesh->pIndex)[i+2]=index;
	}

	//não esquecer que o ultimo tri volta ao primeiro indice (sem contar com o do centro)
	((unsigned int*)mesh->pIndex)[mesh->numIndex-2]=1;

	//o calculo para o angulo
	ang=0.0f;
	offset=360.0f/((float)precision);

	//fazer à mão o primeiro vertice
	pos.Set(0.0f,0.0f,0.0f);
	normal.Set(0.0f,1.0f,0.0f);
	texCoord.Set(0.5f,0.5f,0.0f);
	mesh->SetData(Mesh::Pos,0,pos);
	mesh->SetData(Mesh::Normal,0,normal);
	mesh->SetData(Mesh::TexCoords,0,texCoord);

	//a partir de agora a normal é sempre a mesma
	normal.Set(0.0f,1.0f,0.0f);

	//agora sim os vertices restantes
	for(i=1; i<mesh->numElements; i++,ang+=offset)
	{
		//calcular a posicao
		Math::sinCosR(ang * 0.017453292519943295769236907, pos.z, pos.x, radius);

		//a coordenada de textura
		Math::sinCosR(ang*0.017453292519943295769236907, texCoord.y, texCoord.x);
		texCoord.x=(texCoord.x+1.0f)*0.5f;
		texCoord.y=(texCoord.y+1.0f)*0.5f;

		//mandar as coisas
		mesh->SetData(Mesh::Pos,i,pos);
		mesh->SetData(Mesh::Normal,i,normal);
		mesh->SetData(Mesh::TexCoords,i,texCoord);
	}

	//posso sair numa boa
	return mesh;
}

Mesh* Factory::CreateCone(const float radius, const float height, const int precision)
{
	HorseRadish::Vector pos;
	float ang,offset,texCoord[2],texOffset;
	int i,index;
	Mesh* mesh;

	//opsy daisies
	if (radius<=0.0f || height<=0.0f || precision<=2)
		return nullptr;

	//crio a mesh
	mesh=criaNovaMesh(precision*2,precision*3);
	if (mesh==nullptr)
		return nullptr;

	//parte mais simples, os indices...
	for(i=0,index=0; i<mesh->numIndex; i+=3,index++)
	{
		((unsigned int*)mesh->pIndex)[i+0]=precision+index;
		((unsigned int*)mesh->pIndex)[i+1]=index+1;
		((unsigned int*)mesh->pIndex)[i+2]=index;
	}

	//não esquecer que o ultimo tri volta ao primeiro indice (sem contar com o do centro)
	((unsigned int*)mesh->pIndex)[mesh->numIndex-2]=0;

	//o calculo para o angulo
	ang=0.0f;
	offset=360.0f/((float)precision);
	texOffset=1.0f/((float)(precision-1));

	//calculo os vertices para a base
	texCoord[0]=0.0f;
	texCoord[1]=0.0f;
	for(i=0; i<precision; i++,ang+=offset,texCoord[0]+=texOffset)
	{
		//calcular a posicao
		pos.y=0.0f;
		Math::sinCosR(ang * 0.017453292519943295769236907, pos.z, pos.x, radius);
	
		//escrevo
		mesh->SetData(Mesh::Pos,i,pos);
		mesh->SetData(Mesh::TexCoords,i,texCoord);
	}

	//e agora todos os outros que muito simplesmente estão no 0,height,0
	pos.Set(0.0f,height,0.0f);
	texCoord[0]=0.0f;
	texCoord[1]=1.0f;
	for(i=precision; i<precision*2; i++,texCoord[0]+=texOffset)
	{
		//escrevo
		mesh->SetData(Mesh::Pos,i,pos);
		mesh->SetData(Mesh::TexCoords,i,texCoord);
	}

	//nesta geometria, calcula-se isto à mão
	mesh->Ortho(MESH_ORTHO_CREATE_NORMALS | MESH_ORTHO_PER_POSITION_NRM);

	//posso sair numa boa
	return mesh;
}

Mesh* Factory::CreateCylinder(const float topRadius, const float bottomRadius, const float height, const int precision)
{
	float ang,offset,realHeight,texCoord[2],texOffset;
	int i,index;
	Mesh* mesh;
	HorseRadish::Vector pos;

	//opsy daisies
	if (topRadius<=0.0f || bottomRadius<=0.0f || height<=0.0f || precision<=2)
		return nullptr;

	//crio a mesh
	mesh=criaNovaMesh(precision*2,precision*2*3);
	if (mesh==nullptr)
		return nullptr;

	//parte mais simples, os indices...
	for(i=0,index=0; i<mesh->numIndex; i+=6,index+=2)
	{
		((unsigned int*)mesh->pIndex)[i+0]=index;
		((unsigned int*)mesh->pIndex)[i+1]=index+2;
		((unsigned int*)mesh->pIndex)[i+2]=index+1;

		((unsigned int*)mesh->pIndex)[i+3]=index+3;
		((unsigned int*)mesh->pIndex)[i+4]=index+1;
		((unsigned int*)mesh->pIndex)[i+5]=index+2;
	}

	//não esquecer que os ultimos dois tris voltam aos primeiros indices (sem contar com o do centro)
	((unsigned int*)mesh->pIndex)[mesh->numIndex-5]=0;
	((unsigned int*)mesh->pIndex)[mesh->numIndex-3]=1;
	((unsigned int*)mesh->pIndex)[mesh->numIndex-1]=0;

	//o calculo para o angulo
	ang=0.0f;
	offset=360.0f/((float)precision);
	texOffset=1.0f/((float)(precision-1));

	//calculo os vertices
	realHeight=height*0.5f;
	texCoord[0]=1.0f;
	for(i=0; i<precision*2; i+=2,ang+=offset,texCoord[0]-=texOffset)
	{
		//calcular a posicao de cima e escreve
		pos.y=realHeight;
		Math::sinCosR(ang * 0.017453292519943295769236907, pos.z, pos.x, topRadius);
		texCoord[1]=1.0f;
		mesh->SetData(Mesh::Pos,i,pos);
		mesh->SetData(Mesh::TexCoords,i,texCoord);

		//calcular a posicao de baixo e escreve
		pos.y=-realHeight;
		Math::sinCosR(ang * 0.017453292519943295769236907, pos.z, pos.x, bottomRadius);
		texCoord[1]=0.0f;
		mesh->SetData(Mesh::Pos,i+1,pos);
		mesh->SetData(Mesh::TexCoords,i+1,texCoord);
	}

	//nesta geometria, calcula-se isto à mão
	mesh->Ortho(MESH_ORTHO_CREATE_NORMALS | MESH_ORTHO_PER_POSITION_NRM);

	//posso sair numa boa
	return mesh;
}

Mesh* Factory::CreateBox(const float width, const float height, const float depth, const int precision)
{
	float realWidth,realHeight,realDepth,offsetW,offsetH,offsetD,offsetTex,*walker,*walkerT,*walkerN;
	int i,j,index,jump,jump2;
	Mesh *mesh;

	//opsy daisies
	if (width<=0.0f || height<=0.0f || depth<=0.0f || precision<=0)
		return nullptr;

	//crio a mesh
	mesh=criaNovaMesh((2+precision-1)*(precision+1)*6,precision*precision*6*6);
	if (mesh==nullptr)
		return nullptr;

	//parte mais simples, os indices...
	for(i=0,index=jump=jump2=0; i<mesh->numIndex; i+=6)
	{
		((unsigned int*)mesh->pIndex)[i+0]=index;
		((unsigned int*)mesh->pIndex)[i+1]=index+1;
		((unsigned int*)mesh->pIndex)[i+2]=index+precision+1;

		((unsigned int*)mesh->pIndex)[i+3]=index+1;
		((unsigned int*)mesh->pIndex)[i+4]=index+precision+2;
		((unsigned int*)mesh->pIndex)[i+5]=index+precision+1;

		index++;
		jump++;
		if (jump==precision)
		{
			index++;
			jump=0;
			jump2++;
			if (jump2==precision)
			{
				index+=precision+1;
				jump2=0;
			}
		}
	}

	//usar estes valores
	realWidth=width*0.5f;
	realHeight=height*0.5f;
	realDepth=depth*0.5f;
	offsetW=width/((float)precision);
	offsetH=height/((float)precision);
	offsetD=depth/((float)precision);
	offsetTex=1.0f/((float)precision);
	walker=mesh->FindAttribData(Mesh::Pos);
	walkerT=mesh->FindAttribData(Mesh::TexCoords);
	walkerN=mesh->FindAttribData(Mesh::Normal);

	//criar para cada face os vertices
	//face da frente
	for(i=0; i<(precision+1); i++)
	{
		for(j=0; j<(2+precision-1); j++,walker+=3,walkerT+=2,walkerN+=3)
		{
			walker[0]=offsetW*((float)j)-realWidth;
			walker[1]=offsetH*((float)i)-realHeight;
			walker[2]=realDepth;

			walkerT[0]=offsetTex*((float)j);
			walkerT[1]=offsetTex*((float)i);

			walkerN[0]=walkerN[1]=0.0f;
			walkerN[2]=1.0f;
		}
	}

	//face da direita
	for(i=0; i<(precision+1); i++)
	{
		for(j=0; j<(2+precision-1); j++,walker+=3,walkerT+=2,walkerN+=3)
		{
			walker[0]=realWidth;
			walker[1]=offsetH*((float)i)-realHeight;
			walker[2]=realDepth-offsetD*((float)j);

			walkerT[0]=offsetTex*((float)j);
			walkerT[1]=offsetTex*((float)i);

			walkerN[1]=walkerN[2]=0.0f;
			walkerN[0]=1.0f;
		}
	}

	//face de trás
	for(i=0; i<(precision+1); i++)
	{
		for(j=0; j<(2+precision-1); j++,walker+=3,walkerT+=2,walkerN+=3)
		{
			walker[0]=realWidth-offsetW*((float)j);
			walker[1]=offsetH*((float)i)-realHeight;
			walker[2]=-realDepth;

			walkerT[0]=offsetTex*((float)j);
			walkerT[1]=offsetTex*((float)i);

			walkerN[0]=walkerN[1]=0.0f;
			walkerN[2]=-1.0f;
		}
	}

	//face da esquerda
	for(i=0; i<(precision+1); i++)
	{
		for(j=0; j<(2+precision-1); j++,walker+=3,walkerT+=2,walkerN+=3)
		{
			walker[0]=-realWidth;
			walker[1]=offsetH*((float)i)-realHeight;
			walker[2]=offsetD*((float)j)-realDepth;

			walkerT[0]=offsetTex*((float)j);
			walkerT[1]=offsetTex*((float)i);

			walkerN[1]=walkerN[2]=0.0f;
			walkerN[0]=-1.0f;
		}
	}

	//face da cima
	for(i=0; i<(precision+1); i++)
	{
		for(j=0; j<(2+precision-1); j++,walker+=3,walkerT+=2,walkerN+=3)
		{
			walker[0]=offsetW*((float)j)-realWidth;
			walker[1]=realHeight;
			walker[2]=realDepth-offsetD*((float)i);

			walkerT[0]=offsetTex*((float)j);
			walkerT[1]=offsetTex*((float)i);

			walkerN[0]=walkerN[2]=0.0f;
			walkerN[1]=1.0f;
		}
	}

	//face da baixo
	for(i=0; i<(precision+1); i++)
	{
		for(j=0; j<(2+precision-1); j++,walker+=3,walkerT+=2,walkerN+=3)
		{
			walker[0]=offsetW*((float)j)-realWidth;
			walker[1]=-realHeight;
			walker[2]=offsetD*((float)i)-realDepth;

			walkerT[0]=offsetTex*((float)j);
			walkerT[1]=offsetTex*((float)i);

			walkerN[0]=walkerN[2]=0.0f;
			walkerN[1]=-1.0f;
		}
	}

	//posso sair numa boa
	return mesh;
}

Mesh* Factory::CreatePlane(const float width, const float height, const int precision)
{
	float realWidth,realHeight,offsetW,offsetH,offsetTex,*walker,*walkerT,*walkerN;
	int i,j,index,jump;
	Mesh *mesh;

	//opsy daisies
	if (width<=0.0f || height<=0.0f || precision<=0)
		return nullptr;

	//crio a mesh
	mesh=criaNovaMesh((precision+1)*(precision+1),precision*precision*6);
	if (mesh==nullptr)
		return nullptr;

	//parte mais simples, os indices...
	for(i=0,index=jump=0; i<mesh->numIndex; i+=6)
	{
		((unsigned int*)mesh->pIndex)[i+0]=index;
		((unsigned int*)mesh->pIndex)[i+1]=index+1;
		((unsigned int*)mesh->pIndex)[i+2]=index+precision+1;

		((unsigned int*)mesh->pIndex)[i+3]=index+1;
		((unsigned int*)mesh->pIndex)[i+4]=index+precision+2;
		((unsigned int*)mesh->pIndex)[i+5]=index+precision+1;

		index++;
		jump++;
		if (jump==precision)
		{
			index++;
			jump=0;
		}
	}

	//usar estes valores
	realWidth=width*0.5f;
	realHeight=height*0.5f;
	offsetW=width/((float)precision);
	offsetH=height/((float)precision);
	offsetTex=1.0f/((float)precision);
	walker=mesh->FindAttribData(Mesh::Pos);
	walkerT=mesh->FindAttribData(Mesh::TexCoords);
	walkerN=mesh->FindAttribData(Mesh::Normal);

	//criar os vértices da face da frente
	for(i=0; i<=precision; i++)
	{
		for(j=0; j<=precision; j++,walker+=3,walkerT+=2,walkerN+=3)
		{
			walker[0]=offsetW*((float)j)-realWidth;
			walker[1]=offsetH*((float)i)-realHeight;
			walker[2]=0.0f;

			walkerT[0]=offsetTex*((float)j);
			walkerT[1]=offsetTex*((float)i);

			walkerN[0]=walkerN[1]=0.0f;
			walkerN[2]=1.0f;
		}
	}

	//posso sair numa boa
	return mesh;
}

Mesh* Factory::CreateSphere(const float radius, const int slices, const int stacks)
{
	float rho,drho,theta,dtheta,s,t,ds,dt,sinRho,cosRho;
	unsigned int *pIndeces;
	int i,j,index,last;
	HorseRadish::Vector calc,tex;
	Mesh *mesh;

	//opsy daisies
	if (radius<=0.0f || slices<=2 || stacks<=2)
		return nullptr;

	//crio a mesh
	mesh = criaNovaMesh((stacks-1)*slices+2,(stacks-2)*slices*2*3+slices*2*3);
	if (mesh == nullptr)
		return nullptr;

	//coisas uteis
	ds = 1.0f / ((float)slices);
	dt = 1.0f / ((float)stacks);
	t = 1.0f;
	drho = Math::PI / (float)stacks;
	dtheta = Math::TWO_PI / (float) (slices-1);

	//dá mais jeito, como é óbvio
	pIndeces=(unsigned int*)mesh->pIndex;

	//calcular os vértices
	for(i=index=0; i<=stacks; i++,t-=dt)
	{
		//passa
		rho = ((float)i) * drho;
		s = 0.0f;

		//se for o primeiro (o de cima)
		if (i==0)
		{
			calc.x = calc.z = 0.0f;
			calc.y = 1.0f;
			tex.x=0.5;
			tex.y=1.0;

			mesh->SetData(Mesh::Normal,index,calc);
			calc*=radius;
			mesh->SetData(Mesh::Pos,index,calc);
			mesh->SetData(Mesh::TexCoords,index,tex);
			index++;
			continue;
		}

		//se for o ultimo (o de baixo)
		if (i==stacks)
		{
			calc.x = calc.z = 0.0f;
			calc.y = -1.0f;
			tex.x=0.5;
			tex.y=0.0;

			mesh->SetData(Mesh::Normal,index,calc);
			calc*=radius;
			mesh->SetData(Mesh::Pos,index,calc);
			mesh->SetData(Mesh::TexCoords,index,tex);
			index++;
			continue;
		}

		//posso calcular isto
		Math::sinCosR(rho, sinRho, cosRho);

		//para todas as slices
		for(j=0; j<slices; j++,s+=ds)
		{
			//angulo
			theta= ((float)j) * dtheta;

			//posição
			Math::sinCosR(theta, calc.x, calc.z);
			calc.x*=-sinRho;
			calc.z*=sinRho;
			calc.y=cosRho;

			//coordenada de text
			tex.x=s;
			tex.y=t;

			//escrever as coisas
			mesh->SetData(Mesh::Normal,index,calc);
			calc*=radius;
			mesh->SetData(Mesh::Pos,index,calc);
			mesh->SetData(Mesh::TexCoords,index,tex);
			index++;
		}
	}

	//a primeira slice é sempre diferente
	index=1;
	for(j=0; j<(slices-1); j++,index++)
	{
		*(pIndeces++)=0;
		*(pIndeces++)=index+1;
		*(pIndeces++)=index;
	}
	*(pIndeces++)=0;
	*(pIndeces++)=1;
	*(pIndeces++)=index;

	//calcular os indices (menos o ultimo)
	index=1;
	for(i=0; i<(stacks-2); i++)
	{
		for(j=0; j<(slices-1); j++,index++)
		{
			*(pIndeces++)=index;
			*(pIndeces++)=index+slices+1;
			*(pIndeces++)=index+slices;

			*(pIndeces++)=index;
			*(pIndeces++)=index+1;
			*(pIndeces++)=index+slices+1;
		}

		*(pIndeces++)=index;
		*(pIndeces++)=(i*slices+1)+slices;
		*(pIndeces++)=index+slices;

		*(pIndeces++)=index;
		*(pIndeces++)=(i*slices+1);
		*(pIndeces++)=(i*slices+1)+slices;
		index++;
	}

	//a ultima slice também é diferente
	last=(stacks-1)*slices+1;
	for(j=0; j<(slices-1); j++,index++)
	{
		*(pIndeces++)=index;
		*(pIndeces++)=index+1;
		*(pIndeces++)=last;
	}
	*(pIndeces++)=index;
	*(pIndeces++)=(i*slices+1);
	*(pIndeces++)=last;

	//posso sair numa boa
	return mesh;
}

Mesh* Factory::CreateParabloid(const float xRadius, const float yRadius, const float zRadius, const int slices, const int stacks)
{
	Mesh *mesh;
	float *walkerP;
	int stride,i;

	//opsy daisies
	if (xRadius<=0.0f || yRadius<=0.0f || zRadius<=0.0f || slices<=2 || stacks<=2)
		return nullptr;

	//criar uma mesh unitária
	mesh = CreateSphere(1.0, slices, stacks);

	//multiplicar o x,y,z pelos respectivos valores...
	walkerP = mesh->FindAttribData(Mesh::Pos);
	stride=3;
	if (mesh->strideAttrib!=0)
		stride=mesh->strideAttrib/sizeof(float);
	for(i=0; i<mesh->numElements; i++,walkerP+=stride)
	{
		walkerP[0]*=xRadius;
		walkerP[1]*=yRadius;
		walkerP[2]*=zRadius;
	}

	//recriar normais e já tá
	mesh->Ortho(MESH_ORTHO_CREATE_NORMALS | MESH_ORTHO_PER_POSITION_NRM);
	return mesh;
}

Mesh* Factory::CreateTorusKnot(const int turns, const float innerRadius, const float outRadius, const int innerPrecision, const int outPrecision)
{
	float jWalk,jOut,xTexC,jTexC,matrix[16],wOutTurns,wOut2x,auxCalc;
	int i,j,curP,stride;
	unsigned int *walkerI;
	HorseRadish::Vector P1,P2,T,B,N;
	Mesh* mesh;

	//opsy daisies
	if (turns<=0 || outRadius<=0.0f || innerRadius<=0 || outPrecision<=2 || innerPrecision<=2)
		return nullptr;

	//como avanço
	jOut=Math::TWO_PI/((float)outPrecision);
	jTexC=1.0f/((float)outPrecision);

	//crio a mesh
	mesh=criaNovaMesh((outPrecision+1)*(innerPrecision+1),(innerPrecision*2*3)*outPrecision);
	if (mesh==nullptr)
		return nullptr;
	
	//crio os pontos
	curP=0;
	for(i=0,xTexC=jWalk=0.0f; i<=outPrecision; i++,xTexC+=jTexC,jWalk+=jOut)
	{
		if (i==outPrecision)
			jWalk=0.0f;

		wOutTurns=float(turns)*jWalk;
		wOut2x=2.0f*jWalk;
		auxCalc=1.0+0.3*cos(wOutTurns);
		P1.x=auxCalc*cos(wOut2x);
		P1.y=auxCalc*sin(wOutTurns)*0.3;
		P1.z=auxCalc*sin(wOut2x);
		P1*=outRadius;

		wOutTurns=float(turns)*(jWalk+jOut);
		wOut2x=2.0f*(jWalk+jOut);
		auxCalc=1.0+0.3*cos(wOutTurns);
		P2.x=auxCalc*cos(wOut2x);
		P2.y=auxCalc*sin(wOutTurns)*0.3;
		P2.z=auxCalc*sin(wOut2x);
		P2*=outRadius;
		if ((i+1)==outPrecision)
		{
			P2.x=1.3*outRadius;
			P2.y=P2.z=0.0f;
		}

		T = P2;
		T-= P1;
		T.Normaliza();

		N = P2;
		N+= P1;
		N.Normaliza();

		B.CalcCrossProduct(T,N);
		N.CalcCrossProduct(B,T);
		B.Normaliza();
		N.Normaliza();

		matrix[0]=N.x;
		matrix[1]=N.y;
		matrix[2]=N.z;
		matrix[4]=B.x;
		matrix[5]=B.y;
		matrix[6]=B.z;
		matrix[8]=T.x;
		matrix[9]=T.y;
		matrix[10]=T.z;
		matrix[12]=P1.x;
		matrix[13]=P1.y;
		matrix[14]=P1.z;

		curP+=drawAroundPoint(mesh,curP,xTexC,innerRadius,innerPrecision,matrix);
	}

	//agora crio os indices excluindo o ultimo arco
	walkerI=(unsigned int*)mesh->pIndex;
	stride=innerPrecision+1;
	for(i=0; i<outPrecision; i++)
	{
		//para cada ponto do arco
		for(j=0; j<innerPrecision; j++)
		{
			walkerI[0]=j+(i*stride);
			walkerI[1]=j+(i*stride)+1;
			walkerI[2]=j+(i*stride)+stride;

			walkerI[3]=j+(i*stride)+1;
			walkerI[4]=j+(i*stride)+stride+1;
			walkerI[5]=j+(i*stride)+stride;

			walkerI+=6;
		}
	}

	//criar normais e já tá
	mesh->Ortho(MESH_ORTHO_CREATE_NORMALS | MESH_ORTHO_PER_POSITION_NRM);
	return mesh;
}

Mesh* Factory::CreatePQTorus(const int P, const int Q, const float innerRadius, const float outRadius, const int innerPrecision, const int outPrecision)
{
	float r,wOut,jOut,xTexC,jTexC,matrix[16];
	int i,j,curP,stride;
	unsigned int *walkerI;
	HorseRadish::Vector P1,P2,T,B,N;
	Mesh* mesh;

	//opsy daisies
	if (P<=0 || Q<=0 || outRadius<=0.0f || innerRadius<=0 || outPrecision<=2 || innerPrecision<=2)
		return nullptr;

	//como avanço
	jOut=Math::TWO_PI/((float)outPrecision);
	jTexC=1.0f/((float)outPrecision);

	//crio a mesh
	mesh=criaNovaMesh((outPrecision+1)*(innerPrecision+1),(innerPrecision*2*3)*outPrecision);
	if (mesh==nullptr)
		return nullptr;
	
	//crio os pontos
	curP=0;
	for(i=0,wOut=xTexC=0.0f; i<=outPrecision; wOut+=jOut,xTexC+=jTexC,i++)
	{
		if (i==outPrecision)
			wOut=0.0f;

		r= 0.5f * ( 2.0f + sin ( ((float)Q) * wOut ) );
		P1.x = r * cos( ((float)P) * wOut ) * outRadius;
		P1.y = r * cos( ((float)Q) * wOut ) * outRadius;
		P1.z = r * sin( ((float)P) * wOut ) * outRadius;

		r= 0.5f * ( 2.0f + sin ( ((float)Q) * (wOut + jOut) ) );
		P2.x = r * cos( ((float)P) * (wOut + jOut) ) * outRadius;
		P2.y = r * cos( ((float)Q) * (wOut + jOut) ) * outRadius;
		P2.z = r * sin( ((float)P) * (wOut + jOut) ) * outRadius;
		if ((i+1)==outPrecision)
		{
			P2.x = P2.y = outRadius;
			P2.z = 0.0f;
		}

		T = P2;
		T-= P1;
		T.Normaliza();

		N = P2;
		N+= P1;
		N.Normaliza();

		B.CalcCrossProduct(T,N);
		N.CalcCrossProduct(B,T);
		B.Normaliza();
		N.Normaliza();

		matrix[0]=N.x;
		matrix[1]=N.y;
		matrix[2]=N.z;
		matrix[4]=B.x;
		matrix[5]=B.y;
		matrix[6]=B.z;
		matrix[8]=T.x;
		matrix[9]=T.y;
		matrix[10]=T.z;
		matrix[12]=P1.x;
		matrix[13]=P1.y;
		matrix[14]=P1.z;

		curP+=drawAroundPoint(mesh,curP,xTexC,innerRadius,innerPrecision,matrix);
	}

	//agora crio os indices excluindo o ultimo arco
	walkerI=(unsigned int*)mesh->pIndex;
	stride=innerPrecision+1;
	for(i=0; i<outPrecision; i++)
	{
		//para cada ponto do arco
		for(j=0; j<innerPrecision; j++)
		{
			walkerI[0]=j+(i*stride);
			walkerI[1]=j+(i*stride)+1;
			walkerI[2]=j+(i*stride)+stride;

			walkerI[3]=j+(i*stride)+1;
			walkerI[4]=j+(i*stride)+stride+1;
			walkerI[5]=j+(i*stride)+stride;

			walkerI+=6;
		}
	}

	//criar normais e já tá
	mesh->Ortho(MESH_ORTHO_CREATE_NORMALS | MESH_ORTHO_PER_POSITION_NRM);
	return mesh;
}

Mesh* Factory::CreateTeapot(const float radius, const int precision)
{
	HorseRadish::Vector data[32][4][4];
	unsigned int *walker,*meta;
	Mesh *mesh;
	int curVert;

	//quanto do que vou ter que ter
	if (radius<=0.0f || precision<0)
		return nullptr;

	//crio a mesh
	mesh=criaNovaMesh(32*pow(4.0f,precision)*4,32*pow(4.0f,precision)*6);
	if (mesh==nullptr)
		return nullptr;

	//construir os dados
	for(int i=0;i<32;i++) 
	{
		for(int j=0;j<4;j++) 
		{
			for(int k=0;k<4;k++)
			{
				curVert=indexTP[i][j][k]-1;
				data[i][j][k].x=vertTP[curVert][0];
				data[i][j][k].y=vertTP[curVert][1];
				data[i][j][k].z=vertTP[curVert][2];
			}
		}
	}

	//crio os vertices todos
	curVert=0;
	for(int i=0; i<32; i++) 
		divide_patch(mesh,data[i],precision,&curVert);

	//crio finalmente os indice (que é facil)
	walker=(unsigned int*)mesh->pIndex;
	meta=walker+mesh->numIndex;
	curVert=0;

	//se for divisoes par tenho de mudar a ordem de desenho
	if (precision%2==0)
	{
		while(walker<meta)
		{
			*(walker++)=curVert+2;	*(walker++)=curVert+1;		*(walker++)=curVert+0;
			*(walker++)=curVert+3;	*(walker++)=curVert+2;		*(walker++)=curVert+0;
			curVert+=4;
		}
	}
	else
	{
		while(walker<meta)
		{
			*(walker++)=curVert+0;	*(walker++)=curVert+1;		*(walker++)=curVert+2;
			*(walker++)=curVert+0;	*(walker++)=curVert+2;		*(walker++)=curVert+3;
			curVert+=4;
		}
	}


	//já tá
	mesh->RemoveDuplicate(Mesh::Pos);
	mesh->CenterMass(0.0f,0.0f,0.0f);
	mesh->MaxBBox(radius);
	mesh->Ortho(MESH_ORTHO_CREATE_NORMALS | MESH_ORTHO_PER_POSITION_NRM);
	return mesh;
}

Model* Factory::Read3DS(HorseRadish::Streams::StreamReader * const streamReader)
{
	//basta chamar isto
	return mfRead3DS(streamReader);
}

Model* Factory::ReadOBJ(HorseRadish::Streams::StreamReader * const streamReader)
{
	//basta chamar isto
	return mfReadOBJ(streamReader);
}

Model* Factory::ReadCollada(HorseRadish::Streams::StreamReader * const streamReader)
{
	//basta chamar isto
	return mfReadCollada(streamReader);
}

}//namespace Geometry
}//namespace HorseRadish