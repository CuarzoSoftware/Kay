#ifndef AKTIMER_H
#define AKTIMER_H

#include <AK/AKObject.h>
#include <AK/AKEventSource.h>
#include <sys/timerfd.h>

/**
 * @brief Timer Event Source.
 *
 * This class represents a timer object that can execute a callback function
 * after a specified timeout in milliseconds.
 */
class AK::AKTimer : public AKObject
{
public:
    /**
     * @brief Callback function type. Called on timeout.
     */
    using Callback = std::function<void(AKTimer*)>;

    /**
     * @brief Constructs a reusable timer.
     *
     * The timer is not started immediately and is not destroyed after finishing.
     * After the callback is triggered, the timer is disabled until start() is called
     * again within or outside the callback.
     *
     * @param callback The callback function to be called when the timer event is triggered.
     */
    AKTimer(const Callback &callback = nullptr) noexcept;

    /**
     * @brief Creates a one-shot timer.
     *
     * This static method creates a one-shot timer that is automatically destroyed after the timeout,
     * except if start() is called from the callback.
     *
     * @param timeoutMs The timeout in milliseconds.
     * @param callback The callback function to be called when the timer event is triggered.
     */
    static void OneShot(UInt64 timeoutMs, const Callback &callback);

    AKCLASS_NO_COPY(AKTimer)

    /**
     * @brief Destroys the AKTimer object without triggering the callback if running.
     */
    ~AKTimer() noexcept;

    /**
     * @brief Starts the timer.
     *
     * Starts the timer with the specified timeout in milliseconds. If the timer
     * is already running, it will be restarted with the new timeout and the callback
     * will not be triggered.
     *
     * @note If no callback was provided, the timer will not start.
     *
     * @param timeoutMs The timeout in milliseconds. Even if passing 0, the callback will be triggered later by the event loop.
     */
    void start(UInt64 timeoutMs);

    /**
     * @brief Stops the timer.
     *
     * Stops the timer if it is currently running.
     *
     * @param notifyIfRunning If `true`, the callback will be called if the timer is running.
     */
    void stop(bool notifyIfRunning = false);

    /**
     * @brief Sets the callback function.
     *
     * Sets the callback function to be called when the timer event is triggered.
     *
     * @param callback The callback function.
     */
    void setCallback(const Callback &callback) noexcept
    {
        m_callback = callback;
    }

    /**
     * @brief Gets the callback function.
     *
     * Returns the callback function that is set for the timer.
     *
     * @return The callback function.
     */
    const Callback &callback() const noexcept
    {
        return m_callback;
    }

    /**
     * @brief Gets the timeout.
     *
     * Returns the timeout value in milliseconds.
     *
     * @return The timeout in milliseconds.
     */
    UInt64 timeoutMs() const noexcept
    {
        return m_timeoutMs;
    }

    /**
     * @brief Checks if the timer is running.
     *
     * @return `true` if the timer is running, `false` otherwise.
     */
    bool running() const noexcept
    {
        return m_running;
    }

private:
    AKTimer(bool oneShoot, const Callback &callback, UInt64 timeoutMs) noexcept;
    void init() noexcept;

    Callback m_callback;
    UInt64 m_timeoutMs { 0 };
    AKEventSource *m_source { nullptr };
    bool m_running { false };
    bool m_oneShoot;
};


#endif // AKTIMER_H
