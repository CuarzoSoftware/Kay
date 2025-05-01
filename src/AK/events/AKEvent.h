#ifndef AKEVENT_H
#define AKEVENT_H

#include <AK/AK.h>

#define AKEVENT_DECLARE_COPY AKEvent *copy() const noexcept override { return new AK_GET_CLASS(this)(*this); }

/**
 * @defgroup AKEvents Events
 * @brief List of all event types.
 * @{
 */

/**
 * @brief Base class for events.
 */
class AK::AKEvent
{
public:

    /**
     * @brief Defines the type of event.
     */
    enum Type
    {
        Destroy,

        PointerMove,
        PointerScroll,
        PointerButton,
        PointerEnter,
        PointerLeave,
        PointerSwipeBegin,
        PointerSwipeUpdate,
        PointerSwipeEnd,
        PointerPinchBegin,
        PointerPinchUpdate,
        PointerPinchEnd,
        PointerHoldBegin,
        PointerHoldEnd,

        KeyboardKey,
        KeyboardModifiers,
        KeyboardEnter,
        KeyboardLeave,

        TouchMove,
        TouchFrame,
        TouchDown,
        TouchUp,
        TouchCancel,

        WindowState,
        WindowClose,

        Render,
        Bake,

        SceneChanged,
        Layout,

        Vibrancy,

        User = 1000
    };

    /**
     * @brief Destructor.
     */
    virtual ~AKEvent() noexcept = default;

    /**
     * @brief Retrieves the type of the event.
     */
    Type type() const noexcept
    {
        return m_type;
    }

    /**
     * @brief Checks if the event type is any of the given types.
     *
     * @return `true` if any of the types match, `false` otherwise.
     */
    template<typename... Types>
    constexpr bool typeIsAnyOf(Types...types) const noexcept
    {
        for (const auto t : {types...})
            if (t == type())
                return true;
        return false;
    }

    /**
     * @brief Sets the serial of the event.
     */
    void setSerial(UInt32 serial) noexcept
    {
        m_serial = serial;
    }

    /**
     * @brief Retrieves the serial of the event.
     */
    UInt32 serial() const noexcept
    {
        return m_serial;
    }

    /**
     * @brief Sets the time the event was generated in milliseconds.
     */
    void setMs(UInt32 ms) noexcept
    {
        m_ms = ms;
    }

    /**
     * @brief Retrieves the time the event was generated in milliseconds.
     */
    UInt32 ms() const noexcept
    {
        return m_ms;
    }

    /**
     * @brief Sets the time the event was generated in microseconds.
     */
    void setUs(UInt32 us) noexcept
    {
        m_us = us;
    }

    /**
     * @brief Retrieves the time the event was generated in microseconds.
     */
    UInt64 us() const noexcept
    {
        return m_us;
    }

    /**
     * @brief Creates a deep copy of the event.
     *
     * @return A pointer to the copied event.
     *
     * @note The returned event must be deleted when no longer used.
     */
    virtual AKEvent *copy() const noexcept
    {
        return new AKEvent(*this);
    }

    void setUserData(void *data) noexcept
    {
        m_userData = data;
    }

    void *userData() const noexcept
    {
        return m_userData;
    }

    bool isAccepted() const noexcept
    {
        return m_isAccepted;
    }

    void accept() const noexcept
    {
        m_isAccepted = true;
    }

    void ignore() const noexcept
    {
        m_isAccepted = false;
    }

protected:
    AKEvent(Type type, UInt32 serial, UInt32 ms, UInt64 us) noexcept :
        m_type(type),
        m_serial(serial),
        m_ms(ms),
        m_us(us)
    {}
    Type m_type;
    UInt32 m_serial;
    UInt32 m_ms;
    UInt64 m_us;
    void *m_userData { nullptr };
    mutable bool m_isAccepted { true };
};

/**
 * @}
 */

#endif // AKEVENT_H
