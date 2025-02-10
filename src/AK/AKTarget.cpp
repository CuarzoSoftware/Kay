#include <AK/AKTarget.h>
#include <AK/nodes/AKNode.h>
#include <AK/AKScene.h>
#include <AK/AKPainter.h>

void AK::AKTarget::setViewport(const SkRect &viewport) noexcept
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

void AK::AKTarget::setBakedComponentsScale(Int32 scale) noexcept
{
    if (scale == m_bakedComponentsScale)
        return;

    m_bakedComponentsScale = scale;
    markDirty();
}

void AK::AKTarget::markDirty() noexcept
{
    if (isDirty())
        return;

    m_isDirty = true;
    on.markedDirty.notify(*this);
}

AK::AKTarget::AKTarget(AKScene *scene) noexcept :
    m_scene(scene)
{
    m_sceneLink = scene->m_targets.size();
}

AK::AKTarget::~AKTarget()
{
    m_scene->m_targets[m_sceneLink] = m_scene->m_targets.back();
    m_scene->m_targets.back()->m_sceneLink = m_sceneLink;
    m_scene->m_targets.pop_back();
}
