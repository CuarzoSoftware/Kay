#ifndef AKSTATEDEACTIVATEDEVENT_H
#define AKSTATEDEACTIVATEDEVENT_H

#include <AK/events/AKStateEvent.h>
#include <AK/AKTime.h>

/**
 * @brief Deactivated event.
 * @ingroup AKEvents
 *
 * This event is triggered when the scene to which the node belongs becomes deactivated.
 * In desktop applications, each window should have its own dedicated scene, so this
 * event indicates that a window has become deactivated.
 *
 * Nodes should update their styles to be less noticeable. All nodes are activated
 * upon creation.
 *
 * @note This event is not triggered when a node is moved from an activated
 *       scene to a deactivated one.
 *
 * @see AK::AKStateActivatedEvent
 */
class AK::AKStateDeactivatedEvent : public AKStateEvent
{
public:
    AKStateDeactivatedEvent(UInt32 serial = AKTime::nextSerial(),
                          UInt32 ms = AKTime::ms(), UInt64 us = AKTime::us()) noexcept :
        AKStateEvent(Subtype::Deactivated, serial, ms, us) {}
};

#endif // AKSTATEDEACTIVATEDEVENT_H
