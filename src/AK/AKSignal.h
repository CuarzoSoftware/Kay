#ifndef AKSIGNAL_H
#define AKSIGNAL_H

#include <AK/AK.h>
#include <cassert>
#include <functional>
#include <vector>

class AK::AKListener
{
public:
    ~AKListener();

    bool wasNotified() const noexcept
    {
        return notified;
    }
protected:
    AKListener(AKObject *object, AKSignalBase *signal) noexcept;
    friend class AKSignalBase;
    AKSignalBase *signal;
    AKObject *object;
    size_t signalLink, objectLink;
    bool notified { false };
};

template<class...Args>
class AK::AKListenerTemplate : public AKListener
{
public:
    AKListenerTemplate(AKObject *object, AKSignalBase *signal, const std::function<void(Args...)> &callback) noexcept :
        AKListener(object, signal),
        m_callback(callback)
    {}

    const std::function<void(Args...)> &callback() const noexcept
    {
        return m_callback;
    }
private:
    std::function<void(Args...)> m_callback;
};

class AK::AKSignalBase
{
public:
    ~AKSignalBase() noexcept
    {
        while (!listeners.empty())
            delete listeners.back();
    }
protected:
    AKSignalBase() noexcept = default;
    friend class AKListener;
    std::vector<AKListener*> listeners;
    bool changed { true };
    void setNotified(AKListener &listener, bool state) noexcept
    {
        listener.notified = state;
    }
};

template<class...Args>
class AK::AKSignal : public AKSignalBase
{
public:

    AKListener *subscribe(AKObject *listenerOwner, const std::function<void(Args...)> &callback) noexcept
    {
        assert("Invalid subscriber object" && listenerOwner);
        listeners.push_back(new AKListenerTemplate<Args...>(listenerOwner, this, callback));
        return listeners.back();
    }

    void notify(Args...data)
    {
        for (auto *listener : listeners)
            setNotified(*listener, false);

        retry:
        changed = false;

        for (auto *listener : listeners)
        {
            if (listener->wasNotified())
                continue;

            setNotified(*listener, true);
            static_cast<AK::AKListenerTemplate<Args...>*>(listener)->callback()(data...);

            if (changed)
                goto retry;
        }
    }
};

#endif // AKSIGNAL_H
