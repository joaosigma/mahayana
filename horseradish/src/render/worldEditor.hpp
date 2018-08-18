#pragma once

#include "world.hpp"

#include "common/mesh.hpp"
#include "common/stream.hpp"
#include "common/OpenGL/tools/frustum.hpp"
#include "common/OpenGL/tools/viewport.hpp"

#include <array>

namespace hr::render
{
	class WorldEditor
		: public World
	{
		struct AreaData
		{
			struct ObjectData
			{
				size_t objectId = 0;

				std::string name;
				std::string matDiffusePath, matNormalPath;
				struct {
					size_t numVertices = 0, numIndices = 0;
					size_t fstreamVertexOffset = 0, fstreamIndexOffset = 0;
				} geom;
			};

			struct AnimSetData
			{
				struct Animation
				{
					size_t animId = 0;
					std::string name;
				};

				struct JointData
				{
					size_t index = 0;
					int32_t parentIndex = -1;
					std::array<char, 256> name;
				};

				size_t animSetId = 0;
				std::string name;
				std::vector<Animation> anims;
				std::vector<JointData> joints;
			};

			std::unordered_map<size_t, ObjectData> objects;
			std::unordered_map<size_t, AnimSetData> animationSets;
			std::string pathScene, pathBin;
		};
		std::unordered_map<AreaId, AreaData> mAreasData;

	public:
		WorldEditor();

		AreaId newArea(std::string_view scenePath, std::string_view binPath);

		bool importMesh(AreaId areaId, std::string_view name, const hr::geom::Mesh& mesh);
		bool importObj(AreaId areaId, std::string_view basePath, std::string_view fileName);
		bool importMD5(AreaId areaId, std::string_view md5Path, std::string newAnimSetName);
		bool importMD5Anim(AreaId areaId, std::string_view animSetParentName, std::string_view md5AnimPath, std::string newAnimName);

		void processMesh(AreaId areaId, const std::vector<size_t>& objectIds, std::function<void(hr::geom::Mesh&)> cb);

		std::vector<size_t> unusedObjects(AreaId areaId) const;
		void removeObjects(AreaId areaId, const std::vector<std::string_view>& objectsNames);

	private:
		size_t genObjectId(Area& area) const;
		size_t genAnimSetId(Area& area) const;

		size_t findAnimSetId(AreaData& area, std::string_view name);

		void saveArea(Area& area);
	};
}
