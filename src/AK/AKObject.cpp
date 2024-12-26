#include <AK/AKObject.h>
#include <AK/AKWeak.h>
#include <AK/AKSignal.h>

using namespace AK;

AKObject::~AKObject() noexcept
{
    notifyDestruction();
}

void AKObject::notifyDestruction() noexcept
{
    if (m_destroyed)
        return;

    m_destroyed = true;
    on.destroyed.notify(this);

    while (!m_listeners.empty())
        delete m_listeners.back();

    while (!m_weakRefs.empty())
    {
        AKWeak<AKObject> *weak { (AKWeak<AKObject>*)m_weakRefs.back() };
        weak->m_object = nullptr;
        m_weakRefs.pop_back();

        if (weak->m_onDestroyCallback)
            (*weak->m_onDestroyCallback)(this);
    }
}
