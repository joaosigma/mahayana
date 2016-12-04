#pragma once

#include "tools/camera.hpp"

#include "common/mesh.hpp"
#include "common/fileSystem.hpp"
#include "common/OpenGL/objects.hpp"
#include "common/OpenGL/tools/frustum.hpp"
#include "common/OpenGL/tools/viewport.hpp"

#include <map>
#include <vector>

namespace hr { namespace render
{
	class Concept
	{
	public:
		size_t id = 0;
		std::string name;
		hr::geom::Mesh mesh;
		hr::BBox meshBBox;
		std::string matDiffusePath, matNormalPath;

		struct RenderData
		{
			int meshVBOVertexOffset = 0;
			unsigned int meshDrawIndirectOffset = 0;
			void *meshTriListOffset = nullptr;

			hr::gl::objects::Texture texDiffuse, texNormal;

			RenderData()
			{ }

		} renderData;
	};

	class Object
	{
	public:
		enum class Type: unsigned int { Static = 1, Instance = 2 };

		Type type;
		size_t conceptId;
		hr::BBox bbox;

		//instance data: [{quat, translate}, ...]
	};

	class World
	{
	public:
		std::map<size_t, Concept> mConcepts;
		std::vector<Object> mObjects;

		struct RenderData{
			std::vector<Object*> objects;
		}mRenderData;

	private:
		size_t genId() const;

	public:
		World();
		~World();

		void cleanup();

		bool importAll(hr::streams::StreamReader &streamScene, hr::streams::StreamReader &streamGeom);
		bool exportAll(hr::streams::StreamWriter &streamScene, hr::streams::StreamWriter &streamGeom);

		bool importObj(const std::string& basePath, const std::string& fileName);

		void optimizeConcept(size_t conceptId);
		void optimizeConcepts();

		std::vector<size_t> unusedConcepts() const;
		void removeConcepts(const std::vector<size_t>& conceptIds);

		void loadData(hr::io::FileSystem& fileSystem);
		void prepareNextFrame(const tools::Camera& hrCamera, const hr::gl::tools::Viewport& hrViewport);
	};
} }
