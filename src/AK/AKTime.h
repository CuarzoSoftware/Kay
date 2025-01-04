#ifndef AKTIME_H
#define AKTIME_H

#include <AK/AK.h>
#include <ctime>
#include <limits>

/**
 * @brief Time utilities
 */
class AK::AKTime
{
public:

    AKTime() = delete;

    /**
     * @brief Serial.
     *
     * This method returns a new positive integer number each time it is called, incrementally.
     */
    static UInt32 nextSerial() noexcept
    {
        static UInt32 serial { 1 };

        if (serial == std::numeric_limits<UInt32>::max())
            serial = 1;
        else
            serial++;

        return serial;
    }

    /**
     * @brief Milliseconds
     *
     * Monotonic time with a granularity of milliseconds and an undefined base.
     */
    static UInt32 ms() noexcept;

    /**
     * @brief Microseconds
     *
     * Monotonic time with a granularity of microseconds and an undefined base.
     */
    static UInt32 us() noexcept;

    /**
     * @brief Nanoseconds
     *
     * Monotonic time with nanosecond granularity and undefined base.
     */
    static timespec ns() noexcept;
};

#endif // LTIME_H
