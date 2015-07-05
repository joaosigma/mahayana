#pragma once

#include "tools\camera.hpp"

#include "common\mesh.hpp"
#include "common\FileSystem.hpp"
#include "common\OpenGL\objects.hpp"
#include "common\OpenGL\tools\frustum.hpp"
#include "common\OpenGL\tools\viewport.hpp"

#include <vector>

namespace HorseRadish
{
	namespace Render
	{
		class Concept
		{
		public:
			std::string name;
			HorseRadish::Geometry::Mesh mesh;
			std::string matDiffusePath, matNormalPath;

			struct RenderData
			{
				int meshVBOVertexOffset;
				unsigned int meshDrawIndirectOffset;
				void *meshTriListOffset;

				HorseRadish::OpenGL::Objects::Texture texDiffuse, texNormal;

				RenderData()
					: meshVBOVertexOffset(0), meshDrawIndirectOffset(0), meshTriListOffset(nullptr)
				{ }

			} renderData;
		};

		class Object
		{
		public:
			enum class Type { Static = 1, Instance = 2 };

			Type type;
			std::string conceptName;
			HorseRadish::BBox bbox;

			//instance data: [{quat, translate}, ...]
		};

		class World
		{
		public:
			std::unordered_map<std::string, Concept> mConcepts;
			std::vector<Object> mObjects;

			struct RenderData{
				std::vector<Object*> objects;
			}mRenderData;

		public:
			World();
			~World();

			void Cleanup();

			bool ImportJSON(HorseRadish::Streams::StreamReader &stream);
			bool ExportJSON(HorseRadish::Streams::StreamWriter &stream);

			bool importObj(const std::string& basePath, const std::string& fileName);

			void LoadData(HorseRadish::IO::FileSystem * const fileSystem);
			void PrepareNextFrame(const Tools::Camera& hrCamera, const HorseRadish::OpenGL::Tools::Viewport& hrViewport);
		};

	} //Render
} //HorseRadish
