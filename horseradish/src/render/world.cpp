#include "world.hpp"

#include "common/encoders.hpp"
#include "common/imageFactory.hpp"

#include "libs/lz4/lz4.h"
#include "libs/lz4/lz4hc.h"
#include "libs/cppformat/format.h"
#include "libs/rapidjson/document.h"
#include "libs/rapidjson/rapidjson.h"
#include "libs/rapidjson/prettywriter.h"
#include "libs/rapidjson/stringbuffer.h"
#include "libs/tinyobjloader/tiny_obj_loader.h"

#include <algorithm>

namespace HorseRadish { namespace Render
{
	World::World()
	{ }

	World::~World()
	{
		cleanup();
	}

	void World::cleanup()
	{
		mConcepts.clear();
		mObjects.clear();

		mRenderData.objects.clear();
	}

	bool World::importJSON(HorseRadish::Streams::StreamReader &stream)
	{
		std::shared_ptr<unsigned char> buffer;
		size_t bufferSize;

		stream.stream().cloneAllContent(buffer, bufferSize, [](size_t requiredSize)
		{
			std::shared_ptr<unsigned char> buffer(new unsigned char[requiredSize + 1], std::default_delete<unsigned char[]>());
			buffer.get()[requiredSize] = '\0';
			return buffer;
		});

		rapidjson::Document d;
		d.ParseInsitu(reinterpret_cast<char*>(buffer.get()));

		{
			auto& jsonConcepts = d["concepts"];
			assert(jsonConcepts.IsArray());

			mConcepts.rehash(jsonConcepts.Size());
			for (rapidjson::Value::ConstValueIterator itr = jsonConcepts.Begin(); itr != jsonConcepts.End(); ++itr)
			{
				std::string conceptName = (*itr)["name"].GetString();

				if ((*itr).HasMember("mesh"))
				{
					auto& jsonMesh = ((*itr).FindMember("mesh"))->value;

					size_t numVertices = jsonMesh["numVertices"].GetUint();
					size_t numIndices = jsonMesh["numIndices"].GetUint();

					HorseRadish::Geometry::Mesh newMesh(numVertices, numIndices);

					{
						auto& jsonVertexData = jsonMesh["vertexData"];

						std::vector<unsigned char> dataBase64;
						HorseRadish::Encoders::decodeBase64(jsonVertexData.GetString(), jsonVertexData.GetStringLength(), dataBase64);

						if (dataBase64.size() == newMesh.sizeVertices())
						{
							memcpy(newMesh.vertices(), dataBase64.data(), newMesh.sizeVertices());
						}
						else
						{
							auto decompressSize = LZ4_decompress_safe(reinterpret_cast<const char*>(dataBase64.data()), reinterpret_cast<char*>(newMesh.vertices()), dataBase64.size(), newMesh.sizeVertices());
							assert(decompressSize == newMesh.sizeVertices());
						}
					}

					{
						auto& jsonIndexData = jsonMesh["indexData"];

						std::vector<unsigned char> dataBase64;
						HorseRadish::Encoders::decodeBase64(jsonIndexData.GetString(), jsonIndexData.GetStringLength(), dataBase64);

						if (dataBase64.size() == newMesh.sizeIndices())
						{
							memcpy(newMesh.indices(), dataBase64.data(), newMesh.sizeIndices());
						}
						else
						{
							auto decompressSize = LZ4_decompress_safe(reinterpret_cast<const char*>(dataBase64.data()), reinterpret_cast<char*>(newMesh.indices()), dataBase64.size(), newMesh.sizeIndices());
							assert(decompressSize == newMesh.sizeIndices());
						}
					}

					assert(newMesh.check());
					mConcepts[conceptName].mesh = std::move(newMesh);
				}

				if ((*itr).HasMember("material"))
				{
					auto& jsonMaterial = ((*itr).FindMember("material"))->value;

					mConcepts[conceptName].matDiffusePath = jsonMaterial["diffusePath"].GetString();
					mConcepts[conceptName].matNormalPath = jsonMaterial["normalPath"].GetString();
				}
			}
		}

		{
			auto& jsonObjects = d["objects"];
			assert(jsonObjects.IsArray());

			mObjects.reserve(jsonObjects.Size());
			for (rapidjson::Value::ConstValueIterator itr = jsonObjects.Begin(); itr != jsonObjects.End(); ++itr)
			{
				Object newObject;
				newObject.type = static_cast<Object::Type>((*itr)["type"].GetInt());
				newObject.conceptName = (*itr)["concept"].GetString();

				std::vector<unsigned char> dataBase64;
				HorseRadish::Encoders::decodeBase64((*itr)["bbox"].GetString(), dataBase64);

				assert(dataBase64.size() == (sizeof(float) * 6));
				newObject.bbox.setMinMax(reinterpret_cast<const float*>(dataBase64.data()), reinterpret_cast<const float*>(dataBase64.data()) + 3);

				mObjects.push_back(newObject);
			}
		}

		return true;
	}

	bool World::exportJSON(HorseRadish::Streams::StreamWriter &stream)
	{
		rapidjson::StringBuffer s;
		rapidjson::PrettyWriter<rapidjson::StringBuffer> writer(s);

		writer.StartObject();

		writer.String("version");
		writer.StartArray();
		writer.Int(1);
		writer.Int(0);
		writer.Int(0);
		writer.EndArray();

		writer.String("concepts");
		writer.StartArray();
		for (auto& concept : mConcepts)
		{
			writer.StartObject();

			writer.String("name");
			writer.String(concept.first.c_str());

			writer.String("mesh");
			writer.StartObject();

				writer.String("numVertices");
				writer.Int(concept.second.mesh.numVertices());

				writer.String("numIndices");
				writer.Int(concept.second.mesh.numIndices());

				writer.String("vertexData");
				{
					//TODO: seems that lz4 is corrupting the buffer

					//auto maxCompressedSize = LZ4_compressBound(curGeom.mesh.sizeVertices());
					//auto compressedBuffer = std::unique_ptr<char[]>(new char[maxCompressedSize]);
					//auto compressedSize = LZ4_compress_HC(reinterpret_cast<const char*>(curGeom.mesh.dataVertices()), compressedBuffer.get(), curGeom.mesh.sizeVertices(), maxCompressedSize, 16);
					//
					//if ((compressedSize > 0) && (compressedSize < curGeom.mesh.sizeVertices()))
					//	writer.String(HorseRadish::Encoders::EncodeBase64(compressedBuffer.get(), compressedSize).c_str());
					//else
						writer.String(HorseRadish::Encoders::encodeBase64(concept.second.mesh.vertices(), concept.second.mesh.sizeVertices()).c_str());
				}

				writer.String("indexData");
				{
					//TODO: seems that lz4 is corrupting the buffer

					//auto maxCompressedSize = LZ4_compressBound(curGeom.mesh.sizeIndices());
					//auto compressedBuffer = std::unique_ptr<char[]>(new char[maxCompressedSize]);
					//auto compressedSize = LZ4_compress_HC(reinterpret_cast<const char*>(curGeom.mesh.dataIndices()), compressedBuffer.get(), curGeom.mesh.sizeIndices(), maxCompressedSize, 16);
					//
					//if ((compressedSize > 0) && (compressedSize < curGeom.mesh.sizeIndices()))
					//	writer.String(HorseRadish::Encoders::EncodeBase64(compressedBuffer.get(), compressedSize).c_str());
					//else
						writer.String(HorseRadish::Encoders::encodeBase64(concept.second.mesh.indices(), concept.second.mesh.sizeIndices()).c_str());
				}

			writer.EndObject();

			writer.String("material");
			writer.StartObject();

				writer.String("diffusePath");
				writer.String(concept.second.matDiffusePath.c_str());

				writer.String("normalPath");
				writer.String(concept.second.matNormalPath.c_str());

			writer.EndObject();

			writer.EndObject();
		}
		writer.EndArray();

		writer.String("objects");
		writer.StartArray();
		for (auto& curObject : mObjects)
		{
			writer.StartObject();

			writer.String("type");
			writer.Int(static_cast<int>(curObject.type));

			writer.String("concept");
			writer.String(curObject.conceptName.c_str());

			float bbox[6];
			curObject.bbox.min(bbox);
			curObject.bbox.max(bbox + 3);

			writer.String("bbox");
			writer.String(HorseRadish::Encoders::encodeBase64(bbox, sizeof(float) * 6).c_str());

			writer.EndObject();
		}
		writer.EndArray();

		writer.EndObject();

		stream.write(s.GetString(), s.GetSize());

		return true;
	}

	bool World::importObj(const std::string& basePath, const std::string& fileName)
	{
		std::vector<tinyobj::shape_t> shapes;
		std::vector<tinyobj::material_t> materials;

		auto errorDesc = tinyobj::LoadObj(shapes, materials, (basePath + fileName).c_str(), basePath.c_str());
		if (!errorDesc.empty())
			return false;

		if (shapes.empty())
			return true;

		mObjects.reserve(mObjects.size() + shapes.size());
		for (const auto& shape : shapes)
		{
			if (shape.mesh.indices.empty())
				continue;

			if (mConcepts.find(shape.name) != mConcepts.end())
				continue;

			auto& concept = mConcepts[shape.name];
			concept.name = shape.name;

			HorseRadish::Geometry::Mesh newMesh(shape.mesh.positions.size() / 3, shape.mesh.indices.size());

			for (unsigned int i = 0; i < newMesh.numVertices(); i++)
			{
				newMesh.vertices()[i].pos[0] = shape.mesh.positions[i * 3 + 0];
				newMesh.vertices()[i].pos[1] = shape.mesh.positions[i * 3 + 1];
				newMesh.vertices()[i].pos[2] = shape.mesh.positions[i * 3 + 2];

				newMesh.vertices()[i].uv[0] = shape.mesh.texcoords[i * 2 + 0];
				newMesh.vertices()[i].uv[1] = shape.mesh.texcoords[i * 2 + 1];

				newMesh.vertices()[i].normal[0] = HorseRadish::Geometry::Mesh::pack(shape.mesh.positions[i * 3 + 0]);
				newMesh.vertices()[i].normal[1] = HorseRadish::Geometry::Mesh::pack(shape.mesh.positions[i * 3 + 1]);
				newMesh.vertices()[i].normal[2] = HorseRadish::Geometry::Mesh::pack(shape.mesh.positions[i * 3 + 2]);
			}

			for (unsigned int i = 0; i < newMesh.numIndices(); i++)
				newMesh.indices()[i] = shape.mesh.indices[i];
				
			assert(newMesh.check());
			newMesh.genTangents4();

			std::swap(concept.mesh, newMesh);

			Object newObject;
			newObject.type = Object::Type::Static;
			newObject.conceptName = shape.name;
			newObject.bbox = concept.mesh.getBoundingBox();
			mObjects.push_back(newObject);
		}

		return true;
	}

	void World::loadData(HorseRadish::IO::FileSystem& fileSystem)
	{
		mRenderData.objects.reserve(mObjects.size());
	}

	void World::prepareNextFrame(const Tools::Camera& hrCamera, const HorseRadish::OpenGL::Tools::Viewport& hrViewport)
	{
		auto camPos = hrCamera.getPos();

		HorseRadish::OpenGL::Tools::Frustum camFrustum;
		camFrustum.setCamPosition(camPos);
		camFrustum.setZNear(hrViewport.znear());
		camFrustum.setZFar(hrViewport.zfar());
		camFrustum.calculateFrustum(hrViewport.getProjection(HorseRadish::OpenGL::Tools::Viewport::ProjectionType::Proj3D), hrCamera.modelView());

		mRenderData.objects.clear();
		for (auto& curObject : mObjects)
		{
			if (!camFrustum.testBox(curObject.bbox))
				continue;

			mRenderData.objects.push_back(&curObject);
		}

		if (mRenderData.objects.empty())
			return;
	}
} }