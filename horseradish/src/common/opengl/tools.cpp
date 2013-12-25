#include "tools.hpp"
#include "common\Sorting.hpp"
#include "common\Hashing.hpp"

namespace HorseRadish
{

namespace OpenGL
{

namespace Tools
{

//§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§
//§§§§§§   Classe UniformCache	§§§§§
//§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§
int UniformCache::comparePrograms(const ProgramData &objA, const ProgramData &objB)
{
	if (objA.programID < objB.programID)
		return -1;
	if (objA.programID > objB.programID)
		return 1;
	return 0;
}
int UniformCache::compareUniforms(const ProgramData::UniformData &objA, const ProgramData::UniformData &objB)
{
	if (objA.uniformCRC32 < objB.uniformCRC32)
		return -1;
	if (objA.uniformCRC32 > objB.uniformCRC32)
		return 1;
	return 0;
}

UniformCache::UniformCache()
{
	//não é preciso nada
}

UniformCache::~UniformCache()
{
	//basta limpar tudo
	this->Clean();
}

bool UniformCache::AddUniform(const unsigned int programID, const char * const uniformName, const int uniformPos)
{
	bool crieiPrograma, crieiUniform;
	int programaAlvoIndex, uniformAlvoIndex;
	unsigned int uniformCRC32;
	ProgramData *programaAlvo, programDummy;
	ProgramData::UniformData *uniformAlvo, uniformDummy;

	//por defeito não criei nada
	crieiPrograma = crieiUniform = false;

	//tiro o programa alvo e se não tenho nada, tenho de criar
	programDummy.programID = programID;
	programaAlvoIndex = HorseRadish::Sorting::BinarySearch<ProgramData>(this->arrayPrograms.GetMainPointer(), this->arrayPrograms.GetNumElements(), UniformCache::comparePrograms, programDummy);
	if (programaAlvoIndex < 0)
	{
		//crio um novo
		programaAlvo = this->arrayPrograms.Add();
		programaAlvo->programID = programID;
		crieiPrograma = true;
	}
	else
	{
		//o ponteiro para o programa
		programaAlvo = this->arrayPrograms + programaAlvoIndex;
	}

	//agora calculo o CRC do uniform em questão
	uniformCRC32 = HorseRadish::Hashing::CalculateCRC32(uniformName, strlen(uniformName));

	//agora neste programa, procuro se já existe este uniform e se não existir crio espaço para mais
	uniformDummy.uniformCRC32 = uniformCRC32;
	uniformAlvoIndex = HorseRadish::Sorting::BinarySearch<ProgramData::UniformData>(programaAlvo->arrayUniforms.GetMainPointer(), programaAlvo->arrayUniforms.GetNumElements(), UniformCache::compareUniforms, uniformDummy);
	if (uniformAlvoIndex < 0)	
	{
		//crio um novo
		uniformAlvo = programaAlvo->arrayUniforms.Add();
		uniformAlvo->uniformCRC32 = uniformCRC32;
		crieiUniform = true;
	}
	else
	{
		//o ponteiro para o uniform
		uniformAlvo = programaAlvo->arrayUniforms + uniformAlvoIndex;
	}

	//chegando aqui só tenho de gravar a posição do uniform
	uniformAlvo->uniformPos = uniformPos;

	//se acrescentei alguma coisa, tenho de reordenar a lista (o uniform tem de estar antes do programa)
	if (crieiUniform)
		programaAlvo->arrayUniforms.QuickSort(UniformCache::compareUniforms);
	if (crieiPrograma)
		this->arrayPrograms.QuickSort(UniformCache::comparePrograms);

	//tá tudo
	return true;
}

int UniformCache::GetUniformPos(const unsigned int programID, const char * const uniformName, const bool create)
{
	unsigned int uniformCRC32;
	int programaAlvoIndex, uniformAlvoIndex;
	ProgramData *programaAlvo, programDummy;
	ProgramData::UniformData uniformDummy;

	//tiro o programa alvo e se não tenho nada, crio o que for preciso
	programDummy.programID = programID;
	programaAlvoIndex = HorseRadish::Sorting::BinarySearch<ProgramData>(this->arrayPrograms.GetMainPointer(), this->arrayPrograms.GetNumElements(), UniformCache::comparePrograms, programDummy);
	if (programaAlvoIndex < 0)
	{
		int uniformPos;

		//se ninguem quer que eu crio esta nova entrada, bazo
		if (create == false)
			return -1;
		
		//peço a posição do uniform ao GL, crio uma nova entrada e posso devolver a posição que acabei de tirar
		uniformPos = HorseRadish::OpenGL::glGetUniformLocation(programID, uniformName);
		AddUniform(programID, uniformName, uniformPos);
		return uniformPos;
	}

	//o ponteiro para o programa
	programaAlvo = this->arrayPrograms + programaAlvoIndex;

	//agora calculo o CRC do uniform em questão
	uniformCRC32 = HorseRadish::Hashing::CalculateCRC32(uniformName, strlen(uniformName));

	//agora pesquiso pelo uniform em questão
	uniformDummy.uniformCRC32 = uniformCRC32;
	programaAlvoIndex = HorseRadish::Sorting::BinarySearch<ProgramData::UniformData>(programaAlvo->arrayUniforms.GetMainPointer(), programaAlvo->arrayUniforms.GetNumElements(), UniformCache::compareUniforms, uniformDummy);
	if (programaAlvoIndex < 0)
	{
		int uniformPos;

		//se ninguem quer que eu crio esta nova entrada, bazo
		if (create == false)
			return -1;
		
		//peço a posição do uniform ao GL, crio uma nova entrada e posso devolver a posição que acabei de tirar
		uniformPos = HorseRadish::OpenGL::glGetUniformLocation(programID, uniformName);
		AddUniform(programID, uniformName, uniformPos);
		return uniformPos;
	}

	//basta devolver a posição do uniform que acabei de achar
	return programaAlvo->arrayUniforms[programaAlvoIndex].uniformPos;
}

void UniformCache::Clean()
{
	//basta mandar limpar isto
	this->arrayPrograms.Clear();
}

//§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§
//§§§§§§   Classe ImmediateMode		§§§§§
//§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§
int ImmediateMode::draw()
{
	int numElementosDesenhados;

	//se as coisas não estão bem, posso sair
	if (checkStateDraw() == false)
		return 0;

	//por omissão
	numElementosDesenhados = 0;

	//se for para desenhar quadrados
	if (this->stateGeomType == GeometryType::Quads)
	{
		//calculo já isto
		numElementosDesenhados = this->curVertex / 4;

		//se realmente tiver alguma coisa
		if (numElementosDesenhados > 0)
		{
			unsigned short *newIndexWriter;

			//onde vou começar a escrever os novos indices
			newIndexWriter = this->bufferIndices + (numElementosDesenhados * 6 - 6);

			//tenho de arranjar todos os indices dos quads (expandir de 4 para 2*3 tris)
			for(int curQuad = curVertex - 4; curQuad >= 0; curQuad -= 4)
			{
				unsigned short index1, index2, index3, index4;

				//leio os indices a usar
				index1 = this->bufferIndices[curQuad + 0];
				index2 = this->bufferIndices[curQuad + 1];
				index3 = this->bufferIndices[curQuad + 2];
				index4 = this->bufferIndices[curQuad + 3];

				//posso escrever os novos indices
				newIndexWriter[0] = newIndexWriter[3] = index1;
				newIndexWriter[1] = index2;
				newIndexWriter[2] = newIndexWriter[4] = index3;
				newIndexWriter[5] = index4;
				newIndexWriter -= 6;
			}

			//posso actualizar os buffers
			HorseRadish::OpenGL::glBindBuffer(GL_ARRAY_BUFFER, this->glArrayBufferID);
			HorseRadish::OpenGL::glBufferSubData(GL_ARRAY_BUFFER, 0, numElementosDesenhados * 4 * sizeof(VertexDataLayout), this->bufferData);
			HorseRadish::OpenGL::glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, this->glElementArrayBufferID);
			HorseRadish::OpenGL::glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, 0, sizeof(unsigned short) * numElementosDesenhados * 6, this->bufferIndices);

			//e mando desenhar
			HorseRadish::OpenGL::glDrawRangeElements(GL_TRIANGLES, 0, numElementosDesenhados * 4, numElementosDesenhados * 6, GL_UNSIGNED_SHORT, (void*)0);
		}
	}
	//se for para desenhar triangulos
	else if (this->stateGeomType == GeometryType::Tris)
	{
		//calculo já isto
		numElementosDesenhados = this->curVertex / 3;

		//se realmente tiver alguma coisa
		if (numElementosDesenhados > 0)
		{
			//posso actualizar os buffers
			HorseRadish::OpenGL::glBindBuffer(GL_ARRAY_BUFFER, this->glArrayBufferID);
			HorseRadish::OpenGL::glBufferSubData(GL_ARRAY_BUFFER, 0, numElementosDesenhados * 3 * sizeof(VertexDataLayout), this->bufferData);
			HorseRadish::OpenGL::glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, this->glElementArrayBufferID);
			HorseRadish::OpenGL::glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, 0, sizeof(unsigned short) * numElementosDesenhados * 3, this->bufferIndices);

			//e mando desenhar
			HorseRadish::OpenGL::glDrawRangeElements(GL_TRIANGLES, 0, numElementosDesenhados * 3, numElementosDesenhados * 3, GL_UNSIGNED_SHORT, (void*)0);
		}
	}
	//se for para desenhar linhas
	else if (this->stateGeomType == GeometryType::Lines)
	{
		//calculo já isto
		numElementosDesenhados = this->curVertex / 2;

		//se realmente tiver alguma coisa
		if (numElementosDesenhados > 0)
		{
			//posso actualizar os buffers
			HorseRadish::OpenGL::glBindBuffer(GL_ARRAY_BUFFER, this->glArrayBufferID);
			HorseRadish::OpenGL::glBufferSubData(GL_ARRAY_BUFFER, 0, numElementosDesenhados * 2 * sizeof(VertexDataLayout), this->bufferData);
			HorseRadish::OpenGL::glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, this->glElementArrayBufferID);
			HorseRadish::OpenGL::glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, 0, sizeof(unsigned short) * numElementosDesenhados * 2, this->bufferIndices);

			//e mando desenhar
			HorseRadish::OpenGL::glDrawRangeElements(GL_LINES, 0, numElementosDesenhados * 2, numElementosDesenhados * 2, GL_UNSIGNED_SHORT, (void*)0);
		}
	}
	//se for para desenhar linestrips
	else if (this->stateGeomType == GeometryType::LineStrip)
	{
		//calculo já isto
		numElementosDesenhados = this->curVertex - 1;

		//se realmente tiver alguma coisa
		if (numElementosDesenhados > 0)
		{
			//posso actualizar os buffers
			HorseRadish::OpenGL::glBindBuffer(GL_ARRAY_BUFFER, this->glArrayBufferID);
			HorseRadish::OpenGL::glBufferSubData(GL_ARRAY_BUFFER, 0, curVertex * sizeof(VertexDataLayout), this->bufferData);
			HorseRadish::OpenGL::glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, this->glElementArrayBufferID);
			HorseRadish::OpenGL::glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, 0, sizeof(unsigned short) * this->curVertex, this->bufferIndices);

			//e mando desenhar
			HorseRadish::OpenGL::glDrawRangeElements(GL_LINE_STRIP, 0, this->curVertex, this->curVertex, GL_UNSIGNED_SHORT, (void*)0);
		}
	}

	//não tenho mais nada pra desenhar
	this->curVertex = 0;

	//posso sair
	return numElementosDesenhados;
}

void ImmediateMode::resetState()
{
	//não estou a desenhar nada
	this->stateGeomType = GeometryType::None;
	this->curVertex = 0;

	//as coordenadas de textura e cores
	this->stateUV[0] = 0.0f;
	this->stateUV[1] = 0.0f;
	this->stateColor[0] = 0;
	this->stateColor[1] = 0;
	this->stateColor[2] = 0;
	this->stateColor[3] = 255;
}

bool ImmediateMode::checkStateDraw() const
{
	//se não tenho geometria de jeito ou vértices
	if ((stateGeomType == GeometryType::None) || (curVertex <= 0))
		return false;

	//se for para desenhar quadrados, tenho de ter pelo menos 4 vertices
	if ((stateGeomType == GeometryType::Quads) && (curVertex >= 4))
		return true;

	//se for para desenhar triangulos, tenho de ter pelo menos 3 vertices
	if ((stateGeomType == GeometryType::Tris) && (curVertex >= 3))
		return true;

	//se for para desenhar linhas ou linestrips, tenho de ter pelo menos 2 vertices
	if (((stateGeomType == GeometryType::Lines) || (stateGeomType == GeometryType::LineStrip)) && (curVertex >= 2))
		return true;

	//chegando aqui não sei o que é para desenhar, logo é erro
	return false;
}

ImmediateMode::ImmediateMode(const int maxVertexCount)
{
	unsigned int maxElementArray;

	//posso já guardar isto
	this->maxVertexCount = (maxVertexCount < 20) ? 20 : maxVertexCount;

	//limpar isto
	this->bufferData = nullptr;
	this->bufferIndices = nullptr;

	//por omissão
	resetState();

	//preciso de calcular o tamanho do buffer para guardar os indices
	//é preciso ter indices extra para os quadrados (têm de ser transformados em tris)
	maxElementArray = ((this->maxVertexCount / 4) * 6) + 6;
	
	//crio o VBO para os dados
	HorseRadish::OpenGL::glGenBuffers(1, &this->glArrayBufferID);
	HorseRadish::OpenGL::glBindBuffer(GL_ARRAY_BUFFER, this->glArrayBufferID);
	HorseRadish::OpenGL::glBufferData(GL_ARRAY_BUFFER, this->maxVertexCount * sizeof(VertexDataLayout), nullptr, GL_DYNAMIC_DRAW);

	//crio o VBO para os indices
	HorseRadish::OpenGL::glGenBuffers(1, &this->glElementArrayBufferID);
	HorseRadish::OpenGL::glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, this->glElementArrayBufferID);
	HorseRadish::OpenGL::glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(unsigned short) * maxElementArray, nullptr, GL_DYNAMIC_DRAW);
	
	//crio o VAO
	HorseRadish::OpenGL::glGenVertexArrays(1, &this->glVertexArrayID);
	HorseRadish::OpenGL::glBindVertexArray(this->glVertexArrayID);
	HorseRadish::OpenGL::glBindBuffer(GL_ARRAY_BUFFER, this->glArrayBufferID);
	HorseRadish::OpenGL::glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, this->glElementArrayBufferID);
	HorseRadish::OpenGL::glEnableVertexAttribArray(0);
	HorseRadish::OpenGL::glEnableVertexAttribArray(1);
	HorseRadish::OpenGL::glEnableVertexAttribArray(4);
	HorseRadish::OpenGL::glVertexAttribPointer(0, 3, GL_FLOAT, false, sizeof(VertexDataLayout), (void*)0);
	HorseRadish::OpenGL::glVertexAttribPointer(1, 2, GL_FLOAT, false, sizeof(VertexDataLayout), (void*)12);
	HorseRadish::OpenGL::glVertexAttribPointer(4, 4, GL_UNSIGNED_BYTE, true, sizeof(VertexDataLayout), (void*)20);
	HorseRadish::OpenGL::glBindVertexArray(0);

	//crio os buffers
	this->bufferData = new VertexDataLayout[this->maxVertexCount]; 
	this->bufferIndices = new unsigned short[maxElementArray];
}

ImmediateMode::~ImmediateMode()
{
	//limpo o estado
	resetState();

	//apago tudo do GL
	HorseRadish::OpenGL::glDeleteVertexArrays(1, &this->glVertexArrayID);
	HorseRadish::OpenGL::glDeleteBuffers(1, &this->glElementArrayBufferID);
	HorseRadish::OpenGL::glDeleteBuffers(1, &this->glArrayBufferID);
	this->glVertexArrayID = 0;
	this->glElementArrayBufferID = 0;
	this->glArrayBufferID = 0;

	//limpo os buffers
	delete[] this->bufferData;
	delete[] this->bufferIndices;
	this->bufferData = nullptr;
	this->bufferIndices = nullptr;
}

void ImmediateMode::BeginDraw(const GeometryType geometryType)
{
	//a primeira coisa a fazer é reset do estado
	resetState();

	//se não for para fazer nada, tudo bem
	if (geometryType == GeometryType::None)
		return;

	//guardo o tipo de geometria que tenho de usar
	stateGeomType = geometryType;

	//faço bind do VAO que já tem tudo configurado
	HorseRadish::OpenGL::glBindVertexArray(this->glVertexArrayID);
}

void ImmediateMode::Draw()
{
	//basta mandar desenhar
	this->draw();
}

int ImmediateMode::EndDraw()
{
	int numElementosDesenhados;

	//mando desenhar
	numElementosDesenhados = this->draw();

	//como acabei o desenho, tenho de limpar tudo
	this->resetState();

	//tiro o VAO anterior para evitar que alguém o modifique
	HorseRadish::OpenGL::glBindVertexArray(0);

	//e pronto
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
	VertexDataLayout *vertexData;

	//se já não tenho buffer ou o tipo de dados é inválido
	if ((this->curVertex >= this->maxVertexCount) || (this->stateGeomType == GeometryType::None))
		return;

	//isto dá jeito
	vertexData = this->bufferData + this->curVertex;

	//actualizo o buffer com os dados do vertice
	vertexData->px = x;
	vertexData->py = y;
	vertexData->pz = z;
	vertexData->tu = this->stateUV[0];
	vertexData->tv = this->stateUV[1];
	vertexData->cr = this->stateColor[0];
	vertexData->cg = this->stateColor[1];
	vertexData->cb = this->stateColor[2];
	vertexData->ca = this->stateColor[3];

	//não me posso esquecer do indice
	this->bufferIndices[this->curVertex] = this->curVertex;

	//posso avançar para o próximo vértice
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
	this->AddPosition(x+width, y);
	this->AddPosition(x+width, y+height);
	this->AddPosition(x, y+height);
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
	//se for o número livre the vertices disponíveis
	if (infoType == InfoType::FreeVertexCount)
		return (this->maxVertexCount - this->curVertex);
	
	//se for o número máximo de vértices que posso ter
	if (infoType == InfoType::MaxVertexCount)
		return (this->maxVertexCount);

	//chegando aqui não sei o que se passa
	return 0;
}

}//namespace Tools
}//namespace OpenGL
}//namespace HorseRadish