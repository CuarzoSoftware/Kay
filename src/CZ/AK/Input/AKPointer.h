#ifndef CZ_AKPOINTER_H
#define CZ_AKPOINTER_H

#include <CZ/AK/AKObject.h>
#include <CZ/Core/CZWeak.h>
#include <CZ/Core/Events/CZPointerEnterEvent.h>
#include <CZ/Core/Events/CZPointerLeaveEvent.h>
#include <CZ/Core/Events/CZPointerButtonEvent.h>
#include <CZ/Core/Events/CZPointerMoveEvent.h>
#include <CZ/Core/Events/CZPointerScrollEvent.h>

class CZ::AKPointer : public AKObject
{
public:

    /**
     *  @brief Most recent pointer events handled by Kay.
     */
    struct EventHistory
    {
        /// The most recent pointer enter event dispatched to an AKScene.
        CZPointerEnterEvent enter;

        /// The most recent pointer leave event dispatched to AKApp.
        CZPointerLeaveEvent leave;

        /// The most recent pointer button event dispatched to AKApp.
        CZPointerButtonEvent button;

        /// The most recent pointer move event dispatched to AKApp.
        CZPointerMoveEvent move;

        /// The most recent pointer scroll event dispatched to AKApp.
        CZPointerScrollEvent scroll;
    };

    /**
     *  @brief Most recent pointer event history.
     */
    const EventHistory &history() const noexcept{ return m_history; }

    /**
     * @brief Returns the current cursor position in world coordinates.
     *
     * This value reflects the position from the most recent pointer enter or move event.
     */
    SkPoint pos() const noexcept { return history().move.pos; }


    // TODO: Add pressed buttons

    /**
     * @brief Returns the scene currently holding pointer focus.
     *
     * @note In the context of Marco, a scene can be thought of as a window.
     *
     * This represents the last scene that received a CZPointerEnterEvent.
     * Scenes that are not focused will ignore pointer events.
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
