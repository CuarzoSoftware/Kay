#ifndef AKAPPLICATION_H
#define AKAPPLICATION_H

#include <AK/AKTimer.h>
#include <AK/input/AKPointer.h>
#include <AK/input/AKKeyboard.h>
#include <AK/AKEventSource.h>
#include <AK/AKSafeEventQueue.h>

#include <modules/skparagraph/include/FontCollection.h>
#include <include/gpu/ganesh/GrContextOptions.h>

#include <EGL/egl.h>
#include <sys/epoll.h>
#include <unordered_map>

/**
 * @brief Core application class
 *
 * AKApplication manages the application's event loop and allows integration with other event loops.
 * It supports dynamic event sources and provides access to pointer, keyboard, and touch managers.
 *
 * @note A single instance can exist per process.
 *
 * @section OpenGL Contexts
 *
 * AKApplication does not create OpenGL contexts but requires a valid, bound OpenGL context before instantiation.
 * Additional threads using AKApplication must share the OpenGL context.
 *
 * It automatically creates Skia contexts and OpenGL objects using the existing contexts.
 * When used in a separate thread, call `freeGLContextData()` before thread destruction to prevent resource leaks.
 * This step is unnecessary for the thread where the application was created.
 */
class AK::AKApplication : public AKObject
{
public:
    AKApplication() noexcept;
    AKCLASS_NO_COPY(AKApplication)
    sk_sp<SkFontMgr> fontManager() const noexcept;
    sk_sp<skia::textlayout::FontCollection> fontCollection() const noexcept;
    static AKGLContext *glContext() noexcept;
    static void freeGLContextData() noexcept;
    const GrContextOptions &skContextOptions() const noexcept { return m_skContextOptions; };

    /**
     * @brief Immediately sends an event to the specified object.
     *
     * This method sends the provided event to the given object, processing it right away.
     *
     * @param event The event to be sent to the object.
     * @param object The object that will receive and handle the event.
     * @return `true` if the event was handled/accepted, `false` otherwise.
     */
    bool sendEvent(const AKEvent &event, AKObject &object);

    /**
     * @brief Adds an event to the queue for later dispatch by the event loop.
     *
     * This method queues the provided event to be dispatched to the given object later by the event loop.
     *
     * @param event The event to be queued for dispatch.
     * @param object The object that will receive the event when dispatched.
     */
    void postEvent(const AKEvent &event, AKObject &object) noexcept;

    AKPointer &pointer() noexcept;
    AKKeyboard &keyboard() noexcept;

    AKEventSource *addEventSource(Int32 fd, UInt32 events, const AKEventSource::Callback &callback) noexcept;
    void removeEventSource(AKEventSource *source) noexcept;

    void unlockLoop() const noexcept;
    void processLoop(int timeout = -1);
    int exec();
    int fd() const noexcept { return m_epollFd; };
protected:
    friend class AKScene;
    friend class AKAnimation;
    void setPointer(AKPointer *pointer) noexcept;
    void setKeyboard(AKKeyboard *keyboard) noexcept;
    void processAnimations();

    std::vector<AKAnimation*> m_animations;
    std::unique_ptr<AKTimer> m_animationsTimer;
    bool m_animationsChanged { false };
    std::unique_ptr<AKPointer> m_pointer;
    std::unique_ptr<AKKeyboard> m_keyboard;
    GrContextOptions m_skContextOptions;
    sk_sp<SkFontMgr> m_fontManager;
    sk_sp<skia::textlayout::FontCollection> m_fontCollection;
    std::unordered_map<EGLContext, AKGLContext*> m_glContexts;

    int m_epollFd;
    std::vector<epoll_event> m_epollEvents;
    std::vector<std::shared_ptr<AKEventSource>> m_currentEventSources;
    std::vector<std::shared_ptr<AKEventSource>> m_pendingEventSources;
    std::shared_ptr<AKBooleanEventSource> m_mainEventSource;
    bool m_eventSourcesChanged { false };

    AKSafeEventQueue m_eventQueue;
};

#endif // AKAPPLICATION_H
