#pragma once

#include "tools/camera.hpp"

#include "common/mesh.hpp"
#include "common/fileSystem.hpp"
#include "common/OpenGL/objects.hpp"
#include "common/OpenGL/tools/frustum.hpp"
#include "common/OpenGL/tools/viewport.hpp"

#include <vector>

namespace hr { namespace render
{
	class Concept
	{
	public:
		std::string name;
		hr::geom::Mesh mesh;
		std::string matDiffusePath, matNormalPath;

		struct RenderData
		{
			int meshVBOVertexOffset;
			unsigned int meshDrawIndirectOffset;
			void *meshTriListOffset;

			hr::gl::objects::Texture texDiffuse, texNormal;

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
		hr::BBox bbox;

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

		void cleanup();

		bool importJSON(hr::streams::StreamReader &stream);
		bool exportJSON(hr::streams::StreamWriter &stream);

		bool importObj(const std::string& basePath, const std::string& fileName);

		void loadData(hr::io::FileSystem& fileSystem);
		void prepareNextFrame(const tools::Camera& hrCamera, const hr::gl::tools::Viewport& hrViewport);
	};
} }
