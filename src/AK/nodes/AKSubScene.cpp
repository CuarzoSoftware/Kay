#include <include/gpu/GrDirectContext.h>
#include <include/core/SkCanvas.h>
#include <AK/nodes/AKSubScene.h>
#include <AK/AKSurface.h>

using namespace AK;

AKSubScene::AKSubScene(AKNode *parent) noexcept : AKBakeable(parent)
{
    setClipsChildren(true);
    m_caps |= Scene;
}

void AKSubScene::bakeChildren(OnBakeParams *params) noexcept
{
    TargetData *parentTargetData { t };
    AKTarget *target;
    bool isNewTarget;

    if (m_targets.find(t->target) == m_targets.end())
    {
        target = m_scene.createTarget(parentTargetData->target->painter());
        target->root = this;
        target->m_isSubScene = true;
        m_targets[t->target] = target;
        isNewTarget = true;
    }
    else {
        target = m_targets[t->target];
        isNewTarget = false;
    }


    target->age = isNewTarget ? 0 : 1;
    target->surface = params->surface->surface();
    target->viewport = SkRect::MakeWH(params->surface->size().width(), params->surface->size().height());
    //target->scale = t->target->scale;
    target->dstRect = params->surface->imageSrcRect();
    target->inClipRegion = params->clip;
    target->inDamageRegion = params->damage;
    target->outDamageRegion = params->damage;
    target->outOpaqueRegion = params->opaque;

    SkCanvas &c { *params->surface->surface()->getCanvas() };
    c.save();
    c.setMatrix(SkMatrix());
    m_scene.render(target);
    c.restore();
    t = parentTargetData;
}

void AKSubScene::onBake(OnBakeParams *params)
{
    //canvas->clear(SkScalarRoundToInt(t->target->m_xyScale.x()) == 2 ? SK_ColorGREEN : SK_ColorYELLOW);
    bakeChildren(params);
}
