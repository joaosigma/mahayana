module;

#include <cassert>
#include <cstdint>

module core:meshAnim.impl;

import :types;
import :matrix;
import :meshAnim;

namespace hr::geom
{
    namespace
    {
        void animateMesh(const Matrix4f& rootNodeTrans, std::span<const Matrix4f> animatedJoints, const MeshAnim& baseMesh, Mesh<VertexShading, uint16_t>& animatedMesh)
        {
            assert(baseMesh.mesh().numVertices() == animatedMesh.numVertices());
            assert(baseMesh.mesh().numIndices() == animatedMesh.numIndices());

            auto numVertices = baseMesh.mesh().numVertices();
            for (size_t curVertex = 0; curVertex < numVertices; curVertex++)
            {
                auto finalMat = Matrix4f::zero();
                {
                    assert(baseMesh.numJointsPerVertex() <= 4);
                    std::array<MeshAnim::VertexJoint, 4> vertexJoints;
                    baseMesh.collectVertexJoints(curVertex, vertexJoints);

                    for (const auto& vertexJoint : vertexJoints)
                    {
                        assert(vertexJoint.index < animatedJoints.size());
                        if (vertexJoint.weight == 0)
                            continue;

                        auto jointMat = animatedJoints[vertexJoint.index];
                        jointMat *= types::unpackFloat<uint16_t>(vertexJoint.weight);

                        finalMat += jointMat;
                    }
                }

                finalMat = finalMat * rootNodeTrans;
                auto finalMat3x3 = finalMat.clone(Matrix4f::CloneTransform::InverseTranspose).convert<Matrix3, float>();

                auto& inVertex = baseMesh.mesh().vertex(curVertex);
                auto& outVertex = animatedMesh.vertex(curVertex);

                auto bindPos = inVertex.getPos();
                auto bindNormal = inVertex.getNormal();
                auto bindTangentFull = inVertex.getTangent();
                auto bindTangent = bindTangentFull.convert<float, 3>();

                finalMat.transform(bindPos);
                finalMat3x3.transform(bindNormal);
                finalMat3x3.transform(bindTangent);

                bindNormal.normalize();
                bindTangent.normalize();

                outVertex.setPos(bindPos);
                outVertex.setNormal(bindNormal);
                outVertex.setTangent(bindTangent.convert<float, 4>(bindTangentFull[3]));
            }
        }
    }

    MeshAnim::MeshAnim(TMesh mesh, SkinningType skinningType)
      : mMesh{std::move(mesh)}, mSkinningType{skinningType}
    {
        if (mMesh.numVertices() <= 0)
            return;

        auto totalVertexJoints = numJoints();

        mVertexJoints = std::unique_ptr<VertexJoint[]>(new VertexJoint[totalVertexJoints]);
        std::memset(mVertexJoints.get(), 0, sizeof(VertexJoint) * totalVertexJoints);
    }

    void MeshAnim::correctWeights() noexcept
    {
        auto jointsPerVertex = numJointsPerVertex();
        auto numVertices = mMesh.numVertices();

        // make sure that, for each vertex, the weights aren't all zero (if they are, them assume the first joint has all the weight)
        for (size_t i = 0; i < numVertices; i++)
        {
            size_t sum = 0;
            for (size_t ij = 0; ij < jointsPerVertex; ij++)
                sum += mVertexJoints[(i * jointsPerVertex) + ij].weight;

            assert(sum <= std::numeric_limits<uint16_t>::max());
            if (sum == 0)
                mVertexJoints[(i * jointsPerVertex) + 0].weight = std::numeric_limits<uint16_t>::max();
        }
    }

    void MeshAnim::collectVertexJoints(size_t vertexIndex, std::array<MeshAnim::VertexJoint, 4>& vertexJoints) const noexcept
    {
        auto startIndex = (vertexIndex % mMesh.numVertices()) * numJointsPerVertex();
        vertexJoints[0] = mVertexJoints[startIndex + 0];
        vertexJoints[1] = mVertexJoints[startIndex + 1];
        vertexJoints[2] = mVertexJoints[startIndex + 2];
        vertexJoints[3] = mVertexJoints[startIndex + 3];
    }

    void MeshAnim::collectVertexJoints(size_t vertexIndex, std::array<MeshAnim::VertexJoint, 8>& vertexJoints) const noexcept
    {
        auto startIndex = (vertexIndex % mMesh.numVertices()) * numJointsPerVertex();

        switch (mSkinningType)
        {
            case SkinningType::Vertex4Joints:
                vertexJoints[0] = mVertexJoints[startIndex + 0];
                vertexJoints[1] = mVertexJoints[startIndex + 1];
                vertexJoints[2] = mVertexJoints[startIndex + 2];
                vertexJoints[3] = mVertexJoints[startIndex + 3];
                vertexJoints[4] = vertexJoints[5] = vertexJoints[6] = vertexJoints[7] = {0, 0};
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

    Matrix4f SkeletonAnim::calcJointGlobalTransform(size_t jointIndex) const noexcept
    {
        assert((jointIndex >= 0) && (jointIndex < mLastAnimation.joints.size()));
        const auto& joint = mLastAnimation.joints[jointIndex];

        if (joint.parentIndex < 0)
            return joint.localTransform;
        return joint.localTransform * calcJointGlobalTransform(static_cast<size_t>(joint.parentIndex));
    }

    SkeletonAnim::SkeletonAnim(std::string name, Matrix4f rootTransform, std::vector<Joint> jointData)
      : mName{std::move(name)}, mRootTransform{rootTransform}, mJoints{std::move(jointData)}
    {
        mLastAnimation.joints.reserve(mJoints.size());
        mLastAnimation.jointsFinalTransform.reserve(mJoints.size());
        for (const auto& joint : mJoints)
        {
            AnimatedJoint newJoint;
            newJoint.parentIndex = joint.parentIndex;
            newJoint.updatedScale = joint.localTransform.scale;
            newJoint.updatedRotation = joint.localTransform.rotation;
            newJoint.updatedTranslation = joint.localTransform.translation;
            newJoint.localTransform = Matrix4f::identity();

            mLastAnimation.joints.push_back(std::move(newJoint));
            mLastAnimation.jointsFinalTransform.push_back(Matrix4f::identity());
        }
    }

    bool SkeletonAnim::animationAdd(size_t animId, Animation animation)
    {
        if (mAnimations.contains(animId) || animation.samplers.empty())
            return false;

        // some checks
        for (const auto& sampler : animation.samplers)
        {
            for (const auto& channel : sampler.channels)
            {
                // frame data must coincide with the number of timestamps
                if (channel.frameData.size() != sampler.timePoints.size())
                    return false;

                // joint index must be valid
                if (channel.jointIndex >= mJoints.size())
                    return false;
            }
        }

        mAnimations[animId] = std::move(animation);
        return true;
    }

    /*void SkeletonAnim::calculateBBoxes(const MeshAnim& baseMesh)
    {
        Mesh animatedMesh{ baseMesh.mesh() };

        //for each animation, calculate the timepoints to sample the bboxes
        for (auto& [animId, anim] : mAnimations)
        {
            anim.bboxes.clear();
            if (anim.samplers.empty())
                continue;

            double avgStep{0.0};
            for (const auto& sampler : anim.samplers)
            {
                if (sampler.timePoints.empty())
                    continue;

                double accum{0.0};
                for (size_t i = 1; i < sampler.timePoints.size(); i++)
                    accum += sampler.timePoints[i] - sampler.timePoints[i - 1];


                avgStep += accum / static_cast<double>(sampler.timePoints.size());
            }

            avgStep /= static_cast<double>(anim.samplers.size());

            anim.bboxes.reserve(Math::ftoi((anim.maxTimePoint - anim.minTimePoint) / avgStep) + 1);

            for (double curStep = anim.minTimePoint; curStep < anim.maxTimePoint; curStep+= avgStep)
                anim.bboxes.push_back(Animation::TimePointBBox{.timePoint = static_cast<float>(curStep), .bbox = {}});

            if (anim.bboxes.back().timePoint != anim.maxTimePoint)
                anim.bboxes.push_back(Animation::TimePointBBox{.timePoint = anim.maxTimePoint, .bbox = {}});
        }

        //we have all the timePoints calculated, now all we're missing are the bboxes
        for (auto& [animId, anim] : mAnimations)
        {
            for (auto& tbbox : anim.bboxes)
            {
                animate(animId, tbbox.timePoint);
                updateMesh(baseMesh, animatedMesh);

                tbbox.bbox = animatedMesh.getBoundingBox();
            }
        }
    }*/

    void SkeletonAnim::animate(float time)
    {
        if (mAnimations.empty())
            return;

        mLastAnimation.curTime += std::max(time, 0.0f) - mLastAnimation.lastTime;
        mLastAnimation.lastTime = time;

        assert(mAnimations.contains(mLastAnimation.curAnimId));

        // interpolate animations
        {
            // adjust time according to animation type
            if (mAnimationType == AnimationType::RepeatCurrent)
            {
                auto& anim = mAnimations[mLastAnimation.curAnimId];

                while (mLastAnimation.curTime >= anim.maxTimePoint)
                    mLastAnimation.curTime -= anim.maxTimePoint;
            }
            else if (mAnimationType == AnimationType::CycleAll)
            {
                auto& anim = mAnimations[mLastAnimation.curAnimId];
                if (mLastAnimation.curTime > anim.maxTimePoint)
                {
                    // find the next animation
                    auto it = mAnimations.find(mLastAnimation.curAnimId);
                    assert(it != mAnimations.end());

                    it++;
                    if (it == mAnimations.end())
                        it = mAnimations.begin();

                    while (mLastAnimation.curTime >= anim.maxTimePoint)
                        mLastAnimation.curTime -= anim.maxTimePoint;
                    while (mLastAnimation.curTime >= it->second.maxTimePoint)
                        mLastAnimation.curTime -= it->second.maxTimePoint;
                    mLastAnimation.curAnimId = it->first;

                    // easier to just restart
                    animate(time);
                }
            }

            auto& anim = mAnimations[mLastAnimation.curAnimId];
            assert(mLastAnimation.curTime < anim.maxTimePoint);

            // animate
            for (const auto& sampler : anim.samplers)
            {
                if ((mLastAnimation.curTime < sampler.minTimePoint) || (mLastAnimation.curTime > sampler.maxTimePoint))
                    continue;

                // pick the correct frame
                size_t frameIndex{0};
                for (;; frameIndex++)
                {
                    assert((frameIndex + 1) < sampler.timePoints.size());
                    if ((mLastAnimation.curTime >= sampler.timePoints[frameIndex]) && (mLastAnimation.curTime <= sampler.timePoints[frameIndex + 1]))
                        break;
                }

                // normalize t (for this sampler)
                float samplerTime;
                {
                    auto timeStart = sampler.timePoints[frameIndex];
                    auto timeEnd = sampler.timePoints[frameIndex + 1];
                    samplerTime = Math::fClamp((mLastAnimation.curTime - timeStart) / (timeEnd - timeStart), 0.0f, 1.0f);
                }

                // animate all channels
                for (const auto& channel : sampler.channels)
                {
                    const auto& dataStart = channel.frameData[frameIndex];
                    const auto& dataNext = channel.frameData[frameIndex + 1];

                    auto& jointAnimated = mLastAnimation.joints[channel.jointIndex];

                    switch (channel.target)
                    {
                        case Animation::Channel::Target::Scale:
                        {
                            jointAnimated.updatedScale = Vector4f::calcLerp(dataStart, dataNext, samplerTime).convert<float, 3>();
                            break;
                        }
                        case Animation::Channel::Target::Translation:
                        {
                            jointAnimated.updatedTranslation = Vector4f::calcLerp(dataStart, dataNext, samplerTime).convert<float, 3>();
                            break;
                        }
                        case Animation::Channel::Target::Rotation:
                        {
                            jointAnimated.updatedRotation = Quaternionf::sLerp(Quaternionf::from(dataStart), Quaternionf::from(dataNext), samplerTime);
                            break;
                        }
                        case Animation::Channel::Target::None:
                        default:
                            break;
                    }
                }
            }
        }

        // calculate for each joint the new transformation and store it in the corresponding node
        for (size_t jIndex = 0; jIndex < mLastAnimation.joints.size(); jIndex++)
        {
            auto& jointAnimated = mLastAnimation.joints[jIndex];

            jointAnimated.localTransform = Matrix4f::scale(jointAnimated.updatedScale);
            jointAnimated.localTransform *= jointAnimated.updatedRotation;
            jointAnimated.localTransform *= Matrix4f::translation(jointAnimated.updatedTranslation);

            jointAnimated.localTransform *= mJoints[jIndex].localTransform.matrix;
        }

        // calculate final transformation for each joint
        for (size_t jIndex = 0; jIndex < mJoints.size(); jIndex++)
            mLastAnimation.jointsFinalTransform[jIndex] = mJoints[jIndex].transformInvert * calcJointGlobalTransform(jIndex);
    }

    void SkeletonAnim::animateSetup(AnimationType animationType, size_t startAnimId)
    {
        if (mAnimations.empty())
            return;

        mAnimationType = animationType;
        if ((mAnimations.size() == 1) && (mAnimationType == AnimationType::CycleAll))
            mAnimationType = AnimationType::RepeatCurrent;

        auto it = mAnimations.find(startAnimId);
        mLastAnimation.curAnimId = (it != mAnimations.end()) ? startAnimId : mAnimations.begin()->first;
    }

    void SkeletonAnim::animateReset()
    {
        for (size_t jIndex = 0; jIndex < mJoints.size(); jIndex++)
            mLastAnimation.jointsFinalTransform[jIndex] = Matrix4f::identity();
    }

    void SkeletonAnim::updateMesh(const MeshAnim& baseMesh, TMesh& animatedMesh) const
    {
        if (baseMesh.mesh().numVertices() != animatedMesh.numVertices())
            return;

        animateMesh(mRootTransform, mLastAnimation.jointsFinalTransform, baseMesh, animatedMesh);
    }
}
