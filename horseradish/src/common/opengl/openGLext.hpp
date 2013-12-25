/*§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§
§§																	§§
§§	>>>>> EXTENSÕES SUPORTADAS										§§
§§																	§§
§§				- GL_ARB_debug_output								§§
§§				- GL_ARB_draw_buffers_blend	(core in GL 4.0)		§§
§§				- GL_ARB_map_buffer_alignment (core in GL 4.2)		§§
§§				- GL_ARB_sample_shading (core in GL 4.0)			§§
§§				- GL_ARB_texture_cube_map_array (core in GL 4.0)	§§
§§				- GL_ARB_texture_gather								§§
§§				- GL_ARB_texture_storage (core in GL 4.2)			§§
§§																	§§
§§				- GL_EXT_direct_state_access						§§
§§				- GL_EXT_texture_compression_s3tc					§§
§§				- GL_EXT_texture_filter_anisotropic					§§
§§																	§§
§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§*/

#pragma once
#ifndef __HOPENGL_EXTENSIONS__
#define __HOPENGL_EXTENSIONS__

#include "openGL.h"

namespace HorseRadish
{

namespace OpenGL
{

namespace Extensions
{

	void ExtensionsLoad(const HorseRadish::hChar *openGLModuleName);
	bool ExtensionExists(const char * const extension);

#ifdef __cplusplus
extern "C" {
#endif

#ifndef APIENTRY
#define APIENTRY
#endif
#ifndef APIENTRYP
#define APIENTRYP APIENTRY *
#endif
#ifndef GLAPI
#define GLAPI extern
#endif

#ifndef GL_ARB_debug_output
#define GL_DEBUG_OUTPUT_SYNCHRONOUS_ARB				0x8242
#define GL_MAX_DEBUG_MESSAGE_LENGTH_ARB				0x9143
#define GL_MAX_DEBUG_LOGGED_MESSAGES_ARB			0x9144
#define GL_DEBUG_LOGGED_MESSAGES_ARB				0x9145
#define GL_DEBUG_NEXT_LOGGED_MESSAGE_LENGTH_ARB		0x8243
#define GL_DEBUG_CALLBACK_FUNCTION_ARB				0x8244
#define GL_DEBUG_CALLBACK_USER_PARAM_ARB			0x8245
#define GL_DEBUG_SOURCE_API_ARB						0x8246
#define GL_DEBUG_SOURCE_WINDOW_SYSTEM_ARB			0x8247
#define GL_DEBUG_SOURCE_SHADER_COMPILER_ARB			0x8248
#define GL_DEBUG_SOURCE_THIRD_PARTY_ARB				0x8249
#define GL_DEBUG_SOURCE_APPLICATION_ARB				0x824A
#define GL_DEBUG_SOURCE_OTHER_ARB					0x824B
#define GL_DEBUG_TYPE_ERROR_ARB						0x824C
#define GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR_ARB		0x824D
#define GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR_ARB		0x824E
#define GL_DEBUG_TYPE_PORTABILITY_ARB				0x824F
#define GL_DEBUG_TYPE_PERFORMANCE_ARB				0x8250
#define GL_DEBUG_TYPE_OTHER_ARB						0x8251
#define GL_DEBUG_SEVERITY_HIGH_ARB					0x9146
#define GL_DEBUG_SEVERITY_MEDIUM_ARB				0x9147
#define GL_DEBUG_SEVERITY_LOW_ARB					0x9148
#endif

#ifndef GL_ARB_draw_buffers_blend
#endif

#ifndef GL_ARB_map_buffer_alignment
#define GL_MIN_MAP_BUFFER_ALIGNMENT_ARB		0x90BC
#endif

#ifndef GL_ARB_sample_shading
#define GL_SAMPLE_SHADING_ARB                 0x8C36
#define GL_MIN_SAMPLE_SHADING_VALUE_ARB       0x8C37
#endif

#ifndef GL_ARB_texture_cube_map_array
#define GL_TEXTURE_CUBE_MAP_ARRAY_ARB				0x9009
#define GL_TEXTURE_BINDING_CUBE_MAP_ARRAY_ARB		0x900A
#define GL_PROXY_TEXTURE_CUBE_MAP_ARRAY_ARB			0x900B
#define GL_SAMPLER_CUBE_MAP_ARRAY_ARB				0x900C
#define GL_SAMPLER_CUBE_MAP_ARRAY_SHADOW_ARB		0x900D
#define GL_INT_SAMPLER_CUBE_MAP_ARRAY_ARB			0x900E
#define GL_UNSIGNED_INT_SAMPLER_CUBE_MAP_ARRAY_ARB	0x900F
#endif

#ifndef GL_ARB_texture_gather
#define GL_MIN_PROGRAM_TEXTURE_GATHER_OFFSET_ARB		0x8E5E
#define GL_MAX_PROGRAM_TEXTURE_GATHER_OFFSET_ARB		0x8E5F
#define GL_MAX_PROGRAM_TEXTURE_GATHER_COMPONENTS_ARB	0x8F9F
#endif

#ifndef GL_ARB_texture_storage
#define GL_TEXTURE_IMMUTABLE_FORMAT_ARB		0x912F
#endif

#ifndef GL_EXT_direct_state_access
#define GL_PROGRAM_MATRIX_EXT				0x8E2D
#define GL_TRANSPOSE_PROGRAM_MATRIX_EXT		0x8E2E
#define GL_PROGRAM_MATRIX_STACK_DEPTH_EXT	0x8E2F
#endif

#ifndef GL_EXT_texture_compression_s3tc
#define GL_COMPRESSED_RGB_S3TC_DXT1_EXT		0x83F0
#define GL_COMPRESSED_RGBA_S3TC_DXT1_EXT	0x83F1
#define GL_COMPRESSED_RGBA_S3TC_DXT3_EXT	0x83F2
#define GL_COMPRESSED_RGBA_S3TC_DXT5_EXT	0x83F3
#endif

#ifndef GL_EXT_texture_filter_anisotropic
#define GL_TEXTURE_MAX_ANISOTROPY_EXT		0x84FE
#define GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT	0x84FF
#endif

#ifndef GL_ARB_debug_output
#define GL_ARB_debug_output 1

typedef void (APIENTRY *GLDEBUGPROCARB)(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar* message, GLvoid* userParam);

GLAPI void (APIENTRYP glDebugMessageControlARB) (GLenum source, GLenum type, GLenum severity, GLsizei count, const GLuint* ids, GLboolean enabled);
GLAPI void (APIENTRYP glDebugMessageInsertARB) (GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar* buf);
GLAPI void (APIENTRYP glDebugMessageCallbackARB) (GLDEBUGPROCARB callback, GLvoid* userParam);
GLAPI GLuint (APIENTRYP glGetDebugMessageLogARB) (GLuint count, GLsizei bufsize, GLenum* sources, GLenum* types, GLuint* ids, GLenum* severities, GLsizei* lengths, GLchar* messageLog);
#endif

#ifndef GL_ARB_draw_buffers_blend
#define GL_ARB_draw_buffers_blend 1

GLAPI void (APIENTRYP glBlendEquationiARB) (GLuint, GLenum);
GLAPI void (APIENTRYP glBlendEquationSeparateiARB) (GLuint, GLenum, GLenum);
GLAPI void (APIENTRYP glBlendFunciARB) (GLuint, GLenum, GLenum);
GLAPI void (APIENTRYP glBlendFuncSeparateiARB) (GLuint, GLenum, GLenum, GLenum, GLenum);
#endif

#ifndef GL_ARB_map_buffer_alignment
#define GL_ARB_map_buffer_alignment 1
#endif

#ifndef GL_ARB_sample_shading
#define GL_ARB_sample_shading 1

GLAPI void (APIENTRYP glMinSampleShadingARB) (GLclampf);
#endif

#ifndef GL_ARB_texture_cube_map_array
#define GL_ARB_texture_cube_map_array 1
#endif

#ifndef GL_ARB_texture_gather
#define GL_ARB_texture_gather 1
#endif

#ifndef GL_ARB_texture_storage
#define GL_ARB_texture_storage 1

GLAPI void (APIENTRYP glTexStorage1DARB)		(GLenum target, GLsizei levels, GLenum internalformat, GLsizei width);
GLAPI void (APIENTRYP glTexStorage2DARB)		(GLenum target, GLsizei levels, GLenum internalformat, GLsizei width, GLsizei height);
GLAPI void (APIENTRYP glTexStorage3DARB)		(GLenum target, GLsizei levels, GLenum internalformat, GLsizei width, GLsizei height, GLsizei depth);
GLAPI void (APIENTRYP glTextureStorage1DEXT)	(GLuint texture, GLenum target, GLsizei levels, GLenum internalformat, GLsizei width);
GLAPI void (APIENTRYP glTextureStorage2DEXT)	(GLuint texture, GLenum target, GLsizei levels, GLenum internalformat, GLsizei width, GLsizei height);
GLAPI void (APIENTRYP glTextureStorage3DEXT)	(GLuint texture, GLenum target, GLsizei levels, GLenum internalformat, GLsizei width, GLsizei height, GLsizei depth);
#endif

#ifndef GL_EXT_direct_state_access
#define GL_EXT_direct_state_access 1

GLAPI void (APIENTRYP glBindMultiTextureEXT)		(GLenum texunit, GLenum target, GLuint texture);
GLAPI void (APIENTRYP glMultiTexCoordPointerEXT)	(GLenum texunit, GLint size, GLenum type, GLsizei stride, const GLvoid *pointer);
GLAPI void (APIENTRYP glProgramUniform1fEXT)		(GLuint program, GLint location, GLfloat v0);
GLAPI void (APIENTRYP glProgramUniform2fEXT)		(GLuint program, GLint location, GLfloat v0, GLfloat v1);
GLAPI void (APIENTRYP glProgramUniform3fEXT)		(GLuint program, GLint location, GLfloat v0, GLfloat v1, GLfloat v2);
GLAPI void (APIENTRYP glProgramUniform4fEXT)		(GLuint program, GLint location, GLfloat v0, GLfloat v1, GLfloat v2, GLfloat v3);
GLAPI void (APIENTRYP glProgramUniform1iEXT)		(GLuint program, GLint location, GLint v0);
GLAPI void (APIENTRYP glProgramUniform2iEXT)		(GLuint program, GLint location, GLint v0, GLint v1);
GLAPI void (APIENTRYP glProgramUniform3iEXT)		(GLuint program, GLint location, GLint v0, GLint v1, GLint v2);
GLAPI void (APIENTRYP glProgramUniform4iEXT)		(GLuint program, GLint location, GLint v0, GLint v1, GLint v2, GLint v3);
GLAPI void (APIENTRYP glProgramUniform1fvEXT)		(GLuint program, GLint location, GLsizei count, const GLfloat *value);
GLAPI void (APIENTRYP glProgramUniform2fvEXT)		(GLuint program, GLint location, GLsizei count, const GLfloat *value);
GLAPI void (APIENTRYP glProgramUniform3fvEXT)		(GLuint program, GLint location, GLsizei count, const GLfloat *value);
GLAPI void (APIENTRYP glProgramUniform4fvEXT)		(GLuint program, GLint location, GLsizei count, const GLfloat *value);
GLAPI void (APIENTRYP glProgramUniform1ivEXT)		(GLuint program, GLint location, GLsizei count, const GLint *value);
GLAPI void (APIENTRYP glProgramUniform2ivEXT)		(GLuint program, GLint location, GLsizei count, const GLint *value);
GLAPI void (APIENTRYP glProgramUniform3ivEXT)		(GLuint program, GLint location, GLsizei count, const GLint *value);
GLAPI void (APIENTRYP glProgramUniform4ivEXT)		(GLuint program, GLint location, GLsizei count, const GLint *value);
GLAPI void (APIENTRYP glProgramUniformMatrix2fvEXT)	(GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLfloat *value);
GLAPI void (APIENTRYP glProgramUniformMatrix3fvEXT)	(GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLfloat *value);
GLAPI void (APIENTRYP glProgramUniformMatrix4fvEXT)	(GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLfloat *value);
GLAPI void (APIENTRYP glGenerateTextureMipmapEXT)	(GLuint texture, GLenum target);
GLAPI void (APIENTRYP glTextureParameteriEXT)			(GLuint texture, GLenum target, GLenum pname, GLint param);
GLAPI void (APIENTRYP glTextureParameterivEXT)			(GLuint texture, GLenum target, GLenum pname, const GLint *param);
GLAPI void (APIENTRYP glTextureParameterfEXT)			(GLuint texture, GLenum target, GLenum pname, GLfloat param);
GLAPI void (APIENTRYP glTextureParameterfvEXT)			(GLuint texture, GLenum target, GLenum pname, const GLfloat *param);
GLAPI void (APIENTRYP glTextureParameterIivEXT)			(GLuint texture, GLenum target, GLenum pname, const GLint *params);
GLAPI void (APIENTRYP glTextureParameterIuivEXT)		(GLuint texture, GLenum target, GLenum pname, const GLuint *params);
GLAPI void (APIENTRYP glTextureImage1DEXT)				(GLuint texture, GLenum target, GLint level, GLint internalformat, GLsizei width, GLint border, GLenum format, GLenum type, const GLvoid *pixels);
GLAPI void (APIENTRYP glTextureImage2DEXT)				(GLuint texture, GLenum target, GLint level, GLint internalformat, GLsizei width, GLsizei height, GLint border, GLenum format, GLenum type, const GLvoid *pixels);
GLAPI void (APIENTRYP glTextureSubImage1DEXT)			(GLuint texture, GLenum target, GLint level, GLint xoffset, GLsizei width, GLenum format, GLenum type, const GLvoid *pixels);
GLAPI void (APIENTRYP glTextureSubImage2DEXT)			(GLuint texture, GLenum target, GLint level, GLint xoffset, GLint yoffset, GLsizei width, GLsizei height, GLenum format, GLenum type, const GLvoid *pixels);
GLAPI void (APIENTRYP glCopyTextureImage1DEXT)			(GLuint texture, GLenum target, GLint level, GLenum internalformat, GLint x, GLint y, GLsizei width, GLint border);
GLAPI void (APIENTRYP glCopyTextureImage2DEXT)			(GLuint texture, GLenum target, GLint level, GLenum internalformat, GLint x, GLint y, GLsizei width, GLsizei height, GLint border);
GLAPI void (APIENTRYP glCopyTextureSubImage1DEXT)		(GLuint texture, GLenum target, GLint level, GLint xoffset, GLint x, GLint y, GLsizei width);
GLAPI void (APIENTRYP glCopyTextureSubImage2DEXT)		(GLuint texture, GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint x, GLint y, GLsizei width, GLsizei height);
GLAPI void (APIENTRYP glTextureImage3DEXT)				(GLuint texture, GLenum target, GLint level, GLint internalformat, GLsizei width, GLsizei height, GLsizei depth, GLint border, GLenum format, GLenum type, const GLvoid *pixels);
GLAPI void (APIENTRYP glTextureSubImage3DEXT)			(GLuint texture, GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLsizei width, GLsizei height, GLsizei depth, GLenum format, GLenum type, const GLvoid *pixels);
GLAPI void (APIENTRYP glCopyTextureSubImage3DEXT)		(GLuint texture, GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLint x, GLint y, GLsizei width, GLsizei height);
GLAPI void (APIENTRYP glTextureBufferEXT)				(GLuint texture, GLenum target, GLenum internalformat, GLuint buffer);
GLAPI void (APIENTRYP glCompressedTextureImage3DEXT)	(GLuint texture, GLenum target, GLint level, GLenum internalformat, GLsizei width, GLsizei height, GLsizei depth, GLint border, GLsizei imageSize, const GLvoid *data);
GLAPI void (APIENTRYP glCompressedTextureImage2DEXT)	(GLuint texture, GLenum target, GLint level, GLenum internalformat, GLsizei width, GLsizei height, GLint border, GLsizei imageSize, const GLvoid *data);
GLAPI void (APIENTRYP glCompressedTextureImage1DEXT)	(GLuint texture, GLenum target, GLint level, GLenum internalformat, GLsizei width, GLint border, GLsizei imageSize, const GLvoid *data);
GLAPI void (APIENTRYP glCompressedTextureSubImage3DEXT)	(GLuint texture, GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLsizei width, GLsizei height, GLsizei depth, GLenum format, GLsizei imageSize, const GLvoid *data);
GLAPI void (APIENTRYP glCompressedTextureSubImage2DEXT)	(GLuint texture, GLenum target, GLint level, GLint xoffset, GLint yoffset, GLsizei width, GLsizei height, GLenum format, GLsizei imageSize, const GLvoid *data);
GLAPI void (APIENTRYP glCompressedTextureSubImage1DEXT)	(GLuint texture, GLenum target, GLint level, GLint xoffset, GLsizei width, GLenum format, GLsizei imageSize, const GLvoid *data);
GLAPI void (APIENTRYP glTextureRenderbufferEXT)			(GLuint texture, GLenum target, GLuint renderbuffer);
#endif

#ifndef GL_EXT_texture_compression_s3tc
#define GL_EXT_texture_compression_s3tc 1
#endif

#ifndef GL_EXT_texture_filter_anisotropic
#define GL_EXT_texture_filter_anisotropic 1
#endif

#ifdef __cplusplus
}
#endif

}//namespace Extensions
}//namespace OpenGL
}//namespace HorseRadish

#endif
