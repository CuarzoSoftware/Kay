#include <AK/AKObject.h>
#include <AK/AKWeak.h>
#include <AK/AKSignal.h>
#include <AK/AKApplication.h>
#include <AK/events/AKDestroyEvent.h>

using namespace AK;

void AKObject::installEventFilter(AKObject *monitor) const noexcept
{
    if (!monitor)
        return;

    const auto &it { monitor->m_eventFilterSubscriptions.find((AKObject*)this) };

    // Already installed, move to front
    if (it != monitor->m_eventFilterSubscriptions.end())
        m_installedEventFilters.erase(it->second);

    m_installedEventFilters.push_front(monitor);
    monitor->m_eventFilterSubscriptions[(AKObject*)this] = m_installedEventFilters.begin();
}

void AKObject::removeEventFilter(AKObject *monitor) const noexcept
{
    if (!monitor)
        return;

    const auto &it { monitor->m_eventFilterSubscriptions.find((AKObject*)this) };

    if (it != monitor->m_eventFilterSubscriptions.end())
    {
        m_installedEventFilters.erase(it->second);
        monitor->m_eventFilterSubscriptions.erase(it);
    }
}

void AKObject::destroyLater() noexcept
{
    akApp()->postEvent(AKDestroyEvent(), *this);
}

AKObject::~AKObject() noexcept
{
    notifyDestruction();

    while (!m_eventFilterSubscriptions.empty())
        m_eventFilterSubscriptions.begin()->first->removeEventFilter(this);

    while (!m_installedEventFilters.empty())
        removeEventFilter(m_installedEventFilters.back());
}

void AKObject::notifyDestruction() noexcept
{
    if (m_destroyed)
        return;

    m_destroyed = true;
    on.destroyed.notify(this);

    while (!m_listeners.empty())
        delete m_listeners.back();

    while (!m_weakRefs.empty())
    {
        AKWeak<AKObject> *weak { (AKWeak<AKObject>*)m_weakRefs.back() };
        weak->m_object = nullptr;
        m_weakRefs.pop_back();

        if (weak->m_onDestroyCallback)
            (*weak->m_onDestroyCallback)(this);
    }
}
