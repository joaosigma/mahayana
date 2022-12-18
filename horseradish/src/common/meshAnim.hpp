#pragma once

#include "mesh.hpp"

#include "vector.hpp"
#include "matrix.hpp"
#include "quaternion.hpp"
#include "serialize.hpp"
#include "serializeSupport.hpp"

#include <array>
#include <string>
#include <span>
#include <string_view>
#include <functional>
#include <optional>
#include <vector>
#include <unordered_map>

namespace hr::geom
{
	/*
	* This class represents a mesh with animation data, or in other words, for each vertex
	* in the source mesh we store information for either 4 or 8 joints.
	*
	* No animation data is stored here.
	*/

	class MeshAnim
	{
		using TMesh = Mesh<VertexShading, uint16_t>;

	public:
		enum class SkinningType { Vertex4Joints, Vertex8Joints };

#pragma pack(push, 1)
		struct VertexJoint
		{
			uint16_t jointIndex;
			uint16_t jointWeight;
		};
#pragma pack(pop)

	private:
		TMesh mMesh;
		SkinningType mSkinningType{ SkinningType::Vertex4Joints };
		std::unique_ptr<VertexJoint[]> mVertexJoints;

	public:
		MeshAnim() = default;
		MeshAnim(TMesh mesh, SkinningType skinningType);

		size_t numJoints() const noexcept
		{
			return mMesh.numVertices() * numJointsPerVertex();
		}

		size_t numJointsPerVertex() const noexcept
		{
			switch (mSkinningType)
			{
				case SkinningType::Vertex4Joints:
					return 4;
				case SkinningType::Vertex8Joints:
					return 8;
				default:
					break;
			}

			return 0;
		}

		size_t sizeVerticesJoints() const noexcept
		{
			return (numJoints() * sizeof(VertexJoint));
		}

		SkinningType skinningType() const noexcept
		{
			return mSkinningType;
		}

		const VertexJoint* verticesJoints() const noexcept
		{
			return mVertexJoints.get();
		}
		VertexJoint* verticesJoints() noexcept
		{
			return mVertexJoints.get();
		}

		TMesh& mesh() noexcept
		{
			return mMesh;
		}
		const TMesh& mesh() const noexcept
		{
			return mMesh;
		}

		void correctWeights() noexcept;
		
		VertexJoint& vertexJoint(size_t vertexIndex, size_t jointIndex) noexcept;
		const VertexJoint& vertexJoint(size_t vertexIndex, size_t jointIndex) const noexcept;

		void collectVertexJoints(size_t vertexIndex, std::array<VertexJoint, 4>& vertexJoints) const noexcept;
		void collectVertexJoints(size_t vertexIndex, std::array<VertexJoint, 8>& vertexJoints) const noexcept;
	};

	/*
	* This class stores animations.
	*
	* It doesn't need any MeshAnim to work, except in the updateMesh(...) method which will then store the final animation
	*   result in a Mesh. It uses the MeshAnim to access the per-vertex joint information.
	*
	* The animation set has a name, an hierarchy of nodes and joints (with per-joint information like transformation matrices) and
	*	a collection of animations. Each animation has an ID and a list of samplers. Each sampler has a timeline and a list of
	*	channels, where each channel alters / animates a specific joint local transform.
	*
	* NOTE: it is not the responsibility of this class to make sure that the joints indices in MeshAnim correspond correctly to the
	*   ones stored here.
	*/

	class MeshAnimSet
	{
		using TMesh = Mesh<VertexShading, uint16_t>;

	public:
		struct Node
		{
			std::string name;
			int32_t parentIndex{ -1 }; //-1 means no parent
			Matrix4f localTransform{ Matrix4f::identity() };
		};

		struct Joint
		{
			size_t nodeIndex;
			int32_t parentIndex{-1};

			struct
			{
				Vector3f scale;
				Vector3f translation;
				Quaternionf rotation;
				Matrix4f matrix{Matrix4f::identity()};
			} localTransform;

			Matrix4f transformInvert{Matrix4f::identity()};
		};

		struct Animation
		{
			struct Channel
			{
				enum class Target{ None, Scale, Rotation, Translation };

				Target target{ Target::None };
				size_t nodeIndex{ 0 };
				std::vector<Vector4f> frameData;
			};

			struct Sampler
			{
				std::vector<float> timePoints;
				std::vector<Channel> channels;
				float minTimePoint = 0.0f;
				float maxTimePoint = 0.0f;
			};

			std::string name;
			std::vector<Sampler> samplers;
			float minTimePoint = 0.0f;
			float maxTimePoint = 0.0f;
		};

	private:
		std::string mName;
		std::vector<Node> mNodes;
		std::vector<Joint> mJoints;
		size_t mJointsRootNodeIndex{ 0 };
		std::unordered_map<size_t, Animation> mAnimations;

		struct AnimatedJoint
		{
			int32_t parentIndex{-1};

			Vector3f updatedScale;
			Vector3f updatedTranslation;
			Quaternionf updatedRotation;

			Matrix4f localTransform{ Matrix4f::identity() };
		};

		struct
		{
			BBox<> bbox;

			std::vector<AnimatedJoint> joints;

			Matrix4f jointsRootTransform{ Matrix4f::identity() };
			std::vector<Matrix4f> jointsFinalTransform;

		} mLastAnimation;

		Matrix4f calcNodeGlobalTransform(size_t nodeIndex) const noexcept;
		Matrix4f calcJointGlobalTransform(size_t jointIndex) const noexcept;

	public:
		MeshAnimSet() = default;
		explicit MeshAnimSet(std::string name, std::vector<Node> nodeData, size_t jointsRootNodeIndex, std::vector<Joint> jointData);

		std::string_view name() const noexcept{ return mName; }

		std::span<const Node> nodes() const noexcept{ return mNodes; }
		std::span<const Joint> joints() const noexcept{ return mJoints; }

		size_t jointsRootNodeIndex() const noexcept{ return mJointsRootNodeIndex; }

		size_t numNodes() const noexcept{ return mNodes.size(); }
		size_t numJoints() const noexcept{ return mJoints.size(); }
		size_t numAnimations() const noexcept{ return mAnimations.size(); }

		bool animationAdd(size_t animId, Animation animation);
		void animationUpdateBBoxes(size_t animId, const MeshAnim& baseMesh);

		void animate(size_t animId, float time);
		void animateReset();

		BBox<> updatedBBox() const;
		void updateMesh(const MeshAnim& baseMesh, TMesh& animatedMesh) const;
	};
}

namespace hr
{
	//serialization support

	template<>
	struct serialize::Support<geom::MeshAnimSet>
	{
		template<class TArchiveWriter>
		static void save(TArchiveWriter& writer, const geom::MeshAnimSet& animSet)
		{
			using serialize::NamedParam;
			writer << NamedParam("name", animSet.mName);
		}

		template<class TArchiveReader>
		static void load(TArchiveReader& reader, geom::MeshAnimSet& animSet)
		{
			reader >> animSet.mName;
		}
	};

	template<>
	struct serialize::Support<geom::MeshAnimSet::Node>
	{
		template<class TArchiveWriter>
		static void save(TArchiveWriter& writer, const geom::MeshAnimSet::Node& node)
		{
			using serialize::NamedParam;
			writer << NamedParam("name", node.name) << NamedParam("parentIndex", node.parentIndex) << NamedParam("localTransform", node.localTransform);
		}

		template<class TArchiveReader>
		static void load(TArchiveReader& reader, geom::MeshAnimSet::Node& node)
		{
			reader >> node.name >> node.parentIndex >> node.localTransform;
		}
	};

	template<>
	struct serialize::Support<geom::MeshAnimSet::Joint>
	{
		template<class TArchiveWriter>
		static void save(TArchiveWriter& writer, const geom::MeshAnimSet::Joint& joint)
		{
			using serialize::NamedParam;
			writer << NamedParam("nodeIndex", joint.nodeIndex) << NamedParam("parentIndex", joint.parentIndex);
			writer << NamedParam("scale", joint.localTransform.scale) << NamedParam("translation", joint.localTransform.translation) << NamedParam("rotation", joint.localTransform.rotation) << NamedParam("localTransform", joint.localTransform.matrix);
			writer << NamedParam("transformInvert", joint.transformInvert);
		}

		template<class TArchiveReader>
		static void load(TArchiveReader& reader, geom::MeshAnimSet::Joint& joint)
		{
			reader >> joint.nodeIndex >> joint.parentIndex;
			reader >> joint.localTransform.scale >> joint.localTransform.translation >> joint.localTransform.rotation >> joint.localTransform.matrix;
			reader >> joint.transformInvert;
		}
	};

	template<>
	struct serialize::Support<geom::MeshAnimSet::Animation::Channel>
	{
		template<class TArchiveWriter>
		static void save(TArchiveWriter& writer, const geom::MeshAnimSet::Animation::Channel& channel)
		{
			using serialize::NamedParam;
			writer << NamedParam("target", channel.target) << NamedParam("nodeIndex", channel.nodeIndex) << NamedParam("frameData", channel.frameData);
		}

		template<class TArchiveReader>
		static void load(TArchiveReader& reader, geom::MeshAnimSet::Animation::Channel& channel)
		{
			reader >> channel.target >> channel.nodeIndex >> channel.frameData;
		}
	};

	template<>
	struct serialize::Support<geom::MeshAnimSet::Animation::Sampler>
	{
		template<class TArchiveWriter>
		static void save(TArchiveWriter& writer, const geom::MeshAnimSet::Animation::Sampler& sampler)
		{
			using serialize::NamedParam;
			writer << NamedParam("timePoints", sampler.timePoints) << NamedParam("channels", sampler.channels) << NamedParam("minTimePoint", sampler.minTimePoint) << NamedParam("maxTimePoint", sampler.maxTimePoint);
		}

		template<class TArchiveReader>
		static void load(TArchiveReader& reader, geom::MeshAnimSet::Animation::Sampler& sampler)
		{
			reader >> sampler.timePoints >> sampler.channels >> sampler.minTimePoint >> sampler.maxTimePoint;
		}
	};

	template<>
	struct serialize::Support<geom::MeshAnimSet::Animation>
	{
		template<class TArchiveWriter>
		static void save(TArchiveWriter& writer, const geom::MeshAnimSet::Animation& anim)
		{
			using serialize::NamedParam;
			writer << NamedParam("id", anim.name) << NamedParam("samplers", anim.samplers) << NamedParam("minTimePoint", anim.minTimePoint) << NamedParam("maxTimePoint", anim.maxTimePoint);
		}

		template<class TArchiveReader>
		static void load(TArchiveReader& reader, geom::MeshAnimSet::Animation& anim)
		{
			reader >> anim.name >> anim.samplers >> anim.minTimePoint >> anim.maxTimePoint;
		}
	};
}
