#include <AK/AKSceneTarget.h>
#include <AK/nodes/AKNode.h>
#include <AK/AKScene.h>
#include <AK/AKPainter.h>

using namespace AK;

void AKSceneTarget::setViewport(const SkRect &viewport) noexcept
{
    if (m_viewport == viewport)
        return;

    m_viewport = viewport;
    m_needsFullRepaint = true;

    if (!m_scene->isSubScene())
    {
        if (m_scene->root())
            m_globalIViewport = SkRect::MakeXYWH(
                m_viewport.x() + float(m_scene->root()->globalRect().x()),
                m_viewport.y() + float(m_scene->root()->globalRect().y()),
                m_viewport.width(), m_viewport.height()).roundOut();
        else
            m_globalIViewport = SkRect::MakeXYWH(
                m_viewport.x(),
                m_viewport.y(),
                m_viewport.width(), m_viewport.height()).roundOut();
    }

    markDirty();
}

void AKSceneTarget::setBakedComponentsScale(Int32 scale) noexcept
{
    if (scale == m_bakedComponentsScale)
        return;

    m_bakedComponentsScale = scale;
    markDirty();
}

void AKSceneTarget::markDirty() noexcept
{
    if (isDirty())
        return;

    m_isDirty = true;
    on.markedDirty.notify(*this);
}

AKSceneTarget::AKSceneTarget(AKScene *scene) noexcept :
    AKTarget(),
    m_scene(scene)
{
    m_sceneLink = scene->m_targets.size();
}

AKSceneTarget::~AKSceneTarget()
{
    m_scene->m_targets[m_sceneLink] = m_scene->m_targets.back();
    m_scene->m_targets.back()->m_sceneLink = m_sceneLink;
    m_scene->m_targets.pop_back();
    notifyDestruction();
}
