#include <AKTarget.h>
#include <AKNode.h>

AK::AKTarget::AKTarget(AKScene *scene) noexcept : m_scene(scene)
{
    m_nodes.reserve(64);
}

AK::AKTarget::~AKTarget()
{
    for (auto *node : m_nodes)
        node->m_targets.erase(this);
}
