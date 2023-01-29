#pragma once

#include "world.hpp"

#include "common/mesh.hpp"
#include "common/stream.hpp"
#include "common/random.hpp"

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
				std::optional<size_t> animSetId;
			};

			struct AnimSetData
			{
				struct Animation
				{
					size_t id{0};
					std::string name;
				};

				size_t id{0};
				std::string name;
				std::vector<Animation> animations;
			};

			std::unordered_map<size_t, ObjectData> objects;
			std::unordered_map<size_t, Material> materials;
			std::unordered_map<size_t, TextureSet> textureSets;
			std::unordered_map<size_t, AnimSetData> animationSets;
			std::string pathScene, pathBin;
		};

		Random mRandom;
		std::unordered_map<AreaId, AreaData> mAreasData;

	private:
		static size_t genObjectId(const Area& area, Random& rand);
		static size_t genMaterialId(const Area& area, Random& rand);
		static size_t genTextureSetId(const Area& area, Random& rand);
		static size_t genAnimSetId(const Area& area, Random& rand);

		static size_t findAnimSetId(const AreaData& area, std::string_view name);

	public:
		WorldEditor() = default;

		AreaId newArea(std::string_view scenePath, std::string_view binPath);

		bool importMesh(AreaId areaId, std::string_view name, const hr::geom::Mesh<geom::VertexFull, uint32_t>& mesh);
		bool importObj(AreaId areaId, std::string_view basePath, std::string_view fileName);
		bool importGLTF(AreaId areaId, std::string_view gltfPath);

		void processMesh(AreaId areaId, std::span<const size_t> objectIds, const std::function<void(geom::Mesh<geom::VertexFull, uint32_t>&)>& cb);

		std::vector<size_t> unusedObjects(AreaId areaId) const;
		void removeObjects(AreaId areaId, std::span<std::string_view> objectsNames);

	private:
		void optimizeTextureSets(Area& area);
		void saveArea(Area& area);
	};
}
