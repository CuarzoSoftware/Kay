#include <AK/AKSafeEventQueue.h>
#include <AK/events/AKEvent.h>
#include <AK/AKApplication.h>

using namespace AK;

void AKSafeEventQueue::addEvent(const AKEvent &event, AKObject &object) noexcept
{
    m_queue.emplace(AKWeak<AKObject>(&object), std::unique_ptr<AKEvent>(event.copy()));
}

void AKSafeEventQueue::dispatch()
{
    while (!m_queue.empty())
    {
        if (m_queue.front().object)
            akApp()->sendEvent(*m_queue.front().event, *m_queue.front().object.get());
        m_queue.pop();
    }
}
