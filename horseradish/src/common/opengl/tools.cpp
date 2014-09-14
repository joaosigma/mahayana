#include "tools.hpp"

#include <algorithm>

namespace HorseRadish
{
	namespace OpenGL
	{
		namespace Tools
		{
			UniformCache::UniformCache()
			{
			}

			UniformCache::~UniformCache()
			{
				this->Clean();
			}

			void UniformCache::AddUniform(const unsigned int programID, const char * const uniformName, const int uniformPos)
			{
				mapUniforms[std::make_pair(programID, std::string(uniformName))] = uniformPos;
			}

			int UniformCache::GetUniformPos(const unsigned int programID, const char * const uniformName, const bool create)
			{
				auto progUniformName = std::make_pair(programID, std::string(uniformName));
				auto uniformIndex = mapUniforms.find(progUniformName);

				if (uniformIndex != mapUniforms.end())
					return uniformIndex->second;

				int uniformPos = HorseRadish::OpenGL::glGetUniformLocation(programID, uniformName);
				mapUniforms[progUniformName] = uniformPos;
				return uniformPos;
			}

			void UniformCache::Clean()
			{
				this->mapUniforms.clear();
			}

			int ImmediateMode::draw()
			{
				if (checkStateDraw() == false)
					return 0;

				int numElementosDesenhados = 0;

				if (this->stateGeomType == GeometryType::Quads)
				{
					numElementosDesenhados = this->curVertex / 4;

					if (numElementosDesenhados > 0)
					{
						auto newIndexWriter = this->bufferIndices + (numElementosDesenhados * 6 - 6);

						for (int curQuad = curVertex - 4; curQuad >= 0; curQuad -= 4)
						{
							unsigned short index1 = this->bufferIndices[curQuad + 0];
							unsigned short index2 = this->bufferIndices[curQuad + 1];
							unsigned short index3 = this->bufferIndices[curQuad + 2];
							unsigned short index4 = this->bufferIndices[curQuad + 3];

							newIndexWriter[0] = newIndexWriter[3] = index1;
							newIndexWriter[1] = index2;
							newIndexWriter[2] = newIndexWriter[4] = index3;
							newIndexWriter[5] = index4;
							newIndexWriter -= 6;
						}

						HorseRadish::OpenGL::glNamedBufferSubData(this->glArrayBufferID, 0, numElementosDesenhados * 4 * sizeof(VertexDataLayout), this->bufferData);
						HorseRadish::OpenGL::glNamedBufferSubData(this->glElementArrayBufferID, 0, sizeof(unsigned short) * numElementosDesenhados * 6, this->bufferIndices);

						HorseRadish::OpenGL::glBindBuffer(GL_ARRAY_BUFFER, this->glArrayBufferID);
						HorseRadish::OpenGL::glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, this->glElementArrayBufferID);
						HorseRadish::OpenGL::glDrawRangeElements(GL_TRIANGLES, 0, numElementosDesenhados * 4, numElementosDesenhados * 6, GL_UNSIGNED_SHORT, (void*)0);
					}
				}
				else if (this->stateGeomType == GeometryType::Tris)
				{
					numElementosDesenhados = this->curVertex / 3;

					if (numElementosDesenhados > 0)
					{
						HorseRadish::OpenGL::glNamedBufferSubData(this->glArrayBufferID, 0, numElementosDesenhados * 3 * sizeof(VertexDataLayout), this->bufferData);
						HorseRadish::OpenGL::glNamedBufferSubData(this->glElementArrayBufferID, 0, sizeof(unsigned short) * numElementosDesenhados * 3, this->bufferIndices);

						HorseRadish::OpenGL::glBindBuffer(GL_ARRAY_BUFFER, this->glArrayBufferID);
						HorseRadish::OpenGL::glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, this->glElementArrayBufferID);
						HorseRadish::OpenGL::glDrawRangeElements(GL_TRIANGLES, 0, numElementosDesenhados * 3, numElementosDesenhados * 3, GL_UNSIGNED_SHORT, (void*)0);
					}
				}
				else if (this->stateGeomType == GeometryType::Lines)
				{
					numElementosDesenhados = this->curVertex / 2;

					if (numElementosDesenhados > 0)
					{
						HorseRadish::OpenGL::glNamedBufferSubData(this->glArrayBufferID, 0, numElementosDesenhados * 2 * sizeof(VertexDataLayout), this->bufferData);
						HorseRadish::OpenGL::glNamedBufferSubData(this->glElementArrayBufferID, 0, sizeof(unsigned short) * numElementosDesenhados * 2, this->bufferIndices);

						HorseRadish::OpenGL::glBindBuffer(GL_ARRAY_BUFFER, this->glArrayBufferID);
						HorseRadish::OpenGL::glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, this->glElementArrayBufferID);
						HorseRadish::OpenGL::glDrawRangeElements(GL_LINES, 0, numElementosDesenhados * 2, numElementosDesenhados * 2, GL_UNSIGNED_SHORT, (void*)0);
					}
				}
				else if (this->stateGeomType == GeometryType::LineStrip)
				{
					numElementosDesenhados = this->curVertex - 1;

					if (numElementosDesenhados > 0)
					{
						HorseRadish::OpenGL::glNamedBufferSubData(this->glArrayBufferID, 0, curVertex * sizeof(VertexDataLayout), this->bufferData);
						HorseRadish::OpenGL::glNamedBufferSubData(this->glElementArrayBufferID, 0, sizeof(unsigned short) * this->curVertex, this->bufferIndices);

						HorseRadish::OpenGL::glBindBuffer(GL_ARRAY_BUFFER, this->glArrayBufferID);
						HorseRadish::OpenGL::glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, this->glElementArrayBufferID);
						HorseRadish::OpenGL::glDrawRangeElements(GL_LINE_STRIP, 0, this->curVertex, this->curVertex, GL_UNSIGNED_SHORT, (void*)0);
					}
				}

				this->curVertex = 0;

				return numElementosDesenhados;
			}

			void ImmediateMode::resetState()
			{
				this->stateGeomType = GeometryType::None;
				this->curVertex = 0;

				this->stateUV[0] = 0.0f;
				this->stateUV[1] = 0.0f;
				this->stateColor[0] = 0;
				this->stateColor[1] = 0;
				this->stateColor[2] = 0;
				this->stateColor[3] = 255;
			}

			bool ImmediateMode::checkStateDraw() const
			{
				if ((stateGeomType == GeometryType::None) || (curVertex <= 0))
					return false;

				if ((stateGeomType == GeometryType::Quads) && (curVertex >= 4))
					return true;

				if ((stateGeomType == GeometryType::Tris) && (curVertex >= 3))
					return true;

				if (((stateGeomType == GeometryType::Lines) || (stateGeomType == GeometryType::LineStrip)) && (curVertex >= 2))
					return true;

				return false;
			}

			ImmediateMode::ImmediateMode(const int maxVertexCount)
				: bufferData(nullptr), bufferIndices(nullptr)
			{
				this->maxVertexCount = (maxVertexCount < 20) ? 20 : maxVertexCount;

				resetState();

				unsigned int maxElementArray = ((this->maxVertexCount / 4) * 6) + 6;

				HorseRadish::OpenGL::glCreateBuffers(1, &this->glArrayBufferID); //GL_ARRAY_BUFFER
				HorseRadish::OpenGL::glNamedBufferStorage(this->glArrayBufferID, this->maxVertexCount * sizeof(VertexDataLayout), nullptr, GL_DYNAMIC_STORAGE_BIT);

				HorseRadish::OpenGL::glCreateBuffers(1, &this->glElementArrayBufferID); //GL_ELEMENT_ARRAY_BUFFER
				HorseRadish::OpenGL::glNamedBufferStorage(this->glElementArrayBufferID, sizeof(unsigned short) * maxElementArray, nullptr, GL_DYNAMIC_STORAGE_BIT);

				HorseRadish::OpenGL::glCreateVertexArrays(1, &this->glVertexArrayID);

				HorseRadish::OpenGL::glEnableVertexArrayAttrib(this->glVertexArrayID, 0);
				HorseRadish::OpenGL::glEnableVertexArrayAttrib(this->glVertexArrayID, 1);
				HorseRadish::OpenGL::glEnableVertexArrayAttrib(this->glVertexArrayID, 4);

				HorseRadish::OpenGL::glVertexArrayAttribBinding(this->glVertexArrayID, 0, 0);
				HorseRadish::OpenGL::glVertexArrayAttribFormat(this->glVertexArrayID, 0, 3, GL_FLOAT, false, 0);

				HorseRadish::OpenGL::glVertexArrayAttribBinding(this->glVertexArrayID, 1, 0);
				HorseRadish::OpenGL::glVertexArrayAttribFormat(this->glVertexArrayID, 1, 2, GL_FLOAT, false, 12);

				HorseRadish::OpenGL::glVertexArrayAttribBinding(this->glVertexArrayID, 4, 0);
				HorseRadish::OpenGL::glVertexArrayAttribFormat(this->glVertexArrayID, 4, 4, GL_UNSIGNED_BYTE, true, 20);

				HorseRadish::OpenGL::glVertexArrayElementBuffer(this->glVertexArrayID, this->glElementArrayBufferID);
				HorseRadish::OpenGL::glVertexArrayVertexBuffer(this->glVertexArrayID, 0, this->glArrayBufferID, 0, sizeof(VertexDataLayout));

				this->bufferData = new VertexDataLayout[this->maxVertexCount];
				this->bufferIndices = new unsigned short[maxElementArray];
			}

			ImmediateMode::~ImmediateMode()
			{
				resetState();

				HorseRadish::OpenGL::glDeleteVertexArrays(1, &this->glVertexArrayID);
				HorseRadish::OpenGL::glDeleteBuffers(1, &this->glElementArrayBufferID);
				HorseRadish::OpenGL::glDeleteBuffers(1, &this->glArrayBufferID);
				this->glVertexArrayID = 0;
				this->glElementArrayBufferID = 0;
				this->glArrayBufferID = 0;

				delete[] this->bufferData;
				delete[] this->bufferIndices;
				this->bufferData = nullptr;
				this->bufferIndices = nullptr;
			}

			void ImmediateMode::BeginDraw(const GeometryType geometryType)
			{
				resetState();

				if (geometryType == GeometryType::None)
					return;

				stateGeomType = geometryType;

				HorseRadish::OpenGL::glBindVertexArray(this->glVertexArrayID);
			}

			void ImmediateMode::Draw()
			{
				this->draw();
			}

			int ImmediateMode::EndDraw()
			{
				auto numElementosDesenhados = this->draw();

				this->resetState();

				HorseRadish::OpenGL::glBindVertexArray(0);

				return numElementosDesenhados;
			}

			void ImmediateMode::AddPosition(const float &x)
			{
				this->AddPosition(x, 0.0f, 0.0f);
			}

			void ImmediateMode::AddPosition(const float &x, const float &y)
			{
				this->AddPosition(x, y, 0.0f);
			}

			void ImmediateMode::AddPosition(const float &x, const float &y, const float &z)
			{
				if ((this->curVertex >= this->maxVertexCount) || (this->stateGeomType == GeometryType::None))
					return;

				auto vertexData = this->bufferData + this->curVertex;

				vertexData->px = x;
				vertexData->py = y;
				vertexData->pz = z;
				vertexData->tu = this->stateUV[0];
				vertexData->tv = this->stateUV[1];
				vertexData->cr = this->stateColor[0];
				vertexData->cg = this->stateColor[1];
				vertexData->cb = this->stateColor[2];
				vertexData->ca = this->stateColor[3];

				this->bufferIndices[this->curVertex] = this->curVertex;

				this->curVertex++;
			}

			void ImmediateMode::AddTexCoord(const float &u, const float &v)
			{
				stateUV[0] = u;
				stateUV[1] = v;
			}

			void ImmediateMode::AddColor(const unsigned char &r, const unsigned char &g, const unsigned char &b)
			{
				stateColor[0] = r;
				stateColor[1] = g;
				stateColor[2] = b;
				stateColor[3] = 255;
			}

			void ImmediateMode::AddColor(const unsigned char &r, const unsigned char &g, const unsigned char &b, const unsigned char &a)
			{
				stateColor[0] = r;
				stateColor[1] = g;
				stateColor[2] = b;
				stateColor[3] = a;
			}

			void ImmediateMode::AddColorF(const float &rgb)
			{
				stateColor[0] = Color::ConvertColor(rgb);
				stateColor[1] = stateColor[0];
				stateColor[2] = stateColor[0];
				stateColor[3] = 255;
			}

			void ImmediateMode::AddColorF(const float &rgb, const float &a)
			{
				stateColor[0] = Color::ConvertColor(rgb);
				stateColor[1] = stateColor[0];
				stateColor[2] = stateColor[0];
				stateColor[3] = Color::ConvertColor(a);
			}

			void ImmediateMode::AddColorF(const float &r, const float &g, const float &b)
			{
				stateColor[0] = Color::ConvertColor(r);
				stateColor[1] = Color::ConvertColor(g);
				stateColor[2] = Color::ConvertColor(b);
				stateColor[3] = 255;
			}

			void ImmediateMode::AddColorF(const float &r, const float &g, const float &b, const float &a)
			{
				stateColor[0] = Color::ConvertColor(r);
				stateColor[1] = Color::ConvertColor(g);
				stateColor[2] = Color::ConvertColor(b);
				stateColor[3] = Color::ConvertColor(a);
			}

			void ImmediateMode::AddColorRGB(const unsigned char * const values)
			{
				stateColor[0] = values[0];
				stateColor[1] = values[1];
				stateColor[2] = values[2];
				stateColor[3] = 255;
			}

			void ImmediateMode::AddColorRGB(const float * const values)
			{
				stateColor[0] = HorseRadish::Color::ConvertColor(values[0]);
				stateColor[1] = HorseRadish::Color::ConvertColor(values[1]);
				stateColor[2] = HorseRadish::Color::ConvertColor(values[2]);
				stateColor[3] = 255;
			}

			void ImmediateMode::AddQuad(const float &x, const float &y, const float &width, const float &height)
			{
				if (stateGeomType != GeometryType::Quads)
					return;

				this->AddPosition(x, y);
				this->AddPosition(x + width, y);
				this->AddPosition(x + width, y + height);
				this->AddPosition(x, y + height);
			}

			void ImmediateMode::AddQuadTexCoords(const float &x, const float &y, const float &width, const float &height, const bool &normalizedTexCoords)
			{
				if (stateGeomType != GeometryType::Quads)
					return;

				if (normalizedTexCoords == true)
				{
					stateUV[0] = 0.0f;
					stateUV[1] = 0.0f;
					this->AddPosition(x, y);
					stateUV[0] = 1.0f;
					this->AddPosition(x + width, y);
					stateUV[1] = 1.0f;
					this->AddPosition(x + width, y + height);
					stateUV[0] = 0.0f;
					this->AddPosition(x, y + height);
				}
				else
				{
					stateUV[0] = x;
					stateUV[1] = y;
					this->AddPosition(x, y);
					stateUV[0] = x + width;
					this->AddPosition(x + width, y);
					stateUV[1] = y + height;
					this->AddPosition(x + width, y + height);
					stateUV[0] = x;
					this->AddPosition(x, y + height);
				}
			}

			void ImmediateMode::AddLine(const float &x1, const float &y1, const float &x2, const float &y2)
			{
				if (stateGeomType != GeometryType::Lines)
					return;

				this->AddPosition(x1, y1);
				this->AddPosition(x2, y2);
			}

			void ImmediateMode::AddLineH(const float &x1, const float &x2, const float &y)
			{
				if (stateGeomType != GeometryType::Lines)
					return;

				this->AddPosition(x1, y);
				this->AddPosition(x2, y);
			}

			void ImmediateMode::AddLineV(const float &x, const float &y1, const float &y2)
			{
				if (stateGeomType != GeometryType::Lines)
					return;

				this->AddPosition(x, y1);
				this->AddPosition(x, y2);
			}

			unsigned int ImmediateMode::GetInfo(const InfoType infoType) const
			{
				if (infoType == InfoType::FreeVertexCount)
					return (this->maxVertexCount - this->curVertex);

				if (infoType == InfoType::MaxVertexCount)
					return (this->maxVertexCount);

				return 0;
			}

		} //Tools
	} //OpenGL
} //HorseRadish