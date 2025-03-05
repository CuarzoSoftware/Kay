#ifndef AKEVENTSOURCE_H
#define AKEVENTSOURCE_H

#include <AK/AKObject.h>
#include <functional>
#include <sys/epoll.h>

class AK::AKEventSource : public AKObject
{
public:
    using Callback = std::function<void(int, UInt32)>;
    int fd() const noexcept { return m_fd; };
    UInt32 events() const noexcept { return m_event.events; };

    struct Deleter { void operator()(AKEventSource *p) { delete p; } };
private:
    friend class AKApplication;
    AKEventSource(int fd, UInt32 events, const Callback &callback) noexcept : m_callback(callback), m_fd(fd)
    {
        m_event.events = events;
        m_event.data.ptr = this;
    }
    AKCLASS_NO_COPY(AKEventSource);
    ~AKEventSource() = default;
    Callback m_callback;
    epoll_event m_event;
    int m_fd;
};

#endif // AKEVENTSOURCE_H
