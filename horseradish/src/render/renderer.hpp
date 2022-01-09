#pragma once

#include "tools/camera.hpp"
#include "../common/opengl/tools/viewport.hpp"

#include <string_view>
#include <functional>

namespace hr::render
{
	class IRenderObject
	{
	public:
		typedef size_t ObjectId;

		static constexpr ObjectId InvalidObjectId = 0;

	public:
		virtual ~IRenderObject() = default;

		virtual ObjectId id() const = 0;

		virtual BBox<> bbox() const = 0;

		virtual size_t numVertices() const = 0;
		virtual size_t readVertices(void* const destBuffer, size_t requestedDataSize) const = 0;

		virtual size_t numIndices() const = 0;
		virtual size_t readIndices(void* const destBuffer, size_t requestedDataSize) const = 0;

		virtual std::string_view texDiffusePath() const = 0;
		virtual std::string_view texNormalPath() const = 0;
	};

	class IRenderObjectManager
	{
	public:
		virtual ~IRenderObjectManager() = default;

		virtual size_t numObjects() const = 0;
		virtual void iterateObjects(std::function<void(IRenderObject&)> cb) const = 0;
	};

	class IRenderer
	{
	public:
		typedef size_t SceneId;

		static constexpr SceneId InvalidSceneId = 0;

	public:
		virtual ~IRenderer() = default;

		virtual SceneId loadScene(const IRenderObjectManager& manager) = 0;
		virtual void unloadScene(SceneId sceneId) = 0;

		virtual void updateVertexData(SceneId sceneId, const IRenderObjectManager& manager) = 0;
		virtual void prepareNextFrame(SceneId sceneId, const std::vector<IRenderObject::ObjectId>& objects) = 0;
	};
}
