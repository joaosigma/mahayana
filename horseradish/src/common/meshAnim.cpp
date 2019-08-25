#pragma once

#include "meshAnim.hpp"

namespace hr::geom
{
	namespace
	{
		void animateMesh(const std::vector<Matrix>& animatedJoints, const MeshAnim& bindPoseMesh, Mesh& animatedMesh)
		{
			auto numVertices = bindPoseMesh.mesh().numVertices();
			for (size_t curVertex = 0; curVertex < numVertices; curVertex++)
			{
				Vector3f bindPos, finalPos(0.0f);
				Vector3f bindNormal, finalNormal(0.0f);
				Vector3f bindTangent, finalTangent(0.0f);

				bindPos.set(bindPoseMesh.mesh().vertices()[curVertex].pos);
				Mesh::unpack(bindPoseMesh.mesh().vertices()[curVertex].normal, bindNormal.data(), 3);
				Mesh::unpack(bindPoseMesh.mesh().vertices()[curVertex].tangent, bindTangent.data(), 3);

				std::array<MeshAnim::VertexJoint, 8> vertexJoints;
				bindPoseMesh.collectVertexJoints(curVertex, vertexJoints);

				for (const auto& vertexJoint : vertexJoints)
				{
					if (vertexJoint.jointWeight == 0)
						continue;

					Vector3f transPos;
					Vector3f transNormal;
					Vector3f transTangent;
					animatedJoints[vertexJoint.jointIndex].transform(bindPos, transPos);

					Matrix3 mat3x3(animatedJoints[vertexJoint.jointIndex]);
					mat3x3.transform(bindNormal, transNormal);
					mat3x3.transform(bindTangent, transTangent);

					transPos *= Mesh::unpack(vertexJoint.jointWeight);
					transNormal *= Mesh::unpack(vertexJoint.jointWeight);
					transTangent *= Mesh::unpack(vertexJoint.jointWeight);

					finalPos += transPos;
					finalNormal += transNormal;
					finalTangent += transTangent;
				}

				finalNormal.normalize();
				finalTangent.normalize();

				finalPos.write(animatedMesh.vertices()[curVertex].pos);
				Mesh::pack(finalNormal.data(), animatedMesh.vertices()[curVertex].normal, 3);
				Mesh::pack(finalTangent.data(), animatedMesh.vertices()[curVertex].tangent, 3);
			}
		}
	}

	size_t MeshAnim::numJointsPerVertex() const
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

	MeshAnim::MeshAnim(Mesh mesh, SkinningType skinningType)
		: mMesh(std::move(mesh))
		, mSkinningType(skinningType)
	{
		if (mMesh.numVertices() <= 0)
			return;

		auto totalVertexJoints = mMesh.numVertices() * numJointsPerVertex();

		mVertexJoints = std::unique_ptr<VertexJoint[]>(new VertexJoint[totalVertexJoints]);
		std::memset(mVertexJoints.get(), 0, sizeof(VertexJoint) * totalVertexJoints);
	}

	MeshAnim::VertexJoint& MeshAnim::vertexJoint(size_t vertexIndex, size_t jointIndex)
	{
		auto index = ((vertexIndex % mMesh.numVertices()) * numJointsPerVertex()) + (jointIndex % numJointsPerVertex());
		return *(mVertexJoints.get() + index);
	}

	const MeshAnim::VertexJoint& MeshAnim::vertexJoint(size_t vertexIndex, size_t jointIndex) const
	{
		auto index = ((vertexIndex % mMesh.numVertices()) * numJointsPerVertex()) + (jointIndex % numJointsPerVertex());
		return *(mVertexJoints.get() + index);
	}

	void MeshAnim::collectVertexJoints(size_t vertexIndex, std::array<MeshAnim::VertexJoint, 4>& vertexJoints) const
	{
		auto startIndex = (vertexIndex % mMesh.numVertices()) * numJointsPerVertex();
		vertexJoints[0] = mVertexJoints[startIndex + 0];
		vertexJoints[1] = mVertexJoints[startIndex + 1];
		vertexJoints[2] = mVertexJoints[startIndex + 2];
		vertexJoints[3] = mVertexJoints[startIndex + 3];
	}

	void MeshAnim::collectVertexJoints(size_t vertexIndex, std::array<MeshAnim::VertexJoint, 8>& vertexJoints) const
	{
		auto startIndex = (vertexIndex % mMesh.numVertices()) * numJointsPerVertex();

		switch (mSkinningType)
		{
		case SkinningType::Vertex4Joints:
			vertexJoints[0] = mVertexJoints[startIndex + 0];
			vertexJoints[1] = mVertexJoints[startIndex + 1];
			vertexJoints[2] = mVertexJoints[startIndex + 2];
			vertexJoints[3] = mVertexJoints[startIndex + 3];
			vertexJoints[4] = vertexJoints[5] = vertexJoints[6] = vertexJoints[7] = { 0, 0 };
			return;
		case SkinningType::Vertex8Joints:
			vertexJoints[0] = mVertexJoints[startIndex + 0];
			vertexJoints[1] = mVertexJoints[startIndex + 1];
			vertexJoints[2] = mVertexJoints[startIndex + 2];
			vertexJoints[3] = mVertexJoints[startIndex + 3];
			vertexJoints[4] = mVertexJoints[startIndex + 4];
			vertexJoints[5] = mVertexJoints[startIndex + 5];
			vertexJoints[6] = mVertexJoints[startIndex + 6];
			vertexJoints[7] = mVertexJoints[startIndex + 7];
			return;
		default:
			break;
		}
	}

	MeshAnimSet::MeshAnimSet(std::vector<Matrix> bindPoseInverseTrans)
		: mBindPoseInverted(std::move(bindPoseInverseTrans))
	{
		mLastAnimation.joints.resize(mBindPoseInverted.size());
	}

	MeshAnimSet::MeshAnimSet(std::string name, std::vector<Matrix> bindPoseInverseTrans)
		: MeshAnimSet(std::move(bindPoseInverseTrans))
	{
		mName = std::move(name);
	}

	const std::string& MeshAnimSet::name() const
	{
		return mName;
	}

	const Matrix& MeshAnimSet::bindPoseInvertedMat(size_t jointIndex) const
	{
		return mBindPoseInverted[jointIndex % mBindPoseInverted.size()];
	}

	float MeshAnimSet::animFrameRate(size_t animId) const
	{
		return mAnimations.at(animId).frameRate;
	}

	const std::vector<MeshAnimSet::Frame>& MeshAnimSet::animFrames(size_t animId) const
	{
		return mAnimations.at(animId).frames;
	}

	size_t MeshAnimSet::numJoints() const
	{
		return mBindPoseInverted.size();
	}

	size_t MeshAnimSet::numAnimations() const
	{
		return mAnimations.size();
	}

	bool MeshAnimSet::animAdd(size_t animId, float frameRate, std::vector<Frame> frames)
	{
		if ((frameRate <= 0.0f) || frames.empty() || (mAnimations.find(animId) != mAnimations.end()))
			return false;

		for (const auto& frame : frames)
		{
			if (frame.joints.size() != mBindPoseInverted.size())
				return false;
		}

		Anim anim;
		anim.frameRate = frameRate;
		anim.frames = std::move(frames);
		anim.framePeriodSeconds = static_cast<float>(anim.frames.size()) / anim.frameRate;

		mAnimations[animId] = std::move(anim);

		return true;
	}

	void MeshAnimSet::animUpdateBBoxes(size_t animId, const MeshAnim& bindPoseMesh)
	{
		if (mAnimations.find(animId) == mAnimations.end())
			return;

		Mesh animatedMesh(bindPoseMesh.mesh());
		auto animatedJoints = mLastAnimation.joints;

		//for each frame of this animation
		for (auto& frame : mAnimations[animId].frames)
		{
			//update joints
			for (size_t curJointIndex = 0; curJointIndex < animatedJoints.size(); curJointIndex++)
			{
				auto& pos = frame.joints[curJointIndex].pos;
				auto& rot = frame.joints[curJointIndex].rot;

				auto& finalMat = animatedJoints[curJointIndex];

				finalMat.setTranslation(pos);
				finalMat *= rot;
				finalMat *= mBindPoseInverted[curJointIndex];
			}

			//update mesh and extract bbox
			animateMesh(animatedJoints, bindPoseMesh, animatedMesh);
			frame.bbox += animatedMesh.getBoundingBox();
		}
	}

	void MeshAnimSet::animate(size_t animId, float time)
	{
		if (mAnimations.find(animId) == mAnimations.end())
			return;

		auto& anim = mAnimations[animId];

		//calculate frames and final t
		size_t indexStart, indexEnd;
		{
			if (time <= 0.0f)
				time = 0.0f;
			while (time >= anim.framePeriodSeconds)
				time -= anim.framePeriodSeconds;

			indexStart = Math::ftoi(Math::floor(time * anim.frameRate));
			indexStart = (indexStart >= anim.frames.size()) ? 0 : indexStart;

			indexEnd = indexStart + 1;
			indexEnd = (indexEnd >= anim.frames.size()) ? 0 : indexEnd;

			mLastAnimation.tNormalized = Math::fClamp(time * anim.frameRate - static_cast<float>(indexStart), 0.0f, 1.0f);
		}

		//calculate final matrix transform for each joint

		auto& frameA = anim.frames[indexStart];
		auto& frameB = anim.frames[indexEnd];

		for (size_t curJointIndex = 0; curJointIndex < mLastAnimation.joints.size(); curJointIndex++)
		{
			Vector3f pos;
			Quaternion rot;
			
			pos.storeInterpolate(frameA.joints[curJointIndex].pos, frameB.joints[curJointIndex].pos, mLastAnimation.tNormalized);
			rot.setNLerp(frameA.joints[curJointIndex].rot, frameB.joints[curJointIndex].rot, mLastAnimation.tNormalized);

			auto& finalMat = mLastAnimation.joints[curJointIndex];
			
			finalMat.setTranslation(pos);
			finalMat *= rot;
			finalMat *= mBindPoseInverted[curJointIndex];
		}

		//and the interpolated bbox
		mLastAnimation.bbox.setMin(Vector3f::evalLinear(frameA.bbox.min(), frameB.bbox.min(), mLastAnimation.tNormalized));
		mLastAnimation.bbox.setMax(Vector3f::evalLinear(frameA.bbox.max(), frameB.bbox.max(), mLastAnimation.tNormalized));
	}

	BBox MeshAnimSet::updatedBBox() const
	{
		return mLastAnimation.bbox;
	}

	void MeshAnimSet::updateMesh(const MeshAnim& bindPoseMesh, Mesh& animatedMesh) const
	{
		if (bindPoseMesh.mesh().numVertices() != animatedMesh.numVertices())
			return;

		animateMesh(mLastAnimation.joints, bindPoseMesh, animatedMesh);
	}
}
