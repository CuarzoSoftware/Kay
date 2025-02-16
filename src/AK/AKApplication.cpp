#include <AK/AKApplication.h>
#include <AK/AKGLContext.h>
#include <AK/AKLog.h>
#include <EGL/egl.h>

#include <include/gpu/ganesh/gl/GrGLAssembleInterface.h>
#include <include/gpu/ganesh/gl/GrGLDirectContext.h>
#include <include/ports/SkFontMgr_fontconfig.h>

using namespace AK;

static AKApplication *_app { nullptr };

static auto interface = GrGLMakeAssembledInterface(nullptr, (GrGLGetProc)*[](void *, const char *p) -> void * {
    return (void *)eglGetProcAddress(p);
});

AKApplication *AK::AKApp() noexcept
{
    return _app;
}

AKApplication::AKApplication() noexcept
{
    assert("Only a single AKApplication instance can exist per process" && _app == nullptr);
    _app = this;
    AKLog::init();
    m_fontManager = SkFontMgr_New_FontConfig(nullptr);
    assert("Failed to create the font manager" && m_fontManager);
    m_skContextOptions.fShaderCacheStrategy = GrContextOptions::ShaderCacheStrategy::kBackendBinary;
    m_skContextOptions.fAvoidStencilBuffers = true;
    m_skContextOptions.fPreferExternalImagesOverES3 = true;
    m_skContextOptions.fDisableGpuYUVConversion = true;
    m_skContextOptions.fReducedShaderVariations = true;
    m_skContextOptions.fSuppressPrints = true;
    m_skContextOptions.fSuppressMipmapSupport = true;
    m_skContextOptions.fSkipGLErrorChecks = GrContextOptions::Enable::kYes;
    m_skContextOptions.fBufferMapThreshold = -1;
    m_skContextOptions.fDisableDistanceFieldPaths = true;
    m_skContextOptions.fAllowPathMaskCaching = true;
    m_skContextOptions.fGlyphCacheTextureMaximumBytes = 2048 * 1024 * 4;
    m_skContextOptions.fUseDrawInsteadOfClear = GrContextOptions::Enable::kYes;
    m_skContextOptions.fReduceOpsTaskSplitting = GrContextOptions::Enable::kYes;
    m_skContextOptions.fDisableDriverCorrectnessWorkarounds = true;
    m_skContextOptions.fRuntimeProgramCacheSize = 1024;
    m_skContextOptions.fInternalMultisampleCount = 0;
    m_skContextOptions.fDisableTessellationPathRenderer = false;
    m_skContextOptions.fAllowMSAAOnNewIntel = true;
    m_skContextOptions.fAlwaysUseTexStorageWhenAvailable = true;
}

sk_sp<SkFontMgr> AKApplication::fontManager() const noexcept
{
    return m_fontManager;
}

AKGLContext *AKApplication::glContext() noexcept
{
    assert("Please ensure a valid OpenGL context is bound before calling AKApplication::skContext()" && eglGetCurrentContext() != EGL_NO_CONTEXT);

    auto it { _app->m_glContexts.find(eglGetCurrentContext()) };
    if (it != _app->m_glContexts.end())
        return it->second;

    sk_sp<GrDirectContext> skContext = GrDirectContexts::MakeGL(interface, _app->skContextOptions());
    assert("Failed to create GrDirectContext. Please ensure a valid OpenGL context is bound before calling AKApplication::skContext()." && skContext);
    auto *akContext = new AKGLContext(skContext);
    _app->m_glContexts[eglGetCurrentContext()] = akContext;
    return akContext;
}

void AKApplication::freeGLContext() noexcept
{
    auto it { _app->m_glContexts.find(eglGetCurrentContext()) };
    if (it == _app->m_glContexts.end())
        return;
    delete it->second;
    _app->m_glContexts.erase(it);
}

