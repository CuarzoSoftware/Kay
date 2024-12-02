#include <AK/AKObject.h>
#include <AK/AKWeak.h>

using namespace AK;

std::vector<void *> &AKWeakUtils::objectRefs(const AKObject *object) noexcept
{
    return object->m_weakRefs;
}

bool AKWeakUtils::isObjectDestroyed(const AKObject *object) noexcept
{
    return object->m_destroyed;
}
