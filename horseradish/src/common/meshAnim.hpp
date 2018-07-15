#pragma once

#include "mesh.hpp"
#include "vector.hpp"
#include "matrix.hpp"
#include "quaternion.hpp"

#include <array>
#include <vector>
#include <unordered_map>

namespace hr::geom
{
	class MeshAnim
	{
	public:
		enum class SkinningType { Vertex4Joints, Vertex8Joints };

#pragma pack(push, 1)
		struct VertexJoint
		{
			std::uint16_t jointIndex;
			std::uint16_t jointWeight;
		};
#pragma pack(pop)

	private:
		Mesh mMesh;
		SkinningType mSkinningType;
		std::unique_ptr<VertexJoint[]> mVertexJoints;

		size_t numJointsPerVertex() const;

	public:
		MeshAnim() = default;
		MeshAnim(Mesh mesh, SkinningType skinningType);

		const VertexJoint* verticesJoints() const
		{
			return mVertexJoints.get();
		}
		VertexJoint* verticesJoints()
		{
			return mVertexJoints.get();
		}

		size_t sizeVerticesJoints() const
		{
			return (mMesh.numVertices() * numJointsPerVertex() * sizeof(VertexJoint));
		}

		SkinningType skinningType() const
		{
			return mSkinningType;
		}

		Mesh& mesh()
		{
			return mMesh;
		}
		const Mesh& mesh() const
		{
			return mMesh;
		}
		
		VertexJoint& vertexJoint(size_t vertexIndex, size_t jointIndex);
		const VertexJoint& vertexJoint(size_t vertexIndex, size_t jointIndex) const;

		void collectVertexJoints(size_t vertexIndex, std::array<VertexJoint, 4>& vertexJoints) const;
		void collectVertexJoints(size_t vertexIndex, std::array<VertexJoint, 8>& vertexJoints) const;
	};

	class MeshAnimSet
	{
	public:
		struct Joint
		{
			Vector3f pos;
			Quaternion rot;
		};

		struct Frame
		{
			BBox bbox;
			std::vector<Joint> joints;
		};

	private:
		struct Anim
		{
			size_t id = 0;
			float frameRate = 0;
			float framePeriodSeconds = 0.0f;
			std::vector<Frame> frames;
		};

		std::string mName;
		std::vector<Matrix> mBindPoseInverted;
		std::unordered_map<size_t, Anim> mAnimations;

		struct
		{
			BBox bbox;
			double tNormalized = 0.0;
			std::vector<Matrix> joints;

		} mLastAnimation;

	public:
		MeshAnimSet() = default;
		MeshAnimSet(std::vector<Matrix> bindPoseInverseTrans);
		MeshAnimSet(std::string name, std::vector<Matrix> bindPoseInverseTrans);

		const std::string& name() const;

		const Matrix& bindPoseInvertedMat(size_t jointIndex) const;
		float animFrameRate(size_t animId) const;
		const std::vector<Frame>& animFrames(size_t animId) const;

		size_t numJoints() const;
		size_t numAnimations() const;

		bool animAdd(size_t animId, float frameRate, std::vector<Frame> frames);
		void animUpdateBBoxes(size_t animId, const MeshAnim& bindPoseMesh);

		void animate(size_t animId, float time);
		BBox updatedBBox() const;
		void updateMesh(const MeshAnim& bindPoseMesh, Mesh& animatedMesh) const;

	};
}
