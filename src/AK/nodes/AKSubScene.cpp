#include "AK/AKTime.h"
#include <cassert>
#include <include/gpu/GrDirectContext.h>
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
    m_caps |= Scene;
}

void AKSubScene::bakeChildren(OnBakeParams *params) noexcept
{
    TargetData *parentTargetData { t };
    const bool isNewTarget { m_target == nullptr };

    if (isNewTarget)
    {
        m_target = m_scene.createTarget();

        m_target->on.markedDirty.subscribe(this, [this](AKTarget &){
            addChange(Chg_Layout);
            m_target->m_isDirty = false;
        });
    }

    if (!m_target->isDirty() && params->damage->isEmpty() && parentTargetData->changes.none())
        return;

    m_target->enableUpdateLayout(false);
    m_target->setAge((isNewTarget) ? 0 : 1);
    m_target->setBakedComponentsScale(scale());
    m_target->setSurface(params->surface->surface());
    m_target->setViewport(SkRect::MakeWH(params->surface->size().width(), params->surface->size().height()));
    m_target->setDstRect(params->surface->imageSrcRect());
    //target->inClipRegion = params->clip;
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
