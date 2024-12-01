#include <AK/AKTarget.h>
#include <AK/AKNode.h>
#include <AK/AKScene.h>

AK::AKTarget::AKTarget(AKScene *scene) noexcept : m_scene(scene)
{
    m_nodes.reserve(64);
    m_sceneLink = scene->m_targets.size();
}

AK::AKTarget::~AKTarget()
{
    for (auto *node : m_nodes)
        node->m_targets.erase(this);

    m_scene->m_targets[m_sceneLink] = m_scene->m_targets.back();
    m_scene->m_targets.back()->m_sceneLink = m_sceneLink;
    m_scene->m_targets.pop_back();
}
