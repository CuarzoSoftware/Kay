#include <cassert>
#include <include/gpu/GrDirectContext.h>
#include <include/core/SkCanvas.h>
#include <AK/nodes/AKSubScene.h>
#include <AK/AKSurface.h>

using namespace AK;

AKSubScene::AKSubScene(AKNode *parent) noexcept : AKBakeable(parent)
{
    enableChildrenClipping(true);
    m_caps |= Scene;
}

void AKSubScene::bakeChildren(OnBakeParams *params) noexcept
{
    TargetData *parentTargetData { t };
    AKTarget *target;
    bool isNewTarget;

    if (m_sceneTargets.find(t->target) == m_sceneTargets.end())
    {
        target = m_scene.createTarget(parentTargetData->target->painter());
        target->AKObject::on.destroyed.subscribe(this, [this](AKObject *obj){
            AKTarget *target { static_cast<AKTarget*>(obj) };
            m_sceneTargets.erase(target);
        });

        target->on.markedDirty.subscribe(this, [this](AKTarget &){
            addChange(Chg_Layout);
        });
        target->setRoot(this);
        target->m_isSubScene = true;
        m_sceneTargets[t->target] = target;
        isNewTarget = true;
    }
    else {
        target = m_sceneTargets[t->target];
        isNewTarget = false;
    }

    target->setAge((isNewTarget) ? 0 : 1);
    target->setSurface(params->surface->surface());
    target->setViewport(SkRect::MakeWH(params->surface->size().width(), params->surface->size().height()));
    target->setDstRect(params->surface->imageSrcRect());
    //target->inClipRegion = params->clip;
    target->inDamageRegion = params->damage;
    target->outDamageRegion = params->damage;
    target->outOpaqueRegion = params->opaque;

    SkCanvas &c { *params->surface->surface()->getCanvas() };
    c.save();
    c.resetMatrix();
    m_scene.render(target);
    c.restore();
    t = parentTargetData;
}

void AKSubScene::onBake(OnBakeParams *params)
{
    bakeChildren(params);
}
