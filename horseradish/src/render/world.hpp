#pragma once

#include "tools\camera.hpp"

#include "common\Mesh.hpp"
#include "common\MeshFactory.hpp"
#include "common\FileSystem.hpp"
#include "common\OpenGL\objects.hpp"
#include "common\OpenGL\tools\frustum.hpp"
#include "common\OpenGL\tools\viewport.hpp"

#include <vector>

namespace HorseRadish
{
	namespace Render
	{
		class Light;
		class Geometry;
		class TextureSet;
		class Surface;
		class Material;
		class MaterialLibrary;
		class Brush;
		class World;

		class Light
		{
		public:
			enum LightType { Omni = 0xfab1, Spot = 0xfab2, Direct = 0xfab3 };

		public:
			LightType type;
			HorseRadish::Vector position, lastPos, target, color;
			bool active, insideCamera, noShadows;
			HorseRadish::OpenGL::Tools::Frustum frustum;
			unsigned int scissor[4];

			struct Matrices{
				HorseRadish::Matrix mView, mProj, mInvView, mInvProj, mTrans, mInvTrans, mSBiasScale;
			}matrices;

			struct Textures{
				//const HorseRadish::OpenGL::Objects::ObjectGL *spotTex, *attenTex, *cubeEnvTex;
			}textures;

		public:
			Light();
			~Light();
		};

		class Geometry
		{
		public:
			enum GeometryType
			{
				Static1 = 0xfaa3,	//pos(3f), uv1(2f), uv2(2f), cor(4b) = 32 bytes
				Static2 = 0xfaa4,	//pos(3f), uv1(2f), uv2(2f), cor(4b), normal(3f), tangentB(4f), extra(1f) = 64 bytes
				Static3 = 0xfaa5,	//pos(3f), uv(2f), normal(3f), tangentB(4f), extra(4f) = 64 bytes
				Anim1 = 0xfaa1,		//pos(3f), uv(2f), bIndex(4b), bWeight(4b), cor(4b) = 32 bytes
				Anim2 = 0xfaa2		//pos(3f), uv(2f), bIndex(4b), bWeight(4b), cor(4b), normal(3f), tangentB(4f), extra(1f) = 64 bytes
			};

		public:
			int id;
			GeometryType type;
			HorseRadish::Geometry::Mesh mesh;
			HorseRadish::BBox bbox;
			HorseRadish::BSphere bsphere;
			int renderVBOVertexOffset;
			void *renderTriListOffset;

		public:
			Geometry();
			~Geometry();
		};

		class TextureSet
		{
		public:
			int id;

			struct Texture{
				int type, format, filter, flags;
				HorseRadish::String filePath;
			};
			std::vector<Texture> texs;

		public:
			TextureSet();
			~TextureSet();
		};

		class Surface
		{
		public:
			enum SurfaceType { Static = 0xfac1, Matrix = 0xfac2, Instance = 0xfac3, Anim = 0xfac4 };

			int id;
			SurfaceType type;

			std::vector<Surface*> childs;

			Material *material;
			Geometry *geometry;

			TextureSet *texSet;
			struct TextureData
			{
				struct Lighting
				{
					HorseRadish::OpenGL::Objects::Texture diffuse;
					HorseRadish::OpenGL::Objects::Texture normal;
					HorseRadish::OpenGL::Objects::Texture spec;
					HorseRadish::OpenGL::Objects::Texture misc;
				}lighting;

			}*texData;

			struct RenderValues{
				float distToCam;
			}renderValues;

			Surface();
			~Surface();
		};

		class Material
		{
		public:
			enum MaterialProperties
			{
				ShadowCaster = (1 << 0),
				ShadowReceiver = (1 << 1),
				ShadowTwoSided = (1 << 2),
				TwoSided = (1 << 3),
				PerformLighting = (1 << 4),
				PerformParallax = (1 << 5),
				Transparent = (1 << 6),
				PerformShadering = (1 << 7),
				IsDecal = (1 << 8),
				IsInfinite = (1 << 9),
				PerformAlphaTest = (1 << 10),
				IsSolid = (1 << 11),
				Debug = (1 << 12)
			};

		public:
			struct Shadering{
				//const HorseRadish::OpenGL::Objects::ObjectGL **shaderMapTex;
				//const HorseRadish::OpenGL::Objects::ObjectGL *progGLSL;
				int numShaderMaps;
			};

			struct Lighting{
				float specPow, parallaxScale, parallaxBias;
				//const HorseRadish::OpenGL::Objects::ObjectGL *texNormal, *texSpec, *texDiffuse, *texAux;
			};

			const MaterialLibrary *materialLib;

			Shadering dataShadering;
			Lighting dataLighting;
			MaterialProperties matProperties;
			float alphaTestValue;

		public:
			Material();
			~Material();
		};

		class MaterialLibrary
		{

		public:
			struct Shadering
			{
				struct ShaderMap{
					HorseRadish::String name, mapName;
					unsigned short mapFlagType, mapFlagFormat, mapFlagBit, mapFlagWrap, mapFlagFilter;
				};
				struct ShaderParam
				{
					HorseRadish::String name, paramName;
					float paramValue;
				};

				int shaderingNeeds;
				HorseRadish::String progVertex, progFragment;
				int blendSFactor, blendDFactor;
				std::vector<ShaderMap> shaderMaps;
				std::vector<ShaderParam> shaderParams;
			};

			struct Lighting{
				HorseRadish::String nomeBump, nomeNormal, nomeSpec, nomeColor, nomeOcclusion;
				float specPow, heightScale, parallaxScale, parallaxBias;
			};

			HorseRadish::String nomeMaterial, nomeFicheiro, descMaterial;
			unsigned int nomeMaterialMD5;
			Shadering dataShadering;
			Lighting dataLighting;
			unsigned int flags, flagsCollision;
			float alphaTestValue;

		public:
			MaterialLibrary();
			~MaterialLibrary();
		};

		class Brush
		{

		public:
			Brush();
			~Brush();
		};

		class World
		{
		public:
			static const int HRFChunckTextureSetsID;
			static const int HRFChunckGeometriesID;
			static const int HRFChunckSurfacesID;

		private:
			int genGeometryID, genSurfaceID;

		public:
			std::vector<Light> lights;
			std::vector<Geometry> geometries;
			std::vector<Material> materials;
			std::vector<TextureSet> textureSets;

			std::vector<Surface> surfacesTotal;
			std::vector<Surface*> surfacesRoot;

			struct RenderContent{
				std::vector<Surface*> surfaces;
				std::vector<Light *> lights;

			}renderContent;

			static bool prepareMeshForType(HorseRadish::Geometry::Mesh * const modelMesh, const Geometry::GeometryType geomType);

			void arranjaPonteirosGeom(const Geometry * const originalList);
			void arranjaPonteirosSurf(const Surface * const originalList);
			Geometry* createGeometry(HorseRadish::Geometry::Mesh * const modelMesh, const Geometry::GeometryType geomType);
			int processModel(HorseRadish::Geometry::Model * const modelo, const Geometry::GeometryType geomType, const bool joinModels);

			bool writeGeometries(HorseRadish::Streams::StreamWriter * const streamWriter);
			bool readGeometries(HorseRadish::Streams::StreamReader * const streamReader);
			bool writeTextureSets(HorseRadish::Streams::StreamWriter * const streamWriter);
			bool readTextureSets(HorseRadish::Streams::StreamReader * const streamReader);
			bool writeSurfaces(HorseRadish::Streams::StreamWriter * const streamWriter);
			bool readSurfaces(HorseRadish::Streams::StreamReader * const streamReader);

		public:
			World();
			~World();

			void Cleanup();

			int ImportHRF(HorseRadish::Streams::Stream &fileStream);
			int ExportHRF(HorseRadish::Streams::Stream &fileStream);

			int CreateGeometry3DS(HorseRadish::Streams::StreamReader &streamReader, const Geometry::GeometryType geomType, const bool joinModels);
			int CreateGeometryOBJ(HorseRadish::Streams::StreamReader &streamReader, const Geometry::GeometryType geomType, const bool joinModels);
			int CreateGeometryCOLLADA(HorseRadish::Streams::StreamReader &streamReader, const Geometry::GeometryType geomType, const bool joinModels);
			int CreateSurface(const Surface::SurfaceType surfType, int surfaceParentID, Material * const material, Geometry * const geometry);

			void LoadData(HorseRadish::IO::FileSystem * const fileSystem);
			void PrepareNextFrame(const Tools::Camera& hrCamera, const HorseRadish::OpenGL::Tools::Viewport& hrViewport);
		};

	} //Render
} //HorseRadish
