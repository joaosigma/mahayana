#pragma once

#include "tools/camera.hpp"

#include "renderer.hpp"

#include "common/mesh.hpp"
#include "common/stream.hpp"
#include "common/OpenGL/tools/frustum.hpp"
#include "common/OpenGL/tools/viewport.hpp"

#include <map>
#include <atomic>
#include <vector>

namespace hr::render
{
	class World
	{
	public:
		typedef size_t AreaId;

		static constexpr AreaId InvalidAreaId = 0;

	protected:
		class Object
		{
		public:
			size_t objectId = 0;
			hr::BBox bbox;
		};

		class Instance
		{
		public:
			enum class Type : unsigned int { Static = 1, Instance = 2 };

			Type type;
			size_t objectId;
			hr::BBox bbox;

			//instance data: [{quat, translate}, ...]
		};

		struct Area
		{
			AreaId id;
			IRenderer::SceneId sceneId;
			std::map<size_t, Object> mObjects;
			std::vector<Instance> mInstances;
			hr::streams::FileStream mGeomFileStream;
		};

	protected:
		std::unordered_map<AreaId, Area> mAreas;
		std::atomic<AreaId> mGenAreaIds{ 1 };

	private:
		bool m_editorMode = false;

	public:
		World(bool editorMode = false);
		virtual ~World();

		AreaId loadArea(IRenderer& renderer, const std::string& scenePath, const std::string& geomPath);
		void unloadArea(AreaId areaId);

		void prepareNextFrame(IRenderer& renderer, const tools::Camera& hrCamera, const hr::gl::tools::Viewport& hrViewport);

	protected:
		AreaId addEmptyArea();

	private:
		bool load(IRenderer& renderer, Area& area, const std::string& scenePath, const std::string& geomPath);
	};

	class WorldEditor
		: public World
	{
		struct AreaData
		{
			struct ObjectData
			{
				size_t objectId;
				std::string name;
				std::string matDiffusePath, matNormalPath;
				struct {
					size_t numVertices = 0, numIndices = 0;
					size_t fstreamVertexOffset = 0, fstreamIndexOffset = 0;
				} geom;
			};

			std::unordered_map<size_t, ObjectData> objects;
			std::string pathScene, pathGeom;
		};
		std::unordered_map<AreaId, AreaData> mAreasData;

	public:
		static void createEmptyArea(const std::string& scenePath, const std::string& geomPath);

	public:
		WorldEditor();

		AreaId newArea(std::string_view scenePath, std::string_view geomPath);

		bool importMesh(AreaId areaId, std::string_view name, const hr::geom::Mesh& mesh);
		bool importObj(AreaId areaId, std::string_view basePath, std::string_view fileName);

		void processMesh(AreaId areaId, const std::vector<size_t>& objectIds, std::function<void(hr::geom::Mesh&)> cb);

		std::vector<size_t> unusedObjects(AreaId areaId) const;
		void removeObjects(AreaId areaId, const std::vector<size_t>& objectIds);

	private:
		size_t genObjectId(Area& area) const;

		void addMesh(Area& area, size_t geomId, const hr::geom::Mesh& mesh);
		void saveArea(Area& area);
	};
}
