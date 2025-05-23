#ifndef AKSAFEEVENTQUEUE_H
#define AKSAFEEVENTQUEUE_H

#include <AK/AKObject.h>
#include <AK/AKWeak.h>
#include <queue>

/**
 * @brief Event queue with weak object references.
 *
 * This queue stores weak references to target objects. Before dispatching events,
 * it checks if the objects are still alive and discards them otherwise.
 *
 * Use this in situations where objects may be destroyed while handling events in the queue.
 *
 * Internally, it triggers AKApplication::postEvent().
 */
class AK::AKSafeEventQueue
{
public:

    /**
     * @brief Default constructor.
     */
    AKSafeEventQueue() noexcept = default;

    /**
     * @brief Move constructor.
     *
     * After the move, the other queue is left empty and can be safely reused.
     */
    AKSafeEventQueue(AKSafeEventQueue &&other) noexcept
    {
        *this = std::move(other);
    }

    /**
     * @brief Move assignment operator.
     *
     * Transfers the contents from another AKSafeEventQueue to this one.
     *
     * After the move, the other queue is left empty and can be safely reused.
     *
     * @param other The other AKSafeEventQueue to move from.
     * @return A reference to this AKSafeEventQueue.
     */
    AKSafeEventQueue &operator=(AKSafeEventQueue &&other) noexcept
    {
        if (this != &other)
        {
            m_queue = std::move(other.m_queue);
            other.m_queue = std::queue<SafeEvent>();
        }
        return *this;
    }

    AKSafeEventQueue(const AKSafeEventQueue&) = delete;
    AKSafeEventQueue& operator=(const AKSafeEventQueue&) = delete;

    /**
     * @brief Adds an event to the queue.
     *
     * @param event The event to be added.
     * @param object The target object associated with the event.
     */
    void addEvent(const AKEvent &event, AKObject &object) noexcept;

    /**
     * @brief Dispatches events in the queue.
     *
     * This method processes each event in the queue, discarding events
     * associated with destroyed objects and triggering AKApplication::sendEvent()
     * for remaining events.
     */
    void dispatch();

private:

    struct SafeEvent
    {
        AKWeak<AKObject> object;
        std::unique_ptr<AKEvent> event;
    };

    std::queue<SafeEvent> m_queue;
};

#endif // AKSAFEEVENTQUEUE_H
