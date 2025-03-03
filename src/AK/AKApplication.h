#ifndef AKAPPLICATION_H
#define AKAPPLICATION_H

#include <AK/input/AKPointer.h>
#include <AK/input/AKKeyboard.h>
#include <AK/AKEventSource.h>
#include <AK/AKSafeEventQueue.h>

#include <modules/skparagraph/include/FontCollection.h>
#include <include/gpu/ganesh/GrContextOptions.h>

#include <EGL/egl.h>
#include <sys/epoll.h>
#include <unordered_map>

class AK::AKApplication : public AKObject
{
public:
    AKApplication() noexcept;
    AKCLASS_NO_COPY(AKApplication)
    sk_sp<SkFontMgr> fontManager() const noexcept;
    sk_sp<skia::textlayout::FontCollection> fontCollection() const noexcept;
    static AKGLContext *glContext() noexcept;
    static void freeGLContext() noexcept;
    const GrContextOptions &skContextOptions() const noexcept { return m_skContextOptions; };
    std::vector<AKNode*> animated;

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
    void setPointer(AKPointer *pointer) noexcept;
    void setKeyboard(AKKeyboard *keyboard) noexcept;
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
