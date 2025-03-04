#include <AK/AKApplication.h>
#include <AK/AKTimer.h>

using namespace AK;

AKTimer::AKTimer(const Callback &callback) noexcept :
    m_callback(callback),
    m_oneShoot(false)
{
    init();
}

void AKTimer::OneShoot(UInt64 timeoutMs, const Callback &callback)
{
    if (!callback)
        return;

    new AKTimer(true, callback, timeoutMs);
}

AKTimer::~AKTimer() noexcept
{
    if (m_source)
        akApp()->removeEventSource(m_source);
}

void AKTimer::start(UInt64 timeoutMs)
{
    m_timeoutMs = timeoutMs;

    if (!m_callback)
        return;

    m_running = true;

    if (timeoutMs == 0)
    {
        constexpr itimerspec timeout { {0, 0}, {0, 1} };
        timerfd_settime(m_source->fd(), 0, &timeout, nullptr);
    }
    else
    {
        const itimerspec timeout ( {0, 0}, {static_cast<__time_t>(timeoutMs / 1000), static_cast<__syscall_slong_t>((timeoutMs % 1000) * 1000000)} );
        timerfd_settime(m_source->fd(), 0, &timeout, nullptr);
    }
}

void AKTimer::stop(bool notifyIfRunning)
{
    if (!m_running)
        return;

    m_running = false;
    constexpr itimerspec timeout { {0, 0}, {0, 0} };
    timerfd_settime(m_source->fd(), 0, &timeout, nullptr);

    AKWeak<AKTimer> ref { this };

    if (notifyIfRunning && m_callback)
        m_callback(this);

    if (ref && !m_running && m_oneShoot)
        delete this;
}

AKTimer::AKTimer(bool oneShoot, const Callback &callback, UInt64 timeoutMs) noexcept :
    m_callback(callback),
    m_oneShoot(oneShoot)
{
    init();
    start(timeoutMs);
}

void AKTimer::init() noexcept
{
    assert("AKObjects must be created after AKApplication" && akApp());

    m_source = akApp()->addEventSource(timerfd_create(CLOCK_MONOTONIC, TFD_CLOEXEC | TFD_NONBLOCK), EPOLLIN, [this](int fd, UInt32 events) {

        if (events & EPOLLIN)
        {
            constexpr itimerspec timeout { {0, 0}, {0, 0} };
            timerfd_settime(fd, 0, &timeout, nullptr);
            UInt64 expirations;
            read(fd, &expirations, sizeof(expirations));

            const bool wasRunning { m_running };
            AKWeak<AKTimer> ref { this };
            m_running = false;

            if (wasRunning && m_callback)
                m_callback(this);

            if (ref && !m_running && m_oneShoot)
                delete this;
        }
    });
}
