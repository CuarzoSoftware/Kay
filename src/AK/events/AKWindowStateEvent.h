#ifndef AKSTATEACTIVATEDEVENT_H
#define AKSTATEACTIVATEDEVENT_H

#include <AK/events/AKWindowEvent.h>
#include <AK/AKWindowState.h>
#include <AK/AKBitset.h>
#include <AK/AKTime.h>

/**
 * @brief Window state event.
 * @ingroup AKEvents
 *
 * This event is triggered when the scene to which the node belongs becomes activated/deactivated.
 * In desktop applications, each window should have its own dedicated scene, so this
 * event in such context indicates that a window has become activated/deactivated.
 *
 * Nodes should update their styles to be more/less noticeable. All nodes default to an activated
 * style upon creation.
 *
 * @note This event is not triggered when a node is moved from a deactivated
 *       scene to an activated one.
 */
class AK::AKWindowStateEvent : public AKWindowEvent
{
public:
    AKEVENT_DECLARE_COPY

    /**
     * @brief Event constructor.
     */
    AKWindowStateEvent(AKBitset<AKWindowState> states, AKBitset<AKWindowState> changes, UInt32 serial = AKTime::nextSerial(),
                          UInt32 ms = AKTime::ms(), UInt64 us = AKTime::us()) noexcept :
        AKWindowEvent(WindowState, serial, ms, us), m_states(states), m_changes(changes) {}

    AKBitset<AKWindowState> states() const noexcept
    {
        return m_states;
    }

    void setStates(AKBitset<AKWindowState> states) noexcept
    {
        m_states = states;
    }

    AKBitset<AKWindowState> changes() const noexcept
    {
        return m_changes;
    }

    void setChanges(AKBitset<AKWindowState> changes) noexcept
    {
        m_changes = changes;
    }

private:
    AKBitset<AKWindowState> m_states;
    AKBitset<AKWindowState> m_changes;
};

#endif // AKSTATEACTIVATEDEVENT_H
