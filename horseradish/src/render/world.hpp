#pragma once

#include "tools/camera.hpp"

#include "renderer.hpp"

#include "common/color.hpp"
#include "common/mesh.hpp"
#include "common/meshAnim.hpp"
#include "common/stream.hpp"
#include "common/OpenGL/tools/frustum.hpp"
#include "common/OpenGL/tools/viewport.hpp"

#include <map>
#include <span>
#include <atomic>
#include <vector>

namespace hr::render
{
	class World
	{
	public:
		typedef size_t AreaId;

		static constexpr AreaId InvalidAreaId = 0;

		struct Timestep
		{
			float t = 0.0f;
		};

	protected:
		struct Material
		{
			size_t id = 0;
			Colorf baseColor{Colorf::white()};
			float metallicFactor = 0.5f, roughnessFactor = 0.5f;
			std::string name;
		};

		struct TextureSet
		{
			size_t id = 0;
			std::string diffusePath;
			std::string normalPath;
			std::string metallicRoughnessPath;
		};

		class Object
		{
		public:
			enum class Type : unsigned int { Static = 1 };

			Type type = Type::Static;
			size_t id = 0;
			size_t materialId = 0;
			size_t textureSetId = 0;
			hr::BBox<> bbox;

			struct {
				bool hasAnim = false;
				size_t animSetId = 0;
				geom::MeshAnim meshAnim;
				geom::Mesh<geom::VertexShading, uint16_t> meshAnimated;
			} anim;
		};

		class Instance
		{
		public:
			enum class Type : unsigned int { Static = 1, Instance = 2 };

			Type type = Type::Static;
			size_t objectId = 0;
			hr::BBox<> bbox;

			//instance data: [{quat, translate}, ...]
		};

		class SkeletonAnim
		{
		public:
			size_t id = 0;
			geom::SkeletonAnim skeletonAnim;
		};

		struct Area
		{
			AreaId id;
			IRenderer::SceneId sceneId;

			std::map<size_t, Object> mObjects;
			std::map<size_t, Material> mMaterials;
			std::map<size_t, TextureSet> mTextureSets;

			std::vector<Instance> mInstances;

			std::map<size_t, SkeletonAnim> mSkeletonAnims;
		};

	public:
		static bool migrateBinData(const std::string& binPathOld, const std::string& binPathNew);

	protected:
		static bool geomFileCreate(hr::streams::FileStream& fstream);

		static bool geomFileAddMesh(hr::streams::FileStream& fstream, size_t geomId, const geom::Mesh<geom::VertexFull, uint32_t>& mesh);
		static bool geomFileAddMeshAnim(hr::streams::FileStream& fstream, size_t geomId, const hr::geom::MeshAnim& meshAnim, size_t animSetId);
		static bool geomFileAddAnimationSet(hr::streams::FileStream& fstream, size_t animSetId, const hr::geom::SkeletonAnim& skeletonAnim);
		static bool geomFileAddAnimation(hr::streams::FileStream& fstream, size_t animId, size_t animSetId, const hr::geom::SkeletonAnim::Animation& animation);

		static bool geomFileTransformMeshes(hr::streams::FileStream& fstream, std::span<const size_t> geomIds, const std::function<void(geom::Mesh<geom::VertexFull, uint32_t>&)>& cb);
		static bool geomFileRetrieveOffsets(hr::streams::FileStream& fstream, size_t geomId, size_t& vertexOffset, size_t& indexOffset);
		static bool geomFileRemoveGeom(hr::streams::FileStream& fstreamOld, hr::streams::FileStream& fstreamNew, std::vector<size_t> geomIds);

	protected:
		std::unordered_map<AreaId, Area> mAreas;
		std::atomic<AreaId> mGenAreaIds{ 1 };

	public:
		World();
		virtual ~World();

		AreaId loadArea(IRenderer& renderer, const std::string& scenePath, const std::string& binPath);
		void unloadArea(AreaId areaId);

		void prepareNextFrame(Timestep& timestep, IRenderer& renderer, const tools::Camera& hrCamera, const hr::gl::tools::Viewport& hrViewport);

		void queryObjectSkeletonAnim(size_t objectId, const std::function<void(const geom::SkeletonAnim&)>& cb) const noexcept;

	private:
		bool loadAnimations(hr::streams::StreamReader& sreader, Area& area);
		bool load(IRenderer& renderer, Area& area, const std::string& scenePath, const std::string& binPath);
	};
}
