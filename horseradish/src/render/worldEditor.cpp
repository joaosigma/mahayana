#include "worldEditor.hpp"

#include "common/meshAnim.hpp"
#include "common/encoders.hpp"
#include "common/imageFactory.hpp"
#include "common/fileSystem.hpp"
#include "common/quaternion.hpp"
#include "common/stringUtils.hpp"

#include "libs/rapidjson/document.h"
#include "libs/rapidjson/rapidjson.h"
#include "libs/rapidjson/prettywriter.h"
#include "libs/rapidjson/stringbuffer.h"

#include "libs/tinygltf/tiny_gltf.h"
#include "libs/tinyobjloader/tiny_obj_loader.h"

#include "glcorearb.h"

#include <array>
#include <cstdint>
#include <algorithm>
#include <filesystem>
#include <unordered_set>

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

	WorldEditor::AreaId WorldEditor::newArea(std::string_view scenePath, std::string_view binPath)
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
		areaData.pathBin = binPath;

		//create "empty" files
		{
			if (std::filesystem::exists(std::string(scenePath)))
				std::filesystem::remove(std::string(scenePath));
			if (std::filesystem::exists(std::string(binPath)))
				std::filesystem::remove(std::string(binPath));

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
				hr::streams::FileStream streamGeom(std::string(binPath), false, true);

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
			hr::streams::FileStream geomFileStream(areaData.pathBin, true, true);
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

		tinyobj::ObjReader objReader;
		{
			std::string fullPath;
			fullPath.reserve(basePath.size() + fileName.size() + 1);
			fullPath.append(basePath).append(fileName);

			tinyobj::ObjReaderConfig objConfig;
			objConfig.triangulate = true;
			objConfig.vertex_color = false;
			objConfig.mtl_search_path = ""; //use the same folder as the obj file

			objReader.ParseFromFile(fullPath.c_str(), objConfig);
		}

		if (!objReader.Valid())
			return false;

		if (objReader.GetShapes().empty())
			return true;

		auto& area = mAreas[areaId];
		auto& areaData = mAreasData[areaId];

		area.mInstances.reserve(area.mInstances.size() + objReader.GetShapes().size());

		for (const auto& shape : objReader.GetShapes())
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
						auto& objVertexAttribs = objReader.GetAttrib();

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
					hr::streams::FileStream geomFileStream(areaData.pathBin, true, true);
					World::geomFileAddMesh(geomFileStream, objectId, newMesh);
				}
			}

			//process material
			if (!shape.mesh.material_ids.empty())
			{
				auto& objMaterials = objReader.GetMaterials();

				auto materialId = shape.mesh.material_ids[0];
				if ((materialId >= 0) && (materialId < objMaterials.size()))
				{
					const auto& mat = objMaterials[materialId];

					if (!mat.diffuse_texname.empty())
						objectData.matDiffusePath = mat.diffuse_texname;
					else if (!mat.ambient_texname.empty())
						objectData.matDiffusePath = mat.ambient_texname;

					if (!mat.normal_texname.empty())
						objectData.matNormalPath = mat.normal_texname;
					else if (!mat.bump_texname.empty())
						objectData.matNormalPath = mat.bump_texname;
					else if (mat.unknown_parameter.find("bump") != mat.unknown_parameter.end())
						objectData.matNormalPath = mat.unknown_parameter.find("bump")->second;
					else if (mat.unknown_parameter.find("map_bump") != mat.unknown_parameter.end())
						objectData.matNormalPath = mat.unknown_parameter.find("map_bump")->second;
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
			char name[256];
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

						sscanf(parser.data(), "%s %d ( %f %f %f ) ( %f %f %f )", joint.name, &joint.parent, &joint.pos[0], &joint.pos[1], &joint.pos[2], &qx, &qy, &qz);

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
			animSetData.name = meshAnimSet.name();

			{
				hr::streams::FileStream geomFileStream(areaData.pathBin, true, true);
				World::geomFileAddAnimationSet(geomFileStream, animSetId, meshAnimSet);
			}

			assert(md5Model.bindPose.size() == meshAnimSet.numJoints());

			animSetData.joints.reserve(md5Model.bindPose.size());
			for (auto& joint : md5Model.bindPose)
			{
				AreaData::AnimSetData::JointData jointData;

				jointData.index = animSetData.joints.size();
				jointData.parentIndex = joint.parent;

				{
					std::string_view name(joint.name);

					while (!name.empty() && name[0] == '"')
						name = name.substr(1);
					while (!name.empty() && name[name.size() - 1] == '"')
						name = name.substr(0, name.size() - 1);

					std::fill(jointData.name.begin(), jointData.name.end(), 0);
					std::memcpy(jointData.name.data(), name.data(), std::min(name.size(), jointData.name.size() - 1));
				}

				animSetData.joints.push_back(std::move(jointData));
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
			
			{
				auto fullName = StringUtils::eraseCopy(md5Mesh.shader, '"');
				auto slashIndex = fullName.find_last_of('/');
				auto finalPart = (slashIndex == std::string::npos) ? fullName : fullName.substr(slashIndex + 1);

				objectData.name = fmt::format("{0}/{1}", meshAnimSet.name(), finalPart);
				objectData.matDiffusePath = fmt::format("texs/{0}/{1}.albedo.png", meshAnimSet.name(), finalPart);
				objectData.matNormalPath = fmt::format("texs/{0}/{1}.normal.png", meshAnimSet.name(), finalPart);
			}

			assert(md5Mesh.mesh.mesh().check());
			md5Mesh.mesh.mesh().genNormals();
			md5Mesh.mesh.mesh().genTangents4();
			md5Mesh.mesh.mesh().optimizeIndices();

			//we store new geometry immediately
			{
				hr::streams::FileStream geomFileStream(areaData.pathBin, true, true);
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

			size_t numFrames = 0, numJoints = 0, frameRate = 0, numAnimatedComponents = 0;
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
		}

		//we have everything, but we still have to calculate one final thing: the bboxes for all frames
		{
			geom::MeshAnimSet meshAnimSet;
			std::vector<hr::geom::MeshAnim> meshes;
			{
				hr::streams::FileStream geomFileStream(areaData.pathBin, true, false);

				if (!World::loadAnimationSets(geomFileStream, animSetId, meshAnimSet))
					return false;

				if (!World::loadAnimationSetMeshes(geomFileStream, animSetId, meshes))
					return false;
			}

			meshAnimSet.animAdd(0, md5Anim.frameRate, animFrames);
			for (auto&& mesh : meshes)
				meshAnimSet.animUpdateBBoxes(0, mesh);

			auto& updatedAnims = meshAnimSet.animFrames(0);
			for (size_t i = 0; i < updatedAnims.size(); i++)
				animFrames[i].bbox = updatedAnims[i].bbox;
		}

		//create the animation in memory and in the geom file
		{
			auto& animSetData = areaData.animationSets[animSetId];

			for (auto&& frame : animFrames) //number of joints must match
			{
				if (frame.joints.size() != animSetData.joints.size())
					return false;
			}

			size_t newAnimId = 1; //generate anim id
			while (std::find_if(animSetData.anims.begin(), animSetData.anims.end(), [&newAnimId](const auto &anim) { return (anim.animId == newAnimId); }) != animSetData.anims.end())
				newAnimId++;

			animSetData.anims.push_back(AreaData::AnimSetData::Animation{ newAnimId, std::move(newAnimName) });

			hr::streams::FileStream geomFileStream(areaData.pathBin, true, true);
			World::geomFileAddAnimation(geomFileStream, newAnimId, animSetId, md5Anim.frameRate, std::move(animFrames));
		}

		//need to save everything to file (new geometry was already saved)
		saveArea(area);

		return true;
	}

	bool WorldEditor::importGLTF(AreaId areaId, std::string_view gltfPath)
	{
		if (mAreas.find(areaId) == mAreas.end())
			return false;

		tinygltf::Model gltfModel;
		{
			std::string err, warn;
			tinygltf::TinyGLTF gltf_ctx;

			bool ret = false;
			if (std::filesystem::path(gltfPath.begin(), gltfPath.end()).extension().string() == "glb")
				ret = gltf_ctx.LoadBinaryFromFile(&gltfModel, &err, &warn, gltfPath.data());
			else
				ret = gltf_ctx.LoadASCIIFromFile(&gltfModel, &err, &warn, gltfPath.data());

			if (!ret)
				return false;
		}

		if (gltfModel.scenes.empty() || gltfModel.nodes.empty() || gltfModel.meshes.empty())
			return true;

		auto& area = mAreas[areaId];
		auto& areaData = mAreasData[areaId];

		//aux stuff
		struct Skin {
			std::vector<size_t> jointsIndices;
			std::vector<Matrix> inverseBindMats;
		};
		std::vector<Skin> skins;

		//read the skins
		{
			skins.reserve(gltfModel.skins.size());
			for (const auto& gltfSkin : gltfModel.skins)
			{
				Skin targetSkin;

				targetSkin.jointsIndices.reserve(gltfSkin.joints.size());
				targetSkin.inverseBindMats.reserve(gltfSkin.joints.size());

				for (const auto& index : gltfSkin.joints)
					targetSkin.jointsIndices.push_back(index);

				{
					const auto& gltfAccessor = gltfModel.accessors[gltfSkin.inverseBindMatrices];
					if (gltfAccessor.bufferView < 0)
						continue;

					const auto& gltfBufferView = gltfModel.bufferViews[gltfAccessor.bufferView];
					if (gltfBufferView.target != GL_ARRAY_BUFFER)
						continue;

					const auto& gltfBuffer = gltfModel.buffers[gltfBufferView.buffer];
					if (gltfBuffer.data.empty())
						continue;

					auto gltfDataStride = gltfAccessor.ByteStride(gltfBufferView);
					if (gltfDataStride < 0)
						continue;

					auto gltfData = gltfBuffer.data.data() + gltfBufferView.byteOffset + gltfAccessor.byteOffset;

					for (size_t curIndex = 0; curIndex < targetSkin.jointsIndices.size(); curIndex++)
					{
						Matrix mat(reinterpret_cast<const float*>(gltfData));
						gltfData += gltfDataStride;

						targetSkin.inverseBindMats.push_back(std::move(mat));
					}
				}

				skins.push_back(std::move(targetSkin));
			}
		}

		//to recursively read all nodes

		std::function<void(const tinygltf::Node&, const Matrix&)> recurNodes;
		recurNodes = [&](const tinygltf::Node& gltfNode, const Matrix& previousTransform)
		{
			Matrix newTransform;
			Matrix nodeTransform;
			{
				if (gltfNode.matrix.size() == 16)
					nodeTransform = Matrix(gltfNode.matrix.data());
				else
				{
					if (gltfNode.translation.size() == 3)
						nodeTransform.mulTranslation(static_cast<float>(gltfNode.translation[0]), static_cast<float>(gltfNode.translation[1]), static_cast<float>(gltfNode.translation[2]));
					if (gltfNode.rotation.size() == 4)
						nodeTransform *= Quaternion(gltfNode.rotation.data());
					if (gltfNode.scale.size() == 3)
						nodeTransform.mulScale(static_cast<float>(gltfNode.scale[0]), static_cast<float>(gltfNode.scale[1]), static_cast<float>(gltfNode.scale[2]));
				}

				newTransform = previousTransform * nodeTransform;
			}

			if (gltfNode.mesh >= 0)
			{
				auto& gltfMesh = gltfModel.meshes[gltfNode.mesh];
				for (const auto& gltfPrim : gltfMesh.primitives)
				{
					if ((gltfPrim.mode != TINYGLTF_MODE_TRIANGLES) || (gltfPrim.indices < 0))
						continue;
					
					hr::geom::MeshAnim newMeshAnim;
					bool hasPos = false;
					bool hasNormal = false;
					bool hasTangent = false;
					bool hasTexCoords = false;
					bool hasJoints = false;
					bool hasJointsWheights = false;

					//read indices and prepare mesh
					{
						const auto& gltfAccessor = gltfModel.accessors[gltfPrim.indices];
						if ((gltfAccessor.bufferView < 0) || (gltfAccessor.count <= 0) || ((gltfAccessor.count % 3) != 0) || gltfAccessor.normalized)
							continue;

						const auto& gltfBufferView = gltfModel.bufferViews[gltfAccessor.bufferView];
						if (gltfBufferView.target != GL_ELEMENT_ARRAY_BUFFER)
							continue;

						const auto& gltfBuffer = gltfModel.buffers[gltfBufferView.buffer];
						if (gltfBuffer.data.empty())
							continue;

						if ((gltfAccessor.maxValues.size() == 1) && (gltfAccessor.maxValues[0] >= hr::geom::Mesh::maxVertexCount()))
							continue;

						auto gltfData = gltfBuffer.data.data() + gltfBufferView.byteOffset;

						std::unique_ptr<unsigned short[]> newIndices;
						switch (gltfAccessor.componentType)
						{
						case TINYGLTF_COMPONENT_TYPE_UNSIGNED_BYTE:
							if (gltfAccessor.ByteStride(gltfBufferView) != 1)
								continue;

							newIndices = std::unique_ptr<unsigned short[]>(new unsigned short[gltfAccessor.count]);
							for (size_t i = 0; i < gltfAccessor.count; i++)
								newIndices[i] = (gltfData + gltfAccessor.byteOffset)[i];
							break;
						case TINYGLTF_COMPONENT_TYPE_UNSIGNED_SHORT:
							if (gltfAccessor.ByteStride(gltfBufferView) != 2)
								continue;

							newIndices = std::unique_ptr<unsigned short[]>(new unsigned short[gltfAccessor.count]);
							for (size_t i = 0; i < gltfAccessor.count; i++)
								newIndices[i] = (reinterpret_cast<const unsigned short*>(gltfData + gltfAccessor.byteOffset))[i];
							break;
						case TINYGLTF_COMPONENT_TYPE_UNSIGNED_INT:
							if ((gltfAccessor.ByteStride(gltfBufferView) != 4) || (gltfAccessor.maxValues.size() != 1))
								continue;

							newIndices = std::unique_ptr<unsigned short[]>(new unsigned short[gltfAccessor.count]);
							for (size_t i = 0; i < gltfAccessor.count; i++)
								newIndices[i] = static_cast<unsigned short>((reinterpret_cast<const uint32_t*>(gltfData + gltfAccessor.byteOffset))[i]);
							break;
						default:
							continue;
						}

						size_t numVertices = 0;
						for (size_t i = 0; i < gltfAccessor.count; i++)
							numVertices = std::max<size_t>(numVertices, newIndices[i]);

						numVertices++;
						auto newVertices = std::unique_ptr<hr::geom::Mesh::VertexData[]>(new hr::geom::Mesh::VertexData[numVertices]);
						
						newMeshAnim = hr::geom::MeshAnim(
							hr::geom::Mesh(std::move(newVertices), numVertices, std::move(newIndices), gltfAccessor.count),
							hr::geom::MeshAnim::SkinningType::Vertex4Joints);
					}

					//read vertex data
					for (const auto& gltfAttrib : gltfPrim.attributes)
					{
						if (gltfAttrib.second < 0)
							continue;

						const auto& gltfAccessor = gltfModel.accessors[gltfAttrib.second];
						if (gltfAccessor.bufferView < 0)
							continue;

						const auto& gltfBufferView = gltfModel.bufferViews[gltfAccessor.bufferView];
						if (gltfBufferView.target != GL_ARRAY_BUFFER)
							continue;

						const auto& gltfBuffer = gltfModel.buffers[gltfBufferView.buffer];
						if (gltfBuffer.data.empty())
							continue;

						auto gltfDataStride = gltfAccessor.ByteStride(gltfBufferView);
						if (gltfDataStride < 0)
							continue;

						auto vertexData = newMeshAnim.mesh().vertices();
						auto vertexJoints = newMeshAnim.verticesJoints();
						auto gltfData = gltfBuffer.data.data() + gltfBufferView.byteOffset + gltfAccessor.byteOffset;

						if ((gltfAttrib.first == "POSITION") && (gltfAccessor.type == TINYGLTF_TYPE_VEC3) && (gltfAccessor.componentType == TINYGLTF_COMPONENT_TYPE_FLOAT))
						{
							hasPos = true;
							for (size_t curIndex = 0; curIndex < newMeshAnim.mesh().numVertices(); curIndex++)
							{
								std::memcpy(vertexData[curIndex].pos, gltfData, sizeof(float) * 3);
								gltfData += gltfDataStride;
							}
						}
						else if ((gltfAttrib.first == "TEXCOORD_0") && (gltfAccessor.type == TINYGLTF_TYPE_VEC2) && (gltfAccessor.componentType == TINYGLTF_COMPONENT_TYPE_FLOAT))
						{
							hasTexCoords = true;
							for (size_t curIndex = 0; curIndex < newMeshAnim.mesh().numVertices(); curIndex++)
							{
								std::memcpy(vertexData[curIndex].uv, gltfData, sizeof(float) * 2);
								gltfData += gltfDataStride;
							}
						}
						else if ((gltfAttrib.first == "NORMAL") && (gltfAccessor.type == TINYGLTF_TYPE_VEC3) && (gltfAccessor.componentType == TINYGLTF_COMPONENT_TYPE_FLOAT))
						{
							hasNormal = true;
							for (size_t curIndex = 0; curIndex < newMeshAnim.mesh().numVertices(); curIndex++)
							{
								hr::geom::Mesh::pack(reinterpret_cast<const float*>(gltfData), vertexData[curIndex].normal, 3);
								gltfData += gltfDataStride;
							}
						}
						else if ((gltfAttrib.first == "TANGENT") && (gltfAccessor.type == TINYGLTF_TYPE_VEC4) && (gltfAccessor.componentType == TINYGLTF_COMPONENT_TYPE_FLOAT))
						{
							hasTangent = true;
							for (size_t curIndex = 0; curIndex < newMeshAnim.mesh().numVertices(); curIndex++)
							{
								hr::geom::Mesh::pack(reinterpret_cast<const float*>(gltfData), vertexData[curIndex].tangent, 4);
								gltfData += gltfDataStride;
							}
						}
						else if ((gltfAttrib.first == "JOINTS_0") && (gltfAccessor.type == TINYGLTF_TYPE_VEC4))
						{
							if (gltfAccessor.componentType == TINYGLTF_COMPONENT_TYPE_UNSIGNED_BYTE)
							{
								hasJoints = true;
								for (size_t curIndex = 0; curIndex < newMeshAnim.mesh().numVertices(); curIndex++)
								{
									vertexJoints[(curIndex * 4) + 0].jointIndex = gltfData[0];
									vertexJoints[(curIndex * 4) + 1].jointIndex = gltfData[1];
									vertexJoints[(curIndex * 4) + 2].jointIndex = gltfData[2];
									vertexJoints[(curIndex * 4) + 3].jointIndex = gltfData[3];
									gltfData += gltfDataStride;
								}
							}
							else if (gltfAccessor.componentType == TINYGLTF_COMPONENT_TYPE_UNSIGNED_SHORT)
							{
								hasJoints = true;
								for (size_t curIndex = 0; curIndex < newMeshAnim.mesh().numVertices(); curIndex++)
								{
									auto dataPtr = reinterpret_cast<const unsigned short*>(gltfData);
									vertexJoints[(curIndex * 4) + 0].jointIndex = dataPtr[0];
									vertexJoints[(curIndex * 4) + 1].jointIndex = dataPtr[1];
									vertexJoints[(curIndex * 4) + 2].jointIndex = dataPtr[2];
									vertexJoints[(curIndex * 4) + 3].jointIndex = dataPtr[3];
									gltfData += gltfDataStride;
								}
							}
						}
						else if ((gltfAttrib.first == "WEIGHTS_0") && (gltfAccessor.type == TINYGLTF_TYPE_VEC4))
						{
							if (gltfAccessor.componentType == TINYGLTF_COMPONENT_TYPE_UNSIGNED_SHORT)
							{
								hasJointsWheights = true;
								for (size_t curIndex = 0; curIndex < newMeshAnim.mesh().numVertices(); curIndex++)
								{
									auto dataPtr = reinterpret_cast<const unsigned short*>(gltfData);
									vertexJoints[(curIndex * 4) + 0].jointWeight = dataPtr[0];
									vertexJoints[(curIndex * 4) + 1].jointWeight = dataPtr[1];
									vertexJoints[(curIndex * 4) + 2].jointWeight = dataPtr[2];
									vertexJoints[(curIndex * 4) + 3].jointWeight = dataPtr[3];
									gltfData += gltfDataStride;
								}
							}
							else if (gltfAccessor.componentType == TINYGLTF_COMPONENT_TYPE_FLOAT)
							{
								hasJointsWheights = true;
								for (size_t curIndex = 0; curIndex < newMeshAnim.mesh().numVertices(); curIndex++)
								{
									auto dataPtr = reinterpret_cast<const float*>(gltfData);
									vertexJoints[(curIndex * 4) + 0].jointWeight = hr::geom::Mesh::pack(dataPtr[0]);
									vertexJoints[(curIndex * 4) + 1].jointWeight = hr::geom::Mesh::pack(dataPtr[1]);
									vertexJoints[(curIndex * 4) + 2].jointWeight = hr::geom::Mesh::pack(dataPtr[2]);
									vertexJoints[(curIndex * 4) + 3].jointWeight = hr::geom::Mesh::pack(dataPtr[3]);
									gltfData += gltfDataStride;
								}
							}
						}
					}

					if (!newMeshAnim.mesh().check() || !hasPos || !hasTexCoords)
						continue;

					if (!hasNormal)
						newMeshAnim.mesh().genNormals();
					if (!hasTangent)
						newMeshAnim.mesh().genTangents4();

					newMeshAnim.mesh().optimizeIndices();

					newMeshAnim.mesh().transform(newTransform, Matrix3(newTransform));

					//we can now create a new object
					{
						//create object
						auto objectId = genObjectId(area);
						auto& object = area.mObjects[objectId];
						auto& objectData = areaData.objects[objectId];

						//we store new geometry immediately
						{
							hr::streams::FileStream geomFileStream(areaData.pathBin, true, true);
							World::geomFileAddMesh(geomFileStream, objectId, newMeshAnim.mesh());
						}

						//object can already be prepared
						object.id = objectId;
						object.type = Object::Type::Static;
						object.bbox = newMeshAnim.mesh().getBoundingBox();
						objectData.objectId = objectId;
						objectData.name = gltfMesh.name;
						objectData.geom.numVertices = newMeshAnim.mesh().numVertices();
						objectData.geom.numIndices = newMeshAnim.mesh().numIndices();
						{
							hr::streams::FileStream geomFileStream(areaData.pathBin, true, true);
							World::geomFileRetrieveOffsets(geomFileStream, objectId, objectData.geom.fstreamVertexOffset, objectData.geom.fstreamIndexOffset);
						}

						//read material info
						if (gltfPrim.material >= 0)
						{
							const auto& gltfMat = gltfModel.materials[gltfPrim.material];
							if (!gltfMat.name.empty())
								objectData.name = objectData.name + "_" + gltfMat.name;

							auto extractImageUri = [](const  tinygltf::Model& model, const tinygltf::Material& material, std::string_view componentName) -> std::string
							{
								auto itComponent = material.values.find(std::string(componentName));
								if (itComponent == material.values.end())
								{
									itComponent = material.additionalValues.find(std::string(componentName));
									if (itComponent == material.additionalValues.end())
										return {};
								}

								auto itIndex = itComponent->second.json_double_value.find("index");
								auto itTexCoord = itComponent->second.json_double_value.find("texCoord");
								if ((itIndex == itComponent->second.json_double_value.end()) && (itTexCoord == itComponent->second.json_double_value.end()))
									return {};

								auto texIndex = static_cast<int>(itIndex->second);
								if ((texIndex < 0) && (static_cast<int>(itTexCoord->second) != 0)) // we support only one set of UVs
									return {};

								if (model.textures[texIndex].source < 0)
									return {};

								return model.images[model.textures[texIndex].source].uri;
							};

							objectData.matDiffusePath = extractImageUri(gltfModel, gltfMat, "baseColorTexture");
							objectData.matNormalPath = extractImageUri(gltfModel, gltfMat, "normalTexture");
						}

						//create an object associated with the concept
						Instance newInstance;
						newInstance.type = Instance::Type::Static;
						newInstance.objectId = object.id;
						newInstance.bbox = object.bbox;
						area.mInstances.push_back(newInstance);
					}
				}
			}

			for (const auto& nodeIndex : gltfNode.children)
				recurNodes(gltfModel.nodes[nodeIndex], newTransform);
		};

		//recursively parse all nodes *only* in the default scene
		for (const auto& nodeIndex : gltfModel.scenes[gltfModel.defaultScene].nodes)
			recurNodes(gltfModel.nodes[nodeIndex], {});

		//need to save everything to file (new geometry was already saved)
		saveArea(area);

		return true;
	}

	void WorldEditor::processMesh(AreaId areaId, const std::vector<size_t>& objectIds, const std::function<void(hr::geom::Mesh&)>& cb)
	{
		if (!cb || (mAreas.find(areaId) == mAreas.end()))
			return;

		auto& area = mAreas[areaId];
		auto& areaData = mAreasData[areaId];

		hr::streams::FileStream geomFileStream(areaData.pathBin, true, true);

		World::geomFileTransformMeshes(geomFileStream, objectIds, cb);
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

	void WorldEditor::removeObjects(AreaId areaId, const std::vector<std::string_view>& objectsNames)
	{
		if (objectsNames.empty() || (mAreas.find(areaId) == mAreas.end()))
			return;

		auto& area = mAreas[areaId];
		auto& areaData = mAreasData[areaId];

		std::unordered_set<size_t> objectIds;
		for (auto&& targetName : objectsNames)
		{
			for (const auto&[objectId, object] : areaData.objects)
			{
				if (object.name == targetName)
					objectIds.insert(objectId);
			}
		}

		if (objectIds.empty())
			return;

		for (const auto& objectId : objectIds)
		{
			area.mObjects.erase(objectId);
			area.mInstances.erase(
				std::remove_if(area.mInstances.begin(), area.mInstances.end(), [objectId](const auto& instance) { return (instance.objectId == objectId); }),
				area.mInstances.end());

			areaData.objects.erase(objectId);
		}

		saveArea(area); //this only takes care of the scene

		//now we take care of the binary data
		{
			std::filesystem::rename(areaData.pathBin, areaData.pathBin + ".tmp");

			{
				hr::streams::FileStream geomFileStreamOld(areaData.pathBin + ".tmp", true, false);
				hr::streams::FileStream geomFileStreamNew(areaData.pathBin, false, true);

				std::vector<size_t> finalObjectIds;
				finalObjectIds.insert(finalObjectIds.end(), objectIds.begin(), objectIds.end());

				geomFileRemoveGeom(geomFileStreamOld, geomFileStreamNew, std::move(finalObjectIds));
			}

			std::filesystem::remove(areaData.pathBin + ".tmp");
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

		if (std::filesystem::exists(areaData.pathScene))
			std::filesystem::remove(areaData.pathScene);

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

			writer.String("joints");
			writer.StartArray();
			for (auto& joint : animSet.second.joints)
			{
				writer.StartObject();

				writer.String("index");
				writer.Uint(joint.index);

				writer.String("parent");
				writer.Int(joint.parentIndex);

				writer.String("name");
				writer.String(joint.name.data());

				writer.EndObject();
			}
			writer.EndArray();

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