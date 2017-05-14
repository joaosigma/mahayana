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
		struct {
			size_t numVertices = 0, numIndices = 0;
			size_t fstreamVertexOffset = 0, fstreamIndexOffset = 0;
			hr::BBox bbox;
		} geom;
		std::string matDiffusePath, matNormalPath;

		struct
		{
			int meshVBOVertexOffset = 0;
			unsigned int meshDrawIndirectOffset = 0;
			void *meshTriListOffset = nullptr;

			hr::gl::objects::Texture texDiffuse, texNormal;

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

			hr::gl::objects::Buffer vboMeshData, vboMeshIndexData, vboIndirectDraw;
			hr::gl::objects::VertexArray vaoMesh;
		}mRenderData;

		hr::streams::FileStream mGeomFileStream;

	private:
		bool m_editorMode = false;

	public:
		World(const std::string& scenePath, const std::string& geomPath, bool editorMode = false);
		virtual ~World();

		void loadData(hr::io::FileSystem& fileSystem);
		void prepareNextFrame(const tools::Camera& hrCamera, const hr::gl::tools::Viewport& hrViewport);

	private:
		void cleanup();
		bool loadWorld(const std::string& scenePath, const std::string& geomPath);
	};

	class WorldEditor
		: public World
	{
		struct
		{
			std::string scene, geom;
		} m_paths;

	public:
		static void createEmptyScene(const std::string& scenePath, const std::string& geomPath);

	public:
		WorldEditor(const std::string& scenePath, const std::string& geomPath);

		bool importMesh(const std::string& name, const hr::geom::Mesh& mesh);
		bool importObj(const std::string& basePath, const std::string& fileName);

		void processMesh(const std::vector<size_t>& conceptIds, std::function<void(hr::geom::Mesh&)> cb);

		std::vector<size_t> unusedConcepts() const;
		void removeConcepts(const std::vector<size_t>& conceptIds);

	private:
		size_t genId() const;

		void addMesh(size_t geomId, const hr::geom::Mesh& mesh);
		void saveScene();
	};
} }
