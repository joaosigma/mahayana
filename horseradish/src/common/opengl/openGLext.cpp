#include "openGLext.hpp"
#include "common\UTF.hpp"

#define GETADDR(var, name, type)  var = (type)ptrGlGetProcAddress(name);

using namespace HorseRadish::OpenGL;

#ifdef GL_ARB_debug_output
typedef void (APIENTRYP PFNGLDEBUGMESSAGECONTROLARB) (GLenum source, GLenum type, GLenum severity, GLsizei count, const GLuint* ids, GLboolean enabled);
typedef void (APIENTRYP PFNGLDEBUGMESSAGEINSERTARB) (GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar* buf);
typedef void (APIENTRYP PFNGLDEBUGMESSAGECALLBACKARB) (Extensions::GLDEBUGPROCARB callback, GLvoid* userParam);
typedef GLuint (APIENTRYP PFNGLGETDEBUGMESSAGELOGARB) (GLuint count, GLsizei bufsize, GLenum* sources, GLenum* types, GLuint* ids, GLenum* severities, GLsizei* lengths, GLchar* messageLog);
#endif

#ifdef GL_ARB_draw_buffers_blend
typedef void (APIENTRYP PFNGLBLENDEQUATIONIARBPROC) (GLuint buf, GLenum mode);
typedef void (APIENTRYP PFNGLBLENDEQUATIONSEPARATEIARBPROC) (GLuint buf, GLenum modeRGB, GLenum modeAlpha);
typedef void (APIENTRYP PFNGLBLENDFUNCIARBPROC) (GLuint buf, GLenum src, GLenum dst);
typedef void (APIENTRYP PFNGLBLENDFUNCSEPARATEIARBPROC) (GLuint buf, GLenum srcRGB, GLenum dstRGB, GLenum srcAlpha, GLenum dstAlpha);
#endif

#ifdef GL_ARB_sample_shading
typedef void (APIENTRYP PFNGLMINSAMPLESHADINGARBPROC) (GLclampf value);
#endif

#ifdef GL_ARB_texture_storage
typedef void (APIENTRY * PFNGLTEXSTORAGE1DARB) (GLenum target, GLsizei levels, GLenum internalformat, GLsizei width);
typedef void (APIENTRY * PFNGLTEXSTORAGE2DARB) (GLenum target, GLsizei levels, GLenum internalformat, GLsizei width, GLsizei height);
typedef void (APIENTRY * PFNGLTEXSTORAGE3DARB) (GLenum target, GLsizei levels, GLenum internalformat, GLsizei width, GLsizei height, GLsizei depth);
typedef void (APIENTRY * PFNGLTEXTURESTORAGE1DEXT) (GLuint texture, GLenum target, GLsizei levels, GLenum internalformat, GLsizei width);
typedef void (APIENTRY * PFNGLTEXTURESTORAGE2DEXT) (GLuint texture, GLenum target, GLsizei levels, GLenum internalformat, GLsizei width, GLsizei height);
typedef void (APIENTRY * PFNGLTEXTURESTORAGE3DEXT) (GLuint texture, GLenum target, GLsizei levels, GLenum internalformat, GLsizei width, GLsizei height, GLsizei depth);
#endif

#ifdef GL_EXT_direct_state_access
typedef void (APIENTRY * PFNGLBINDMULTITEXTUREEXT) (GLenum texunit, GLenum target, GLuint texture);
typedef void (APIENTRY * PFNGLMULTITEXCOORDPOINTEREXT) (GLenum texunit, GLint size, GLenum type, GLsizei stride, const GLvoid *pointer);
typedef void (APIENTRY * PFNGLPROGRAMUNIFORM1FEXT) (GLuint program, GLint location, GLfloat v0);
typedef void (APIENTRY * PFNGLPROGRAMUNIFORM2FEXT) (GLuint program, GLint location, GLfloat v0, GLfloat v1);
typedef void (APIENTRY * PFNGLPROGRAMUNIFORM3FEXT) (GLuint program, GLint location, GLfloat v0, GLfloat v1, GLfloat v2);
typedef void (APIENTRY * PFNGLPROGRAMUNIFORM4FEXT) (GLuint program, GLint location, GLfloat v0, GLfloat v1, GLfloat v2, GLfloat v3);
typedef void (APIENTRY * PFNGLPROGRAMUNIFORM1IEXT) (GLuint program, GLint location, GLint v0);
typedef void (APIENTRY * PFNGLPROGRAMUNIFORM2IEXT) (GLuint program, GLint location, GLint v0, GLint v1);
typedef void (APIENTRY * PFNGLPROGRAMUNIFORM3IEXT) (GLuint program, GLint location, GLint v0, GLint v1, GLint v2);
typedef void (APIENTRY * PFNGLPROGRAMUNIFORM4IEXT) (GLuint program, GLint location, GLint v0, GLint v1, GLint v2, GLint v3);
typedef void (APIENTRY * PFNGLPROGRAMUNIFORM1FVEXT) (GLuint program, GLint location, GLsizei count, const GLfloat *value);
typedef void (APIENTRY * PFNGLPROGRAMUNIFORM2FVEXT) (GLuint program, GLint location, GLsizei count, const GLfloat *value);
typedef void (APIENTRY * PFNGLPROGRAMUNIFORM3FVEXT) (GLuint program, GLint location, GLsizei count, const GLfloat *value);
typedef void (APIENTRY * PFNGLPROGRAMUNIFORM4FVEXT) (GLuint program, GLint location, GLsizei count, const GLfloat *value);
typedef void (APIENTRY * PFNGLPROGRAMUNIFORM1IVEXT) (GLuint program, GLint location, GLsizei count, const GLint *value);
typedef void (APIENTRY * PFNGLPROGRAMUNIFORM2IVEXT) (GLuint program, GLint location, GLsizei count, const GLint *value);
typedef void (APIENTRY * PFNGLPROGRAMUNIFORM3IVEXT) (GLuint program, GLint location, GLsizei count, const GLint *value);
typedef void (APIENTRY * PFNGLPROGRAMUNIFORM4IVEXT) (GLuint program, GLint location, GLsizei count, const GLint *value);
typedef void (APIENTRY * PFNGLPROGRAMUNIFORMMATRIX2FVEXT) (GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLfloat *value);
typedef void (APIENTRY * PFNGLPROGRAMUNIFORMMATRIX3FVEXT) (GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLfloat *value);
typedef void (APIENTRY * PFNGLPROGRAMUNIFORMMATRIX4FVEXT) (GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLfloat *value);
typedef void (APIENTRY * PFNGLGENERATETEXTUREMIPMAPEXT) (GLuint texture, GLenum target);
typedef void (APIENTRY * PFNGLTEXTUREPARAMETERIEXT) (GLuint texture, GLenum target, GLenum pname, GLint param);
typedef void (APIENTRY * PFNGLTEXTUREPARAMETERIVEXT) (GLuint texture, GLenum target, GLenum pname, const GLint *param);
typedef void (APIENTRY * PFNGLTEXTUREPARAMETERFEXT) (GLuint texture, GLenum target, GLenum pname, GLfloat param);
typedef void (APIENTRY * PFNGLTEXTUREPARAMETERFVEXT) (GLuint texture, GLenum target, GLenum pname, const GLfloat *param);
typedef void (APIENTRY * PFNGLTEXTUREPARAMETERIIVEXT) (GLuint texture, GLenum target, GLenum pname, const GLint *params);
typedef void (APIENTRY * PFNGLTEXTUREPARAMETERIUIVEXT) (GLuint texture, GLenum target, GLenum pname, const GLuint *params);
typedef void (APIENTRY * PFNGLTEXTUREIMAGE1DEXT) (GLuint texture, GLenum target, GLint level, GLint internalformat, GLsizei width, GLint border, GLenum format, GLenum type, const GLvoid *pixels);
typedef void (APIENTRY * PFNGLTEXTUREIMAGE2DEXT) (GLuint texture, GLenum target, GLint level, GLint internalformat, GLsizei width, GLsizei height, GLint border, GLenum format, GLenum type, const GLvoid *pixels);
typedef void (APIENTRY * PFNGLTEXTURESUBIMAGE1DEXT) (GLuint texture, GLenum target, GLint level, GLint xoffset, GLsizei width, GLenum format, GLenum type, const GLvoid *pixels);
typedef void (APIENTRY * PFNGLTEXTURESUBIMAGE2DEXT) (GLuint texture, GLenum target, GLint level, GLint xoffset, GLint yoffset, GLsizei width, GLsizei height, GLenum format, GLenum type, const GLvoid *pixels);
typedef void (APIENTRY * PFNGLCOPYTEXTUREIMAGE1DEXT) (GLuint texture, GLenum target, GLint level, GLenum internalformat, GLint x, GLint y, GLsizei width, GLint border);
typedef void (APIENTRY * PFNGLCOPYTEXTUREIMAGE2DEXT) (GLuint texture, GLenum target, GLint level, GLenum internalformat, GLint x, GLint y, GLsizei width, GLsizei height, GLint border);
typedef void (APIENTRY * PFNGLCOPYTEXTURESUBIMAGE1DEXT) (GLuint texture, GLenum target, GLint level, GLint xoffset, GLint x, GLint y, GLsizei width);
typedef void (APIENTRY * PFNGLCOPYTEXTURESUBIMAGE2DEXT) (GLuint texture, GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint x, GLint y, GLsizei width, GLsizei height);
typedef void (APIENTRY * PFNGLTEXTUREIMAGE3DEXT) (GLuint texture, GLenum target, GLint level, GLint internalformat, GLsizei width, GLsizei height, GLsizei depth, GLint border, GLenum format, GLenum type, const GLvoid *pixels);
typedef void (APIENTRY * PFNGLTEXTURESUBIMAGE3DEXT) (GLuint texture, GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLsizei width, GLsizei height, GLsizei depth, GLenum format, GLenum type, const GLvoid *pixels);
typedef void (APIENTRY * PFNGLCOPYTEXTURESUBIMAGE3DEXT) (GLuint texture, GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLint x, GLint y, GLsizei width, GLsizei height);
typedef void (APIENTRY * PFNGLTEXTUREBUFFEREXT) (GLuint texture, GLenum target, GLenum internalformat, GLuint buffer);
typedef void (APIENTRY * PFNGLCOMPRESSEDTEXTUREIMAGE3DEXT) (GLuint texture, GLenum target, GLint level, GLenum internalformat, GLsizei width, GLsizei height, GLsizei depth, GLint border, GLsizei imageSize, const GLvoid *data);
typedef void (APIENTRY * PFNGLCOMPRESSEDTEXTUREIMAGE2DEXT) (GLuint texture, GLenum target, GLint level, GLenum internalformat, GLsizei width, GLsizei height, GLint border, GLsizei imageSize, const GLvoid *data);
typedef void (APIENTRY * PFNGLCOMPRESSEDTEXTUREIMAGE1DEXT) (GLuint texture, GLenum target, GLint level, GLenum internalformat, GLsizei width, GLint border, GLsizei imageSize, const GLvoid *data);
typedef void (APIENTRY * PFNGLCOMPRESSEDTEXTURESUBIMAGE3DEXT) (GLuint texture, GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLsizei width, GLsizei height, GLsizei depth, GLenum format, GLsizei imageSize, const GLvoid *data);
typedef void (APIENTRY * PFNGLCOMPRESSEDTEXTURESUBIMAGE2DEXT) (GLuint texture, GLenum target, GLint level, GLint xoffset, GLint yoffset, GLsizei width, GLsizei height, GLenum format, GLsizei imageSize, const GLvoid *data);
typedef void (APIENTRY * PFNGLCOMPRESSEDTEXTURESUBIMAGE1DEXT) (GLuint texture, GLenum target, GLint level, GLint xoffset, GLsizei width, GLenum format, GLsizei imageSize, const GLvoid *data);
typedef void (APIENTRY * PFNGLTEXTURERENDERBUFFEREXT) (GLuint texture, GLenum target, GLuint renderbuffer);
#endif

namespace HorseRadish
{

namespace OpenGL
{

namespace Extensions
{

#ifdef GL_ARB_debug_output
PFNGLDEBUGMESSAGECONTROLARB glDebugMessageControlARB;
PFNGLDEBUGMESSAGEINSERTARB glDebugMessageInsertARB;
PFNGLDEBUGMESSAGECALLBACKARB glDebugMessageCallbackARB;
PFNGLGETDEBUGMESSAGELOGARB glGetDebugMessageLogARB;
#endif

#ifdef GL_ARB_draw_buffers_blend
PFNGLBLENDEQUATIONIARBPROC glBlendEquationiARB;
PFNGLBLENDEQUATIONSEPARATEIARBPROC glBlendEquationSeparateiARB;
PFNGLBLENDFUNCIARBPROC glBlendFunciARB;
PFNGLBLENDFUNCSEPARATEIARBPROC glBlendFuncSeparateiARB;
#endif

#ifdef GL_ARB_sample_shading
PFNGLMINSAMPLESHADINGARBPROC glMinSampleShadingARB;
#endif

#ifdef GL_ARB_texture_storage
PFNGLTEXSTORAGE1DARB glTexStorage1DARB;
PFNGLTEXSTORAGE2DARB glTexStorage2DARB;
PFNGLTEXSTORAGE3DARB glTexStorage3DARB;
PFNGLTEXTURESTORAGE1DEXT glTextureStorage1DEXT;
PFNGLTEXTURESTORAGE2DEXT glTextureStorage2DEXT;
PFNGLTEXTURESTORAGE3DEXT glTextureStorage3DEXT;
#endif

#ifdef GL_EXT_direct_state_access
PFNGLBINDMULTITEXTUREEXT glBindMultiTextureEXT;
PFNGLMULTITEXCOORDPOINTEREXT glMultiTexCoordPointerEXT;
PFNGLPROGRAMUNIFORM1FEXT glProgramUniform1fEXT;
PFNGLPROGRAMUNIFORM2FEXT glProgramUniform2fEXT;
PFNGLPROGRAMUNIFORM3FEXT glProgramUniform3fEXT;
PFNGLPROGRAMUNIFORM4FEXT glProgramUniform4fEXT;
PFNGLPROGRAMUNIFORM1IEXT glProgramUniform1iEXT;
PFNGLPROGRAMUNIFORM2IEXT glProgramUniform2iEXT;
PFNGLPROGRAMUNIFORM3IEXT glProgramUniform3iEXT;
PFNGLPROGRAMUNIFORM4IEXT glProgramUniform4iEXT;
PFNGLPROGRAMUNIFORM1FVEXT glProgramUniform1fvEXT;
PFNGLPROGRAMUNIFORM2FVEXT glProgramUniform2fvEXT;
PFNGLPROGRAMUNIFORM3FVEXT glProgramUniform3fvEXT;
PFNGLPROGRAMUNIFORM4FVEXT glProgramUniform4fvEXT;
PFNGLPROGRAMUNIFORM1IVEXT glProgramUniform1ivEXT;
PFNGLPROGRAMUNIFORM2IVEXT glProgramUniform2ivEXT;
PFNGLPROGRAMUNIFORM3IVEXT glProgramUniform3ivEXT;
PFNGLPROGRAMUNIFORM4IVEXT glProgramUniform4ivEXT;
PFNGLPROGRAMUNIFORMMATRIX2FVEXT glProgramUniformMatrix2fvEXT;
PFNGLPROGRAMUNIFORMMATRIX3FVEXT glProgramUniformMatrix3fvEXT;
PFNGLPROGRAMUNIFORMMATRIX4FVEXT glProgramUniformMatrix4fvEXT;
PFNGLGENERATETEXTUREMIPMAPEXT glGenerateTextureMipmapEXT;
PFNGLTEXTUREPARAMETERIEXT glTextureParameteriEXT;
PFNGLTEXTUREPARAMETERIVEXT glTextureParameterivEXT;
PFNGLTEXTUREPARAMETERFEXT glTextureParameterfEXT;
PFNGLTEXTUREPARAMETERFVEXT glTextureParameterfvEXT;
PFNGLTEXTUREPARAMETERIIVEXT glTextureParameterIivEXT;
PFNGLTEXTUREPARAMETERIUIVEXT glTextureParameterIuivEXT;
PFNGLTEXTUREIMAGE1DEXT glTextureImage1DEXT;
PFNGLTEXTUREIMAGE2DEXT glTextureImage2DEXT;
PFNGLTEXTURESUBIMAGE1DEXT glTextureSubImage1DEXT;
PFNGLTEXTURESUBIMAGE2DEXT glTextureSubImage2DEXT;
PFNGLCOPYTEXTUREIMAGE1DEXT glCopyTextureImage1DEXT;
PFNGLCOPYTEXTUREIMAGE2DEXT glCopyTextureImage2DEXT;
PFNGLCOPYTEXTURESUBIMAGE1DEXT glCopyTextureSubImage1DEXT;
PFNGLCOPYTEXTURESUBIMAGE2DEXT glCopyTextureSubImage2DEXT;
PFNGLTEXTUREIMAGE3DEXT glTextureImage3DEXT;
PFNGLTEXTURESUBIMAGE3DEXT glTextureSubImage3DEXT;
PFNGLCOPYTEXTURESUBIMAGE3DEXT glCopyTextureSubImage3DEXT;
PFNGLTEXTUREBUFFEREXT glTextureBufferEXT;
PFNGLCOMPRESSEDTEXTUREIMAGE3DEXT glCompressedTextureImage3DEXT;
PFNGLCOMPRESSEDTEXTUREIMAGE2DEXT glCompressedTextureImage2DEXT;
PFNGLCOMPRESSEDTEXTUREIMAGE1DEXT glCompressedTextureImage1DEXT;
PFNGLCOMPRESSEDTEXTURESUBIMAGE3DEXT glCompressedTextureSubImage3DEXT;
PFNGLCOMPRESSEDTEXTURESUBIMAGE2DEXT glCompressedTextureSubImage2DEXT;
PFNGLCOMPRESSEDTEXTURESUBIMAGE1DEXT glCompressedTextureSubImage1DEXT;
PFNGLTEXTURERENDERBUFFEREXT glTextureRenderbufferEXT;
#endif

void ExtensionsLoad(const HorseRadish::hChar *openGLModuleName)
{
	PROC (APIENTRY *ptrGlGetProcAddress)(LPCSTR lpcstr);
	wchar_t openGLModuleNameWChar[128];

	//tenho de converter o nome do modulo de OpenGL para WideChar
	HorseRadish::UTF::ConvertUTF8To(openGLModuleName, HorseRadish::UTF::Windows, openGLModuleNameWChar, sizeof(openGLModuleNameWChar));

	//tento obter este ponteiro
	ptrGlGetProcAddress = (PROC (APIENTRY *)(LPCSTR lpcstr))GetProcAddress(GetModuleHandle(openGLModuleNameWChar), "wglGetProcAddress");
	if (ptrGlGetProcAddress == nullptr)
		return;

	#ifdef GL_ARB_debug_output
	GETADDR(glDebugMessageControlARB, "glDebugMessageControlARB", PFNGLDEBUGMESSAGECONTROLARB);
	GETADDR(glDebugMessageInsertARB, "glDebugMessageInsertARB", PFNGLDEBUGMESSAGEINSERTARB);
	GETADDR(glDebugMessageCallbackARB, "glDebugMessageCallbackARB", PFNGLDEBUGMESSAGECALLBACKARB);
	GETADDR(glGetDebugMessageLogARB, "glGetDebugMessageLogARB", PFNGLGETDEBUGMESSAGELOGARB);
	#endif

	#ifdef GL_ARB_draw_buffers_blend
	GETADDR(glBlendEquationiARB, "glBlendEquationiARB", PFNGLBLENDEQUATIONIARBPROC);
	GETADDR(glBlendEquationSeparateiARB, "glBlendEquationSeparateiARB", PFNGLBLENDEQUATIONSEPARATEIARBPROC);
	GETADDR(glBlendFunciARB, "glBlendFunciARB", PFNGLBLENDFUNCIARBPROC);
	GETADDR(glBlendFuncSeparateiARB, "glBlendFuncSeparateiARB", PFNGLBLENDFUNCSEPARATEIARBPROC);
	#endif

	#ifdef GL_ARB_sample_shading
	GETADDR(glMinSampleShadingARB, "glMinSampleShadingARB", PFNGLMINSAMPLESHADINGARBPROC);
	#endif

	#ifdef GL_ARB_texture_storage
	GETADDR(glTexStorage1DARB, "glTexStorage1D", PFNGLTEXSTORAGE1DARB);
	GETADDR(glTexStorage2DARB, "glTexStorage2D", PFNGLTEXSTORAGE2DARB);
	GETADDR(glTexStorage3DARB, "glTexStorage3D", PFNGLTEXSTORAGE3DARB);
	GETADDR(glTextureStorage1DEXT, "glTextureStorage1DEXT", PFNGLTEXTURESTORAGE1DEXT);
	GETADDR(glTextureStorage2DEXT, "glTextureStorage2DEXT", PFNGLTEXTURESTORAGE2DEXT);
	GETADDR(glTextureStorage3DEXT, "glTextureStorage3DEXT", PFNGLTEXTURESTORAGE3DEXT);
	#endif

	#ifdef GL_EXT_direct_state_access
	GETADDR(glBindMultiTextureEXT, "glBindMultiTextureEXT", PFNGLBINDMULTITEXTUREEXT);
	GETADDR(glMultiTexCoordPointerEXT, "glMultiTexCoordPointerEXT", PFNGLMULTITEXCOORDPOINTEREXT);
	GETADDR(glProgramUniform1fEXT, "glProgramUniform1fEXT", PFNGLPROGRAMUNIFORM1FEXT);
	GETADDR(glProgramUniform2fEXT, "glProgramUniform2fEXT", PFNGLPROGRAMUNIFORM2FEXT);
	GETADDR(glProgramUniform3fEXT, "glProgramUniform3fEXT", PFNGLPROGRAMUNIFORM3FEXT);
	GETADDR(glProgramUniform4fEXT, "glProgramUniform4fEXT", PFNGLPROGRAMUNIFORM4FEXT);
	GETADDR(glProgramUniform1iEXT, "glProgramUniform1iEXT", PFNGLPROGRAMUNIFORM1IEXT);
	GETADDR(glProgramUniform2iEXT, "glProgramUniform2iEXT", PFNGLPROGRAMUNIFORM2IEXT);
	GETADDR(glProgramUniform3iEXT, "glProgramUniform3iEXT", PFNGLPROGRAMUNIFORM3IEXT);
	GETADDR(glProgramUniform4iEXT, "glProgramUniform4iEXT", PFNGLPROGRAMUNIFORM4IEXT);
	GETADDR(glProgramUniform1fvEXT, "glProgramUniform1fvEXT", PFNGLPROGRAMUNIFORM1FVEXT);
	GETADDR(glProgramUniform2fvEXT, "glProgramUniform2fvEXT", PFNGLPROGRAMUNIFORM2FVEXT);
	GETADDR(glProgramUniform3fvEXT, "glProgramUniform3fvEXT", PFNGLPROGRAMUNIFORM3FVEXT);
	GETADDR(glProgramUniform4fvEXT, "glProgramUniform4fvEXT", PFNGLPROGRAMUNIFORM4FVEXT);
	GETADDR(glProgramUniform1ivEXT, "glProgramUniform1ivEXT", PFNGLPROGRAMUNIFORM1IVEXT);
	GETADDR(glProgramUniform2ivEXT, "glProgramUniform2ivEXT", PFNGLPROGRAMUNIFORM2IVEXT);
	GETADDR(glProgramUniform3ivEXT, "glProgramUniform3ivEXT", PFNGLPROGRAMUNIFORM3IVEXT);
	GETADDR(glProgramUniform4ivEXT, "glProgramUniform4ivEXT", PFNGLPROGRAMUNIFORM4IVEXT);
	GETADDR(glProgramUniformMatrix2fvEXT, "glProgramUniformMatrix2fvEXT", PFNGLPROGRAMUNIFORMMATRIX2FVEXT);
	GETADDR(glProgramUniformMatrix3fvEXT, "glProgramUniformMatrix3fvEXT", PFNGLPROGRAMUNIFORMMATRIX3FVEXT);
	GETADDR(glProgramUniformMatrix4fvEXT, "glProgramUniformMatrix4fvEXT", PFNGLPROGRAMUNIFORMMATRIX4FVEXT);
	GETADDR(glGenerateTextureMipmapEXT, "glGenerateTextureMipmapEXT", PFNGLGENERATETEXTUREMIPMAPEXT);
	GETADDR(glTextureParameteriEXT, "glTextureParameteriEXT", PFNGLTEXTUREPARAMETERIEXT);
	GETADDR(glTextureParameterivEXT, "glTextureParameterivEXT", PFNGLTEXTUREPARAMETERIVEXT);
	GETADDR(glTextureParameterfEXT, "glTextureParameterfEXT", PFNGLTEXTUREPARAMETERFEXT);
	GETADDR(glTextureParameterfvEXT, "glTextureParameterfvEXT", PFNGLTEXTUREPARAMETERFVEXT);
	GETADDR(glTextureParameterIivEXT, "glTextureParameterIivEXT", PFNGLTEXTUREPARAMETERIIVEXT);
	GETADDR(glTextureParameterIuivEXT, "glTextureParameterIuivEXT", PFNGLTEXTUREPARAMETERIUIVEXT);
	GETADDR(glTextureImage1DEXT, "glTextureImage1DEXT", PFNGLTEXTUREIMAGE1DEXT);
	GETADDR(glTextureImage2DEXT, "glTextureImage2DEXT", PFNGLTEXTUREIMAGE2DEXT);
	GETADDR(glTextureSubImage1DEXT, "glTextureSubImage1DEXT", PFNGLTEXTURESUBIMAGE1DEXT);
	GETADDR(glTextureSubImage2DEXT, "glTextureSubImage2DEXT", PFNGLTEXTURESUBIMAGE2DEXT);
	GETADDR(glCopyTextureImage1DEXT, "glCopyTextureImage1DEXT", PFNGLCOPYTEXTUREIMAGE1DEXT);
	GETADDR(glCopyTextureImage2DEXT, "glCopyTextureImage2DEXT", PFNGLCOPYTEXTUREIMAGE2DEXT);
	GETADDR(glCopyTextureSubImage1DEXT, "glCopyTextureSubImage1DEXT", PFNGLCOPYTEXTURESUBIMAGE1DEXT);
	GETADDR(glCopyTextureSubImage2DEXT, "glCopyTextureSubImage2DEXT", PFNGLCOPYTEXTURESUBIMAGE2DEXT);
	GETADDR(glTextureImage3DEXT, "glTextureImage3DEXT", PFNGLTEXTUREIMAGE3DEXT);
	GETADDR(glTextureSubImage3DEXT, "glTextureSubImage3DEXT", PFNGLTEXTURESUBIMAGE3DEXT);
	GETADDR(glCopyTextureSubImage3DEXT, "glCopyTextureSubImage3DEXT", PFNGLCOPYTEXTURESUBIMAGE3DEXT);
	GETADDR(glTextureBufferEXT, "glTextureBufferEXT", PFNGLTEXTUREBUFFEREXT);
	GETADDR(glCompressedTextureImage3DEXT, "glCompressedTextureImage3DEXT", PFNGLCOMPRESSEDTEXTUREIMAGE3DEXT);
	GETADDR(glCompressedTextureImage2DEXT, "glCompressedTextureImage2DEXT", PFNGLCOMPRESSEDTEXTUREIMAGE2DEXT);
	GETADDR(glCompressedTextureImage1DEXT, "glCompressedTextureImage1DEXT", PFNGLCOMPRESSEDTEXTUREIMAGE1DEXT);
	GETADDR(glCompressedTextureSubImage3DEXT, "glCompressedTextureSubImage3DEXT", PFNGLCOMPRESSEDTEXTURESUBIMAGE3DEXT);
	GETADDR(glCompressedTextureSubImage2DEXT, "glCompressedTextureSubImage2DEXT", PFNGLCOMPRESSEDTEXTURESUBIMAGE2DEXT);
	GETADDR(glCompressedTextureSubImage1DEXT, "glCompressedTextureSubImage1DEXT", PFNGLCOMPRESSEDTEXTURESUBIMAGE1DEXT);
	GETADDR(glTextureRenderbufferEXT, "glTextureRenderbufferEXT", PFNGLTEXTURERENDERBUFFEREXT);
	#endif
}

bool ExtensionExists(const char * const extensionName)
{
	int numExtensions;

	//verificar parametros
	if (extensionName==nullptr || *extensionName=='\0')
		return false;

	//preciso do numero de extensoes
	OpenGL::glGetIntegerv(GL_NUM_EXTENSIONS, &numExtensions);
	if (numExtensions <= 0)
		return false;

	//para cada extensão disponível
	for(int curIndex=0; curIndex<numExtensions; curIndex++)
	{
		const char *curExt;

		//tiro a extensão actual
		curExt = (const char*)OpenGL::glGetStringi(GL_EXTENSIONS, curIndex);
		if ((curExt == nullptr) || (*curExt=='\0'))
			continue;

		//se for esta...
		if (stricmp(curExt, extensionName) == 0)
			return true;
	}

	//chegando aqui não encontrei nada
	return false;
}

}//namespace Extensions
}//namespace OpenGL
}//namespace HorseRadish