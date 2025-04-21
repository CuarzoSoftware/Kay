#include <include/core/SkCanvas.h>
#include <AK/events/AKBakeEvent.h>
#include <AK/nodes/AKSubScene.h>
#include <AK/AKSurface.h>
#include <AK/AKLog.h>

using namespace AK;

AKSubScene::AKSubScene(AKNode *parent) noexcept : AKBakeable(parent)
{
    m_scene.setRoot(this);
    enableChildrenClipping(true);
    m_target = m_scene.createTarget();
    m_target->setRenderCalculatesLayout(false);
    m_target->on.markedDirty.subscribe(this, [this](AKSceneTarget &){
        addChange(CHLayout);
        m_target->m_isDirty = false;
    });

    m_caps |= Scene;
}

void AKSubScene::bakeChildren(const AKBakeEvent &event) noexcept
{
    TargetData *parentTargetData { t };

    if (!m_target->isDirty() && event.damage.isEmpty() && parentTargetData->changes.none())
        return;

    m_target->setAge(event.damage.isEmpty() ? 1 : 0);
    m_target->setBakedComponentsScale(scale());
    m_target->setSurface(event.surface.surface());
    m_target->setViewport(SkRect::MakeWH(event.surface.size().width(), event.surface.size().height()));
    m_target->setDstRect(event.surface.imageSrcRect());
    //m_target->inClipRegion = params->clip;
    m_target->inDamageRegion = &event.damage;
    m_target->outDamageRegion = &event.damage;
    m_target->outOpaqueRegion = &event.opaque;
    m_target->outInvisibleRegion = &invisibleRegion;
    event.canvas().save();
    event.canvas().resetMatrix();
    m_scene.render(m_target);
    event.canvas().restore();
    t = parentTargetData;
}

void AKSubScene::bakeEvent(const AKBakeEvent &event)
{
    bakeChildren(event);
}
