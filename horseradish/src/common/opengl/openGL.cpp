#include "openGL.hpp"

#include "common/stringUtils.hpp"

static HINSTANCE openGLModule;

#define GETADDR_NATIVE(var, name, type)  if ((var = (type)GetProcAddress(nativeOpenGLModule, name)) == nullptr) return false;
#define GETADDR_BASE(var, name, type)  if ((var = (type)wglProcAddressOpenGL(name)) == nullptr) return false;
#define GETADDR_BASE_OPTIONAL(var, name, type)  var = (type)wglProcAddressOpenGL(name);

typedef PROC (APIENTRY *PFNWGLGETPROCADDRESSPROC)(LPCSTR lpcstr);

static
bool loadGLFunctions(HINSTANCE nativeOpenGLModule, PFNWGLGETPROCADDRESSPROC wglProcAddressOpenGL)
{
	using namespace HorseRadish::OpenGL;

#ifdef GL_VERSION_1_0
	GETADDR_NATIVE(glCullFace, "glCullFace", PFNGLCULLFACEPROC);
	GETADDR_NATIVE(glFrontFace, "glFrontFace", PFNGLFRONTFACEPROC);
	GETADDR_NATIVE(glHint, "glHint", PFNGLHINTPROC);
	GETADDR_NATIVE(glLineWidth, "glLineWidth", PFNGLLINEWIDTHPROC);
	GETADDR_NATIVE(glPointSize, "glPointSize", PFNGLPOINTSIZEPROC);
	GETADDR_NATIVE(glPolygonMode, "glPolygonMode", PFNGLPOLYGONMODEPROC);
	GETADDR_NATIVE(glScissor, "glScissor", PFNGLSCISSORPROC);
	GETADDR_NATIVE(glTexParameterf, "glTexParameterf", PFNGLTEXPARAMETERFPROC);
	GETADDR_NATIVE(glTexParameterfv, "glTexParameterfv", PFNGLTEXPARAMETERFVPROC);
	GETADDR_NATIVE(glTexParameteri, "glTexParameteri", PFNGLTEXPARAMETERIPROC);
	GETADDR_NATIVE(glTexParameteriv, "glTexParameteriv", PFNGLTEXPARAMETERIVPROC);
	GETADDR_NATIVE(glTexImage1D, "glTexImage1D", PFNGLTEXIMAGE1DPROC);
	GETADDR_NATIVE(glTexImage2D, "glTexImage2D", PFNGLTEXIMAGE2DPROC);
	GETADDR_NATIVE(glDrawBuffer, "glDrawBuffer", PFNGLDRAWBUFFERPROC);
	GETADDR_NATIVE(glClear, "glClear", PFNGLCLEARPROC);
	GETADDR_NATIVE(glClearColor, "glClearColor", PFNGLCLEARCOLORPROC);
	GETADDR_NATIVE(glClearStencil, "glClearStencil", PFNGLCLEARSTENCILPROC);
	GETADDR_NATIVE(glClearDepth, "glClearDepth", PFNGLCLEARDEPTHPROC);
	GETADDR_NATIVE(glStencilMask, "glStencilMask", PFNGLSTENCILMASKPROC);
	GETADDR_NATIVE(glColorMask, "glColorMask", PFNGLCOLORMASKPROC);
	GETADDR_NATIVE(glDepthMask, "glDepthMask", PFNGLDEPTHMASKPROC);
	GETADDR_NATIVE(glDisable, "glDisable", PFNGLDISABLEPROC);
	GETADDR_NATIVE(glEnable, "glEnable", PFNGLENABLEPROC);
	GETADDR_NATIVE(glFinish, "glFinish", PFNGLFINISHPROC);
	GETADDR_NATIVE(glFlush, "glFlush", PFNGLFLUSHPROC);
	GETADDR_NATIVE(glBlendFunc, "glBlendFunc", PFNGLBLENDFUNCPROC);
	GETADDR_NATIVE(glLogicOp, "glLogicOp", PFNGLLOGICOPPROC);
	GETADDR_NATIVE(glStencilFunc, "glStencilFunc", PFNGLSTENCILFUNCPROC);
	GETADDR_NATIVE(glStencilOp, "glStencilOp", PFNGLSTENCILOPPROC);
	GETADDR_NATIVE(glDepthFunc, "glDepthFunc", PFNGLDEPTHFUNCPROC);
	GETADDR_NATIVE(glPixelStoref, "glPixelStoref", PFNGLPIXELSTOREFPROC);
	GETADDR_NATIVE(glPixelStorei, "glPixelStorei", PFNGLPIXELSTOREIPROC);
	GETADDR_NATIVE(glReadBuffer, "glReadBuffer", PFNGLREADBUFFERPROC);
	GETADDR_NATIVE(glReadPixels, "glReadPixels", PFNGLREADPIXELSPROC);
	GETADDR_NATIVE(glGetBooleanv, "glGetBooleanv", PFNGLGETBOOLEANVPROC);
	GETADDR_NATIVE(glGetDoublev, "glGetDoublev", PFNGLGETDOUBLEVPROC);
	GETADDR_NATIVE(glGetError, "glGetError", PFNGLGETERRORPROC);
	GETADDR_NATIVE(glGetFloatv, "glGetFloatv", PFNGLGETFLOATVPROC);
	GETADDR_NATIVE(glGetIntegerv, "glGetIntegerv", PFNGLGETINTEGERVPROC);
	GETADDR_NATIVE(glGetString, "glGetString", PFNGLGETSTRINGPROC);
	GETADDR_NATIVE(glGetTexImage, "glGetTexImage", PFNGLGETTEXIMAGEPROC);
	GETADDR_NATIVE(glGetTexParameterfv, "glGetTexParameterfv", PFNGLGETTEXPARAMETERFVPROC);
	GETADDR_NATIVE(glGetTexParameteriv, "glGetTexParameteriv", PFNGLGETTEXPARAMETERIVPROC);
	GETADDR_NATIVE(glGetTexLevelParameterfv, "glGetTexLevelParameterfv", PFNGLGETTEXLEVELPARAMETERFVPROC);
	GETADDR_NATIVE(glGetTexLevelParameteriv, "glGetTexLevelParameteriv", PFNGLGETTEXLEVELPARAMETERIVPROC);
	GETADDR_NATIVE(glIsEnabled, "glIsEnabled", PFNGLISENABLEDPROC);
	GETADDR_NATIVE(glDepthRange, "glDepthRange", PFNGLDEPTHRANGEPROC);
	GETADDR_NATIVE(glViewport, "glViewport", PFNGLVIEWPORTPROC);
#endif

#ifdef GL_VERSION_1_1
	GETADDR_NATIVE(glDrawArrays, "glDrawArrays", PFNGLDRAWARRAYSPROC);
	GETADDR_NATIVE(glDrawElements, "glDrawElements", PFNGLDRAWELEMENTSPROC);
	GETADDR_NATIVE(glGetPointerv, "glGetPointerv", PFNGLGETPOINTERVPROC);
	GETADDR_NATIVE(glPolygonOffset, "glPolygonOffset", PFNGLPOLYGONOFFSETPROC);
	GETADDR_NATIVE(glCopyTexImage1D, "glCopyTexImage1D", PFNGLCOPYTEXIMAGE1DPROC);
	GETADDR_NATIVE(glCopyTexImage2D, "glCopyTexImage2D", PFNGLCOPYTEXIMAGE2DPROC);
	GETADDR_NATIVE(glCopyTexSubImage1D, "glCopyTexSubImage1D", PFNGLCOPYTEXSUBIMAGE1DPROC);
	GETADDR_NATIVE(glCopyTexSubImage2D, "glCopyTexSubImage2D", PFNGLCOPYTEXSUBIMAGE2DPROC);
	GETADDR_NATIVE(glTexSubImage1D, "glTexSubImage1D", PFNGLTEXSUBIMAGE1DPROC);
	GETADDR_NATIVE(glTexSubImage2D, "glTexSubImage2D", PFNGLTEXSUBIMAGE2DPROC);
	GETADDR_NATIVE(glBindTexture, "glBindTexture", PFNGLBINDTEXTUREPROC);
	GETADDR_NATIVE(glDeleteTextures, "glDeleteTextures", PFNGLDELETETEXTURESPROC);
	GETADDR_NATIVE(glGenTextures, "glGenTextures", PFNGLGENTEXTURESPROC);
	GETADDR_NATIVE(glIsTexture, "glIsTexture", PFNGLISTEXTUREPROC);
#endif

#ifdef GL_VERSION_1_2
	GETADDR_BASE(glDrawRangeElements, "glDrawRangeElements", PFNGLDRAWRANGEELEMENTSPROC);
	GETADDR_BASE(glTexImage3D, "glTexImage3D", PFNGLTEXIMAGE3DPROC);
	GETADDR_BASE(glTexSubImage3D, "glTexSubImage3D", PFNGLTEXSUBIMAGE3DPROC);
	GETADDR_BASE(glCopyTexSubImage3D, "glCopyTexSubImage3D", PFNGLCOPYTEXSUBIMAGE3DPROC);
#endif

#ifdef GL_VERSION_1_3
	GETADDR_BASE(glActiveTexture, "glActiveTexture", PFNGLACTIVETEXTUREPROC);
	GETADDR_BASE(glSampleCoverage, "glSampleCoverage", PFNGLSAMPLECOVERAGEPROC);
	GETADDR_BASE(glCompressedTexImage3D, "glCompressedTexImage3D", PFNGLCOMPRESSEDTEXIMAGE3DPROC);
	GETADDR_BASE(glCompressedTexImage2D, "glCompressedTexImage2D", PFNGLCOMPRESSEDTEXIMAGE2DPROC);
	GETADDR_BASE(glCompressedTexImage1D, "glCompressedTexImage1D", PFNGLCOMPRESSEDTEXIMAGE1DPROC);
	GETADDR_BASE(glCompressedTexSubImage3D, "glCompressedTexSubImage3D", PFNGLCOMPRESSEDTEXSUBIMAGE3DPROC);
	GETADDR_BASE(glCompressedTexSubImage2D, "glCompressedTexSubImage2D", PFNGLCOMPRESSEDTEXSUBIMAGE2DPROC);
	GETADDR_BASE(glCompressedTexSubImage1D, "glCompressedTexSubImage1D", PFNGLCOMPRESSEDTEXSUBIMAGE1DPROC);
	GETADDR_BASE(glGetCompressedTexImage, "glGetCompressedTexImage", PFNGLGETCOMPRESSEDTEXIMAGEPROC);
#endif

#ifdef GL_VERSION_1_4
	GETADDR_BASE(glBlendFuncSeparate, "glBlendFuncSeparate", PFNGLBLENDFUNCSEPARATEPROC);
	GETADDR_BASE(glMultiDrawArrays, "glMultiDrawArrays", PFNGLMULTIDRAWARRAYSPROC);
	GETADDR_BASE(glMultiDrawElements, "glMultiDrawElements", PFNGLMULTIDRAWELEMENTSPROC);
	GETADDR_BASE(glPointParameterf, "glPointParameterf", PFNGLPOINTPARAMETERFPROC);
	GETADDR_BASE(glPointParameterfv, "glPointParameterfv", PFNGLPOINTPARAMETERFVPROC);
	GETADDR_BASE(glPointParameteri, "glPointParameteri", PFNGLPOINTPARAMETERIPROC);
	GETADDR_BASE(glPointParameteriv, "glPointParameteriv", PFNGLPOINTPARAMETERIVPROC);
	GETADDR_BASE(glBlendColor, "glBlendColor", PFNGLBLENDCOLORPROC);
	GETADDR_BASE(glBlendEquation, "glBlendEquation", PFNGLBLENDEQUATIONPROC);
#endif

#ifdef GL_VERSION_1_5
	GETADDR_BASE(glGenQueries, "glGenQueries", PFNGLGENQUERIESPROC);
	GETADDR_BASE(glDeleteQueries, "glDeleteQueries", PFNGLDELETEQUERIESPROC);
	GETADDR_BASE(glIsQuery, "glIsQuery", PFNGLISQUERYPROC);
	GETADDR_BASE(glBeginQuery, "glBeginQuery", PFNGLBEGINQUERYPROC);
	GETADDR_BASE(glEndQuery, "glEndQuery", PFNGLENDQUERYPROC);
	GETADDR_BASE(glGetQueryiv, "glGetQueryiv", PFNGLGETQUERYIVPROC);
	GETADDR_BASE(glGetQueryObjectiv, "glGetQueryObjectiv", PFNGLGETQUERYOBJECTIVPROC);
	GETADDR_BASE(glGetQueryObjectuiv, "glGetQueryObjectuiv", PFNGLGETQUERYOBJECTUIVPROC);
	GETADDR_BASE(glBindBuffer, "glBindBuffer", PFNGLBINDBUFFERPROC);
	GETADDR_BASE(glDeleteBuffers, "glDeleteBuffers", PFNGLDELETEBUFFERSPROC);
	GETADDR_BASE(glGenBuffers, "glGenBuffers", PFNGLGENBUFFERSPROC);
	GETADDR_BASE(glIsBuffer, "glIsBuffer", PFNGLISBUFFERPROC);
	GETADDR_BASE(glBufferData, "glBufferData", PFNGLBUFFERDATAPROC);
	GETADDR_BASE(glBufferSubData, "glBufferSubData", PFNGLBUFFERSUBDATAPROC);
	GETADDR_BASE(glGetBufferSubData, "glGetBufferSubData", PFNGLGETBUFFERSUBDATAPROC);
	GETADDR_BASE(glMapBuffer, "glMapBuffer", PFNGLMAPBUFFERPROC);
	GETADDR_BASE(glUnmapBuffer, "glUnmapBuffer", PFNGLUNMAPBUFFERPROC);
	GETADDR_BASE(glGetBufferParameteriv, "glGetBufferParameteriv", PFNGLGETBUFFERPARAMETERIVPROC);
	GETADDR_BASE(glGetBufferPointerv, "glGetBufferPointerv", PFNGLGETBUFFERPOINTERVPROC);
#endif

#ifdef GL_VERSION_2_0
	GETADDR_BASE(glBlendEquationSeparate, "glBlendEquationSeparate", PFNGLBLENDEQUATIONSEPARATEPROC);
	GETADDR_BASE(glDrawBuffers, "glDrawBuffers", PFNGLDRAWBUFFERSPROC);
	GETADDR_BASE(glStencilOpSeparate, "glStencilOpSeparate", PFNGLSTENCILOPSEPARATEPROC);
	GETADDR_BASE(glStencilFuncSeparate, "glStencilFuncSeparate", PFNGLSTENCILFUNCSEPARATEPROC);
	GETADDR_BASE(glStencilMaskSeparate, "glStencilMaskSeparate", PFNGLSTENCILMASKSEPARATEPROC);
	GETADDR_BASE(glAttachShader, "glAttachShader", PFNGLATTACHSHADERPROC);
	GETADDR_BASE(glBindAttribLocation, "glBindAttribLocation", PFNGLBINDATTRIBLOCATIONPROC);
	GETADDR_BASE(glCompileShader, "glCompileShader", PFNGLCOMPILESHADERPROC);
	GETADDR_BASE(glCreateProgram, "glCreateProgram", PFNGLCREATEPROGRAMPROC);
	GETADDR_BASE(glCreateShader, "glCreateShader", PFNGLCREATESHADERPROC);
	GETADDR_BASE(glDeleteProgram, "glDeleteProgram", PFNGLDELETEPROGRAMPROC);
	GETADDR_BASE(glDeleteShader, "glDeleteShader", PFNGLDELETESHADERPROC);
	GETADDR_BASE(glDetachShader, "glDetachShader", PFNGLDETACHSHADERPROC);
	GETADDR_BASE(glDisableVertexAttribArray, "glDisableVertexAttribArray", PFNGLDISABLEVERTEXATTRIBARRAYPROC);
	GETADDR_BASE(glEnableVertexAttribArray, "glEnableVertexAttribArray", PFNGLENABLEVERTEXATTRIBARRAYPROC);
	GETADDR_BASE(glGetActiveAttrib, "glGetActiveAttrib", PFNGLGETACTIVEATTRIBPROC);
	GETADDR_BASE(glGetActiveUniform, "glGetActiveUniform", PFNGLGETACTIVEUNIFORMPROC);
	GETADDR_BASE(glGetAttachedShaders, "glGetAttachedShaders", PFNGLGETATTACHEDSHADERSPROC);
	GETADDR_BASE(glGetAttribLocation, "glGetAttribLocation", PFNGLGETATTRIBLOCATIONPROC);
	GETADDR_BASE(glGetProgramiv, "glGetProgramiv", PFNGLGETPROGRAMIVPROC);
	GETADDR_BASE(glGetProgramInfoLog, "glGetProgramInfoLog", PFNGLGETPROGRAMINFOLOGPROC);
	GETADDR_BASE(glGetShaderiv, "glGetShaderiv", PFNGLGETSHADERIVPROC);
	GETADDR_BASE(glGetShaderInfoLog, "glGetShaderInfoLog", PFNGLGETSHADERINFOLOGPROC);
	GETADDR_BASE(glGetShaderSource, "glGetShaderSource", PFNGLGETSHADERSOURCEPROC);
	GETADDR_BASE(glGetUniformLocation, "glGetUniformLocation", PFNGLGETUNIFORMLOCATIONPROC);
	GETADDR_BASE(glGetUniformfv, "glGetUniformfv", PFNGLGETUNIFORMFVPROC);
	GETADDR_BASE(glGetUniformiv, "glGetUniformiv", PFNGLGETUNIFORMIVPROC);
	GETADDR_BASE(glGetVertexAttribdv, "glGetVertexAttribdv", PFNGLGETVERTEXATTRIBDVPROC);
	GETADDR_BASE(glGetVertexAttribfv, "glGetVertexAttribfv", PFNGLGETVERTEXATTRIBFVPROC);
	GETADDR_BASE(glGetVertexAttribiv, "glGetVertexAttribiv", PFNGLGETVERTEXATTRIBIVPROC);
	GETADDR_BASE(glGetVertexAttribPointerv, "glGetVertexAttribPointerv", PFNGLGETVERTEXATTRIBPOINTERVPROC);
	GETADDR_BASE(glIsProgram, "glIsProgram", PFNGLISPROGRAMPROC);
	GETADDR_BASE(glIsShader, "glIsShader", PFNGLISSHADERPROC);
	GETADDR_BASE(glLinkProgram, "glLinkProgram", PFNGLLINKPROGRAMPROC);
	GETADDR_BASE(glShaderSource, "glShaderSource", PFNGLSHADERSOURCEPROC);
	GETADDR_BASE(glUseProgram, "glUseProgram", PFNGLUSEPROGRAMPROC);
	GETADDR_BASE(glUniform1f, "glUniform1f", PFNGLUNIFORM1FPROC);
	GETADDR_BASE(glUniform2f, "glUniform2f", PFNGLUNIFORM2FPROC);
	GETADDR_BASE(glUniform3f, "glUniform3f", PFNGLUNIFORM3FPROC);
	GETADDR_BASE(glUniform4f, "glUniform4f", PFNGLUNIFORM4FPROC);
	GETADDR_BASE(glUniform1i, "glUniform1i", PFNGLUNIFORM1IPROC);
	GETADDR_BASE(glUniform2i, "glUniform2i", PFNGLUNIFORM2IPROC);
	GETADDR_BASE(glUniform3i, "glUniform3i", PFNGLUNIFORM3IPROC);
	GETADDR_BASE(glUniform4i, "glUniform4i", PFNGLUNIFORM4IPROC);
	GETADDR_BASE(glUniform1fv, "glUniform1fv", PFNGLUNIFORM1FVPROC);
	GETADDR_BASE(glUniform2fv, "glUniform2fv", PFNGLUNIFORM2FVPROC);
	GETADDR_BASE(glUniform3fv, "glUniform3fv", PFNGLUNIFORM3FVPROC);
	GETADDR_BASE(glUniform4fv, "glUniform4fv", PFNGLUNIFORM4FVPROC);
	GETADDR_BASE(glUniform1iv, "glUniform1iv", PFNGLUNIFORM1IVPROC);
	GETADDR_BASE(glUniform2iv, "glUniform2iv", PFNGLUNIFORM2IVPROC);
	GETADDR_BASE(glUniform3iv, "glUniform3iv", PFNGLUNIFORM3IVPROC);
	GETADDR_BASE(glUniform4iv, "glUniform4iv", PFNGLUNIFORM4IVPROC);
	GETADDR_BASE(glUniformMatrix2fv, "glUniformMatrix2fv", PFNGLUNIFORMMATRIX2FVPROC);
	GETADDR_BASE(glUniformMatrix3fv, "glUniformMatrix3fv", PFNGLUNIFORMMATRIX3FVPROC);
	GETADDR_BASE(glUniformMatrix4fv, "glUniformMatrix4fv", PFNGLUNIFORMMATRIX4FVPROC);
	GETADDR_BASE(glValidateProgram, "glValidateProgram", PFNGLVALIDATEPROGRAMPROC);
	GETADDR_BASE(glVertexAttrib1d, "glVertexAttrib1d", PFNGLVERTEXATTRIB1DPROC);
	GETADDR_BASE(glVertexAttrib1dv, "glVertexAttrib1dv", PFNGLVERTEXATTRIB1DVPROC);
	GETADDR_BASE(glVertexAttrib1f, "glVertexAttrib1f", PFNGLVERTEXATTRIB1FPROC);
	GETADDR_BASE(glVertexAttrib1fv, "glVertexAttrib1fv", PFNGLVERTEXATTRIB1FVPROC);
	GETADDR_BASE(glVertexAttrib1s, "glVertexAttrib1s", PFNGLVERTEXATTRIB1SPROC);
	GETADDR_BASE(glVertexAttrib1sv, "glVertexAttrib1sv", PFNGLVERTEXATTRIB1SVPROC);
	GETADDR_BASE(glVertexAttrib2d, "glVertexAttrib2d", PFNGLVERTEXATTRIB2DPROC);
	GETADDR_BASE(glVertexAttrib2dv, "glVertexAttrib2dv", PFNGLVERTEXATTRIB2DVPROC);
	GETADDR_BASE(glVertexAttrib2f, "glVertexAttrib2f", PFNGLVERTEXATTRIB2FPROC);
	GETADDR_BASE(glVertexAttrib2fv, "glVertexAttrib2fv", PFNGLVERTEXATTRIB2FVPROC);
	GETADDR_BASE(glVertexAttrib2s, "glVertexAttrib2s", PFNGLVERTEXATTRIB2SPROC);
	GETADDR_BASE(glVertexAttrib2sv, "glVertexAttrib2sv", PFNGLVERTEXATTRIB2SVPROC);
	GETADDR_BASE(glVertexAttrib3d, "glVertexAttrib3d", PFNGLVERTEXATTRIB3DPROC);
	GETADDR_BASE(glVertexAttrib3dv, "glVertexAttrib3dv", PFNGLVERTEXATTRIB3DVPROC);
	GETADDR_BASE(glVertexAttrib3f, "glVertexAttrib3f", PFNGLVERTEXATTRIB3FPROC);
	GETADDR_BASE(glVertexAttrib3fv, "glVertexAttrib3fv", PFNGLVERTEXATTRIB3FVPROC);
	GETADDR_BASE(glVertexAttrib3s, "glVertexAttrib3s", PFNGLVERTEXATTRIB3SPROC);
	GETADDR_BASE(glVertexAttrib3sv, "glVertexAttrib3sv", PFNGLVERTEXATTRIB3SVPROC);
	GETADDR_BASE(glVertexAttrib4Nbv, "glVertexAttrib4Nbv", PFNGLVERTEXATTRIB4NBVPROC);
	GETADDR_BASE(glVertexAttrib4Niv, "glVertexAttrib4Niv", PFNGLVERTEXATTRIB4NIVPROC);
	GETADDR_BASE(glVertexAttrib4Nsv, "glVertexAttrib4Nsv", PFNGLVERTEXATTRIB4NSVPROC);
	GETADDR_BASE(glVertexAttrib4Nub, "glVertexAttrib4Nub", PFNGLVERTEXATTRIB4NUBPROC);
	GETADDR_BASE(glVertexAttrib4Nubv, "glVertexAttrib4Nubv", PFNGLVERTEXATTRIB4NUBVPROC);
	GETADDR_BASE(glVertexAttrib4Nuiv, "glVertexAttrib4Nuiv", PFNGLVERTEXATTRIB4NUIVPROC);
	GETADDR_BASE(glVertexAttrib4Nusv, "glVertexAttrib4Nusv", PFNGLVERTEXATTRIB4NUSVPROC);
	GETADDR_BASE(glVertexAttrib4bv, "glVertexAttrib4bv", PFNGLVERTEXATTRIB4BVPROC);
	GETADDR_BASE(glVertexAttrib4d, "glVertexAttrib4d", PFNGLVERTEXATTRIB4DPROC);
	GETADDR_BASE(glVertexAttrib4dv, "glVertexAttrib4dv", PFNGLVERTEXATTRIB4DVPROC);
	GETADDR_BASE(glVertexAttrib4f, "glVertexAttrib4f", PFNGLVERTEXATTRIB4FPROC);
	GETADDR_BASE(glVertexAttrib4fv, "glVertexAttrib4fv", PFNGLVERTEXATTRIB4FVPROC);
	GETADDR_BASE(glVertexAttrib4iv, "glVertexAttrib4iv", PFNGLVERTEXATTRIB4IVPROC);
	GETADDR_BASE(glVertexAttrib4s, "glVertexAttrib4s", PFNGLVERTEXATTRIB4SPROC);
	GETADDR_BASE(glVertexAttrib4sv, "glVertexAttrib4sv", PFNGLVERTEXATTRIB4SVPROC);
	GETADDR_BASE(glVertexAttrib4ubv, "glVertexAttrib4ubv", PFNGLVERTEXATTRIB4UBVPROC);
	GETADDR_BASE(glVertexAttrib4uiv, "glVertexAttrib4uiv", PFNGLVERTEXATTRIB4UIVPROC);
	GETADDR_BASE(glVertexAttrib4usv, "glVertexAttrib4usv", PFNGLVERTEXATTRIB4USVPROC);
	GETADDR_BASE(glVertexAttribPointer, "glVertexAttribPointer", PFNGLVERTEXATTRIBPOINTERPROC);
#endif

#ifdef GL_VERSION_2_1
	GETADDR_BASE(glUniformMatrix2x3fv, "glUniformMatrix2x3fv", PFNGLUNIFORMMATRIX2X3FVPROC);
	GETADDR_BASE(glUniformMatrix3x2fv, "glUniformMatrix3x2fv", PFNGLUNIFORMMATRIX3X2FVPROC);
	GETADDR_BASE(glUniformMatrix2x4fv, "glUniformMatrix2x4fv", PFNGLUNIFORMMATRIX2X4FVPROC);
	GETADDR_BASE(glUniformMatrix4x2fv, "glUniformMatrix4x2fv", PFNGLUNIFORMMATRIX4X2FVPROC);
	GETADDR_BASE(glUniformMatrix3x4fv, "glUniformMatrix3x4fv", PFNGLUNIFORMMATRIX3X4FVPROC);
	GETADDR_BASE(glUniformMatrix4x3fv, "glUniformMatrix4x3fv", PFNGLUNIFORMMATRIX4X3FVPROC);
#endif

#ifdef GL_VERSION_3_0
	GETADDR_BASE(glColorMaski, "glColorMaski", PFNGLCOLORMASKIPROC);
	GETADDR_BASE(glGetBooleani_v, "glGetBooleani_v", PFNGLGETBOOLEANI_VPROC);
	GETADDR_BASE(glGetIntegeri_v, "glGetIntegeri_v", PFNGLGETINTEGERI_VPROC);
	GETADDR_BASE(glEnablei, "glEnablei", PFNGLENABLEIPROC);
	GETADDR_BASE(glDisablei, "glDisablei", PFNGLDISABLEIPROC);
	GETADDR_BASE(glIsEnabledi, "glIsEnabledi", PFNGLISENABLEDIPROC);
	GETADDR_BASE(glBeginTransformFeedback, "glBeginTransformFeedback", PFNGLBEGINTRANSFORMFEEDBACKPROC);
	GETADDR_BASE(glEndTransformFeedback, "glEndTransformFeedback", PFNGLENDTRANSFORMFEEDBACKPROC);
	GETADDR_BASE(glBindBufferRange, "glBindBufferRange", PFNGLBINDBUFFERRANGEPROC);
	GETADDR_BASE(glBindBufferBase, "glBindBufferBase", PFNGLBINDBUFFERBASEPROC);
	GETADDR_BASE(glTransformFeedbackVaryings, "glTransformFeedbackVaryings", PFNGLTRANSFORMFEEDBACKVARYINGSPROC);
	GETADDR_BASE(glGetTransformFeedbackVarying, "glGetTransformFeedbackVarying", PFNGLGETTRANSFORMFEEDBACKVARYINGPROC);
	GETADDR_BASE(glClampColor, "glClampColor", PFNGLCLAMPCOLORPROC);
	GETADDR_BASE(glBeginConditionalRender, "glBeginConditionalRender", PFNGLBEGINCONDITIONALRENDERPROC);
	GETADDR_BASE(glEndConditionalRender, "glEndConditionalRender", PFNGLENDCONDITIONALRENDERPROC);
	GETADDR_BASE(glVertexAttribIPointer, "glVertexAttribIPointer", PFNGLVERTEXATTRIBIPOINTERPROC);
	GETADDR_BASE(glGetVertexAttribIiv, "glGetVertexAttribIiv", PFNGLGETVERTEXATTRIBIIVPROC);
	GETADDR_BASE(glGetVertexAttribIuiv, "glGetVertexAttribIuiv", PFNGLGETVERTEXATTRIBIUIVPROC);
	GETADDR_BASE(glVertexAttribI1i, "glVertexAttribI1i", PFNGLVERTEXATTRIBI1IPROC);
	GETADDR_BASE(glVertexAttribI2i, "glVertexAttribI2i", PFNGLVERTEXATTRIBI2IPROC);
	GETADDR_BASE(glVertexAttribI3i, "glVertexAttribI3i", PFNGLVERTEXATTRIBI3IPROC);
	GETADDR_BASE(glVertexAttribI4i, "glVertexAttribI4i", PFNGLVERTEXATTRIBI4IPROC);
	GETADDR_BASE(glVertexAttribI1ui, "glVertexAttribI1ui", PFNGLVERTEXATTRIBI1UIPROC);
	GETADDR_BASE(glVertexAttribI2ui, "glVertexAttribI2ui", PFNGLVERTEXATTRIBI2UIPROC);
	GETADDR_BASE(glVertexAttribI3ui, "glVertexAttribI3ui", PFNGLVERTEXATTRIBI3UIPROC);
	GETADDR_BASE(glVertexAttribI4ui, "glVertexAttribI4ui", PFNGLVERTEXATTRIBI4UIPROC);
	GETADDR_BASE(glVertexAttribI1iv, "glVertexAttribI1iv", PFNGLVERTEXATTRIBI1IVPROC);
	GETADDR_BASE(glVertexAttribI2iv, "glVertexAttribI2iv", PFNGLVERTEXATTRIBI2IVPROC);
	GETADDR_BASE(glVertexAttribI3iv, "glVertexAttribI3iv", PFNGLVERTEXATTRIBI3IVPROC);
	GETADDR_BASE(glVertexAttribI4iv, "glVertexAttribI4iv", PFNGLVERTEXATTRIBI4IVPROC);
	GETADDR_BASE(glVertexAttribI1uiv, "glVertexAttribI1uiv", PFNGLVERTEXATTRIBI1UIVPROC);
	GETADDR_BASE(glVertexAttribI2uiv, "glVertexAttribI2uiv", PFNGLVERTEXATTRIBI2UIVPROC);
	GETADDR_BASE(glVertexAttribI3uiv, "glVertexAttribI3uiv", PFNGLVERTEXATTRIBI3UIVPROC);
	GETADDR_BASE(glVertexAttribI4uiv, "glVertexAttribI4uiv", PFNGLVERTEXATTRIBI4UIVPROC);
	GETADDR_BASE(glVertexAttribI4bv, "glVertexAttribI4bv", PFNGLVERTEXATTRIBI4BVPROC);
	GETADDR_BASE(glVertexAttribI4sv, "glVertexAttribI4sv", PFNGLVERTEXATTRIBI4SVPROC);
	GETADDR_BASE(glVertexAttribI4ubv, "glVertexAttribI4ubv", PFNGLVERTEXATTRIBI4UBVPROC);
	GETADDR_BASE(glVertexAttribI4usv, "glVertexAttribI4usv", PFNGLVERTEXATTRIBI4USVPROC);
	GETADDR_BASE(glGetUniformuiv, "glGetUniformuiv", PFNGLGETUNIFORMUIVPROC);
	GETADDR_BASE(glBindFragDataLocation, "glBindFragDataLocation", PFNGLBINDFRAGDATALOCATIONPROC);
	GETADDR_BASE(glGetFragDataLocation, "glGetFragDataLocation", PFNGLGETFRAGDATALOCATIONPROC);
	GETADDR_BASE(glUniform1ui, "glUniform1ui", PFNGLUNIFORM1UIPROC);
	GETADDR_BASE(glUniform2ui, "glUniform2ui", PFNGLUNIFORM2UIPROC);
	GETADDR_BASE(glUniform3ui, "glUniform3ui", PFNGLUNIFORM3UIPROC);
	GETADDR_BASE(glUniform4ui, "glUniform4ui", PFNGLUNIFORM4UIPROC);
	GETADDR_BASE(glUniform1uiv, "glUniform1uiv", PFNGLUNIFORM1UIVPROC);
	GETADDR_BASE(glUniform2uiv, "glUniform2uiv", PFNGLUNIFORM2UIVPROC);
	GETADDR_BASE(glUniform3uiv, "glUniform3uiv", PFNGLUNIFORM3UIVPROC);
	GETADDR_BASE(glUniform4uiv, "glUniform4uiv", PFNGLUNIFORM4UIVPROC);
	GETADDR_BASE(glTexParameterIiv, "glTexParameterIiv", PFNGLTEXPARAMETERIIVPROC);
	GETADDR_BASE(glTexParameterIuiv, "glTexParameterIuiv", PFNGLTEXPARAMETERIUIVPROC);
	GETADDR_BASE(glGetTexParameterIiv, "glGetTexParameterIiv", PFNGLGETTEXPARAMETERIIVPROC);
	GETADDR_BASE(glGetTexParameterIuiv, "glGetTexParameterIuiv", PFNGLGETTEXPARAMETERIUIVPROC);
	GETADDR_BASE(glClearBufferiv, "glClearBufferiv", PFNGLCLEARBUFFERIVPROC);
	GETADDR_BASE(glClearBufferuiv, "glClearBufferuiv", PFNGLCLEARBUFFERUIVPROC);
	GETADDR_BASE(glClearBufferfv, "glClearBufferfv", PFNGLCLEARBUFFERFVPROC);
	GETADDR_BASE(glClearBufferfi, "glClearBufferfi", PFNGLCLEARBUFFERFIPROC);
	GETADDR_BASE(glGetStringi, "glGetStringi", PFNGLGETSTRINGIPROC);
	GETADDR_BASE(glIsRenderbuffer, "glIsRenderbuffer", PFNGLISRENDERBUFFERPROC);
	GETADDR_BASE(glBindRenderbuffer, "glBindRenderbuffer", PFNGLBINDRENDERBUFFERPROC);
	GETADDR_BASE(glDeleteRenderbuffers, "glDeleteRenderbuffers", PFNGLDELETERENDERBUFFERSPROC);
	GETADDR_BASE(glGenRenderbuffers, "glGenRenderbuffers", PFNGLGENRENDERBUFFERSPROC);
	GETADDR_BASE(glRenderbufferStorage, "glRenderbufferStorage", PFNGLRENDERBUFFERSTORAGEPROC);
	GETADDR_BASE(glGetRenderbufferParameteriv, "glGetRenderbufferParameteriv", PFNGLGETRENDERBUFFERPARAMETERIVPROC);
	GETADDR_BASE(glIsFramebuffer, "glIsFramebuffer", PFNGLISFRAMEBUFFERPROC);
	GETADDR_BASE(glBindFramebuffer, "glBindFramebuffer", PFNGLBINDFRAMEBUFFERPROC);
	GETADDR_BASE(glDeleteFramebuffers, "glDeleteFramebuffers", PFNGLDELETEFRAMEBUFFERSPROC);
	GETADDR_BASE(glGenFramebuffers, "glGenFramebuffers", PFNGLGENFRAMEBUFFERSPROC);
	GETADDR_BASE(glCheckFramebufferStatus, "glCheckFramebufferStatus", PFNGLCHECKFRAMEBUFFERSTATUSPROC);
	GETADDR_BASE(glFramebufferTexture1D, "glFramebufferTexture1D", PFNGLFRAMEBUFFERTEXTURE1DPROC);
	GETADDR_BASE(glFramebufferTexture2D, "glFramebufferTexture2D", PFNGLFRAMEBUFFERTEXTURE2DPROC);
	GETADDR_BASE(glFramebufferTexture3D, "glFramebufferTexture3D", PFNGLFRAMEBUFFERTEXTURE3DPROC);
	GETADDR_BASE(glFramebufferRenderbuffer, "glFramebufferRenderbuffer", PFNGLFRAMEBUFFERRENDERBUFFERPROC);
	GETADDR_BASE(glGetFramebufferAttachmentParameteriv, "glGetFramebufferAttachmentParameteriv", PFNGLGETFRAMEBUFFERATTACHMENTPARAMETERIVPROC);
	GETADDR_BASE(glGenerateMipmap, "glGenerateMipmap", PFNGLGENERATEMIPMAPPROC);
	GETADDR_BASE(glBlitFramebuffer, "glBlitFramebuffer", PFNGLBLITFRAMEBUFFERPROC);
	GETADDR_BASE(glRenderbufferStorageMultisample, "glRenderbufferStorageMultisample", PFNGLRENDERBUFFERSTORAGEMULTISAMPLEPROC);
	GETADDR_BASE(glFramebufferTextureLayer, "glFramebufferTextureLayer", PFNGLFRAMEBUFFERTEXTURELAYERPROC);
	GETADDR_BASE(glMapBufferRange, "glMapBufferRange", PFNGLMAPBUFFERRANGEPROC);
	GETADDR_BASE(glFlushMappedBufferRange, "glFlushMappedBufferRange", PFNGLFLUSHMAPPEDBUFFERRANGEPROC);
	GETADDR_BASE(glBindVertexArray, "glBindVertexArray", PFNGLBINDVERTEXARRAYPROC);
	GETADDR_BASE(glDeleteVertexArrays, "glDeleteVertexArrays", PFNGLDELETEVERTEXARRAYSPROC);
	GETADDR_BASE(glGenVertexArrays, "glGenVertexArrays", PFNGLGENVERTEXARRAYSPROC);
	GETADDR_BASE(glIsVertexArray, "glIsVertexArray", PFNGLISVERTEXARRAYPROC);
#endif

#ifdef GL_VERSION_3_1
	GETADDR_BASE(glDrawArraysInstanced, "glDrawArraysInstanced", PFNGLDRAWARRAYSINSTANCEDPROC);
	GETADDR_BASE(glDrawElementsInstanced, "glDrawElementsInstanced", PFNGLDRAWELEMENTSINSTANCEDPROC);
	GETADDR_BASE(glTexBuffer, "glTexBuffer", PFNGLTEXBUFFERPROC);
	GETADDR_BASE(glPrimitiveRestartIndex, "glPrimitiveRestartIndex", PFNGLPRIMITIVERESTARTINDEXPROC);
	GETADDR_BASE(glCopyBufferSubData, "glCopyBufferSubData", PFNGLCOPYBUFFERSUBDATAPROC);
	GETADDR_BASE(glGetUniformIndices, "glGetUniformIndices", PFNGLGETUNIFORMINDICESPROC);
	GETADDR_BASE(glGetActiveUniformsiv, "glGetActiveUniformsiv", PFNGLGETACTIVEUNIFORMSIVPROC);
	GETADDR_BASE(glGetActiveUniformName, "glGetActiveUniformName", PFNGLGETACTIVEUNIFORMNAMEPROC);
	GETADDR_BASE(glGetUniformBlockIndex, "glGetUniformBlockIndex", PFNGLGETUNIFORMBLOCKINDEXPROC);
	GETADDR_BASE(glGetActiveUniformBlockiv, "glGetActiveUniformBlockiv", PFNGLGETACTIVEUNIFORMBLOCKIVPROC);
	GETADDR_BASE(glGetActiveUniformBlockName, "glGetActiveUniformBlockName", PFNGLGETACTIVEUNIFORMBLOCKNAMEPROC);
	GETADDR_BASE(glUniformBlockBinding, "glUniformBlockBinding", PFNGLUNIFORMBLOCKBINDINGPROC);
#endif

#ifdef GL_VERSION_3_2
	GETADDR_BASE(glDrawElementsBaseVertex, "glDrawElementsBaseVertex", PFNGLDRAWELEMENTSBASEVERTEXPROC);
	GETADDR_BASE(glDrawRangeElementsBaseVertex, "glDrawRangeElementsBaseVertex", PFNGLDRAWRANGEELEMENTSBASEVERTEXPROC);
	GETADDR_BASE(glDrawElementsInstancedBaseVertex, "glDrawElementsInstancedBaseVertex", PFNGLDRAWELEMENTSINSTANCEDBASEVERTEXPROC);
	GETADDR_BASE(glMultiDrawElementsBaseVertex, "glMultiDrawElementsBaseVertex", PFNGLMULTIDRAWELEMENTSBASEVERTEXPROC);
	GETADDR_BASE(glProvokingVertex, "glProvokingVertex", PFNGLPROVOKINGVERTEXPROC);
	GETADDR_BASE(glFenceSync, "glFenceSync", PFNGLFENCESYNCPROC);
	GETADDR_BASE(glIsSync, "glIsSync", PFNGLISSYNCPROC);
	GETADDR_BASE(glDeleteSync, "glDeleteSync", PFNGLDELETESYNCPROC);
	GETADDR_BASE(glClientWaitSync, "glClientWaitSync", PFNGLCLIENTWAITSYNCPROC);
	GETADDR_BASE(glWaitSync, "glWaitSync", PFNGLWAITSYNCPROC);
	GETADDR_BASE(glGetInteger64v, "glGetInteger64v", PFNGLGETINTEGER64VPROC);
	GETADDR_BASE(glGetSynciv, "glGetSynciv", PFNGLGETSYNCIVPROC);
	GETADDR_BASE(glGetInteger64i_v, "glGetInteger64i_v", PFNGLGETINTEGER64I_VPROC);
	GETADDR_BASE(glGetBufferParameteri64v, "glGetBufferParameteri64v", PFNGLGETBUFFERPARAMETERI64VPROC);
	GETADDR_BASE(glFramebufferTexture, "glFramebufferTexture", PFNGLFRAMEBUFFERTEXTUREPROC);
	GETADDR_BASE(glTexImage2DMultisample, "glTexImage2DMultisample", PFNGLTEXIMAGE2DMULTISAMPLEPROC);
	GETADDR_BASE(glTexImage3DMultisample, "glTexImage3DMultisample", PFNGLTEXIMAGE3DMULTISAMPLEPROC);
	GETADDR_BASE(glGetMultisamplefv, "glGetMultisamplefv", PFNGLGETMULTISAMPLEFVPROC);
	GETADDR_BASE(glSampleMaski, "glSampleMaski", PFNGLSAMPLEMASKIPROC);
#endif

#ifdef GL_VERSION_3_3
	GETADDR_BASE(glBindFragDataLocationIndexed, "glBindFragDataLocationIndexed", PFNGLBINDFRAGDATALOCATIONINDEXEDPROC);
	GETADDR_BASE(glGetFragDataIndex, "glGetFragDataIndex", PFNGLGETFRAGDATAINDEXPROC);
	GETADDR_BASE(glGenSamplers, "glGenSamplers", PFNGLGENSAMPLERSPROC);
	GETADDR_BASE(glDeleteSamplers, "glDeleteSamplers", PFNGLDELETESAMPLERSPROC);
	GETADDR_BASE(glIsSampler, "glIsSampler", PFNGLISSAMPLERPROC);
	GETADDR_BASE(glBindSampler, "glBindSampler", PFNGLBINDSAMPLERPROC);
	GETADDR_BASE(glSamplerParameteri, "glSamplerParameteri", PFNGLSAMPLERPARAMETERIPROC);
	GETADDR_BASE(glSamplerParameteriv, "glSamplerParameteriv", PFNGLSAMPLERPARAMETERIVPROC);
	GETADDR_BASE(glSamplerParameterf, "glSamplerParameterf", PFNGLSAMPLERPARAMETERFPROC);
	GETADDR_BASE(glSamplerParameterfv, "glSamplerParameterfv", PFNGLSAMPLERPARAMETERFVPROC);
	GETADDR_BASE(glSamplerParameterIiv, "glSamplerParameterIiv", PFNGLSAMPLERPARAMETERIIVPROC);
	GETADDR_BASE(glSamplerParameterIuiv, "glSamplerParameterIuiv", PFNGLSAMPLERPARAMETERIUIVPROC);
	GETADDR_BASE(glGetSamplerParameteriv, "glGetSamplerParameteriv", PFNGLGETSAMPLERPARAMETERIVPROC);
	GETADDR_BASE(glGetSamplerParameterIiv, "glGetSamplerParameterIiv", PFNGLGETSAMPLERPARAMETERIIVPROC);
	GETADDR_BASE(glGetSamplerParameterfv, "glGetSamplerParameterfv", PFNGLGETSAMPLERPARAMETERFVPROC);
	GETADDR_BASE(glGetSamplerParameterIuiv, "glGetSamplerParameterIuiv", PFNGLGETSAMPLERPARAMETERIUIVPROC);
	GETADDR_BASE(glQueryCounter, "glQueryCounter", PFNGLQUERYCOUNTERPROC);
	GETADDR_BASE(glGetQueryObjecti64v, "glGetQueryObjecti64v", PFNGLGETQUERYOBJECTI64VPROC);
	GETADDR_BASE(glGetQueryObjectui64v, "glGetQueryObjectui64v", PFNGLGETQUERYOBJECTUI64VPROC);
	GETADDR_BASE(glVertexAttribDivisor, "glVertexAttribDivisor", PFNGLVERTEXATTRIBDIVISORPROC);
	GETADDR_BASE(glVertexAttribP1ui, "glVertexAttribP1ui", PFNGLVERTEXATTRIBP1UIPROC);
	GETADDR_BASE(glVertexAttribP1uiv, "glVertexAttribP1uiv", PFNGLVERTEXATTRIBP1UIVPROC);
	GETADDR_BASE(glVertexAttribP2ui, "glVertexAttribP2ui", PFNGLVERTEXATTRIBP2UIPROC);
	GETADDR_BASE(glVertexAttribP2uiv, "glVertexAttribP2uiv", PFNGLVERTEXATTRIBP2UIVPROC);
	GETADDR_BASE(glVertexAttribP3ui, "glVertexAttribP3ui", PFNGLVERTEXATTRIBP3UIPROC);
	GETADDR_BASE(glVertexAttribP3uiv, "glVertexAttribP3uiv", PFNGLVERTEXATTRIBP3UIVPROC);
	GETADDR_BASE(glVertexAttribP4ui, "glVertexAttribP4ui", PFNGLVERTEXATTRIBP4UIPROC);
	GETADDR_BASE(glVertexAttribP4uiv, "glVertexAttribP4uiv", PFNGLVERTEXATTRIBP4UIVPROC);
#endif

#ifdef GL_VERSION_4_0
	GETADDR_BASE(glMinSampleShading, "glMinSampleShading", PFNGLMINSAMPLESHADINGPROC);
	GETADDR_BASE(glBlendEquationi, "glBlendEquationi", PFNGLBLENDEQUATIONIPROC);
	GETADDR_BASE(glBlendEquationSeparatei, "glBlendEquationSeparatei", PFNGLBLENDEQUATIONSEPARATEIPROC);
	GETADDR_BASE(glBlendFunci, "glBlendFunci", PFNGLBLENDFUNCIPROC);
	GETADDR_BASE(glBlendFuncSeparatei, "glBlendFuncSeparatei", PFNGLBLENDFUNCSEPARATEIPROC);
	GETADDR_BASE(glDrawArraysIndirect, "glDrawArraysIndirect", PFNGLDRAWARRAYSINDIRECTPROC);
	GETADDR_BASE(glDrawElementsIndirect, "glDrawElementsIndirect", PFNGLDRAWELEMENTSINDIRECTPROC);
	GETADDR_BASE(glUniform1d, "glUniform1d", PFNGLUNIFORM1DPROC);
	GETADDR_BASE(glUniform2d, "glUniform2d", PFNGLUNIFORM2DPROC);
	GETADDR_BASE(glUniform3d, "glUniform3d", PFNGLUNIFORM3DPROC);
	GETADDR_BASE(glUniform4d, "glUniform4d", PFNGLUNIFORM4DPROC);
	GETADDR_BASE(glUniform1dv, "glUniform1dv", PFNGLUNIFORM1DVPROC);
	GETADDR_BASE(glUniform2dv, "glUniform2dv", PFNGLUNIFORM2DVPROC);
	GETADDR_BASE(glUniform3dv, "glUniform3dv", PFNGLUNIFORM3DVPROC);
	GETADDR_BASE(glUniform4dv, "glUniform4dv", PFNGLUNIFORM4DVPROC);
	GETADDR_BASE(glUniformMatrix2dv, "glUniformMatrix2dv", PFNGLUNIFORMMATRIX2DVPROC);
	GETADDR_BASE(glUniformMatrix3dv, "glUniformMatrix3dv", PFNGLUNIFORMMATRIX3DVPROC);
	GETADDR_BASE(glUniformMatrix4dv, "glUniformMatrix4dv", PFNGLUNIFORMMATRIX4DVPROC);
	GETADDR_BASE(glUniformMatrix2x3dv, "glUniformMatrix2x3dv", PFNGLUNIFORMMATRIX2X3DVPROC);
	GETADDR_BASE(glUniformMatrix2x4dv, "glUniformMatrix2x4dv", PFNGLUNIFORMMATRIX2X4DVPROC);
	GETADDR_BASE(glUniformMatrix3x2dv, "glUniformMatrix3x2dv", PFNGLUNIFORMMATRIX3X2DVPROC);
	GETADDR_BASE(glUniformMatrix3x4dv, "glUniformMatrix3x4dv", PFNGLUNIFORMMATRIX3X4DVPROC);
	GETADDR_BASE(glUniformMatrix4x2dv, "glUniformMatrix4x2dv", PFNGLUNIFORMMATRIX4X2DVPROC);
	GETADDR_BASE(glUniformMatrix4x3dv, "glUniformMatrix4x3dv", PFNGLUNIFORMMATRIX4X3DVPROC);
	GETADDR_BASE(glGetUniformdv, "glGetUniformdv", PFNGLGETUNIFORMDVPROC);
	GETADDR_BASE(glGetSubroutineUniformLocation, "glGetSubroutineUniformLocation", PFNGLGETSUBROUTINEUNIFORMLOCATIONPROC);
	GETADDR_BASE(glGetSubroutineIndex, "glGetSubroutineIndex", PFNGLGETSUBROUTINEINDEXPROC);
	GETADDR_BASE(glGetActiveSubroutineUniformiv, "glGetActiveSubroutineUniformiv", PFNGLGETACTIVESUBROUTINEUNIFORMIVPROC);
	GETADDR_BASE(glGetActiveSubroutineUniformName, "glGetActiveSubroutineUniformName", PFNGLGETACTIVESUBROUTINEUNIFORMNAMEPROC);
	GETADDR_BASE(glGetActiveSubroutineName, "glGetActiveSubroutineName", PFNGLGETACTIVESUBROUTINENAMEPROC);
	GETADDR_BASE(glUniformSubroutinesuiv, "glUniformSubroutinesuiv", PFNGLUNIFORMSUBROUTINESUIVPROC);
	GETADDR_BASE(glGetUniformSubroutineuiv, "glGetUniformSubroutineuiv", PFNGLGETUNIFORMSUBROUTINEUIVPROC);
	GETADDR_BASE(glGetProgramStageiv, "glGetProgramStageiv", PFNGLGETPROGRAMSTAGEIVPROC);
	GETADDR_BASE(glPatchParameteri, "glPatchParameteri", PFNGLPATCHPARAMETERIPROC);
	GETADDR_BASE(glPatchParameterfv, "glPatchParameterfv", PFNGLPATCHPARAMETERFVPROC);
	GETADDR_BASE(glBindTransformFeedback, "glBindTransformFeedback", PFNGLBINDTRANSFORMFEEDBACKPROC);
	GETADDR_BASE(glDeleteTransformFeedbacks, "glDeleteTransformFeedbacks", PFNGLDELETETRANSFORMFEEDBACKSPROC);
	GETADDR_BASE(glGenTransformFeedbacks, "glGenTransformFeedbacks", PFNGLGENTRANSFORMFEEDBACKSPROC);
	GETADDR_BASE(glIsTransformFeedback, "glIsTransformFeedback", PFNGLISTRANSFORMFEEDBACKPROC);
	GETADDR_BASE(glPauseTransformFeedback, "glPauseTransformFeedback", PFNGLPAUSETRANSFORMFEEDBACKPROC);
	GETADDR_BASE(glResumeTransformFeedback, "glResumeTransformFeedback", PFNGLRESUMETRANSFORMFEEDBACKPROC);
	GETADDR_BASE(glDrawTransformFeedback, "glDrawTransformFeedback", PFNGLDRAWTRANSFORMFEEDBACKPROC);
	GETADDR_BASE(glDrawTransformFeedbackStream, "glDrawTransformFeedbackStream", PFNGLDRAWTRANSFORMFEEDBACKSTREAMPROC);
	GETADDR_BASE(glBeginQueryIndexed, "glBeginQueryIndexed", PFNGLBEGINQUERYINDEXEDPROC);
	GETADDR_BASE(glEndQueryIndexed, "glEndQueryIndexed", PFNGLENDQUERYINDEXEDPROC);
	GETADDR_BASE(glGetQueryIndexediv, "glGetQueryIndexediv", PFNGLGETQUERYINDEXEDIVPROC);
#endif

#ifdef GL_VERSION_4_1
	GETADDR_BASE(glReleaseShaderCompiler, "glReleaseShaderCompiler", PFNGLRELEASESHADERCOMPILERPROC);
	GETADDR_BASE(glShaderBinary, "glShaderBinary", PFNGLSHADERBINARYPROC);
	GETADDR_BASE(glGetShaderPrecisionFormat, "glGetShaderPrecisionFormat", PFNGLGETSHADERPRECISIONFORMATPROC);
	GETADDR_BASE(glDepthRangef, "glDepthRangef", PFNGLDEPTHRANGEFPROC);
	GETADDR_BASE(glClearDepthf, "glClearDepthf", PFNGLCLEARDEPTHFPROC);
	GETADDR_BASE(glGetProgramBinary, "glGetProgramBinary", PFNGLGETPROGRAMBINARYPROC);
	GETADDR_BASE(glProgramBinary, "glProgramBinary", PFNGLPROGRAMBINARYPROC);
	GETADDR_BASE(glProgramParameteri, "glProgramParameteri", PFNGLPROGRAMPARAMETERIPROC);
	GETADDR_BASE(glUseProgramStages, "glUseProgramStages", PFNGLUSEPROGRAMSTAGESPROC);
	GETADDR_BASE(glActiveShaderProgram, "glActiveShaderProgram", PFNGLACTIVESHADERPROGRAMPROC);
	GETADDR_BASE(glCreateShaderProgramv, "glCreateShaderProgramv", PFNGLCREATESHADERPROGRAMVPROC);
	GETADDR_BASE(glBindProgramPipeline, "glBindProgramPipeline", PFNGLBINDPROGRAMPIPELINEPROC);
	GETADDR_BASE(glDeleteProgramPipelines, "glDeleteProgramPipelines", PFNGLDELETEPROGRAMPIPELINESPROC);
	GETADDR_BASE(glGenProgramPipelines, "glGenProgramPipelines", PFNGLGENPROGRAMPIPELINESPROC);
	GETADDR_BASE(glIsProgramPipeline, "glIsProgramPipeline", PFNGLISPROGRAMPIPELINEPROC);
	GETADDR_BASE(glGetProgramPipelineiv, "glGetProgramPipelineiv", PFNGLGETPROGRAMPIPELINEIVPROC);
	GETADDR_BASE(glProgramUniform1i, "glProgramUniform1i", PFNGLPROGRAMUNIFORM1IPROC);
	GETADDR_BASE(glProgramUniform1iv, "glProgramUniform1iv", PFNGLPROGRAMUNIFORM1IVPROC);
	GETADDR_BASE(glProgramUniform1f, "glProgramUniform1f", PFNGLPROGRAMUNIFORM1FPROC);
	GETADDR_BASE(glProgramUniform1fv, "glProgramUniform1fv", PFNGLPROGRAMUNIFORM1FVPROC);
	GETADDR_BASE(glProgramUniform1d, "glProgramUniform1d", PFNGLPROGRAMUNIFORM1DPROC);
	GETADDR_BASE(glProgramUniform1dv, "glProgramUniform1dv", PFNGLPROGRAMUNIFORM1DVPROC);
	GETADDR_BASE(glProgramUniform1ui, "glProgramUniform1ui", PFNGLPROGRAMUNIFORM1UIPROC);
	GETADDR_BASE(glProgramUniform1uiv, "glProgramUniform1uiv", PFNGLPROGRAMUNIFORM1UIVPROC);
	GETADDR_BASE(glProgramUniform2i, "glProgramUniform2i", PFNGLPROGRAMUNIFORM2IPROC);
	GETADDR_BASE(glProgramUniform2iv, "glProgramUniform2iv", PFNGLPROGRAMUNIFORM2IVPROC);
	GETADDR_BASE(glProgramUniform2f, "glProgramUniform2f", PFNGLPROGRAMUNIFORM2FPROC);
	GETADDR_BASE(glProgramUniform2fv, "glProgramUniform2fv", PFNGLPROGRAMUNIFORM2FVPROC);
	GETADDR_BASE(glProgramUniform2d, "glProgramUniform2d", PFNGLPROGRAMUNIFORM2DPROC);
	GETADDR_BASE(glProgramUniform2dv, "glProgramUniform2dv", PFNGLPROGRAMUNIFORM2DVPROC);
	GETADDR_BASE(glProgramUniform2ui, "glProgramUniform2ui", PFNGLPROGRAMUNIFORM2UIPROC);
	GETADDR_BASE(glProgramUniform2uiv, "glProgramUniform2uiv", PFNGLPROGRAMUNIFORM2UIVPROC);
	GETADDR_BASE(glProgramUniform3i, "glProgramUniform3i", PFNGLPROGRAMUNIFORM3IPROC);
	GETADDR_BASE(glProgramUniform3iv, "glProgramUniform3iv", PFNGLPROGRAMUNIFORM3IVPROC);
	GETADDR_BASE(glProgramUniform3f, "glProgramUniform3f", PFNGLPROGRAMUNIFORM3FPROC);
	GETADDR_BASE(glProgramUniform3fv, "glProgramUniform3fv", PFNGLPROGRAMUNIFORM3FVPROC);
	GETADDR_BASE(glProgramUniform3d, "glProgramUniform3d", PFNGLPROGRAMUNIFORM3DPROC);
	GETADDR_BASE(glProgramUniform3dv, "glProgramUniform3dv", PFNGLPROGRAMUNIFORM3DVPROC);
	GETADDR_BASE(glProgramUniform3ui, "glProgramUniform3ui", PFNGLPROGRAMUNIFORM3UIPROC);
	GETADDR_BASE(glProgramUniform3uiv, "glProgramUniform3uiv", PFNGLPROGRAMUNIFORM3UIVPROC);
	GETADDR_BASE(glProgramUniform4i, "glProgramUniform4i", PFNGLPROGRAMUNIFORM4IPROC);
	GETADDR_BASE(glProgramUniform4iv, "glProgramUniform4iv", PFNGLPROGRAMUNIFORM4IVPROC);
	GETADDR_BASE(glProgramUniform4f, "glProgramUniform4f", PFNGLPROGRAMUNIFORM4FPROC);
	GETADDR_BASE(glProgramUniform4fv, "glProgramUniform4fv", PFNGLPROGRAMUNIFORM4FVPROC);
	GETADDR_BASE(glProgramUniform4d, "glProgramUniform4d", PFNGLPROGRAMUNIFORM4DPROC);
	GETADDR_BASE(glProgramUniform4dv, "glProgramUniform4dv", PFNGLPROGRAMUNIFORM4DVPROC);
	GETADDR_BASE(glProgramUniform4ui, "glProgramUniform4ui", PFNGLPROGRAMUNIFORM4UIPROC);
	GETADDR_BASE(glProgramUniform4uiv, "glProgramUniform4uiv", PFNGLPROGRAMUNIFORM4UIVPROC);
	GETADDR_BASE(glProgramUniformMatrix2fv, "glProgramUniformMatrix2fv", PFNGLPROGRAMUNIFORMMATRIX2FVPROC);
	GETADDR_BASE(glProgramUniformMatrix3fv, "glProgramUniformMatrix3fv", PFNGLPROGRAMUNIFORMMATRIX3FVPROC);
	GETADDR_BASE(glProgramUniformMatrix4fv, "glProgramUniformMatrix4fv", PFNGLPROGRAMUNIFORMMATRIX4FVPROC);
	GETADDR_BASE(glProgramUniformMatrix2dv, "glProgramUniformMatrix2dv", PFNGLPROGRAMUNIFORMMATRIX2DVPROC);
	GETADDR_BASE(glProgramUniformMatrix3dv, "glProgramUniformMatrix3dv", PFNGLPROGRAMUNIFORMMATRIX3DVPROC);
	GETADDR_BASE(glProgramUniformMatrix4dv, "glProgramUniformMatrix4dv", PFNGLPROGRAMUNIFORMMATRIX4DVPROC);
	GETADDR_BASE(glProgramUniformMatrix2x3fv, "glProgramUniformMatrix2x3fv", PFNGLPROGRAMUNIFORMMATRIX2X3FVPROC);
	GETADDR_BASE(glProgramUniformMatrix3x2fv, "glProgramUniformMatrix3x2fv", PFNGLPROGRAMUNIFORMMATRIX3X2FVPROC);
	GETADDR_BASE(glProgramUniformMatrix2x4fv, "glProgramUniformMatrix2x4fv", PFNGLPROGRAMUNIFORMMATRIX2X4FVPROC);
	GETADDR_BASE(glProgramUniformMatrix4x2fv, "glProgramUniformMatrix4x2fv", PFNGLPROGRAMUNIFORMMATRIX4X2FVPROC);
	GETADDR_BASE(glProgramUniformMatrix3x4fv, "glProgramUniformMatrix3x4fv", PFNGLPROGRAMUNIFORMMATRIX3X4FVPROC);
	GETADDR_BASE(glProgramUniformMatrix4x3fv, "glProgramUniformMatrix4x3fv", PFNGLPROGRAMUNIFORMMATRIX4X3FVPROC);
	GETADDR_BASE(glProgramUniformMatrix2x3dv, "glProgramUniformMatrix2x3dv", PFNGLPROGRAMUNIFORMMATRIX2X3DVPROC);
	GETADDR_BASE(glProgramUniformMatrix3x2dv, "glProgramUniformMatrix3x2dv", PFNGLPROGRAMUNIFORMMATRIX3X2DVPROC);
	GETADDR_BASE(glProgramUniformMatrix2x4dv, "glProgramUniformMatrix2x4dv", PFNGLPROGRAMUNIFORMMATRIX2X4DVPROC);
	GETADDR_BASE(glProgramUniformMatrix4x2dv, "glProgramUniformMatrix4x2dv", PFNGLPROGRAMUNIFORMMATRIX4X2DVPROC);
	GETADDR_BASE(glProgramUniformMatrix3x4dv, "glProgramUniformMatrix3x4dv", PFNGLPROGRAMUNIFORMMATRIX3X4DVPROC);
	GETADDR_BASE(glProgramUniformMatrix4x3dv, "glProgramUniformMatrix4x3dv", PFNGLPROGRAMUNIFORMMATRIX4X3DVPROC);
	GETADDR_BASE(glValidateProgramPipeline, "glValidateProgramPipeline", PFNGLVALIDATEPROGRAMPIPELINEPROC);
	GETADDR_BASE(glGetProgramPipelineInfoLog, "glGetProgramPipelineInfoLog", PFNGLGETPROGRAMPIPELINEINFOLOGPROC);
	GETADDR_BASE(glVertexAttribL1d, "glVertexAttribL1d", PFNGLVERTEXATTRIBL1DPROC);
	GETADDR_BASE(glVertexAttribL2d, "glVertexAttribL2d", PFNGLVERTEXATTRIBL2DPROC);
	GETADDR_BASE(glVertexAttribL3d, "glVertexAttribL3d", PFNGLVERTEXATTRIBL3DPROC);
	GETADDR_BASE(glVertexAttribL4d, "glVertexAttribL4d", PFNGLVERTEXATTRIBL4DPROC);
	GETADDR_BASE(glVertexAttribL1dv, "glVertexAttribL1dv", PFNGLVERTEXATTRIBL1DVPROC);
	GETADDR_BASE(glVertexAttribL2dv, "glVertexAttribL2dv", PFNGLVERTEXATTRIBL2DVPROC);
	GETADDR_BASE(glVertexAttribL3dv, "glVertexAttribL3dv", PFNGLVERTEXATTRIBL3DVPROC);
	GETADDR_BASE(glVertexAttribL4dv, "glVertexAttribL4dv", PFNGLVERTEXATTRIBL4DVPROC);
	GETADDR_BASE(glVertexAttribLPointer, "glVertexAttribLPointer", PFNGLVERTEXATTRIBLPOINTERPROC);
	GETADDR_BASE(glGetVertexAttribLdv, "glGetVertexAttribLdv", PFNGLGETVERTEXATTRIBLDVPROC);
	GETADDR_BASE(glViewportArrayv, "glViewportArrayv", PFNGLVIEWPORTARRAYVPROC);
	GETADDR_BASE(glViewportIndexedf, "glViewportIndexedf", PFNGLVIEWPORTINDEXEDFPROC);
	GETADDR_BASE(glViewportIndexedfv, "glViewportIndexedfv", PFNGLVIEWPORTINDEXEDFVPROC);
	GETADDR_BASE(glScissorArrayv, "glScissorArrayv", PFNGLSCISSORARRAYVPROC);
	GETADDR_BASE(glScissorIndexed, "glScissorIndexed", PFNGLSCISSORINDEXEDPROC);
	GETADDR_BASE(glScissorIndexedv, "glScissorIndexedv", PFNGLSCISSORINDEXEDVPROC);
	GETADDR_BASE(glDepthRangeArrayv, "glDepthRangeArrayv", PFNGLDEPTHRANGEARRAYVPROC);
	GETADDR_BASE(glDepthRangeIndexed, "glDepthRangeIndexed", PFNGLDEPTHRANGEINDEXEDPROC);
	GETADDR_BASE(glGetFloati_v, "glGetFloati_v", PFNGLGETFLOATI_VPROC);
	GETADDR_BASE(glGetDoublei_v, "glGetDoublei_v", PFNGLGETDOUBLEI_VPROC);
#endif

#ifdef GL_VERSION_4_2
	GETADDR_BASE(glDrawArraysInstancedBaseInstance, "glDrawArraysInstancedBaseInstance", PFNGLDRAWARRAYSINSTANCEDBASEINSTANCEPROC);
	GETADDR_BASE(glDrawElementsInstancedBaseInstance, "glDrawElementsInstancedBaseInstance", PFNGLDRAWELEMENTSINSTANCEDBASEINSTANCEPROC);
	GETADDR_BASE(glDrawElementsInstancedBaseVertexBaseInstance, "glDrawElementsInstancedBaseVertexBaseInstance", PFNGLDRAWELEMENTSINSTANCEDBASEVERTEXBASEINSTANCEPROC);
	GETADDR_BASE(glGetInternalformativ, "glGetInternalformativ", PFNGLGETINTERNALFORMATIVPROC);
	GETADDR_BASE(glGetActiveAtomicCounterBufferiv, "glGetActiveAtomicCounterBufferiv", PFNGLGETACTIVEATOMICCOUNTERBUFFERIVPROC);
	GETADDR_BASE(glBindImageTexture, "glBindImageTexture", PFNGLBINDIMAGETEXTUREPROC);
	GETADDR_BASE(glMemoryBarrier, "glMemoryBarrier", PFNGLMEMORYBARRIERPROC);
	GETADDR_BASE(glTexStorage1D, "glTexStorage1D", PFNGLTEXSTORAGE1DPROC);
	GETADDR_BASE(glTexStorage2D, "glTexStorage2D", PFNGLTEXSTORAGE2DPROC);
	GETADDR_BASE(glTexStorage3D, "glTexStorage3D", PFNGLTEXSTORAGE3DPROC);
	GETADDR_BASE(glDrawTransformFeedbackInstanced, "glDrawTransformFeedbackInstanced", PFNGLDRAWTRANSFORMFEEDBACKINSTANCEDPROC);
	GETADDR_BASE(glDrawTransformFeedbackStreamInstanced, "glDrawTransformFeedbackStreamInstanced", PFNGLDRAWTRANSFORMFEEDBACKSTREAMINSTANCEDPROC);
#endif

#ifdef GL_VERSION_4_3
	GETADDR_BASE(glClearBufferData, "glClearBufferData", PFNGLCLEARBUFFERDATAPROC);
	GETADDR_BASE(glClearBufferSubData, "glClearBufferSubData", PFNGLCLEARBUFFERSUBDATAPROC);
	GETADDR_BASE(glDispatchCompute, "glDispatchCompute", PFNGLDISPATCHCOMPUTEPROC);
	GETADDR_BASE(glDispatchComputeIndirect, "glDispatchComputeIndirect", PFNGLDISPATCHCOMPUTEINDIRECTPROC);
	GETADDR_BASE(glCopyImageSubData, "glCopyImageSubData", PFNGLCOPYIMAGESUBDATAPROC);
	GETADDR_BASE(glFramebufferParameteri, "glFramebufferParameteri", PFNGLFRAMEBUFFERPARAMETERIPROC);
	GETADDR_BASE(glGetFramebufferParameteriv, "glGetFramebufferParameteriv", PFNGLGETFRAMEBUFFERPARAMETERIVPROC);
	GETADDR_BASE(glGetInternalformati64v, "glGetInternalformati64v", PFNGLGETINTERNALFORMATI64VPROC);
	GETADDR_BASE(glInvalidateTexSubImage, "glInvalidateTexSubImage", PFNGLINVALIDATETEXSUBIMAGEPROC);
	GETADDR_BASE(glInvalidateTexImage, "glInvalidateTexImage", PFNGLINVALIDATETEXIMAGEPROC);
	GETADDR_BASE(glInvalidateBufferSubData, "glInvalidateBufferSubData", PFNGLINVALIDATEBUFFERSUBDATAPROC);
	GETADDR_BASE(glInvalidateBufferData, "glInvalidateBufferData", PFNGLINVALIDATEBUFFERDATAPROC);
	GETADDR_BASE(glInvalidateFramebuffer, "glInvalidateFramebuffer", PFNGLINVALIDATEFRAMEBUFFERPROC);
	GETADDR_BASE(glInvalidateSubFramebuffer, "glInvalidateSubFramebuffer", PFNGLINVALIDATESUBFRAMEBUFFERPROC);
	GETADDR_BASE(glMultiDrawArraysIndirect, "glMultiDrawArraysIndirect", PFNGLMULTIDRAWARRAYSINDIRECTPROC);
	GETADDR_BASE(glMultiDrawElementsIndirect, "glMultiDrawElementsIndirect", PFNGLMULTIDRAWELEMENTSINDIRECTPROC);
	GETADDR_BASE(glGetProgramInterfaceiv, "glGetProgramInterfaceiv", PFNGLGETPROGRAMINTERFACEIVPROC);
	GETADDR_BASE(glGetProgramResourceIndex, "glGetProgramResourceIndex", PFNGLGETPROGRAMRESOURCEINDEXPROC);
	GETADDR_BASE(glGetProgramResourceName, "glGetProgramResourceName", PFNGLGETPROGRAMRESOURCENAMEPROC);
	GETADDR_BASE(glGetProgramResourceiv, "glGetProgramResourceiv", PFNGLGETPROGRAMRESOURCEIVPROC);
	GETADDR_BASE(glGetProgramResourceLocation, "glGetProgramResourceLocation", PFNGLGETPROGRAMRESOURCELOCATIONPROC);
	GETADDR_BASE(glGetProgramResourceLocationIndex, "glGetProgramResourceLocationIndex", PFNGLGETPROGRAMRESOURCELOCATIONINDEXPROC);
	GETADDR_BASE(glShaderStorageBlockBinding, "glShaderStorageBlockBinding", PFNGLSHADERSTORAGEBLOCKBINDINGPROC);
	GETADDR_BASE(glTexBufferRange, "glTexBufferRange", PFNGLTEXBUFFERRANGEPROC);
	GETADDR_BASE(glTexStorage2DMultisample, "glTexStorage2DMultisample", PFNGLTEXSTORAGE2DMULTISAMPLEPROC);
	GETADDR_BASE(glTexStorage3DMultisample, "glTexStorage3DMultisample", PFNGLTEXSTORAGE3DMULTISAMPLEPROC);
	GETADDR_BASE(glTextureView, "glTextureView", PFNGLTEXTUREVIEWPROC);
	GETADDR_BASE(glBindVertexBuffer, "glBindVertexBuffer", PFNGLBINDVERTEXBUFFERPROC);
	GETADDR_BASE(glVertexAttribFormat, "glVertexAttribFormat", PFNGLVERTEXATTRIBFORMATPROC);
	GETADDR_BASE(glVertexAttribIFormat, "glVertexAttribIFormat", PFNGLVERTEXATTRIBIFORMATPROC);
	GETADDR_BASE(glVertexAttribLFormat, "glVertexAttribLFormat", PFNGLVERTEXATTRIBLFORMATPROC);
	GETADDR_BASE(glVertexAttribBinding, "glVertexAttribBinding", PFNGLVERTEXATTRIBBINDINGPROC);
	GETADDR_BASE(glVertexBindingDivisor, "glVertexBindingDivisor", PFNGLVERTEXBINDINGDIVISORPROC);
	GETADDR_BASE(glDebugMessageControl, "glDebugMessageControl", PFNGLDEBUGMESSAGECONTROLPROC);
	GETADDR_BASE(glDebugMessageInsert, "glDebugMessageInsert", PFNGLDEBUGMESSAGEINSERTPROC);
	GETADDR_BASE(glDebugMessageCallback, "glDebugMessageCallback", PFNGLDEBUGMESSAGECALLBACKPROC);
	GETADDR_BASE(glGetDebugMessageLog, "glGetDebugMessageLog", PFNGLGETDEBUGMESSAGELOGPROC);
	GETADDR_BASE(glPushDebugGroup, "glPushDebugGroup", PFNGLPUSHDEBUGGROUPPROC);
	GETADDR_BASE(glPopDebugGroup, "glPopDebugGroup", PFNGLPOPDEBUGGROUPPROC);
	GETADDR_BASE(glObjectLabel, "glObjectLabel", PFNGLOBJECTLABELPROC);
	GETADDR_BASE(glGetObjectLabel, "glGetObjectLabel", PFNGLGETOBJECTLABELPROC);
	GETADDR_BASE(glObjectPtrLabel, "glObjectPtrLabel", PFNGLOBJECTPTRLABELPROC);
	GETADDR_BASE(glGetObjectPtrLabel, "glGetObjectPtrLabel", PFNGLGETOBJECTPTRLABELPROC);
#endif

#ifdef GL_VERSION_4_4
	GETADDR_BASE(glBufferStorage, "glBufferStorage", PFNGLBUFFERSTORAGEPROC);
	GETADDR_BASE(glClearTexImage, "glClearTexImage", PFNGLCLEARTEXIMAGEPROC);
	GETADDR_BASE(glClearTexSubImage, "glClearTexSubImage", PFNGLCLEARTEXSUBIMAGEPROC);
	GETADDR_BASE(glBindBuffersBase, "glBindBuffersBase", PFNGLBINDBUFFERSBASEPROC);
	GETADDR_BASE(glBindBuffersRange, "glBindBuffersRange", PFNGLBINDBUFFERSRANGEPROC);
	GETADDR_BASE(glBindTextures, "glBindTextures", PFNGLBINDTEXTURESPROC);
	GETADDR_BASE(glBindSamplers, "glBindSamplers", PFNGLBINDSAMPLERSPROC);
	GETADDR_BASE(glBindImageTextures, "glBindImageTextures", PFNGLBINDIMAGETEXTURESPROC);
	GETADDR_BASE(glBindVertexBuffers, "glBindVertexBuffers", PFNGLBINDVERTEXBUFFERSPROC);
#endif

#ifdef GL_VERSION_4_5
	GETADDR_BASE(glClipControl, "glClipControl", PFNGLCLIPCONTROLPROC);
	GETADDR_BASE(glCreateTransformFeedbacks, "glCreateTransformFeedbacks", PFNGLCREATETRANSFORMFEEDBACKSPROC);
	GETADDR_BASE(glTransformFeedbackBufferBase, "glTransformFeedbackBufferBase", PFNGLTRANSFORMFEEDBACKBUFFERBASEPROC);
	GETADDR_BASE(glTransformFeedbackBufferRange, "glTransformFeedbackBufferRange", PFNGLTRANSFORMFEEDBACKBUFFERRANGEPROC);
	GETADDR_BASE(glGetTransformFeedbackiv, "glGetTransformFeedbackiv", PFNGLGETTRANSFORMFEEDBACKIVPROC);
	GETADDR_BASE(glGetTransformFeedbacki_v, "glGetTransformFeedbacki_v", PFNGLGETTRANSFORMFEEDBACKI_VPROC);
	GETADDR_BASE(glGetTransformFeedbacki64_v, "glGetTransformFeedbacki64_v", PFNGLGETTRANSFORMFEEDBACKI64_VPROC);
	GETADDR_BASE(glCreateBuffers, "glCreateBuffers", PFNGLCREATEBUFFERSPROC);
	GETADDR_BASE(glNamedBufferStorage, "glNamedBufferStorage", PFNGLNAMEDBUFFERSTORAGEPROC);
	GETADDR_BASE(glNamedBufferData, "glNamedBufferData", PFNGLNAMEDBUFFERDATAPROC);
	GETADDR_BASE(glNamedBufferSubData, "glNamedBufferSubData", PFNGLNAMEDBUFFERSUBDATAPROC);
	GETADDR_BASE(glCopyNamedBufferSubData, "glCopyNamedBufferSubData", PFNGLCOPYNAMEDBUFFERSUBDATAPROC);
	GETADDR_BASE(glClearNamedBufferData, "glClearNamedBufferData", PFNGLCLEARNAMEDBUFFERDATAPROC);
	GETADDR_BASE(glClearNamedBufferSubData, "glClearNamedBufferSubData", PFNGLCLEARNAMEDBUFFERSUBDATAPROC);
	GETADDR_BASE(glMapNamedBuffer, "glMapNamedBuffer", PFNGLMAPNAMEDBUFFERPROC);
	GETADDR_BASE(glMapNamedBufferRange, "glMapNamedBufferRange", PFNGLMAPNAMEDBUFFERRANGEPROC);
	GETADDR_BASE(glUnmapNamedBuffer, "glUnmapNamedBuffer", PFNGLUNMAPNAMEDBUFFERPROC);
	GETADDR_BASE(glFlushMappedNamedBufferRange, "glFlushMappedNamedBufferRange", PFNGLFLUSHMAPPEDNAMEDBUFFERRANGEPROC);
	GETADDR_BASE(glGetNamedBufferParameteriv, "glGetNamedBufferParameteriv", PFNGLGETNAMEDBUFFERPARAMETERIVPROC);
	GETADDR_BASE(glGetNamedBufferParameteri64v, "glGetNamedBufferParameteri64v", PFNGLGETNAMEDBUFFERPARAMETERI64VPROC);
	GETADDR_BASE(glGetNamedBufferPointerv, "glGetNamedBufferPointerv", PFNGLGETNAMEDBUFFERPOINTERVPROC);
	GETADDR_BASE(glGetNamedBufferSubData, "glGetNamedBufferSubData", PFNGLGETNAMEDBUFFERSUBDATAPROC);
	GETADDR_BASE(glCreateFramebuffers, "glCreateFramebuffers", PFNGLCREATEFRAMEBUFFERSPROC);
	GETADDR_BASE(glNamedFramebufferRenderbuffer, "glNamedFramebufferRenderbuffer", PFNGLNAMEDFRAMEBUFFERRENDERBUFFERPROC);
	GETADDR_BASE(glNamedFramebufferParameteri, "glNamedFramebufferParameteri", PFNGLNAMEDFRAMEBUFFERPARAMETERIPROC);
	GETADDR_BASE(glNamedFramebufferTexture, "glNamedFramebufferTexture", PFNGLNAMEDFRAMEBUFFERTEXTUREPROC);
	GETADDR_BASE(glNamedFramebufferTextureLayer, "glNamedFramebufferTextureLayer", PFNGLNAMEDFRAMEBUFFERTEXTURELAYERPROC);
	GETADDR_BASE(glNamedFramebufferDrawBuffer, "glNamedFramebufferDrawBuffer", PFNGLNAMEDFRAMEBUFFERDRAWBUFFERPROC);
	GETADDR_BASE(glNamedFramebufferDrawBuffers, "glNamedFramebufferDrawBuffers", PFNGLNAMEDFRAMEBUFFERDRAWBUFFERSPROC);
	GETADDR_BASE(glNamedFramebufferReadBuffer, "glNamedFramebufferReadBuffer", PFNGLNAMEDFRAMEBUFFERREADBUFFERPROC);
	GETADDR_BASE(glInvalidateNamedFramebufferData, "glInvalidateNamedFramebufferData", PFNGLINVALIDATENAMEDFRAMEBUFFERDATAPROC);
	GETADDR_BASE(glInvalidateNamedFramebufferSubData, "glInvalidateNamedFramebufferSubData", PFNGLINVALIDATENAMEDFRAMEBUFFERSUBDATAPROC);
	GETADDR_BASE(glClearNamedFramebufferiv, "glClearNamedFramebufferiv", PFNGLCLEARNAMEDFRAMEBUFFERIVPROC);
	GETADDR_BASE(glClearNamedFramebufferuiv, "glClearNamedFramebufferuiv", PFNGLCLEARNAMEDFRAMEBUFFERUIVPROC);
	GETADDR_BASE(glClearNamedFramebufferfv, "glClearNamedFramebufferfv", PFNGLCLEARNAMEDFRAMEBUFFERFVPROC);
	GETADDR_BASE(glClearNamedFramebufferfi, "glClearNamedFramebufferfi", PFNGLCLEARNAMEDFRAMEBUFFERFIPROC);
	GETADDR_BASE(glBlitNamedFramebuffer, "glBlitNamedFramebuffer", PFNGLBLITNAMEDFRAMEBUFFERPROC);
	GETADDR_BASE(glCheckNamedFramebufferStatus, "glCheckNamedFramebufferStatus", PFNGLCHECKNAMEDFRAMEBUFFERSTATUSPROC);
	GETADDR_BASE(glGetNamedFramebufferParameteriv, "glGetNamedFramebufferParameteriv", PFNGLGETNAMEDFRAMEBUFFERPARAMETERIVPROC);
	GETADDR_BASE(glGetNamedFramebufferAttachmentParameteriv, "glGetNamedFramebufferAttachmentParameteriv", PFNGLGETNAMEDFRAMEBUFFERATTACHMENTPARAMETERIVPROC);
	GETADDR_BASE(glCreateRenderbuffers, "glCreateRenderbuffers", PFNGLCREATERENDERBUFFERSPROC);
	GETADDR_BASE(glNamedRenderbufferStorage, "glNamedRenderbufferStorage", PFNGLNAMEDRENDERBUFFERSTORAGEPROC);
	GETADDR_BASE(glNamedRenderbufferStorageMultisample, "glNamedRenderbufferStorageMultisample", PFNGLNAMEDRENDERBUFFERSTORAGEMULTISAMPLEPROC);
	GETADDR_BASE(glGetNamedRenderbufferParameteriv, "glGetNamedRenderbufferParameteriv", PFNGLGETNAMEDRENDERBUFFERPARAMETERIVPROC);
	GETADDR_BASE(glCreateTextures, "glCreateTextures", PFNGLCREATETEXTURESPROC);
	GETADDR_BASE(glTextureBuffer, "glTextureBuffer", PFNGLTEXTUREBUFFERPROC);
	GETADDR_BASE(glTextureBufferRange, "glTextureBufferRange", PFNGLTEXTUREBUFFERRANGEPROC);
	GETADDR_BASE(glTextureStorage1D, "glTextureStorage1D", PFNGLTEXTURESTORAGE1DPROC);
	GETADDR_BASE(glTextureStorage2D, "glTextureStorage2D", PFNGLTEXTURESTORAGE2DPROC);
	GETADDR_BASE(glTextureStorage3D, "glTextureStorage3D", PFNGLTEXTURESTORAGE3DPROC);
	GETADDR_BASE(glTextureStorage2DMultisample, "glTextureStorage2DMultisample", PFNGLTEXTURESTORAGE2DMULTISAMPLEPROC);
	GETADDR_BASE(glTextureStorage3DMultisample, "glTextureStorage3DMultisample", PFNGLTEXTURESTORAGE3DMULTISAMPLEPROC);
	GETADDR_BASE(glTextureSubImage1D, "glTextureSubImage1D", PFNGLTEXTURESUBIMAGE1DPROC);
	GETADDR_BASE(glTextureSubImage2D, "glTextureSubImage2D", PFNGLTEXTURESUBIMAGE2DPROC);
	GETADDR_BASE(glTextureSubImage3D, "glTextureSubImage3D", PFNGLTEXTURESUBIMAGE3DPROC);
	GETADDR_BASE(glCompressedTextureSubImage1D, "glCompressedTextureSubImage1D", PFNGLCOMPRESSEDTEXTURESUBIMAGE1DPROC);
	GETADDR_BASE(glCompressedTextureSubImage2D, "glCompressedTextureSubImage2D", PFNGLCOMPRESSEDTEXTURESUBIMAGE2DPROC);
	GETADDR_BASE(glCompressedTextureSubImage3D, "glCompressedTextureSubImage3D", PFNGLCOMPRESSEDTEXTURESUBIMAGE3DPROC);
	GETADDR_BASE(glCopyTextureSubImage1D, "glCopyTextureSubImage1D", PFNGLCOPYTEXTURESUBIMAGE1DPROC);
	GETADDR_BASE(glCopyTextureSubImage2D, "glCopyTextureSubImage2D", PFNGLCOPYTEXTURESUBIMAGE2DPROC);
	GETADDR_BASE(glCopyTextureSubImage3D, "glCopyTextureSubImage3D", PFNGLCOPYTEXTURESUBIMAGE3DPROC);
	GETADDR_BASE(glTextureParameterf, "glTextureParameterf", PFNGLTEXTUREPARAMETERFPROC);
	GETADDR_BASE(glTextureParameterfv, "glTextureParameterfv", PFNGLTEXTUREPARAMETERFVPROC);
	GETADDR_BASE(glTextureParameteri, "glTextureParameteri", PFNGLTEXTUREPARAMETERIPROC);
	GETADDR_BASE(glTextureParameterIiv, "glTextureParameterIiv", PFNGLTEXTUREPARAMETERIIVPROC);
	GETADDR_BASE(glTextureParameterIuiv, "glTextureParameterIuiv", PFNGLTEXTUREPARAMETERIUIVPROC);
	GETADDR_BASE(glTextureParameteriv, "glTextureParameteriv", PFNGLTEXTUREPARAMETERIVPROC);
	GETADDR_BASE(glGenerateTextureMipmap, "glGenerateTextureMipmap", PFNGLGENERATETEXTUREMIPMAPPROC);
	GETADDR_BASE(glBindTextureUnit, "glBindTextureUnit", PFNGLBINDTEXTUREUNITPROC);
	GETADDR_BASE(glGetTextureImage, "glGetTextureImage", PFNGLGETTEXTUREIMAGEPROC);
	GETADDR_BASE(glGetCompressedTextureImage, "glGetCompressedTextureImage", PFNGLGETCOMPRESSEDTEXTUREIMAGEPROC);
	GETADDR_BASE(glGetTextureLevelParameterfv, "glGetTextureLevelParameterfv", PFNGLGETTEXTURELEVELPARAMETERFVPROC);
	GETADDR_BASE(glGetTextureLevelParameteriv, "glGetTextureLevelParameteriv", PFNGLGETTEXTURELEVELPARAMETERIVPROC);
	GETADDR_BASE(glGetTextureParameterfv, "glGetTextureParameterfv", PFNGLGETTEXTUREPARAMETERFVPROC);
	GETADDR_BASE(glGetTextureParameterIiv, "glGetTextureParameterIiv", PFNGLGETTEXTUREPARAMETERIIVPROC);
	GETADDR_BASE(glGetTextureParameterIuiv, "glGetTextureParameterIuiv", PFNGLGETTEXTUREPARAMETERIUIVPROC);
	GETADDR_BASE(glGetTextureParameteriv, "glGetTextureParameteriv", PFNGLGETTEXTUREPARAMETERIVPROC);
	GETADDR_BASE(glCreateVertexArrays, "glCreateVertexArrays", PFNGLCREATEVERTEXARRAYSPROC);
	GETADDR_BASE(glDisableVertexArrayAttrib, "glDisableVertexArrayAttrib", PFNGLDISABLEVERTEXARRAYATTRIBPROC);
	GETADDR_BASE(glEnableVertexArrayAttrib, "glEnableVertexArrayAttrib", PFNGLENABLEVERTEXARRAYATTRIBPROC);
	GETADDR_BASE(glVertexArrayElementBuffer, "glVertexArrayElementBuffer", PFNGLVERTEXARRAYELEMENTBUFFERPROC);
	GETADDR_BASE(glVertexArrayVertexBuffer, "glVertexArrayVertexBuffer", PFNGLVERTEXARRAYVERTEXBUFFERPROC);
	GETADDR_BASE(glVertexArrayVertexBuffers, "glVertexArrayVertexBuffers", PFNGLVERTEXARRAYVERTEXBUFFERSPROC);
	GETADDR_BASE(glVertexArrayAttribBinding, "glVertexArrayAttribBinding", PFNGLVERTEXARRAYATTRIBBINDINGPROC);
	GETADDR_BASE(glVertexArrayAttribFormat, "glVertexArrayAttribFormat", PFNGLVERTEXARRAYATTRIBFORMATPROC);
	GETADDR_BASE(glVertexArrayAttribIFormat, "glVertexArrayAttribIFormat", PFNGLVERTEXARRAYATTRIBIFORMATPROC);
	GETADDR_BASE(glVertexArrayAttribLFormat, "glVertexArrayAttribLFormat", PFNGLVERTEXARRAYATTRIBLFORMATPROC);
	GETADDR_BASE(glVertexArrayBindingDivisor, "glVertexArrayBindingDivisor", PFNGLVERTEXARRAYBINDINGDIVISORPROC);
	GETADDR_BASE(glGetVertexArrayiv, "glGetVertexArrayiv", PFNGLGETVERTEXARRAYIVPROC);
	GETADDR_BASE(glGetVertexArrayIndexediv, "glGetVertexArrayIndexediv", PFNGLGETVERTEXARRAYINDEXEDIVPROC);
	GETADDR_BASE(glGetVertexArrayIndexed64iv, "glGetVertexArrayIndexed64iv", PFNGLGETVERTEXARRAYINDEXED64IVPROC);
	GETADDR_BASE(glCreateSamplers, "glCreateSamplers", PFNGLCREATESAMPLERSPROC);
	GETADDR_BASE(glCreateProgramPipelines, "glCreateProgramPipelines", PFNGLCREATEPROGRAMPIPELINESPROC);
	GETADDR_BASE(glCreateQueries, "glCreateQueries", PFNGLCREATEQUERIESPROC);
	GETADDR_BASE(glMemoryBarrierByRegion, "glMemoryBarrierByRegion", PFNGLMEMORYBARRIERBYREGIONPROC);
	GETADDR_BASE(glGetTextureSubImage, "glGetTextureSubImage", PFNGLGETTEXTURESUBIMAGEPROC);
	GETADDR_BASE(glGetCompressedTextureSubImage, "glGetCompressedTextureSubImage", PFNGLGETCOMPRESSEDTEXTURESUBIMAGEPROC);
	GETADDR_BASE(glGetGraphicsResetStatus, "glGetGraphicsResetStatus", PFNGLGETGRAPHICSRESETSTATUSPROC);
	GETADDR_BASE(glGetnCompressedTexImage, "glGetnCompressedTexImage", PFNGLGETNCOMPRESSEDTEXIMAGEPROC);
	GETADDR_BASE(glGetnTexImage, "glGetnTexImage", PFNGLGETNTEXIMAGEPROC);
	GETADDR_BASE(glGetnUniformdv, "glGetnUniformdv", PFNGLGETNUNIFORMDVPROC);
	GETADDR_BASE(glGetnUniformfv, "glGetnUniformfv", PFNGLGETNUNIFORMFVPROC);
	GETADDR_BASE(glGetnUniformiv, "glGetnUniformiv", PFNGLGETNUNIFORMIVPROC);
	GETADDR_BASE(glGetnUniformuiv, "glGetnUniformuiv", PFNGLGETNUNIFORMUIVPROC);
	GETADDR_BASE(glReadnPixels, "glReadnPixels", PFNGLREADNPIXELSPROC);
	GETADDR_BASE(glTextureBarrier, "glTextureBarrier", PFNGLTEXTUREBARRIERPROC);
#endif

#ifdef GL_ARB_bindless_texture
	GETADDR_BASE_OPTIONAL(glGetTextureHandleARB, "glGetTextureHandleARB", PFNGLGETTEXTUREHANDLEARBPROC);
	GETADDR_BASE_OPTIONAL(glGetTextureSamplerHandleARB, "glGetTextureSamplerHandleARB", PFNGLGETTEXTURESAMPLERHANDLEARBPROC);
	GETADDR_BASE_OPTIONAL(glMakeTextureHandleResidentARB, "glMakeTextureHandleResidentARB", PFNGLMAKETEXTUREHANDLERESIDENTARBPROC);
	GETADDR_BASE_OPTIONAL(glMakeTextureHandleNonResidentARB, "glMakeTextureHandleNonResidentARB", PFNGLMAKETEXTUREHANDLENONRESIDENTARBPROC);
	GETADDR_BASE_OPTIONAL(glGetImageHandleARB, "glGetImageHandleARB", PFNGLGETIMAGEHANDLEARBPROC);
	GETADDR_BASE_OPTIONAL(glMakeImageHandleResidentARB, "glMakeImageHandleResidentARB", PFNGLMAKEIMAGEHANDLERESIDENTARBPROC);
	GETADDR_BASE_OPTIONAL(glMakeImageHandleNonResidentARB, "glMakeImageHandleNonResidentARB", PFNGLMAKEIMAGEHANDLENONRESIDENTARBPROC);
	GETADDR_BASE_OPTIONAL(glUniformHandleui64ARB, "glUniformHandleui64ARB", PFNGLUNIFORMHANDLEUI64ARBPROC);
	GETADDR_BASE_OPTIONAL(glUniformHandleui64vARB, "glUniformHandleui64vARB", PFNGLUNIFORMHANDLEUI64VARBPROC);
	GETADDR_BASE_OPTIONAL(glProgramUniformHandleui64ARB, "glProgramUniformHandleui64ARB", PFNGLPROGRAMUNIFORMHANDLEUI64ARBPROC);
	GETADDR_BASE_OPTIONAL(glProgramUniformHandleui64vARB, "glProgramUniformHandleui64vARB", PFNGLPROGRAMUNIFORMHANDLEUI64VARBPROC);
	GETADDR_BASE_OPTIONAL(glIsTextureHandleResidentARB, "glIsTextureHandleResidentARB", PFNGLISTEXTUREHANDLERESIDENTARBPROC);
	GETADDR_BASE_OPTIONAL(glIsImageHandleResidentARB, "glIsImageHandleResidentARB", PFNGLISIMAGEHANDLERESIDENTARBPROC);
	GETADDR_BASE_OPTIONAL(glVertexAttribL1ui64ARB, "glVertexAttribL1ui64ARB", PFNGLVERTEXATTRIBL1UI64ARBPROC);
	GETADDR_BASE_OPTIONAL(glVertexAttribL1ui64vARB, "glVertexAttribL1ui64vARB", PFNGLVERTEXATTRIBL1UI64VARBPROC);
	GETADDR_BASE_OPTIONAL(glGetVertexAttribLui64vARB, "glGetVertexAttribLui64vARB", PFNGLGETVERTEXATTRIBLUI64VARBPROC);
#endif

#ifdef GL_ARB_cl_event
	GETADDR_BASE_OPTIONAL(glCreateSyncFromCLeventARB, "glCreateSyncFromCLeventARB", PFNGLCREATESYNCFROMCLEVENTARBPROC);
#endif

#ifdef GL_ARB_compute_variable_group_size
	GETADDR_BASE_OPTIONAL(glDispatchComputeGroupSizeARB, "glDispatchComputeGroupSizeARB", PFNGLDISPATCHCOMPUTEGROUPSIZEARBPROC);
#endif

#ifdef GL_ARB_debug_output
#endif

#ifdef GL_ARB_draw_buffers_blend
#endif

#ifdef GL_ARB_indirect_parameters
	GETADDR_BASE_OPTIONAL(glMultiDrawArraysIndirectCountARB, "glMultiDrawArraysIndirectCountARB", PFNGLMULTIDRAWARRAYSINDIRECTCOUNTARBPROC);
	GETADDR_BASE_OPTIONAL(glMultiDrawElementsIndirectCountARB, "glMultiDrawElementsIndirectCountARB", PFNGLMULTIDRAWELEMENTSINDIRECTCOUNTARBPROC);
#endif

#ifdef GL_ARB_robustness
#endif

#ifdef GL_ARB_sample_shading
#endif

#ifdef GL_ARB_shading_language_include
	GETADDR_BASE_OPTIONAL(glNamedStringARB, "glNamedStringARB", PFNGLNAMEDSTRINGARBPROC);
	GETADDR_BASE_OPTIONAL(glDeleteNamedStringARB, "glDeleteNamedStringARB", PFNGLDELETENAMEDSTRINGARBPROC);
	GETADDR_BASE_OPTIONAL(glCompileShaderIncludeARB, "glCompileShaderIncludeARB", PFNGLCOMPILESHADERINCLUDEARBPROC);
	GETADDR_BASE_OPTIONAL(glIsNamedStringARB, "glIsNamedStringARB", PFNGLISNAMEDSTRINGARBPROC);
	GETADDR_BASE_OPTIONAL(glGetNamedStringARB, "glGetNamedStringARB", PFNGLGETNAMEDSTRINGARBPROC);
	GETADDR_BASE_OPTIONAL(glGetNamedStringivARB, "glGetNamedStringivARB", PFNGLGETNAMEDSTRINGIVARBPROC);
#endif

#ifdef GL_ARB_sparse_buffer
	GETADDR_BASE_OPTIONAL(glBufferPageCommitmentARB, "glBufferPageCommitmentARB", PFNGLBUFFERPAGECOMMITMENTARBPROC);
	GETADDR_BASE_OPTIONAL(glNamedBufferPageCommitmentEXT, "glNamedBufferPageCommitmentEXT", PFNGLNAMEDBUFFERPAGECOMMITMENTEXTPROC);
	GETADDR_BASE_OPTIONAL(glNamedBufferPageCommitmentARB, "glNamedBufferPageCommitmentARB", PFNGLNAMEDBUFFERPAGECOMMITMENTARBPROC);
#endif

#ifdef GL_ARB_sparse_texture
	GETADDR_BASE_OPTIONAL(glTexPageCommitmentARB, "glTexPageCommitmentARB", PFNGLTEXPAGECOMMITMENTARBPROC);
#endif

	return true;
}

namespace HorseRadish { namespace OpenGL
{
#ifdef GL_VERSION_1_0
		PFNGLCULLFACEPROC glCullFace;
		PFNGLFRONTFACEPROC glFrontFace;
		PFNGLHINTPROC glHint;
		PFNGLLINEWIDTHPROC glLineWidth;
		PFNGLPOINTSIZEPROC glPointSize;
		PFNGLPOLYGONMODEPROC glPolygonMode;
		PFNGLSCISSORPROC glScissor;
		PFNGLTEXPARAMETERFPROC glTexParameterf;
		PFNGLTEXPARAMETERFVPROC glTexParameterfv;
		PFNGLTEXPARAMETERIPROC glTexParameteri;
		PFNGLTEXPARAMETERIVPROC glTexParameteriv;
		PFNGLTEXIMAGE1DPROC glTexImage1D;
		PFNGLTEXIMAGE2DPROC glTexImage2D;
		PFNGLDRAWBUFFERPROC glDrawBuffer;
		PFNGLCLEARPROC glClear;
		PFNGLCLEARCOLORPROC glClearColor;
		PFNGLCLEARSTENCILPROC glClearStencil;
		PFNGLCLEARDEPTHPROC glClearDepth;
		PFNGLSTENCILMASKPROC glStencilMask;
		PFNGLCOLORMASKPROC glColorMask;
		PFNGLDEPTHMASKPROC glDepthMask;
		PFNGLDISABLEPROC glDisable;
		PFNGLENABLEPROC glEnable;
		PFNGLFINISHPROC glFinish;
		PFNGLFLUSHPROC glFlush;
		PFNGLBLENDFUNCPROC glBlendFunc;
		PFNGLLOGICOPPROC glLogicOp;
		PFNGLSTENCILFUNCPROC glStencilFunc;
		PFNGLSTENCILOPPROC glStencilOp;
		PFNGLDEPTHFUNCPROC glDepthFunc;
		PFNGLPIXELSTOREFPROC glPixelStoref;
		PFNGLPIXELSTOREIPROC glPixelStorei;
		PFNGLREADBUFFERPROC glReadBuffer;
		PFNGLREADPIXELSPROC glReadPixels;
		PFNGLGETBOOLEANVPROC glGetBooleanv;
		PFNGLGETDOUBLEVPROC glGetDoublev;
		PFNGLGETERRORPROC glGetError;
		PFNGLGETFLOATVPROC glGetFloatv;
		PFNGLGETINTEGERVPROC glGetIntegerv;
		PFNGLGETSTRINGPROC glGetString;
		PFNGLGETTEXIMAGEPROC glGetTexImage;
		PFNGLGETTEXPARAMETERFVPROC glGetTexParameterfv;
		PFNGLGETTEXPARAMETERIVPROC glGetTexParameteriv;
		PFNGLGETTEXLEVELPARAMETERFVPROC glGetTexLevelParameterfv;
		PFNGLGETTEXLEVELPARAMETERIVPROC glGetTexLevelParameteriv;
		PFNGLISENABLEDPROC glIsEnabled;
		PFNGLDEPTHRANGEPROC glDepthRange;
		PFNGLVIEWPORTPROC glViewport;
#endif

#ifdef GL_VERSION_1_1
		PFNGLDRAWARRAYSPROC glDrawArrays;
		PFNGLDRAWELEMENTSPROC glDrawElements;
		PFNGLGETPOINTERVPROC glGetPointerv;
		PFNGLPOLYGONOFFSETPROC glPolygonOffset;
		PFNGLCOPYTEXIMAGE1DPROC glCopyTexImage1D;
		PFNGLCOPYTEXIMAGE2DPROC glCopyTexImage2D;
		PFNGLCOPYTEXSUBIMAGE1DPROC glCopyTexSubImage1D;
		PFNGLCOPYTEXSUBIMAGE2DPROC glCopyTexSubImage2D;
		PFNGLTEXSUBIMAGE1DPROC glTexSubImage1D;
		PFNGLTEXSUBIMAGE2DPROC glTexSubImage2D;
		PFNGLBINDTEXTUREPROC glBindTexture;
		PFNGLDELETETEXTURESPROC glDeleteTextures;
		PFNGLGENTEXTURESPROC glGenTextures;
		PFNGLISTEXTUREPROC glIsTexture;
#endif

#ifdef GL_VERSION_1_2
		PFNGLDRAWRANGEELEMENTSPROC glDrawRangeElements;
		PFNGLTEXIMAGE3DPROC glTexImage3D;
		PFNGLTEXSUBIMAGE3DPROC glTexSubImage3D;
		PFNGLCOPYTEXSUBIMAGE3DPROC glCopyTexSubImage3D;
#endif

#ifdef GL_VERSION_1_3
		PFNGLACTIVETEXTUREPROC glActiveTexture;
		PFNGLSAMPLECOVERAGEPROC glSampleCoverage;
		PFNGLCOMPRESSEDTEXIMAGE3DPROC glCompressedTexImage3D;
		PFNGLCOMPRESSEDTEXIMAGE2DPROC glCompressedTexImage2D;
		PFNGLCOMPRESSEDTEXIMAGE1DPROC glCompressedTexImage1D;
		PFNGLCOMPRESSEDTEXSUBIMAGE3DPROC glCompressedTexSubImage3D;
		PFNGLCOMPRESSEDTEXSUBIMAGE2DPROC glCompressedTexSubImage2D;
		PFNGLCOMPRESSEDTEXSUBIMAGE1DPROC glCompressedTexSubImage1D;
		PFNGLGETCOMPRESSEDTEXIMAGEPROC glGetCompressedTexImage;
#endif

#ifdef GL_VERSION_1_4
		PFNGLBLENDFUNCSEPARATEPROC glBlendFuncSeparate;
		PFNGLMULTIDRAWARRAYSPROC glMultiDrawArrays;
		PFNGLMULTIDRAWELEMENTSPROC glMultiDrawElements;
		PFNGLPOINTPARAMETERFPROC glPointParameterf;
		PFNGLPOINTPARAMETERFVPROC glPointParameterfv;
		PFNGLPOINTPARAMETERIPROC glPointParameteri;
		PFNGLPOINTPARAMETERIVPROC glPointParameteriv;
		PFNGLBLENDCOLORPROC glBlendColor;
		PFNGLBLENDEQUATIONPROC glBlendEquation;
#endif

#ifdef GL_VERSION_1_5
		PFNGLGENQUERIESPROC glGenQueries;
		PFNGLDELETEQUERIESPROC glDeleteQueries;
		PFNGLISQUERYPROC glIsQuery;
		PFNGLBEGINQUERYPROC glBeginQuery;
		PFNGLENDQUERYPROC glEndQuery;
		PFNGLGETQUERYIVPROC glGetQueryiv;
		PFNGLGETQUERYOBJECTIVPROC glGetQueryObjectiv;
		PFNGLGETQUERYOBJECTUIVPROC glGetQueryObjectuiv;
		PFNGLBINDBUFFERPROC glBindBuffer;
		PFNGLDELETEBUFFERSPROC glDeleteBuffers;
		PFNGLGENBUFFERSPROC glGenBuffers;
		PFNGLISBUFFERPROC glIsBuffer;
		PFNGLBUFFERDATAPROC glBufferData;
		PFNGLBUFFERSUBDATAPROC glBufferSubData;
		PFNGLGETBUFFERSUBDATAPROC glGetBufferSubData;
		PFNGLMAPBUFFERPROC glMapBuffer;
		PFNGLUNMAPBUFFERPROC glUnmapBuffer;
		PFNGLGETBUFFERPARAMETERIVPROC glGetBufferParameteriv;
		PFNGLGETBUFFERPOINTERVPROC glGetBufferPointerv;
#endif

#ifdef GL_VERSION_2_0
		PFNGLBLENDEQUATIONSEPARATEPROC glBlendEquationSeparate;
		PFNGLDRAWBUFFERSPROC glDrawBuffers;
		PFNGLSTENCILOPSEPARATEPROC glStencilOpSeparate;
		PFNGLSTENCILFUNCSEPARATEPROC glStencilFuncSeparate;
		PFNGLSTENCILMASKSEPARATEPROC glStencilMaskSeparate;
		PFNGLATTACHSHADERPROC glAttachShader;
		PFNGLBINDATTRIBLOCATIONPROC glBindAttribLocation;
		PFNGLCOMPILESHADERPROC glCompileShader;
		PFNGLCREATEPROGRAMPROC glCreateProgram;
		PFNGLCREATESHADERPROC glCreateShader;
		PFNGLDELETEPROGRAMPROC glDeleteProgram;
		PFNGLDELETESHADERPROC glDeleteShader;
		PFNGLDETACHSHADERPROC glDetachShader;
		PFNGLDISABLEVERTEXATTRIBARRAYPROC glDisableVertexAttribArray;
		PFNGLENABLEVERTEXATTRIBARRAYPROC glEnableVertexAttribArray;
		PFNGLGETACTIVEATTRIBPROC glGetActiveAttrib;
		PFNGLGETACTIVEUNIFORMPROC glGetActiveUniform;
		PFNGLGETATTACHEDSHADERSPROC glGetAttachedShaders;
		PFNGLGETATTRIBLOCATIONPROC glGetAttribLocation;
		PFNGLGETPROGRAMIVPROC glGetProgramiv;
		PFNGLGETPROGRAMINFOLOGPROC glGetProgramInfoLog;
		PFNGLGETSHADERIVPROC glGetShaderiv;
		PFNGLGETSHADERINFOLOGPROC glGetShaderInfoLog;
		PFNGLGETSHADERSOURCEPROC glGetShaderSource;
		PFNGLGETUNIFORMLOCATIONPROC glGetUniformLocation;
		PFNGLGETUNIFORMFVPROC glGetUniformfv;
		PFNGLGETUNIFORMIVPROC glGetUniformiv;
		PFNGLGETVERTEXATTRIBDVPROC glGetVertexAttribdv;
		PFNGLGETVERTEXATTRIBFVPROC glGetVertexAttribfv;
		PFNGLGETVERTEXATTRIBIVPROC glGetVertexAttribiv;
		PFNGLGETVERTEXATTRIBPOINTERVPROC glGetVertexAttribPointerv;
		PFNGLISPROGRAMPROC glIsProgram;
		PFNGLISSHADERPROC glIsShader;
		PFNGLLINKPROGRAMPROC glLinkProgram;
		PFNGLSHADERSOURCEPROC glShaderSource;
		PFNGLUSEPROGRAMPROC glUseProgram;
		PFNGLUNIFORM1FPROC glUniform1f;
		PFNGLUNIFORM2FPROC glUniform2f;
		PFNGLUNIFORM3FPROC glUniform3f;
		PFNGLUNIFORM4FPROC glUniform4f;
		PFNGLUNIFORM1IPROC glUniform1i;
		PFNGLUNIFORM2IPROC glUniform2i;
		PFNGLUNIFORM3IPROC glUniform3i;
		PFNGLUNIFORM4IPROC glUniform4i;
		PFNGLUNIFORM1FVPROC glUniform1fv;
		PFNGLUNIFORM2FVPROC glUniform2fv;
		PFNGLUNIFORM3FVPROC glUniform3fv;
		PFNGLUNIFORM4FVPROC glUniform4fv;
		PFNGLUNIFORM1IVPROC glUniform1iv;
		PFNGLUNIFORM2IVPROC glUniform2iv;
		PFNGLUNIFORM3IVPROC glUniform3iv;
		PFNGLUNIFORM4IVPROC glUniform4iv;
		PFNGLUNIFORMMATRIX2FVPROC glUniformMatrix2fv;
		PFNGLUNIFORMMATRIX3FVPROC glUniformMatrix3fv;
		PFNGLUNIFORMMATRIX4FVPROC glUniformMatrix4fv;
		PFNGLVALIDATEPROGRAMPROC glValidateProgram;
		PFNGLVERTEXATTRIB1DPROC glVertexAttrib1d;
		PFNGLVERTEXATTRIB1DVPROC glVertexAttrib1dv;
		PFNGLVERTEXATTRIB1FPROC glVertexAttrib1f;
		PFNGLVERTEXATTRIB1FVPROC glVertexAttrib1fv;
		PFNGLVERTEXATTRIB1SPROC glVertexAttrib1s;
		PFNGLVERTEXATTRIB1SVPROC glVertexAttrib1sv;
		PFNGLVERTEXATTRIB2DPROC glVertexAttrib2d;
		PFNGLVERTEXATTRIB2DVPROC glVertexAttrib2dv;
		PFNGLVERTEXATTRIB2FPROC glVertexAttrib2f;
		PFNGLVERTEXATTRIB2FVPROC glVertexAttrib2fv;
		PFNGLVERTEXATTRIB2SPROC glVertexAttrib2s;
		PFNGLVERTEXATTRIB2SVPROC glVertexAttrib2sv;
		PFNGLVERTEXATTRIB3DPROC glVertexAttrib3d;
		PFNGLVERTEXATTRIB3DVPROC glVertexAttrib3dv;
		PFNGLVERTEXATTRIB3FPROC glVertexAttrib3f;
		PFNGLVERTEXATTRIB3FVPROC glVertexAttrib3fv;
		PFNGLVERTEXATTRIB3SPROC glVertexAttrib3s;
		PFNGLVERTEXATTRIB3SVPROC glVertexAttrib3sv;
		PFNGLVERTEXATTRIB4NBVPROC glVertexAttrib4Nbv;
		PFNGLVERTEXATTRIB4NIVPROC glVertexAttrib4Niv;
		PFNGLVERTEXATTRIB4NSVPROC glVertexAttrib4Nsv;
		PFNGLVERTEXATTRIB4NUBPROC glVertexAttrib4Nub;
		PFNGLVERTEXATTRIB4NUBVPROC glVertexAttrib4Nubv;
		PFNGLVERTEXATTRIB4NUIVPROC glVertexAttrib4Nuiv;
		PFNGLVERTEXATTRIB4NUSVPROC glVertexAttrib4Nusv;
		PFNGLVERTEXATTRIB4BVPROC glVertexAttrib4bv;
		PFNGLVERTEXATTRIB4DPROC glVertexAttrib4d;
		PFNGLVERTEXATTRIB4DVPROC glVertexAttrib4dv;
		PFNGLVERTEXATTRIB4FPROC glVertexAttrib4f;
		PFNGLVERTEXATTRIB4FVPROC glVertexAttrib4fv;
		PFNGLVERTEXATTRIB4IVPROC glVertexAttrib4iv;
		PFNGLVERTEXATTRIB4SPROC glVertexAttrib4s;
		PFNGLVERTEXATTRIB4SVPROC glVertexAttrib4sv;
		PFNGLVERTEXATTRIB4UBVPROC glVertexAttrib4ubv;
		PFNGLVERTEXATTRIB4UIVPROC glVertexAttrib4uiv;
		PFNGLVERTEXATTRIB4USVPROC glVertexAttrib4usv;
		PFNGLVERTEXATTRIBPOINTERPROC glVertexAttribPointer;
#endif

#ifdef GL_VERSION_2_1
		PFNGLUNIFORMMATRIX2X3FVPROC glUniformMatrix2x3fv;
		PFNGLUNIFORMMATRIX3X2FVPROC glUniformMatrix3x2fv;
		PFNGLUNIFORMMATRIX2X4FVPROC glUniformMatrix2x4fv;
		PFNGLUNIFORMMATRIX4X2FVPROC glUniformMatrix4x2fv;
		PFNGLUNIFORMMATRIX3X4FVPROC glUniformMatrix3x4fv;
		PFNGLUNIFORMMATRIX4X3FVPROC glUniformMatrix4x3fv;
#endif

#ifdef GL_VERSION_3_0
		PFNGLCOLORMASKIPROC glColorMaski;
		PFNGLGETBOOLEANI_VPROC glGetBooleani_v;
		PFNGLGETINTEGERI_VPROC glGetIntegeri_v;
		PFNGLENABLEIPROC glEnablei;
		PFNGLDISABLEIPROC glDisablei;
		PFNGLISENABLEDIPROC glIsEnabledi;
		PFNGLBEGINTRANSFORMFEEDBACKPROC glBeginTransformFeedback;
		PFNGLENDTRANSFORMFEEDBACKPROC glEndTransformFeedback;
		PFNGLBINDBUFFERRANGEPROC glBindBufferRange;
		PFNGLBINDBUFFERBASEPROC glBindBufferBase;
		PFNGLTRANSFORMFEEDBACKVARYINGSPROC glTransformFeedbackVaryings;
		PFNGLGETTRANSFORMFEEDBACKVARYINGPROC glGetTransformFeedbackVarying;
		PFNGLCLAMPCOLORPROC glClampColor;
		PFNGLBEGINCONDITIONALRENDERPROC glBeginConditionalRender;
		PFNGLENDCONDITIONALRENDERPROC glEndConditionalRender;
		PFNGLVERTEXATTRIBIPOINTERPROC glVertexAttribIPointer;
		PFNGLGETVERTEXATTRIBIIVPROC glGetVertexAttribIiv;
		PFNGLGETVERTEXATTRIBIUIVPROC glGetVertexAttribIuiv;
		PFNGLVERTEXATTRIBI1IPROC glVertexAttribI1i;
		PFNGLVERTEXATTRIBI2IPROC glVertexAttribI2i;
		PFNGLVERTEXATTRIBI3IPROC glVertexAttribI3i;
		PFNGLVERTEXATTRIBI4IPROC glVertexAttribI4i;
		PFNGLVERTEXATTRIBI1UIPROC glVertexAttribI1ui;
		PFNGLVERTEXATTRIBI2UIPROC glVertexAttribI2ui;
		PFNGLVERTEXATTRIBI3UIPROC glVertexAttribI3ui;
		PFNGLVERTEXATTRIBI4UIPROC glVertexAttribI4ui;
		PFNGLVERTEXATTRIBI1IVPROC glVertexAttribI1iv;
		PFNGLVERTEXATTRIBI2IVPROC glVertexAttribI2iv;
		PFNGLVERTEXATTRIBI3IVPROC glVertexAttribI3iv;
		PFNGLVERTEXATTRIBI4IVPROC glVertexAttribI4iv;
		PFNGLVERTEXATTRIBI1UIVPROC glVertexAttribI1uiv;
		PFNGLVERTEXATTRIBI2UIVPROC glVertexAttribI2uiv;
		PFNGLVERTEXATTRIBI3UIVPROC glVertexAttribI3uiv;
		PFNGLVERTEXATTRIBI4UIVPROC glVertexAttribI4uiv;
		PFNGLVERTEXATTRIBI4BVPROC glVertexAttribI4bv;
		PFNGLVERTEXATTRIBI4SVPROC glVertexAttribI4sv;
		PFNGLVERTEXATTRIBI4UBVPROC glVertexAttribI4ubv;
		PFNGLVERTEXATTRIBI4USVPROC glVertexAttribI4usv;
		PFNGLGETUNIFORMUIVPROC glGetUniformuiv;
		PFNGLBINDFRAGDATALOCATIONPROC glBindFragDataLocation;
		PFNGLGETFRAGDATALOCATIONPROC glGetFragDataLocation;
		PFNGLUNIFORM1UIPROC glUniform1ui;
		PFNGLUNIFORM2UIPROC glUniform2ui;
		PFNGLUNIFORM3UIPROC glUniform3ui;
		PFNGLUNIFORM4UIPROC glUniform4ui;
		PFNGLUNIFORM1UIVPROC glUniform1uiv;
		PFNGLUNIFORM2UIVPROC glUniform2uiv;
		PFNGLUNIFORM3UIVPROC glUniform3uiv;
		PFNGLUNIFORM4UIVPROC glUniform4uiv;
		PFNGLTEXPARAMETERIIVPROC glTexParameterIiv;
		PFNGLTEXPARAMETERIUIVPROC glTexParameterIuiv;
		PFNGLGETTEXPARAMETERIIVPROC glGetTexParameterIiv;
		PFNGLGETTEXPARAMETERIUIVPROC glGetTexParameterIuiv;
		PFNGLCLEARBUFFERIVPROC glClearBufferiv;
		PFNGLCLEARBUFFERUIVPROC glClearBufferuiv;
		PFNGLCLEARBUFFERFVPROC glClearBufferfv;
		PFNGLCLEARBUFFERFIPROC glClearBufferfi;
		PFNGLGETSTRINGIPROC glGetStringi;
		PFNGLISRENDERBUFFERPROC glIsRenderbuffer;
		PFNGLBINDRENDERBUFFERPROC glBindRenderbuffer;
		PFNGLDELETERENDERBUFFERSPROC glDeleteRenderbuffers;
		PFNGLGENRENDERBUFFERSPROC glGenRenderbuffers;
		PFNGLRENDERBUFFERSTORAGEPROC glRenderbufferStorage;
		PFNGLGETRENDERBUFFERPARAMETERIVPROC glGetRenderbufferParameteriv;
		PFNGLISFRAMEBUFFERPROC glIsFramebuffer;
		PFNGLBINDFRAMEBUFFERPROC glBindFramebuffer;
		PFNGLDELETEFRAMEBUFFERSPROC glDeleteFramebuffers;
		PFNGLGENFRAMEBUFFERSPROC glGenFramebuffers;
		PFNGLCHECKFRAMEBUFFERSTATUSPROC glCheckFramebufferStatus;
		PFNGLFRAMEBUFFERTEXTURE1DPROC glFramebufferTexture1D;
		PFNGLFRAMEBUFFERTEXTURE2DPROC glFramebufferTexture2D;
		PFNGLFRAMEBUFFERTEXTURE3DPROC glFramebufferTexture3D;
		PFNGLFRAMEBUFFERRENDERBUFFERPROC glFramebufferRenderbuffer;
		PFNGLGETFRAMEBUFFERATTACHMENTPARAMETERIVPROC glGetFramebufferAttachmentParameteriv;
		PFNGLGENERATEMIPMAPPROC glGenerateMipmap;
		PFNGLBLITFRAMEBUFFERPROC glBlitFramebuffer;
		PFNGLRENDERBUFFERSTORAGEMULTISAMPLEPROC glRenderbufferStorageMultisample;
		PFNGLFRAMEBUFFERTEXTURELAYERPROC glFramebufferTextureLayer;
		PFNGLMAPBUFFERRANGEPROC glMapBufferRange;
		PFNGLFLUSHMAPPEDBUFFERRANGEPROC glFlushMappedBufferRange;
		PFNGLBINDVERTEXARRAYPROC glBindVertexArray;
		PFNGLDELETEVERTEXARRAYSPROC glDeleteVertexArrays;
		PFNGLGENVERTEXARRAYSPROC glGenVertexArrays;
		PFNGLISVERTEXARRAYPROC glIsVertexArray;
#endif

#ifdef GL_VERSION_3_1
		PFNGLDRAWARRAYSINSTANCEDPROC glDrawArraysInstanced;
		PFNGLDRAWELEMENTSINSTANCEDPROC glDrawElementsInstanced;
		PFNGLTEXBUFFERPROC glTexBuffer;
		PFNGLPRIMITIVERESTARTINDEXPROC glPrimitiveRestartIndex;
		PFNGLCOPYBUFFERSUBDATAPROC glCopyBufferSubData;
		PFNGLGETUNIFORMINDICESPROC glGetUniformIndices;
		PFNGLGETACTIVEUNIFORMSIVPROC glGetActiveUniformsiv;
		PFNGLGETACTIVEUNIFORMNAMEPROC glGetActiveUniformName;
		PFNGLGETUNIFORMBLOCKINDEXPROC glGetUniformBlockIndex;
		PFNGLGETACTIVEUNIFORMBLOCKIVPROC glGetActiveUniformBlockiv;
		PFNGLGETACTIVEUNIFORMBLOCKNAMEPROC glGetActiveUniformBlockName;
		PFNGLUNIFORMBLOCKBINDINGPROC glUniformBlockBinding;
#endif

#ifdef GL_VERSION_3_2
		PFNGLDRAWELEMENTSBASEVERTEXPROC glDrawElementsBaseVertex;
		PFNGLDRAWRANGEELEMENTSBASEVERTEXPROC glDrawRangeElementsBaseVertex;
		PFNGLDRAWELEMENTSINSTANCEDBASEVERTEXPROC glDrawElementsInstancedBaseVertex;
		PFNGLMULTIDRAWELEMENTSBASEVERTEXPROC glMultiDrawElementsBaseVertex;
		PFNGLPROVOKINGVERTEXPROC glProvokingVertex;
		PFNGLFENCESYNCPROC glFenceSync;
		PFNGLISSYNCPROC glIsSync;
		PFNGLDELETESYNCPROC glDeleteSync;
		PFNGLCLIENTWAITSYNCPROC glClientWaitSync;
		PFNGLWAITSYNCPROC glWaitSync;
		PFNGLGETINTEGER64VPROC glGetInteger64v;
		PFNGLGETSYNCIVPROC glGetSynciv;
		PFNGLGETINTEGER64I_VPROC glGetInteger64i_v;
		PFNGLGETBUFFERPARAMETERI64VPROC glGetBufferParameteri64v;
		PFNGLFRAMEBUFFERTEXTUREPROC glFramebufferTexture;
		PFNGLTEXIMAGE2DMULTISAMPLEPROC glTexImage2DMultisample;
		PFNGLTEXIMAGE3DMULTISAMPLEPROC glTexImage3DMultisample;
		PFNGLGETMULTISAMPLEFVPROC glGetMultisamplefv;
		PFNGLSAMPLEMASKIPROC glSampleMaski;
#endif

#ifdef GL_VERSION_3_3
		PFNGLBINDFRAGDATALOCATIONINDEXEDPROC glBindFragDataLocationIndexed;
		PFNGLGETFRAGDATAINDEXPROC glGetFragDataIndex;
		PFNGLGENSAMPLERSPROC glGenSamplers;
		PFNGLDELETESAMPLERSPROC glDeleteSamplers;
		PFNGLISSAMPLERPROC glIsSampler;
		PFNGLBINDSAMPLERPROC glBindSampler;
		PFNGLSAMPLERPARAMETERIPROC glSamplerParameteri;
		PFNGLSAMPLERPARAMETERIVPROC glSamplerParameteriv;
		PFNGLSAMPLERPARAMETERFPROC glSamplerParameterf;
		PFNGLSAMPLERPARAMETERFVPROC glSamplerParameterfv;
		PFNGLSAMPLERPARAMETERIIVPROC glSamplerParameterIiv;
		PFNGLSAMPLERPARAMETERIUIVPROC glSamplerParameterIuiv;
		PFNGLGETSAMPLERPARAMETERIVPROC glGetSamplerParameteriv;
		PFNGLGETSAMPLERPARAMETERIIVPROC glGetSamplerParameterIiv;
		PFNGLGETSAMPLERPARAMETERFVPROC glGetSamplerParameterfv;
		PFNGLGETSAMPLERPARAMETERIUIVPROC glGetSamplerParameterIuiv;
		PFNGLQUERYCOUNTERPROC glQueryCounter;
		PFNGLGETQUERYOBJECTI64VPROC glGetQueryObjecti64v;
		PFNGLGETQUERYOBJECTUI64VPROC glGetQueryObjectui64v;
		PFNGLVERTEXATTRIBDIVISORPROC glVertexAttribDivisor;
		PFNGLVERTEXATTRIBP1UIPROC glVertexAttribP1ui;
		PFNGLVERTEXATTRIBP1UIVPROC glVertexAttribP1uiv;
		PFNGLVERTEXATTRIBP2UIPROC glVertexAttribP2ui;
		PFNGLVERTEXATTRIBP2UIVPROC glVertexAttribP2uiv;
		PFNGLVERTEXATTRIBP3UIPROC glVertexAttribP3ui;
		PFNGLVERTEXATTRIBP3UIVPROC glVertexAttribP3uiv;
		PFNGLVERTEXATTRIBP4UIPROC glVertexAttribP4ui;
		PFNGLVERTEXATTRIBP4UIVPROC glVertexAttribP4uiv;
#endif

#ifdef GL_VERSION_4_0
		PFNGLMINSAMPLESHADINGPROC glMinSampleShading;
		PFNGLBLENDEQUATIONIPROC glBlendEquationi;
		PFNGLBLENDEQUATIONSEPARATEIPROC glBlendEquationSeparatei;
		PFNGLBLENDFUNCIPROC glBlendFunci;
		PFNGLBLENDFUNCSEPARATEIPROC glBlendFuncSeparatei;
		PFNGLDRAWARRAYSINDIRECTPROC glDrawArraysIndirect;
		PFNGLDRAWELEMENTSINDIRECTPROC glDrawElementsIndirect;
		PFNGLUNIFORM1DPROC glUniform1d;
		PFNGLUNIFORM2DPROC glUniform2d;
		PFNGLUNIFORM3DPROC glUniform3d;
		PFNGLUNIFORM4DPROC glUniform4d;
		PFNGLUNIFORM1DVPROC glUniform1dv;
		PFNGLUNIFORM2DVPROC glUniform2dv;
		PFNGLUNIFORM3DVPROC glUniform3dv;
		PFNGLUNIFORM4DVPROC glUniform4dv;
		PFNGLUNIFORMMATRIX2DVPROC glUniformMatrix2dv;
		PFNGLUNIFORMMATRIX3DVPROC glUniformMatrix3dv;
		PFNGLUNIFORMMATRIX4DVPROC glUniformMatrix4dv;
		PFNGLUNIFORMMATRIX2X3DVPROC glUniformMatrix2x3dv;
		PFNGLUNIFORMMATRIX2X4DVPROC glUniformMatrix2x4dv;
		PFNGLUNIFORMMATRIX3X2DVPROC glUniformMatrix3x2dv;
		PFNGLUNIFORMMATRIX3X4DVPROC glUniformMatrix3x4dv;
		PFNGLUNIFORMMATRIX4X2DVPROC glUniformMatrix4x2dv;
		PFNGLUNIFORMMATRIX4X3DVPROC glUniformMatrix4x3dv;
		PFNGLGETUNIFORMDVPROC glGetUniformdv;
		PFNGLGETSUBROUTINEUNIFORMLOCATIONPROC glGetSubroutineUniformLocation;
		PFNGLGETSUBROUTINEINDEXPROC glGetSubroutineIndex;
		PFNGLGETACTIVESUBROUTINEUNIFORMIVPROC glGetActiveSubroutineUniformiv;
		PFNGLGETACTIVESUBROUTINEUNIFORMNAMEPROC glGetActiveSubroutineUniformName;
		PFNGLGETACTIVESUBROUTINENAMEPROC glGetActiveSubroutineName;
		PFNGLUNIFORMSUBROUTINESUIVPROC glUniformSubroutinesuiv;
		PFNGLGETUNIFORMSUBROUTINEUIVPROC glGetUniformSubroutineuiv;
		PFNGLGETPROGRAMSTAGEIVPROC glGetProgramStageiv;
		PFNGLPATCHPARAMETERIPROC glPatchParameteri;
		PFNGLPATCHPARAMETERFVPROC glPatchParameterfv;
		PFNGLBINDTRANSFORMFEEDBACKPROC glBindTransformFeedback;
		PFNGLDELETETRANSFORMFEEDBACKSPROC glDeleteTransformFeedbacks;
		PFNGLGENTRANSFORMFEEDBACKSPROC glGenTransformFeedbacks;
		PFNGLISTRANSFORMFEEDBACKPROC glIsTransformFeedback;
		PFNGLPAUSETRANSFORMFEEDBACKPROC glPauseTransformFeedback;
		PFNGLRESUMETRANSFORMFEEDBACKPROC glResumeTransformFeedback;
		PFNGLDRAWTRANSFORMFEEDBACKPROC glDrawTransformFeedback;
		PFNGLDRAWTRANSFORMFEEDBACKSTREAMPROC glDrawTransformFeedbackStream;
		PFNGLBEGINQUERYINDEXEDPROC glBeginQueryIndexed;
		PFNGLENDQUERYINDEXEDPROC glEndQueryIndexed;
		PFNGLGETQUERYINDEXEDIVPROC glGetQueryIndexediv;
#endif

#ifdef GL_VERSION_4_1
		PFNGLRELEASESHADERCOMPILERPROC glReleaseShaderCompiler;
		PFNGLSHADERBINARYPROC glShaderBinary;
		PFNGLGETSHADERPRECISIONFORMATPROC glGetShaderPrecisionFormat;
		PFNGLDEPTHRANGEFPROC glDepthRangef;
		PFNGLCLEARDEPTHFPROC glClearDepthf;
		PFNGLGETPROGRAMBINARYPROC glGetProgramBinary;
		PFNGLPROGRAMBINARYPROC glProgramBinary;
		PFNGLPROGRAMPARAMETERIPROC glProgramParameteri;
		PFNGLUSEPROGRAMSTAGESPROC glUseProgramStages;
		PFNGLACTIVESHADERPROGRAMPROC glActiveShaderProgram;
		PFNGLCREATESHADERPROGRAMVPROC glCreateShaderProgramv;
		PFNGLBINDPROGRAMPIPELINEPROC glBindProgramPipeline;
		PFNGLDELETEPROGRAMPIPELINESPROC glDeleteProgramPipelines;
		PFNGLGENPROGRAMPIPELINESPROC glGenProgramPipelines;
		PFNGLISPROGRAMPIPELINEPROC glIsProgramPipeline;
		PFNGLGETPROGRAMPIPELINEIVPROC glGetProgramPipelineiv;
		PFNGLPROGRAMUNIFORM1IPROC glProgramUniform1i;
		PFNGLPROGRAMUNIFORM1IVPROC glProgramUniform1iv;
		PFNGLPROGRAMUNIFORM1FPROC glProgramUniform1f;
		PFNGLPROGRAMUNIFORM1FVPROC glProgramUniform1fv;
		PFNGLPROGRAMUNIFORM1DPROC glProgramUniform1d;
		PFNGLPROGRAMUNIFORM1DVPROC glProgramUniform1dv;
		PFNGLPROGRAMUNIFORM1UIPROC glProgramUniform1ui;
		PFNGLPROGRAMUNIFORM1UIVPROC glProgramUniform1uiv;
		PFNGLPROGRAMUNIFORM2IPROC glProgramUniform2i;
		PFNGLPROGRAMUNIFORM2IVPROC glProgramUniform2iv;
		PFNGLPROGRAMUNIFORM2FPROC glProgramUniform2f;
		PFNGLPROGRAMUNIFORM2FVPROC glProgramUniform2fv;
		PFNGLPROGRAMUNIFORM2DPROC glProgramUniform2d;
		PFNGLPROGRAMUNIFORM2DVPROC glProgramUniform2dv;
		PFNGLPROGRAMUNIFORM2UIPROC glProgramUniform2ui;
		PFNGLPROGRAMUNIFORM2UIVPROC glProgramUniform2uiv;
		PFNGLPROGRAMUNIFORM3IPROC glProgramUniform3i;
		PFNGLPROGRAMUNIFORM3IVPROC glProgramUniform3iv;
		PFNGLPROGRAMUNIFORM3FPROC glProgramUniform3f;
		PFNGLPROGRAMUNIFORM3FVPROC glProgramUniform3fv;
		PFNGLPROGRAMUNIFORM3DPROC glProgramUniform3d;
		PFNGLPROGRAMUNIFORM3DVPROC glProgramUniform3dv;
		PFNGLPROGRAMUNIFORM3UIPROC glProgramUniform3ui;
		PFNGLPROGRAMUNIFORM3UIVPROC glProgramUniform3uiv;
		PFNGLPROGRAMUNIFORM4IPROC glProgramUniform4i;
		PFNGLPROGRAMUNIFORM4IVPROC glProgramUniform4iv;
		PFNGLPROGRAMUNIFORM4FPROC glProgramUniform4f;
		PFNGLPROGRAMUNIFORM4FVPROC glProgramUniform4fv;
		PFNGLPROGRAMUNIFORM4DPROC glProgramUniform4d;
		PFNGLPROGRAMUNIFORM4DVPROC glProgramUniform4dv;
		PFNGLPROGRAMUNIFORM4UIPROC glProgramUniform4ui;
		PFNGLPROGRAMUNIFORM4UIVPROC glProgramUniform4uiv;
		PFNGLPROGRAMUNIFORMMATRIX2FVPROC glProgramUniformMatrix2fv;
		PFNGLPROGRAMUNIFORMMATRIX3FVPROC glProgramUniformMatrix3fv;
		PFNGLPROGRAMUNIFORMMATRIX4FVPROC glProgramUniformMatrix4fv;
		PFNGLPROGRAMUNIFORMMATRIX2DVPROC glProgramUniformMatrix2dv;
		PFNGLPROGRAMUNIFORMMATRIX3DVPROC glProgramUniformMatrix3dv;
		PFNGLPROGRAMUNIFORMMATRIX4DVPROC glProgramUniformMatrix4dv;
		PFNGLPROGRAMUNIFORMMATRIX2X3FVPROC glProgramUniformMatrix2x3fv;
		PFNGLPROGRAMUNIFORMMATRIX3X2FVPROC glProgramUniformMatrix3x2fv;
		PFNGLPROGRAMUNIFORMMATRIX2X4FVPROC glProgramUniformMatrix2x4fv;
		PFNGLPROGRAMUNIFORMMATRIX4X2FVPROC glProgramUniformMatrix4x2fv;
		PFNGLPROGRAMUNIFORMMATRIX3X4FVPROC glProgramUniformMatrix3x4fv;
		PFNGLPROGRAMUNIFORMMATRIX4X3FVPROC glProgramUniformMatrix4x3fv;
		PFNGLPROGRAMUNIFORMMATRIX2X3DVPROC glProgramUniformMatrix2x3dv;
		PFNGLPROGRAMUNIFORMMATRIX3X2DVPROC glProgramUniformMatrix3x2dv;
		PFNGLPROGRAMUNIFORMMATRIX2X4DVPROC glProgramUniformMatrix2x4dv;
		PFNGLPROGRAMUNIFORMMATRIX4X2DVPROC glProgramUniformMatrix4x2dv;
		PFNGLPROGRAMUNIFORMMATRIX3X4DVPROC glProgramUniformMatrix3x4dv;
		PFNGLPROGRAMUNIFORMMATRIX4X3DVPROC glProgramUniformMatrix4x3dv;
		PFNGLVALIDATEPROGRAMPIPELINEPROC glValidateProgramPipeline;
		PFNGLGETPROGRAMPIPELINEINFOLOGPROC glGetProgramPipelineInfoLog;
		PFNGLVERTEXATTRIBL1DPROC glVertexAttribL1d;
		PFNGLVERTEXATTRIBL2DPROC glVertexAttribL2d;
		PFNGLVERTEXATTRIBL3DPROC glVertexAttribL3d;
		PFNGLVERTEXATTRIBL4DPROC glVertexAttribL4d;
		PFNGLVERTEXATTRIBL1DVPROC glVertexAttribL1dv;
		PFNGLVERTEXATTRIBL2DVPROC glVertexAttribL2dv;
		PFNGLVERTEXATTRIBL3DVPROC glVertexAttribL3dv;
		PFNGLVERTEXATTRIBL4DVPROC glVertexAttribL4dv;
		PFNGLVERTEXATTRIBLPOINTERPROC glVertexAttribLPointer;
		PFNGLGETVERTEXATTRIBLDVPROC glGetVertexAttribLdv;
		PFNGLVIEWPORTARRAYVPROC glViewportArrayv;
		PFNGLVIEWPORTINDEXEDFPROC glViewportIndexedf;
		PFNGLVIEWPORTINDEXEDFVPROC glViewportIndexedfv;
		PFNGLSCISSORARRAYVPROC glScissorArrayv;
		PFNGLSCISSORINDEXEDPROC glScissorIndexed;
		PFNGLSCISSORINDEXEDVPROC glScissorIndexedv;
		PFNGLDEPTHRANGEARRAYVPROC glDepthRangeArrayv;
		PFNGLDEPTHRANGEINDEXEDPROC glDepthRangeIndexed;
		PFNGLGETFLOATI_VPROC glGetFloati_v;
		PFNGLGETDOUBLEI_VPROC glGetDoublei_v;
#endif

#ifdef GL_VERSION_4_2
		PFNGLDRAWARRAYSINSTANCEDBASEINSTANCEPROC glDrawArraysInstancedBaseInstance;
		PFNGLDRAWELEMENTSINSTANCEDBASEINSTANCEPROC glDrawElementsInstancedBaseInstance;
		PFNGLDRAWELEMENTSINSTANCEDBASEVERTEXBASEINSTANCEPROC glDrawElementsInstancedBaseVertexBaseInstance;
		PFNGLGETINTERNALFORMATIVPROC glGetInternalformativ;
		PFNGLGETACTIVEATOMICCOUNTERBUFFERIVPROC glGetActiveAtomicCounterBufferiv;
		PFNGLBINDIMAGETEXTUREPROC glBindImageTexture;
		PFNGLMEMORYBARRIERPROC glMemoryBarrier;
		PFNGLTEXSTORAGE1DPROC glTexStorage1D;
		PFNGLTEXSTORAGE2DPROC glTexStorage2D;
		PFNGLTEXSTORAGE3DPROC glTexStorage3D;
		PFNGLDRAWTRANSFORMFEEDBACKINSTANCEDPROC glDrawTransformFeedbackInstanced;
		PFNGLDRAWTRANSFORMFEEDBACKSTREAMINSTANCEDPROC glDrawTransformFeedbackStreamInstanced;
#endif

#ifdef GL_VERSION_4_3
		PFNGLCLEARBUFFERDATAPROC glClearBufferData;
		PFNGLCLEARBUFFERSUBDATAPROC glClearBufferSubData;
		PFNGLDISPATCHCOMPUTEPROC glDispatchCompute;
		PFNGLDISPATCHCOMPUTEINDIRECTPROC glDispatchComputeIndirect;
		PFNGLCOPYIMAGESUBDATAPROC glCopyImageSubData;
		PFNGLFRAMEBUFFERPARAMETERIPROC glFramebufferParameteri;
		PFNGLGETFRAMEBUFFERPARAMETERIVPROC glGetFramebufferParameteriv;
		PFNGLGETINTERNALFORMATI64VPROC glGetInternalformati64v;
		PFNGLINVALIDATETEXSUBIMAGEPROC glInvalidateTexSubImage;
		PFNGLINVALIDATETEXIMAGEPROC glInvalidateTexImage;
		PFNGLINVALIDATEBUFFERSUBDATAPROC glInvalidateBufferSubData;
		PFNGLINVALIDATEBUFFERDATAPROC glInvalidateBufferData;
		PFNGLINVALIDATEFRAMEBUFFERPROC glInvalidateFramebuffer;
		PFNGLINVALIDATESUBFRAMEBUFFERPROC glInvalidateSubFramebuffer;
		PFNGLMULTIDRAWARRAYSINDIRECTPROC glMultiDrawArraysIndirect;
		PFNGLMULTIDRAWELEMENTSINDIRECTPROC glMultiDrawElementsIndirect;
		PFNGLGETPROGRAMINTERFACEIVPROC glGetProgramInterfaceiv;
		PFNGLGETPROGRAMRESOURCEINDEXPROC glGetProgramResourceIndex;
		PFNGLGETPROGRAMRESOURCENAMEPROC glGetProgramResourceName;
		PFNGLGETPROGRAMRESOURCEIVPROC glGetProgramResourceiv;
		PFNGLGETPROGRAMRESOURCELOCATIONPROC glGetProgramResourceLocation;
		PFNGLGETPROGRAMRESOURCELOCATIONINDEXPROC glGetProgramResourceLocationIndex;
		PFNGLSHADERSTORAGEBLOCKBINDINGPROC glShaderStorageBlockBinding;
		PFNGLTEXBUFFERRANGEPROC glTexBufferRange;
		PFNGLTEXSTORAGE2DMULTISAMPLEPROC glTexStorage2DMultisample;
		PFNGLTEXSTORAGE3DMULTISAMPLEPROC glTexStorage3DMultisample;
		PFNGLTEXTUREVIEWPROC glTextureView;
		PFNGLBINDVERTEXBUFFERPROC glBindVertexBuffer;
		PFNGLVERTEXATTRIBFORMATPROC glVertexAttribFormat;
		PFNGLVERTEXATTRIBIFORMATPROC glVertexAttribIFormat;
		PFNGLVERTEXATTRIBLFORMATPROC glVertexAttribLFormat;
		PFNGLVERTEXATTRIBBINDINGPROC glVertexAttribBinding;
		PFNGLVERTEXBINDINGDIVISORPROC glVertexBindingDivisor;
		PFNGLDEBUGMESSAGECONTROLPROC glDebugMessageControl;
		PFNGLDEBUGMESSAGEINSERTPROC glDebugMessageInsert;
		PFNGLDEBUGMESSAGECALLBACKPROC glDebugMessageCallback;
		PFNGLGETDEBUGMESSAGELOGPROC glGetDebugMessageLog;
		PFNGLPUSHDEBUGGROUPPROC glPushDebugGroup;
		PFNGLPOPDEBUGGROUPPROC glPopDebugGroup;
		PFNGLOBJECTLABELPROC glObjectLabel;
		PFNGLGETOBJECTLABELPROC glGetObjectLabel;
		PFNGLOBJECTPTRLABELPROC glObjectPtrLabel;
		PFNGLGETOBJECTPTRLABELPROC glGetObjectPtrLabel;
#endif

#ifdef GL_VERSION_4_4
		PFNGLBUFFERSTORAGEPROC glBufferStorage;
		PFNGLCLEARTEXIMAGEPROC glClearTexImage;
		PFNGLCLEARTEXSUBIMAGEPROC glClearTexSubImage;
		PFNGLBINDBUFFERSBASEPROC glBindBuffersBase;
		PFNGLBINDBUFFERSRANGEPROC glBindBuffersRange;
		PFNGLBINDTEXTURESPROC glBindTextures;
		PFNGLBINDSAMPLERSPROC glBindSamplers;
		PFNGLBINDIMAGETEXTURESPROC glBindImageTextures;
		PFNGLBINDVERTEXBUFFERSPROC glBindVertexBuffers;
#endif

#ifdef GL_VERSION_4_5
		PFNGLCLIPCONTROLPROC glClipControl;
		PFNGLCREATETRANSFORMFEEDBACKSPROC glCreateTransformFeedbacks;
		PFNGLTRANSFORMFEEDBACKBUFFERBASEPROC glTransformFeedbackBufferBase;
		PFNGLTRANSFORMFEEDBACKBUFFERRANGEPROC glTransformFeedbackBufferRange;
		PFNGLGETTRANSFORMFEEDBACKIVPROC glGetTransformFeedbackiv;
		PFNGLGETTRANSFORMFEEDBACKI_VPROC glGetTransformFeedbacki_v;
		PFNGLGETTRANSFORMFEEDBACKI64_VPROC glGetTransformFeedbacki64_v;
		PFNGLCREATEBUFFERSPROC glCreateBuffers;
		PFNGLNAMEDBUFFERSTORAGEPROC glNamedBufferStorage;
		PFNGLNAMEDBUFFERDATAPROC glNamedBufferData;
		PFNGLNAMEDBUFFERSUBDATAPROC glNamedBufferSubData;
		PFNGLCOPYNAMEDBUFFERSUBDATAPROC glCopyNamedBufferSubData;
		PFNGLCLEARNAMEDBUFFERDATAPROC glClearNamedBufferData;
		PFNGLCLEARNAMEDBUFFERSUBDATAPROC glClearNamedBufferSubData;
		PFNGLMAPNAMEDBUFFERPROC glMapNamedBuffer;
		PFNGLMAPNAMEDBUFFERRANGEPROC glMapNamedBufferRange;
		PFNGLUNMAPNAMEDBUFFERPROC glUnmapNamedBuffer;
		PFNGLFLUSHMAPPEDNAMEDBUFFERRANGEPROC glFlushMappedNamedBufferRange;
		PFNGLGETNAMEDBUFFERPARAMETERIVPROC glGetNamedBufferParameteriv;
		PFNGLGETNAMEDBUFFERPARAMETERI64VPROC glGetNamedBufferParameteri64v;
		PFNGLGETNAMEDBUFFERPOINTERVPROC glGetNamedBufferPointerv;
		PFNGLGETNAMEDBUFFERSUBDATAPROC glGetNamedBufferSubData;
		PFNGLCREATEFRAMEBUFFERSPROC glCreateFramebuffers;
		PFNGLNAMEDFRAMEBUFFERRENDERBUFFERPROC glNamedFramebufferRenderbuffer;
		PFNGLNAMEDFRAMEBUFFERPARAMETERIPROC glNamedFramebufferParameteri;
		PFNGLNAMEDFRAMEBUFFERTEXTUREPROC glNamedFramebufferTexture;
		PFNGLNAMEDFRAMEBUFFERTEXTURELAYERPROC glNamedFramebufferTextureLayer;
		PFNGLNAMEDFRAMEBUFFERDRAWBUFFERPROC glNamedFramebufferDrawBuffer;
		PFNGLNAMEDFRAMEBUFFERDRAWBUFFERSPROC glNamedFramebufferDrawBuffers;
		PFNGLNAMEDFRAMEBUFFERREADBUFFERPROC glNamedFramebufferReadBuffer;
		PFNGLINVALIDATENAMEDFRAMEBUFFERDATAPROC glInvalidateNamedFramebufferData;
		PFNGLINVALIDATENAMEDFRAMEBUFFERSUBDATAPROC glInvalidateNamedFramebufferSubData;
		PFNGLCLEARNAMEDFRAMEBUFFERIVPROC glClearNamedFramebufferiv;
		PFNGLCLEARNAMEDFRAMEBUFFERUIVPROC glClearNamedFramebufferuiv;
		PFNGLCLEARNAMEDFRAMEBUFFERFVPROC glClearNamedFramebufferfv;
		PFNGLCLEARNAMEDFRAMEBUFFERFIPROC glClearNamedFramebufferfi;
		PFNGLBLITNAMEDFRAMEBUFFERPROC glBlitNamedFramebuffer;
		PFNGLCHECKNAMEDFRAMEBUFFERSTATUSPROC glCheckNamedFramebufferStatus;
		PFNGLGETNAMEDFRAMEBUFFERPARAMETERIVPROC glGetNamedFramebufferParameteriv;
		PFNGLGETNAMEDFRAMEBUFFERATTACHMENTPARAMETERIVPROC glGetNamedFramebufferAttachmentParameteriv;
		PFNGLCREATERENDERBUFFERSPROC glCreateRenderbuffers;
		PFNGLNAMEDRENDERBUFFERSTORAGEPROC glNamedRenderbufferStorage;
		PFNGLNAMEDRENDERBUFFERSTORAGEMULTISAMPLEPROC glNamedRenderbufferStorageMultisample;
		PFNGLGETNAMEDRENDERBUFFERPARAMETERIVPROC glGetNamedRenderbufferParameteriv;
		PFNGLCREATETEXTURESPROC glCreateTextures;
		PFNGLTEXTUREBUFFERPROC glTextureBuffer;
		PFNGLTEXTUREBUFFERRANGEPROC glTextureBufferRange;
		PFNGLTEXTURESTORAGE1DPROC glTextureStorage1D;
		PFNGLTEXTURESTORAGE2DPROC glTextureStorage2D;
		PFNGLTEXTURESTORAGE3DPROC glTextureStorage3D;
		PFNGLTEXTURESTORAGE2DMULTISAMPLEPROC glTextureStorage2DMultisample;
		PFNGLTEXTURESTORAGE3DMULTISAMPLEPROC glTextureStorage3DMultisample;
		PFNGLTEXTURESUBIMAGE1DPROC glTextureSubImage1D;
		PFNGLTEXTURESUBIMAGE2DPROC glTextureSubImage2D;
		PFNGLTEXTURESUBIMAGE3DPROC glTextureSubImage3D;
		PFNGLCOMPRESSEDTEXTURESUBIMAGE1DPROC glCompressedTextureSubImage1D;
		PFNGLCOMPRESSEDTEXTURESUBIMAGE2DPROC glCompressedTextureSubImage2D;
		PFNGLCOMPRESSEDTEXTURESUBIMAGE3DPROC glCompressedTextureSubImage3D;
		PFNGLCOPYTEXTURESUBIMAGE1DPROC glCopyTextureSubImage1D;
		PFNGLCOPYTEXTURESUBIMAGE2DPROC glCopyTextureSubImage2D;
		PFNGLCOPYTEXTURESUBIMAGE3DPROC glCopyTextureSubImage3D;
		PFNGLTEXTUREPARAMETERFPROC glTextureParameterf;
		PFNGLTEXTUREPARAMETERFVPROC glTextureParameterfv;
		PFNGLTEXTUREPARAMETERIPROC glTextureParameteri;
		PFNGLTEXTUREPARAMETERIIVPROC glTextureParameterIiv;
		PFNGLTEXTUREPARAMETERIUIVPROC glTextureParameterIuiv;
		PFNGLTEXTUREPARAMETERIVPROC glTextureParameteriv;
		PFNGLGENERATETEXTUREMIPMAPPROC glGenerateTextureMipmap;
		PFNGLBINDTEXTUREUNITPROC glBindTextureUnit;
		PFNGLGETTEXTUREIMAGEPROC glGetTextureImage;
		PFNGLGETCOMPRESSEDTEXTUREIMAGEPROC glGetCompressedTextureImage;
		PFNGLGETTEXTURELEVELPARAMETERFVPROC glGetTextureLevelParameterfv;
		PFNGLGETTEXTURELEVELPARAMETERIVPROC glGetTextureLevelParameteriv;
		PFNGLGETTEXTUREPARAMETERFVPROC glGetTextureParameterfv;
		PFNGLGETTEXTUREPARAMETERIIVPROC glGetTextureParameterIiv;
		PFNGLGETTEXTUREPARAMETERIUIVPROC glGetTextureParameterIuiv;
		PFNGLGETTEXTUREPARAMETERIVPROC glGetTextureParameteriv;
		PFNGLCREATEVERTEXARRAYSPROC glCreateVertexArrays;
		PFNGLDISABLEVERTEXARRAYATTRIBPROC glDisableVertexArrayAttrib;
		PFNGLENABLEVERTEXARRAYATTRIBPROC glEnableVertexArrayAttrib;
		PFNGLVERTEXARRAYELEMENTBUFFERPROC glVertexArrayElementBuffer;
		PFNGLVERTEXARRAYVERTEXBUFFERPROC glVertexArrayVertexBuffer;
		PFNGLVERTEXARRAYVERTEXBUFFERSPROC glVertexArrayVertexBuffers;
		PFNGLVERTEXARRAYATTRIBBINDINGPROC glVertexArrayAttribBinding;
		PFNGLVERTEXARRAYATTRIBFORMATPROC glVertexArrayAttribFormat;
		PFNGLVERTEXARRAYATTRIBIFORMATPROC glVertexArrayAttribIFormat;
		PFNGLVERTEXARRAYATTRIBLFORMATPROC glVertexArrayAttribLFormat;
		PFNGLVERTEXARRAYBINDINGDIVISORPROC glVertexArrayBindingDivisor;
		PFNGLGETVERTEXARRAYIVPROC glGetVertexArrayiv;
		PFNGLGETVERTEXARRAYINDEXEDIVPROC glGetVertexArrayIndexediv;
		PFNGLGETVERTEXARRAYINDEXED64IVPROC glGetVertexArrayIndexed64iv;
		PFNGLCREATESAMPLERSPROC glCreateSamplers;
		PFNGLCREATEPROGRAMPIPELINESPROC glCreateProgramPipelines;
		PFNGLCREATEQUERIESPROC glCreateQueries;
		PFNGLMEMORYBARRIERBYREGIONPROC glMemoryBarrierByRegion;
		PFNGLGETTEXTURESUBIMAGEPROC glGetTextureSubImage;
		PFNGLGETCOMPRESSEDTEXTURESUBIMAGEPROC glGetCompressedTextureSubImage;
		PFNGLGETGRAPHICSRESETSTATUSPROC glGetGraphicsResetStatus;
		PFNGLGETNCOMPRESSEDTEXIMAGEPROC glGetnCompressedTexImage;
		PFNGLGETNTEXIMAGEPROC glGetnTexImage;
		PFNGLGETNUNIFORMDVPROC glGetnUniformdv;
		PFNGLGETNUNIFORMFVPROC glGetnUniformfv;
		PFNGLGETNUNIFORMIVPROC glGetnUniformiv;
		PFNGLGETNUNIFORMUIVPROC glGetnUniformuiv;
		PFNGLREADNPIXELSPROC glReadnPixels;
		PFNGLTEXTUREBARRIERPROC glTextureBarrier;
#endif

#ifdef GL_ARB_bindless_texture
		PFNGLGETTEXTUREHANDLEARBPROC glGetTextureHandleARB;
		PFNGLGETTEXTURESAMPLERHANDLEARBPROC glGetTextureSamplerHandleARB;
		PFNGLMAKETEXTUREHANDLERESIDENTARBPROC glMakeTextureHandleResidentARB;
		PFNGLMAKETEXTUREHANDLENONRESIDENTARBPROC glMakeTextureHandleNonResidentARB;
		PFNGLGETIMAGEHANDLEARBPROC glGetImageHandleARB;
		PFNGLMAKEIMAGEHANDLERESIDENTARBPROC glMakeImageHandleResidentARB;
		PFNGLMAKEIMAGEHANDLENONRESIDENTARBPROC glMakeImageHandleNonResidentARB;
		PFNGLUNIFORMHANDLEUI64ARBPROC glUniformHandleui64ARB;
		PFNGLUNIFORMHANDLEUI64VARBPROC glUniformHandleui64vARB;
		PFNGLPROGRAMUNIFORMHANDLEUI64ARBPROC glProgramUniformHandleui64ARB;
		PFNGLPROGRAMUNIFORMHANDLEUI64VARBPROC glProgramUniformHandleui64vARB;
		PFNGLISTEXTUREHANDLERESIDENTARBPROC glIsTextureHandleResidentARB;
		PFNGLISIMAGEHANDLERESIDENTARBPROC glIsImageHandleResidentARB;
		PFNGLVERTEXATTRIBL1UI64ARBPROC glVertexAttribL1ui64ARB;
		PFNGLVERTEXATTRIBL1UI64VARBPROC glVertexAttribL1ui64vARB;
		PFNGLGETVERTEXATTRIBLUI64VARBPROC glGetVertexAttribLui64vARB;
#endif

#ifdef GL_ARB_cl_event
		PFNGLCREATESYNCFROMCLEVENTARBPROC glCreateSyncFromCLeventARB;
#endif

#ifdef GL_ARB_compute_variable_group_size
		PFNGLDISPATCHCOMPUTEGROUPSIZEARBPROC glDispatchComputeGroupSizeARB;
#endif

#ifdef GL_ARB_debug_output
#endif

#ifdef GL_ARB_draw_buffers_blend
#endif

#ifdef GL_ARB_indirect_parameters
		PFNGLMULTIDRAWARRAYSINDIRECTCOUNTARBPROC glMultiDrawArraysIndirectCountARB;
		PFNGLMULTIDRAWELEMENTSINDIRECTCOUNTARBPROC glMultiDrawElementsIndirectCountARB;
#endif

#ifdef GL_ARB_robustness
#endif

#ifdef GL_ARB_sample_shading
#endif

#ifdef GL_ARB_shading_language_include
		PFNGLNAMEDSTRINGARBPROC glNamedStringARB;
		PFNGLDELETENAMEDSTRINGARBPROC glDeleteNamedStringARB;
		PFNGLCOMPILESHADERINCLUDEARBPROC glCompileShaderIncludeARB;
		PFNGLISNAMEDSTRINGARBPROC glIsNamedStringARB;
		PFNGLGETNAMEDSTRINGARBPROC glGetNamedStringARB;
		PFNGLGETNAMEDSTRINGIVARBPROC glGetNamedStringivARB;
#endif

#ifdef GL_ARB_sparse_buffer
		PFNGLBUFFERPAGECOMMITMENTARBPROC glBufferPageCommitmentARB;
		PFNGLNAMEDBUFFERPAGECOMMITMENTEXTPROC glNamedBufferPageCommitmentEXT;
		PFNGLNAMEDBUFFERPAGECOMMITMENTARBPROC glNamedBufferPageCommitmentARB;
#endif

#ifdef GL_ARB_sparse_texture
		PFNGLTEXPAGECOMMITMENTARBPROC glTexPageCommitmentARB;
#endif

		bool OpenGLLoadLibrary(const char* const glLibName)
		{
			if (openGLModule != nullptr)
				return false;

			auto openGLLibraryNameWChar = HorseRadish::StringUtils::conv2UTF16(glLibName);
			
			openGLModule = LoadLibrary(openGLLibraryNameWChar.c_str());
			return (openGLModule != nullptr);
		}

		bool OpenGLUnloadLibrary(void)
		{
			if (openGLModule == nullptr)
				return false;

			FreeLibrary(openGLModule);
			openGLModule = nullptr;
			return true;
		}

		bool OpenGLGetProcs(void)
		{
			if (openGLModule == nullptr)
				return false;

			auto ptrWGlGetProcAddress = (PFNWGLGETPROCADDRESSPROC)GetProcAddress(openGLModule, "wglGetProcAddress");
			if (ptrWGlGetProcAddress == nullptr)
				return false;

			if (!loadGLFunctions(openGLModule, ptrWGlGetProcAddress))
			{
				FreeLibrary(openGLModule);
				openGLModule = nullptr;
				return false;
			}

			return true;
		}
	}
}