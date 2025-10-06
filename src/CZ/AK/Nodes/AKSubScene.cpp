#include <CZ/skia/core/SkCanvas.h>
#include <CZ/AK/Events/AKBakeEvent.h>
#include <CZ/AK/Nodes/AKSubScene.h>
#include <CZ/Ream/RSurface.h>
#include <CZ/AK/AKLog.h>

#include <CZ/AK/Nodes/AKButton.h>

using namespace CZ;

AKSubScene::AKSubScene(AKNode *parent) noexcept : AKBakeable(parent)
{
    m_caps |= SubSceneBit;
    m_scene->setRoot(this);

    // Nodes are always clipped by the RSurface bounds
    enableChildrenClipping(true);

    m_target = m_scene->makeTarget();

    // The parent scene already does this
    m_target->layoutOnRender = false;

    m_target->onMarkedDirty.subscribe(this, [this](AKTarget &) {
        addChange(CHLayout);
        m_target->m_isDirty = false;
    });
}

void AKSubScene::bakeChildren(const AKBakeEvent &event) noexcept
{
    TargetData *parentTargetData { tData };

    if (!m_target->isDirty() && event.damage.isEmpty() && parentTargetData->changes.none())
        return;

    auto geo { event.surface->geometry() };
    geo.viewport.offsetTo(worldRect().x(), worldRect().y());
    event.surface->setGeometry(geo);

    // Viewport and dst are set by the parent scene
    m_target->age = parentTargetData->changes.test(CHSize) ? 0 : 1;
    m_target->setBakedNodesScale(scale());
    m_target->surface = event.surface;
    m_target->inDamage = &event.damage;
    m_target->outDamage = &event.damage;
    m_target->outOpaque = &event.opaque;
    m_target->outInvisible = &invisibleRegion;
    // TODO: m_target->inClipRegion = params->clip;

    m_scene->render(m_target);
    tData = parentTargetData;
}


