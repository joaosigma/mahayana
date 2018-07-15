#pragma once

#include "tools/camera.hpp"

#include "renderer.hpp"

#include "common/mesh.hpp"
#include "common/meshAnim.hpp"
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

		struct Timestep
		{
			float t = 0.0f;
		};

	protected:
		class Object
		{
		public:
			enum class Type : unsigned int { Static = 1 };

			Type type = Type::Static;
			size_t id = 0;
			hr::BBox bbox;

			struct {
				bool hasAnim = false;
				size_t animSetId = 0;
				geom::MeshAnim meshAnim;
				geom::Mesh meshAnimated;
			} anim;
		};

		class Instance
		{
		public:
			enum class Type : unsigned int { Static = 1, Instance = 2 };

			Type type = Type::Static;
			size_t objectId = 0;
			hr::BBox bbox;

			//instance data: [{quat, translate}, ...]
		};

		class AnimationSet
		{
		public:
			size_t id = 0;
			geom::MeshAnimSet animationSet;
		};

		struct Area
		{
			AreaId id;
			IRenderer::SceneId sceneId;

			std::map<size_t, Object> mObjects;
			std::vector<Instance> mInstances;

			std::map<size_t, AnimationSet> mAnimationSets;
		};

	protected:
		static bool geomFileCreate(hr::streams::FileStream& fstream);
		static bool geomFileAddMesh(hr::streams::FileStream& fstream, size_t geomId, const hr::geom::Mesh& mesh);
		static bool geomFileAddMeshAnim(hr::streams::FileStream& fstream, size_t geomId, const hr::geom::MeshAnim& meshAnim, size_t animSetId);
		static bool geomFileAddAnimationSet(hr::streams::FileStream& fstream, size_t animSetId, const hr::geom::MeshAnimSet& animSet);
		static bool geomFileAddAnimation(hr::streams::FileStream& fstream, size_t animId, size_t animSetId, float frameRate, const std::vector<hr::geom::MeshAnimSet::Frame>& animation);

	protected:
		std::unordered_map<AreaId, Area> mAreas;
		std::atomic<AreaId> mGenAreaIds{ 1 };

	public:
		World();
		virtual ~World();

		AreaId loadArea(IRenderer& renderer, const std::string& scenePath, const std::string& geomPath);
		void unloadArea(AreaId areaId);

		void prepareNextFrame(Timestep& timestep, IRenderer& renderer, const tools::Camera& hrCamera, const hr::gl::tools::Viewport& hrViewport);

	private:
		bool loadAnimations(hr::streams::StreamReader& sreader, Area& area);
		bool load(IRenderer& renderer, Area& area, const std::string& scenePath, const std::string& geomPath);
	};
}
