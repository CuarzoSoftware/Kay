#include <AK/events/AKEvent.h>
#include <AK/AKAnimation.h>
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
    auto source = std::shared_ptr<AKEventSource>(new AKEventSource(fd, events, callback), AKEventSource::Deleter());

    if (epoll_ctl(m_epollFd, EPOLL_CTL_ADD, source->fd(), &source->m_event) == -1)
    {
        AKLog::error("[AKApplication::addEventSource] Failed to add event source (epoll_ctl).");
        return nullptr;
    }

    m_eventSourcesChanged = true;
    m_pendingEventSources.emplace_back(source);
    return m_pendingEventSources.back().get();
}

void AKApplication::removeEventSource(AKEventSource *source) noexcept
{
    for (size_t i = 0; i < m_pendingEventSources.size(); i++)
    {
        if (m_pendingEventSources[i].get() == source)
        {
            // If unset it means EPOLL_CTL_ADD failed
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

        if (m_currentEventSources.size() > m_epollEvents.size())
        {
            m_epollEvents.reserve(m_pendingEventSources.size());

            while (m_currentEventSources.size() > m_epollEvents.size())
                m_epollEvents.emplace_back();
        }
    }

    int events = epoll_wait(m_epollFd, m_epollEvents.data(), m_epollEvents.size(), timeout);

    if (events == -1)
        return;

    AKEventSource *source;

    for (int i = 0; i < events; i++)
    {
        source = static_cast<AKEventSource*>(m_epollEvents[i].data.ptr);

        if (!source->m_callback)
            continue;

        source->m_callback(source->fd(), m_epollEvents[i].events);
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

void AKApplication::processAnimations()
{
    Int64 elapsed;
    Int64 duration;

    for (AKAnimation *a : m_animations)
        a->m_processed = false;

retry:
    m_animationsChanged = false;

    for (AKAnimation *a : m_animations)
    {
        if (a->m_processed)
            continue;

        if (a->m_pendingDestroy)
        {
            delete a;
            goto retry;
        }

        a->m_processed = true;

        if (!a->m_running)
            continue;

        elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(
                      std::chrono::steady_clock::now() - a->m_beginTime).count();

        duration = static_cast<Int64>(a->m_duration);

        if (elapsed >= duration)
            a->m_value = 1.0;
        else
            a->m_value = static_cast<Float64>(elapsed)/static_cast<Float64>(duration);

        if (a->m_onUpdate)
        {
            a->m_onUpdate(a);

            if (m_animationsChanged)
                goto retry;
        }

        if (a->m_value == 1.0)
        {
            a->stop();

            if (m_animationsChanged)
                goto retry;
        }
    }
}
