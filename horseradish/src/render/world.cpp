#include "world.hpp"

#include "common/encoders.hpp"
#include "common/imageFactory.hpp"
#include "common/fileSystem.hpp"

#include "libs/lz4/lz4.h"
#include "libs/lz4/lz4hc.h"
#include "libs/fmt/format.h"
#include "libs/rapidjson/document.h"
#include "libs/rapidjson/rapidjson.h"
#include "libs/rapidjson/prettywriter.h"
#include "libs/rapidjson/stringbuffer.h"
#include "libs/tinyobjloader/tiny_obj_loader.h"

#include <array>
#include <cstdint>
#include <algorithm>
#include <unordered_set>

namespace hr { namespace render
{
	namespace
	{
#pragma pack(push, 1)
		std::array<unsigned char, 6> FileBinSig = { 'h', 'r', 'g', 'e', 'o', 'm' };

		struct GeomHeader
		{
			unsigned char fileSig[FileBinSig.size()];
			unsigned char version;
			uint32_t numGeoms;
			uint32_t numAnimSets;
			uint32_t numAnims;
			uint8_t reserved[1024];
		};

		enum class GeomType : uint16_t { Static = 1, Animated = 2};
		enum class GeomFlags : uint32_t { None = 0, AnimExtraBoneSet = (1<<0) };

		struct GeomChunkInfo
		{
			uint32_t id;
			uint16_t type;
			uint32_t flags;
			uint32_t geomsOffset;
			uint32_t size;
			uint32_t numVertices;
			uint32_t numIndices;
			uint32_t animSetId;
		};

		struct AnimSetChunkInfo
		{
			uint32_t id;
			uint32_t flags;
			uint32_t geomsOffset;
			uint32_t size;
			uint16_t numJoints;
		};

		struct AnimChunkInfo
		{
			uint32_t id;
			uint32_t flags;
			uint32_t geomsOffset;
			uint32_t size;
			float frameRate;
			uint32_t numFrames;
			uint32_t animSetId;
		};
#pragma pack(pop)

		using FileWriteDataCallback = std::function<bool(hr::streams::StreamWriter&)>;

		bool fileAddData(hr::streams::FileStream& fstream, size_t numGeoms, size_t numAnimSets, size_t numAnims, FileWriteDataCallback cbWriteData, FileWriteDataCallback cbWriteGeomChunk, FileWriteDataCallback cbWriteAnimSetChunk, FileWriteDataCallback cbWriteAnimChunk)
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

		bool fileRemoveData(hr::streams::FileStream& fstreamIn, hr::streams::FileStream& fstreamOut, std::vector<uint32_t> removeGeomsIds, std::vector<uint32_t> removeAnimSetIds, std::vector<uint32_t> removeAnimIds)
		{
			hr::streams::StreamReader freader(fstreamIn);
			hr::streams::StreamWriter fwriter(fstreamOut);

			GeomHeader geomHeader;

			std::vector<GeomChunkInfo> oldGeomChunks;
			std::vector<AnimChunkInfo> oldAnimChunks;
			std::vector<AnimSetChunkInfo> oldAnimSetChunks;

			//read header and every chunk info
			{
				freader.seek(hr::streams::Stream::SeekOrigin::Begin, 0);
				if (freader.read(&geomHeader, sizeof(GeomHeader)) != sizeof(GeomHeader))
					return false;

				int geomChunksSize = sizeof(GeomChunkInfo) * geomHeader.numGeoms;
				int animChunksSize = sizeof(AnimChunkInfo) * geomHeader.numAnims;
				int animSetchunksSize = sizeof(AnimSetChunkInfo) * geomHeader.numAnimSets;

				if (geomHeader.numGeoms > 0)
				{
					oldGeomChunks.resize(geomHeader.numGeoms);

					freader.seek(hr::streams::Stream::SeekOrigin::End, -(geomChunksSize + animSetchunksSize + animChunksSize));
					if (freader.read(oldGeomChunks.data(), geomChunksSize) != geomChunksSize)
						return false;
				}

				if (geomHeader.numAnimSets > 0)
				{
					oldAnimSetChunks.resize(geomHeader.numAnimSets);

					freader.seek(hr::streams::Stream::SeekOrigin::End, -(animSetchunksSize + animChunksSize));
					if (freader.read(oldAnimSetChunks.data(), animSetchunksSize) != animSetchunksSize)
						return false;
				}

				if (geomHeader.numAnims > 0)
				{
					oldAnimChunks.resize(geomHeader.numAnims);

					freader.seek(hr::streams::Stream::SeekOrigin::End, -animChunksSize);
					if (freader.read(oldAnimChunks.data(), animChunksSize) != animChunksSize)
						return false;
				}
			}

			{
				//make sure the all anim sets to remove actually exist
				removeAnimSetIds.erase(std::remove_if(removeAnimSetIds.begin(), removeAnimSetIds.end(), [&oldAnimSetChunks](uint32_t targetId)
				{
					return (std::find_if(oldAnimSetChunks.begin(), oldAnimSetChunks.end(), [targetId](const AnimSetChunkInfo& chunkInfo)
					{
						return (chunkInfo.id == targetId);
					}) == oldAnimSetChunks.end());
				}), removeAnimSetIds.end());


				//also erase every geom and animation related to anim sets (that are about to be removed)
				for (const auto& animSetId : removeAnimSetIds)
				{
					for (const auto& geomData : oldGeomChunks)
					{
						if (geomData.animSetId == animSetId)
							removeGeomsIds.push_back(geomData.id);
					}

					for (const auto& animData : oldAnimChunks)
					{
						if (animData.animSetId == animSetId)
							removeAnimIds.push_back(animData.id);
					}
				}

				//make sure the all geom and animations to remove actually exist

				removeGeomsIds.erase(std::remove_if(removeGeomsIds.begin(), removeGeomsIds.end(), [&oldGeomChunks](uint32_t targetId)
				{
					return (std::find_if(oldGeomChunks.begin(), oldGeomChunks.end(), [targetId](const GeomChunkInfo& chunkInfo)
					{
						return (chunkInfo.id == targetId);
					}) == oldGeomChunks.end());
				}), removeGeomsIds.end());

				removeAnimIds.erase(std::remove_if(removeAnimIds.begin(), removeAnimIds.end(), [&oldAnimChunks](uint32_t targetId)
				{
					return (std::find_if(oldAnimChunks.begin(), oldAnimChunks.end(), [targetId](const AnimChunkInfo& chunkInfo)
					{
						return (chunkInfo.id == targetId);
					}) == oldAnimChunks.end());
				}), removeAnimIds.end());
			}

			//just create a new set of chunks
			std::vector<GeomChunkInfo> newGeomChunks;
			std::vector<AnimChunkInfo> newAnimChunks;
			std::vector<AnimSetChunkInfo> newAnimSetChunks;

			{
				newGeomChunks.reserve(oldGeomChunks.size());
				std::copy_if(oldGeomChunks.begin(), oldGeomChunks.end(), std::back_inserter(newGeomChunks), [&removeGeomsIds](const GeomChunkInfo& chunkInfo)
				{
					return (std::find(removeGeomsIds.begin(), removeGeomsIds.end(), chunkInfo.id) == removeGeomsIds.end());
				});

				newAnimSetChunks.reserve(oldAnimSetChunks.size());
				std::copy_if(oldAnimSetChunks.begin(), oldAnimSetChunks.end(), std::back_inserter(newAnimSetChunks), [&removeAnimSetIds](const AnimSetChunkInfo& chunkInfo)
				{
					return (std::find(removeAnimSetIds.begin(), removeAnimSetIds.end(), chunkInfo.id) == removeAnimSetIds.end());
				});

				newAnimChunks.reserve(oldAnimChunks.size());
				std::copy_if(oldAnimChunks.begin(), oldAnimChunks.end(), std::back_inserter(newAnimChunks), [&removeAnimIds](const AnimChunkInfo& chunkInfo)
				{
					return (std::find(removeAnimIds.begin(), removeAnimIds.end(), chunkInfo.id) == removeAnimIds.end());
				});
			}

			//we can write a new header
			geomHeader.numGeoms = newGeomChunks.size();
			geomHeader.numAnims = newAnimChunks.size();
			geomHeader.numAnimSets = newAnimSetChunks.size();
			if (fwriter.write(&geomHeader, sizeof(GeomHeader)) != sizeof(GeomHeader))
				return false;

			//write all data chunks

			for (auto& chunkData : newGeomChunks)
			{
				chunkData.geomsOffset = fwriter.position();

				auto it = std::find_if(oldGeomChunks.begin(), oldGeomChunks.end(), [targetId = chunkData.id](const GeomChunkInfo& chunkInfo)
				{
					return (chunkInfo.id == targetId);
				});
				assert(it != oldGeomChunks.end());

				freader.seek(hr::streams::Stream::SeekOrigin::Begin, it->geomsOffset);

				if (fwriter.write(freader, chunkData.size) != chunkData.size)
					return false;
			}

			for (auto& chunkData : newAnimSetChunks)
			{
				chunkData.geomsOffset = fwriter.position();

				auto it = std::find_if(oldAnimSetChunks.begin(), oldAnimSetChunks.end(), [targetId = chunkData.id](const AnimSetChunkInfo& chunkInfo)
				{
					return (chunkInfo.id == targetId);
				});
				assert(it != oldAnimSetChunks.end());

				freader.seek(hr::streams::Stream::SeekOrigin::Begin, it->geomsOffset);

				if (fwriter.write(freader, chunkData.size) != chunkData.size)
					return false;
			}

			for (auto& chunkData : newAnimChunks)
			{
				chunkData.geomsOffset = fwriter.position();

				auto it = std::find_if(oldAnimChunks.begin(), oldAnimChunks.end(), [targetId = chunkData.id](const AnimChunkInfo& chunkInfo)
				{
					return (chunkInfo.id == targetId);
				});
				assert(it != oldAnimChunks.end());

				freader.seek(hr::streams::Stream::SeekOrigin::Begin, it->geomsOffset);

				if (fwriter.write(freader, chunkData.size) != chunkData.size)
					return false;
			}

			//write all chunks

			if (fwriter.write(newGeomChunks.data(), sizeof(GeomChunkInfo) * newGeomChunks.size()) != (sizeof(GeomChunkInfo) * newGeomChunks.size()))
				return false;

			if (fwriter.write(newAnimSetChunks.data(), sizeof(AnimSetChunkInfo) * newAnimSetChunks.size()) != (sizeof(AnimSetChunkInfo) * newAnimSetChunks.size()))
				return false;

			if (fwriter.write(newAnimChunks.data(), sizeof(AnimChunkInfo) * newAnimChunks.size()) != (sizeof(AnimChunkInfo) * newAnimChunks.size()))
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

	bool World::migrateBinData(const std::string& binPathOld, const std::string& binPathNew)
	{
		hr::streams::FileStream fstreamOld(binPathOld, true, false);
		hr::streams::FileStream fstreamNew(binPathNew, false, true);

		hr::streams::StreamReader streamOld(fstreamOld);
		hr::streams::StreamWriter streamNew(fstreamNew);

#pragma pack(push, 1)
		struct GeomHeaderOld
		{
			unsigned char fileSig[FileBinSig.size()];
			unsigned char version;
			uint32_t numGeoms;
		};

		struct GeomChunkInfoOld
		{
			uint32_t id;
			uint32_t geomsOffset;
			uint32_t size;
			uint32_t numVertices;
			uint32_t numIndices;
			float bboxMin[3], bboxMax[3];
		};
#pragma pack(pop)

		std::vector<GeomChunkInfoOld> oldGeomChunks;
		{
			GeomHeaderOld oldGeomHeader;
			if (streamOld.read(&oldGeomHeader, sizeof(GeomHeaderOld)) != sizeof(GeomHeaderOld))
				return false;
			if (std::memcmp(oldGeomHeader.fileSig, FileBinSig.data(), sizeof(oldGeomHeader.fileSig)) != 0)
				return false;

			if (oldGeomHeader.numGeoms > 0)
			{
				streamOld.seek(hr::streams::Stream::SeekOrigin::End, -(sizeof(GeomChunkInfoOld) * oldGeomHeader.numGeoms));

				oldGeomChunks.reserve(oldGeomHeader.numGeoms);
				for (size_t curGeom = 0; curGeom < oldGeomHeader.numGeoms; ++curGeom)
				{
					GeomChunkInfoOld chunkInfo;
					streamOld.read(&chunkInfo, sizeof(GeomChunkInfoOld));

					oldGeomChunks.push_back(std::move(chunkInfo));
				}
			}
		}

		//write the new header
		{
			GeomHeader geomHeader;
			geomHeader.numGeoms = oldGeomChunks.size();
			geomHeader.numAnims = 0;
			geomHeader.numAnimSets = 0;
			geomHeader.version = 1;
			std::memcpy(geomHeader.fileSig, FileBinSig.data(), sizeof(geomHeader.fileSig));
			std::memset(geomHeader.reserved, 0, sizeof(geomHeader.reserved));

			if (streamNew.write(&geomHeader, sizeof(geomHeader)) != sizeof(geomHeader))
				return false;
		}

		//move data from only file to another

		std::vector<GeomChunkInfo> newGeomChunks;
		newGeomChunks.reserve(oldGeomChunks.size());

		for (const auto& oldGeomChunk : oldGeomChunks)
		{
			//create the new geom chunk
			GeomChunkInfo newGeomChunk;
			newGeomChunk.id = oldGeomChunk.id;
			newGeomChunk.type = static_cast<uint16_t>(GeomType::Static);
			newGeomChunk.flags = static_cast<uint32_t>(GeomFlags::None);
			newGeomChunk.numVertices = oldGeomChunk.numVertices;
			newGeomChunk.numIndices = oldGeomChunk.numIndices;
			newGeomChunk.size = oldGeomChunk.size + (sizeof(float) * 6); //must add the bbox
			newGeomChunk.geomsOffset = streamNew.position();
			newGeomChunk.animSetId = 0;
			newGeomChunks.push_back(std::move(newGeomChunk));

			streamOld.seek(hr::streams::Stream::SeekOrigin::Begin, oldGeomChunk.geomsOffset + sizeof(GeomHeaderOld));
			streamNew.write(streamOld, oldGeomChunk.size);
			streamNew.write(oldGeomChunk.bboxMin, sizeof(float) * 3);
			streamNew.write(oldGeomChunk.bboxMax, sizeof(float) * 3);
		}

		//write the new geom chunks
		for (const auto& geomChunk : newGeomChunks)
		{
			if (streamNew.write(&geomChunk, sizeof(GeomChunkInfo)) != sizeof(GeomChunkInfo))
				return false;
		}

		return true;
	}

	bool World::geomFileCreate(hr::streams::FileStream& fstream)
	{
		GeomHeader geomHeader;
		geomHeader.numGeoms = 0;
		geomHeader.numAnims = 0;
		geomHeader.numAnimSets = 0;
		geomHeader.version = 1;
		std::memcpy(geomHeader.fileSig, FileBinSig.data(), sizeof(geomHeader.fileSig));
		std::memset(geomHeader.reserved, 0, sizeof(geomHeader.reserved));

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
		newGeomChunk.type = static_cast<uint16_t>(GeomType::Static);
		newGeomChunk.flags = static_cast<uint32_t>(GeomFlags::None);
		newGeomChunk.numVertices = mesh.numVertices();
		newGeomChunk.numIndices = mesh.numIndices();
		newGeomChunk.size = 0;
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

			newGeomChunk.size = fwriter.position() - newGeomChunk.geomsOffset;

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
		newGeomChunk.type = static_cast<uint16_t>(GeomType::Animated);
		newGeomChunk.flags = static_cast<uint32_t>((meshAnim.skinningType() == geom::MeshAnim::SkinningType::Vertex8Joints) ? GeomFlags::AnimExtraBoneSet : GeomFlags::None);
		newGeomChunk.numVertices = meshAnim.mesh().numVertices();
		newGeomChunk.numIndices = meshAnim.mesh().numIndices();
		newGeomChunk.size = 0;
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

			newGeomChunk.size = fwriter.position() - newGeomChunk.geomsOffset;

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
		newAnimSetChunk.numJoints = static_cast<uint16_t>(animSet.numJoints());
		newAnimSetChunk.size = 0;
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

			newAnimSetChunk.size = fwriter.position() - newAnimSetChunk.geomsOffset;

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
		newAnimChunk.frameRate = frameRate;
		newAnimChunk.numFrames = animation.size();
		newAnimChunk.size = 0;
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

			newAnimChunk.size = fwriter.position() - newAnimChunk.geomsOffset;

			return true;

		}, nullptr, nullptr, [&newAnimChunk](hr::streams::StreamWriter& fwriter)
		{
			if (fwriter.write(&newAnimChunk, sizeof(AnimChunkInfo)) != sizeof(AnimChunkInfo))
				return false;

			return true;

		});

		return success;
	}

	bool World::geomFileTransformMeshes(hr::streams::FileStream& fstream, const std::vector<size_t>& geomIds, const std::function<void(hr::geom::Mesh&)>& cb)
	{
		if (!cb)
			return false;

		hr::streams::StreamReader streamBin(fstream);

		GeomHeader geomHeader;
		{
			if (streamBin.read(&geomHeader, sizeof(GeomHeader)) != sizeof(GeomHeader))
				return false;
			if (std::memcmp(geomHeader.fileSig, FileBinSig.data(), sizeof(geomHeader.fileSig)) != 0)
				return false;

			streamBin.seek(hr::streams::Stream::SeekOrigin::Begin, 0);
		}

		if (geomHeader.numGeoms <= 0)
			return false;

		int geomChunksSize = sizeof(GeomChunkInfo) * geomHeader.numGeoms;
		int animChunksSize = sizeof(AnimChunkInfo) * geomHeader.numAnims;
		int animSetchunksSize = sizeof(AnimSetChunkInfo) * geomHeader.numAnimSets;

		for (size_t curGeom = 0; curGeom < geomHeader.numGeoms; ++curGeom)
		{
			GeomChunkInfo chunkInfo;
			streamBin.seek(hr::streams::Stream::SeekOrigin::End, -(geomChunksSize + animSetchunksSize + animChunksSize));
			streamBin.seek(hr::streams::Stream::SeekOrigin::Current, sizeof(GeomChunkInfo) * curGeom);
			streamBin.read(&chunkInfo, sizeof(GeomChunkInfo));

			if (!geomIds.empty() && (std::find(geomIds.begin(), geomIds.end(), static_cast<size_t>(chunkInfo.id)) == geomIds.end()))
				continue;

			auto numVertices = static_cast<size_t>(chunkInfo.numVertices);
			auto numIndices = static_cast<size_t>(chunkInfo.numIndices);
			hr::geom::Mesh mesh(numVertices, numIndices);

			streamBin.seek(hr::streams::Stream::SeekOrigin::Begin, chunkInfo.geomsOffset);
			streamBin.read(mesh.vertices(), mesh.sizeVertices());
			streamBin.read(mesh.indices(), mesh.sizeIndices());

			cb(mesh);

			if ((mesh.numVertices() != numVertices) || (mesh.numIndices() != numIndices))
				continue;

			{
				hr::streams::StreamWriter streamWriter(fstream);

				streamWriter.seek(hr::streams::Stream::SeekOrigin::Begin, chunkInfo.geomsOffset);
				streamWriter.write(mesh.vertices(), mesh.sizeVertices());
				streamWriter.write(mesh.indices(), mesh.sizeIndices());

				auto bbox = mesh.getBoundingBox();

				if (streamWriter.write(bbox.min().data(), sizeof(float) * 3) != sizeof(float) * 3)
					return false;
				if (streamWriter.write(bbox.max().data(), sizeof(float) * 3) != sizeof(float) * 3)
					return false;
			}
		}

		return true;
	}

	bool World::geomFileRetrieveOffsets(hr::streams::FileStream& fstream, size_t geomId, size_t& vertexOffset, size_t& indexOffset)
	{
		vertexOffset = indexOffset = 0;

		hr::streams::StreamReader streamBin(fstream);

		GeomHeader geomHeader;
		{
			if (streamBin.read(&geomHeader, sizeof(GeomHeader)) != sizeof(GeomHeader))
				return false;
			if (std::memcmp(geomHeader.fileSig, FileBinSig.data(), sizeof(geomHeader.fileSig)) != 0)
				return false;

			streamBin.seek(hr::streams::Stream::SeekOrigin::Begin, 0);
		}

		if (geomHeader.numGeoms <= 0)
			return false;

		int geomChunksSize = sizeof(GeomChunkInfo) * geomHeader.numGeoms;
		int animChunksSize = sizeof(AnimChunkInfo) * geomHeader.numAnims;
		int animSetchunksSize = sizeof(AnimSetChunkInfo) * geomHeader.numAnimSets;

		streamBin.seek(hr::streams::Stream::SeekOrigin::End, -(geomChunksSize + animSetchunksSize + animChunksSize));

		for (size_t curGeom = 0; curGeom < geomHeader.numGeoms; ++curGeom)
		{
			GeomChunkInfo chunkInfo;
			streamBin.read(&chunkInfo, sizeof(GeomChunkInfo));

			if (chunkInfo.id == geomId)
			{
				vertexOffset = static_cast<size_t>(chunkInfo.geomsOffset);
				indexOffset = geom::Mesh::sizeVertices(chunkInfo.numVertices);
				return true;
			}
		}

		return false;
	}

	bool World::geomFileRemoveGeom(hr::streams::FileStream& fstreamOld, hr::streams::FileStream& fstreamNew, std::vector<size_t> geomIds)
	{
		return fileRemoveData(fstreamOld, fstreamNew, std::move(geomIds), {}, {});
	}

	bool World::loadAnimationSets(hr::streams::FileStream& fstream, size_t animSetId, geom::MeshAnimSet& meshAnimSet)
	{
		hr::streams::StreamReader stream(fstream);
		stream.seek(hr::streams::Stream::SeekOrigin::Begin, 0);

		AnimSetChunkInfo animSetInfo;
		std::memset(&animSetInfo, 0, sizeof(AnimSetChunkInfo));

		//read infos
		{
			GeomHeader geomHeader;
			if (stream.read(&geomHeader, sizeof(GeomHeader)) != sizeof(GeomHeader))
				return false;

			if (geomHeader.numAnimSets <= 0)
				return false;

			int animChunksSize = sizeof(AnimChunkInfo) * geomHeader.numAnims;
			int animSetchunksSize = sizeof(AnimSetChunkInfo) * geomHeader.numAnimSets;

			stream.seek(hr::streams::Stream::SeekOrigin::End, -(animSetchunksSize + animChunksSize));

			bool animSetFound = false;
			for (size_t curAnimSet = 0; curAnimSet < geomHeader.numAnimSets; ++curAnimSet)
			{
				stream.read(&animSetInfo, sizeof(AnimSetChunkInfo));

				animSetFound = (animSetInfo.id == animSetId);
				if (animSetFound)
					break;
			}

			if (!animSetFound)
				return false;
		}

		std::vector<Matrix> bindPoseMats;
		bindPoseMats.resize(animSetInfo.numJoints);

		stream.seek(hr::streams::Stream::SeekOrigin::Begin, animSetInfo.geomsOffset);

		for (size_t curJointIndex = 0; curJointIndex < animSetInfo.numJoints; curJointIndex++)
			stream.read(bindPoseMats[curJointIndex].data(), sizeof(float) * 16);

		meshAnimSet = geom::MeshAnimSet(std::move(bindPoseMats));
		return true;
	}

	bool World::loadAnimationSetMeshes(hr::streams::FileStream& fstream, size_t animSetId, std::vector<hr::geom::MeshAnim>& meshes)
	{
		hr::streams::StreamReader stream(fstream);
		stream.seek(hr::streams::Stream::SeekOrigin::Begin, 0);

		//read infos
		std::vector<GeomChunkInfo> geomChunkInfos;
		{
			GeomHeader geomHeader;
			if (stream.read(&geomHeader, sizeof(GeomHeader)) != sizeof(GeomHeader))
				return false;

			if (geomHeader.numGeoms <= 0)
				return false;

			int geomChunksSize = sizeof(GeomChunkInfo) * geomHeader.numGeoms;
			int animChunksSize = sizeof(AnimChunkInfo) * geomHeader.numAnims;
			int animSetchunksSize = sizeof(AnimSetChunkInfo) * geomHeader.numAnimSets;

			stream.seek(hr::streams::Stream::SeekOrigin::End, -(geomChunksSize + animSetchunksSize + animChunksSize));

			for (size_t curGeom = 0; curGeom < geomHeader.numGeoms; ++curGeom)
			{
				GeomChunkInfo geomChunkInfo;
				stream.read(&geomChunkInfo, sizeof(GeomChunkInfo));

				if (geomChunkInfo.animSetId != animSetId)
					continue;
				if (geomChunkInfo.type != static_cast<uint16_t>(GeomType::Animated))
					continue;

				geomChunkInfos.push_back(std::move(geomChunkInfo));
			}
		}

		for (const auto& geomChunk : geomChunkInfos)
		{
			stream.seek(hr::streams::Stream::SeekOrigin::Begin, geomChunk.geomsOffset);

			hr::geom::MeshAnim meshAnim;
			{
				hr::geom::Mesh mesh(geomChunk.numVertices, geomChunk.numIndices); //bind pose
				stream.read(mesh.vertices(), mesh.sizeVertices());
				stream.read(mesh.indices(), mesh.sizeIndices());

				stream.skip(sizeof(float) * 6); //bbox

				if ((geomChunk.flags & static_cast<uint16_t>(GeomFlags::AnimExtraBoneSet)) != 0)
					meshAnim = hr::geom::MeshAnim(std::move(mesh), hr::geom::MeshAnim::SkinningType::Vertex8Joints);
				else
					meshAnim = hr::geom::MeshAnim(std::move(mesh), hr::geom::MeshAnim::SkinningType::Vertex4Joints);

				stream.read(meshAnim.verticesJoints(), meshAnim.sizeVerticesJoints());
			}

			meshes.push_back(std::move(meshAnim));
		}

		return true;
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

	World::AreaId World::loadArea(IRenderer& renderer, const std::string& scenePath, const std::string& binPath)
	{
		auto id = mGenAreaIds++;

		auto& area = mAreas[id];
		area.id = id;

		auto success = load(renderer, area, scenePath, binPath);
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
		//camFrustum.setZFar(hrViewport.zfar());
		camFrustum.calculateFrustum(hrViewport.getProjection(hr::gl::tools::Viewport::ProjectionType::Proj3D), hrCamera.modelView());

		//run animations and update vertex data
		{
			class RendererObjectProxy
				: public hr::render::IRenderObject
			{
				BBox mBbox;
				size_t mObjectId = 0;
				hr::geom::Mesh& mMesh;

			public:
				RendererObjectProxy(size_t objectId, BBox bbox, hr::geom::Mesh& mesh)
					: mBbox{ std::move(bbox) }
					, mObjectId{ objectId }
					, mMesh{ mesh }
				{ }

			private:
				size_t id() const override
				{
					return mObjectId;
				}

				BBox bbox() const override
				{
					return mBbox;
				}

				size_t numVertices() const override
				{
					return mMesh.numVertices();
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
					assert(requestedDataSize == mMesh.sizeVertices());
					std::memcpy(destBuffer, mMesh.vertices(), requestedDataSize);

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

					auto bbox = animSet->second.animationSet.updatedBBox();

					if (!camFrustum.testBox(bbox)) //no point in updating the object if its entire anim set is not visible
						continue;

					animSet->second.animationSet.updateMesh(object.anim.meshAnim, object.anim.meshAnimated);
					
					targetObjects.push_back(RendererObjectProxy(objectId, std::move(bbox), object.anim.meshAnimated));
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

						if (!camFrustum.testBox(animSet->second.animationSet.updatedBBox())) //no point in updating the object if its entire anim set is not visible
							continue;
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

	bool World::load(IRenderer& renderer, Area& area, const std::string& scenePath, const std::string& binPath)
	{
		hr::streams::FileStream sceneFileStream(scenePath, true, false);
		hr::streams::StreamReader streamScene(sceneFileStream);

		hr::streams::FileStream binFileStream(binPath, true, false);
		hr::streams::StreamReader streamBin(binFileStream);

		//check file
		GeomHeader geomHeader;
		{
			if (streamBin.read(&geomHeader, sizeof(GeomHeader)) != sizeof(GeomHeader))
				return false;
			if (std::memcmp(geomHeader.fileSig, FileBinSig.data(), sizeof(geomHeader.fileSig)) != 0)
				return false;

			streamBin.seek(hr::streams::Stream::SeekOrigin::Begin, 0);
		}

		//load all animation sets and corresponding animations
		{
			loadAnimations(streamBin, area);
			streamBin.seek(hr::streams::Stream::SeekOrigin::Begin, 0);
		}

		std::map<size_t, RendererObjectProxy> mRendererObjects;

		//read objects and instances
		{
			std::unordered_map<size_t, GeomChunkInfo> geomInfo;

			//read geom infos
			{
				int geomChunksSize = sizeof(GeomChunkInfo) * geomHeader.numGeoms;
				int animChunksSize = sizeof(AnimChunkInfo) * geomHeader.numAnims;
				int animSetchunksSize = sizeof(AnimSetChunkInfo) * geomHeader.numAnimSets;

				if (geomHeader.numGeoms > 0)
				{
					streamBin.seek(hr::streams::Stream::SeekOrigin::End, -(geomChunksSize + animSetchunksSize + animChunksSize));

					geomInfo.rehash(geomHeader.numGeoms);
					for (size_t curGeom = 0; curGeom < geomHeader.numGeoms; ++curGeom)
					{
						GeomChunkInfo chunkInfo;
						streamBin.read(&chunkInfo, sizeof(GeomChunkInfo));

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
							auto& objectRenderer = mRendererObjects.try_emplace(objectId, binFileStream).first->second;

							object.id = objectId;
							object.type = Object::Type::Static;
							objectRenderer.objectId = objectId;

							//read geom info and bbox
							{
								auto geomIt = geomInfo.find(object.id);
								if (geomIt == geomInfo.end())
									continue;

								{
									streamBin.seek(hr::streams::Stream::SeekOrigin::Begin, geomIt->second.geomsOffset);

									object.anim.hasAnim = (geomIt->second.type == static_cast<uint16_t>(GeomType::Animated));
									if (object.anim.hasAnim)
									{
										object.anim.animSetId = geomIt->second.animSetId;

										object.anim.meshAnimated = hr::geom::Mesh(geomIt->second.numVertices, geomIt->second.numIndices); //bind pose
										streamBin.read(object.anim.meshAnimated.vertices(), object.anim.meshAnimated.sizeVertices());
										streamBin.read(object.anim.meshAnimated.indices(), object.anim.meshAnimated.sizeIndices());
									}
									else
									{
										streamBin.seek(hr::streams::Stream::SeekOrigin::Current, hr::geom::Mesh::sizeVertices(geomIt->second.numVertices));
										streamBin.seek(hr::streams::Stream::SeekOrigin::Current, hr::geom::Mesh::sizeIndices(geomIt->second.numIndices));
									}

									{
										Vector3f bboxMin, bboxMax;

										streamBin.read(bboxMin.data(), sizeof(float) * 3);
										streamBin.read(bboxMax.data(), sizeof(float) * 3);

										object.bbox.setMinMax(bboxMin, bboxMax);
									}

									if (object.anim.hasAnim)
									{
										if ((geomIt->second.flags & static_cast<uint16_t>(GeomFlags::AnimExtraBoneSet)) != 0)
											object.anim.meshAnim = hr::geom::MeshAnim(object.anim.meshAnimated, hr::geom::MeshAnim::SkinningType::Vertex8Joints);
										else
											object.anim.meshAnim = hr::geom::MeshAnim(object.anim.meshAnimated, hr::geom::MeshAnim::SkinningType::Vertex4Joints);

										streamBin.read(object.anim.meshAnim.verticesJoints(), object.anim.meshAnim.sizeVerticesJoints());
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