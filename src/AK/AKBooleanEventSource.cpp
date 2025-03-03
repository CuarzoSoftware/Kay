#include <AK/AKBooleanEventSource.h>
#include <AK/AKApplication.h>
#include <AK/AKLog.h>
#include <sys/eventfd.h>

using namespace AK;

std::shared_ptr<AKBooleanEventSource> AKBooleanEventSource::Make(bool enabled, const Callback &callback) noexcept
{
    assert(akApp());
    AKBooleanEventSource *instance { new AKBooleanEventSource() };
    instance->m_callback = callback;
    instance->m_state = enabled;
    const int fd { eventfd(enabled ? 1 : 0, EFD_CLOEXEC) };

    if (fd == -1)
    {
        AKLog::error("[AKBooleanEventSource::Make] Failed to create fd (eventfd).");
        delete instance;
        return nullptr;
    }

    instance->m_source = akApp()->addEventSource(fd, EPOLLIN, [instance](int, UInt32){
        instance->setState(false);
        if (instance->m_callback)
            instance->m_callback(instance);
    });

    if (!instance->m_source)
    {
        AKLog::error("[AKBooleanEventSource::Make] Failed to add event source.");
        close(fd);
        delete instance;
        return nullptr;
    }

    return std::shared_ptr<AKBooleanEventSource>(instance);
}

void AKBooleanEventSource::setState(bool enabled) noexcept
{
    if (m_state == enabled)
        return;

    m_state = enabled;

    if (m_state)
        eventfd_write(m_source->fd(), 1);
    else
    {
        eventfd_t value;
        eventfd_read(m_source->fd(), &value);
    }
}

AKBooleanEventSource::~AKBooleanEventSource() noexcept
{
    if (m_source)
    {
        const int fd { m_source->fd() };
        akApp()->removeEventSource(m_source);
        close(fd);
    }
}
