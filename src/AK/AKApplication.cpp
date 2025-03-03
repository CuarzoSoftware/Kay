#include <AK/events/AKEvent.h>
#include <AK/AKBooleanEventSource.h>
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

AKApplication *AK::akApp() noexcept
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
    m_fontCollection = sk_make_sp<skia::textlayout::FontCollection>();
    m_fontCollection->setDefaultFontManager(akFontManager());
    m_fontCollection->enableFontFallback();
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

    m_epollFd = epoll_create1(EPOLL_CLOEXEC);

    if (m_epollFd == -1)
    {
        AKLog::fatal("[AKApplication] Failed to create epoll fd.");
        exit(EXIT_FAILURE);
    }

    m_mainEventSource = AKBooleanEventSource::Make(false, [this](auto)
    {
        AKSafeEventQueue tmp { std::move(m_eventQueue) };
        tmp.dispatch();
    });
}

sk_sp<SkFontMgr> AKApplication::fontManager() const noexcept
{
    return m_fontManager;
}

sk_sp<skia::textlayout::FontCollection> AKApplication::fontCollection() const noexcept
{
    return m_fontCollection;
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

bool AKApplication::sendEvent(const AKEvent &event, AKObject &object)
{
    event.accept();

    for (AKObject *filter : object.m_installedEventFilters)
        if (filter->eventFilter(event, object))
            return true;

    return object.event(event);
}

void AKApplication::postEvent(const AKEvent &event, AKObject &object) noexcept
{
    unlockLoop();
    m_eventQueue.addEvent(event, object);
}

AKPointer &AKApplication::pointer() noexcept
{
    if (!m_pointer)
        m_pointer.reset(new AKPointer());

    return *m_pointer.get();
}

AKKeyboard &AKApplication::keyboard() noexcept
{
    if (!m_keyboard)
        m_keyboard.reset(new AKKeyboard());

    return *m_keyboard.get();
}

AKEventSource *AKApplication::addEventSource(Int32 fd, UInt32 events, const AKEventSource::Callback &callback) noexcept
{
    epoll_event event;
    event.events = events;
    event.data.fd = fd;

    if (epoll_ctl(m_epollFd, EPOLL_CTL_ADD, fd, &event) == -1)
    {
        AKLog::error("[AKApplication::addEventSource] Failed to add event source (epoll_ctl).");
        return nullptr;
    }
    else
        AKLog::debug("[AKApplication::addEventSource] Added fd %d.", fd);

    m_eventSourcesChanged = true;
    m_pendingEventSources.emplace_back(std::shared_ptr<AKEventSource>(new AKEventSource(fd, events, callback), AKEventSource::Deleter()));
    return m_pendingEventSources.back().get();
}

void AKApplication::removeEventSource(AKEventSource *source) noexcept
{
    for (size_t i = 0; i < m_pendingEventSources.size(); i++)
    {
        if (m_pendingEventSources[i].get() == source)
        {
            AKLog::debug("[AKApplication::removeEventSource] Removed fd %d.", m_pendingEventSources[i].get()->fd());
            epoll_ctl(m_epollFd, EPOLL_CTL_DEL, m_pendingEventSources[i].get()->fd(), NULL);
            m_pendingEventSources.erase(m_pendingEventSources.begin() + i);
            m_eventSourcesChanged = true;
            return;
        }
    }
}

void AKApplication::unlockLoop() const noexcept
{
    m_mainEventSource->setState(true);
}

void AKApplication::processLoop(int timeout)
{
    if (m_eventSourcesChanged)
    {
        m_currentEventSources = m_pendingEventSources;
        m_eventSourcesChanged = false;
        m_epollEvents.clear();
        m_epollEvents.reserve(m_pendingEventSources.size());

        for (const auto &eventSource : m_currentEventSources)
            m_epollEvents.emplace_back(eventSource->m_event);
    }

    epoll_wait(m_epollFd, m_epollEvents.data(), m_epollEvents.size(), timeout);

    for (size_t i = 0; i < m_epollEvents.size(); i++)
    {
        if (m_epollEvents[i].events == 0 || !m_currentEventSources[i]->m_callback)
            continue;

        m_currentEventSources[i]->m_callback(m_epollEvents[i].data.fd, m_epollEvents[i].events);
    }
}

int AKApplication::exec()
{
    while (1)
        processLoop(-1);

    return 0;
}

void AKApplication::setPointer(AKPointer *pointer) noexcept
{
    if (m_pointer.get() == pointer)
        return;

    m_pointer.reset(pointer);
}

void AKApplication::setKeyboard(AKKeyboard *keyboard) noexcept
{
    if (m_keyboard.get() == keyboard)
        return;

    m_keyboard.reset(keyboard);
}
