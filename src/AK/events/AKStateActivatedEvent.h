#ifndef AKSTATEACTIVATEDEVENT_H
#define AKSTATEACTIVATEDEVENT_H

#include <AK/events/AKStateEvent.h>
#include <AK/AKTime.h>

/**
 * @brief Activated event.
 *
 * This event is triggered when the scene to which the node belongs becomes active.
 * In desktop applications, each window should have its own dedicated scene, so this
 * event indicates that a window has become active.
 *
 * Nodes should update their styles to be more noticeable. All nodes are activated
 * upon creation.
 *
 * @note This event is not triggered when a node is moved from a deactivated
 *       scene to an activated one.
 *
 * @see AK::AKStateDeactivatedEvent
 */
class AK::AKStateActivatedEvent : public AKStateEvent
{
public:

    /**
     * @brief Event constructor.
     */
    AKStateActivatedEvent(UInt32 serial = AKTime::nextSerial(),
                          UInt32 ms = AKTime::ms(), UInt64 us = AKTime::us()) noexcept :
        AKStateEvent(Subtype::Activated, serial, ms, us) {}
};

#endif // AKSTATEACTIVATEDEVENT_H
