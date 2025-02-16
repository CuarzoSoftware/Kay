#include "AK/AKTime.h"
#include <cassert>
#include <include/core/SkCanvas.h>
#include <AK/nodes/AKSubScene.h>
#include <AK/AKSurface.h>
#include <AK/AKLog.h>

using namespace AK;

AKSubScene::AKSubScene(AKNode *parent) noexcept : AKBakeable(parent)
{
    m_scene.m_isSubScene = true;
    m_scene.setRoot(this);
    enableChildrenClipping(true);
    m_target = m_scene.createTarget();
    m_target->setRenderCalculatesLayout(false);
    m_target->on.markedDirty.subscribe(this, [this](AKTarget &){
        addChange(CHLayout);
        m_target->m_isDirty = false;
    });

    m_caps |= Scene;
}

void AKSubScene::bakeChildren(OnBakeParams *params) noexcept
{
    TargetData *parentTargetData { t };

    if (!m_target->isDirty() && params->damage->isEmpty() && parentTargetData->changes.none())
        return;

    m_target->setAge(params->damage->isEmpty() ? 1 : 0);
    m_target->setBakedComponentsScale(scale());
    m_target->setSurface(params->surface->surface());
    m_target->setViewport(SkRect::MakeWH(params->surface->size().width(), params->surface->size().height()));
    m_target->setDstRect(params->surface->imageSrcRect());
    //m_target->inClipRegion = params->clip;
    m_target->inDamageRegion = params->damage;
    m_target->outDamageRegion = params->damage;
    m_target->outOpaqueRegion = params->opaque;
    SkCanvas &c { *params->surface->surface()->getCanvas() };
    c.save();
    c.resetMatrix();
    m_scene.render(m_target);
    c.restore();
    t = parentTargetData;
}

void AKSubScene::onBake(OnBakeParams *params)
{
    bakeChildren(params);
}
