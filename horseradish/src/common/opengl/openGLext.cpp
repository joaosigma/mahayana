#include "openGLext.hpp"

#include "common/stringUtils.hpp"

#define GETADDR(var, name, type) var = (type)wglProcAddressOpenGL(name);

namespace hr::gl::extensions
{
    namespace
    {
        typedef PROC(APIENTRY* PFNWGLGETPROCADDRESSPROC)(LPCSTR lpcstr);

        void loadGLExtFunctions(PFNWGLGETPROCADDRESSPROC wglProcAddressOpenGL)
        {
#ifdef GL_NV_path_rendering
            GETADDR(glGenPathsNV, "glGenPathsNV", PFNGLGENPATHSNVPROC);
            GETADDR(glDeletePathsNV, "glDeletePathsNV", PFNGLDELETEPATHSNVPROC);
            GETADDR(glIsPathNV, "glIsPathNV", PFNGLISPATHNVPROC);
            GETADDR(glPathCommandsNV, "glPathCommandsNV", PFNGLPATHCOMMANDSNVPROC);
            GETADDR(glPathCoordsNV, "glPathCoordsNV", PFNGLPATHCOORDSNVPROC);
            GETADDR(glPathSubCommandsNV, "glPathSubCommandsNV", PFNGLPATHSUBCOMMANDSNVPROC);
            GETADDR(glPathSubCoordsNV, "glPathSubCoordsNV", PFNGLPATHSUBCOORDSNVPROC);
            GETADDR(glPathStringNV, "glPathStringNV", PFNGLPATHSTRINGNVPROC);
            GETADDR(glPathGlyphsNV, "glPathGlyphsNV", PFNGLPATHGLYPHSNVPROC);
            GETADDR(glPathGlyphRangeNV, "glPathGlyphRangeNV", PFNGLPATHGLYPHRANGENVPROC);
            GETADDR(glWeightPathsNV, "glWeightPathsNV", PFNGLWEIGHTPATHSNVPROC);
            GETADDR(glCopyPathNV, "glCopyPathNV", PFNGLCOPYPATHNVPROC);
            GETADDR(glInterpolatePathsNV, "glInterpolatePathsNV", PFNGLINTERPOLATEPATHSNVPROC);
            GETADDR(glTransformPathNV, "glTransformPathNV", PFNGLTRANSFORMPATHNVPROC);
            GETADDR(glPathParameterivNV, "glPathParameterivNV", PFNGLPATHPARAMETERIVNVPROC);
            GETADDR(glPathParameteriNV, "glPathParameteriNV", PFNGLPATHPARAMETERINVPROC);
            GETADDR(glPathParameterfvNV, "glPathParameterfvNV", PFNGLPATHPARAMETERFVNVPROC);
            GETADDR(glPathParameterfNV, "glPathParameterfNV", PFNGLPATHPARAMETERFNVPROC);
            GETADDR(glPathDashArrayNV, "glPathDashArrayNV", PFNGLPATHDASHARRAYNVPROC);
            GETADDR(glPathStencilFuncNV, "glPathStencilFuncNV", PFNGLPATHSTENCILFUNCNVPROC);
            GETADDR(glPathStencilDepthOffsetNV, "glPathStencilDepthOffsetNV", PFNGLPATHSTENCILDEPTHOFFSETNVPROC);
            GETADDR(glStencilFillPathNV, "glStencilFillPathNV", PFNGLSTENCILFILLPATHNVPROC);
            GETADDR(glStencilStrokePathNV, "glStencilStrokePathNV", PFNGLSTENCILSTROKEPATHNVPROC);
            GETADDR(glStencilFillPathInstancedNV, "glStencilFillPathInstancedNV", PFNGLSTENCILFILLPATHINSTANCEDNVPROC);
            GETADDR(glStencilStrokePathInstancedNV, "glStencilStrokePathInstancedNV", PFNGLSTENCILSTROKEPATHINSTANCEDNVPROC);
            GETADDR(glPathCoverDepthFuncNV, "glPathCoverDepthFuncNV", PFNGLPATHCOVERDEPTHFUNCNVPROC);
            GETADDR(glCoverFillPathNV, "glCoverFillPathNV", PFNGLCOVERFILLPATHNVPROC);
            GETADDR(glCoverStrokePathNV, "glCoverStrokePathNV", PFNGLCOVERSTROKEPATHNVPROC);
            GETADDR(glCoverFillPathInstancedNV, "glCoverFillPathInstancedNV", PFNGLCOVERFILLPATHINSTANCEDNVPROC);
            GETADDR(glCoverStrokePathInstancedNV, "glCoverStrokePathInstancedNV", PFNGLCOVERSTROKEPATHINSTANCEDNVPROC);
            GETADDR(glGetPathParameterivNV, "glGetPathParameterivNV", PFNGLGETPATHPARAMETERIVNVPROC);
            GETADDR(glGetPathParameterfvNV, "glGetPathParameterfvNV", PFNGLGETPATHPARAMETERFVNVPROC);
            GETADDR(glGetPathCommandsNV, "glGetPathCommandsNV", PFNGLGETPATHCOMMANDSNVPROC);
            GETADDR(glGetPathCoordsNV, "glGetPathCoordsNV", PFNGLGETPATHCOORDSNVPROC);
            GETADDR(glGetPathDashArrayNV, "glGetPathDashArrayNV", PFNGLGETPATHDASHARRAYNVPROC);
            GETADDR(glGetPathMetricsNV, "glGetPathMetricsNV", PFNGLGETPATHMETRICSNVPROC);
            GETADDR(glGetPathMetricRangeNV, "glGetPathMetricRangeNV", PFNGLGETPATHMETRICRANGENVPROC);
            GETADDR(glGetPathSpacingNV, "glGetPathSpacingNV", PFNGLGETPATHSPACINGNVPROC);
            GETADDR(glIsPointInFillPathNV, "glIsPointInFillPathNV", PFNGLISPOINTINFILLPATHNVPROC);
            GETADDR(glIsPointInStrokePathNV, "glIsPointInStrokePathNV", PFNGLISPOINTINSTROKEPATHNVPROC);
            GETADDR(glGetPathLengthNV, "glGetPathLengthNV", PFNGLGETPATHLENGTHNVPROC);
            GETADDR(glPointAlongPathNV, "glPointAlongPathNV", PFNGLPOINTALONGPATHNVPROC);
            GETADDR(glMatrixLoad3x2fNV, "glMatrixLoad3x2fNV", PFNGLMATRIXLOAD3X2FNVPROC);
            GETADDR(glMatrixLoad3x3fNV, "glMatrixLoad3x3fNV", PFNGLMATRIXLOAD3X3FNVPROC);
            GETADDR(glMatrixLoadTranspose3x3fNV, "glMatrixLoadTranspose3x3fNV", PFNGLMATRIXLOADTRANSPOSE3X3FNVPROC);
            GETADDR(glMatrixMult3x2fNV, "glMatrixMult3x2fNV", PFNGLMATRIXMULT3X2FNVPROC);
            GETADDR(glMatrixMult3x3fNV, "glMatrixMult3x3fNV", PFNGLMATRIXMULT3X3FNVPROC);
            GETADDR(glMatrixMultTranspose3x3fNV, "glMatrixMultTranspose3x3fNV", PFNGLMATRIXMULTTRANSPOSE3X3FNVPROC);
            GETADDR(glStencilThenCoverFillPathNV, "glStencilThenCoverFillPathNV", PFNGLSTENCILTHENCOVERFILLPATHNVPROC);
            GETADDR(glStencilThenCoverStrokePathNV, "glStencilThenCoverStrokePathNV", PFNGLSTENCILTHENCOVERSTROKEPATHNVPROC);
            GETADDR(glStencilThenCoverFillPathInstancedNV, "glStencilThenCoverFillPathInstancedNV", PFNGLSTENCILTHENCOVERFILLPATHINSTANCEDNVPROC);
            GETADDR(glStencilThenCoverStrokePathInstancedNV, "glStencilThenCoverStrokePathInstancedNV", PFNGLSTENCILTHENCOVERSTROKEPATHINSTANCEDNVPROC);
            GETADDR(glPathGlyphIndexRangeNV, "glPathGlyphIndexRangeNV", PFNGLPATHGLYPHINDEXRANGENVPROC);
            GETADDR(glPathGlyphIndexArrayNV, "glPathGlyphIndexArrayNV", PFNGLPATHGLYPHINDEXARRAYNVPROC);
            GETADDR(glPathMemoryGlyphIndexArrayNV, "glPathMemoryGlyphIndexArrayNV", PFNGLPATHMEMORYGLYPHINDEXARRAYNVPROC);
            GETADDR(glProgramPathFragmentInputGenNV, "glProgramPathFragmentInputGenNV", PFNGLPROGRAMPATHFRAGMENTINPUTGENNVPROC);
            GETADDR(glGetProgramResourcefvNV, "glGetProgramResourcefvNV", PFNGLGETPROGRAMRESOURCEFVNVPROC);
#endif
        }
    }

#ifdef GL_NV_path_rendering
    PFNGLGENPATHSNVPROC glGenPathsNV;
    PFNGLDELETEPATHSNVPROC glDeletePathsNV;
    PFNGLISPATHNVPROC glIsPathNV;
    PFNGLPATHCOMMANDSNVPROC glPathCommandsNV;
    PFNGLPATHCOORDSNVPROC glPathCoordsNV;
    PFNGLPATHSUBCOMMANDSNVPROC glPathSubCommandsNV;
    PFNGLPATHSUBCOORDSNVPROC glPathSubCoordsNV;
    PFNGLPATHSTRINGNVPROC glPathStringNV;
    PFNGLPATHGLYPHSNVPROC glPathGlyphsNV;
    PFNGLPATHGLYPHRANGENVPROC glPathGlyphRangeNV;
    PFNGLWEIGHTPATHSNVPROC glWeightPathsNV;
    PFNGLCOPYPATHNVPROC glCopyPathNV;
    PFNGLINTERPOLATEPATHSNVPROC glInterpolatePathsNV;
    PFNGLTRANSFORMPATHNVPROC glTransformPathNV;
    PFNGLPATHPARAMETERIVNVPROC glPathParameterivNV;
    PFNGLPATHPARAMETERINVPROC glPathParameteriNV;
    PFNGLPATHPARAMETERFVNVPROC glPathParameterfvNV;
    PFNGLPATHPARAMETERFNVPROC glPathParameterfNV;
    PFNGLPATHDASHARRAYNVPROC glPathDashArrayNV;
    PFNGLPATHSTENCILFUNCNVPROC glPathStencilFuncNV;
    PFNGLPATHSTENCILDEPTHOFFSETNVPROC glPathStencilDepthOffsetNV;
    PFNGLSTENCILFILLPATHNVPROC glStencilFillPathNV;
    PFNGLSTENCILSTROKEPATHNVPROC glStencilStrokePathNV;
    PFNGLSTENCILFILLPATHINSTANCEDNVPROC glStencilFillPathInstancedNV;
    PFNGLSTENCILSTROKEPATHINSTANCEDNVPROC glStencilStrokePathInstancedNV;
    PFNGLPATHCOVERDEPTHFUNCNVPROC glPathCoverDepthFuncNV;
    PFNGLCOVERFILLPATHNVPROC glCoverFillPathNV;
    PFNGLCOVERSTROKEPATHNVPROC glCoverStrokePathNV;
    PFNGLCOVERFILLPATHINSTANCEDNVPROC glCoverFillPathInstancedNV;
    PFNGLCOVERSTROKEPATHINSTANCEDNVPROC glCoverStrokePathInstancedNV;
    PFNGLGETPATHPARAMETERIVNVPROC glGetPathParameterivNV;
    PFNGLGETPATHPARAMETERFVNVPROC glGetPathParameterfvNV;
    PFNGLGETPATHCOMMANDSNVPROC glGetPathCommandsNV;
    PFNGLGETPATHCOORDSNVPROC glGetPathCoordsNV;
    PFNGLGETPATHDASHARRAYNVPROC glGetPathDashArrayNV;
    PFNGLGETPATHMETRICSNVPROC glGetPathMetricsNV;
    PFNGLGETPATHMETRICRANGENVPROC glGetPathMetricRangeNV;
    PFNGLGETPATHSPACINGNVPROC glGetPathSpacingNV;
    PFNGLISPOINTINFILLPATHNVPROC glIsPointInFillPathNV;
    PFNGLISPOINTINSTROKEPATHNVPROC glIsPointInStrokePathNV;
    PFNGLGETPATHLENGTHNVPROC glGetPathLengthNV;
    PFNGLPOINTALONGPATHNVPROC glPointAlongPathNV;
    PFNGLMATRIXLOAD3X2FNVPROC glMatrixLoad3x2fNV;
    PFNGLMATRIXLOAD3X3FNVPROC glMatrixLoad3x3fNV;
    PFNGLMATRIXLOADTRANSPOSE3X3FNVPROC glMatrixLoadTranspose3x3fNV;
    PFNGLMATRIXMULT3X2FNVPROC glMatrixMult3x2fNV;
    PFNGLMATRIXMULT3X3FNVPROC glMatrixMult3x3fNV;
    PFNGLMATRIXMULTTRANSPOSE3X3FNVPROC glMatrixMultTranspose3x3fNV;
    PFNGLSTENCILTHENCOVERFILLPATHNVPROC glStencilThenCoverFillPathNV;
    PFNGLSTENCILTHENCOVERSTROKEPATHNVPROC glStencilThenCoverStrokePathNV;
    PFNGLSTENCILTHENCOVERFILLPATHINSTANCEDNVPROC glStencilThenCoverFillPathInstancedNV;
    PFNGLSTENCILTHENCOVERSTROKEPATHINSTANCEDNVPROC glStencilThenCoverStrokePathInstancedNV;
    PFNGLPATHGLYPHINDEXRANGENVPROC glPathGlyphIndexRangeNV;
    PFNGLPATHGLYPHINDEXARRAYNVPROC glPathGlyphIndexArrayNV;
    PFNGLPATHMEMORYGLYPHINDEXARRAYNVPROC glPathMemoryGlyphIndexArrayNV;
    PFNGLPROGRAMPATHFRAGMENTINPUTGENNVPROC glProgramPathFragmentInputGenNV;
    PFNGLGETPROGRAMRESOURCEFVNVPROC glGetProgramResourcefvNV;
#endif

    void extensionsLoad(const char* const openGLModuleName)
    {
        if (!openGLModuleName)
            return;

        auto openGLModuleNameWChar = hr::StringUtils::conv2Native(openGLModuleName);

        auto ptrWGlGetProcAddress = (PFNWGLGETPROCADDRESSPROC)GetProcAddress(GetModuleHandle(openGLModuleNameWChar.c_str()), "wglGetProcAddress");
        if (!ptrWGlGetProcAddress)
            return;

        loadGLExtFunctions(ptrWGlGetProcAddress);
    }

    bool extensionExists(const char* const extensionName)
    {
        if (!extensionName || *extensionName == '\0')
            return false;

        GLint numExtensions;
        hr::gl::glGetIntegerv(GL_NUM_EXTENSIONS, &numExtensions);
        if (numExtensions <= 0)
            return false;

        for (GLint curIndex = 0; curIndex < numExtensions; curIndex++)
        {
            auto curExt = reinterpret_cast<const char*>(hr::gl::glGetStringi(GL_EXTENSIONS, curIndex));
            if (!curExt || (*curExt == '\0'))
                continue;

            if (stricmp(curExt, extensionName) == 0)
                return true;
        }

        return false;
    }
}
