#pragma once

#include "common\Platform.hpp"

#include "glcorearb.h"

namespace HorseRadish
{
	namespace OpenGL
	{
		bool OpenGLLoadLibrary(const char* const glLibName);
		bool OpenGLUnloadLibrary(void);
		bool OpenGLGetProcs(void);

#ifdef GL_VERSION_1_0
		GLAPI void (APIENTRYP glCullFace) (GLenum mode);
		GLAPI void (APIENTRYP glFrontFace) (GLenum mode);
		GLAPI void (APIENTRYP glHint) (GLenum target, GLenum mode);
		GLAPI void (APIENTRYP glLineWidth) (GLfloat width);
		GLAPI void (APIENTRYP glPointSize) (GLfloat size);
		GLAPI void (APIENTRYP glPolygonMode) (GLenum face, GLenum mode);
		GLAPI void (APIENTRYP glScissor) (GLint x, GLint y, GLsizei width, GLsizei height);
		GLAPI void (APIENTRYP glTexParameterf) (GLenum target, GLenum pname, GLfloat param);
		GLAPI void (APIENTRYP glTexParameterfv) (GLenum target, GLenum pname, const GLfloat *params);
		GLAPI void (APIENTRYP glTexParameteri) (GLenum target, GLenum pname, GLint param);
		GLAPI void (APIENTRYP glTexParameteriv) (GLenum target, GLenum pname, const GLint *params);
		GLAPI void (APIENTRYP glTexImage1D) (GLenum target, GLint level, GLint internalformat, GLsizei width, GLint border, GLenum format, GLenum type, const void *pixels);
		GLAPI void (APIENTRYP glTexImage2D) (GLenum target, GLint level, GLint internalformat, GLsizei width, GLsizei height, GLint border, GLenum format, GLenum type, const void *pixels);
		GLAPI void (APIENTRYP glDrawBuffer) (GLenum buf);
		GLAPI void (APIENTRYP glClear) (GLbitfield mask);
		GLAPI void (APIENTRYP glClearColor) (GLfloat red, GLfloat green, GLfloat blue, GLfloat alpha);
		GLAPI void (APIENTRYP glClearStencil) (GLint s);
		GLAPI void (APIENTRYP glClearDepth) (GLdouble depth);
		GLAPI void (APIENTRYP glStencilMask) (GLuint mask);
		GLAPI void (APIENTRYP glColorMask) (GLboolean red, GLboolean green, GLboolean blue, GLboolean alpha);
		GLAPI void (APIENTRYP glDepthMask) (GLboolean flag);
		GLAPI void (APIENTRYP glDisable) (GLenum cap);
		GLAPI void (APIENTRYP glEnable) (GLenum cap);
		GLAPI void (APIENTRYP glFinish) (void);
		GLAPI void (APIENTRYP glFlush) (void);
		GLAPI void (APIENTRYP glBlendFunc) (GLenum sfactor, GLenum dfactor);
		GLAPI void (APIENTRYP glLogicOp) (GLenum opcode);
		GLAPI void (APIENTRYP glStencilFunc) (GLenum func, GLint ref, GLuint mask);
		GLAPI void (APIENTRYP glStencilOp) (GLenum fail, GLenum zfail, GLenum zpass);
		GLAPI void (APIENTRYP glDepthFunc) (GLenum func);
		GLAPI void (APIENTRYP glPixelStoref) (GLenum pname, GLfloat param);
		GLAPI void (APIENTRYP glPixelStorei) (GLenum pname, GLint param);
		GLAPI void (APIENTRYP glReadBuffer) (GLenum src);
		GLAPI void (APIENTRYP glReadPixels) (GLint x, GLint y, GLsizei width, GLsizei height, GLenum format, GLenum type, void *pixels);
		GLAPI void (APIENTRYP glGetBooleanv) (GLenum pname, GLboolean *data);
		GLAPI void (APIENTRYP glGetDoublev) (GLenum pname, GLdouble *data);
		GLAPI GLenum (APIENTRYP glGetError) (void);
		GLAPI void (APIENTRYP glGetFloatv) (GLenum pname, GLfloat *data);
		GLAPI void (APIENTRYP glGetIntegerv) (GLenum pname, GLint *data);
		GLAPI const GLubyte *(APIENTRYP glGetString) (GLenum name);
		GLAPI void (APIENTRYP glGetTexImage) (GLenum target, GLint level, GLenum format, GLenum type, void *pixels);
		GLAPI void (APIENTRYP glGetTexParameterfv) (GLenum target, GLenum pname, GLfloat *params);
		GLAPI void (APIENTRYP glGetTexParameteriv) (GLenum target, GLenum pname, GLint *params);
		GLAPI void (APIENTRYP glGetTexLevelParameterfv) (GLenum target, GLint level, GLenum pname, GLfloat *params);
		GLAPI void (APIENTRYP glGetTexLevelParameteriv) (GLenum target, GLint level, GLenum pname, GLint *params);
		GLAPI GLboolean (APIENTRYP glIsEnabled) (GLenum cap);
		GLAPI void (APIENTRYP glDepthRange) (GLdouble near, GLdouble far);
		GLAPI void (APIENTRYP glViewport) (GLint x, GLint y, GLsizei width, GLsizei height);
#endif

#ifdef GL_VERSION_1_1
		GLAPI void (APIENTRYP glDrawArrays) (GLenum mode, GLint first, GLsizei count);
		GLAPI void (APIENTRYP glDrawElements) (GLenum mode, GLsizei count, GLenum type, const void *indices);
		GLAPI void (APIENTRYP glGetPointerv) (GLenum pname, void **params);
		GLAPI void (APIENTRYP glPolygonOffset) (GLfloat factor, GLfloat units);
		GLAPI void (APIENTRYP glCopyTexImage1D) (GLenum target, GLint level, GLenum internalformat, GLint x, GLint y, GLsizei width, GLint border);
		GLAPI void (APIENTRYP glCopyTexImage2D) (GLenum target, GLint level, GLenum internalformat, GLint x, GLint y, GLsizei width, GLsizei height, GLint border);
		GLAPI void (APIENTRYP glCopyTexSubImage1D) (GLenum target, GLint level, GLint xoffset, GLint x, GLint y, GLsizei width);
		GLAPI void (APIENTRYP glCopyTexSubImage2D) (GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint x, GLint y, GLsizei width, GLsizei height);
		GLAPI void (APIENTRYP glTexSubImage1D) (GLenum target, GLint level, GLint xoffset, GLsizei width, GLenum format, GLenum type, const void *pixels);
		GLAPI void (APIENTRYP glTexSubImage2D) (GLenum target, GLint level, GLint xoffset, GLint yoffset, GLsizei width, GLsizei height, GLenum format, GLenum type, const void *pixels);
		GLAPI void (APIENTRYP glBindTexture) (GLenum target, GLuint texture);
		GLAPI void (APIENTRYP glDeleteTextures) (GLsizei n, const GLuint *textures);
		GLAPI void (APIENTRYP glGenTextures) (GLsizei n, GLuint *textures);
		GLAPI GLboolean (APIENTRYP glIsTexture) (GLuint texture);
#endif

#ifdef GL_VERSION_1_2
		GLAPI void (APIENTRYP glDrawRangeElements) (GLenum mode, GLuint start, GLuint end, GLsizei count, GLenum type, const void *indices);
		GLAPI void (APIENTRYP glTexImage3D) (GLenum target, GLint level, GLint internalformat, GLsizei width, GLsizei height, GLsizei depth, GLint border, GLenum format, GLenum type, const void *pixels);
		GLAPI void (APIENTRYP glTexSubImage3D) (GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLsizei width, GLsizei height, GLsizei depth, GLenum format, GLenum type, const void *pixels);
		GLAPI void (APIENTRYP glCopyTexSubImage3D) (GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLint x, GLint y, GLsizei width, GLsizei height);
#endif

#ifdef GL_VERSION_1_3
		GLAPI void (APIENTRYP glActiveTexture) (GLenum texture);
		GLAPI void (APIENTRYP glSampleCoverage) (GLfloat value, GLboolean invert);
		GLAPI void (APIENTRYP glCompressedTexImage3D) (GLenum target, GLint level, GLenum internalformat, GLsizei width, GLsizei height, GLsizei depth, GLint border, GLsizei imageSize, const void *data);
		GLAPI void (APIENTRYP glCompressedTexImage2D) (GLenum target, GLint level, GLenum internalformat, GLsizei width, GLsizei height, GLint border, GLsizei imageSize, const void *data);
		GLAPI void (APIENTRYP glCompressedTexImage1D) (GLenum target, GLint level, GLenum internalformat, GLsizei width, GLint border, GLsizei imageSize, const void *data);
		GLAPI void (APIENTRYP glCompressedTexSubImage3D) (GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLsizei width, GLsizei height, GLsizei depth, GLenum format, GLsizei imageSize, const void *data);
		GLAPI void (APIENTRYP glCompressedTexSubImage2D) (GLenum target, GLint level, GLint xoffset, GLint yoffset, GLsizei width, GLsizei height, GLenum format, GLsizei imageSize, const void *data);
		GLAPI void (APIENTRYP glCompressedTexSubImage1D) (GLenum target, GLint level, GLint xoffset, GLsizei width, GLenum format, GLsizei imageSize, const void *data);
		GLAPI void (APIENTRYP glGetCompressedTexImage) (GLenum target, GLint level, void *img);
#endif

#ifdef GL_VERSION_1_4
		GLAPI void (APIENTRYP glBlendFuncSeparate) (GLenum sfactorRGB, GLenum dfactorRGB, GLenum sfactorAlpha, GLenum dfactorAlpha);
		GLAPI void (APIENTRYP glMultiDrawArrays) (GLenum mode, const GLint *first, const GLsizei *count, GLsizei drawcount);
		GLAPI void (APIENTRYP glMultiDrawElements) (GLenum mode, const GLsizei *count, GLenum type, const void *const*indices, GLsizei drawcount);
		GLAPI void (APIENTRYP glPointParameterf) (GLenum pname, GLfloat param);
		GLAPI void (APIENTRYP glPointParameterfv) (GLenum pname, const GLfloat *params);
		GLAPI void (APIENTRYP glPointParameteri) (GLenum pname, GLint param);
		GLAPI void (APIENTRYP glPointParameteriv) (GLenum pname, const GLint *params);
		GLAPI void (APIENTRYP glBlendColor) (GLfloat red, GLfloat green, GLfloat blue, GLfloat alpha);
		GLAPI void (APIENTRYP glBlendEquation) (GLenum mode);
#endif

#ifdef GL_VERSION_1_5
		GLAPI void (APIENTRYP glGenQueries) (GLsizei n, GLuint *ids);
		GLAPI void (APIENTRYP glDeleteQueries) (GLsizei n, const GLuint *ids);
		GLAPI GLboolean (APIENTRYP glIsQuery) (GLuint id);
		GLAPI void (APIENTRYP glBeginQuery) (GLenum target, GLuint id);
		GLAPI void (APIENTRYP glEndQuery) (GLenum target);
		GLAPI void (APIENTRYP glGetQueryiv) (GLenum target, GLenum pname, GLint *params);
		GLAPI void (APIENTRYP glGetQueryObjectiv) (GLuint id, GLenum pname, GLint *params);
		GLAPI void (APIENTRYP glGetQueryObjectuiv) (GLuint id, GLenum pname, GLuint *params);
		GLAPI void (APIENTRYP glBindBuffer) (GLenum target, GLuint buffer);
		GLAPI void (APIENTRYP glDeleteBuffers) (GLsizei n, const GLuint *buffers);
		GLAPI void (APIENTRYP glGenBuffers) (GLsizei n, GLuint *buffers);
		GLAPI GLboolean (APIENTRYP glIsBuffer) (GLuint buffer);
		GLAPI void (APIENTRYP glBufferData) (GLenum target, GLsizeiptr size, const void *data, GLenum usage);
		GLAPI void (APIENTRYP glBufferSubData) (GLenum target, GLintptr offset, GLsizeiptr size, const void *data);
		GLAPI void (APIENTRYP glGetBufferSubData) (GLenum target, GLintptr offset, GLsizeiptr size, void *data);
		GLAPI void *(APIENTRYP glMapBuffer) (GLenum target, GLenum access);
		GLAPI GLboolean (APIENTRYP glUnmapBuffer) (GLenum target);
		GLAPI void (APIENTRYP glGetBufferParameteriv) (GLenum target, GLenum pname, GLint *params);
		GLAPI void (APIENTRYP glGetBufferPointerv) (GLenum target, GLenum pname, void **params);
#endif

#ifdef GL_VERSION_2_0
		GLAPI void (APIENTRYP glBlendEquationSeparate) (GLenum modeRGB, GLenum modeAlpha);
		GLAPI void (APIENTRYP glDrawBuffers) (GLsizei n, const GLenum *bufs);
		GLAPI void (APIENTRYP glStencilOpSeparate) (GLenum face, GLenum sfail, GLenum dpfail, GLenum dppass);
		GLAPI void (APIENTRYP glStencilFuncSeparate) (GLenum face, GLenum func, GLint ref, GLuint mask);
		GLAPI void (APIENTRYP glStencilMaskSeparate) (GLenum face, GLuint mask);
		GLAPI void (APIENTRYP glAttachShader) (GLuint program, GLuint shader);
		GLAPI void (APIENTRYP glBindAttribLocation) (GLuint program, GLuint index, const GLchar *name);
		GLAPI void (APIENTRYP glCompileShader) (GLuint shader);
		GLAPI GLuint (APIENTRYP glCreateProgram) (void);
		GLAPI GLuint (APIENTRYP glCreateShader) (GLenum type);
		GLAPI void (APIENTRYP glDeleteProgram) (GLuint program);
		GLAPI void (APIENTRYP glDeleteShader) (GLuint shader);
		GLAPI void (APIENTRYP glDetachShader) (GLuint program, GLuint shader);
		GLAPI void (APIENTRYP glDisableVertexAttribArray) (GLuint index);
		GLAPI void (APIENTRYP glEnableVertexAttribArray) (GLuint index);
		GLAPI void (APIENTRYP glGetActiveAttrib) (GLuint program, GLuint index, GLsizei bufSize, GLsizei *length, GLint *size, GLenum *type, GLchar *name);
		GLAPI void (APIENTRYP glGetActiveUniform) (GLuint program, GLuint index, GLsizei bufSize, GLsizei *length, GLint *size, GLenum *type, GLchar *name);
		GLAPI void (APIENTRYP glGetAttachedShaders) (GLuint program, GLsizei maxCount, GLsizei *count, GLuint *shaders);
		GLAPI GLint (APIENTRYP glGetAttribLocation) (GLuint program, const GLchar *name);
		GLAPI void (APIENTRYP glGetProgramiv) (GLuint program, GLenum pname, GLint *params);
		GLAPI void (APIENTRYP glGetProgramInfoLog) (GLuint program, GLsizei bufSize, GLsizei *length, GLchar *infoLog);
		GLAPI void (APIENTRYP glGetShaderiv) (GLuint shader, GLenum pname, GLint *params);
		GLAPI void (APIENTRYP glGetShaderInfoLog) (GLuint shader, GLsizei bufSize, GLsizei *length, GLchar *infoLog);
		GLAPI void (APIENTRYP glGetShaderSource) (GLuint shader, GLsizei bufSize, GLsizei *length, GLchar *source);
		GLAPI GLint (APIENTRYP glGetUniformLocation) (GLuint program, const GLchar *name);
		GLAPI void (APIENTRYP glGetUniformfv) (GLuint program, GLint location, GLfloat *params);
		GLAPI void (APIENTRYP glGetUniformiv) (GLuint program, GLint location, GLint *params);
		GLAPI void (APIENTRYP glGetVertexAttribdv) (GLuint index, GLenum pname, GLdouble *params);
		GLAPI void (APIENTRYP glGetVertexAttribfv) (GLuint index, GLenum pname, GLfloat *params);
		GLAPI void (APIENTRYP glGetVertexAttribiv) (GLuint index, GLenum pname, GLint *params);
		GLAPI void (APIENTRYP glGetVertexAttribPointerv) (GLuint index, GLenum pname, void **pointer);
		GLAPI GLboolean (APIENTRYP glIsProgram) (GLuint program);
		GLAPI GLboolean (APIENTRYP glIsShader) (GLuint shader);
		GLAPI void (APIENTRYP glLinkProgram) (GLuint program);
		GLAPI void (APIENTRYP glShaderSource) (GLuint shader, GLsizei count, const GLchar *const*string, const GLint *length);
		GLAPI void (APIENTRYP glUseProgram) (GLuint program);
		GLAPI void (APIENTRYP glUniform1f) (GLint location, GLfloat v0);
		GLAPI void (APIENTRYP glUniform2f) (GLint location, GLfloat v0, GLfloat v1);
		GLAPI void (APIENTRYP glUniform3f) (GLint location, GLfloat v0, GLfloat v1, GLfloat v2);
		GLAPI void (APIENTRYP glUniform4f) (GLint location, GLfloat v0, GLfloat v1, GLfloat v2, GLfloat v3);
		GLAPI void (APIENTRYP glUniform1i) (GLint location, GLint v0);
		GLAPI void (APIENTRYP glUniform2i) (GLint location, GLint v0, GLint v1);
		GLAPI void (APIENTRYP glUniform3i) (GLint location, GLint v0, GLint v1, GLint v2);
		GLAPI void (APIENTRYP glUniform4i) (GLint location, GLint v0, GLint v1, GLint v2, GLint v3);
		GLAPI void (APIENTRYP glUniform1fv) (GLint location, GLsizei count, const GLfloat *value);
		GLAPI void (APIENTRYP glUniform2fv) (GLint location, GLsizei count, const GLfloat *value);
		GLAPI void (APIENTRYP glUniform3fv) (GLint location, GLsizei count, const GLfloat *value);
		GLAPI void (APIENTRYP glUniform4fv) (GLint location, GLsizei count, const GLfloat *value);
		GLAPI void (APIENTRYP glUniform1iv) (GLint location, GLsizei count, const GLint *value);
		GLAPI void (APIENTRYP glUniform2iv) (GLint location, GLsizei count, const GLint *value);
		GLAPI void (APIENTRYP glUniform3iv) (GLint location, GLsizei count, const GLint *value);
		GLAPI void (APIENTRYP glUniform4iv) (GLint location, GLsizei count, const GLint *value);
		GLAPI void (APIENTRYP glUniformMatrix2fv) (GLint location, GLsizei count, GLboolean transpose, const GLfloat *value);
		GLAPI void (APIENTRYP glUniformMatrix3fv) (GLint location, GLsizei count, GLboolean transpose, const GLfloat *value);
		GLAPI void (APIENTRYP glUniformMatrix4fv) (GLint location, GLsizei count, GLboolean transpose, const GLfloat *value);
		GLAPI void (APIENTRYP glValidateProgram) (GLuint program);
		GLAPI void (APIENTRYP glVertexAttrib1d) (GLuint index, GLdouble x);
		GLAPI void (APIENTRYP glVertexAttrib1dv) (GLuint index, const GLdouble *v);
		GLAPI void (APIENTRYP glVertexAttrib1f) (GLuint index, GLfloat x);
		GLAPI void (APIENTRYP glVertexAttrib1fv) (GLuint index, const GLfloat *v);
		GLAPI void (APIENTRYP glVertexAttrib1s) (GLuint index, GLshort x);
		GLAPI void (APIENTRYP glVertexAttrib1sv) (GLuint index, const GLshort *v);
		GLAPI void (APIENTRYP glVertexAttrib2d) (GLuint index, GLdouble x, GLdouble y);
		GLAPI void (APIENTRYP glVertexAttrib2dv) (GLuint index, const GLdouble *v);
		GLAPI void (APIENTRYP glVertexAttrib2f) (GLuint index, GLfloat x, GLfloat y);
		GLAPI void (APIENTRYP glVertexAttrib2fv) (GLuint index, const GLfloat *v);
		GLAPI void (APIENTRYP glVertexAttrib2s) (GLuint index, GLshort x, GLshort y);
		GLAPI void (APIENTRYP glVertexAttrib2sv) (GLuint index, const GLshort *v);
		GLAPI void (APIENTRYP glVertexAttrib3d) (GLuint index, GLdouble x, GLdouble y, GLdouble z);
		GLAPI void (APIENTRYP glVertexAttrib3dv) (GLuint index, const GLdouble *v);
		GLAPI void (APIENTRYP glVertexAttrib3f) (GLuint index, GLfloat x, GLfloat y, GLfloat z);
		GLAPI void (APIENTRYP glVertexAttrib3fv) (GLuint index, const GLfloat *v);
		GLAPI void (APIENTRYP glVertexAttrib3s) (GLuint index, GLshort x, GLshort y, GLshort z);
		GLAPI void (APIENTRYP glVertexAttrib3sv) (GLuint index, const GLshort *v);
		GLAPI void (APIENTRYP glVertexAttrib4Nbv) (GLuint index, const GLbyte *v);
		GLAPI void (APIENTRYP glVertexAttrib4Niv) (GLuint index, const GLint *v);
		GLAPI void (APIENTRYP glVertexAttrib4Nsv) (GLuint index, const GLshort *v);
		GLAPI void (APIENTRYP glVertexAttrib4Nub) (GLuint index, GLubyte x, GLubyte y, GLubyte z, GLubyte w);
		GLAPI void (APIENTRYP glVertexAttrib4Nubv) (GLuint index, const GLubyte *v);
		GLAPI void (APIENTRYP glVertexAttrib4Nuiv) (GLuint index, const GLuint *v);
		GLAPI void (APIENTRYP glVertexAttrib4Nusv) (GLuint index, const GLushort *v);
		GLAPI void (APIENTRYP glVertexAttrib4bv) (GLuint index, const GLbyte *v);
		GLAPI void (APIENTRYP glVertexAttrib4d) (GLuint index, GLdouble x, GLdouble y, GLdouble z, GLdouble w);
		GLAPI void (APIENTRYP glVertexAttrib4dv) (GLuint index, const GLdouble *v);
		GLAPI void (APIENTRYP glVertexAttrib4f) (GLuint index, GLfloat x, GLfloat y, GLfloat z, GLfloat w);
		GLAPI void (APIENTRYP glVertexAttrib4fv) (GLuint index, const GLfloat *v);
		GLAPI void (APIENTRYP glVertexAttrib4iv) (GLuint index, const GLint *v);
		GLAPI void (APIENTRYP glVertexAttrib4s) (GLuint index, GLshort x, GLshort y, GLshort z, GLshort w);
		GLAPI void (APIENTRYP glVertexAttrib4sv) (GLuint index, const GLshort *v);
		GLAPI void (APIENTRYP glVertexAttrib4ubv) (GLuint index, const GLubyte *v);
		GLAPI void (APIENTRYP glVertexAttrib4uiv) (GLuint index, const GLuint *v);
		GLAPI void (APIENTRYP glVertexAttrib4usv) (GLuint index, const GLushort *v);
		GLAPI void (APIENTRYP glVertexAttribPointer) (GLuint index, GLint size, GLenum type, GLboolean normalized, GLsizei stride, const void *pointer);
#endif

#ifdef GL_VERSION_2_1
		GLAPI void (APIENTRYP glUniformMatrix2x3fv) (GLint location, GLsizei count, GLboolean transpose, const GLfloat *value);
		GLAPI void (APIENTRYP glUniformMatrix3x2fv) (GLint location, GLsizei count, GLboolean transpose, const GLfloat *value);
		GLAPI void (APIENTRYP glUniformMatrix2x4fv) (GLint location, GLsizei count, GLboolean transpose, const GLfloat *value);
		GLAPI void (APIENTRYP glUniformMatrix4x2fv) (GLint location, GLsizei count, GLboolean transpose, const GLfloat *value);
		GLAPI void (APIENTRYP glUniformMatrix3x4fv) (GLint location, GLsizei count, GLboolean transpose, const GLfloat *value);
		GLAPI void (APIENTRYP glUniformMatrix4x3fv) (GLint location, GLsizei count, GLboolean transpose, const GLfloat *value);
#endif

#ifdef GL_VERSION_3_0
		GLAPI void (APIENTRYP glColorMaski) (GLuint index, GLboolean r, GLboolean g, GLboolean b, GLboolean a);
		GLAPI void (APIENTRYP glGetBooleani_v) (GLenum target, GLuint index, GLboolean *data);
		GLAPI void (APIENTRYP glGetIntegeri_v) (GLenum target, GLuint index, GLint *data);
		GLAPI void (APIENTRYP glEnablei) (GLenum target, GLuint index);
		GLAPI void (APIENTRYP glDisablei) (GLenum target, GLuint index);
		GLAPI GLboolean (APIENTRYP glIsEnabledi) (GLenum target, GLuint index);
		GLAPI void (APIENTRYP glBeginTransformFeedback) (GLenum primitiveMode);
		GLAPI void (APIENTRYP glEndTransformFeedback) (void);
		GLAPI void (APIENTRYP glBindBufferRange) (GLenum target, GLuint index, GLuint buffer, GLintptr offset, GLsizeiptr size);
		GLAPI void (APIENTRYP glBindBufferBase) (GLenum target, GLuint index, GLuint buffer);
		GLAPI void (APIENTRYP glTransformFeedbackVaryings) (GLuint program, GLsizei count, const GLchar *const*varyings, GLenum bufferMode);
		GLAPI void (APIENTRYP glGetTransformFeedbackVarying) (GLuint program, GLuint index, GLsizei bufSize, GLsizei *length, GLsizei *size, GLenum *type, GLchar *name);
		GLAPI void (APIENTRYP glClampColor) (GLenum target, GLenum clamp);
		GLAPI void (APIENTRYP glBeginConditionalRender) (GLuint id, GLenum mode);
		GLAPI void (APIENTRYP glEndConditionalRender) (void);
		GLAPI void (APIENTRYP glVertexAttribIPointer) (GLuint index, GLint size, GLenum type, GLsizei stride, const void *pointer);
		GLAPI void (APIENTRYP glGetVertexAttribIiv) (GLuint index, GLenum pname, GLint *params);
		GLAPI void (APIENTRYP glGetVertexAttribIuiv) (GLuint index, GLenum pname, GLuint *params);
		GLAPI void (APIENTRYP glVertexAttribI1i) (GLuint index, GLint x);
		GLAPI void (APIENTRYP glVertexAttribI2i) (GLuint index, GLint x, GLint y);
		GLAPI void (APIENTRYP glVertexAttribI3i) (GLuint index, GLint x, GLint y, GLint z);
		GLAPI void (APIENTRYP glVertexAttribI4i) (GLuint index, GLint x, GLint y, GLint z, GLint w);
		GLAPI void (APIENTRYP glVertexAttribI1ui) (GLuint index, GLuint x);
		GLAPI void (APIENTRYP glVertexAttribI2ui) (GLuint index, GLuint x, GLuint y);
		GLAPI void (APIENTRYP glVertexAttribI3ui) (GLuint index, GLuint x, GLuint y, GLuint z);
		GLAPI void (APIENTRYP glVertexAttribI4ui) (GLuint index, GLuint x, GLuint y, GLuint z, GLuint w);
		GLAPI void (APIENTRYP glVertexAttribI1iv) (GLuint index, const GLint *v);
		GLAPI void (APIENTRYP glVertexAttribI2iv) (GLuint index, const GLint *v);
		GLAPI void (APIENTRYP glVertexAttribI3iv) (GLuint index, const GLint *v);
		GLAPI void (APIENTRYP glVertexAttribI4iv) (GLuint index, const GLint *v);
		GLAPI void (APIENTRYP glVertexAttribI1uiv) (GLuint index, const GLuint *v);
		GLAPI void (APIENTRYP glVertexAttribI2uiv) (GLuint index, const GLuint *v);
		GLAPI void (APIENTRYP glVertexAttribI3uiv) (GLuint index, const GLuint *v);
		GLAPI void (APIENTRYP glVertexAttribI4uiv) (GLuint index, const GLuint *v);
		GLAPI void (APIENTRYP glVertexAttribI4bv) (GLuint index, const GLbyte *v);
		GLAPI void (APIENTRYP glVertexAttribI4sv) (GLuint index, const GLshort *v);
		GLAPI void (APIENTRYP glVertexAttribI4ubv) (GLuint index, const GLubyte *v);
		GLAPI void (APIENTRYP glVertexAttribI4usv) (GLuint index, const GLushort *v);
		GLAPI void (APIENTRYP glGetUniformuiv) (GLuint program, GLint location, GLuint *params);
		GLAPI void (APIENTRYP glBindFragDataLocation) (GLuint program, GLuint color, const GLchar *name);
		GLAPI GLint (APIENTRYP glGetFragDataLocation) (GLuint program, const GLchar *name);
		GLAPI void (APIENTRYP glUniform1ui) (GLint location, GLuint v0);
		GLAPI void (APIENTRYP glUniform2ui) (GLint location, GLuint v0, GLuint v1);
		GLAPI void (APIENTRYP glUniform3ui) (GLint location, GLuint v0, GLuint v1, GLuint v2);
		GLAPI void (APIENTRYP glUniform4ui) (GLint location, GLuint v0, GLuint v1, GLuint v2, GLuint v3);
		GLAPI void (APIENTRYP glUniform1uiv) (GLint location, GLsizei count, const GLuint *value);
		GLAPI void (APIENTRYP glUniform2uiv) (GLint location, GLsizei count, const GLuint *value);
		GLAPI void (APIENTRYP glUniform3uiv) (GLint location, GLsizei count, const GLuint *value);
		GLAPI void (APIENTRYP glUniform4uiv) (GLint location, GLsizei count, const GLuint *value);
		GLAPI void (APIENTRYP glTexParameterIiv) (GLenum target, GLenum pname, const GLint *params);
		GLAPI void (APIENTRYP glTexParameterIuiv) (GLenum target, GLenum pname, const GLuint *params);
		GLAPI void (APIENTRYP glGetTexParameterIiv) (GLenum target, GLenum pname, GLint *params);
		GLAPI void (APIENTRYP glGetTexParameterIuiv) (GLenum target, GLenum pname, GLuint *params);
		GLAPI void (APIENTRYP glClearBufferiv) (GLenum buffer, GLint drawbuffer, const GLint *value);
		GLAPI void (APIENTRYP glClearBufferuiv) (GLenum buffer, GLint drawbuffer, const GLuint *value);
		GLAPI void (APIENTRYP glClearBufferfv) (GLenum buffer, GLint drawbuffer, const GLfloat *value);
		GLAPI void (APIENTRYP glClearBufferfi) (GLenum buffer, GLint drawbuffer, GLfloat depth, GLint stencil);
		GLAPI const GLubyte *(APIENTRYP glGetStringi) (GLenum name, GLuint index);
		GLAPI GLboolean (APIENTRYP glIsRenderbuffer) (GLuint renderbuffer);
		GLAPI void (APIENTRYP glBindRenderbuffer) (GLenum target, GLuint renderbuffer);
		GLAPI void (APIENTRYP glDeleteRenderbuffers) (GLsizei n, const GLuint *renderbuffers);
		GLAPI void (APIENTRYP glGenRenderbuffers) (GLsizei n, GLuint *renderbuffers);
		GLAPI void (APIENTRYP glRenderbufferStorage) (GLenum target, GLenum internalformat, GLsizei width, GLsizei height);
		GLAPI void (APIENTRYP glGetRenderbufferParameteriv) (GLenum target, GLenum pname, GLint *params);
		GLAPI GLboolean (APIENTRYP glIsFramebuffer) (GLuint framebuffer);
		GLAPI void (APIENTRYP glBindFramebuffer) (GLenum target, GLuint framebuffer);
		GLAPI void (APIENTRYP glDeleteFramebuffers) (GLsizei n, const GLuint *framebuffers);
		GLAPI void (APIENTRYP glGenFramebuffers) (GLsizei n, GLuint *framebuffers);
		GLAPI GLenum (APIENTRYP glCheckFramebufferStatus) (GLenum target);
		GLAPI void (APIENTRYP glFramebufferTexture1D) (GLenum target, GLenum attachment, GLenum textarget, GLuint texture, GLint level);
		GLAPI void (APIENTRYP glFramebufferTexture2D) (GLenum target, GLenum attachment, GLenum textarget, GLuint texture, GLint level);
		GLAPI void (APIENTRYP glFramebufferTexture3D) (GLenum target, GLenum attachment, GLenum textarget, GLuint texture, GLint level, GLint zoffset);
		GLAPI void (APIENTRYP glFramebufferRenderbuffer) (GLenum target, GLenum attachment, GLenum renderbuffertarget, GLuint renderbuffer);
		GLAPI void (APIENTRYP glGetFramebufferAttachmentParameteriv) (GLenum target, GLenum attachment, GLenum pname, GLint *params);
		GLAPI void (APIENTRYP glGenerateMipmap) (GLenum target);
		GLAPI void (APIENTRYP glBlitFramebuffer) (GLint srcX0, GLint srcY0, GLint srcX1, GLint srcY1, GLint dstX0, GLint dstY0, GLint dstX1, GLint dstY1, GLbitfield mask, GLenum filter);
		GLAPI void (APIENTRYP glRenderbufferStorageMultisample) (GLenum target, GLsizei samples, GLenum internalformat, GLsizei width, GLsizei height);
		GLAPI void (APIENTRYP glFramebufferTextureLayer) (GLenum target, GLenum attachment, GLuint texture, GLint level, GLint layer);
		GLAPI void *(APIENTRYP glMapBufferRange) (GLenum target, GLintptr offset, GLsizeiptr length, GLbitfield access);
		GLAPI void (APIENTRYP glFlushMappedBufferRange) (GLenum target, GLintptr offset, GLsizeiptr length);
		GLAPI void (APIENTRYP glBindVertexArray) (GLuint array);
		GLAPI void (APIENTRYP glDeleteVertexArrays) (GLsizei n, const GLuint *arrays);
		GLAPI void (APIENTRYP glGenVertexArrays) (GLsizei n, GLuint *arrays);
		GLAPI GLboolean (APIENTRYP glIsVertexArray) (GLuint array);
#endif

#ifdef GL_VERSION_3_1
		GLAPI void (APIENTRYP glDrawArraysInstanced) (GLenum mode, GLint first, GLsizei count, GLsizei instancecount);
		GLAPI void (APIENTRYP glDrawElementsInstanced) (GLenum mode, GLsizei count, GLenum type, const void *indices, GLsizei instancecount);
		GLAPI void (APIENTRYP glTexBuffer) (GLenum target, GLenum internalformat, GLuint buffer);
		GLAPI void (APIENTRYP glPrimitiveRestartIndex) (GLuint index);
		GLAPI void (APIENTRYP glCopyBufferSubData) (GLenum readTarget, GLenum writeTarget, GLintptr readOffset, GLintptr writeOffset, GLsizeiptr size);
		GLAPI void (APIENTRYP glGetUniformIndices) (GLuint program, GLsizei uniformCount, const GLchar *const*uniformNames, GLuint *uniformIndices);
		GLAPI void (APIENTRYP glGetActiveUniformsiv) (GLuint program, GLsizei uniformCount, const GLuint *uniformIndices, GLenum pname, GLint *params);
		GLAPI void (APIENTRYP glGetActiveUniformName) (GLuint program, GLuint uniformIndex, GLsizei bufSize, GLsizei *length, GLchar *uniformName);
		GLAPI GLuint (APIENTRYP glGetUniformBlockIndex) (GLuint program, const GLchar *uniformBlockName);
		GLAPI void (APIENTRYP glGetActiveUniformBlockiv) (GLuint program, GLuint uniformBlockIndex, GLenum pname, GLint *params);
		GLAPI void (APIENTRYP glGetActiveUniformBlockName) (GLuint program, GLuint uniformBlockIndex, GLsizei bufSize, GLsizei *length, GLchar *uniformBlockName);
		GLAPI void (APIENTRYP glUniformBlockBinding) (GLuint program, GLuint uniformBlockIndex, GLuint uniformBlockBinding);
#endif

#ifdef GL_VERSION_3_2
		GLAPI void (APIENTRYP glDrawElementsBaseVertex) (GLenum mode, GLsizei count, GLenum type, const void *indices, GLint basevertex);
		GLAPI void (APIENTRYP glDrawRangeElementsBaseVertex) (GLenum mode, GLuint start, GLuint end, GLsizei count, GLenum type, const void *indices, GLint basevertex);
		GLAPI void (APIENTRYP glDrawElementsInstancedBaseVertex) (GLenum mode, GLsizei count, GLenum type, const void *indices, GLsizei instancecount, GLint basevertex);
		GLAPI void (APIENTRYP glMultiDrawElementsBaseVertex) (GLenum mode, const GLsizei *count, GLenum type, const void *const*indices, GLsizei drawcount, const GLint *basevertex);
		GLAPI void (APIENTRYP glProvokingVertex) (GLenum mode);
		GLAPI GLsync (APIENTRYP glFenceSync) (GLenum condition, GLbitfield flags);
		GLAPI GLboolean (APIENTRYP glIsSync) (GLsync sync);
		GLAPI void (APIENTRYP glDeleteSync) (GLsync sync);
		GLAPI GLenum (APIENTRYP glClientWaitSync) (GLsync sync, GLbitfield flags, GLuint64 timeout);
		GLAPI void (APIENTRYP glWaitSync) (GLsync sync, GLbitfield flags, GLuint64 timeout);
		GLAPI void (APIENTRYP glGetInteger64v) (GLenum pname, GLint64 *data);
		GLAPI void (APIENTRYP glGetSynciv) (GLsync sync, GLenum pname, GLsizei bufSize, GLsizei *length, GLint *values);
		GLAPI void (APIENTRYP glGetInteger64i_v) (GLenum target, GLuint index, GLint64 *data);
		GLAPI void (APIENTRYP glGetBufferParameteri64v) (GLenum target, GLenum pname, GLint64 *params);
		GLAPI void (APIENTRYP glFramebufferTexture) (GLenum target, GLenum attachment, GLuint texture, GLint level);
		GLAPI void (APIENTRYP glTexImage2DMultisample) (GLenum target, GLsizei samples, GLenum internalformat, GLsizei width, GLsizei height, GLboolean fixedsamplelocations);
		GLAPI void (APIENTRYP glTexImage3DMultisample) (GLenum target, GLsizei samples, GLenum internalformat, GLsizei width, GLsizei height, GLsizei depth, GLboolean fixedsamplelocations);
		GLAPI void (APIENTRYP glGetMultisamplefv) (GLenum pname, GLuint index, GLfloat *val);
		GLAPI void (APIENTRYP glSampleMaski) (GLuint maskNumber, GLbitfield mask);
#endif

#ifdef GL_VERSION_3_3
		GLAPI void (APIENTRYP glBindFragDataLocationIndexed) (GLuint program, GLuint colorNumber, GLuint index, const GLchar *name);
		GLAPI GLint (APIENTRYP glGetFragDataIndex) (GLuint program, const GLchar *name);
		GLAPI void (APIENTRYP glGenSamplers) (GLsizei count, GLuint *samplers);
		GLAPI void (APIENTRYP glDeleteSamplers) (GLsizei count, const GLuint *samplers);
		GLAPI GLboolean (APIENTRYP glIsSampler) (GLuint sampler);
		GLAPI void (APIENTRYP glBindSampler) (GLuint unit, GLuint sampler);
		GLAPI void (APIENTRYP glSamplerParameteri) (GLuint sampler, GLenum pname, GLint param);
		GLAPI void (APIENTRYP glSamplerParameteriv) (GLuint sampler, GLenum pname, const GLint *param);
		GLAPI void (APIENTRYP glSamplerParameterf) (GLuint sampler, GLenum pname, GLfloat param);
		GLAPI void (APIENTRYP glSamplerParameterfv) (GLuint sampler, GLenum pname, const GLfloat *param);
		GLAPI void (APIENTRYP glSamplerParameterIiv) (GLuint sampler, GLenum pname, const GLint *param);
		GLAPI void (APIENTRYP glSamplerParameterIuiv) (GLuint sampler, GLenum pname, const GLuint *param);
		GLAPI void (APIENTRYP glGetSamplerParameteriv) (GLuint sampler, GLenum pname, GLint *params);
		GLAPI void (APIENTRYP glGetSamplerParameterIiv) (GLuint sampler, GLenum pname, GLint *params);
		GLAPI void (APIENTRYP glGetSamplerParameterfv) (GLuint sampler, GLenum pname, GLfloat *params);
		GLAPI void (APIENTRYP glGetSamplerParameterIuiv) (GLuint sampler, GLenum pname, GLuint *params);
		GLAPI void (APIENTRYP glQueryCounter) (GLuint id, GLenum target);
		GLAPI void (APIENTRYP glGetQueryObjecti64v) (GLuint id, GLenum pname, GLint64 *params);
		GLAPI void (APIENTRYP glGetQueryObjectui64v) (GLuint id, GLenum pname, GLuint64 *params);
		GLAPI void (APIENTRYP glVertexAttribDivisor) (GLuint index, GLuint divisor);
		GLAPI void (APIENTRYP glVertexAttribP1ui) (GLuint index, GLenum type, GLboolean normalized, GLuint value);
		GLAPI void (APIENTRYP glVertexAttribP1uiv) (GLuint index, GLenum type, GLboolean normalized, const GLuint *value);
		GLAPI void (APIENTRYP glVertexAttribP2ui) (GLuint index, GLenum type, GLboolean normalized, GLuint value);
		GLAPI void (APIENTRYP glVertexAttribP2uiv) (GLuint index, GLenum type, GLboolean normalized, const GLuint *value);
		GLAPI void (APIENTRYP glVertexAttribP3ui) (GLuint index, GLenum type, GLboolean normalized, GLuint value);
		GLAPI void (APIENTRYP glVertexAttribP3uiv) (GLuint index, GLenum type, GLboolean normalized, const GLuint *value);
		GLAPI void (APIENTRYP glVertexAttribP4ui) (GLuint index, GLenum type, GLboolean normalized, GLuint value);
		GLAPI void (APIENTRYP glVertexAttribP4uiv) (GLuint index, GLenum type, GLboolean normalized, const GLuint *value);
#endif

#ifdef GL_VERSION_4_0
		GLAPI void (APIENTRYP glMinSampleShading) (GLfloat value);
		GLAPI void (APIENTRYP glBlendEquationi) (GLuint buf, GLenum mode);
		GLAPI void (APIENTRYP glBlendEquationSeparatei) (GLuint buf, GLenum modeRGB, GLenum modeAlpha);
		GLAPI void (APIENTRYP glBlendFunci) (GLuint buf, GLenum src, GLenum dst);
		GLAPI void (APIENTRYP glBlendFuncSeparatei) (GLuint buf, GLenum srcRGB, GLenum dstRGB, GLenum srcAlpha, GLenum dstAlpha);
		GLAPI void (APIENTRYP glDrawArraysIndirect) (GLenum mode, const void *indirect);
		GLAPI void (APIENTRYP glDrawElementsIndirect) (GLenum mode, GLenum type, const void *indirect);
		GLAPI void (APIENTRYP glUniform1d) (GLint location, GLdouble x);
		GLAPI void (APIENTRYP glUniform2d) (GLint location, GLdouble x, GLdouble y);
		GLAPI void (APIENTRYP glUniform3d) (GLint location, GLdouble x, GLdouble y, GLdouble z);
		GLAPI void (APIENTRYP glUniform4d) (GLint location, GLdouble x, GLdouble y, GLdouble z, GLdouble w);
		GLAPI void (APIENTRYP glUniform1dv) (GLint location, GLsizei count, const GLdouble *value);
		GLAPI void (APIENTRYP glUniform2dv) (GLint location, GLsizei count, const GLdouble *value);
		GLAPI void (APIENTRYP glUniform3dv) (GLint location, GLsizei count, const GLdouble *value);
		GLAPI void (APIENTRYP glUniform4dv) (GLint location, GLsizei count, const GLdouble *value);
		GLAPI void (APIENTRYP glUniformMatrix2dv) (GLint location, GLsizei count, GLboolean transpose, const GLdouble *value);
		GLAPI void (APIENTRYP glUniformMatrix3dv) (GLint location, GLsizei count, GLboolean transpose, const GLdouble *value);
		GLAPI void (APIENTRYP glUniformMatrix4dv) (GLint location, GLsizei count, GLboolean transpose, const GLdouble *value);
		GLAPI void (APIENTRYP glUniformMatrix2x3dv) (GLint location, GLsizei count, GLboolean transpose, const GLdouble *value);
		GLAPI void (APIENTRYP glUniformMatrix2x4dv) (GLint location, GLsizei count, GLboolean transpose, const GLdouble *value);
		GLAPI void (APIENTRYP glUniformMatrix3x2dv) (GLint location, GLsizei count, GLboolean transpose, const GLdouble *value);
		GLAPI void (APIENTRYP glUniformMatrix3x4dv) (GLint location, GLsizei count, GLboolean transpose, const GLdouble *value);
		GLAPI void (APIENTRYP glUniformMatrix4x2dv) (GLint location, GLsizei count, GLboolean transpose, const GLdouble *value);
		GLAPI void (APIENTRYP glUniformMatrix4x3dv) (GLint location, GLsizei count, GLboolean transpose, const GLdouble *value);
		GLAPI void (APIENTRYP glGetUniformdv) (GLuint program, GLint location, GLdouble *params);
		GLAPI GLint (APIENTRYP glGetSubroutineUniformLocation) (GLuint program, GLenum shadertype, const GLchar *name);
		GLAPI GLuint (APIENTRYP glGetSubroutineIndex) (GLuint program, GLenum shadertype, const GLchar *name);
		GLAPI void (APIENTRYP glGetActiveSubroutineUniformiv) (GLuint program, GLenum shadertype, GLuint index, GLenum pname, GLint *values);
		GLAPI void (APIENTRYP glGetActiveSubroutineUniformName) (GLuint program, GLenum shadertype, GLuint index, GLsizei bufsize, GLsizei *length, GLchar *name);
		GLAPI void (APIENTRYP glGetActiveSubroutineName) (GLuint program, GLenum shadertype, GLuint index, GLsizei bufsize, GLsizei *length, GLchar *name);
		GLAPI void (APIENTRYP glUniformSubroutinesuiv) (GLenum shadertype, GLsizei count, const GLuint *indices);
		GLAPI void (APIENTRYP glGetUniformSubroutineuiv) (GLenum shadertype, GLint location, GLuint *params);
		GLAPI void (APIENTRYP glGetProgramStageiv) (GLuint program, GLenum shadertype, GLenum pname, GLint *values);
		GLAPI void (APIENTRYP glPatchParameteri) (GLenum pname, GLint value);
		GLAPI void (APIENTRYP glPatchParameterfv) (GLenum pname, const GLfloat *values);
		GLAPI void (APIENTRYP glBindTransformFeedback) (GLenum target, GLuint id);
		GLAPI void (APIENTRYP glDeleteTransformFeedbacks) (GLsizei n, const GLuint *ids);
		GLAPI void (APIENTRYP glGenTransformFeedbacks) (GLsizei n, GLuint *ids);
		GLAPI GLboolean (APIENTRYP glIsTransformFeedback) (GLuint id);
		GLAPI void (APIENTRYP glPauseTransformFeedback) (void);
		GLAPI void (APIENTRYP glResumeTransformFeedback) (void);
		GLAPI void (APIENTRYP glDrawTransformFeedback) (GLenum mode, GLuint id);
		GLAPI void (APIENTRYP glDrawTransformFeedbackStream) (GLenum mode, GLuint id, GLuint stream);
		GLAPI void (APIENTRYP glBeginQueryIndexed) (GLenum target, GLuint index, GLuint id);
		GLAPI void (APIENTRYP glEndQueryIndexed) (GLenum target, GLuint index);
		GLAPI void (APIENTRYP glGetQueryIndexediv) (GLenum target, GLuint index, GLenum pname, GLint *params);
#endif

#ifdef GL_VERSION_4_1
		GLAPI void (APIENTRYP glReleaseShaderCompiler) (void);
		GLAPI void (APIENTRYP glShaderBinary) (GLsizei count, const GLuint *shaders, GLenum binaryformat, const void *binary, GLsizei length);
		GLAPI void (APIENTRYP glGetShaderPrecisionFormat) (GLenum shadertype, GLenum precisiontype, GLint *range, GLint *precision);
		GLAPI void (APIENTRYP glDepthRangef) (GLfloat n, GLfloat f);
		GLAPI void (APIENTRYP glClearDepthf) (GLfloat d);
		GLAPI void (APIENTRYP glGetProgramBinary) (GLuint program, GLsizei bufSize, GLsizei *length, GLenum *binaryFormat, void *binary);
		GLAPI void (APIENTRYP glProgramBinary) (GLuint program, GLenum binaryFormat, const void *binary, GLsizei length);
		GLAPI void (APIENTRYP glProgramParameteri) (GLuint program, GLenum pname, GLint value);
		GLAPI void (APIENTRYP glUseProgramStages) (GLuint pipeline, GLbitfield stages, GLuint program);
		GLAPI void (APIENTRYP glActiveShaderProgram) (GLuint pipeline, GLuint program);
		GLAPI GLuint (APIENTRYP glCreateShaderProgramv) (GLenum type, GLsizei count, const GLchar *const*strings);
		GLAPI void (APIENTRYP glBindProgramPipeline) (GLuint pipeline);
		GLAPI void (APIENTRYP glDeleteProgramPipelines) (GLsizei n, const GLuint *pipelines);
		GLAPI void (APIENTRYP glGenProgramPipelines) (GLsizei n, GLuint *pipelines);
		GLAPI GLboolean (APIENTRYP glIsProgramPipeline) (GLuint pipeline);
		GLAPI void (APIENTRYP glGetProgramPipelineiv) (GLuint pipeline, GLenum pname, GLint *params);
		GLAPI void (APIENTRYP glProgramUniform1i) (GLuint program, GLint location, GLint v0);
		GLAPI void (APIENTRYP glProgramUniform1iv) (GLuint program, GLint location, GLsizei count, const GLint *value);
		GLAPI void (APIENTRYP glProgramUniform1f) (GLuint program, GLint location, GLfloat v0);
		GLAPI void (APIENTRYP glProgramUniform1fv) (GLuint program, GLint location, GLsizei count, const GLfloat *value);
		GLAPI void (APIENTRYP glProgramUniform1d) (GLuint program, GLint location, GLdouble v0);
		GLAPI void (APIENTRYP glProgramUniform1dv) (GLuint program, GLint location, GLsizei count, const GLdouble *value);
		GLAPI void (APIENTRYP glProgramUniform1ui) (GLuint program, GLint location, GLuint v0);
		GLAPI void (APIENTRYP glProgramUniform1uiv) (GLuint program, GLint location, GLsizei count, const GLuint *value);
		GLAPI void (APIENTRYP glProgramUniform2i) (GLuint program, GLint location, GLint v0, GLint v1);
		GLAPI void (APIENTRYP glProgramUniform2iv) (GLuint program, GLint location, GLsizei count, const GLint *value);
		GLAPI void (APIENTRYP glProgramUniform2f) (GLuint program, GLint location, GLfloat v0, GLfloat v1);
		GLAPI void (APIENTRYP glProgramUniform2fv) (GLuint program, GLint location, GLsizei count, const GLfloat *value);
		GLAPI void (APIENTRYP glProgramUniform2d) (GLuint program, GLint location, GLdouble v0, GLdouble v1);
		GLAPI void (APIENTRYP glProgramUniform2dv) (GLuint program, GLint location, GLsizei count, const GLdouble *value);
		GLAPI void (APIENTRYP glProgramUniform2ui) (GLuint program, GLint location, GLuint v0, GLuint v1);
		GLAPI void (APIENTRYP glProgramUniform2uiv) (GLuint program, GLint location, GLsizei count, const GLuint *value);
		GLAPI void (APIENTRYP glProgramUniform3i) (GLuint program, GLint location, GLint v0, GLint v1, GLint v2);
		GLAPI void (APIENTRYP glProgramUniform3iv) (GLuint program, GLint location, GLsizei count, const GLint *value);
		GLAPI void (APIENTRYP glProgramUniform3f) (GLuint program, GLint location, GLfloat v0, GLfloat v1, GLfloat v2);
		GLAPI void (APIENTRYP glProgramUniform3fv) (GLuint program, GLint location, GLsizei count, const GLfloat *value);
		GLAPI void (APIENTRYP glProgramUniform3d) (GLuint program, GLint location, GLdouble v0, GLdouble v1, GLdouble v2);
		GLAPI void (APIENTRYP glProgramUniform3dv) (GLuint program, GLint location, GLsizei count, const GLdouble *value);
		GLAPI void (APIENTRYP glProgramUniform3ui) (GLuint program, GLint location, GLuint v0, GLuint v1, GLuint v2);
		GLAPI void (APIENTRYP glProgramUniform3uiv) (GLuint program, GLint location, GLsizei count, const GLuint *value);
		GLAPI void (APIENTRYP glProgramUniform4i) (GLuint program, GLint location, GLint v0, GLint v1, GLint v2, GLint v3);
		GLAPI void (APIENTRYP glProgramUniform4iv) (GLuint program, GLint location, GLsizei count, const GLint *value);
		GLAPI void (APIENTRYP glProgramUniform4f) (GLuint program, GLint location, GLfloat v0, GLfloat v1, GLfloat v2, GLfloat v3);
		GLAPI void (APIENTRYP glProgramUniform4fv) (GLuint program, GLint location, GLsizei count, const GLfloat *value);
		GLAPI void (APIENTRYP glProgramUniform4d) (GLuint program, GLint location, GLdouble v0, GLdouble v1, GLdouble v2, GLdouble v3);
		GLAPI void (APIENTRYP glProgramUniform4dv) (GLuint program, GLint location, GLsizei count, const GLdouble *value);
		GLAPI void (APIENTRYP glProgramUniform4ui) (GLuint program, GLint location, GLuint v0, GLuint v1, GLuint v2, GLuint v3);
		GLAPI void (APIENTRYP glProgramUniform4uiv) (GLuint program, GLint location, GLsizei count, const GLuint *value);
		GLAPI void (APIENTRYP glProgramUniformMatrix2fv) (GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLfloat *value);
		GLAPI void (APIENTRYP glProgramUniformMatrix3fv) (GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLfloat *value);
		GLAPI void (APIENTRYP glProgramUniformMatrix4fv) (GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLfloat *value);
		GLAPI void (APIENTRYP glProgramUniformMatrix2dv) (GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLdouble *value);
		GLAPI void (APIENTRYP glProgramUniformMatrix3dv) (GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLdouble *value);
		GLAPI void (APIENTRYP glProgramUniformMatrix4dv) (GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLdouble *value);
		GLAPI void (APIENTRYP glProgramUniformMatrix2x3fv) (GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLfloat *value);
		GLAPI void (APIENTRYP glProgramUniformMatrix3x2fv) (GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLfloat *value);
		GLAPI void (APIENTRYP glProgramUniformMatrix2x4fv) (GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLfloat *value);
		GLAPI void (APIENTRYP glProgramUniformMatrix4x2fv) (GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLfloat *value);
		GLAPI void (APIENTRYP glProgramUniformMatrix3x4fv) (GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLfloat *value);
		GLAPI void (APIENTRYP glProgramUniformMatrix4x3fv) (GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLfloat *value);
		GLAPI void (APIENTRYP glProgramUniformMatrix2x3dv) (GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLdouble *value);
		GLAPI void (APIENTRYP glProgramUniformMatrix3x2dv) (GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLdouble *value);
		GLAPI void (APIENTRYP glProgramUniformMatrix2x4dv) (GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLdouble *value);
		GLAPI void (APIENTRYP glProgramUniformMatrix4x2dv) (GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLdouble *value);
		GLAPI void (APIENTRYP glProgramUniformMatrix3x4dv) (GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLdouble *value);
		GLAPI void (APIENTRYP glProgramUniformMatrix4x3dv) (GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLdouble *value);
		GLAPI void (APIENTRYP glValidateProgramPipeline) (GLuint pipeline);
		GLAPI void (APIENTRYP glGetProgramPipelineInfoLog) (GLuint pipeline, GLsizei bufSize, GLsizei *length, GLchar *infoLog);
		GLAPI void (APIENTRYP glVertexAttribL1d) (GLuint index, GLdouble x);
		GLAPI void (APIENTRYP glVertexAttribL2d) (GLuint index, GLdouble x, GLdouble y);
		GLAPI void (APIENTRYP glVertexAttribL3d) (GLuint index, GLdouble x, GLdouble y, GLdouble z);
		GLAPI void (APIENTRYP glVertexAttribL4d) (GLuint index, GLdouble x, GLdouble y, GLdouble z, GLdouble w);
		GLAPI void (APIENTRYP glVertexAttribL1dv) (GLuint index, const GLdouble *v);
		GLAPI void (APIENTRYP glVertexAttribL2dv) (GLuint index, const GLdouble *v);
		GLAPI void (APIENTRYP glVertexAttribL3dv) (GLuint index, const GLdouble *v);
		GLAPI void (APIENTRYP glVertexAttribL4dv) (GLuint index, const GLdouble *v);
		GLAPI void (APIENTRYP glVertexAttribLPointer) (GLuint index, GLint size, GLenum type, GLsizei stride, const void *pointer);
		GLAPI void (APIENTRYP glGetVertexAttribLdv) (GLuint index, GLenum pname, GLdouble *params);
		GLAPI void (APIENTRYP glViewportArrayv) (GLuint first, GLsizei count, const GLfloat *v);
		GLAPI void (APIENTRYP glViewportIndexedf) (GLuint index, GLfloat x, GLfloat y, GLfloat w, GLfloat h);
		GLAPI void (APIENTRYP glViewportIndexedfv) (GLuint index, const GLfloat *v);
		GLAPI void (APIENTRYP glScissorArrayv) (GLuint first, GLsizei count, const GLint *v);
		GLAPI void (APIENTRYP glScissorIndexed) (GLuint index, GLint left, GLint bottom, GLsizei width, GLsizei height);
		GLAPI void (APIENTRYP glScissorIndexedv) (GLuint index, const GLint *v);
		GLAPI void (APIENTRYP glDepthRangeArrayv) (GLuint first, GLsizei count, const GLdouble *v);
		GLAPI void (APIENTRYP glDepthRangeIndexed) (GLuint index, GLdouble n, GLdouble f);
		GLAPI void (APIENTRYP glGetFloati_v) (GLenum target, GLuint index, GLfloat *data);
		GLAPI void (APIENTRYP glGetDoublei_v) (GLenum target, GLuint index, GLdouble *data);
#endif

#ifdef GL_VERSION_4_2
		GLAPI void (APIENTRYP glDrawArraysInstancedBaseInstance) (GLenum mode, GLint first, GLsizei count, GLsizei instancecount, GLuint baseinstance);
		GLAPI void (APIENTRYP glDrawElementsInstancedBaseInstance) (GLenum mode, GLsizei count, GLenum type, const void *indices, GLsizei instancecount, GLuint baseinstance);
		GLAPI void (APIENTRYP glDrawElementsInstancedBaseVertexBaseInstance) (GLenum mode, GLsizei count, GLenum type, const void *indices, GLsizei instancecount, GLint basevertex, GLuint baseinstance);
		GLAPI void (APIENTRYP glGetInternalformativ) (GLenum target, GLenum internalformat, GLenum pname, GLsizei bufSize, GLint *params);
		GLAPI void (APIENTRYP glGetActiveAtomicCounterBufferiv) (GLuint program, GLuint bufferIndex, GLenum pname, GLint *params);
		GLAPI void (APIENTRYP glBindImageTexture) (GLuint unit, GLuint texture, GLint level, GLboolean layered, GLint layer, GLenum access, GLenum format);
		GLAPI void (APIENTRYP glMemoryBarrier) (GLbitfield barriers);
		GLAPI void (APIENTRYP glTexStorage1D) (GLenum target, GLsizei levels, GLenum internalformat, GLsizei width);
		GLAPI void (APIENTRYP glTexStorage2D) (GLenum target, GLsizei levels, GLenum internalformat, GLsizei width, GLsizei height);
		GLAPI void (APIENTRYP glTexStorage3D) (GLenum target, GLsizei levels, GLenum internalformat, GLsizei width, GLsizei height, GLsizei depth);
		GLAPI void (APIENTRYP glDrawTransformFeedbackInstanced) (GLenum mode, GLuint id, GLsizei instancecount);
		GLAPI void (APIENTRYP glDrawTransformFeedbackStreamInstanced) (GLenum mode, GLuint id, GLuint stream, GLsizei instancecount);
#endif

#ifdef GL_VERSION_4_3
		GLAPI void (APIENTRYP glClearBufferData) (GLenum target, GLenum internalformat, GLenum format, GLenum type, const void *data);
		GLAPI void (APIENTRYP glClearBufferSubData) (GLenum target, GLenum internalformat, GLintptr offset, GLsizeiptr size, GLenum format, GLenum type, const void *data);
		GLAPI void (APIENTRYP glDispatchCompute) (GLuint num_groups_x, GLuint num_groups_y, GLuint num_groups_z);
		GLAPI void (APIENTRYP glDispatchComputeIndirect) (GLintptr indirect);
		GLAPI void (APIENTRYP glCopyImageSubData) (GLuint srcName, GLenum srcTarget, GLint srcLevel, GLint srcX, GLint srcY, GLint srcZ, GLuint dstName, GLenum dstTarget, GLint dstLevel, GLint dstX, GLint dstY, GLint dstZ, GLsizei srcWidth, GLsizei srcHeight, GLsizei srcDepth);
		GLAPI void (APIENTRYP glFramebufferParameteri) (GLenum target, GLenum pname, GLint param);
		GLAPI void (APIENTRYP glGetFramebufferParameteriv) (GLenum target, GLenum pname, GLint *params);
		GLAPI void (APIENTRYP glGetInternalformati64v) (GLenum target, GLenum internalformat, GLenum pname, GLsizei bufSize, GLint64 *params);
		GLAPI void (APIENTRYP glInvalidateTexSubImage) (GLuint texture, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLsizei width, GLsizei height, GLsizei depth);
		GLAPI void (APIENTRYP glInvalidateTexImage) (GLuint texture, GLint level);
		GLAPI void (APIENTRYP glInvalidateBufferSubData) (GLuint buffer, GLintptr offset, GLsizeiptr length);
		GLAPI void (APIENTRYP glInvalidateBufferData) (GLuint buffer);
		GLAPI void (APIENTRYP glInvalidateFramebuffer) (GLenum target, GLsizei numAttachments, const GLenum *attachments);
		GLAPI void (APIENTRYP glInvalidateSubFramebuffer) (GLenum target, GLsizei numAttachments, const GLenum *attachments, GLint x, GLint y, GLsizei width, GLsizei height);
		GLAPI void (APIENTRYP glMultiDrawArraysIndirect) (GLenum mode, const void *indirect, GLsizei drawcount, GLsizei stride);
		GLAPI void (APIENTRYP glMultiDrawElementsIndirect) (GLenum mode, GLenum type, const void *indirect, GLsizei drawcount, GLsizei stride);
		GLAPI void (APIENTRYP glGetProgramInterfaceiv) (GLuint program, GLenum programInterface, GLenum pname, GLint *params);
		GLAPI GLuint (APIENTRYP glGetProgramResourceIndex) (GLuint program, GLenum programInterface, const GLchar *name);
		GLAPI void (APIENTRYP glGetProgramResourceName) (GLuint program, GLenum programInterface, GLuint index, GLsizei bufSize, GLsizei *length, GLchar *name);
		GLAPI void (APIENTRYP glGetProgramResourceiv) (GLuint program, GLenum programInterface, GLuint index, GLsizei propCount, const GLenum *props, GLsizei bufSize, GLsizei *length, GLint *params);
		GLAPI GLint (APIENTRYP glGetProgramResourceLocation) (GLuint program, GLenum programInterface, const GLchar *name);
		GLAPI GLint (APIENTRYP glGetProgramResourceLocationIndex) (GLuint program, GLenum programInterface, const GLchar *name);
		GLAPI void (APIENTRYP glShaderStorageBlockBinding) (GLuint program, GLuint storageBlockIndex, GLuint storageBlockBinding);
		GLAPI void (APIENTRYP glTexBufferRange) (GLenum target, GLenum internalformat, GLuint buffer, GLintptr offset, GLsizeiptr size);
		GLAPI void (APIENTRYP glTexStorage2DMultisample) (GLenum target, GLsizei samples, GLenum internalformat, GLsizei width, GLsizei height, GLboolean fixedsamplelocations);
		GLAPI void (APIENTRYP glTexStorage3DMultisample) (GLenum target, GLsizei samples, GLenum internalformat, GLsizei width, GLsizei height, GLsizei depth, GLboolean fixedsamplelocations);
		GLAPI void (APIENTRYP glTextureView) (GLuint texture, GLenum target, GLuint origtexture, GLenum internalformat, GLuint minlevel, GLuint numlevels, GLuint minlayer, GLuint numlayers);
		GLAPI void (APIENTRYP glBindVertexBuffer) (GLuint bindingindex, GLuint buffer, GLintptr offset, GLsizei stride);
		GLAPI void (APIENTRYP glVertexAttribFormat) (GLuint attribindex, GLint size, GLenum type, GLboolean normalized, GLuint relativeoffset);
		GLAPI void (APIENTRYP glVertexAttribIFormat) (GLuint attribindex, GLint size, GLenum type, GLuint relativeoffset);
		GLAPI void (APIENTRYP glVertexAttribLFormat) (GLuint attribindex, GLint size, GLenum type, GLuint relativeoffset);
		GLAPI void (APIENTRYP glVertexAttribBinding) (GLuint attribindex, GLuint bindingindex);
		GLAPI void (APIENTRYP glVertexBindingDivisor) (GLuint bindingindex, GLuint divisor);
		GLAPI void (APIENTRYP glDebugMessageControl) (GLenum source, GLenum type, GLenum severity, GLsizei count, const GLuint *ids, GLboolean enabled);
		GLAPI void (APIENTRYP glDebugMessageInsert) (GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar *buf);
		GLAPI void (APIENTRYP glDebugMessageCallback) (GLDEBUGPROC callback, const void *userParam);
		GLAPI GLuint (APIENTRYP glGetDebugMessageLog) (GLuint count, GLsizei bufSize, GLenum *sources, GLenum *types, GLuint *ids, GLenum *severities, GLsizei *lengths, GLchar *messageLog);
		GLAPI void (APIENTRYP glPushDebugGroup) (GLenum source, GLuint id, GLsizei length, const GLchar *message);
		GLAPI void (APIENTRYP glPopDebugGroup) (void);
		GLAPI void (APIENTRYP glObjectLabel) (GLenum identifier, GLuint name, GLsizei length, const GLchar *label);
		GLAPI void (APIENTRYP glGetObjectLabel) (GLenum identifier, GLuint name, GLsizei bufSize, GLsizei *length, GLchar *label);
		GLAPI void (APIENTRYP glObjectPtrLabel) (const void *ptr, GLsizei length, const GLchar *label);
		GLAPI void (APIENTRYP glGetObjectPtrLabel) (const void *ptr, GLsizei bufSize, GLsizei *length, GLchar *label);
#endif

#ifdef GL_VERSION_4_4
		GLAPI void (APIENTRYP glBufferStorage) (GLenum target, GLsizeiptr size, const void *data, GLbitfield flags);
		GLAPI void (APIENTRYP glClearTexImage) (GLuint texture, GLint level, GLenum format, GLenum type, const void *data);
		GLAPI void (APIENTRYP glClearTexSubImage) (GLuint texture, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLsizei width, GLsizei height, GLsizei depth, GLenum format, GLenum type, const void *data);
		GLAPI void (APIENTRYP glBindBuffersBase) (GLenum target, GLuint first, GLsizei count, const GLuint *buffers);
		GLAPI void (APIENTRYP glBindBuffersRange) (GLenum target, GLuint first, GLsizei count, const GLuint *buffers, const GLintptr *offsets, const GLsizeiptr *sizes);
		GLAPI void (APIENTRYP glBindTextures) (GLuint first, GLsizei count, const GLuint *textures);
		GLAPI void (APIENTRYP glBindSamplers) (GLuint first, GLsizei count, const GLuint *samplers);
		GLAPI void (APIENTRYP glBindImageTextures) (GLuint first, GLsizei count, const GLuint *textures);
		GLAPI void (APIENTRYP glBindVertexBuffers) (GLuint first, GLsizei count, const GLuint *buffers, const GLintptr *offsets, const GLsizei *strides);
#endif

#ifdef GL_VERSION_4_5
		GLAPI void (APIENTRYP glClipControl) (GLenum origin, GLenum depth);
		GLAPI void (APIENTRYP glCreateTransformFeedbacks) (GLsizei n, GLuint *ids);
		GLAPI void (APIENTRYP glTransformFeedbackBufferBase) (GLuint xfb, GLuint index, GLuint buffer);
		GLAPI void (APIENTRYP glTransformFeedbackBufferRange) (GLuint xfb, GLuint index, GLuint buffer, GLintptr offset, GLsizei size);
		GLAPI void (APIENTRYP glGetTransformFeedbackiv) (GLuint xfb, GLenum pname, GLint *param);
		GLAPI void (APIENTRYP glGetTransformFeedbacki_v) (GLuint xfb, GLenum pname, GLuint index, GLint *param);
		GLAPI void (APIENTRYP glGetTransformFeedbacki64_v) (GLuint xfb, GLenum pname, GLuint index, GLint64 *param);
		GLAPI void (APIENTRYP glCreateBuffers) (GLsizei n, GLuint *buffers);
		GLAPI void (APIENTRYP glNamedBufferStorage) (GLuint buffer, GLsizei size, const void *data, GLbitfield flags);
		GLAPI void (APIENTRYP glNamedBufferData) (GLuint buffer, GLsizei size, const void *data, GLenum usage);
		GLAPI void (APIENTRYP glNamedBufferSubData) (GLuint buffer, GLintptr offset, GLsizei size, const void *data);
		GLAPI void (APIENTRYP glCopyNamedBufferSubData) (GLuint readBuffer, GLuint writeBuffer, GLintptr readOffset, GLintptr writeOffset, GLsizei size);
		GLAPI void (APIENTRYP glClearNamedBufferData) (GLuint buffer, GLenum internalformat, GLenum format, GLenum type, const void *data);
		GLAPI void (APIENTRYP glClearNamedBufferSubData) (GLuint buffer, GLenum internalformat, GLintptr offset, GLsizei size, GLenum format, GLenum type, const void *data);
		GLAPI void *(APIENTRYP glMapNamedBuffer) (GLuint buffer, GLenum access);
		GLAPI void *(APIENTRYP glMapNamedBufferRange) (GLuint buffer, GLintptr offset, GLsizei length, GLbitfield access);
		GLAPI GLboolean (APIENTRYP glUnmapNamedBuffer) (GLuint buffer);
		GLAPI void (APIENTRYP glFlushMappedNamedBufferRange) (GLuint buffer, GLintptr offset, GLsizei length);
		GLAPI void (APIENTRYP glGetNamedBufferParameteriv) (GLuint buffer, GLenum pname, GLint *params);
		GLAPI void (APIENTRYP glGetNamedBufferParameteri64v) (GLuint buffer, GLenum pname, GLint64 *params);
		GLAPI void (APIENTRYP glGetNamedBufferPointerv) (GLuint buffer, GLenum pname, void **params);
		GLAPI void (APIENTRYP glGetNamedBufferSubData) (GLuint buffer, GLintptr offset, GLsizei size, void *data);
		GLAPI void (APIENTRYP glCreateFramebuffers) (GLsizei n, GLuint *framebuffers);
		GLAPI void (APIENTRYP glNamedFramebufferRenderbuffer) (GLuint framebuffer, GLenum attachment, GLenum renderbuffertarget, GLuint renderbuffer);
		GLAPI void (APIENTRYP glNamedFramebufferParameteri) (GLuint framebuffer, GLenum pname, GLint param);
		GLAPI void (APIENTRYP glNamedFramebufferTexture) (GLuint framebuffer, GLenum attachment, GLuint texture, GLint level);
		GLAPI void (APIENTRYP glNamedFramebufferTextureLayer) (GLuint framebuffer, GLenum attachment, GLuint texture, GLint level, GLint layer);
		GLAPI void (APIENTRYP glNamedFramebufferDrawBuffer) (GLuint framebuffer, GLenum buf);
		GLAPI void (APIENTRYP glNamedFramebufferDrawBuffers) (GLuint framebuffer, GLsizei n, const GLenum *bufs);
		GLAPI void (APIENTRYP glNamedFramebufferReadBuffer) (GLuint framebuffer, GLenum src);
		GLAPI void (APIENTRYP glInvalidateNamedFramebufferData) (GLuint framebuffer, GLsizei numAttachments, const GLenum *attachments);
		GLAPI void (APIENTRYP glInvalidateNamedFramebufferSubData) (GLuint framebuffer, GLsizei numAttachments, const GLenum *attachments, GLint x, GLint y, GLsizei width, GLsizei height);
		GLAPI void (APIENTRYP glClearNamedFramebufferiv) (GLuint framebuffer, GLenum buffer, GLint drawbuffer, const GLint *value);
		GLAPI void (APIENTRYP glClearNamedFramebufferuiv) (GLuint framebuffer, GLenum buffer, GLint drawbuffer, const GLuint *value);
		GLAPI void (APIENTRYP glClearNamedFramebufferfv) (GLuint framebuffer, GLenum buffer, GLint drawbuffer, const GLfloat *value);
		GLAPI void (APIENTRYP glClearNamedFramebufferfi) (GLuint framebuffer, GLenum buffer, const GLfloat depth, GLint stencil);
		GLAPI void (APIENTRYP glBlitNamedFramebuffer) (GLuint readFramebuffer, GLuint drawFramebuffer, GLint srcX0, GLint srcY0, GLint srcX1, GLint srcY1, GLint dstX0, GLint dstY0, GLint dstX1, GLint dstY1, GLbitfield mask, GLenum filter);
		GLAPI GLenum (APIENTRYP glCheckNamedFramebufferStatus) (GLuint framebuffer, GLenum target);
		GLAPI void (APIENTRYP glGetNamedFramebufferParameteriv) (GLuint framebuffer, GLenum pname, GLint *param);
		GLAPI void (APIENTRYP glGetNamedFramebufferAttachmentParameteriv) (GLuint framebuffer, GLenum attachment, GLenum pname, GLint *params);
		GLAPI void (APIENTRYP glCreateRenderbuffers) (GLsizei n, GLuint *renderbuffers);
		GLAPI void (APIENTRYP glNamedRenderbufferStorage) (GLuint renderbuffer, GLenum internalformat, GLsizei width, GLsizei height);
		GLAPI void (APIENTRYP glNamedRenderbufferStorageMultisample) (GLuint renderbuffer, GLsizei samples, GLenum internalformat, GLsizei width, GLsizei height);
		GLAPI void (APIENTRYP glGetNamedRenderbufferParameteriv) (GLuint renderbuffer, GLenum pname, GLint *params);
		GLAPI void (APIENTRYP glCreateTextures) (GLenum target, GLsizei n, GLuint *textures);
		GLAPI void (APIENTRYP glTextureBuffer) (GLuint texture, GLenum internalformat, GLuint buffer);
		GLAPI void (APIENTRYP glTextureBufferRange) (GLuint texture, GLenum internalformat, GLuint buffer, GLintptr offset, GLsizei size);
		GLAPI void (APIENTRYP glTextureStorage1D) (GLuint texture, GLsizei levels, GLenum internalformat, GLsizei width);
		GLAPI void (APIENTRYP glTextureStorage2D) (GLuint texture, GLsizei levels, GLenum internalformat, GLsizei width, GLsizei height);
		GLAPI void (APIENTRYP glTextureStorage3D) (GLuint texture, GLsizei levels, GLenum internalformat, GLsizei width, GLsizei height, GLsizei depth);
		GLAPI void (APIENTRYP glTextureStorage2DMultisample) (GLuint texture, GLsizei samples, GLenum internalformat, GLsizei width, GLsizei height, GLboolean fixedsamplelocations);
		GLAPI void (APIENTRYP glTextureStorage3DMultisample) (GLuint texture, GLsizei samples, GLenum internalformat, GLsizei width, GLsizei height, GLsizei depth, GLboolean fixedsamplelocations);
		GLAPI void (APIENTRYP glTextureSubImage1D) (GLuint texture, GLint level, GLint xoffset, GLsizei width, GLenum format, GLenum type, const void *pixels);
		GLAPI void (APIENTRYP glTextureSubImage2D) (GLuint texture, GLint level, GLint xoffset, GLint yoffset, GLsizei width, GLsizei height, GLenum format, GLenum type, const void *pixels);
		GLAPI void (APIENTRYP glTextureSubImage3D) (GLuint texture, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLsizei width, GLsizei height, GLsizei depth, GLenum format, GLenum type, const void *pixels);
		GLAPI void (APIENTRYP glCompressedTextureSubImage1D) (GLuint texture, GLint level, GLint xoffset, GLsizei width, GLenum format, GLsizei imageSize, const void *data);
		GLAPI void (APIENTRYP glCompressedTextureSubImage2D) (GLuint texture, GLint level, GLint xoffset, GLint yoffset, GLsizei width, GLsizei height, GLenum format, GLsizei imageSize, const void *data);
		GLAPI void (APIENTRYP glCompressedTextureSubImage3D) (GLuint texture, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLsizei width, GLsizei height, GLsizei depth, GLenum format, GLsizei imageSize, const void *data);
		GLAPI void (APIENTRYP glCopyTextureSubImage1D) (GLuint texture, GLint level, GLint xoffset, GLint x, GLint y, GLsizei width);
		GLAPI void (APIENTRYP glCopyTextureSubImage2D) (GLuint texture, GLint level, GLint xoffset, GLint yoffset, GLint x, GLint y, GLsizei width, GLsizei height);
		GLAPI void (APIENTRYP glCopyTextureSubImage3D) (GLuint texture, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLint x, GLint y, GLsizei width, GLsizei height);
		GLAPI void (APIENTRYP glTextureParameterf) (GLuint texture, GLenum pname, GLfloat param);
		GLAPI void (APIENTRYP glTextureParameterfv) (GLuint texture, GLenum pname, const GLfloat *param);
		GLAPI void (APIENTRYP glTextureParameteri) (GLuint texture, GLenum pname, GLint param);
		GLAPI void (APIENTRYP glTextureParameterIiv) (GLuint texture, GLenum pname, const GLint *params);
		GLAPI void (APIENTRYP glTextureParameterIuiv) (GLuint texture, GLenum pname, const GLuint *params);
		GLAPI void (APIENTRYP glTextureParameteriv) (GLuint texture, GLenum pname, const GLint *param);
		GLAPI void (APIENTRYP glGenerateTextureMipmap) (GLuint texture);
		GLAPI void (APIENTRYP glBindTextureUnit) (GLuint unit, GLuint texture);
		GLAPI void (APIENTRYP glGetTextureImage) (GLuint texture, GLint level, GLenum format, GLenum type, GLsizei bufSize, void *pixels);
		GLAPI void (APIENTRYP glGetCompressedTextureImage) (GLuint texture, GLint level, GLsizei bufSize, void *pixels);
		GLAPI void (APIENTRYP glGetTextureLevelParameterfv) (GLuint texture, GLint level, GLenum pname, GLfloat *params);
		GLAPI void (APIENTRYP glGetTextureLevelParameteriv) (GLuint texture, GLint level, GLenum pname, GLint *params);
		GLAPI void (APIENTRYP glGetTextureParameterfv) (GLuint texture, GLenum pname, GLfloat *params);
		GLAPI void (APIENTRYP glGetTextureParameterIiv) (GLuint texture, GLenum pname, GLint *params);
		GLAPI void (APIENTRYP glGetTextureParameterIuiv) (GLuint texture, GLenum pname, GLuint *params);
		GLAPI void (APIENTRYP glGetTextureParameteriv) (GLuint texture, GLenum pname, GLint *params);
		GLAPI void (APIENTRYP glCreateVertexArrays) (GLsizei n, GLuint *arrays);
		GLAPI void (APIENTRYP glDisableVertexArrayAttrib) (GLuint vaobj, GLuint index);
		GLAPI void (APIENTRYP glEnableVertexArrayAttrib) (GLuint vaobj, GLuint index);
		GLAPI void (APIENTRYP glVertexArrayElementBuffer) (GLuint vaobj, GLuint buffer);
		GLAPI void (APIENTRYP glVertexArrayVertexBuffer) (GLuint vaobj, GLuint bindingindex, GLuint buffer, GLintptr offset, GLsizei stride);
		GLAPI void (APIENTRYP glVertexArrayVertexBuffers) (GLuint vaobj, GLuint first, GLsizei count, const GLuint *buffers, const GLintptr *offsets, const GLsizei *strides);
		GLAPI void (APIENTRYP glVertexArrayAttribBinding) (GLuint vaobj, GLuint attribindex, GLuint bindingindex);
		GLAPI void (APIENTRYP glVertexArrayAttribFormat) (GLuint vaobj, GLuint attribindex, GLint size, GLenum type, GLboolean normalized, GLuint relativeoffset);
		GLAPI void (APIENTRYP glVertexArrayAttribIFormat) (GLuint vaobj, GLuint attribindex, GLint size, GLenum type, GLuint relativeoffset);
		GLAPI void (APIENTRYP glVertexArrayAttribLFormat) (GLuint vaobj, GLuint attribindex, GLint size, GLenum type, GLuint relativeoffset);
		GLAPI void (APIENTRYP glVertexArrayBindingDivisor) (GLuint vaobj, GLuint bindingindex, GLuint divisor);
		GLAPI void (APIENTRYP glGetVertexArrayiv) (GLuint vaobj, GLenum pname, GLint *param);
		GLAPI void (APIENTRYP glGetVertexArrayIndexediv) (GLuint vaobj, GLuint index, GLenum pname, GLint *param);
		GLAPI void (APIENTRYP glGetVertexArrayIndexed64iv) (GLuint vaobj, GLuint index, GLenum pname, GLint64 *param);
		GLAPI void (APIENTRYP glCreateSamplers) (GLsizei n, GLuint *samplers);
		GLAPI void (APIENTRYP glCreateProgramPipelines) (GLsizei n, GLuint *pipelines);
		GLAPI void (APIENTRYP glCreateQueries) (GLenum target, GLsizei n, GLuint *ids);
		GLAPI void (APIENTRYP glMemoryBarrierByRegion) (GLbitfield barriers);
		GLAPI void (APIENTRYP glGetTextureSubImage) (GLuint texture, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLsizei width, GLsizei height, GLsizei depth, GLenum format, GLenum type, GLsizei bufSize, void *pixels);
		GLAPI void (APIENTRYP glGetCompressedTextureSubImage) (GLuint texture, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLsizei width, GLsizei height, GLsizei depth, GLsizei bufSize, void *pixels);
		GLAPI GLenum (APIENTRYP glGetGraphicsResetStatus) (void);
		GLAPI void (APIENTRYP glGetnCompressedTexImage) (GLenum target, GLint lod, GLsizei bufSize, void *pixels);
		GLAPI void (APIENTRYP glGetnTexImage) (GLenum target, GLint level, GLenum format, GLenum type, GLsizei bufSize, void *pixels);
		GLAPI void (APIENTRYP glGetnUniformdv) (GLuint program, GLint location, GLsizei bufSize, GLdouble *params);
		GLAPI void (APIENTRYP glGetnUniformfv) (GLuint program, GLint location, GLsizei bufSize, GLfloat *params);
		GLAPI void (APIENTRYP glGetnUniformiv) (GLuint program, GLint location, GLsizei bufSize, GLint *params);
		GLAPI void (APIENTRYP glGetnUniformuiv) (GLuint program, GLint location, GLsizei bufSize, GLuint *params);
		GLAPI void (APIENTRYP glReadnPixels) (GLint x, GLint y, GLsizei width, GLsizei height, GLenum format, GLenum type, GLsizei bufSize, void *data);
		GLAPI void (APIENTRYP glTextureBarrier) (void);
#endif

#ifdef GL_ARB_bindless_texture
		GLAPI GLuint64 (APIENTRYP glGetTextureHandleARB) (GLuint texture);
		GLAPI GLuint64 (APIENTRYP glGetTextureSamplerHandleARB) (GLuint texture, GLuint sampler);
		GLAPI void (APIENTRYP glMakeTextureHandleResidentARB) (GLuint64 handle);
		GLAPI void (APIENTRYP glMakeTextureHandleNonResidentARB) (GLuint64 handle);
		GLAPI GLuint64 (APIENTRYP glGetImageHandleARB) (GLuint texture, GLint level, GLboolean layered, GLint layer, GLenum format);
		GLAPI void (APIENTRYP glMakeImageHandleResidentARB) (GLuint64 handle, GLenum access);
		GLAPI void (APIENTRYP glMakeImageHandleNonResidentARB) (GLuint64 handle);
		GLAPI void (APIENTRYP glUniformHandleui64ARB) (GLint location, GLuint64 value);
		GLAPI void (APIENTRYP glUniformHandleui64vARB) (GLint location, GLsizei count, const GLuint64 *value);
		GLAPI void (APIENTRYP glProgramUniformHandleui64ARB) (GLuint program, GLint location, GLuint64 value);
		GLAPI void (APIENTRYP glProgramUniformHandleui64vARB) (GLuint program, GLint location, GLsizei count, const GLuint64 *values);
		GLAPI GLboolean (APIENTRYP glIsTextureHandleResidentARB) (GLuint64 handle);
		GLAPI GLboolean (APIENTRYP glIsImageHandleResidentARB) (GLuint64 handle);
		GLAPI void (APIENTRYP glVertexAttribL1ui64ARB) (GLuint index, GLuint64EXT x);
		GLAPI void (APIENTRYP glVertexAttribL1ui64vARB) (GLuint index, const GLuint64EXT *v);
		GLAPI void (APIENTRYP glGetVertexAttribLui64vARB) (GLuint index, GLenum pname, GLuint64EXT *params);
#endif

#ifdef GL_ARB_cl_event
		GLAPI GLsync (APIENTRYP glCreateSyncFromCLeventARB) (struct _cl_context *context, struct _cl_event *event, GLbitfield flags);
#endif

#ifdef GL_ARB_compute_variable_group_size
		GLAPI void (APIENTRYP glDispatchComputeGroupSizeARB) (GLuint num_groups_x, GLuint num_groups_y, GLuint num_groups_z, GLuint group_size_x, GLuint group_size_y, GLuint group_size_z);
#endif

#ifdef GL_ARB_debug_output //no need (subset of 4.3)
#endif

#ifdef GL_ARB_draw_buffers_blend //no need (subset of 4.0)
#endif

#ifdef GL_ARB_indirect_parameters
		GLAPI void (APIENTRYP glMultiDrawArraysIndirectCountARB) (GLenum mode, GLintptr indirect, GLintptr drawcount, GLsizei maxdrawcount, GLsizei stride);
		GLAPI void (APIENTRYP glMultiDrawElementsIndirectCountARB) (GLenum mode, GLenum type, GLintptr indirect, GLintptr drawcount, GLsizei maxdrawcount, GLsizei stride);
#endif

#ifdef GL_ARB_robustness //no need (subset of 4.5)
#endif

#ifdef GL_ARB_sample_shading //no need (subset of 4.0)
#endif

#ifdef GL_ARB_shading_language_include
		GLAPI void (APIENTRYP glNamedStringARB) (GLenum type, GLint namelen, const GLchar *name, GLint stringlen, const GLchar *string);
		GLAPI void (APIENTRYP glDeleteNamedStringARB) (GLint namelen, const GLchar *name);
		GLAPI void (APIENTRYP glCompileShaderIncludeARB) (GLuint shader, GLsizei count, const GLchar *const*path, const GLint *length);
		GLAPI GLboolean (APIENTRYP glIsNamedStringARB) (GLint namelen, const GLchar *name);
		GLAPI void (APIENTRYP glGetNamedStringARB) (GLint namelen, const GLchar *name, GLsizei bufSize, GLint *stringlen, GLchar *string);
		GLAPI void (APIENTRYP glGetNamedStringivARB) (GLint namelen, const GLchar *name, GLenum pname, GLint *params);
#endif

#ifdef GL_ARB_sparse_buffer
		GLAPI void (APIENTRYP glBufferPageCommitmentARB) (GLenum target, GLintptr offset, GLsizei size, GLboolean commit);
		GLAPI void (APIENTRYP glNamedBufferPageCommitmentEXT) (GLuint buffer, GLintptr offset, GLsizei size, GLboolean commit);
		GLAPI void (APIENTRYP glNamedBufferPageCommitmentARB) (GLuint buffer, GLintptr offset, GLsizei size, GLboolean commit);
#endif

#ifdef GL_ARB_sparse_texture
		GLAPI void (APIENTRYP glTexPageCommitmentARB) (GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLsizei width, GLsizei height, GLsizei depth, GLboolean resident);
#endif

	} //OpenGL
} //HorseRadish
