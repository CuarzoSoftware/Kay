#ifndef AKBOOLEANEVENTSOURCE_H
#define AKBOOLEANEVENTSOURCE_H

#include <AK/AKEventSource.h>
#include <memory>

/**
 * @brief A toggleable event source.
 *
 * When enabled, this event source wakes up the main loop. Once processed,
 * it disables itself and the provided callback is triggered. The event source can be re-enabled
 * during the callback or at any later time using `setState()`.
 *
 * Internally, it uses an `eventfd`.
 */
class AK::AKBooleanEventSource
{
public:

    AKCLASS_NO_COPY(AKBooleanEventSource);

    /**
     * @brief Callback function type. Called when the event is triggered.
     */
    using Callback = std::function<void(AKBooleanEventSource*)>;

    /**
     * @brief Creates an AKBooleanEventSource.
     *
     * @param callback The callback function to be triggered.
     * @param enabled Initial state of the event source.
     * @return A shared pointer to the created AKBooleanEventSource, or `nullptr` on failure.
     */
    static std::shared_ptr<AKBooleanEventSource> Make(bool enabled, const Callback &callback) noexcept;

    /**
     * @brief Gets the current state of the event source.
     */
    bool state() const noexcept { return m_state; };

    /**
     * @brief Sets the state of the event source.
     */
    void setState(bool enabled) noexcept;

    /**
     * @brief Destructor for AKBooleanEventSource.
     */
    ~AKBooleanEventSource() noexcept;

private:
    AKBooleanEventSource() noexcept = default;
    AKEventSource *m_source { nullptr };
    Callback m_callback { nullptr };
    bool m_state;
};

#endif // AKBOOLEANEVENTSOURCE_H
