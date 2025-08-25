#ifndef MPROXY_H
#define MPROXY_H

#include <CZ/Marco/Marco.h>
#include <CZ/AK/AKObject.h>
#include <wayland-client.h>

template<class T>
class CZ::MProxy : public AKObject
{
public:
    MProxy(void *proxy = nullptr, UInt32 name = 0) noexcept :
        m_proxy(proxy),
        m_name(name){}

    UInt32 version() noexcept
    {
        return m_proxy ? wl_proxy_get_version((wl_proxy*)m_proxy) : 0;
    }

    void set(void *proxy, UInt32 name = 0) noexcept
    {
        m_proxy = proxy;
        m_name = name;
    }

    T *get() const noexcept
    {
        return static_cast<T*>(m_proxy);
    }

    void setName(UInt32 name) noexcept
    {
        m_name = name;
    }

    UInt32 name() const noexcept
    {
        return m_name;
    }

    operator T *() const noexcept
    {
        return get();
    }

    void reset() noexcept
    {
        m_proxy = nullptr;
        m_name = 0;
    }
private:
    void *m_proxy;
    UInt32 m_name;
};

#endif // MPROXY_H
