#include <AK/AKTarget.h>
#include <AK/nodes/AKNode.h>
#include <AK/AKScene.h>
#include <AK/AKPainter.h>

AK::AKTarget::AKTarget(AKScene *scene, std::shared_ptr<AKPainter> painter) noexcept :
    m_scene(scene),
    m_painter(painter)
{
    m_nodes.reserve(64);
    m_sceneLink = scene->m_targets.size();

    if (!m_painter)
        m_painter = AKPainter::Make();
}

AK::AKTarget::~AKTarget()
{
    for (auto *node : m_nodes)
        node->m_targets.erase(this);

    m_scene->m_targets[m_sceneLink] = m_scene->m_targets.back();
    m_scene->m_targets.back()->m_sceneLink = m_sceneLink;
    m_scene->m_targets.pop_back();
}
