#include <AKObject.h>
#include <AKWeak.h>

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

    while (!m_weakRefs.empty())
    {
        AKWeak<AKObject> *weak { (AKWeak<AKObject>*)m_weakRefs.back() };
        weak->m_object = nullptr;
        m_weakRefs.pop_back();

        if (weak->m_onDestroyCallback)
            (*weak->m_onDestroyCallback)(this);
    }
}
