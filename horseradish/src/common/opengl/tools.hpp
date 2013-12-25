#pragma once
#ifndef __HOPENGL_TOOLS__
#define __HOPENGL_TOOLS__

#include "openGL.h"
#include "objects.hpp"
#include "common\Ray.hpp"
#include "common\Containers.hpp"

namespace HorseRadish
{

namespace OpenGL
{

namespace Tools
{

//algumas declarações
class UniformCache;
class ImmediateMode;
class FontManager;
class Font;
class Viewport;
class Frustum;
class Camera;

//§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§
//§§§§§§   Classe UniformCache	§§§§§
//§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§
class UniformCache
{
private:
	struct ProgramData
	{
		struct UniformData
		{
			unsigned int uniformCRC32;
			int uniformPos;
		};
		unsigned int programID;
		HorseRadish::Containers::Array<UniformData> arrayUniforms;
	};
	HorseRadish::Containers::Array<ProgramData> arrayPrograms;

	static int comparePrograms(const ProgramData &objA, const ProgramData &objB);
	static int compareUniforms(const ProgramData::UniformData &objA, const ProgramData::UniformData &objB);

public:
	UniformCache();
	~UniformCache();

	bool AddUniform(const unsigned int programID, const char * const uniformName, const int uniformPos);
	int GetUniformPos(const unsigned int programID, const char * const uniformName, const bool create = true);
	void Clean();
};

//§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§
//§§§§§§   Classe ImmediateMode		§§§§§
//§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§
class ImmediateMode
{
public:
	enum GeometryType{
		None, Quads, Tris, Lines, LineStrip
		};
	enum InfoType{
		FreeVertexCount, MaxVertexCount
		};

private:
	struct VertexDataLayout
	{
		float px, py, pz, tu, tv;
		unsigned char cr, cg, cb, ca;
	};

	HorseRadish::OpenGL::GLuint glArrayBufferID, glElementArrayBufferID, glVertexArrayID;
	VertexDataLayout *bufferData;
	unsigned short *bufferIndices;
	int maxVertexCount, curVertex;
	float stateUV[2];
	GeometryType stateGeomType;
	unsigned char stateColor[4];

	int draw();
	void resetState();
	bool checkStateDraw() const;

public:
	ImmediateMode(const int maxVertexCount);
	~ImmediateMode();

	void BeginDraw(const GeometryType geometryType);
	void Draw();
	int EndDraw();

	void AddPosition(const float &x);
	void AddPosition(const float &x, const float &y);
	void AddPosition(const float &x, const float &y, const float &z);
	void AddTexCoord(const float &u, const float &v);
	void AddColor(const unsigned char &r, const unsigned char &g, const unsigned char &b);
	void AddColor(const unsigned char &r, const unsigned char &g, const unsigned char &b, const unsigned char &a);
	void AddColorF(const float &rgb);
	void AddColorF(const float &rgb, const float &a);
	void AddColorF(const float &r, const float &g, const float &b);
	void AddColorF(const float &r, const float &g, const float &b, const float &a);
	void AddColorRGB(const unsigned char * const values);
	void AddColorRGB(const float * const values);
	void AddQuad(const float &x, const float &y, const float &width, const float &height);
	void AddQuadTexCoords(const float &x, const float &y, const float &width, const float &height, const bool &normalizedTexCoords);
	void AddLine(const float &x1, const float &y1, const float &x2, const float &y2);
	void AddLineH(const float &x1, const float &x2, const float &y);
	void AddLineV(const float &x, const float &y1, const float &y2);

	unsigned int GetInfo(const InfoType infoType) const;
};

/*§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§
§§§§§§   -= Classe FontManager =-   §§§§§
§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§*/
class FontManager
{
private:
	HorseRadish::Containers::Array<Font*> fonts;
	HorseRadish::OpenGL::GLuint glArrayBufferID, glElementArrayBufferID, glVertexArrayID;

	friend class Font;

public:
	FontManager();
	~FontManager();

	Font* CreateFont(const int fontSize, const char * const fontFilePath, unsigned int glProgramID);
};

/*§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§
§§§§§§   -= Classe Font =-   §§§§§
§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§*/
class Font
{
private:
	struct VertexDataLayout
	{
		float px, py, tu, tv;
		unsigned char rgba[4];
	};
	
	struct KerningData
	{
		unsigned short char1, char2;
		short offset;

		KerningData()
		{
			this->char1 = this->char2 = 0;
			this->offset = 0;
		}
	};

	struct CharacterData
	{
		unsigned short unicodeID;
		unsigned short minUV[2], maxUV[2];
		short offset[2];
		short advance;
		const KerningData *kernData;

		CharacterData()
		{
			this->unicodeID = 0;
			this->minUV[0] = this->minUV[1] = 0;
			this->maxUV[0] = this->maxUV[1] = 0;
			this->offset[0] = this->offset[1] = 0;
			this->advance = 0;
			this->kernData = nullptr;
		}
	};
	
private:
	HorseRadish::Containers::Array<CharacterData> charData;
	HorseRadish::Containers::Array<KerningData> kerningData;
	HorseRadish::Color stateColor;
	unsigned short maxHeight, baseHeight, fontSize;
	unsigned int glTexID, glSamplerID, glProgID, glUniformSampler, glUniformMatrix;
	bool activo, paintStarted;
	FontManager *fontManager;

	mutable VertexDataLayout *fontDataArray;
	mutable unsigned int numCharWritten;

	static unsigned short primitiveRestartIndex;
	static int numMaxChar;

	static int sortCharData(const CharacterData &objA, const CharacterData &objB);
	static int findCharData(const CharacterData &objA, const CharacterData &objB);

	Font(FontManager * const fontManager, const int fontSize, const char * const fontFilePath, unsigned int glProgramID);
	~Font();

	void commitGL() const;
	bool createCharData();
	void cleanCharData();
	const CharacterData* getCharData(int unicodeID) const;
	const int getCharKerning(const CharacterData * const leftCharData, unsigned short rightCharUnicodeID) const;
	bool initFont(const char * const fontFilePath);
	void internalWrite(const float &px, const float &py, HorseRadish::String::Iterator &strIt) const;

	friend class FontManager;

public:
	void write(const float &px, const float &py, HorseRadish::String::Iterator &iterator) const;
	void write(HorseRadish::String::Iterator &iterator) const;
	void write(const float &px, const float &py, const HorseRadish::String &texto) const;
	void write(const HorseRadish::String &texto) const;
	int  writeChar(const unsigned int &unicodeChar) const;
	int	 writeChar(const float &px, const float &py, const unsigned int &unicodeChar) const;

	void draw() const;

	void setColor(const float &r, const float &g, const float &b, const float &a);
	void setColor(const float &r, const float &g, const float &b);
	void setColor(const float * const novaCor);

	bool getOperacional() const { return activo; }
	int	 getMaxHeight() const { return maxHeight; }
	int	 getCharWidth(const unsigned int &unicodeChar) const;
	int	 getStringWidth(const HorseRadish::String &texto) const;
	int	 getStringWidth(const HorseRadish::String &texto, const unsigned int numCharMeta) const;

	void paintBegin(const float * const tranformationMatrix);
	void paintEnd();
};

//§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§
//§§§§§§   Classe Viewport	§§§§§
//§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§
class Viewport
{

public:
	enum ProjectionType { Proj3D, Proj3DInf, Proj2D };

private:
	float fov,zNear,zFar;
	int origin[2],dim[2];
	ProjectionType projType;
	HorseRadish::Matrix mp2D,mp3D,mpInfinite,mCurrent;

	void clearAll();
	void setMatrices();

	Viewport(void);
public:
	Viewport(const Viewport &viewport);
	explicit Viewport(const int startX, const int startY, const int width, const int height);
	explicit Viewport(const int width, const int height);
	~Viewport();

	void setProjection(const ProjectionType projType);
	void setViewport(const int width, const int height);
	void setFOV(const float fov);
	void setZRange(const float zNear, const float zFar);

	const float *getProjCurrent() const;
	const float *getProj2D() const;
	const float *getProj3D() const;
	const float *getProj3DInf() const;
	float getFOV() const;
	float getZNear() const;
	float getZFar() const;
	void getPointOnZNear(float * const vec) const;
	void getPointOnZNear(HorseRadish::Vector * const center) const;
	void getPointOnZNear(float * const x, float * const y, float * const z) const;
	int getWidth() const {return dim[0];}
	int getHeight() const {return dim[1];}
	int getOriginX() const {return origin[0];}
	int getOriginY() const {return origin[1];}

	void projectPoint(const float * const modelView, HorseRadish::Vector * const listPoints, const int numPoints) const;
	void projectPoint(const float * const modelView, const float * const auxMat, HorseRadish::Vector * const listPoints, const int numPoints) const;
	void updateGL() const;
};

//§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§
//§§§§§§   Classe Frustum	§§§§§
//§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§
class Frustum
{
	HorseRadish::Plane planes[6];
	HorseRadish::Vector position;
	float zNear,zFar;

	void extractPlanes(const HorseRadish::Vector4 &col1, const HorseRadish::Vector4 &col2, const HorseRadish::Vector4 &col3, const HorseRadish::Vector4 &col4);
	bool sweptSpherePlaneIntersect(float &t0, float &t1, const HorseRadish::Plane &plane, const HorseRadish::Vector &sphereCenter, const float &sphereRadius, const HorseRadish::Vector &sweepDir) const;

public:
	enum IntersectionType { FullInside, FullOutside, FrustumIntersect };
	enum PlaneIndex { PlaneLeft = 0, PlaneRight = 1, PlaneTop = 2, PlaneBottom = 3, PlaneNear = 4, PlaneFar = 5 };

	Frustum() {zNear=zFar=0.0f;}
	~Frustum() {;}

	Frustum& operator=(const Frustum& frustum);

	bool testCube(const HorseRadish::Vector &ponto, const float &size) const;
	bool testBox(const HorseRadish::Vector &min, const HorseRadish::Vector &max) const;
	bool testBox(const HorseRadish::BBox &bbox) const;
	bool testSphere(const HorseRadish::Vector &ponto, const float &raio) const;
	bool testSphere(const HorseRadish::BSphere &bsphere) const;
	bool testSphereBox(const HorseRadish::BSphere &bsphere, const HorseRadish::BBox &bbox) const;
	bool testPoint(const HorseRadish::Vector &ponto) const;
	bool testPolygon(const HorseRadish::Vector * const pontos, const int numPoints) const;
	bool testSquare(const HorseRadish::Vector * const pontos) const;
	bool testTri(const HorseRadish::Vector * const pontos) const;
	bool testSweptSphere(const HorseRadish::Vector &sphereCenter, const float &sphereRadius, const HorseRadish::Vector &sweepDir) const;

	float getZNear(void) const {return zNear;}
	float getZFar(void) const {return zFar;}
	const HorseRadish::Vector& getCamPosition(void) const {return position;}
	const HorseRadish::Plane& getPlane(const PlaneIndex planeIndex) const {return planes[planeIndex];}
	void getCorners(HorseRadish::Vector * const points) const;

	IntersectionType ClassifyBox(const HorseRadish::BBox &bbox) const;
	IntersectionType ClassifyFrustum(const Frustum &frustum) const;

	float DotNormals(const PlaneIndex planeA, const PlaneIndex planeB) const;

	void CalculateFrustum(const float * const transformation);
	void CalculateFrustum(const float * const matProjection, const float * const matModelview);
	void CalculateFrustum(const float * const modelView, const float * const projectionMatrix, const HorseRadish::Vector &pos, const float zNear, const float zFar);
	
	void SetIndividualPlane(const PlaneIndex planeIndex, const HorseRadish::Plane &plane);
	void SetFrustum(const Frustum *const frustum);
	void SetFrustum(const HorseRadish::Vector &bboxMin, const HorseRadish::Vector &bboxMax);
	void SetFrustum(const HorseRadish::Vector &center, const float radius);
	void SetFrustum(const HorseRadish::BBox &bbox);
	void SetZNear(const float ZNear) {zNear=ZNear;}
	void SetZFar(const float ZFar) {zFar=ZFar;}
	void SetCamPosition(const float x, const float y, const float z) { position.Set(x,y,z); }
	void SetCamPosition(const HorseRadish::Vector &pos) { position.Set(pos); }
};

/*§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§
§§§§§§   -= Classe Camera =-   §§§§§
§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§*/
class Camera
{

public:
	enum CameraType { FirstPerson, OnSphere };
	enum CameraInput { Keyboard, Mouse };
	enum CameraComponent { Position = (1 << 0), Target = (1 << 1) };
	enum CameraAction { None = 0, Forward = (1 << 0), Backward = (1 << 1), StrifeLeft = (1 << 2), StrifeRight = (1 << 3), Up = (1 << 4), Down = (1 << 5), Run = (1 << 6) };

private:
	HorseRadish::Vector camPos,camDir,camUp;
	float rato[3][2],sumRato[2],ratoS,keyS,absFocus,onSphereMaxDist,onSphereMinDist;
	HorseRadish::Matrix modelView;
	HorseRadish::Vector *pointsPos,*pointsTarget;
	int numPos,numTarget;
	CameraType targetMode;

	void commitFirstPerson(const CameraAction &actionBitfield, const float &mouseDeltaX, const float &mouseDeltaY, const bool updatePosition, const float timeDeltaS);
	void commitOnSphere(const CameraAction &actionBitfield, const float &mouseDeltaX, const float &mouseDeltaY, const float timeDeltaS);

public:
	Camera();
	~Camera();

	void CommitInput(const CameraAction &actionBitfield, const float &mouseDeltaX, const float &mouseDeltaY, const bool updatePosition, const float timeDeltaS);
	void CommitCatmullRom(const CameraComponent &component, const float &normalizedTime);
	void CommitHermite(const CameraComponent &component, const float &normalizedTime);

	void PathClear(const CameraComponent &componentsBitField);
	void PathAdd(const CameraComponent &component, const float x, const float y, const float z);
	void PathAdd(const CameraComponent &component, const HorseRadish::Vector &vec);
	
	bool SetCamType(const CameraType &type);
	void SetSensitivity(const CameraInput &input, const float s);
	void SetPos(const HorseRadish::Vector &pos);
	void SetPos(const float x, const float y, const float z);
	void SetTarget(const HorseRadish::Vector &target);
	void SetTarget(const float x, const float y, const float z);
	void SetDir(const HorseRadish::Vector &direction);
	void SetDir(const float x, const float y, const float z);
	void SetAbsoluteFocus(const float focus);
	void SetOnSphereDists(const float minDist, const float maxDist);

	void GetPos(HorseRadish::Vector &pos) const;
	HorseRadish::Vector GetPos() const;
	void GetTarget(HorseRadish::Vector &target) const;
	HorseRadish::Vector GetTarget() const;
	void GetViewDir(HorseRadish::Vector &dir) const;
	HorseRadish::Vector GetViewDir() const;
	void GetStrideDir(HorseRadish::Vector &dir) const;
	HorseRadish::Vector GetStrideDir() const;

	float GetFocalDist() const;
	float GetSensitivity(const CameraInput &input) const;
	void GetRay(HorseRadish::Ray &ray) const;
	Camera::CameraType GetTargetMode() const;

	const float* GetModelView() const;
	void GetModelView(float * const mat) const;
	void GetFrustumCorners() const;
};

}//namespace Tools
}//namespace OpenGL
}//namespace HorseRadish

#endif