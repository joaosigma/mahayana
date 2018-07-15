#include "worldEditor.hpp"

#include "common/meshAnim.hpp"
#include "common/encoders.hpp"
#include "common/imageFactory.hpp"
#include "common/fileSystem.hpp"
#include "common/quaternion.hpp"
#include "common/stringUtils.hpp"

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

namespace hr::render
{
	namespace
	{
		class MD5Parser
		{
			std::string mFileData;
			std::string_view mPtr;

		public:
			MD5Parser(std::string_view filePath)
			{
				mFileData = hr::streams::FileStream::readEntireFileAsString(std::string(filePath));
				rewind();
			}

			const char* data() const
			{
				return mPtr.data();
			}

			MD5Parser& rewind()
			{
				mPtr = mFileData;
				return *this;
			}

			bool skipNumber()
			{
				auto pos = mPtr.find_first_not_of("+-0123456789.");
				if (pos == std::string_view::npos)
					return false;

				mPtr = mPtr.substr(pos);
				return true;
			}

			bool skipWhite(size_t skipChars = 0)
			{
				if (skipChars > 0)
					mPtr = mPtr.substr(skipChars);

				auto pos = mPtr.find_first_not_of(" \n\r\t");
				if (pos == std::string_view::npos)
					return false;

				mPtr = mPtr.substr(pos);
				return true;
			}

			bool moveToNext(char character)
			{
				auto pos = mPtr.find(character);
				if (pos == std::string_view::npos)
					return false;

				mPtr = mPtr.substr(pos);
				return true;
			}

			bool moveToNext(std::string_view str)
			{
				auto pos = mPtr.find(str);
				if (pos == std::string_view::npos)
					return false;

				mPtr = mPtr.substr(pos);
				return true;
			}

			bool moveToNextLine()
			{
				auto pos = mPtr.find_first_of("\r\n");
				if (pos == std::string_view::npos)
					return false;

				mPtr = mPtr.substr(pos);
				return true;
			}

			std::string readString()
			{
				auto pos = mPtr.find_first_of(" \n\r\t");
				if ((pos == std::string_view::npos) || (pos <= 0))
					return {};

				std::string str;
				str.resize(pos);
				std::memcpy(str.data(), mPtr.data(), pos);

				return str;
			}
		};
	}

	WorldEditor::WorldEditor()
	{ }

	WorldEditor::AreaId WorldEditor::newArea(std::string_view scenePath, std::string_view geomPath)
	{
		AreaId newId;
		{
			newId = mGenAreaIds++;

			auto& area = mAreas[newId];
			area.id = newId;
		}

		assert(mAreasData.find(newId) == mAreasData.end());
		auto& areaData = mAreasData[newId];

		areaData.pathScene = scenePath;
		areaData.pathGeom = geomPath;

		//create "empty" files
		{
			if (std::experimental::filesystem::exists(std::string(scenePath)))
				std::experimental::filesystem::remove(std::string(scenePath));
			if (std::experimental::filesystem::exists(std::string(geomPath)))
				std::experimental::filesystem::remove(std::string(geomPath));

			{
				hr::streams::FileStream streamScene(std::string(scenePath), false, true);

				rapidjson::StringBuffer s;
				rapidjson::PrettyWriter<rapidjson::StringBuffer> writer(s);

				writer.StartObject();

				writer.String("version");
				writer.StartArray();
				writer.Int(1);
				writer.Int(0);
				writer.Int(0);
				writer.EndArray();

				writer.String("objects");
				writer.StartArray();
				writer.EndArray();

				writer.String("instances");
				writer.StartArray();
				writer.EndArray();

				writer.EndObject();

				streamScene.write(s.GetString(), s.GetSize());
			}

			{
				hr::streams::FileStream streamGeom(std::string(geomPath), false, true);

				World::geomFileCreate(streamGeom);
			}
		}

		return newId;
	}

	bool WorldEditor::importMesh(AreaId areaId, std::string_view name, const hr::geom::Mesh& mesh)
	{
		if (!mesh.check() || (mAreas.find(areaId) == mAreas.end()))
			return false;

		auto& area = mAreas[areaId];
		auto& areaData = mAreasData[areaId];

		//create object
		auto objectId = genObjectId(area);
		{
			auto& object = area.mObjects[objectId];
			auto& objectData = areaData.objects[objectId];
			
			object.id = objectId;
			object.type = Object::Type::Static;
			objectData.objectId = objectId;
			objectData.name = name;
		}

		//we store new geometry immediately
		{
			hr::streams::FileStream geomFileStream(areaData.pathGeom, true, true);
			World::geomFileAddMesh(geomFileStream, objectId, mesh);
		}

		//create an instance associated with the object
		{
			Instance newInstance;
			newInstance.type = Instance::Type::Static;
			newInstance.objectId = objectId;
			newInstance.bbox = mesh.getBoundingBox();
			area.mInstances.push_back(newInstance);
		}

		//need to save everything to file (new geometry was already saved)
		saveArea(area);

		return true;
	}

	bool WorldEditor::importObj(AreaId areaId, std::string_view basePath, std::string_view fileName)
	{
		if (mAreas.find(areaId) == mAreas.end())
			return false;

		tinyobj::attrib_t objVertexAttribs;
		std::vector<tinyobj::shape_t> objShapes;
		std::vector<tinyobj::material_t> objMaterials;

		{
			std::string fullPath;
			fullPath.reserve(basePath.size() + fileName.size() + 1);
			fullPath.append(basePath).append(fileName);

			std::string errorDesc;
			auto success = tinyobj::LoadObj(&objVertexAttribs, &objShapes, &objMaterials, &errorDesc, fullPath.c_str(), std::string(basePath).c_str());
			if (!success)
				return false;
		}

		if (objShapes.empty())
			return true;

		auto& area = mAreas[areaId];
		auto& areaData = mAreasData[areaId];

		area.mInstances.reserve(area.mInstances.size() + objShapes.size());

		for (const auto& shape : objShapes)
		{
			//ignore empty shapes
			if (shape.mesh.indices.empty())
				continue;

			//all tris must belong to the same material
			if (!shape.mesh.material_ids.empty())
			{
				if (std::count(shape.mesh.material_ids.begin(), shape.mesh.material_ids.end(), shape.mesh.material_ids[0]) != shape.mesh.material_ids.size())
					continue;
			}

			//all indices must have position and tex coords
			if (std::any_of(shape.mesh.indices.begin(), shape.mesh.indices.end(), [](const tinyobj::index_t& index)
			{
				return ((index.vertex_index == -1) || (index.texcoord_index == -1));
			}))
				continue;

			//ignore shapes if a object with the same name already exists
			if (std::find_if(areaData.objects.begin(), areaData.objects.end(), [&name = shape.name](const auto& keyValue) { return (keyValue.second.name == name); }) != areaData.objects.end())
				continue;

			//create object
			auto objectId = genObjectId(area);
			auto& object = area.mObjects[objectId];
			auto& objectData = areaData.objects[objectId];

			object.id = objectId;
			object.type = Object::Type::Static;
			objectData.objectId = objectId;
			objectData.name = shape.name;

			//process mesh
			{
				bool ignoreNormals = std::any_of(shape.mesh.indices.begin(), shape.mesh.indices.end(), [](const tinyobj::index_t& index)
				{
					return (index.normal_index == -1);
				});

				hr::geom::Mesh newMesh;
				{
					struct hashableIndex
						: public tinyobj::index_t
					{
						struct hash
						{
							size_t operator()(const hashableIndex& index) const
							{
								size_t seed = index.vertex_index;
								seed ^= index.texcoord_index + 0x9e3779b9 + (seed << 6) + (seed >> 2);
								seed ^= index.normal_index + 0x9e3779b9 + (seed << 6) + (seed >> 2);
								return seed;
							}
						};

						hashableIndex(tinyobj::index_t index)
							: tinyobj::index_t(index)
						{ }

						bool operator ==(const hashableIndex& other) const
						{
							return (vertex_index == other.vertex_index) && (texcoord_index == other.texcoord_index) && (normal_index == other.normal_index);
						}
					};

					std::unordered_map<hashableIndex, size_t, hashableIndex::hash> mapping;

					for (const auto& index : shape.mesh.indices)
					{
						if (mapping.find(index) != mapping.end())
							continue;

						mapping[index] = mapping.size();
					}

					if (mapping.size() >= hr::geom::Mesh::maxVertexCount())
						continue;

					newMesh = hr::geom::Mesh(mapping.size(), shape.mesh.indices.size());

					for (const auto& keyValue : mapping)
					{
						auto vertexIndex = keyValue.second;

						newMesh.vertices()[vertexIndex].pos[0] = objVertexAttribs.vertices[keyValue.first.vertex_index * 3 + 0];
						newMesh.vertices()[vertexIndex].pos[1] = objVertexAttribs.vertices[keyValue.first.vertex_index * 3 + 1];
						newMesh.vertices()[vertexIndex].pos[2] = objVertexAttribs.vertices[keyValue.first.vertex_index * 3 + 2];

						newMesh.vertices()[vertexIndex].uv[0] = objVertexAttribs.texcoords[keyValue.first.texcoord_index * 2 + 0];
						newMesh.vertices()[vertexIndex].uv[1] = objVertexAttribs.texcoords[keyValue.first.texcoord_index * 2 + 1];

						if (!ignoreNormals)
						{
							newMesh.vertices()[vertexIndex].normal[0] = hr::geom::Mesh::pack(objVertexAttribs.normals[keyValue.first.normal_index * 3 + 0]);
							newMesh.vertices()[vertexIndex].normal[1] = hr::geom::Mesh::pack(objVertexAttribs.normals[keyValue.first.normal_index * 3 + 1]);
							newMesh.vertices()[vertexIndex].normal[2] = hr::geom::Mesh::pack(objVertexAttribs.normals[keyValue.first.normal_index * 3 + 2]);
						}
					}

					size_t curIndex = 0;
					for (const auto& index : shape.mesh.indices)
					{
						auto mappingIt = mapping.find(index);
						assert(mappingIt != mapping.end());

						newMesh.indices()[curIndex++] = static_cast<unsigned short>(mappingIt->second);
					}
				}

				assert(newMesh.check());
				if (ignoreNormals)
					newMesh.genNormals();
				newMesh.genTangents4();

				newMesh.optimizeIndices();

				//we store new geometry immediately
				{
					hr::streams::FileStream geomFileStream(areaData.pathGeom, true, true);
					World::geomFileAddMesh(geomFileStream, objectId, newMesh);
				}
			}

			//process material
			if (!shape.mesh.material_ids.empty())
			{
				auto materialId = shape.mesh.material_ids[0];
				if ((materialId >= 0) && (materialId < objMaterials.size()))
				{
					const auto& mat = objMaterials[materialId];

					if (!mat.diffuse_texname.empty())
						objectData.matDiffusePath = mat.diffuse_texname;
					else if (!mat.ambient_texname.empty())
						objectData.matDiffusePath = mat.ambient_texname;
					else
						materialId = materialId;

					if (!mat.normal_texname.empty())
						objectData.matNormalPath = mat.normal_texname;
					else if (!mat.bump_texname.empty())
						objectData.matNormalPath = mat.bump_texname;
					else if (mat.unknown_parameter.find("bump") != mat.unknown_parameter.end())
						objectData.matNormalPath = mat.unknown_parameter.find("bump")->second;
					else if (mat.unknown_parameter.find("map_bump") != mat.unknown_parameter.end())
						objectData.matNormalPath = mat.unknown_parameter.find("map_bump")->second;
					else
						materialId = materialId;
				}
			}

			//create an object associated with the concept
			Instance newInstance;
			newInstance.type = Instance::Type::Static;
			newInstance.objectId = object.id;
			newInstance.bbox = object.bbox;
			area.mInstances.push_back(newInstance);
		}

		//need to save everything to file (new geometry was already saved)
		saveArea(area);

		return true;
	}

	bool WorldEditor::importMD5(AreaId areaId, std::string_view md5Path, std::string newAnimSetName)
	{
		if (mAreas.find(areaId) == mAreas.end())
			return false;

		struct MD5Joint
		{
			int parent;
			Vector3f pos;
			Quaternion rot;
		};
		typedef std::vector<MD5Joint> MD5Skeleton;

		struct MD5Mesh
		{
			struct MD5Weight
			{
				Vector3f pos;
				int jointIndex;
				float bias;
			};

			struct MD5VertexData
			{
				int weightStart, weightCount;
			};

			std::string shader;
			std::vector<MD5Weight> weights;
			std::vector<MD5VertexData> vertexData;

			hr::geom::MeshAnim mesh;
		};

		struct MD5Model
		{
			MD5Skeleton bindPose;
			std::vector<MD5Mesh> meshes;
		};

		MD5Model md5Model;

		//load mesh
		{
			MD5Parser parser(md5Path);

			//read joints
			{
				if (!parser.moveToNext("numJoints"))
					return false;

				size_t numJoints;
				sscanf_s(parser.data(), "numJoints %d", &numJoints);

				if (!parser.moveToNext("joints") || !parser.moveToNext("{") || !parser.skipWhite(1))
					return false;

				md5Model.bindPose.reserve(numJoints);
				for (size_t i = 0; i < numJoints; i++)
				{
					{
						MD5Joint joint;
						float qx, qy, qz;

						char stringAux[128];
						sscanf(parser.data(), "%s %d ( %f %f %f ) ( %f %f %f )", stringAux, &joint.parent, &joint.pos[0], &joint.pos[1], &joint.pos[2], &qx, &qy, &qz);

						joint.rot.set(qx, qy, qz, 0.0f).expandWNormalized();
						md5Model.bindPose.push_back(std::move(joint));
					}

					parser.moveToNextLine();
					parser.skipWhite();
				}

				assert(md5Model.bindPose.size() == numJoints); //must have read them all
			}

			//read meshes
			{
				parser.rewind();
				if (!parser.moveToNext("numMeshes"))
					return false;

				size_t numMeshes;
				sscanf_s(parser.data(), "numMeshes %d", &numMeshes);

				md5Model.meshes.reserve(numMeshes);
				while (numMeshes > 0)
				{
					MD5Mesh md5Mesh;

					if (!parser.moveToNext("mesh") || !parser.moveToNext("{") || !parser.skipWhite(1))
						return false;
					if (!parser.moveToNext("shader") || !parser.moveToNext(' ') || !parser.skipWhite(1))
						return false;

					md5Mesh.shader = parser.readString();

					std::vector<std::tuple<float, float, int, int>> vertData;
					std::vector<std::tuple<int, int, int>> trisData;

					// read verts
					{
						if (!parser.moveToNext("numverts"))
							return false;

						size_t numVerts;
						sscanf_s(parser.data(), "numverts %d", &numVerts);
						parser.moveToNextLine();

						vertData.resize(numVerts);
						std::unordered_set<int> usedIndices;

						while (numVerts > 0)
						{
							if (!parser.moveToNext("vert"))
								return false;

							int index;
							std::tuple<float, float, int, int> data;
							sscanf(parser.data(), "vert %d ( %f %f ) %d %d", &index, &std::get<0>(data), &std::get<1>(data), &std::get<2>(data), &std::get<3>(data));
							parser.moveToNextLine();

							assert(usedIndices.find(index) == usedIndices.end()); //cannot repeat
							
							usedIndices.insert(index);
							vertData[index] = std::move(data);
							numVerts--;
						}

						assert(numVerts == 0); //must have read them all
						assert(usedIndices.size() == vertData.size());
					}

					// read tris
					{
						if (!parser.moveToNext("numtris"))
							return false;

						size_t numTris;
						sscanf_s(parser.data(), "numtris %d", &numTris);
						parser.moveToNextLine();

						trisData.resize(numTris);
						std::unordered_set<int> usedIndices;

						while (numTris > 0)
						{
							if (!parser.moveToNext("tri"))
								return false;

							int index;
							std::tuple<int, int, int> data;
							sscanf(parser.data(), "tri %d %d %d %d", &index, &std::get<0>(data), &std::get<1>(data), &std::get<2>(data));
							parser.moveToNextLine();

							assert(usedIndices.find(index) == usedIndices.end()); //cannot repeat

							usedIndices.insert(index);
							trisData[index] = std::move(data);
							numTris--;
						}

						assert(numTris == 0); //must have read them all
						assert(usedIndices.size() == trisData.size());
					}

					// read weights
					{
						if (!parser.moveToNext("numweights"))
							return false;

						size_t numWeights;
						sscanf_s(parser.data(), "numweights %d", &numWeights);
						parser.moveToNextLine();

						md5Mesh.weights.resize(numWeights);
						std::unordered_set<int> usedWeights;

						while (numWeights > 0)
						{
							if (!parser.moveToNext("weight"))
								return false;

							int index;
							MD5Mesh::MD5Weight weight;
							sscanf(parser.data(), "weight %d %d %f ( %f %f %f )", &index, &weight.jointIndex, &weight.bias, &weight.pos[0], &weight.pos[1], &weight.pos[2]);
							parser.moveToNextLine();

							assert(usedWeights.find(index) == usedWeights.end()); //cannot repeat

							usedWeights.insert(index);
							md5Mesh.weights[index] = std::move(weight);
							numWeights--;
						}

						assert(numWeights == 0); //must have read them all
						assert(usedWeights.size() == md5Mesh.weights.size());
					}

					//got everything, prepare some stuff

					{
						auto mesh = hr::geom::Mesh(vertData.size(), trisData.size() * 3);
						int maxWeightCount = 0;

						auto vertexWalker = mesh.vertices();
						for (auto&& v : vertData)
						{
							vertexWalker->uv[0] = std::get<0>(v);
							vertexWalker->uv[1] = std::get<1>(v);
							vertexWalker++;

							MD5Mesh::MD5VertexData vData;
							vData.weightStart = std::get<2>(v);
							vData.weightCount = std::get<3>(v);
							md5Mesh.vertexData.push_back(std::move(vData));

							maxWeightCount = (vData.weightCount > maxWeightCount) ? vData.weightCount : maxWeightCount;
						}

						auto indexWalker = mesh.indices();
						for (auto&& tri : trisData)
						{
							indexWalker[0] = std::get<0>(tri);
							indexWalker[1] = std::get<1>(tri);
							indexWalker[2] = std::get<2>(tri);
							indexWalker+=3;
						}

						mesh.check();

						assert(maxWeightCount <= 8);
						if (maxWeightCount <= 4)
							md5Mesh.mesh = geom::MeshAnim(std::move(mesh), geom::MeshAnim::SkinningType::Vertex4Joints);
						else if (maxWeightCount <= 8)
							md5Mesh.mesh = geom::MeshAnim(std::move(mesh), geom::MeshAnim::SkinningType::Vertex8Joints);
					}

					//done with this mesh

					md5Model.meshes.push_back(std::move(md5Mesh));

					numMeshes--;
				}
			}
		}

		//convert stuff (MD5 space corresponds to our X, -Z, Y)
		{
			for (auto& joint : md5Model.bindPose)
			{
				joint.pos = Vector3f(joint.pos[0], joint.pos[2], -joint.pos[1]);
				joint.rot = Quaternion(joint.rot[0], joint.rot[2], -joint.rot[1], joint.rot[3]);
			}

			for (auto& md5Mesh : md5Model.meshes)
			{
				auto& mesh = md5Mesh.mesh.mesh();

				mesh.invertTriWinding();

				for (auto& weight : md5Mesh.weights)
					weight.pos = Vector3f(weight.pos[0], weight.pos[2], -weight.pos[1]);
			}
		}

		//we can now calculate the bind pose of all our meshes (our anim meshes must be saved in bind pose)
		for (auto&& md5Mesh : md5Model.meshes)
		{
			size_t curIndex = 0;
			for (auto& vertData : md5Mesh.vertexData)
			{
				Vector3f finalPos(0.0f, 0.0f, 0.0f);

				float accumWeights = 0.0f;
				for (int i = 0; i < vertData.weightCount; i++)
				{
					auto& weight = md5Mesh.weights[vertData.weightStart + i];
					auto& joint = md5Model.bindPose[weight.jointIndex];

					auto rotPos = joint.rot.unitRotate(weight.pos);
					finalPos += (joint.pos + rotPos) * weight.bias;

					auto& vertexJoint = md5Mesh.mesh.vertexJoint(curIndex, i); //also, we store the bone indices / weights
					vertexJoint.jointIndex = weight.jointIndex;
					vertexJoint.jointWeight = geom::Mesh::pack(weight.bias);

					accumWeights += weight.bias;
				}

				md5Mesh.mesh.mesh().vertices()[curIndex].pos[0] = finalPos[0];
				md5Mesh.mesh.mesh().vertices()[curIndex].pos[1] = finalPos[1];
				md5Mesh.mesh.mesh().vertices()[curIndex].pos[2] = finalPos[2];
				curIndex++;
			}
		}

		//a MD5 model corresponds to a mesh anim set (joints + several meshes)
		geom::MeshAnimSet meshAnimSet;
		{
			std::vector<Matrix> invMatrix;
			invMatrix.reserve(md5Model.bindPose.size());

			//the basis for a MeshAnimSet are the bind pose inverse matrices of all the joints

			for (const auto& joint : md5Model.bindPose)
			{
				//we want to store the inverse of the matrix: P (joint.pos) * R (joint.rot)
				//which is equivalent to: R transpose * -P

				Matrix mat(joint.rot);
				mat.transpose();
				mat.mulTranslation(-joint.pos[0], -joint.pos[1], -joint.pos[2]);

				invMatrix.push_back(std::move(mat));
			}

			meshAnimSet = geom::MeshAnimSet(std::move(newAnimSetName), std::move(invMatrix)); //now we can create a mesh anim set
		}

		auto& area = mAreas[areaId];
		auto& areaData = mAreasData[areaId];

		//create the animation set in memory and in the geom file
		auto animSetId = genAnimSetId(area);
		{
			auto& animSet = area.mAnimationSets[animSetId];
			auto& animSetData = areaData.animationSets[animSetId];

			animSet.id = animSetId;
			animSetData.animSetId = animSetId;
			animSetData.numJoints = meshAnimSet.numJoints();
			animSetData.name = meshAnimSet.name();

			{
				hr::streams::FileStream geomFileStream(areaData.pathGeom, true, true);
				World::geomFileAddAnimationSet(geomFileStream, animSetId, meshAnimSet);
			}
		}

		//create each mesh in memory and in the geom file
		for (auto&& md5Mesh : md5Model.meshes)
		{
			//create object
			auto objectId = genObjectId(area);
			auto& object = area.mObjects[objectId];
			auto& objectData = areaData.objects[objectId];

			object.id = objectId;
			object.type = Object::Type::Static;
			objectData.objectId = objectId;
			objectData.name = StringUtils::eraseCopy(md5Mesh.shader, '"');
			objectData.matDiffusePath = "texs/hellknight/hellknight_b_albedo.png";
			objectData.matNormalPath = "texs/hellknight/hellknight_normal.png";

			assert(md5Mesh.mesh.mesh().check());
			md5Mesh.mesh.mesh().genNormals();
			md5Mesh.mesh.mesh().genTangents4();
			md5Mesh.mesh.mesh().optimizeIndices();

			//we store new geometry immediately
			{
				hr::streams::FileStream geomFileStream(areaData.pathGeom, true, true);
				World::geomFileAddMeshAnim(geomFileStream, objectId, md5Mesh.mesh, animSetId);
			}

			//create an object associated with the concept
			Instance newInstance;
			newInstance.type = Instance::Type::Static;
			newInstance.objectId = object.id;
			newInstance.bbox = object.bbox;
			area.mInstances.push_back(newInstance);
		}

		//need to save everything to file (new geometry was already saved)
		saveArea(area);

		return true;
	}

	bool WorldEditor::importMD5Anim(AreaId areaId, std::string_view animSetParentName, std::string_view md5AnimPath, std::string newAnimName)
	{
		if (mAreas.find(areaId) == mAreas.end())
			return false;

		auto& area = mAreas[areaId];
		auto& areaData = mAreasData[areaId];

		auto animSetId = findAnimSetId(areaData, animSetParentName);
		if (animSetId == 0)
			return false;

		struct MD5Joint
		{
			int parent;
			Vector3f pos;
			Quaternion rot;
		};
		typedef std::vector<MD5Joint> MD5Skeleton;

		struct MD5Anim
		{
			float frameRate;
			std::vector<MD5Skeleton> frameSkeletons;
		};
		MD5Anim md5Anim;

		//load anim

		{
			MD5Parser parser(md5AnimPath);

			size_t numFrames, numJoints, frameRate, numAnimatedComponents;
			{
				if (parser.moveToNext("numFrames"))
					sscanf_s(parser.data(), "numFrames %d", &numFrames);

				parser.rewind();
				if (parser.moveToNext("numJoints"))
					sscanf_s(parser.data(), "numJoints %d", &numJoints);

				parser.rewind();
				if (parser.moveToNext("frameRate"))
					sscanf_s(parser.data(), "frameRate %d", &frameRate);

				parser.rewind();
				if (parser.moveToNext("numAnimatedComponents"))
					sscanf_s(parser.data(), "numAnimatedComponents %d", &numAnimatedComponents);

				if ((numFrames <= 0) || (numJoints <= 0) || (frameRate <= 0) || (numAnimatedComponents <= 0))
					return false;
			}

			struct AnimBaseframe {
				Vector3f pos;
				Quaternion rot;
			};
			struct AnimFrame {
				float *animComponents;
				int numAnimatedComponentes;
			};
			struct AnimJoint {
				int parent, flag, startIndex;
			};

			std::vector<AnimJoint> animJoints;
			std::vector<AnimFrame> animFrames;
			std::vector<AnimBaseframe> animBaseFrame;

			animJoints.reserve(numJoints);
			animBaseFrame.reserve(numJoints);

			animFrames.resize(numFrames);
			for (auto&& curFrame : animFrames)
			{
				curFrame.numAnimatedComponentes = numAnimatedComponents;
				curFrame.animComponents = new float[curFrame.numAnimatedComponentes];
			}

			//read hierarchy
			{
				if (!parser.moveToNext("hierarchy") || !parser.moveToNext("{") || !parser.skipWhite(1))
					return false;

				for (size_t i = 0; i < numJoints; i++)
				{
					AnimJoint joint;

					char stringAux[128];
					sscanf(parser.data(), "%s %d %d %d", stringAux, &joint.parent, &joint.flag, &joint.startIndex);
					animJoints.push_back(std::move(joint));

					parser.moveToNextLine();
					parser.skipWhite();
				}

				assert(animJoints.size() == numJoints);
			}

			//read baseframe
			{
				if (!parser.moveToNext("baseframe") || !parser.moveToNext("{") || !parser.skipWhite(1))
					return false;

				for (size_t i = 0; i < numJoints; i++)
				{
					float px, py, pz, qx, qy, qz;

					sscanf(parser.data(), "( %f %f %f ) ( %f %f %f )", &px, &py, &pz, &qx, &qy, &qz);

					AnimBaseframe baseFrame;
					baseFrame.pos.set(px, py, pz);
					baseFrame.rot.set(qx, qy, qz, 0.0f);
					animBaseFrame.push_back(std::move(baseFrame));

					parser.moveToNextLine();
					parser.skipWhite();
				}

				assert(animBaseFrame.size() == numJoints);
			}

			//read frames
			for (size_t curFrame = 0; curFrame < numFrames; ++curFrame)
			{
				if (!parser.moveToNext("frame"))
					return false;

				size_t frameIndex;
				sscanf(parser.data(), "frame %d", &frameIndex);
				if (frameIndex >= animFrames.size())
					return false;

				if (!parser.moveToNext("{") || !parser.skipWhite(1))
					return false;

				auto componentWalker = animFrames[frameIndex].animComponents;
				for (size_t i = 0; i < numAnimatedComponents; i++)
				{
					sscanf(parser.data(), "%f", componentWalker);
					componentWalker++;

					parser.skipNumber();
					parser.skipWhite();
				}
			}

			//generate all skeletons
			{
				md5Anim.frameRate = static_cast<float>(frameRate);

				for (const auto& frame : animFrames)
				{
					MD5Skeleton frameSkeleton;
					frameSkeleton.resize(numJoints);

					for (size_t i = 0; i < numJoints; i++)
					{
						const auto& animJoint = animJoints[i];

						auto& targetJoint = frameSkeleton[i];
						targetJoint.parent = animJoint.parent;
						targetJoint.pos = animBaseFrame[i].pos;
						targetJoint.rot = animBaseFrame[i].rot;

						size_t curComponent = animJoint.startIndex;
						if (animJoint.flag & (1 << 0)) //TX
							targetJoint.pos[0] = frame.animComponents[curComponent++];
						if (animJoint.flag & (1 << 1)) //TY
							targetJoint.pos[1] = frame.animComponents[curComponent++];
						if (animJoint.flag & (1 << 2)) //TZ
							targetJoint.pos[2] = frame.animComponents[curComponent++];
						if (animJoint.flag & (1 << 3)) //RX
							targetJoint.rot[0] = frame.animComponents[curComponent++];
						if (animJoint.flag & (1 << 4)) //RY
							targetJoint.rot[1] = frame.animComponents[curComponent++];
						if (animJoint.flag & (1 << 5)) //RZ
							targetJoint.rot[2] = frame.animComponents[curComponent++];

						targetJoint.rot.expandWNormalized();
					}

					for (size_t i = 0; i < numJoints; i++)
					{
						const auto& animJoint = animJoints[i];
						auto& targetJoint = frameSkeleton[i];

						if (targetJoint.parent < 0)
							continue;

						auto& parentJoint = frameSkeleton[targetJoint.parent];

						targetJoint.pos = parentJoint.rot.unitRotate(targetJoint.pos) + parentJoint.pos;

						auto newRot = parentJoint.rot;
						newRot *= targetJoint.rot;
						targetJoint.rot = newRot;
					}

					md5Anim.frameSkeletons.push_back(std::move(frameSkeleton));
				}
			}
		}

		//convert stuff (MD5 space corresponds to our X, -Z, Y)
		{
			for (auto& skeleton : md5Anim.frameSkeletons)
			{
				for (auto&& joint : skeleton)
				{
					joint.pos = Vector3f(joint.pos[0], joint.pos[2], -joint.pos[1]);
					joint.rot = Quaternion(joint.rot[0], joint.rot[2], -joint.rot[1], joint.rot[3]);
				}
			}
		}

		//convert to our type of frames
		std::vector<geom::MeshAnimSet::Frame> animFrames;
		{
			animFrames.reserve(md5Anim.frameSkeletons.size());

			for (const auto& frame : md5Anim.frameSkeletons)
			{
				std::vector<geom::MeshAnimSet::Joint> skeleton;
				skeleton.reserve(frame.size());

				for (const auto& joint : frame)
				{
					geom::MeshAnimSet::Joint newJoint;
					newJoint.pos.set(joint.pos[0], joint.pos[1], joint.pos[2]);
					newJoint.rot.set(joint.rot[0], joint.rot[1], joint.rot[2], joint.rot[3]);

					skeleton.push_back(std::move(newJoint));
				}

				geom::MeshAnimSet::Frame newFrame;
				newFrame.bbox.reset();
				newFrame.joints = std::move(skeleton);

				animFrames.push_back(std::move(newFrame));
			}

			/*for (auto&& md5Mesh : model.meshes)
				meshAnimSet.animUpdateBBoxes(animId, md5Mesh.mesh);*/
		}

		//create the animation in memory and in the geom file
		{
			auto& animSetData = areaData.animationSets[animSetId];

			for (auto&& frame : animFrames) //number of joints must match
			{
				if (frame.joints.size() != animSetData.numJoints)
					return false;
			}

			size_t newAnimId = 1; //generate anim id
			while (std::find_if(animSetData.anims.begin(), animSetData.anims.end(), [&newAnimId](const auto &anim) { return (anim.animId == newAnimId); }) != animSetData.anims.end())
				newAnimId++;

			animSetData.anims.push_back(AreaData::AnimSetData::Animation{ newAnimId, std::move(newAnimName) });

			hr::streams::FileStream geomFileStream(areaData.pathGeom, true, true);
			World::geomFileAddAnimation(geomFileStream, newAnimId, animSetId, md5Anim.frameRate, std::move(animFrames));
		}

		//need to save everything to file (new geometry was already saved)
		saveArea(area);

		return true;
	}

	void WorldEditor::processMesh(AreaId areaId, const std::vector<size_t>& objectIds, std::function<void(hr::geom::Mesh&)> cb)
	{
		if (!cb || (mAreas.find(areaId) == mAreas.end()))
			return;

		auto& area = mAreas[areaId];
		auto& areaData = mAreasData[areaId];

		hr::streams::FileStream geomFileStream(areaData.pathGeom, true, true);

		auto func = [&geomFileStream, cb](Area& area, AreaData::ObjectData& object)
		{
			hr::geom::Mesh mesh(object.geom.numVertices, object.geom.numIndices);

			geomFileStream.seek(hr::streams::Stream::SeekOrigin::Begin, object.geom.fstreamVertexOffset);
			geomFileStream.read(mesh.vertices(), mesh.sizeVertices());

			geomFileStream.seek(hr::streams::Stream::SeekOrigin::Begin, object.geom.fstreamIndexOffset);
			geomFileStream.read(mesh.indices(), mesh.sizeIndices());

			{
				auto numVertices = mesh.numVertices();
				auto numIndices = mesh.numIndices();
				cb(mesh);

				if ((mesh.numVertices() != numVertices) || (mesh.numIndices() != numIndices))
					return;
			}

			geomFileStream.seek(hr::streams::Stream::SeekOrigin::Begin, object.geom.fstreamVertexOffset);
			geomFileStream.write(mesh.vertices(), mesh.sizeVertices());

			geomFileStream.seek(hr::streams::Stream::SeekOrigin::Begin, object.geom.fstreamIndexOffset);
			geomFileStream.write(mesh.indices(), mesh.sizeIndices());
		};

		if (objectIds.empty())
		{
			for (auto& [objectId, object] : area.mObjects)
			{
				assert(areaData.objects.find(objectId) != areaData.objects.end());
				func(area, areaData.objects[objectId]);
			}
		}
		else
		{
			for (const auto& objectId : objectIds)
			{
				auto it = area.mObjects.find(objectId);
				if (it == area.mObjects.end())
					continue;

				assert(areaData.objects.find(objectId) != areaData.objects.end());
				func(area, areaData.objects[objectId]);
			}
		}
	}

	std::vector<size_t> WorldEditor::unusedObjects(AreaId areaId) const
	{
		auto areaIt = mAreas.find(areaId);
		if (areaIt == mAreas.end())
			return { };

		const auto& area = areaIt->second;

		std::vector<size_t> objectIds;
		for (const auto& [objectId, object] : area.mObjects)
		{
			if (std::find_if(area.mInstances.begin(), area.mInstances.end(), [objectId](const auto& instance) { return (instance.objectId == objectId); }) != area.mInstances.end())
				continue;

			objectIds.push_back(objectId);
		}

		return objectIds;
	}

	void WorldEditor::removeObjects(AreaId areaId, const std::vector<size_t>& objectIds)
	{
		if (mAreas.find(areaId) == mAreas.end())
			return;

		auto& area = mAreas[areaId];

		for (const auto& objectId : objectIds)
		{
			area.mObjects.erase(objectId);
			std::remove_if(area.mInstances.begin(), area.mInstances.end(), [objectId](const auto& instance) { return (instance.objectId == objectId); });
		}
	}

	size_t WorldEditor::genObjectId(Area& area) const
	{
		size_t curId = 1;
		while (area.mObjects.find(curId) != area.mObjects.end())
			curId++;

		return curId;
	}

	size_t WorldEditor::genAnimSetId(Area& area) const
	{
		size_t curId = 1;
		while (area.mAnimationSets.find(curId) != area.mAnimationSets.end())
			curId++;

		return curId;
	}

	size_t WorldEditor::findAnimSetId(AreaData& area, std::string_view name)
	{
		for (auto&&[animSetId, animSet] : area.animationSets)
		{
			if (animSet.name == name)
				return animSetId;
		}

		return 0;
	}

	void WorldEditor::saveArea(Area& area)
	{
		assert(mAreasData.find(area.id) != mAreasData.end());
		auto& areaData = mAreasData[area.id];

		//the area is always exported whole, which means that we can destroy the old version completly
		//also: we only need to save the scene (geom was already taken care of)

		if (std::experimental::filesystem::exists(areaData.pathScene))
			std::experimental::filesystem::remove(areaData.pathScene);

		hr::streams::FileStream streamScene(areaData.pathScene, false, true);

		rapidjson::StringBuffer s;
		rapidjson::PrettyWriter<rapidjson::StringBuffer> writer(s);

		writer.StartObject();

		writer.String("version");
		writer.StartArray();
		writer.Int(1);
		writer.Int(0);
		writer.Int(0);
		writer.EndArray();

		writer.String("animationSets");
		writer.StartArray();
		for (auto& animSet : areaData.animationSets)
		{
			writer.StartObject();

			writer.String("id");
			writer.Uint(animSet.second.animSetId);

			writer.String("name");
			writer.String(animSet.second.name.c_str());


			writer.String("animation");
			writer.StartArray();
			for (auto& anim : animSet.second.anims)
			{
				writer.StartObject();

				writer.String("id");
				writer.Uint(anim.animId);

				writer.String("name");
				writer.String(anim.name.c_str());

				writer.EndObject();
			}
			writer.EndArray();

			writer.EndObject();
		}
		writer.EndArray();

		writer.String("objects");
		writer.StartArray();
		for (auto& object : areaData.objects)
		{
			writer.StartObject();

			writer.String("id");
			writer.Uint(object.second.objectId);

			writer.String("type");
			writer.Uint(static_cast<unsigned int>(Object::Type::Static));

			writer.String("name");
			writer.String(object.second.name.c_str());

			writer.String("material");
			writer.StartObject();
			writer.String("diffusePath");
			writer.String(object.second.matDiffusePath.c_str());

			writer.String("normalPath");
			writer.String(object.second.matNormalPath.c_str());
			writer.EndObject();

			writer.EndObject();
		}
		writer.EndArray();

		writer.String("instances");
		writer.StartArray();
		for (auto& curObject : area.mInstances)
		{
			writer.StartObject();

			writer.String("type");
			writer.Uint(static_cast<unsigned int>(curObject.type));

			writer.String("objectId");
			writer.Uint(curObject.objectId);

			writer.EndObject();
		}
		writer.EndArray();

		writer.EndObject();

		streamScene.write(s.GetString(), s.GetSize());
	}
}