#pragma once

#include "openGL.hpp"
#include "common\Platform.hpp"

#include "glext.h"

namespace HorseRadish { namespace OpenGL { namespace Extensions {

	void ExtensionsLoad(const char* const openGLModuleName);
	bool ExtensionExists(const char * const extension);

#ifdef GL_EXT_direct_state_access
	GLAPI void (APIENTRYP glMatrixLoadfEXT) (GLenum mode, const GLfloat *m);
	GLAPI void (APIENTRYP glMatrixLoaddEXT) (GLenum mode, const GLdouble *m);
	GLAPI void (APIENTRYP glMatrixMultfEXT) (GLenum mode, const GLfloat *m);
	GLAPI void (APIENTRYP glMatrixMultdEXT) (GLenum mode, const GLdouble *m);
	GLAPI void (APIENTRYP glMatrixLoadIdentityEXT) (GLenum mode);
	GLAPI void (APIENTRYP glMatrixRotatefEXT) (GLenum mode, GLfloat angle, GLfloat x, GLfloat y, GLfloat z);
	GLAPI void (APIENTRYP glMatrixRotatedEXT) (GLenum mode, GLdouble angle, GLdouble x, GLdouble y, GLdouble z);
	GLAPI void (APIENTRYP glMatrixScalefEXT) (GLenum mode, GLfloat x, GLfloat y, GLfloat z);
	GLAPI void (APIENTRYP glMatrixScaledEXT) (GLenum mode, GLdouble x, GLdouble y, GLdouble z);
	GLAPI void (APIENTRYP glMatrixTranslatefEXT) (GLenum mode, GLfloat x, GLfloat y, GLfloat z);
	GLAPI void (APIENTRYP glMatrixTranslatedEXT) (GLenum mode, GLdouble x, GLdouble y, GLdouble z);
	GLAPI void (APIENTRYP glMatrixFrustumEXT) (GLenum mode, GLdouble left, GLdouble right, GLdouble bottom, GLdouble top, GLdouble zNear, GLdouble zFar);
	GLAPI void (APIENTRYP glMatrixOrthoEXT) (GLenum mode, GLdouble left, GLdouble right, GLdouble bottom, GLdouble top, GLdouble zNear, GLdouble zFar);
	GLAPI void (APIENTRYP glMatrixPopEXT) (GLenum mode);
	GLAPI void (APIENTRYP glMatrixPushEXT) (GLenum mode);
	GLAPI void (APIENTRYP glMatrixLoadTransposefEXT) (GLenum mode, const GLfloat *m);
	GLAPI void (APIENTRYP glMatrixLoadTransposedEXT) (GLenum mode, const GLdouble *m);
	GLAPI void (APIENTRYP glMatrixMultTransposefEXT) (GLenum mode, const GLfloat *m);
	GLAPI void (APIENTRYP glMatrixMultTransposedEXT) (GLenum mode, const GLdouble *m);
#endif

#ifdef GL_NV_path_rendering
	GLAPI GLuint (APIENTRYP glGenPathsNV) (GLsizei range);
	GLAPI void (APIENTRYP glDeletePathsNV) (GLuint path, GLsizei range);
	GLAPI GLboolean (APIENTRYP glIsPathNV) (GLuint path);
	GLAPI void (APIENTRYP glPathCommandsNV) (GLuint path, GLsizei numCommands, const GLubyte *commands, GLsizei numCoords, GLenum coordType, const void *coords);
	GLAPI void (APIENTRYP glPathCoordsNV) (GLuint path, GLsizei numCoords, GLenum coordType, const void *coords);
	GLAPI void (APIENTRYP glPathSubCommandsNV) (GLuint path, GLsizei commandStart, GLsizei commandsToDelete, GLsizei numCommands, const GLubyte *commands, GLsizei numCoords, GLenum coordType, const void *coords);
	GLAPI void (APIENTRYP glPathSubCoordsNV) (GLuint path, GLsizei coordStart, GLsizei numCoords, GLenum coordType, const void *coords);
	GLAPI void (APIENTRYP glPathStringNV) (GLuint path, GLenum format, GLsizei length, const void *pathString);
	GLAPI void (APIENTRYP glPathGlyphsNV) (GLuint firstPathName, GLenum fontTarget, const void *fontName, GLbitfield fontStyle, GLsizei numGlyphs, GLenum type, const void *charcodes, GLenum handleMissingGlyphs, GLuint pathParameterTemplate, GLfloat emScale);
	GLAPI void (APIENTRYP glPathGlyphRangeNV) (GLuint firstPathName, GLenum fontTarget, const void *fontName, GLbitfield fontStyle, GLuint firstGlyph, GLsizei numGlyphs, GLenum handleMissingGlyphs, GLuint pathParameterTemplate, GLfloat emScale);
	GLAPI void (APIENTRYP glWeightPathsNV) (GLuint resultPath, GLsizei numPaths, const GLuint *paths, const GLfloat *weights);
	GLAPI void (APIENTRYP glCopyPathNV) (GLuint resultPath, GLuint srcPath);
	GLAPI void (APIENTRYP glInterpolatePathsNV) (GLuint resultPath, GLuint pathA, GLuint pathB, GLfloat weight);
	GLAPI void (APIENTRYP glTransformPathNV) (GLuint resultPath, GLuint srcPath, GLenum transformType, const GLfloat *transformValues);
	GLAPI void (APIENTRYP glPathParameterivNV) (GLuint path, GLenum pname, const GLint *value);
	GLAPI void (APIENTRYP glPathParameteriNV) (GLuint path, GLenum pname, GLint value);
	GLAPI void (APIENTRYP glPathParameterfvNV) (GLuint path, GLenum pname, const GLfloat *value);
	GLAPI void (APIENTRYP glPathParameterfNV) (GLuint path, GLenum pname, GLfloat value);
	GLAPI void (APIENTRYP glPathDashArrayNV) (GLuint path, GLsizei dashCount, const GLfloat *dashArray);
	GLAPI void (APIENTRYP glPathStencilFuncNV) (GLenum func, GLint ref, GLuint mask);
	GLAPI void (APIENTRYP glPathStencilDepthOffsetNV) (GLfloat factor, GLfloat units);
	GLAPI void (APIENTRYP glStencilFillPathNV) (GLuint path, GLenum fillMode, GLuint mask);
	GLAPI void (APIENTRYP glStencilStrokePathNV) (GLuint path, GLint reference, GLuint mask);
	GLAPI void (APIENTRYP glStencilFillPathInstancedNV) (GLsizei numPaths, GLenum pathNameType, const void *paths, GLuint pathBase, GLenum fillMode, GLuint mask, GLenum transformType, const GLfloat *transformValues);
	GLAPI void (APIENTRYP glStencilStrokePathInstancedNV) (GLsizei numPaths, GLenum pathNameType, const void *paths, GLuint pathBase, GLint reference, GLuint mask, GLenum transformType, const GLfloat *transformValues);
	GLAPI void (APIENTRYP glPathCoverDepthFuncNV) (GLenum func);
	GLAPI void (APIENTRYP glCoverFillPathNV) (GLuint path, GLenum coverMode);
	GLAPI void (APIENTRYP glCoverStrokePathNV) (GLuint path, GLenum coverMode);
	GLAPI void (APIENTRYP glCoverFillPathInstancedNV) (GLsizei numPaths, GLenum pathNameType, const void *paths, GLuint pathBase, GLenum coverMode, GLenum transformType, const GLfloat *transformValues);
	GLAPI void (APIENTRYP glCoverStrokePathInstancedNV) (GLsizei numPaths, GLenum pathNameType, const void *paths, GLuint pathBase, GLenum coverMode, GLenum transformType, const GLfloat *transformValues);
	GLAPI void (APIENTRYP glGetPathParameterivNV) (GLuint path, GLenum pname, GLint *value);
	GLAPI void (APIENTRYP glGetPathParameterfvNV) (GLuint path, GLenum pname, GLfloat *value);
	GLAPI void (APIENTRYP glGetPathCommandsNV) (GLuint path, GLubyte *commands);
	GLAPI void (APIENTRYP glGetPathCoordsNV) (GLuint path, GLfloat *coords);
	GLAPI void (APIENTRYP glGetPathDashArrayNV) (GLuint path, GLfloat *dashArray);
	GLAPI void (APIENTRYP glGetPathMetricsNV) (GLbitfield metricQueryMask, GLsizei numPaths, GLenum pathNameType, const void *paths, GLuint pathBase, GLsizei stride, GLfloat *metrics);
	GLAPI void (APIENTRYP glGetPathMetricRangeNV) (GLbitfield metricQueryMask, GLuint firstPathName, GLsizei numPaths, GLsizei stride, GLfloat *metrics);
	GLAPI void (APIENTRYP glGetPathSpacingNV) (GLenum pathListMode, GLsizei numPaths, GLenum pathNameType, const void *paths, GLuint pathBase, GLfloat advanceScale, GLfloat kerningScale, GLenum transformType, GLfloat *returnedSpacing);
	GLAPI GLboolean (APIENTRYP glIsPointInFillPathNV) (GLuint path, GLuint mask, GLfloat x, GLfloat y);
	GLAPI GLboolean (APIENTRYP glIsPointInStrokePathNV) (GLuint path, GLfloat x, GLfloat y);
	GLAPI GLfloat (APIENTRYP glGetPathLengthNV) (GLuint path, GLsizei startSegment, GLsizei numSegments);
	GLAPI GLboolean (APIENTRYP glPointAlongPathNV) (GLuint path, GLsizei startSegment, GLsizei numSegments, GLfloat distance, GLfloat *x, GLfloat *y, GLfloat *tangentX, GLfloat *tangentY);
	GLAPI void (APIENTRYP glMatrixLoad3x2fNV) (GLenum matrixMode, const GLfloat *m);
	GLAPI void (APIENTRYP glMatrixLoad3x3fNV) (GLenum matrixMode, const GLfloat *m);
	GLAPI void (APIENTRYP glMatrixLoadTranspose3x3fNV) (GLenum matrixMode, const GLfloat *m);
	GLAPI void (APIENTRYP glMatrixMult3x2fNV) (GLenum matrixMode, const GLfloat *m);
	GLAPI void (APIENTRYP glMatrixMult3x3fNV) (GLenum matrixMode, const GLfloat *m);
	GLAPI void (APIENTRYP glMatrixMultTranspose3x3fNV) (GLenum matrixMode, const GLfloat *m);
	GLAPI void (APIENTRYP glStencilThenCoverFillPathNV) (GLuint path, GLenum fillMode, GLuint mask, GLenum coverMode);
	GLAPI void (APIENTRYP glStencilThenCoverStrokePathNV) (GLuint path, GLint reference, GLuint mask, GLenum coverMode);
	GLAPI void (APIENTRYP glStencilThenCoverFillPathInstancedNV) (GLsizei numPaths, GLenum pathNameType, const void *paths, GLuint pathBase, GLenum fillMode, GLuint mask, GLenum coverMode, GLenum transformType, const GLfloat *transformValues);
	GLAPI void (APIENTRYP glStencilThenCoverStrokePathInstancedNV) (GLsizei numPaths, GLenum pathNameType, const void *paths, GLuint pathBase, GLint reference, GLuint mask, GLenum coverMode, GLenum transformType, const GLfloat *transformValues);
	GLAPI GLenum (APIENTRYP glPathGlyphIndexRangeNV) (GLenum fontTarget, const void *fontName, GLbitfield fontStyle, GLuint pathParameterTemplate, GLfloat emScale, GLuint baseAndCount[2]);
	GLAPI GLenum (APIENTRYP glPathGlyphIndexArrayNV) (GLuint firstPathName, GLenum fontTarget, const void *fontName, GLbitfield fontStyle, GLuint firstGlyphIndex, GLsizei numGlyphs, GLuint pathParameterTemplate, GLfloat emScale);
	GLAPI GLenum (APIENTRYP glPathMemoryGlyphIndexArrayNV) (GLuint firstPathName, GLenum fontTarget, GLsizeiptr fontSize, const void *fontData, GLsizei faceIndex, GLuint firstGlyphIndex, GLsizei numGlyphs, GLuint pathParameterTemplate, GLfloat emScale);
	GLAPI void (APIENTRYP glProgramPathFragmentInputGenNV) (GLuint program, GLint location, GLenum genMode, GLint components, const GLfloat *coeffs);
	GLAPI void (APIENTRYP glGetProgramResourcefvNV) (GLuint program, GLenum programInterface, GLuint index, GLsizei propCount, const GLenum *props, GLsizei bufSize, GLsizei *length, GLfloat *params);
#endif

} }  }
