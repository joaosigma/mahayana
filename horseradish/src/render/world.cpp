#include "world.hpp"

#include "common/encoders.hpp"
#include "common/imageFactory.hpp"
#include "common/fileSystem.hpp"

#include "libs/lz4/lz4.h"
#include "libs/lz4/lz4hc.h"
#include "libs/cppformat/format.h"
#include "libs/rapidjson/document.h"
#include "libs/rapidjson/rapidjson.h"
#include "libs/rapidjson/prettywriter.h"
#include "libs/rapidjson/stringbuffer.h"
#include "libs/tinyobjloader/tiny_obj_loader.h"

#include <array>
#include <cstdint>
#include <algorithm>
#include <unordered_set>
#include <experimental/filesystem>

namespace hr { namespace render
{
	namespace
	{
#pragma pack(push, 1)
		std::array<unsigned char, 6> GeomFileSig = { 'h', 'r', 'g', 'e', 'o', 'm' };

		struct GeomHeader
		{
			unsigned char fileSig[GeomFileSig.size()];
			unsigned char version;
			std::uint32_t numGeoms;
			std::uint32_t numAnimSets; //new
			std::uint32_t numAnims; //new
		};

		/*struct GeomChunkInfo
		{
			std::uint32_t id;
			std::uint32_t geomsOffset;
			std::uint32_t size;
			std::uint32_t numVertices;
			std::uint32_t numIndices;
			float bboxMin[3], bboxMax[3];
		};*/

		enum class GeomType : std::uint16_t { Static = 1, Animated = 2};
		enum class GeomFlags : std::uint32_t { None = 0, AnimExtraBoneSet = (1<<0) };

		struct GeomChunkInfo
		{
			std::uint32_t id;
			std::uint16_t type;
			std::uint32_t flags;
			std::uint32_t geomsOffset;
			std::uint32_t size;
			std::uint32_t numVertices;
			std::uint32_t numIndices;
			std::uint32_t animSetId;
		};

		struct AnimSetChunkInfo
		{
			std::uint32_t id;
			std::uint32_t flags;
			std::uint32_t geomsOffset;
			std::uint32_t size;
			std::uint16_t numJoints;
		};

		struct AnimChunkInfo
		{
			std::uint32_t id;
			std::uint32_t flags;
			std::uint32_t geomsOffset;
			std::uint32_t size;
			float frameRate;
			std::uint32_t numFrames;
			std::uint32_t animSetId;
		};
#pragma pack(pop)

		bool fileAddData(hr::streams::FileStream& fstream, size_t numGeoms, size_t numAnimSets, size_t numAnims, std::function<bool(hr::streams::StreamWriter&)> cbWriteData, std::function<bool(hr::streams::StreamWriter&)> cbWriteGeomChunk, std::function<bool(hr::streams::StreamWriter&)> cbWriteAnimSetChunk, std::function<bool(hr::streams::StreamWriter&)> cbWriteAnimChunk)
		{
			std::unique_ptr<GeomChunkInfo[]> geomChunksTmp;
			std::unique_ptr<AnimChunkInfo[]> animChunksTmp;
			std::unique_ptr<AnimSetChunkInfo[]> animSetChunksTmp;

			int geomChunksSize = sizeof(GeomChunkInfo) * numGeoms;
			int animChunksSize = sizeof(AnimChunkInfo) * numAnims;
			int animSetchunksSize = sizeof(AnimSetChunkInfo) * numAnimSets;

			if ((numGeoms > 0) || (numAnimSets > 0) || (numAnims > 0))
			{
				hr::streams::StreamReader freader(fstream);

				if (numGeoms > 0)
				{
					geomChunksTmp = std::unique_ptr<GeomChunkInfo[]>(new GeomChunkInfo[numGeoms]);

					freader.seek(hr::streams::Stream::SeekOrigin::End, -(geomChunksSize + animSetchunksSize + animChunksSize));
					if (freader.read(geomChunksTmp.get(), geomChunksSize) != geomChunksSize)
						return false;
				}

				if (numAnimSets > 0)
				{
					animSetChunksTmp = std::unique_ptr<AnimSetChunkInfo[]>(new AnimSetChunkInfo[numAnimSets]);

					freader.seek(hr::streams::Stream::SeekOrigin::End, -(animSetchunksSize + animChunksSize));
					if (freader.read(animSetChunksTmp.get(), animSetchunksSize) != animSetchunksSize)
						return false;
				}

				if (numAnims > 0)
				{
					animChunksTmp = std::unique_ptr<AnimChunkInfo[]>(new AnimChunkInfo[numAnims]);

					freader.seek(hr::streams::Stream::SeekOrigin::End, -animChunksSize);
					if (freader.read(animChunksTmp.get(), animChunksSize) != animChunksSize)
						return false;
				}
			}

			hr::streams::StreamWriter fwriter(fstream);

			{
				fwriter.seek(hr::streams::Stream::SeekOrigin::End, -(geomChunksSize + animSetchunksSize + animChunksSize));

				if (!cbWriteData(fwriter))
					return false;
			}

			if (geomChunksTmp)
			{
				if (fwriter.write(geomChunksTmp.get(), geomChunksSize) != geomChunksSize)
					return false;
			}

			if (cbWriteGeomChunk && !cbWriteGeomChunk(fwriter))
				return false;

			if (animSetChunksTmp)
			{
				if (fwriter.write(animSetChunksTmp.get(), animSetchunksSize) != animSetchunksSize)
					return false;
			}

			if (cbWriteAnimSetChunk && !cbWriteAnimSetChunk(fwriter))
				return false;

			if (animChunksTmp)
			{
				if (fwriter.write(animChunksTmp.get(), animChunksSize) != animChunksSize)
					return false;
			}

			if (cbWriteAnimChunk && !cbWriteAnimChunk(fwriter))
				return false;

			return true;
		}
	}

	class RendererObjectProxy
		: public hr::render::IRenderObject
	{
	public:
		size_t objectId = 0;
		struct {
			hr::BBox bbox;
			size_t numVertices = 0, numIndices = 0;
			size_t fstreamVertexOffset = 0, fstreamIndexOffset = 0;
		} geom;
		std::string matDiffusePath, matNormalPath;
		hr::streams::FileStream& fileStream;

	public:
		RendererObjectProxy(hr::streams::FileStream& fileStream)
			: fileStream(fileStream)
		{ }

	private:
		size_t id() const override
		{
			return objectId;
		}

		BBox bbox() const override
		{
			return geom.bbox;
		}

		size_t numVertices() const override
		{
			return geom.numVertices;
		}

		size_t numIndices() const override
		{
			return geom.numIndices;
		}

		std::string_view texDiffusePath() const override
		{
			return matDiffusePath;
		}

		std::string_view texNormalPath() const override
		{
			return matNormalPath;
		}

		size_t readVertices(void* const destBuffer, size_t requestedDataSize) const override
		{
			fileStream.seek(hr::streams::Stream::SeekOrigin::Begin, geom.fstreamVertexOffset);
			
			auto bytesRead = fileStream.read(destBuffer, requestedDataSize);
			assert(bytesRead == requestedDataSize);
			
			return bytesRead;
		}

		size_t readIndices(void* const destBuffer, size_t requestedDataSize) const override
		{
			fileStream.seek(hr::streams::Stream::SeekOrigin::Begin, geom.fstreamIndexOffset);
			
			auto bytesRead = fileStream.read(destBuffer, requestedDataSize);
			assert(bytesRead == requestedDataSize);

			return bytesRead;
		}
	};

	bool World::geomFileCreate(hr::streams::FileStream& fstream)
	{
		GeomHeader geomHeader;
		geomHeader.numGeoms = 0;
		geomHeader.numAnims = 0;
		geomHeader.numAnimSets = 0;
		geomHeader.version = 1;
		std::memcpy(geomHeader.fileSig, GeomFileSig.data(), sizeof(geomHeader.fileSig));

		fstream.write(&geomHeader, sizeof(geomHeader));
		return true;
	}

	bool World::geomFileAddMesh(hr::streams::FileStream& fstream, size_t geomId, const hr::geom::Mesh& mesh)
	{
		size_t numGeoms = 0, numAnimSets = 0, numAnims = 0;

		//update file header
		{
			GeomHeader geomHeader;
			{
				hr::streams::StreamReader stream(fstream);

				stream.seek(hr::streams::Stream::SeekOrigin::Begin, 0);
				if (stream.read(&geomHeader, sizeof(GeomHeader)) != sizeof(GeomHeader))
					return false;
			}

			numGeoms = geomHeader.numGeoms;
			numAnims = geomHeader.numAnims;
			numAnimSets = geomHeader.numAnimSets;

			geomHeader.numGeoms++;

			{
				hr::streams::StreamWriter stream(fstream);

				stream.seek(hr::streams::Stream::SeekOrigin::Begin, 0);
				if (stream.write(&geomHeader, sizeof(GeomHeader)) != sizeof(GeomHeader))
					return false;
			}
		}

		GeomChunkInfo newGeomChunk;
		newGeomChunk.id = geomId;
		newGeomChunk.type = static_cast<std::uint16_t>(GeomType::Static);
		newGeomChunk.flags = static_cast<std::uint32_t>(GeomFlags::None);
		newGeomChunk.numVertices = mesh.numVertices();
		newGeomChunk.numIndices = mesh.numIndices();
		newGeomChunk.size = mesh.sizeVertices() + mesh.sizeIndices() + (sizeof(float) * 6); //vertices + indices + bbox
		newGeomChunk.geomsOffset = 0;
		newGeomChunk.animSetId = 0;
	
		auto success = fileAddData(fstream, numGeoms, numAnimSets, numAnims, [&mesh, &newGeomChunk](hr::streams::StreamWriter& fwriter)
		{
			newGeomChunk.geomsOffset = fwriter.position();

			if (fwriter.write(mesh.vertices(), mesh.sizeVertices()) != mesh.sizeVertices())
				return false;
			if (fwriter.write(mesh.indices(), mesh.sizeIndices()) != mesh.sizeIndices())
				return false;

			auto bbox = mesh.getBoundingBox();

			if (fwriter.write(bbox.min().data(), sizeof(float) * 3) != sizeof(float) * 3)
				return false;
			if (fwriter.write(bbox.max().data(), sizeof(float) * 3) != sizeof(float) * 3)
				return false;

			return true;

		}, [&newGeomChunk](hr::streams::StreamWriter& fwriter)
		{
			if (fwriter.write(&newGeomChunk, sizeof(GeomChunkInfo)) != sizeof(GeomChunkInfo))
				return false;

			return true;

		}, nullptr, nullptr);

		return success;
	}

	bool World::geomFileAddMeshAnim(hr::streams::FileStream& fstream, size_t geomId, const hr::geom::MeshAnim& meshAnim, size_t animSetId)
	{
		size_t numGeoms = 0, numAnimSets = 0, numAnims = 0;

		//update file header
		{
			GeomHeader geomHeader;
			{
				hr::streams::StreamReader stream(fstream);

				stream.seek(hr::streams::Stream::SeekOrigin::Begin, 0);
				if (stream.read(&geomHeader, sizeof(GeomHeader)) != sizeof(GeomHeader))
					return false;
			}

			numGeoms = geomHeader.numGeoms;
			numAnims = geomHeader.numAnims;
			numAnimSets = geomHeader.numAnimSets;

			geomHeader.numGeoms++;

			{
				hr::streams::StreamWriter stream(fstream);

				stream.seek(hr::streams::Stream::SeekOrigin::Begin, 0);
				if (stream.write(&geomHeader, sizeof(GeomHeader)) != sizeof(GeomHeader))
					return false;
			}
		}

		GeomChunkInfo newGeomChunk;
		newGeomChunk.id = geomId;
		newGeomChunk.type = static_cast<std::uint16_t>(GeomType::Animated);
		newGeomChunk.flags = static_cast<std::uint32_t>((meshAnim.skinningType() == geom::MeshAnim::SkinningType::Vertex8Joints) ? GeomFlags::AnimExtraBoneSet : GeomFlags::None);
		newGeomChunk.numVertices = meshAnim.mesh().numVertices();
		newGeomChunk.numIndices = meshAnim.mesh().numIndices();
		newGeomChunk.size = meshAnim.mesh().sizeVertices() + meshAnim.mesh().sizeIndices() + (sizeof(float) * 6) + meshAnim.sizeVerticesJoints(); //vertices + indices + bbox + vertexJoints
		newGeomChunk.geomsOffset = 0;
		newGeomChunk.animSetId = animSetId;

		auto success = fileAddData(fstream, numGeoms, numAnimSets, numAnims, [&meshAnim, &newGeomChunk](hr::streams::StreamWriter& fwriter)
		{
			newGeomChunk.geomsOffset = fwriter.position();

			if (fwriter.write(meshAnim.mesh().vertices(), meshAnim.mesh().sizeVertices()) != meshAnim.mesh().sizeVertices())
				return false;
			if (fwriter.write(meshAnim.mesh().indices(), meshAnim.mesh().sizeIndices()) != meshAnim.mesh().sizeIndices())
				return false;

			auto bbox = meshAnim.mesh().getBoundingBox();

			if (fwriter.write(bbox.min().data(), sizeof(float) * 3) != sizeof(float) * 3)
				return false;
			if (fwriter.write(bbox.max().data(), sizeof(float) * 3) != sizeof(float) * 3)
				return false;

			if (fwriter.write(meshAnim.verticesJoints(), meshAnim.sizeVerticesJoints()) != meshAnim.sizeVerticesJoints())
				return false;

			return true;

		}, [&newGeomChunk](hr::streams::StreamWriter& fwriter)
		{
			if (fwriter.write(&newGeomChunk, sizeof(GeomChunkInfo)) != sizeof(GeomChunkInfo))
				return false;

			return true;

		}, nullptr, nullptr);

		return success;
	}

	bool World::geomFileAddAnimationSet(hr::streams::FileStream& fstream, size_t animSetId, const hr::geom::MeshAnimSet& animSet)
	{
		size_t numGeoms = 0, numAnimSets = 0, numAnims = 0;

		//update file header
		{
			GeomHeader geomHeader;
			{
				hr::streams::StreamReader stream(fstream);

				stream.seek(hr::streams::Stream::SeekOrigin::Begin, 0);
				if (stream.read(&geomHeader, sizeof(GeomHeader)) != sizeof(GeomHeader))
					return false;
			}

			numGeoms = geomHeader.numGeoms;
			numAnims = geomHeader.numAnims;
			numAnimSets = geomHeader.numAnimSets;

			geomHeader.numAnimSets++;

			{
				hr::streams::StreamWriter stream(fstream);

				stream.seek(hr::streams::Stream::SeekOrigin::Begin, 0);
				if (stream.write(&geomHeader, sizeof(GeomHeader)) != sizeof(GeomHeader))
					return false;
			}
		}

		AnimSetChunkInfo newAnimSetChunk;
		newAnimSetChunk.id = animSetId;
		newAnimSetChunk.flags = 0;
		newAnimSetChunk.size = (sizeof(float) * 16) * animSet.numJoints(); //one mat4x4 for each joint
		newAnimSetChunk.numJoints = animSet.numJoints();
		newAnimSetChunk.geomsOffset = 0;

		auto success = fileAddData(fstream, numGeoms, numAnimSets, numAnims, [&animSet, &newAnimSetChunk](hr::streams::StreamWriter& fwriter)
		{
			newAnimSetChunk.geomsOffset = fwriter.position();

			for (size_t curJoint = 0; curJoint < animSet.numJoints(); curJoint++)
			{
				auto& mat = animSet.bindPoseInvertedMat(curJoint);
				if (fwriter.write(mat.data(), sizeof(float) * 16) != sizeof(float) * 16)
					return false;
			}

			return true;

		}, nullptr, [&newAnimSetChunk](hr::streams::StreamWriter& fwriter)
		{
			if (fwriter.write(&newAnimSetChunk, sizeof(AnimSetChunkInfo)) != sizeof(AnimSetChunkInfo))
				return false;

			return true;

		}, nullptr);

		return success;
	}

	bool World::geomFileAddAnimation(hr::streams::FileStream& fstream, size_t animId, size_t animSetId, float frameRate, const std::vector<hr::geom::MeshAnimSet::Frame>& animation)
	{
		size_t numGeoms = 0, numAnimSets = 0, numAnims = 0;

		//update file header
		{
			GeomHeader geomHeader;
			{
				hr::streams::StreamReader stream(fstream);

				stream.seek(hr::streams::Stream::SeekOrigin::Begin, 0);
				if (stream.read(&geomHeader, sizeof(GeomHeader)) != sizeof(GeomHeader))
					return false;
			}

			numGeoms = geomHeader.numGeoms;
			numAnims = geomHeader.numAnims;
			numAnimSets = geomHeader.numAnimSets;

			geomHeader.numAnims++;

			{
				hr::streams::StreamWriter stream(fstream);

				stream.seek(hr::streams::Stream::SeekOrigin::Begin, 0);
				if (stream.write(&geomHeader, sizeof(GeomHeader)) != sizeof(GeomHeader))
					return false;
			}
		}

		AnimChunkInfo newAnimChunk;
		newAnimChunk.id = animId;
		newAnimChunk.flags = 0;
		newAnimChunk.animSetId = animSetId;
		newAnimChunk.size = ((sizeof(float) * (3 + 4)) * animation.size()) + (sizeof(float) * 6); //one (vec3 + quaternion) for each frame of animation plus the bbox
		newAnimChunk.frameRate = frameRate;
		newAnimChunk.numFrames = animation.size();
		newAnimChunk.geomsOffset = 0;

		auto success = fileAddData(fstream, numGeoms, numAnimSets, numAnims, [&animation, &newAnimChunk](hr::streams::StreamWriter& fwriter)
		{
			newAnimChunk.geomsOffset = fwriter.position();

			for (const auto& curFrame : animation)
			{
				for (const auto& curJoint : curFrame.joints)
				{
					if (fwriter.write(curJoint.pos.data(), sizeof(float) * 3) != sizeof(float) * 3)
						return false;
					if (fwriter.write(curJoint.rot.data(), sizeof(float) * 4) != sizeof(float) * 4)
						return false;
				}

				if (fwriter.write(curFrame.bbox.min().data(), sizeof(float) * 3) != sizeof(float) * 3)
					return false;
				if (fwriter.write(curFrame.bbox.max().data(), sizeof(float) * 3) != sizeof(float) * 3)
					return false;
			}

			return true;

		}, nullptr, nullptr, [&newAnimChunk](hr::streams::StreamWriter& fwriter)
		{
			if (fwriter.write(&newAnimChunk, sizeof(AnimChunkInfo)) != sizeof(AnimChunkInfo))
				return false;

			return true;

		});

		return success;
	}

	World::World()
	{ }

	World::~World()
	{
		std::vector<AreaId> ids;

		ids.reserve(mAreas.size());
		for (auto&& [areaId, area] : mAreas)
			ids.push_back(areaId);

		for (auto&& areaId : ids)
			unloadArea(areaId);
	}

	World::AreaId World::loadArea(IRenderer& renderer, const std::string& scenePath, const std::string& geomPath)
	{
		auto id = mGenAreaIds++;

		auto& area = mAreas[id];
		area.id = id;

		auto success = load(renderer, area, scenePath, geomPath);
		if (!success)
		{
			unloadArea(id);
			return 0;
		}

		return id;
	}

	void World::unloadArea(AreaId areaId)
	{

	}

	void World::prepareNextFrame(Timestep& timestep, IRenderer& renderer, const tools::Camera& hrCamera, const hr::gl::tools::Viewport& hrViewport)
	{
		auto camPos = hrCamera.getPos();

		hr::gl::tools::Frustum camFrustum;
		camFrustum.setCamPosition(camPos);
		camFrustum.setZNear(hrViewport.znear());
		camFrustum.setZFar(hrViewport.zfar());
		camFrustum.calculateFrustum(hrViewport.getProjection(hr::gl::tools::Viewport::ProjectionType::Proj3D), hrCamera.modelView());

		//run animations and update vertex data
		{
			class RendererObjectProxy
				: public hr::render::IRenderObject
			{
			public:
				size_t objectId = 0;
				hr::geom::Mesh& mesh;

			public:
				RendererObjectProxy(size_t objectId, hr::geom::Mesh& mesh)
					: objectId(objectId)
					, mesh(mesh)
				{ }

			private:
				size_t id() const override
				{
					return objectId;
				}

				BBox bbox() const override
				{
					return {};
				}

				size_t numVertices() const override
				{
					return mesh.numVertices();
				}

				size_t numIndices() const override
				{
					return 0;
				}

				std::string_view texDiffusePath() const override
				{
					return {};
				}

				std::string_view texNormalPath() const override
				{
					return {};
				}

				size_t readVertices(void* const destBuffer, size_t requestedDataSize) const override
				{
					assert(requestedDataSize == mesh.sizeVertices());
					std::memcpy(destBuffer, mesh.vertices(), requestedDataSize);

					return requestedDataSize;
				}

				size_t readIndices(void* const destBuffer, size_t requestedDataSize) const override
				{
					return 0;
				}
			};

			struct RendererProxy
				: public IRenderObjectManager
			{
				std::vector<RendererObjectProxy>& renderObjects;

				RendererProxy(std::vector<RendererObjectProxy>& renderObjects)
					: renderObjects(renderObjects)
				{ }

				size_t numObjects() const override
				{
					return renderObjects.size();
				}

				void iterateObjects(std::function<void(IRenderObject&)> cb) const override
				{
					if (!cb)
						return;

					for (auto& object : renderObjects)
						cb(object);
				}
			};

			std::vector<RendererObjectProxy> targetObjects;

			for (auto&[areaId, area] : mAreas)
			{
				for (auto&[animSetId, animSet] : area.mAnimationSets)
					animSet.animationSet.animate(1, timestep.t);

				targetObjects.clear();

				for (auto&[objectId, object] : area.mObjects)
				{
					if (!object.anim.hasAnim)
						continue;

					auto animSet = area.mAnimationSets.find(object.anim.animSetId);
					if (animSet == area.mAnimationSets.end())
						continue;

					/*if (!camFrustum.testBox(animSet->second.animationSet.updatedBBox())) //no point in updating the object if its entire anim set is not visible
						continue;*/

					animSet->second.animationSet.updateMesh(object.anim.meshAnim, object.anim.meshAnimated);
					
					targetObjects.push_back(RendererObjectProxy(objectId, object.anim.meshAnimated));
				}

				renderer.updateVertexData(area.sceneId, RendererProxy(targetObjects));
			}
		}

		{
			std::vector<IRenderObject::ObjectId> targetObjects;

			for (const auto&[areaId, area] : mAreas)
			{
				targetObjects.clear();

				for (auto& instance : area.mInstances)
				{
					auto itObject = area.mObjects.find(instance.objectId);
					if (itObject == area.mObjects.end())
						continue;

					if (itObject->second.anim.hasAnim)
					{
						auto animSet = area.mAnimationSets.find(itObject->second.anim.animSetId);
						if (animSet == area.mAnimationSets.end())
							continue;

						/*if (!camFrustum.testBox(animSet->second.animationSet.updatedBBox())) //no point in updating the object if its entire anim set is not visible
							continue;*/
					}
					else
					{
						if (!camFrustum.testBox(instance.bbox))
							continue;
					}

					targetObjects.push_back(instance.objectId);
				}

				renderer.prepareNextFrame(area.sceneId, targetObjects);
			}
		}
	}

	bool World::loadAnimations(hr::streams::StreamReader& sreader, Area& area)
	{
		std::vector<AnimChunkInfo> animInfos;
		std::vector<AnimSetChunkInfo> animSetInfos;

		//read infos
		{
			GeomHeader geomHeader;
			if (sreader.read(&geomHeader, sizeof(GeomHeader)) != sizeof(GeomHeader))
				return false;

			int animChunksSize = sizeof(AnimChunkInfo) * geomHeader.numAnims;
			int animSetchunksSize = sizeof(AnimSetChunkInfo) * geomHeader.numAnimSets;

			if (geomHeader.numAnimSets > 0)
			{
				sreader.seek(hr::streams::Stream::SeekOrigin::End, -(animSetchunksSize + animChunksSize));

				animSetInfos.reserve(geomHeader.numAnimSets);
				for (size_t curGeom = 0; curGeom < geomHeader.numAnimSets; ++curGeom)
				{
					AnimSetChunkInfo chunkInfo;
					sreader.read(&chunkInfo, sizeof(AnimSetChunkInfo));

					animSetInfos.push_back(std::move(chunkInfo));
				}
			}

			if (geomHeader.numAnims > 0)
			{
				sreader.seek(hr::streams::Stream::SeekOrigin::End, -animChunksSize);

				animInfos.reserve(geomHeader.numAnims);
				for (size_t curGeom = 0; curGeom < geomHeader.numAnims; ++curGeom)
				{
					AnimChunkInfo chunkInfo;
					sreader.read(&chunkInfo, sizeof(AnimChunkInfo));

					animInfos.push_back(std::move(chunkInfo));
				}
			}
		}

		//read everything
		for (const auto& animSetInfo : animSetInfos)
		{
			auto& animSet = area.mAnimationSets[animSetInfo.id];

			{
				std::vector<Matrix> bindPoseMats;
				bindPoseMats.resize(animSetInfo.numJoints);

				sreader.seek(hr::streams::Stream::SeekOrigin::Begin, animSetInfo.geomsOffset);

				for (size_t curJointIndex = 0; curJointIndex < animSetInfo.numJoints; curJointIndex++)
					sreader.read(bindPoseMats[curJointIndex].data(), sizeof(float) * 16);

				animSet.animationSet = geom::MeshAnimSet(std::move(bindPoseMats));
			}

			for (const auto& animInfo : animInfos)
			{
				if (animInfo.animSetId != animSetInfo.id)
					continue;

				sreader.seek(hr::streams::Stream::SeekOrigin::Begin, animInfo.geomsOffset);

				std::vector<geom::MeshAnimSet::Frame> animFrames;
				animFrames.reserve(animInfo.numFrames);

				for (size_t curFrameIndex = 0; curFrameIndex < animInfo.numFrames; curFrameIndex++)
				{
					std::vector<geom::MeshAnimSet::Joint> skeleton;
					skeleton.resize(animSetInfo.numJoints);

					for (size_t curJointIndex = 0; curJointIndex < animSetInfo.numJoints; curJointIndex++)
					{
						sreader.read(skeleton[curJointIndex].pos.data(), sizeof(float) * 3);
						sreader.read(skeleton[curJointIndex].rot.data(), sizeof(float) * 4);
					}

					Vector3f bboxMin, bboxMax;
					sreader.read(bboxMin.data(), sizeof(float) * 3);
					sreader.read(bboxMax.data(), sizeof(float) * 3);

					geom::MeshAnimSet::Frame newFrame;
					newFrame.bbox.setMinMax(bboxMin, bboxMax);
					newFrame.joints = std::move(skeleton);

					animFrames.push_back(std::move(newFrame));
				}

				animSet.animationSet.animAdd(animInfo.id, animInfo.frameRate, std::move(animFrames));
			}
		}

		return true;
	}

	bool World::load(IRenderer& renderer, Area& area, const std::string& scenePath, const std::string& geomPath)
	{
		hr::streams::FileStream sceneFileStream(scenePath, true, false);
		hr::streams::StreamReader streamScene(sceneFileStream);

		hr::streams::FileStream geomFileStream(geomPath, true, false);
		hr::streams::StreamReader streamGeom(geomFileStream);

		//check file
		{
			GeomHeader geomHeader;
			if (streamGeom.read(&geomHeader, sizeof(GeomHeader)) != sizeof(GeomHeader))
				return false;
			if (std::memcmp(geomHeader.fileSig, GeomFileSig.data(), sizeof(geomHeader.fileSig)) != 0)
				return false;

			streamGeom.seek(hr::streams::Stream::SeekOrigin::Begin, 0);
		}

		//load all animation sets and corresponding animations
		{
			loadAnimations(streamGeom, area);
			streamGeom.seek(hr::streams::Stream::SeekOrigin::Begin, 0);
		}

		std::map<size_t, RendererObjectProxy> mRendererObjects;

		//read objects and instances
		{
			std::unordered_map<size_t, GeomChunkInfo> geomInfo;

			//read geom infos
			{
				GeomHeader geomHeader;
				if (streamGeom.read(&geomHeader, sizeof(GeomHeader)) != sizeof(GeomHeader))
					return false;
				if (std::memcmp(geomHeader.fileSig, GeomFileSig.data(), sizeof(geomHeader.fileSig)) != 0)
					return false;

				int geomChunksSize = sizeof(GeomChunkInfo) * geomHeader.numGeoms;
				int animChunksSize = sizeof(AnimChunkInfo) * geomHeader.numAnims;
				int animSetchunksSize = sizeof(AnimSetChunkInfo) * geomHeader.numAnimSets;

				if (geomHeader.numGeoms > 0)
				{
					streamGeom.seek(hr::streams::Stream::SeekOrigin::End, -(geomChunksSize + animSetchunksSize + animChunksSize));

					geomInfo.rehash(geomHeader.numGeoms);
					for (size_t curGeom = 0; curGeom < geomHeader.numGeoms; ++curGeom)
					{
						GeomChunkInfo chunkInfo;
						streamGeom.read(&chunkInfo, sizeof(GeomChunkInfo));

						geomInfo[chunkInfo.id] = chunkInfo;
					}
				}
			}

			//read scene
			{
				std::shared_ptr<unsigned char> buffer;
				size_t bufferSize;

				streamScene.stream().cloneAllContent(buffer, bufferSize, [](size_t requiredSize)
				{
					std::shared_ptr<unsigned char> buffer(new unsigned char[requiredSize + 1], std::default_delete<unsigned char[]>());
					buffer.get()[requiredSize] = '\0';
					return buffer;
				});

				rapidjson::Document d;
				d.ParseInsitu(reinterpret_cast<char*>(buffer.get()));

				{
					auto& jsonConcepts = d["objects"];
					assert(jsonConcepts.IsArray());

					for (rapidjson::Value::ConstValueIterator itr = jsonConcepts.Begin(); itr != jsonConcepts.End(); ++itr)
					{
						size_t objectId = (*itr)["id"].GetUint();

						auto objectType = static_cast<Object::Type>((*itr)["type"].GetUint());

						if (objectType == Object::Type::Static)
						{
							auto& object = area.mObjects[objectId];
							auto& objectRenderer = mRendererObjects.try_emplace(objectId, geomFileStream).first->second;

							object.id = objectId;
							object.type = Object::Type::Static;
							objectRenderer.objectId = objectId;

							//read geom info and bbox
							{
								auto geomIt = geomInfo.find(object.id);
								if (geomIt == geomInfo.end())
									continue;

								{
									streamGeom.seek(hr::streams::Stream::SeekOrigin::Begin, geomIt->second.geomsOffset);

									object.anim.hasAnim = (geomIt->second.type == static_cast<std::uint16_t>(GeomType::Animated));
									if (object.anim.hasAnim)
									{
										object.anim.animSetId = geomIt->second.animSetId;

										object.anim.meshAnimated = hr::geom::Mesh(geomIt->second.numVertices, geomIt->second.numIndices); //bind pose
										streamGeom.read(object.anim.meshAnimated.vertices(), object.anim.meshAnimated.sizeVertices());
										streamGeom.read(object.anim.meshAnimated.indices(), object.anim.meshAnimated.sizeIndices());
									}
									else
									{
										streamGeom.seek(hr::streams::Stream::SeekOrigin::Current, hr::geom::Mesh::sizeVertices(objectRenderer.geom.numVertices));
										streamGeom.seek(hr::streams::Stream::SeekOrigin::Current, hr::geom::Mesh::sizeIndices(objectRenderer.geom.numIndices));
									}

									{
										Vector3f bboxMin, bboxMax;

										streamGeom.read(bboxMin.data(), sizeof(float) * 3);
										streamGeom.read(bboxMax.data(), sizeof(float) * 3);

										object.bbox.setMinMax(bboxMin, bboxMax);
									}

									if (object.anim.hasAnim)
									{
										if ((geomIt->second.flags & static_cast<std::uint16_t>(GeomFlags::AnimExtraBoneSet)) != 0)
											object.anim.meshAnim = hr::geom::MeshAnim(object.anim.meshAnimated, hr::geom::MeshAnim::SkinningType::Vertex8Joints);
										else
											object.anim.meshAnim = hr::geom::MeshAnim(object.anim.meshAnimated, hr::geom::MeshAnim::SkinningType::Vertex4Joints);

										streamGeom.read(object.anim.meshAnim.verticesJoints(), object.anim.meshAnim.sizeVerticesJoints());
									}
								}

								objectRenderer.geom.numVertices = geomIt->second.numVertices;
								objectRenderer.geom.numIndices = geomIt->second.numIndices;
								objectRenderer.geom.fstreamVertexOffset = geomIt->second.geomsOffset;
								objectRenderer.geom.fstreamIndexOffset = objectRenderer.geom.fstreamVertexOffset + hr::geom::Mesh::sizeVertices(objectRenderer.geom.numVertices);
								objectRenderer.geom.bbox = object.bbox;
							}

							//read material info
							if ((*itr).HasMember("material"))
							{
								auto& jsonMaterial = ((*itr).FindMember("material"))->value;

								objectRenderer.matDiffusePath = jsonMaterial["diffusePath"].GetString();
								objectRenderer.matNormalPath = jsonMaterial["normalPath"].GetString();
							}
						}
					}
				}

				{
					auto& jsonObjects = d["instances"];
					assert(jsonObjects.IsArray());

					area.mInstances.reserve(jsonObjects.Size());
					for (rapidjson::Value::ConstValueIterator itr = jsonObjects.Begin(); itr != jsonObjects.End(); ++itr)
					{
						Instance newInstance;
						newInstance.type = static_cast<Instance::Type>((*itr)["type"].GetUint());
						newInstance.objectId = (*itr)["objectId"].GetUint();

						if (area.mObjects.find(newInstance.objectId) != area.mObjects.end())
						{
							newInstance.bbox = area.mObjects[newInstance.objectId].bbox;
							area.mInstances.push_back(newInstance);
						}
					}
				}
			}
		}

		//send data to the render
		{
			struct RendererProxy
				: public IRenderObjectManager
			{
				std::map<size_t, RendererObjectProxy>& renderObjects;

				RendererProxy(std::map<size_t, RendererObjectProxy>& renderObjects)
					: renderObjects(renderObjects)
				{ }

				size_t numObjects() const override
				{
					return renderObjects.size();
				}

				void iterateObjects(std::function<void(IRenderObject&)> cb) const override
				{
					if (!cb)
						return;

					for (auto&[objectId, object] : renderObjects)
						cb(object);
				}
			};

			area.sceneId = renderer.loadScene(RendererProxy(mRendererObjects));
		}

		return true;
	}
} }