#ifndef CZ_AKPOINTER_H
#define CZ_AKPOINTER_H

#include <CZ/AK/AKObject.h>
#include <CZ/Core/CZWeak.h>
#include <CZ/Events/CZPointerEnterEvent.h>
#include <CZ/Events/CZPointerLeaveEvent.h>
#include <CZ/Events/CZPointerButtonEvent.h>
#include <CZ/Events/CZPointerMoveEvent.h>
#include <CZ/Events/CZPointerScrollEvent.h>

class CZ::AKPointer : public AKObject
{
public:
    struct EventHistory
    {
        CZPointerEnterEvent enter;
        CZPointerLeaveEvent leave;
        CZPointerButtonEvent button;
        CZPointerMoveEvent move;
        CZPointerScrollEvent scroll;
    };

    SkPoint pos() const noexcept { return history().move.pos; }
    const EventHistory &history() const noexcept{ return m_history; }

    // TODO: Add pressed buttons

    /**
     * @brief Returns the scene currently holding pointer focus.
     *
     * @note In the context of Marco, a scene can be thought of as a window.
     *
     * This represents the last scene that received a CZPointerEnterEvent.
     * Scenes that are not focused will ignore other pointer events.
     * Each scene provides information about its focused nodes.
     *
     * @see onFocusChanged
     *
     * @return The focused AKScene, or nullptr if none.
     */
    AKScene *focus() const noexcept { return m_focus; }
    CZSignal<> onFocusChanged;

protected:
    AKPointer() = default;

private:
    friend class AKApp;
    friend class AKScene;
    CZWeak<AKScene> m_focus;
    EventHistory m_history {};
};

#endif // CZ_AKPOINTER_H
