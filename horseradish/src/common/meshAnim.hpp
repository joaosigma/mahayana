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
#include <map>

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
			uint16_t index;
			uint16_t weight;
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
	* This class stores a skeleton and associated animations.
	*
	* It doesn't need any MeshAnim to work, except in the updateMesh(...) method which will then store the final animation
	*   result in a Mesh. It uses the MeshAnim to access the per-vertex joint information.
	*
	* It has a name, a skeleton (an hierarchy of joints) and a collection of animations. Each animation has
	*	an ID and a list of samplers. Each sampler has a timeline and a list of channels, where each channel alters / animates
	*	a specific joint local transform.
	*
	* NOTE: it is not the responsibility of this class to make sure that the joints indices in MeshAnim correspond correctly to the
	*   ones stored here. Several MeshAnim can (or not) share the same skeleton.
	*/

	class SkeletonAnim
	{
		using TMesh = Mesh<VertexShading, uint16_t>;

	public:
		enum class AnimationType { RepeatCurrent, CycleAll };

		struct Joint
		{
			std::string name;
			int32_t parentIndex{ -1 };

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
				uint32_t jointIndex{ 0 };
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
		AnimationType mAnimationType{AnimationType::RepeatCurrent};
		std::vector<Joint> mJoints;
		Matrix4f mRootTransform{ Matrix4f::identity() };
		std::map<size_t, Animation> mAnimations;

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
			float curTime{0.0f}, lastTime{0.0f};
			size_t curAnimId{0};

			std::vector<AnimatedJoint> joints;
			std::vector<Matrix4f> jointsFinalTransform;

		} mLastAnimation;

		Matrix4f calcJointGlobalTransform(size_t jointIndex) const noexcept;

	public:
		SkeletonAnim() = default;
		explicit SkeletonAnim(std::string name, Matrix4f rootTransform, std::vector<Joint> jointData);

		std::string_view name() const noexcept{ return mName; }

		const Matrix4f& rootTransform() const noexcept{ return mRootTransform; }
		std::span<const Joint> joints() const noexcept{ return mJoints; }

		size_t numJoints() const noexcept{ return mJoints.size(); }
		size_t numAnimations() const noexcept{ return mAnimations.size(); }

		bool animationAdd(size_t animId, Animation animation);

		//void calculateBBoxes(const MeshAnim& baseMesh);

		void animate(float time);
		void animateSetup(AnimationType animationType, size_t startAnimId);
		void animateReset();

		void updateMesh(const MeshAnim& baseMesh, TMesh& animatedMesh) const;
	};
}

namespace hr
{
	//serialization support

	template<>
	struct serialize::Support<geom::SkeletonAnim::Joint>
	{
		template<class TArchiveWriter>
		static void save(TArchiveWriter& writer, const geom::SkeletonAnim::Joint& joint)
		{
			using serialize::NamedParam;
			writer << NamedParam("name", joint.name) << NamedParam("parentIndex", joint.parentIndex);
			writer << NamedParam("scale", joint.localTransform.scale) << NamedParam("translation", joint.localTransform.translation) << NamedParam("rotation", joint.localTransform.rotation) << NamedParam("localTransform", joint.localTransform.matrix);
			writer << NamedParam("transformInvert", joint.transformInvert);
		}

		template<class TArchiveReader>
		static void load(TArchiveReader& reader, geom::SkeletonAnim::Joint& joint)
		{
			reader >> joint.name >> joint.parentIndex;
			reader >> joint.localTransform.scale >> joint.localTransform.translation >> joint.localTransform.rotation >> joint.localTransform.matrix;
			reader >> joint.transformInvert;
		}
	};

	template<>
	struct serialize::Support<geom::SkeletonAnim::Animation::Channel>
	{
		template<class TArchiveWriter>
		static void save(TArchiveWriter& writer, const geom::SkeletonAnim::Animation::Channel& channel)
		{
			using serialize::NamedParam;
			writer << NamedParam("target", channel.target) << NamedParam("jointIndex", channel.jointIndex) << NamedParam("frameData", channel.frameData);
		}

		template<class TArchiveReader>
		static void load(TArchiveReader& reader, geom::SkeletonAnim::Animation::Channel& channel)
		{
			reader >> channel.target >> channel.jointIndex >> channel.frameData;
		}
	};

	template<>
	struct serialize::Support<geom::SkeletonAnim::Animation::Sampler>
	{
		template<class TArchiveWriter>
		static void save(TArchiveWriter& writer, const geom::SkeletonAnim::Animation::Sampler& sampler)
		{
			using serialize::NamedParam;
			writer << NamedParam("timePoints", sampler.timePoints) << NamedParam("channels", sampler.channels) << NamedParam("minTimePoint", sampler.minTimePoint) << NamedParam("maxTimePoint", sampler.maxTimePoint);
		}

		template<class TArchiveReader>
		static void load(TArchiveReader& reader, geom::SkeletonAnim::Animation::Sampler& sampler)
		{
			reader >> sampler.timePoints >> sampler.channels >> sampler.minTimePoint >> sampler.maxTimePoint;
		}
	};

	template<>
	struct serialize::Support<geom::SkeletonAnim::Animation>
	{
		template<class TArchiveWriter>
		static void save(TArchiveWriter& writer, const geom::SkeletonAnim::Animation& anim)
		{
			using serialize::NamedParam;
			writer << NamedParam("id", anim.name)
				<< NamedParam("samplers", anim.samplers)
				<< NamedParam("minTimePoint", anim.minTimePoint) << NamedParam("maxTimePoint", anim.maxTimePoint);
		}

		template<class TArchiveReader>
		static void load(TArchiveReader& reader, geom::SkeletonAnim::Animation& anim)
		{
			reader >> anim.name >> anim.samplers >> anim.minTimePoint >> anim.maxTimePoint;
		}
	};
}
